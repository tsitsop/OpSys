#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <dirent.h>
#include <math.h>
#include <pthread.h>
#include <ctype.h>
#include <sys/stat.h>

/* struct sent to children */
typedef struct {
    int tcpsd;
    struct sockaddr_in client;
    socklen_t fromlen;
} tcpInfo;

pthread_mutex_t m;


/* function headers */
void* tcp_thread(void*);
void getList(char*, int*, char**);
int saveFile(tcpInfo*, char*, int);
int readFile(tcpInfo*, char*);


int main(int argc, char* argv[]) {
	/* make sure there are enough arguments */
	if (argc != 3) {
		fprintf(stderr, "ERROR too few arguments.\n");

		return EXIT_FAILURE;
	}

	/* make sure storage directory exists */
	DIR* directory = opendir("storage");
	if (!directory) {
		perror("ERROR storage directory not found.");

		return EXIT_FAILURE;
	} else {
		closedir(directory);
	}

	/***************************************/
	/************** TCP Setup **************/
	/***************************************/
  	int tcpsd;
  	struct sockaddr_in tcpserver;
  	int tcplength;
  	
  	/* create TCP socket */
	tcpsd = socket(PF_INET, SOCK_STREAM, 0);
	if (tcpsd == -1) {
		perror("ERROR socket() failed");
	
		return EXIT_FAILURE;
	}

	tcpserver.sin_family = PF_INET;
  	tcpserver.sin_addr.s_addr = INADDR_ANY;
  	tcpserver.sin_port = htons(atoi(argv[1]));

  	tcplength = sizeof(tcpserver);

  	if (bind(tcpsd, (struct sockaddr *)&tcpserver, tcplength) == -1) {
		perror("ERROR bind() failed");

		return EXIT_FAILURE;
	}

	/* identify this port as a listener port */
	if (listen(tcpsd, 5) == -1 ) {
		perror("ERROR listen() failed");

		return EXIT_FAILURE;
	}

	/***************************************/
	/************** UDP Setup **************/
	/***************************************/
	int udpsd;
	struct sockaddr_in udpserver;
	int udplength;

	/* create UDP socket */
	udpsd = socket(AF_INET, SOCK_DGRAM, 0);
	if (udpsd < 0) {
		perror("ERROR Failed to create the socket");
	}
	udpserver.sin_family = AF_INET;
	udpserver.sin_addr.s_addr = INADDR_ANY; // <--- htonl(INADDR_ANY)
	udpserver.sin_port = htons(atoi(argv[2]));
	
	/* bind to a specific (OS-assigned) port number */
	if ( bind( udpsd, (struct sockaddr *) &udpserver, sizeof( udpserver ) ) < 0 ) {
		perror("ERROR bind() failed");
		
		return EXIT_FAILURE;
  	}

  	udplength = sizeof(udpserver);

  	if ( getsockname( udpsd, (struct sockaddr *) &udpserver, (socklen_t *) &udplength ) < 0 ) {
	    perror("ERROR getsockname() failed");
    
    	return EXIT_FAILURE;
  	}

  	printf("Started server\n");
  	printf("Listening for TCP connections on port: %d\n", ntohs(tcpserver.sin_port));
  	printf("Listening for UDP datagrams on port: %d\n", ntohs(udpserver.sin_port));
  	fflush(stdout);


  	//////////////////////////////////////////
	/* listen for connections and datagrams */
	//////////////////////////////////////////
	fd_set readfds;
	struct sockaddr_in client;
	int fromlen = sizeof(client);
	int n;
	pthread_t tid;


	while (1) {
		FD_ZERO(&readfds);
		FD_SET(tcpsd, &readfds);
		FD_SET(udpsd, &readfds);

		/* BLOCK */
	    int ready = select(FD_SETSIZE, &readfds, NULL, NULL, NULL);

	    if (ready == -1) {
	    	perror("ERROR select() failed");

	    	return EXIT_FAILURE;
	    }

	    /* if tcp connection */
	    if (FD_ISSET(tcpsd, &readfds)) {
	    	int newsock = accept(tcpsd, (struct sockaddr *)&client, (socklen_t *)&fromlen);

	    	printf("Rcvd incoming TCP connection from: %s\n", inet_ntoa(client.sin_addr));
	    	fflush(stdout);

	    	tcpInfo* info = malloc(sizeof(tcpInfo));
	    	info->tcpsd = newsock;
	    	info->client = client;
	    	info->fromlen = fromlen;  

	    	/* create child thread */
	        int rc = pthread_create(&tid, NULL, tcp_thread, info);
	        if ( rc != 0 ) {
	            fprintf( stderr, "ERROR pthread_create() failed (%d): %s\n", rc, strerror( rc ) );
	            exit(EXIT_FAILURE);
	        }
	    }

	    /* if udp datagram */
	    if (FD_ISSET(udpsd, &readfds)) {
	    	char command[6];
	    		/* read a datagram from the remote client side (BLOCKING) */
    			n = recvfrom(udpsd, command, 5, 0, (struct sockaddr *) &client, (socklen_t *) &fromlen);

    			if (n < 0) {
    				perror("ERROR recvfrom() failed");
    				exit(EXIT_FAILURE);
    			}

    			printf("Rcvd incoming UDP datagram from: %s\n", inet_ntoa(client.sin_addr));
	    		fflush(stdout);

    			command[5] = '\0';
    			if (strncmp(command, "LIST\n", 5) != 0) {
    				char errorMessage[24] = "ERROR Invalid Command\n\0"; 
    				sendto(udpsd, errorMessage, 24, 0, (struct sockaddr *) &client, fromlen);
    				fprintf(stderr, "ERROR Invalid command\n");
    				continue;
    			}

				printf("Received LIST\n");
				fflush(stdout);

    			char* listStr;
    			getList(command, &n, &listStr);
    			
    			sendto(udpsd, listStr, n-1, 0, (struct sockaddr *) &client, fromlen);
	    		printf("Sent %s", listStr);
    			fflush(stdout);

	    		free(listStr);
	    }
	}  	

	close(tcpsd);
	close(udpsd);

	return EXIT_SUCCESS;
}

