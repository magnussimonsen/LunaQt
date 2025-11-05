#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>

#include "header.h"
#include "sysdep.h"
#include "funcs.h"
#include "interval.h"
#include "spread.h"
#include "stack.h"
#include "builtin.h"
#include "getvalue.h"

extern int builtinsp;

double (*func) (double);

void funceval (double *x, double *y)
/* evaluates the function stored in func with pointers. */
{	*y=func(*x);
}

header *map1 (void f(double *, double *),
	void fc(double *, double *, double *, double *),
	header *hd)
/***** map
	do the function elementwise to the value.
	te value may be real or complex!
******/
{	double x,y;
	dims *d;
	header *hd1;
	double *m,*m1;
	long i,n;
	if (hd->type==s_real)
	{	f(realof(hd),&x);
		return new_real(x,"");
	}
	else if (hd->type==s_matrix)
	{	d=dimsof(hd);
		hd1=new_matrix(d->r,d->c,"");
		if (error) return new_string("Fehler",6,"");
		m=matrixof(hd);
		m1=matrixof(hd1);
		n=d->c*d->r;
		for (i=0; i<n; i++)
			{	f(m,m1); m++; m1++;
				if (error) break;
			}
		return hd1;
	}
	else if (fc && hd->type==s_complex)
	{	fc(realof(hd),imagof(hd),&x,&y);
		return new_complex(x,y,"");
	}
	else if (fc && hd->type==s_cmatrix)
	{	d=dimsof(hd);
		hd1=new_cmatrix(d->r,d->c,"");
		if (error) return new_string("Fehler",6,"");
		m=matrixof(hd);
		m1=matrixof(hd1);
		n=d->c*d->r;
		for (i=0; i<n; i++)
			{	fc(m,m+1,m1,m1+1); m+=2; m1+=2;
				if (error) break;
			}
		return hd1;
	}
	wrongarg(hd);
	if (fc!=0) output("This function can only use real or complex values!\n"); 
	else output("This function can only use real values!\n"); 
	error=3;
	return new_string("Fehler",6,"");
}

header *map1i (void f(double *, double *, double *, double *),
	header *hd)
/***** map
	do the function elementwise to the value.
	te value may be real or complex!
******/
{	double x,y;
	header *result;
	double *m,*mr;
	int i,j;
	long k,n;
	if (hd->type==s_interval)
	{	f(aof(hd),bof(hd),&x,&y);
		return new_interval(x,y,"");
	}
	else if (hd->type==s_imatrix)
	{	getmatrix(hd,&i,&j,&m);
		result=new_imatrix(i,j,"");
		if (error) goto stop;
		n=(long)i*j;
		mr=matrixof(result);
		for (k=0; k<n; k++)
		{	f(m,m+1,mr,mr+1);
			m+=2; mr+=2;
			if (error) break;
		}
		return result;
	}
	wrongarg(hd);
	output("This function can only use interval values!\n"); 
	error=3;
	stop:
	return new_string("Fehler",6,"");
}

header *map1ir (
	void f(double *, double *, double *),
	header *hd)
/***** map
	do the function elementwise to the value.
	te value may be real or complex! the result is always real.
******/
{	double x;
	header *result;
	double *m,*mr;
	int i,j;
	long k,n;
	if (f && hd->type==s_interval)
	{	f(aof(hd),bof(hd),&x);
		return new_real(x,"");
	}
	else if (hd->type==s_imatrix)
	{	getmatrix(hd,&i,&j,&m);
		result=new_matrix(i,j,"");
		if (error) goto stop;
		n=(long)i*j;
		mr=matrixof(result);
		for (k=0; k<n; k++)
		{	f(m,m+1,mr);
			m+=2; mr++;
			if (error) break;
		}
		return result;
	}
	wrongarg(hd);
	output("This function can only use interval values!\n"); 
	error=3;
	stop :
	return new_string("Fehler",6,"");
}

