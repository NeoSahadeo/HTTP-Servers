#!/usr/bin/env bash

exec=${1:0:-2}
clang -O2 $1 -o $exec
# ./$exec $@
