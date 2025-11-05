#include <float.h>
#include <math.h>
#include <string.h>

#include "interval.h"
#include "spread.h"
#include "stack.h"
#include "express.h"
#include "getvalue.h"

#define error(s) error=1, output(s"\n")
#define errorn(s,n) error=n, output(s"\n")

static long nmark;

#define increase(r,n) nmark=(n); if (!freeramfrom((r),(nmark))) outofram(); (r)+=nmark;

double eps_1_up=(1+DBL_EPSILON),eps_1_down=(1-DBL_EPSILON);

static double min (double x, double y)
{	if (x<y) return x;
	else return y;
}

static double max (double x, double y)
{	if (x>y) return x;
	else return y;
}

double round_up (double x)
{	if (x>0) return x*eps_1_up;
	else return x*eps_1_down;
}

double round_down (double x)
{	if (x>0) return x*eps_1_down;
	else return x*eps_1_up;
}

void interval_add (double *a, double *b, double *a1, double *b1,
	double *ar, double *br)
{	*ar=round_down(*a+*a1);
	*br=round_up(*b+*b1);
}

void interval_sub (double *a, double *b, double *a1, double *b1,
	double *ar, double *br)
{	*ar=round_down(*a-*b1);
	*br=round_up(*b-*a1);
}

void interval_mult (double *a, double *b, double *a1, double *b1,
	double *ar, double *br)
{   if (*a>=0)
	{	if (*a1>=0)
		{	*ar=round_down(*a**a1);
			*br=round_up(*b**b1);
		}
		else if (*b1>=0)
		{	*ar=round_down(*b**a1);
			*br=round_up(*b**b1);
		}
		else
		{	*ar=round_down(*b**a1);
			*br=round_up(*a**b1);
		}
	}
	else if (*b>=0)
	{	if (*a1>=0)
		{	*ar=round_down(*a**b1);
			*br=round_up(*b**b1);
		}
		else if (*b1>=0)
		{	*ar=min(round_down(*a**b1),round_down(*b**a1));
			*br=max(round_up(*b**b1),round_up(*a**a1));
		}
		else
		{	*ar=round_down(*b**a1);
			*br=round_up(*a**a1);
		}
	}
	else
	{	if (*a1>=0)
		{	*ar=round_down(*a**b1);
			*br=round_up(*b**a1);
		}
		else if (*b1>=0)
		{	*ar=round_down(*a**b1);
			*br=round_up(*a**a1);
		}
		else
		{	*ar=round_down(*b**b1);
			*br=round_up(*a**a1);
		}
	}
}

void interval_div (double *a, double *b, double *a1, double *b1,
	double *ar, double *br)
{   if (*a>=0)
	{	if (*a1>0)
		{	*ar=round_down(*a/ *b1);
			*br=round_up(*b/ *a1);
		}
		else if (*b1>=0) error("Division by 0");
		else
		{	*ar=round_down(*b/ *b1);
			*br=round_up(*a/ *a1);
		}
	}
	else if (*b>=0)
	{	if (*a1>0)
		{	*ar=round_down(*a/ *a1);
			*br=round_up(*b/ *a1);
		}
		else if ( *b1>=0) error("Division by 0");
		else
		{	*ar=round_down(*b/ *b1);
			*br=round_up(*a/ *b1);
		}
	}
	else
	{	if (*a1>0)
		{	*ar=round_down(*a/ *a1);
			*br=round_up(*b/ *b1);
		}
		else if ( *b1>=0) error("Division by 0");
		else
		{	*ar=round_down(*b/ *a1);
			*br=round_up(*a/ *b1);
		}
	}
}

void interval_invert (double *a, double *b, double *ar, double *br)
{	*ar=-*b;
	*br=-*a;
}

