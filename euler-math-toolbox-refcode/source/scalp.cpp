#include <float.h>
#include <math.h>
#include <string.h>

#include "interval.h"
#include "express.h"
#include "stack.h"
#include "getvalue.h"

/************************ Exact Scalar Product ******************/

#define LR_SIZE 128
#define LR_BASE 64
#define LR_U_MIN -64
#define LR_E_MAX 63
#define LR_DOUBLE_LENGTH 8

#define error(s) error=1, output(s"\n")

typedef struct
{	int E,U,S;
	unsigned short M[LR_SIZE];
} Accu;

Accu A1,A2,h,g,h1,g1,hh;
int accumode=s_real;

#define accu(a,n) (((a)->M)[LR_BASE+(n)])
#define iszero(a) ((a)->U==(a)->E && accu((a),(a)->U)==0)

#define MI 65536.0

static double EXP2 [17] =
	{1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384.0,
		32768.0,65536.0};

static void truncate (Accu *a)
{   while (accu(a,a->E)==0 && a->E>a->U) a->E--;
	while (accu(a,a->U)==0 && a->E>a->U) a->U++;
	if (a->E==a->U && accu(a,a->E)==0)
	{	a->E=a->U=0; accu(a,0)=0; a->S=1;
	}
}

void accuload (Accu *a, double x)
{   double y;
	int n;
	if (x==0) { a->S=1; a->E=a->U=0; accu(a,0)=0; return; }
	if (x<0) { a->S=-1; x=-x; }
	else a->S=1;
	if (x>1)
	{   x=frexp(x,&n);
		a->E=n/16; n-=a->E*16; x*=EXP2[n];
		a->U=a->E;
		while (x!=0.0)
		{   accu(a,a->U)=(unsigned short)(y=floor(x)); x-=y; a->U--;
			x*=MI;
		}
		a->U++;
		if (a->U<=LR_U_MIN || a->U<a->E-LR_DOUBLE_LENGTH) return;
	}
	else if (x<1)
	{   x=frexp(x,&n);
		a->E=n/16-1; n-=a->E*16; x*=EXP2[n];
		a->U=a->E;
		while (x!=0.0)
		{   accu(a,a->U)=(unsigned short)(y=floor(x)); x-=y; a->U--;
			x*=MI;
		}
		a->U++;
		if (a->U<=LR_U_MIN || a->U<a->E-LR_DOUBLE_LENGTH) return;
	}
	else
	{	accu(a,0)=1; a->E=0; a->U=0;
	}
	truncate(a);
}

#define exp16(n) ldexp(1.0,(n)*16)

double accuget (Accu *a)
{	double y;
	int j;
	y=accu(a,a->E);
	for (j=a->E-1; j>=a->U && j>=a->E-LR_DOUBLE_LENGTH; j--)
	{	y=y*MI+accu(a,j);
	}
	return a->S*y*exp16(j+1);
}

void accucopy (Accu *a, Accu *b)
{   int i;
	a->E=b->E; a->U=b->U; a->S=b->S;
	for (i=a->U; i<=a->E; i++) accu(a,i)=accu(b,i);
}

void accuadd (Accu *a, Accu *b);

void accusub (Accu *a, Accu *b)
{	unsigned long l,l1;
	int i,j;
	int o=0;
	if (iszero(b)) return;
	if (iszero(a)) { accucopy(a,b); a->S=-a->S; return; }
	if (b->S!=a->S)
	{	if (a->S<0) { b->S=-1; accuadd(a,b); b->S=1; return; }
		else { b->S=1; accuadd(a,b); b->S=-1; return; }
	}
	else
	{	if (b->E<a->E)
		{	greater :
			if (b->U<a->U)
			{   for (j=b->U; j<a->U; j++) accu(a,j)=0;
				a->U=b->U;
			}
			i=b->U;
			while (i<=b->E)
			{	l=accu(a,i); l1=(unsigned long)accu(b,i)+o;
				if (l<l1) { l+=65536l; o=1; }
				else o=0;
				accu(a,i)=(unsigned short)(l-l1);
				i++;
			}
			if (o) while (1)
			{	if (accu(a,i)>0) { accu(a,i)--; break; }
				accu(a,i)=65535u;
				i++;
			}
		}
		else if (b->E>a->E)
		{	less :
			i=a->U;
			if (b->U<a->U)
			{	for (j=b->U; j<a->U; j++) accu(a,j)=accu(b,j);
				a->U=b->U;
			}
			while (i<=a->E)
			{	if (i>=b->U) l=accu(b,i);
				else l=0;
				l1=(unsigned long)accu(a,i)+o;
				if (l<l1) { l+=65536l; o=1; }
				else o=0;
				accu(a,i)=(unsigned short)(l-l1);
				i++;
			}
			if (o) while (1)
			{	if (i>=b->U && accu(b,i)>0)
				{	accu(a,i)=accu(b,i)-1; i++; break;
				}
				accu(a,i)=65535u;
				i++;
			}
			for (; i<=b->E; i++) accu(a,i)=accu(b,i);
			a->E=b->E;
			a->S=-a->S;
		}
		else
		{	i=a->E;
			while (i>=a->U)
			{   if (i<b->U) goto greater;
				if (accu(a,i)<accu(b,i)) goto less;
				if (accu(a,i)>accu(b,i)) goto greater;
				i--;
			}
			if (i>=b->U) goto less;
			accu(a,0)=0; a->E=a->U=0; a->S=1;
			return;
		}
	}
	truncate(a);
}

