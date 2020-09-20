#!/bin/bash
#
# Run tests directly from buildtree 
# by setting LD_LIBRARY_PATH.
#

LD_LIBRARY_PATH="../" ./evfd $@
