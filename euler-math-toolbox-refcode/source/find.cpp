#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <stdlib.h>

#include "header.h"
#include "funcs.h"
#include "find.h"
#include "matheh.h"
#include "interval.h"
#include "stack.h"
#include "sysdep.h"
#include "getvalue.h"

#include "AlgLib\src\ap.h"
#include "AlgLib\src\fasttransforms.h"

extern int zerorounding;

void minmax (double *x, LONG n, double *min, double *max, 
	int *imin, int *imax)
/***** minmax
	compute the total minimum and maximum of n double numbers.
*****/
{	LONG i;
	if (n==0)
	{	*min=0; *max=0; *imin=0; *imax=0; return; }
	*min=*x; *max=*x; 
	*imin=0; *imax=0; x++;
	for (i=1; i<n; i++)
	{	if (!isnan(*x))
		{	if (isnan(*min)) { *min=*x; *max=*x; *imin=(int)i; *imax=(int)i; }
			else if (*x<*min) { *min=*x; *imin=(int)i; }
			else if (*x>*max) { *max=*x; *imax=(int)i; }
		}
		x++;
	}
}

extern void mcpxtranspose (header *hd);

void transpose (header *hd)
/***** transpose 
	transpose a matrix
*****/
{	header *hd1,*st=hd;
	double *m,*m1,*mh;
	int c,r,i,j;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		hd1=new_matrix(c,r,""); if (error) return;
		m1=matrixof(hd1);
		for (i=0; i<r; i++)
		{	mh=m1+i;
			for (j=0; j<c; j++)
			{	*mh=*m++; mh+=r;
			}
		}
	}
	else if (hd->type==s_cmatrix)
	{	getmatrix(hd,&r,&c,&m);
		hd1=new_cmatrix(c,r,""); if (error) return;
		m1=matrixof(hd1);
		for (i=0; i<r; i++)
		{   mh=m1+(LONG)2*i;
			for (j=0; j<c; j++)
			{	*mh=*m++; *(mh+1)=*m++;
                mh+=(LONG)2*r;
			}
		}
	}
	else if (hd->type==s_imatrix)
	{	getmatrix(hd,&r,&c,&m);
		hd1=new_imatrix(c,r,""); if (error) return;
		m1=matrixof(hd1);
		for (i=0; i<r; i++)
		{   mh=m1+(LONG)2*i;
			for (j=0; j<c; j++)
			{	*mh=*m++; *(mh+1)=*m++;
				mh+=(LONG)2*r;
			}
		}
	}
	else if (hd->type==s_cpxmatrix)
	{	mcpxtranspose(st);
		return;
	}
	else if (hd->type==s_real || hd->type==s_complex
		|| hd->type==s_interval)
	{	hd1=hd;
	}
	else
	{	error=24; output("\' not defined for this value!\n");
		return;
	}
	moveresult(st,hd1);
}

int real_compare (const double *x, const double *y)
{
	if (*x<=*y) return -1;
	else if (*x==*y) return 0;
	else return 1;
}

void mrdrop (header *hd)
{
	header *st=hd,*hd1,*result;
	hd1=nextof(hd);
	hd=getvalue(hd); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	if (!(hd1->type==s_matrix || hd1->type==s_real)
		|| !(hd->type == s_matrix || hd->type==s_cmatrix || hd->type==s_imatrix || 
				hd->type == s_real || hd->type == s_complex || hd->type == s_interval))
		need_arg_in("drop","real row vector and index vector");
	double *m,*m1;
	int r,c,r1,c1;
	getmatrix(hd,&r,&c,&m);
	getmatrix(hd1,&r1,&c1,&m1);
	if (r1!=1 || r!=1) need_arg_in("drop", "real row vector and index vector");
	int sorted=0;
	// Fix negative indices and round indices:
	for (int i=0; i<c1; i++)
	{
		if (m1[i]<-0.5) m1[i]+=c+1;
		m1[i] = (int)(m1[i] + 0.5);
	}
	restart:
	int count=0;
	for (int i=0; i<c1; i++)
	{
		if (i>0 && m1[i]<m1[i-1])
		{
			if (!sorted)
			{
				qsort(m1,c1,sizeof(double),
					(int (*) (const void *, const void *))real_compare);
				sorted=1;
				goto restart;
			}
			else 
				need_arg_in("drop","sorted index vector");
		}
		if (m1[i]>=1 && m1[i]<=c)
		{
			if (!(i>0 && m1[i]==m1[i-1])) count++;
		}
	}
	if (hd->type == s_real || hd->type == s_matrix)
	{
		result = new_matrix(1, c - count, "");
		if (error) return;
		double* mr = matrixof(result);
		int k = 0;
		while (k < c1 && m1[k] < 1) k++;
		for (int i = 0; i < c; i++)
		{
			if (k < c1 && m1[k] == i + 1)
			{
				while (k < c1 && m1[k] == i + 1) k++;
				m++;
				continue;
			}
			*mr++ = *m++;
		}
		moveresult(st, result);
	}
	else
	{
		if (hd->type==s_cmatrix || hd->type==s_complex) result = new_cmatrix(1, c - count, "");
		else result = new_imatrix(1, c - count, "");
		if (error) return;
		double* mr = matrixof(result);
		int k = 0;
		while (k < c1 && m1[k] < 1) k++;
		for (int i = 0; i < c; i++)
		{
			if (k < c1 && m1[k] == i + 1)
			{
				while (k < c1 && m1[k] == i + 1) k++;
				m++; m++;
				continue;
			}
			*mr++ = *m++;
			*mr++ = *m++;
		}
		moveresult(st, result);
	}
}

void vectorize (header *init, header *step, header *end)
{	double vinit,vstep,vend,*m;
	int count;
	header *result,*st=init;
	init=getvalue(init); step=getvalue(step); end=getvalue(end);
	if (error) return;
	if (init->type!=s_real || step->type!=s_real || end->type!=s_real)
	{	output("The : allows only real arguments!\n");
		error=15; return;
	}
	vinit=*realof(init); vstep=*realof(step); vend=*realof(end);
	if (isnan(vinit) || isnan(vstep) || isnan(vend))
	{
		output("Illegal value NAN in : operator.\n");
		error=1; return;
	}
	if (vstep==0)
	{	output("A step size of 0 is not allowed in : \n");
		error=401; return;
	}
	if (fabs(vend-vinit)/fabs(vstep)+1+epsilon>INT_MAX)
	{	output1("A vector can only have %d elements\n",INT_MAX);
		error=402; return;
	}
	count=(int)(floor(fabs(vend-vinit)/fabs(vstep)+1+epsilon));
	if ((vend>vinit && vstep<0) || (vend<vinit && vstep>0))
		count=0;
	result=new_matrix(1,count,""); if (error) return;
	m=matrixof(result);
	while (count>=0)
	{	*m++=vinit;
		vinit+=vstep;
		count--;
	}
	moveresult(st,result);
}

