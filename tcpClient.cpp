#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include <cstring>
#include "netdb.h"

using namespace std;

////////////////DATA TYPES FOR READING IN INFO//////////////////////////
struct dataToServer {
   double coord[2];
   char message[20];
   //Total: 32 bytes
};

struct dataFromServer {
   double coord[8][2];
   double pitch[8][2];
   char message[20];
};

//////////////////END OF DATA TYPES/////////////////////////////////////


int main(int argc, char **argv)
{
  int sockfd, portno, rv;
  char buffer[256];
  struct sockaddr_in serv_addr;
  struct hostent *server;

  // #0
  ///////////////USER ERROR CHECKING///////////////
  if(argc < 3)
    {
      cout << "Error " << "Please put in at least 3 arguments\n";
      return 1;
    }

  // #1
  ///////////////Make Socket, DON'T BIND//////////
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd < 0)
    {
      cout << "Error " << "Could not create socket\n";
      return 1;
    }

  // #2
  ////////////Setup server info based on CLI////////
  server = gethostbyname(argv[1]);  // <------------------ARG_1
  if(server == NULL)
    cout << "Error " << "no such host available\n";

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;

  bcopy((char *) server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length); // Copy element of arg1 to arg2 of length arg3

  portno = atoi(argv[2]);           // <------------------ARG_2
  serv_addr.sin_port = htons(portno);

  // #3
  ////////////Connect to the server/////////////////
  if(connect(sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    cout << "Error " << "could not connect\n";

  
  // #4
  ///////////WRITE call for the server//////////////
  bzero(buffer, 256); // To be removed... no longer needed

  dataToServer x;
  x.coord[0] = 3.5;
  x.coord[1] = 312.2;
  x.coord[2] = 57.9;
  x.message[0] = 'h';
  x.message[1] = 'i';
  x.message[2] = '\0';

  rv = write(sockfd, (void *)&x, sizeof(dataToServer));
  if(rv < 0)
     perror("could not write to socket\n");

  //#5
  //////////READ CALL for the server//////////////
  rv = read(sockfd, buffer, 255);
  if(rv < 0)
    cout << "Error " << "could not read from socket\n";

  for(int i = 0; buffer[i] != '\0'; i++)
    cout << buffer[i];
  cout << endl;

  return 1;
}
