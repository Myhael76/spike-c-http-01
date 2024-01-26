#!/bin/sh

WORKSPACE_DIR=${1:-${WORKSPACE_DIR:-/workspace}}

mkdir -p "${WORKSPACE_DIR}/output"

gcc -static "${WORKSPACE_DIR}/src/main.c" \
  -o "${WORKSPACE_DIR}/build/server.bin"

# gcc "${WORKSPACE_DIR}/src/main.c" \
#   -o "${WORKSPACE_DIR}/build/server_d.bin"

chmod u+x "${WORKSPACE_DIR}/build/server"*.bin

docker build -t my-hello-world .

docker run --rm my-hello-world