void mfft (header *hd)
{	header *st=hd,*result;
	double *m,*mr;
	unsigned long nn[2];
	int r,c,i;
    long l;
	hd=getvalue(hd);
	if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	make_complex(st); hd=st; }
	if (hd->type!=s_cmatrix && hd->type!=s_complex)
		need_arg_in("fft","complex matrix");
	getmatrix(hd,&r,&c,&m);
	result=new_cmatrix(r,c,""); if (error) return;
	mr=matrixof(result);
	memmove((char *)mr,(char *)m,(LONG)2*r*c*sizeof(double));
	if (r==1)
	{	fft(mr,c,1);
	}
	else
	{   nn[0]=r; nn[1]=c;
		for (l=1,i=0; i<20; i++,l*=2)
			if (l==r) goto cont1;
		goto err;
		cont1 :
		for (l=1,i=0; i<20; i++,l*=2)
			if (l==c) goto cont;
		err :
		output("fft columns must be a power of 2\n");
		error=1; return;
		cont :
		fftn(mr-1,nn,2,1);
		for (i=0; i<r*c; i++) mr[2*i+1]=-mr[2*i+1];
	}
	if (error) return;
	moveresult(st,result);
}

void mifft (header *hd)
{	header *st=hd,*result;
	double *m,*mr,f;
	unsigned long nn[2],l;
	int r,c,i;
	hd=getvalue(hd);
	if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	make_complex(st); hd=st; }
	if (hd->type!=s_cmatrix && hd->type!=s_complex)
		need_arg_in("ifft","complex matrix");
	getmatrix(hd,&r,&c,&m);
	result=new_cmatrix(r,c,""); if (error) return;
	mr=matrixof(result);
	memmove((char *)mr,(char *)m,(LONG)2*r*c*sizeof(double));
	if (r==1)
	{	fft(mr,c,-1);
	}
	else
	{   nn[0]=r; nn[1]=c;
		for (l=1,i=0; i<20; i++,l*=2)
			if ((int)l==r) goto cont1;
		goto err;
        cont1 :
		for (l=1,i=0; i<20; i++,l*=2)
			if ((int)l==c) goto cont;
		err :
		output("ifft columns must be a power of 2\n");
		error=1; return;
		cont :
		fftn(mr-1,nn,2,1);
		f=(double)c*r;
		for (i=0; i<r*c; i++) 
		{
			mr[2*i]=mr[2*i]/f;
			mr[2*i+1]=-mr[2*i+1]/f;
		}
	}
	if (error) return;
	moveresult(st,result);
}

void mtridiag (header *hd)
{	header *result,*st=hd,*result1;
	double *m,*mr;
	int r,c,*rows,i;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_matrix)
	{	getmatrix(hd,&c,&r,&m);
		if (c!=r || c==0) need_arg_in("hb","real or complex matrix");
		result=new_matrix(c,c,""); if (error) return;
		result1=new_matrix(1,c,""); if (error) return;
		mr=matrixof(result);
		memmove(mr,m,(LONG)c*c*sizeof(double));
		tridiag(mr,c,&rows);
		mr=matrixof(result1);
		for (i=0; i<c; i++) *mr++=rows[i]+1;
	}
	else if (hd->type==s_cmatrix)
	{	getmatrix(hd,&c,&r,&m);
		if (c!=r || c==0) need_arg_in("hb","real or complex matrix");
		result=new_cmatrix(c,c,""); if (error) return;
		result1=new_matrix(1,c,""); if (error) return;
		mr=matrixof(result);
        memmove(mr,m,(LONG)c*c*(LONG)2*sizeof(double));
		ctridiag(mr,c,&rows);
		mr=matrixof(result1);
		for (i=0; i<c; i++) *mr++=rows[i]+1;
	}
	else need_arg_in("hb","real or complex matrix");
	moveresult(st,result);
	moveresult((header *)newram,result1);
}

void mcharpoly (header *hd)
{	header *result,*st=hd,*result1;
	double *m,*mr;
	int r,c;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_matrix)
	{	getmatrix(hd,&c,&r,&m);
		if (c!=r || c==0) need_arg_in("charpoly","real or complex square matrix");
		result=new_matrix(c,c,""); if (error) return;
		result1=new_matrix(1,c+1,""); if (error) return;
		mr=matrixof(result);
		memmove(mr,m,(LONG)c*c*sizeof(double));
		charpoly(mr,c,matrixof(result1));
	}
	else if (hd->type==s_cmatrix)
	{	getmatrix(hd,&c,&r,&m);
		if (c!=r || c==0) need_arg_in("charpoly","real or complex square matrix");
		result=new_cmatrix(c,c,""); if (error) return;
		result1=new_cmatrix(1,c+1,""); if (error) return;
		mr=matrixof(result);
        memmove(mr,m,(LONG)c*c*(LONG)2*sizeof(double));
		ccharpoly(mr,c,matrixof(result1));
	}
	else need_arg_in("charpoly","real or complex square matrix");
	moveresult(st,result1);
}

void mscompare (header *hd)
{	header *st=hd,*hd1,*result;
	hd=getvalue(hd);
	hd1=getvalue(nextof(st));
	if (error) return;
	if (hd->type==s_string && hd1->type==s_string)
	{	result=new_real(strcmp(stringof(hd),stringof(hd1)),"");
		if (error) return;
	}
	else need_arg_in("stringcompare","two strings");
	moveresult(st,result);
}

/**
multofsorted(v,x)
count how often x is in the sorted vector v.
works for numbers and strings.
*/
void mmultofsorted (header *hd)
{	
	header *st=hd,*hd1,*result;
	double *m,*m1,*mr;
	int i,j,k,c,r,c1,r1;
	hd=getvalue(hd);
	hd1=getvalue(nextof(st));
	if (error) return;
	if (hd->type==s_smatrix) // strings
	{
		if (hd1->type==s_smatrix || hd1->type==s_string)
		{
			char *p=stringof(hd);
			char **ind=(char **)newram;
			int pn=0;
			while(true)
			{
				ind[pn++]=p;
				if ((char *)(ind+pn)>ramend) outofram();
				p=p+strlen(p)+1;
				if (*p==1) break;
			}
			newram+=pn*sizeof(char *);
			result=new_matrix(1,1,"");
			double *mres=matrixof(result);
			char *q=stringof(hd1);
			int n=0;
			while (true)
			{
				if (strcmp(q,ind[0])<0) k=0;
				else if (strcmp(q,ind[pn-1])>0) k=0;
				else
				{
					int i1=0,i2=pn-1;
					while (true)
					{
						if (i1==i2) { k=i1+1; break; }
						int i3=(i1+i2)/2;
						if (i3==i1) 
						{
							if (strcmp(q,ind[i2])>=0) k=i2+1;
							else k=i1+1;
							break;
						}
						if (strcmp(q,ind[i3])<0) i2=i3;
						else i1=i3;
					}
					if (strcmp(q,ind[k-1])) k=0;
					else
					{
						int count=1;
						while (k>=2 && strcmp(q,ind[k-2])==0) { k--; count++; }
						k=count;
					}
				}
				*mres=k;
				n++;
				q=q+strlen(q)+1;
				mres++;
				if ((char *)mres>=ramend) outofram();
				newram=(char *)mres;
				if (hd1->type==s_string || *q==1) break;
			}
			dimsof(result)->c=n;
			result->size=newram-(char *)result;
		}
		else
			need_arg_in("multofsorted","string or string vector");
	}
	else if (hd->type==s_real || hd->type==s_matrix) // numbers
	{
		if ((hd1->type!=s_matrix && hd1->type!=s_real)) 
				need_arg_in("find","real row vector");
		getmatrix(hd,&c,&r,&m);
		getmatrix(hd1,&c1,&r1,&m1);
		if (c!=1 && r!=1)
			need_arg_in("multofsorted","row vector");
		if (r!=1) c=r;
		result=new_matrix(c1,r1,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r1; i++)
			for (j=0; j<c1; j++)
			{	
				k=0;
				if (isnan(*m1)) k=isnan(m[c-1])?c:0;
				else if (*m1<m[0]) k=0;
				else if (*m1>m[c-1]) k=0;
				else
				{
					int i1=0,i2=c-1;
					while (true)
					{
						if (i1==i2) { k=i1+1; break; }
						int i3=(i1+i2)/2;
						if (i3==i1) { k=(*m1>=m[i2])?i2+1:i1+1; break; }
						if (*m1<m[i3]) i2=i3;
						else i1=i3;
					}
					if (*m1!=m[k-1]) k=0;
					else
					{
						int count=1;
						while (k>=2 && *m1==m[k-2]) { k--; count++; }
						k=count;
					}
				}
				*mr++=k; m1++;
			}
	}
	else
		need_arg_in("multofsorted","sorted vector, vector");
	moveresult(st,result);
}

