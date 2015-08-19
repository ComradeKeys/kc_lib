ALUTLIBS=-lalut -lopenal
VORBISLIBS=-lvorbis -lvorbisenc -lvorbisfile
CFLAGS  = -O3 -Wall -pedantic
PRJ     = kc_sound
CC      = g++
OBJDIR    = obj/
SRCDIR    = src/
OBJFILES  = $(OBJDIR)main.o $(OBJDIR)kc_sound.o

$(PRJ) : $(OBJFILES)
	$(CC) -o bin/$(PRJ) $(OBJFILES) $(ALUTLIBS) $(CFLAGS)

$(OBJDIR)main.o : $(SRCDIR)main.cpp $(SRCDIR)kc_sound.h
	$(CC) -o $(OBJDIR)main.o -c $(SRCDIR)main.cpp $(ALUTLIBS) $(CFLAGS)

$(OBJDIR)kc_sound.o : $(SRCDIR)kc_sound.cpp $(SRCDIR)kc_sound.h
	$(CC) -o $(OBJDIR)kc_sound.o -c $(SRCDIR)kc_sound.cpp $(ALUTLIBS) $(CFLAGS)

clean :
	rm $(OBJDIR)*.o bin/$(PRJ)
