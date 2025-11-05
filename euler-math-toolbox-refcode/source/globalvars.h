
void init_globalvars();
void exit_globalvars();

void mglobalput (header *);
void mglobalget (header *);
void mglobalrem (header *);
void mglobalclear (header *);

void mglist (header *);
void mglistadd (header *);
void mglistlength (header *hd);
void mglistget (header *hd);
void mglistput (header *hd);
void mglistinsert (header *hd);
void mglistdelete (header *hd);
void mglistrem (header *hd);

header *globalput (char *name, header *hd, int flags=0);
header *globalget (char *name);
header *globalfind (char *name);
void globalremove (char *name);
int listglobal (char *name);
void protectglobal ();

void clear_global ();
int list_lists (char *name);