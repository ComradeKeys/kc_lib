#include <iostream>
#include "kc_skynet.h"

using namespace std;

int main(int argc, char ** argv) {

   kc_Client<int, int> client;
   client.init(((string)argv[1]), (atoi(argv[2])));
   client.start();   
      
   int msg;
   while(1) {
     // Send a message to the server 
     cout << "What will we say to the server?\n";
     cin >> msg;
     if(msg == 4500) break;
     client.writeData(msg);

     // Output what we get back
     msg = client.getServerUpdate(AWAIT_SERVER);
     cout << "The server says back..." << msg << endl;     
   }
   // Close the connection
   cout << "Attempting to close client connection...\n";
   client.turnOff();
   
   return 1;
}
