#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>

#include "header.h"
#include "sysdep.h"
#include "funcs.h"
#include "interval.h"
#include "builtin.h"
#include "express.h"
#include "stack.h"
#include "spread.h"
#include "builtin.h"
#include "maxima.h"
#include "elementary.h"
#include "getvalue.h"

// routines to interpret Euler expressions

void equal_params_3 (header **hd1, header **hd2, header **hd3)
/* Make parameter values of equal type (real, complex, interval)
*/
{	header *h1,*h2,*h3;
	h1=getvariablesub(*hd1); if (error) return;
	h2=getvariablesub(*hd2); if (error) return;
	h3=getvariablesub(*hd3); if (error) return;
	if (iscomplex(h1) || iscomplex(h2) || iscomplex (h3))
	{	make_complex(*hd1);
		*hd2=nextof(*hd1);
		make_complex(*hd2);
		*hd3=nextof(*hd2);
		make_complex(*hd3);
	}
	else if (isinterval(h1) || isinterval(h2) || isinterval(h3))
	{	make_interval(*hd1);
		*hd2=nextof(*hd1);
		make_interval(*hd2);
		*hd3=nextof(*hd2);
		make_interval(*hd3);
	}
	if (error) return;
	*hd1=getvalue(*hd1); if (error) return;
	*hd2=getvalue(*hd2); if (error) return;
	*hd3=getvalue(*hd3); if (error) return;
}

void equal_params_2 (header **hd1, header **hd2)
/* Make parameter values of equal type (real, complex, interval)
*/
{	
	header *h1,*h2;
	h1=getvariablesub(*hd1); if (error) return;
	h2=getvariablesub(*hd2); if (error) return;
	if (iscomplex(h1) || iscomplex(h2))
	{	make_complex(*hd1);
		*hd2=nextof(*hd1);
		make_complex(*hd2);
	}
	else if (isinterval(h1) || isinterval(h2))
	{	make_interval(*hd1);
		*hd2=nextof(*hd1);
		make_interval(*hd2);
	}
	if (error) return;
	*hd1=getvalue(*hd1); if (error) return;
	*hd2=getvalue(*hd2); if (error) return;
}

void real_add (double *x, double *y, double *z)
{	*z=*x+*y;
}

void complex_add (double *x, double *xi, double *y, double *yi,
	double *z, double *zi)
{	*z=*x+*y;
	*zi=*xi+*yi;
}

void add (header *hd, header *hd1)
/***** add
	add the values.
*****/
{   
	header *h=hd;
	while (h && h->type==s_reference) h=referenceof(h);
	if (h && (h->type==s_string || h->type==s_ssubmatrix)) 
	{
		mhconcat(hd);
		return;
	}
	h=hd1;
	while (h && h->type==s_reference) h=referenceof(h);
	if (h && (h->type==s_string || h->type==s_ssubmatrix)) 
	{
		mhconcat(hd);
		return;
	}
	else spreadf2(real_add,complex_add,interval_add,hd,"+");
}

void real_subtract (double *x, double *y, double *z)
{	*z=*x-*y;
}

void complex_subtract (double *x, double *xi, double *y, double *yi,
	double *z, double *zi)
{	*z=*x-*y;
	*zi=*xi-*yi;
}

void subtract (header *hd, header *hd1)
/***** add
	add the values.
*****/
{   spreadf2(real_subtract,complex_subtract,interval_sub,hd,"-");
}

void real_multiply (double *x, double *y, double *z)
{	*z=*x*(*y);
}

void complex_multiply (double *x, double *xi, double *y, double *yi,
	double *z, double *zi)
{	double h=*x * *y - *xi * *yi;
	*zi=*x * *yi + *xi * *y;
	*z=h;
}

void dotmultiply (header *hd, header *hd1)
/***** add
	multiply the values.
*****/
{   
	spreadf2(real_multiply,complex_multiply,interval_mult,hd,"*");
}

void real_divide (double *x, double *y, double *z)
{
#ifdef FLOAT_TEST
	if (*y==0.0)
	{	*z=NAN; 
		if (!pnan && !error) 
		{	output("Floating point error!\n"); error=1; 
		}
		return;
	}
#endif
	*z=*x/(*y);
}

void complex_divide (double *x, double *xi, double *y, double *yi,
	double *z, double *zi)
{	double r,h;
	r=*y * *y + *yi * *yi;
#ifdef FLOAT_TEST
	if (r==0)
	{	r=NAN; 
		if (!pnan && !error) 
		{	output("Floating point error!\n"); error=1; 
		}
	}
#endif
	h=(*x * *y + *xi * *yi)/r;
	*zi=(*xi * *y - *x * *yi)/r;
	*z=h;
}

void dotdivide (header *hd, header *hd1)
/***** add
	add the values.
*****/
{   spreadf2(real_divide,complex_divide,interval_div,hd,"/");
}

