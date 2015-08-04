#ifndef BORING_STRUCTS
#define BORING_STRUCTS

struct toServerStuff {
   int x;
   int y;
   char message[32];
};

struct toClientStuff {
   int x[8];
   int y[8];
   char message[8][32];
};

#endif
