#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "header.h"
#include "stack.h"
#include "getvalue.h"
#include "maxima.h"

extern int nosubmref,reportindex;

/* 
functions that manipulate the stack .
*/

#define superalign(n) ((((n)-1)/ALIGNMENT+1)*ALIGNMENT)

void kill_local (char *name);

void clear (void)
/***** clear
	clears the stack and remove all variables and functions.
*****/
{	
	char name[32];
	scan_space();
	int bracket=0;
	if (*next=='(')
	{	
		bracket=1; next++;
	}
	while(1)
	{	
		scan_name(name); if (error) return;
		kill_local(name);
		scan_space();
		if (*next==',') { next++; scan_space(); continue; }
		else break;
	}
	if (bracket)
	{	
		if (*next!=')')
		{	error=1; output("Closing bracket missing");
		}
		else next++;
	}
}

void remvalue (void)
/***** clear
	clears the stack and remove all variables and functions.
*****/
{	char name[32];
	scan_space();
	int bracket=0;
	if (*next=='(')
	{	bracket=1; next++;
	}
	while(1)
	{	scan_name(name); if (error) return;
		kill_local(name);
		char s[MAXLINE];
		strcpy(s,"remvalue("); strcat(s,name); strcat(s,")$"); callmaxima(s,0);
		scan_space();
		if (*next==',') { next++; scan_space(); continue; }
		else break;
	}
	if (bracket)
	{	if (*next!=')')
		{	error=1; output("Closing bracket missing");
		}
		else next++;
	}
	checkcommand("remvalue");
}

void clearall (void)
/***** clear
	clears the stack and remove all variables and functions.
*****/
{	
	scan_space();
	if (*next==';' || *next==',' || *next==0)
	{	endlocal=startlocal;
	}
	else error=1;
}

int strends (char *s, char *str)
{
	int n=strlen(s),m=strlen(str);
	if (n<m) return 0;
	return strcmp(s+n-m,str)==0;
}

int xor (char *n)
/***** xor
	compute a hashcode for the name n.
*****/
{	int r=0;
	while (*n) r^=*n++;
	return r;
}

