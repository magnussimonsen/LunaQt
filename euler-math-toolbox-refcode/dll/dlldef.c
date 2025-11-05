#include <string.h>
#include <windows.h>
#include <stdio.h>

#ifdef ERROR
#undef ERROR
#endif

#include "dlldef.h"

__declspec(dllexport) int DllEntryPoint (HINSTANCE hInstance,
	DWORD reason, LPVOID res)
{	return TRUE;
}

void (*output) (char *s);
void (*output1) (char *s,...);
int (*wait_key) (int *scan);
int (*test_key) ();
int (*eval_euler) (char *name, header *hd, char **xnewram);

int error=0;
char *errorstring="Unknown Error";

EXPORT void init (
	void (*foutput) (char *), 
	void (*foutput1) (char *, ...), 
	int (*fwait_key) (int *), 
	int (*ftest_key) (),
	int (*feval_euler) (char *, header *, char **))
{	
	output=foutput; output1=foutput1; 
	wait_key=fwait_key; test_key=ftest_key;
	eval_euler=feval_euler;
}

char *ram,*newram,*ramend;

void start (char *newr, char *rend)
{
	ram=newr;
	newram=newr;
	ramend=rend;
	error=0;
}

header *make_header (int type, int size)
{	
	header *hd=(header *)newram;
	if (newram+size>ramend) 
	{
		error=1; return 0;
	}
	hd->size=size;
	hd->type=type;
	hd->flags=0;
	*(hd->name)=0;
	hd->xor=0;
	newram+=size;
	return hd;
}

header *new_string (char *s)
{	
	int size=sizeof(header)+(strlen(s)/2+1)*2;
	header *hd=make_header(s_string,size);
	if (hd) strcpy(stringof(hd),s);
	return hd;
}

header *new_real (double x)
{	
	int size=sizeof(header)+sizeof(double);
	header *hd=make_header(s_real,size);
	if (hd) *realof(hd)=x;
	return hd;
}

header *new_complex (double x, double y)
{	
	int size=sizeof(header)+2*sizeof(double);
	header *hd=make_header(s_complex,size);
	if (hd)
	{	realof(hd)[0]=x;
		realof(hd)[1]=y;
	}
	return hd;
}

header *new_interval (double a, double b)
{	
	int size=sizeof(header)+2*sizeof(double);
	header *hd=make_header(s_interval,size);
	if (hd)
	{	realof(hd)[0]=a;
		realof(hd)[1]=b;
	}
	return hd;
}

header *new_matrix (int rows, int columns)
{	
	int size=sizeof(header)+sizeof(dims)+
		rows*columns*sizeof(double);
	header *hd=make_header(s_matrix,size);
	if (hd)
	{	dimsof(hd)->c=columns;
		dimsof(hd)->r=rows;
	}
	return hd;
}

header *new_cpxmatrix (int rows, int columns, int elements)
{
	int size=sizeof(header)+sizeof(dims)+
		sizeof(int)+(2*sizeof(int)+sizeof(double))*elements;
	header *hd=make_header(s_cpxmatrix,size);
	if (hd)
	{	dimsof(hd)->c=columns;
		dimsof(hd)->r=rows;
		*nelementsof(hd)=elements;
	}
	return hd;	
}

header *new_cmatrix (int rows, int columns)
{	
	int size=sizeof(header)+sizeof(dims)+
		2*rows*columns*sizeof(double);
	header *hd=make_header(s_cmatrix,size);
	if (hd)
	{	dimsof(hd)->c=columns;
		dimsof(hd)->r=rows;
	}
	return hd;
}

header *new_imatrix (int rows, int columns)
{	
	int size=sizeof(header)+sizeof(dims)+
		2*rows*columns*sizeof(double);
	header *hd=make_header(s_imatrix,size);
	if (hd)
	{	dimsof(hd)->c=columns;
		dimsof(hd)->r=rows;
	}
	return hd;
}

header *new_smatrix ()
{
	int size=sizeof(header)+1;
	header *hd=make_header(s_smatrix,size);
	if (hd) *stringof(hd)=1;
	return hd;
}

header *new_binary (int size)
{
	header *hd=make_header(s_binary,sizeof(header)+sizeof(int)+size);
	if (hd) *binsize(hd)=size;
	return hd;
}

void print (char *s, ...)
{	
	char text [32000];
	va_list v;
	va_start(v,s);
	vsprintf(text,s,v);
	output(text);
}

char *getstring (header *hd)
{
	if (hd->type!=s_string) { error=1; return 0; }
	return stringof(hd);
}

char *getstringvector (header *hd)
{
	if (hd->type!=s_smatrix) { error=1; return 0; }
	return stringof(hd);
}

int appendstring (header *hd, char *s)
{
	char *q=((char *)hd)+hd->size;
	int n=strlen(s);
	if (q+n+1>ramend)
	{
		error=1; return 0;
	}
	strcpy(q-1,s);
	q+=n;
	*q++=1;
	hd->size=q-(char *)hd;
	newram=q;
	return 1;
}

double getreal (header *hd)
{
	if (hd->type!=s_real) { error=1; return 0; }
	return *realof(hd);
}

int getint (header *hd)
{
	if (hd->type!=s_real) { error=1; return 0; }
	double x=*realof(hd);
	int n=(int)x;
	if (n!=x) { error=1; return 0; }
	return n;
}

void getmatrix (header *hd, double **m, int *r, int *c)
{
	if (hd->type==s_matrix)
	{
		*m=matrixof(hd);
		*r=dimsof(hd)->r;
		*c=dimsof(hd)->c;
	}
	else if (hd->type==s_real) 
	{
		*m=realof(hd);
		*r=*c=1;
	}
	else { error=1; }
}

void getcomplexmatrix (header *hd, double **m, int *r, int *c)
{
	if (hd->type==s_cmatrix)
	{
		*m=matrixof(hd);
		*r=dimsof(hd)->r;
		*c=dimsof(hd)->c;
	}
	else if (hd->type==s_complex) 
	{
		*m=realof(hd);
		*r=*c=1;
	}
	else { error=1; }
}

void getintervalmatrix (header *hd, double **m, int *r, int *c)
{
	if (hd->type==s_imatrix)
	{
		*m=matrixof(hd);
		*r=dimsof(hd)->r;
		*c=dimsof(hd)->c;
	}
	else if (hd->type==s_interval) 
	{
		*m=realof(hd);
		*r=*c=1;
	}
	else { error=1; }
}

void getcpx (header *hd, cpxelement **p, int *r, int *c, int *k)
{
	if (hd->type==s_cpxmatrix)
	{
		*p=elementsof(hd);
		*r=dimsof(hd)->r;
		*c=dimsof(hd)->c;
		*k=*nelementsof(hd);
	}
	else { error=1; }
}

char *getram (long size)
{
	if (newram+size>ramend)
	{
		error=1; return 0;
	}
	char *p=newram;
	newram+=size;
	return p;
}

void moveresults (header *hd)
{
	long size=newram-(char *)hd;
	memmove(ram,(char *)hd,size);
	newram=ram+size;
}
