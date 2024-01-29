#!/bin/sh

./buildC.sh || exit 1
./buildDocker.sh || exit 2

