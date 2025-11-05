// The data types of Euler
typedef enum { s_real,s_complex,s_matrix,s_cmatrix,
	s_reference,s_command,s_submatrix,s_csubmatrix,s_string,s_udf,
	s_interval,s_imatrix,s_isubmatrix,s_smatrix,
	s_cpxmatrix,s_ssubmatrix,s_list,s_binary }
	stacktyp;

#ifdef WIN64
#define LL long long
#else
#define LL long
#endif

// header for elements on the stack
typedef struct
{	LL size; char name[32]; int xor; int type; int flags;
}
header;

// the flags in use in Euler
#define FLAGTRACE 1
#define FLAGVAR 2
#define PROTECT 4
#define RELAX 8
#define MAP 16
#define SEMICOLON 32
#define SYMBOLIC 64
#define GLOBAL 256
#define MATLAB 512
#define INFIX 1024
#define POSTFIX 2048
#define PREFIX 4096
#define STRONG 8192
#define LATEX 16384
#define COMMENT 32768

// dimensions for submatrixes
typedef struct
{	int c,r;
}
dims;

// element of a compressed matrix
typedef struct 
{	int c,r; double x;
} 
cpxelement;

extern char *ram,*newram,*ramend;

void start (char *newram, char *ramend);

extern int error;
extern char *errorstring;

// access the data elements of the header

// for strings and string vectors
// string vectors consist of a sequence 
// of 0 terminated strings and then 1
#define stringof(hd) ((char *)((hd)+1))

// for reals, and complex or interval numbers (two reals)
#define realof(hd) ((double *)((hd)+1))

// for matrices (start of double values row by row,
// or pairs for complex or interval values)
#define matrixof(hd) ((double *)((char *)((hd)+1)+sizeof(dims)))

// for matrices (returns a dims structure)
#define dimsof(hd) ((dims *)((hd)+1))

// get the dimensions of a matrix (check for matrix type first)
#define cols(hd) (dimsof(hd)->c)
#define rows(hd) (dimsof(hd)->r)

// get the number of elements of a compressed matrix
#define nelementsof(hd) ((int *)((char *)((hd)+1)+sizeof(dims)))

// get the position of the elements of a compressed matrix
#define elementsof(hd) ((cpxelement *)((char *)((hd)+1)+sizeof(dims)+sizeof(int)))

// get the size of binary data
#define binsize(hd) ((int *)((char *)((hd)+1)))

// get the binary data
#define bindata(hd) ((void *)((char *)((hd)+1)+sizeof(int)))

// create a new header at newram (which is increased)
// take care of ramend!
header *make_header (int type, int size);

// create a new heap elements with string content
header *new_string (char *s);

// create a new double
header *new_real (double x);

// create a new complex number
header *new_complex (double x, double y);

// create a new interval
header *new_interval (double a, double b);

// create a new matrix, use matrixof to fill it row by row
header *new_matrix (int rows, int columns);

// create a new complex matrix
header *new_cmatrix (int rows, int columns);

// create a new interval matrix
header *new_imatrix (int rows, int columns);

// create a new string vector
header *new_smatrix ();

// create a new compressed matrix
header *new_cpxmatrix (int rows, int columns, int elements);

// create a new binary
header *new_binary (int n);

// for wait_key or test_key (lets the user interrupt the computation)
typedef enum { key_none, cursor_up, cursor_down, cursor_left, cursor_right,
	escape, deletekey, backspace, clear_home, switch_screen, enter,
	space, line_end, line_start, fk1, fk2, fk3, fk4, fk5, fk6, fk7,
	fk8, fk9, fk10, word_left, word_right, help }
	scantyp;

extern void (*output) (char *s);
extern void (*output1) (char *format,...);
extern int (*wait_key) (int *scan);
extern int (*test_key) ();
extern int (*eval_euler) (char *name, header *hd, char **newram);

// Declaration for exporting a function from a DLL
// e.g. init() and all your functions must be exported
#ifdef VISUALCPP
#define EXPORT  extern "C" __declspec(dllexport) // for Visual C++
#else
#define EXPORT  __declspec(dllexport)
#endif

// Use these to create error in the main function only!
// Otherwise return to main and use IFERROR(...)

// generate an error (only in main!)
#define ERROR(s) { error=1; errorstring=s; sprintf(ram,"%s",s); return ram; }

// generate an error, if error is not 0. (only in main!)
#define IFERROR(s) { if (error) { errorstring=s; sprintf(ram,"%s",s); return ram; } }

