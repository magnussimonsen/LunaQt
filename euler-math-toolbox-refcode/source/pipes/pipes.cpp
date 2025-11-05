/*
Very simple implementation of an Euler interface.
*/

#include <string.h>
#include <process.h>
#include <direct.h>

#include "../sysdep.h"
#include "../header.h"
#include "../mainloop.h"
#include "../string/string.h"

char bom[] = { 239 - 256,187 - 256,191 - 256,0 };
char* BOM = bom;

int commas=1,reportindex=1,brackets=1,alwaysrelax=0,
	conditionvectors=0,userdirectmode=1,spaces=1,
	assignments=0,enablemaxima=0,startinmaxima=0,
	usedirectmode=0,startinmaximamode=0,allowoverwrite=1,
	allowvaroverwrite=0,matlab=0,underflows=0,
	nomultiline=0,answermaxima=1,usecolors=1,fatterlines=1,
	boldfont=0,scalelines=1,fontlines=1,gscreenlines=40,
	allowsymbolicdp=0,script=0,insertsmaller=0,inevaluation=0,
	second=0;

char hardspace=(char)160;

extern int udf;

void edit (char *s)
{	
	if (!udf) printf(">");
	gets_s(s,256);
	if (feof(stdin)) exit(0);
}

int sys_wait (double delay, int *scan) { return 0; }
int sys_wait (double delay, int *scan, char *text) { return 0; }


int wait_key (int *scan) { return key_none; }
int wait_key (int *scan, char *statustext) { return key_none; }

// Asynchronous keyboard input

int test_key (void) { return key_none; }
int test_code (void) { return key_none; }

// mouse nad keyboard intput

int mouse (double *, double *) { return key_none; }
int mouse (double *, double *, char *statustext) { return key_none; }

// graphics functions

void gclear (void) {}
void gclip(double c, double r, double c1, double r1) {}
void gline (double c, double r, double c1, double r1, int color,
	int st, double width) {}
void gtext (double c, double r, char *text, int color, int centered, double size) {}
void gvtext (double c, double r, char *text, int color, int centered, double size) {}
void gvutext (double c, double r, char *text, int color, int centered, double size) {}
void gmarker (double c, double r, int color, int st, double size) {}
void gfill (double c[], int st, int n, int connect[]) {}
void gfillh (double c[], int n, double hue, int color, int connect) {}
void gbar (double c, double r, double c1, double r1, double hue,
		   int color, int connect) {}
void gbar1 (double c, double r, double c1, double r1,
			int color, int connect) {}
void gbarRGB (double c1, double r1, double c2, double r2, int c, int r,
	int tred, int tgreen, int tblue,
	double *red, double *green, double *blue) {}
void gscale (double s) {}
void gfill (double c[], int fillcolor, int wirecolor, int n, int connect[]) {}
void gpolygon (double c[], int n, int st, int color, int connect) {}
void gpath (double c[], int n, int color, int style, double width) {}


// postscript output

void dump_postscript (FILE *out) {}
void pswindow (double w, double h) {}

// pixel size

void getpixelsize (double *x, double *y) {}

// display mode

void text_mode (void) {}
void graphic_mode (void) {}

// directories

char *cd (char *dir)
{	static char path[256];
	_chdir(dir);
	if (_getcwd(path,256)) return path;
	return dir;
}

char homedir[1024];
char userhomedir[1024];
char startdir[1024];

char *dir (char *pattern) { return ""; }
void sethomedir () {}
void setstartdir ()
{	
	strcpy(startdir,homedir);
	strcat(startdir,"\\");
}
char *gethomedir ()
{	return homedir;
}
char *getstartdir ()
{	return startdir;
}
char *getuserhomedir ()
{	return userhomedir;
}

int makedir (char *dir) { return 0; }

char *getexamples () { return ""; }

double myclock (void) { return 0.0; }

int printstarted=0;

void gprint (char *s)
{	if (!printstarted && strlen(s)<3)
	{	printstarted=1;
		return;
	}
	printstarted=1;	
	printf(s);
}

void gflush (void) {} /* flush out graphics */

// text output

int linelength=70;

void clear_screen (void) {}

// edit mode

void edit_on (void) {}
void edit_off (void) {}

// play sound

void sys_playwav (char *file) {}

// frames (pages)

int nframes () { return 0; }
void showframe (int n) {}
void addframe () {}
void addframe (int w, int h) {}
void deleteframes () {}
void copytoframe (int n) {}
void copyfromframe (int n) {}

// colors

double getcolor (int i, int j) { return 0.0; }
void gethuecolor (int i, int *red, int *green, int *blue, double hue)
{
	*red=0; *green=0; *blue=0;
}
void getfillcolor (int i, int *red, int *green, int *blue)
{
	*red=0; *green=0; *blue=0;
}

int setcolor(int i, double red, double green, double blue) { return 0; }
void resetcolors() {}

// notebooks

void insert_image (int i, char *name, int flag, 
				   double crop1, double crop2, double crop3, double crop4) 
{
	error=1;
}
int load_image (int i, char *name, double scale, int reduce, char *latex, int mathjax)
{
	error=1; return 0;
}
int load_ag_images (int i, char *name1, char *name2)
{
	error=1; return 0;
}
void clear_notebook () {}
void setstatus (char *text) {}