void mindexofsorted (header *hd)
{	
	header *st=hd,*hd1,*result;
	double *m,*m1,*mr;
	int i,j,k,c,r,c1,r1;
	hd=getvalue(hd);
	hd1=getvalue(nextof(st));
	if (error) return;
	if (hd->type==s_smatrix)
	{
		if (hd1->type==s_smatrix || hd1->type==s_string)
		{
			char *p=stringof(hd);
			char **ind=(char **)newram;
			int pn=0;
			while(true)
			{
				ind[pn++]=p;
				if ((char *)(ind+pn)>ramend) outofram();
				p=p+strlen(p)+1;
				if (*p==1) break;
			}
			newram+=pn*sizeof(char *);
			result=new_matrix(1,1,"");
			double *mres=matrixof(result);
			char *q=stringof(hd1);
			int n=0;
			while (true)
			{
				if (strcmp(q,ind[0])<0) k=0;
				else if (strcmp(q,ind[pn-1])>0) k=0;
				else
				{
					int i1=0,i2=pn-1;
					while (true)
					{
						if (i1==i2) { k=i1+1; break; }
						int i3=(i1+i2)/2;
						if (i3==i1) 
						{
							if (strcmp(q,ind[i2])>=0) k=i2+1;
							else k=i1+1;
							break;
						}
						if (strcmp(q,ind[i3])<0) i2=i3;
						else i1=i3;
					}
					if (k>0 && strcmp(q,ind[k-1])) k=0;
				}
				*mres=k;
				n++;
				q=q+strlen(q)+1;
				mres++;
				if ((char *)mres>=ramend) outofram();
				newram=(char *)mres;
				if (hd1->type==s_string || *q==1) break;
			}
			dimsof(result)->c=n;
			result->size=newram-(char *)result;
		}
		else
			need_arg_in("indexofsorted","string or string vector");
	}
	else if (hd->type==s_real || hd->type==s_matrix)
	{
		if ((hd1->type!=s_matrix && hd1->type!=s_real)) 
				need_arg_in("find","real row vector");
		getmatrix(hd,&c,&r,&m);
		getmatrix(hd1,&c1,&r1,&m1);
		if (c!=1 && r!=1)
			need_arg_in("indexofsorted","row vector");
		if (r!=1) c=r;
		result=new_matrix(c1,r1,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r1; i++)
			for (j=0; j<c1; j++)
			{	
				k=0;
				if (isnan(*m1)) k=isnan(m[c-1])?c:0;
				else if (*m1<m[0]) k=0;
				else if (*m1>m[c-1]) k=0;
				else
				{
					int i1=0,i2=c-1;
					while (true)
					{
						if (i1==i2) { k=i1+1; break; }
						int i3=(i1+i2)/2;
						if (i3==i1) { k=(*m1>=m[i2])?i2+1:i1+1; break; }
						if (*m1<m[i3]) i2=i3;
						else i1=i3;
					}
					if (*m1!=m[k-1]) k=0;
				}
				*mr++=k; m1++;
			}
	}
	else
		need_arg_in("indexofsorted","sorted vector, vector");
	moveresult(st,result);
}

void mindexof (header *hd)
{	
	header *st=hd,*hd1,*result;
	double *m,*m1,*mr;
	int i,j,k,c,r,c1,r1;
	hd=getvalue(hd);
	hd1=getvalue(nextof(st));
	if (error) return;
	if (hd->type==s_smatrix)
	{
		if (hd1->type==s_string)
		{
			result=new_real(0,"");
			char *p=stringof(hd);
			char *q=stringof(hd1);
			c=0;
			while (true)
			{
				int comp=strcmp(p,q);
				c++;
				if (comp==0) { *realof(result)=c; break; }
				p=p+strlen(p)+1;
				if (*p==1) { *realof(result)=0; break; }
			}
		}
		else if (hd1->type==s_smatrix)
		{
			result=new_matrix(1,1,"");
			double *mres=matrixof(result);
			char *q=stringof(hd1);
			int n=0;
			while (true)
			{
				char *p=stringof(hd);
				c=0;
				while (true)
				{
					int comp=strcmp(p,q);
					c++;
					if (comp==0) { *mres=c; break; }
					p=p+strlen(p)+1;
					if (*p==1) { *mres=0; break; }
				}
				n++;
				q=q+strlen(q)+1;
				mres++;
				if ((char *)mres>=ramend) outofram();
				newram=(char *)mres;
				if (*q==1) break;
			}
			dimsof(result)->c=n;
			result->size=newram-(char *)result;
		}
		else
			need_arg_in("indexof","scalar or vector");
	}
	else if (hd->type==s_matrix ||hd->type==s_real)
	{
		if ((hd1->type!=s_matrix && hd1->type!=s_real)) 
				need_arg_in("indexof","two row vectors");
		getmatrix(hd,&c,&r,&m);
		getmatrix(hd1,&c1,&r1,&m1);
		if (c!=1 && r!=1)
			need_arg_in("indexof","two row vectors");
		if (r!=1) c=r;
		result=new_matrix(c1,r1,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r1; i++)
			for (j=0; j<c1; j++)
			{	
				*mr=0;
				for (k=0; k<c; k++)
				{
					if ((isnan(*m1) && isnan(m[k])) || 
						*m1 == m[k]) { *mr=k+1; break; }
				}
				mr++;
				m1++;
			}
	}
	else
		need_arg_in("indexof","vector, vector");
	moveresult(st,result);
}

