#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/* define global variables */
unsigned int counts[3] = {0,0,0};

/* struct sent to children */
typedef struct {
    char* fileName;
} child_t;

/* struct sent to grandchildren */
typedef struct {
    char* fileName;
    int charType;
} grandchild_t;

/* function headers */
void* child_thread_function(void*);
void* grandchild_thread_function(void*);

int main(int argc, char* argv[]) {
    if (argc < 2) {
		fprintf(stderr, "ERROR: No files to read.\n");
        return EXIT_FAILURE;
	}

    int i = 0;
    pthread_t tid[argc-1];

    /* create all children */
    for (i = 0; i < argc-1; i+=1) {
        /* dynamically allocate memory for the next thread */
        child_t* child = malloc(sizeof(child_t));
        child->fileName = argv[i];

        /* create child thread */
        int rc = pthread_create(&tid[i], NULL, child_thread_function, child);
        if ( rc != 0 ) {
            fprintf( stderr, "pthread_create() failed (%d): %s\n",
                    rc, strerror( rc ) );
            return EXIT_FAILURE;
        }
    }

    /* wait for child threads to return */
    for (i = 0; i < argc-1; i++) {
        unsigned int * x[];
        pthread_join( tid[i], (void **)&x );    /* BLOCKING CALL */

        counts[0] += x[0];
        counts[1] += x[1];
        counts[2] += x[2];

        printf( "MAIN: Joined a child thread that returned %u.\n", *x );
        free( x );
    }
}

void* child_thread_function(void* arg) {
    child_t* child = (child_t*) arg;

    int j = 0;
    pthread_t gtid[3];
    unsigned int* c = malloc(3*sizeof(unsigned int)); /* character counts */

    /* create all grandchildren */
    for (j = 0; j < 3; j++) {
        /* dynamically allocate memory for the next thread */
        grandchild_t* grandchild = malloc(sizeof(grandchild_t));
        grandchild->fileName = child->fileName;
        grandchild->charType = j;

        /* create grandchild thread */
        int rc = pthread_create(&gtid[j], NULL, grandchild_thread_function, grandchild);
        if ( rc != 0 ) {
            fprintf( stderr, "pthread_create() failed (%d): %s\n",
                    rc, strerror( rc ) );
            return EXIT_FAILURE;
        }
    }

    /* wait for grandchildren to complete */
    for (j = 0; j < 3; j++) {
        unsigned int * x;
        pthread_join( tid[j], (void **)&x );    /* BLOCKING CALL */
        c[j] = x;
        printf( "MAIN: Joined a child thread that returned %u.\n", *x );
        free( x );
    }

    /* send the character counts to parent upon completion */
    pthread_exit(c);
}

void* grandchild_thread_function(void* arg) {
    grandchild_t grandchild = (grandchild_t*) arg;
    #ifdef DEBUG_MODE
		if (grandchild->charType == 0) {
			printf("In alphanumeric grandchild process! I am looking in %s\n", grandchild->fileName);
			fflush(stdout);
		} else if (grandchild->charType == 1) {
			printf("In whitespace grandchild process!   I am looking in %s\n", grandchild->fileName );
			fflush(stdout);
		} else if (grandchild->charType == 2) {
			printf("In other grandchild process!        I am looking in %s\n", grandchild->fileName);
			fflush(stdout);
		}
	#endif

	/* Open File */
	FILE* fp = fopen(grandchild->fileName, "r");
	if (fp == NULL) {
		fprintf(stderr, "ERROR: Could not open %s\n", grandchild->fileName);
		exit(EXIT_FAILURE);
	}

	unsigned int count = 0;
	char c;
	while (!feof(fp)) { 
		c = fgetc(fp);
		if (grandchild->charType == 0 && isalnum(c)) {
			count += 1;
		} else if (grandchild->charType == 1 && isspace(c)) {
			count += 1;
		} else if (grandchild->charType == 2 && !isalnum(c) && !isspace(c)) {
			count += 1;
		}
	}	

    /* Remove EOF character from other count */
	if (grandchild->charType == 2) {
		count -= 1;
	}

	/* Close file */
	if (fclose(fp) != 0) {
		fprintf(stderr, "ERROR: Could not close %s\n", grandchild->fileName);
		exit(EXIT_FAILURE);
	}

    pthread_exit(count);
}