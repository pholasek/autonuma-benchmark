/*

nmstat - NUMA monitoring tool to show per-node usage of memory
Copyright (C) 2012 Bill Gray (bgray@redhat.com), Red Hat Inc

nmstat is free software; you can redistribute it and/or modify it under the
terms of the GNU Lesser General Public License as published by the Free
Software Foundation; version 2.1.

nmstat is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
details.

You should find a copy of v2.1 of the GNU Lesser General Public License
somewhere on your Linux system; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

*/ 


// Compile with: gcc -g -std=gnu99 -o nmstat nmstat.c 



#define __USE_MISC
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>


#define BUF_SIZE 2048
#define KILOBYTE (1024)
#define MEGABYTE (1024 * 1024)


// Structure to accumulate memory info from /proc/<PID>/numa_maps for a
// specific process, or from /sys/devices/system/node/node?/meminfo for
// system-wide data.  Tables are defined below for each process and for
// system-wide data.
#define MAX_NODES 128
typedef struct meminfo {
    int index;
    int seen;
    char *token;
    char *label;
    double node[MAX_NODES];
} meminfo_t, *meminfo_p;


#define PROC_HUGE_INDEX 0
#define PROC_PRIVATE_INDEX 3
#define PROC_TOTAL_INDEX   4


meminfo_t proc_table[] = {
    { 0, 0, "huge", "huge" },
    { 1, 0, "heap", "heap" },
    { 2, 0, "stack", "stack" },
    { 3, 0, "N", "private" },
    { 4, 0, "total", "total" }
};
#define PROC_TABLE_SIZE (sizeof(proc_table) / sizeof(proc_table[0]))


meminfo_t sys_table[] = {
    {  0, 0, "MemTotal", "MemTotal" },
    {  1, 0, "MemFree", "MemFree" },
    {  2, 0, "MemUsed", "MemUsed" },
    {  3, 0, "Active", "Active" },
    {  4, 0, "Inactive", "Inactive" },
    {  5, 0, "HighTotal", "HighTotal" },
    {  6, 0, "HighFree", "HighFree" },
    {  7, 0, "LowTotal", "LowTotal" },
    {  8, 0, "LowFree", "LowFree" },
    {  9, 0, "Active(anon)", "Active(anon)" },
    { 10, 0, "Inactive(anon)", "Inactive(anon)" },
    { 11, 0, "Active(file)", "Active(file)" },
    { 12, 0, "Inactive(file)", "Inactive(file)" },
    { 13, 0, "Unevictable", "Unevictable" },
    { 14, 0, "Mlocked", "Mlocked" },
    { 15, 0, "Dirty", "Dirty" },
    { 16, 0, "Writeback", "Writeback" },
    { 17, 0, "FilePages", "FilePages" },
    { 18, 0, "Mapped", "Mapped" },
    { 19, 0, "AnonPages", "AnonPages" },
    { 20, 0, "Shmem", "Shmem" },
    { 21, 0, "KernelStack", "KernelStack" },
    { 22, 0, "PageTables", "PageTables" },
    { 23, 0, "NFS_Unstable", "NFS_Unstable" },
    { 24, 0, "Bounce", "Bounce" },
    { 25, 0, "WritebackTmp", "WritebackTmp" },
    { 26, 0, "Slab", "Slab" },
    { 27, 0, "SReclaimable", "SReclaimable" },
    { 28, 0, "SUnreclaim", "SUnreclaim" },
    { 29, 0, "AnonHugePages", "AnonHugePages" },
    { 30, 0, "HugePages_Total", "HugePages_Total" },
    { 31, 0, "HugePages_Free", "HugePages_Free" },
    { 32, 0, "HugePages_Surp", "HugePages_Surp" }
};
#define SYS_TABLE_SIZE (sizeof(sys_table) / sizeof(sys_table[0]))


meminfo_t numastat_table[] = {
    { 0, 0, "numa_hit", "Numa_Hit" },
    { 1, 0, "numa_miss", "Numa_Miss" },
    { 2, 0, "numa_foreign", "Numa_Foreign" },
    { 3, 0, "interleave_hit", "Interleave_Hit" },
    { 4, 0, "local_node", "Local_Node" },
    { 5, 0, "other_node", "Other_Node" },
};
#define NUMASTAT_TABLE_SIZE (sizeof(numastat_table) / sizeof(numastat_table[0]))


