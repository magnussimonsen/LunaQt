#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <string.h>

#include "header.h"
#include "polynom.h"
#include "funcs.h"
#include "matheh.h"
#include "interval.h"
#include "express.h"
#include "stack.h"
#include "spread.h"
#include "getvalue.h"

#define max(x,y) ((x)>(y)?(x):(y))

double *polynom;
int degree;

void peval (double *x, double *r)
{	int i;
	double *p=polynom+degree,res;
	res=*p--;
	for (i=degree-1; i>=0; i--) res=res**x+(*p--);
	*r=res;
}

void cpeval (double *x, double *xi, double *z, double *zi)
{	int i;
	double *p,h,hi;
	p=polynom+(2l*degree);
	*z=*p; *zi=*(p+1);
	p-=2;
	for (i=degree-1; i>=0; i--)
	{	complex_multiply(x,xi,z,zi,&h,&hi);
		*z= h + *p;
		*zi=hi+*(p+1); p-=2;
	}
}

void polyval (header *hd)
{	
	header *st=hd,*hd1,*result;
	int r,c;
	hd1=nextof(hd);
	equal_params_2(&hd,&hd1);
	getmatrix(hd,&r,&c,&polynom);
	if (r!=1) need_arg_in("polyval","polynomial row vector, x values");
	degree=c-1;
	if (degree<0) need_arg_in("polyval","polynomial row vector, x values");
	if (isinterval(hd)) result=map1i(ipeval,hd1);
	else result=map1(peval,cpeval,hd1);
	moveresult(st,result);
}

void polyadd (header *hd)
{	header *st=hd,*hd1,*result;
	int c,c1,c2,i,r;
	double *m1,*m2,*mr;
	complex *mc1,*mc2,*mcr;
	interval *mi1,*mi2,*mir;
	hd1=next_param(st);
	equal_params_2(&hd,&hd1); if (error) return;
	getmatrix(hd,&r,&c1,&m1); if (r!=1) need_arg_in("polyadd","two row vectors");
	getmatrix(hd1,&r,&c2,&m2); if (r!=1) need_arg_in("polyadd","two row vectors");
	c=max(c1,c2);
	if (iscomplex(hd)) /* complex values */
	{	mc1=(complex *)m1; mc2=(complex *)m2;
		result=new_cmatrix(1,c,""); if (error) return;
		mcr=(complex *)matrixof(result);
		for (i=0; i<c; i++)
		{	if (i>=c1) { c_copy(*mcr,*mc2); mcr++; mc2++; }
			else if (i>=c2) { c_copy(*mcr,*mc1); mcr++; mc1++; }
			else { c_add(*mc1,*mc2,*mcr); mc1++; mc2++; mcr++; }
		}
	}
	else if (isinterval(hd))
	{	mi1=(interval *)m1; mi2=(interval *)m2;
		result=new_imatrix(1,c,""); if (error) return;
		mir=(interval *)matrixof(result);
		for (i=0; i<c; i++)
		{	if (i>=c1) { i_copy(*mir,*mi2); mir++; mi2++; }
			else if (i>=c2) { i_copy(*mir,*mi1); mir++; mi1++; }
			else { i_add(*mi1,*mi2,*mir); mi1++; mi2++; mir++; }
		}
	}
	else if (isreal(hd))
	{	result=new_matrix(1,c,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<c; i++)
		{	if (i>=c1) { *mr++ = *m2++; }
			else if (i>=c2) { *mr++ = *m1++; }
			else { *mr++ = *m1++ + *m2++; }
		}
	}
	else 
		need_arg_in("polyadd","two row vectors");
	moveresult(st,result);
}

