/********
Contains various macros and functions for stack usage, plus many other often needed functions.
********/

#ifndef HEADERH
#define HEADERH

#include <stdio.h>

/********
ramstart: start of the Euler stack for variables and functions.
ramend: upper limit for the stack.
startlocal, endlocal: start and current end of local variables.
newram: start of new stack space (end of current stack).
udfend: end of space for user defined functions (udf). start is at ramstart.
endglobal: end of global variables.
********/

#ifdef WIN64
#define LONG _int64
#define ULONG unsigned _int64
#else
#define LONG long
#define ULONG unsigned long
#endif

extern char *ramstart,*ramend,*startlocal,*endlocal,
	*newram,*udfend,*endglobal;

// test if n bytes of ram are still free, starting from newram
#define freeram(n) (((ULONG)(newram)+(ULONG)(n))<=(ULONG)ramend)
// test if n bytes are free starting from position r
#define freeramfrom(r,n) (((ULONG)(r)+(ULONG)(n))<=(ULONG)ramend)

// The tabulator key is ASCII 9
#define TAB 9

// Our version of Pi in IEEE
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef ALIGNMENT
#define ALIGNMENT 8
#endif

// the delimiter between directory in paths
#ifdef unix
#define PATH_DELIM_CHAR '/'
#define PATH_DELIM_STR "/"
#else
#define PATH_DELIM_CHAR '\\'
#define PATH_DELIM_STR "\\"
#endif

// test for the function keys
extern char fktext[12][64];

extern int error; // global flag for errors. reset on each new command.
extern int udf; // mode for editing (udf = user defined function). udf=2 is a special edit mode.
extern int udfon; // running a udf?
extern int easyunits; // allows units without dollars.
extern int linelength; // global line length (default is about 70).
extern int stringon; // input moves inside a string.
extern int roundbrackets; // allow v(1) for vectors.
extern int relax; // relax all syntax restrictions.
extern int defineasrelax; // define all new functions as relaxes functions.

extern long loopindex; // current index in loop ...
extern long returnloopindex; // in case return uses #.

extern char *next; // next character to interpret (in current line, udf or file buffer)

extern char *udfline; // start of line in udf for error messages

extern int actargn,actsp; // global place to denote current argument numbers and semicolon position.
extern int searchglobal; // flag to search globally for variables or not.

extern int usematlab,matlab; // flags for Matlab mode

extern int hchar,wchar; // height and width of character for graphics output

extern int linew,fieldw; // for formatting numbers
extern double maxexpo,minexpo; // limits for exponential format
extern char expoformat[],fixedformat[],goodformat[]; // printf-formats
extern char useroutput[],scalarformat[]; // printf-formats
extern int scalardigits,usescalarformat; // special format for scalar numbers
extern double scalarminexpo,scalarmaxexpo; // exponential range for scalars

extern FILE *infile,*outfile; // open input and output file (only 1 each).

void output(char *s); // print output
void print(char *form, ...); // print formatted output
void println(char *form, ...); // print formatted output
void output1(char *form, ...); // the same
int output1hold(int s, char *form, ...); // print at position s and return current position

#define MAXLINE 32*1024
extern char input_line[]; // the buffer for the input line

#define MAXWORK 256

extern char *outputbuffer,*outputbufferend;
extern int outputbuffererror;

extern int trace; // global trace flag
extern int traceerrors; // global trace flag for errors
extern int outputing; // output on or off
extern int errorout; // error has already been printed

// the types of stack elements in Euler (typeof(expression))
typedef enum { 
	s_real, // 0 real
	s_complex, // 1 complex
	s_matrix, // 2 real matrix
	s_cmatrix, // 3 complex matrix
	s_reference, // 4 reference to stack element
	s_command, // 5 command
	s_submatrix, // 6 real submatrix
	s_csubmatrix, // 7 complex submatrix
	s_string, // 8 string
	s_udf, // 9 reference to udf
	s_interval, // 10 interval
	s_imatrix, // 11 interval matrix
	s_isubmatrix,// 12 submatrix of interval matrix
	s_smatrix, // 13 matrix of strings
	s_cpxmatrix, // 14 compressed matrix
	s_ssubmatrix, // 15 submatrix of strings
	s_list, // 16 list of stack elements
	s_binary // 17 binary data
}
	stacktyp;

extern char * s_names[]; // names of stack elements

// maximal length of an Euler name
#define maxname 32

#define FLAGTRACE 0x00000001
#define FLAGVAR 0x00000002
#define PROTECT 0x00000004
#define RELAX 0x00000008
#define MAP 0x00000010
#define SEMICOLON 0x00000020
#define SYMBOLIC 0x00000040
#define GLOBAL 0x00000080

#define MATLAB 0x00000100
#define INFIX 0x00000200
#define POSTFIX 0x00000400
#define PREFIX 0x00000800
#define STRONG 0x00001000
#define LATEX 0x00002000
#define COMMENT 0x00004000
#define ARGS 0x00008000

