#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "header.h"
#include "stack.h"
#include "cpx.h"
#include "getvalue.h"
#include "mainloop.h"

// routines for compressed matrices

void mcompress (header *hd)
// compress a thin matrix
{	
	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix)
		need_arg_in("cpx","real matrix");
	double *m;
	int r,c;
	getmatrix(hd,&r,&c,&m);
	result=new_cpxmatrix(r,c,0,""); if (error) return;
	xelement *mr=elementsof(result);
	int count=0;
	for (int i=0; i<r; i++)
		for (int j=0; j<c; j++)
		{	
			if ((char *)(mr+3)>ramend)
			{	
				output("Stack overflow in compress!\n");
				error=1; return;
			}
			if (fabs(*m)>epsilon)
			{	
				mr->r=i; mr->c=j;
				mr->x=*m++;
				count++;
				mr++;
			}
			else m++;
		}
	*nelementsof(result)=count;
	newram=(char *)mr;
	result->size=newram-(char *)result;
	moveresult(st,result);
}

void mdecompress (header *hd)
// decompress a compressed matrix
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_cpxmatrix)
		need_arg_in("decpx","compressed matrix");
	int r1=dimsof(hd)->r;
	int c1=dimsof(hd)->c;
	result=new_matrix(r1,c1,""); if (error) return;
	double *mr=matrixof(result);
	for (int i=0; i<r1; i++)
		for (int j=0; j<c1; j++)
			*mr++=0.0;
	xelement *m=elementsof(hd);
	mr=matrixof(result);
	int r=*nelementsof(hd);
	for (int k=0; k<r; k++)
	{	mr[m->r*c1+m->c]=m->x;
		m++;
	}
	moveresult(st,result);	
}

void mcompressmult (header *hd)
// multiply a compressed matrix by a vector (or a matrix).
{	header *st=hd,*hd1,*result;
	hd1=nextof(hd);
	hd=getvalue(hd); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	if (hd->type!=s_cpxmatrix)
		need_arg_in("cpxmult","compressed matrix, real or compressed matrix");
	if (hd1->type==s_matrix)
	{	
		int r=dimsof(hd)->r;
		int c=dimsof(hd)->c;
		int r1=dimsof(hd1)->r;
		int c1=dimsof(hd1)->c;
		if (c!=r1)
		{	output1("Cannot multiply a %dx%d and a %dx%d matrix\n",r,c,r1,c1);
			error=1; return;
		}
		result=new_matrix(r,c1,""); if (error) return;
		double *mr=matrixof(result);
		for (int i=0; i<r; i++)
			for (int j=0; j<c1; j++)
				*mr++=0.0;
		mr=matrixof(result);
		xelement *m=elementsof(hd);
		int nm=*nelementsof(hd);
		double *m1=matrixof(hd1);
		for (int k=0; k<nm; k++)
		{	for (int j=0; j<c1; j++)
			{	mr[m->r*c1+j]+=m->x*m1[m->c*c1+j];
			}
			m++;
		}
	}
	else if (hd1->type==s_cpxmatrix)
	{	
		int r=dimsof(hd)->r;
		int c=dimsof(hd)->c;
		int r1=dimsof(hd1)->r;
		int c1=dimsof(hd1)->c;
		if (c!=r1)
		{	output1("Cannot multiply a %dx%d and a %dx%d matrix\n",r,c,r1,c1);
			error=1; return;
		}
		result=new_cpxmatrix(r,c1,0,""); if (error) return;
		int *rows=(int *)malloc(r1*sizeof(int));
		double *res=(double *)malloc(c1*sizeof(double));
		int n1=*nelementsof(hd1);
		xelement *m1=elementsof(hd1);
		xelement *mr=elementsof(result);
		// collect the start points of the rows in second matrix
		int k=0;
		for (int i=0; i<r1; i++)
		{	
			if (k>=n1 || m1[k].r!=i) rows[i]=-1;
			else 
			{	rows[i]=k;
				while (k<n1 && m1[k].r==i) k++;
			}
		}
		int count=0;
		xelement *m=elementsof(hd);
		// int n=*nelementsof(hd);
		// multiply matrices
		for (int i=0; i<r; i++)
		{	
			if (m->r==i) // there could be an element in row i of result
			{	
				for (int j=0; j<r1; j++) res[j]=0.0;
				while (m->r==i)
				{	
					if (rows[m->c]>=0) // look through row of hd1
					{	for (k=rows[m->c]; k<n1; k++)
						{	
							if (m1[k].r!=m->c) break;
							res[m1[k].c]+=m->x*m1[k].x;
						}
					}
					m++;
				}
				for (int j=0; j<r1; j++)
				{	if (fabs(res[j])>epsilon)
					{	
						mr->r=i; mr->c=j; mr->x=res[j];
						mr++; count++;
						if ((char *)mr+1>ramend) outofram();
					}
				}
			}
		}
		free(rows);
		free(res);
		*nelementsof(result)=count;
		newram=(char *)mr;
		result->size=newram-(char *)result;
	}
	else need_arg_in("cpxmult","compressed matrix, real or compressed matrix");
	moveresult(st,result);
}

