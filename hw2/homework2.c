#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

int main(int argc, char* argv[]) {
	if (argc < 2) {
		fprintf(stderr, "ERROR: NO files to read.\n");
	}

	pid_t = pid;

	/* Create child process for each file */
	int i;
	for (i = 0; i < argc-1; i++) {
		pid = fork();

		if (pid == -1) {
    		perror("ERROR: fork() failed");
		}

		/* Send file name to child via pipe */


		if (pid == 0) { /* In child process */
			int status;

			/* Create alphanumeric child */
			pid = fork();

			if (pid == -1) {
    			perror("ERROR: fork() failed");
			}

			if (pid == 0) { /* In child process */
				// search file for all alphanumeric characters and count them up
				// send number via pipe to parent?
			} else { /* In parent process */
				// need to wait for my child (the grandchild) to terminate
				wait(&status);
				// send number via pipe to parent?
			}

			/* Create whitespace child */
			pid = fork();

			if (pid == -1) {
    			perror("ERROR: fork() failed");
			}

			if (pid == 0) { /* In child process */
				// search file for all whitespace characters and count them up
				// send number via pipe to parent?
			} else { /* In parent process */
				// need to wait for my child (the grandchild) to terminate 
				wait(&status);
				// send number via pipe to parent?
			}

			/* Create other child */
			pid = fork();

			if (pid == -1) {
    			perror("ERROR: fork() failed");
			}

			if (pid == 0) { /* In child process */
				// search file for all other characters and count them up
				// send number via pipe to parent?
			} else { /* In parent process */
				// need to wait for my child (the grandchild) to terminate
				wait(&status);
				// send number via pipe to parent?
			}
		} else { /* In parent process */
			// need to wait for my children to terminate
			// extract data from pipe?
		}
	}


	return EXIT_SUCCESS;
}


/*

p[0] reads from buffer
p[1] writes to buffer

-- fd table always copied from parent to child - can be reading and writing to same buffer


-- going to need to transfer what file each child reads from somehow

-- then going to need to store character data in buffer - array??
	- If have array where 
		a[0] = num alphanumerical
		a[1] = num whitespace
		a[2] = num other
	  each child can read current data in buffer and add their counts to it then write it back. 








*/