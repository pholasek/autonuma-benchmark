#!/bin/awk

#
#  Copyright (C) 2012  Red Hat, Inc.
#
#  This work is licensed under the terms of the GNU GPL, version 2. See
#  the COPYING file in the top-level directory.
#
#  Tool for AutoNUMA benchmarking scripts
#

BEGIN \
{
	out = ""
}

{
	width = NF
}

END \
{
	out="set style data lines\nset terminal pdfcairo\nset output \"";
	out=out""FILENAME".pdf\"\nplot "
	for(i=1;i<=NF-1;i++) {
		out=out"\""FILENAME"\" using 1:"i+1" title 'Node "i"'"
		if (i < NF-1)
			out=out", "
	}
	print out
} 
