#!/bin/sh

# shellcheck disable=SC3043

WORKSPACE_DIR=${1:-${WORKSPACE_DIR:-/workspace}}

if [ ! -f "${WORKSPACE_DIR}/00.common/shell-lib/common.sh" ]; then
  echo "ERROR: common.sh not found in directory ${WORKSPACE_DIR}/00.common/shell-lib" >&2
  exit 101
fi

# shellcheck source=SCRIPTDIR/../../00.common/shell-lib/common.sh
. "${WORKSPACE_DIR}/00.common/shell-lib/common.sh"

SRC_DIR="${WORKSPACE_DIR}/01.src"
CACHE_DIR="${WORKSPACE_DIR}/02.pre-commit/01.local-build/cache"

mkdir -p "${CACHE_DIR}"

# GCC static
logI "Building gcc-default-server-static.bin..."
gcc "${SRC_DIR}/main.c"\
  -static \
  -o "${CACHE_DIR}/gcc-default-server-static.bin"

result1=$?
if [ $result1 -ne 0 ]; then
  logE "Failed to build gcc-default-server-static.bin, result code ${result1}"
  exit 1
fi
MIN_FILE_SIZE=$(stat -c %s "${CACHE_DIR}/gcc-default-server-static.bin")
logD "gcc-default-server-static.bin size: ${MIN_FILE_SIZE}"

cp "${CACHE_DIR}/gcc-default-server-static.bin" "${CACHE_DIR}/server-static.bin"

# clang static
logI "Building gcc-default-server-static.bin..."
clang "${SRC_DIR}/main.c"\
  -static \
  -o "${CACHE_DIR}/clang-default-server-static.bin"

result2=$?
if [ $result2 -ne 0 ]; then
  logE "Failed to build clang-default-server-static.bin, result code ${result2}"
  exit 2
fi

THIS_FILE_SIZE=$(stat -c %s "${CACHE_DIR}/clang-default-server-static.bin")
logD "clang-default-server-static.bin size: ${THIS_FILE_SIZE}"

if [ "$THIS_FILE_SIZE" -lt "$MIN_FILE_SIZE" ]; then
  logD "clang-default-server-static.bin is smaller than gcc-default-server-static.bin"
  logD "Replacing gcc-default-server-static.bin with clang-default-server-static.bin"
  cp "${CACHE_DIR}/clang-default-server-static.bin" "${CACHE_DIR}/server-static.bin"
fi

logI "Granting execution permission to ${CACHE_DIR}/*.bin..."
chmod u+x "${CACHE_DIR}"/*.bin

TODO: find out how to optimize the size of the binary files.
