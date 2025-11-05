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

#include "AlgLib/src/specialfunctions.h"

double gamm (double x)
{
	try
	{
		return alglib::gammafunction(x);
	}
	catch (alglib::ap_error)
	{
		if (!pnan) error=1;
		return NAN;
	}
}

double gammln (double x)
{
	double sign;
	try
	{
			return alglib::lngamma(x,sign);
	}
	catch (alglib::ap_error)
	{
		if (!pnan) error=1;
		return NAN;
	}
}

void mgammaln (header *hd)
{	spread1(gammln,0,hd);
	test_error("gammaln");
}

static double gammaparams[]=
{0.99999999999980993, 676.5203681218851,
-1259.1392167224028, 771.32342877765313,
-176.61502916214059, 12.507343278686905,
-0.13857109526572012, 9.9843695780195716e-6,
1.5056327351493116e-7};

void cpow (double *x, double *xi, double *y, double *yi,
	double *z, double *zi);
void cexp (double *x, double *xi, double *z, double *zi);
void csin (double *x, double *xi, double *z, double *zi);
void complex_multiply (double *x, double *xi, double *y, double *yi,
	double *z, double *zi);

void cgamma (double *xr, double *xi, double *zr, double *zi)
{
	if (fabs(*xi)<epsilon) 
	{
		if (*xr <= 0 && fabs((int)*xr - *xr) < epsilon)
		{
			*zr = NAN; *zi = 0;
		}
		else
		{
			*zr=gamm(*xr); *zi=0;
		}
	}
	else
	{
		double pi=3.14159265358979323846;
		if (*xr<0.5)
		{
			double hr,hi,vr=1-*xr,vi=-*xi;
			cgamma(&vr,&vi,&hr,&hi);
			double h1r,h1i,ur=pi*(*xr),ui=pi*(*xi);
			csin(&ur,&ui,&h1r,&h1i);
			double tr,ti;
			complex_multiply(&hr,&hi,&h1r,&h1i,&tr,&ti);
			double r=tr*tr+ti*ti;
			*zr=pi*tr/r;
			*zi=-pi*ti/r;
		}
		else
		{
			double zhr=*xr-1.0,zhi=*xi;
			double xhr=gammaparams[0],xhi=0.0;
			for (int i=1; i<9; i++)
			{
				double r=(zhr+i)*(zhr+i)+zhi*zhi;
				xhr += gammaparams[i]*(zhr+i)/r;
				xhi -= gammaparams[i]*zhi/r;
			}
			double tr=zhr+7.5,ti=zhi;
			double hr,hi,ur=zhr+0.5,ui=zhi;
			cpow(&tr,&ti,&ur,&ui,&hr,&hi);
			double h1r,h1i,vr=-tr,vi=-ti;
			cexp(&vr,&vi,&h1r,&h1i);
			complex_multiply(&hr,&hi,&h1r,&h1i,&tr,&ti);
			complex_multiply(&tr,&ti,&xhr,&xhi,zr,zi);
			double h=sqrt(2*pi);
			*zr *= h;
			*zi *= h;
		}
	}
}

void mgamma (header *hd)
{	spread1(gamm,cgamma,hd);
	test_error("gamma");
}

#define ITMAX 400
#define EPS epsilon
#define FPMIN 1.0e-60

void gammai (double *x, double *a, double *res)
{
	if (*x < 0.0 || *a <= 0.0)
	{	
		error=1; return;
	}
	try
	{
		*res=alglib::incompletegamma(*a,*x);
	}
	catch (alglib::ap_error)
	{
		print("AlgLib: gammai(%g,%g) could not be evaluated.\n",*x,*a);
		if (!pnan) error=1;
		*res=NAN;
	}
}

void mgammai (header *hd)
{   
	spreadf2(gammai,0,0,hd,"gammai");
	test_error("gamma");
}

void gammaic (double *x, double *a, double *res)
{
	if (*x < 0.0 || *a <= 0.0)
	{	
		error=1; return;
	}
	try
	{
		*res=alglib::incompletegammac(*a,*x);
	}
	catch (alglib::ap_error)
	{
		print("AlgLib: gammaic(%g,%g) could not be evaluated.\n",*x,*a);
		if (!pnan) error=1;
		*res=NAN;
	}
}

