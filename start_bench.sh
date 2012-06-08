#!/bin/bash

#
#  Copyright (C) 2012  Red Hat, Inc.
#
#  This work is licensed under the terms of the GNU GPL, version 2. See
#  the COPYING file in the top-level directory.
#
#  Tool for AutoNUMA benchmarking scripts
#
#  Requirements: numactl-devel, gnuplot
#

usage()
{
	echo -e "./start_bench.sh [-sth]"
	echo -e "\t-s : run numa02_SMT test additionally"
	echo -e "\t-t : run numa01_THREAD_ALLOC test additionally"
	echo -e "\t-h : this help"
}

parse_numa() 
{
	numactl --hardware | gawk -v file="numa01.c" -f preproc.awk > numa01.prep.c
	numactl --hardware | gawk -v file="numa02.c" -f preproc.awk > numa02.prep.c
}

run_test()
{
	echo "$TESTNAME"
	nice -n -20 ./plot.sh $TESTNAME &
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

while getopts "sth" opt; do
	case $opt in
		s)
			SMT=1
			;;
		t)
			TALLOC=1
			;;
		h)
			usage
			exit 0
			;;
		\?)
			echo "Wrong argument $opt"
			usage
			exit 1
			;;
	esac
done

cleanup
parse_numa
run_bench
