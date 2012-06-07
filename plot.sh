#!/bin/bash

cleanup()
{
	exit 0
}

trap cleanup SIGTERM

rm -f columns.txt
touch columns.txt
TIME=0

while :
do
	pidof $1
	[ $? == 0 ] && break;
done

while :
do
	./nmstat `pidof $1` | awk -v t=$TIME '/[0-9]+/ {for(i=3;i<=NF;i++) {sum_i+=$i}} END {printf "%d ",t; for(i=3;i<=NF;i++) {printf "%.2f ",sum_i}}'
	TIME=$TIME+10
	sleep 3
done