header *map1r (void f(double *, double *),
	void fc(double *, double *, double *),
	header *hd)
/***** map
	do the function elementwise to the value.
	te value may be real or complex! the result is always real.
******/
{	double x;
	dims *d;
	header *hd1;
	double *m,*m1;
	int i,n;
	if (hd->type==s_real)
	{	f(realof(hd),&x);
		return new_real(x,"");
	}
	else if (hd->type==s_matrix)
	{	d=dimsof(hd);
		hd1=new_matrix(d->r,d->c,"");
		if (error) return new_string("Fehler",6,"");
		m=matrixof(hd);
		m1=matrixof(hd1);
		n=d->c*d->r;
		for (i=0; i<n; i++)
			{	f(m,m1); m++; m1++;
				if (error) break;
			}
		return hd1;
	}
	else if (fc!=0 && hd->type==s_complex)
	{	fc(realof(hd),imagof(hd),&x);
		return new_real(x,"");
	}
	else if (fc!=0 && hd->type==s_cmatrix)
	{	d=dimsof(hd);
		hd1=new_matrix(d->r,d->c,"");
		if (error) return new_string("Fehler",6,"");
		m=matrixof(hd);
		m1=matrixof(hd1);
		n=d->c*d->r;
		for (i=0; i<n; i++)
			{	fc(m,m+1,m1); m+=2; m1++;
				if (error) break;
			}
		return hd1;
	}
	wrongarg(hd);
	if (fc!=0) output("This function can only use real or complex values!\n"); 
	else output("This function can only use real values!\n"); 
	error=3;
	return new_string("Fehler",6,"");
}

