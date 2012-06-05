/*
 *  Copyright (C) 2012  Red Hat, Inc.
 *
 *  This work is licensed under the terms of the GNU GPL, version 2. See
 *  the COPYING file in the top-level directory.
 */

#define _GNU_SOURCE
#include <pthread.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <numaif.h>
#include <sched.h>
#include <sys/wait.h>
#include <sys/file.h>

#if 1
#define THREADS 128
#define SIZE (1UL*1024*1024*1024)
#else
#define THREADS 8
#define SIZE (500*1024*1024)
#endif
#define TOTALSIZE (4UL*1024*1024*1024*200)
#define THREAD_SIZE (SIZE/THREADS)
//#define HARD_BIND
//#define INVERSE_BIND

static void *thread(void * arg)
{
	char *p = arg;
	int i;
	for (i = 0; i < TOTALSIZE/SIZE; i++) {
		bzero(p, THREAD_SIZE);
		asm volatile("" : : : "memory");
	}
	return NULL;
}

#ifdef HARD_BIND
static void bind(int node)
{
	int i;
	unsigned long nodemask;
	cpu_set_t cpumask;
	CPU_ZERO(&cpumask);
	switch (node) {
		case 0:
			nodemask = 0x1 << node;
			for (i = 0; i < 8; i++)
				CPU_SET(i, &cpumask);
			for (i = 64; i < 72; i++)
				CPU_SET(i, &cpumask);
			break;
		case 1:
			nodemask = 0x1 << node;
			for (i = 8; i < 16; i++)
				CPU_SET(i, &cpumask);
			for (i = 72; i < 80; i++)
				CPU_SET(i, &cpumask);
			break;
		case 2:
			nodemask = 0x1 << node;
			for (i = 16; i < 24; i++)
				CPU_SET(i, &cpumask);
			for (i = 80; i < 88; i++)
				CPU_SET(i, &cpumask);
			break;
		case 3:
			nodemask = 0x1 << node;
			for (i = 24; i < 32; i++)
				CPU_SET(i, &cpumask);
			for (i = 88; i < 96; i++)
				CPU_SET(i, &cpumask);
			break;
		case 4:
			nodemask = 0x1 << node;
			for (i = 32; i < 40; i++)
				CPU_SET(i, &cpumask);
			for (i = 96; i < 104; i++)
				CPU_SET(i, &cpumask);
			break;
		case 5:
			nodemask = 0x1 << node;
			for (i = 40; i < 48; i++)
				CPU_SET(i, &cpumask);
			for (i = 104; i < 112; i++)
				CPU_SET(i, &cpumask);
			break;
		case 6:
			nodemask = 0x1 << node;
			for (i = 48; i < 56; i++)
				CPU_SET(i, &cpumask);
			for (i = 112; i < 120; i++)
				CPU_SET(i, &cpumask);
			break;
		case 7:
			nodemask = 0x1 << node;
			for (i = 56; i < 64; i++)
				CPU_SET(i, &cpumask);
			for (i = 120; i < 128; i++)
				CPU_SET(i, &cpumask);
			break;
		default:
			break;
	}
	if (sched_setaffinity(0, sizeof(cpumask), &cpumask) < 0)
		perror("sched_setaffinity"), exit(1);
	if (set_mempolicy(MPOL_BIND, &nodemask, 9) < 0)
		perror("set_mempolicy"), printf("%lu\n", nodemask), exit(1);
}
#else
static void bind(int node) {}
#endif

int main()
{
	int i;
	pthread_t pthread[THREADS];
	char *p;
	pid_t pid;
	int f;

	p = malloc(SIZE);
	if (!p)
		perror("malloc"), exit(1);
	bind(0);
	bzero(p, SIZE/8);
	bind(1);
	bzero(p+SIZE/8, SIZE/8);
	bind(2);
	bzero(p+SIZE/4, SIZE/8);
	bind(3);
	bzero(p+SIZE/4+SIZE/8, SIZE/8);
	bind(4);
	bzero(p+SIZE/2, SIZE/8);
	bind(5);
	bzero(p+SIZE/2+SIZE/8, SIZE/8);
	bind(6);
	bzero(p+SIZE/2+SIZE/4, SIZE/8);
	bind(7);
	bzero(p+SIZE/2+SIZE/4+SIZE/8, SIZE/8);
	for (i = 0; i < THREADS; i++) {
		char *_p = p + THREAD_SIZE * i;
#ifdef INVERSE_BIND
		bind((THREADS - 1 - i) / 16);
#else
		bind(i / 16);
#endif
		if (pthread_create(&pthread[i], NULL, thread, _p) != 0)
			perror("pthread_create"), exit(1);
	}
	for (i = 0; i < THREADS; i++)
		if (pthread_join(pthread[i], NULL) != 0)
			perror("pthread_join"), exit(1);
	return 0;
}