#define PYTHONFLAG 0x00010000
#define VARARGS 0x00020000
#define UTF 0x00040000
#define ADDITIONAL 0x00080000
#define HEAP 0x00100000
#define GLOBALREF 0x00200000
#define ALLOWASSIGNED 0x00400000

#define ZEROBASED 0x00800000

#define PREFIXLINE 0x01000000

// test for alignment
typedef struct
{	LONG size; char name[maxname]; int xor; stacktyp type; int flags;
	}
	headertest;

// header structure of one stack element
typedef struct
{	LONG size; char name[maxname]; int xor; stacktyp type; int flags;
// alignment is not used in Windows. at most, headers are aligned, when they are defined.
#ifdef HEADER_ALIGNMENT
	char dummy[((sizeof(headertest)-1)/HEADER_ALIGNMENT+1)*HEADER_ALIGNMENT-
		sizeof(headertest)];
#endif
	}
	header;

// test for alignment
typedef struct { int c,r;
	} dimstest;

// structure for number of columns and rows
typedef struct { int c,r;
// not used in Windows:
#ifdef DIMS_ALIGNMENT
	char dummy[((sizeof(dimstest)-1)/DIMS_ALIGNMENT+1)*DIMS_ALIGNMENT-
		sizeof(dimstest)];
#endif
	} dims;

// test for alignment
typedef struct { int c,r; double x;
	} xelementtest;

// structure for item in compressed matrix
typedef struct { int c,r; double x;
// not used in Windows:
#ifdef ELEMENTS_ALIGNMENT
	char dummy[((sizeof(xelementtest)-1)/DIMS_ALIGNMENT+1)*DIMS_ALIGNMENT-
		sizeof(xelementtest)];
#endif
	} xelement;

// define a long integer
typedef struct { int s; } inttyp;

// one real stack element
typedef struct { header hd; double val; } realtyp;

extern double epsilon; // internal epsilon
extern double changedepsilon; // to keep global value for local internal epsilon

// command types for Euler commands
typedef enum { c_quit, c_return, c_if, c_then, c_else, c_for, c_repeat,
	c_end, c_endif, c_continue, c_break, c_until, c_while, c_loop,
	c_elseif, c_noudf, c_udf, c_global, c_useglobal, c_none, c_allv }
	comtyp;

// structure to hold reference to one internal command
typedef struct { char *name; comtyp nr; void (*f)(void); } commandtyp;

extern commandtyp command_list[]; // list of commands
extern commandtyp *preview_command (int *l); // function to check for a command
extern commandtyp *find_command (char *name); // find a command by name

// structure to hold a reference to one builtin function
typedef struct { char *name; int nargs;  void (*f) (header *); }
	builtintyp;
extern builtintyp builtin_list[]; // list of builtin functions

#define realof(hd) ((double *)((hd)+1))
#define matrixof(hd) ((double *)((char *)((hd)+1)+sizeof(dims)))
#define elementsof(hd) ((xelement *)((char *)((hd)+1)+sizeof(dims)+sizeof(int)))
#define nelementsof(hd) ((int *)((char *)((hd)+1)+sizeof(dims)))
#define dimsof(hd) ((dims *)((hd)+1))
#define commandof(hd) ((int *)((hd)+1))
#define referenceof(hd) (*((header **)((hd)+1)))
#define imagof(hd) ((double *)((hd)+1)+1)
#define rowsof(hd) ((int *)((dims *)((header **)((hd)+1)+1)+1))
#define colsof(hd) ((int *)((dims *)((header **)((hd)+1)+1)+1)+submdimsof((hd))->r)
#define submrefof(hd) (*((header **)((hd)+1)))
#define submdimsof(hd) ((dims *)((header **)((hd)+1)+1))
#define indexof(hd) (int *)((header **)((header *)(hd+1))+1)
#define stringof(hd) ((char *)((hd)+1))
#define udfof(hd) ((char *)(hd)+(*((ULONG *)((hd)+1))))
#define udfstartof(hd) ((ULONG *)((hd)+1))
#define helpof(hd) ((char *)(hd)+sizeof(header)+sizeof(ULONG))
#define nextof(hd) ((header *)((char *)(hd)+(hd)->size))
#define listof(hd) ((header *)((hd)+1))
#define binsize(hd) ((int *)((char *)((hd)+1)))
#define bindata(hd) ((void *)((char *)((hd)+1)+sizeof(int)))

#define hdsetsize(hd,n) hd->size=(((n-1)/ALIGNMENT)+1)*ALIGNMENT
#define hdsetsizeto(hd,p) hd->size=(((((char *)p-(char *)hd)-1)/ALIGNMENT)+1)*ALIGNMENT

#define mat(m,c,i,j) (m+(((LONG)(c))*(i)+(j)))
#define cmat(m,c,i,j) (m+(2*(((LONG)(c))*(i)+(j))))
#define imat(m,c,i,j) (m+(2*(((LONG)(c))*(i)+(j))))

