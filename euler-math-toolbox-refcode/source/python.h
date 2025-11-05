char *py_run (char *s);
char *py_run_file (char *s);
void py_set (char *name, header *hd);
void py_get (char *name, char **xnewram, char *ramend);
void py_eval (char *name, header *hd[], int n, char **xnewram, char *ramend);

void do_python ();
void mpyeval (header *hd);
void mpyset (header *hd);
void mloadpython (header *hd);
void python_call (char *name, int n, header *hd);

void get_python ();
void get_python_function (char *name, header *hd);

void do_pythonmode ();