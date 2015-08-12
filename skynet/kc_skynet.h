#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <netinet/in.h>
#include "netdb.h"
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include <cstring>

#ifndef KC_SKYNET
#define KC_SKYNET
#define MAX_CONNECTIONS 16
using namespace std;


template <class from_Server, class from_Client>
class kc_Client {
 private:
  // Shared memory stuff
  from_Server * shm_fromServer;
  from_Client * shm_fromClient;
  bool * isAccessingServer;
  bool * writePending;
  bool * runClient;
   
  // Strictly networkings stuff 
  int sockfd;
  struct sockaddr_in serv_addr;
  struct hostent *server;

  void initSharedMemory()
  {
    cout << "\tInitializing client...\n";

    ////////////////////SHARED MEMORY STUFF/////////////////            
    // Build our from_server info segment
    int shmid;
    key_t key = 2357;
    if((shmid = shmget(key, sizeof(from_Server), IPC_CREAT | 0666)) < 0) {
      perror("shmget");
      return;
    }

    //Attach the from_server segment to our data space
    if((int)(shm_fromServer = (from_Server *)shmat(shmid, NULL, 0)) == -1) {
      perror("shmat");
      return;
    }
    //    shm_fromServer = new from_Server;      
      
    // Build our from_client info segment      
    key = 7266;
    if((shmid = shmget(key, sizeof(from_Client), IPC_CREAT | 0666)) < 0) {
      perror("shmget");
      return;
    }

    //Attach the from_client segment to our data space
    if((int)(shm_fromClient = (from_Client *)shmat(shmid, NULL, 0)) == -1) {
      perror("shmat");
      return;
    }
    //    shm_fromClient = new from_Client;

    // Build our canRead bools info segment      
    key = 7497;
    if((shmid = shmget(key, sizeof(bool), IPC_CREAT | 0666)) < 0) {
      perror("shmget");
      return;
    }

    //Attach the from_client segment to our data space
    if((int)(isAccessingServer = (bool *)shmat(shmid, NULL, 0)) == -1) {
      perror("shmat");
      return;
    }
    //isAccessingServer = new bool;
    *isAccessingServer = false;
      
    key = 5216;
    if((shmid = shmget(key, sizeof(bool), IPC_CREAT | 0666)) < 0) {
      perror("shmget");
      return;
    }
      
    //Attach the from_client segment to our data space
    if((int)(writePending = (bool *)shmat(shmid, NULL, 0)) == -1) {
      perror("shmat");
      return;
    }
    //    writePending = new bool;
    *writePending = true;

    key = 3312;
    if((shmid = shmget(key, sizeof(bool), IPC_CREAT | 0666)) < 0) {
      perror("shmget");
      return;
    }
      
    //Attach the do we continue the client communication segment to our data space
    if((int)(runClient = (bool *)shmat(shmid, NULL, 0)) == -1) {
      perror("shmat");
      return;
    }
    //    runClient = new bool;
    *runClient = true;
    cout << "\tFinisihed initializing client\n";
  }
   
 public:
  kc_Client() {;}
  kc_Client(string ip, int port) {
    init(ip, port);
  }
  void init(string ip, int port) {      
    initSharedMemory();
    *runClient = true;
    /////////////////////SOCKET STUFF//////////////////////      
    // #1
    ///////////////Make Socket, DON'T BIND//////////       
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
      cout << "Error " << "Could not create socket\n";
      return;
    }
            