void mcpxseidel (header *hd)
// one step of the Gauss Seidel algorithm
{	header *st=hd,*hd1,*hd2,*hd3,*result;
	hd1=nextof(hd);
	hd2=nextof(hd1);
	hd3=nextof(hd2);
	hd=getvalue(hd); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	hd2=getvalue(hd2); if (error) return;
	hd3=getvalue(hd3); if (error) return;
	if (hd->type!=s_cpxmatrix || hd1->type!=s_matrix || hd2->type!=s_matrix 
		|| hd3->type!=s_real)
		need_arg_in("cpxseidel","compressed matrix, real column vector, real column vector, real");
	int r=dimsof(hd)->r;
	int c=dimsof(hd)->c;
	int r1=dimsof(hd1)->r;
	int c1=dimsof(hd1)->c;
	int r2=dimsof(hd2)->r;
	int c2=dimsof(hd2)->c;
	if (c!=r || c!=r1 || c1!=1 || c!=r2 || c2!=1)
		need_arg_in("cpxseidel","compressed matrix, real column vector, real column vector, real");
	result=new_matrix(r,1,"");
	double *mr=matrixof(result);
	double *m2=matrixof(hd2);
	for (int i=0; i<r; i++)
		mr[i]=m2[i];
	xelement *m=elementsof(hd);
	int nm=*nelementsof(hd);
	int nk=0;
	double *m1=matrixof(hd1);
	double omega=*realof(hd3);
	for (int i=0; i<r; i++)
	{	double old=mr[i];
		mr[i]=m1[i];
		double dd=0.0;
		while (m->r==i && nk<nm)
		{	if (m->c==i) dd=m->x;
			else mr[i]-=m->x*mr[m->c];
			m++; nk++;
		}
		if (dd==0.0)
		{	output1("Diagonal element (%d,%d) is zero in cpxseidel!\n",i+1,i+1);
			error=1; return;
		}
		mr[i]/=dd;
		mr[i]=omega*mr[i]+(1-omega)*old;
	}
	moveresult(st,result);
}

int comparexelements (const xelement *e1, const xelement *e2)
{	if (e1->r < e2->r) return -1;
	else if (e1->r == e2->r)
	{	if (e1->c < e2->c) return -1;
		else if (e1->c == e2->c) return 0;
		else return 1;
	}
	else return 1;
}

extern void out_cpxmatrix (header *hd);

