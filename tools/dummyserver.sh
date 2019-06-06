#!/bin/sh
#
# Creates a dummy server on the specified port, if any.  Default port is
# 10001.  `socat' must be available on the system.

if [ "$#" -eq 0 ]
then
    port=10001
elif [ "$#" -eq 1 ]
then
    port=$1
else
    echo "Usage: $0 [port]" >& 2
    exit 1
fi

socat /dev/null,ignoreeof tcp-listen:"$port",fork,reuseaddr
