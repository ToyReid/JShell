// Toy Reid
// Lab 7 - jsh3
/* jsh is a barebones shell that reads lines from standard input.
   If they are non-blank, jsh3 attempts to execute them. File redirection
   with <, >, and >> is supported. Piping with | is also supported. */

#include "jsh.h"

void tFork(ProgInfo *pi, int ampFound, int fdToStdin, int fdToStdout) {
	int child, status, pid;
	
	child = fork();
	(pi->args)[pi->numArgs] = NULL; // Null terminate command string

	if(child == 0) {
		if(fdToStdin != -1) {
			tDup2(fdToStdin, 0);
			close(fdToStdin);
		}
		if(fdToStdout != -1) {
			tDup2(fdToStdout, 1);
			close(fdToStdout);
		}
		execvp(pi->args[0], pi->args);
		perror(pi->args[0]);
		exit(1);
	}

	if(!ampFound) { // Wait unless & symbol was found
		while(1) {
			pid = wait(&status);
			if(pid == child) break;
		}
	}

	pi->numArgs = 0;
}

void tDup2(int f1, int f2) {
	if(dup2(f1, f2) < 0) {
      perror("jsh3: dup2(f1, f2)");
      exit(1);
    }
    close(f1);
}

void processFile(ProgInfo *pi, int print) {
	int i, ampFound, fdToStdout, fdToStdin, pipeFound, pipeDone, haveInput;
	int pipefd[2];
	IS is = new_inputstruct(NULL);

	while(get_line(is) >= 0) { // Process all lines in file
		if(is->NF != 0 && (is->fields[0] == (void *)EOF)) break;
		ampFound = 0; pipeFound = 0; fdToStdout = -1; fdToStdin = -1;
		pipeDone = 0; haveInput = 0;
		pi->numArgs = 0;
		for(i = 0; i < is->NF; i++) { // Copy inputstruct's fields into program's arugments
			// Check for & and redirect symbols
			if(!strcmp(is->fields[i], "&")) ampFound = 1;
			else if(!strcmp(is->fields[i], ">")) { // Next field used for output; overwrite with O_TRUNC
				i++;
				fdToStdout = open(is->fields[i], O_WRONLY | O_TRUNC | O_CREAT, 0644);
				if(fdToStdout < 0) exit(1);
			}
			else if(!strcmp(is->fields[i], "<")) { // Next field used for input
				i++;
				fdToStdin = open(is->fields[i], O_RDONLY);
				if(fdToStdin < 0) exit(1);
				haveInput = 1;
			}
			else if(!strcmp(is->fields[i], ">>")) { // Next field used for output; append with O_APPEND
				i++;
				fdToStdout = open(is->fields[i], O_WRONLY | O_APPEND | O_CREAT, 0644);
				if(fdToStdout < 0) exit(1);
			}
			else if(!strcmp(is->fields[i], "|")) { // Process pipe
				if(pipe(pipefd) < 0) {
					perror("pipe");
					exit(1);
				}
				close(fdToStdout);
				fdToStdout = pipefd[1]; // Set write file to pipe's write file
				if(haveInput == !(pipeDone == 0)) close(fdToStdin);
				pipeDone = 1;
				pipeFound = 1;
			}
			else { // Field is a command, so add it to args
				pi->args[pi->numArgs] = strdup(is->fields[i]);
				pi->numArgs++;
			}

			if(is->NF - 1 == i || pipeFound) { // Last field or encountered pipe, need to fork process
				tFork(pi, ampFound, fdToStdin, fdToStdout);
				// Close and reset file descriptors
				if(fdToStdin != -1) {
					close(fdToStdin);
					fdToStdin = -1;
				}
				if(fdToStdout != -1) {
					close(fdToStdout);
					fdToStdout = -1; 
				}
				if(pipeFound) {
					fdToStdin = pipefd[0]; // Set read file to pipe's read file
					pipeFound = 0; // Reset to handle other pipes
				}
				haveInput = 0; // 
			}
		}
		if(print) { printf("jsh3: "); fflush(stdout); }
	}
	jettison_inputstruct(is);
}

int main(int argc, char** argv) {
	int status, zomb;
	ProgInfo *pi;
	struct rusage rusage;
	int print = 0;

	pi = malloc(sizeof(struct progInfo));
	if((strcmp(argv[1], "-") == 0) && argc > 1) print = 0;
	else print = 1;
	pi->args = malloc(256); // Arbitrary buffer size
	pi->numArgs = 0;

	if(print) { printf("jsh3: "); fflush(stdout); }

	processFile(pi, print);

	while(1) { // Kill zombies
		zomb = wait3(&status, WNOHANG, &rusage);
		if(zomb < 1) break;
	}
	free(pi->args);
	free(pi);

	return 0;
}
