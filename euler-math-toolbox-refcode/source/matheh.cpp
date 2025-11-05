#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "header.h"
#include "sysdep.h"
#include "matheh.h"
#include "stack.h"
#include "getvalue.h"

char *ram;
int *perm,*col,signdet,luflag=0;
double **lumat,*c,det;
complex **c_lumat,*c_c,c_det;
int rank;

static long nmark;

#define superalign(n) ((((n)-1)/ALIGNMENT+1)*ALIGNMENT)
#define increase(r,n) nmark=superalign(n); if (!freeramfrom((r),(nmark))) outofram(); (r)+=nmark;

/***************** real linear systems *******************/

void lu (double *a, int n, int m)
/***** lu
	lu decomposition of a
*****/
{	int i,j,k,mm,j0,kh,j1;
	double *d,piv,temp,*temp1;

	if (!luflag)
	{
		/* get place for result c and move a to c */
		c=(double *)ram;
		increase(ram,(LONG)n*m*sizeof(double));
		memmove((char *)c,(char *)a,(LONG)n*m*sizeof(double));
	}
	else c=a;

	/* inititalize lumat */
	lumat=(double **)ram;
	increase(ram,(LONG)n*sizeof(double *));
	d=c;
	for (i=0; i<n; i++) { lumat[i]=d; d+=m; }

	/* get place for perm */
	perm=(int *)ram;
	increase(ram,(LONG)n*sizeof(int));

	/* get place for col */
	col=(int *)ram;
	increase(ram,(LONG)m*sizeof(int));

	/* gauss algorithm */
	for (k=0; k<n; k++) perm[k]=k;
	signdet=1; rank=0; det=1.0; k=0;
	for (kh=0; kh<m; kh++)
	{   piv=fabs(lumat[k][kh]);
		if (!luflag)
		{	for (j=kh+1; j<m; j++) piv+=fabs(lumat[k][j]);
			if (piv==0)
			{	output("Determinant zero!\n");
				error=131; return;
			}
			piv=fabs(lumat[k][kh])/piv;
		}
		j0=k;
		for (j=k+1; j<n; j++)
		{	temp=fabs(lumat[j][kh]);
			if (!luflag)
			{	for (j1=kh+1; j1<m; j1++) temp+=fabs(lumat[j][j1]);
				if (temp==0)
				{	output("Determinant zero!\n");
					error=131; return;
				}
				temp=fabs(lumat[j][kh])/temp;
			}
			if (piv<temp)
			{	piv=temp; j0=j;
			}
		}
		if (luflag && piv<epsilon)
		{	signdet=0;
			col[kh]=0;
		}
		else
		{	col[kh]=1; rank++;
			det*=lumat[j0][kh];
			if (!luflag && det==0)
			{	output("Determinant zero!\n");
				error=131; return;
			}
			if (j0!=k)
			{	signdet=-signdet;
				mm=perm[j0]; perm[j0]=perm[k]; perm[k]=mm;
				temp1=lumat[j0]; lumat[j0]=lumat[k]; lumat[k]=temp1;
			}
			for (j=k+1; j<n; j++)
				if (lumat[j][kh] != 0.0)
				{	lumat[j][kh] /= lumat[k][kh];
					for (temp=lumat[j][kh], mm=kh+1; mm<m; mm++)
						lumat[j][mm]-=temp*lumat[k][mm];
				}
			k++;
			if (k>=n) { kh++; break; }
        }
	}
	if (k<n || kh<m)
	{	signdet=0;
		if (!luflag)
		{	error=131; output("Determinant zero!\n");
			return;
		}
	}
	for (j=kh; j<m; j++) col[j]=0;
	det=det*signdet;
	if (!luflag && fabs(det)<epsilon)
	{	error=131; output("Determinant zero!\n");
		return;
	}
}


void solvesim (double *a, int n, double *rs, int m, double *res)
/**** solvesim
	solve simultanuously a linear system.
****/
{	double **x,**b,*h,sum;
	int i,k,l,j;
	ram=newram; luflag=0;
	lu(a,n,n); if (error) return;

	/* initialize x and b */
	x=(double **)ram;
	increase(ram,(LONG)n*sizeof(double *));
	h=res; for (i=0; i<n; i++) { x[i]=h; h+=m; }
	b=(double **)ram;
	increase(ram,(LONG)n*sizeof(double *));
	h=rs; for (i=0; i<n; i++) { b[i]=h; h+=m; }

	for (l=0; l<m; l++)
	{	x[0][l]=b[perm[0]][l];
		for (k=1; k<n; k++)
		{	x[k][l]=b[perm[k]][l];
			for (j=0; j<k; j++)
				x[k][l] -= lumat[k][j]*x[j][l];
		}
		x[n-1][l] /= lumat[n-1][n-1];
		for (k=n-2; k>=0; k--)
		{	for (sum=0.0, j=k+1; j<n; j++)
				sum+=lumat[k][j]*x[j][l];
			x[k][l] = (x[k][l]-sum)/lumat[k][k];
		}
	}
}

void make_lu (double *a, int n, int m,
	int **rows, int **cols, int *rankp,
	double *detp)
{	ram=newram;
	luflag=1; lu(a,n,m); newram=ram;
	if (error) return;
	*rows=perm; *cols=col; *rankp=rank; *detp=det;
}

void lu_solve (double *a, int n, double *rs, int m, double *res)
{	double **x,**b,*h,sum,*d;
	int i,k,l,j;
	ram=newram;
	
	/* initialize x and b */
	x=(double **)ram;
	increase(ram,(LONG)n*sizeof(double *));
	h=res; for (i=0; i<n; i++) { x[i]=h; h+=m; }

	b=(double **)ram;
	increase(ram,(LONG)n*sizeof(double *));
	h=rs; for (i=0; i<n; i++) { b[i]=h; h+=m; }
	
	/* inititalize lumat */
	lumat=(double **)ram;
	increase(ram,(LONG)n*sizeof(double *));
	d=a;
	for (i=0; i<n; i++) { lumat[i]=d; d+=n; }
	
	for (l=0; l<m; l++)
	{	x[0][l]=b[0][l];
		for (k=1; k<n; k++)
		{	x[k][l]=b[k][l];
			for (j=0; j<k; j++)
				x[k][l] -= lumat[k][j]*x[j][l];
		}
		x[n-1][l] /= lumat[n-1][n-1];
		for (k=n-2; k>=0; k--)
		{	for (sum=0.0, j=k+1; j<n; j++)
				sum+=lumat[k][j]*x[j][l];
			x[k][l] = (x[k][l]-sum)/lumat[k][k];
		}
	}
}

/******************* complex linear systems **************/

void c_add (complex x, complex y, complex z)
{	z[0]=x[0]+y[0];
	z[1]=x[1]+y[1];
}

void c_sub (complex x, complex y, complex z)
{	z[0]=x[0]-y[0];
	z[1]=x[1]-y[1];
}

void c_mult (complex x, complex y, complex z)
{	double h;
	h=x[0]*y[0]-x[1]*y[1];
	z[1]=x[0]*y[1]+x[1]*y[0];
	z[0]=h;
}

void c_div (complex x, complex y, complex z)
{	double r,h;
	r=y[0]*y[0]+y[1]*y[1];
	h=(x[0]*y[0]+x[1]*y[1])/r;
	z[1]=(x[1]*y[0]-x[0]*y[1])/r;
	z[0]=h;
}

double c_abs (complex x)
{	return sqrt(x[0]*x[0]+x[1]*x[1]);
}

