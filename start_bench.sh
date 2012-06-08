#!/bin/bash

#
#  Copyright (C) 2012  Red Hat, Inc.
#
#  This work is licensed under the terms of the GNU GPL, version 2. See
#  the COPYING file in the top-level directory.
#
#  Tool for AutoNUMA benchmarking scripts
#

parse_numa() 
{
	numactl --hardware | gawk -v file="numa01.c" -f preproc.awk > numa01.prep.c
	numactl --hardware | gawk -v file="numa02.c" -f preproc.awk > numa02.prep.c
}

run_test()
{
	echo "$TESTNAME"
	./plot.sh $TESTNAME &
	PLOT_PID=$!
	/usr/bin/time -f"%e" ./$TESTNAME
	kill -s SIGTERM $PLOT_PID
	gawk -f genplot.awk $TESTNAME.txt | gnuplot
}

run_bench()
{
	make
	TESTNAME=numa01
	run_test
	if [ $TALLOC -eq 1 ] ; then
		TESTNAME=numa01_THREAD_ALLOC
		run_test
	fi
	TESTNAME=numa02
	run_test
	if [ $SMT -eq 1 ] ; then
		TESTNAME=numa02_SMT
		run_test
	fi
}

cleanup() 
{
	make clean
}

SMT=0
TALLOC=0

while getopts "st" opt; do
	case $opt in
		s)
			SMT=1
			;;
		t)
			TALLOC=1
			;;
		\?)
			echo "Wrong argument $opt"
			exit 1
			;;
	esac
done

cleanup
parse_numa
run_bench