    // #2
    ////////////Setup server info based on CLI////////
    server = gethostbyname(ip.c_str());
    if(server == NULL) {
      cout << "Error " << "no such host available\n";
      return;
    }
      
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *) server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length); 

    serv_addr.sin_port = htons(port);
  }      
  from_Server * getServerDataP() { return shm_fromServer;}
  from_Client * getClientDataP() { return shm_fromClient;}
  // How we read from the the server once we have a new packet.
  from_Server getServerUpdate()  {
    from_Server copy;
    while(*isAccessingServer){;}
    *isAccessingServer = true; // Block the data from being read/written as we read it
    copy = *shm_fromServer;
    *isAccessingServer = false; // Free the data for writing/reading
    return copy; // This may be the same data as before... Potentially optimize this..
  }
  // How we write to the server
  void writeData(from_Client input) {
    if(*writePending);
      //!;cout << "\t\tWRITE PENDING IN WRITE-DATA FUNCTION\n";

    //!cout << "ON THE USER LEVEL P: " << writePending << endl;
    //!cout << "\tin this 'writeData' we are inserting: " << input << endl;
    *shm_fromClient = input;
    *writePending = false;
    //if(!*writePending)
    //  cout << "\t\tWRITE NOTE PENDING\n";
  }
  void skipWrite() {
    *writePending = false;
  }
  void start() {
    int rv; // Error checking
      
    // #3
    ////////////Connect to the server/////////////////
    if(connect(sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
      cout << "Error " << "could not connect\n";
      return;
    }
       
    from_Server copy; // what we copy over from the server's data
    pid_t pid = fork();
    if(pid == 0)
      {
	while(*runClient) 
	  {
            // #4
            ///////////WRITE call for the server//////////////
            while(*writePending) { 
	      ;
	      /*
	      if(!*writePending)
		cout << "\t\tWRITE NOTE PENDING\n";
	      else
	      cout << "\t\tWRITE PENDING\n"; */
	    }
	    //!cout << "\tWriting to the server\n";
            rv = write(sockfd, (void *)shm_fromClient, sizeof(from_Client));
            if(rv < 0)
	      perror("could not write to socket\n");
            *writePending = true;
            
            //#5
            //////////READ CALL for the server//////////////
	    //!cout << "\tReading from the server\n";
            rv = read(sockfd, &copy, sizeof(from_Server));      
            if(rv < 0)
	      cout << "Error " << "could not read from socket\n";
            while(*isAccessingServer){;}
            *isAccessingServer = true;
            *shm_fromServer = copy; // Dump the data over...
            *isAccessingServer = false;
	    //!cout << "\tWe read " << *shm_fromServer << " from the server\n" << endl;
	  }
	// Client connection ended... we close here...
      }
  }
  void turnOff() {*runClient = false;}
};


template <class in_Server, class from_Server, class from_Client>
class kc_Server {
 private:
  // Shared memory stuff
  in_Server * shm_inServer;
  bool * isAccessingData;
  bool * serverRunning;
  bool * childFlag;
  int  * numConnections;
  int * maxConnections;
  from_Server (*function)(in_Server*, from_Client*, int); // The function that the client-server operates on. Each thread processes data here.
   
  // Strictly networking stuff 
  int sockfd, newsockfd, clienlen;
  int rv; // Error checking
  struct sockaddr_in serv_addr, cli_addr;

  int secureId() {
    return 5;//BLARG -> FIX THIS YESTERDAY
  }

  void initSharedMemory(int maxConn)
  {
    ////////////////////SHARED MEMORY STUFF/////////////////            
    // Build our from_server info segment
    int shmid;
    key_t key = 2578;
    if((shmid = shmget(key, sizeof(in_Server), IPC_CREAT | 0666)) < 0) {
      perror("shmget");
      return;
    }

    //Attach the from_server segment to our data space
    if((int)(shm_inServer = (in_Server *)shmat(shmid, NULL, 0)) == -1) {
      perror("shmat");
      return;
    }
    //shm_inServer = new in_Server;      

    // Build our canRead bools info segment      
    key = 9897;
    if((shmid = shmget(key, sizeof(bool), IPC_CREAT | 0666)) < 0) {
      perror("shmget");
      return;
    }

    //Attach the boolean value to our data space
    if((int)(isAccessingData = (bool *)shmat(shmid, NULL, 0)) == -1) {
      perror("shmat");
      return;
    }
    //isAccessingData = new bool;
    *isAccessingData = false;
      
    key = 8280;
    if((shmid = shmget(key, sizeof(bool), IPC_CREAT | 0666)) < 0) {
      perror("shmget");
      return;
    }
      
    //Attach the child creating bit segment to our shared memory
    if((int)(childFlag = (bool *)shmat(shmid, NULL, 0)) == -1) {
      perror("shmat");
      return;
    }
    //childFlag = new bool;
    *childFlag = true;

    key = 6236;
    if((shmid = shmget(key, sizeof(bool), IPC_CREAT | 0666)) < 0) {
      perror("shmget");
      return;
    }
      
    //Attach the do we continue the server communication segment to our data space
    if((int)(serverRunning = (bool *)shmat(shmid, NULL, 0)) == -1) {
      perror("shmat");
      return;
    }
    //serverRunning = new bool;
    *serverRunning = true;

    key = 2789;
    if((shmid = shmget(key, sizeof(int), IPC_CREAT | 0666)) < 0) {
      perror("shmget");
      return;
    }
      
    //Attach the connection tracking number to our server
    if((int)(numConnections = (int *)shmat(shmid, NULL, 0)) == -1) {
      perror("shmat");
      return;
    }
    //numConnections = new int;
    *numConnections = 0;

    key = 2797;
    if((shmid = shmget(key, sizeof(int), IPC_CREAT | 0666)) < 0) {
      perror("shmget");
      return;
    }
      
    //Attach the max connections number to our server
    if((int)(maxConnections = (int *)shmat(shmid, NULL, 0)) == -1) {
      perror("shmat");
      return;
    }
    //maxConnections = new int;
    *maxConnections = maxConn;
  }
   
