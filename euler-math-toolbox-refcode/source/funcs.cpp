#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>

#include <regex>
#include <string>

#include "header.h"
#include "sysdep.h"
#include "funcs.h"
#include "matheh.h"
#include "polynom.h"
#include "interval.h"
#include "spread.h"
#include "express.h"
#include "stack.h"
#include "builtin.h"
#include "getvalue.h"
#include "otherfunctions.h"

#include "AlgLib\src\specialfunctions.h"

int usecomplex=0;

char fktext[12][64];

// Not more than MAXARGN
char *argname[] =
	{ "arg1","arg2","arg3","arg4","arg5","arg6","arg7","arg8","arg9",
		"arg10",
	"arg11","arg12","arg13","arg14","arg15","arg16","arg17","arg18","arg19",
		"arg20"	} ;
int xors[20];

void csin (double *x, double *xi, double *z, double *zi)
{	*z=cosh(*xi)*sin(*x);
	*zi=sinh(*xi)*cos(*x);
}

void msin (header *hd)
{	spread1i(sin,csin,isin,hd);
	test_error("sin");
}

void ccos (double *x, double *xi, double *z, double *zi)
{	*z=cosh(*xi)*cos(*x);
	*zi=-sinh(*xi)*sin(*x);
}

void mcos (header *hd)
{	spread1i(cos,ccos,icos,hd);
	test_error("cos");
}

void ctan (double *x, double *xi, double *z, double *zi)
{	double s,si,c,ci;
	csin(x,xi,&s,&si); ccos(x,xi,&c,&ci);
	complex_divide(&s,&si,&c,&ci,z,zi);
}

double rtan (double x)
{	if (cos(x)==0.0) return NAN;
	return tan(x);
}

void mtan (header *hd)
{	spread1i(
#ifdef FLOAT_TEST
	rtan,
#else
	tan,
#endif
	ctan,itan,hd);
	test_error("tan");
}

double ratan (double x)
{	if (x<=-M_PI && x>=M_PI)
	{	
		if (!pnan && !error) 
		{	output("atan defined between -pi and pi!\n"); error=ERROR_RANGE; 
		}
		return NAN; 
	}
	else return atan(x);
}

void carg (double *x, double *xi, double *z)
{
#ifdef FLOAT_TEST
	if (*x==0.0 && *xi==0.0) *z=0.0;
#endif
	*z = atan2(*xi,*x);
}

double rlog (double x)
{	
#ifdef FLOAT_TEST
	if (x<=0)
	{	if (!pnan && !error) 
		{	
			output("log defined only for positive numbers!\n"); 
			error=ERROR_RANGE; 
		}
		return NAN; 
	}
	else
#endif
	return log(x);
}

void cclog (double *x, double *xi, double *z, double *zi)
{
#ifdef FLOAT_TEST
	*z=rlog(sqrt(*x * *x + *xi * *xi));
#else
	*z=log(sqrt(*x * *x + *xi * *xi));
#endif
	carg(x,xi,zi);
}

double rsign (double x)
{	if (x<0) return -1;
	else if (x<=0) return 0;
	else return 1;
}

void msign (header *hd)
{	spread1(rsign,0,hd);
	test_error("sign");
}

double rdegree (double x)
{	return x/180*M_PI;
}

void mdegree (header *hd)
{	spread1i(rdegree,0,idegree,hd);
	test_error("degree");
}

double rpercent (double x)
{	return x*0.01;
}

void mpercent (header *hd)
{	spread1i(rpercent,0,ipercent,hd);
	test_error("percent");
}

void catan (double *x, double *xi, double *y, double *yi)
{	double h,hi,g,gi,t,ti;
	h=1-*xi; hi=*x; g=1+*xi; gi=-*x;
	complex_divide(&h,&hi,&g,&gi,&t,&ti);
	cclog(&t,&ti,&h,&hi);
	*y=hi/2; *yi=-h/2;
}

void matan (header *hd)
{	spread1i(
#ifdef FLOAT_TEST
	ratan,
#else
	atan,
#endif
	catan,iatan,hd);
	test_error("atan");
}

double rasin (double x)
{	if (x<-1 || x>1)
	{	if (!pnan && !error) 
		{	output("Floating point error!\n"); error=ERROR_FLOATING_POINT; 
		}
		return NAN; 
	}
	else return asin(x);
}

void csqrt (double *x, double *xi, double *z, double *zi)
{	double a,r;
	carg(x,xi,&a); a=a/2.0;
	r=sqrt(sqrt(*x * *x + *xi * *xi));
	*z=r*cos(a);
	*zi=r*sin(a);
}

void casin (double *x, double *xi, double *y, double *yi)
{	double h,hi,g,gi;
	complex_multiply(x,xi,x,xi,&h,&hi);
	h=1-h; hi=-hi;
	csqrt(&h,&hi,&g,&gi);
	h=-*xi+g; hi=*x+gi;
	cclog(&h,&hi,yi,y);
	*yi=-*yi;
}

void masin (header *hd)
{	spread1i(
#ifdef FLOAT_TEST
	rasin,
#else
	asin,
#endif
	casin,iasin,hd);
	test_error("asin");
}

double racos (double x)
{	if (x<-1 || x>1)
	{	if (!pnan && !error) 
		{	output("Floating point error!\n"); error=ERROR_FLOATING_POINT; 
		}
		return NAN; 
	}
	else return acos(x);
}

void cacos (double *x, double *xi, double *y, double *yi)
{	double h,hi,g,gi;
	complex_multiply(x,xi,x,xi,&h,&hi);
	h=1-h; hi=-hi;
	csqrt(&h,&hi,&g,&gi);
	hi=*xi+g; h=*x-gi;
	cclog(&h,&hi,yi,y);
	*yi=-*yi;
}

void macos (header *hd)
{	spread1i(
#ifdef FLOAT_TEST
	racos,
#else
	acos,
#endif
	cacos,iacos,hd);
	test_error("acos");
}

void cexp (double *x, double *xi, double *z, double *zi)
{	double r=exp(*x);
	*z=cos(*xi)*r;
	*zi=sin(*xi)*r;
}

void rcexp (double *x, double *xi, double *z, double *zi)
{	double r;
    if (*x>=690)
	{	if (!pnan && !error) 
		{	output("Floating point error!\n"); error=ERROR_FLOATING_POINT; 
		}
		*z=*zi=NAN;
	}
	else if (*x<=700)
	{	*z=*zi=0;
	}
    r=exp(*x);
	*z=cos(*xi)*r;
	*zi=sin(*xi)*r;
}

inline double rexp (double x)
{	if (x>=690)
	{	if (!pnan && !error) 
		{	output("Floating point error!\n"); error=ERROR_FLOATING_POINT; 
		}
		return NAN;
	}
	else if (x<=-690)
	{	if (underflows && !pnan && !error)
		{	output("Floating point error!\n"); error=ERROR_FLOATING_POINT; 
		}
		return 0;
	}
	else return exp(x);
}

void mexp (header *hd)
{
#ifdef FLOAT_TEST
	spread1i(rexp,rcexp,iexp,hd);
#else
	spread1i(exp,cexp,iexp,hd);
#endif
	test_error("exp");
}

double rarg (double x)
{	if (x>=0) return 0.0;
	else return M_PI;
}

void mlog (header *hd)
{
	if (usecomplex) mcomplex(hd);
#ifdef FLOAT_TEST
	spread1i(rlog,cclog,ilog,hd);
#else
	spread1i(log,cclog,ilog,hd);
#endif
	test_error("log");
}

double rsqrt (double x)
{	
	if (x<0.0)
	{	
		if (!pnan && !error) 
		{	output("Floating point error!\n"); error=ERROR_FLOATING_POINT; 
		}
		return NAN; 
	}
	else return sqrt(x);
}

void msqrt (header *hd)
{
	if (usecomplex) mcomplex(hd);
	spread1i(
#ifdef FLOAT_TEST
	rsqrt,
#else
	sqrt,
#endif
	csqrt,isqrt,hd);
	test_error("sqrt");
}

void mceil (header *hd)
{	spread1(ceil,0,hd);
	test_error("ceil");
}

void mfloor (header *hd)
{	spread1(floor,0,hd);
	test_error("floor");
}

void cconj (double *x, double *xi, double *z, double *zi)
{	*zi=-*xi; *z=*x;
}

double ident (double x)
{	return x;
}

void mconj (header *hd)
{	spread1(ident,cconj,hd);
	test_error("conj");
}

double rnot (double x)
{	if (x!=0.0) return 0.0;
	else return 1.0;
}

void cnot (double *x, double *xi, double *r)
{	if (*x==0.0 && *xi==0.0) *r=1.0;
	else *r=0.0;
}

void mnot (header *hd)
{	spread1r(rnot,cnot,hd);
	test_error("!");
}

double rbitnot (double x)
{	return 0xFFFFFFFF^((unsigned int)x);
}

void mbitnot (header *hd)
{	spread1r(rbitnot,0,hd);
	test_error("bitnot");
}

void crealpart (double *x, double *xi, double *z)
{	*z=*x;
}

void mre (header *hd)
{	spread1r(ident,crealpart,hd);
	test_error("re");
}

void creal (double *x, double *xi, double *z)
{	if (fabs(*xi)>epsilon)
	{	if (!pnan && !error) 
		{	output("Not a real number!\n"); error=ERROR_RANGE;
		}
		*z=NAN; 
		return;
	}
	*z=*x;
}

void mreal (header *hd)
{	spread1r(ident,creal,hd);
}

double zero (double x)
{	return 0.0;
}

void cimagpart (double *x, double *xi, double *z)
{	*z=*xi;
}

void mim (header *hd)
{	spread1r(zero,cimagpart,hd);
	test_error("im");
}

void marg (header *hd)
{	spread1r(rarg,carg,hd);
	test_error("arg");
}

void cxabs (double *x, double *xi, double *z)
{	*z=sqrt(*x * *x + *xi * *xi);
}

void mabs (header *hd)
{   spread1ir(fabs,cxabs,iabs,hd);
	test_error("abs");
}

void mpi (header *hd)
{	new_real(M_PI,"");
}

void mfalse (header *hd)
{	new_real(0,"");
}

void mtrue (header *hd)
{	new_real(1,"");
}

void mnan (header *hd)
{	new_real(NAN,"");
}

double risnan (double x)
{	return !(x==x);
}

void cisnan (double *x, double *y, double *xr)
{	*xr=!(*x==*x && *y==*y);
}

void misnan (header *hd)
{	spread1r(risnan,cisnan,hd);
}

void mE (header *hd)
{	new_real(2.718281828459045235360287,"");
}

void mI (header *hd)
{	new_complex(0,1,"");
}

void margn (header *hd)
{	new_real(actargn,"");
}

void mtime (header *hd)
{	new_real(myclock(),"");
}

void mgetnow (header *hd)
{
	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real) need_arg_in("getnow","utc-flag");
	int year,month,day,hour,minute,second,millisecond;
	getnow(&year,&month,&day,&hour,&minute,&second,&millisecond,(int)(*realof(hd)));
	result=new_matrix(1,7,""); if (error) return;
	double *m=matrixof(result);
	*m++=year; *m++=month; *m++=day;
	*m++=hour; *m++=minute; *m++=second; *m++=millisecond;
	moveresult(st,result);
}

void mprintnow (header *hd)
{
	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real) need_arg_in("printnow","utc-flag");
	char s[1024];
	printnow(s,1024,(int)(*realof(hd)));
	result=new_string(s,strlen(s)+2,""); if (error) return;
	moveresult(st,result);
}

void mhardspace (header *hd)
{
	new_real(hardspace,"");
}

void mfree (header *hd)
{	new_real((double)(ramend-endlocal),"");
}

void mepsilon (header *hd)
{	new_real(epsilon,"");
}

void msetepsilon (header *hd)
{	header *stack=hd,*hd1,*result;
	hd1=getvalue(hd); if (error) return;
	if (hd1->type!=s_real) need_arg_in("setepsilon","real");
	result=new_real(epsilon,"");
	changedepsilon=epsilon=*realof(hd1);
	moveresult(stack,result);
}

void mlocalepsilon (header *hd)
{	header *stack=hd,*hd1,*result;
	hd1=getvalue(hd); if (error) return;
	if (hd1->type!=s_real) need_arg_in("localepsilon","real");
	result=new_real(epsilon,"");
	epsilon=*realof(hd1);
	moveresult(stack,result);
}

void mindex (header *hd)
{	new_real((double)loopindex,"");
}

void rmod (double *x, double *n, double *y)
{
	if (*n==0.0)
		*y=*x;
	else {
		*y=fmod(*x,*n);
		if (*y<0) *y+=*n;
	}
}

void mmod (header *hd)
{	spreadf2(rmod,0,0,hd,"mod");
	test_error("mod");
}

void cpow (double *x, double *xi, double *y, double *yi,
	double *z, double *zi)
{	double l,li,w,wi;
	if (fabs(*y)<epsilon && fabs(*yi)<epsilon)
	{	*z=1.0; *zi=0.0; return;
	}
	if (fabs(*x)<epsilon && fabs(*xi)<epsilon)
	{	*z=*zi=0.0; return;
	}
	cclog(x,xi,&l,&li);
	complex_multiply(y,yi,&l,&li,&w,&wi);
	cexp(&w,&wi,z,zi);
}

void rpow (double *x, double *y, double *z)
{	int n;
	if (*x>0.0)
	{	
		if (error) *z=pow(*x,*y);
		else
		{	
			*z=pow(*x,*y);
			if (error && log(*x)*(*y)<0)
			{	
				if (!underflows) error=0;
				*z=0;
			}
		}
	}
	else if (*x==0.0) 
	{	if (*y==0.0) *z=1.0; else *z=0.0;
	}
	else if (*y==0.0)
	{	*z=1.0;
	}
	else if ((n=(int)*y)==*y)
	{	if (n%2) *z=-pow(-*x,n);
		else *z=pow(-*x,n);
	}
	else if ((n=(int)(1/(*y)))==1/(*y) && (n%2)==1)
	{	*z=-pow(-*x,*y);
	}
	else
	{   if (pnan) *z=NAN;
		else
		{	
			output("^ defined for positive numbers or integer exponent!\n"); 
			output("Use complex numbers?\n"); 
			error=ERROR_WRONG_ARGUMENT;
		}
		return;
	}
}

void mpower (header *hd)
{	
	if (usecomplex)
	{
		header *hd1=nextof(hd);
		mcomplex(hd1);
	}
	spreadf2(rpow,cpow,ipow,hd,"^");
	if (error==ERROR_FLOATING_POINT)
	{
		output("Underflow or overflow error.\n");
	}
	test_error("^");
}

void rrelerror (double *x, double *y, double *z)
{	if (fabs(*y)<epsilon) *z=fabs(*x);
	else *z=fabs((*x)/(*y)-1);
}

void crelerror (double *x, double *xi, double *y, double *yi, 
	double *z)
{	double r=(*y)*(*y)+(*yi)*(*yi);
	if (sqrt(r)<epsilon)
	{	*z=sqrt((*x)*(*x)+(*xi)*(*xi));
	}
	else
	{	*z=sqrt(((*x)*(*x)+(*xi)*(*xi))/r)-1;
	}
}

void mrelerror (header *hd)
{	spreadf2r(rrelerror,crelerror,0,hd,"relerror");
	test_error("relerror")
}

void rbitand(double *x, double *y, double *z)
{	*z=((unsigned int)*x)&((unsigned int)*y);
}

void mbitand (header *hd)
{	spreadf2r(rbitand,0,0,hd,"bitand");
	test_error("bitand");
}

void rbitor(double *x, double *y, double *z)
{	*z=((unsigned int)*x)|((unsigned int)*y);
}

void mbitor (header *hd)
{	spreadf2r(rbitor,0,0,hd,"bitor");
	test_error("bitor");
}

void rbitxor(double *x, double *y, double *z)
{	*z=((unsigned int)*x)^((unsigned int)*y);
}

void mbitxor (header *hd)
{	spreadf2r(rbitxor,0,0,hd,"bitxor");
	test_error("bitxor");
}

void rgreater (double *x, double *y, double *z)
{	if (*x>*y) *z=1.0;
	else *z=0.0;
}

void igreater (double *xa, double *xb, double *ya, double *yb,
	double *z)
{	if (*xa>*yb) *z=1.0;
	else *z=0.0;
}

void mgreater (header *hd)
{   header *st=hd,*hd1,*result,*hdv;
	hdv=getvariable(hd);
	if (hdv->type==s_string)
	{   hd=getvalue(hd);
		hd1=getvalue(nextof(st)); if (error) return;
		if (hd1->type!=s_string) need_arg_in(">","two strings");
		result=new_real(strcmp(stringof(hd),stringof(hd1))>0,"");
		moveresult(st,result);
	}
	else spreadf2r(rgreater,0,igreater,st,"==");
	test_error("==");
}

void rless (double *x, double *y, double *z)
{	if (*x<*y) *z=1.0;
	else *z=0.0;
}

void ilesst (double *xa, double *xb, double *ya, double *yb,
	double *z)
{	if (*xb<*ya) *z=1.0;
	else *z=0.0;
}

void mless (header *hd)
{   header *st=hd,*hd1,*result,*hdv;
	hdv=getvariable(hd);
	if (hdv->type==s_string)
	{   hd=getvalue(hd);
		hd1=getvalue(nextof(st)); if (error) return;
		if (hd1->type!=s_string) need_arg_in("<","two strings");
		result=new_real(strcmp(stringof(hd),stringof(hd1))<0,"");
		moveresult(st,result);
	}
	else spreadf2r(rless,0,ilesst,st,"<");
	test_error("<");
}

void rgreatereq (double *x, double *y, double *z)
{	if (*x>=*y) *z=1.0;
	else *z=0.0;
}

void igreatereq (double *xa, double *xb, double *ya, double *yb,
	double *z)
{	if (*xa>=*yb) *z=1.0;
	else *z=0.0;
}