void c_copy (complex x, complex y)
{	x[0]=y[0];
	x[1]=y[1];
}

void c_lu (double *a, int n, int m)
/***** clu
	lu decomposition of a
*****/
{	int i,j,k,mm,j0,kh,j1;
	double piv,temp;
	complex t,*h,*temp1;

	if (!luflag)
	{	/* get place for result c and move a to c */
		c_c=(complex *)ram;
		increase(ram,(LONG)n*m*sizeof(complex));
		memmove((char *)c_c,(char *)a,(LONG)n*m*sizeof(complex));
	}
	else c_c=(complex *)a;

	/* inititalize c_lumat */
	c_lumat=(complex **)ram;
	increase(ram,(LONG)n*sizeof(complex *));
	h=c_c;
	for (i=0; i<n; i++) { c_lumat[i]=h; h+=m; }

	/* get place for perm */
	perm=(int *)ram;
	increase(ram,(LONG)n*sizeof(int));

	/* get place for col */
	col=(int *)ram;
	increase(ram,(LONG)m*sizeof(int));

	for (k=0; k<n; k++) perm[k]=k;
	signdet=1; rank=0; c_det[0]=1.0; c_det[1]=0.0; k=0;
	for (kh=0; kh<m; kh++)
	{	piv=c_abs(c_lumat[k][kh]);
		if (!luflag)
		{	for (j=kh+1; j<m; j++)
			{	piv+=c_abs(c_lumat[k][j]);
			}
			if (piv==0)
			{	output("Determinant zero!\n");
				error=131; return;
			}
			piv=c_abs(c_lumat[k][kh])/piv;
		}
		j0=k;
		for (j=k+1; j<n; j++)
		{	temp=c_abs(c_lumat[j][kh]);
			if (!luflag)
			{	for (j1=kh+1; j1<m; j1++) temp+=c_abs(c_lumat[j][j1]);
				if (temp==0)
				{	output("Determinant zero!\n");
					error=131; return;
				}
				temp=c_abs(c_lumat[j][kh])/temp;
			}
			if (piv<temp)
			{	piv=temp; j0=j;
			}
		}
		if (luflag && piv<epsilon)
		{	signdet=0;
			if (luflag)
			{	col[kh]=0;
				continue;
			}
		}
		else
		{	col[kh]=1; rank++;
			c_mult(c_det,c_lumat[j0][kh],c_det);
			if (!luflag && c_det[0]==0 && c_det[1]==0)
			{	output("Determinant zero!\n");
				error=131; return;
			}
		}
		if (j0!=k)
		{	signdet=-signdet;
			mm=perm[j0]; perm[j0]=perm[k]; perm[k]=mm;
			temp1=c_lumat[j0]; c_lumat[j0]=c_lumat[k];
				c_lumat[k]=temp1;
		}
		for (j=k+1; j<n; j++)
			if (c_lumat[j][kh][0] != 0.0 || c_lumat[j][kh][1]!=0.0)
			{	c_div(c_lumat[j][kh],c_lumat[k][kh],c_lumat[j][kh]);
				for (mm=kh+1; mm<m; mm++)
				{	c_mult(c_lumat[j][kh],c_lumat[k][mm],t);
					c_sub(c_lumat[j][mm],t,c_lumat[j][mm]);
				}
			}
		k++;
		if (k>=n) { kh++; break; }
	}
	if (k<n || kh<m)
	{	signdet=0;
		if (!luflag)
		{	error=131; output("Determinant zero!\n");
			return;
		}
	}
	for (j=kh; j<m; j++) col[j]=0;
	c_det[0]=c_det[0]*signdet; c_det[1]=c_det[1]*signdet;
}

void c_solvesim (double *a, int n, double *rs, int m, double *res)
/**** solvesim
	solve simultanuously a linear system.
****/
{	complex **x,**b,*h;
	complex sum,t;
	int i,k,l,j;
	ram=newram;
	luflag=0; c_lu(a,n,n); if (error) return;
	
	/* initialize x and b */
	x=(complex **)ram;
	increase(ram,(LONG)n*sizeof(complex *));
	h=(complex *)res; for (i=0; i<n; i++) { x[i]=h; h+=m; }

	b=(complex **)ram;
	increase(ram,(LONG)n*sizeof(complex *));
	h=(complex *)rs; for (i=0; i<n; i++) { b[i]=h; h+=m; }
	
	for (l=0; l<m; l++)
	{	c_copy(x[0][l],b[perm[0]][l]);
		for (k=1; k<n; k++)
		{	c_copy(x[k][l],b[perm[k]][l]);
			for (j=0; j<k; j++)
			{	c_mult(c_lumat[k][j],x[j][l],t);
				c_sub(x[k][l],t,x[k][l]);
			}
		}
		c_div(x[n-1][l],c_lumat[n-1][n-1],x[n-1][l]);
		for (k=n-2; k>=0; k--)
		{	sum[0]=0; sum[1]=0.0;
			for (j=k+1; j<n; j++)
			{	c_mult(c_lumat[k][j],x[j][l],t);
				c_add(sum,t,sum);
			}
			c_sub(x[k][l],sum,t);
			c_div(t,c_lumat[k][k],x[k][l]);
		}
	}
}

void cmake_lu (double *a, int n, int m,
	int **rows, int **cols, int *rankp,
	double *detp, double *detip)
{	ram=newram;
	luflag=1; c_lu(a,n,m); newram=ram;
	if (error) return;
	*rows=perm; *cols=col; *rankp=rank; 
	*detp=c_det[0]; *detip=c_det[1];
}

void clu_solve (double *a, int n, double *rs, int m, double *res)
/**** solvesim
	solve simultanuously a linear system.
****/
{	complex **x,**b,*h;
	complex sum,t;
	int i,k,l,j;
	ram=newram;
	
	/* initialize x and b */
	x=(complex **)ram;
	increase(ram,(LONG)n*sizeof(complex *));
	h=(complex *)res; for (i=0; i<n; i++) { x[i]=h; h+=m; }
	
	b=(complex **)ram;
	increase(ram,(LONG)n*sizeof(complex *));
	h=(complex *)rs; for (i=0; i<n; i++) { b[i]=h; h+=m; }
	
	/* inititalize c_lumat */
	c_lumat=(complex **)ram;
	increase(ram,(LONG)n*sizeof(complex *));
	h=(complex *)a;
	for (i=0; i<n; i++) { c_lumat[i]=h; h+=n; }
	
	for (l=0; l<m; l++)
	{	c_copy(x[0][l],b[0][l]);
		for (k=1; k<n; k++)
		{	c_copy(x[k][l],b[k][l]);
			for (j=0; j<k; j++)
			{	c_mult(c_lumat[k][j],x[j][l],t);
				c_sub(x[k][l],t,x[k][l]);
			}
		}
		c_div(x[n-1][l],c_lumat[n-1][n-1],x[n-1][l]);
		for (k=n-2; k>=0; k--)
		{	sum[0]=0; sum[1]=0.0;
			for (j=k+1; j<n; j++)
			{	c_mult(c_lumat[k][j],x[j][l],t);
				c_add(sum,t,sum);
			}
			c_sub(x[k][l],sum,t);
			c_div(t,c_lumat[k][k],x[k][l]);
		}
	}
}

/************** bauhuber algorithm ***************/

#define ITERMAX 200
#define EPS (64*DBL_EPSILON)
#define QR 0.1
#define QI 0.8
#define EPSROOT (64*epsilon)
#define BETA (2096*EPSROOT)