void mfind (header *hd)
{	
	header *st=hd,*hd1,*result;
	double *m,*m1,*mr;
	int i,j,k,c,r,c1,r1;
	hd=getvalue(hd);
	hd1=getvalue(nextof(st));
	if (error) return;
	if (hd->type==s_smatrix)
	{
		if (hd1->type==s_string)
		{
			result=new_real(0,"");
			char *p=stringof(hd);
			char *q=stringof(hd1);
			c=0;
			while (true)
			{
				int comp=strcmp(p,q);
				if (comp>0) { *realof(result)=c; break; }
				c++;
				if (comp==0) { *realof(result)=c; break; }
				p=p+strlen(p)+1;
				if (*p==1) { *realof(result)=c; break; }
			}
		}
		else if (hd1->type==s_smatrix)
		{
			char *p=stringof(hd);
			char **ind=(char **)newram;
			int pn=0;
			while(true)
			{
				ind[pn++]=p;
				if ((char *)(ind+pn)>ramend) outofram();
				p=p+strlen(p)+1;
				if (*p==1) break;
			}
			newram+=pn*sizeof(int);
			result=new_matrix(1,1,"");
			double *mres=matrixof(result);
			char *q=stringof(hd1);
			int n=0;
			while (true)
			{
				if (strcmp(q,ind[0])<0) k=0;
				else if (strcmp(q,ind[pn-1])>=0) k=pn;
				else
				{
					int i1=0,i2=pn-1;
					while (true)
					{
						if (i1==i2) { k=i1+1; break; }
						int i3=(i1+i2)/2;
						if (i3==i1) 
						{
							if (strcmp(q,ind[i2])>=0) k=i2+1;
							else k=i1+1;
							break;
						}
						if (strcmp(q,ind[i3])<0) i2=i3;
						else i1=i3;
					}
					while (true)
					{
						if (k-1<pn-1 && strcmp(q,ind[k])>=0) k++;
						else break;
					}
				}
				*mres=k;
				n++;
				q=q+strlen(q)+1;
				mres++;
				if ((char *)mres>=ramend) outofram();
				newram=(char *)mres;
				if (*q==1) break;
			}
			dimsof(result)->c=n;
			result->size=newram-(char *)result;
		}
		else
			need_arg_in("find","sorted vector, scalar or vector");
	}
	else
	{
		if ((hd->type!=s_matrix && hd->type!=s_real) || 
			(hd1->type!=s_matrix && hd1->type!=s_real)) 
				need_arg_in("find","sorted row vector, real row vector");
		getmatrix(hd,&c,&r,&m);
		getmatrix(hd1,&c1,&r1,&m1);
		if (c!=1 && r!=1)
			need_arg_in("find","sorted vector, scalar or vector");
		if (r!=1) c=r;
		result=new_matrix(c1,r1,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r1; i++)
			for (j=0; j<c1; j++)
			{	
				k=0;
				if (*m1<m[0]) k=0;
				else if (*m1>m[c-1]) k=c;
				else
				{
					int i1=0,i2=c-1;
					while (true)
					{
						if (i1==i2) { k=i1+1; break; }
						int i3=(i1+i2)/2;
						if (i3==i1) { k=(*m1>=m[i2])?i2+1:i1+1; break; }
						if (*m1<m[i3]) i2=i3;
						else i1=i3;
					}
					while (true)
					{
						if (k<c-1 && *m1>=m[k+1]) k++;
						else break;
					}
				}
				*mr++=k; m1++;
			}
	}
	moveresult(st,result);
}

void mdiag2 (header *hd)
{	
	header *st=hd,*hd1,*result;
	int c,r,i,n,l;
	double *m,*mh,*mr;
	hd1=next_param(hd);
	hd=getvalue(hd); 
	if (hd1) hd1=getvalue(hd1);
	else hd1=new_real(0,"");
	if (error) return;
	if (hd1->type!=s_real)
		need_arg_in("diag","matrix and index of diagonal");
	n=(int)*realof(hd1);
	if (hd->type==s_matrix || hd->type==s_real)
	{	getmatrix(hd,&r,&c,&m);
		result=new_matrix(1,r,""); if (error) return;
		mr=matrixof(result); l=0;
		for (i=0; i<r; i++)
		{	if (i+n>=c) break;
			if (i+n>=0) { l++; *mr++=*mat(m,c,i,i+n); }
		}
		dimsof(result)->c=l;
		result->size=matrixsize(1,c);
	}
	else if (hd->type==s_cmatrix || hd->type==s_complex)
	{	getmatrix(hd,&r,&c,&m);
		result=new_cmatrix(1,r,""); if (error) return;
		mr=matrixof(result); l=0;
		for (i=0; i<r; i++)
		{	if (i+n>=c) break;
			if (i+n>=0) 
			{	l++;
				mh=cmat(m,c,i,i+n);
				*mr++=*mh++;
				*mr++=*mh;
			}
		}
		dimsof(result)->c=l;
		result->size=cmatrixsize(1,c);
	}
	else
		need_arg_in("diag","matrix and index of diagonal");
	moveresult(st,result);
	if (usematlab) transpose(st);
}