void mgammaic (header *hd)
{   
	spreadf2(gammaic,0,0,hd,"gammaic");
	test_error("gamma");
}

void tdis (double *x, double *a, double *res)
{
	try
	{
		if ((int)(*a)!=*a) throw alglib::ap_error();
		else *res=alglib::studenttdistribution((int)*a,*x);
	}
	catch (alglib::ap_error)
	{
		print("AlgLib: tdis(%g,%g) could not be evaluated.\n",*x,*a);
		if (!pnan) error=1;
		*res=NAN;
	}
}

void mtdis (header *hd)
{   
	spreadf2(tdis,0,0,hd,"tdis");
	test_error("tdis");
}

void invtdis (double *x, double *a, double *res)
{
	try
	{
		if ((int)(*a)!=*a) throw alglib::ap_error();
		else 
			*res=alglib::invstudenttdistribution((int)*a,*x);
	}
	catch (alglib::ap_error e)
	{
		print("AlgLib: invtdis(%g,%g) could not be evaluated.\n",*x,*a);
		if (!pnan) error=1;
		*res=NAN;
	}
}

void minvtdis (header *hd)
{   
	spreadf2(invtdis,0,0,hd,"invtdis");
	test_error("invtdis");
}

double fdis (double x, double a, double b)
{	
	try
	{
		if ((int)(a)!=a) throw alglib::ap_error();
		if ((int)(b)!=b) throw alglib::ap_error();
		return alglib::fdistribution((int)a,(int)b,x);
	}
	catch (alglib::ap_error)
	{
		print("AlgLib: fdis(%g,%g,%g) could not be evaluated.\n",x,a,b);
		if (!pnan) error=1;
		return NAN;
	}
}

void mfdis (header *hd)
{	header *result,*st=hd,*hda,*hdb;
	double x;
	hda=nextof(hd);
	hdb=nextof(hda);
	hd=getvalue(hd); if (error) return;
	hda=getvalue(hda); if (error) return;
	hdb=getvalue(hdb); if (error) return;
	if (hd->type!=s_real || hda->type!=s_real ||
		hdb->type!=s_real)
			wrong_arg_in("fdis");
	x=fdis(*realof(hd),*realof(hda),*realof(hdb));
	if (error) return;
	result=new_real(x,""); if (error) return;
	moveresult(st,result);
}

double fdisc (double x, double a, double b)
{	
	try
	{
		if ((int)(a)!=a) throw alglib::ap_error();
		if ((int)(b)!=b) throw alglib::ap_error();
		return alglib::fcdistribution((int)a,(int)b,x);
	}
	catch (alglib::ap_error)
	{
		print("fcdis(%g,%g,%g) could not be evaluated.\n",x,a,b);
		if (!pnan) error=1;
		return NAN;
	}
}

void mfdisc (header *hd)
{	
	header *result,*st=hd,*hda,*hdb;
	double x;
	hda=nextof(hd);
	hdb=nextof(hda);
	hd=getvalue(hd); if (error) return;
	hda=getvalue(hda); if (error) return;
	hdb=getvalue(hdb); if (error) return;
	if (hd->type!=s_real || hda->type!=s_real ||
		hdb->type!=s_real)
			wrong_arg_in("fdisc");
	x=fdisc(*realof(hd),*realof(hda),*realof(hdb));
	if (error) return;
	result=new_real(x,""); if (error) return;
	moveresult(st,result);
}

double finvdis (double x, double a, double b)
{	
	try
	{
		if ((int)(a)!=a) throw alglib::ap_error();
		if ((int)(b)!=b) throw alglib::ap_error();
		return alglib::invfdistribution((int)a,(int)b,x);
	}
	catch (alglib::ap_error)
	{
		print("finvcdis(%g,%g,%g) could not be evaluated.\n",x,a,b);
		if (!pnan) error=1;
		return NAN;
	}
}