void quadloes (double ar, double ai, double br, double bi,
	double cr, double ci, double *treal, double *timag)
{	
	double pr,pi,qr,qi,h;
	pr=br*br-bi*bi; pi=2*br*bi;
	qr=ar*cr-ai*ci; qi=ar*ci+ai*cr;
	pr=pr-4*qr; pi=pi-4*qi;
	h=sqrt(pr*pr+pi*pi);
	qr=h+pr; if (qr<0.0) qr=0; 
	qr=sqrt(qr/2);
	qi=h-pr; if (qi<0.0) qi=0; 
	qi=sqrt(qi/2);
	if (pi<0.0) qi=-qi;
	h=qr*br+qi*bi;
	if (h>0.0) { qr=-qr; qi=-qi; }
	pr=qr-br; pi=qi-bi;
	h=pr*pr+pi*pi;
	*treal=2*(cr*pr+ci*pi)/h;
	*timag=2*(ci*pr-cr*pi)/h;
}

int cxdiv (double ar, double ai, double br, double bi,
	double *cr, double *ci)
{	double temp;
	if (br==0.0 && bi==0.0) return 1;
	if (fabs(br)>fabs(bi))
	{	temp=bi/br; br=temp*bi+br;
		*cr=(ar+temp*ai)/br;
		*ci=(ai-temp*ar)/br;
	}
	else
	{	temp=br/bi; bi=temp*br+bi;
		*cr=(temp*ar+ai)/bi;
		*ci=(temp*ai-ar)/bi;
	}
	return 0;
}

double cxxabs (double ar, double ai)
{	if (ar==0.0) return fabs(ai);
	if (ai==0.0) return fabs(ar);
	return sqrt(ai*ai+ar*ar);
}

void chorner (int n, int iu, double *ar, double *ai,
	double xr, double xi, double *pr, double *pi,
	double *p1r, double *p1i, double *p2r, double *p2i,
	double *rf1)
{	register int i,j;
	int i1;
	double temp,hh,tempr=0.0,tempi=0.0;
	*pr=ar[n]; *pi=ai[n];
	*p1r=*p2r=0.0; *p1i=*p2i=0.0;
	*rf1=cxxabs(*pr,*pi);
	i1=n-iu;
	for (j=n-iu,i=n-1; i>=iu; i--,j--)
	{	if (i<n-1)
		{	tempr=*p1r; tempi=*p1i;
			*p1r=*p1r * xr - *p1i * xi;
			*p1i=*p1i * xr + tempr * xi;
		}
		*p1r+=*pr; *p1i+=*pi;
		temp=*pr;
		*pr=*pr * xr - *pi * xi + ar[i];
		*pi=*pi * xr + temp * xi + ai[i];
		temp=cxxabs(*p1r,*p1i);
		hh=cxxabs(*pr,*pi); if (hh>temp) temp=hh;
		if (temp>*rf1)
		{	*rf1=temp; i1=j-1;
		}
		if (i<n-1)
		{	temp=*p2r;
			*p2r=*p2r * xr - *p2i * xi + tempr*2;
			*p2i=*p2i * xr + temp * xi + tempi*2;
		}
	}
	temp=cxxabs(xr,xi);
	if (temp!=0.0)
		*rf1=pow(temp,(double)i1)*(i1+1);
	else
		*rf1=cxxabs(*p1r,*p1i);
	*rf1*=EPS;
}

void scpoly (int n, double *ar, double *ai, double *scal)
{	double p,h;
	int i;
	*scal=0.0;
	p=cxxabs(ar[n],ai[n]);
	for (i=0; i<n; i++)
	{	ai[i]/=p; ar[i]/=p;
		h=pow(cxxabs(ar[i],ai[i]),1.0/(n-i));
		if (h>*scal) *scal=h;
	}
	ar[n]/=p; ai[n]/=p;
	if (*scal==0.0) *scal=1.0;
	for (p=1.0,i=n-1; i>=0; i--)
	{	p*= *scal;
		ar[i]/=p; ai[i]/=p;
	}
}

void bauroot (int n, int iu, double *ar, double *ai, 
	double *x0r, double *x0i)
{	
	int count=0;
	start :
	int iter=0,i=0,aborted=0;
	double xoldr,xoldi,xnewr,xnewi,h,h1,h2,h3,h4,dzmax,dzmin,
		dxr=1,dxi=0,tempr,tempi,abs_pold,abs_pnew,abs_p1new,
		temp,ss,u,v,
		pr,pi,p1r,p1i,p2r,p2i,abs_pnoted=1e20;
		
	dxr=dxi=xoldr=xoldi=0.0;
	if (n-iu==1)
	{	
		quadloes(0.0,0.0,ar[n],ai[n],
			ar[n-1],ai[n-1],x0r,x0i);
		goto stop;
	}
	if (n-iu==2)
	{	
		quadloes(ar[n],ai[n],ar[n-1],ai[n-1],
			ar[n-2],ai[n-2],x0r,x0i);
		goto stop;
	}
	xnewr=*x0r; xnewi=*x0i;
	chorner(n,iu,ar,ai,xnewr,xnewi,&pr,&pi,&p1r,&p1i,&p2r,&p2i,&ss);
	iter++;
	abs_pnew=cxxabs(pr,pi);
	if (abs_pnew==0) { goto stop; }
	abs_pold=abs_pnew;
	dzmin=BETA*(1+cxxabs(xnewr,xnewi));
	while (!aborted)
	{	abs_p1new=cxxabs(p1r,p1i);
		iter++;
		if (abs_pnew>abs_pold) /* Spiraling */
		{	i=0;
			temp=dxr;
			dxr=QR*dxr-QI*dxi;
			dxi=QR*dxi+QI*temp;
		}
		else /* Newton step */
		{	
			dzmax=1.0+cxxabs(xnewr,xnewi);
			h1=p1r*p1r-p1i*p1i-pr*p2r+pi*p2i;
			h2=2*p1r*p1i-pr*p2i-pi*p2r;
			if (abs_p1new>10*ss && cxxabs(h1,h2)>100*ss*ss)
				/* do a Newton step */
			{	i++;
				if (i>2) i=2;
				tempr=pr*p1r-pi*p1i;
				tempi=pr*p1i+pi*p1r;
				cxdiv(-tempr,-tempi,h1,h2,&dxr,&dxi);
				if (cxxabs(dxr,dxi)>dzmax)
				{	temp=dzmax/cxxabs(dxr,dxi);
					dxr*=temp; dxi*=temp;
					i=0;
				}
				if (i==2 && cxxabs(dxr,dxi)<dzmin/EPSROOT &&
					cxxabs(dxr,dxi)>0)
				{	i=0;
					cxdiv(xnewr-xoldr,xnewi-xoldi,dxr,dxi,&h3,&h4);
					h3+=1;
					h1=h3*h3-h4*h4;
					h2=2*h3*h4;
					cxdiv(dxr,dxi,h1,h2,&h3,&h4);
					if (cxxabs(h3,h4)<50*dzmin)
					{	dxr+=h3; dxi+=h4;
					}
				}
				xoldr=xnewr; xoldi=xnewi;
				abs_pold=abs_pnew;
			}
			else /* saddle point, minimize into direction pr+i*pi */
			{	i=0;
				h=dzmax/abs_pnew;
				dxr=h*pr; dxi=h*pi;
				xoldr=xnewr; xoldi=xnewi;
				abs_pold=abs_pnew;
				do
				{	chorner(n,iu,ar,ai,xnewr+dxr,xnewi+dxi,&u,&v,
						&h,&h1,&h2,&h3,&h4);
					dxr*=2; dxi*=2;
				}
				while (fabs(cxxabs(u,v)/abs_pnew-1)<EPSROOT);
			}
		} /* end of Newton step */
		xnewr=xoldr+dxr;
		xnewi=xoldi+dxi;
		dzmin=BETA*(1+cxxabs(xoldr,xoldi));
		chorner(n,iu,ar,ai,xnewr,xnewi,&pr,&pi,
			&p1r,&p1i,&p2r,&p2i,&ss);
		abs_pnew=cxxabs(pr,pi);
		if (abs_pnew==0.0) break;
		if (cxxabs(dxr,dxi)<dzmin && abs_pnew<1e-5
			&& iter>5) break;
		if (iter>ITERMAX)
		{	iter=0;
			if (abs_pnew>=abs_pnoted)
			{	if (count>10)
				{	xnewr=xoldr; xnewi=xoldi;
					break;
				}
				else 
				{	*x0r=xnewr*(1+rand()/10000);
					*x0i=xnewi*(1+rand()/10000);
					count++;
					goto start;
				}
			}
			abs_pnoted=abs_pnew;
			if (test_key()==escape) { error=700; return; }
		}
	}
	*x0r=xnewr; *x0i=xnewi;
	stop: ;
/*
	chorner(n,iu,ar,ai,*x0r,*x0i,&pr,&pi,&p1r,&p1i,&p2r,&p2i,&ss);
	abs_pnew=cxxabs(pr,pi);
	printf("%20.5e +i* %20.5e, %20.5e\n",
		*x0r,*x0i,abs_pnew);
*/
}