void mband (header *hd)
{	header *st=hd,*hd1,*hd2,*result;
	int i,j,c,r,n1,n2;
	double *m,*mr;
	hd1=next_param(hd);
	hd2=next_param(hd1);
	hd=getvalue(hd); hd1=getvalue(hd1); hd2=getvalue(hd2);
	if (error) return;
	if (hd1->type!=s_real || hd2->type!=s_real)
		need_arg_in("band","matrix and indeces of diagonals");
	n1=(int)*realof(hd1); n2=(int)*realof(hd2);
	if (hd->type==s_matrix || hd->type==s_real)
	{	getmatrix(hd,&r,&c,&m);
		result=new_matrix(r,c,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
			for (j=0; j<c; j++)
			{	if (j-i>=n1 && j-i<=n2) *mr++=*m++;
				else { *mr++=0.0; m++; }
			}
	}
	else if (hd->type==s_cmatrix || hd->type==s_complex)
	{	getmatrix(hd,&r,&c,&m);
		result=new_cmatrix(r,c,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
			for (j=0; j<c; j++)
			{	if (j-i>=n1 && j-i<=n2) { *mr++=*m++; *mr++=*m++; }
				else { *mr++=0.0; *mr++=0.0; m+=2; }
			}
	}
	else
		need_arg_in("band","matrix and indeces of diagonals");
	moveresult(st,result);
}

void morthogonal (header *hd)
{	header *st=hd,*result=0,*res1;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix && hd->type!=s_cmatrix)
		need_arg_in("orthogonal","real matrix");
	int r,c;
	double *m,*mr,*m1;
	getmatrix(hd,&r,&c,&m);
	if (hd->type==s_matrix)
	{	result=new_matrix(r,c,""); if (error) return;
		mr=matrixof(result);
		res1=new_matrix(1,c,""); if (error) return;
		m1=matrixof(res1);
		int count=0;
		for (int j=0; j<c; j++)
		{	if (count>=r)
			{	for (int i=0; i<r; i++) mr[i*c+j]=0;
				m1[j]=0; 
				continue;
			}
			for (int i=0; i<r; i++) mr[i*c+j]=m[i*c+j];
			for (int j1=0; j1<j; j1++)
			{	if (m1[j1]==0.0) continue;
				double sp=0;
				for (int i=0; i<r; i++) sp+=mr[i*c+j1]*m[i*c+j];
				for (int i=0; i<r; i++) mr[i*c+j]-=sp*mr[i*c+j1];
			}
			double norm=0;
			for (int i=0; i<r; i++) norm+=mr[i*c+j]*mr[i*c+j];
			norm=sqrt(norm);
			if (fabs(norm)>epsilon)
			{	for (int i=0; i<r; i++) mr[i*c+j]/=norm;
				m1[j]=1.0;
				count++;
			}
			else m1[j]=0.0;
		}
	}
	else if (hd->type==s_cmatrix)
	{	result=new_cmatrix(r,c,""); if (error) return;
		mr=matrixof(result);
		res1=new_matrix(1,c,""); if (error) return;
		m1=matrixof(res1);
		int count=0;
		for (int j=0; j<c; j++)
		{	if (count>=r)
			{	for (int i=0; i<r; i++) 
				{	int k=2*(i*c+j);
					mr[k]=0; mr[k+1]=0;
				}
				m1[j]=0; 
				continue;
			}
			for (int i=0; i<r; i++)
			{	int k=2*(i*c+j);
				mr[k]=m[k];
				mr[k+1]=m[k+1];
			}
			for (int j1=0; j1<j; j1++)
			{	if (m1[j1]==0.0) continue;
				double sp=0,spi=0;
				for (int i=0; i<r; i++)
				{	int k=2*(i*c+j),k1=2*(i*c+j1);
					sp+=m[k]*mr[k1]+m[k+1]*mr[k1+1];
					spi+=mr[k1]*m[k+1]-mr[k1+1]*m[k];
				}
				for (int i=0; i<r; i++) 
				{	int k=2*(i*c+j),k1=2*(i*c+j1);
					mr[k]-=sp*mr[k1]-spi*mr[k1+1];
					mr[k+1]-=sp*mr[k1+1]+spi*mr[k1];
				}
			}
			double norm=0;
			for (int i=0; i<r; i++) 
			{	int k=2*(i*c+j);
				norm+=mr[k]*mr[k]+mr[k+1]*mr[k+1];
			}
			norm=sqrt(norm);
			if (fabs(norm)>epsilon)
			{	for (int i=0; i<r; i++) 
				{	int k=2*(i*c+j);
					mr[k]/=norm; mr[k+1]/=norm;
				}
				m1[j]=1.0;
				count++;
			}
			else m1[j]=0.0;
		}
	}
	moveresult1(st,result);
}

void mdup (header *hd)
{	header *result,*st=hd,*hd1;
	double x,*m,*m1,*m2;
	int c,i,n,j,r;
	hd=getvalue(hd);
	hd1=next_param(st);
	if (!hd1)
		need_arg_in("dup","vector and integer");
	hd1=getvalue(hd1); if (error) return;
	if (hd1->type!=s_real)
		need_arg_in("dup","vector and integer");
	x=*realof(hd1); n=(int)x;
	if (n<1 || x>=INT_MAX)
		need_arg_in("dup","vector and integer");
	if (hd->type==s_matrix && dimsof(hd)->r==1)
	{	c=dimsof(hd)->c;
		result=new_matrix(n,c,"");
		if (error) return;
		m1=matrixof(hd); m2=matrixof(result);
		for (i=0; i<n; i++)
		{	m=mat(m2,c,i,0);
			memmove((char *)m,(char *)m1,c*sizeof(double));
		}
	}
	else if (hd->type==s_matrix && dimsof(hd)->c==1)
	{	r=dimsof(hd)->r;
		result=new_matrix(r,n,"");
		if (error) return;
		m1=matrixof(hd); m2=matrixof(result);
		for (i=0; i<r; i++)
		{	for (j=0; j<n; j++)
				*mat(m2,n,i,j)=*mat(m1,1,i,0);
		}
	}
	else if (hd->type==s_real)
	{	result=new_matrix(n,1,""); if (error) return;
		m1=matrixof(result);
		for (i=0; i<n; i++) *m1++=*realof(hd);
	}
	else if (hd->type==s_cmatrix && dimsof(hd)->r==1)
	{	c=dimsof(hd)->c;
		result=new_cmatrix(n,c,"");
		if (error) return;
		m1=matrixof(hd); m2=matrixof(result);
		for (i=0; i<n; i++)
		{	m=cmat(m2,c,i,0);
			memmove((char *)m,(char *)m1,(LONG)2*c*sizeof(double));
		}
	}
	else if (hd->type==s_cmatrix && dimsof(hd)->c==1)
	{	r=dimsof(hd)->r;
		result=new_cmatrix(r,n,"");
		if (error) return;
		m1=matrixof(hd); m2=matrixof(result);
		for (i=0; i<r; i++)
		{	for (j=0; j<n; j++)
				copy_complex(cmat(m2,n,i,j),cmat(m1,1,i,0));
		}
	}
	else if (hd->type==s_complex)
	{	result=new_cmatrix(n,1,""); if (error) return;
		m1=matrixof(result);
		for (i=0; i<n; i++) { *m1++=*realof(hd); *m1++=*imagof(hd); }
	}
	else if (hd->type==s_imatrix && dimsof(hd)->r==1)
	{	c=dimsof(hd)->c;
		result=new_imatrix(n,c,"");
		if (error) return;
		m1=matrixof(hd); m2=matrixof(result);
		for (i=0; i<n; i++)
		{	m=imat(m2,c,i,0);
			memmove((char *)m,(char *)m1,(LONG)2*c*sizeof(double));
		}
	}
	else if (hd->type==s_imatrix && dimsof(hd)->c==1)
	{	r=dimsof(hd)->r;
		result=new_imatrix(r,n,"");
		if (error) return;
		m1=matrixof(hd); m2=matrixof(result);
		for (i=0; i<r; i++)
		{	for (j=0; j<n; j++)
				copy_interval(cmat(m2,n,i,j),cmat(m1,1,i,0));
		}
	}
	else if (hd->type==s_interval)
	{	result=new_imatrix(n,1,""); if (error) return;
		m1=matrixof(result);
		for (i=0; i<n; i++) { *m1++=*aof(hd); *m1++=*bof(hd); }
	}
	else
		need_arg_in("dup","vector and integer");
	moveresult(st,result);
}

void make_complex (header *hd)
/**** make_complex
	make a function argument complex.
****/
{	
	header *old=hd,*nextarg;
	ULONG size;
	int r,c,i,j;
	double *m,*m1;
	hd=getvariablesub(hd);
	if (iscomplex(hd)) return;
	if (isinterval(hd))
	{	output("Cannot convert from interval to complex.\n");
		error=1; return;
	}
	hd=getvalue(hd);
	if (hd->type==s_real)
	{	size=sizeof(header)+2*sizeof(double);
		nextarg=nextof(old);
		if (!freeram((size-old->size)))
		{	output("Memory overflow!\n"); error=180; return; }
		if (newram>(char *)nextarg)
			memmove((char *)old+size,(char *)nextarg,
				newram-(char *)nextarg);
		newram+=size-old->size;
		*(old->name)=0; old->size=size;
		old->type=s_complex;
		*realof(old)=*realof(hd);
		*imagof(old)=0.0;
	}
	else if (hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		size=cmatrixsize(r,c);
		nextarg=nextof(old);
		if (!freeram(size-old->size))
		{	output("Memory overflow!\n"); error=180; return; }
		if (newram>(char *)nextarg)
			memmove((char *)old+size,(char *)nextarg,
				newram-(char *)nextarg);
		newram+=size-old->size;
		*(old->name)=0; old->size=size;
		old->type=s_cmatrix;
		dimsof(old)->r=r; dimsof(old)->c=c;
		m1=matrixof(old);
		for (i=r-1; i>=0; i--)
			for (j=c-1; j>=0; j--)
			{	*cmat(m1,c,i,j)=*mat(m,c,i,j);
				*(cmat(m1,c,i,j)+1)=0.0;
			}
	}
	else
		need_arg_in("complex","real matrix");
}

void mvconcat (header *hd)
{	
	header *st=hd,*hd1,*result;
	double *m,*m1;
	int r,c,r1,c1,i;
	hd=getvalue(hd);
	hd1=next_param(st);
	if (!hd1)
		need_arg_in("_","matrices");
	hd1=getvalue(hd1); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	
		if (hd1->type==s_real || hd1->type==s_matrix)
		{	
			getmatrix(hd,&r,&c,&m);
			getmatrix(hd1,&r1,&c1,&m1);
			if (c==0) r=0;
			if (c1==0) r1=0;
			if ((LONG)r+r1>INT_MAX)
				overflow_in("_");
			int c2=(c>c1)?c:c1;
			result=new_matrix(r+r1,c2,"");
			double *mr=matrixof(result);
			double fill=0.0;
			if (hd->type==s_real) fill=*m;
			double fill1=0.0;
			if (hd1->type==s_real) fill1=*m1;
			for (i=0; i<r; i++)
			{	for (int j=0; j<c2; j++)
				{	if (j<c) *mr++=*m++;
					else *mr++=fill;
				}
			}			
			for (i=0; i<r1; i++)
			{	for (int j=0; j<c2; j++)
				{	if (j<c1) *mr++=*m1++;
					else *mr++=fill1;
				}
			}			
		}
		else if (hd1->type==s_complex || hd1->type==s_cmatrix)
		{	
			make_complex(st);
			mvconcat(st);
			return;
		}
		else if (hd1->type==s_interval || hd1->type==s_imatrix)
		{	
			make_interval(st);
			mvconcat(st);
			return;
		}
		else
			need_arg_in("_","matrices");
	}
	else if (hd->type==s_complex || hd->type==s_cmatrix)
	{	
		if (hd1->type==s_complex || hd1->type==s_cmatrix)
		{	
			getmatrix(hd,&r,&c,&m);
			getmatrix(hd1,&r1,&c1,&m1);
			if (c==0) r=0;
			if (c1==0) r1=0;
			if ((LONG)r+r1>INT_MAX)
				overflow_in("_");
			int c2=(c>c1)?c:c1;
			result=new_cmatrix(r+r1,c2,"");
			double *mr=matrixof(result);
			double fillr=0.0,filli=0.0;
			if (hd->type==s_complex) { fillr=*m; filli=*(m+1); }
			double fill1r=0.0,fill1i=0.0;
			if (hd1->type==s_complex) { fill1r=*m1; fill1i=*(m1+1); }
			for (i=0; i<r; i++)
			{	for (int j=0; j<c2; j++)
				{	if (j<c) { *mr++=*m++; *mr++=*m++; }
					else { *mr++=fillr; *mr++=filli; }
				}
			}			
			for (i=0; i<r1; i++)
			{	for (int j=0; j<c2; j++)
				{	if (j<c1) { *mr++=*m1++; *mr++=*m1++; }
					else { *mr++=fill1r; *mr++=fill1i; }
				}
			}			
		}
		else if (hd1->type==s_real || hd1->type==s_matrix)
		{	make_complex(next_param(st));
			mvconcat(st);
			return;
		}
		else
			need_arg_in("_","matrices");
	}
	else if (hd->type==s_interval || hd->type==s_imatrix)
	{	
		if (hd1->type==s_interval || hd1->type==s_imatrix)
		{	
			getmatrix(hd,&r,&c,&m);
			getmatrix(hd1,&r1,&c1,&m1);
			if (c==0) r=0;
			if (c1==0) r1=0;
			if ((LONG)r+r1>INT_MAX)
				overflow_in("_");
			int c2=(c>c1)?c:c1;
			result=new_imatrix(r+r1,c2,"");
			double *mr=matrixof(result);
			double fillr=0.0,filli=0.0;
			if (hd->type==s_interval) { fillr=*m; filli=*(m+1); }
			double fill1r=0.0,fill1i=0.0;
			if (hd1->type==s_interval) { fill1r=*m1; fill1i=*(m1+1); }
			for (i=0; i<r; i++)
			{	for (int j=0; j<c2; j++)
				{	if (j<c) { *mr++=*m++; *mr++=*m++; }
					else { *mr++=fillr; *mr++=filli; }
				}
			}			
			for (i=0; i<r1; i++)
			{	for (int j=0; j<c2; j++)
				{	if (j<c1) { *mr++=*m1++; *mr++=*m1++; }
					else { *mr++=fill1r; *mr++=fill1i; }
				}
			}
		}
		else if (hd1->type==s_real || hd1->type==s_matrix)
		{	make_interval(next_param(st));
			mvconcat(st);
			return;
		}
		else
			need_arg_in("_","matrices");
	}
	else
		need_arg_in("_","matrices");
	moveresult(st,result);
}

