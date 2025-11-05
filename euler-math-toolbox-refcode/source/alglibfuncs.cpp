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
#include "alglibfuncs.h"
#include "mainloop.h"

#include "AlgLib/src/solvers.h"
#include "AlgLib/src/ap.h"
#include "AlgLib/src/linalg.h"
#include "AlgLib/src/integration.h"

void malsolve (header *hd)
{
	header *st=hd,*hda=hd,*hdb,*hdflag;
	hdb=nextof(hda);
	hdflag=nextof(hdb);
	hda=getvalue(hda);
	hdb=getvalue(hdb);
	hdflag=getvalue(hdflag);
	if (error) return;
	if (hda->type!=s_matrix || hdb->type!=s_matrix || hdflag->type!=s_real)
		need_arg_in("alsolve","real square matrix, column vector, accuracy flag");
	double *ma,*mb;
	int ca,ra,cb,rb;
	getmatrix(hda,&ra,&ca,&ma);
	getmatrix(hdb,&rb,&cb,&mb);
	if (ra<1 || ca!=ra || ca!=rb || cb<1)
	{
		print("Need nxn matrix and nxm vector in alsolve, got %dx%d and %dx%d.\n",ra,ca,rb,cb);
		error=1; return;
	}
	header *result,*hdinfo,*hdcondition;
	try
	{
		alglib::real_2d_array A;
		A.setcontent(ra,ca,ma);
		alglib::densesolverreport rep;
		alglib::ae_int_t info;
		if (cb==1)
		{
			alglib::real_1d_array b;
			b.setcontent(rb,mb);
			alglib::real_1d_array cx;
			alglib::rmatrixsolve(A,ra,b,info,rep,cx);
			result=new_matrix(rb,1,""); if (error) return;
			memmove((char *)matrixof(result),(char *)cx.getcontent(),rb*sizeof(double));
		}
		else
		{
			alglib::real_2d_array b;
			b.setcontent(rb,cb,mb);
			alglib::real_2d_array cx;
			alglib::rmatrixsolvem(A,ra,b,cb,*realof(hdflag)!=0,info,rep,cx);
			result=new_matrix(rb,cb,""); if (error) return;
			double *mr=matrixof(result);
			for (int i=0; i<rb; i++)
				memmove((char *)(mr+i*ra),(char *)cx[i],rb*sizeof(double));
		}
		hdinfo=new_real((double)info,""); if (error) return;
		hdcondition=new_real(rep.r1,""); if (error) return;
	}
	catch (alglib::ap_error)
	{
		print("AlgLib solver alsolve returned an error!\n");
		error=1; return;
	}
	moveresult1(st,result);
}

void maleigen (header *hd)
{
	header *st=hd,*hd1,*vres,*wres;
	hd1=nextof(hd);
	hd=getvalue(hd); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	if (hd->type!=s_matrix || hd1->type!=s_real)
		need_arg_in("aleigen","real matrix and flag");
	double *m;
	int r,c;
	getmatrix(hd,&r,&c,&m);
	if (r!=c || r<1)
		need_arg_in("aleigen","real symmetric matrix");
	try
	{
		alglib::real_2d_array A,WL,WR;
		A.setcontent(r,c,m);
		alglib::real_1d_array vr,vi;
		int needw=(*realof(hd)!=0);
		int flag=alglib::rmatrixevd(A,r,needw,vr,vi,WL,WR);
		vres=new_cmatrix(1,r,""); if (error) return;
		double *mr=matrixof(vres);
		for (int i=0; i<r; i++)
		{
			mr[2*i]=vr[i]; mr[2*i+1]=vi[i];
		}
		new_real(flag,"");
		if (needw)
		{
			wres=new_cmatrix(c,c,""); if (error) return;
			mr=matrixof(wres);
			for (int i=0; i<r; i++)
			{
				for (int j=0; j<c; j++)
				{
					if (vi[j]==0)
					{
						*mr++=WR[i][j]; *mr++=0.0;
					}
					else
					{
						*mr++=WR[i][j]; *mr++=WR[i][j+1];
						*mr++=WR[i][j]; *mr++=-WR[i][j+1];
						j++;
					}
				}
			}
		}
	}
	catch (alglib::ap_error)
	{
		print("AlgLib solver aleigen returned an error!\n");
		error=1; return;
	}
	moveresult1(st,vres);
}

