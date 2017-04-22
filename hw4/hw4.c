#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/select.h>

/* function headers */
void* tcp_thread(void*);
void* udp_thread(void*);

int main(int argc, char* argv[]) {
	/* make sure there are enough arguments */
	if (argc != 3) {
		perror("ERROR: too few arguments.");

		return EXIT_FAILURE;
	}

	/* make sure storage directory exists */
	DIR* directory = opendir("storage");
	if (!directory) {
		perror("ERROR: storage directory not found.");

		return EXIT_FAILURE;
	} else {
		closedir(directory)
	}

	/***************************************/
	/************** TCP Setup **************/
	/***************************************/
  	int tcpsd;
  	struct sockaddr_in tcpserver;
  	int tcplength
  	
  	/* create TCP socket */
	tcpsd = socket(PF_INET, SOCK_STREAM, 0);
	if (tcpsd == -1) {
		perror("ERROR: socket() failed");
	
		return EXIT_FAILURE;
	}

	tcpserver.sin_family = PF_INET;
  	tcpserver.sin_addr.s_addr = INADDR_ANY;
  	tcpserver.sin_port = htons(argv[1]); // <-------------------------------assign port number for tcp

  	tcplength = sizeof(tcpserver);

  	if (bind(tcpsd, (struct sockaddr *)&tcpserver, tcplength) == -1) {
		perror("ERROR: bind() failed");

		return EXIT_FAILURE;
	}

	/* identify this port as a listener port */
	if (listen(tcpsd, 5) == -1 ) {
		perror("ERROR: listen() failed");

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
		perror("ERROR: Failed to create the socket");
	}
	udpserver.sin_family = AF_INET;
	udpserver.sin_addr.s_addr = INADDR_ANY; // <--- htonl(INADDR_ANY)
	udpserver.sin_port = htons(argv[2]); // <-------------------------------assign port number for udp
	
	/* bind to a specific (OS-assigned) port number */
	if ( bind( udpsd, (struct sockaddr *) &udpserver, sizeof( udpserver ) ) < 0 ) {
		perror("ERROR: bind() failed");
		
		return EXIT_FAILURE;
  	}

  	udplength = sizeof(udpserver);

  	if ( getsockname( udpsd, (struct sockaddr *) &udpserver, (socklen_t *) &udplength ) < 0 ) {
	    perror("ERROR: getsockname() failed");
    
    	return EXIT_FAILURE;
  	}

  	printf("Started server\n");
  	printf("Listening for TCP connections on port: %d\n", ntohs(tcpserver.sin_port));
  	printf("Listening for UDP datagrams on port: %d\n", ntohs(udpserver.sin_port));
  	fflush(stdout);

	/* listen for connections and datagrams */
	fd_set readfds;
	int client_sockets[ 100 ];
	int client_socket_index = 0;  /* next free spot */
	struct sockaddr_in client;
	int fromlen = sizeof(client);
	char buffer[]
	char* command = malloc(5*sizeof(char));
	int n;
	int numStorageFiles = 0;
	char** storageFileNames;
	int numStorageFilesBytes = 1;

	while (1) {
		FD_ZERO(&readfds);
		FD_SET(tcpsd, &readfds);
		FD_SET(udpsd, &readfds);

		/* BLOCK */
	    int ready = select(FD_SETSIZE, &readfds, NULL, NULL, NULL);

	    /* if tcp connection */
	    if (FD_ISSET(tcpsd, &readfds)) {
	    	int newsock = accept(tcpsd, (struct sockaddr *)&client, (socklen_t *)&fromlen);

	    	printf("Rcvd incoming TCP connection from: %s\n", newsock);
	    	fflush(stdout);
	    	client_sockets[ client_socket_index++ ] = newsock;

	    	/* need to spawn thread for child */
	    }

	    /* if udp datagram */
	    if (FD_ISSET(udpsd, &readfds)) {
	    	int newsock = accept(udpsd, (struct sockaddr *)&client, (socklen_t *)&fromlen);

	    	printf("Rcvd incoming UDP datagram from: %s\n", newsock);
	    	fflush(stdout);
	    	client_sockets[ client_socket_index++ ] = newsock;

	    	/* need to do this iteratively */
	    	while(1) {
	    		/* read a datagram from the remote client side (BLOCKING) */
    			n = recvfrom(udpsd, command, 5, 0, (struct sockaddr *) &client, (socklen_t *) &fromlen);
    			command[5] = '\0';

    			if (n < 0) {
    				perror("ERROR: recvfrom() failed");
    			}

    			if (!strcmp(command, "LIST")) {
    				fprintf(stderr, "ERROR: Invalid command %s\n", command);
    			}

    			if (numStorageFiles != 0) {
    				numStorageFilesBytes = floor(log10(abs(numStorageFiles))) + 1;
    			}

    			
    			printf("Received %s\n", command);
    			fflush(stdout);

    			char* listStr = malloc(numStorageFilesBytes + 1); /* "numStorageFiles\0" */

    			/* create string to send back */
    			sprintf(listStr, "%d", numStorageFiles);

    			int i = 0;
    			char* newListStr;
    			for (i = 0; i < numStorageFiles; i++) {
    				newListStr = malloc(strlen(listStr) + strlen(storageFileNames[i]) + 1);
    				strcat(newListStr, listStr);
    				strcat(newListStr, ' ' + storageFileNames[i]);
    				listStr = malloc(strlen(newListStr)+1);
    				strcpy(listStr, newListStr);
    			}

    			listStr = realloc(listStr, strlen(newListStr)+2);
    			strcat(listStr, '\n');

    			printf("Sent %s\n", listStr);

    			n = strlen(listStr);

    			sendto(udpsd, listStr, n, 0, (struct sockaddr *) &client, fromlen);
	    	}
	    }
	}  	

	return EXIT_SUCCESS;
}



/* ADD ERROR CHECKING EVERYWHERE - when concatenating strings, allocating memory etc */