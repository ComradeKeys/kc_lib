#pragma once

#include <AL/al.h>
#include <AL/alut.h>
#include <vorbis/vorbisfile.h>
#include <cstdio>
#include <iostream>
#include <vector>
#include <stdlib.h>

#define BUFFER_SIZE     32768       // 32 KB buffers

class kc_sound {


 public:
  kc_sound(int argc, char **argv);
  ~kc_sound();
  
  void play_ogg(const char *filename);

 private:
  ALint        state;       // The state of the sound source
  ALuint       bufferID;    // The OpenAL sound buffer ID
  ALuint       sourceID;    // The OpenAL sound source
  ALenum       format;      // The sound data format
  ALsizei      freq;        // The frequency of the sound data
  std::vector<char> bufferData;  // The sound buffer data from file
  void load_ogg(const char *filename);

};
