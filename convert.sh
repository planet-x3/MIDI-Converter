#!/bin/bash

[ $# -ne 3 ] && echo "Usage: $0 inputfile outfile headerbyte" && exit 1

echo "Converting $1..."

if [ -f "$2.csv" ]; then
	echo "Not generating a new CSV."
else
	midicsv "$1" | sed "s/ //g" | cut -d"," -f2- | sort -s -n -k 1,1 > "$2.csv"
fi

./csv2bin.py "$2.csv" "$2" $3
