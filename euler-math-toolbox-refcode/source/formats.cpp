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

int zerorounding;

extern int outputlength,ioutputlength,iformat,ilinew;

extern double fraceps;

void mzerorounding (header *hd)
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real) need_arg_in("zerorounding","real flag");
	int old=zerorounding;
	if (*realof(hd)) zerorounding=1;
	else zerorounding=0;
	result=new_real(old,"");
	if (error) return;
	moveresult(st,result);
}

int dense=2;

void mdense (header *hd)
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real) need_arg_in("denseoutput","integer 0-10");
	int old=dense;
	dense=(int)(*realof(hd));
	if (dense<0) dense=0;
	if (dense>=10) dense=10;
	result=new_real(old,"");
	if (error) return;
	moveresult(st,result);
}

int cformat=0,cformatn;
double cformatm;

void setmformat (int l, int d)
{
	cformat=1; cformatn=l; cformatm=d;
	if (l<2 || l>80 || d<0 || d>(DBL_DIG+4))
		need_arg_in("format","2<=total<=80, 0<=digits<=20");
	if (d>l-3) d=l-3;
	outputlength=l;
	if (outputlength>64) outputlength=64;
	if (outputlength<1) outputlength=1;
	sprintf(fixedformat,"%%0.%df",d);
	sprintf(expoformat,"%%0.%de",d);
	sprintf(goodformat,"%%0.%dg",d);
	minexpo=1e-26;
	maxexpo=1e20;
	fieldw=l;
	linew=linelength/fieldw;
	if (linew<=0) linew=1;
	if (iformat==0)
	{	ioutputlength=2*outputlength;
		ilinew=linelength/ioutputlength;
		if (ilinew<=0) ilinew=1;
	}
	fraceps=0;
}

void mformat (header *hd)
{	
	header *st=hd,*result;
	static int l=10,d=5;
	int oldl=l,oldd=d;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix || dimsof(hd)->r!=1 || dimsof(hd)->c!=2)
		need_arg_in("format","1x2 vector [total,digits]");
	l=(int)*matrixof(hd); d=(int)*(matrixof(hd)+1);
	setmformat(l,d);
	result=new_matrix(1,2,""); if (error) return;
	*matrixof(result)=oldl;
	*(matrixof(result)+1)=oldd;
	moveresult(st,result);
}

void setgformat (int l, int d)
{
	cformat=2; cformatn=l; cformatm=d;
	if (l>0)
	{
		outputlength=l;
		if (outputlength>64) outputlength=64;
		if (outputlength<1) outputlength=1;
		sprintf(fixedformat,"%%0.%dg",d);
		sprintf(expoformat,"%%0.%dg",d);
		sprintf(goodformat,"%%0.%dg",d);
		minexpo=pow(10.0,-d);
		maxexpo=pow(10.0,l-d-3);
		fieldw=l;
		linew=linelength/fieldw;
		if (linew<=0) linew=1;
		if (iformat==0)
		{	
			ioutputlength=2*outputlength;
			ilinew=linelength/ioutputlength;
			if (ilinew<=0) ilinew=1;
		}
	}
	else
	{
		outputlength=d+8;
		if (outputlength>64) outputlength=64;
		if (outputlength<1) outputlength=1;
		sprintf(fixedformat,"%%0.%dg",d);
		sprintf(expoformat,"%%0.%dg",d);
		sprintf(goodformat,"%%0.%dg",d);
		minexpo=pow(10.0,-14);
		maxexpo=pow(10.0,14);
		fieldw=outputlength;
		linew=linelength/fieldw;
		if (linew<=0) linew=1;
		if (iformat==0)
		{	
			ioutputlength=2*outputlength;
			ilinew=linelength/ioutputlength;
			if (ilinew<=0) ilinew=1;
		}
	}
	fraceps=0;
}
	
