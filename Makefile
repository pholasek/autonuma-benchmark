#
# numa benchmarks Makefile
#

CC=gcc
CFLAGS=-O2 -lnuma -pthread

all: numa01 numa01

numa01: numa01.c
	$(CC) $< $(CFLAGS) -o $@
numa02: numa02.c
	$(CC) $< $(CFLAGS) -o $@