int nojump;

// demos

void demo (char *file) {}

// Maxima disabled

int killo=1;
int maximarunning=0;

void start_maxima (char *s) {}
void end_maxima () {}
int runmaxima (char *s, int output)
{	output1("Maxima cannot be used in this version!\n");
	return 0; 
}
int maxima_question () { return 0; }
void maxima_break () {}
int maxima_running () { return maximarunning; }
char *maxima_output () { return ""; }

int stacksize=512;

// ********* Not a number NAN *******************

// unsigned long nan[2]={0xffffffff, 0x7fffffff};
// double NAN=*(double*)nan; // system dependend NAN
int pnan=0; // flag: produce NAN (or produce errors)
int usenan = 1;

int _matherr (struct _exception *except)
{   if (!pnan) error=1;
	return 0;
}

int isnan (double x)
// system dependend function to detect a NAN in IEEE
{	return !(x==x);
}

void insert_image (int i, char *name, int flag) {}
void insert_pixels (int n, char *name, double *m, int r, int c) {}

void savepng (char *filename, int w, int h, int aa) {}
void saveps (char *filename) {}
void savesvg (char *filename, int w, int h) {}

void addmenu (char *item) {}
void addsubmenu (char *item) {}

void notify_error (char *next) {}

void dostartup () {}

int get_pixels (char *filename, double *m, int *r, int *c) { return 0; }
int put_pixels (char *filename, double *m, int r, int c) { return 0; }

int red[16]=  {255, 0,160, 50, 50, 50,150,180,120, 80,240,100,100,100,240,240};
int green[16]={255, 0, 50,180, 50,150,150,180,120, 80,130,240,150,240,180,240};
int blue[16]= {255, 0, 50, 50,180,150, 50,180,120, 80,100,100,240,240,100,100};

void getcolor (int i, int *red, int *green, int *blue) {}

char *getusereulerdir () { return ""; }
char *getuserdir () { return ""; }
char *getmaximadir () { return ""; }


void getnow (int *year, int *month, int *day, 
	int *hour, int *minute, int *second, int *milliseconds,
	int utc)
{
	*year=2000; *month=0; *day=0; *hour=0; *minute=0; *second=0; *milliseconds=0;
}
void printnow (char *s, int size,
	int utc)
{
	strcpy(s,"Not implemented");
}

int mousedrag (double *x, double *y, long *time, int *keycode, char *statustext)
{
	error=1; return 0;
}
int mousestate (double *x, double *y)
{
	error=1; return 0;
}

int setfont (int n, char *s)
{
	return 0;
}

extern double getaspect ()
{
	return 1.0;
}
extern double setaspect (double aspect)
{
	return 1.0;
}

int file_copy (char *source, char *destination)
{
	error=1; return 0;
}
int file_exists (char *name)
{
	error=1; return 0;
}

int doreplay() { return 0; }



int execute (char *prog, char *param, char *dir, 
	int outp, int hidden, int wait, 
	char *buffer, int size)
{
	_spawnl(_P_WAIT,prog,param);
	return 0;
}

int execread (char *buffer, int size)
{
	return 0;
}

int execwrite (char *s, int nl)
{
	return 0;
}

void execkill () {}

void insert_latex (char *s) {}

int runmaxima (char *s, int output, int replace)
{
	return 0;
}

void tcc (char *filename) 
{
	print("TinyC not implemented in this version.\n");
	error=1; return;
}
void get_tc (char *name, int type) 
{
	print("TinyC not implemented in this version.\n");
	error=1; return;
}
void show_script_error () {}

void Warning (char *text, char *title) 
{ 
	fprintf(stderr,"Warning: %s (%s)\n",text,title); 
}

char *py_run (char *s) 
{ 
	print("Python not implemented in this version.\n");
	error=1; return 0;
}
char *py_run_file (char *s)
{ 
	print("Python not implemented in this version.\n");
	error=1; return 0;
}
void py_set (char *name, header *hd) {}
void py_get (char *name, char **xnewram, char *ramend) {}
void py_eval (char *name, header *hd[], int n, char **xnewram, char *ramend) {}

void clearudfinput () {}

char *chartoutf (int charcode, char *p) { return p; }
unsigned char *exent (unsigned char *p, unsigned char *q, unsigned char *ramend) { return p; }
char * toutf (char *s) { return s; }
char * fromutf (char *s) { return s; }

int startnet () { return 0; }
void endnet () {}
int opennetfile (char *url) { return 0; }
void closenetfile () {}
char *netgetline () { return ""; }
int neteof () { return 1; }

char *texpng (char *formula, int alias, double factor, int transparent, int textcolor) { return formula; }

int getstacksize () { return ramend-newram; }

void addloadedfile(char* s) {}
void flog (char *,...) {}

int main (int argc, char* argv[])
{	
	strcpy(homedir,_getcwd(NULL,1024));

	memory_init();

	main_loop (argc,argv);
	return 0;
}