void mhconcat (header *hd)
{	header *st=hd,*hd1,*result;
	double *m,*m1,*mr;
	int r,c,r1,c1,i;
	hd=getvalue(hd);
	hd1=next_param(st);
	if (!hd1)
		need_arg_in("|","matrices or strings");
	static char work[32],format[32];
	hd1=getvalue(hd1); if (error) return;
	if (hd->type==s_string)
	{	
		int utf=hd->flags&UTF;
		if (hd1->type==s_string)
		{	
			utf|=hd1->flags&UTF;
			result=new_string(stringof(hd),
				strlen(stringof(hd))+strlen(stringof(hd1))+1,"");
			strcat(stringof(result),stringof(hd1));
			if (utf) result->flags|=UTF;
		}
		else if (hd1->type==s_smatrix)
		{	
			utf|=hd1->flags&UTF;
			result=new_smatrix("");
			char *str1=stringof(result);
			strcpy(str1,stringof(hd));
			str1+=strlen(str1)+1;
			char *str=stringof(hd1);
			while (*str!=1)
			{	strcpy(str1,str);
				str+=strlen(str)+1; str1+=strlen(str1)+1;
			}
			*str1++=1;
			result->size=((int)((str1-(char *)result)/ALIGNMENT)+1)*ALIGNMENT;
			newram=(char *)result+result->size;
			if (utf) result->flags|=UTF;
		}
		else if (hd1->type==s_real)
		{	
			double x=*realof(hd1);
			if (isnan(x)) strcpy(work,"NAN");
			else 
			{
				if (zerorounding && fabs(x)<1e-12) x=0;
				sprintf(work,scalarformat,x);
			}
			result=new_string(stringof(hd),
				strlen(stringof(hd))+strlen(work)+1,"");
			strcat(stringof(result),work);
			if (utf) result->flags|=UTF;
		}
		else if (hd1->type==s_complex)
		{	double x=*realof(hd1),y=*(realof(hd1)+1);
			if (zerorounding && fabs(x)<1e-12) x=0;
			if (zerorounding && fabs(y)<1e-12) y=0;
			if (y<0)
			{
				sprintf(format,"%s-%si",goodformat,goodformat);
				sprintf(work,format,x,-y);
			}
			else
			{
				sprintf(format,"%s+%si",goodformat,goodformat);
				sprintf(work,format,x,y);
			}
			result=new_string(stringof(hd),
				strlen(stringof(hd))+strlen(work)+1,"");
			strcat(stringof(result),work);
			if (utf) result->flags|=UTF;
		}
		else if (hd1->type==s_interval)
		{	double x=*realof(hd1),y=*(realof(hd1)+1);
			if (zerorounding && fabs(x)<1e-12) x=0;
			if (zerorounding && fabs(y)<1e-12) y=0;
			sprintf(format,"~%s,%s~",goodformat,goodformat);
			sprintf(work,format,x,y);
			result=new_string(stringof(hd),
				strlen(stringof(hd))+strlen(work)+1,"");
			strcat(stringof(result),work);
			if (utf) result->flags|=UTF;
		}
		else if (hd1->type==s_matrix)
		{	getmatrix(hd1,&r,&c,&m);
			if (r==0 || c==0)
			{	result=hd;
			}
			else
				need_arg_in("|","matrices or strings");
		}
		else
			need_arg_in("|","matrices or strings");
	}
	else if (hd->type==s_real || hd->type==s_matrix)
	{	
		if (hd1->type==s_real || hd1->type==s_matrix)
		{	
			getmatrix(hd,&r,&c,&m);
			getmatrix(hd1,&r1,&c1,&m1);
			if ((LONG)c+c1>INT_MAX)
				need_arg_in("|","matrices or strings");
			int r2=(r>r1)?r:r1;
			result=new_matrix(r2,c+c1,"");
			mr=matrixof(result);
			double fill=0.0;
			if (hd->type==s_real) fill=*m;
			double fill1=0.0;
			if (hd1->type==s_real) fill1=*m1;
			for (i=0; i<r2; i++)
			{	for (int j=0; j<c; j++)
				{	if (i<r) *mr++=*m++;
					else *mr++=fill;
				}
				for (int j=0; j<c1; j++)
				{	if (i<r1) *mr++=*m1++;
					else *mr++=fill1;
				}
			}
		}
		else if (hd1->type==s_complex || hd1->type==s_cmatrix)
		{	
			make_complex(st);
			mhconcat(st);
			return;
		}
		else if (hd1->type==s_interval || hd1->type==s_imatrix)
		{	
			make_interval(st);
			mhconcat(st);
			return;
		}
		else if (hd->type==s_real && hd1->type==s_string)
		{	
			double x=*realof(hd);
			if (zerorounding && fabs(x)<1e-12) x=0;
			sprintf(work,scalarformat,x);
			result=new_string(work,
				strlen(stringof(hd1))+strlen(work)+1,"");
			strcat(stringof(result),stringof(hd1));
		}
		else if (hd->type==s_matrix && hd1->type==s_string)
		{
			getmatrix(hd,&r,&c,&m);
			if (c!=0)
			{
				print("Can only append a string to an empty vector!\n");
				error=10; return;
			}
			result=new_smatrix("");
			char *p=stringof(result);
			strcpy(p,stringof(hd1));
			p+=strlen(p)+1;
			*p++=1;
			newram=p;
			result->size=p-(char *)result;
		}
		else
			need_arg_in("|","matrices or strings");
	}
	else if (hd->type==s_complex || hd->type==s_cmatrix)
	{	
		if (hd1->type==s_complex || hd1->type==s_cmatrix)
		{	getmatrix(hd,&r,&c,&m);
			getmatrix(hd1,&r1,&c1,&m1);
			if ((LONG)c+c1>INT_MAX)
				need_arg_in("|","matrices or strings");
			int r2=(r>r1)?r:r1;
			result=new_cmatrix(r2,c+c1,"");
			mr=matrixof(result);
			double fillr=0.0,filli=0.0;
			if (hd->type==s_complex) { fillr=*m; filli=*(m+1); }
			double fill1r=0.0,fill1i=0.0;
			if (hd1->type==s_complex) { fill1r=*m1; fill1i=*(m1+1); }
			for (i=0; i<r2; i++)
			{	for (int j=0; j<c; j++)
				{	if (i<r) { *mr++=*m++; *mr++=*m++; }
					else { *mr++=fillr; *mr++=filli; }
				}
				for (int j=0; j<c1; j++)
				{	if (i<r1) { *mr++=*m1++; *mr++=*m1++; }
					else { *mr++=fill1r; *mr++=fill1i; }
				}
			}
		}
		else if (hd1->type==s_real || hd1->type==s_matrix)
		{	make_complex(next_param(st)); if (error) return;
			mhconcat(st);
			return;
		}
		else if (hd->type==s_complex && hd1->type==s_string)
		{	double x=*realof(hd),y=*(realof(hd)+1);
			if (zerorounding && fabs(x)<1e-12) x=0;
			if (zerorounding && fabs(y)<1e-12) y=0;
			sprintf(format,"%s+%si",goodformat,goodformat);
			sprintf(work,format,x,y);
			result=new_string(work,
				strlen(stringof(hd1))+strlen(work)+1,"");
			strcat(stringof(result),stringof(hd1));
		}
		else
			need_arg_in("|","matrices or strings");
	}
	else if (hd->type==s_interval || hd->type==s_imatrix)
	{	
		if (hd1->type==s_interval || hd1->type==s_imatrix)
		{	getmatrix(hd,&r,&c,&m);
			getmatrix(hd1,&r1,&c1,&m1);
			if ((LONG)c+c1>INT_MAX)
				need_arg_in("|","matrices or strings");
			int r2=(r>r1)?r:r1;
			result=new_imatrix(r2,c+c1,"");
			mr=matrixof(result);
			double fillr=0.0,filli=0.0;
			if (hd->type==s_complex) { fillr=*m; filli=*(m+1); }
			double fill1r=0.0,fill1i=0.0;
			if (hd1->type==s_complex) { fill1r=*m1; fill1i=*(m1+1); }
			for (i=0; i<r2; i++)
			{	for (int j=0; j<c; j++)
				{	if (i<r) { *mr++=*m++; *mr++=*m++; }
					else { *mr++=fillr; *mr++=filli; }
				}
				for (int j=0; j<c1; j++)
				{	if (i<r1) { *mr++=*m1++; *mr++=*m1++; }
					else { *mr++=fill1r; *mr++=fill1i; }
				}
			}
		}
		else if (hd1->type==s_real || hd1->type==s_matrix)
		{	make_interval(next_param(st)); if (error) return;
			mhconcat(st);
			return;
		}
		else if (hd->type==s_interval && hd1->type==s_string)
		{	double x=*realof(hd),y=*(realof(hd)+1);
			if (zerorounding && fabs(x)<1e-12) x=0;
			if (zerorounding && fabs(y)<1e-12) y=0;
			sprintf(format,"~%s,%s~",goodformat,goodformat);
			sprintf(work,format,x,y);
			result=new_string(work,
				strlen(stringof(hd1))+strlen(work)+1,"");
			strcat(stringof(result),stringof(hd1));
		}
		else
			need_arg_in("|","matrices or strings");
	}
	else if (hd->type==s_smatrix)
	{	
		result=new_smatrix("");
		char *str1=stringof(result);
		char *str=stringof(hd);
		while (*str!=1)
		{	strcpy(str1,str);
			str+=strlen(str)+1; str1+=strlen(str1)+1;
		}
		if (hd1->type==s_string)
		{	strcpy(str1,stringof(hd1));
			str1+=strlen(str1)+1;
		}
		else if (hd1->type==s_smatrix)
		{	char *str=stringof(hd1);
			while (*str!=1)
			{	strcpy(str1,str);
				str+=strlen(str)+1; str1+=strlen(str1)+1;
			}
		}
		else if (hd1->type==s_matrix)
		{	getmatrix(hd1,&r,&c,&m);
			if (r==0 || c==0)
			{	result=hd;
			}
			else
				need_arg_in("|","matrices or strings");
		}
		else
			need_arg_in("|","matrices or strings");
		*str1++=1;
		result->size=((int)((str1-(char *)result)/ALIGNMENT)+1)*ALIGNMENT;
		newram=(char *)result+result->size;
	}
	else
		need_arg_in("|","matrices or strings");
	moveresult(st,result);
}