header * mapf2 (void f (double *, double *, double *),
	void fc (double *, double *, double *, double *, double *, double *),
	void fi (double *, double *, double *, double *, double *, double *),
	header *hd1, header *hd2, char *name)
{	int t1,t2,t,r1,c1,r2,c2,rr,cr,r,c; /* means real */
	double *m1,*m2,*m,x,y,null=0.0,*l1,*l2;
	header *result;
	if (isreal(hd1)) t1=0;
	else if (iscomplex(hd1)) t1=1;
	else if (isinterval(hd1)) t1=2;
	else
	{	wrongarg(hd1); return 0;
	}
	if (isreal(hd2)) t2=0;
	else if (iscomplex(hd2)) t2=1;
	else if (isinterval(hd2)) t2=2;
	else
	{	wrongarg(hd2); return 0;
	}
	if ((t1==1 && t2==2) || (t1==2 && t2==1))
	{	output1("Cannot combine complex and intervals in %s!\n",name);
		error=10; return 0;
	}
	if (t1==0 && t2==0 && f==0)
	{	output1("Cannot evaluate %s for reals!\n",name);
		error=10; return 0;
	}
	if (fc==0 && (t1==1 || t2==1))
	{	output1("Cannot evaluate %s for complex values!\n",name);
		error=10; return 0;
	}
	if (fi==0 && (t1==2 || t2==2))
	{	output1("Cannot evaluate %s for intervals!\n",name);
		error=10; return 0;
	}
	getmatrix(hd1,&r1,&c1,&m1); l1=m1;
	getmatrix(hd2,&r2,&c2,&m2); l2=m2;
	if (r1==0 || c1==0 || r2==0 || c2==0 || (r1>1 && r2>1 && (r1!=r2)) ||
	 (c1>1 && c2>1 && (c1!=c2)))
	{   output1("Cannot combine a %dx%d and a %dx%d matrix for %s!\n",r1,c1,r2,c2,name);
		error=10; return 0;
	}
	rr=r1; if (rr<r2) rr=r2;
	cr=c1; if (cr<c2) cr=c2;
	t=t1; if (t2!=0) t=t2;
	switch (t)
	{	case 0 :
			if (rr==1 && cr==1)
			{	f(m1,m2,&x);
				return new_real(x,"");
			}
			result=new_matrix(rr,cr,"");
			if (error) return 0;
			m=matrixof(result);
			for (r=0; r<rr; r++)
			{	for (c=0; c<cr; c++)
				{	f(m1,m2,m);
					if (error) break;
					if (c1>1) m1++;
					if (c2>1) m2++;
					m++;
				}
				if (r1==1) m1=l1;
				else if (c1==1) m1++;
				if (r2==1) m2=l2;
				else if (c2==1) m2++;
				if (error) break;
			}
			return result;
		case 1 :
			if (rr==1 && cr==1)
			{	if (t1==0) fc(m1,&null,m2,m2+1,&x,&y);
				else if (t2==0) fc(m1,m1+1,m2,&null,&x,&y);
				else fc(m1,m1+1,m2,m2+1,&x,&y);
				return new_complex(x,y,"");
			}
			result=new_cmatrix(rr,cr,"");
			if (error) return 0;
			m=matrixof(result);
			for (r=0; r<rr; r++)
			{	for (c=0; c<cr; c++)
				{	if (t1==0)
					{	fc(m1,&null,m2,m2+1,m,m+1);
						if (c1>1) m1++;
						if (c2>1) m2+=2;
					}
					else if (t2==0)
					{	fc(m1,m1+1,m2,&null,m,m+1);
						if (c1>1) m1+=2;
						if (c2>1) m2++;
					}
					else
					{	fc(m1,m1+1,m2,m2+1,m,m+1);
						if (c1>1) m1+=2;
						if (c2>1) m2+=2;
					}
					if (error) break;
					m+=2;
				}
				if (r1==1) m1=l1;
				else if (c1==1)
				{	if (t1==0) m1++;
					else m1+=2;
				}
				if (r2==1) m2=l2;
				else if (c2==1)
				{	if (t2==0) m2++;
					else m2+=2;
				}
				if (error) break;
			}
			return result;
		case 2 :
			if (rr==1 && cr==1)
			{	if (t1==0) fi(m1,m1,m2,m2+1,&x,&y);
				else if (t2==0) fi(m1,m1+1,m2,m2,&x,&y);
				else fi(m1,m1+1,m2,m2+1,&x,&y);
				return new_interval(x,y,"");
			}
			result=new_imatrix(rr,cr,"");
			if (error) return 0;
			m=matrixof(result);
			for (r=0; r<rr; r++)
			{	for (c=0; c<cr; c++)
				{	if (t1==0)
					{	fi(m1,m1,m2,m2+1,m,m+1);
						if (c1>1) m1++;
						if (c2>1) m2+=2;
					}
					else if (t2==0)
					{	fi(m1,m1+1,m2,m2,m,m+1);
						if (c1>1) m1+=2;
						if (c2>1) m2++;
					}
					else
					{	fi(m1,m1+1,m2,m2+1,m,m+1);
						if (c1>1) m1+=2;
						if (c2>1) m2+=2;
					}
					if (error) break;
					m+=2;
				}
				if (r1==1) m1=l1;
				else if (c1==1)
				{	if (t1==0) m1++;
					else m1+=2;
				}
				if (r2==1) m2=l2;
				else if (c2==1)
				{	if (t2==0) m2++;
					else m2+=2;
				}
				if (error) break;
			}
			return result;
	}
	return 0;
}

void spreadf2 (void f (double *, double *, double *),
	void fc (double *, double *, double *, double *, double *, double *),
	void fi (double *, double *, double *, double *, double *, double *),
	header *hd, char *name)
{	header *result,*st=hd,*hd1;
	hd1=next_param(st); if (!hd1 || error) return;
	if (hd1->flags&ARGS && next_param(hd1))
	{	
		output("Too many parameters for operator! (Multiple returns?)\n");
		error=10; return;
	}
	hd=getvalue(hd); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	result=mapf2(f,fc,fi,hd,hd1,name);
	if (!error) moveresult(st,result);
}