void mgreatereq (header *hd)
{   header *st=hd,*hd1,*result,*hdv;
	hdv=getvariable(hd);
	if (hdv->type==s_string)
	{   hd=getvalue(hd);
		hd1=getvalue(nextof(st)); if (error) return;
		if (hd1->type!=s_string) need_arg_in(">=","two strings");
		result=new_real(strcmp(stringof(hd),stringof(hd1))>=0,"");
		moveresult(st,result);
	}
	else spreadf2r(rgreatereq,0,igreatereq,st,">=");
	test_error(">=");
}

void rlesseq (double *x, double *y, double *z)
{	if (*x<=*y) *z=1.0;
	else *z=0.0;
}

void ilesseq (double *xa, double *xb, double *ya, double *yb,
	double *z)
{	if (*xb<=*ya) *z=1.0;
	else *z=0.0;
}

void mlesseq (header *hd)
{   header *st=hd,*hd1,*result,*hdv;
	hdv=getvariable(hd);
	if (hdv->type==s_string)
	{   hd=getvalue(hd);
		hd1=getvalue(nextof(st)); if (error) return;
		if (hd1->type!=s_string) need_arg_in("<=","two strings");
		result=new_real(strcmp(stringof(hd),stringof(hd1))<=0,"");
		moveresult(st,result);
	}
	else spreadf2r(rlesseq,0,ilesseq,st,"<=");
	test_error("<=");
}

void ror (double *x, double *y, double *z)
{	if (*x!=0.0 || *y!=0.0) *z=1.0;
	else *z=0.0;
}

void mor (header *hd)
{	spreadf2(ror,0,ior,hd,"||");
	test_error("||");
}

void mintersects (header *hd)
{	spreadf2r(0,0,iintersects,hd,"&&");
	test_error("intersects");
}

void rrand (double *x, double *y, double *z)
{	if (*x!=0.0 && *y!=0.0) *z=1.0;
	else *z=0.0;
}

void mand (header *hd)
{	spreadf2(rrand,0,iand,hd,"&&");
	test_error("&&");
}

void requal (double *x, double *y, double *z)
{	
	if (isnan(*x) && isnan(*y)) *z=1;
	else if (*x==*y) *z=1.0;
	else *z=0.0;
}

void cequal (double *x, double *xi, double *y, double *yi, double *z)
{	if (*x==*y && *xi==*yi) *z=1.0;
	else *z=0.0;
}

void mtype (header *hd)
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	result=new_real(hd->type,"");
	moveresult(st,result);
}

void missymbolic (header *hd)
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	result=new_real(hd->type==s_string && (hd->flags&SYMBOLIC),"");
	moveresult(st,result);
}

void mprinttype (header *hd)
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	char *p=s_names[hd->type];
	result=new_string(p,strlen(p),"");
	moveresult(st,result);
}

void mequal (header *hd)
{   header *st=hd,*hd1,*result;
	hd=getvalue(hd); if (error) return;
	hd1=nextof(st); hd1=getvalue(hd1); if (error) return;
	if (hd->type==s_string)
	{   if (hd1->type!=s_string) result=new_real(0,"");
		else result=new_real(strcmp(stringof(hd),stringof(hd1))==0,"");
		moveresult(st,result);
	}
	else if (hd1->type==s_string)
	{	result=new_real(0,"");
		moveresult(st,result);
	}
	else
	{	result=mapf2r(requal,cequal,cequal,hd,hd1,"==");
		if (!error) moveresult(st,result);
	}
	test_error("==");
}

void runequal (double *x, double *y, double *z)
{	
	if (isnan(*x) && !isnan(*y)) *z=1;
	if (!isnan(*x) && isnan(*y)) *z=1;
	if (*x!=*y) *z=1.0;
	else *z=0.0;
}

void cunequal (double *x, double *xi, double *y, double *yi, double *z)
{	if (*x!=*y || *xi!=*yi) *z=1.0;
	else *z=0.0;
}

void iunequal (double *x, double *xi, double *y, double *yi, double *z)
{	if (*x!=*y || *xi!=*yi) *z=1.0;
	else *z=0.0;
}

void munequal (header *hd)
{   header *st=hd,*hd1,*result;
	hd=getvalue(hd); if (error) return;
	hd1=nextof(st); hd1=getvalue(hd1); if (error) return;
	if (hd->type==s_string)
	{   if (hd1->type!=s_string) result=new_real(0,"");
		else result=new_real(strcmp(stringof(hd),stringof(hd1))!=0,"");
		moveresult(st,result);
	}
	else if (hd1->type==s_string)
	{	result=new_real(1,"");
		moveresult(st,result);
	}
	else
	{	result=mapf2r(runequal,cunequal,iunequal,hd,hd1,"<>");
		if (!error) moveresult(st,result);
	}
	test_error("<>");
}


void raboutequal (double *x, double *y, double *z)
{   double rx=fabs(*x),ry=fabs(*y);
	if (rx<epsilon)
	{	if (ry<epsilon) *z=1.0;
		else *z=0.0;
	}
	else if (ry<epsilon)
	{	if (rx<epsilon) *z=1.0;
		else *z=0.0;
	}
	else if (fabs(*x-*y)/rx<epsilon) *z=1.0;
	else *z=0.0;
}

void caboutequal
	(double *x, double *xi, double *y, double *yi, double *z)
{	raboutequal(x,y,z);
	if (*z!=0.0) raboutequal(xi,yi,z);
}

void iaboutequal (double *x, double *xi, double *y, double *yi, double *z)
{	raboutequal(x,y,z);
	if (*z!=0.0) raboutequal(xi,yi,z);
}

void maboutequal (header *hd)
{	spreadf2r(raboutequal,caboutequal,iaboutequal,hd,"~=");
	test_error("~=");
}

void mlusolve (header *hd)
{	header *st=hd,*hd1,*result;
	double *m,*m1;
	int r,c,r1,c1;
	hd=getvalue(hd);
	hd1=next_param(st);
	if (hd1) hd1=getvalue(hd1);
	if (error) return;
	if (hd->type==s_matrix || hd->type==s_real)
	{	getmatrix(hd,&r,&c,&m);
		if (hd1->type==s_cmatrix)
		{	make_complex(st);
			mlusolve(st); return;
		}
		if (hd1->type!=s_matrix && hd1->type!=s_real)
			need_arg_in("lusolve","nxn matrix A, nxm matrix or vector b");
		getmatrix(hd1,&r1,&c1,&m1);
		if (c!=r || c<1 || r!=r1)
			need_arg_in("lusolve","nxn matrix A, nxm matrix or vector b");
		result=new_matrix(r,c1,""); if (error) return;
		lu_solve(m,r,m1,c1,matrixof(result));
		if (error) return;
		moveresult(st,result);
	}
	else if (hd->type==s_cmatrix || hd->type==s_complex)
	{	getmatrix(hd,&r,&c,&m);
		if (hd1->type==s_matrix || hd1->type==s_real)
		{	make_complex(next_param(st));
			mlusolve(st); return;
		}
		if (hd1->type!=s_cmatrix && hd1->type!=s_complex)
			need_arg_in("lusolve","nxn matrix A, nxm matrix or vector b");
		getmatrix(hd1,&r1,&c1,&m1);
		if (c!=r || c<1 || r!=r1)
			need_arg_in("lusolve","nxn matrix A, nxm matrix or vector b");
		result=new_cmatrix(r,c1,""); if (error) return;
		clu_solve(m,r,m1,c1,matrixof(result));
		if (error) return;
		moveresult(st,result);
	}
	else
			need_arg_in("lusolve","nxn matrix A, nxm matrix or vector b");
}

void msolve (header *hd)
{	header *st=hd,*hd1,*result;
	double *m,*m1;
	int r,c,r1,c1;
	hd1=nextof(st);
	equal_params_2(&hd,&hd1);
	if (error) return;
	if (hd->type==s_matrix || hd->type==s_real)
	{	getmatrix(hd,&r,&c,&m);
		if (hd1->type==s_cmatrix)
		{	make_complex(st);
			msolve(st); return;	
		}
		if (hd1->type!=s_matrix && hd1->type!=s_real)
			need_arg_in("\\","nxn matrix A, nxm matrix or vector b");
		getmatrix(hd1,&r1,&c1,&m1);
		if (c!=r || c<1 || r!=r1)
			need_arg_in("\\","nxn matrix A, nxm matrix or vector b");
		result=new_matrix(r,c1,""); if (error) return;
		solvesim(m,r,m1,c1,matrixof(result));
		if (error) return;
		moveresult(st,result);
	}
	else if (hd->type==s_cmatrix || hd->type==s_complex)
	{	getmatrix(hd,&r,&c,&m);
		if (hd1->type==s_matrix || hd1->type==s_real)
		{	make_complex(next_param(st));
			msolve(st); return;
		}
		if (hd1->type!=s_cmatrix && hd1->type!=s_complex)
			need_arg_in("\\","nxn matrix A, nxm matrix or vector b");
		getmatrix(hd1,&r1,&c1,&m1);
		if (c!=r || c<1 || r!=r1)
			need_arg_in("\\","nxn matrix A, nxm matrix or vector b");
		result=new_cmatrix(r,c1,""); if (error) return;
		c_solvesim(m,r,m1,c1,matrixof(result));
		if (error) return;
		moveresult(st,result);
	}
	/* Needs to be fixed or cancelled :
	else if (hd->type==s_imatrix || hd->type==s_interval)
	{	getmatrix(hd,&r,&c,&m);
		if (hd1->type==s_matrix || hd1->type==s_real)
		{	make_interval(next_param(st));
			msolve(st); return;
		}
		if (hd1->type!=s_imatrix && hd1->type!=s_interval)
			wrong_arg_in("\\");
		getmatrix(hd1,&r1,&c1,&m1);
		if (c!=r || c<1 || r!=r1) wrong_arg_in("\\");
		result=new_imatrix(r,c1,""); if (error) return;
		i_solvesim(m,r,m1,c1,matrixof(result));
		if (error) return;
		moveresult(st,result);
	}
	*/
	else
			need_arg_in("\\","nxn matrix A, nxm matrix or vector b");
}

void mcomplex (header *hd)
{	header *st=hd,*result;
	double *m,*mr;
	LONG i,n;
	int c,r;
	hd=getvalue(hd);
	if (hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_cmatrix(r,c,""); if (error) return;
		n=(LONG)r*c;
        mr=matrixof(result)+(LONG)2*(n-1);
		m+=n-1;
		for (i=0; i<n; i++)
		{	*mr=*m--; *(mr+1)=0.0; mr-=2;
		}
		moveresult(st,result);
	}
	else if (hd->type==s_real)
	{	result=new_complex(*realof(hd),0.0,""); if (error) return;
		moveresult(st,result);
	}
}

void msum (header *hd)
{	header *st=hd,*result;
	int c,r,i,j;
	double *m,*mr,s,si,x,y;
	if (usematlab) transpose(hd);
	if (error) return;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_matrix(r,1,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++) 
		{	s=0.0;
			for (j=0; j<c; j++) s+=*m++;
			*mr++=s;
		}
	}
	else if (hd->type==s_complex || hd->type==s_cmatrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_cmatrix(r,1,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++) 
		{	s=0.0; si=0.0;
			for (j=0; j<c; j++) { s+=*m++; si+=*m++; }
			*mr++=s; *mr++=si;
		}
	}
	else if (hd->type==s_interval || hd->type==s_imatrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_imatrix(r,1,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
		{	s=0.0; si=0.0;
			for (j=0; j<c; j++)
			{	interval_add(&s,&si,m,m+1,&x,&y);
				s=x; si=y; m+=2;
			}
			*mr++=s; *mr++=si;
		}
	}
	else need_arg_in("sum","matrix");
	result=getvalue(result);
	moveresult(st,result);
	if (usematlab) transpose(st);
}

void mcolsum (header *hd)
{	
	header *st=hd,*result;
	int c,r,i,j;
	double *m,*mr,s,si,x,y;
	if (usematlab) transpose(hd);
	if (error) return;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	
		getmatrix(hd,&r,&c,&m);
		result=new_matrix(1,c,""); if (error) return;
		mr=matrixof(result);
		for (j=0; j<c; j++) 
		{	
			s=0.0;
			for (i=0; i<r; i++) s+=m[c*i+j];
			*mr++=s;
		}
	}
	else if (hd->type==s_complex || hd->type==s_cmatrix)
	{	
		getmatrix(hd,&r,&c,&m);
		result=new_cmatrix(1,c,""); if (error) return;
		mr=matrixof(result);
		for (j=0; j<c; j++) 
		{	
			s=0.0; si=0.0;
			for (i=0; i<r; i++) { s+=m[2*(c*i+j)]; si+=m[2*(c*i+j)+1]; }
			*mr++=s; *mr++=si;
		}
	}
	else if (hd->type==s_interval || hd->type==s_imatrix)
	{	
		getmatrix(hd,&r,&c,&m);
		result=new_imatrix(1,c,""); if (error) return;
		mr=matrixof(result);
		for (j=0; j<c; j++) 
		{	
			s=0.0; si=0.0;
			for (i=0; i<r; i++) 
			{	
				interval_add(&s,&si,m+2*(c*i+j),m+2*(c*i+j),&x,&y);
				s=x; si=y;
			}
			*mr++=s; *mr++=si;
		}
	}
	else need_arg_in("sum","matrix");
	result=getvalue(result);
	moveresult(st,result);
	if (usematlab) transpose(st);
}

