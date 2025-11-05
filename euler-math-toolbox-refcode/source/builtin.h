#ifndef BUILTINH
#define BUILTINH

void sort_builtin (void);
int exec_builtin (char *name, int nargs, header *hd);
builtintyp *find_builtin (char *name, int nargs=-1);

#endif