// do the command x and check the result. if 0, generate an error. (only in main!)
#define CHECK(x,s) { if (!(x)) ERROR(s); }

// print with C format strings
void print (char *format, ...);

// get the real number from the header, set the error, if not a real
double getreal (header *hd);

// get a real matrix with columns and rows
void getmatrix (header *hd, double **m, int *r, int *c);

// get the string from a string
char *getstring (header *hd);

// get the real and convert to int
int getint (header *hd);

// get the string
char *getstring (header *hd);

// get a chunk of the Euler RAM
char *getram (long size);

// move the result to the proper place at ram.
// This is only necessary, if you have used getram before generating hd.
void moveresults (header *hd);

// append a string to a string vector
// the string vector must be the last element on the stack.
int appendstring (header *hd, char *s);

// macro for real scalar arguments
// defines double x
#define ARG_DOUBLE(x) \
double x; \
CHECK(argn<np,"Too many ARG macros"); \
x=getreal(hd[argn++]); \
IFERROR("Need a real argument.");

// macro for real scalar integer values
#define ARG_INT(x) \
int x; \
CHECK(argn<np,"Too many ARG macros"); \
x=getint(hd[argn++]); \
IFERROR("Need an integer argument.");

// macro for real matrices or scalar arguments
// defines double *v and n (rows), m (columns)
#define ARG_DOUBLE_MATRIX(v,n,m) \
double *v; int n,m;  \
CHECK(argn<np,"Too many ARG macros"); \
getmatrix(hd[argn++],&v,&n,&m); \
IFERROR("Need a matrix argument.");

// macro for real matrices or scalar arguments
// defines double *v and n (rows), m (columns)
#define ARG_COMPLEX_MATRIX(v,n,m) \
double *v; int n,m;  \
CHECK(argn<np,"Too many ARG macros"); \
getcomplexmatrix(hd[argn++],&v,&n,&m); \
IFERROR("Need a matrix argument.");

// macro for real matrices or scalar arguments
// defines double *v and n (rows), m (columns)
#define ARG_INTERVAL_MATRIX(v,n,m) \
double *v; int n,m;  \
CHECK(argn<np,"Too many ARG macros"); \
getintervalmatrix(hd[argn++],&v,&n,&m); \
IFERROR("Need a matrix argument.");

// macro for string arguments
// defines char *s
#define ARG_STRING(s) \
char *s; \
CHECK(argn<np,"Too many ARG macros"); \
s=getstring(hd[argn++]); \
IFERROR("Need a string argument.");

// macro for string arguments
// defines char *s
#define ARG_STRING_VECTOR(s) \
char *s; \
CHECK(argn<np,"Too many ARG macros"); \
s=getstringvector(hd[argn++]); \
IFERROR("Need a string argument.");

// macro for binary arguments
// defines void *p and int n (size)
#define ARG_BINARY(p,n) \
CHECK(argn<np,"Too many ARG macros"); \
header *hd##p=hd[argn++]; \
int n=*binsize(hd##p); \
void *p=bindata(hd##p); \
CHECK(hd##p->type==s_binary,"Need binary data.");

// macro for compressed matrices
// defines cpxelement *p, n (rows), m (columns), k (elements)
#define ARG_CPX(p,n,m,k) \
CHECK(argn<np,"Too many ARG macros"); \
header *hd##p=hd[argn++]; \
cpxelement *p; int n,m,k; \
get_cpx(hd##p,&p,&n,&m,&k); \
IFERROR("Need a compressed matrix.");

// use the following macro for string arguments
#define RES_DOUBLE_MATRIX(v,n,m) \
header *hd##v=new_matrix(n,m); \
IFERROR("Out of stack space."); \
double *v=matrixof(hd##v)

// use the following macro for string arguments
#define RES_COMPLEX_MATRIX(v,n,m) \
header *hd##v=new_cmatrix(n,m); \
IFERROR("Out of stack space."); \
double *v=matrixof(hd##v)

// use the following macro for string arguments
#define RES_INTERVAL_MATRIX(v,n,m) \
header *hd##v=new_imatrix(n,m); \
IFERROR("Out of stack space."); \
double *v=matrixof(hd##v)

#define RES_BINARY(p,n) \
header *hd##p=new_binary(n); \
IFERROR("Out of stack space."); \
void *p=bindata(hd##p);

#define EVAL(s,hd) \
error=eval_euler(s,hd,&newram); newram=(char *)hd