void mprod (header *hd)
{	header *st=hd,*result;
	int c,r,i,j;
	double *m,*mr,s,si,h,hi,x,y;
	if (usematlab) transpose(hd);
	if (error) return;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_matrix(r,1,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++) 
		{	s=1.0;
			for (j=0; j<c; j++) s*=*m++;
			*mr++=s;
		}
	}
	else if (hd->type==s_complex || hd->type==s_cmatrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_cmatrix(r,1,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
		{	s=1.0;
			for (j=0; j<c; j++)
			{	complex_multiply(&s,&si,m,m+1,&h,&hi);
				s=h; si=hi; m+=2;
			}
			*mr++=s; *mr++=si;
		}
	}
	else if (hd->type==s_interval || hd->type==s_imatrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_imatrix(r,1,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
		{	s=1.0; si=1.0;
			for (j=0; j<c; j++)
			{	interval_mult(&s,&si,m,m+1,&x,&y);
				s=x; si=y; m+=2;
			}
			*mr++=s; *mr++=si;
		}
	}
	else need_arg_in("prod","matrix");
	result=getvalue(result);
	moveresult(st,result);
	if (usematlab) transpose(st);
}

int smatrixsize (header *hd)
{	
	int i=0;
	char *str=stringof(hd);
	while (*str!=1)
	{	i++;
		str+=strlen(str)+1;
	}
	return i;
}

int listsize (header *hd)
{
	int i=0;
	header *hd1=listof(hd);
	while (((char *)hd1-(char *)hd)<hd->size)
	{
		hd1=nextof(hd1); i++;
	}
	return i;
}

void msize (header *hd)
{	header *result,*st=hd,*hd1,*end=(header *)newram;
	int r,c,r0=0,c0=0;
	if (!hd) need_arg_in("size","matrices");
	result=new_matrix(1,2,""); if (error) return;
	while (end>hd)
	{	
		hd1=getvariable(hd); if (!hd1) varnotfound("size");
		if (hd1->type==s_matrix || hd1->type==s_cmatrix
			|| hd1->type==s_imatrix)
		{	r=dimsof(hd1)->r;
			c=dimsof(hd1)->c;
		}
		else if (hd1->type==s_real || hd1->type==s_complex
			|| hd1->type==s_interval)
		{	r=c=1;
		}
		else if (hd1->type==s_submatrix || hd1->type==s_csubmatrix
			|| hd1->type==s_isubmatrix)
		{	r=submdimsof(hd1)->r;
			c=submdimsof(hd1)->c;
		}
		else if (hd1->type==s_smatrix)
		{	r=1;
			c=smatrixsize(hd1);
		}
		else if (hd1->type==s_string)
		{	r=1;
			c=1;
		}
		else if (hd1->type==s_cpxmatrix)
		{	
			r=dimsof(hd1)->r;
			c=dimsof(hd1)->c;
		}
		else if (hd1->type==s_list)
		{
			r=1;
			c=listsize(hd1);
		}
		else need_arg_in("size","matrices");
		if ((r>1 && r0>1 && r!=r0) || (c>1 && c0>1 && c!=c0))
		{	
			output1("Matrix %dx%d and %dx%d not compatible in size!\n",r0,c0,r,c);
			error=ERROR_MATRIX_SIZE; return;
		}
		else
		{	if (r>r0) r0=r;
			if (c>c0) c0=c;
		}
		hd=nextof(hd);
	}
	*matrixof(result)=r0;
	*(matrixof(result)+1)=c0;
	moveresult(st,result);
}

void mcols (header *hd)
{	header *st=hd,*res;
	int n;
	hd=getvalue(hd); if (error) return;
	switch (hd->type)
	{	case s_matrix :
		case s_cmatrix :
		case s_imatrix : 
		case s_cpxmatrix : n=dimsof(hd)->c; break;
		case s_submatrix :
		case s_csubmatrix :
		case s_isubmatrix : n=submdimsof(hd)->c; break;
		case s_real :
		case s_complex :
		case s_interval : n=1; break;
		case s_string : n=1; break;
		case s_smatrix : n=smatrixsize(hd); break;
		case s_list : n=listsize(hd); break;
		default : need_arg_in("cols","matrix type");
	}
	res=new_real(n,""); if (error) return;
	moveresult(st,res);
}

void mrows (header *hd)
{	header *st=hd,*res;
	int n;
	hd=getvalue(hd); if (error) return;
	switch (hd->type)
	{	case s_matrix :
		case s_cmatrix :
		case s_imatrix :  
		case s_cpxmatrix : n=dimsof(hd)->r; break;
		case s_submatrix :
		case s_csubmatrix :
		case s_isubmatrix : n=submdimsof(hd)->r; break;
		case s_real :
		case s_complex :
		case s_interval : 
		case s_smatrix : 
		case s_list :
		case s_string : n=1; break;
		default : need_arg_in("rows","matrix type");
	}
	res=new_real(n,""); if (error) return;
	moveresult(st,res);
}

void mzerosmat (header *hd)
{	
	header *result,*st=hd;
	double *m;
	int r,c;
	LONG i,n;
	get_size(hd,&r,&c,"zeros");
	if (error) return;
	result=new_matrix(r,c,""); if (error) return;
	m=matrixof(result);
	n=c*r;
	for (i=0; i<n; i++) *m++=0.0;
	moveresult(st,result);
}

void mlinspace (header *hd)
{
	header *st=hd,*hda,*hdb,*hdn,*result;
	hda=hd;
	hdb=nextof(hda);
	hdn=nextof(hdb);
	hda=getvalue(hda); if (error) return;
	hdb=getvalue(hdb); if (error) return;
	hdn=getvalue(hdn); if (error) return;
	if (hda->type!=s_real || hdb->type!=s_real 
		|| hdn->type!=s_real)
		need_arg_in("linspace","real a,b,n");
	int n=(int)(*realof(hdn));
	if (n<1)
		need_arg_in("linspace","integer n>0");
	result=new_matrix(1,n+1,""); if (error) return;
	double a=*realof(hda);
	double b=*realof(hdb);
	double h=(b-a)/n;
	double *m=matrixof(result);
	for (int i=0; i<n; i++)
	{
		*m++=a+i*h;
	}
	*m=b;
	moveresult(st,result);
}

void mones (header *hd)
{	header *result,*st=hd;
	int r,c;
	LONG i,n;
	get_size(hd,&r,&c,"ones");
	if (error) return;
	result=new_matrix(r,c,""); if (error) return;
	double *m=matrixof(result);
	n=c*r;
	for (i=0; i<n; i++) *m++=1.0;
	moveresult(st,result);
}

void mdiag (header *hd)
{	header *result,*st=hd,*hd1,*hd2=0;
	double *m,*md;
	int r,c,i,ik=0,k,rd,cd;
	LONG l,n;
	hd1=get_size(hd,&r,&c,"diag"); if (error) return;
	hd2=next_param(hd1);
	if (!hd2) need_arg_in("diag","size n, m, diagonal k, vector v");
	hd1=getvalue(hd1); if (error) return;
	hd2=getvalue(hd2); if (error) return;
	if	(hd1->type!=s_real) need_arg_in("diag","size n, m, diagonal k, vector v");
	k=(int)*realof(hd1);
	if (hd2->type==s_matrix || hd2->type==s_real)
	{	result=new_matrix(r,c,""); if (error) return;
		m=matrixof(result);
		n=(LONG)c*r;
		for (l=0; l<n; l++) *m++=0.0;
		getmatrix(hd2,&rd,&cd,&md);
		if (rd!=1 || cd<1) need_arg_in("diag","size n, m, diagonal k, vector v");
		m=matrixof(result);
		for (i=0; i<r; i++)
		{	if (i+k>=0 && i+k<c)
			{	*mat(m,c,i,i+k)=*md;
				ik++; if (ik<cd) md++;
			}
		}
	}
	else if (hd2->type==s_cmatrix || hd2->type==s_complex)
	{	result=new_cmatrix(r,c,""); if (error) return;
		m=matrixof(result);
		n=(LONG)2*(LONG)c*r;
		for (l=0; l<n; l++) *m++=0.0;
		getmatrix(hd2,&rd,&cd,&md);
		if (rd!=1 || cd<1) need_arg_in("diag","size n, m, diagonal k, vector v");
		m=matrixof(result);
		for (i=0; i<r; i++)
		{	if (i+k>=0 && i+k<c)
			{	*cmat(m,c,i,i+k)=*md;
				*(cmat(m,c,i,i+k)+1)=*(md+1);
				ik++; if (ik<cd) md+=2;
			}
		}
	}
	else if (hd2->type==s_imatrix || hd2->type==s_interval)
	{	result=new_imatrix(r,c,""); if (error) return;
		m=matrixof(result);
		n=(LONG)2*(LONG)c*r;
		for (l=0; l<n; l++) *m++=0.0;
		getmatrix(hd2,&rd,&cd,&md);
		if (rd!=1 || cd<1) need_arg_in("diag","size n, m, diagonal k, vector v");
		m=matrixof(result);
		for (i=0; i<r; i++)
		{	if (i+k>=0 && i+k<c)
			{	*imat(m,c,i,i+k)=*md;
				*(imat(m,c,i,i+k)+1)=*(md+1);
				ik++; if (ik<cd) md+=2;
			}
		}
	}
	else need_arg_in("diag","size n, m, diagonal k, vector v");
	moveresult(st,result);
}

void msetdiag (header *hd)
{	header *result,*st=hd,*hd1,*hd2=0;
	double *m,*md,*mhd;
	int r,c,i,ik=0,k,rd,cd;
	hd=getvalue(st); if (error) return;
	if (hd->type!=s_matrix && hd->type!=s_cmatrix && hd->type!=s_imatrix)
		need_arg_in("setdiag","matrix, integer, vector or real");
	getmatrix(hd,&r,&c,&mhd);
	hd1=next_param(st); if (hd1) hd2=next_param(hd1);
	if (hd1) hd1=getvalue(hd1);
	if (hd2) hd2=getvalue(hd2);
	if (error) return;
	if	(hd1->type!=s_real)
		need_arg_in("setdiag","matrix, integer, vector or real");
	k=(int)*realof(hd1);
	if (hd->type==s_matrix &&
			(hd2->type==s_complex || hd2->type==s_cmatrix))
		{	make_complex(st); msetdiag(st); return;
		}
	else if (hd->type==s_cmatrix &&
			(hd2->type==s_real || hd2->type==s_matrix))
		{	make_complex(nextof(nextof(st))); msetdiag(st); return;
		}
	else if (hd->type==s_matrix &&
			(hd2->type==s_interval || hd2->type==s_imatrix))
		{	make_interval(st); msetdiag(st); return;
		}
	else if (hd->type==s_imatrix &&
			(hd2->type==s_real || hd2->type==s_matrix))
		{	make_interval(nextof(nextof(st))); msetdiag(st); return;
		}
	if (hd->type==s_matrix)
	{	result=new_matrix(r,c,""); if (error) return;
		m=matrixof(result);
		memmove((char *)m,(char *)mhd,(LONG)c*r*sizeof(double));
		getmatrix(hd2,&rd,&cd,&md);
		if (rd!=1 || cd<1)
			need_arg_in("setdiag","matrix, integer, vector or real");
		for (i=0; i<r; i++)
		{	if (i+k>=0 && i+k<c)
			{	*mat(m,c,i,i+k)=*md;
				ik++; if (ik<cd) md++;
			}
		}
	}
	else if (hd->type==s_cmatrix)
	{	result=new_cmatrix(r,c,""); if (error) return;
		m=matrixof(result);
        memmove((char *)m,(char *)mhd,(LONG)c*r*(LONG)2*sizeof(double));
		getmatrix(hd2,&rd,&cd,&md);
		if (rd!=1 || cd<1)
			need_arg_in("setdiag","matrix, integer, vector or real");
		m=matrixof(result);
		for (i=0; i<r; i++)
		{	if (i+k>=0 && i+k<c) 
			{	*cmat(m,c,i,i+k)=*md;
				*(cmat(m,c,i,i+k)+1)=*(md+1);
				ik++; if (ik<cd) md+=2;
			}
		}
	}
	else if (hd->type==s_imatrix)
	{	result=new_imatrix(r,c,""); if (error) return;
		m=matrixof(result);
        memmove((char *)m,(char *)mhd,(LONG)c*r*(LONG)2*sizeof(double));
		getmatrix(hd2,&rd,&cd,&md);
		if (rd!=1 || cd<1)
			need_arg_in("setdiag","matrix, integer, vector or real");
		m=matrixof(result);
		for (i=0; i<r; i++)
		{	if (i+k>=0 && i+k<c) 
			{	*cmat(m,c,i,i+k)=*md;
				*(cmat(m,c,i,i+k)+1)=*(md+1);
				ik++; if (ik<cd) md+=2;
			}
		}
	}
	else
		need_arg_in("setdiag","matrix, integer, vector or real");
	moveresult(st,result);
}

void mextrema (header *hd)
{	header *result,*st=hd;
	double x,*m,*mr,min,max;
	int r,c,i,j,imin,imax;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_matrix(r,4,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++) 
		{	min=max=*m; imin=imax=0; m++;
			for (j=1; j<c; j++) 
			{	x=*m++;
				if (x<min || isnan(min)) { min=x; imin=j; }
				if (x>max || isnan(max)) { max=x; imax=j; }
			}
			*mr++=min; *mr++=imin+1; *mr++=max; *mr++=imax+1;
		}
	}
	else need_arg_in("extrema","real matrix");
	moveresult(st,result);
}

void mcummax (header *hd)
{	header *result,*st=hd;
	double *m,*mr;
	int r,c,i,j;
	if (usematlab) transpose(hd);
	if (error) return;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	
		getmatrix(hd,&r,&c,&m);
		result=new_matrix(r,c,"");
		if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++) 
		{	
			*mr++=*m++;
			for (j=1; j<c; j++) 
			{	
				if (*m<*(mr-1)) *mr=*(mr-1);
				else *mr=*m;
				mr++; m++;
			}
		}
	}
	else need_arg_in("cummax","real matrix");
	moveresult(st,result);
	if (usematlab) transpose(st);
}

void mcummin (header *hd)
{	header *result,*st=hd;
	double *m,*mr;
	int r,c,i,j;
	if (usematlab) transpose(hd);
	if (error) return;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	
		getmatrix(hd,&r,&c,&m);
		result=new_matrix(r,c,"");
		if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++) 
		{	
			*mr++=*m++;
			for (j=1; j<c; j++) 
			{	
				if (*m>*(mr-1)) *mr=*(mr-1);
				else *mr=*m;
				mr++; m++;
			}
		}
	}
	else need_arg_in("cummax","real matrix");
	moveresult(st,result);
	if (usematlab) transpose(st);
}

void mcumsum (header *hd)
{	header *result,*st=hd;
	double *m,*mr,sum=0,sumr=0,sumi=0;
	int r,c,i,j;
	if (usematlab) transpose(hd);
	if (error) return;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		if (c<1) result=new_matrix(r,1,"");
		else result=new_matrix(r,c,"");
		if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++) 
		{	if (c>=1) sum=*m++;
			*mr++=sum;
			for (j=1; j<c; j++) 
			{	sum+=*m++;
				*mr++=sum;
			}
		}
	}
	else if (hd->type==s_complex || hd->type==s_cmatrix)
	{	getmatrix(hd,&r,&c,&m);
		if (c<1) result=new_cmatrix(r,1,"");
		else result=new_cmatrix(r,c,"");
		if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
		{	if (c>=1) { sumr=*m++; sumi=*m++; }
			*mr++=sumr; *mr++=sumi;
			for (j=1; j<c; j++) 
			{	sumr+=*m++; *mr++=sumr;
				sumi+=*m++; *mr++=sumi;
			}
		}
	}
	else need_arg_in("cumsum","matrix");
	moveresult(st,result);
	if (usematlab) transpose(st);
}

void mcumprod (header *hd)
{	header *result,*st=hd;
	double *m,*mr,sum=1,sumi=1,sumr=0,x,y;
	int r,c,i,j;
	if (usematlab) transpose(hd);
	if (error) return;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		if (c<1) result=new_matrix(r,1,"");
		else result=new_matrix(r,c,"");
		if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++) 
		{	if (c>=1) sum=*m++; 
			*mr++=sum;
			for (j=1; j<c; j++) 
			{	sum*=*m++;
				*mr++=sum;
			}
		}
	}
	else if (hd->type==s_complex || hd->type==s_cmatrix)
	{	getmatrix(hd,&r,&c,&m);
		if (c<1) result=new_cmatrix(r,1,"");
		else result=new_cmatrix(r,c,"");
		if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
		{	if (c>=1) { sumr=*m++; sumi=*m++; }
			*mr++=sumr; *mr++=sumi;
			for (j=1; j<c; j++)
			{	sum=sumr*(*m)-sumi*(*(m+1));
				sumi=sumr*(*(m+1))+sumi*(*m);
				sumr=sum;
				m+=2;
				*mr++=sumr;
				*mr++=sumi;
			}
		}
	}
	else if (hd->type==s_interval || hd->type==s_imatrix)
	{	getmatrix(hd,&r,&c,&m);
		if (c<1) result=new_imatrix(r,1,"");
		else result=new_imatrix(r,c,"");
		if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
		{	if (c>=1) { sumr=*m++; sumi=*m++; }
			*mr++=sumr; *mr++=sumi;
			for (j=1; j<c; j++)
			{   interval_mult(m,m+1,&sumr,&sumi,&x,&y);
				sumr=x; sumi=y;
				m+=2;
				*mr++=sumr;
				*mr++=sumi;
			}
		}
	}
	else if (hd->type==s_interval || hd->type==s_imatrix)
	{	getmatrix(hd,&r,&c,&m);
		if (c<1) result=new_imatrix(r,1,"");
		else result=new_imatrix(r,c,"");
		if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
		{	if (c>=1) { sumr=*m++; sumi=*m++; }
			*mr++=sumr; *mr++=sumi;
			for (j=1; j<c; j++)
			{   interval_add(m,m+1,&sumr,&sumi,&x,&y);
				sumr=x; sumi=y;
				m+=2;
				*mr++=sumr;
				*mr++=sumi;
			}
		}
	}
	else need_arg_in("cumprod","matrix");
	moveresult(st,result);
	if (usematlab) transpose(st);
}

void mwait (header *hd)
{	header *st=hd,*result;
	double now;
	int h;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real) need_arg_in("wait","optional real, optional string");
	now=myclock();
	int c=sys_wait(*realof(hd),&h);
	if (h==escape) { output("Interrupt\n"); error=ERROR_ESCAPE; return; }
	now=myclock()-now;
	if (h==0 || now>*realof(hd)) now=*realof(hd);
	if (h!=0 && now>=*realof(hd)) now=*realof(hd)*0.999;
	result=new_real(now,"");
	if (error) return;
	new_real(c,"");
	moveresult1(st,result);
}

void mwait0 (header *hd)
{	double now;
	int h;
	now=myclock();
	int c=sys_wait(600,&h);
	if (h==escape) { output("Interrupt\n"); error=ERROR_ESCAPE; return; }
	now=myclock()-now;
	if (h==0 || now>600) now=600;
	if (h!=0 && now>=600) now=600*0.999;
	new_real(now,"");
	new_real(c,"");
}

void mwaitstatus (header *hd)
{	header *st=hd,*hd1,*result;
	double now;
	int h;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real) need_arg_in("wait","optional real, optional string");
	hd1=nextof(hd); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	if (hd1->type!=s_string) need_arg_in("wait","optional real, optional string");
	now=myclock();
	sys_wait(*realof(hd),&h,stringof(hd1));
	if (h==escape) { output("Interrupt\n"); error=ERROR_ESCAPE; return; }
	now=myclock()-now;
	if (h==0 || now>*realof(hd)) now=*realof(hd);
	if (h!=0 && now>=*realof(hd)) now=*realof(hd)*0.999;
	result=new_real(now,"");
	if (error) return;
	moveresult(st,result);
}

void mkey (header *hd)
{	int scan,key;
	key=wait_key(&scan);
	if (scan==escape) { output("Interrupt\n"); error=ERROR_ESCAPE; return; }
	if (key) new_real(key,"");
	else new_real(scan,"");
}

void mkeystatus (header *hd)
{	int scan,key;
	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) need_arg_in("key","string");
	key=wait_key(&scan,stringof(hd));
	if (scan==escape) { output("Interrupt\n"); error=ERROR_ESCAPE; return; }
	if (key) result=new_real(key,"");
	else result=new_real(scan,"");
	moveresult(st,result);
}

void msetstatus (header *hd)
// set the status line text
{	
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) need_arg_in("setstatus","string");
	setstatus(stringof(hd));
}

void mcode (header *hd)
{	new_real(test_code(),"");
}

void mfastrandom (header *hd)
{	header *st=hd,*result;
	double *m;
	int r,c;
	LONG k,n;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix || dimsof(hd)->r!=1 || dimsof(hd)->c!=2
		|| *(m=matrixof(hd))<0 || *m>=INT_MAX
		|| *(m+1)<0 || *(m+1)>INT_MAX)
		need_arg_in("random","1x2 vector");
	r=(int)*m;
	c=(int)*(m+1);
	result=new_matrix(r,c,""); if (error) return;
	m=matrixof(result);
	n=(LONG)c*r;
	for (k=0; k<n; k++) *m++=(double)rand()/(double)RAND_MAX;
	moveresult(st,result);
}

void mnormal (header *hd)
{	header *st=hd,*result;
	double *m,r1,r2;
	int r,c;
	LONG k,n;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix || dimsof(hd)->r!=1 || dimsof(hd)->c!=2
		|| *(m=matrixof(hd))<0 || *m>=INT_MAX 
		|| *(m+1)<0 || *(m+1)>INT_MAX)
		need_arg_in("normal","1x2 vector");
	r=(int)*m;
	c=(int)*(m+1);
	result=new_matrix(r,c,""); if (error) return;
	m=matrixof(result);
	n=(LONG)c*r;
	for (k=0; k<n; k++) 
	{	r1=(double)rand()/(double)RAND_MAX;
		if (r1==0.0) *m++=0.0;
		else
		{	r2=(double)rand()/(double)RAND_MAX;
			*m++=sqrt(-2*log(r1))*cos(2*M_PI*r2);
		}
	}
	moveresult(st,result);
}

