#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>
using namespace std;

#define MAX_ARGS 128

// SIG

void init_sigaction();

void handle_zombie(int sig) {
	init_sigaction();
	int status = 0;
	pid_t pid = -1;
	do {
		if (pid != -1)
			cout << endl << "process[" << pid << "]: terminated with status " << status;
		pid = wait(&status);
	} while (pid != -1);
}

void init_sigaction() {
	signal(SIGCHLD, handle_zombie);
}

// STRINGS

char delim[] = " ";
char* av[MAX_ARGS];
int aI;

void execute(bool amp) {
	int status;
	pid_t cid = vfork();
	switch (cid) {
	case -1: // can't
		cerr << "can't fork a new process";
		break;
	case 0: // i'm a child
		if (execvp(av[0], av) == -1)
			cerr << "no such program \'" << av[0] << "\'", exit(1);
		break;
	default:
		// i'm a parent
		cout << "process[" << cid << "]: started";
		if (!amp) {
			cout << " in foreground";
			wait(&status);
			cout << endl << "process[" << cid << "]: terminated with status " << status;
		}
		cout << endl;
		break;
	}
}

// MAIN

int main() {

	// init
	char line[1 << 16];
	init_sigaction();
	while (cout << " _$") {
		// input
		cin.getline(line, sizeof line);
		if (!strlen(line))
			continue;
		// pipe
		char *tok = strtok(line, delim);
		if (strcmp(tok, "exit") == 0)
			break;
		while (tok) {
			// token
			aI = -1;
			bool amp = false;
			while (tok) {
				++aI;
				free(av[aI]);
				av[aI] = (char*) malloc(strlen(tok) + 1);
				strcpy(av[aI], tok);
				if (strcmp(av[aI], "&") == 0)
					amp = true;
				tok = strtok(NULL, " ");
				if (strcmp(av[aI], "|") == 0) {
					--aI;
					break;
				}
			}
			if (strcmp(av[aI], "&") == 0)
				--aI;
			for (int i = aI + 1; i < MAX_ARGS; ++i)
				if (av[i])
					free(av[i]), av[i] = 0;
//			memset(av[i], '\0', sizeof av[i]);
			// execute
			execute(amp);
		}
	}
	return 0;
}

/*

 echo 1 2 3 4 5 6 7 8 9
 gedit
 gedit & | gnome-shell & | gnome-calculator & | charmap &

 */