void polymult (header *hd)
{	header *st=hd,*hd1,*result;
	int c,c1,c2,i,r,j,k;
	double *m1,*m2,*mr,x;
	complex *mc1,*mc2,*mcr,xc,hc;
	interval *mi1,*mi2,*mir,xi,hi;
	hd1=next_param(st);
	equal_params_2(&hd,&hd1); if (error) return;
	getmatrix(hd,&r,&c1,&m1); if (r!=1) need_arg_in("polymult","two row vectors");
	getmatrix(hd1,&r,&c2,&m2); if (r!=1) need_arg_in("polymult","two row vectors");
	if ((LONG)c1+c2-1>INT_MAX) overflow_in("polymult");
	c=c1+c2-1;
	if (iscomplex(hd))
	{	mc1=(complex *)m1; mc2=(complex *)m2;
		result=new_cmatrix(1,c,""); if (error) return;
		mcr=(complex *)matrixof(result);
		c_copy(xc,*mc1); mc1++;
		for (i=0; i<c2; i++) c_mult(xc,mc2[i],mcr[i]);
		for (j=1; j<c1; j++)
		{	c_copy(xc,*mc1); mc1++;
			for (k=j,i=0; i<c2-1; i++,k++)
			{	c_mult(xc,mc2[i],hc);
				c_add(hc,mcr[k],mcr[k]);
			}
			c_mult(xc,mc2[i],mcr[k]);
		}
	}
	else if (isinterval(hd))
	{	mi1=(interval *)m1; mi2=(interval *)m2;
		result=new_imatrix(1,c,""); if (error) return;
		mir=(interval *)matrixof(result);
		i_copy(xi,*mi1); mi1++;
		for (i=0; i<c2; i++) i_mult(xi,mi2[i],mir[i]);
		for (j=1; j<c1; j++)
		{	i_copy(xi,*mi1); mi1++;
			for (k=j,i=0; i<c2-1; i++,k++)
			{	i_mult(xi,mi2[i],hi);
				c_add(hi,mir[k],mir[k]);
			}
			c_mult(xi,mi2[i],mir[k]);
		}
	}
	else if (isreal(hd))
	{	result=new_matrix(1,c,""); if (error) return;
		mr=matrixof(result);
		x=*m1++;
		for (i=0; i<c2; i++) mr[i]=x*m2[i];
		for (j=1; j<c1; j++)
		{	x=*m1++;
			for (k=j,i=0; i<c2-1; i++,k++) mr[k]+=x*m2[i];
			mr[k]=x*m2[i];
		}
	}
	else need_arg_in("polymult","two row vectors");
	moveresult(st,result);
}

void polydiv (header *hd)
{	header *st=hd,*hd1,*result,*rest;
	int c1,c2,i,r,j;
	double *m1,*m2,*mr,*mh,x,l;
	complex *mc1,*mc2,*mcr,*mch,xc,lc,hc;
	interval *mi1,*mi2,*mir,*mih,xi,li,hi;
	hd1=next_param(st);
	equal_params_2(&hd,&hd1); if (error) return;
	getmatrix(hd,&r,&c1,&m1); if (r!=1) need_arg_in("polydiv","two row vectors");
	getmatrix(hd1,&r,&c2,&m2); if (r!=1) need_arg_in("polydiv","two row vectors");
	if (c1<c2)
	{	result=new_real(0.0,"");
		rest=(header *)newram;
		moveresult(rest,hd1);
	}
	else if (iscomplex(hd))
	{	mc1=(complex *)m1; mc2=(complex *)m2;
		result=new_cmatrix(1,c1-c2+1,""); if (error) return;
		mcr=(complex *)matrixof(result);
		rest=new_cmatrix(1,c2,""); if (error) return;
		mch=(complex *)newram;
		if (!freeram(c1*sizeof(complex)))
		{	output("Out of memory!\n"); error=190; return;
		}
		memmove((char *)mch,(char *)mc1,c1*sizeof(complex));
		c_copy(lc,mc2[c2-1]);
		if (lc[0]==0.0 && lc[1]==0.0) need_arg_in("polymult","nonzero divisor");
		for (i=c1-c2; i>=0; i--)
		{	c_div(mch[c2+i-1],lc,xc); c_copy(mcr[i],xc);
			for(j=0; j<c2; j++)
			{	c_mult(mc2[j],xc,hc);
				c_sub(mch[i+j],hc,mch[i+j]);
			}
		}
		memmove((char *)matrixof(rest),(char *)mch,c2*sizeof(complex));
	}
	else if (isinterval(hd))
	{	mi1=(interval *)m1; mi2=(interval *)m2;
		result=new_imatrix(1,c1-c2+1,""); if (error) return;
		mir=(interval *)matrixof(result);
		rest=new_imatrix(1,c2,""); if (error) return;
		mih=(complex *)newram;
		if (!freeram(c1*sizeof(complex)))
		{	output("Out of memory!\n"); error=190; return;
		}
		memmove((char *)mih,(char *)mi1,c1*sizeof(interval));
		i_copy(li,mi2[c2-1]);
		if (li[0]<=0.0 && li[1]>=0.0) need_arg_in("polymult","nonzeros divisor");
		for (i=c1-c2; i>=0; i--)
		{	i_div(mih[c2+i-1],li,xi); c_copy(mir[i],xi);
			for(j=0; j<c2; j++)
			{	i_mult(mi2[j],xi,hi);
				i_sub(mih[i+j],hi,mih[i+j]);
			}
		}
		memmove((char *)matrixof(rest),(char *)mih,c2*sizeof(interval));
	}
	else if (isreal(hd))
	{	result=new_matrix(1,c1-c2+1,""); if (error) return;
		mr=matrixof(result);
		rest=new_matrix(1,c2,""); if (error) return;
		mh=(double *)newram;
		if (!freeram(c1*sizeof(double)))
		{	output("Out of memory!\n"); error=190; return;
		}
		memmove((char *)mh,(char *)m1,c1*sizeof(double));
		l=m2[c2-1];
		if (l==0.0) need_arg_in("polymult","nonzeros divisor");
		for (i=c1-c2; i>=0; i--)
		{	x=mh[c2+i-1]/l; mr[i]=x;
			for(j=0; j<c2; j++) mh[i+j]-=m2[j]*x;
		}
		memmove((char *)matrixof(rest),(char *)mh,c2*sizeof(double));
	}
	else need_arg_in("polymult","two row vectors");
	moveresult(st,result);
	moveresult(nextof(st),rest);
}