void accuadd (Accu *a, Accu *b)
{	unsigned long l;
	int i,j;
	int o=0;
	if (iszero(b)) return;
	if (iszero(a)) { accucopy(a,b); return; }
	if (b->S==a->S)
	{   if (b->U>a->E)
		{	for (j=a->E+1; j<b->U; j++) accu(a,j)=0;
			a->E=b->U-1;
		}
		i=b->U;
		while (i<=b->E)
		{	if (i<a->U)
			{   accu(a,i)=accu(b,i);
			}
			else
			{	if (i>a->E)
				{   l=(unsigned long)accu(b,i)+o;
					a->E=i;
				}
				else l=((unsigned long)accu(a,i)+accu(b,i))+o;
				accu(a,i)=(unsigned short)(l&65535l);
				if (l>=65536l) o=1;
				else o=0;
			}
			i++;
		}
		while (o)
		{	if (i>LR_E_MAX)
			{	output("Accumulator overflow.\n");
				error=1; return;
			}
			if (i>a->E) { accu(a,i)=1; a->E=i; i++; break; }
			if (i<a->U) { accu(a,i)=1; i++; break; }
			l=(unsigned long)accu(a,i)+1;
			accu(a,i)=(unsigned short)(l&65535l);
			if (l>=65536l) o=1;
			else o=0;
			i++;
		}
		if (i<a->U) for (j=i; j<a->U; j++) accu(a,j)=0;
		if (b->U<a->U) a->U=b->U;
	}
	else if (b->S<0) { b->S=1; accusub(a,b); b->S=-1; return; }
	else if (a->S<0) { b->S=-1; accusub(a,b); b->S=1; return; }
	truncate(a);
}

void accumult (Accu *a, Accu *b)
{   unsigned long l,l1,o1;
	static Accu hm;
	int i,j,k,o,start=1;
	if (iszero(a)) return;
	if (iszero(b)) { accuload(a,0.0); return; }
	for (i=a->U; i<=a->E; i++)
	{	for (o1=0,o=0,j=b->U,k=i+j; j<=b->E; j++,k++)
		{   if (k>LR_E_MAX || k<LR_U_MIN)
			{	output("Accumulator overflow.\n");
				error=1; return;
			}
			l=(unsigned long)accu(a,i)*accu(b,j)+o1;
			l1=(start?0:(unsigned long)accu(&hm,k))+(l&65535l)+o;
			if (l1>=65536l)
			{	accu(&hm,k)=(unsigned short)(l1&65535l); o=1;
			}
			else
			{	accu(&hm,k)=(unsigned short)l1; o=0;
			}
			o1=(l>>16)&65535l;
		}
		start=0;
		if (k>LR_E_MAX || k<LR_U_MIN)
		{	output("Accumulator overflow.\n");
			error=1; return;
		}
		accu(&hm,k)=(unsigned short)(o1+o);
	}
	if (a->S==b->S) a->S=1;
	else a->S=-1;
	a->U=b->U+a->U;
	a->E=b->E+a->E;
	if (accu(&hm,a->E+1)!=0) a->E++;
	for (i=a->U; i<=a->E; i++) accu(a,i)=accu(&hm,i);
	truncate(a);
}