void minterval (header *hd)
{	header *st=hd,*hd1=nextof(hd),*result=0;
	int i,j;
	long n,k;
	double *m1,*m2,*m;
	hd=getvalue(hd);
	hd1=getvalue(hd1);
	if (error) return;
	if (hd->type==s_real)
	{	if (hd1->type==s_real)
		{	result=new_interval(*realof(hd),*realof(hd1),"");
			if (error) return;
			if (*aof(result)>*bof(result)) error("Empty Interval");
		}
		else if (hd1->type==s_matrix)
		{   i=dimsof(hd1)->r; j=dimsof(hd1)->c;
			result=new_imatrix(i,j,"");
			if (error) return;
			n=(long)i*j;
			m=matrixof(result);
			m1=matrixof(hd1);
			for (k=0; k<n; k++)
			{	*m++=*realof(hd);
				*m++=*m1++;
				if (*(m-2)>*(m-1))
				{	error("Empty interval");
					return;
				}
			}
		}
		else error("Illegal argument for ~");
	}
	else if (hd->type==s_matrix)
	{	if (hd1->type==s_real)
		{	i=dimsof(hd)->r; j=dimsof(hd)->c;
			result=new_imatrix(i,j,"");
			if (error) return;
			n=(long)i*j;
			m=matrixof(result);
			m1=matrixof(hd);
			for (k=0; k<n; k++)
			{	*m++=*m1++;
				*m++=*realof(hd1);
				if (*(m-2)>*(m-1))
				{	error("Empty interval");
					return;
				}
			}
		}
		else if (hd1->type==s_matrix)
		{	i=dimsof(hd1)->r; j=dimsof(hd1)->c;
			result=new_imatrix(i,j,"");			
			if (i!=dimsof(hd)->r || j!=dimsof(hd)->c)
				error("Matrix dimensions must agree for ~");
			if (error) return;
			n=(long)i*j;
			m=matrixof(result);
			m1=matrixof(hd);
			m2=matrixof(hd1);
			for (k=0; k<n; k++)
			{	*m++=*m1++;
				*m++=*m2++;
				if (*(m-2)>*(m-1))
				{	error("Empty interval");
					return;
				}
			}
		}
		else error("Illegal argument for ~a,b~");
	}
	else error("Illegal argument for ~a,b~");
	if (error) return;
	moveresult(st,result);
}

void minterval1 (header *hd)
{	header *st=hd,*result=0;
	int i,j;
	long n,k;
	double *m1,*m;
	hd=getvalue(hd);
	if (error) return;
	if (hd->type==s_real)
	{	result=new_interval(round_down(*realof(hd)),
				round_up(*realof(hd)),"");
	}
	else if (hd->type==s_matrix)
	{	i=dimsof(hd)->r; j=dimsof(hd)->c;
		result=new_imatrix(i,j,"");
		if (error) return;
		n=(long)i*j;
		m=matrixof(result);
		m1=matrixof(hd);
		for (k=0; k<n; k++)
		{	*m++=round_down(*m1);
			*m++=round_up(*m1++);
		}
	}
	else if (isinterval(hd)) return;
	else error("Illegal argument for ~a~");
	if (error) return;
	moveresult(st,result);
}

void isin (double *a, double *b, double *ar, double *br)
{	
	if (*b-*a>2*M_PI)
	{
		*ar=-1; *br=1; return;
	}
	if (cos(*a)>0)
	{
		if (cos(*b)>0)
		{
			if (*b-*a<M_PI)
			{
				*ar=round_down(sin(*a));
				*br=round_up(sin(*b));
			}
			else
			{
				*ar=-1; *br=1;
			}
		}
		else
		{
			*br=1;
			*ar=round_down(sin(*a));
			double h=round_down(sin(*b));
			if (h<*ar) *ar=h;
		}
	}
	else
	{
		if (cos(*b)<=0)
		{
			if (*b-*a<M_PI)
			{
				*ar=round_down(sin(*b));
				*br=round_up(sin(*a));
			}
			else
			{
				*ar=-1; *br=1;
			}
		}
		else
		{
			*ar=-1;
			*br=round_up(sin(*a));
			double h=round_up(sin(*b));
			if (h>*br) *br=h;
		}
	}
}

void icos (double *a, double *b, double *ar, double *br)
{	
	if (*b-*a>2*M_PI)
	{
		*ar=-1; *br=1; return;
	}
	if (sin(*a)<0)
	{
		if (sin(*b)<0)
		{
			if (*b-*a<M_PI)
			{
				*ar=round_down(cos(*a));
				*br=round_up(cos(*b));
			}
			else
			{
				*ar=-1; *br=1;
			}
		}
		else
		{
			*br=1;
			*ar=round_down(cos(*a));
			double h=round_down(cos(*b));
			if (h<*ar) *ar=h;
		}
	}
	else
	{
		if (sin(*b)>=0)
		{
			if (*b-*a<M_PI)
			{
				*ar=round_down(cos(*b));
				*br=round_up(cos(*a));
			}
			else
			{
				*ar=-1; *br=1;
			}
		}
		else
		{
			*ar=-1;
			*br=round_up(cos(*a));
			double h=round_up(cos(*b));
			if (h>*br) *br=h;
		}
	}
}

