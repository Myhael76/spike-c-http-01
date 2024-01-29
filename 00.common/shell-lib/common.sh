#!/bin/sh

# Decide if this is used interactively or not

export COLORED_OUTPUT="${COLORED_OUTPUT:-yes}"

if [ "${COLORED_OUTPUT}" = "yes" ]; then
  #set -e
  export NC='\e[m'                 # No Color
  export Red='\033[0;31m'          # Red
  export Green="\033[0;32m"        # Green
  export Yellow="\033[0;33m"       # Yellow
  export Blue="\033[0;34m"         # Blue
  export Cyan="\033[0;36m"         # Cyan

  logI() {
    printf "${Green}[INFO ]${NC} %s\n" "$@"
  }
  logE() {
    printf "${Red}[ERROR] %s${NC}\n" "$@"
  }
  logW() {
    printf "${Cyan}[WARN ] %s${NC}\n" "$@"
  }
  logD() {
    printf "${Yellow}[DEBUG] %s${NC}\n" "$@"
  }
else
  logI() {
    printf "[INFO ] %s\n" "$@"
  }
  logE() {
    printf "[ERROR] %s\n" "$@"
  }
  logW() {
    printf "[WARN ] %s\n" "$@"
  }
  logD() {
    printf "[DEBUG] %s\n" "$@"
  }
fi