#define matrixsize(c,r) (sizeof(header)+sizeof(dims)+(c)*(LONG)(r)*sizeof(double))
#define cmatrixsize(c,r) (sizeof(header)+sizeof(dims)+2l*(c)*(LONG)(r)*sizeof(double))
#define imatrixsize(c,r) (sizeof(header)+sizeof(dims)+2l*(c)*(LONG)(r)*sizeof(double))

#define aof(hd) ((double *)((hd)+1))
#define bof(hd) ((double *)((hd)+1)+1)

#define isreal(hd) (((hd)->type==s_real || (hd)->type==s_matrix))
#define isinterval(hd) (((hd)->type==s_interval || (hd)->type==s_imatrix))
#define iscomplex(hd) (((hd)->type==s_complex || (hd)->type==s_cmatrix))
#define isrealorcomplex(hd) (((hd)->type==s_complex || (hd)->type==s_cmatrix || (hd)->type==s_real || (hd)->type==s_matrix))

header *next_param (header *hd);

void give_out (header *hd);

int command (void);

/* express.c */

int scan_elementary(void);
header *scan (int skiplineends=0);
header *scan_value(void);
void moveresult (header *stack, header *result);
void moveresult1 (header *stack, header *result);
void copy_complex (double *, double *);
void complex_divide (double *, double *, double *, double *, double *,
	double *);
void complex_multiply (double *, double *, double *, double *, double *,
	double *);

void interpret_udf (header *var, header *args, int nargs, int sp);

/* several */

void mnot (header *hd);
void mand (header *hd);
void mor (header *hd);
void mvconcat (header *hd);
void mhconcat (header *hd);

void sort_builtin (void);
void sort_commands (void);

void make_xors (void);

void get_udf (void);

extern FILE *metafile;

header *assign (header *var, header *value);

#define ERROR_WRONG_ARGUMENT 26
#define ERROR_MEMORY 120
#define ERROR_RANGE 50
#define ERROR_FLOATING_POINT 51
#define ERROR_MATRIX_SIZE 52
#define ERROR_ESCAPE 53
#define ERROR_INPUT 54
#define ERROR_SUPERFLUOUS 55
#define ERROR_USER 56
#define ERROR_INTERNAL 57
#define ERROR_NOT_IN_FUNCTION 58
#define ERROR_ARGUMENT 59

#define wrong_arg() { error=ERROR_WRONG_ARGUMENT; output("Wrong argument\n"); return; }
#define wrong_arg_in(x) { error=ERROR_WRONG_ARGUMENT; output1("Wrong arguments for %s\n",x); return; }
#define need_arg_in(x,s) { error=ERROR_WRONG_ARGUMENT; output1("Need %s for %s\n",s,x); return; }
#define overflow_in(s) { error=ERROR_WRONG_ARGUMENT; output1("Matrix too large in %s\n",s); return; }
#define test_error(x) { if (error) { output1("Error in %s\n",x); } }
#define varnotfound(x) { output1("Variable not found in %s\n",x); }
#define outofram() { output("Out of Memory!\n"); error=ERROR_MEMORY; return; }
#define checkram(p) { if ((char *)(p)>ramend) { output("Out of Memory!\n"); error=ERROR_MEMORY; return; } }
#define checkcommand(p) { scan_space(); if (*next && *next!=';') { print("Command \"%s\" needs a semicolon!\n",p); error=1; return; } if (*next) next++; }

extern char currentfilename[1024];

#define ERETVALX(string,parameter,number,value) { output1(string,parameter); error=number; return value; }
#define ERETVAL(string,number,value) { output(string); error=number; return value; }
#define ERET(string,number)  { output(string); error=number; return; }

#define xisalpha(c) ((c>='A' && c<='Z') || (c>='a' && c<='z') || (c<0 && c>-63) || c==hardspace)
#define xisdigit(c) (c>='0' && c<='9')

int memory_init (void);

extern char *stringelement (header *hd, int i);
extern int smatrixsize (header *hd);

extern void tofrac (double x, char *s);

extern void invalidate_udflist ();

extern char *currentfunction;

int issymbolic (header *hd);
char * gettype (header *hd);
void wrongarg (header *hd);

double getvre(header *hd, int i);
double getvim(header *hd, int i);

int isnone(header *hd);
header *new_none (int k=2);

#define strstarts(s,str) (!strncmp(s,str,strlen(str)))
int strends(char *s, char *str);

#define MAXARGN 20

extern char *argname[];
extern int xors[];

void print_error (char *p);

int scan_space (int skiplineneds=0);
int skip_space (void);
void scan_name (char *name, int printerror=1);
void next_line (void);
void next_simple_line ();
void trace_udfline(char *);
void getmatrix (header *hd, int *r, int *c, double **x);
void getmatrix (header *hd, int *r, int *c, double **x, char *name);
header *searchvar (char *name);
header *searchudf (char *name);
header *searchalludf (char *name);
header *searchudfmap (char *name);
void map1f (header *hd, char *name);

#endif

