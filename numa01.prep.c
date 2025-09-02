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
#include <time.h>
#include <sys/wait.h>
#include <sys/file.h>

#define THREADS 176
#define NODEMASK1 3
#define NODEMASK2 24
#define SIZE (3UL*1024*1024*1024) 
//#define THREAD_ALLOC
#ifdef THREAD_ALLOC
#define THREAD_SIZE (SIZE/THREADS)
#else
#define THREAD_SIZE SIZE
#endif
//#define HARD_BIND
//#define INVERSE_BIND
//#define NO_BIND_FORCE_SAME_NODE

static char *p_global;
static unsigned long nodemask_global;

void *thread(void * arg)
{
	char *p = arg;
	int i;
#ifndef THREAD_ALLOC
	int nr = 50;
#else
	int nr = 1000;
#endif
#ifdef NO_BIND_FORCE_SAME_NODE
	if (set_mempolicy(MPOL_BIND, &nodemask_global, 3) < 0)
		perror("set_mempolicy"), printf("%lu\n", nodemask_global),
			exit(1);
#endif
	bzero(p_global, SIZE);
#ifdef NO_BIND_FORCE_SAME_NODE
	if (set_mempolicy(MPOL_DEFAULT, NULL, 3) < 0)
		perror("set_mempolicy"), exit(1);
#endif
	for (i = 0; i < nr; i++) {
		bzero(p, THREAD_SIZE);
		asm volatile("" : : : "memory");
	}
	return NULL;
}

