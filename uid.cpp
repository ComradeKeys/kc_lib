#include <iostream>

using namespace std;

struct connection{
  int num;
  connection * next;
};

int totalConnections = 0;
connection * head = NULL;
connection * cursor = NULL;

// Make a connection available on our linked list
void removeConnection(int connectionNum) {
  cursor = head;
  if(connectionNum > totalConnections)
    return;
  for(int i = 0; i < connectionNum; i++) {
    cursor = cursor->next;
  }
  cursor->num = -1;
}

int getConnectionNum() {
  int con = 0;
  cursor = head;
  if(!cursor) { // If this is the first connection
    head = new connection;
    cursor = head;
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
    ++totalConnections;
    cursor->next = new connection;
    cursor = cursor->next;
    cursor->next = NULL;
    return cursor->num = totalConnections;
  }
}

int main(int argc, char **argv) {  
  // First node
  char reply = 'y';
  while(1) {
    cout << "Add connection or delete a connection? (a/n/d) ";
    cin >> reply;    
    if(reply == 'n') break;
    if(reply == 'a') {
      int newCon =  getConnectionNum();
      cout << "New connection secured on node " << newCon << endl;
    }
    if(reply == 'd') {
      int conNumber;
      cout << "\tWhich connection would you like to remove? ";
      cin >> conNumber;
      removeConnection(conNumber);
    }
  }
  cout << "Your list has: " << totalConnections << " nodes.\n";
  return 1;
}