double rfak (double x)
{	int i,n;
	double res=1;
	if (x<2 || x>INT_MAX) return 1.0;
	n=(int)x;
	for (i=2; i<=n; i++) res=res*i;
	return res;
}

void mfak (header *hd)
{	spread1(rfak,0,hd);
	test_error("fak");
}

double rlogfak (double x)
{	int i,n;
	double res=0;
	if (x<2 || x>INT_MAX) return 0.0;
	n=(int)x;
	for (i=2; i<=n; i++) res=res+log((double)i);
	return res;
}

void mlogfak (header *hd)
{	spread1(rlogfak,0,hd);
	test_error("logfak");
}

void rbin (double *x, double *y, double *z)
{   long i;
	double n,m,k;
	double res;
	n=*x; m=*y;
	if (m<=0) *z=1.0;
	else
	{	res=k=n-m+1;
		for (i=2; i<=m; i++) { k++; res=res*k/i; }
		*z=res;
	}
}

void mbin (header *hd)
{	spreadf2(rbin,0,0,hd,"bin");
	test_error("bin");
}

void rlogbin (double *x, double *y, double *z)
{   long i,n,m,k;
	double res;
	n=(long)*x; m=(long)*y;
	if (m<=0) *z=0.0;
	else
	{   k=n-m+1;
		res=log((double)(n-m+1));
		for (i=2; i<=m; i++) { k++; res+=log((double)k)-log((double)i); }
		*z=res;
	}
}

void mlogbin (header *hd)
{	
	spreadf2(rlogbin,0,0,hd,"logbin");
	test_error("logbin");
}

extern double gauss (double x);
#define PI 3.14159265358979323846

double qnormal (double x)
{
	return 1/sqrt(2*PI)*exp(-x*x/2);
}

double invgauss (double a)
{	
	if (a>=1) { return NAN; }
	if (a<=0) { return NAN; }
	try
	{
		return alglib::invnormaldistribution(a);
	}
	catch (alglib::ap_error)
	{
		if (!pnan) error=1;
		return NAN;
	}
}

void minvgauss (header *hd)
{	
	spread1(invgauss,0,hd);
	test_error("invnormaldis");
}

void rmax (double *x, double *y, double *z)
{	if (*x>*y) *z=*x;
	else *z=*y;
}

void mmax (header *hd)
{   spreadf2(rmax,0,imax,hd,"max");
	test_error("max");
}


void rtan2 (double *x, double *xi, double *z)
{
#ifdef FLOAT_TEST
	if (*x==0.0 && *xi==0.0) *z=0.0;
#endif
	*z = atan2(*xi,*x);
}

void mtan2 (header *hd)
{	spreadf2(rtan2,0,0,hd,"tan2");
	test_error("tan2");
}

void rmin (double *x, double *y, double *z)
{	if (*x>*y) *z=*y;
	else *z=*x;
}

void mmin (header *hd)
{	spreadf2(rmin,0,imin,hd,"min");
	test_error("min");
}

typedef struct { double val; int ind; } sorttyp;

int sorttyp_compare (const sorttyp *x, const sorttyp *y)
{	
	if (isnan(x->val))
	{	if (isnan(y->val)) return 0;
		else return 1;
	}
	if (isnan(y->val)) return -1;
	if (x->val>y->val) return 1;
	else if (x->val==y->val) return 0;
	else return -1;
}

typedef struct { char *val; int ind; } ssorttyp;

int ssorttyp_compare (const ssorttyp *x, const ssorttyp *y)
{	
	return strcmp(x->val,y->val);
}

void msort (header *hd)
{	
	header *st=hd,*result,*result1;
	double *m,*m1;
	sorttyp *t;
	int r,c,i;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix) 
	{
		getmatrix(hd,&r,&c,&m);
		if (c==1 || r==1) result=new_matrix(r,c,"");
		else need_arg_in("sort","1xn or nx1 vector");
		if (error) return;
		result1=new_matrix(r,c,"");
		if (error) return;
		if (c==1) c=r;
		if (c==0) 
			need_arg_in("sort","nonempty real vector");
		if (!freeram(c*sizeof(sorttyp)))
		{	output("Out of memory!\n"); error=ERROR_MEMORY; return; 
		}
		t=(sorttyp *)newram;
		for (i=0; i<c; i++)
		{	t->val=*m++; t->ind=i; t++;
		}
		qsort(newram,c,sizeof(sorttyp),
			(int (*) (const void *, const void *))sorttyp_compare);
		m=matrixof(result); m1=matrixof(result1);
		t=(sorttyp *)newram;
		for (i=0; i<c; i++)
		{	*m++=t->val; *m1++=t->ind+1; t++;
		}
	}
	else if (hd->type==s_smatrix)
	{
		char *p=stringof(hd);
		ssorttyp *v=(ssorttyp *)newram;
		int ns=0;
		while (true)
		{
			checkram(v+1);
			v[ns].val=p;
			v[ns].ind=ns;
			ns++;
			p=p+strlen(p)+1;
			if (*p==1) break;
		}
		newram=(char *)(v+ns);
		qsort(v,ns,sizeof(ssorttyp),
			(int (*) (const void *, const void *))ssorttyp_compare);
		result=new_smatrix("");
		p=stringof(result);
		for (int i=0; i<ns; i++)
		{
			int ls=strlen(v[i].val);
			checkram(p+ls+2);
			strcpy(p,v[i].val); p+=ls+1;
		}
		*p++=1;
		result->size=p-(char *)result;
		newram=(char *)p;
		result1=new_matrix(1,ns,"");
		m1=matrixof(result1);
		for (int i=0; i<ns; i++) *m1++=v[i].ind+1;
	}
	else if (hd->type==s_string)
	{
		return;
	}
	else
		need_arg_in("sort","real or string vector");
	moveresult1(st,result);
}

void munique (header *hd)
{	
	header *st=hd,*result,*result1;
	double *m,*m1;
	sorttyp *t;
	int r,c,i;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real && hd->type!=s_matrix) 
		need_arg_in("unique","real matrix");
	getmatrix(hd,&r,&c,&m);
	if (!(r==1 || (r==0 && c==0))) need_arg_in("unique","real row vector");
	if (c==0) 
	{	result=new_matrix(1,0,"");
		result1=new_matrix(1,0,"");
		moveresult1(st,result);
		return;
	}
	if (c==1) 
	{	result=new_matrix(1,1,"");
		*(matrixof(result))=*m;
		result1=new_matrix(1,1,"");
		*(matrixof(result1))=1.0;
		moveresult1(st,result);
		return;
	}
	if (!freeram(c*sizeof(sorttyp)))
	{	
		output("Out of memory!\n"); error=ERROR_MEMORY; return; 
	}
	sorttyp *tnote=(sorttyp *)newram;
	newram=(char *)(tnote+c);
	t=tnote;
	m=matrixof(hd);
	for (i=0; i<c; i++)
	{	t->val=*m++; t->ind=i; t++;
	}
	qsort(tnote,c,sizeof(sorttyp),
		(int (*) (const void *, const void *))sorttyp_compare);
	t=tnote;
	int count=1;
	for (i=0; i<c-1; i++)
	{	if (t->val!=(t+1)->val) count++;
		t++;
	}
	result=new_matrix(1,count,"");
	if (error) return;
	result1=new_matrix(1,count,"");
	if (error) return;
	m=matrixof(result); m1=matrixof(result1);
	t=tnote;
	for (i=0; i<c-1; i++)
	{	if (t->val!=(t+1)->val)
		{	*m++=t->val; *m1++=t->ind+1; 
		}
		t++;
	}
	*m++=t->val; *m1++=t->ind+1;
	moveresult1(st,result);
}


typedef struct { double *val; int ind; int m; } lexsorttyp;

int lexsorttyp_compare (const lexsorttyp *x, const lexsorttyp *y)
{	double *mx=x->val,*my=y->val;
	for (int i=0; i<x->m; i++)
	{	if (*mx>*my) return 1;
		if (*mx<*my) return -1;
		mx++; my++;
	}
	return 0;
}

void mlexsort (header *hd)
{	header *st=hd,*result,*result1;
	double *m,*m1;
	lexsorttyp *t;
	int r,c,i;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real && hd->type!=s_matrix) 
		need_arg_in("lexsort","real matrix");
	getmatrix(hd,&r,&c,&m);
	if (r==0 || c==0) need_arg_in("lexsort","nonempty real matrix");
	result=new_matrix(r,c,"");
	if (error) return;
	result1=new_matrix(1,r,"");
	if (error) return;
	if (!freeram(r*sizeof(sorttyp)))
	{	output("Out of memory!\n"); error=ERROR_MEMORY; return; 
	}
	t=(lexsorttyp *)newram;
	for (i=0; i<r; i++)
	{	t->val=m; m+=c; t->ind=i; t->m=c; t++;
	}
	qsort(newram,r,sizeof(lexsorttyp),
		(int (*) (const void *, const void *))lexsorttyp_compare);
	m=matrixof(result); m1=matrixof(result1);
	t=(lexsorttyp *)newram;
	for (i=0; i<r; i++)
	{	double *h=t->val;
		for (int j=0; j<c; j++) *m++=*h++; 
		*m1++=t->ind+1; 
		t++;
	}
	moveresult(st,result);
	moveresult(nextof(st),result1);
}

void mnonzeros (header *hd)
{	header *st=hd,*result;
	double *m,*mr;
	int r,c,i,k;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real && hd->type!=s_matrix) need_arg_in("nonzeros","row vector");
	getmatrix(hd,&r,&c,&m);
	if (r!=1 && c!=1) need_arg_in("nonzeros","row vector");
	if (c==1) c=r;
	result=new_matrix(1,c,""); if (error) return;
	k=0; mr=matrixof(result);
	for (i=0; i<c; i++)
	{	if (*m++!=0.0)
		{	*mr++=i+1; k++;
		}
	}
	dimsof(result)->c=k;
	result->size=matrixsize(1,k);
	moveresult(st,result);
}

void mmnonzeros (header *hd)
{
	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_matrix)
	{
		double *m,*mr;
		int c,r;
		getmatrix(hd,&r,&c,&m);
		result=new_matrix(0,2,""); if (error) return;
		mr=matrixof(result);
		int n=0;
		for (int i=0; i<r; i++)
			for (int j=0; j<c; j++)
				if (*m++!=0.0)
				{
					checkram(mr+2);
					*mr++=i+1;
					*mr++=j+1;
					n++;
				}
		dimsof(result)->r=n;
		result->size=(char *)mr-(char *)result;
		newram=(char *)mr;
	}
	else if (hd->type==s_cmatrix)
	{
		double *m,*mr;
		int c,r;
		getmatrix(hd,&r,&c,&m);
		result=new_matrix(0,2,""); if (error) return;
		mr=matrixof(result);
		int n=0;
		for (int i=0; i<r; i++)
			for (int j=0; j<c; j++)
			{	
				if (*m!=0.0 && *(m+1)!=0.0)
				{
					checkram(mr+2);
					*mr++=i+1;
					*mr++=j+1;
					n++;
				}
				m+=2;
			}
		dimsof(result)->r=n;
		result->size=(char *)mr-(char *)result;
		newram=(char *)mr;
	}
	else
		need_arg_in("mnonzeros","real or complex matrix");
	moveresult(st,result);
}

void mmget (header *hd)
{
	header *st=hd,*hdi,*result;
	hdi=nextof(hd);
	hd=getvalue(hd);
	hdi=getvalue(hdi);
	if (error) return;
	if (hdi->type==s_matrix && (dimsof(hdi)->r==0 || dimsof(hdi)->c==0))
	{
		result=new_matrix(1,0,""); if (error) return;
		moveresult(st,result);
		return;
	}
	else if (hdi->type!=s_matrix || (dimsof(hdi)->c!=2))
		need_arg_in("mget","real or complex matrix, nx2 index matrix");
	if (hd->type==s_matrix)
	{
		double *m,*mi,*mr;
		int c,r;
		getmatrix(hd,&r,&c,&m);
		int n=dimsof(hdi)->r;
		mi=matrixof(hdi);
		result=new_matrix(1,n,"");
		mr=matrixof(result);
		for (int i=0; i<n; i++)
		{
			int ri=(int)*mi++;
			int ci=(int)*mi++;
			if (ri<1 || ri>r || ci<1 || ci>c)
			{
				print("Index %d,%d out of range in mget.\n",ri,ci);
				error=1; return;
			}
			*mr++=*(m+(c*(ri-1)+ci-1));
		}
	}
	else if (hd->type==s_cmatrix)
	{
		double *m,*mi,*mr;
		int c,r;
		getmatrix(hd,&r,&c,&m);
		int n=dimsof(hdi)->r;
		mi=matrixof(hdi);
		result=new_cmatrix(1,n,"");
		mr=matrixof(result);
		for (int i=0; i<n; i++)
		{
			int ri=(int)*mi++;
			int ci=(int)*mi++;
			if (ri<1 || ri>r || ci<1 || ci>c)
			{
				print("Index %d,%d out of range in mget.\n",ri,ci);
				error=1; return;
			}
			*mr++=*(m+(c*(ri-1)+ci-1));
			*mr++=*(m+(c*(ri-1)+ci));
		}
	}
	else
		need_arg_in("mget","real or complex matrix, nx2 index matrix");
	moveresult(st,result);
}

void mmset (header *hd)
{
	header *st=hd,*hdi,*hd1,*result;
	hdi=nextof(hd); hd1=nextof(hdi);
	hd=getvalue(hd); hdi=getvalue(hdi); hd1=getvalue(hd1);
	if (error) return;
	if (hdi->type==s_matrix && (dimsof(hdi)->r==0 || dimsof(hdi)->c==0))
	{
		moveresult(st,hd);
		return;
	}
	else if (hdi->type!=s_matrix || (dimsof(hdi)->c!=2))
		need_arg_in("mget","real or complex matrix, nx2 index matrix");
	if (hd->type==s_matrix && (hd1->type==s_matrix || hd1->type==s_real))
	{
		double *m,*m1,*mi,*mr;
		int c,r;
		getmatrix(hd,&r,&c,&m);
		int n=dimsof(hdi)->r;
		mi=matrixof(hdi);
		result=new_matrix(r,c,"");
		mr=matrixof(result);
		int c1,r1;
		getmatrix(hd1,&r1,&c1,&m1);
		if ((r1>1 && r1!=r) || (c1>1 && c1!=c))
		{
			print("Cannot assign from %d,%d to %d,%d in mset.\n",r1,c1,r,c);
			error=1; return;
		}
		memmove(mr,m,r*c*sizeof(double));
		for (int i=0; i<n; i++)
		{
			int ri=(int)*mi++;
			int ci=(int)*mi++;
			if (ri<1 || ri>r || ci<1 || ci>c)
			{
				print("Index %d,%d out of range in mget.\n",ri,ci);
				error=1; return;
			}
			int k=c*(ri-1)+ci-1;
			int k1=c1*(r1>1?(ri-1):0)+(c1>1?ci-1:0);
			mr[k]=m1[k1];
		}
	}
	else if (hd->type==s_cmatrix && (hd1->type==s_cmatrix || hd1->type==s_complex))
	{
		double *m,*m1,*mi,*mr;
		int c,r;
		getmatrix(hd,&r,&c,&m);
		int n=dimsof(hdi)->r;
		mi=matrixof(hdi);
		result=new_cmatrix(r,c,"");
		mr=matrixof(result);
		int c1,r1;
		getmatrix(hd1,&r1,&c1,&m1);
		if ((r1>1 && r1!=r) || (c1>1 && c1!=c))
		{
			print("Cannot assign from %d,%d to %d,%d in mset.\n",r1,c1,r,c);
			error=1; return;
		}
		memmove(mr,m,r*c*sizeof(double)*2);
		for (int i=0; i<n; i++)
		{
			int ri=(int)*mi++;
			int ci=(int)*mi++;
			if (ri<1 || ri>r || ci<1 || ci>c)
			{
				print("Index %d,%d out of range in mget.\n",ri,ci);
				error=1; return;
			}
			int k=c*(ri-1)+ci-1;
			int k1=c1*(r1>1?(ri-1):0)+(c1>1?ci-1:0);
			mr[2*k]=m1[2*k1];
			mr[2*k+1]=m1[2*k1+1];
		}
	}
	else if (hd->type==s_cmatrix && (hd1->type==s_matrix || hd1->type==s_real))
	{
		double *m,*m1,*mi,*mr;
		int c,r;
		getmatrix(hd,&r,&c,&m);
		int n=dimsof(hdi)->r;
		mi=matrixof(hdi);
		result=new_cmatrix(r,c,"");
		mr=matrixof(result);
		int c1,r1;
		getmatrix(hd1,&r1,&c1,&m1);
		if ((r1>1 && r1!=r) || (c1>1 && c1!=c))
		{
			print("Cannot assign from %d,%d to %d,%d in mset.\n",r1,c1,r,c);
			error=1; return;
		}
		memmove(mr,m,r*c*sizeof(double)*2);
		for (int i=0; i<n; i++)
		{
			int ri=(int)*mi++;
			int ci=(int)*mi++;
			if (ri<1 || ri>r || ci<1 || ci>c)
			{
				print("Index %d,%d out of range in mget.\n",ri,ci);
				error=1; return;
			}
			int k=c*(ri-1)+ci-1;
			int k1=c1*(r1>1?(ri-1):0)+(c1>1?ci-1:0);
			mr[2*k]=m1[2*k1];
			mr[2*k+1]=0.0;
		}
	}
	else if (hd->type==s_matrix && (hd1->type==s_cmatrix || hd1->type==s_complex))
	{
		double *m,*m1,*mi,*mr;
		int c,r;
		getmatrix(hd,&r,&c,&m);
		int n=dimsof(hdi)->r;
		mi=matrixof(hdi);
		result=new_cmatrix(r,c,"");
		mr=matrixof(result);
		int c1,r1;
		getmatrix(hd1,&r1,&c1,&m1);
		if ((r1>1 && r1!=r) || (c1>1 && c1!=c))
		{
			print("Cannot assign from %d,%d to %d,%d in mset.\n",r1,c1,r,c);
			error=1; return;
		}
		double *mrh=mr,*mh=m;
		for (int i=0; i<r; i++)
			for (int j=0; j<c; j++)
			{
				*mrh++=*mh++;
				*mrh++=0.0;
			}
		for (int i=0; i<n; i++)
		{
			int ri=(int)*mi++;
			int ci=(int)*mi++;
			if (ri<1 || ri>r || ci<1 || ci>c)
			{
				print("Index %d,%d out of range in mget.\n",ri,ci);
				error=1; return;
			}
			int k=c*(ri-1)+ci-1;
			int k1=c1*(r1>1?(ri-1):0)+(c1>1?ci-1:0);
			mr[2*k]=m1[2*k1];
			mr[2*k+1]=0.0;
		}
	}
	else
		need_arg_in("mset","real or complex matrix, nx2 index matrix and another matrix");
	moveresult(st,result);
}