// To allow re-ordering the memory categories in sys_table and numastat_table,
// a simple hash index is used to look up the categories.  The hash table size
// should be bigger than necessary to reduce collisions (and because these hash
// algorithms depend on unused buckets.
#define HASH_TABLE_SIZE 151
struct hash_entry {
    char *name;
    int index;
} hash_table[HASH_TABLE_SIZE];


int verbose = 0;
int num_nodes = 0;
int show_the_system_info = 0;
int print_header = 1;
int hash_collisions = 0;
char *pattern = NULL;
char *prog_name = NULL;
double page_size_in_bytes = 0;
double huge_page_size_in_bytes = 0;


void print_usage_and_exit() {
    fprintf(stderr, "Usage: %s [-v] [-V] [-n <pattern>] [-s] [ PID... ]\n", prog_name);
    fprintf(stderr, "-v makes some reports more verbose\n");
    fprintf(stderr, "-V shows the code version\n");
    fprintf(stderr, "-n <pattern> looks up PIDs with <pattern> in command\n");
    fprintf(stderr, "-s shows some system wide memory usage\n");
    exit(EXIT_FAILURE);
}


void print_version() {
    char *version_string = "20120412";
    printf("%s version: %s: %s\n", prog_name, version_string, __DATE__);
    exit(EXIT_SUCCESS);
}


char *command_name_for_pid(int pid) {
    // get command name field from status file
    char fname[128];
    static char buf[BUF_SIZE];
    snprintf(fname, sizeof(fname), "/proc/%d/status", pid);
    FILE *fs = fopen(fname, "r");
    if (!fs) {
        return NULL;
    } else {
        while (fgets(buf, BUF_SIZE, fs)) {
            if (strstr(buf, "Name:") == buf) {
                fclose(fs);
                char *p = &buf[5];
                while (isspace(*p)) {
                    p++;
                }
                if (p[strlen(p) - 1] == '\n') {
                    p[strlen(p) - 1] = '\0';
                }
                return p;
            }
        }
        fclose(fs);
    }
    return NULL;
}


void show_process_info(int pid, int show_sub_categories) {
    // Zero per-node page counts for all memory categories
    for (int ix = 0;  (ix < PROC_TABLE_SIZE);  ix++) {
	memset(&proc_table[ix].node, 0, sizeof(proc_table[ix].node));
    }
    // Open numa_map for this PID
    char fname[128];
    snprintf(fname, sizeof(fname), "/proc/%d/numa_maps", pid);
    FILE *fs = fopen(fname, "r");
    if (!fs) {
	fprintf(stderr, "Can't find numa_map for pid %d\n", pid);
        return;
    }
    // Add up sub-category memory used from each node.  Must go line by line
    // through the numa_map figuring out which category memory, node, and the
    // amount.
    char buf[BUF_SIZE];
    while (fgets(buf, BUF_SIZE, fs)) {
	int category = PROC_PRIVATE_INDEX; // init category to the catch-all...
	const char *delimiters = " \t\r\n";
	char *p = strtok(buf, delimiters);
	while (p) {
	    // If the memory category for this line is still the catch-all
	    // (i.e.  private), then see if the current token is a special
	    // keyword for a specific memory sub-category.
	    if (category == PROC_PRIVATE_INDEX) {
		for (int ix = 0;  (ix < PROC_PRIVATE_INDEX);  ix++) {
		    if (!strncmp(p, proc_table[ix].token, strlen(proc_table[ix].token))) {
			category = ix;
			break;
		    }
		}
	    }
            // If the current token is a per-node pages quantity, parse the
            // node number and accumulate the number of pages in the specific
            // category (and also add to the total).
	    if (p[0] == 'N') {
		int node_num = (int)strtol(&p[1], &p, 10);
		if (p[0] != '=') {
		    perror("node value parse error");
		    exit(EXIT_FAILURE);
		}
		long pages = strtol(&p[1], &p, 10);
		double MBs;  // init MBs with proper page size multiplier
                if (category == PROC_HUGE_INDEX) {
		    MBs = huge_page_size_in_bytes;
		} else {
		    MBs = page_size_in_bytes;
		}
		MBs *= (double)pages;
		MBs /= (double)MEGABYTE;
		proc_table[category].node[node_num] += MBs;
		proc_table[PROC_TOTAL_INDEX].node[node_num] += MBs;
	    }
	    // Get next token on the line
	    p = strtok(NULL, delimiters);
	}
    }
    fclose(fs);
    // Print header information for the process(es).  Vary some stuff according
    // to number of processes and detail being shown.
    if (print_header) {
	print_header = show_sub_categories;  // Don't reprint header unless showing detail
	printf("\nPer-node process memory usage (in MBs)");
	if (show_sub_categories) {
	    printf(" for PID %d (%s)", pid, command_name_for_pid(pid));
	}
	printf(":\n", pid);
	// Print proc_table header
	if (show_sub_categories) {
	    printf("%15s", " ");
	} else {
	    printf("%15s", "PID   ");
	}
	for (int ix = 0;  (ix < num_nodes);  ix++) {
	    char node_label[32];
	    snprintf(node_label, sizeof(node_label), "Node %d", ix);
	    printf("%15s", node_label);
	}
	printf("\n");
	if (show_sub_categories) {
	    printf("%15s", " ");
	} else {
	    printf("%15s", "----------");
	}
	for (int ix = 0;  (ix < num_nodes);  ix++) {
	    printf("%15s", "----------");
	}
	printf("\n");
    }
    // Print the process memory info -- optionally showing the sub-categories
    for (int ix = (show_sub_categories ? 0 : PROC_TOTAL_INDEX);  (ix <= PROC_TOTAL_INDEX);  ix++) {
	if (show_sub_categories || (ix != PROC_TOTAL_INDEX)) {
	    printf("%15s", proc_table[ix].label);
	} else {
            char tmp_buf[64];
            snprintf(tmp_buf, 15, "%d (%s)", pid, command_name_for_pid(pid));
	    printf("%15s", tmp_buf);
	}
	for (int node_ix = 0;  (node_ix < num_nodes);  node_ix++) {
	    printf("%15.2f", proc_table[ix].node[node_ix]);
	}
	printf("\n");
    }
}  // show_process_info