void minvfdis (header *hd)
{	
	header *result,*st=hd,*hda,*hdb;
	double x;
	hda=nextof(hd);
	hdb=nextof(hda);
	hd=getvalue(hd); if (error) return;
	hda=getvalue(hda); if (error) return;
	hdb=getvalue(hdb); if (error) return;
	if (hd->type!=s_real || hda->type!=s_real ||
		hdb->type!=s_real)
			wrong_arg_in("invcfdis");
	x=finvdis(*realof(hd),*realof(hda),*realof(hdb));
	if (error) return;
	result=new_real(x,""); if (error) return;
	moveresult(st,result);
}

void chidis (double *x, double *a, double *res)
{
	try
	{
		if ((int)(*a)!=*a) throw alglib::ap_error();
		else *res=alglib::chisquaredistribution((int)*a,*x);
	}
	catch (alglib::ap_error)
	{
		print("AlgLib: chidis(%g,%g) could not be evaluated.\n",*x,*a);
		if (!pnan) error=1;
		*res=NAN;
	}
}

void mchidis (header *hd)
{   
	spreadf2(chidis,0,0,hd,"chidis");
	test_error("chidis");
}

void chicdis (double *x, double *a, double *res)
{
	try
	{
		if ((int)(*a)!=*a) throw alglib::ap_error();
		else *res=alglib::chisquarecdistribution((int)*a,*x);
	}
	catch (alglib::ap_error)
	{
		print("AlgLib: chicdis(%g,%g) could not be evaluated.\n",*x,*a);
		if (!pnan) error=1;
		*res=NAN;
	}
}

void mchicdis (header *hd)
{   
	spreadf2(chicdis,0,0,hd,"chicdis");
	test_error("chicdis");
}

void invchidis (double *x, double *a, double *res)
{
	try
	{
		if ((int)(*a)!=*a) throw alglib::ap_error();
		else *res=alglib::invchisquaredistribution((int)*a,*x);
	}
	catch (alglib::ap_error)
	{
		print("AlgLib: invchidis(%g,%g) could not be evaluated.\n",*x,*a);
		if (!pnan) error=1;
		*res=NAN;
	}
}

void minvchidis (header *hd)
{   
	spreadf2(invchidis,0,0,hd,"invchidis");
	test_error("invchidis");
}

double gauss (double z)
{   
	try
	{
		return alglib::normaldistribution(z);
	}
	catch (alglib::ap_error)
	{
		if (!pnan) error=1;
		return NAN;
	}
}

void mgauss (header *hd)
{	
	spread1(gauss,0,hd);
	test_error("normaldis");
}

double erf (double z)
{
	try
	{
		return alglib::errorfunction(z);
	}
	catch (alglib::ap_error)
	{
		if (!pnan) error=1;
		return NAN;
	}
}

void merf (header *hd)
{
	spread1(erf,0,hd);
	test_error("erf");
}

double erfc (double z)
{
	try
	{
		return alglib::errorfunctionc(z);
	}
	catch (alglib::ap_error)
	{
		if (!pnan) error=1;
		return NAN;
	}
}

void merfc (header *hd)
{
	spread1(erfc,0,hd);
	test_error("erfc");
}

#define IM1 2147483563
#define IM2 2147483399
#define AM (1.0/IM1)
#define IMM1 (IM1-1)
#define IA1 40014
#define IA2 40692
#define IQ1 53668
#define IQ2 52774
#define IR1 12211
#define IR2 3791
#define NTAB 32
#define NDIV (1+IMM1/NTAB)
#define RNMX (1.0-EPS)

static long randseed=1234512345;
#define IDUM2 123456789
static long idum2=IDUM2;
static long iy=0;
static long iv[NTAB];

double ran2 (void)
{
	int j;
	long k;
	double temp;

	if (randseed <= 0) {
		if (-(randseed) < 1) randseed=1;
		else randseed = -(randseed);
		idum2=(randseed);
		for (j=NTAB+7;j>=0;j--) {
			k=(randseed)/IQ1;
			randseed=IA1*(randseed-k*IQ1)-k*IR1;
			if (randseed < 0) randseed += IM1;
			if (j < NTAB) iv[j] = randseed;
		}
		iy=iv[0];
	}
	k=(randseed)/IQ1;
	randseed=IA1*(randseed-k*IQ1)-k*IR1;
	if (randseed < 0) randseed += IM1;
	k=idum2/IQ2;
	idum2=IA2*(idum2-k*IQ2)-k*IR2;
	if (idum2 < 0) idum2 += IM2;
	j=iy/NDIV;
	iy=iv[j]-idum2;
	iv[j] = randseed;
	if (iy < 1) iy += IMM1;
	if ((temp=AM*iy) > RNMX) return RNMX;
	else return temp;
}