void accuimult (double a, double b, double a1, double b1)
{   if (a>=0)
	{	if (a1>=0)
		{	accuload(&h,a); accuload(&g,a1);
			accuload(&h1,b); accuload(&g1,b1);
		}
		else if (b1>=0)
		{	accuload(&h,b); accuload(&g,a1);
			accuload(&h1,b); accuload(&g1,b1);
		}
		else
		{	accuload(&h,b); accuload(&g,a1);
			accuload(&h1,a); accuload(&g1,b1);
		}
	}
	else if (b>=0)
	{	if (a1>=0)
		{	accuload(&h,a); accuload(&g,b1);
			accuload(&h1,b); accuload(&g1,b1);
		}
		else if (b1>=0)
		{   if (a*b1 <= b*a1)
			{	accuload(&h,a); accuload(&g,b1);
			}
			else
			{	accuload(&h,b); accuload(&g,a1);
			}
			if (b*b1 >= a*a1)
			{	accuload(&h1,b); accuload(&g1,b1);
			}
			else
			{	accuload(&h1,a); accuload(&g1,a1);
			}
		}		else
		{	accuload(&h,b); accuload(&g,a1);
			accuload(&h1,a); accuload(&g1,b1);
		}
	}
	else
	{	if (a1>=0)
		{	accuload(&h,a); accuload(&g,b1);
			accuload(&h1,b); accuload(&g1,a1);
		}
		else if (b1>=0)
		{	accuload(&h,a); accuload(&g,b1);
			accuload(&h1,a); accuload(&g1,a1);
		}
		else
		{	accuload(&h,b); accuload(&g,b1);
			accuload(&h1,a); accuload(&g1,a1);
		}
	}
	accumult(&g,&h); accuadd(&A1,&g); if (error) return;
	accumult(&g1,&h1); accuadd(&A2,&g1);
}

void maccuload (header *hd)
{	header *st=hd,*result;
	int r,c;
	double *m;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{   getmatrix(hd,&r,&c,&m);
		if (r!=1)
		{	output("Accuload needs one or two vectors.\n");
			error=1; return;
		}
		accuload(&A1,*m++);
		accumode=s_real;
		while (c>1)
		{	accuload(&h,*m++); accuadd(&A1,&h); c--;
			if (error) return;
		}
		result=new_real(accuget(&A1),"");
		if (error) return;
		moveresult(st,result);
		return;
	}
	else if (hd->type==s_complex || hd->type==s_cmatrix)
	{   getmatrix(hd,&r,&c,&m);
		if (r!=1)
		{	output("Accuload needs one or two vectors.\n");
			error=1; return;
		}
		accuload(&A1,*m++);
		accuload(&A2,*m++);
		accumode=s_complex;
		while (c>1)
		{	accuload(&h,*m++); accuadd(&A1,&h);
			if (error) return;
			accuload(&h,*m++); accuadd(&A2,&h);
			if (error) return;
			c--;
		}
		result=new_complex(accuget(&A1),accuget(&A2),"");
		if (error) return;
		moveresult(st,result);
		return;
	}
	else if (hd->type==s_interval || hd->type==s_imatrix)
	{   getmatrix(hd,&r,&c,&m);
		if (r!=1)
		{	output("Accuload needs one or two vectors.\n");
			error=1; return;
		}
		accuload(&A1,*m++);
		accuload(&A2,*m++);
		accumode=s_interval;
		while (c>1)
		{	accuload(&h,*m++); accuadd(&A1,&h);
			if (error) return;
			accuload(&h,*m++); accuadd(&A2,&h);
			if (error) return;
			c--;
		}
		result=new_interval(
			round_down(accuget(&A1)),round_up(accuget(&A2)),"");
		if (error) return;
		moveresult(st,result);
		return;
	}
	else error("Wrong argument for accuload.\n");
}

