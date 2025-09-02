#!/bin/awk

#
#  Copyright (C) 2012  Red Hat, Inc.
#
#  This work is licensed under the terms of the GNU GPL, version 2. See
#  the COPYING file in the top-level directory.
#
#  Tool for AutoNUMA benchmarking scripts
#

# TODO 
#	sparse numbering of nodes 

function get_mask(from, to)
{
	mask = ""

	for (i = 0; i <= to; i++) {
		if (i >= from && i <= to)
			mask = "1"mask
		else
			mask = "0"mask
	}

	return bin2dec(mask)
}

function bin2dec(bin)
{
	res = 0

	for (i = length(bin); i > 0; i--) {
		if (substr(bin,i,1) == "1")
			res += 2**(length(bin)-i)
	}

	return res
}

BEGIN \
{
	cpus = 0
	nodes = 0
	first_h = ""
	second_h = ""
	nodemask1 = ""
	nodemask2 = ""
	inverse = ""
	nodemap = ""
	bindmap = ""
}

/available/ \
{
	nodes = $2
	nodemask1 = get_mask(0, $2 / 2 - 1)
	nodemask2 = get_mask($2 / 2, $2 - 1)
	first_h = "\t\tnodemask = "nodemask1";\n"
	second_h = "\t\tnodemask = "nodemask2";\n"
	ndmask1 = "#define NODEMASK1 "nodemask1
	ndmask2 = "#define NODEMASK2 "nodemask2
}

/node [0-9]+ cpus/ \
{
	curr = $2
	# NUMA01
	if (curr < nodes / 2) {
		for (i = 4; i <= NF; i++)
			first_h	= first_h"\t\tCPU_SET("$i", &cpumask);\n"
	} else {
		for (i = 4; i <= NF; i++)
			second_h = second_h"\t\tCPU_SET("$i", &cpumask);\n"
	}
	# NUMA02
	nodemap = nodemap"\t\tcase "curr":\n"
	for (i = 4; i <= NF; i++)
		nodemap = nodemap"\t\t\tCPU_SET("$i", &cpumask);\n"
	nodemap = nodemap"\t\t\tbreak;\n"
	bindmap = bindmap"\tbind("curr");\n\tbzero(p"
	for (i = 0; i < curr; i++)
		bindmap = bindmap"+SIZE/"nodes
	bindmap = bindmap", SIZE/"nodes");\n"
	if (NF > 3 && $NF > cpus)
		cpus = $NF
}

END \
{
	cpus += 1
	if (MoF)
		cpus = MoF
	threads_count = int(cpus / 2)
	if (threads_count < 1)
		threads_count = 1
	threads = "#define THREADS "threads_count
	ncpus = "#define NCPUS "cpus
	nnodes = "#define NNODES "(nodes)
	if (file == "numa01.c")
		while ( (getline < file) > 0) {
			if ($0 ~ /THREADS_VAL/) {
				print threads
				continue
			}
			if ($0 ~ /NDMASK1/) {
				print ndmask1
				continue
			}
			if ($0 ~ /NDMASK2/) {
				print ndmask2
				continue
			}
			if ($0 ~ /FIRST_HALF/) {
				print first_h
				continue
			}
			if ($0 ~ /SECOND_HALF/) {
				print second_h
				continue
			}
			print $0
		}
	else
		while ( (getline < file) > 0) {
			if ($0 ~ /CPUNUM/) {
				print ncpus
				continue
			}
			if ($0 ~ /NODENUM/) {
				print nnodes
				continue
			}
			if ($0 ~ /NODEMAP/) {
				print nodemap
				continue
			}
			if ($0 ~ /BINDMAP/) {
				print bindmap
				continue
			}
			print $0
		}
}