header * mapf2r (void f (double *, double *, double *),
	void fc (double *, double *, double *, double *, double *),
	void fi (double *, double *, double *, double *, double *),
	header *hd1, header *hd2, char *name)
{	int t1,t2,t,r1,c1,r2,c2,r,c,rr,cr; /* means real */
	double *m1,*m2,*m,x,null=0.0,*l1,*l2;
	header *result;
	if (isreal(hd1)) t1=0;
	else if (iscomplex(hd1)) t1=1;
	else if (isinterval(hd1)) t1=2;
	else
	{	wrongarg(hd1); return 0;
	}
	if (isreal(hd2)) t2=0;
	else if (iscomplex(hd2)) t2=1;
	else if (isinterval(hd2)) t2=2;
	else
	{	wrongarg(hd2); return 0;
	}
	if ((t1==1 && t2==2) || (t1==2 && t2==1))
	{	output1("Cannot combine complex and intervals in %s!\n",name);
		error=1; return 0;
	}
	if (t1==0 && t2==0 && f==0)
	{	output1("Cannot evaluate %s for reals!\n",name);
		error=1; return 0;
	}
	if (fc==0 && (t1==1 || t2==1))
	{	output1("Cannot evaluate %s for complex values!\n",name);
		error=1; return 0;
	}
	if (fi==0 && (t1==2 || t2==2))
	{	output1("Cannot evaluate %s for intervals!\n",name);
		error=1; return 0;
	}
	getmatrix(hd1,&r1,&c1,&m1); l1=m1;
	getmatrix(hd2,&r2,&c2,&m2); l2=m2;
	if (r1==0 || c1==0 || r2==0 || c2==0 || (r1>1 && r2>1 && (r1!=r2)) ||
	 (c1>1 && c2>1 && (c1!=c2)))
	{   output1("Cannot combine a %dx%d and a %dx%d matrix!\n",r1,c1,r2,c2);
		error=1; return 0;
	}
	rr=r1; if (rr<r2) rr=r2;
	cr=c1; if (cr<c2) cr=c2;
	t=t1; if (t2!=0) t=t2;
	switch (t)
	{	case 0 :
			if (rr==1 && cr==1)
			{	f(m1,m2,&x);
				return new_real(x,"");
			}
			result=new_matrix(rr,cr,"");
			if (error) return 0;
			m=matrixof(result);
			for (r=0; r<rr; r++)
			{	for (c=0; c<cr; c++)
				{	f(m1,m2,m);
					if (error) break;
					if (c1>1) m1++;
					if (c2>1) m2++;
					m++;
				}
				if (r1==1) m1=l1;
				else if (c1==1) m1++;
				if (r2==1) m2=l2;
				else if (c2==1) m2++;
				if (error) break;
			}
			return result;
		case 1 :
			if (rr==1 && cr==1)
			{	if (t1==0) fc(m1,&null,m2,m2+1,&x);
				else if (t2==0) fc(m1,m1+1,m2,&null,&x);
				else fc(m1,m1+1,m2,m2+1,&x);
				return new_real(x,"");
			}
			result=new_matrix(rr,cr,"");
			if (error) return 0;
			m=matrixof(result);
			for (r=0; r<rr; r++)
			{	for (c=0; c<cr; c++)
				{	if (t1==0)
					{	fc(m1,&null,m2,m2+1,m);
						if (c1>1) m1++;
						if (c2>1) m2+=2;
					}
					else if (t2==0)
					{	fc(m1,m1+1,m2,&null,m);
						if (c1>1) m1+=2;
						if (c2>1) m2++;
					}
					else
					{	fc(m1,m1+1,m2,m2+1,m);
						if (c1>1) m1+=2;
						if (c2>1) m2+=2;
					}
					if (error) break;
					m++;
				}
				if (r1==1) m1=l1;
				else if (c1==1)
				{	if (t1==0) m1++;
					else m1+=2;
				}
				if (r2==1) m2=l2;
				else if (c2==1)
				{	if (t2==0) m2++;
					else m2+=2;
				}
				if (error) break;
			}
			return result;
		case 2 :
			if (rr==1 && cr==1)
			{	if (t1==0) fi(m1,m1,m2,m2+1,&x);
				else if (t2==0) fi(m1,m1+1,m2,m2,&x);
				else fi(m1,m1+1,m2,m2+1,&x);
				return new_real(x,"");
			}
			result=new_matrix(rr,cr,"");
			if (error) return 0;
			m=matrixof(result);
			for (r=0; r<rr; r++)
			{	for (c=0; c<cr; c++)
				{	if (t1==0)
					{	fi(m1,m1,m2,m2+1,m);
						if (c1>1) m1++;
						if (c2>1) m2+=2;
					}
					else if (t2==0)
					{	fi(m1,m1+1,m2,m2,m);
						if (c1>1) m1+=2;
						if (c2>1) m2++;
					}
					else
					{	fi(m1,m1+1,m2,m2+1,m);
						if (c1>1) m1+=2;
						if (c2>1) m2+=2;
					}
					if (error) break;
					m++;
				}
				if (r1==1) m1=l1;
				else if (c1==1)
				{	if (t1==0) m1++;
					else m1+=2;
				}
				if (r2==1) m2=l2;
				else if (c2==1)
				{	if (t2==0) m2++;
					else m2+=2;
				}
				if (error) break;
			}
			return result;
	}
	return 0;
}

