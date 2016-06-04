#!/bin/bash

#
# Define expectations
#
EXPECTED_GCC_VERSION="4.8"
EXPECTED_LIBOBOE_VERSION="2.0"


#
# Define helpers
#
EXIT_CODE=0

semver() {
  test "$(echo "$@" | tr " " "\n" | sed 's/^[0-9]\./0&/; s/\.\([0-9]\)$/.0\1/; s/\.\([0-9]\)\./.0\1./g; s/\.\([0-9]\)\./.0\1./g' | sort | sed 's/^0// ; s/\.0/./g'  | head -n 1)" != "$1"
}


#
# Detect build tool presence and version
#
if ! type gcc &> /dev/null; then
  echo "Unable to locate gcc."
  EXIT_CODE=1
else
  FOUND_GCC_VERSION=$(gcc -dumpversion)

  if semver $EXPECTED_GCC_VERSION $FOUND_GCC_VERSION; then
    echo "Expected gcc $EXPECTED_GCC_VERSION+, found $FOUND_GCC_VERSION."
    EXIT_CODE=1
  fi
fi


#
# Detect liboboe presence and version
#
OUTPUT=$(ldconfig -p 2> /dev/null | grep liboboe 2> /dev/null)
if [ -z "$OUTPUT" ]; then
  echo "Unable to locate liboboe."
  EXIT_CODE=1
else
  # NOTE: Can't use ldconfig output for version detection,
  # as the library may be numbered incorrectly.
  cat > /tmp/test-version.c <<- EOM
#include <stdio.h>
#include "oboe/oboe.h"
int main(int argc, char *argv[]) {
	puts(oboe_config_get_version_string());
	return 0;
}
EOM
  gcc -o /tmp/test-version /tmp/test-version.c -loboe
  FOUND_LIBOBOE_VERSION=$(/tmp/test-version)
  rm /tmp/test-version.c /tmp/test-version

  if semver $EXPECTED_LIBOBOE_VERSION $FOUND_LIBOBOE_VERSION; then
    echo "Expected liboboe $EXPECTED_LIBOBOE_VERSION+, found $FOUND_LIBOBOE_VERSION."
    EXIT_CODE=1
  fi
fi

exit $EXIT_CODE
