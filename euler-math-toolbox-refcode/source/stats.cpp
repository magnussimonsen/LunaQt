#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>

#include "header.h"
#include "stack.h"
#include "spread.h"
#include "getvalue.h"
#include "sysdep.h"
#include "stats.h"

#include "AlgLib/src/statistics.h"

void mquantile (header *hd)
{
	header *st=hd,*hdq,*result;
	hdq=nextof(hd);
	hd=getvalue(hd); if (error) return;
	hdq=getvalue(hdq); if (error) return;
	if (hd->type!=s_matrix || hdq->type!=s_real)
		need_arg_in("quantile","real vector and real quantile")
	double *m,q;
	int r,c;
	getmatrix(hd,&r,&c,&m);
	q=*realof(hdq);
	if (r!=1 || q<0 || q>1)
		need_arg_in("quantile","1xn vector and quantile 0<=q<=1");
	try
	{
		alglib::real_1d_array v;
		v.setcontent(c,m);
		double res;
		alglib::samplepercentile(v,q,res);
		result=new_real(res,"");
	}
	catch (alglib::ap_error)
	{
		print("AlgLib quantile could not evaluate quantile %g!\n",q);
		error=1; return;
	}
	moveresult(st,result);
}

double bindis (double **x, int argn, void *pp, char *name)
{
	double p=*x[2];
	int k=(int)*x[0];
	int n=(int)*x[1];
	if (n<0 || k<0 || k>n || p<0 || p>1)
	{
		print("Illegal parameter in bindis(%d,%d,%g).\n",k,n,p);
		error=1; return 0;
	}
	try
	{
		return alglib::binomialdistribution(k,n,p);
	}
	catch (alglib::ap_error)
	{
		print("AlgLib bindis could not evaluate %d out of %d with p=%g!\n",k,n,p);
		error=1; return 0;
	}
}

void mbindis (header *hd)
{
	spreadrn(hd,3,bindis,0,"bindis");
}

double bincdis (double **x, int argn, void *pp, char *name)
{
	double p=*x[2];
	int k=(int)*x[0];
	int n=(int)*x[1];
	if (n<0 || k<0 || k>n || p<0 || p>1)
	{
		print("Illegal parameter in bindis(%d,%d,%g).\n",k,n,p);
		error=1; return 0;
	}
	try
	{
		return alglib::binomialcdistribution(k,n,p);
	}
	catch (alglib::ap_error)
	{
		print("AlgLib bindis could not evaluate %d out of %d with p=%g!\n",k,n,p);
		error=1; return 0;
	}
}

void mbincdis (header *hd)
{
	spreadrn(hd,3,bincdis,0,"bindis");
}

double invpbindis (double **x, int argn, void *pp, char *name)
{
	double p=*x[2];
	int k=(int)*x[0];
	int n=(int)*x[1];
	if (n<0 || k<0 || k>n || p<0 || p>1)
	{
		print("Illegal parameter in bindis(%d,%d,%g).\n",k,n,p);
		error=1; return 0;
	}
	try
	{
		return alglib::invbinomialdistribution(k,n,p);
	}
	catch (alglib::ap_error)
	{
		print("AlgLib invpbindis could not evaluate %d out of %d with p=%g!\n",k,n,p);
		error=1; return 0;
	}
}

void minvpbindis (header *hd)
{
	spreadrn(hd,3,invpbindis,0,"invpbindis");
}