void* tcp_thread(void* connectionInfo) {
	tcpInfo* info = (tcpInfo*)connectionInfo;

	int n;
	int n2 = 0;
	n2++;
	char command[5];
	char buffer[1025];
	char* returnStr;


	do {
		/* read the command */
		n = recv(info->tcpsd, buffer, 1024, 0 );

		buffer[n] = '\0';
		sscanf(buffer, "%4s%*s", command);

		command[4] = '\0';

		if (n == -1) {
	        perror( "ERROR recv() failed" );
	 		pthread_detach(pthread_self());
	        pthread_exit(NULL);
	    } else if (n == 0) {
	    	break;
	    }
	    // printf("got %s\n", buffer);
	    // printf("^-- this was %d bytes\n", n);

	    //printf("[child %u] Received %s", (unsigned int)pthread_self(), buffer);
		//fflush(stdout);

	    if (!strncmp(command, "LIST", 5)) { /* list files */
	    	printf("[child %u] Received %s", (unsigned int)pthread_self(), buffer);
			fflush(stdout);
	
			getList(command, &n, &returnStr);		
	
			n2 = send( info->tcpsd, returnStr, n-1, 0 );
			printf("[child %u] Sent %s", (unsigned int)pthread_self(), returnStr);
			fflush(stdout);
	
			free(returnStr);
		} else if (!strncmp(command, "SAVE", 5)) { /* save file */
			if (saveFile(info, buffer, n) == 1) {
				n = send( info->tcpsd, "ACK\n", 4, 0 );
				printf("[child %u] Sent ACK\n", (unsigned int)pthread_self());
				fflush(stdout);
			}
			continue;
		} else if (!strncmp(command, "READ", 5)) { /* read file */
			readFile(info, buffer);
			
			continue;
		}else {
			fprintf(stderr, "ERROR INVALID COMMAND\n");
			n = send(info->tcpsd, "ERROR INVALID COMMAND\n", 22, 0);
			continue;
		}
		
	} while (n > 0);


	printf("[child %u] Client disconnected\n", (unsigned int)pthread_self());
	
	close(info->tcpsd);

	pthread_detach(pthread_self());
	pthread_exit(NULL);
}