void mgformat (header *hd)
{	
	header *st=hd,*result;
	static int l=10,d=5;
	int oldl=l,oldd=d;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_matrix && dimsof(hd)->r==1 && dimsof(hd)->c==2)
	{
		l=(int)*matrixof(hd); d=(int)*(matrixof(hd)+1);
		if (l<0 || l>64 || d<0 || d>20)
			need_arg_in("goodformat","0<=total<=64, 0<=digits<=20");
		if (d>l-3) d=l-3;
		if (d<1) d=1;
		setgformat(l,d);
	}
	else if (hd->type==s_real)
	{
		l=(int)*realof(hd);
		setgformat(0,l);
	}
	else
		need_arg_in("goodformat","integer or 1x2 vector");
	result=new_matrix(1,2,""); if (error) return;
	*matrixof(result)=oldl;
	*(matrixof(result)+1)=oldd;
	moveresult(st,result);
	strcpy(useroutput,"");
}

void seteformat (int l, int d)
{
	cformat=3; cformatn=l; cformatm=d;
	outputlength=l;
	if (outputlength>64) outputlength=64;
	if (outputlength<1) outputlength=1;
	sprintf(fixedformat,"%%0.%de",d);
	sprintf(expoformat,"%%0.%de",d);
	sprintf(goodformat,"%%0.%dg",d);
	minexpo=pow(10.0,-d);
	maxexpo=pow(10.0,l-d-3);
	fieldw=l;
	linew=linelength/fieldw;
	if (linew<=0) linew=1;
	if (iformat==0)
	{	ioutputlength=2*outputlength;
		ilinew=linelength/ioutputlength;
		if (ilinew<=0) ilinew=1;
	}
	fraceps=0;
}

void meformat (header *hd)
{	header *st=hd,*result;
	static int l=10,d=5;
	int oldl=l,oldd=d;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix || dimsof(hd)->r!=1 || dimsof(hd)->c!=2)
		need_arg_in("expformat","1x2 vector");
	l=(int)*matrixof(hd); d=(int)*(matrixof(hd)+1);
	if (l<2 || l>80 || d<0 || d>20)
		need_arg_in("expformat","2<=total<=80, 0<=digits<=20");
	if (d>l-3) d=l-3;
	seteformat(l,d);
	result=new_matrix(1,2,""); if (error) return;
	*matrixof(result)=oldl;
	*(matrixof(result)+1)=oldd;
	moveresult(st,result);
}

void setfformat (int l, int d)
{
	cformat=4; cformatn=l; cformatm=d;
	outputlength=l;
	if (outputlength>64) outputlength=64;
	if (outputlength<1) outputlength=1;
	sprintf(fixedformat,"%%0.%df",d);
	sprintf(expoformat,"%%0.%df",d);
	sprintf(goodformat,"%%0.%dg",d);
	minexpo=pow(10.0,-d);
	maxexpo=pow(10.0,l-d-3);
	fieldw=l;
	linew=linelength/fieldw;
	if (linew<=0) linew=1;
	if (iformat==0)
	{	ioutputlength=2*outputlength;
		ilinew=linelength/ioutputlength;
		if (ilinew<=0) ilinew=1;
	}
	fraceps=0;
}

	void mfformat (header *hd)
{	header *st=hd,*result;
	static int l=10,d=5;
	int oldl=l,oldd=d;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix || dimsof(hd)->r!=1 || dimsof(hd)->c!=2)
		need_arg_in("fixedformat","1x2 vector");
	l=(int)*matrixof(hd); d=(int)*(matrixof(hd)+1);
	if (l<2 || l>80 || d<0 || d>(DBL_DIG+4))
		need_arg_in("fixedformat","2<=total<=80, 0<=digits<=20");
	if (d>l-3) d=l-3;
	setfformat(l,d);
	result=new_matrix(1,2,""); if (error) return;
	*matrixof(result)=oldl;
	*(matrixof(result)+1)=oldd;
	moveresult(st,result);
	strcpy(useroutput,"");
}

void miformat (header *hd)
{	
	header *st=hd,*result;
	int oldi=iformat,k;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real) need_arg_in("iformat","integer");
	k=(int)(*realof(hd));
	if (k>0)
	{   iformat=k;
		if (k>80) k=80;
		if (k<21) k=21;
		ioutputlength=k;
		ilinew=k;
		if (ilinew<=0) ilinew=1;
	}
	else
	{   iformat=0;
		ilinew=linew/2;
		if (ilinew<=0) ilinew=1;
		ioutputlength=outputlength*2;
	}
	result=new_real(oldi,""); if (error) return;
	moveresult(st,result);
	fraceps=0;
}

