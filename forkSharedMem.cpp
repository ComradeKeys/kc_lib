#include <stdio.h>     //Fork call, some Shared memory calls...
#include <unistd.h>
#include <sys/types.h> //Shared memory stuff (this/next two)
#include <sys/ipc.h>
#include <sys/shm.h>
#include <iostream>

using namespace std;

#define SHMSZ 27

int main(int argc, char **argv)
{
  printf("Building shared memory data\n");

  ////////////SHARED MEMORY SEGMENT///////////
  //Shared memory definitions
  int shmid;
  key_t key;
  char *shm, *s;

  //Shared memory naming. Our key is "5678"
  key = 5678;

  //Build the segment
  if((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0) {
    perror("shmget");
    return 1;
  }

  //Attach the segment to our data space
  if((int)(shm = (char *)shmat(shmid, NULL, 0)) == -1) {
    perror("shmat");
    return 1;
  }

  //Make a little poitner copy...
  s = shm;
  ////////////END OF SHARED MEMORY SEG///////
  

  
  //Fork process portion
  pid_t pid = fork();
  
  if (pid == 0)
    {
      cout << "As the CHILD PROCESS we are now going "
	   << "to output what we read. We have:\n";

      while(*s != NULL)
	cout << *s++;
      cout << endl;

      cout << "CHILD process done\n\n";
    }
  else if (pid > 30)
    {
      cout << "As the PARENT process we are assigning "
	   << "'abcde.... ' to our code\n";
      
      //Put something in our memory segment
      char c;
      for(c = 'a'; c <= 'z'; c++)
	*s++ = c;

      *s = NULL;

      cout << "PARENT Done assigning 'abcde....' to our code\n\n";
    }

  return 0;
}
