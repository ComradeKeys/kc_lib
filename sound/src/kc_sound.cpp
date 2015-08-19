#include "kc_sound.h"

/*F******************************************************************
 * DEFAULT CONSTRUCTOR
 * 
 * PURPOSE : initializes alut for us and does error checking
 *
 * RETURN :  
 *
 * NOTES :   
 *F*/
kc_sound::kc_sound() {

  // Initialize the environment
  alutInit(0, NULL);

  // Capture errors
  alGetError();

}
/*F******************************************************************
 * DESTRUCTOR
 * 
 * PURPOSE : Exits out of the libraries we are using and alut
 *
 * RETURN :  
 *
 * NOTES :   
 *F*/
kc_sound::~kc_sound() {

  // Clean up sources and buffers
  alDeleteSources(1, &source);
  alDeleteBuffers(1, &buffer);

  // Exit everything
  alutExit();
}
/*F******************************************************************
 * kc_sound::play_wav(const char *filename)
 * 
 * PURPOSE : plays the .wav file passed given that it exists and is
 *           valid.
 *
 * RETURN :  void
 *
 * NOTES :   
 *F*/
void kc_sound::play_wav(const char *filename) {

  // Load pcm data into buffer
  buffer = alutCreateBufferFromFile(filename);

  // Create sound source (use buffer to fill source)
  alGenSources(1, &source);
  alSourcei(source, AL_BUFFER, buffer);
  // Play
  alSourcePlay(source);

  // Wait for the song to complete
  do {
    alGetSourcei(source, AL_SOURCE_STATE, &state);
  } while (state == AL_PLAYING);

}
