#ifndef STACKH
#define STACKH

void clear (void);
void remvalue (void);
void clearall (void);

header *new_matrix (int r, int c, char *name);
header *new_cmatrix (int r, int c, char *name);
header *new_imatrix (int r, int c, char *name);
header *new_cpxmatrix (int r, int c, int elements, char *name);
header *new_real (double x, char *name);
header *new_complex (double x, double y, char *name);
header *new_reference (header *hd, char *name);
header *new_submatrix (header *hd, header *cols, header *rows,
	char *name, int zerobased=0);
header *new_csubmatrix (header *hd, header *cols, header *rows,
	char *name, int zerobased=0);
header *new_isubmatrix (header *hd, header *cols, header *rows,
	char *name, int zerobased=0);
header *new_ssubmatrix (header *hd, int i, char *name);
header *new_command (int no);
header *new_string (char *s, ULONG size, char *name);
header *new_string (char *s, char *name);
header *new_smatrix (char *name);
header *new_udf (char *name);
header *new_subm (header *var, LONG l, char *name);
header *new_csubm (header *var, LONG l, char *name);
header *new_isubm (header *var, LONG l, char *name);
header *new_interval (double x, double y, char *name);
header *new_list (char *name);

int xor (char *n);
void setname (header *hd, char *name);

void kill_udf (char *name);

#define ParamHasValue 1
#define ParamStartsVector 2*ParamHasValue
#define ParamEndsVector 2*ParamStartsVector
#define ParamReal 2*ParamEndsVector
#define ParamComplex 2*ParamReal
#define ParamInterval 2*ParamComplex
#define ParamString 2*ParamInterval
#define ParamMatrix 2*ParamString
#define ParamVector 2*ParamMatrix
#define ParamColumn 2*ParamVector
#define ParamScalar 2*ParamColumn
#define ParamCPX 2*ParamScalar
#define ParamInteger 2*ParamCPX
#define ParamPositive 2*ParamInteger
#define ParamNonNegative 2*ParamPositive
#define ParamNone 2*ParamNonNegative
#define ParamCall 2*ParamNone

header *get_size (header *hd, int *n, int *m, char *function);
header *get_realmatrix (header *hd, double **m, int *r, int *c, char *function);

char *getram (long size, char *name);
int isscalar (header *hd);

header *nextafter (header *hd);

#endif
