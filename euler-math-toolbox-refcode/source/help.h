#ifndef HELPH
#define HELPH

typedef struct _helpline
{	struct _helpline *next;
	char *text;
} helpline;

typedef struct _helpitem
{	struct _helpitem *next;
	helpline *help;
	char *item;
	char **items;
} helpitem;

typedef struct _maximahelpline
{	struct _maximahelpline *next;
	char *text;
	char *item;
} maximahelpline;

int externhelp (char *text);
void loadhelp (char *name, int more=0);
void unloadhelp ();
void loadexamples (char *name);

void gethelp (char *start, char *extend);

int help_file (char *name, char *out=0);

char * newline (FILE *in);

void do_type_name (char *name);
int do_list_name (char *name);
void do_listvar_name (char *name);

void putstatustopic (char *s, int maxima=0);
char *getstatustopic ();

void putlasttopic (char *s, int maxima=0);
char *getlasttopics ();

void gethelp (char *start, char *extend, char *help); 

#define MAXHELP 32000
void getwindowhelp (char *start, char *help);

#endif