void maccuadd (header *hd)
{	header *st=hd,*result;
	int r,c;
	double *m;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{   getmatrix(hd,&r,&c,&m);
		if (r!=1)
		{	output("Accuload needs one or two vectors.\n");
			error=1; return;
		}
		if (accumode==s_real || accumode==s_complex)
		{	accuload(&h,*m++); accuadd(&A1,&h);
			if (error) return;
		}
		else
		{	accuload(&h,*m++);
			accuadd(&A1,&h); accuadd(&A2,&h);
			if (error) return;
		}
		accumode=s_real;
		while (c>1)
		{	if (accumode==s_real || accumode==s_complex)
			{	accuload(&h,*m++); accuadd(&A1,&h);
				if (error) return;
			}
			else
			{   accuload(&h,*m++);
				accuadd(&A1,&h); accuadd(&A2,&h);
				if (error) return;
			}
			c--;
		}
		result=new_real(accuget(&A1),"");
		if (error) return;
		moveresult(st,result);
		return;
	}
	else if (hd->type==s_complex || hd->type==s_cmatrix)
	{   getmatrix(hd,&r,&c,&m);
		if (r!=1)
		{	output("Accuload needs one or two vectors.\n");
			error=1; return;
		}
		if (accumode==s_interval)
		{	output("Cannot add complex to an interval accu.\n");
			error=1; return;
		}
		if (accumode==s_real)
		{   accuload(&A2,0.0); accumode=s_complex;
		}
		accuload(&h,*m++); accuadd(&A1,&h);
		accuload(&h,*m++); accuadd(&A2,&h);
		if (error) return;
		while (c>1)
		{	accuload(&h,*m++); accuadd(&A1,&h);
			accuload(&h,*m++); accuadd(&A2,&h);
			if (error) return;
			c--;
		}
		result=new_complex(accuget(&A1),accuget(&A2),"");
		if (error) return;
		moveresult(st,result);
		return;
	}
	else if (hd->type==s_interval || hd->type==s_imatrix)
	{   getmatrix(hd,&r,&c,&m);
		if (r!=1)
		{	output("Accuload needs one or two vectors.\n");
			error=1; return;
		}
		if (accumode==s_complex)
		{	output("Cannot add interval to a complex accu.\n");
			error=1; return;
		}
		if (accumode==s_real)
		{   A2=A1; accumode=s_interval;
		}
		accuload(&h,*m++); accuadd(&A1,&h);
		accuload(&h,*m++); accuadd(&A2,&h);
		if (error) return;
		while (c>1)
		{	accuload(&h,*m++); accuadd(&A1,&h);
			accuload(&h,*m++); accuadd(&A2,&h);
			if (error) return;
			c--;
		}
		result=new_interval(
			round_down(accuget(&A1)),round_up(accuget(&A2)),"");
		if (error) return;
		moveresult(st,result);
		return;
	}
	else error("Wrong argument for accuload.\n");
}

void maccuload2 (header *hd)
{	header *st=hd,*hd1,*result;
	int r,c,r1,c1;
	double *m,*m1;
	hd1=nextof(hd);
	equal_params_2(&hd,&hd1); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{   getmatrix(hd,&r,&c,&m);
		getmatrix(hd1,&r1,&c1,&m1);
		if (r!=1 || r1!=1)
		{	output("Accuload needs one or two vectors.\n");
			error=1; return;
		}
		if (c==0 || c!=c1)
		{	output("Wrong vectors in accuload.\n");
			error=1; return;
		}
		accuload(&A1,*m++);
		accuload(&h,*m1++);
		accumult(&A1,&h);
		accumode=s_real;
		if (error) return;
		while (c>1)
		{	accuload(&h,*m++); accuload(&h1,*m1++);
			accumult(&h,&h1);
			accuadd(&A1,&h); c--;
			if (error) return;
		}
		result=new_real(accuget(&A1),"");
		if (error) return;
		moveresult(st,result);
		return;
	}
	else if (hd->type==s_complex || hd->type==s_cmatrix)
	{   getmatrix(hd,&r,&c,&m);
		getmatrix(hd1,&r1,&c1,&m1);
		if (r!=1)		{	output("Accuload needs one or two vectors.\n");
			error=1; return;
		}
		accuload(&h,*m++);
		accuload(&h1,*m++);
		accuload(&g,*m1++);
		accuload(&g1,*m1++);
		accucopy(&hh,&h); accumult(&hh,&g); accucopy(&A1,&hh);
		accucopy(&hh,&h1); accumult(&hh,&g1); accusub(&A1,&hh);
		accucopy(&hh,&h); accumult(&hh,&g1); accucopy(&A2,&hh);
		accucopy(&hh,&h1); accumult(&hh,&g); accuadd(&A2,&hh);
		if (error) return;
		accumode=s_complex;
		while (c>1)
		{	accuload(&h,*m++);
			accuload(&h1,*m++);
			accuload(&g,*m1++);
			accuload(&g1,*m1++);
			accucopy(&hh,&h); accumult(&hh,&g); accuadd(&A1,&hh);
			accucopy(&hh,&h1); accumult(&hh,&g1); accusub(&A1,&hh);
			accucopy(&hh,&h); accumult(&hh,&g1); accuadd(&A2,&hh);
			accucopy(&hh,&h1); accumult(&hh,&g); accuadd(&A2,&hh);
			if (error) return;
			c--;
		}
		result=new_complex(accuget(&A1),accuget(&A2),"");
		if (error) return;
		moveresult(st,result);
		return;
	}
	else if (hd->type==s_interval || hd->type==s_imatrix)
	{   getmatrix(hd,&r,&c,&m);
		getmatrix(hd1,&r1,&c1,&m1);
		if (r!=1)
		{	output("Accuload needs one or two vectors.\n");
			error=1; return;
		}
		accuload(&A1,0);
		accuload(&A2,0);
		accuimult(*m,*(m+1),*m1,*(m1+1)); m+=2; m1+=2;
		if (error) return;
		while (c>1)
		{	accuimult(*m,*(m+1),*m1,*(m1+1)); m+=2; m1+=2;
			if (error) return;
			c--;
		}
		result=new_interval(accuget(&A1),accuget(&A2),"");
		if (error) return;
		moveresult(st,result);
		return;
	}
	else error("Wrong argument for accuload.\n");
}