void cscalp (double *s, double *si, double *x, double *xi,
	double *y, double *yi);
void ccopy (double *y, double *x, double *xi);

void wmultiply (header *hd)
/***** multiply
	matrix multiplication for weakly nonzero matrices.
*****/
{	header *result,*st=hd,*hd1;
	dims *d,*d1;
	double *m,*m1,*m2,*mm1,*mm2,x,y,null=0.0;
	int i,j,c,r,k;
	hd=getvalue(hd); hd1=getvalue(nextof(st));
	if (error) return;
	if (hd->type==s_matrix && hd1->type==s_matrix)
	{	d=dimsof(hd);
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
				{	if ((*mm1!=0.0)&&(*mm2!=0.0)) x+=(*mm1)*(*mm2);
					mm1++; mm2+=d1->c;
				}
				*mat(m,c,i,j)=x;
			}
		moveresult(st,result);
		return;
	}
	if (hd->type==s_matrix && hd1->type==s_cmatrix)
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
				{	if ((*mm2!=0.0 || *(mm2+1)!=0.0) &&
							(*mm1!=0.0))
					cscalp(&x,&y,mm1,&null,mm2,mm2+1);
					mm1++; mm2+=2*d1->c;
				}
				ccopy(cmat(m,c,i,j),&x,&y);
			}
		moveresult(st,result);
		return;
	}
	if (hd->type==s_cmatrix && hd1->type==s_matrix)
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
			{	mm1=cmat(m1,d->c,i,0); mm2=m2+j;
				x=0.0; y=0.0;
				for (k=0; k<d->c; k++)
				{	if ((*mm2!=0.0) &&
							(*mm1!=0.0 ||  *(mm1+1)!=0.0))
					cscalp(&x,&y,mm1,mm1+1,mm2,&null);
					mm1+=2; mm2+=d1->c;
				}
				ccopy(cmat(m,c,i,j),&x,&y);
			}
		moveresult(st,result);
		return;
	}
	if (hd->type==s_cmatrix && hd1->type==s_cmatrix)
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
            {   mm1=cmat(m1,d->c,i,0); mm2=m2+(LONG)2*j;
				x=0.0; y=0.0;
				for (k=0; k<d->c; k++)
				{	if ((*mm2!=0.0 || *(mm2+1)!=0.0) &&
							(*mm1!=0.0 || *(mm1+1)!=0.0))
					cscalp(&x,&y,mm1,mm1+1,mm2,mm2+1);
					mm1+=2; mm2+=2*d1->c;
				}
				ccopy(cmat(m,c,i,j),&x,&y);
			}
		moveresult(st,result);
		return;
	}
	else
		need_arg_in("bandmult","two matrices");
}