static void polydiv (int n, int iu, double *ar, double *ai,
	double x0r, double x0i)
{	int i;
	for (i=n-1; i>iu; i--)
	{	ar[i]+=ar[i+1]*x0r-ai[i+1]*x0i;
		ai[i]+=ai[i+1]*x0r+ar[i+1]*x0i;
	}
}

void bauhuber (double *p, int n, double *result, int all,
	double startr, double starti)
{	
	double *ar,*ai,scalefak=1.0;
	int i;
	double x0r,x0i;
	ram=newram;
	if (!freeram(2*(n+1)*sizeof(double))) outofram();
	ar=(double *)ram;
	ai=ar+n+1;
	for (i=0; i<=n; i++)
	{	ar[i]=p[2*i];
		ai[i]=p[2*i+1];
	}
/*	scpoly(n,ar,ai,&scalefak); */
	/* scalefak=1; */
	x0r=startr; x0i=starti;
	if (all)
	{	for (i=0; i<n; i++)
		{	bauroot(n,i,ar,ai,&x0r,&x0i);
			ar[i]=scalefak*x0r;
			ai[i]=scalefak*x0i;
			if (error) 
			{	output("Bauhuber-Iteration failed!\n"); 
				error=311; return;
			}
			polydiv(n,i,ar,ai,x0r,x0i);
			x0i=-x0i;
		}
		for (i=0; i<n; i++)
		{	result[2*i]=ar[i]; result[2*i+1]=ai[i];
		}
	}
	else
	{	bauroot(n,0,ar,ai,&x0r,&x0i);
		result[0]=scalefak*x0r; 
		result[1]=scalefak*x0i;
	}
}

/**************** tridiagonalization *********************/

double **mg;

void tridiag ( double *a, int n, int **rows)
/***** tridiag
	tridiag. a with n rows and columns.
	r[] contains the new indices of the rows.
*****/
{	char *ram=newram;
	int rh;
	double **m,maxi,*mh,lambda,h;
	int i,j,ipiv,ik,jk,k,*r;
	
	/* make a pointer array to the rows of m : */
	m=(double **)ram; increase(ram,n*sizeof(double *));
	for (i=0; i<n; i++) { m[i]=a; a+=n; }
	r=(int *)ram; increase(ram,n*sizeof(double *)); *ram=0;
	for (i=0; i<n; i++) r[i]=i;
	
	/* start algorithm : */
	for (j=0; j<n-2; j++) /* need only go the (n-2)-th column */
	{	/* determine pivot */
		jk=r[j]; maxi=fabs(m[j+1][jk]); ipiv=j+1;
		for (i=j+2; i<n; i++)
		{	h=fabs(m[i][jk]);
			if (h>maxi) { maxi=h; ipiv=i; }
		}
		if (maxi<epsilon) continue;
		/* exchange with pivot : */
		if (ipiv!=j+1)
		{	mh=m[j+1]; m[j+1]=m[ipiv]; m[ipiv]=mh;
			rh=r[j+1]; r[j+1]=r[ipiv]; r[ipiv]=rh;
		}
		/* zero elements */
		for (i=j+2; i<n; i++)
		{	jk=r[j]; m[i][jk]=lambda=-m[i][jk]/m[j+1][jk];
			for (k=j+1; k<n; k++) 
			{	ik=r[k]; m[i][ik]+=lambda*m[j+1][ik];
			}
			/* same for columns */
			jk=r[j+1]; ik=r[i];
			for (k=0; k<n; k++) m[k][jk]-=lambda*m[k][ik];
		}
	}
	*rows=r; mg=m;
}

complex **cmg;

void ctridiag ( double *ca, int n, int **rows)
/***** tridiag
	tridiag. a with n rows and columns.
	r[] contains the new indices of the rows.
*****/
{	char *ram=newram;
	int rh;
	complex **m,*mh,lambda,*a=(complex *)ca,help;
	double maxi,h;
	int i,j,ipiv,ik,jk,k,*r;
	
	/* make a pointer array to the rows of m : */
	m=(complex **)ram; increase(ram,n*sizeof(double *));
	for (i=0; i<n; i++) { m[i]=a; a+=n; }
	r=(int *)ram; increase(ram,n*sizeof(complex *)); *ram=0;
	for (i=0; i<n; i++) r[i]=i;
	
	/* start algorithm : */
	for (j=0; j<n-2; j++) /* need only go the (n-2)-th column */
	{	/* determine pivot */
		jk=r[j]; maxi=c_abs(m[j+1][jk]); ipiv=j+1;
		for (i=j+2; i<n; i++)
		{	h=c_abs(m[i][jk]);
			if (h>maxi) { maxi=h; ipiv=i; }
		}
		if (maxi<epsilon) continue;
		/* exchange with pivot : */
		if (ipiv!=j+1)
		{	mh=m[j+1]; m[j+1]=m[ipiv]; m[ipiv]=mh;
			rh=r[j+1]; r[j+1]=r[ipiv]; r[ipiv]=rh;
		}
		/* zero elements */
		for (i=j+2; i<n; i++)
		{	jk=r[j];
			c_div(m[i][jk],m[j+1][jk],lambda);
			lambda[0]=-lambda[0]; lambda[1]=-lambda[1];
			c_copy(m[i][jk],lambda);
			for (k=j+1; k<n; k++)
			{	ik=r[k];
				c_mult(lambda,m[j+1][ik],help);
				c_add(m[i][ik],help,m[i][ik]);
			}
			/* same for columns */
			jk=r[j+1]; ik=r[i];
			for (k=0; k<n; k++)
			{	c_mult(lambda,m[k][ik],help);
				c_sub(m[k][jk],help,m[k][jk]);
			}
		}
	}
	*rows=r; cmg=m;
}

