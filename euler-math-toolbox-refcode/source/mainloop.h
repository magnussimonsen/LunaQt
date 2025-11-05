#ifndef _MAINLOOP
#define _MAINLOOP

#include "header.h"

void main_loop (int argc, char *argv[]);

FILE *open_file_in_path (char *filename, char *mode="r", int currentfirst=0, int eulerfile=0);

#endif