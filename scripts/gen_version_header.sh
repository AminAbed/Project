#!/bin/bash
SRCDIR=$1
DESTFILE=$2

SCRIPTDIR=$( cd "$( dirname "$0" )" && pwd )

gitrev=`$SCRIPTDIR/gitrev.sh $SRCDIR`
date=`date`

cat <<EOF > $DESTFILE
#define D_BUILD_NO "$gitrev"
#define D_BUILD_DATE "$date"
EOF