void charpoly (double *m1, int n, double *p)
/***** charpoly
	compute the chracteristic polynomial of m.
*****/
{	int i,j,k,*r;
	double **m,h1,h2;
	tridiag(m1,n,&r); m=mg; /* unusual global variable handling */
	/* compute the p_n rekursively : */
	m[0][r[0]]=-m[0][r[0]]; /* first one is x-a(0,0). */
	for (j=1; j<n; j++)
	{	m[0][r[j]]=-m[0][r[j]];
		for (k=1; k<=j; k++)
		{	h1=-m[k][r[j]]; h2=m[k][r[k-1]]; 
			for (i=0; i<k; i++) 
				m[i][r[j]]=m[i][r[j]]*h2+m[i][r[k-1]]*h1;
			m[k][r[j]]=h1;
		}
		for (i=0; i<j; i++) m[i+1][r[j]]+=m[i][r[j-1]];
	}
	for (i=0; i<n; i++) p[i]=m[i][r[n-1]];
	p[n]=1.0;
}

void ccharpoly (double *m1, int n, double *p)
/***** charpoly
	compute the chracteristic polynomial of m.
*****/
{	int *r,i,j,k;
	complex **m,h1,h2,g1,g2,*pc=(complex *)p;
	ctridiag(m1,n,&r); m=cmg; /* unusual global variable handling */
	/* compute the p_n rekursively : */
	m[0][r[0]][0]=-m[0][r[0]][0];
	m[0][r[0]][1]=-m[0][r[0]][1]; /* first one is x-a(0,0). */
	for (j=1; j<n; j++)
	{	m[0][r[j]][0]=-m[0][r[j]][0];
		m[0][r[j]][1]=-m[0][r[j]][1];
		for (k=1; k<=j; k++)
		{	h1[0]=-m[k][r[j]][0]; h1[1]=-m[k][r[j]][1]; 
			c_copy(h2,m[k][r[k-1]]); 
			for (i=0; i<k; i++)
			{	c_mult(h2,m[i][r[j]],g1);
				c_mult(h1,m[i][r[k-1]],g2);
				c_add(g1,g2,m[i][r[j]]);
			}
			c_copy(m[k][r[j]],h1);
		}
		for (i=0; i<j; i++)
		{	c_add(m[i+1][r[j]],m[i][r[j-1]],m[i+1][r[j]]);
		}
	}
	for (i=0; i<n; i++) c_copy(pc[i],m[i][r[n-1]]);
	pc[n][0]=1.0; pc[n][1]=0.0;
}

/***************** jacobi-givens eigenvalues **************/


double rotate (double *m, int j, int k, int n)
{
	double theta,t,s,c,tau,h,pivot;
	int l;
	pivot=*mat(m,n,j,k);
	if (fabs(pivot)<epsilon) return 0;
	theta=(*mat(m,n,j,j)-*mat(m,n,k,k))/(2*pivot);
	t=1/(fabs(theta)+sqrt(1+theta*theta));
	if (theta<0) t=-t;
	c=1/sqrt(1+t*t); s=t*c;
	tau=s/(1+c);
	for (l=0; l<n; l++)
	{
		if (l==j || l==k) continue;
		h=*mat(m,n,l,j);
		*mat(m,n,j,l)=*mat(m,n,l,j)=h+s*(*mat(m,n,l,k)-tau*h);
		*mat(m,n,l,k)-=s*(h+tau* *mat(m,n,l,k));
		*mat(m,n,k,l)=*mat(m,n,l,k);
	}
	*mat(m,n,j,j)+=t*pivot;
	*mat(m,n,k,k)-=t*pivot;
	*mat(m,n,j,k)=*mat(m,n,k,j)=0;
	return fabs(pivot);
}

void mjacobi (header *hd)
{
	header *st=hd,*result,*hd1;
	double *m,max,neumax,*mr;
	int r,c,i,j;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix) need_arg_in("real symmetric matrix","jacobi");
	getmatrix(hd,&r,&c,&m);
	if (r!=c) need_arg_in("real symmetric matrix","jacobi");
	if (r<2) { moveresult(st,hd); return; }
	hd1=new_matrix(r,r,""); if (error) return;
	m=matrixof(hd1);
	memmove(m,matrixof(hd),(long)r*r*sizeof(double));
	while(1)
	{
		max=0.0;
		for (i=0; i<r-1; i++)
			for (j=i+1; j<r; j++)
				if ((neumax=rotate(m,i,j,r))>max)
					max=neumax;
		if (max<epsilon) break;
	}
	result=new_matrix(1,r,""); if (error) return;
	mr=matrixof(result);
	for (i=0; i<r; i++) *mr++=*mat(m,r,i,i);
	moveresult(st,result);
}


/*************** simplex-method *********************/

// #define SIMPLEXTEST

void exchange (int i0, int j0, int m, int n, int *iv,
	double **a, double *b, double *c)
{
	int i,j;
	double h;
	a[i0][j0]=h=1/a[i0][j0];
	for (j=0; j<n; j++)
		if (j!=j0) a[i0][j]*=h;
	b[i0]*=h;
	for (i=0; i<m; i++)
		if (i!=i0)
		{	h=a[i][j0];
			for (j=0; j<n; j++)
				if (j!=j0) a[i][j]-=h*a[i0][j];
			b[i]-=h*b[i0];
		}
	h=c[j0];
	for (j=0; j<n; j++)
		if (j!=j0) c[j]-=h*a[i0][j];
	h=a[i0][j0];
	for (i=0; i<m; i++)
		if (i!=i0) a[i][j0]*=-h;
	c[j0]*=-h;
	i=iv[j0]; iv[j0]=iv[n+i0]; iv[n+i0]=i;
}

void printsimplex (int m, int n, int *iv,
	double **a, double *b, double *c)
{
	int i,j;
	print("%13s","");
	for (j=0; j<n; j++) print("%12d ",iv[j]>=0?iv[j]+1:iv[j]);
	print("\n");
	for (i=0; i<m; i++)
	{
		print("%12d ",iv[n+i]>=0?iv[n+i]+1:iv[n+i]);
		for (j=0; j<n; j++) print("%12.4f ",a[i][j]);
		print(" %12.4f\n",b[i]);
	}
	print("%13s","");
	for (j=0; j<n; j++) print("%12.4f ",c[j]);
	print("\n\n");
}

int simplex (int m, int n, int *iv,
	double **a, double *b, double *c, double *x,
	int *eq, int *re, int opt)
