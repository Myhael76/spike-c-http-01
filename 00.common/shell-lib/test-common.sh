#!/bin/sh

# shellcheck source-path=SCRIPTDIR

. ./common.sh

logI "1 - logI qualifier only should be green"
logE "2 - logE should be red"
logW "3 - logW should be cyan"
logD "4 - logD should be yellow"

export COLORED_OUTPUT="no"
. ./common.sh

logI "5 - logI should NOT be green"
logE "6 - logE should NOT be red"
logW "7 - logW should NOT be cyan"
logD "8 - logD should NOT be yellow"

export COLORED_OUTPUT="yes"
. ./common.sh

logI "11 - logI qualifier only should be green"
logE "12 - logE should be red"
logW "13 - logW should be cyan"
logD "14 - logD should be yellow"

unset COLORED_OUTPUT
. ./common.sh

logI "21 - logI qualifier only should be green"
logE "22 - logE should be red"
logW "23 - logW should be cyan"
logD "24 - logD should be yellow"