void *make_header (stacktyp type, ULONG size, char *name)
/***** make_header
	pushes a new element on the stack.
	return the position after the header.
******/
{	header *hd;
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

header *new_imatrix (int rows, int columns, char *name)
/***** new_matrix
	pops a new interval matrix on the stack.
*****/
{	ULONG size;
	dims *d;
	header *hd=(header *)newram;
	size=matrixsize(rows,2*columns);
	d=(dims *)make_header(s_imatrix,size,name);
	if (d) { d->c=columns; d->r=rows; }
	return hd;
}

header *new_cpxmatrix (int rows, int columns, int elements, char *name)
/***** new_cpxmatrix
	pops a new compressed matrix on the stack.
*****/
{	ULONG size;
	dims *d;
	header *hd=(header *)newram;
	size=matrixsize(0,0)+sizeof(int)+(2*sizeof(int)+sizeof(double))*elements;
	d=(dims *)make_header(s_cpxmatrix,size,name);
	if (d) { d->c=columns; d->r=rows; *nelementsof(hd)=elements; }
	return hd;
}

header *new_command (int no)
/***** new_command
	pops a command on stack.
*****/
{	ULONG size;
	int *d;
	header *hd=(header *)newram;
	size=sizeof(header)+sizeof(int);
	d=(int *)make_header(s_command,size,"");
	if (d) *d=no;
	return hd;
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

header *new_string (char *s, ULONG length, char *name)
/***** new real
	pops a string on stack.
*****/
{	ULONG size;
	char *d;
	header *hd=(header *)newram;
	size=sizeof(header)+((int)((length+1)/ALIGNMENT)+1)*ALIGNMENT;
	d=(char *)make_header(s_string,size,name);
	if (d) { strncpy(d,s,length); d[length]=0; }
	return hd;
}

header *new_string (char *s, char *name)
/***** new real
	pops a string on stack.
*****/
{	ULONG size;
	char *d;
	header *hd=(header *)newram;
	int length=strlen(s);
	size=sizeof(header)+((int)((length+1)/ALIGNMENT)+1)*ALIGNMENT;
	d=(char *)make_header(s_string,size,name);
	if (d) { strncpy(d,s,length); d[length]=0; }
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

header *new_list (char *name)
/***** new string vector
	pops an empty list on stack.
*****/
{	ULONG size;
	header *hd=(header *)newram;
	size=sizeof(header);
	make_header(s_list,size,name);
	return hd;
}

header *new_udf (char *name)
/***** new real
	pops a udf on stack.
*****/
{	ULONG size;
	ULONG *d;
	header *hd=(header *)newram;
	size=sizeof(header)+sizeof(inttyp)+ALIGNMENT;
	d=(ULONG *)make_header(s_udf,size,name);
	if (d) { *d=sizeof(header)+sizeof(ULONG); *((char *)(d+1))=0; }
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

header *new_interval (double x, double y, char *name)
{	ULONG size;
	double *d;
	header *hd=(header *)newram;
	size=sizeof(header)+2*sizeof(double);
	d=(double *)make_header(s_interval,size,name);
	if (d) { *d=x; *(d+1)=y; }
	return hd;
}

header *new_reference (header *ref, char *name)
{	ULONG size;
	header **d;
	header *hd=(header *)newram;
	size=sizeof(header)+sizeof(header *);
	d=(header **)make_header(s_reference,size,name);
	if (d) *d=ref;
	return hd;
}

header *new_subm (header *var, LONG l, char *name)
/* makes a new submatrix, which is a single element */
{	ULONG size;
	header **d,*hd=(header *)newram;
	dims *dim;
	int *n,r,c;
	size=sizeof(header)+sizeof(header *)+
		sizeof(dims)+2*sizeof(int);
	d=(header **)make_header(s_submatrix,size,name);
	if (d) *d=var;
	else return hd;
	dim=(dims *)(d+1);
	dim->r=1; dim->c=1;
	n=(int *)(dim+1);
	c=dimsof(var)->c;
	if (c==0 || dimsof(var)->r==0)
	{	output("Matrix is empty!\n"); error=1031; return hd;
	}
	else r=(int)(l/c);
	*n++=r;
	*n=(int)(l-(LONG)r*c-1);
	return hd;
}

header *new_csubm (header *var, LONG l, char *name)
/* makes a new submatrix, which is a single element */
{	ULONG size;
	header **d,*hd=(header *)newram;
	dims *dim;
	int *n,r,c;
	size=sizeof(header)+sizeof(header *)+
		sizeof(dims)+2*sizeof(int);
	d=(header **)make_header(s_csubmatrix,size,name);
	if (d) *d=var;
	else return hd;
	dim=(dims *)(d+1);
	dim->r=1; dim->c=1;
	n=(int *)(dim+1);
	c=dimsof(var)->c;
	if (c==0 || dimsof(var)->r==0)
	{	output("Matrix is empty!\n"); error=1031; return hd;
	}
	else r=(int)(l/c);
	*n++=r;
	*n=(int)(l-r*c-1);
	return hd;
}

header *new_isubm (header *var, LONG l, char *name)
/* makes a new submatrix, which is a single element */
{	ULONG size;
	header **d,*hd=(header *)newram;
	dims *dim;
	int *n,r,c;
	size=sizeof(header)+sizeof(header *)+
		sizeof(dims)+2*sizeof(int);
	d=(header **)make_header(s_isubmatrix,size,name);
	if (d) *d=var;
	else return hd;
	dim=(dims *)(d+1);
	dim->r=1; dim->c=1;
	n=(int *)(dim+1);
	c=dimsof(var)->c;
	if (c==0 || dimsof(var)->r==0)
	{	output("Matrix is empty!\n"); error=1031; return hd;
	}
	else r=(int)(l/c);
	*n++=r;
	*n=(int)(l-r*c-1);
	return hd;
}

header *hnew_submatrix (header *var, header *rows, header *cols,
	char *name, int type, int zerobased)
{	
	// print("Creating sub-matrix of %s with zerobased=%d\n",var->name,zerobased);
	ULONG size;
	header **d;
	double *mr=0,*mc=0,x,*mvar;
	dims *dim;
	int c,r,*n,i,c0,r0,cvar,rvar,allc=0,allr=0;
	header *hd=(header *)newram;
	getmatrix(var,&rvar,&cvar,&mvar);
	if (rows->type==s_matrix)
	{	
		if (dimsof(rows)->r==1) r=dimsof(rows)->c;
		else if (dimsof(rows)->c==1) r=dimsof(rows)->r;
		else
		{	
			output("Illegal index!\n"); error=41; return 0;
		}
		mr=matrixof(rows);
	}
	else if (rows->type==s_real)
	{	
		r=1; mr=realof(rows);
	}
	else if (rows->type==s_command && *commandof(rows)==c_allv)
	{	allr=1; r=rvar;
	}
	else
	{	output("Illegal index!\n"); error=41; return 0;
	}
	if (cols->type==s_matrix)
	{	
		if (dimsof(cols)->r==1) c=dimsof(cols)->c;
		else if (dimsof(cols)->c==1) c=dimsof(cols)->r;
		else
		{	output("Illegal index!\n"); error=41; return 0;
		}
		mc=matrixof(cols);
	}
	else if (cols->type==s_real)
	{	
		c=1; mc=realof(cols);
	}
	else if (cols->type==s_command && *commandof(cols)==c_allv)
	{	
		allc=1; c=cvar;
	}
	else
	{	
		output("Illegal index!\n"); error=41; return 0;
	}
	size=sizeof(header)+sizeof(header *)+
		sizeof(dims)+((LONG)r+c)*sizeof(int);
	d=(header **)make_header((stacktyp)type,size,name);
	if (d) *d=var;
	else return hd;
	dim = (dims *)(d+1);
	n=(int *)(dim+1);
	r0=0;
	if (allr)
	{	
		for (i=0; i<rvar; i++) *n++=i;
		r0=rvar;
	}
	else for (i=0; i<r; i++)
	{	
		x=*mr++;
		if (!((x<1.0-zerobased) || (x>=rvar+1-zerobased)) )
		{	
			*n++=(int)(x-1)+zerobased; r0++;
		}
		else if (!((x>-1.0) || (x<=-rvar-1)) )
		{	
			*n++=rvar-1-(int)(-x-1); r0++;
		}		
		else if (reportindex && (!udfon || !relax))
		{	
			output1("Row index %d out of bounds!\n",(int)x);
			error=1; return 0;
		}
	}
	c0=0;
	if (allc)
	{	for (i=0; i<cvar; i++) *n++=i;
		c0=cvar;
	}
	else for (i=0; i<c; i++) 
	{	
		x=*mc++;
		if (!((x<1.0-zerobased) || (x>=cvar+1-zerobased))) 
		{	*n++=(int)(x-1)+zerobased; c0++;
		}
		else if (!((x>-1.0) || (x<=-cvar-1)) )
		{	*n++=cvar-1-(int)(-x-1); c0++;
		}
		else if (reportindex && (!udfon || !relax))
		{	output1("Column index %d out of bounds!\n",(int)x);
			error=1; return 0;
		}
	}
	dim->r=r0; dim->c=c0;
	size=(char *)n-(char *)hd;
	size=((size-1)/ALIGNMENT+1)*ALIGNMENT;
	newram=(char *)hd+size;
	hd->size=size;
	return hd;
}

header *hnew_ssubmatrix (header *var, int i, char *name)
{
	header *hd=(header *)newram;
	int size=sizeof(header)+sizeof(header *)+sizeof(int);
	if (!make_header(s_ssubmatrix,size,name)) return hd;
	referenceof(hd)=var;
	*indexof(hd)=i;
	return hd;
}

header *built_smatrix (header *var, header *rows, header *cols, int zerobased)
/***** built_smatrix
	built a submatrix from the matrix hd on the stack.
*****/
{	
	double *mr=0,*mc=0,*mvar,*m;
	int n,c,r,c0,r0,i,j,cvar,rvar,allc=0,allr=0,*pr,*pc,*nc,*nr;
	header *hd;
	char *ram;
	LONG size;
	getmatrix(var,&rvar,&cvar,&mvar);
	if (rows->type==s_matrix)
	{	if (dimsof(rows)->r==1) r=dimsof(rows)->c;
		else if (dimsof(rows)->c==1) r=dimsof(rows)->r;
		else
		{	output("Illegal index!\n"); error=41; return 0;
		}
		mr=matrixof(rows);
	}
	else if (rows->type==s_real)
	{	r=1; mr=realof(rows);
	}
	else if (rows->type==s_command && *commandof(rows)==c_allv)
	{	allr=1; r=rvar;
	}
	else
	{	output("Illegal index!\n"); error=41; return 0;
	}
	if (cols->type==s_matrix)
	{	if (dimsof(cols)->r==1) c=dimsof(cols)->c;
		else if (dimsof(cols)->c==1) c=dimsof(cols)->r;
		else
		{	output("Illegal index!\n"); error=41; return 0;
		}
		mc=matrixof(cols);
	}
	else if (cols->type==s_real)
	{	c=1; mc=realof(cols);
	}
	else if (cols->type==s_command && *commandof(cols)==c_allv)
	{	allc=1; c=cvar;
	}
	else
	{	output("Illegal index!\n"); error=41; return 0;
	}
	ram=newram;
	size=superalign(((LONG)c+(LONG)r)*sizeof(int));
	if (!freeram(size))
	{	output("Out of memory!\n"); error=710; return 0;
	}
	nr=pr=(int *)ram; nc=pc=pr+r;
	newram=ram+size;
	c0=0; r0=0;
	if (allc) { for (i=0; i<c; i++) pc[i]=i; c0=c; }
	else for (i=0; i<c; i++)
	{	n=(int)(*mc++)-1+zerobased;
		if (n>=0 && n<cvar) { *nc++=n; c0++; }
		else if (cvar+n+1>=0 && n<-1) { *nc++=cvar+n+1; c0++; }
		else if (reportindex && (!udfon || !relax))
		{	output1("Index %d out of bounds!\n",n+1);
			error=1; return 0;
		}
	}
	if (allr) { for (i=0; i<r; i++) pr[i]=i; r0=r; }
	else for (i=0; i<r; i++) 
	{	n=(int)(*mr++)-1+zerobased;
		if (n>=0 && n<rvar) { *nr++=n; r0++; }
		else if (rvar+n+1>=0 && n<-1) { *nr++=rvar+n+1; r0++; }
		else if (reportindex && (!udfon || !relax))
		{	output1("Index %d out of bounds!\n",n+1);
			error=1; return 0;
		}
	}
	if (c0==1 && r0==1)
	{	return new_real(*mat(mvar,cvar,pr[0],pc[0]),"");
	}
	hd=new_matrix(r0,c0,""); if (error) return 0;
	m=matrixof(hd);
	for (i=0; i<r0; i++)
		for (j=0; j<c0; j++)
			*m++=*mat(mvar,cvar,pr[i],pc[j]);
	return hd;
}

header *built_csmatrix (header *var, header *rows, header *cols, int zerobased)
/***** built_csmatrix
	built a complex submatrix from the matrix hd on the stack.
*****/
{	double *mr=0,*mc=0,*mvar,*mh,*m;
	int n,c,r,c0,r0,i,j,cvar,rvar,allc=0,allr=0,*pc,*pr,*nc,*nr;
	header *hd;
	char *ram;
	LONG size;
	getmatrix(var,&rvar,&cvar,&mvar);
	if (rows->type==s_matrix)
	{	if (dimsof(rows)->r==1) r=dimsof(rows)->c;
		else if (dimsof(rows)->c==1) r=dimsof(rows)->r;
		else
		{	output("Illegal index!\n"); error=41; return 0;
		}
		mr=matrixof(rows);
	}
	else if (rows->type==s_real)
	{	r=1; mr=realof(rows);
	}
	else if (rows->type==s_command && *commandof(rows)==c_allv)
	{	allr=1; r=rvar;
	}
	else
	{	output("Illegal index!\n"); error=41; return 0;
	}
	if (cols->type==s_matrix)
	{	if (dimsof(cols)->r==1) c=dimsof(cols)->c;
		else if (dimsof(cols)->c==1) c=dimsof(cols)->r;
		else
		{	output("Illegal index!\n"); error=41; return 0;
		}
		mc=matrixof(cols);
	}
	else if (cols->type==s_real)
	{	c=1; mc=realof(cols);
	}
	else if (cols->type==s_command && *commandof(cols)==c_allv)
	{	allc=1; c=cvar;
	}
	else
	{	output("Illegal index!\n"); error=41; return 0;
	}
	ram=newram;
	size=superalign(((LONG)c+(LONG)r)*sizeof(int));
	if (!freeram(size))
	{	output("Out of memory!\n"); error=710; return 0;
	}
	nr=pr=(int *)ram; nc=pc=pr+r;
	newram=ram+size;
	c0=0; r0=0;
	if (allc) { for (i=0; i<c; i++) pc[i]=i; c0=c; }
	else for (i=0; i<c; i++)
	{	n=(int)(*mc++)-1+zerobased;
		if (n>=0 && n<cvar) { *nc++=n; c0++; }
		else if (cvar+n+1>=0 && n<-1) { *nc++=cvar+n+1; c0++; }
		else if (reportindex && (!udfon || !relax))
		{	output1("Index %d out of bounds!\n",n+1);
			error=1; return 0;
		}
	}
	if (allr) { for (i=0; i<r; i++) pr[i]=i; r0=r; }
	else for (i=0; i<r; i++) 
	{	n=(int)(*mr++)-1+zerobased;
		if (n>=0 && n<rvar) { *nr++=n; r0++; }
		else if (rvar+n+1>=0 && n<-1) { *nr++=rvar+n+1; r0++; }
		else if (reportindex && (!udfon || !relax))
		{	output1("Index %d out of bounds!\n",n+1);
			error=1; return 0;
		}
	}
	if (c0==1 && r0==1)
	{	m=cmat(mvar,cvar,pr[0],pc[0]);
		return new_complex(*m,*(m+1),"");
	}
	hd=new_cmatrix(r0,c0,""); if (error) return 0;
	m=matrixof(hd);
	for (i=0; i<r0; i++)
		for (j=0; j<c0; j++)
		{	mh=cmat(mvar,cvar,pr[i],pc[j]);
			*m++=*mh++;
			*m++=*mh;
		}
	return hd;
}

header *built_ismatrix (header *var, header *rows, header *cols, int zerobased)
/***** built_csmatrix
	built a complex submatrix from the matrix hd on the stack.
*****/
{	double *mr=0,*mc=0,*mvar,*mh,*m;
	int n,c,r,c0,r0,i,j,cvar,rvar,allc=0,allr=0,*pc,*pr,*nc,*nr;
	header *hd;
	char *ram;
	LONG size;
	getmatrix(var,&rvar,&cvar,&mvar);
	if (rows->type==s_matrix)
	{	if (dimsof(rows)->r==1) r=dimsof(rows)->c;
		else if (dimsof(rows)->c==1) r=dimsof(rows)->r;
		else
		{	output("Illegal index!\n"); error=41; return 0;
		}
		mr=matrixof(rows);
	}
	else if (rows->type==s_real)
	{	r=1; mr=realof(rows);
	}
	else if (rows->type==s_command && *commandof(rows)==c_allv)
	{	allr=1; r=rvar;
	}
	else
	{	output("Illegal index!\n"); error=41; return 0;
	}
	if (cols->type==s_matrix)
	{	if (dimsof(cols)->r==1) c=dimsof(cols)->c;
		else if (dimsof(cols)->c==1) c=dimsof(cols)->r;
		else
		{	output("Illegal index!\n"); error=41; return 0;
		}
		mc=matrixof(cols);
	}
	else if (cols->type==s_real)
	{	c=1; mc=realof(cols);
	}
	else if (cols->type==s_command && *commandof(cols)==c_allv)
	{	allc=1; c=cvar;
	}
	else
	{	output("Illegal index!\n"); error=41; return 0;
	}
	ram=newram;
	size=superalign(((LONG)c+(LONG)r)*sizeof(int));
	if (!freeram(size))
	{	output("Out of memory!\n"); error=710; return 0;
	}
	nr=pr=(int *)ram; nc=pc=pr+r;
	newram=ram+size;
	c0=0; r0=0;
	if (allc) { for (i=0; i<c; i++) pc[i]=i; c0=c; }
	else for (i=0; i<c; i++)
	{	n=(int)(*mc++)-1+zerobased;
		if (n>=0 && n<cvar) { *nc++=n; c0++; }
		else if (cvar+n+1>=0 && n<-1) { *nc++=cvar+n+1; c0++; }
		else if (reportindex && (!udfon || !relax))
		{	output1("Index %d out of bounds!\n",n+1);
			error=1; return 0;
		}
	}
	if (allr) { for (i=0; i<r; i++) pr[i]=i; r0=r; }
	else for (i=0; i<r; i++)
	{	n=(int)(*mr++)-1+zerobased;
		if (n>=0 && n<rvar) { *nr++=n; r0++; }
		else if (rvar+n+1>=0 && n<-1) { *nr++=rvar+n+1; r0++; }
		else if (reportindex && (!udfon || !relax))
		{	output1("Index %d out of bounds!\n",n+1);
			error=1; return 0;
		}
	}
	if (c0==1 && r0==1)
	{	m=imat(mvar,cvar,pr[0],pc[0]);
		return new_interval(*m,*(m+1),"");
	}
	hd=new_imatrix(r0,c0,""); if (error) return 0;
	m=matrixof(hd);
	for (i=0; i<r0; i++)
		for (j=0; j<c0; j++)
		{	mh=cmat(mvar,cvar,pr[i],pc[j]);
			*m++=*mh++;
			*m++=*mh;
		}
	return hd;
}

header *new_submatrix (header *hd, header *rows, header *cols,
	char *name, int zerobased)
{	
	if (nosubmref) return built_smatrix(hd,rows,cols,zerobased);
	return hnew_submatrix(hd,rows,cols,name,s_submatrix,zerobased);
}

header *new_csubmatrix (header *hd, header *rows, header *cols,
	char *name, int zerobased)
{	if (nosubmref) return built_csmatrix(hd,rows,cols,zerobased);
	return hnew_submatrix(hd,rows,cols,name,s_csubmatrix,zerobased);
}

header *new_isubmatrix (header *hd, header *rows, header *cols,
	char *name, int zerobased)
{	if (nosubmref) return built_ismatrix(hd,rows,cols,zerobased);
	return hnew_submatrix(hd,rows,cols,name,s_isubmatrix,zerobased);
}

header *new_ssubmatrix (header *var, int i, char *name)
{	
	if (nosubmref)
	{
		char *str=stringelement(var,i);
		if (error) return 0;
		header *res=new_string(str,strlen(str)+1,"");
		if (var->flags&UTF) res->flags|=UTF;
		return res;
	}
	else
	{
		return hnew_ssubmatrix(var,i,name);
	}
}

void kill_udf (char *name)
/***** kill_udf
	kill a local variable name, if there is one.
*****/
{	
	ULONG size,rest;
	header *hd=(header *)ramstart;
	while ((char *)hd<udfend)
	{	
		if (!strcmp(hd->name,name)) /* found! */
		{	
			size=hd->size;
			rest=newram-(char *)hd-size;
			if (size && rest) memmove((char *)hd,(char *)hd+size,rest);
			endlocal-=size; startlocal-=size; newram-=size;
			udfend-=size;
			invalidate_udflist();
			return;
		}
		hd=(header *)((char *)hd+hd->size);
	}
}

header *get_vector_or_reals (header *hd, int *n, int *m, char *function, char *type)
// get the [n,m] or n and m
{	header *ret=hd,*hd1;
	hd1=next_param(hd);
	hd=getvalue(hd); if (error) return ret;
	if (hd->type==s_matrix)
	{	ret=next_param(ret);
		if (dimsof(hd)->r==1 && dimsof(hd)->c==2)
		{	double *x=matrixof(hd),rows,cols;
			rows=*x++; cols=*x++;
			if (rows<0 || rows>=INT_MAX || cols<0 || cols>=INT_MAX)
			{	output1("Illegal size %g, %g in %s\n",
					rows,cols,function);
				error=1; return ret;
			}
			*n=(int)rows; *m=(int)cols;
		}
		else
		{	output1("Matrix for %s must be 1x2 not %dx%d in %s\n",
				type,dimsof(hd)->r,dimsof(hd)->c,function);
			error=1; return ret;
		}
	}
	else if (hd->type==s_real)
	{	double rows,cols;
		rows=*realof(hd);
		if (!hd1)
		{	rows=1; cols=*realof(hd);
			*n=(int)rows; *m=(int)cols;
			ret=next_param(ret);
			goto cont;
		}
		ret=next_param(hd1);
		hd1=getvalue(hd1); if (error) return ret;
		if (!hd1->type==s_real)
		{	output1("Expected %s [n,m] or n, m in %s\n",type,function);
			error=1; return ret;
		}
		cols=*realof(hd1);
		cont:
		if (rows<0 || rows>=INT_MAX || cols<0 || cols>=INT_MAX)
		{	output1("Illegal %s %g, %g in %s\n",
				type,rows,cols,function);
			error=1; return ret;
		}
		*n=(int)rows; *m=(int)cols;
	}
	else
	{	output1("Expected size [n,m] or n, m in %s\n",function);
		error=1; return ret;
	}
	return ret;
}

header *get_size (header *hd, int *n, int *m, char *function)
{	return get_vector_or_reals(hd,n,m,function,"size");
}

header *get_realmatrix (header *hd, double **m, int *r, int *c, char *function)
// get a matrix from the stack
{	header *ret=next_param(hd);
	hd=getvalue(hd); if (error) return ret;
	if (hd->type==s_matrix)
	{	*m=matrixof(hd); *r=dimsof(hd)->r; *c=dimsof(hd)->c;
	}
	else if (hd->type==s_real)
	{	*m=realof(hd); *r=1; *c=1;
	}
	else
	{	output1("Expect matrix in %s\n",function);
		error=1; return ret;
	}
	return ret;
}

void moveresult (header *stack, header *result)
/***** moveresult
	move the result to the start of stack.
*****/
{	if (stack==result) return;
	memmove((char *)stack,(char *)result,result->size);
	newram=(char *)stack+stack->size;
}

void moveresult1 (header *stack, header *result)
/***** moveresult
	move several results to the start of stack.
*****/
{	ULONG size;
	if (stack==result) return;
	size=newram-(char *)result;
	memmove((char *)stack,(char *)result,size);
	newram=(char *)stack+size;
}

char *getram (long size, char *name)
/**
	get a portion of the stack for temporary use.
	the function should reset the old newram after use.
*/
{
	if (newram+size>ramend)
	{
		print("Out of stack space in %s\n",name);
		error=1; return newram;
	}
	char *result=newram;
	newram+=size;
	return result;
}

int isscalar (header *hd)
{
	if (hd->type==s_real || hd->type==s_complex || hd->type==s_interval) return 1;
	if ((hd->type==s_matrix || hd->type==s_cmatrix || hd->type==s_imatrix)
		&& (dimsof(hd)->r==1 && dimsof(hd)->c==1)) return 1;
	return 0;
}

double getvre (header *hd, int i)
{
	if (hd->type==s_real) return *realof(hd);
	else if (hd->type==s_matrix) return matrixof(hd)[i];
	else if (hd->type==s_complex) return *realof(hd);
	else if (hd->type==s_cmatrix) return matrixof(hd)[2*i];
	else return 0.0;
}

double getvim (header *hd, int i)
{
	if (hd->type==s_real) return 0.0;
	else if (hd->type==s_matrix) return 0.0;
	else if (hd->type==s_complex) return *(realof(hd)+1);
	else if (hd->type==s_cmatrix) return matrixof(hd)[2*i+1];
	else return 0.0;
}

int isnone (header *hd)
{
	if (!hd) return 1;
	return hd->type==s_string && *stringof(hd)>=1 && *stringof(hd)<=5 && *(stringof(hd)+1)==0;
}

header *new_none (int k)
{
	header *hd=new_string(" ","");
	*stringof(hd)=(char)k;
	return hd;
}

header *nextafter (header *hd)
	// next parameter after hd, unless hd has flag args.
	// then return newram.
{
	if (hd->flags&ARGS) 
		return (header *)newram;
	else if (newram>(char *)hd)
	{ 
		newram=(char *)nextof(hd); 
		return (header *)(newram);
	}
	else 
	{
		return hd;
	}
}