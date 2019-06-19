#!/bin/bash

NUM_CPU=$(nproc)

if [ -z "$DECIMATION" ]
then
    DECIMATION=60
fi

if [ -z "$FILTER" ]
then
    PROJECT_DIR=$(realpath "$(dirname "$0")"/..)
    FILTER="$PROJECT_DIR"/rasseiver/examples/LPDFilter.fcf
fi

if [ -z "$PORT" ]
then
   PORT=10001
fi

echo "Make a PGO build of rasseiver"
echo
echo "==== STAGE 1 ===="
echo "Build instrumented executable"

cmake "$@" -DPROFILE_GENERATE=1 -DPROFILE_USE=0
make -j$NUM_CPU

echo
echo "==== STAGE 2 ===="
echo "Run instrumented executable"

cat >pgo-config <<EOF
device=dummy
count=20
port=$PORT
filter=$FILTER
decimation=$DECIMATION
EOF

if [ -z "$HOST" ]
then
    socat /dev/null,ignoreeof tcp-listen:$PORT &
else
    echo "host=$HOST" >>pgo-config
fi

rasseiver/rasseiver pgo-config

rm pgo-config

echo
echo "==== STAGE 3 ===="
echo "Rebuild executable with generated profile"

cmake "$@" -DPROFILE_GENERATE=0 -DPROFILE_USE=1
make -j$NUM_CPU

echo "Done."