void mrandom (header *hd)
{	header *st=hd,*result;
	double *m;
	int r,c;
	LONG k,n;
	if (!hd)
	{	new_real(ran2(),"");
		return;
	}
	get_size(hd,&r,&c,"random"); if (error) return;
	result=new_matrix(r,c,""); if (error) return;
	m=matrixof(result);
	n=(LONG)c*r;
	for (k=0; k<n; k++) *m++=ran2();
	moveresult(st,result);
}

void mintrandom1 (header *hd)
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real) need_arg_in("random","optional size n, m, integer k");
	result=new_real((int)(ran2()*(*realof(hd)))+1,"");
	moveresult(st,result);
}

void mintrandom2 (header *hd)
{	header *st=hd,*result;
	double *m;
	int r,c;
	LONG k,n;
	hd=get_size(hd,&r,&c,"normal"); if (error) return;
	double f;
	if (!hd)
	{
		 f=c; c=r; r=1;
	}
	else
	{
		hd=getvalue(hd); if (error) return;
		if (hd->type!=s_real) need_arg_in("random","optional size n, m, integer k");
		f=*realof(hd);
	}
	result=new_matrix(r,c,""); if (error) return;
	m=matrixof(result);
	n=(LONG)c*r;
	for (k=0; k<n; k++) *m++=(int)(ran2()*f)+1;
	moveresult(st,result);
}

void mshuffle (header *hd)
{  header *st=hd,*result;
	double *m,*mr,x;
	int i,j,n;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix || dimsof(hd)->r!=1)
		wrong_arg_in("shuffle");
	n=dimsof(hd)->c;
	result=new_matrix(1,n,"");
	m=matrixof(hd); mr=matrixof(result);
	for (i=0; i<n; i++) *mr++=*m++;
	mr=matrixof(result);
	for (i=n-1; i>0; i--)
	{	j=(int)floor(ran2()*(i+1));
		if (i!=j)
		{	x=*(mr+i); *(mr+i)=*(mr+j); *(mr+j)=x;
		}
	}
	moveresult(st,result);

}

int randomres=0;

double gasdev (void)
{	
	static double f=0,rsq=1,v1=1,v2=1;
	if (randomres)
	{	
		randomres=0; return v2*f;
	}
	do
	{	
		v1=2.0*ran2()-1.0;
		v2=2.0*ran2()-1.0;
		rsq=v1*v1+v2*v2;
	} 
	while (rsq >= 1.0 || rsq <= 0.00001);
	f=sqrt(-2.0*log(rsq)/rsq);
	randomres=1;
	return v1*f;
}

void mnormalnew (header *hd)
{	
	header *st=hd,*result;
	double *m;
	int r,c;
	LONG k,n;
	if (!hd)
	{	new_real(gasdev(),"");
		return;
	}
	get_size(hd,&r,&c,"normal"); if (error) return;
	result=new_matrix(r,c,""); if (error) return;
	m=matrixof(result);
	n=(LONG)c*r;
	for (k=0; k<n; k++) *m++=gasdev();
	moveresult(st,result);
}

void mseed (header *hd)
{   
	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real) wrong_arg_in("seed");
	result=new_real(*realof(hd),"");
	double x=fabs(*realof(hd));
	x=log(x+1);
	randseed=-labs((long)((x-floor(x))*LONG_MAX));
	randomres=0;
	moveresult(st,result);
}

/************ BETA Functions *******************/

double sbetai (double **x, int n, void *p, char *name)
{
	try
	{
		return alglib::incompletebeta(*x[1],*x[2],*x[0]);
	}
	catch (alglib::ap_error)
	{
		if (!pnan) error=1;
		return NAN;
	}
}

void mbetai (header *hd)
{
	spreadrn(hd,3,sbetai,0,"betai");
}