void setfracformat (int l, double eps)
{
	cformat=5; cformatn=l; cformatm=eps;
	outputlength=l;
	fieldw=l;
	linew=linelength/fieldw;
	if (linew<=0) linew=1;
    fraceps=eps;
}

void mfracformat (header *hd)
{	
	header *st=hd,*result;
	int oldl=outputlength,l;
	double oldeps=fraceps,eps;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix || dimsof(hd)->r!=1 || dimsof(hd)->c!=2)
		need_arg_in("fracformat","1x2 vector");
	l=(int)*matrixof(hd);
	eps=*(matrixof(hd)+1);
	if (l<2 || l>80 || eps<=0 || eps>1)
		need_arg_in("fixedformat","2<=total<=80, 0<eps<=1");
	setfracformat(l,eps);
	result=new_matrix(1,2,""); if (error) return;
	*matrixof(result)=oldl;
	*(matrixof(result)+1)=oldeps;
	moveresult(st,result);
	strcpy(useroutput,"");
}

void mgetformat (header *hd)
{
	header *result=new_matrix(1,9,""); if (error) return;
	double *m=matrixof(result);
	*m++=cformat;
	*m++=cformatn;
	*m++=cformatm;
	*m++=usescalarformat;
	*m++=scalardigits;
	*m++=dense;
	*m++=zerorounding;
	*m++=minexpo;
	*m++=maxexpo;
}

void msetformat (header *hd)
{
	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	result=(header *)newram;
	mgetformat(result);
	if (hd->type!=s_matrix || dimsof(hd)->r!=1 || dimsof(hd)->c!=9)
		need_arg_in("setformat","1x9 vector");
	double *m=matrixof(hd);
	switch ((int)m[0])
	{
		case 1 : setfformat((int)m[1],(int)m[2]); break;
		case 2 : setgformat((int)m[1],(int)m[2]); break;
		case 3 : seteformat((int)m[1],(int)m[2]); break;
		case 4 : setfformat((int)m[1],(int)m[2]); break;
		case 5 : setfracformat((int)m[1],m[2]); break;
	}
	usescalarformat=(int)m[3];
	scalardigits=(int)m[4];
	dense=(int)m[5];
	zerorounding=(int)m[6];
	minexpo=m[7];
	maxexpo=m[8];
	moveresult(st,result);
}

void muserformat (header *hd)
{	
	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string)
		need_arg_in("userformat","string containing function name");
	strcpy(useroutput,stringof(hd));
	outputlength=1;
	result=new_string(useroutput,""); if (error) return;
	moveresult(st,result);
}

void mscalarformat (header *hd)
{
	header *st=hd,*result;
	static int l=10,d=5;
	hd=getvalue(hd); if (error) return;
	int oldusescalarformat=usescalarformat;
	if (hd->type==s_real)
	{
		l=(int)*realof(hd);
		usescalarformat=(l!=0);
	}
	else
		need_arg_in("scalarformat","real");
	result=new_real(oldusescalarformat,""); if (error) return;
	moveresult(st,result);
	strcpy(useroutput,"");
	fraceps=0;
}

void msetscalarformat (header *hd)
{
	header *st=hd,*result;
	static int l=10,d=5;
	hd=getvalue(hd); if (error) return;
	int oldscalarformat=scalardigits;
	if (hd->type==s_real)
	{
		l=(int)*realof(hd);
		if (l<0) l=0;
		scalardigits=l;
		usescalarformat=1;
		sprintf(scalarformat,"%%0.%dg",l);
		if (scalardigits>10)
		{
			scalarmaxexpo=pow(10,scalardigits-2);
			scalarminexpo=1/scalarmaxexpo;
		}
		else
		{
			scalarminexpo=1e-10;
			scalarmaxexpo=1e10;
		}
	}
	else
		need_arg_in("scalarformat","real");
	result=new_real(oldscalarformat,""); if (error) return;
	moveresult(st,result);
	strcpy(useroutput,"");
	fraceps=0;
}

