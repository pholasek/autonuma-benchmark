#
#  Copyright (C) 2012  Red Hat, Inc.
#
#  This work is licensed under the terms of the GNU GPL, version 2. See
#  the COPYING file in the top-level directory.
#
#  Tool for AutoNUMA benchmarking scripts
#


#!/bin/bash

# TODO plotting
# per-node memory use = use nmstat.c from Bill Gray

parse_numa() 
{
	# use gawk
	# TODO
	#cat numactl_hw.tpl | gawk -v file="numa01.c" -f preproc.awk > numa01.prep.c
	#cat numactl_hw.tpl | gawk -v file="numa02.c" -f preproc.awk > numa02.prep.c
	numactl --hardware | gawk -v file="numa01.c" -f preproc.awk > numa01.prep.c
	numactl --hardware | gawk -v file="numa02.c" -f preproc.awk > numa02.prep.c
}

run_test()
{
	#TODO add more
	make
	echo "numa01"
	/usr/bin/time -f"%e" ./numa01
	if [ $TALLOC -eq 1 ] ; then
		echo "numa01_THREAD_ALLOC"
		/usr/bin/time -f"%e" ./numa01_THREAD_ALLOC
	fi
	echo "numa02"
	/usr/bin/time -f"%e" ./numa02 
	if [ $SMT -eq 1 ] ; then
		echo "numa02_SMT"
		/usr/bin/time -f"%e" ./numa02_SMT
	fi
}

cleanup() 
{
	make clean
}

SMT=0
TALLOC=0
# TODO add more

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

parse_numa
run_test
cleanup