void mstatistics (header *hd)
{	header *st=hd,*hd1,*result;
	int i,n,r,c,k;
	double *m,*mr;
	hd=getvalue(hd);
	hd1=next_param(st);
	if (hd1) hd1=getvalue(hd1); if (error) return;
	if (hd1->type!=s_real || hd->type!=s_matrix) need_arg_in("count","row vector and integer");
	if (*realof(hd1)>INT_MAX || *realof(hd1)<2) need_arg_in("count","row vector and integer");
	n=(int)*realof(hd1);
	getmatrix(hd,&r,&c,&m);
	if (r!=1 && c!=1) need_arg_in("count","row vector and integer");
	if (c==1) c=r;
	result=new_matrix(1,n,""); if (error) return;
	mr=matrixof(result); for (i=0; i<n; i++) *mr++=0.0;
	mr=matrixof(result);
	for (i=0; i<c; i++)
	{	if (*m>=0 && *m<n)
		{	k=(int)floor(*m);
			mr[k]+=1.0;
		}
		m++;
	}
	moveresult(st,result);
}

extern int nomultiline,inevaluation;

void minput (header *hd)
{	
	header *st=hd,*result;
	char input[MAXLINE],*oldnext;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) need_arg_in("input","prompt string");
	retry: output(stringof(hd)); output("? ");
	nojump=1;
	edit(input);
	if (!strcmp(input,"escape"))
	{
		error=ERROR_ESCAPE; return;
	}
	stringon=1; inevaluation=1;
	oldnext=next; next=input; result=scan_value(); next=oldnext;
	stringon=0; inevaluation=0;
	if (error) 
	{	output("Error in input!\n"); error=ERROR_INPUT; 
		error=0; goto retry;
	}
	moveresult(st,result);
}

void minput1 (header *hd)
	// line minput, but returns an error, if the line is empty.
{	
	header *st=hd,*result;
	char input[MAXLINE],*oldnext;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) need_arg_in("input","prompt string");
	retry: output(stringof(hd)); output("? ");
	nojump=1;
	edit(input);
	if (!strcmp(input,"escape") || *input==0)
	{
		error=ERROR_ESCAPE; return;
	}
	stringon=1; inevaluation=1;
	oldnext=next; next=input; result=scan_value(); next=oldnext;
	stringon=0; inevaluation=0;
	if (error) 
	{	output("Error in input!\n"); error=ERROR_INPUT; 
		error=0; goto retry;
	}
	moveresult(st,result);
}

void mlineinput (header *hd)
{	header *st=hd,*result;
	char input[MAXLINE];
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) need_arg_in("lineinput","prompt string");
	nojump=1;
	output(stringof(hd)); output("? ");
	edit(input);
	result=new_string(input,strlen(input),"");
	moveresult(st,result);
}

extern char *currentfunction;

void mevaluate (header *hd)
{	header *st=hd,*result;
	char *oldnext;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) need_arg_in("evaluate","string");
	stringon=1;
	oldnext=next; next=stringof(hd); 
	char *oc=currentfunction;
	currentfunction="evaluate";
	result=scan(); 
	scan_space();
	if (*next!=0)
	{	output1("Error in Evaluate, superfluous characters found.\n");
		error=ERROR_SUPERFLUOUS;
	}
	currentfunction=oc;
	next=oldnext;
	stringon=0;
    if (error) { result=new_string("Syntax error in \"evaluate\"!",16,""); }
	moveresult(st,result);
}

void mmax1 (header *hd)
{	header *result,*st=hd;
	double x,*m,*mr,max,max1;
	int r,c,i,j;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_matrix(r,1,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
		{	max=*m++;
			for (j=1; j<c; j++)
			{	x=*m++;
				if (x>max || isnan(max)) max=x;
			}
			*mr++=max;
		}
	}
	else if (hd->type==s_interval || hd->type==s_imatrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_imatrix(r,1,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
		{	max=*m++;
			max1=*m++;
			for (j=1; j<c; j++)
			{	imax(&max,&max1,m,m+1,&max,&max1);
				m+=2;
			}
			*mr++=max;
			*mr++=max1;
		}
	}
	else need_arg_in("max","real or interval matrix type");
	result=getvalue(result);
	moveresult(st,result);
}

void mmin1 (header *hd)
{	header *result,*st=hd;
	double x,*m,*mr,max=0,max1=0;
	int r,c,i,j;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_matrix(r,1,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++) 
		{	max=*m; m++;
			for (j=1; j<c; j++) 
			{	x=*m++;
				if (x<max || isnan(max)) max=x;
			}
			*mr++=max;
		}
	}
	else if (hd->type==s_interval || hd->type==s_imatrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_imatrix(r,1,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
		{	max=*m++;
			max1=*m++;
			for (j=1; j<c; j++)
			{	imin(&max,&max1,m,m+1,&max,&max1);
				m+=2;
			}
			*mr++=max;
			*mr++=max1;
		}
	}
	else need_arg_in("min","real or interval matrix type");
	result=getvalue(result);
	moveresult(st,result);
}

void mfirstnonzero (header *hd)
{	header *result,*st=hd;
	double *m,*mr;
	int r,c,i,j;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	
		getmatrix(hd,&r,&c,&m);
		result=new_matrix(r,1,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++) 
		{	
			*mr=0.0;
			for (j=0; j<c; j++) 
			{	
				if (m[j]!=0.0)
				{
					*mr=j+1; break;
				}
			}
			mr++;
			m+=c;
		}
	}
	else need_arg_in("firstnonzero","real matrix or scalar");
	result=getvalue(result);
	moveresult(st,result);
}

void mlastnonzero (header *hd)
{	header *result,*st=hd;
	double *m,*mr;
	int r,c,i,j;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	
		getmatrix(hd,&r,&c,&m);
		result=new_matrix(r,1,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++) 
		{	
			*mr=0.0;
			for (j=c-1; j>=0; j--) 
			{	
				if (m[j]!=0.0)
				{
					*mr=j+1; break;
				}
			}
			mr++;
			m+=c;
		}
	}
	else need_arg_in("lastnonzero","real matrix or scalar");
	result=getvalue(result);
	moveresult(st,result);
}

void make_xors (void)
{	int i;
	for (i=0; i<20; i++) xors[i]=xor(argname[i]);
}

void mdo (header *hd)
{	header *st=hd,*hd1,*result;
	int count=0;
	ULONG size;
	if (!hd) need_arg_in("do","function and arguments");
	hd=getvalue(hd);
	result=hd1=next_param(st);
	if (hd->type!=s_string) need_arg_in("do","function and arguments");
	if (error) return;
	hd=searchudf(stringof(hd));
	if (!hd || hd->type!=s_udf) need_arg_in("do","function and arguments");
	while (hd1)
	{	strcpy(hd1->name,argname[count]);
		hd1->xor=xors[count];
		hd1=next_param(hd1); count++;
	}
	if (result)
	{	size=(char *)result-(char *)st;
		if (size>0 && newram!=(char *)result)
			memmove((char *)st,(char *)result,newram-(char *)result);
		newram-=size;
	}
	interpret_udf(hd,st,count,0);
}

void mlu (header *hd)
{	header *st=hd,*result,*res1,*res2,*res3;
	double *m,*mr,*m1,*m2,det,deti;
	int r,c,*rows,*cols,rank,i;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_matrix || hd->type==s_real)
	{	getmatrix(hd,&r,&c,&m);
		if (r<1) need_arg_in("lu","real or complex matrix");
		result=new_matrix(r,c,""); if (error) return;
		mr=matrixof(result);
		memmove((char *)mr,(char *)m,(LONG)r*c*sizeof(double));
		make_lu(mr,r,c,&rows,&cols,&rank,&det); if (error) return;
		res1=new_matrix(1,rank,""); if (error) return;
		res2=new_matrix(1,c,""); if (error) return;
		res3=new_real(det,""); if (error) return;
		m1=matrixof(res1);
		for (i=0; i<rank; i++)
		{	*m1++=*rows+1;
			rows++;
		}
		m2=matrixof(res2);
		for (i=0; i<c; i++)
		{	*m2++=*cols++;
		}
		moveresult(st,getvalue(result)); st=nextof(st);
		moveresult(st,getvalue(res1)); st=nextof(st);
		moveresult(st,getvalue(res2)); st=nextof(st);
		moveresult(st,getvalue(res3));
	}
	else if (hd->type==s_cmatrix || hd->type==s_complex)
	{	getmatrix(hd,&r,&c,&m);
		if (r<1) need_arg_in("lu","real or complex matrix");
		result=new_cmatrix(r,c,""); if (error) return;
		mr=matrixof(result);
        memmove((char *)mr,(char *)m,(LONG)r*c*(LONG)2*sizeof(double));
		cmake_lu(mr,r,c,&rows,&cols,&rank,&det,&deti); 
			if (error) return;
		res1=new_matrix(1,rank,""); if (error) return;
		res2=new_matrix(1,c,""); if (error) return;
		res3=new_complex(det,deti,""); if (error) return;
		m1=matrixof(res1);
		for (i=0; i<rank; i++)
		{	*m1++=*rows+1;
			rows++;
		}
		m2=matrixof(res2);
		for (i=0; i<c; i++)
		{	*m2++=*cols++;
		}
		moveresult(st,getvalue(result)); st=nextof(st);
		moveresult(st,getvalue(res1)); st=nextof(st);
		moveresult(st,getvalue(res2)); st=nextof(st);
		moveresult(st,getvalue(res3));
	}
	else need_arg_in("lu","real or complex matrix");
}

void miscomplex (header *hd)
{	header *st=hd,*result;
	hd=getvalue(hd);
	if (hd->type==s_complex || hd->type==s_cmatrix)
		result=new_real(1.0,"");
	else result=new_real(0.0,"");
	if (error) return;
	moveresult(st,result);
}

void misreal (header *hd)
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
		result=new_real(1.0,"");
	else result=new_real(0.0,"");
	if (error) return;
	moveresult(st,result);
}

void misfunction (header *hd)
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_string
		&& (searchudf(stringof(hd))!=0 || searchudfmap(stringof(hd))!=0 
				|| find_builtin(stringof(hd))!=0))
			result=new_real(1.0,"");
	else result=new_real(0.0,"");
	if (error) return;
	moveresult(st,result);
}

void miscall (header *hd)
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_string)
		result=new_real(1.0,"");
	else if (hd->type==s_list && hd+1<nextof(hd) && (hd+1)->type==s_string)
		result=new_real(1.0,"");
	else
		result=new_real(0.0,"");
	if (error) return;
	moveresult(st,result);
}

void misvar (header *hd)
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_string
		&& searchvar(stringof(hd))!=0)
			result=new_real(1.0,"");
	else result=new_real(0.0,"");
	if (error) return;
	moveresult(st,result);
}

void misempty (header *hd)
{	
	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (isnone(hd)) result=new_real(1.0,"");
	else if (hd->type==s_string && *stringof(hd)==0) result=new_real(1.0,"");
	else if ((hd->type==s_matrix || hd->type==s_cmatrix || hd->type==s_imatrix)
			&& (dimsof(hd)->c==0 || dimsof(hd)->r==0)) result=new_real(1.0,"");
	else if (hd->type==s_smatrix && *stringof(hd)==1) result=new_real(1.0,"");
	else result=new_real(0.0,"");
	if (error) return;
	moveresult(st,result);
}

void misinterval (header *hd)
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_interval || hd->type==s_imatrix)
		result=new_real(1.0,"");
	else result=new_real(0.0,"");
	if (error) return;
	moveresult(st,result);
}

double rounder;

double rround (double x)
{	x*=rounder;
	if (x>0) x=floor(x+0.5);
	else x=-floor(-x+0.5);
	return x/rounder;
}

void cround (double *x, double *xi, double *z, double *zi)
{	*z=rround(*x);
	*zi=rround(*xi);
}

double frounder[]={1.0,10.0,100.0,1000.0,10000.0,100000.0,1000000.0,
10000000.0,100000000.0,1000000000.0,10000000000.0};

void mround (header *hd)
// round a number with n decimal places: round(x,n)
{	header *hd1;
	int n;
	hd1=next_param(hd);
	if (hd1) hd1=getvalue(hd1); if (error) return;
	if (hd1 && hd1->type!=s_real) need_arg_in("round","real, or real and int");
	if (hd1) n=(int)(*realof(hd1));
	else n=0;
	if (n>=0 && n<11) rounder=frounder[n];
	else rounder=pow(10.0,n);
	spread1(rround,cround,hd);
	test_error("round");
}

void mchar (header *hd)
// get a string with character with ascii number n: ascii(n)
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real) need_arg_in("char","integer");
	result=new_string("a",1,""); if (error) return;
	*stringof(result)=(char)*realof(hd);
	moveresult(st,result);
}

void mchartostr (header *hd)
{
	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	double *m;
	int n;
	if (hd->type==s_matrix && dimsof(hd)->r==1) 
	{
		n=dimsof(hd)->c;
		m=matrixof(hd);
	}
	else if (hd->type==s_real)
	{
		n=1;
		m=realof(hd);
	}
	else need_arg_in("chartostr","real row vector");
	result=new_string("",n,"");
	char *p=stringof(result);
	for (int i=0; i<n; i++) *p++=(char)(*m++);
	*p++=0;
	moveresult(st,result);
}

unsigned char *utftochar (unsigned char *p, int *res)
{
    int charcode = 0;
    int t = *p++;
    if (t < 128)
    {
		*res=t;
        return p;
    }
    int high_bit_mask = (1 << 6) -1;
    int high_bit_shift = 0;
    int total_bits = 0;
    const int other_bits = 6;
    while((t & 0xC0) == 0xC0)
    {
        t <<= 1;
        t &= 0xff;
        total_bits += 6;
        high_bit_mask >>= 1; 
        high_bit_shift++;
        charcode <<= other_bits;
		if (*p==0) break;
        charcode |= (*p++) & ((1 << other_bits)-1);
    } 
    charcode |= ((t >> high_bit_shift) & high_bit_mask) << total_bits;
	*res=charcode;
    return p;
}


void mutf (header *hd)
{
	header *st=hd,*result;
	hd=getvalue(hd);
	if (error) return;
	if (hd->type!=s_string)
		need_arg_in("utf","string");
	// print("%s\n",stringof(hd));
	result=new_string("",2,""); if (error) return;
	unsigned char *p=(unsigned char *)stringof(result);
	unsigned char *q=(unsigned char *)toutf(stringof(hd));
	p=exent(p,q,(unsigned char *)ramend);
	if (!p)
	{
		print("Illegal entity string, entity not found.\n%s\n",stringof(hd));
		error=1; return;
	}
	result->size=p-(unsigned char *)result;
	result->flags|=UTF;
	moveresult(st,result);
}

void mfromutf (header *hd)
{
	header *st=hd,*result;
	hd=getvalue(hd);
	if (error) return;
	if (hd->type!=s_string)
		need_arg_in("utf","string");
	unsigned char *q=(unsigned char *)fromutf(stringof(hd));
	if (!q)
	{
		print("Could not convert this string from UTF.\n");
		error=1; return;
	}
	result=new_string((char *)q,""); if (error) return;
	moveresult(st,result);
}

void mtoutf (header *hd)
{
	header *st=hd,*result;
	hd=getvalue(hd);
	if (error) return;
	if (hd->type!=s_string)
		need_arg_in("utf","string");
	unsigned char *q=(unsigned char *)toutf(stringof(hd));
	if (!q)
	{
		print("Could not convert this string to UTF.\n");
		error=1; return;
	}
	result=new_string((char *)q,""); if (error) return;
	result->flags|=UTF;
	moveresult(st,result);
}

void mchartoutf (header *hd)
{
	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	double *m;
	int n;
	if (hd->type==s_matrix && dimsof(hd)->r==1) 
	{
		n=dimsof(hd)->c;
		m=matrixof(hd);
	}
	else if (hd->type==s_real)
	{
		n=1;
		m=realof(hd);
	}
	else need_arg_in("chartoutf","real row vector");
	result=new_string("",0,"");
	char *p=stringof(result);
	for (int i=0; i<n; i++) 
	{
		if (p+8>ramend)
			outofram();
		p=chartoutf((int)*m++,p);
	}
	*p++=0;
	result->size=p-(char *)result;
	result->flags|=UTF;
	moveresult(st,result);
}


void mstrtochar (header *hd)
{
	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) need_arg_in("strtochar","string");
	unsigned char *p=(unsigned char *)stringof(hd);
	int n=strlen((char *)p);
	result=new_matrix(1,n,""); if (error) return;
	double *m=matrixof(result);
	if (hd->flags&UTF)
	{
		int n=0;
		while (*p)
		{
			int res;
			p=utftochar((unsigned char *)p,&res);
			*m++=res;
			n++;
		}
		dimsof(result)->c=n;
	}
	else
	{
		for (int i=0; i<n; i++) *m++=*p++;
	}
	moveresult(st,result);
}

