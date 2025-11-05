#ifndef SYSDEPH
#define SYSDEPH

#include <stdio.h>

/*
This header contains all things, a host system for the Euler 
core must provide. For a more detailed description, read the
"readme.txt" file in the Euler source folder.
*/

// Line Editor

void edit (char *s);

// constants for keyboard input

typedef enum { key_none, cursor_up, cursor_down, cursor_left, cursor_right,
	escape, deletekey, backspace, clear_home, switch_screen, enter,
	space, line_end, line_start, fk1, fk2, fk3, fk4, fk5, fk6, fk7,
	fk8, fk9, fk10, word_left, word_right, help, escapecode, page_up, page_down }
	scantyp;

extern char hardspace;

// Synchronous keyboard input

int sys_wait (double delay, int *scan);
int sys_wait (double delay, int *scan, char *text);

int wait_key (int *scan);
int wait_key (int *scan, char *statustext);

// Asynchronous keyboard input

int test_key (void);
int test_code (void);

// mouse nad keyboard intput

int mouse (double *, double *);
int mouse (double *, double *, char *statustext);
int mousedrag (double *x, double *y, long *time, int *keycode, char *status);
int mousestate (double *x, double *y);

// Declarations for graphics

typedef enum { line_none, line_solid, line_dotted, line_dashed, 
	line_dashdot, line_dashdotdash, line_dotdashdot, line_arrow }
	linetyp;
typedef enum { marker_cross, marker_circle, marker_diamond, marker_dot,
	marker_plus, marker_square, marker_star, marker_largedot,
	marker_filledcircle, marker_filleddiamond, marker_filledlargedot,
	marker_filledsquare,marker_vertical,marker_minus,marker_diamond_hollow,
	marker_square_hollow,marker_circle_hollow }
	markertyp;
typedef enum { fill_blank, fill_filled } filltyp;
typedef enum { bar_solid, bar_framed, bar_frame, bar_vhatch,
	bar_hhatch, bar_diagonal1, bar_diagonal2, bar_cross, bar_plus,
	bar_transparent } bartyp;

// graphics functions

#define MAXPOLYGON 4096

void gclear (void);
void gclip(double c, double r, double c1, double r1);
void gline (double c, double r, double c1, double r1, int color,
	int st, double width);
void gpath (double c[], int n, int color, int st, double width);
void gtext (double c, double r, char *text, int color, int centered, double size);
void gvtext (double c, double r, char *text, int color, int centered, double size);
void gvutext (double c, double r, char *text, int color, int centered, double size);
void gmarker (double c, double r, int color, int st, double markerfactor);
void gfill (double c[], int fillcolor, int wirecolor, int n, int connect[]);
void gpolygon (double c[], int n, int st, int color, int connect);
void gfillh (double c[], int n, double hue, int color, int connect);
void gbar (double c, double r, double c1, double r1, double hue,
	int color, int connect);
void gbar1 (double c, double r, double c1, double r1,
	int color, int connect);
void gbarRGB (double c1, double r1, double c2, double r2, int c, int r,
	int tred, int tgreen, int tblue,
	double *red, double *green, double *blue);

// postscript output

void dump_postscript (FILE *out);
void pswindow (double w, double h);

// pixel size

void getpixelsize (double *x, double *y);

// display mode

void text_mode (void);
void graphic_mode (void);

// directories

char *cd (char *dir);
int makedir (char *dir);
char *dir (char *pattern);
void sethomedir ();
void setstartdir ();
char *gethomedir ();
char *getuserhomedir ();
char *getusereulerdir ();
char *getuserdir ();
char *getstartdir ();
char *getmaximadir ();
char *getexamples ();

// execute external program

int execute (char *prog, char *param, char *dir, 
	int outp, int hidden, int wait, 
	char *buffer, int size);

int execread (char *buffer, int size);
int execwrite (char *s, int nl);

void execkill ();

double myclock (void);
void getnow (int *year, int *month, int *day, 
	int *hour, int *minute, int *second, int *milliseconds,
	int utc);
void printnow (char *s, int size, int utc);

void gprint (char *s); /* print an output text (no newline) */
void gflush (void); /* flush out graphics */

// text output

extern int linelength;

void clear_screen (void);

// edit mode

void edit_on (void);
void edit_off (void);

// play sound

void sys_playwav (char *file);

// frames (pages)

int nframes ();
void showframe (int n);
void addframe ();
void addframe (int w, int h);
void deleteframes ();
void copytoframe (int n);
void copyfromframe (int n);

// colors

double getcolor (int i, int j);
int setcolor(int i, double red, double green, double blue);
void resetcolors();

// notebooks

void insert_image (int i, char *name, int flag, 
	double crop1, double crop2, double crop3, double crop4);
void insert_latex (char *formula);
int load_image (int i, char *name, double scale=1.0, int reduce=0, char *latex=0, int mathjax=0);
int load_ag_images (int i, char *name1, char *name2);
void clear_notebook ();
void setstatus (char *text);
extern int nojump;

// Maxima

extern int killo;
extern int display2d;

void end_maxima ();
void start_maxima (char *s);
int runmaxima (char *s, int output, int replace);
int maxima_question ();
void maxima_break ();
int maxima_running ();
char *maxima_output ();

// extern double NAN;
extern int pnan,usenan;
extern int underflows;
extern int isnan (double x);

void dostartup ();

extern int spaces,brackets,assignments,commas;

int get_pixels (char *filename, double *m, int *r, int *c);
int put_pixels (char *filename, double *m, int r, int c);
void insert_pixels (int n, char *name, double *m, int r, int c);

void notify_error (char *next);

void getcolor (int i, int *red, int *green, int *blue);
void gethuecolor (int i, int *red, int *green, int *blue, double hue);
void getfillcolor (int i, int *red, int *green, int *blue);

void savepng (char *filename, int w, int h, int aa);
void saveps (char *filename);
void savesvg (char *filename, int w, int h);

void addmenu (char *item);
void addsubmenu (char *item);
void addloadedfile (char *item);

void tcc (char *filename);
void get_tc (char *name, int argn);

void dump (char *s, ...);
void Warning (char *text, char *title);
void show_script_error ();

int setfont (int n, char *s);

extern double getaspect ();
extern double setaspect (double aspect);

int file_copy (char *source, char *destination);
int file_exists (char *name);

int doreplay();

void clearudfinput();

void closedll (char *name);

char *chartoutf (int charcode, char *p);
unsigned char *exent (unsigned char *p, unsigned char *q, unsigned char *ramend=0);
char * toutf (char *s);
char * fromutf (char *s);

int startnet ();
void endnet ();
int opennetfile (char *url);
void closenetfile ();
char *netgetline ();
int neteof ();

char *texpng (char *formula, int alias=1, double factor=1.0, int transparent=1, int textcolor=-1);

int getstacksize ();

void flog (char *s, ...);

#endif