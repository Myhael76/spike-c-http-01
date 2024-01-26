FROM mcr.microsoft.com/devcontainers/base:alpine

ARG __WORKSPACE_FOLDER=/workspace

RUN apk --no-cache update \
  && apk add --no-cache docker docker-compose \
  && addgroup vscode docker \
  && mkdir -p ${__WORKSPACE_FOLDER} \
  && chown vscode:vscode ${__WORKSPACE_FOLDER}

USER vscode

RUN cd ${__WORKSPACE_FOLDER} \
    && git config --global --add safe.directory ${__WORKSPACE_FOLDER}

VOLUME "${__WORKSPACE_FOLDER}"