void mtolower (header *hd)
// return the string with all lower case characters
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) need_arg_in("tolower","string");
	result=new_string(stringof(hd),strlen(stringof(hd)),""); if (error) return;
	char *p=stringof(result);
	while (*p) *p++=(char)tolower(*p);
	moveresult(st,result);
}

void mtoupper (header *hd)
// return the string with all upper case characters
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) need_arg_in("toupper","string");
	result=new_string(stringof(hd),strlen(stringof(hd)),""); if (error) return;
	char *p=stringof(result);
	while (*p) *p++=(char)toupper(*p);
	moveresult(st,result);
}

void mascii (header *hd)
// get the ascii number of the first character in a string
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) need_arg_in("ascii","string");
	result=new_real(*stringof(hd),""); if (error) return;
	moveresult(st,result);
}

void mstrlen (header *hd)
// get the length of a string
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) need_arg_in("strlen","string");
	if (hd->flags&UTF)
	{
		int n=0;
		unsigned char *p=(unsigned char *)stringof(hd);
		while (*p)
		{
			int res;
			p=utftochar(p,&res);
			n++;
		}
		result=new_real(n,""); if (error) return;
	}
	else
	{
		result=new_real(strlen(stringof(hd)),""); if (error) return;
	}
	moveresult(st,result);
}

void msubstring (header *hd)
// get a substring of a string
{	header *st=hd,*hd1,*hd2,*result;
	hd1=next_param(hd);
	hd2=next_param(hd1);
	hd=getvalue(hd); hd1=getvalue(hd1); hd2=getvalue(hd2); if (error) return;
	if (hd->type!=s_string || hd1->type!=s_real || hd2->type!=s_real) 
		need_arg_in("substring","string and two integers");
	int n=(int)*realof(hd1),m=(int)*realof(hd2),k=strlen(stringof(hd));
	if (n<0) n=k+n;
	else if (n>=0) n=n-1;
	if (m<0) m=k+m;
	else if (m>=0) m=m-1;
	if (n<0) n=0;
	if (m>k) m=k-1;
	if (m<n)
	{	result=new_string("",1,""); if (error) return;
	}
	else
	{	result=new_string("",m-n+2,""); if (error) return;
		strncpy(stringof(result),stringof(hd)+n,m-n+1);
		*(stringof(result)+(m-n+2))=0;
	}
	moveresult(st,result);
}

void mstrfind (header *hd)
// get a substring of a string
{	
	header *st=hd,*hd1,*hd2,*result;
	hd1=next_param(hd);
	hd2=next_param(hd1);
	hd=getvalue(hd); hd1=getvalue(hd1); 
	if (hd2) hd2=getvalue(hd2); 
	if (error) return;
	if (hd->type!=s_string || hd1->type!=s_string || (hd2 && hd2->type!=s_real)) 
		need_arg_in("strfind","string, substring, start position");
	int n=0;
	if (hd2) 
	{
		n=(int)*realof(hd2);
		if (n<0) n=strlen(stringof(hd))+n+1;
		if (n<0) n=0;
		else n=n-1;
	}
	char *p=0;
	if (n<(int)strlen(stringof(hd))) p=strstr(stringof(hd)+n,stringof(hd1));
	int k=0;
	if (p) k=p-stringof(hd)+1;
	result=new_real(k,""); if (error) return;
	moveresult(st,result);
}

/**
Search through a string with a regular expression.
*/
void mstrxfind (header *hd)
{	header *st=hd,*hd1,*hd2,*result;
	hd1=next_param(hd);
	hd2=next_param(hd1);
	hd=getvalue(hd); hd1=getvalue(hd1); 
	if (hd2) hd2=getvalue(hd2); 
	if (error) return;
	if (hd->type!=s_string || hd1->type!=s_string || (hd2 && hd2->type!=s_real)) 
		need_arg_in("strreg","string, substring, start position");
	int n=1;
	if (hd2) n=(int)*realof(hd2);
	if (n<0) n=strlen(stringof(hd))+n+1;
	if (n<=0) n=0;
	else n=n-1;
	using namespace std::tr1;
	std::string str=stringof(hd)+n;
	smatch res;
	try
	{
		regex rgx(stringof(hd1));
		regex_search(str,res,rgx);
	}
	catch (std::regex_error& e)
	{
		if (e.code() == std::regex_constants::error_collate)
			print("Invalid collating element name.\n");
		if (e.code() == std::regex_constants::error_ctype)
			print("Invalid character class name.\n");
		if (e.code() == std::regex_constants::error_escape)
			print("Invalid escaped character.\n");
		if (e.code() == std::regex_constants::error_backref)
			print("Invalid backward reference.\n");
		if (e.code() == std::regex_constants::error_brack)
			print("Invalid bracket.\n");
		if (e.code() == std::regex_constants::error_paren)
			print("Invalid parenthesis.\n");
		if (e.code() == std::regex_constants::error_brace)
			print("Invalid braces.\n");
		if (e.code() == std::regex_constants::error_badbrace)
			print("Invalid range specifier in braces.\n");
		if (e.code() == std::regex_constants::error_range)
			print("Invalid character range.\n");
		if (e.code() == std::regex_constants::error_space)
			print("Insufficient memory.\n");
		if (e.code() == std::regex_constants::error_complexity)
			print("Too complex.\n");
		if (e.code() == std::regex_constants::error_badrepeat)
			print("Invalid repeat specifier\n");
		if (e.code() == std::regex_constants::error_stack)
			print("Out of stack.\n");
		print("Error in regular expression\n%s\n",stringof(hd1));
		error=1; return;
	}
	if (res.size()>0)
	{
		result=new_real(res.position()+n+1,"");
		std::string r=res[0];
		new_string((char *)r.c_str(),""); if (error) return;
		header *h=new_smatrix(""); if (error) return;
		char *p=stringof(h);
		for (int i=1; i<(int)res.size(); i++)
		{
			r=res[i];
			char *hr=(char *)r.c_str();
			int nr=strlen(hr);
			checkram(p+nr+2);
			strcpy(p,hr);
			p+=nr+1;
		}
		*p++=1;
		newram=p;
		h->size=p-(char *)h;
	}
	else
	{
		result=new_real(0,"");
	}
	moveresult1(st,result);
}

void mstrrepl (header *hd)
{
	header *st=hd,*hd1,*hd2,*result;
	hd1=next_param(hd);
	hd2=next_param(hd1);
	hd=getvalue(hd); hd1=getvalue(hd1); hd2=getvalue(hd2); if (error) return;
	if (hd->type!=s_string || hd1->type!=s_string || hd2->type!=s_string)
		need_arg_in("strrepl","string, search string and replacement");
	result=new_string("",""); if (error) return;
	char *s=stringof(hd);
	char *search=stringof(hd1);
	char *repl=stringof(hd2);
	char *p=stringof(result);
	int nr=strlen(repl);
	while (*s)
	{
		char *h=strstr(s,search);
		if (!h) break;
		if (h>s)
		{
			if (p+(h-s)>ramend) outofram();
			strncpy(p,s,h-s);
			p+=h-s;
		}
		s=h+strlen(search);
		if (p+nr>ramend) outofram();
		strcpy(p,repl);
		p+=nr;
		if (test_key())
		{
			error=1; break;
		}
	}
	if (p+strlen(s)+1>ramend) outofram();
	strcpy(p,s); p+=strlen(s)+1;
	newram=p+strlen(p)+1;
	result->size=(size_t)(newram-(char *)result);
	moveresult(st,result);
}

void mstrxrepl (header *hd)
{
	header *st=hd,*hd1,*hd2,*result;
	hd1=next_param(hd);
	hd2=next_param(hd1);
	hd=getvalue(hd); hd1=getvalue(hd1); hd2=getvalue(hd2); if (error) return;
	if (hd->type!=s_string || hd1->type!=s_string || hd2->type!=s_string)
		need_arg_in("strrepl","string, search string and replacement");
	result=new_string("",""); if (error) return;
	std::string str=stringof(hd);
	std::string strrepl=stringof(hd2);
	std::string strres;
	using namespace std::tr1;
	smatch res;
	try
	{
		regex rgx(stringof(hd1));
		strres = regex_replace(str,rgx,strrepl);
	}
	catch (std::regex_error& e)
	{
		if (e.code() == std::regex_constants::error_collate)
			print("Invalid collating element name.\n");
		if (e.code() == std::regex_constants::error_ctype)
			print("Invalid character class name.\n");
		if (e.code() == std::regex_constants::error_escape)
			print("Invalid escaped character.\n");
		if (e.code() == std::regex_constants::error_backref)
			print("Invalid backward reference.\n");
		if (e.code() == std::regex_constants::error_brack)
			print("Invalid bracket.\n");
		if (e.code() == std::regex_constants::error_paren)
			print("Invalid parenthesis.\n");
		if (e.code() == std::regex_constants::error_brace)
			print("Invalid braces.\n");
		if (e.code() == std::regex_constants::error_badbrace)
			print("Invalid range specifier in braces.\n");
		if (e.code() == std::regex_constants::error_range)
			print("Invalid character range.\n");
		if (e.code() == std::regex_constants::error_space)
			print("Insufficient memory.\n");
		if (e.code() == std::regex_constants::error_complexity)
			print("Too complex.\n");
		if (e.code() == std::regex_constants::error_badrepeat)
			print("Invalid repeat specifier\n");
		if (e.code() == std::regex_constants::error_stack)
			print("Out of stack.\n");
		print("Error in regular expression\n%s\n",stringof(hd1));
		error=1; return;
	}
	result=new_string((char *)strres.c_str(),"");
	moveresult1(st,result);
}

void mstrtokens (header *hd)
{
	header *st=hd,*hd1,*result;
	hd1=next_param(hd);
	hd=getvalue(hd); if (error) return;
	if (hd1) hd1=getvalue(hd1); if (error) return;
	if (hd->type!=s_string || (hd1 && hd1->type!=s_string)) need_arg_in("strtokens","two strings");
	result=new_smatrix("");
	char *p=stringof(hd);
	char *pc=";, \x09";
	if (hd1) pc=stringof(hd1);
	char *r=stringof(result);
	int n=strspn(p,pc);
	p+=n;
	while (true)
	{
		int n=strcspn(p,pc);
		checkram(r+n+2);
		strncpy(r,p,n);
		r+=n; *r++=0;
		p+=n; if (*p==0) break;
		n=strspn(p,pc);
		p+=n; if (*p==0) break;
	}
	*r++=1;
	newram=r;
	result->size=r-(char *)result;
	moveresult(st,result);
}

void merror (header *hd)
// issue an error in Euler
{	
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) need_arg_in("error","string");
	output1("Error : %s\n",stringof(hd));
	error=ERROR_USER;
}

extern int preventoutput;

void merrlevel (header *hd)
{	
	header *st=hd,*res;
	char *oldnext;
	int en;
	hd=getvalue(hd);
	res=new_real(0,"");
	if (hd->type!=s_string) need_arg_in("errorlevel","string");
	int po0=preventoutput;
	preventoutput=1;
	int so0=stringon;
	stringon=1;
	oldnext=next; next=stringof(hd);
	scan();
	next=oldnext;
	header *hd1=nextof(res);
	if (hd1->type==s_reference)
	{	
		hd1=getvalue(hd1);
		if (hd1==0) error=ERROR_INTERNAL;
	}
	preventoutput=po0;
	stringon=so0;
	en=error; error=0;
	if (en)
	{	
		*realof(res)=en;
		moveresult(st,res);
	}
	else
	{	
		moveresult1(st,res);
	}
}

void mprintf (header *hd)
{	header *st=hd,*hd1,*result;
	char string[MAXLINE];
	hd1=next_param(hd);
	hd=getvalue(hd);
	hd1=getvalue(hd1); if (error) return;
	if (hd->type!=s_string)
		need_arg_in("printf","format string");
	char *p=stringof(hd);
	int length=strlen(p);
	char format=0;
	int type=0;
	while (*p)
	{
		if (*p=='%')
		{
			p++;
			if (*p!='%') 
			{
				if (format)
				{
					print("Only one format specification in printf allowed!\n");
					error=1; return;				
				}
				int n=0;
				while (*p && (*p=='-' || *p=='.' || xisdigit(*p))) 
				{
					if (xisdigit(*p)) n=10*n+(*p-'0');
					if (n+length>MAXLINE-32)
					{
						print("Format too long!\n");
						error=1; return;
					}
					if (!xisdigit(*p)) n=0;
					p++;
				}
				if (!*p)
				{
					print("Need a format specification in printf!\n");
					error=1; return;
				}
				format=*p;
			}
		}
		p++;
	}
	if (format=='f' || format=='e' || format=='g' || format=='F' || format=='E' || format=='G'
		|| format=='a' || format=='A') 
	{
		if (hd1->type!=s_real) need_arg_in("printf","double format needs a real");
		type=0;
	}
	else if (format=='d' || format=='i' || format=='u' || format=='o' || format=='x' || format=='X') 
	{
		if (hd1->type!=s_real) need_arg_in("printf","double format needs a real");
		type=1;
	}
	else if (format=='s') 
	{
		if (hd1->type!=s_string) need_arg_in("printf","format %s needs a string");
		type=2;
	}
	else if (format=='c') 
	{
		if (hd1->type!=s_real) need_arg_in("printf","format %c needs a real");
		type=3;
	}
	else if (format==0)
	{
		print("No format string!\n",format);
		error=1; return;
	}
	else
	{
		print("Illegal format string %c in format",format);
		error=1; return;
	}
	double x;
	switch (type)
	{
		case 0 :
			x=*realof(hd1);
			if (x==0) x=0.0;
			sprintf(string,stringof(hd),x);
			break;
		case 1 :
			x=*realof(hd1);
			sprintf(string,stringof(hd),(int)x);
			break;
		case 2 :
			sprintf(string,stringof(hd),stringof(hd1));
			break;
		case 3 :
			x=*realof(hd1);
			sprintf(string,stringof(hd),(char)x);
			break;
	}
	result=new_string(string,strlen(string),""); if (error) return;
	moveresult(st,result);
}

/**
print(x,digits,length,separator,dot)
*/
void mprint (header *hd)
{
	header *st=hd,*hdn,*hdl,*hdsep,*hddot,*res;
	hdn=nextof(hd);
	hdl=nextof(hdn);
	hdsep=nextof(hdl);
	hddot=nextof(hdsep);
	hd=getvalue(hd);
	hdn=getvalue(hdn);
	hdl=getvalue(hdl);
	hdsep=getvalue(hdsep);
	hddot=getvalue(hddot);
	if (error) return;
	if (hd->type!=s_real || hdn->type!=s_real || hdl->type!=s_real ||
			hdsep->type!=s_string || hddot->type!=s_string)
				need_arg_in("print","x,digizs,length,separator,dot");
	char format[32];
	sprintf(format,"%%1.%0df",(int)(abs(*realof(hdn))));
	char s[MAXLINE],t[MAXLINE];
	sprintf(s,format,*realof(hd));
	char *sep=stringof(hdsep);
	int nsep=strlen(sep);
	if (!isnone(hdsep) && nsep>0)
	{
		char *ps=s,*pt=t;
		while (!isdigit(*ps)) *pt++=*ps++;
		char *h=ps;
		while (*h && isdigit(*h)) h++;
		int n=(int)(h-ps);
		while (ps<h)
		{
			*pt++=*ps++;
			if (n>3 && n%3==1)
			{
				strcpy(pt,sep); pt=pt+nsep;
			}
			n--;
		}
		strcpy(pt,ps);
		strcpy(s,t);
	}
	int nl=(int)abs(*realof(hdl));
	if (nl>(int)strlen(s))
	{
		char *pt=t;
		for (int i=0; i<(int)(nl-strlen(s)); i++) *pt++=' ';
		strcpy(pt,s);
		strcpy(s,t);
	}
	if (!isnone(hddot) && *stringof(hddot))
	{
		char *h=s;
		char c=*stringof(hddot);
		while (*h)
		{
			if (*h=='.') { *h=c; break; }
			h++;
		}
	}
	res=new_string(s,"");
	moveresult(st,res);
}

/**
Return a string to n plarces according to mode (left,center,right)
*/
void mprintstr (header *hd)
{
	header *st=hd,*hdn,*hdmode,*hdchar,*res;
	hdn=nextof(hd); 
	hdmode=next_param(hdn);
	hdchar=next_param(hdmode);
	hd=getvalue(hd);
	hdn=getvalue(hdn);
	if (hdmode) hdmode=getvalue(hdmode);
	if (hdchar) hdchar=getvalue(hdchar);
	if (hd->type!=s_string || hdn->type!=s_real
		|| (hdmode && hdmode->type!=s_real) 
		|| (hdchar && hdchar->type!=s_real && hdchar->type!=s_string) )
		need_arg_in("printstr","string, length, mode (optional), fillcharacter (optional)");
	int n=(int)*realof(hdn);
	if (n<0) n=0;
	if (n>MAXLINE/2) n=MAXLINE/2;
	int mode=1;
	if (hdmode) mode=(int)*realof(hdmode);
	char ch=' ';
	if (hdchar && hdchar->type==s_real) ch=(char)*realof(hdchar);
	if (hdchar && hdchar->type==s_string) ch=*stringof(hdchar);
	if (ch==0 || ch==1) ch=' ';
	char *p=stringof(hd);
	int pn=strlen(p);
	if (n<pn) n=pn;
	res=new_string("",n,"");
	char *s=stringof(res);
	for (int i=0; i<n; i++) *s++=ch;
	s=stringof(res);
	if (mode<0) memcpy(s,p,pn);
	else if (mode>0) memcpy(s+(n-pn),p,pn);
	else memcpy(s+(n-pn)/2,p,pn);
	moveresult(st,res);
}

