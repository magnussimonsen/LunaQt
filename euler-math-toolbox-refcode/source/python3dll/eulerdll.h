#ifdef WIN64
#define LONG _int64
#define ULONG unsigned _int64
#else
#define LONG long
#define ULONG unsigned long
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef ALIGNMENT
#define ALIGNMENT 8
#endif

typedef enum { s_real,s_complex,s_matrix,s_cmatrix,
	s_reference,s_command,s_submatrix,s_csubmatrix,s_string,s_udf,
	s_interval,s_imatrix,s_isubmatrix,s_smatrix,
	s_cpxmatrix,s_ssubmatrix,s_list,s_binary }
	stacktyp;

extern char * s_names[];

#define maxname 32

typedef struct
{	LONG size; char name[maxname]; int xor; stacktyp type; int flags;
}
header;

typedef struct { int c,r;
} 
dims;

typedef struct { int c,r; double x;
} 
xelement;

#define freeram(n) (((unsigned long)(newram)+(unsigned long)(n))<=(unsigned long)ramend)

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

header *new_matrix (int r, int c, char *name);
header *new_cmatrix (int r, int c, char *name);
header *new_imatrix (int r, int c, char *name);
header *new_cpxmatrix (int r, int c, int elements, char *name);
header *new_real (double x, char *name);
header *new_complex (double x, double y, char *name);
header *new_reference (header *hd, char *name);
header *new_submatrix (header *hd, header *cols, header *rows,
	char *name);
header *new_csubmatrix (header *hd, header *cols, header *rows,
	char *name);
header *new_isubmatrix (header *hd, header *cols, header *rows,
	char *name);
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

#define matrixsize(c,r) (sizeof(header)+sizeof(dims)+(c)*(LONG)(r)*sizeof(double))
#define cmatrixsize(c,r) (sizeof(header)+sizeof(dims)+2l*(c)*(LONG)(r)*sizeof(double))

#define setsize(hd,n) hd->size=(((n-1)/ALIGNMENT)+1)*ALIGNMENT
#define setsizeto(hd,p) hd->size=(((((char *)p-(char *)hd)-1)/ALIGNMENT)+1)*ALIGNMENT

extern char *newram,*ramend;