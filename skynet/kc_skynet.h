// Procedural dependencies
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <netinet/in.h>
#include "netdb.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
// Object dependencies
#include <strings.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include <cstring>

#ifndef KC_SKYNET
#define KC_SKYNET
#define MAX_CONNECTIONS 16
using namespace std;

enum CLIENT_FLAGS {ASYNC = 0, SYNC = 1}; // Rename ASYNC, SYNC

template <class from_Server, class from_Client>
 class kc_Client {
 private:
  // Shared memory stuff
  from_Server * shm_fromServer;
  from_Client * shm_fromClient;
  bool * accessingFromServerData;
  bool * writePending;

  // Current state flags
  bool initialized;
  bool clientRunning;
  
  // Our process id
  int pid;

  // Strictly networkings stuff
  int sockfd;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  
  int initSharedMemory() {
    ////////////////////SHARED MEMORY STUFF/////////////////            
    // Build our from_server info segment
    int shmid;
    key_t key = 2357;
    if((shmid = shmget(key, sizeof(from_Server), IPC_CREAT | 0666)) < 0) {
      perror("shmget");
      return 1;
    }

    //Attach the from_server segment to our data space
    if((int)(shm_fromServer = (from_Server *)shmat(shmid, NULL, 0)) == -1) {
      perror("shmat");
      return 1;
    }
      
    // Build our from_client info segment      
    key = 7266;
    if((shmid = shmget(key, sizeof(from_Client), IPC_CREAT | 0666)) < 0) {
      perror("shmget");
      return 1;
    }

    //Attach the from_client segment to our data space
    if((int)(shm_fromClient = (from_Client *)shmat(shmid, NULL, 0)) == -1) {
      perror("shmat");
      return 1;
    }
    
    // Build our canRead bools info segment      
    key = 7497;
    if((shmid = shmget(key, sizeof(bool), IPC_CREAT | 0666)) < 0) {
      perror("shmget");
      return 1;
    }
    
    //Attach the from_client segment to our data space
    if((int)(accessingFromServerData = (bool *)shmat(shmid, NULL, 0)) == -1) {
      perror("shmat");
      return 1;
    }
    *accessingFromServerData = false;
      
    key = 5216;
    if((shmid = shmget(key, sizeof(bool), IPC_CREAT | 0666)) < 0) {
      perror("shmget");
      return 1;
    }
      
    //Attach the from_client segment to our data space
    if((int)(writePending = (bool *)shmat(shmid, NULL, 0)) == -1) {
      perror("shmat");
      return 1;
    }
    *writePending = true;
    return 0;
  }
   
 public:
  kc_Client() {initialized = false;}
  kc_Client(string ip, int port) {
     initialized = false;
     init(ip, port);
  }
  int init(string ip, int port) {
     if(!initialized) { 
        if(initSharedMemory())
           return 1;
     }

    // Defaults that bank on possible errors
    pid = -1;
    clientRunning = false;
    
    /////////////////////SOCKET STUFF///////////////
    // #1
    ///////////////Make Socket, DON'T BIND//////////       
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
      printf("Error:Could not create socket\n");
      return 1;
    }
            
    // #2
    ////////////Setup server info based on CLI////////
    server = gethostbyname(ip.c_str());
    if(server == NULL) {
      printf("Error: no such host available\n");
      return 1;
    }
      
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *) server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length); 

    serv_addr.sin_port = htons(port);
    initialized = true;
    return 0;
  }      
  from_Server * getServerDataP() { return shm_fromServer;}
  from_Client * getClientDataP() { return shm_fromClient;}

  // How we read from the the server once we have a new packet.
  from_Server getServerUpdate(bool waitFlag = 1) {
     if(waitFlag == SYNC) 
        while(!*writePending) { usleep(1);} // Await the server's response

    from_Server copy;
    if(!initialized)
       return copy; // Return garbage value
    while(*accessingFromServerData) {usleep(1);}
    *accessingFromServerData = true;  // Block the data from being accessed as use it
    copy = *shm_fromServer;
    *accessingFromServerData = false; // Free the data for writing/reading
    return copy;                // This may be the same data as before.
  }
  // How we write to the server
  void writeData(from_Client input) {
    if(!initialized) return;
    while(!*writePending){usleep(1);} // Wait until it's time to write
    *shm_fromClient = input;
    *writePending = false;
  }
  void skipWrite() {
    if(!initialized) return;
    while(!*writePending){usleep(1);} // Wait until it's time to write
    *writePending = false;   //The same data is sent in the next packet
  }
  int start() {
    if(!initialized) {
       printf("Cannot start unitialized client\n");
       return 1;
    }
    int rv; // Error checking
      
    // #3
    ////////////Connect to the server/////////////////
    if(connect(sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
      printf("Error: could not connect\n");
      return 1;
    }

    clientRunning = true;
    from_Server copy; // what we copy over from the server's data
    pid = fork();
    if(pid == 0) {
         while(1) {
            // #4
            ///////////WRITE call for the server//////////////
            while(*writePending) { usleep(1);}
            rv = write(sockfd, (void *)shm_fromClient, sizeof(from_Client));
            if(rv < 0) {
              printf("could not write to socket\n");
              return 1;
            }
              
            //#5
            //////////READ CALL for the server//////////////
            rv = read(sockfd, &copy, sizeof(from_Server));      
            if(rv < 0) {
              printf("Error could not read from socket\n");
              return 1;
            }
            while(*accessingFromServerData) { usleep(1);}
            *accessingFromServerData = true;
            *shm_fromServer = copy; // Dump the data over...
            *accessingFromServerData = false;

            *writePending = true; // It's time to write again
	  }
      }// Client connection ended... closed here
    return 0;
  }

  void turnOff() {
     if(pid) {
        kill(pid,SIGTERM); // Kill the child process
        clientRunning = false;
     }
  }
};

