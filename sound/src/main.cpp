#include "kc_sound.h"

/*F******************************************************************
 * main(int argc, char **argv)
 * 
 * PURPOSE : entry into the program
 *
 * RETURN :  int
 *
 * NOTES :   Best viewed with emacs
 *F*/
int main(int argc, char *argv[]) {

  kc_sound sound;

  sound.play_wav("creation.wav");
  return 0;
}