void itan (double *a, double *b, double *ar, double *br)
{   if (*a>=M_PI/2 || *b<=-M_PI/2) { error("tan out of range"); return; }
	*ar=round_down(tan(*a));
	*br=round_up(tan(*b));
}

void iatan (double *a, double *b, double *ar, double *br)
{	*ar=round_down(atan(*a));
	*br=round_up(atan(*b));
}

void iasin (double *a, double *b, double *ar, double *br)
{	*ar=round_down(asin(*a));
	*br=round_up(asin(*b));
}

void iacos (double *a, double *b, double *ar, double *br)
{	*ar=round_down(acos(*b));
	*br=round_up(acos(*a));
}

void iexp (double *a, double *b, double *ar, double *br)
{	*ar=round_down(exp(*a));
	*br=round_up(exp(*b));
}

void ilog (double *a, double *b, double *ar, double *br)
{   if (*a<=0) { error("Log out of range"); return; }
	*ar=round_down(log(*a));
	*br=round_up(log(*b));
}

void isqrt (double *a, double *b, double *ar, double *br)
{   if (*a<=0) { error("Sqrt out of range"); return; }
	*ar=round_down(sqrt(*a));
	*br=round_up(sqrt(*b));
}

void iabs (double *a, double *b, double *ar, double *br)
{   if (*a<0)
	{	if (*b<0)
		{	*ar=-*b;
			*br=-*a;
		}
		else
		{	*ar=0;
			if (*b>-*a) *br=*b;
			else *br=-*a;
		}
	}
	else
	{	*ar=*a;
		*br=*b;
	}
}

void idegree (double *a, double *b, double *ar, double *br)
{
	*ar=round_down(*a/180*M_PI);
	*br=round_up(*b/180*M_PI);
}

void ipercent (double *a, double *b, double *ar, double *br)
{
	*ar=round_down(*a/100);
	*br=round_up(*b/100);
}

void ipow (double *a, double *b, double *a1, double *b1, double *ar, double *br)
{   int n;
	if (*a>0)
	{	if (*a>=1)
		{	if (*a1>=0)
			{	*ar=round_down(pow(*a,*a1));
				*br=round_up(pow(*b,*b1));
			}
			else if (*b1>0)
			{	*ar=round_down(pow(*b,*a1));
				*br=round_up(pow(*b,*b1));
			}
			else
			{	*ar=round_down(pow(*b,*a1));
				*br=round_up(pow(*a,*b1));
			}
		}
		else if (*b>1)
		{	*ar=round_down(min(pow(*a,*b1),pow(*b,*a1)));
			*br=round_up(max(pow(*b,*b1),pow(*a,*a1)));
		}
		else
		{	if (*a1>=0)
			{	*br=round_up(pow(*b,*a1));
				*ar=round_down(pow(*a,*b1));
			}
			else if (*b1>0)
			{	*br=round_up(pow(*a,*a1));
				*ar=round_down(pow(*a,*b1));
			}
			else
			{	*br=round_up(pow(*a,*a1));
				*ar=round_down(pow(*b,*b1));
			}
		}
	}
	else if (*a1==*b1 && *b1==(n=(int)*a1))
	{	if (n%2==0)
		{	if (n>0)
			{	if (*b>=0)
				{	*ar=0;
					*br=round_up(max(pow(*b,n),pow(-*a,n)));
				}
				else
				{	*ar=round_down(pow(-*b,n));
					*br=round_up(pow(-*a,n));
				}
			}
			else if (n==0)
			{	*ar=*br=1;
			}
			else
			{   if (*b>=0) error("0^n undefined for negative n");
				else
				{	*ar=round_down(pow(-*a,n));
					*br=round_up(pow(-*b,n));
				}
			}
		}
		else
		{	if (n>0)
			{	*ar=round_down(pow(*a,n));
				*br=round_up(pow(*b,n));
			}			else
			{	*br=-round_down(pow(*b,n));
				*ar=-round_up(pow(*a,n));
			}
		}	
	}
	else errorn("Cannot raise negative number to non-integer power.",10);
}

void imax (double *a, double *b, double *a1, double *b1, double *ar, double *br)
{	*br=max(*b,*b1);
	*ar=max(*a,*a1);
}

void imin (double *a, double *b, double *a1, double *b1, double *ar, double *br)
{	*br=min(*b,*b1);
	*ar=min(*a,*a1);
}

