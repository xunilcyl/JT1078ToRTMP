#!/bin/bash

pid=`ps aux | grep JT1078ToRTMP | grep -v grep |  awk '{print $2}'`
if [ -z $pid ]; then
    echo "program is not started."
    exit 1
fi

echo "try to stop process $pid"
kill -9 $pid
echo "done"

