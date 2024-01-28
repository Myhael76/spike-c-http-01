#!/bin/sh

# Decide if this is used interactively or not

export COLORED_OUTPUT="${COLORED_OUTPUT:-yes}"

if [ "${COLORED_OUTPUT}" = "yes" ]; then
  export NC='\e[m' 				  	# No Color
  export Red='\033[0;31m' 		  	# Red
  export Green="\x1B[32m" 			# Green
  export Yellow="\033[0;33m" 		  # Yellow
  export Blue="\033[0;34m" 		    # Blue
  export Cyan="\033[0;36m" 		  	# Cyan

  logI() {
    # shellcheck disable=SC3037
    echo -e "${Green}[INFO ]${NC} " "$@"
  }
  logE() {
    # shellcheck disable=SC3037
    echo -e "${Red}[ERROR]${NC} " "$@"
  }

  logW() {
    # shellcheck disable=SC3037
    echo -e "${Cyan}[WARN ]${NC} " "$@"
  }

  logD() {
    # shellcheck disable=SC3037
    echo -e "${Yellow}[DEBUG]${NC} " "$@"
  }
else
  logI() {
    # shellcheck disable=SC3037
    echo "[INFO ] " "$@"
  }
  logE() {
    # shellcheck disable=SC3037
    echo "[ERROR] " "$@"
  }
  logW() {
    # shellcheck disable=SC3037
    echo "[WARN ] " "$@"
  }
  logD() {
    # shellcheck disable=SC3037
    echo "[DEBUG] " "$@"
  }
fi
