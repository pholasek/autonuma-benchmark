#!/bin/bash

#
#  Copyright (C) 2012  Red Hat, Inc.
#
#  This work is licensed under the terms of the GNU GPL, version 2. See
#  the COPYING file in the top-level directory.
#
#  Tool for AutoNUMA benchmarking scripts
#

set -x

cleanup()
{
	exit 0
}

trap cleanup SIGTERM

rm -f $1.txt
touch $1.txt
PERIOD=3
TIME=0

while :
do
	pidof $1
	[ $? == 0 ] && break;
done

while :
do
	./nmstat -v `pidof $1` | awk -v t=$TIME '/total/ {for(i=2;i<=NF;i++) {sum[i]+=$i}} END {printf "%d ",t; for(i=2;i<=NF;i++) {printf "%.2f ",sum[i]} print ""}' >> $1.txt
	TIME=$(($TIME+$PERIOD))
	sleep $PERIOD
done