header *alf;
header *hdalx;
char *alend;
int alparams;

void alftest (double x, double xma, double xmx, double &y, void *ptr)
{
	header *hd;
	hd=new_real(x,"");
	if (hdalx)
	{
		size_t m=alend-(char *)hdalx;
		memmove(newram,(char *)hdalx,m);
		newram+=m;
	}
	interpret_udf(alf,hd,1+alparams,0);
	if (error || hd->type!=s_real)
	{
		error=1; y=0;
		throw alglib::ap_error("Error in Function Evaluation");
	}
	y=*realof(hd);
	newram=(char *)hd;
}

void malintegrate (header *hd)
{
	header *st=hd,*hd1,*hd2;
	hd1=next_param(hd);
	hd2=next_param(hd1);
	hdalx=next_param(hd2);
	alparams=0;
	if (hdalx)
	{
		header *h=hdalx;
		while (h)
		{
			alparams++;
			h=next_param(h);
		}
	}
	alend=newram;
	if (error) return;
	hd=getvalue(hd);
	hd1=getvalue(hd1);
	hd2=getvalue(hd2);
	if (!hd2)
		need_arg_in("alintegrate","function, a and b");
	if (error) return;
	if (hd->type!=s_string || hd1->type!=s_real || hd2->type!=s_real)
		need_arg_in("alintegrate","function, a and b");
	double a=*realof(hd1),b=*realof(hd2);

	alf=searchudf(stringof(hd));
	if (!alf)
		need_arg_in("alintegrate","function, a and b");

	double y;

	try
	{
		alglib::autogkstate state;
		alglib::autogksmooth(a,b,state);
		alglib::autogkintegrate(state,alftest,0);
		alglib::autogkreport report;
		alglib::autogkresults(state,y,report);
	}
	catch (alglib::ap_error)
	{
		print("AlgLib integration returned an error!\n");
		error=1; return;
	}

	newram=(char *)st;
	new_real(y,"");
}

void malsingular (header *hd)
{
	header *st=hd,*hd1,*hd2,*hd3,*hd4;
	hd1=next_param(hd);
	hd2=next_param(hd1);
	hd3=next_param(hd2);
	hd4=next_param(hd3);
	if (!hd4)
		need_arg_in("alintegrate","function, a and b, alpha and beta");
	hdalx=next_param(hd4);
	alparams=0;
	if (hdalx)
	{
		header *h=hdalx;
		while (h)
		{
			alparams++;
			h=next_param(h);
		}
	}
	alend=newram;
	if (error) return;
	hd=getvalue(hd);
	hd1=getvalue(hd1);
	hd2=getvalue(hd2);
	hd3=getvalue(hd3);
	hd4=getvalue(hd4);
	if (error) return;
	if (hd->type!=s_string || hd1->type!=s_real || hd2->type!=s_real
			|| hd3->type!=s_real || hd4->type!=s_real)
		need_arg_in("alintegrate","function, a and b, alpha and beta");
	double a=*realof(hd1),b=*realof(hd2);
	double alpha=*realof(hd3),beta=*realof(hd4);

	alf=searchudf(stringof(hd));
	if (!alf)
		need_arg_in("alintegrate","function, a and b");

	double y;

	try
	{
		alglib::autogkstate state;
		alglib::autogksingular(a,b,alpha,beta,state);
		alglib::autogkintegrate(state,alftest,0);
		alglib::autogkreport report;
		alglib::autogkresults(state,y,report);
	}
	catch (alglib::ap_error)
	{
		print("AlgLib integration returned an error!\n");
		error=1; return;
	}

	newram=(char *)st;
	new_real(y,"");
}

