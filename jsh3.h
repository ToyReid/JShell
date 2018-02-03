// Toy Reid
// Lab 7 - jsh3
/* jsh3 is a barebones shell that reads lines from standard input.
   If they are non-blank, jsh3 attempts to execute them. File redirection
   with <, >, and >> is supported. Piping with | is also supported. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <fcntl.h>
#include "fields.h"

/* Struct used to hold all args as well as the number of args currently tracked */
typedef struct progInfo {
	int numArgs;
	char** args;
} ProgInfo;

/* tFork forks and executes a process given by args. If files have been
   redirected, tFork dups stdin and stdout to the proper file descriptors.
   If ampFound is not true, it will wait for the forked process to finish. */
void tFork(ProgInfo *pi, int ampFound, int fdToStdin, int fdToStdout);

/* Used to call dup2 on the appropriate file descriptors and then closes the old
   descriptor. */
void tDup2(int fd1, int fd2);

/* processFile performs the majority of I/O using Dr. Plank's inputstructs.
   Reads commands into args, then properly dups stdin and stdout based on given
   file redirection parameters, and finally calls tFork to fork and execute them.
   It handles piping by calling pipe() and running the pipe's read/write file
   descriptors to piped commands' original file descriptors. */
void processFile(ProgInfo *pi, int print);