void spreadf2r (void f (double *, double *, double *),
	void fc (double *, double *, double *, double *, double *),
	void fi (double *, double *, double *, double *, double *),
	header *hd, char *name)
{	header *result,*st=hd,*hd1;
	hd1=next_param(st); if (!hd1 || error) return;
	if (next_param(hd1))
	{	
		output("Too many parameters for operator! (Multiple returns?)\n");
		error=1; return;
	}
	hd=getvalue(hd); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	result=mapf2r(f,fc,fi,hd,hd1,name);
	if (!error) moveresult(st,result);
}

void spread1 (double f (double),
	void fc (double *, double *, double *, double *),
	header *hd)
{	header *result,*st=hd;
	hd=getvalue(hd);
	if (error) return;
	func=f;
	result=map1(funceval,fc,hd);
	if (!error) moveresult(st,result);
}

void spread1i (double f (double),
	void fc (double *, double *, double *, double *),
	void fi (double *, double *, double *, double *),
	header *hd)
{	header *result,*st=hd;
	hd=getvalue(hd);
	if (error) return;
	func=f;
	if (isinterval(hd)) result=map1i(fi,hd);
	else result=map1(funceval,fc,hd);
	if (!error) moveresult(st,result);
}

void spread1r (double f (double),
	void fc (double *, double *, double *),
	header *hd)
{	header *result,*st=hd;
	hd=getvalue(hd);
	if (error) return;
	func=f;
	result=map1r(funceval,fc,hd);
	if (!error) moveresult(st,result);
}

void spread1ir (double f (double),
	void fc (double *, double *, double *),
	void fi (double *, double *, double *, double *),
	header *hd)
{	header *result,*st=hd;
	hd=getvalue(hd);
	if (error) return;
	func=f;
	if (isinterval(hd)) result=map1i(fi,hd);
	else result=map1r(funceval,fc,hd);
	if (!error) moveresult(st,result);
}

void spreadir (
	void fi (double *, double *, double *),
	header *hd)
{	header *result,*st=hd;
	hd=getvalue(hd);
	if (error) return;
	result=map1ir(fi,hd);
	if (!error) moveresult(st,result);
}

void spreadir1 (
	void f (double *, double *),
	void fi (double *, double *, double *),
	header *hd)
{	header *result,*st=hd;
	hd=getvalue(hd);
	if (error) return;
	if (isinterval(hd)) result=map1ir(fi,hd);
	else result=map1(f,0,hd);
	if (!error) moveresult(st,result);
}