void fid (double *a, double *b)
{	*b=*a;
}

void ileft (double *a, double *b, double *r)
{	*r=*a;
}

void mleft (header *hd)
{	spreadir1(fid,ileft,hd);
}

void iright (double *a, double *b, double *r)
{	*r=*b;
}

void mright (header *hd)
{	spreadir1(fid,iright,hd);
}

void imiddle (double *a, double *b, double *r)
{	*r=(*a+*b)/2;
}

void mmiddle (header *hd)
{	spreadir1(fid,imiddle,hd);
}

void idiameter (double *a, double *b, double *r)
{	*r=*b-*a;
}

void fnull (double *a, double *b)
{	*b=0;
}

void mdiameter (header *hd)
{	spreadir1(fnull,idiameter,hd);
}

void ior (double *a, double *b, double *a1, double *b1,
	double *ar, double *br)
{	*ar=min(*a,*a1);
	*br=max(*b,*b1);
}

void iintersects (double *a, double *b, double *a1, double *b1,
	double *z)
{   if (*b>=*a1 && *a<=*b1) *z=1.0;
	else *z=0.0;
}

void iand (double *a, double *b, double *a1, double *b1,
	double *ar, double *br)
{	*ar=max(*a,*a1);
	*br=min(*b,*b1);
	if (*ar>*br) error("Empty intersection.");
}

void iless1 (double *a, double *b, double *a1, double *b1, double *r)
{	*r=(*a>*a1 && *b<=*b1) || (*a>=*a1 && *b<*b1);
}

void miless (header *hd)
{   spreadf2r(0,0,iless1,hd,"<");
}


void ilesseq1 (double *a, double *b, double *a1, double *b1, double *r)
{	*r=*a>=*a1 && *b<=*b1;
}

void milesseq (header *hd)
{   spreadf2r(0,0,ilesseq1,hd,"<=");
}

void copy_interval (double *x, double *y)
{	*x++=*y++;
	*x=*y;
}

void make_interval (header *hd)
/**** make_interval
	make a function argument interval.
****/
{	header *old=hd,*nextarg;
	ULONG size;
	int r,c,i,j;
	double *m,*m1;
	hd=getvariablesub(hd);
	if (isinterval(hd)) return;
	if (iscomplex(hd))
	{	output("Cannot convert from complex to interval.\n");
		error=1; return;
	}
	hd=getvalue(hd);
	if (hd->type==s_real)
	{	size=sizeof(header)+2*sizeof(double);
		nextarg=nextof(old);
		if (!freeram(size-old->size))
		{	output("Memory overflow!\n"); error=180; return; }
		if (newram>(char *)nextarg)
			memmove((char *)old+size,(char *)nextarg,
				newram-(char *)nextarg);
		newram+=size-old->size;
		*(old->name)=0; old->size=size;
		old->type=s_interval;
		*aof(old)=*realof(hd);
		*bof(old)=*realof(hd);
	}
	else if (hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		size=imatrixsize(r,c);
		nextarg=nextof(old);
		if (!freeram(size-old->size))
		{	output("Memory overflow!\n"); error=180; return; }
		if (newram>(char *)nextarg)
			memmove((char *)old+size,(char *)nextarg,
				newram-(char *)nextarg);
		newram+=size-old->size;
		*(old->name)=0; old->size=size;
		old->type=s_imatrix;
		dimsof(old)->r=r; dimsof(old)->c=c;
		m1=matrixof(old);
		for (i=r-1; i>=0; i--)
			for (j=c-1; j>=0; j--)
			{	*imat(m1,c,i,j)=*mat(m,c,i,j);
				*(imat(m1,c,i,j)+1)=*mat(m,c,i,j);
			}
	}
	else
	{	output("Argument not real or complex.\n");
		error=1; return;
	}
}

/******************* interval linear systems **************/

void i_add (interval x, interval y, interval z)
{	z[0]=round_down(x[0]+y[0]);
	z[1]=round_up(x[1]+y[1]);
}

void i_sub (interval x, interval y, interval z)
{	z[0]=round_down(x[0]-y[1]);
	z[1]=round_up(x[1]-y[0]);
}

void i_mult (interval x, interval y, interval z)
{	double h,h1;
	interval_mult(x,x+1,y,y+1,&h,&h1);
	z[0]=h;
	z[1]=h1;
}

void i_div (interval x, interval y, interval z)
{	double h,h1;
	interval_div(x,x+1,y,y+1,&h,&h1);
	z[0]=h;
	z[1]=h1;
}

