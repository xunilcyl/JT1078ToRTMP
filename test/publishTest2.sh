#!/bin/bash

export LD_LIBRARY_PATH=../lib

function publish() {
    allocResult=`curl -v -d "{\"method\":\"allocMediaPortReq\", \"params\":{\"uniqueID\": "$1"}, \"id\":100}" "http://127.0.0.1:8888/media" | grep \"port\" | awk '{print $2}'`
    if [ -z $allocResult ]; then
        echo "allocate port failed"
        return 1
    fi

    echo $allocResult
    port=${allocResult:1:5}
    if [ -z $port ]; then
        echo "port invalid: $port"
    fi
    
    echo "get port $port"
    
    ./h264sender $port tcpdump1078_fn_fps_174s.bin 174 &
    
    if [ $? -eq 0 ]; then
        echo "send media stream success."
    else
        echo "send media failed"
        return 2;
    fi
}

if [ $# -lt 2 ]; then
    echo "usage: $0 <num>"
    echo "       <num> represents the number of publishers"
    exit 1
fi

for ((i=$1; i<=$2; i++))
do
    publish $i
done

exit 0;