int hash_ix(char *s) {
    int c;
    unsigned int h = 17;
    while (c = *s++) {
        // h * 33 + c
        h = ((h << 5) + h) + c;
    }
    return (h % HASH_TABLE_SIZE);
}


int hash_lookup(char *s) {
    int ix = hash_ix(s);
    while (hash_table[ix].name) { // Assumes big table with blank entries
        if (!strcmp(s, hash_table[ix].name)) {
            return hash_table[ix].index;  // found it
        }
        ix += 1;
        if (ix >= HASH_TABLE_SIZE) {
            ix = 0;
        }
    }
    return -1;
}


int hash_insert(char *s, int i) {
    int ix = hash_ix(s);
    while (hash_table[ix].name) { // assumes no duplicate entries
        hash_collisions += 1;
        ix += 1;
        if (ix >= HASH_TABLE_SIZE) {
            ix = 0;
        }
    }
    hash_table[ix].name = s;
    hash_table[ix].index = i;
    return ix;
}


void show_numastat_info() {
    // Build numastat_table hash index and zero the numastat_table per-node values
    memset(hash_table, 0, sizeof(hash_table));
    for (int ix = 0;  (ix < NUMASTAT_TABLE_SIZE);  ix++) {
        hash_insert(numastat_table[ix].token, numastat_table[ix].index);
	memset(&numastat_table[ix].node, 0, sizeof(numastat_table[ix].node));
    }
    // printf("There are %d numastat table hash collisions.\n", hash_collisions);
    // Open /sys/devices/system/node/node?/numastat for each node and store data in numastat_table.
    for (int node_num = 0;  (node_num < num_nodes);  node_num++) {
        char buf[BUF_SIZE];
        char fname[128];
        snprintf(fname, sizeof(fname), "/sys/devices/system/node/node%d/numastat", node_num);
        FILE *fs = fopen(fname, "r");
        if (!fs) {
            perror(fname);
            exit(EXIT_FAILURE);
        }
        while (fgets(buf, BUF_SIZE, fs)) {
            char *tok[32];
            int tokens = 0;
            const char *delimiters = " \t\r\n:";
            char *p = strtok(buf, delimiters);
            if (p == NULL) {
                continue; // Skip blank lines;
            }
            while (p) {
                tok[tokens++] = p;
                p = strtok(NULL, delimiters);
                // Code below assumes >= 2 tokens
                // example line: "numa_miss 16463"
            }
            int index = hash_lookup(tok[0]);
            if (index >= 0) {
		double MBs = page_size_in_bytes;
		MBs *= (double)atol(tok[1]);
		MBs /= (double)MEGABYTE;
                numastat_table[index].seen = 1;
                numastat_table[index].node[node_num] += MBs;
            } else {
                printf("Token %s not in hash table.\n", tok[0]);
            }
        }
        fclose(fs);
    }
    // Print numastat_table header
    printf("\nPer-node numastat info (in MBs):\n");
    printf("%15s", " ");
    for (int ix = 0;  (ix < num_nodes);  ix++) {
        char node_label[32];
        snprintf(node_label, sizeof(node_label), "Node %d", ix);
        printf("%15s", node_label);
    }
    printf("\n");
    printf("%15s", " ");
    for (int ix = 0;  (ix < num_nodes);  ix++) {
        printf("%15s", "----------");
    }
    printf("\n");
    // Print the numastat memory data
    for (int ix = 0;  (ix < NUMASTAT_TABLE_SIZE);  ix++) {
        if (!numastat_table[ix].seen) {
            continue;
        }
        printf("%15s", numastat_table[ix].label);
	for (int node_ix = 0;  (node_ix < num_nodes);  node_ix++) {
	    printf("%15.2f", numastat_table[ix].node[node_ix]);
	}
	printf("\n");
    }
}  // show_numastat_info