double i_abs (interval x)
{	return max(fabs(x[1]),fabs(x[2]));
}

void i_copy (interval x, interval y)
{	x[0]=y[0];
	x[1]=y[1];
}

static char *ram;
static int *perm,*col,signdet,luflag=0;
static interval **i_lumat,*i_c,i_det;
static int rank;

void i_lu (double *a, int n, int m)
/***** clu
	lu decomposition of a
*****/
{	int i,j,k,mm,j0,kh;
	double *d,piv,temp,zmax,help;
	interval t,*h,*temp1;

	if (!luflag)
	{	/* get place for result c and move a to c */
		i_c=(interval *)ram;
		increase(ram,(LONG)n*m*sizeof(interval));
		memmove((char *)i_c,(char *)a,(LONG)n*m*sizeof(interval));
	}
	else i_c=(interval *)a;

	/* inititalize c_lumat */
	i_lumat=(interval **)ram;
	increase(ram,(LONG)n*sizeof(interval *));
	h=i_c;
	for (i=0; i<n; i++) { i_lumat[i]=h; h+=m; }

	/* get place for perm */
	perm=(int *)ram;
	increase(ram,(LONG)n*sizeof(int));

	/* get place for col */
	col=(int *)ram;
	increase(ram,(LONG)m*sizeof(int));

	/* d is a vector needed by the algorithm */
	d=(double *)ram;
	increase(ram,(LONG)n*sizeof(double));

	/* gauss algorithm */
	for (k=0; k<n; k++)
	{	perm[k]=k;
		for (zmax=0.0, j=0; j<m; j++)
			if ( (help=i_abs(i_lumat[k][j])) >zmax) zmax=help;
		if (zmax==0.0) { error=130; return; }
		d[k]=zmax;
	}
	signdet=1; rank=0; i_det[0]=1.0; i_det[1]=0.0; k=0;
	for (kh=0; kh<m; kh++)
	{	piv=i_abs(i_lumat[k][kh])/d[k];
		j0=k;
		for (j=k+1; j<n; j++)
		{	temp=i_abs(i_lumat[j][kh])/d[j];
			if (piv<temp)
			{	piv=temp; j0=j;
			}
		}
		col[kh]=1; rank++;
		i_mult(i_det,i_lumat[j0][kh],i_det);
		if (i_det[0]<=0 && i_det[1]>=0)
		{	output("Determinant may be 0\n");
			error=131; return;
		}
		if (j0!=k)
		{	signdet=-signdet;
			mm=perm[j0]; perm[j0]=perm[k]; perm[k]=mm;
			temp=d[j0]; d[j0]=d[k]; d[k]=temp;
			temp1=i_lumat[j0]; i_lumat[j0]=i_lumat[k];
				i_lumat[k]=temp1;
		}
		for (j=k+1; j<n; j++)
			if (i_lumat[j][kh][0] != 0.0 || i_lumat[j][kh][1]!=0.0)
			{	i_div(i_lumat[j][kh],i_lumat[k][kh],i_lumat[j][kh]);
				if (error) return;
				for (mm=kh+1; mm<m; mm++)
				{	i_mult(i_lumat[j][kh],i_lumat[k][mm],t);
					i_sub(i_lumat[j][mm],t,i_lumat[j][mm]);
				}
			}
		k++;
		if (k>=n) { kh++; break; }
	}
	if (k<n || kh<m)
	{	signdet=0;
		error=131; output("Determinant zero!\n");
		return;
	}
	for (j=kh; j<m; j++) col[j]=0;
	if (signdet<0)
	{	i_det[0]=i_det[1]*signdet; i_det[1]=i_det[0]*signdet;
	}
}