/*
Minimize c'x under all x with a*x<=b, x>=0.
iv[0]..iv[n-1] contain the variable indices of the rows,
iv[n]..iv[n+m-1] contain the variable indices of the columns.
eq: contains the direction of the equations (-1,0,1)
re: contains 0 or 1 for unrestricted or restricted variables
opt: direction of optimization
a,b and c will be changed here.
Results: -1 unfeasable, 1 unbounded, 0 solution found
This procedure uses Bland's anticycling rule.
*/
{	
	int i,i0,iv0,j,j0,jv0;
	double mi,h;
	int res=0;

	for (j=0; j<n; j++) x[j]=0;

#ifdef SIMPLEXTEST
	print("Start\n\n");
	printsimplex(m,n,iv,a,b,c);
#endif

	// Balance the lines
	for (i=0; i<m; i++)
	{	
		h=fabs(b[i]);
		for (j=0; j<n; j++) h+=fabs(a[i][j]);
		if (h>epsilon)
		{	
			b[i]/=h;
			for (j=0; j<n; j++) a[i][j]/=h;
		}
		if (eq[i]>0)
		{
			for (j=0; j<n; j++) a[i][j]=-a[i][j];
			b[i]=-b[i];
		}
	}
	for (j=0; j<n; j++)
		if (re[j]<0)
		{
			for (i=0; i<m; i++) a[i][j]=-a[i][j];
			c[j]=-c[j];
		}

#ifdef SIMPLEXTEST
	print("After Balancing and adjusting >=\n\n");
	printsimplex(m,n,iv,a,b,c);
#endif

	// Exchange unrestricted variables immediately
	for (j=0; j<n; j++)
		if (!re[j])
		{
			h=0; i0=-1;
			// Find maximal value in unrestricted column
			for (i=0; i<m; i++)
				if (iv[n+i]>=0 && fabs(a[i][j])>h)
				{
					h=fabs(a[i][j]); i0=i;
				}
			if (i0>=0)
			{
				exchange(i0,j,m,n,iv,a,b,c);
				iv[n+i0]=-iv[n+i0]-1;
				if (eq[i0]==0) iv[j]=-iv[j]-1;
			}
			else // only zeros
			{
				if (opt>=0)
				{
					if (c[j]>epsilon) return 1;
					else iv[j]=-iv[j]-1;
				}
				else if (opt<0)
				{
					if (c[j]<-epsilon) return 1;
					else iv[j]=-iv[j]-1;
				}
			}
		}

#ifdef SIMPLEXTEST
	print("After exchanging unrestricted variables\n\n");
	printsimplex(m,n,iv,a,b,c);
#endif

	for (i=0; i<m; i++)
	{
		if (eq[i]==0 && iv[n+i]>=0)
		{
			j0=-1; h=0;
			for (j=0; j<n; j++)
			{
				if (iv[j]>=0 && fabs(a[i][j])>h)
				{
					j0=j; h=fabs(a[i][j]);
				}
			}
			if (j0>=0)
			{
				exchange(i,j0,m,n,iv,a,b,c);
				iv[j0]=-iv[j0]-1;
			}
			else
			{
				if (fabs(b[i])>epsilon) return -1;
				else iv[n+i]=-iv[n+i]-1;
			}
		}
	}

#ifdef SIMPLEXTEST
	print("After correcting equations\n\n");
	printsimplex(m,n,iv,a,b,c);
#endif

	// Search a feasible starting point
	while (1)
	{
		/* Search for a row with negative b[i],
		take least index on tie */
		iv0=n+m; i0=m;
		for (i=0; i<m; i++)
			if (iv[n+i]>=0 && b[i]<-epsilon && iv[n+i]<iv0) 
			{ 
				i0=i; iv0=iv[n+i]; 
			}
		if (i0>=m) 
		{
			// print("Feasible point found.\n");
			break;
		}
		// print("Correct row %d.\n",i0);
		/* Search for a negative lambda */
		for (j=0; j<n; j++)
			if (iv[j]>=0 && a[i0][j]<-epsilon) break;
		if (j>=n) return -1; // no feasible point
		/* Search for a possible column,
		take least variable index on tie */
		jv0=iv[j]; j0=j; j++;
		for (; j<n; j++)
		{	
			if (iv[j]>=0 && a[i0][j]<-epsilon)
			{	if (iv[j]<jv0)
				{	j0=j; jv0=iv[j];
				}
			}
		}
		/* Exchange column j0 and i0 */
		exchange(i0,j0,m,n,iv,a,b,c);
#ifdef SIMPLEXTEST
		printsimplex(m,n,iv,a,b,c);
#endif
		if (test_key()==escape) { error=301; return 2; }
	}

#ifdef SIMPLEXTEST
	print("After Feasible point\n\n");
	printsimplex(m,n,iv,a,b,c);
#endif

	if (opt==0) goto solution;

	/* Search optimum */
	while (1)
	{
		/* Search for a profitable column,
		take least index on tie */
		jv0=n+m; j0=n;
		if (opt<0)
		{
			for (j=0; j<n; j++)
				if (iv[j]>=0 && c[j]<-epsilon && iv[j]<jv0) 
				{ 
					j0=j; jv0=iv[j]; 
				}
		}
		else
		{
			for (j=0; j<n; j++)
				if (iv[j]>=0 && c[j]>epsilon && iv[j]<jv0) 
				{ 
					j0=j; jv0=iv[j]; 
				}
		}
		if (j0>=n) break;
		/* Search for a positive lambda */
		for (i=0; i<m; i++)
			if (iv[n+i]>=0 && a[i][j0]>epsilon) break;
		if (i>=m)
		{
			res=1;
			break;
		}
		/* Search for a possible row,
		take least variable index on tie */
		iv0=iv[n+i]; i0=i; mi=b[i]/a[i][j0]; i++;
		for (; i<m; i++)
		{	
			if (iv[n+i]>=0 && a[i][j0]>epsilon)
			{	
				h=b[i]/a[i][j0];
				if (h<mi-epsilon || (h<=mi+epsilon && iv[n+i]<iv0))
				{	mi=h; i0=i; iv0=iv[n+i];
				}
			}
		}
		/* Exchange column j0 and i0 */
		exchange(i0,j0,m,n,iv,a,b,c);
#ifdef SIMPLEXTEST
		printsimplex(m,n,iv,a,b,c);
#endif
		if (test_key()==escape) { error=301; return 2; }
	}

#ifdef SIMPLEXTEST
	print("At end\n\n");
	printsimplex(m,n,iv,a,b,c);
#endif

	/* Compute the solution x */
	solution :
	for (i=0; i<n+m; i++)
	{
		if (iv[n+i]<0) iv[n+i]=-iv[n+i]-1;
	}
	for (i=0; i<m; i++)
	{   
		if (iv[n+i]<n) 
		{
			x[iv[n+i]]=b[i];
			if (re[iv[n+i]]<0)
				x[iv[n+i]] = -x[iv[n+i]];
		}
	}
	return res;
}

int signum (double x)
{
	if (x>epsilon) return 1;
	if (x<-epsilon) return -1;
	return 0;
}

#define max(i,j) (i>j?i:j)
#define min(i,j) (i<j?i:j)