void show_system_info() {
    // Build sys_table hash index and zero the sys_table per-node values
    memset(hash_table, 0, sizeof(hash_table));
    for (int ix = 0;  (ix < SYS_TABLE_SIZE);  ix++) {
        hash_insert(sys_table[ix].token, sys_table[ix].index);
	memset(&sys_table[ix].node, 0, sizeof(sys_table[ix].node));
    }
    // printf("There are %d sys_table hash collisions.\n", hash_collisions);
    // Open /sys/devices/system/node/node?/meminfo for each node and store data in sys_table.
    for (int node_num = 0;  (node_num < num_nodes);  node_num++) {
        char buf[BUF_SIZE];
        char fname[128];
        snprintf(fname, sizeof(fname), "/sys/devices/system/node/node%d/meminfo", node_num);
        FILE *fs = fopen(fname, "r");
        if (!fs) {
            perror(fname);
            exit(EXIT_FAILURE);
        }
        while (fgets(buf, BUF_SIZE, fs)) {
            char *tok[32];
            int tokens = 0;
            const char *delimiters = " \t\r\n:";
            char *p = strtok(buf, delimiters);
            if (p == NULL) {
                continue; // Skip blank lines;
            }
            while (p) {
                tok[tokens++] = p;
                p = strtok(NULL, delimiters);
                // Code below assumes >= 4 tokens
                // example line: "Node 3 Inactive:  210680 kB"
            }
            int index = hash_lookup(tok[2]);
            if (index >= 0) {
		double MBs;
                // start with the right value multiplier
                if (!strncmp("HugePages", tok[2], 9)) {
		    MBs = huge_page_size_in_bytes;
		} else {
		    MBs = KILOBYTE;
		}
		MBs *= (double)atol(tok[3]);
		MBs /= (double)MEGABYTE;
                sys_table[index].seen = 1;
                sys_table[index].node[node_num] += MBs;
            } else {
                printf("Token %s not in hash table.\n", tok[2]);
            }
        }
        fclose(fs);
    }
    // Print sys_table header
    printf("\nPer-node system memory usage (in MBs):\n");
    printf("%15s", " ");
    for (int ix = 0;  (ix < num_nodes);  ix++) {
        char node_label[32];
        snprintf(node_label, sizeof(node_label), "Node %d", ix);
        printf("%15s", node_label);
    }
    printf("\n");
    printf("%15s", " ");
    for (int ix = 0;  (ix < num_nodes);  ix++) {
        printf("%15s", "----------");
    }
    printf("\n");
    // Print the system memory data
    for (int ix = 0;  (ix < SYS_TABLE_SIZE);  ix++) {
        if (!sys_table[ix].seen) {
            continue;
        }
        printf("%15s", sys_table[ix].label);
	for (int node_ix = 0;  (node_ix < num_nodes);  node_ix++) {
	    printf("%15.2f", sys_table[ix].node[node_ix]);
	}
	printf("\n");
    }
    show_numastat_info();
}  // show_system_info