void dd (header *hd)
{	header *st=hd,*hd1,*result;
	int c1,c2,i,j,r;
	double *m1,*m2,*mr;
	complex *mc1,*mc2,*mcr,hc1,hc2;
	interval *mi1,*mi2,*mir,hi1,hi2;
	hd1=next_param(st);
	equal_params_2(&hd,&hd1); if (error) return;
	getmatrix(hd,&r,&c1,&m1); if (r!=1) need_arg_in("interp","two row vectors");
	getmatrix(hd1,&r,&c2,&m2); if (r!=1) need_arg_in("interp","two row vectors");
	if (c1!=c2) need_arg_in("interp","two row vectors of same length");
	if (iscomplex(hd)) /* complex values */
	{	mc1=(complex *)m1; mc2=(complex *)m2;
		result=new_cmatrix(1,c1,""); if (error) return;
		mcr=(complex *)matrixof(result);
		memmove((char *)mcr,(char *)mc2,c1*sizeof(complex));
		for (i=1; i<c1; i++)
		{	for (j=c1-1; j>=i; j--)
			{	if (mc1[j][0]==mc1[j-i][0] &&
					mc1[j][1]==mc1[j-i][1]) 
						need_arg_in("interp","pairwise different interpolation points");
				c_sub(mcr[j],mcr[j-1],hc1);
				c_sub(mc1[j],mc1[j-i],hc2);
				c_div(hc1,hc2,mcr[j]);
			}
		}
	}
	else if (isinterval(hd)) /* complex values */
	{	mi1=(interval *)m1; mi2=(interval *)m2;
		result=new_imatrix(1,c1,""); if (error) return;
		mir=(interval *)matrixof(result);
		memmove((char *)mir,(char *)mi2,c1*sizeof(interval));
		for (i=1; i<c1; i++)
		{	for (j=c1-1; j>=i; j--)
			{	i_sub(mir[j],mir[j-1],hi1);
				i_sub(mi1[j],mi1[j-i],hi2);
				if (hi2[0]<=0 && hi2[1]>=0)
				{	output("Interval points coincide\n");
					error=1; return;
				}
				i_div(hi1,hi2,mir[j]);
			}
		}
	}
	else if (isreal(hd))
	{	result=new_matrix(1,c1,""); if (error) return;
		mr=matrixof(result);
		memmove((char *)mr,(char *)m2,c1*sizeof(double));
		for (i=1; i<c1; i++)
		{	for (j=c1-1; j>=i; j--)
			{	if (m1[j]==m1[j-i])
					need_arg_in("interp","pairwise different interpolation points");
				mr[j]=(mr[j]-mr[j-1])/(m1[j]-m1[j-i]);
			}
		}	
	}
	else need_arg_in("interp","two row vectors");
	moveresult(st,result);
}

