#!/bin/sh

# shellcheck disable=SC3043

WORKSPACE_DIR=${1:-${WORKSPACE_DIR:-/workspace}}

if [ ! -f "${WORKSPACE_DIR}/00.common/shell-lib/common.sh" ]; then
  echo "ERROR: common.sh not found in directory ${WORKSPACE_DIR}/00.common/shell-lib" >&2
  exit 101
fi

# shellcheck source=SCRIPTDIR/../../00.common/shell-lib/common.sh
. "${WORKSPACE_DIR}/00.common/shell-lib/common.sh"

CACHE_DIR="${WORKSPACE_DIR}/02.pre-commit/01.local-build/cache"

mkdir -p "${CACHE_DIR}"

staticContainerImageBuild(){
  local imageName="${1}"
  local buildDir="${2:-${CACHE_DIR}}"
  local binaryName="${3}"

  logI "Building image ${imageName} with docker using binary file ${binaryName}..."
  strip --strip-debug "${buildDir}/../${binaryName}"
  docker build \
  --build-arg __file__="${binaryName}" \
  -t "${imageName}-d" \
  -f "${buildDir}/Dockerfile-static" "${buildDir}"

  result2=$?
  if [ $result2 -ne 0 ]; then
    logE "Failed to build image ${imageName} using binary file ${binaryName}, result code ${result2}"
    return 3
  fi

  logI "Testing the image ${imageName}-d ..."
  docker run --rm "${imageName}-d" || return 4
}

staticContainerImageBuild my-hello-world-01 "${CACHE_DIR}/.." "server-static.bin"