void mmap1 (header *hd)
{	map1f(hd,0);
}

void map1f (header *hd, char *f)
// map a function to all matrix arguments
// function is either called from map("f",...)
// or, if f!=0, to map the function f to the arguments
{   
	// print("Mapping %s \n",f);

	header *st=hd,*result,*hds[16],*hq,*hdudf=0;
	int i,n=0,t[16],row[16],col[16],rescol=1,resrow=1,r,c,mt=0,nsp;
	int foundudf=0;
	double *m[16],*mr=0;
	
	if (!hd || hd==(header *)newram)
	// no arguments
	{	print("Map needs arguments for function %s.\n",f);
		error=1; return;
	}
	
	// set hq to the arguments
	if (!f) hq=nextof(hd);
	else hq=hd;
	
	// collect arguments
	int nmap=-1;
	while (n<16)
	{	
		if (hq>=(header *)newram) break;
		// print("varname=\"%s\" type=%d\n",hq->name,hq->type);
		if (nmap==-1 && *hq->name && hq->type!=s_reference) nmap=n;
		// print("nmap=%d\n",nmap);
		hds[n++]=hq;
		hq=nextof(hq);
	}
	if (n<1 || n>=16)
	{	output("Map needs at least one and at most 16 arguments.\n");
		error=1; return;
	}
	
	// global variable indicating the semicolon
	if (builtinsp>0) 
	{	
		if (f) nsp=builtinsp;
		else nsp=builtinsp-1;
	}
	else 
	{	
		// check if f is a map function with semicolon
		if (f)
		{	
			hdudf=searchudf(f);
			if (hdudf)
			{	
				foundudf=1;
				char *p=helpof(hdudf)+2*sizeof(int);
				int isp=*((int *)p);
				if (n<isp)
				{	
					output1("Function %s needs at least %d arguments.\n",f,isp);
					error=1; return;
				}
				if (isp>0) nsp=isp;
				else nsp=n;
			}
			else nsp=n;
		}
		else nsp=n;
	}

	int argn=n;
	// print("nmap=%d nsp=%d\n",nmap,nsp);
	if (nmap!=-1 && nmap<nsp) nsp=argn=nmap;

	// get the function name
	if (!f)
	{	
		hd=getvalue(hd); if (error) return;
		if (hd->type!=s_string)
		{	output("Map needs a string as first argument.\n");
			error=1; return;
		}
		f=stringof(hd);
	}
	
	// check the sizes of all arguments up to the semicolon
	for (i=0; i<nsp; i++)
	{	
		hds[i]=getvalue(hds[i]); if (error) return;
		switch (hds[i]->type)
		{	
			case s_real :
				row[i]=1; col[i]=1; t[i]=1; m[i]=realof(hds[i]);
				break;
			case s_complex :
				row[i]=1; col[i]=1; t[i]=2; m[i]=realof(hds[i]);
				break;
			case s_interval :
				row[i]=1; col[i]=1; t[i]=3; m[i]=realof(hds[i]);
				break;
			case s_matrix :
				t[i]=1;
				mat :
				row[i]=dimsof(hds[i])->r;
				if (row[i]!=1 && resrow!=1 && resrow!=row[i])
				{	output("Rows do not match in map!\n");
					error=1; return;
				}
				if (row[i]!=1) resrow=row[i];
				col[i]=dimsof(hds[i])->c;
				if (col[i]!=1 && rescol!=1 && rescol!=col[i])
				{	output("Colums do not match in map!\n");
					error=1; return;
				}
				if (col[i]!=1) rescol=col[i];
				m[i]=matrixof(hds[i]);
				break;
			case s_cmatrix :
				t[i]=2; goto mat;
			case s_imatrix :
				t[i]=3; goto mat;
			case s_string :
				t[i]=4; break;
			case s_smatrix :
				row[i]=1; col[i]=smatrixsize(hds[i]);
				if (row[i]!=1 && resrow!=1 && resrow!=row[i])
				{	output("Rows do not match in map!\n");
					error=1; return;
				}
				if (row[i]!=1) resrow=row[i];
				if (col[i]!=1 && rescol!=1 && rescol!=col[i])
				{	output("Colums do not match in map!\n");
					error=1; return;
				}
				if (col[i]!=1) rescol=col[i];
				t[i]=5;
				break;
			default :
				wrong_arg_in("map");
		}
	}
	
	// run the function for all elements
	result=0;
	for (r=0; r<resrow; r++)
	{	
		for (c=0; c<rescol; c++)
		{	
			hq=(header *)newram;
			char *str=0;
			
			// extract one element up the semicolon
			for (i=0; i<nsp; i++)
			{	
				switch (t[i])
				{	
					case 1 :
						new_real(*(m[i]),"");
						break;
					case 2 :
						new_complex(*(m[i]),*(m[i]+1),"");
						break;
					case 3 :
						new_interval(*(m[i]),*(m[i]+1),"");
						break;
					case 4 :
						new_string(stringof(hds[i]),strlen(stringof(hds[i])),"");
						break;
					case 5 :
						str=stringelement(hds[i],c+1);
						new_string(str,strlen(str),"");
						break;
					default :
						output("Error!\n");
				}
			}
			
			// extract everything after the semicolon
			for (i=nsp; i<n; i++)
			{	
				moveresult((header *)newram,hds[i]);
			}
			
			// run the function
			if (foundudf)
			{	
				// print("%s n=%d nsp=%d argn=%d\n",hdudf->name,n,nsp,argn);
				interpret_udf(hdudf,hq,argn,0);
			}
			else
			{	
				if (!exec_builtin(f,argn,hq))
				{	
					hdudf=searchudf(f);
					if (!hdudf) hdudf=searchudfmap(f);
					if (error || !hdudf) error=1;
					else foundudf=1;
					// print("%s n=%d nsp=%d argn=%d\n",hdudf->name,n,nsp,argn);
					if (foundudf) interpret_udf(hdudf,hq,argn,0);
				}
			}
			if (error) { output("Error in map.\n"); return; }
			
			// plug the scalar result into the matrix result
			if (!result)
			// result needs to be made
			{   
				if (resrow==1 && rescol==1)
				{   
					moveresult(st,hq); return;
				}
				switch (hq->type)
				{	
					case s_real :
						result=new_matrix(resrow,rescol,"");
						if (error) return;
						mr=matrixof(result); mt=1;
						*mr++=*realof(hq);
						break;
					case s_complex :
						result=new_cmatrix(resrow,rescol,"");
						if (error) return;
						mr=matrixof(result); mt=2;
						*mr++=*realof(hq);
						*mr++=*(realof(hq)+1);
						break;
					case s_interval :
						result=new_imatrix(resrow,rescol,"");
						if (error) return;
						mr=matrixof(result); mt=3;
						*mr++=*realof(hq);
						*mr++=*(realof(hq)+1);
						break;
					case s_string :
						if (isnone(hq))
						{	mt=4;
							result=0;
							break;
						}
					default :
						output("Illegal function result in map.\n");
						error=1; return;
				}
			}
			else
			// result is already made
			{	
				switch (hq->type)
				{	
					case s_real :
						if (mt!=1 && mt!=2) goto err1;
						*mr++=*realof(hq); 
						if (mt == 2) *mr++ = 0;
						break;
					case s_complex :
						if (mt == 1) // real matrix result must be made complex
						{
							header *result1 = new_cmatrix(resrow, rescol, "");
							if (error) return;
							double *mr1 = matrixof(result1); mt = 2;
							double* m1 = matrixof(result);
							while (m1 < mr)
							{
								*mr1++ = *m1++;
								*mr1++ = 0.0;
							}
							result = result1;
							mr = mr1;
							*mr++ = *realof(hq); *mr++ = *(realof(hq) + 1);
							hq = nextof(result);
							break;
						}
						if (mt!=2) goto err1;
						*mr++=*realof(hq); *mr++=*(realof(hq)+1);
						break;
					case s_interval :
						if (mt!=3) goto err1;
						*mr++=*realof(hq); *mr++=*(realof(hq)+1);
						break;
					default :
						if (mt!=4) goto err1;
						break;
						err1 :
						output("Illegal function result in map.\n");
						error=1; return;
				}
				newram=(char *)hq;
			}
			
			// go to next scalar in matrix column
			for (i=0; i<nsp; i++)
				if (col[i]>1)
				{	
					if (t[i]==1) m[i]++;
					else if (t[i]<4) m[i]+=2;
				}
		}
		
		// go to next matrix rows
		for (i=0; i<nsp; i++)
		{   if (t[i]==1)
			{	if (col[i]==1) m[i]++;
				if (row[i]==1) m[i]-=col[i];
			}
			else if (t[i]<4)
			{	if (col[i]==1) m[i]+=2;
				if (row[i]==1) m[i]-=2*col[i];
			}
		}
	}
	
	if (!result) result=new_none();
	moveresult(st,result);
}

