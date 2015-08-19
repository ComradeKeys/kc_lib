#include <iostream>
#include "kc_skynet.h"

using namespace std;

struct ServerMemory {
  int x[32];
  int size;
};

/****************************************************
 * serverFunction:
 *
 * This function is written according to the needs
 * of the programmer. Each connection b/t the server
 * and one of its clients is handled with this 
 * function. The 'ServerMemory' is unique and shared
 * on the server and can influence use for each 
 * client.
 * 
 * In this case the 'int *client' refers to the data
 * sent to the server where 'int' has been designated
 * as the data type from the client to the server.
 * the 'id' field is to identify which number
 * connection this is (1st, 2nd, 3rd connection...). 
 ***************************************************/
int serverFunction(ServerMemory *sm, int *client, int id) {
  cout << "Connection on " << id << " being made...\n";
  int x = sm->size;
  if(x < 7) {
    sm->size += 1;
    return sm->size;
  }
  else {
    sm->size = 1;
    return 1;
  }
}

int main(int argc, char ** argv) {
  // Internal memory, from_server, from_client
  kc_Server<ServerMemory, int, int> server;   

  // Per-client function, PORT_NUMBER, MAXIMUM NUMBER OF CONNECTIONS
  server.init(&serverFunction, 4500, 16);
  server.start();
  return 0;
}
