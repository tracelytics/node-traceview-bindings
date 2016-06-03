#!/bin/bash

#
# Define expectations
#
EXPECTED_BUILD_TOOL_NAME="gcc"
EXPECTED_BUILD_TOOL_VERSION="4.8"
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
if ! type $EXPECTED_BUILD_TOOL_NAME > /dev/null; then
  echo "No $EXPECTED_BUILD_TOOL_NAME installed."
  EXIT_CODE=1
else
  # NOTE: This is incorrect with clang
  FOUND_BUILD_TOOL_VERSION=$($EXPECTED_BUILD_TOOL_NAME -dumpversion)

  if semver $EXPECTED_BUILD_TOOL_VERSION $FOUND_BUILD_TOOL_VERSION; then
    echo "Expected $EXPECTED_BUILD_TOOL_NAME $EXPECTED_BUILD_TOOL_VERSION+, found $FOUND_BUILD_TOOL_VERSION!"
    EXIT_CODE=1
  fi
fi


#
# Detect liboboe presence and version
#
OUTPUT=$(ldconfig -p | grep liboboe &> /dev/null)
if [ -z "$OUTPUT" ]; then
  echo "No liboboe installed."
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
    echo "Expected liboboe $EXPECTED_LIBOBOE_VERSION+, found $FOUND_LIBOBOE_VERSION!"
    EXIT_CODE=1
  fi
fi

exit $EXIT_CODE