void smultiply (header *hd)
/***** multiply
	matrix multiplication for weakly nonzero symmetric matrices.
*****/
{	header *result,*st=hd,*hd1;
	dims *d,*d1;
	double *m,*m1,*m2,*mm1,*mm2,x,y,null=0.0;
	int i,j,c,r,k;
	hd=getvalue(hd); hd1=getvalue(nextof(st));
	if (error) return;
	if (hd->type==s_matrix && hd1->type==s_matrix)
	{	d=dimsof(hd);
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
			for (j=i; j<c; j++)
			{	mm1=mat(m1,d->c,i,0); mm2=m2+j;
				x=0.0;
				for (k=0; k<d->c; k++)
				{	if ((*mm1!=0.0)&&(*mm2!=0.0)) x+=(*mm1)*(*mm2);
					mm1++; mm2+=d1->c;
				}
				*mat(m,c,i,j)=x;
				*mat(m,c,j,i)=x;
			}
		moveresult(st,result);
		return;
	}
	if (hd->type==s_matrix && hd1->type==s_cmatrix)
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
			for (j=i; j<c; j++)
            {   mm1=mat(m1,d->c,i,0); mm2=m2+(LONG)2*j;
				x=0.0; y=0.0;
				for (k=0; k<d->c; k++)
				{	if ((*mm2!=0.0 || *(mm2+1)!=0.0) &&
							(*mm1!=0.0))
					cscalp(&x,&y,mm1,&null,mm2,mm2+1);
					mm1++; mm2+=2*d1->c;
				}
				ccopy(cmat(m,c,i,j),&x,&y); y=-y;
				ccopy(cmat(m,c,j,i),&x,&y);
			}
		moveresult(st,result);
		return;
	}
	if (hd->type==s_cmatrix && hd1->type==s_matrix)
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
			for (j=i; j<c; j++)
			{	mm1=cmat(m1,d->c,i,0); mm2=m2+j;
				x=0.0; y=0.0;
				for (k=0; k<d->c; k++)
				{	if ((*mm2!=0.0) &&
							(*mm1!=0.0 ||  *(mm1+1)!=0.0))
					cscalp(&x,&y,mm1,mm1+1,mm2,&null);
					mm1+=2; mm2+=d1->c;
				}
				ccopy(cmat(m,c,i,j),&x,&y); y=-y;
				ccopy(cmat(m,c,j,i),&x,&y);
			}
		moveresult(st,result);
		return;
	}
	if (hd->type==s_cmatrix && hd1->type==s_cmatrix)
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
			for (j=i; j<c; j++)
            {   mm1=cmat(m1,d->c,i,0); mm2=m2+(LONG)2*j;
				x=0.0; y=0.0;
				for (k=0; k<d->c; k++)
				{	if ((*mm2!=0.0 || *(mm2+1)!=0.0) &&
							(*mm1!=0.0 || *(mm1+1)!=0.0))
					cscalp(&x,&y,mm1,mm1+1,mm2,mm2+1);
					mm1+=2; mm2+=2*d1->c;
				}
				ccopy(cmat(m,c,i,j),&x,&y); 
				y=-y;
				ccopy(cmat(m,c,j,i),&x,&y);
			}
		moveresult(st,result);
		return;
	}
	else wrong_arg();
}

void mfht (header *hd)
{	
	header *st=hd,*result;
	double *m,*mr;
	int r,c;
	hd=getvalue(hd);
	if (error) return;
	if (hd->type!=s_matrix || dimsof(hd)->r!=1)
		need_arg_in("fht","real vector");
	getmatrix(hd,&r,&c,&m);
	result=new_matrix(1,c,""); if (error) return;
	mr=matrixof(result);
	memmove((char *)mr,(char *)m,c*sizeof(double));
	alglib::real_1d_array v;
	v.setcontent(c,mr);
	try
	{
		alglib::fhtr1d(v,c);
	}
	catch (alglib::ap_error)
	{
		error=1;
		return;
	}
	memmove((char *)mr,(char *)v.getcontent(),c*sizeof(double));
	if (error) return;
	moveresult(st,result);
}

void mifht (header *hd)
{	
	header *st=hd,*result;
	double *m,*mr;
	int r,c;
	hd=getvalue(hd);
	if (error) return;
	if (hd->type!=s_matrix || dimsof(hd)->r!=1)
		need_arg_in("fht","real vector");
	getmatrix(hd,&r,&c,&m);
	result=new_matrix(1,c,""); if (error) return;
	mr=matrixof(result);
	memmove((char *)mr,(char *)m,c*sizeof(double));
	alglib::real_1d_array v;
	v.setcontent(c,mr);
	try
	{
		alglib::fhtr1dinv(v,c);
	}
	catch (alglib::ap_error)
	{
		error=1;
		return;
	}
	memmove((char *)mr,(char *)v.getcontent(),c*sizeof(double));
	if (error) return;
	moveresult(st,result);
}