double invsbetai (double **x, int n, void *p, char *name)
{	
	try
	{
		return alglib::invincompletebeta(*x[1],*x[2],*x[0]);
	}
	catch (alglib::ap_error)
	{
		if (!pnan) error=1;
		return NAN;
	}
}

void minvbetai (header *hd)
{	
	spreadrn(hd,3,invsbetai,0,"invbetai");
}

void beta (double *a, double *b, double *res)
{
	try
	{
		*res=alglib::beta(*a,*b);
	}
	catch (alglib::ap_error)
	{
		*res=NAN;
		if (!pnan) error=1;
	}
}

void mbeta (header *hd)
{
	spreadf2(beta,0,0,hd,"beta");
	test_error("beta");
}

double chebev (double a, double b, double c[], int m, double x)
{
	double d=0.0,dd=0.0,sv,y,y2;
	int j;

	if ((x-a)*(x-b) > 0.0)
	{	output("x not in range in routine bessel\n");
		error = 1; return 0;
	}
	y2=2.0*(y=(2.0*x-a-b)/(b-a));
	for (j=m-1;j>=1;j--) {
		sv=d;
		d=y2*d-dd+c[j];
		dd=sv;
	}
	return y*d-dd+0.5*c[0];
}

#define NUSE1 7
#define NUSE2 8

void beschb(double x, double *gam1, double *gam2, double *gampl, double *gammi)
{	double xx;
	static double c1[] = {
		-1.142022680371172e0,6.516511267076e-3,
		3.08709017308e-4,-3.470626964e-6,6.943764e-9,
		3.6780e-11,-1.36e-13};
	static double c2[] = {
		1.843740587300906e0,-0.076852840844786e0,
		1.271927136655e-3,-4.971736704e-6,-3.3126120e-8,
		2.42310e-10,-1.70e-13,-1.0e-15};

	xx=8.0*x*x-1.0;
	*gam1=chebev(-1.0,1.0,c1,NUSE1,xx); if (error) return;
	*gam2=chebev(-1.0,1.0,c2,NUSE2,xx); if (error) return;
	*gampl= *gam2-x*(*gam1);
	*gammi= *gam2+x*(*gam1);
}
#undef NUSE1
#undef NUSE2

static int imaxarg1,imaxarg2;
#define IMAX(a,b) (imaxarg1=(a),imaxarg2=(b),(imaxarg1) > (imaxarg2) ?\
		(imaxarg1) : (imaxarg2))

#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

#undef EPS
#undef FPMIN
#define EPS 1.0e-16
#define FPMIN 1.0e-30
#define MAXIT 10000
#define XMIN 2.0

