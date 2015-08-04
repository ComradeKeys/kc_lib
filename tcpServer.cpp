#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <iostream>
#include <unistd.h>
#include <string>

using namespace std;

////////////////DATA TYPES FOR READING IN INFO//////////////////////////
struct data {
   double coord[2];
   char message[20];
};
//////////////////END OF DATA TYPES/////////////////////////////////////

int main(int argc, char **argv)
{
  int sockfd, newsockfd, portno, clienlen;
  char buffer[256];
  struct sockaddr_in serv_addr, cli_addr;
  int rv;

  // #0
  /////////////////CHECK USER INPUT//////////////////////////////////////
  if(argc < 2)
    {
      cout << "Error " << "Insufficient parameters\n";
      return 1;
    }

  // #1
  ///////////////BUILD OUR SOCKET, STRUCTURE, BIND THEM//////////////////
  // Setup our "int" that is a socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd < 0)
    cout << "Error " << "Error opening port\n";

  // Initialize everything to null
  bzero((char *) &serv_addr, sizeof(serv_addr));

  // Set our server address values
  portno = atoi(argv[1]);                 // User specified port conversion
  serv_addr.sin_family = AF_INET;         // TCP assignement (presumably)
  serv_addr.sin_addr.s_addr = INADDR_ANY; // Get whatever address we have on the PC 
  serv_addr.sin_port = htons(portno);     // Assign the port
  
  // OK. SO here is the magic. We combine the socket we have with that server address value
  // to "combine them." Prof. Vinod Pillai gives an excellent tutorial on youtube. We can
  // bind different sockets we create to the same (or a copy of the) serv_addr for multiple
  // distinct connections on the same port.
  if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    cout << "Error " << "Error on binding\n";


  // #2
  /////////////////////LISTEN STATE////////////////////
  // We say we want to handle up to 5 clients at the same time... WOW!
  listen(sockfd, 5); 

  // Dynamic size made conveniently available in this variable
  clienlen = sizeof(cli_addr);


  // #3
  ////////////////////RECEIVE & READ STATE//////////////
  // create a file descriptor for the client who is connecting to use

  //Fork process portion
  pid_t pid = 1;

  while(pid)
    {
      newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t *)&clienlen);
      pid = fork();
    }
  if(newsockfd < 0)
    cout << "Error " << "Error on accept\n";

  // Clear our buffer
  bzero(buffer, 256);

  // Pull that client's data from the socket INTO our buffer
  data incoming;
  rv = read(newsockfd, (void *)&incoming, sizeof(incoming));  
  if(rv < 0)
    cout << "Error " << "Error reading from socket\n";
  
  cout << "RECEIVED RV IS: \n"
       << "\tcoord 0: " << incoming.coord[0] << endl
       << "\tcoord 1: " << incoming.coord[1] << endl
       << "\tcoord 2: " << incoming.coord[2] << endl
       << "\tmessage: " << incoming.message << endl << endl;

  // #4
  ///////////////////RESPOND BACK////////////////////////
  rv = write(newsockfd, "I got your message", 18);
  if(rv < 0)
    cout << "Error " << "Error writing to socket";

return 1;
}
