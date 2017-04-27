/* tcp-client.c */

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main()
{
  /* create TCP client socket (endpoint) */
  int sd = socket( PF_INET, SOCK_STREAM, 0 );

  if ( sd < 0 )
  {
    perror( "socket() failed" );
    exit( EXIT_FAILURE );
  }

#if 0
  /* localhost maps to 127.0.0.1, which stays on the local machine */
  struct hostent * hp = gethostbyname( "localhost" );

  struct hostent * hp = gethostbyname( "128.113.126.29" );
#endif

  struct hostent * hp = gethostbyname( "localhost" );


  if ( hp == NULL )
  {
    fprintf( stderr, "ERROR: gethostbyname() failed\n" );
    return EXIT_FAILURE;
  }

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  memcpy( (void *)&server.sin_addr, (void *)hp->h_addr, hp->h_length );
  unsigned short port = 9876;
  server.sin_port = htons( port );

  printf( "server address is %s\n", inet_ntoa( server.sin_addr ) );


  printf( "connecting to server.....\n" );
  if ( connect( sd, (struct sockaddr *)&server, sizeof( server ) ) == -1 )
  {
    perror( "connect() failed" );
    return EXIT_FAILURE;
  }

  /* The implementation of the application layer is below... */
  FILE* fp = fopen("sonny1978.jpg", "r");
  char c[100774];
  int i=0;
  if(fp == NULL) 
   {
      perror("Error in opening file");
      return(-1);
   }
   i = fread(c, 1, 100774, fp);
   if (i != 100774) {
    printf("error reading.....\n");
    printf(" i = %d\n", i);
   }

   fclose(fp);


// printf("gonna sprint\n");
  char msg[100800];
  sprintf(msg, "SAVE sonny1978.jpg 100774\n");
  for (i = 26; i < 100800; i++) {
    msg[i] = c[i];
  }
  // printf("sprinted\n");
  int n = write( sd, msg, 100800 );
  
  // char msg[25] = "READ sonny1978.jpg 9 11\n";
  // char* msg = "SAVE chicken.txt 31\nThe quick brown chicken jumps.\n";
  // int n = write( sd, msg, strlen(msg) );

  printf("n = %d\n", n);

  if ( n < strlen( msg ) )
  {
    perror( "write() failed" );
    return EXIT_FAILURE;
  }

  char buffer[ BUFFER_SIZE ];
  n = read( sd, buffer, BUFFER_SIZE - 1 );    /* BLOCKING */

  if ( n == -1 )
  {
    perror( "read() failed" );
    return EXIT_FAILURE;
  }
  else if ( n == 0 )
  {
    printf( "Rcvd no data; also, server socket was closed\n" );
  }
  else
  {
    buffer[n] = '\0';    /* assume we rcvd text-based data */
    printf( "Rcvd from server: %s", buffer );
  }

  close( sd );

  return EXIT_SUCCESS;
}