int saveFile(tcpInfo* info, char* buffer, int messageSize) {
	int i = 0;
	int n;
	char* fileName;
	int bytes;
	char* msg = malloc(1024);
	int sizecommand;
	char* extraptr;

	              /* SAVE fname bytes\nmessage */
	if (sscanf(buffer, "%*s %ms %d\n%*s", &fileName, &bytes) != 2) {
		printf("%s\n", buffer );
		fprintf(stderr, "ERROR INVALID REQUEST p\n");
		n = send(info->tcpsd, "ERROR INVALID REQUEST\n", 22, 0);
		return 0;
	}

	n = sprintf(msg, "SAVE %s %d\n", fileName, bytes);
	sizecommand = n; // this is the size of "SAVE filname bytes\n" */

	printf("[child %u] Received %s", (unsigned int)pthread_self(), msg );
	fflush(stdout);


	// char extra[messageSize-sizecommand];
	
	// copy messagsize-sizecommand bytes from buffer to extra, starting at memory location 
	// memcpy(extra, &buffer[sizecommand], messageSize-sizecommand);
	extraptr = &buffer[sizecommand];



	// printf("n = %d\n", n); // n aka length of the command
	// printf("messageSize = %d\n", messageSize ); // size of entire message
	// printf("command size = %d\n", sizecommand ); // size of command
	// printf("message size - command = %d\n", messageSize-sizecommand); // size of the stuff to be written
	// printf("Extra = %s\n", extra);

	/* make sure fileName is valid */
	if (strlen(fileName) > 32) {
		fprintf(stderr, "ERROR INVALID REQUEST\n");
		n = send(info->tcpsd, "ERROR INVALID REQUEST\n", 22, 0);
		return 0;
	}

	while (fileName[i] != '\0') {
		/* as long as the char is alphanumeric or a '.', it is valid */
		if (!isalnum(fileName[i]) && !fileName[i] == '.') {
			fprintf(stderr, "ERROR INVALID REQUEST\n");
			n = send(info->tcpsd, "ERROR INVALID REQUEST\n", 22, 0);
			return 0;
		}
		i++;
	}

	/* make sure number of bytes is valid */
	if (bytes < 1) {
		fprintf(stderr, "ERROR INVALID REQUEST\n");
		n = send(info->tcpsd, "ERROR INVALID REQUEST\n", 22, 0);
		return 0;
	}

	/* file checking... */
	struct stat statbuffer;
	char filepath[40];
	sprintf(filepath, "storage/%s", fileName);

	/* if the file already exists, send error */
	if (lstat(filepath, &statbuffer) == 0) {
		printf("[child %u] Sent ERROR FILE EXISTS\n", (unsigned int)pthread_self());
		n = send(info->tcpsd, "ERROR FILE EXISTS\n", 18, 0);

		return 0;
	}

	FILE* fp;
	int totBytes = 0;
	pthread_mutex_lock(&m);
	fp = fopen(filepath, "wb+");

	n = fwrite(extraptr, 1, messageSize - sizecommand, fp);
	totBytes += n;

	int written = 0;
	while (totBytes < bytes) {
		/* read the command */
		n = recv(info->tcpsd, buffer, 1024, 0 );
		totBytes += n;

		written = fwrite(buffer, 1, n, fp);
		if (written != n) {
			perror("ERROR WRITING TO FILE");
			n = send(info->tcpsd, "ERROR WRITING TO FILE\n", 22, 0);
			if (fclose(fp) != 0) {
				perror("ERROR CLOSING FILE");
				n = send(info->tcpsd, "ERROR CLOSING FILE\n", 19, 0);
			}
			return 0;
		}
	}

	if (fclose(fp) != 0) {
		perror("ERROR CLOSING FILE");
		n = send(info->tcpsd, "ERROR CLOSING FILE\n", 19, 0);
		return 0;
	}
	pthread_mutex_unlock(&m);

	printf("[child %u] Stored file \"%s\" (%d bytes)\n", (unsigned int)pthread_self(), fileName, bytes);

	free(fileName);

	return 1;
}

