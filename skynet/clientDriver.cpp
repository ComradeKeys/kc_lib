#include <iostream>
#include "kc_skynet.h"
#include "boringStructs.h"

using namespace std;

int main(int argc, char ** argv) {

   cout << "TESTING KC CLIENT\n";
   kc_Client<int, int> client;

   client.init(((string)argv[1]), (atoi(argv[2])));
   
   cout << "Step 1 complete!\n";
   client.start();   
      
   // Testing inputs from the main loop...
   int msg;
   while(1) {
     cout << "What will we say to the server?\n";
     cin >> msg;
     client.writeData(msg);
     usleep(200); // sleep for 200 milliseconds -> get ready for response
     msg = client.getServerUpdate();
     cout << "The server says back..." << msg << endl;     
   }

   client.turnOff();
   
   return 1;
}