void mcpxset (header *hd)
// set entries of a compressed matrix to values,
// the values are stored in rows [i,j,x] in a matrix.
{	header *st=hd,*hd1,*result;
	hd1=nextof(hd);
	hd=getvalue(hd); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	if (hd->type!=s_cpxmatrix || hd1->type!=s_matrix ||
		dimsof(hd1)->c!=3)
			need_arg_in("cpxset","compressed matrix, nx3 matrix");
	int n=*nelementsof(hd);
	int r=dimsof(hd)->r,c=dimsof(hd)->c;
	result=new_cpxmatrix(r,c,n,"");
	if (error) return;
	xelement *m=elementsof(hd);
	xelement *mr=elementsof(result);
	for (int i=0; i<n; i++) *mr++=*m++;
	int k=dimsof(hd1)->r;
	mr=elementsof(result)+n;
	double *m1=matrixof(hd1);
	int count=0;
	for (int h=0; h<k; h++)
	{	int i=(int)*m1++-1;
		int j=(int)*m1++-1;
		double x=*m1++;
		if (i>=0 && j>=0)
		{	if (i>=r) r=i+1;
			if (j>=c) c=j+1;
			mr->r=i; mr->c=j; mr->x=x;
			mr++;
			count++;
		}
	}
	if (count>0)
	{	dimsof(result)->c=c; dimsof(result)->r=r;
		*nelementsof(result)=n+count;
		newram=(char *)mr;
		result->size=newram-(char *)result;
		mr=elementsof(result);
		for (int i=0; i<n; i++)
		{	mr->c=2*mr->c+1; mr++;
		}
		for (int i=n; i<n+count; i++)
		{	mr->c=2*mr->c; mr++;
		}
		mr=elementsof(result);
		qsort(mr,n+count,sizeof(xelement),
			(int (*) (const void *, const void *))comparexelements);
		for (int i=0; i<n+count; i++)
		{	mr->c=mr->c/2; mr++;
		}
		mr=elementsof(result);
		xelement *mr1=mr;
		n=n+count;
		int k=0;
		count=0;
		while (k<n)
		{	if (mr!=mr1) *mr=*mr1++;
			else mr1++;
			k++;
			while (mr->r==mr1->r && mr->c==mr1->c && k<n)
			{	mr1++; k++;
			}
			if (fabs(mr->x)>epsilon) { count++; mr++; }
			if ((char *)mr+1>ramend) outofram();
		}
		*nelementsof(result)=count;
		newram=(char *)mr;
		result->size=newram-(char *)result;
	}
	moveresult(st,result);
}

void mcpxget (header *hd)
{
	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_cpxmatrix)
	{
		print("Need a compressed matrix for cpxmatrix.\n");
		error=1; return;
	}
	int n=*nelementsof(hd);
	xelement *m=elementsof(hd);
	result=new_matrix(n,3,""); if (error) return;
	double *mres=matrixof(result);
	for (int i=0; i<n; i++)
	{	
		*mres++=m->r+1;
		*mres++=m->c+1;
		*mres++=m->x;
		m++;
	}
	moveresult(st,result);
}

void mcpxzeros (header *hd)
// create an empty compressed matrix.
{	header *result,*st=hd;
	int r,c;
	get_size(hd,&r,&c,"cpxzeros");
	result=new_cpxmatrix(r,c,0,""); if (error) return;
	moveresult(st,result);
}

void mcpxtranspose (header *hd)
// transpose a compressed matrix
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_cpxmatrix)
		need_arg_in("cpxtranspose","compressed matrix");
	int n=*nelementsof(hd);
	int r=dimsof(hd)->r,c=dimsof(hd)->c;
	result=new_cpxmatrix(r,c,n,"");
	if (error) return;
	xelement *m=elementsof(hd);
	xelement *mr=elementsof(result);
	for (int i=0; i<n; i++)
	{	mr->r=m->c; mr->c=m->r; mr->x=m->x;
		mr++; m++;
		if ((char *)mr+1>ramend) outofram();
	}
	mr=elementsof(result);
	qsort(mr,n,sizeof(xelement),
		(int (*) (const void *, const void *))comparexelements);
	moveresult(st,result);	
}
