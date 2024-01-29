// inspired by https://github.com/JeffreytheCoder/Simple-HTTP-Server

#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <regex.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static volatile int keepRunning = 1;

void intHandler(int dummy) {
  printf("Interrupted, exiting...\n");
  keepRunning = 0;
}


#define PORT 8080
#define BUFFER_SIZE 104857600
#define MAX_CON 10
#define MAX_EVENTS 10

void log_immediate(const char *message)
{
  printf("%s\n", message);
  fflush(stdout);
}

const char *get_file_extension(const char *file_name)
{
  const char *dot = strrchr(file_name, '.');
  if (!dot || dot == file_name)
  {
    return "";
  }
  return dot + 1;
}

const char *get_mime_type(const char *file_ext)
{
  if (strcasecmp(file_ext, "html") == 0 || strcasecmp(file_ext, "htm") == 0)
  {
    return "text/html";
  }
  else if (strcasecmp(file_ext, "txt") == 0)
  {
    return "text/plain";
  }
  else if (strcasecmp(file_ext, "jpg") == 0 || strcasecmp(file_ext, "jpeg") == 0)
  {
    return "image/jpeg";
  }
  else if (strcasecmp(file_ext, "png") == 0)
  {
    return "image/png";
  }
  else
  {
    return "application/octet-stream";
  }
}

bool case_insensitive_compare(const char *s1, const char *s2)
{
  while (*s1 && *s2)
  {
    if (tolower((unsigned char)*s1) != tolower((unsigned char)*s2))
    {
      return false;
    }
    s1++;
    s2++;
  }
  return *s1 == *s2;
}

char *get_file_case_insensitive(const char *file_name)
{
  DIR *dir = opendir(".");
  if (dir == NULL)
  {
    perror("opendir");
    return NULL;
  }

  struct dirent *entry;
  char *found_file_name = NULL;
  while ((entry = readdir(dir)) != NULL)
  {
    if (case_insensitive_compare(entry->d_name, file_name))
    {
      found_file_name = entry->d_name;
      break;
    }
  }

  closedir(dir);
  return found_file_name;
}

char *url_decode(const char *src)
{
  size_t src_len = strlen(src);
  char *decoded = malloc(src_len + 1);
  size_t decoded_len = 0;

  // decode %2x to hex
  for (size_t i = 0; i < src_len; i++)
  {
    if (src[i] == '%' && i + 2 < src_len)
    {
      int hex_val;
      sscanf(src + i + 1, "%2x", &hex_val);
      decoded[decoded_len++] = hex_val;
      i += 2;
    }
    else
    {
      decoded[decoded_len++] = src[i];
    }
  }

  // add null terminator
  decoded[decoded_len] = '\0';
  return decoded;
}

void build_http_response(const char *file_name,
                         const char *file_ext,
                         char *response,
                         size_t *response_len)
{
  // build HTTP header
  const char *mime_type = get_mime_type(file_ext);
  char *header = (char *)malloc(BUFFER_SIZE * sizeof(char));
  snprintf(header, BUFFER_SIZE,
           "HTTP/1.1 200 OK\r\n"
           "Content-Type: %s\r\n"
           "\r\n",
           mime_type);

  // if file not exist, response is 404 Not Found
  int file_fd = open(file_name, O_RDONLY);
  if (file_fd == -1)
  {
    snprintf(response, BUFFER_SIZE,
             "HTTP/1.1 404 Not Found\r\n"
             "Content-Type: text/plain\r\n"
             "\r\n"
             "404 Not Found");
    *response_len = strlen(response);
    return;
  }

  // get file size for Content-Length
  struct stat file_stat;
  fstat(file_fd, &file_stat);
  off_t file_size = file_stat.st_size;

  // copy header to response buffer
  *response_len = 0;
  memcpy(response, header, strlen(header));
  *response_len += strlen(header);

  // copy file to response buffer
  ssize_t bytes_read;
  while ((bytes_read = read(file_fd,
                            response + *response_len,
                            BUFFER_SIZE - *response_len)) > 0)
  {
    *response_len += bytes_read;
  }
  free(header);
  close(file_fd);
}

