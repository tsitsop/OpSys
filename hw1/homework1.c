#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

int main(int argc, char* argv[]) {
	int outputLength = 0;

	/* Command line argument check */
	if (argc < 2 || argc > 3) {
		fprintf (stderr, "ERROR: Invalid number of arguments.\n");
		
		return EXIT_FAILURE;
	} else if (argc == 2) {
		outputLength = 0;
	} else {
		outputLength = atoi(argv[2]);
	}

	/* Open File */
	FILE* fp = fopen(argv[1], "r");
	if (fp == NULL) {
		fprintf(stderr, "ERROR: Could not open %s\n", argv[1]);

		return EXIT_FAILURE;
	}

	char word[80];
	char** words = calloc(8, sizeof(char*));
	if (words == NULL) {

	}
	int* wordCount = calloc(8, sizeof(int));
	if (wordCount == NULL) {

	}
	int maxSize = 8;
	int uniqueCount, totalCount = 0;

	printf("Allocated initial parallel arrays of size 8.\n");

	while (1) { 
		/* Read in non-alphanumeric characters */
		fscanf(fp, "%*[^A-Za-z0-9]");

		/* If the current character is the end of the file, we've finished reading the input */
		if (feof(fp)) {
			break;
		}

		/* Read in the next word or number */
		fscanf(fp, "%[A-Za-z0-9]", word);

		/* Loop through saved words to see if this word is included already */
		int i;
		for (i = 0; i < uniqueCount; i++) {
			if (strcmp(words[i], word) == 0) {
				wordCount[i] += 1;

				break;
			}
		}

		/* If the word was not found in words, update the arrays. */
		if (i == uniqueCount) {
			/* If the arrays have become too small, double their size. */ 
			if (uniqueCount == maxSize) {
				maxSize *= 2;

				char** newWords = realloc(words, maxSize*sizeof(char*));
				// If failed to allocate the memory, free old stuff and quit.
				if (newWords == NULL) {
					fprintf(stderr, "ERROR: Error allocating memory.\n");
					int j;
					for (j=0; j < uniqueCount; j++) {
						free(words[j]);
					}

					return EXIT_FAILURE;
				}
				words = newWords;

				int* newWordCount = realloc(wordCount, maxSize*sizeof(int));
				// If failed to allocate the memory, free old stuff and quit.
				if (newWordCount == NULL) {
					fprintf(stderr, "ERROR: Error allocating memory.\n");
					free(wordCount);

					return EXIT_FAILURE;
				}
				wordCount = newWordCount;

				printf("Re-allocated parallel arrays to be size %d\n", maxSize);
			}

			words[i] = malloc(sizeof(word));
			strcpy(words[i], word);
			wordCount[i] = 1;
		
			uniqueCount += 1;
		}

		totalCount += 1;
	}

	printf("All done (successfully read %d words; %d unique words).\n", totalCount, uniqueCount);
	printf("All words (and corresponding counts) are:\n");

	/* If no outputLength was specified, must print all words. */
	if (outputLength == 0) {
		outputLength = uniqueCount;
	}

	int j;
	for (j = 0; j < outputLength; j++) {
		printf("%s -- %d\n", words[j], wordCount[j]);
	}

	/* Close file */
	if (fclose(fp) != 0) {
		fprintf(stderr, "ERROR: Could not close %s\n", argv[1]);
	}

	/* Free allocated memory */
	for (j = 0; j < uniqueCount; j++) {
		free(words[j]);
	}
	free(words);
	free(wordCount);

	return EXIT_SUCCESS;
}