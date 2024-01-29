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
rm -f "${CACHE_DIR}"/*.bin

buildWithOptions(){

  local compilerName="${1:-gcc}"
  local compilerOptionsBeforeSourceFile="${2:-}"
  local sourceFile="${3:-main.c}"
  local compilerOptionsAfeterSourceFile="${4:-}"
  local binaryName="${5:-server.bin}"
  local compilerOptionsFinally="${6:-}"

  logI "Building ${sourceFile} using compiler ${compilerName}"
  logI "Output binary file                 : ${binaryName}..."
  logI "Compiler options before source file: ${compilerOptionsBeforeSourceFile}"
  logI "Compiler options after source file : ${compilerOptionsAfeterSourceFile}"
  logI "Compiler options finally           : ${compilerOptionsFinally}"

  # below shellcheck is wrong, because we want to expand the compilerOptionsBeforeSourceFile and compilerOptionsAfeterSourceFile
  # shellcheck disable=SC2086
  "${compilerName}" ${compilerOptionsBeforeSourceFile} "${sourceFile}" ${compilerOptionsAfeterSourceFile} -o "${binaryName}" ${compilerOptionsFinally}

  result=$?
  if [ $result -ne 0 ]; then
    logE "Failed to build, result code ${result}"
    return 1
  fi

  #logI "Testing the binary file ${binaryName}..."
  #"${binaryName}" || return 2

  THIS_FILE_SIZE=$(stat -c %s "${binaryName}")
  logD "${binaryName} size: ${THIS_FILE_SIZE}"

}

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
logI "Building while trying to remove dead code and unused functions, optimized for file size..."
buildWithOptions clang \
  "-O3 -fdata-sections -ffunction-sections" \
  "${SRC_DIR}/main.c" \
  "-static" \
  "${CACHE_DIR}/clang-O3-server-static.bin" \
  "" || exit 2

logI "Building while trying to remove dead code and unused functions, optimized for file size..."
buildWithOptions gcc \
  "-Os -fdata-sections -ffunction-sections" \
  "${SRC_DIR}/main.c" \
  "-static" \
  "${CACHE_DIR}/gcc-no-dc-server-static.bin" \
  "-Wl,--gc-sections" || exit 3

logI "Building while trying to remove dead code and unused functions, optimized for execution time..."
buildWithOptions gcc \
  "-O3 -fdata-sections -ffunction-sections" \
  "${SRC_DIR}/main.c" \
  "-static" \
  "${CACHE_DIR}/gcc-O3-server-static.bin" \
  "-Wl,--gc-sections" || exit 3


logI "Granting execution permission to ${CACHE_DIR}/*.bin..."
chmod u+x "${CACHE_DIR}"/*.bin

# TODO: find out how to optimize the size of the binary files.