double *divx,*divdif;

void rddeval (double *x, double *r)
{	int i;
	double *p=divdif+degree,res;
	res=*p--;
	for (i=degree-1; i>=0; i--) res=res*(*x-divx[i])+(*p--);
	*r=res;
}

void cddeval (double *x, double *xi, double *z, double *zi)
{	int i;
	double *p,h,hi,*dd,xh,xhi;
	p=divdif+(2l*degree);
	dd=divx+(2l*(degree-1));
	*z=*p; *zi=*(p+1);
	p-=2;
	for (i=degree-1; i>=0; i--)
	{	xh=*x-*dd;
		xhi=*xi-*(dd+1); dd-=2;
		complex_multiply(&xh,&xhi,z,zi,&h,&hi);
		*z= h + *p;
		*zi=hi+*(p+1); p-=2;
	}
}

void ddval (header *hd)
{	header *st=hd,*hdd,*hd1,*result;
	int r,c,cd;
	hdd=nextof(st);
	hd1=nextof(hdd);
	equal_params_3(&hd,&hdd,&hd1); if (error) return;
	getmatrix(hd,&r,&c,&divx); if (r!=1 || c<1) 
		need_arg_in("interp","row vectors xd,dd, and x");
	getmatrix(hdd,&r,&cd,&divdif); if (r!=1 || c!=cd) 
		need_arg_in("interp","row vectors xd,dd, and x");
	degree=c-1;
	if (isinterval(hd)) result=map1i(iddeval,hd1);
	else result=map1(rddeval,cddeval,hd1);
	if (error) return;
	moveresult(st,result);
}

void polyzeros (header *hd)
{	header *st=hd,*result;
	int i,j,r,c;
	double *m,*mr,x;
	complex *mc,*mcr,xc,hc;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		if (r!=1) need_arg_in("polycons","row vector");
		result=new_matrix(1,c+1,""); if (error) return;
		mr=matrixof(result);
		mr[0]=-m[0]; mr[1]=1.0;
		for (i=1; i<c; i++)
		{	x=-m[i]; mr[i+1]=1.0;
			for (j=i; j>=1; j--) mr[j]=mr[j-1]+x*mr[j];
			mr[0]*=x;
		}
	}
	else if (hd->type==s_complex || hd->type==s_cmatrix)
	{	getmatrix(hd,&r,&c,&m); mc=(complex *)m;
		if (r!=1) need_arg_in("polycons","row vector");
		result=new_cmatrix(1,c+1,""); if (error) return;
		mcr=(complex *)matrixof(result);
		mcr[0][0]=-mc[0][0]; mcr[0][1]=-mc[0][1];
		mcr[1][0]=1.0; mcr[1][1]=0.0;
		for (i=1; i<c; i++)
		{	xc[0]=-mc[i][0]; xc[1]=-mc[i][1];
			mcr[i+1][0]=1.0; mcr[i+1][1]=0.0;
			for (j=i; j>=1; j--) 
			{	c_mult(xc,mcr[j],hc);
				c_add(hc,mcr[j-1],mcr[j]);
			}
			c_mult(xc,mcr[0],mcr[0]);
		}
	}
	else need_arg_in("polycons","row vector");
	moveresult(st,result);
}

