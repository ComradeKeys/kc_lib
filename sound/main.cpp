#include <AL/al.h>
#include <AL/alut.h>
#include <vorbis/vorbisfile.h>
#include <cstdio>
#include <iostream>
#include <vector>
#include <stdlib.h>

#define BUFFER_SIZE     32768       // 32 KB buffers

using namespace std;


/*F******************************************************************
 * 
 * 
 * PURPOSE : 
 *
 * RETURN :  
 *
 * NOTES :   
 *F*/



/*F******************************************************************
 * load_ogg(const char *filename, vector<char> &buffer, ALenum &format, ALsizei &freq)
 * 
 * PURPOSE : Loads a .ogg file with it's format and frequency.
 *
 * RETURN :  void
 *
 * NOTES :   
 *F*/
void load_ogg(const char *filename, vector<char> &buffer, ALenum &format, ALsizei &freq) {

  int endian = 0;
  int bitStream;
  long bytes;
  char array[BUFFER_SIZE];
  FILE *f;

  // Open for binary reading
  f = fopen(filename, "rb");

  if(f == NULL) {
    fprintf(stderr, "Cannot open %s for reading", filename);
    exit(-1);
  }

  vorbis_info *pInfo;
  OggVorbis_File oggFile;

  // Try opening the given file
  if(ov_open(f, &oggFile, NULL, 0) != 0) {

    fprintf(stderr, "Error opening %s for decoding", filename);
    exit(-1);
  }

  // Get some information about the OGG file
  pInfo = ov_info(&oggFile, -1);

  // Check the number of channels... always use 16-bit samples
  if(pInfo->channels == 1) {
    format  = AL_FORMAT_MONO16;
  } else {
    format  = AL_FORMAT_STEREO16;
  }

  // The frequency of the sampling rate
  freq = pInfo->rate;

  // Keep reading until all is read
  do {

    // Read up to a buffer's worth of decoded sound data
    bytes = ov_read(&oggFile, array, BUFFER_SIZE, endian, 2, 1, &bitStream);

    if(bytes < 0) {
      ov_clear(&oggFile);
      fprintf(stderr, "Error decoding %s", filename);
      exit(-1);
    }

    // Append to end of buffer
    buffer.insert(buffer.end(), array, array + bytes);
  } while(bytes > 0);

  // Clean up!
  ov_clear(&oggFile);
}

/*F******************************************************************
 * 
 * 
 * PURPOSE : 
 *
 * RETURN :  
 *
 * NOTES :   
 *F*/
int kc_init_audio(int argc, char *argv[]) {

  alutInit(&argc, argv);
  

  
}

/*F******************************************************************
 * 
 * 
 * PURPOSE : 
 *
 * RETURN :  
 *
 * NOTES :   
 *F*/
void kc_play_ogg() {

}

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

  ALint        state;       // The state of the sound source
  ALuint       bufferID;    // The OpenAL sound buffer ID
  ALuint       sourceID;    // The OpenAL sound source
  ALenum       format;      // The sound data format
  ALsizei      freq;        // The frequency of the sound data
  vector<char> bufferData;  // The sound buffer data from file

  kc_init_audio(argc, argv);

  // Make sure there is a file name
  if(argc < 2) {
    fprintf(stderr, "Syntax: %s OGGFILE");
    return -1;
  }


  // Create sound buffer and source
  alGenBuffers(1, &bufferID);
  alGenSources(1, &sourceID);

  // Set the source and listener to the same location
  alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
  alSource3f(sourceID, AL_POSITION, 0.0f, 0.0f, 0.0f);

  // Load the OGG file into memory
  load_ogg(argv[1], bufferData, format, freq);

  // Upload sound data to buffer
  alBufferData(bufferID, format, &bufferData[0], static_cast<ALsizei>(bufferData.size()), freq);

  // Attach sound buffer to source
  alSourcei(sourceID, AL_BUFFER, bufferID);

  // Finally, play the sound!!!
  alSourcePlay(sourceID);

  // This is a busy wait loop but should be good enough for example purpose
  do {
    // Query the state of the souce
    alGetSourcei(sourceID, AL_SOURCE_STATE, &state);
  } while (state != AL_STOPPED);

  // Clean up sound buffer and source
  alDeleteBuffers(1, &bufferID);
  alDeleteSources(1, &sourceID);

  // Clean up the OpenAL library
  alutExit();

  return 0;
}