void bessjy (double xnu, double x, double *rj, double *ry,
	double *rjp, double *ryp)
{
	int i,isign,l,nl;
	double a,b,br,bi,c,cr,ci,d,del,del1,den,di,dlr,dli,dr,e,f,fact,fact2,
		fact3,ff,gam,gam1,gam2,gammi,gampl,h,p,pimu,pimu2,q,r,rjl,
		rjl1,rjmu,rjp1,rjpl,rjtemp,ry1,rymu,rymup,rytemp,sum,sum1,
		temp,w,x2,xi,xi2,xmu,xmu2;

	if (xnu < 0.0)
	{
		int ixnu=(int)xnu;
		if (ixnu==xnu)
		{
			bessjy(-xnu,x,rj,ry,rjp,ryp);
			if (ixnu%2) *rj=-*rj;
			return;
		}
		output("bad arguments in bessel\n");
		error = 1; return;
	}
	if (x < -epsilon)
	{
		int ixnu=(int)xnu;
		if (ixnu==xnu)
		{
			bessjy(xnu,-x,rj,ry,rjp,ryp);
			if (ixnu%2) *rj=-*rj;
			return;
		}
		output("bad arguments in bessel\n");
		error = 1; return;
	}
	else if (x < epsilon)
	{
		if (xnu<epsilon) *rj=1;
		else *rj=0;
		return;
	}
	nl=(x < XMIN ? (int)(xnu+0.5) : IMAX(0,(int)(xnu-x+1.5)));
	xmu=xnu-nl;
	xmu2=xmu*xmu;
	xi=1.0/x;
	xi2=2.0*xi;
	w=xi2/M_PI;
	isign=1;
	h=xnu*xi;
	if (h < FPMIN) h=FPMIN;
	b=xi2*xnu;
	d=0.0;
	c=h;
	for (i=1;i<=MAXIT;i++) {
		b += xi2;
		d=b-d;
		if (fabs(d) < FPMIN) d=FPMIN;
		c=b-1.0/c;
		if (fabs(c) < FPMIN) c=FPMIN;
		d=1.0/d;
		del=c*d;
		h=del*h;
		if (d < 0.0) isign = -isign;
		if (fabs(del-1.0) < EPS) break;
	}
	if (i > MAXIT)
	{	output("x too large in bessel\n");
		error=1; return;
	}
	rjl=isign*FPMIN;
	rjpl=h*rjl;
	rjl1=rjl;
	rjp1=rjpl;
	fact=xnu*xi;
	for (l=nl;l>=1;l--) {
		rjtemp=fact*rjl+rjpl;
		fact -= xi;
		rjpl=fact*rjtemp-rjl;
		rjl=rjtemp;
	}
	if (rjl == 0.0) rjl=EPS;
	f=rjpl/rjl;
	if (x < XMIN) {
		x2=0.5*x;
		pimu=M_PI*xmu;
		fact = (fabs(pimu) < EPS ? 1.0 : pimu/sin(pimu));
		d = -log(x2);
		e=xmu*d;
		fact2 = (fabs(e) < EPS ? 1.0 : sinh(e)/e);
		beschb(xmu,&gam1,&gam2,&gampl,&gammi);
		ff=2.0/M_PI*fact*(gam1*cosh(e)+gam2*fact2*d);
		e=exp(e);
		p=e/(gampl*M_PI);
		q=1.0/(e*M_PI*gammi);
		pimu2=0.5*pimu;
		fact3 = (fabs(pimu2) < EPS ? 1.0 : sin(pimu2)/pimu2);
		r=M_PI*pimu2*fact3*fact3;
		c=1.0;
		d = -x2*x2;
		sum=ff+r*q;
		sum1=p;
		for (i=1;i<=MAXIT;i++) {
			ff=(i*ff+p+q)/(i*i-xmu2);
			c *= (d/i);
			p /= (i-xmu);
			q /= (i+xmu);
			del=c*(ff+r*q);
			sum += del;
			del1=c*p-i*del;
			sum1 += del1;
			if (fabs(del) < (1.0+fabs(sum))*EPS) break;
		}
		if (i > MAXIT)
		{	output("bessrl series failed to converge\n");
			error=1; return;
		}
		rymu = -sum;
		ry1 = -sum1*xi2;
		rymup=xmu*xi*rymu-ry1;
		rjmu=w/(rymup-f*rymu);
	} else {
		a=0.25-xmu2;
		p = -0.5*xi;
		q=1.0;
		br=2.0*x;
		bi=2.0;
		fact=a*xi/(p*p+q*q);
		cr=br+q*fact;
		ci=bi+p*fact;
		den=br*br+bi*bi;
		dr=br/den;
		di = -bi/den;
		dlr=cr*dr-ci*di;
		dli=cr*di+ci*dr;
		temp=p*dlr-q*dli;
		q=p*dli+q*dlr;
		p=temp;
		for (i=2;i<=MAXIT;i++) {
			a += 2*(i-1);
			bi += 2.0;
			dr=a*dr+br;
			di=a*di+bi;
			if (fabs(dr)+fabs(di) < FPMIN) dr=FPMIN;
			fact=a/(cr*cr+ci*ci);
			cr=br+cr*fact;
			ci=bi-ci*fact;
			if (fabs(cr)+fabs(ci) < FPMIN) cr=FPMIN;
			den=dr*dr+di*di;
			dr /= den;
			di /= -den;
			dlr=cr*dr-ci*di;
			dli=cr*di+ci*dr;
			temp=p*dlr-q*dli;
			q=p*dli+q*dlr;
			p=temp;
			if (fabs(dlr-1.0)+fabs(dli) < EPS) break;
		}
		if (i > MAXIT)
		{	output("cf2 failed in bessjy\n");
			error=1; return;
		}
		gam=(p-f)/q;
		rjmu=sqrt(w/((p-f)*gam+q));
		rjmu=SIGN(rjmu,rjl);
		rymu=rjmu*gam;
		rymup=rymu*(p+q/gam);
		ry1=xmu*xi*rymu-rymup;
	}
	fact=rjmu/rjl;
	*rj=rjl1*fact;
	*rjp=rjp1*fact;
	for (i=1;i<=nl;i++) {
		rytemp=(xmu+i)*xi2*ry1-rymu;
		rymu=ry1;
		ry1=rytemp;
	}
	*ry=rymu;
	*ryp=xnu*xi*rymu-ry1;
}
#undef EPS
#undef FPMIN
#undef MAXIT
#undef XMIN
#undef PI

