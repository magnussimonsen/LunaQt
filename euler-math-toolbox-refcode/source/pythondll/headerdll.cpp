#define _CRT_SECURE_NO_WARNINGS

#include <string.h>

#include "eulerdll.h"

char *newram,*ramend;
extern int error;
extern char errorline[];

void output (char *s)
{
	strcpy(errorline,s);
}

int xor (char *n)
/***** xor
	compute a hashcode for the name n.
*****/
{	int r=0;
	while (*n) r^=*n++;
	return r;
}

void *make_header (stacktyp type, size_t size, char *name)
/***** make_header
	pushes a new element on the stack.
	return the position after the header.
******/
{	
	header *hd;
	char *erg;
	size=(((size-1)/ALIGNMENT)+1)*ALIGNMENT;
	hd=(header *)(newram);
	if (!freeram(size))
	{	
		output("Stack overflow!\n"); error=2;
		hd->size=sizeof(hd);
		hd->type=(stacktyp)1000;
		hd->flags=0;
		strcpy(hd->name,"");
		hd->xor=0;
		return 0;
	}
	hd->size=size;
	hd->type=type;
	hd->flags=0;
	if (*name)
	{	strcpy(hd->name,name);
		hd->xor=xor(name);
	}
	else
	{	*(hd->name)=0;
		hd->xor=0;
	}
	erg=newram+sizeof(header);
	newram+=size;
	return erg;
}


header *new_real (double x, char *name)
/***** new real
	pops a double on stack.
*****/
{	ULONG size;
	double *d;
	header *hd=(header *)newram;
	size=sizeof(header)+sizeof(double);
	d=(double *)make_header(s_real,size,name);
	if (d) *d=x;
	return hd;
}


header *new_string (char *s, char *name)
/***** new real
	pops a string on stack.
*****/
{	ULONG size;
	char *d;
	header *hd=(header *)newram;
	int l=(int)strlen(s);
	size=sizeof(header)+(((int)(l+1)/ALIGNMENT)+1)*ALIGNMENT;
	d=(char *)make_header(s_string,size,name);
	if (d) { strcpy(d,s); }
	return hd;
}


header *new_complex (double x, double y, char *name)
/***** new real
	pushes a complex on stack.
*****/
{	ULONG size;
	double *d;
	header *hd=(header *)newram;
	size=sizeof(header)+2*sizeof(double);
	d=(double *)make_header(s_complex,size,name);
	if (d) { *d=x; *(d+1)=y; }
	return hd;
}

header *new_matrix (int rows, int columns, char *name)
/***** new_matrix
	pops a new matrix on the stack.
*****/
{	ULONG size;
	dims *d;
	header *hd=(header *)newram;
	size=matrixsize(rows,columns);
	d=(dims *)make_header(s_matrix,size,name);
	if (d) { d->c=columns; d->r=rows; }
	return hd;
}


header *new_smatrix (char *name)
/***** new string vector
	pops an empty string vector on stack.
*****/
{	ULONG size;
	char *d;
	header *hd=(header *)newram;
	size=sizeof(header)+((int)(2/ALIGNMENT)+1)*ALIGNMENT;
	d=(char *)make_header(s_smatrix,size,name);
	if (d) d[0]=0; d[1]=1;
	return hd;
}

header *new_cmatrix (int rows, int columns, char *name)
/***** new_matrix
	pops a new matrix on the stack.
*****/
{	ULONG size;
	dims *d;
	header *hd=(header *)newram;
	size=matrixsize(rows,2*columns);
	d=(dims *)make_header(s_cmatrix,size,name);
	if (d) { d->c=columns; d->r=rows; }
	return hd;
}