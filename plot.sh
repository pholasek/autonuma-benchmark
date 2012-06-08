#!/bin/bash

#
#  Copyright (C) 2012  Red Hat, Inc.
#
#  This work is licensed under the terms of the GNU GPL, version 2. See
#  the COPYING file in the top-level directory.
#
#  Tool for AutoNUMA benchmarking scripts
#

cleanup()
{
	exit 0
}

trap cleanup SIGTERM

rm -f $1.txt
touch $1.txt
PERIOD=1
BEGINTIME=`date +%s`

while :
do
	pidof $1 > /dev/null
	[ $? == 0 ] && break;
done

while :
do
	TIME=`date +%s`
	TIME=$(($TIME-$BEGINTIME))
	./nmstat -v `pidof $1` | awk -v t=$TIME '/total/ {for(i=2;i<=NF;i++) {sum[i]+=$i}} END {printf "%d ",t; for(i=2;i<=NF;i++) {printf "%.2f ",sum[i]} print ""}' >> $1.txt
	sleep $PERIOD
done