void msetkey (header *hd)
/*****
	set a function key
*****/
{	header *st=hd,*hd1,*result;
	char *p;
	int n;
	hd=getvalue(hd); if (error) return;
	hd1=nextof(st); hd1=getvalue(hd1); if (error) return;
	if (hd->type!=s_real || hd1->type!=s_string) need_arg_in("setkey","integer and string");
	n=(int)(*realof(hd))-1; p=stringof(hd1);
	if (n<0 || n>=10 || strlen(p)>63) need_arg_in("setkey","1<=key number<=10, string");
	result=new_string(fktext[n],strlen(fktext[n]),"");
	if (error) return;
	strcpy(fktext[n],p);
	moveresult(st,result);
}

void many (header *hd)
{	header *st=hd,*result;
	int c,r,res=0;
	LONG i,n;
	double *m;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		n=(LONG)(c)*r;
	}
	else if (hd->type==s_complex || hd->type==s_cmatrix)
	{	getmatrix(hd,&r,&c,&m);
        n=(LONG)2*(LONG)(c)*r;
	}
	else need_arg_in("any","matrix");
	for (i=0; i<n; i++)
		if (*m++!=0.0) { res=1; break; }
	result=new_real(res,""); if (error) return;
	moveresult(st,result);
}

void mall (header *hd)
{	header *st=hd,*result;
	int c,r,res=1;
	LONG i,n;
	double *m;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		n=(LONG)(c)*r;
	}
	else if (hd->type==s_complex || hd->type==s_cmatrix)
	{	getmatrix(hd,&r,&c,&m);
        n=(LONG)2*(LONG)(c)*r;
	}
	else need_arg_in("all","matrix");
	for (i=0; i<n; i++)
		if (*m++==0.0) { res=0; break; }
	result=new_real(res,""); if (error) return;
	moveresult(st,result);
}

void mcd (header *hd)
{	
	header *st=hd,*result;
	char *path;
	if (!hd || hd==(header *)newram)
	{	
		path=cd("");
		result=new_string(path,strlen(path),"");
	}
	else
	{	
		hd=getvalue(hd); if (error) return;
		if (hd->type!=s_string) need_arg_in("cd","string");
		path=cd(stringof(hd));
		result=new_string(path,strlen(path),"");
		moveresult(st,result);
	}
}

void mcreatedir (header *hd)
{
	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) need_arg_in("makedir","string");
	result=new_real(makedir(stringof(hd)),"");
	moveresult(st,result);
}

void mhome (header *hd)
{	char *path=gethomedir();
	new_string(path,strlen(path),"");
}

void muserhome (header *hd)
{	char *path=getuserhomedir();
	new_string(path,strlen(path),"");
}

void meulerhome (header *hd)
{	char *path=getusereulerdir();
	new_string(path,strlen(path),"");
}

void muserdir (header *hd)
{	char *path=getuserdir();
	new_string(path,strlen(path),"");
}

void mstart (header *hd)
{	char *path=getstartdir();
	new_string(path,strlen(path),"");
}

void mgetarg (header *hd)
/* return the realof(hd)-st argument */
{	header *st=hd,*hd1,*result;
	int i,n;
	ULONG size;
	hd=getvalue(hd);
	if (!udfon)
	{	
		output("getarg works only in functions!\n");
		error=ERROR_NOT_IN_FUNCTION; return;
	}
	if (hd->type!=s_real) need_arg_in("getarg","nonzero index");
	n=(int)*realof(hd);
	if (n<1) need_arg_in("getarg","nonzero index");
	if (n>actargn)
	{	newram=(char *)st; return;
	}
	result=(header *)startlocal; i=1;
	while (i<n && result<(header *)endlocal)
	{	result=nextof(result); i++;
	}
	hd1=result;
	hd1=nextof(hd1);
	size=(char *)hd1-(char *)result;
	if (size<=0)
	{	
		output1("Error in arg! Argument %d does not exist.\n",n); 
		error=ERROR_ARGUMENT; return;
	}
	memmove((char *)st,(char *)result,size);
	newram=(char *)st+size;
}

void margs (header *hd)
/* return all args from realof(hd)-st argument on */
{	
	header *st=hd,*hd1,*result;
	int i,n;
	ULONG size;
	hd=getvalue(hd);
	if (hd->type!=s_real) need_arg_in("args","integer");
	n=(int)*realof(hd);
	if (n<1) need_arg_in("args","integer>0");
	if (n>actargn)
	{	newram=(char *)st; return;
	}
	result=(header *)startlocal; i=1;
	while (i<n && result<(header *)endlocal)
	{	result=nextof(result); i++;
	}
	hd1=result;
	while (i<actargn+1 && hd1<(header *)endlocal)
	{	hd1=nextof(hd1); i++;
	}
	size=(char *)hd1-(char *)result;
	if (size<=0)
	{	output("Error in args!\n"); error=ERROR_ARGUMENT; return;
	}
	memmove((char *)st,(char *)result,size);
	st->flags|=ARGS;
	newram=(char *)st+size;
}

void margs0 (header *hd)
// copy the extra arguments to the current function
// to the stack position
{	header *st=(header *)newram,*hd1,*result;
	int i,n;
	ULONG size;
	// output1("%d %d\n",actsp,actargn);
	if (actsp==0 || actsp>=actargn)
	{	
		new_none();
		newram=(char *)st;
		return;
	}
	n=actsp+1;
	if (n>actargn) n=actargn;
	result=(header *)startlocal; 
	i=1;
	while (i<n && result<(header *)endlocal)
	{	result=nextof(result); i++;
	}
	hd1=result;
	while (i<actargn+1 && hd1<(header *)endlocal)
	{	hd1=nextof(hd1); i++;
	}
	size=(char *)hd1-(char *)result;
	if (size<=0)
	{	output("Error in args!\n"); error=ERROR_ARGUMENT; return;
	}
	memmove((char *)st,(char *)result,size);
	st->flags|=ARGS;
	newram=(char *)st+size;
}

void mname (header *hd)
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	result=new_string(hd->name,strlen(hd->name),"");
	moveresult(st,result);
}

void msearchnextfile (header *hd)
{	char *name;
	name=dir(0);
	if (name) new_string(name,strlen(name),"");
	else new_string("",0,"");
}

void msearchfile (header *hd)
{	header *st=hd,*result;
	char *name;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) need_arg_in("searchfile","string");
	name=dir(stringof(hd));
	if (name) result=new_string(name,strlen(name),"");
	else result=new_string("",0,"");
	if (error) return;
	moveresult(st,result);
}

void mdir (header *hd)
{
	header *st=hd,*result;
	hd=getvalue(hd);
	if (hd->type!=s_string)
	{
		print("Need a file pattern for dir!\n");
		error=1; return;
	}
	char *pattern=stringof(hd);
	result=new_smatrix("");
	char *p=stringof(result);
	char *s=dir(pattern);
	while (s!=0)
	{
		if (*s!='.')
		{	
			strcpy(p,s);
			p+=strlen(p)+1;
		}
		s=dir(0);
	}
	*p++=1;
	newram=p;
	result->size=p-(char *)result;
	moveresult(st,result);
}

