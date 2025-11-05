#ifndef GETVALUE
#define GETVALUE

header *getvalue (header *);
header *followrefs (header *);
header *getvariable (header *);
header *getvariablesub (header *);
void evaluate (header *var, header *hd, int nargs, int nsp);
char *stringelement (header *hd, int i);
void get_element (int nargs, header *var, header *hd, int bracket, int nsp, int zerobased);
void get_element1 (char *name, header *hd);
void get_element2 (char *name, header *hd, header *hd1);
#ifdef DLL
int exec_dll (char *name, int n, header *hd);
#endif
void direct_reference (header *hd);

#endif