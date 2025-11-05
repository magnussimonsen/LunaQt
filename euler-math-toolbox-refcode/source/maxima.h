#ifndef MAXIMAH
#define MAXIMAH

void do_maxima ();
void do_compatible_maxima ();
void do_maximamode ();

extern int maximamode,bracketlevel,needbracket;

int callmaxima (char *s, int replace);
char *callmxm (char *s, int autoanswer=5, int fixunderscore=0);
char *replaceexpressions (char *s, int mode);
char *replacematrix (char *s);
char *replaceunderscore (char *s);
void mxm (header *hd);
void mxmstartup ();
int callmaxima (char *s, int replace);
void loadmaximahelp (char *file);
void loadmaximastartup (char *file);
int getmaximahelp (char *text, char *extend, char *status, int number);
extern char *tomxm (header *hd, char *start, int fractional);
int printmaximahelp (char *text);
void do_mxmhelp ();
char *tex (char *formula);
void mtex (header *hd);

#define MAXMXOUTPUT MAXLINE

#endif