static int starts_with_digit(const struct dirent *dptr) {
    return (isdigit(dptr->d_name[0]));
}


static int node_and_digits(const struct dirent *dptr) {
    char *p = (char *)(dptr->d_name);
    if (*p++ != 'n') return 0;
    if (*p++ != 'o') return 0;
    if (*p++ != 'd') return 0;
    if (*p++ != 'e') return 0;
    do {
	if (!isdigit(*p++)) return 0;
    } while (*p != '\0');
    return 1;
}


static int get_num_nodes() {
    // Count directory names of the form: /sys/devices/system/node/node<N>
    struct dirent **namelist;
    int files = scandir ("/sys/devices/system/node", &namelist, node_and_digits, NULL);
    if (files < 0) {
        perror ("Couldn't open /sys/devices/system/node");
    } else {
        return files;
    }
}


static double get_huge_page_size_in_bytes() {
    double huge_page_size = 0;;
    FILE *fs = fopen("/proc/meminfo", "r");
    if (!fs) {
        perror("Can't open /proc/meminfo");
        exit(EXIT_FAILURE);
    }
    char buf[BUF_SIZE];
    while (fgets(buf, BUF_SIZE, fs)) {
        if (!strncmp("Hugepagesize", buf, 12)) {
            char *p = &buf[12];
            while ((!isdigit(*p)) && (p < buf + BUF_SIZE)) {
                p++;
            }
            huge_page_size = strtod(p, NULL);
            break;
        }
    }
    fclose(fs);
    return huge_page_size * KILOBYTE;
}


void show_info_for_pids_from_pattern_search() {
    // Search all /proc/<PID>/cmdline files and /proc/<PID>/status:Name fields
    // for matching patterns.  Show the memory details for matching PIDs.
    struct dirent **namelist;
    int files = scandir ("/proc", &namelist, starts_with_digit, NULL);
    if (files < 0) {
        perror ("Couldn't open /proc");
    }
    for (int ix = 0;  (ix < files);  ix++) {
        char buf[BUF_SIZE];
	// First get Name field from status file
        int pid = atoi(namelist[ix]->d_name);
        char *p = command_name_for_pid(pid);
        if (p) {
            strcpy(buf, p); 
        } else {
            buf[0] = '\0';
        }
	// Next copy cmdline file contents onto end of buffer.  Do it a
	// character at a time to convert nulls to spaces.
        char fname[128];
        snprintf(fname, sizeof(fname), "/proc/%s/cmdline", namelist[ix]->d_name);
        FILE *fs = fopen(fname, "r");
        if (fs) {
	    p = buf;
	    while (*p != '\0') {
		p++;
	    }
	    *p++ = ' ';
	    int c;
	    while (((c = fgetc(fs)) != EOF) && (p < buf + BUF_SIZE - 1)) {
		if (c == '\0') {
		    c = ' ';
		}
		*p++ = c;
	    }
            *p++ = '\0';
	    fclose(fs);
	}
	if (strstr(buf, pattern)) {
	    if (pid != getpid()) {
		show_process_info(pid, verbose);
	    }
	}
    }
}


int main(int argc, char **argv) {
    prog_name = argv[0];
    // Figure out page sizes and number of nodes
    num_nodes = get_num_nodes();
    page_size_in_bytes = (double)sysconf(_SC_PAGESIZE);
    huge_page_size_in_bytes = get_huge_page_size_in_bytes();
    // Parse program options
    int opt;
    while ((opt = getopt(argc, argv, "n:svV")) != -1) {
	switch (opt) {
	    case 'n': pattern = optarg; break;
	    case 's': show_the_system_info = 1; break;
	    case 'v': verbose = 1; break;
	    case 'V': print_version(); break;
	    default: print_usage_and_exit(); break;
	}
    }
    // If no PIDs specified, show the system-wide info
    if ((optind == argc) && (!pattern)) {
        show_the_system_info = 1;
    }
    // Print info for each specified PID
    int show_sub_categories = (verbose || ((argc - optind) == 1));
    while (optind < argc) {
	int pid = atoi(argv[optind++]);
	show_process_info(pid, show_sub_categories);
    }
    if (pattern) {
        show_info_for_pids_from_pattern_search();
    }
    // Print system-wide info
    if (show_the_system_info) {
        show_system_info();
    }
    exit(EXIT_SUCCESS);
}