 public:
  kc_Server() {;}
  kc_Server(from_Server (* funct)(in_Server *, from_Client*, int), int portno, int maxConns = MAX_CONNECTIONS) {
    init(funct, portno, maxConns);
  }
  void init(from_Server (* funct)(in_Server *, from_Client*, int), int portno, int maxConns = MAX_CONNECTIONS) {      
    initSharedMemory(maxConns);
    function = funct; // Assign our processing function

    // #1
    ///////////////BUILD OUR SOCKET, STRUCTURE, BIND THEM//////////////////
    // Setup our "int" that is a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
      cout << "Error " << "Error opening port\n";

    // Initialize everything to null
    bzero((char *) &serv_addr, sizeof(serv_addr));

    // Set our server address values
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
    // We say we want to handle up to x clients at the same time... WOW!
    listen(sockfd, 5); // I read somewhere that more than "5" causes issues for some reason...
  }
  in_Server getDataCopy() {
    in_Server copy = *shm_inServer;
    return copy; 
  }

  int getNumPlayers() {
    return *numConnections;
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
  
  void turnOff() { 
    *serverRunning = false;
  }

  void start()
  {
    // Dynamic size made conveniently available in this variable
    clienlen = sizeof(cli_addr);

    //Fork process portion
    pid_t pid = 1;
    //pid = fork(); 
     
    /*(
    if(pid) // Get the parent process out of here.
      return;
    cout << "Spawning server listener process\n";
    while(*serverRunning) { 
    *childFlag = true; */
    while(pid) { // Add conditional for the server being running or NOT running
      cout << "\tMASTER_PROCESS: Listening for clients...\n";
      newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t *)&clienlen);
      pid = fork();
      /*
      if(*childFlag) { // If we can't use PID values then let's use shared memory!
	cout << "\tTRUE FOR CHILD FLAG\n";
	*childFlag = false;
	break;
      }
      else {
	cout << "\tFALSE FOR CHILD FLAG\n";
	} */
    }
    cout << "\tFound a client\n";

    ++*numConnections;
    if(newsockfd < 0 || (*numConnections >= *maxConnections)) { // Socket is bad or the server is "full"
      cout << "Error " << "Error on accept\n";
      return;
    }
    else 
      {
	cout << "\tPROCESS: Connection spanned...\n";
	int idNumber = secureId();
	from_Client input;
	from_Server response; 	
	while(*serverRunning) 
	  {
	    // #3
	    ////////////////////RECEIVE & READ STATE//////////////	    
	    //!cout << "\t\tWaiting to read from client" << endl;
	    cout << "Sock connection: " << newsockfd << " and pointer: " << &newsockfd << endl;
	    rv = read(newsockfd, (void *)&input, sizeof(input));
	    if(rv < 0)
	      cout << "Error " << "Error reading from socket\n";	    

	    // #4 
	    // Run function pointer that was passed into our server. This involves the client and server's data
	    //!cout << "GOT DATA OF " << input << "- NOW TEXTING THEM BACK!!!!\n";
	    while(*isAccessingData){usleep(1);} // Data protection for write data problems in 'while' loop!!!!!!!!!!
	    *isAccessingData = true; 	   
	    if(!(response = function(shm_inServer, &input, idNumber))) 
	      break; // When the function returns a 'NULL' value we exit our loop
	    *isAccessingData = false;

	    // #5
	    ///////////////////RESPOND BACK////////////////////////
	    rv = write(newsockfd, &response, sizeof(from_Server));
	    if(rv < 0) {
	      cout << "Error!\n" << rv << " (Error writing to socket)\n";
	    }
	    //else
	      //!cout << "\t\tWrote back " << response << " to the client\n";
	  }
      }
    // Close this server connection here. 
    --*numConnections;
  }
};


#endif

