FROM mcr.microsoft.com/devcontainers/cpp:ubuntu-22.04

ARG __WORKSPACE_FOLDER=/workspace

RUN apt-get -qy update \
  && apt-get -qy install --no-install-recommends \
    clang \
    clang-format \
    clang-tidy \
    clangd \
    cmake \
    cppcheck \
    doxygen \
    gdb \
    git \
    git-lfs \
    graphviz \
    lld \
    lldb \
    valgrind \
  && rm -rf /var/lib/apt/lists/* \
  && mkdir -p ${__WORKSPACE_FOLDER} \
  && chown vscode:vscode ${__WORKSPACE_FOLDER}

USER vscode

RUN cd ${__WORKSPACE_FOLDER} \
  && git config --global --add safe.directory ${__WORKSPACE_FOLDER}

VOLUME "${__WORKSPACE_FOLDER}"