void *handle_client(void *arg)
{
  log_immediate("handle_client - 1");
  int client_fd = *((int *)arg);

  char *buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));

  log_immediate("Client connected\n");
  // receive request data from client and store into buffer
  ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
  printf("Received %ld bytes\n", bytes_received);
  printf("Request:\n%s\n", buffer);
  log_immediate("2");
  if (bytes_received > 0)
  {
    // check if request is GET
    regex_t regex;
    regcomp(&regex, "^GET /([^ ]*) HTTP/1", REG_EXTENDED);
    regmatch_t matches[2];

    if (regexec(&regex, buffer, 2, matches, 0) == 0)
    {
      // extract filename from request and decode URL
      buffer[matches[1].rm_eo] = '\0';
      const char *url_encoded_file_name = buffer + matches[1].rm_so;
      char *file_name = url_decode(url_encoded_file_name);

      // get file extension
      char file_ext[32];
      strcpy(file_ext, get_file_extension(file_name));

      // build HTTP response
      char *response = (char *)malloc(BUFFER_SIZE * 2 * sizeof(char));
      size_t response_len;
      build_http_response(file_name, file_ext, response, &response_len);

      // send HTTP response to client
      send(client_fd, response, response_len, 0);

      free(response);
      free(file_name);
    }
    regfree(&regex);
  }
  close(client_fd);
  free(arg);
  free(buffer);
  return NULL;
}

int main(int argc, char *argv[])
{
  signal(SIGINT, intHandler);
  signal(SIGTERM, intHandler);
  int server_fd = 0;
  int counter = 0;
  struct sockaddr_in server_addr;
  struct epoll_event event;
  struct epoll_event events[MAX_EVENTS];

  // create server socket
  if ((server_fd = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, 0)) < 0)
  {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // config socket
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT);

  // bind socket to port
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
  if (bind(server_fd,
           (struct sockaddr *)&server_addr,
           sizeof(server_addr)) < 0)
  {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  //setnonblocking(server_fd);
  // listen for connections
  if (listen(server_fd, MAX_CON) < 0)
  {
    perror("listen failed");
    exit(EXIT_FAILURE);
  }

  int epfd = epoll_create(1);
  //epoll_ctl_add(epfd, server_fd, EPOLLIN | EPOLLOUT | EPOLLET);
  event.data.fd = server_fd;
  event.events = EPOLLIN | EPOLLET;
  int s = epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &event);
  if(s == -1)
  {
      perror("epoll_ctl");
      exit(EXIT_FAILURE);
  }
	struct sockaddr_in cli_addr;
  socklen_t cli_addr_len = sizeof(cli_addr);

  printf("Server listening on port %d\n", PORT);
  bool justAccepted = false;
  while (keepRunning)
  {
    log_immediate("in outer while...");
    int nfds = epoll_wait(epfd, events, MAX_EVENTS, 2000);
    printf("epoll_wait received %d events", nfds);

    for (int i = 0; keepRunning && (i < nfds); i++){
      log_immediate("in inner for...");
      if (events[i].data.fd == server_fd){
        log_immediate("in inner if...");

        // client info
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int *client_fd = malloc(sizeof(int));

        // accept client connection
        if ((*client_fd = accept(server_fd,
                                (struct sockaddr *)&client_addr,
                                &client_addr_len)) < 0)
        {
          perror("accept failed");
          continue;
        }
        log_immediate("accepted");
        // create a new thread to handle client request
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, (void *)client_fd);
        pthread_detach(thread_id);
      }
      else{
        log_immediate("in inner else...");
        printf("event fd: %d\n", events[i].data.fd);
      }
    }
  }

  close(server_fd);
  return 0;
}
