#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

/* Function Headers */
int getChars(char*, int);


int main(int argc, char* argv[]) {
	if (argc < 2) {
		fprintf(stderr, "ERROR: No files to read.\n");
	}

	int i;
	pid_t pid;
	int pcp[argc-1][2]; // parent child pipe
	int cgp[argc-1][3][2]; //child grandchild pipe
	int rc;
	int status;
	int status1;
	int* totalCounts = calloc(argc-1, sizeof(int));

	#ifdef DEBUG_MODE
		printf("In parent process! \n");
		fflush(stdout);
	#endif

	printf("PID %d: Program started (top-level process)\n", getpid());
	fflush(stdout);

	/* Create child process for each file */
	for (i = 0; i < argc-1; i++) {
		/* Create pipe between parent and child */
		rc = pipe(pcp[i]);
    	if ( rc == -1 ) {
			perror( "pipe() failed" );
			exit(EXIT_FAILURE);
		}

		/* Create child process */
		pid = fork();
		if (pid == -1) {
    		perror("ERROR: fork() failed");
    		exit(EXIT_FAILURE);
		} else if (pid == 0) { /* In child process */
			/* Create pipes between child and grandchildren */
			int z;
			for (z = 0; z < 3; z++) {
				rc = pipe(cgp[i][z]);
		    	if ( rc == -1 ) {
					perror( "pipe() failed" );
					exit(EXIT_FAILURE);
				}
			}
			
			/* Create grandchildren */
			pid_t gpids[3];
			char* type[3] = {"alphanumeric", "whitespace", "other"};
			for (z = 0; z < 3; z++) {
				gpids[z] = fork();
				if (gpids[z] == -1) {
	    			perror("ERROR: fork() failed");
	    			exit(EXIT_FAILURE);
				} else if (gpids[z] == 0) { /* In grandchild process */
					int chars = getChars(argv[i+1], z); 
					if ( write(cgp[i][z][1], &chars, sizeof(int)) == -1) {
						fprintf(stderr, "ERROR: Could not write to pipe.\n");
						exit(EXIT_FAILURE);
					}
					printf("PID %d: Sent %s count of %d to parent (then exiting)\n", getpid(), type[z], chars);
					fflush(stdout);

					exit(EXIT_SUCCESS);
				}
			}

			printf("PID %d: Processing %s (created three child processes)\n", getpid(), argv[i+1]);
			fflush(stdout);

			int charCounts[3];
			for (z = 0; z < 3; z++) {
				wait(&status);
				if (read(cgp[i][z][0], &charCounts[z], sizeof(int)) == -1) {
					fprintf(stderr, "ERROR: Could not read from pipe.\n");
					exit(EXIT_FAILURE);
				}
			}

			printf("PID %d: File %s contains %d alnum, %d space, and %d other characters\n", getpid(), 
				argv[i+1], charCounts[0], charCounts[1], charCounts[2]);
			fflush(stdout);

			if (write(pcp[i][1], &charCounts, 3*sizeof(int)) == -1) {
				fprintf(stderr, "ERROR: Could not write to pipe.\n");
				exit(EXIT_FAILURE);
			}

			printf("PID %d: Sent %s counts to parent\n", getpid(), argv[i+1]);
			fflush(stdout);

			exit(EXIT_SUCCESS);
		} else {
			printf("PID %d: Created child process for %s\n", getpid(), argv[i+1]);
			fflush(stdout);
		}
	
	}
	
	/* Read in character counts from children */
	for (i=0; i < argc-1; i++) {
		wait(&status1);
		
		int chars[3];
		int z;

		if (read(pcp[i][0], &chars, 3*sizeof(int)) == -1) {
			fprintf(stderr, "ERROR: Could not read from pipe.\n");
			exit(EXIT_FAILURE);
		}

		for(z = 0; z < 3; z++) {
			totalCounts[z] += chars[z];
			#ifdef DEBUG_MODE
				printf("i = %d, chars[%d] = %d\n", i, z, chars[z]);
				fflush(stdout);
			#endif
			fflush(stdout);
		}
	}

	printf("PID %d: All files contain %d alnum, %d space, and %d other characters\n", getpid(), 
				totalCounts[0], totalCounts[1], totalCounts[2]);
	fflush(stdout);
	printf("PID %d: Program ended (top-level process)\n", getpid());
	fflush(stdout);

	return EXIT_SUCCESS;
}

int getChars(char* fileName, int type) {
	#ifdef DEBUG_MODE
		if (type == 0) {
			printf("In alphanumeric grandchild process! I am looking in %s\n", fileName);
			fflush(stdout);
		} else if (type == 1) {
			printf("In whitespace grandchild process!   I am looking in %s\n", fileName );
			fflush(stdout);
		} else if (type == 2) {
			printf("In other grandchild process!        I am looking in %s\n", fileName);
			fflush(stdout);
		}
	#endif

	/* Open File */
	FILE* fp = fopen(fileName, "r");
	if (fp == NULL) {
		fprintf(stderr, "ERROR: Could not open %s\n", fileName);
		exit(EXIT_FAILURE);
	}

	int count = 0;
	char c;
	while (!feof(fp)) { 
		c = fgetc(fp);
		if (type == 0 && isalnum(c)) {
			count += 1;
		} else if (type == 1 && isspace(c)) {
			count += 1;
		} else if (type == 2 && !isalnum(c) && !isspace(c)) {
			count += 1;
		}
	}	

	/* Close file */
	if (fclose(fp) != 0) {
		fprintf(stderr, "ERROR: Could not close %s\n", fileName);
		exit(EXIT_FAILURE);
	}

	/* Remove EOF character from other count */
	if (type == 2) {
		count -= 1;
	}

	return count;
}