void polydd (header *hd)
{	header *st=hd,*hd1,*result;
	int c1,c2,i,j,r;
	double *m1,*m2,*mr,x;
	complex *mc1,*mc2,*mcr,hc,xc;
	hd1=next_param(st);
	equal_params_2(&hd,&hd1); if (error) return;
	getmatrix(hd,&r,&c1,&m1); if (r!=1) need_arg_in("polytrans","row vectors xd,dd");
	getmatrix(hd1,&r,&c2,&m2); if (r!=1) need_arg_in("polytrans","row vectors xd,dd");
	if (c1!=c2) need_arg_in("polytrans","row vectors xd,dd of same size");
	if (iscomplex(hd)) /* complex values */
	{	mc1=(complex *)m1; mc2=(complex *)m2;
		result=new_cmatrix(1,c1,""); if (error) return;
		mcr=(complex *)matrixof(result);
		c_copy(mcr[c1-1],mc2[c1-1]);
		for (i=c1-2; i>=0; i--)
		{	c_copy(xc,mc1[i]);
			c_mult(xc,mcr[i+1],hc);
			c_sub(mc2[i],hc,mcr[i]);
			for (j=i+1; j<c1-1; j++) 
			{	c_mult(xc,mcr[j+1],hc);
				c_sub(mcr[j],hc,mcr[j]);
			}
		}
	}
	else
	{	result=new_matrix(1,c1,""); if (error) return;
		mr=matrixof(result);
		mr[c1-1]=m2[c1-1];
		for (i=c1-2; i>=0; i--)
		{	x=m1[i];
			mr[i]=m2[i]-x*mr[i+1];
			for (j=i+1; j<c1-1; j++) mr[j]=mr[j]-x*mr[j+1];
		}
	}
	moveresult(st,result);
}

void polytrunc (header *hd)
{	header *st=hd,*result;
	double *m;
	complex *mc;
	int i;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_matrix && dimsof(hd)->r==1)
	{	m=matrixof(hd);
		for (i=dimsof(hd)->c-1; i>=0; i--)
		{	if (fabs(m[i])>epsilon) break;
		}
		if (i<0) result=new_real(0.0,"");
		else 
		{	result=new_matrix(1,i+1,"");
			memmove((char *)matrixof(result),(char *)matrixof(hd),
				(i+1)*sizeof(double));
		}
	}
	else if (hd->type==s_complex && dimsof(hd)->r==1)
	{	mc=(complex *)matrixof(hd);
		for (i=dimsof(hd)->c-1; i>=0; i--)
		{	if (fabs(mc[i][0])>epsilon && fabs(mc[i][1])>epsilon) 
				break;
		}
		if (i<0) result=new_complex(0.0,0.0,"");
		else 
		{	result=new_cmatrix(1,i+1,"");
			memmove((char *)matrixof(result),(char *)matrixof(hd),
				(i+1)*sizeof(complex));
		}
	}
	else need_arg_in("polytrunc","row vector");
	moveresult(st,result);
}

void mzeros (header *hd)
{	header *st=hd,*result;
	int r,c;
	double *m;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_matrix) 
	{	make_complex(st); if (error) return;
		hd=getvalue(st); if (error) return; 
	}
	if (hd->type!=s_cmatrix || dimsof(hd)->r!=1 || dimsof(hd)->c<2)
	{	output("Need a complex polynomial\n"); error=300; return; }
	getmatrix(hd,&r,&c,&m);
	result=new_cmatrix(1,c-1,""); if (error) return;
	bauhuber(m,c-1,matrixof(result),1,0.0,0.0);
	moveresult(st,result);
}

void mzeros1 (header *hd)
{	
	header *st=hd,*hd1,*result;
	int r,c;
	double *m,xr,xi;
	hd1=nextof(hd);
	hd1=getvalue(hd1); if (error) return;
	if (hd1->type==s_real)
	{	xr=*realof(hd1); xi=0;
	}
	else if (hd1->type==s_complex)
	{	xr=*realof(hd1); xi=*(realof(hd1)+1);
	}
	else
	{	output("Need a starting value!\n"); error=300; return; }
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_matrix)
	{	make_complex(st); if (error) return;
		hd=getvalue(st); if (error) return;
	}
	if (hd->type!=s_cmatrix || dimsof(hd)->r!=1 || dimsof(hd)->c<2)
	{	output("Need a complex polynomial\n"); error=300; return; }
	getmatrix(hd,&r,&c,&m);
	result=new_complex(0,0,""); if (error) return;
	bauhuber(m,c-1,realof(result),0,xr,xi);
	moveresult(st,result);
}