void msimplex (header *hd)
{	
	header *result,*hda=hd,*hdb,*hdc,*hdeq=0,*hdre=0,*hdmin=0;
	int i,n,m,res;
	int rb,cb,rc,cc,req,ceq,rre,cre;
	double *ma,*mb,*mc,*meq=0,*mre=0;
	double *x,*a,*b,*c,**aa;
	int *ieq,*ire,iopt;
	int *iv;
	char *ram;
	hdb=next_param(hda);
	hdc=next_param(hdb);
	hdeq=next_param(hdc);
	if (hdeq) hdre=next_param(hdeq);
	if (hdre) hdmin=next_param(hdre);
	hda=getvalue(hda); if (error) return;
	hdb=getvalue(hdb); if (error) return;
	hdc=getvalue(hdc); if (error) return;
	if (hdeq) { hdeq=getvalue(hdeq); if (error) return; }
	if (hdre) { hdre=getvalue(hdre); if (error) return; }
	if (hdmin) { hdmin=getvalue(hdmin); if (error) return; }
	if ((hda->type!=s_matrix && hda->type!=s_real) || (hdb->type!=s_matrix && hdb->type!=s_real) ||
		(hdc->type!=s_matrix && hdc->type!=s_real))
	{
		print("simplex needs a real matrix A, a column vector b, and a row vector c.\n");
		error=10; return;
	}
	getmatrix(hda,&m,&n,&ma);
	getmatrix(hdb,&rb,&cb,&mb);
	getmatrix(hdc,&rc,&cc,&mc);
	if (hdeq) getmatrix(hdeq,&req,&ceq,&meq);
	if (hdre) getmatrix(hdre,&rre,&cre,&mre);
	if (cb!=1)
	{
		print("simplex needs a column vector for b.\n");
		error=10; return;
	}
	if (rc!=1)
	{
		print("simplex needs a row vector for c.\n");
		error=10; return;
	}
	if (rb!=m || cc!=n)
	{		
		print("Simplex needs a mxn matrix A, a mx1 vector b, and a 1xn vector c.\n");
		error=10; return;
	}
	if (hdeq && ((req!=m && req!=1) || ceq!=1))
	{
		print("In simplex: equation flags must be mx1 vector, or a real.\n");
		error=10; return;
	}
	if (hdre && ((cre!=n && cre!=1) || rre!=1))
	{
		print("In simplex: restriction flags must be 1xn vector, or a real.\n");
		error=10; return;
	}	
	if (hdmin && hdmin->type!=s_real)
	{
		print("In simplex: optimization direction must be a real.\n");
		error=10; return;
	}	
	result=new_matrix(n,1,""); if (error) return;
	x=matrixof(result);
	for (i=0; i<n; i++) *x++=0.0;

	ram=newram;
	
	a=(double *)ram; increase(ram,(n*m)*sizeof(double));
	memcpy(a,ma,(n*m)*sizeof(double));

	aa=(double **)ram; increase(ram,m*sizeof(double *));
	for (i=0; i<m; i++) aa[i]=a+i*n;

	b=(double *)ram; increase(ram,m*sizeof(double));
	memcpy(b,mb,m*sizeof(double));
	
	c=(double *)ram; increase(ram,n*sizeof(double));
	memcpy(c,mc,n*sizeof(double));	
	
	iv=(int *)ram; increase(ram,(n+m)*sizeof(int));
	for (i=0; i<n+m; i++) iv[i]=i;

	ieq=(int *)ram; increase(ram,m*sizeof(int));
	if (hdeq) for (i=0; i<m; i++) ieq[i]=signum(meq[min(i,req-1)]);
	else for (i=0; i<m; i++) ieq[i]=-1;

	ire=(int *)ram; increase(ram,n*sizeof(int));
	if (hdre) for (i=0; i<n; i++) ire[i]=signum(mre[min(i,cre-1)]);
	else for (i=0; i<n; i++) ire[i]=1;

	if (hdmin) iopt=signum(*realof(hdmin));
	else iopt=-1;

	res=simplex(m,n,iv,aa,b,c,matrixof(result),ieq,ire,iopt);
	moveresult(hd,result);
	new_real(res,"");
	result=new_matrix(1,m,"");
	x=matrixof(result);
	for (i=0; i<m; i++)
	{   
		*(x+i)=0;
	}
	for (i=0; i<n; i++)
	{   
		if (iv[i]>=n) *(x+iv[i]-n)=1;
	}
}

/************** Singular Value Decomposition ************/

#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))
static double maxarg1,maxarg2;
#define FMAX(a,b) (maxarg1=(a),maxarg2=(b),(maxarg1) > (maxarg2) ?\
		(maxarg1) : (maxarg2))
static int iminarg1,iminarg2;
#define IMIN(a,b) (iminarg1=(a),iminarg2=(b),(iminarg1) < (iminarg2) ?\
		(iminarg1) : (iminarg2))
static double sqrarg;
#define SQR(a) ((sqrarg=(a)) == 0.0 ? 0.0 : sqrarg*sqrarg)

double pythag (double a, double b)
{
	double absa,absb;
	absa=fabs(a);
	absb=fabs(b);
	if (absa > absb)
		return absa*sqrt(1.0+SQR(absb/absa));
	else
		return (absb == 0.0 ? 0.0 : absb*sqrt(1.0+SQR(absa/absb)));
}

void svdcmp(double **a, int m, int n, double w[], double **v)
{
	double pythag(double a, double b);
	int flag,i,its,j,jj,k,l=0,nm=0;
	double anorm,c,f,g,h,s,scale,x,y,z,*rv1;

	rv1=(double *)malloc((n+1)*sizeof(double));
	g=scale=anorm=0.0;
	for (i=1;i<=n;i++) {
		l=i+1;
		rv1[i]=scale*g;
		g=s=scale=0.0;
		if (i <= m) {
			for (k=i;k<=m;k++) scale += fabs(a[k][i]);
			if (scale) {
				for (k=i;k<=m;k++) {
					a[k][i] /= scale;
					s += a[k][i]*a[k][i];
				}
				f=a[i][i];
				g = -SIGN(sqrt(s),f);
				h=f*g-s;
				a[i][i]=f-g;
				for (j=l;j<=n;j++) {
					for (s=0.0,k=i;k<=m;k++) s += a[k][i]*a[k][j];
					f=s/h;
					for (k=i;k<=m;k++) a[k][j] += f*a[k][i];
				}
				for (k=i;k<=m;k++) a[k][i] *= scale;
			}
		}
		w[i]=scale *g;
		g=s=scale=0.0;
		if (i <= m && i != n) {
			for (k=l;k<=n;k++) scale += fabs(a[i][k]);
			if (scale) {
				for (k=l;k<=n;k++) {
					a[i][k] /= scale;
					s += a[i][k]*a[i][k];
				}
				f=a[i][l];
				g = -SIGN(sqrt(s),f);
				h=f*g-s;
				a[i][l]=f-g;
				for (k=l;k<=n;k++) rv1[k]=a[i][k]/h;
				for (j=l;j<=m;j++) {
					for (s=0.0,k=l;k<=n;k++) s += a[j][k]*a[i][k];
					for (k=l;k<=n;k++) a[j][k] += s*rv1[k];
				}
				for (k=l;k<=n;k++) a[i][k] *= scale;
			}
		}
		anorm=FMAX(anorm,(fabs(w[i])+fabs(rv1[i])));
	}
	for (i=n;i>=1;i--) {
		if (i < n) {
			if (g) {
				for (j=l;j<=n;j++)
					v[j][i]=(a[i][j]/a[i][l])/g;
				for (j=l;j<=n;j++) {
					for (s=0.0,k=l;k<=n;k++) s += a[i][k]*v[k][j];
					for (k=l;k<=n;k++) v[k][j] += s*v[k][i];
				}
			}
			for (j=l;j<=n;j++) v[i][j]=v[j][i]=0.0;
		}
		v[i][i]=1.0;
		g=rv1[i];
		l=i;
	}
	for (i=IMIN(m,n);i>=1;i--) {
		l=i+1;
		g=w[i];
		for (j=l;j<=n;j++) a[i][j]=0.0;
		if (g) {
			g=1.0/g;
			for (j=l;j<=n;j++) {
				for (s=0.0,k=l;k<=m;k++) s += a[k][i]*a[k][j];
				f=(s/a[i][i])*g;
				for (k=i;k<=m;k++) a[k][j] += f*a[k][i];
			}
			for (j=i;j<=m;j++) a[j][i] *= g;
		} else for (j=i;j<=m;j++) a[j][i]=0.0;
		++a[i][i];
	}
	for (k=n;k>=1;k--) {
		for (its=1;its<=30;its++) {
			flag=1;
			for (l=k;l>=1;l--) {
				nm=l-1;
				if ((double)(fabs(rv1[l])+anorm) == anorm) {
					flag=0;
					break;
				}
				if ((double)(fabs(w[nm])+anorm) == anorm) break;
			}
			if (flag) {
				c=0.0;
				s=1.0;
				for (i=l;i<=k;i++) {
					f=s*rv1[i];
					rv1[i]=c*rv1[i];
					if ((double)(fabs(f)+anorm) == anorm) break;
					g=w[i];
					h=pythag(f,g);
					w[i]=h;
					h=1.0/h;
					c=g*h;
					s = -f*h;
					for (j=1;j<=m;j++) {
						y=a[j][nm];
						z=a[j][i];
						a[j][nm]=y*c+z*s;
						a[j][i]=z*c-y*s;
					}
				}
			}
			z=w[k];
			if (l == k) {
				if (z < 0.0) {
					w[k] = -z;
					for (j=1;j<=n;j++) v[j][k] = -v[j][k];
				}
				break;
			}
			if (its == 30)
			{	output("No convergence in svd\n");
				error=1; return;
			}
			x=w[l];
			nm=k-1;
			y=w[nm];
			g=rv1[nm];
			h=rv1[k];
			f=((y-z)*(y+z)+(g-h)*(g+h))/(2.0*h*y);
			g=pythag(f,1.0);
			f=((x-z)*(x+z)+h*((y/(f+SIGN(g,f)))-h))/x;
			c=s=1.0;
			for (j=l;j<=nm;j++) {
				i=j+1;
				g=rv1[i];
				y=w[i];
				h=s*g;
				g=c*g;
				z=pythag(f,h);
				rv1[j]=z;
				c=f/z;
				s=h/z;
				f=x*c+g*s;
				g = g*c-x*s;
				h=y*s;
				y *= c;
				for (jj=1;jj<=n;jj++) {
					x=v[jj][j];
					z=v[jj][i];
					v[jj][j]=x*c+z*s;
					v[jj][i]=z*c-x*s;
				}
				z=pythag(f,h);
				w[j]=z;
				if (z) {
					z=1.0/z;
					c=f*z;
					s=h*z;
				}
				f=c*g+s*y;
				x=c*y-s*g;
				for (jj=1;jj<=m;jj++) {
					y=a[jj][j];
					z=a[jj][i];
					a[jj][j]=y*c+z*s;
					a[jj][i]=z*c-y*s;
				}
			}
			rv1[l]=0.0;
			rv1[k]=f;
			w[k]=x;
		}
	}
	free(rv1);
}

