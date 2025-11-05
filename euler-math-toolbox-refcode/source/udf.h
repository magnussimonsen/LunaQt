#ifndef UDF

void function_error (char *name, int argn);

char * print_defaults (int defaults);
void printlocalvars (char *startlocal);

extern int printdefaults;

#endif