void maccuadd2 (header *hd)
{	header *st=hd,*hd1,*result;
	int r,c,r1,c1;
	double *m,*m1;
	hd1=nextof(hd);
	equal_params_2(&hd,&hd1); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{   getmatrix(hd,&r,&c,&m);
		getmatrix(hd1,&r1,&c1,&m1);
		if (r!=1 || r1!=1)
		{	output("Accuload needs one or two vectors.\n");
			error=1; return;
		}
		if (c==0 || c!=c1)
		{	output("Wrong vectors in accuload.\n");
			error=1; return;
		}
		accuload(&h,*m++);
		accuload(&h1,*m1++);
		accumult(&h,&h1);
		accuadd(&A1,&h);
		accumode=s_real;
		if (error) return;
		while (c>1)
		{	accuload(&h,*m++); accuload(&h1,*m1++);
			accumult(&h,&h1);
			accuadd(&A1,&h); c--;
			if (error) return;
		}
		result=new_real(accuget(&A1),"");
		if (error) return;
		moveresult(st,result);
		return;
	}
	else if (hd->type==s_complex || hd->type==s_cmatrix)
	{   getmatrix(hd,&r,&c,&m);
		getmatrix(hd1,&r1,&c1,&m1);
		if (r!=1)
		{	output("Accuload needs one or two vectors.\n");
			error=1; return;
		}
		accuload(&h,*m++);
		accuload(&h1,*m++);
		accuload(&g,*m1++);
		accuload(&g1,*m1++);
		accucopy(&hh,&h); accumult(&hh,&g); accuadd(&A1,&hh);
		accucopy(&hh,&h1); accumult(&hh,&g1); accusub(&A1,&hh);
		accucopy(&hh,&h); accumult(&hh,&g1); accuadd(&A2,&hh);
		accucopy(&hh,&h1); accumult(&hh,&g); accuadd(&A2,&hh);
		accumode=s_complex;
		if (error) return;
		while (c>1)		{	accuload(&h,*m++);
			accuload(&h1,*m++);
			accuload(&g,*m1++);
			accuload(&g1,*m1++);
			accucopy(&hh,&h); accumult(&hh,&g); accuadd(&A1,&hh);
			accucopy(&hh,&h1); accumult(&hh,&g1); accusub(&A1,&hh);
			accucopy(&hh,&h); accumult(&hh,&g1); accuadd(&A2,&hh);
			accucopy(&hh,&h1); accumult(&hh,&g); accuadd(&A2,&hh);
			if (error) return;
			c--;
		}
		result=new_complex(accuget(&A1),accuget(&A2),"");
		if (error) return;
		moveresult(st,result);
		return;
	}
	else if (hd->type==s_interval || hd->type==s_imatrix)
	{   getmatrix(hd,&r,&c,&m);
		getmatrix(hd1,&r1,&c1,&m1);
		if (r!=1)
		{	output("Accuload needs one or two vectors.\n");
			error=1; return;
		}
		accuimult(*m,*(m+1),*m1,*(m1+1)); m+=2; m1+=2;
		if (error) return;
		while (c>1)
		{	accuimult(*m,*(m+1),*m1,*(m1+1)); m+=2; m1+=2;
			if (error) return;
			c--;
		}
		result=new_interval(accuget(&A1),accuget(&A2),"");
		if (error) return;
		moveresult(st,result);
		return;
	}
	else error("Wrong argument for accuload.\n");
}