void besselj (double *x, double *k, double *r)
{	double h1,h2;
	bessjy(*k,*x,r,&h1,&h2,&h2);
}

void mbesselj (header *hd)
{   spreadf2(besselj,0,0,hd,"besselj");
	test_error("besselj");
}

void bessely (double *x, double *k, double *r)
{	double h1,h2;
	bessjy(*k,*x,&h1,r,&h2,&h2);
}

void mbessely (header *hd)
{   spreadf2(bessely,0,0,hd,"bessely");
	test_error("bessely");
}

void mbesselall (header *hdx)
{	header *st=hdx,*hd=nextof(hdx);
	double bj,by,bdj,bdy;
	hd=getvalue(hd); if (error) return;
	hdx=getvalue(hdx); if (error) return;
	if (hd->type!=s_real || hdx->type!=s_real)
		wrong_arg_in("besselall");
	bessjy(*realof(hd),*realof(hdx),&bj,&by,&bdj,&bdy);
	if (error) return;
	newram=(char *)st;
	new_real(bj,""); if (error) return;
	new_real(by,""); if (error) return;
	new_real(bdj,""); if (error) return;
	new_real(bdy,"");
}

#define EPS 1.0e-16
#define FPMIN 1.0e-30
#define MAXIT 10000
#define XMIN 2.0
#define PI M_PI

