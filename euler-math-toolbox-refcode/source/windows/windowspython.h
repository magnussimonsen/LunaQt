#ifndef python_h
#define python_h

void py_init ();
void py_exit ();
char *py_run (char *s);
char *py_run_file (char *s);
char *py_result ();

void py_get_double (char *name, double *x);

#endif