//Linked-list node for connection tracking (the number of each connection)
struct connection{
  int num;
  connection * next;
};

template <class in_Server, class from_Server, class from_Client>
 class kc_Server {
 private:
  // Shared memory stuff
  in_Server * shm_inServer;
  bool * isAccessingData;
  int  * numConnections;
  int * maxConnections;
  connection *headCon;
  from_Server (*function)(in_Server*, from_Client*, int); 
   
  // Strictly networking stuff 
  int sockfd, newsockfd, clienlen;
  int rv;           // Error checking
  struct sockaddr_in serv_addr, cli_addr;

  bool initialized; // Object security

  int initSharedMemory(int maxConn) { //maxConn may become depricated in further releases
    ////////////////////SHARED MEMORY STUFF/////////////////            
    // Build our from_server info segment
    int shmid;
    key_t key = 2578;
    if((shmid = shmget(key, sizeof(in_Server), IPC_CREAT | 0666)) < 0) {
      perror("shmget");
      return 1;
    }
    //Attach the from_server segment to our data space
    if((int)(shm_inServer = (in_Server *)shmat(shmid, NULL, 0)) == -1) {
      perror("shmat");
      return 1;
    }

    // Build our canRead bools info segment      
    key = 9897;
    if((shmid = shmget(key, sizeof(bool), IPC_CREAT | 0666)) < 0) {
      perror("shmget");
      return 1;
    }
    //Attach the boolean value to our data space
    if((int)(isAccessingData = (bool *)shmat(shmid, NULL, 0)) == -1) {
      perror("shmat");
      return 1;
    }
    //isAccessingData = new bool;
    *isAccessingData = false;
      
    key = 2789;
    if((shmid = shmget(key, sizeof(int), IPC_CREAT | 0666)) < 0) {
      perror("shmget");
      return 1;
    }     
    //Attach the connection tracking number to our server
    if((int)(numConnections = (int *)shmat(shmid, NULL, 0)) == -1) {
      perror("shmat");
      return 1;
    }
    *numConnections = 0;

    key = 2797;
    if((shmid = shmget(key, sizeof(int), IPC_CREAT | 0666)) < 0) {
      perror("shmget");
      return 1;
    }
    //Attach the max connections number to our server
    if((int)(maxConnections = (int *)shmat(shmid, NULL, 0)) == -1) {
      perror("shmat");
      return 1;
    }
    *maxConnections = maxConn;
    
    key = 3613;
    if((shmid = shmget(key, sizeof(connection), IPC_CREAT | 0666)) < 0) {
      perror("shmget");
      return 1;
    } 
    //Attach the head connection node to our server
    if((int)(headCon = (connection *)shmat(shmid, NULL, 0)) == -1) {
      perror("shmat");
      return 1;
    }
    headCon->num = -2; // Special state for first node in our linked list
    headCon->next = NULL;
    return 0;
  }
   
 public:
  kc_Server() {initialized = false;}
  kc_Server(from_Server (* funct)(in_Server *, from_Client*, int), int portno, int maxConns = MAX_CONNECTIONS) {
     initialized = false;
     init(funct, portno, maxConns);
  }
  int init(from_Server (* funct)(in_Server *, from_Client*, int), int portno, int maxConns = MAX_CONNECTIONS) {      
    if(initSharedMemory(maxConns)) return 1;
    function = funct; // Assign our processing function

    // #1
    ///////////////BUILD OUR SOCKET, STRUCTURE, BIND THEM/////////////////
    // Setup our "int" that is a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
      printf("Error opening port\n");
      return 1;
    }    
      
    // Initialize everything to null
    bzero((char *) &serv_addr, sizeof(serv_addr));

    // Set our server address values
    serv_addr.sin_family = AF_INET;         // TCP assignement (presumably)
    serv_addr.sin_addr.s_addr = INADDR_ANY; // Get whatevere address we are
    serv_addr.sin_port = htons(portno);     // Assign the port

    // OK. We combine the socket we have with that server address value
    // to "combine them." Prof. Vinod Pillai gives an excellent tutorial YT
    // We can bind different sockets we create to the same (or a copy of the) 
    // serv_addr for multiple distinct connections on the same port.
    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
      printf("Error on binding\n");
      return 1;
    }
      
    // #2
    /////////////////////LISTEN STATE////////////////////
    // We say we want to handle up to x clients at the same time
    listen(sockfd, 5); // I read somewhere that more than "5" is bad
    initialized = true; // We are ready to start the server
    return 0;
  }
  in_Server getDataCopy() {
    in_Server copy = *shm_inServer;
    return copy; 
  }

  int getNumPlayers() {
    return *numConnections;
  }

  // Verify that this connection is still active
  bool isConnected(int connectionNum) {
    connection * cursor = headCon;
    if(connectionNum > *numConnections)
      return false;
    for(int i = 0; i < connectionNum; i++)
      cursor = cursor->next;
    return (cursor->num >= 0);
  }

  // Make a connection available on our linked list
  void dropConnection(int connectionNum) {
    connection * cursor = headCon;
    if(connectionNum > *numConnections)
      return;
    for(int i = 0; i < connectionNum; i++)
      cursor = cursor->next;
    if(cursor->num != -1) {
      --*numConnections;
      cursor->num = -1;    
    }
  }

  // Allocate a connection number for the newly connected client
  int registerConnection() {
    if(*numConnections > *maxConnections)
      return -1;
    int con = 0;
    connection * cursor = headCon;
    ++*numConnections;      // We're getting another client, so add one
    if(cursor->num == -2) { // If this is the first connection
      cursor = headCon;
      cursor->next = NULL;
      return cursor->num = con;
    }
    else {
      // If there is an available node in our list, use it
      while(cursor->next) {
	if(cursor->num == -1)
	  return cursor->num = con;
	cursor = cursor->next;
	++con;
      }
      if(cursor->num == -1) return cursor->num = con;

      // If we need to make space for a new node
      cursor->next = new connection;
      cursor = cursor->next;
      cursor->next = NULL;
      return (cursor->num = *numConnections) - 1;
    }
  }

  int getMaxPlayers() { 
    return *maxConnections; 
  }
 
  in_Server * getDataDirect() { 
   return shm_inServer; 
  }

  bool getAccessStatus() { 
    return *isAccessingData;  
  }
  
  int start() {
    // Make sure we aren't running an uninitialized server
    if(!initialized) return 1;

    // Dynamic size made conveniently available in this variable
    clienlen = sizeof(cli_addr);

    //Fork process portion
    pid_t pid = 1;
    while(pid) { // Add conditional for the server being running or NOT running
      printf("\tListening for clients...\n");
      newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t *)&clienlen);
      pid = fork();
    }

    int idNumber = -1;
    if(newsockfd < 0 || (*numConnections == *maxConnections -1)) {
      printf("Error on accept\n"); //Socket bad OR server is "full"
      return;
    }
    else {
	idNumber = registerConnection();
	from_Client input;
	from_Server response; 	
	while(1) {
	    // #3 
	    ////////////////////RECEIVE & READ STATE//////////////	    
	    rv = read(newsockfd, (void *)&input, sizeof(input));
	    if(rv < 1) // Connection dropped 
              break; 

	    // #4 
	    // Run function pointer that was passed into our server
	    while(*isAccessingData){usleep(1);} // Data protection for write data
	    *isAccessingData = true; 	   
	    if(!(response = function(shm_inServer, &input, idNumber))) 
	      break; // When the function returns a 'NULL' value we exit our loop
	    *isAccessingData = false;
            
	    // #5
	    ///////////////////RESPOND BACK////////////////////////
	    rv = write(newsockfd, &response, sizeof(from_Server));
	    if(rv < 1)
              break;
	  }
      }
    // Close this server connection here. 
    dropConnection(idNumber);    
    printf("\tConnection dropped\n");
    exit(0);
  }
};


#endif