int main()
{
	int i;
	pthread_t pthread[THREADS];
	char *p;
	pid_t pid;
	cpu_set_t cpumask;
	int f;
	unsigned long nodemask;

	nodemask_global = (time(NULL) & 1) + 1;
	f = creat("lock", 0400);
	if (f < 0)
		perror("creat"), exit(1);
	if (unlink("lock") < 0)
		perror("unlink"), exit(1);

	if ((pid = fork()) < 0)
		perror("fork"), exit(1);

	p_global = p = malloc(SIZE);
	if (!p)
		perror("malloc"), exit(1);
	CPU_ZERO(&cpumask);
	if (!pid) {
		nodemask = 3;
		CPU_SET(0, &cpumask);
		CPU_SET(1, &cpumask);
		CPU_SET(2, &cpumask);
		CPU_SET(3, &cpumask);
		CPU_SET(4, &cpumask);
		CPU_SET(5, &cpumask);
		CPU_SET(6, &cpumask);
		CPU_SET(7, &cpumask);
		CPU_SET(8, &cpumask);
		CPU_SET(9, &cpumask);
		CPU_SET(10, &cpumask);
		CPU_SET(11, &cpumask);
		CPU_SET(12, &cpumask);
		CPU_SET(13, &cpumask);
		CPU_SET(14, &cpumask);
		CPU_SET(15, &cpumask);
		CPU_SET(16, &cpumask);
		CPU_SET(17, &cpumask);
		CPU_SET(18, &cpumask);
		CPU_SET(19, &cpumask);
		CPU_SET(20, &cpumask);
		CPU_SET(21, &cpumask);
		CPU_SET(22, &cpumask);
		CPU_SET(23, &cpumask);
		CPU_SET(24, &cpumask);
		CPU_SET(25, &cpumask);
		CPU_SET(26, &cpumask);
		CPU_SET(27, &cpumask);
		CPU_SET(28, &cpumask);
		CPU_SET(29, &cpumask);
		CPU_SET(30, &cpumask);
		CPU_SET(31, &cpumask);
		CPU_SET(32, &cpumask);
		CPU_SET(33, &cpumask);
		CPU_SET(34, &cpumask);
		CPU_SET(35, &cpumask);
		CPU_SET(36, &cpumask);
		CPU_SET(37, &cpumask);
		CPU_SET(38, &cpumask);
		CPU_SET(39, &cpumask);
		CPU_SET(40, &cpumask);
		CPU_SET(41, &cpumask);
		CPU_SET(42, &cpumask);
		CPU_SET(43, &cpumask);
		CPU_SET(44, &cpumask);
		CPU_SET(45, &cpumask);
		CPU_SET(46, &cpumask);
		CPU_SET(47, &cpumask);
		CPU_SET(192, &cpumask);
		CPU_SET(193, &cpumask);
		CPU_SET(194, &cpumask);
		CPU_SET(195, &cpumask);
		CPU_SET(196, &cpumask);
		CPU_SET(197, &cpumask);
		CPU_SET(198, &cpumask);
		CPU_SET(199, &cpumask);
		CPU_SET(200, &cpumask);
		CPU_SET(201, &cpumask);
		CPU_SET(202, &cpumask);
		CPU_SET(203, &cpumask);
		CPU_SET(204, &cpumask);
		CPU_SET(205, &cpumask);
		CPU_SET(206, &cpumask);
		CPU_SET(207, &cpumask);
		CPU_SET(208, &cpumask);
		CPU_SET(209, &cpumask);
		CPU_SET(210, &cpumask);
		CPU_SET(211, &cpumask);
		CPU_SET(212, &cpumask);
		CPU_SET(213, &cpumask);
		CPU_SET(214, &cpumask);
		CPU_SET(215, &cpumask);
		CPU_SET(216, &cpumask);
		CPU_SET(217, &cpumask);
		CPU_SET(218, &cpumask);
		CPU_SET(219, &cpumask);
		CPU_SET(220, &cpumask);
		CPU_SET(221, &cpumask);
		CPU_SET(222, &cpumask);
		CPU_SET(223, &cpumask);
		CPU_SET(224, &cpumask);
		CPU_SET(225, &cpumask);
		CPU_SET(226, &cpumask);
		CPU_SET(227, &cpumask);
		CPU_SET(228, &cpumask);
		CPU_SET(229, &cpumask);
		CPU_SET(230, &cpumask);
		CPU_SET(231, &cpumask);
		CPU_SET(232, &cpumask);
		CPU_SET(233, &cpumask);
		CPU_SET(234, &cpumask);
		CPU_SET(235, &cpumask);
		CPU_SET(236, &cpumask);
		CPU_SET(237, &cpumask);
		CPU_SET(238, &cpumask);
		CPU_SET(239, &cpumask);
		CPU_SET(48, &cpumask);
		CPU_SET(49, &cpumask);
		CPU_SET(50, &cpumask);
		CPU_SET(51, &cpumask);
		CPU_SET(52, &cpumask);
		CPU_SET(53, &cpumask);
		CPU_SET(54, &cpumask);
		CPU_SET(55, &cpumask);
		CPU_SET(56, &cpumask);
		CPU_SET(57, &cpumask);
		CPU_SET(58, &cpumask);
		CPU_SET(59, &cpumask);
		CPU_SET(60, &cpumask);
		CPU_SET(61, &cpumask);
		CPU_SET(62, &cpumask);
		CPU_SET(63, &cpumask);
		CPU_SET(64, &cpumask);
		CPU_SET(65, &cpumask);
		CPU_SET(66, &cpumask);
		CPU_SET(67, &cpumask);
		CPU_SET(68, &cpumask);
		CPU_SET(69, &cpumask);
		CPU_SET(70, &cpumask);
		CPU_SET(71, &cpumask);
		CPU_SET(72, &cpumask);
		CPU_SET(73, &cpumask);
		CPU_SET(74, &cpumask);
		CPU_SET(75, &cpumask);
		CPU_SET(76, &cpumask);
		CPU_SET(77, &cpumask);
		CPU_SET(78, &cpumask);
		CPU_SET(79, &cpumask);
		CPU_SET(80, &cpumask);
		CPU_SET(81, &cpumask);
		CPU_SET(82, &cpumask);
		CPU_SET(83, &cpumask);
		CPU_SET(84, &cpumask);
		CPU_SET(85, &cpumask);
		CPU_SET(86, &cpumask);
		CPU_SET(87, &cpumask);
		CPU_SET(88, &cpumask);
		CPU_SET(89, &cpumask);
		CPU_SET(90, &cpumask);
		CPU_SET(91, &cpumask);
		CPU_SET(92, &cpumask);
		CPU_SET(93, &cpumask);
		CPU_SET(94, &cpumask);
		CPU_SET(95, &cpumask);
		CPU_SET(240, &cpumask);
		CPU_SET(241, &cpumask);
		CPU_SET(242, &cpumask);
		CPU_SET(243, &cpumask);
		CPU_SET(244, &cpumask);
		CPU_SET(245, &cpumask);
		CPU_SET(246, &cpumask);
		CPU_SET(247, &cpumask);
		CPU_SET(248, &cpumask);
		CPU_SET(249, &cpumask);
		CPU_SET(250, &cpumask);
		CPU_SET(251, &cpumask);
		CPU_SET(252, &cpumask);
		CPU_SET(253, &cpumask);
		CPU_SET(254, &cpumask);
		CPU_SET(255, &cpumask);
		CPU_SET(256, &cpumask);
		CPU_SET(257, &cpumask);
		CPU_SET(258, &cpumask);
		CPU_SET(259, &cpumask);
		CPU_SET(260, &cpumask);
		CPU_SET(261, &cpumask);
		CPU_SET(262, &cpumask);
		CPU_SET(263, &cpumask);
		CPU_SET(264, &cpumask);
		CPU_SET(265, &cpumask);
		CPU_SET(266, &cpumask);
		CPU_SET(267, &cpumask);
		CPU_SET(268, &cpumask);
		CPU_SET(269, &cpumask);
		CPU_SET(270, &cpumask);
		CPU_SET(271, &cpumask);
		CPU_SET(272, &cpumask);
		CPU_SET(273, &cpumask);
		CPU_SET(274, &cpumask);
		CPU_SET(275, &cpumask);
		CPU_SET(276, &cpumask);
		CPU_SET(277, &cpumask);
		CPU_SET(278, &cpumask);
		CPU_SET(279, &cpumask);
		CPU_SET(280, &cpumask);
		CPU_SET(281, &cpumask);
		CPU_SET(282, &cpumask);
		CPU_SET(283, &cpumask);
		CPU_SET(284, &cpumask);
		CPU_SET(285, &cpumask);
		CPU_SET(286, &cpumask);
		CPU_SET(287, &cpumask);
		CPU_SET(96, &cpumask);
		CPU_SET(97, &cpumask);
		CPU_SET(98, &cpumask);
		CPU_SET(99, &cpumask);
		CPU_SET(100, &cpumask);
		CPU_SET(101, &cpumask);
		CPU_SET(102, &cpumask);
		CPU_SET(103, &cpumask);
		CPU_SET(104, &cpumask);
		CPU_SET(105, &cpumask);
		CPU_SET(106, &cpumask);
		CPU_SET(107, &cpumask);
		CPU_SET(108, &cpumask);
		CPU_SET(109, &cpumask);
		CPU_SET(110, &cpumask);
		CPU_SET(111, &cpumask);
		CPU_SET(112, &cpumask);
		CPU_SET(113, &cpumask);
		CPU_SET(114, &cpumask);
		CPU_SET(115, &cpumask);
		CPU_SET(116, &cpumask);
		CPU_SET(117, &cpumask);
		CPU_SET(118, &cpumask);
		CPU_SET(119, &cpumask);
		CPU_SET(120, &cpumask);
		CPU_SET(121, &cpumask);
		CPU_SET(122, &cpumask);
		CPU_SET(123, &cpumask);
		CPU_SET(124, &cpumask);
		CPU_SET(125, &cpumask);
		CPU_SET(126, &cpumask);
		CPU_SET(127, &cpumask);
		CPU_SET(128, &cpumask);
		CPU_SET(129, &cpumask);
		CPU_SET(130, &cpumask);
		CPU_SET(131, &cpumask);
		CPU_SET(132, &cpumask);
		CPU_SET(133, &cpumask);
		CPU_SET(134, &cpumask);
		CPU_SET(135, &cpumask);
		CPU_SET(136, &cpumask);
		CPU_SET(137, &cpumask);
		CPU_SET(138, &cpumask);
		CPU_SET(139, &cpumask);
		CPU_SET(140, &cpumask);
		CPU_SET(141, &cpumask);
		CPU_SET(142, &cpumask);
		CPU_SET(143, &cpumask);
		CPU_SET(288, &cpumask);
		CPU_SET(289, &cpumask);
		CPU_SET(290, &cpumask);
		CPU_SET(291, &cpumask);
		CPU_SET(292, &cpumask);
		CPU_SET(293, &cpumask);
		CPU_SET(294, &cpumask);
		CPU_SET(295, &cpumask);
		CPU_SET(296, &cpumask);
		CPU_SET(297, &cpumask);
		CPU_SET(298, &cpumask);
		CPU_SET(299, &cpumask);
		CPU_SET(300, &cpumask);
		CPU_SET(301, &cpumask);
		CPU_SET(302, &cpumask);
		CPU_SET(303, &cpumask);
		CPU_SET(304, &cpumask);
		CPU_SET(305, &cpumask);
		CPU_SET(306, &cpumask);
		CPU_SET(307, &cpumask);
		CPU_SET(308, &cpumask);
		CPU_SET(309, &cpumask);
		CPU_SET(310, &cpumask);
		CPU_SET(311, &cpumask);
		CPU_SET(312, &cpumask);
		CPU_SET(313, &cpumask);
		CPU_SET(314, &cpumask);
		CPU_SET(315, &cpumask);
		CPU_SET(316, &cpumask);
		CPU_SET(317, &cpumask);
		CPU_SET(318, &cpumask);
		CPU_SET(319, &cpumask);
		CPU_SET(320, &cpumask);
		CPU_SET(321, &cpumask);
		CPU_SET(322, &cpumask);
		CPU_SET(323, &cpumask);
		CPU_SET(324, &cpumask);
		CPU_SET(325, &cpumask);
		CPU_SET(326, &cpumask);
		CPU_SET(327, &cpumask);
		CPU_SET(328, &cpumask);
		CPU_SET(329, &cpumask);
		CPU_SET(330, &cpumask);
		CPU_SET(331, &cpumask);
		CPU_SET(332, &cpumask);
		CPU_SET(333, &cpumask);
		CPU_SET(334, &cpumask);
		CPU_SET(335, &cpumask);

	} else {
		nodemask = 24;
		CPU_SET(144, &cpumask);
		CPU_SET(145, &cpumask);
		CPU_SET(146, &cpumask);
		CPU_SET(147, &cpumask);
		CPU_SET(148, &cpumask);
		CPU_SET(149, &cpumask);
		CPU_SET(150, &cpumask);
		CPU_SET(151, &cpumask);
		CPU_SET(152, &cpumask);
		CPU_SET(153, &cpumask);
		CPU_SET(154, &cpumask);
		CPU_SET(155, &cpumask);
		CPU_SET(156, &cpumask);
		CPU_SET(157, &cpumask);
		CPU_SET(158, &cpumask);
		CPU_SET(159, &cpumask);
		CPU_SET(160, &cpumask);
		CPU_SET(161, &cpumask);
		CPU_SET(162, &cpumask);
		CPU_SET(163, &cpumask);
		CPU_SET(164, &cpumask);
		CPU_SET(165, &cpumask);
		CPU_SET(166, &cpumask);
		CPU_SET(167, &cpumask);
		CPU_SET(168, &cpumask);
		CPU_SET(169, &cpumask);
		CPU_SET(170, &cpumask);
		CPU_SET(171, &cpumask);
		CPU_SET(172, &cpumask);
		CPU_SET(173, &cpumask);
		CPU_SET(174, &cpumask);
		CPU_SET(175, &cpumask);
		CPU_SET(176, &cpumask);
		CPU_SET(177, &cpumask);
		CPU_SET(178, &cpumask);
		CPU_SET(179, &cpumask);
		CPU_SET(180, &cpumask);
		CPU_SET(181, &cpumask);
		CPU_SET(182, &cpumask);
		CPU_SET(183, &cpumask);
		CPU_SET(184, &cpumask);
		CPU_SET(185, &cpumask);
		CPU_SET(186, &cpumask);
		CPU_SET(187, &cpumask);
		CPU_SET(188, &cpumask);
		CPU_SET(189, &cpumask);
		CPU_SET(190, &cpumask);
		CPU_SET(191, &cpumask);
		CPU_SET(336, &cpumask);
		CPU_SET(337, &cpumask);
		CPU_SET(338, &cpumask);
		CPU_SET(339, &cpumask);
		CPU_SET(340, &cpumask);
		CPU_SET(341, &cpumask);
		CPU_SET(342, &cpumask);
		CPU_SET(343, &cpumask);
		CPU_SET(344, &cpumask);
		CPU_SET(345, &cpumask);
		CPU_SET(346, &cpumask);
		CPU_SET(347, &cpumask);
		CPU_SET(348, &cpumask);
		CPU_SET(349, &cpumask);
		CPU_SET(350, &cpumask);
		CPU_SET(351, &cpumask);
		CPU_SET(352, &cpumask);

	}
#ifdef INVERSE_BIND
	if (nodemask == NODEMASK1)
		nodemask = NODEMASK2;
	else if (nodemask == NODEMASK1)
		nodemask = NODEMASK2;
#endif
#ifdef HARD_BIND
	if (sched_setaffinity(0, sizeof(cpumask), &cpumask) < 0)
		perror("sched_setaffinity"), exit(1);
#endif
#ifdef HARD_BIND
	if (set_mempolicy(MPOL_BIND, &nodemask, 9) < 0)
		perror("set_mempolicy"), printf("%lu\n", nodemask), exit(1);
#endif
	for (i = 0; i < THREADS; i++) {
		char *_p = p;
#ifdef THREAD_ALLOC
		_p += THREAD_SIZE * i;
#endif
		if (pthread_create(&pthread[i], NULL, thread, _p) != 0)
			perror("pthread_create"), exit(1);
	}
	for (i = 0; i < THREADS; i++)
		if (pthread_join(pthread[i], NULL) != 0)
			perror("pthread_join"), exit(1);
	if (pid)
		if (wait(NULL) < 0)
			perror("wait"), exit(1);
	return 0;
}