void bessik (double xnu, double x, double *ri, double *rk, double *rip, double *rkp)
{
	void nrerror(char error_text[]);
	int i,l,nl;
	double a,a1,b,c,d,del,del1,delh,dels,e,f,fact,fact2,ff,gam1,gam2,
		gammi,gampl,h,p,pimu,q,q1,q2,qnew,ril,ril1,rimu,rip1,ripl,
		ritemp,rk1,rkmu,rkmup,rktemp,s,sum,sum1,x2,xi,xi2,xmu,xmu2;


	if (xnu < 0.0)
	{
		int ixnu=(int)xnu;
		if (ixnu==xnu)
		{
			bessik(-xnu,x,ri,rk,rip,rkp);
			if (ixnu%2) *ri=-*ri;
			return;
		}
		output("bad arguments in bessel\n");
		error = 1; return;
	}
	if (x < -epsilon)
	{
		int ixnu=(int)xnu;
		if (ixnu==xnu)
		{
			bessik(xnu,-x,ri,rk,rip,rkp);
			if (ixnu%2) *ri=-*ri;
			return;
		}
		output("bad arguments in bessel\n");
		error = 1; return;
	}
	else if (x < epsilon)
	{
		if (xnu<epsilon) *ri=1;
		else *ri=0;
		return;
	}
	nl=(int)(xnu+0.5);
	xmu=xnu-nl;
	xmu2=xmu*xmu;
	xi=1.0/x;
	xi2=2.0*xi;
	h=xnu*xi;
	if (h < FPMIN) h=FPMIN;
	b=xi2*xnu;
	d=0.0;
	c=h;
	for (i=1;i<=MAXIT;i++) {
		b += xi2;
		d=1.0/(b+d);
		c=b+1.0/c;
		del=c*d;
		h=del*h;
		if (fabs(del-1.0) < EPS) break;
	}
	if (i > MAXIT)
	{	output("x too large in bessik\n");
		error=1; return;
	}
	ril=FPMIN;
	ripl=h*ril;
	ril1=ril;
	rip1=ripl;
	fact=xnu*xi;
	for (l=nl;l>=1;l--) {
		ritemp=fact*ril+ripl;
		fact -= xi;
		ripl=fact*ritemp+ril;
		ril=ritemp;
	}
	f=ripl/ril;
	if (x < XMIN) {
		x2=0.5*x;
		pimu=PI*xmu;
		fact = (fabs(pimu) < EPS ? 1.0 : pimu/sin(pimu));
		d = -log(x2);
		e=xmu*d;
		fact2 = (fabs(e) < EPS ? 1.0 : sinh(e)/e);
		beschb(xmu,&gam1,&gam2,&gampl,&gammi);
		ff=fact*(gam1*cosh(e)+gam2*fact2*d);
		sum=ff;
		e=exp(e);
		p=0.5*e/gampl;
		q=0.5/(e*gammi);
		c=1.0;
		d=x2*x2;
		sum1=p;
		for (i=1;i<=MAXIT;i++) {
			ff=(i*ff+p+q)/(i*i-xmu2);
			c *= (d/i);
			p /= (i-xmu);
			q /= (i+xmu);
			del=c*ff;
			sum += del;
			del1=c*(p-i*ff);
			sum1 += del1;
			if (fabs(del) < fabs(sum)*EPS) break;
		}
		if (i > MAXIT)
		{
		  output("bessk series failed to converge\n");
		  error = 1; return;
		}
		rkmu=sum;
		rk1=sum1*xi2;
	} else {
		b=2.0*(1.0+x);
		d=1.0/b;
		h=delh=d;
		q1=0.0;
		q2=1.0;
		a1=0.25-xmu2;
		q=c=a1;
		a = -a1;
		s=1.0+q*delh;
		for (i=2;i<=MAXIT;i++) {
			a -= 2*(i-1);
			c = -a*c/i;
			qnew=(q1-b*q2)/a;
			q1=q2;
			q2=qnew;
			q += c*qnew;
			b += 2.0;
			d=1.0/(b+a*d);
			delh=(b*d-1.0)*delh;
			h += delh;
			dels=q*delh;
			s += dels;
			if (fabs(dels/s) < EPS) break;
		}
		if (i > MAXIT)
		{
		  output("bessik: failure to converge in cf2\n");
		  error = 1; return;
		}
		h=a1*h;
		rkmu=sqrt(PI/(2.0*x))*exp(-x)/s;
		rk1=rkmu*(xmu+x+0.5-h)*xi;
	}
	rkmup=xmu*xi*rkmu-rk1;
	rimu=xi/(f*rkmu-rkmup);
	*ri=(rimu*ril1)/ril;
	*rip=(rimu*rip1)/ril;
	for (i=1;i<=nl;i++) {
		rktemp=(xmu+i)*xi2*rk1+rkmu;
		rkmu=rk1;
		rk1=rktemp;
	}
	*rk=rkmu;
	*rkp=xnu*xi*rkmu-rk1;
}
#undef EPS
#undef FPMIN
#undef MAXIT
#undef XMIN
#undef PI

void besseli (double *x, double *k, double *r)
{	double h1,h2;
	bessik(*k,*x,r,&h1,&h2,&h2);
}

void mbesseli (header *hd)
{   spreadf2(besseli,0,0,hd,"besseli");
	test_error("besseli");
}

void besselk (double *x, double *k, double *r)
{	double h1,h2;
	bessik(*k,*x,&h1,r,&h2,&h2);
}

void mbesselk (header *hd)
{   spreadf2(besselk,0,0,hd,"besselk");
	test_error("besselk");
}

void mbesselmodall (header *hdx)
{	header *st=hdx,*hd=nextof(hdx);
	double bj,by,bdj,bdy;
	hd=getvalue(hd); if (error) return;
	hdx=getvalue(hdx); if (error) return;
	if (hd->type!=s_real || hdx->type!=s_real)
		wrong_arg_in("besselmodall");
	bessik(*realof(hd),*realof(hdx),&bj,&by,&bdj,&bdy);
	if (error) return;
	newram=(char *)st;
	new_real(bj,""); if (error) return;
	new_real(by,""); if (error) return;
	new_real(bdj,""); if (error) return;
	new_real(bdy,"");
}