void mresiduum (header *hd)
{	header *st=hd,*hd1,*hd2,*result;
	int i,j,k,c,r,c1,r1,c2,r2,scalar=0;
	double *m,*m1,*m2,*mr;
	hd1=nextof(st); hd2=nextof(hd1);
	equal_params_3(&hd,&hd1,&hd2); if (error) return;
	getmatrix(hd,&r,&c,&m);
	getmatrix(hd1,&r1,&c1,&m1);
	getmatrix(hd2,&r2,&c2,&m2);
	if (r2==1 && c2==1)
	{	scalar=1;
		if (c!=r1)
		{	error("Wrong matrix size for residuum.\n");
			return;
		}
	}
	else if (c1!=c2 || r!=r2 || c!=r1)
	{	error("Wrong matrix size for residuum.\n");
		return;
	}
	if (isreal(hd))
	{	result=new_matrix(r,c1,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
			for (j=0; j<c1; j++)
			{	if (scalar) accuload(&A1,-*m2);
				else accuload(&A1,-*mat(m2,c2,i,j));
				for (k=0; k<c; k++)
				{	accuload(&h,*mat(m,c,i,k));
					accuload(&h1,*mat(m1,c1,k,j));
					accumult(&h,&h1);
					accuadd(&A1,&h);
					if (error) return;
				}
				*mat(mr,c1,i,j)=accuget(&A1);
			}
		moveresult(st,result);
	}
	else if (iscomplex(hd))
	{	result=new_cmatrix(r,c1,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
			for (j=0; j<c1; j++)
			{   if (scalar)
				{	accuload(&A1,-*m2);
					accuload(&A2,-*(m2+1));
				}
				else
				{	accuload(&A1,-*cmat(m2,c2,i,j));
					accuload(&A2,-*(cmat(m2,c2,i,j)+1));
				}
				for (k=0; k<c; k++)
				{	accuload(&h,*cmat(m,c,i,k));
					accuload(&h1,*(cmat(m,c,i,k)+1));
					accuload(&g,*cmat(m1,c1,k,j));
					accuload(&g1,*(cmat(m1,c1,k,j)+1));
					accucopy(&hh,&h); accumult(&hh,&g); accuadd(&A1,&hh);
					accucopy(&hh,&h1); accumult(&hh,&g1); accusub(&A1,&hh);
					accucopy(&hh,&h); accumult(&hh,&g1); accuadd(&A2,&hh);
					accucopy(&hh,&h1); accumult(&hh,&g); accuadd(&A2,&hh);
					if (error) return;
				}
				*cmat(mr,c1,i,j)=accuget(&A1);
				*(cmat(mr,c1,i,j)+1)=accuget(&A2);
			}
		moveresult(st,result);
	}
	else if (isinterval(hd))
	{	result=new_imatrix(r,c1,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
			for (j=0; j<c1; j++)
			{	if (scalar)
				{	accuload(&A2,-*m2);
					accuload(&A1,-*(m2+1));
				}
				else
				{	accuload(&A2,-(*imat(m2,c2,i,j)));
					accuload(&A1,-(*(imat(m2,c2,i,j)+1)));
				}
				for (k=0; k<c; k++)
				{	accuimult(*imat(m,c,i,k),*(imat(m,c,i,k)+1),
						*imat(m1,c1,k,j),*(imat(m1,c1,k,j)+1));
					if (error) return;
				}
				*imat(mr,c1,i,j)=accuget(&A1);
				*(imat(mr,c1,i,j)+1)=accuget(&A2);
			}
		moveresult(st,result);
	}
	else error("Wrong argument for residuum!\n");
}

void accuinit ()
{	accuload(&A1,0.0); accuload(&A2,0.0);
	accumode=s_real;
}

void maccu1 (header *hd)
{	header *result;
	double *mr;
	int i;
	result=new_matrix(1,A1.E-A1.U+1,"");
	mr=matrixof(result);
	for (i=A1.E; i>=A1.U; i--) *mr++=A1.S*accu(&A1,i)*exp16(i);
}

void maccu2 (header *hd)
{	header *result;
	double *mr;
	int i;
	result=new_matrix(1,A2.E-A2.U+1,"");
	if (accumode==s_real)
	{	output("Accu is real.\n");
		error=1; return;
	}
	mr=matrixof(result);
	for (i=A2.E; i>=A2.U; i--) *mr++=A2.S*accu(&A2,i)*exp16(i);
}