/**
Spread real function to several arguments.
*/
void spreadrn (header *hd, int nargs,
			double fr (double **, int , void *, char *),
			void *fp (header *),
			char *name)
{
	header *st=hd,*res,*hdx[MAXSPREADN];
	double *mx[MAXSPREADN],*mh[MAXSPREADN];
	int rx[MAXSPREADN],cx[MAXSPREADN];
	int r,c;

	// collect headers
	hdx[0]=hd;
	for (int i=1; i<nargs; i++)
	{
		hdx[i]=next_param(hdx[i-1]);
		if (hdx[i]==0) 
		{
			print("Not enough parameters for %s.\n",name);
			error=1; return; 
		}
	}
	for (int i=0; i<nargs; i++)
	{
		hdx[i]=getvalue(hdx[i]);
		if (error) return;
	}

	// get size of result
	r=c=0;
	for (int i=0; i<nargs; i++)
	{
		if (hdx[i]->type==s_real)
		{
			rx[i]=cx[i]=1;
			mx[i]=realof(hdx[i]);
		}
		else if (hdx[i]->type==s_matrix)
		{
			rx[i]=dimsof(hdx[i])->r;
			cx[i]=dimsof(hdx[i])->c;
			mx[i]=matrixof(hdx[i]);
		}
		else
		{
			print("Parameter %d not real in call to %s.\n",i,name);
			error=1; return; 
		}
		if (rx[i]>r) r=rx[i];
		if (cx[i]>c) c=cx[i];
	}
	for (int i=0; i<nargs; i++)
	{
		if ((rx[i]!=1 && rx[i]!=r) || (cx[i]!=1 && cx[i]!=c))
		{
			print("Cannot combine matrix sizes in %s (found %d-%d-matrix).\n",name,rx[i],cx[i]);
			error=1; return;
		}
	}

	// make result for the real case
	if (r==1 && c==1)
	{
		res=new_real(fr(mx,nargs,0,name),"");
		goto end;
	}

	// make result for matrices
	res=new_matrix(r,c,"");
	double *h=matrixof(res);
	for (int i=0; i<r; i++)
	{
		for (int k=0; k<nargs; k++) mh[k]=mx[k];
		for (int j=0; j<c; j++)
		{
			*h++=fr(mx,nargs,0,name);
			if (error || test_key()) return;
			for (int k=0; k<nargs; k++)
			{
				if (cx[k]>1) mx[k]++;
			}
		}
		for (int k=0; k<nargs; k++)
		{
			if (rx[k]>1) mx[k]=mh[k]+cx[k];
			else mx[k]=mh[k];
		}
	}

	end:
	moveresult(st,res);
}
