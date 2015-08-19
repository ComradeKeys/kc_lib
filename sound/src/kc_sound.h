/*H**********************************************************************    * FILENAME :        kc_sound.h
 *
 * DESCRIPTION :
 *       Handles all the audio for the library, at the moment only plays
 *       .wav files but I plan to support ogg files as well
 *       
 * PUBLIC FUNCTIONS : 
 *       void play_wav(const char *filename)
 *
 * AUTHOR     :    Brigham Keys, Esq
 * START DATE :    08/18/2015 
 *
 *H*/

#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
#include <stdio.h>

class kc_sound {

 public:
  kc_sound();
  ~kc_sound();
  void play_wav(const char *filename);
  
 private:
  ALuint buffer;
  ALuint source;
  int state;

};
