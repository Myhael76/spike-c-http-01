/*
 ** epoll-web-server.c
 **
 ** Single Thread multiuser web Server
 ** epoll-web-server [port]
 ** see https://github.com/Menghongli/C-Web-Server/blob/master/epoll-server.c
 **
 */
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXEPOLLSIZE 1000
#define BACKLOG 200            // how many pending connections queue will hold
#define RECV_BUFFER_SIZE 10240 // Receiving buffer size
#define HELLO_RESPONSE "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"message\":\"hello\"}\0"

// Manage interruptions
static volatile int keepRunning = 1;
static void interruptHandler(int dummy)
{
  printf("Interrupted, signal was %d exiting...\n", dummy);
  keepRunning = 0;
}

static char *recv_buffer = NULL;

int process_request(const struct epoll_event *e)
{
  // printf("Processing a request ...\n");

  // TODO: generalize and add the performance use cases. for now probe only

  // For some reason to be determined, send doesn't work if we don't receive first
  ssize_t bytes_received = 1;
  while (0 < bytes_received)
  {
    bytes_received = recv(e->data.fd, recv_buffer, RECV_BUFFER_SIZE, 0);
    // printf("Received %ld bytes\n", bytes_received);
    // printf("Request:\n%s\n", recv_buffer);
  }

  // printf("Sending %s \n\n", HELLO_RESPONSE);
  if (-1 == send(e->data.fd, HELLO_RESPONSE, strlen(HELLO_RESPONSE), 0)) /* FlawFinder: ignore - Warns against CWE-126, but the string is \0 terminated in the constant above */
  {
    perror("send");
    printf("Error sending...\n");
    return -1;
  }
  return 0;
}

int set_non_blocking(int listening_socket_fd)
{
  int flags, s;
  flags = fcntl(listening_socket_fd, F_GETFL, 0);
  if (flags == -1)
  {
    perror("fcntl");
    return -1;
  }
  flags |= O_NONBLOCK;
  s = fcntl(listening_socket_fd, F_SETFL, flags);
  if (s == -1)
  {
    perror("fcntl");
    return -1;
  }
  return 0;
}

int main(int argc, char *argv[])
{

  signal(SIGINT, interruptHandler);
  signal(SIGTERM, interruptHandler);

  int status;
  int listening_socket_fd, new_fd, kdpfd, nfds, n, curfds;
  struct addrinfo hints;
  struct addrinfo *servinfo; // will point to the results
  struct addrinfo *p;
  struct sockaddr_storage client_addr;
  struct epoll_event ev;
  struct epoll_event *events;
  socklen_t addr_size;

  recv_buffer = (char *)malloc(RECV_BUFFER_SIZE * sizeof(char));

  static const char *port_no = NULL;

  // optionally receive port on first argument, default 8080
  if (argc < 2)
  {
    port_no = "8080";
  }
  else
  {
    port_no = argv[1];
  }

  memset(&hints, 0, sizeof hints); // make sure the struct is empty
  hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
  hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
  int optval = 1;

  // if((status = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0 ) {
  if ((status = getaddrinfo(NULL, port_no, &hints, &servinfo)) != 0)
  {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    return 2;
  }
  // servinfo now points to a linked list of 1 or more struct addrinfos
  // printf("Inspecting sockets variants..");
  uint8_t variantsNo = 0;
  for (p = servinfo; p != NULL; p = p->ai_next, variantsNo++)
  {
    // printf("p[%d]->(socket type: %d; ai_family: %d; ai_family: %d ai_protocol;) ", variantsNo, p->ai_socktype, p->ai_family, p->ai_protocol);

    // make a socket:
    if ((listening_socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
    {
      perror("server: socket");
      continue;
    }

    // make the sock non blocking
    set_non_blocking(listening_socket_fd);

    // bind it to the port
    setsockopt(listening_socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
    if ((bind(listening_socket_fd, p->ai_addr, p->ai_addrlen)) == -1)
    {
      close(listening_socket_fd);
      perror("server: bind");
      continue;
    }

    break;
  }

  if (p == NULL)
  {
    fprintf(stderr, "server: failed to bind\n");
    return 2;
  }

  freeaddrinfo(servinfo); // free the linked-list

  // listen for incoming connection
  if (listen(listening_socket_fd, BACKLOG) == -1)
  {
    perror("listen");
    exit(1);
  }

  printf("server: waiting for connections...\n");

  kdpfd = epoll_create(MAXEPOLLSIZE);
  ev.events = EPOLLIN | EPOLLET;
  ev.data.fd = listening_socket_fd;
  if (epoll_ctl(kdpfd, EPOLL_CTL_ADD, listening_socket_fd, &ev) < 0)
  {
    fprintf(stderr, "epoll set insert error.");
    return -1;
  }
  else
  {
    printf("success insert listening socket into epoll.\n");
  }
  events = (struct epoll_event *)calloc(MAXEPOLLSIZE, sizeof(struct epoll_event));
  curfds = 1;
  while (keepRunning)
  { // loop for accept incoming connection

    // printf("== BEGIN == loop for accept incoming connection\n");
    nfds = epoll_wait(kdpfd, events, curfds, 5000);
    if (nfds == -1)
    {
      perror("epoll_wait");
      break;
    }
    for (n = 0; n < nfds; ++n)
    {
      if (events[n].data.fd == listening_socket_fd)
      {
        addr_size = sizeof client_addr;
        // printf("accepting %d...\n", events[n].data.fd);
        new_fd = accept(events[n].data.fd, (struct sockaddr *)&client_addr, &addr_size);
        if (new_fd == -1)
        {
          if ((errno == EAGAIN) ||
              (errno == EWOULDBLOCK))
          {
            break;
          }
          else
          {
            perror("accept");
            break;
          }
        }
        // printf("server: connection established new_fd=%d...\n", new_fd);
        set_non_blocking(new_fd);
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = new_fd;
        if (epoll_ctl(kdpfd, EPOLL_CTL_ADD, new_fd, &ev) < 0)
        {
          printf("Failed to insert socket into epoll.\n");
        }
        curfds++;
      }
      else
      {
        process_request(&events[n]);

        // if(send(events[n].data.fd, "Hello, world!", 13, 0) == -1)
        // {
        //   perror("send");
        //   break;
        // }
        epoll_ctl(kdpfd, EPOLL_CTL_DEL, events[n].data.fd, &ev);
        curfds--;
        close(events[n].data.fd);
        // printf("closing %d...\n", events[n].data.fd);
      }
    }
    // printf("==  END  == loop for accept incoming connection\n");
  }
  // printf("Main loop ended\nFreeing events...\n");
  free(events);
  // printf("Closing listening socket...");
  // int close_result = close(listening_socket_fd);
  // printf("Result of closing the listening socket is: %d\n", close_result);
  // sleep(2);
  // printf("after final sleep...");
  return 0;
}
