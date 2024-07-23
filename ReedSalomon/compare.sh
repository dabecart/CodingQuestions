#!/bin/bash

if [ "$#" -lt 2 ]; then
    echo "This function needs at least two files as arguments!"
    exit -1
fi

if [ ! -f "$1" ]; then
    echo "$1 is not a file!"
    exit -1
fi
xxd -ps -c 10 "$1" > "$1.hex"

if [ ! -f "$2" ]; then
    echo "$2 is not a file!"
    exit -1
fi
xxd -ps -c 10 "$2" > "$2.hex"

if [ "$#" -eq 3 ]; then
    if [ ! -f "$3" ]; then
        echo "$3 is not a file!"
        exit -1
    fi

    xxd -ps -c 10 "$3" > "$3.hex"
    vimdiff "$1.hex" "$2.hex" "$3.hex"
    rm "$1.hex" "$2.hex" "$3.hex"
else
    vimdiff "$1.hex" "$2.hex"
    rm "$1.hex" "$2.hex"
fi