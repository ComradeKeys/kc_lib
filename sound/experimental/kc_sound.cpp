#include "kc_sound.h"

/*F******************************************************************
 * 
 * 
 * PURPOSE : 
 *
 * RETURN :  
 *
 * NOTES :   
 *F*/
kc_sound::kc_sound(int argc, char **argv) {

  alutInit(&argc, argv);

  // Create sound buffer and source
  alGenBuffers(1, &bufferID);
  alGenSources(1, &sourceID);

  // Set the source and listener to the same location
  alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
  alSource3f(sourceID, AL_POSITION, 0.0f, 0.0f, 0.0f);

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
kc_sound::~kc_sound() {
  // Clean up sound buffer and source
  alDeleteBuffers(1, &bufferID);
  alDeleteSources(1, &sourceID);

  // Clean up the OpenAL library
  alutExit();
}

/*F******************************************************************
 * load_ogg(const char *filename, vector<char> &buffer, ALenum &format, ALsizei &freq)
 * 
 * PURPOSE : Loads a .ogg file with it's format and frequency.
 *
 * RETURN :  void
 *
 * NOTES :   
 *F*/
void kc_sound::load_ogg(const char *filename) {

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
void kc_sound::play_ogg(const char *filename) {

  load_ogg(filename);
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


}