void msvd (header *hd)
{	header *st=hd,*hda,*hdw,*hdv;
	int m,n,i;
	double *mm;
	double **ma,**mv;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix) wrong_arg_in("svd");
	getmatrix(hd,&m,&n,&mm);
	if (m<1 || n<1) wrong_arg_in("svd");
	hda=new_matrix(m,n,""); if (error) return;
	hdw=new_matrix(1,n,""); if (error) return;
	hdv=new_matrix(n,n,""); if (error) return;
	memmove((char *)matrixof(hda),(char *)mm,
		n*sizeof(double)*m);
	ma=(double **)malloc(m*sizeof(double *));
	for (i=0; i<m; i++) ma[i]=matrixof(hda)+i*n-1;
	mv=(double **)malloc(n*sizeof(double *));
	for (i=0; i<n; i++) mv[i]=matrixof(hdv)+i*n-1;
	svdcmp(ma-1,m,n,matrixof(hdw)-1,mv-1);
	free(ma); free(mv);
	if (error) return;
	moveresult1(st,hda);
}

/**************** Toeplitz systems ******************/

void toeplitz (double r[], double x[], double y[], int n)
{
	int j,k,m,m1,m2;
	double pp,pt1,pt2,qq,qt1,qt2,sd,sgd,sgn,shn,sxn;
	double *g,*h,*g1=0,*h1=0;

	if (fabs(r[n]) < epsilon)
	{	output("Toeplitz singular error\n");
		error=1; goto stop;
	}
	g1=(double *)malloc(n*sizeof(double)); g=g1-1;
	h1=(double *)malloc(n*sizeof(double)); h=h1-1;
	x[1]=y[1]/r[n];
	if (n == 1) goto stop;
	g[1]=r[n-1]/r[n];
	h[1]=r[n+1]/r[n];
	for (m=1;m<=n;m++) {
		m1=m+1;
		sxn = -y[m1];
		sd = -r[n];
		for (j=1;j<=m;j++) {
			sxn += r[n+m1-j]*x[j];
			sd += r[n+m1-j]*g[m-j+1];
		}
		if (fabs(sd) < epsilon)
		{	output("Toeplitz singular error\n");
			error=1; goto stop;
		}
		x[m1]=sxn/sd;
		for (j=1;j<=m;j++) x[j] -= x[m1]*g[m-j+1];
		if (m1 == n) goto stop;
		sgn = -r[n-m1];
		shn = -r[n+m1];
		sgd = -r[n];
		for (j=1;j<=m;j++) {
			sgn += r[n+j-m1]*g[j];
			shn += r[n+m1-j]*h[j];
			sgd += r[n+j-m1]*h[m-j+1];
		}
		if (fabs(sd)<epsilon || fabs(sgd)<epsilon)
		{	output("Toeplitz singular error\n");
			error=1; goto stop;
		}
		g[m1]=sgn/sgd;
		h[m1]=shn/sd;
		k=m;
		m2=(m+1) >> 1;
		pp=g[m1];
		qq=h[m1];
		for (j=1;j<=m2;j++) {
			pt1=g[j];
			pt2=g[k];
			qt1=h[j];
			qt2=h[k];
			g[j]=pt1-pp*qt2;
			g[k]=pt2-pp*qt1;
			h[j]=qt1-qq*pt2;
			h[k--]=qt2-qq*pt1;
		}
	}
	output("Toeplitz failed.\n");
	error=1;
	stop : free(g1); free(h1);
}

void msolvetoeplitz (header *hd)
{	header *st=hd,*hdb,*result;
	int n,i;
	double *m,*r;
	hdb=nextof(hd);
	hd=getvalue(hd); if (error) return;
	hdb=getvalue(hdb); if (error) return;
	if (hd->type!=s_matrix || dimsof(hd)->r!=1 ||
		hdb->type!=s_matrix || dimsof(hdb)->c!=1)
		wrong_arg_in("toeplitzsolve");
	n=dimsof(hdb)->r;
	if (n<2 || dimsof(hd)->c!=2*n-1) wrong_arg_in("toeplitzsolve");
	result=new_matrix(n,1,""); if (error) return;
	r=(double *)malloc((2*n-1)*sizeof(double));
	m=matrixof(hd);
	for (i=0; i<2*n-1; i++) r[i]=m[2*n-2-i];
	toeplitz(r-1,matrixof(result)-1,
		matrixof(hdb)-1,n);
	free(r);
	if (error) return;
	moveresult(st,result);
}

void mtoeplitz (header *hd)
{	header *st=hd,*result;
	int i,n;
    double *m,*r;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix || dimsof(hd)->r!=1)
		wrong_arg_in("toeplitz");
	n=dimsof(hd)->c;
	if (n<2 || n%2!=1) wrong_arg_in("toeplitz");
	n=n/2+1;
	result=new_matrix(n,n,""); if (error) return;
	m=matrixof(result);
	r=matrixof(hd);
	for (i=0; i<n; i++)
	{	memmove((char *)(m+i*n),(char *)(r+n-1-i),
			n*sizeof(double));
	}
	moveresult(st,result);
}
