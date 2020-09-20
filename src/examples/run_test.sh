#!/bin/bash
#
# Run tests directly from buildtree 
# by setting LD_LIBRARY_PATH.
#

if [ -z "$1" ]; then
	echo "Run tests directly from buildtree by"
	echo "setting LD_LIBRARY_PATH."
	echo 
	echo "Usage: ./run_test.sh test_name [params]"
	exit 1
fi

TEST=$1
shift

LD_LIBRARY_PATH="../" ./$TEST $@