int readFile(tcpInfo* info, char* buffer) {
	int i = 0;
	int n = 0;
	char* fileName;
	int offset;
	int length;


	              /* READ fname offset length\n */
	if (sscanf(buffer, "%*s %ms %d %d\n", &fileName, &offset, &length) != 3) {
		fprintf(stderr, "ERROR INVALID REQUEST\n");
		n = send(info->tcpsd, "ERROR INVALID REQUEST\n", 22, 0);
		return 0;
	}

	printf("[child %u] Received %s", (unsigned int)pthread_self(), buffer);
	fflush(stdout);

	/* make sure fileName is valid */
	if (strlen(fileName) > 32) {
		fprintf(stderr, "ERROR INVALID REQUEST\n");
		n = send(info->tcpsd, "ERROR INVALID REQUEST\n", 22, 0);
		return 0;
	}

	while (fileName[i] != '\0') {
		/* as long as the char is alphanumeric or a '.', it is valid */
		if (!isalnum(fileName[i]) && !fileName[i] == '.') {
			fprintf(stderr, "ERROR INVALID REQUEST\n");
			n = send(info->tcpsd, "ERROR INVALID REQUEST\n", 22, 0);
			return 0;
		}
		i++;
	}

	/* make sure valid offset */
	if (offset < 0) {
		fprintf(stderr, "ERROR INVALID REQUEST\n");
		n = send(info->tcpsd, "ERROR INVALID REQUEST\n", 22, 0);
		return 0;
	}

	/* make sure valid length */
	if (length < 1) {
		fprintf(stderr, "ERROR INVALID REQUEST\n");
		n = send(info->tcpsd, "ERROR INVALID REQUEST\n", 22, 0);
		return 0;
	}

	struct stat statbuffer;
	char filepath[40];
	sprintf(filepath, "storage/%s", fileName);

	/* if the file doesn't exist, send error */
	if (lstat(filepath, &statbuffer) != 0) {
		printf("[child %u] Sent ERROR NO SUCH FILE\n", (unsigned int)pthread_self());
		n = send(info->tcpsd, "ERROR NO SUCH FILE\n", 20, 0);

		return 0;
	}

	/* check byte range */
	if (statbuffer.st_size < offset + length) {
		printf("[child %u] Sent ERROR INVALID BYTE RANGE\n", (unsigned int)pthread_self());
		n = send(info->tcpsd, "ERROR INVALID BYTE RANGE\n", 26, 0);

		return 0;
	}

	// printf("file size is %d\n", (int) statbuffer.st_size );


	/* READ FILE */
	FILE* fp;

	char bytes[length+1];
	// char trash[1024];
	// printf("gonna lock\n");
	pthread_mutex_lock(&m);
	// printf("locked\n");
	fp = fopen(filepath, "rb");
	if (fp == NULL) {
		fprintf(stderr, "ERROR OPENING FILE DURING READ\n");
		return 0;
	}	

	// make sure fp at beginning
	rewind(fp);

   // printf("gonna read offset%d\n", offset);
	/* skip over offset... */
	// n = fread(trash, 1, offset, fp);
	// printf("n = %d\n", n);
	fseek(fp, offset, SEEK_SET);

	if ( n != 0) {
		perror("ERROR READING OFFSET");
		
		return 0;
	}

	// printf("getting relevant bytes\n");
	/* save relevant bytes */
	n = fread(bytes, 1, length, fp);
	if ( n != length) {
		perror("ERROR READING RELEVANT");
		
		return 0;
	}

	// printf("got relevant bytes\n");
	/* close file */
	if (fclose(fp) != 0) {
		perror("ERROR CLOSING FILE");
		n = send(info->tcpsd, "ERROR CLOSING FILE\n", 19, 0);

		return 0;
	}

	char msg[1024];
	int num = n;

	if (sprintf(msg, "ACK %d\n", n) == -1) {
		printf("ERROR with sprintf in read\n");
	}
	n = send(info->tcpsd, msg, strlen(msg), 0);
	if (n != strlen(msg)) {
		printf("ERROR SENDING ack message in read\n");
	}
	n = send(info->tcpsd, bytes, num, 0);
	if (n != num) {
		printf("ERROR SENDING bytes message in read\n");
	}

	printf("[child %u] Sent %s",  (unsigned int)pthread_self(), msg);

	sprintf(msg, "[child %u] Sent %d bytes of \"%s\" from offset %d", (unsigned int)pthread_self(), num, fileName, offset);
	printf("%s\n", msg);

	pthread_mutex_unlock(&m);

	return 1;
}