void mflipx (header *hd)
{	header *st=hd,*result;
	double *m,*mr,*mr1;
	int i,j,c,r;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_complex || hd->type==s_interval)
	{	moveresult(st,hd); return;
	}
	else if (hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_matrix(r,c,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
		{	mr1=mr+(c-1);
			for (j=0; j<c; j++) *mr1--=*m++;
			mr+=c;
		}
	}
	else if (hd->type==s_cmatrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_cmatrix(r,c,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
		{	mr1=mr+(2l*(c-1)+1);
			for (j=0; j<c; j++)
			{	*mr1--=*(m+1); *mr1--=*m; m+=2;
			}
			mr+=2l*c;
		}
	}
	else if (hd->type==s_imatrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_imatrix(r,c,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
		{	mr1=mr+(2l*(c-1)+1);
			for (j=0; j<c; j++)
			{	*mr1--=*(m+1); *mr1--=*m; m+=2;
			}
			mr+=2l*c;
		}
	}
	else need_arg_in("flipx","matrix");
	moveresult(st,result);
}

void mflipy (header *hd)
{	header *st=hd,*result;
	double *m,*mr;
	int i,c,r;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_complex || hd->type==s_interval)
	{	moveresult(st,hd); return;
	}
	else if (hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_matrix(r,c,""); if (error) return;
		mr=matrixof(result);
		mr+=(long)(r-1)*c;
		for (i=0; i<r; i++)
		{	memmove((char *)mr,(char *)m,c*sizeof(double));
			m+=c; mr-=c;
		}
	}
	else if (hd->type==s_cmatrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_cmatrix(r,c,""); if (error) return;
		mr=matrixof(result);
		mr+=2l*(long)(r-1)*c;
		for (i=0; i<r; i++)
		{	memmove((char *)mr,(char *)m,2l*c*sizeof(double));
			m+=2l*c; mr-=2l*c;
		}
	}
	else if (hd->type==s_imatrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_imatrix(r,c,""); if (error) return;
		mr=matrixof(result);
		mr+=2l*(long)(r-1)*c;
		for (i=0; i<r; i++)
		{	memmove((char *)mr,(char *)m,2l*c*sizeof(double));
			m+=2l*c; mr-=2l*c;
		}
	}
	else need_arg_in("flipy","matrix");
	moveresult(st,result);
}

void mrotleft (header *hd)
{	header *st=hd,*result;
	double *m,*mr,*mr1;
	int i,j,c,r;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_complex || hd->type==s_interval)
	{	moveresult(st,hd); return;
	}
	else if (hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_matrix(r,c,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
		{   mr1=m+1;
        	for (j=0; j<c-1; j++) *mr++=*mr1++;
            *mr++=*m;
            m+=c;
		}
	}
	else if (hd->type==s_cmatrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_cmatrix(r,c,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
		{	mr1=m+2l;
			for (j=0; j<c-1; j++)
			{	*mr++=*mr1++; *mr++=*mr1++;
			}
            *mr++=*m; *mr++=*(m+1);
			m+=2l*c;
		}
	}
	else if (hd->type==s_imatrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_imatrix(r,c,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
		{	mr1=m+2l;
			for (j=0; j<c-1; j++)
			{	*mr++=*mr1++; *mr++=*mr1++;
			}
            *mr++=*m; *mr++=*(m+1);
			m+=2l*c;
		}
	}
	else need_arg_in("rotleft","matrix");
	moveresult(st,result);
}

void mrotright (header *hd)
{	header *st=hd,*result;
	double *m,*mr,*mr1;
	int i,j,c,r;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_complex || hd->type==s_interval)
	{	moveresult(st,hd); return;
	}
	else if (hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_matrix(r,c,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
		{   mr1=m;
            *mr++=*(m+c-1);
        	for (j=1; j<c; j++) *mr++=*mr1++;
            m+=c;
		}
	}
	else if (hd->type==s_cmatrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_cmatrix(r,c,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
		{	mr1=m;
            *mr++=*(m+2l*(c-1)); *mr++=*(m+2l*(c-1)+1);
			for (j=1; j<c; j++)
			{	*mr++=*mr1++; *mr++=*mr1++;
			}
			m+=2l*c;
		}
	}
	else if (hd->type==s_imatrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_imatrix(r,c,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
		{	mr1=m;
            *mr++=*(m+2l*(c-1)); *mr++=*(m+2l*(c-1)+1);
			for (j=1; j<c; j++)
			{	*mr++=*mr1++; *mr++=*mr1++;
			}
			m+=2l*c;
		}
	}
	else need_arg_in("rotright","matrix");
	moveresult(st,result);
}

void mshiftleft (header *hd)
{	header *st=hd,*result;
	double *m,*mr,*mr1;
	int i,j,c,r;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_complex || hd->type==s_interval)
	{	moveresult(st,hd); return;
	}
	else if (hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_matrix(r,c,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
		{   mr1=m+1;
        	for (j=0; j<c-1; j++) *mr++=*mr1++;
            *mr++=0;
            m+=c;
		}
	}
	else if (hd->type==s_cmatrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_cmatrix(r,c,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
		{	mr1=m+2l;
			for (j=0; j<c-1; j++)
			{	*mr++=*mr1++; *mr++=*mr1++;
			}
            *mr++=0; *mr++=0;
			m+=2l*c;
		}
	}
	else if (hd->type==s_imatrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_imatrix(r,c,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
		{	mr1=m+2l;
			for (j=0; j<c-1; j++)
			{	*mr++=*mr1++; *mr++=*mr1++;
			}
            *mr++=0; *mr++=0;
			m+=2l*c;
		}
	}
	else need_arg_in("shiftleft","matrix");
	moveresult(st,result);
}

void mshiftright (header *hd)
{	header *st=hd,*result;
	double *m,*mr,*mr1;
	int i,j,c,r;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_complex || hd->type==s_interval)
	{	moveresult(st,hd); return;
	}
	else if (hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_matrix(r,c,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
		{   mr1=m;
            *mr++=0;
        	for (j=1; j<c; j++) *mr++=*mr1++;
            m+=c;
		}
	}
	else if (hd->type==s_cmatrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_cmatrix(r,c,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
		{	mr1=m;
            *mr++=0; *mr++=0;
			for (j=1; j<c; j++)
			{	*mr++=*mr1++; *mr++=*mr1++;
			}
			m+=2l*c;
		}
	}
	else if (hd->type==s_imatrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_imatrix(r,c,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
		{	mr1=m;
            *mr++=0; *mr++=0;
			for (j=1; j<c; j++)
			{	*mr++=*mr1++; *mr++=*mr1++;
			}
			m+=2l*c;
		}
	}
	else need_arg_in("shiftright","matrix");
	moveresult(st,result);
}

void mredim (header *hd)
{	header *st=hd,*hd1,*result;
	int c1,r1;
	double *m;
	ULONG i,n,size1,size;
	hd1=nextof(hd);
	get_size(hd1,&r1,&c1,"redim"); if (error) return;
	hd=getvalue(hd); if (error) return;
	size1=(long)c1*r1;
	size=(long)dimsof(hd)->c*dimsof(hd)->r;
	if (size<size1) n=size;
	else n=size1;
	if (hd->type==s_matrix)
	{	result=new_matrix(r1,c1,""); if (error) return;
		memmove((char *)matrixof(result),(char *)matrixof(hd),
			n*sizeof(double));
		if (n<size1)
		{	m=matrixof(result)+n;
			for (i=n; i<size1; i++) *m++=0.0;
		}
	}
	else if (hd->type==s_cmatrix)
	{	result=new_cmatrix(r1,c1,""); if (error) return;
		memmove((char *)matrixof(result),(char *)matrixof(hd),
			2*n*sizeof(double));
		if (n<size1)
		{	m=matrixof(result)+2*n;
			for (i=n; i<size1; i++) { *m++=0.0; *m++=0.0; }
		}
	}
	else if (hd->type==s_imatrix)
	{	result=new_imatrix(r1,c1,""); if (error) return;
		memmove((char *)matrixof(result),(char *)matrixof(hd),
			2*n*sizeof(double));
		if (n<size1)
		{	m=matrixof(result)+2*n;
			for (i=n; i<size1; i++) { *m++=0.0; *m++=0.0; }
		}
	}
	else need_arg_in("redim","matrix A, new size n, m");
	moveresult(st,result);
}

void mroundbrackets (header *hd)
// set the acceptance of round zeros
{	
	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real)
	{	
		output("Roundbrackets needs a 1 or 0!\n");
		error=ERROR_WRONG_ARGUMENT; return;
	}
	result=new_real(roundbrackets,"");
	roundbrackets=(*realof(hd)!=0.0);
	moveresult(st,result);
}

void mgivensrot (header *hd)
// one Givens rotation
{	
	header *st=hd,*hd0,*hd1,*hd2,*hd3,*result2,*result3;
	double *m2,*m3,*mr2,*mr3;
	int i0,i,j,r2,c2,r3,c3;
	hd0=nextof(hd);
	hd1=nextof(hd0);
	hd2=nextof(hd1);
	hd3=nextof(hd2);
	hd=getvalue(hd); if (error) return;
	hd0=getvalue(hd0); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	hd2=getvalue(hd2); if (error) return;
	hd3=getvalue(hd3); if (error) return;
	if (hd->type!=s_real || hd0->type!=s_real || hd1->type!=s_real ||
		hd2->type!=s_matrix || hd3->type!=s_matrix ||
		dimsof(hd2)->r!=dimsof(hd3)->r)
			need_arg_in("givensrot","indices j,i1,i2 and two matrices with n rows");
	getmatrix(hd2,&r2,&c2,&m2);
	getmatrix(hd3,&r3,&c3,&m3);
	i0=(int)(*realof(hd));
	i=(int)(*realof(hd0));
	j=(int)(*realof(hd1));
	if (i==j || i<1 || i>r2 || i>c2 || j<1 || j>r2 || i0<1 || i0>c2 || r2!=r3)
		need_arg_in("givensrot","indices j, i1,i2 (different) and two matrices with n rows");
	result2=new_matrix(r2,c2,"");
	mr2=matrixof(result2);
	memcpy(mr2,m2,r2*c2*sizeof(double));
	result3=new_matrix(r3,c3,"");
	mr3=matrixof(result3);
	memcpy(mr3,m3,r3*c3*sizeof(double));
	double *m2i=mr2+c2*(i-1),*m3i=mr3+c3*(i-1);
	double *m2j=mr2+c2*(j-1),*m3j=mr3+c3*(j-1);
	double s=m2j[i0-1];
	double c=-m2i[i0-1];
	double h=sqrt(c*c+s*s);
	if (h!=0)
	{	c/=h; s/=h;
		for (int k=0; k<c2; k++)
		{	double xi=m2i[k],xj=m2j[k];
			m2i[k]=xi*c-xj*s;
			m2j[k]=xi*s+xj*c;
		}
		for (int k=0; k<c3; k++)
		{	double xi=m3i[k],xj=m3j[k];
			m3i[k]=xi*c-xj*s;
			m3j[k]=xi*s+xj*c;
		}
	}
	moveresult1(st,result2);
}

void mgivensqr (header *hd)
// decompose a matrix into Q and R
{	header *st=hd,*hd2=hd,*hd3,*result2,*result3,*result;
	double *m2,*m3,*mr2,*mr3;
	int i,j,r2,c2,r3,c3;
	hd3=nextof(hd2);
	hd2=getvalue(hd2); if (error) return;
	hd3=getvalue(hd3); if (error) return;
	if (hd2->type!=s_matrix || hd3->type!=s_matrix ||
		dimsof(hd2)->r!=dimsof(hd3)->r)
			need_arg_in("givensqr","two real matrices with same number of rows");
	getmatrix(hd2,&r2,&c2,&m2);
	getmatrix(hd3,&r3,&c3,&m3);
	result2=new_matrix(r2,c2,""); if (error) return;
	mr2=matrixof(result2);
	memcpy(mr2,m2,r2*c2*sizeof(double));
	result3=new_matrix(r3,c3,""); if (error) return;
	mr3=matrixof(result3);
	memcpy(mr3,m3,r3*c3*sizeof(double));
	result=new_matrix(1,c2,""); if (error) return;
	double *m=matrixof(result);
	int ri=0;
	for (i=0; i<c2; i++)
	{	
		for (j=ri+1; j<r2; j++)
		{	
			double *m2i=mr2+c2*ri,*m3i=mr3+c3*ri;
			double *m2j=mr2+c2*j,*m3j=mr3+c3*j;
			double s=m2j[i];
			double c=-m2i[i];
			double h=sqrt(c*c+s*s);
			if (fabs(h)>epsilon)
			{	
				c/=h; s/=h;
				for (int k=i; k<c2; k++)
				{	
					double xi=m2i[k],xj=m2j[k];
					m2i[k]=xi*c-xj*s;
					m2j[k]=xi*s+xj*c;
				}
				for (int k=0; k<c3; k++)
				{	
					double xi=m3i[k],xj=m3j[k];
					m3i[k]=xi*c-xj*s;
					m3j[k]=xi*s+xj*c;
				}
			}
		}
		if (fabs(mr2[c2*ri+i])>epsilon)
		{	*m++=1.0; ri++;
		}
		else *m++=0.0;
	}
	moveresult1(st,result2);
}


void mfold (header *hd)
// fold a matrix with another matrix, or with a vector
{	header *st=hd,*hd1,*result;
	hd1=nextof(hd);
	hd=getvalue(hd);
	hd1=getvalue(hd1);
	if (error) return;
	if (hd->type==s_matrix && hd1->type==s_matrix)
	{	double *m,*m1;
		int r,c,r1,c1;
		getmatrix(hd,&r,&c,&m);
		getmatrix(hd1,&r1,&c1,&m1);
		if (r1>r || c1>c)
			need_arg_in("fold","matrix and smaller folding matrix");
		result=new_matrix(r-r1+1,c-c1+1,"");
		double *mr=matrixof(result);
		for (int i=0; i<r-r1+1; i++)
		{	m=matrixof(hd)+i*c;
			for (int j=0; j<c-c1+1; j++)
			{	*mr=0;
				double *mh=m,*mhh=m,*m1h=m1;
				for (int i1=0; i1<r1; i1++)
				{	for (int j1=0; j1<c1; j1++)
					{	*mr+=(*mh++)*(*m1h++);
					}
					mhh=mhh+c;
					mh=mhh;
				}
				m++; mr++;
			}
		}
	}
	else if (hd->type==s_cmatrix && hd1->type==s_matrix)
	{	double *m,*m1;
		int r,c,r1,c1;
		getmatrix(hd,&r,&c,&m);
		getmatrix(hd1,&r1,&c1,&m1);
		if (r1>r || c1>c)
			need_arg_in("fold","matrix and smaller folding matrix");
		result=new_cmatrix(r-r1+1,c-c1+1,"");
		double *mr=matrixof(result);
		for (int i=0; i<r-r1+1; i++)
		{	m=matrixof(hd)+2*i*c;
			for (int j=0; j<c-c1+1; j++)
			{	*mr=0; *(mr+1)=0;
				double *mh=m,*mhh=m,*m1h=m1;
				for (int i1=0; i1<r1; i1++)
				{	for (int j1=0; j1<c1; j1++)
					{	*mr+=(*mh)*(*m1h);
						*(mr+1)+=(*(mh+1))*(*m1h);
						mh+=2; m1h++;
					}
					mhh=mhh+2*c;
					mh=mhh;
				}
				m+=2; mr+=2;
			}
		}
	}
	else if (hd->type==s_cmatrix && hd1->type==s_cmatrix)
	{	double *m,*m1;
		int r,c,r1,c1;
		getmatrix(hd,&r,&c,&m);
		getmatrix(hd1,&r1,&c1,&m1);
		if (r1>r || c1>c)
			need_arg_in("fold","matrix and smaller folding matrix");
		result=new_cmatrix(r-r1+1,c-c1+1,"");
		double *mr=matrixof(result);
		for (int i=0; i<r-r1+1; i++)
		{	m=matrixof(hd)+2*i*c;
			for (int j=0; j<c-c1+1; j++)
			{	*mr=0; *(mr+1)=0;
				double *mh=m,*mhh=m,*m1h=m1;
				for (int i1=0; i1<r1; i1++)
				{	for (int j1=0; j1<c1; j1++)
					{	*mr+=(*mh)*(*m1h)-(*(mh+1))*(*(m1h+1));
						*(mr+1)+=(*mh)*(*(m1h+1))+(*(mh+1))*(*m1h);
						mh+=2; m1h+=2;
					}
					mhh=mhh+2*c;
					mh=mhh;
				}
				m+=2; mr+=2;
			}
		}
	}
	else
		need_arg_in("fold","matrix and smaller folding matrix");
	moveresult(st,result);
}

void mfilter (header *hd)
// filter as in Matlab (almost)
{
	header *hdb=hd;
	header *hda=nextof(hdb);
	header *hdx=nextof(hda);
	header *hdy=nextof(hdx);
	hdb=getvalue(hdb); if (error) return;
	hda=getvalue(hda); if (error) return;
	hdx=getvalue(hdx); if (error) return;
	hdy=getvalue(hdy); if (error) return;
	double *ma,*mb,*mx,*my;
	int ra,ca,rb,cb,rx,cx,ry,cy;
	getmatrix(hdb,&rb,&cb,&mb,"filter"); if (error) return;
	getmatrix(hda,&ra,&ca,&ma,"filter"); if (error) return;
	getmatrix(hdx,&rx,&cx,&mx,"filter"); if (error) return;
	getmatrix(hdy,&ry,&cy,&my,"filter"); if (error) return;
	header *result;
	if (ra!=1 || rb!=1 || rx!=1 || ry!=1)
	{
		print("Filter can only work with row vectors.\n");
		error=1; return;
	}
	if (cx<=cb)
	{
		print("Data must be longer then filter.\n");
		error=1; return;
	}
	if (cy<ca-1)
	{
		print("Not enough start values for feedback filter.\n");
		error=1; return;
	}
	if (ca<1 || cb<1)
	{
		print("Filters must have positive length.\n");
		error=1; return;
	}
	if (isreal(hda) && isreal(hdb) && isreal(hdx) && isreal(hdy))
	{
		if (mb[0]==0)
		{
			print("Feedback filter cannot start with 0.\n");
			error=1; return;
		}
		result=new_matrix(1,cx-cb+1,"");
		double *mres=matrixof(result);
		for (int i=0; i<cx-cb+1; i++)
		{
			double s=0;
			for (int j=0; j<cb; j++) s+=mb[j]*mx[cb-1+i-j];
			for (int j=1; j<ca; j++)
			{
				if (i-j>=0) s-=mres[i-j]*ma[j];
				else 
				{
					s-=my[i-j+cy]*ma[j];
				}
			}
			s=s/ma[0];
			mres[i]=s;
		}
	}
	else if (isrealorcomplex(hda) && isrealorcomplex(hdb) && 
		isrealorcomplex(hdx) && isrealorcomplex(hdy))
	{
		double r=getvre(hda,0)*getvre(hda,0)+getvim(hda,0)*getvim(hda,0);
		if (r==0)
		{
			print("Feedback filter cannot start with 0.\n");
			error=1; return;
		}
		result=new_cmatrix(1,cx-cb+1,"");
		double *mres=matrixof(result);
		for (int i=0; i<cx-cb+1; i++)
		{
			double sre=0,sim=0;
			for (int j=0; j<cb; j++) 
			{
				sre+=getvre(hdb,j)*getvre(hdx,cb-1+i-j)-getvim(hdb,j)*getvim(hdx,cb-1+i-j);
				sim+=getvim(hdb,j)*getvre(hdx,cb-1+i-j)+getvre(hdb,j)*getvim(hdx,cb-1+i-j);
			}
			for (int j=1; j<ca; j++)
			{
				if (i-j>=0)
				{
					sre-=getvre(result,i-j)*getvre(hda,j)-getvim(result,i-j)*getvim(hda,j);
					sim-=getvim(result,i-j)*getvre(hda,j)+getvre(result,i-j)*getvim(hda,j);
				}
				else 
				{
					sre-=getvre(hdy,i-j+cy)*getvre(hda,j)-getvim(hdy,i-j+cy)*getvim(hda,j);
					sim-=getvim(hdy,i-j+cy)*getvre(hda,j)+getvre(hdy,i-j+cy)*getvim(hda,j);
				}
			}
			sre=(sre*getvre(hda,0)+sim*getvim(hda,0))/r;
			sim=(sim*getvre(hda,0)-sre*getvim(hda,0))/r;
			mres[2*i]=sre; mres[2*i+1]=sim;
		}
	}
	else
	{
		print("Use filter(b,a,x,y) with real row vectors of proper length!\n");
		error=1; return;
	}
	moveresult(hd,result);
}

void mtotalmax (header *hd)
// overall maximum of a real matrix
{	
	header *st=hd;
	hd=getvalue(hd); if (error) return;
	int c,r;
	double *m;
	if (isreal(hd))
	{	
		getmatrix(hd,&r,&c,&m);
		double max=*m;
		for (int i=0; i<c; i++)
			for (int j=0; j<r; j++)
			{	if (max<*m) max=*m;
				m++;
			}
		header *result=new_real(max,"");
		moveresult(st,result);
	}
	else need_arg_in("totalmax","real matrix");
}

void mtotalmin (header *hd)
// overall minimum of a real matrix
{	header *st=hd;
	hd=getvalue(hd); if (error) return;
	int c,r;
	double *m;
	if (isreal(hd))
	{	getmatrix(hd,&r,&c,&m);
		double min=*m;
		for (int i=0; i<c; i++)
			for (int j=0; j<r; j++)
			{	if (min>*m) min=*m;
				m++;
			}
		header *result=new_real(min,"");
		moveresult(st,result);
	}
	else need_arg_in("totalmin","real matrix");
}

void mnone (header *hd)
// return an empty, non-printing string
{	
	new_none(1);
}

void mscalp (header *hd)
// salar product of two column or row vectors
{	header *st=hd,*hd1,*result;
	double x=0,xi=0,a,b,null=0.0,h;
	double *m,*m1;
	int r,c,r1,c1;
	hd1=nextof(hd);
	hd=getvalue(hd); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	getmatrix(hd,&r,&c,&m,"scalp"); if (error) return;
	getmatrix(hd1,&r1,&c1,&m1,"scalp"); if (error) return;
	if (!((r==1 && r1==1 && c==c1) || (c==1 && c1==1 && r==r1) ||
		(r==1 && c1==1 && c==r1) || (c==1 && r1==1 && r==c1)))
			need_arg_in("scalp","two row or column vectors of equal length");
	int k=r; 
	if (k==1) k=c;
	if (isreal(hd))
	{	if (isreal(hd1))
		{	for (int i=0; i<k; i++) x+=(*m++)*(*m1++);
			result=new_real(x,"");
		}
		else if (iscomplex(hd1))
		{	for (int i=0; i<k; i++)
			{	h=-*(m1+1);
				complex_multiply(m,&null,m1,&h,&a,&b); m++; m1+=2;
				x=x+a; xi=xi+b;
			}
			result=new_complex(x,xi,"");
		}
		else if (isinterval(hd1))
		{	for (int i=0; i<k; i++)
			{	interval_mult(m,m,m1,m1+1,&a,&b); m++; m1+=2;
				interval_add(&x,&xi,&a,&b,&x,&xi);
			}
			result=new_interval(x,xi,"");
		}
		else
		{	output("Cannot combine a real with this data type in scalp!\n");
			error=ERROR_WRONG_ARGUMENT; return;
		}
	}
	else if (iscomplex(hd))
	{	if (isreal(hd1))
		{	for (int i=0; i<k; i++)
			{	complex_multiply(m,m+1,m1,&null,&a,&b); m+=2; m1++;
				x=x+a; xi=xi+b;
			}
			result=new_complex(x,xi,"");
		}
		else if (iscomplex(hd1))
		{	for (int i=0; i<k; i++)
			{	h=-*(m1+1);
				complex_multiply(m,m+1,m1,&h,&a,&b); m+=2; m1+=2;
				x=x+a; xi=xi+b;
			}
			result=new_complex(x,xi,"");
		}
		else
		{	output("Cannot combine a complex with this data type in scalp!\n");
			error=ERROR_WRONG_ARGUMENT; return;
		}
	}
	else if (isinterval(hd))
	{	if (isreal(hd1))
		{	for (int i=0; i<k; i++)
			{	interval_mult(m,m+1,m1,m1,&a,&b); m+=2; m1++;
				interval_add(&x,&xi,&a,&b,&x,&xi);
			}
			result=new_interval(x,xi,"");
		}
		else if (isinterval(hd1))
		{	for (int i=0; i<k; i++)
			{	interval_mult(m,m+1,m1,m1+1,&a,&b); m+=2; m1+=2;
				interval_add(&x,&xi,&a,&b,&x,&xi);
			}
			result=new_interval(x,xi,"");
		}
		else
		{	output("Cannot combine an interval with this data type in scalp!\n");
			error=ERROR_WRONG_ARGUMENT; return;
		}
	}
	else
		need_arg_in("scalp","two row or column vectors of equal length");
	moveresult(st,result);
}

 void magicodd (double *m, int n)
 {
   int n2=n*n;
   int i=0, j=(n-1)/2;

   for (int k=1; k<=n2; k++) 
   {
     m[i*n+j] = k;
     i--;  j++;
     if (k%n==0) 
     { 
       i+=2; j--; 
     }
	 else
     {
       if (j==n) j-=n;
       else if (i<0) i+=n;
     }
   }
 }

void magicfour (double *m, int n)
{
	int *I=(int *)getram(n*n*sizeof(int),"magic");
	if (error) return;
	int *J=(int *)getram(n*n*sizeof(int),"magic");
	if (error) return;

	int index=1;
	for (int i=0; i<n; i++)
		for (int j=0; j<n; j++)
		{
			I[n*i+j]=((i+1)%4)/2;
			J[j*n+i]=((i+1)%4)/2;
			m[i*n+j]=index;
			index++;
		}

	for (int i=0; i<n; i++)
		for (int j=0; j<n; j++)
		{
			if (I[i*n+j]==J[i*n+j])
			m[i*n+j]=n*n+1-m[i*n+j];
		}
}

void magiceven (double *mres, int n)
{
	int m=(n-2)/4;
	int p=2*m+1;

	double *M=(double *)getram(p*p*sizeof(double),"magic");  
	if (error) return;

	magicodd(M,p);
  
	int *J=(int *)getram(p*p*sizeof(int),"magic");
	if (error) return;

	// Conway's LUX method
	int k=0;
	for (int i=0; i<m+1; i++)
	{	
		for (int j=0; j<p; j++) J[k*p+j]=0;
		k++;
	}
	for (int j=0; j<p; j++) J[k*p+j]=1;
	J[k*p+m]=0; J[(k-1)*p+m]=1;
	k++;
	for (int i=0; i<m-1; i++)
	{	
		for (int j=0; j<p; j++) J[k*p+j]=2;
		k++;
	}

 
	for (int i=0; i<p; i++)
	{
		for (int j=0; j<p; j++)
		{
			double h=M[i*p+j];
			switch (J[i*p+j])
			{
				case 0 :
					mres[2*i*n+2*j]=4*h;
					mres[2*i*n+2*j+1]=4*h-3;
					mres[(2*i+1)*n+2*j]=4*h-2;
					mres[(2*i+1)*n+2*j+1]=4*h-1;
				break;
				case 1 :
					mres[2*i*n+2*j]=4*h-3;
					mres[2*i*n+2*j+1]=4*h;
					mres[(2*i+1)*n+2*j]=4*h-2;
					mres[(2*i+1)*n+2*j+1]=4*h-1;
				break;
				case 2 :
					mres[2*i*n+2*j]=4*h-3;
					mres[2*i*n+2*j+1]=4*h;
					mres[(2*i+1)*n+2*j]=4*h-1;
					mres[(2*i+1)*n+2*j+1]=4*h-2;
				break;
			}
		}
	}

}

void mmagic (header *hd)
{
	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real)
		need_arg_in("magic","an integer");
	int n=(int)*realof(hd);
	if (n!=*realof(hd))
		need_arg_in("magic","an integer");
	if (n<3)
		need_arg_in("magic","an integer greater or equal to 3");
	result=new_matrix(n,n,"");
	if (n%2==1) magicodd(matrixof(result),n);
	else if (n%4==0) magicfour(matrixof(result),n);
	else magiceven(matrixof(result),n);
	moveresult(st,result);
}
