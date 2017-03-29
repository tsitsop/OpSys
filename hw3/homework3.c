#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>

/* define global variables */
unsigned int counts[3] = {0,0,0};
pthread_mutex_t m;

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

    FILE * file;
    for (i = 0; i < argc-1; i++) {
        file = fopen(argv[i+1], "r");
        if (file != NULL) {
            fclose(file);
            free(file);
        } else {
            fprintf(stderr, "ERROR: %s not found.\n", argv[i+1]);
            free(file);
            return EXIT_FAILURE;
        }
    }
    
    printf("THREAD %u: Program started (top-level thread)\n", (unsigned int)pthread_self());
    fflush(stdout);

    pthread_t tid[argc-1];

    /* create all children */
    for (i = 0; i < argc-1; i+=1) {
        /* dynamically allocate memory for the next thread */
        child_t* child = malloc(sizeof(child_t));
        child->fileName = argv[i+1];

        /* create child thread */
        int rc = pthread_create(&tid[i], NULL, child_thread_function, child);
        if ( rc != 0 ) {
            fprintf( stderr, "pthread_create() failed (%d): %s\n",
                    rc, strerror( rc ) );
            return EXIT_FAILURE;
        }
        printf("THREAD %u: Created child thread for %s\n", (unsigned int)pthread_self(), argv[i+1]);
        fflush(stdout);
    }

    /* wait for child threads to return */
    for (i = 0; i < argc-1; i++) {
        unsigned int ** x = malloc (3*sizeof(int*));
        pthread_join( tid[i], (void **)&x );    /* BLOCKING CALL */

        counts[0] += *x[0];
        counts[1] += *x[1];
        counts[2] += *x[2];
        free( x );
    }

    printf("THREAD %u: All files contain %d alnum, %d space, and %d other characters\n", 
        (unsigned int)pthread_self(), counts[0], counts[1], counts[2]);
    fflush(stdout);

    printf("THREAD %u: Program ended (top-level thread)\n", (unsigned int)pthread_self());
    fflush(stdout);

    return 0;
}

/* create grandchildren threads, aggregate current file information */
void* child_thread_function(void* arg) {
    child_t* child = (child_t*) arg;

    printf("THREAD %u: Processing %s (created three child threads)\n", (unsigned int)pthread_self(), child->fileName);
    fflush(stdout);

    int j = 0;
    pthread_t gtid[3];
    int** c = malloc(3*sizeof(unsigned int)); /* character counts */

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
            exit(1);
        }
    }
    
    /* wait for grandchildren to complete */
    for (j = 0; j < 3; j++) {
        int** x = malloc(sizeof(int*));
        pthread_join( gtid[j], (void**)x );    /* BLOCKING CALL */
        c[j] = *x;
        free(x);
    }

    printf("THREAD %u: File %s contains %d alnum, %d space, and %d other characters\n", (unsigned int)pthread_self(), 
				child->fileName, *c[0], *c[1], *c[2]);

    /* send the character counts to parent upon completion */
    pthread_exit(c);
}

/* count characters for file */
void* grandchild_thread_function(void* arg) {
    pthread_mutex_lock(&m);

    grandchild_t* grandchild = (grandchild_t*) arg;
    char* types[3] = {"alphanumeric", "whitespace", "other"};

	/* Open File */
	FILE* fp = fopen(grandchild->fileName, "r");
	if (fp == NULL) {
		fprintf(stderr, "ERROR: Could not open %s\n", grandchild->fileName);
		exit(EXIT_FAILURE);
	}

    int* count = malloc(sizeof(int));
    *count = 0;
	char c;
	while (!feof(fp)) { 
		c = fgetc(fp);
		if (grandchild->charType == 0 && isalnum(c)) {
			*count += 1;
		} else if (grandchild->charType == 1 && isspace(c)) {
			*count += 1;
		} else if (grandchild->charType == 2 && !isalnum(c) && !isspace(c)) {
			*count += 1;
		}
	}	

    /* Remove EOF character from other count */
	if (grandchild->charType == 2) {
		*count -= 1;
	}

	/* Close file */
	if (fclose(fp) != 0) {
		fprintf(stderr, "ERROR: Could not close %s\n", grandchild->fileName);
		exit(EXIT_FAILURE);
	}

    printf("THREAD %u: Added %s count of %d to totals (then exiting)\n", (unsigned int)pthread_self(), types[grandchild->charType], *count);
	fflush(stdout);

    pthread_mutex_unlock(&m);
    pthread_exit(count);
}