void cscalp (double *s, double *si, double *x, double *xi,
	double *y, double *yi)
{	*s += *x * *y - *xi * *yi;
	*si += *x * *yi + *xi * *y;
}

void iscalp (double *s, double *si, double *x, double *xi,
	double *y, double *yi)
{	double a,b;
	interval_mult(x,xi,y,yi,&a,&b);
	interval_add(s,si,&a,&b,s,si);
}

void ccopy (double *y, double *x, double *xi)
{	*y++=*x; *y=*xi;
}

void icopy (double *y, double *x, double *xi)
{	*y++=*x; *y=*xi;
}

void multiply (header *hd, header *hd1)
/***** multiply
	matrix multiplication.
*****/
{	header *result,*st=hd;
	dims *d,*d1;
	double *m,*m1,*m2,*mm1,*mm2,x,y,null=0.0;
	int i,j,c,r,k;
	hd=getvalue(hd); hd1=getvalue(hd1);
	if (error) return;
	if (hd->type==s_matrix && hd1->type==s_matrix)
	{	
		d=dimsof(hd);
		d1=dimsof(hd1);
		if (d->c != d1->r)
		{
			print("Cannot multiply %dx%d and %dx%d matrix\n",d->r,d->c,d1->r,d1->c);
			error=1; return;
		}
		r=d->r; c=d1->c;
		result=new_matrix(r,c,"");
		if (error) return;
		m=matrixof(result);
		m1=matrixof(hd);
		m2=matrixof(hd1);
		for (i=0; i<r; i++)
			for (j=0; j<c; j++)
			{	mm1=mat(m1,d->c,i,0); mm2=m2+j;
				x=0.0;
				for (k=0; k<d->c; k++)
				{	x+=(*mm1)*(*mm2);
					mm1++; mm2+=d1->c;
				}
				*mat(m,c,i,j)=x;
			}
		moveresult(st,result);
		return;
	}
	if (hd->type==s_matrix)
	{	
		if (hd1->type==s_cmatrix)
		{	d=dimsof(hd);
			d1=dimsof(hd1);
			if (d->c != d1->r)
			{
				print("Cannot multiply %dx%d and %dx%d matrix\n",d->r,d->c,d1->r,d1->c);
				error=1; return;
			}
			r=d->r; c=d1->c;
			result=new_cmatrix(r,c,"");
			if (error) return;
			m=matrixof(result);
			m1=matrixof(hd);
			m2=matrixof(hd1);
			for (i=0; i<r; i++)
				for (j=0; j<c; j++)
				{   mm1=mat(m1,d->c,i,0); mm2=m2+(LONG)2*j;
					x=0.0; y=0.0;
					for (k=0; k<d->c; k++)
					{	cscalp(&x,&y,mm1,&null,mm2,mm2+1);
						mm1++; mm2+=2*d1->c;
					}
					ccopy(cmat(m,c,i,j),&x,&y);
				}
			moveresult(st,result);
			return;
		}
		if (hd1->type==s_imatrix)
		{	
			d=dimsof(hd);
			d1=dimsof(hd1);
			if (d->c != d1->r)
			{
				print("Cannot multiply %dx%d and %dx%d matrix\n",d->r,d->c,d1->r,d1->c);
				error=1; return;
			}
			r=d->r; c=d1->c;
			result=new_imatrix(r,c,"");
			if (error) return;
			m=matrixof(result);
			m1=matrixof(hd);
			m2=matrixof(hd1);
			for (i=0; i<r; i++)
				for (j=0; j<c; j++)
				{   mm1=mat(m1,d->c,i,0); mm2=m2+(LONG)2*j;
					x=0.0; y=0.0;
					for (k=0; k<d->c; k++)
					{	iscalp(&x,&y,mm1,mm1,mm2,mm2+1);
						mm1++; mm2+=2*d1->c;
					}
					icopy(imat(m,c,i,j),&x,&y);
				}
			moveresult(st,result);
			return;
		}
	}
	if (hd->type==s_cmatrix && hd1->type==s_matrix)
	{	
		d=dimsof(hd);
		d1=dimsof(hd1);
		if (d->c != d1->r)
		{
			print("Cannot multiply %dx%d and %dx%d matrix\n",d->r,d->c,d1->r,d1->c);
			error=1; return;
		}
		r=d->r; c=d1->c;
		result=new_cmatrix(r,c,"");
		if (error) return;
		m=matrixof(result);
		m1=matrixof(hd);
		m2=matrixof(hd1);
		for (i=0; i<r; i++)
			for (j=0; j<c; j++)
			{	mm1=cmat(m1,d->c,i,0); mm2=m2+j;
				x=0.0; y=0.0;
				for (k=0; k<d->c; k++)
				{	cscalp(&x,&y,mm1,mm1+1,mm2,&null);
					mm1+=2; mm2+=d1->c;
				}
				ccopy(cmat(m,c,i,j),&x,&y);
			}
		moveresult(st,result);
		return;
	}
	if (hd->type==s_cmatrix && hd1->type==s_cmatrix)
	{	
		d=dimsof(hd);
		d1=dimsof(hd1);
		if (d->c != d1->r)
		{
			print("Cannot multiply %dx%d and %dx%d matrix\n",d->r,d->c,d1->r,d1->c);
			error=1; return;
		}
		r=d->r; c=d1->c;
		result=new_cmatrix(r,c,"");
		if (error) return;
		m=matrixof(result);
		m1=matrixof(hd);
		m2=matrixof(hd1);
		for (i=0; i<r; i++)
			for (j=0; j<c; j++)
            {   mm1=cmat(m1,d->c,i,0); mm2=m2+(LONG)2*j;
				x=0.0; y=0.0;
				for (k=0; k<d->c; k++)
				{	cscalp(&x,&y,mm1,mm1+1,mm2,mm2+1);
					mm1+=2; mm2+=2*d1->c;
				}
				ccopy(cmat(m,c,i,j),&x,&y);
			}
		moveresult(st,result);
		return;
	}
	if (hd->type==s_imatrix)
	{	
		if (hd1->type==s_matrix)
		{	d=dimsof(hd);
			d1=dimsof(hd1);
			if (d->c != d1->r)
			{
				print("Cannot multiply %dx%d and %dx%d matrix\n",d->r,d->c,d1->r,d1->c);
				error=1; return;
			}
			r=d->r; c=d1->c;
			result=new_imatrix(r,c,"");
			if (error) return;
			m=matrixof(result);
			m1=matrixof(hd);
			m2=matrixof(hd1);
			for (i=0; i<r; i++)
				for (j=0; j<c; j++)
				{   mm1=imat(m1,d->c,i,0); mm2=m2+j;
					x=0.0; y=0.0;
					for (k=0; k<d->c; k++)
					{	iscalp(&x,&y,mm1,mm1+1,mm2,mm2);
						mm1+=2; mm2+=d1->c;
					}
					icopy(imat(m,c,i,j),&x,&y);
				}
			moveresult(st,result);
			return;
		}
		if (hd1->type==s_imatrix)
		{	d=dimsof(hd);
			d1=dimsof(hd1);
			if (d->c != d1->r)
			{
				print("Cannot multiply %dx%d and %dx%d matrix\n",d->r,d->c,d1->r,d1->c);
				error=1; return;
			}
			r=d->r; c=d1->c;
			result=new_imatrix(r,c,"");
			if (error) return;
			m=matrixof(result);
			m1=matrixof(hd);
			m2=matrixof(hd1);
			for (i=0; i<r; i++)
				for (j=0; j<c; j++)
				{   mm1=imat(m1,d->c,i,0); mm2=m2+(LONG)2*j;
					x=0.0; y=0.0;
					for (k=0; k<d->c; k++)
					{	iscalp(&x,&y,mm1,mm1+1,mm2,mm2+1);
						mm1+=2; mm2+=2*d1->c;
					}
					icopy(imat(m,c,i,j),&x,&y);
				}
			moveresult(st,result);
			return;
		}
	}
	else
	{
		if (isscalar(hd))
		{
			if (isscalar(hd1) || ((hd1->type==s_matrix || hd1->type==s_cmatrix || hd1->type==s_imatrix)
				&& dimsof(hd1)->r==1))
			{
				newram=(char *)nextof(nextof(st));
				dotmultiply(st,nextof(st));
				return;
			}
		}
		if (isscalar(hd1))
		{
			if ((hd->type==s_matrix || hd->type==s_cmatrix || hd->type==s_imatrix)
				&& dimsof(hd)->c==1)
			{
				newram=(char *)nextof(nextof(st));
				dotmultiply(st,nextof(st));
				return;
			}
		}
		print("Cannot dot multiply a %s and a %s (incompatible dimensions).\n",gettype(hd),gettype(hd1));
		error=1; return;
		// dotmultiply(st,nextof(st));
	}
}

void divide (header *hd, header *hd1)
{	dotdivide(hd,hd1);
}

void real_invert (double *x, double *y)
{	*y= -*x;
}

void complex_invert (double *x, double *xi, double *y, double *yi)
{	*y= -*x;
	*yi= -*xi;
}

void invert (header *hd)
/***** invert
	compute -matrix.
*****/
{	header *result,*st=hd;
	hd=getvalue(hd);
	if (error) return;
	if (isinterval(hd))
		result=map1i(interval_invert,hd);
	else result=map1(real_invert,complex_invert,hd);	if (!error) moveresult(st,result);
}

void copy_complex (double *x, double *y)
{	*x++=*y++;
	*x=*y;
}