void getList(char* command, int* n, char** listStr) {
	int numStorageFiles = 0;
	int numStorageFilesBytes = 1;
	struct dirent** fileList;

	pthread_mutex_lock(&m);
	numStorageFiles = scandir("storage", &fileList, NULL, alphasort);
	pthread_mutex_unlock(&m);
	if (numStorageFiles == -1) {
		perror("ERROR scandir() failed\n");
		exit(EXIT_FAILURE);
	} else if (numStorageFiles != 2) {
		numStorageFilesBytes = floor(log10(abs(numStorageFiles - 2))) + 1;
	}

	*listStr = malloc(numStorageFilesBytes + 1); /* "numStorageFiles\0" */

	/* create string to send back */
	sprintf(*listStr, "%d", numStorageFiles-2); /* listStr = "numStorageFiles\0" */
	*n = strlen(*listStr); /* n = numStorageFileBytes  <-- doesnt include the null terminator*/

	int i = 0;
	char* newListStr;
	for (i = 0; i < numStorageFiles; i++) {
		if (i < 2) {
			continue;
		}

		*n = *n + 1 + strlen(fileList[i]->d_name); /* n = numStorageFileBytes + 1 (for space) + length of file name (not includeing null bit) */
		newListStr = malloc(*n + 1); /* newListStr = size of n + 1 (for null bit) */ 
		strcpy(newListStr, *listStr); /* copies the old listStr variable to newListStr, adding a null byte at end */
		newListStr[strlen(*listStr)] = ' '; /* overwrite the null byte with a space */
		newListStr[strlen(*listStr) + 1] = '\0'; /* add null byte after the space */
		strcat(newListStr, fileList[i]->d_name); /* overwrite the null byte, add the file name, add the null byte after file name */
		*listStr = realloc(*listStr, strlen(newListStr)+1);/* resize the original listStr so that it can fit the newListStr. Need +1 because strlen doesnt include null byte */
		strcpy(*listStr, newListStr); /* copy new string into old one, adding the null byte*/
	}

	i = numStorageFiles;
	while (i--) {
		free(fileList[i]);
	}
	free(newListStr);
	free(fileList);

	*n += 2; /* increase n by 2, 1 for the '\n' and 1 for the null byte */

	*listStr = realloc(*listStr, *n);
	(*listStr)[*n-2] = '\n';
	(*listStr)[*n-1] = '\0'; 
}



/* ADD ERROR CHECKING EVERYWHERE - when concatenating strings, allocating memory etc 
   - need to list in alphabetical order
*/
