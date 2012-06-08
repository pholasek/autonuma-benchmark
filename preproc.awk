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
	bindmap = bindmap"\tbind("curr");\n\tbzero(p"
	for (i = 0; i < curr; i++)
		bindmap = bindmap"+SIZE/"nodes
	bindmap = bindmap", SIZE/"nodes");\n"
	if ($NF > cpus)
		cpus = $NF
}

END \
{
	threads = "#define THREADS "(cpus + 1)
	ncpus = "#define NCPUS "(cpus + 1)
	nnodes = "#define NNODES "(nodes)
	if (file == "numa01.c")
		while ( (getline < file) > 0) {
			switch ($0) {
				case /THREADS_VAL/:
					print threads
					break
				case /NDMASK1/:
					print ndmask1
					break
				case /NDMASK2/:
					print ndmask2
					break
				case /FIRST_HALF/:
					print first_h
					break
				case /SECOND_HALF/:
					print second_h
					break
				default:
					print $0
			}
		}
	else
		while ( (getline < file) > 0) {
			switch ($0) {
				case /CPUNUM/:
					print ncpus
					break
				case /NODENUM/:
					print nnodes
					break
				case /NODEMAP/:
					print nodemap
					break
				case /BINDMAP/:
					print bindmap
					break
				default:
					print $0
			}
		}
}