void i_solvesim (double *a, int n, double *rs, int m, double *res)
/**** solvesim
	solve simultanuously a linear system.
****/
{	interval **x,**b,*h;
	interval sum,t;
	int i,k,l,j;
	ram=newram;
	luflag=0; i_lu(a,n,n); if (error) return;

	/* initialize x and b */
	x=(interval **)ram;
	increase(ram,(LONG)n*sizeof(interval *));
	h=(interval *)res; for (i=0; i<n; i++) { x[i]=h; h+=m; }

	b=(interval **)ram;
	increase(ram,(LONG)n*sizeof(interval *));
	h=(interval *)rs; for (i=0; i<n; i++) { b[i]=h; h+=m; }

	for (l=0; l<m; l++)
	{	i_copy(x[0][l],b[perm[0]][l]);
		for (k=1; k<n; k++)
		{	i_copy(x[k][l],b[perm[k]][l]);
			for (j=0; j<k; j++)
			{	i_mult(i_lumat[k][j],x[j][l],t);
				i_sub(x[k][l],t,x[k][l]);
			}
		}
		i_div(x[n-1][l],i_lumat[n-1][n-1],x[n-1][l]);
		for (k=n-2; k>=0; k--)
		{	sum[0]=0; sum[1]=0.0;
			for (j=k+1; j<n; j++)
			{	i_mult(i_lumat[k][j],x[j][l],t);
				i_add(sum,t,sum);
			}
			i_sub(x[k][l],sum,t);
			i_div(t,i_lumat[k][k],x[k][l]);
		}
	}
}

extern double *polynom,*divx,*divdif;
extern int degree;

void ipeval (double *xa, double *xb, double *za, double *zb)
{	int i;
	double *p,ha,hb;
	p=polynom+(2l*degree);
	*za=*p; *zb=*(p+1);
	p-=2;
	for (i=degree-1; i>=0; i--)
	{	interval_mult(xa,xb,za,zb,&ha,&hb);
		interval_add(&ha,&hb,p,p+1,za,zb);
		p-=2;
	}
}

void iddeval (double *x, double *xi, double *z, double *zi)
{	int i;
	double *p,h,hi,*dd,xh,xhi;
	p=divdif+(2l*degree);
	dd=divx+(2l*(degree-1));
	*z=*p; *zi=*(p+1);
	p-=2;
	for (i=degree-1; i>=0; i--)
	{	xh=round_down(*x-*(dd+1));
		xhi=round_up(*xi-*dd);
		dd-=2;
		interval_mult(&xh,&xhi,z,zi,&h,&hi);
		*z=round_down(h+*p);
		*zi=round_up(hi+*(p+1));
		p-=2;
	}
}

void mexpand (header *hd)
{	header *st=hd,*hd1=nextof(hd),*result;
	double *m,*mr,x,d,f;
	int c,r;
	long n;
	hd=getvalue(hd); hd1=getvalue(hd1); if (error) return;
	if (!hd1->type==s_real || (f=*realof(hd1))<=0)
	{	output("expand by a positiv scalar only!\n");
		error=1; return;
	}
	if (hd->type==s_interval)
	{	d=(*bof(hd)-*aof(hd))/2; x=*aof(hd)+d;
		result=new_interval(
			round_down(x-d*f),round_up(x+d*f),""); if (error) return;
	}
	else if (hd->type==s_imatrix)
	{	getmatrix(hd,&c,&r,&m);
		result=new_imatrix(c,r,""); if (error) return;
		mr=matrixof(result);
		n=(long)r*c;
		while (n>0)
		{	d=(*(m+1)-*m)/2; x=*m+d;
			*mr++=round_down(x-d*f); *mr++=round_up(x+d*f); m+=2;
			n--;
		}
	}
	else if (hd->type==s_real)
	{	result=new_interval(round_down(*realof(hd)-f),
				round_up(*realof(hd)+f),"");
		if (error) return;
	}
	else if (hd->type==s_matrix)
	{	getmatrix(hd,&c,&r,&m);
		result=new_imatrix(c,r,""); if (error) return;
		mr=matrixof(result);
		n=(long)r*c;
		while (n>0)
		{	*mr++=round_down(*m-f); *mr++=round_up(*m++ +f);
			n--;
		}
	}
	else wrong_arg();
	moveresult(st,result);
}

void plusminus (header *hd, header *hd1)
{	
	header *result=0;
	int k,i,j,n;
	double *m,*m1;
	hd1=getvalue(hd1); if (error) return;
	if (hd1->type==s_real)
	{	result=new_interval(-fabs(*realof(hd1)),fabs(*realof(hd1)),"");
	}
	else if (hd1->type==s_matrix)
	{	
		i=dimsof(hd1)->r; j=dimsof(hd1)->c;
		result=new_imatrix(i,j,"");
		if (error) return;
		n=(long)i*j;
		m=matrixof(result);
		m1=matrixof(hd1);
		for (k=0; k<n; k++)
		{	*m++=-fabs(*m1);
			*m++=fabs(*m1++);
		}
	}
	else need_arg_in("plusminus","two real numbers");
	moveresult(hd1,result);
	add(hd,hd1);
}
