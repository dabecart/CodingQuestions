#!/bin/bash

# This shell file is used to compare up to three BINARY files by first converting them to .hex files
# so that it can later be easily compared using vimdiff.

if [ "$#" -lt 2 ]; then
    echo "This function needs at least two files as arguments!"
    exit -1
fi

if [ "$#" -gt 3 ]; then
    echo "This function accepts a maximum of three files as arguments!"
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
    rm -f "$3.hex"
else
    vimdiff "$1.hex" "$2.hex"
fi

rm -f "$1.hex" "$2.hex"