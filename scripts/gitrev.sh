#!/bin/sh
# Generates Git Rev number

SRCDIR=$1

# get git revision hash
gitrev=$(cd $SRCDIR && git describe)

# get if source tree is clean
(cd $SRCDIR && git diff --quiet)
gitclean=$?
if [ $gitclean -gt 0 ]; then
  gitrev="$gitrev+"
fi;

echo $gitrev
