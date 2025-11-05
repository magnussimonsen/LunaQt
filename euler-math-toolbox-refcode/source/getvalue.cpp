#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>

#include "header.h"
#include "sysdep.h"
#include "funcs.h"
#include "interval.h"
#include "builtin.h"
#include "express.h"
#include "stack.h"
#include "spread.h"
#include "builtin.h"
#include "maxima.h"
#include "globalvars.h"
#include "udf.h"

extern int reportindex,nosubmref;

extern char *currentfunction;

header *followrefs (header *hd)
{
	while (hd && hd->type==s_reference)
	{	
		if (hd->flags&GLOBALREF)
			return globalfind(hd->name);
		hd=referenceof(hd);
	}
	return hd;
}

header *getvalue (header *hd)
// get an actual value of a reference
// references to functions with no arguments (e.g. pi) should be executed
// submatrices should be resolved to matrices
{	
	if (error) return 0;

	header *old=hd,*mhd,*result;
	dims *d;
	double *m,*mr,*m1,*m2,*m3;
	int r,c,*rind,*cind,*cind1,i,j;
	// go back the chain of references
	while (hd && hd->type==s_reference)
	{	
		old=hd;
		if (hd->flags&GLOBALREF)
		{
			hd=globalfind(hd->name);
			break;
		}
		hd=referenceof(hd);
	}
	if (!hd) // points nowhere
	{	
		mhd=(header *)newram;
		if (exec_builtin(old->name,0,mhd))
		{   return mhd; // yes, a built-in function
		}
		hd=searchudf(old->name);
		if (hd) // yes, a user defined function
		{	
			if (hd->flags&SYMBOLIC) return new_string(hd->name,strlen(hd->name),"");
			interpret_udf(hd,mhd,0,0);
			return mhd;
		}
		// otherwise, an error
		output1("Variable %s not found!\n",old->name);
		if (udfon)
		{	
			if (!strcmp(currentfunction,"evaluate"))
				output("Use global variables or parameters for string evaluation.\n");
			else
				output1("Use global or local variables defined in function %s.\n",
					currentfunction);
		}
		error=10; 
		return new_string("Error",6,"");
	}
	// resolve sub-matrices
	if (hd->type==s_submatrix)
	{	
		mhd=submrefof(hd); d=submdimsof(hd);
		rind=rowsof(hd); cind=colsof(hd);
		getmatrix(mhd,&r,&c,&m);
		if (d->r==1 && d->c==1)
			return new_real(*mat(m,c,*rind,*cind),"");
		result=new_matrix(d->r,d->c,"");
		if (error) return new_string("Error",6,"");
		mr=matrixof(result);
		for (i=0; i<d->r; i++)
		{	cind1=cind;
			m1=mat(mr,d->c,i,0);
			m2=mat(m,c,*rind,0);
			for (j=0; j<d->c; j++)
			{	m1[j]=m2[*cind1];
				cind1++;
			}
			rind++;
		}
		return result;
	}
	if (hd->type==s_csubmatrix)
	{	
		mhd=submrefof(hd); d=submdimsof(hd);
		rind=rowsof(hd); cind=colsof(hd);
		getmatrix(mhd,&r,&c,&m);
		if (d->r==1 && d->c==1)
		{	m=cmat(m,c,*rind,*cind);
			return new_complex(*m,*(m+1),"");
		}
		result=new_cmatrix(d->r,d->c,"");
		if (error) return new_string("Error",6,"");
		mr=matrixof(result);
		for (i=0; i<d->r; i++)
		{	cind1=cind;
			m1=cmat(mr,d->c,i,0);
			m2=cmat(m,c,*rind,0);
			for (j=0; j<d->c; j++)
			{   m3=m2+(LONG)2*(*cind1);
				*m1++=*m3++; *m1++=*m3;
				cind1++;
			}
			rind++;
		}
		return result;
	}
	if (hd->type==s_isubmatrix)
	{	
		mhd=submrefof(hd); d=submdimsof(hd);
		rind=rowsof(hd); cind=colsof(hd);
		getmatrix(mhd,&r,&c,&m);
		if (d->r==1 && d->c==1)
		{	m=cmat(m,c,*rind,*cind);
			return new_interval(*m,*(m+1),"");
		}
		result=new_imatrix(d->r,d->c,"");
		if (error) return new_string("Error",6,"");
		mr=matrixof(result);
		for (i=0; i<d->r; i++)
		{	cind1=cind;
			m1=imat(mr,d->c,i,0);
			m2=imat(m,c,*rind,0);
			for (j=0; j<d->c; j++)
			{   m3=m2+(LONG)2*(*cind1);
				*m1++=*m3++; *m1++=*m3;
				cind1++;
			}
			rind++;
		}
		return result;
	}
	if (hd->type==s_ssubmatrix)
	{
		char *str=stringelement(referenceof(hd),*indexof(hd));
		result=new_string(str,strlen(str)+1,"");
		if (error) return new_string("Error",6,"");
		if (referenceof(hd)->flags&UTF) result->flags|=UTF;
		return result;

	}
	// make 1x1 matrices scalars
	if (hd->type==s_matrix && dimsof(hd)->c==1 && dimsof(hd)->r==1)
	{	
		return new_real(*matrixof(hd),"");
	}
	if (hd->type==s_cmatrix && dimsof(hd)->c==1 && dimsof(hd)->r==1)
	{	
		return new_complex(*matrixof(hd),*(matrixof(hd)+1),"");
	}

	return hd;
}

header *getvariable (header *hd)
//	get an actual variable of a reference.
{	header *hd1;
	while (hd->type==s_reference)
	{	if ((hd1=referenceof(hd))!=0) hd=hd1;
		else break;
	}
	return hd;
}

header *getvariablesub (header *hd)
/***** getvariable
	get an actual variable of a reference.
*****/
{	header *old=hd;
	while (hd && hd->type==s_reference)
		hd=referenceof(hd);
	if (!hd)
	{	
		output1("Variable %s not found!\n",old->name);
		if (udfon)
		{	if (!strcmp(currentfunction,"evaluate"))
				output("Use global variables or parameters to string evaluation.\n");
			else
				output1("Use global or local variables defined in function %s.\n",
					currentfunction);
		}
		error=10; 
		return new_string("Fehler",6,"");
	}
	if (hd->type==s_submatrix || hd->type==s_csubmatrix ||
			hd->type==s_isubmatrix)
	{	hd=submrefof(hd);
	}
	return hd;
}

#ifdef DLL
int exec_dll (char *name, int n, header *hd);
#endif

extern char *currentfunction;

void evaluate (header *var, header *hd, int nargs, int nsp)
{	
	header *st=hd,*result,*end=(header *)newram;
	char *oldstartlocal,*oldendlocal;
	char *oldnext;
	int oldsearchglobal,oldudfon;
	if (var->type!=s_string) need_arg_in("evaluate","string");
	oldstartlocal=startlocal; oldendlocal=endlocal;
	startlocal=(char *)end;
	if (nsp>0) nargs=nsp;
	char *ps=stringof(var);
	oldnext=next; 
	if (strncmp(ps,"@(",2))
	{	
		int count=0;
		char c[]="x";
		while (hd<end)
		{	
			if (count<nargs && !(hd->flags&SEMICOLON))
				new_reference(hd,c);
			else
				new_reference(hd,hd->name);
			c[0]++;
			if (c[0]>'z') c[0]='a';
			hd=nextof(hd);
			count++;
		}
		next=ps;
	}
	else
	{	
		int count=0;
		char c[maxname];
		next=ps+2;
		while (hd<end)
		{	
			scan_name(c);
			if (error) return;
			if (*next==',') next++;
			if (count<nargs && !(hd->flags&SEMICOLON))
				new_reference(hd,c);
			else
				new_reference(hd,hd->name);
			hd=nextof(hd);
			if (*next==')') { next++; break; }
			if (*next==0) break;
		}
		if (*next==0)
		{ 
			output("Anonymous function needs an expression!\n");
			error=1; return; 
		}
		scan_space();
	}
	endlocal=newram;
	stringon=1;
	oldsearchglobal=searchglobal; searchglobal=1;
	oldudfon=udfon; udfon=3;
	char *oc=currentfunction;
	currentfunction="evaluate";

	result=scan_value();
	
	scan_space();
	if (!error && *next!=0)
	{	output1("Error in Evaluate, superfluous characters found.\n");
		error=1;
	}
	if (error)
	{
		print("Error in expression: %s\n",ps);
	}
	currentfunction=oc;
	udfon=oldudfon;
	next=oldnext;
	startlocal=oldstartlocal; endlocal=oldendlocal;
	searchglobal=oldsearchglobal;
	stringon=0;
    if (error) { result=new_string("Syntax error!",16,""); }
	moveresult(st,result);
}

char *stringelement (header *hd, int i)
{	
	char *str=stringof(hd);
	static char empty[]="\x01";
	if (i<1) goto err;
	for (int k=1; k<i; k++)
	{	if (*str==1) goto err;
		str+=strlen(str)+1;
	}
	if (*str==1) goto err;
	return str;
	err: 
	if (reportindex && (!udfon || !relax))
	{	output1("Index %d in string vector out of bounds!\n",i);
		error=1; return empty;
	}
	else return empty;
}

void get_element (int nargs, header *var, header *hd, int bracket, int nsp, int zerobased)
/***** get_elements
	get the element of the matrix.
	bracket=1 is () and bracket=2 is []
*****/
{	
	header *st=hd,*result,*hd1;
	// print("Sub-matrix of %s with zerobased=%d\n",var->name,zerobased);
	var=getvalue(var); if (error) return;
	if (var->type==s_string) /* interpret the string as a function */
	{	
		if (bracket==2)
		{	
			if (nargs!=1)
			{
				output("Can only use one index for strings or string vectors!\n");
				error=1; return;
			}
			hd=getvalue(hd); if (error) return;
			if (hd->type!=s_real || (int)(*realof(hd)+0.5)!=1)
			{
				print("Index %d out of range for string (need string vector).\n",(int)(*realof(hd)+0.5));
				error=1; return;
			}
			header *result=new_string(stringof(var),strlen(stringof(var)),"");
			moveresult(st,result);
			return;
		}
		hd1=searchudf(stringof(var));
		if (hd1 && (hd1->flags&MAP)!=0) map1f(hd,hd1->name);
		else if (hd1) interpret_udf(hd1,hd,nargs,nsp);
		else if ((hd1=searchudfmap(stringof(var)))!=0) map1f(hd,hd1->name);
		else if (exec_builtin(stringof(var),nargs,hd));
#ifdef DLL
		else if (exec_dll(stringof(var),nargs,hd));
#endif
		else
		{	
			// print("%s %d %d\n",stringof(var),nargs,nsp);
			// printlocalvars((char *)hd);
			evaluate(var,hd,nargs,nsp);
		}
		return;
	}
	if (!relax && brackets && bracket==1 && !usematlab)
	{	
		output("Unexpected \"(\". Index () not allowed in strict mode!\n");
		output("In Euler files, use relax to avoid this.\n");
		error=1; return;
	}
	hd=getvalue(hd); if (error) return;
	if (nargs<1 || nargs>2) 
		ERET("Need one or two indices for a matrix reference!\n",30)
	if (var->type==s_smatrix)
	{	
		if (nargs!=1)
		{	error=31; output("Cannot use a string vector as matrix!\n"); return;
		}
		if (hd->type==s_real)
		{	
			result=new_ssubmatrix(var,((int)(*realof(hd))),"");
			if (error) return;
		}
		else if (hd->type==s_matrix && dimsof(hd)->r==1 && dimsof(hd)->c==1)
		{
			result=new_ssubmatrix(var,(int)(*matrixof(hd)),"");
			if (error) return;			
		}
		else if (hd->type==s_matrix && dimsof(hd)->r==1 && dimsof(hd)->c>1)
		{
			double *mi=matrixof(hd);
			result=new_smatrix("");
			char *pres=stringof(result);
			for (int j=0; j<dimsof(hd)->c; j++)
			{
				char *p=stringelement(var,(int)(*mi));
				if (error) return;
				int ns=(int)strlen(p);
				if (newram+ns+1>ramend)
				{
					output("Out of stack space.\n");
					error=1; return;
				}
				strcpy(pres,p); pres+=ns+1;
				mi++;
			}
			*pres++=1;
			result->size=pres-(char *)result;
			if (var->flags&UTF) result->flags|=UTF;
		}
		else
		{
			output("Cannot generate this type of substring vector.\n");
			error=27; return;
		}
		moveresult(st,result);
		return;
	}
	if (nargs==2) 
	{	hd1=getvalue(next_param(st)); if (error) return;
	}
	else
	{	if (dimsof(var)->r==1) { hd1=hd; hd=new_real(1.0-zerobased,""); }
		else hd1=new_command(c_allv);
		if (error) return;
	}
	if (var->type==s_matrix || var->type==s_real)
	{	
		// print("Creating sub-matrix of %s with zerobased=%d\n",var->name,zerobased);
		result=new_submatrix(var,hd,hd1,"",zerobased);
	}
	else if (var->type==s_cmatrix || var->type==s_complex)
	{	result=new_csubmatrix(var,hd,hd1,"",zerobased);
	}
	else if (var->type==s_imatrix || var->type==s_interval)
	{	result=new_isubmatrix(var,hd,hd1,"",zerobased);
	}
	else
	{	error=31; output1("%s is not a matrix!\n",var->name); return;
	}
	if (error) return;
	moveresult(st,result);
}

void get_element1 (char *name, header *hd)
/* get an element of a matrix, referenced by *realof(hd),
   where the matrix is identified with a vector of same length
*/
{	header *st=hd,*result,*var;
	LONG n,l;
	int r,c;
	double *m;
	hd=getvalue(hd);
	var=searchvar(name);
	if (!var)
	{	output1("%s not a variable!\n",name);
		error=1012; return;
	}
	var=getvalue(var); if (error) return;
	if (hd->type!=s_real)
		ERET("Index must be a number!\n",1013)
	if (error) return;
	l=(LONG)(*realof(hd)+epsilon);
	if (l<1)
	{
		output1("Index %d out of bounds!\n",l);
		error=1; return;
	}
	if (var->type==s_real)
	{	
		result=new_reference(var,"");
	}
	else if (var->type==s_complex)
	{	
		result=new_reference(var,"");
	}
	else if (var->type==s_interval)
	{	
		result=new_reference(var,"");
	}
	else if (var->type==s_string)
	{	
		result=new_reference(var,"");
	}
	else if (var->type==s_matrix)
	{	
		getmatrix(var,&r,&c,&m);
		n=(LONG)r*c;
		if (l>n)
		{
			output1("Index %d out of bounds!\n",l);
			error=1; return;
		}
		if (nosubmref) result=new_real(*(m+l-1),"");
		else result=new_subm(var,l,"");
	}
	else if (var->type==s_cmatrix)
	{	getmatrix(var,&r,&c,&m);
		n=(LONG)r*c;
		if (n==0)
			ERET("Matrix is empty!\n",1030)
		if (l>n)
		{
			output1("Index %d out of bounds!\n",l);
			error=1; return;
		}
		if (nosubmref)
		{   m+=(LONG)2*(l-1);
			result=new_complex(*m,*(m+1),"");
		}
		else result=new_csubm(var,l,"");
	}
	else if (var->type==s_imatrix)
	{	getmatrix(var,&r,&c,&m);
		n=(LONG)r*c;
		if (n==0)
			ERET("Matrix is empty!\n",1030)
		if (l>n)
		{
			output1("Index %d out of bounds!\n",l);
			error=1; return;
		}
		if (nosubmref)
		{   m+=(LONG)2*(l-1);
			result=new_interval(*m,*(m+1),"");
		}
		else result=new_isubm(var,l,"");
	}
	else if (var->type==s_smatrix)
	{
		int sn=smatrixsize(var);
		int i=(int)*realof(hd);
		if (i<=0 || i>sn)
		{
			print("Index %d out of bounds!",i);
			error=1; return;
		}
		char *se=stringelement(var,i);
		result=new_string(se,strlen(se),"");
		if (var->flags&UTF) result->flags|=UTF;
	}
	else if (var->type==s_string)
	{
		result=new_string(stringof(var),strlen(stringof(var)),"");
	}
	else
	{	output1("%s not a variable of proper type for {}!\n",
			name);
		error=1011; return;
	}
	moveresult(st,result);
}

void get_element2 (char *name, header *hd, header *hd1)
/* 
get an element of a matrix, referenced by *realof(hd).
Expects i,j, and uses i=1 resp. j=1, if the matrix is a vector.
*/
{	
	header *st=hd,*result,*var;
	LONG l;
	int r,c,i,j;
	double *m;
	hd=getvalue(hd); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	var=searchvar(name);
	if (!var)
	{	output1("%s not a variable!\n",name);
		error=1012; return;
	}
	var=getvalue(var); if (error) return;
	if (hd->type!=s_real || hd1->type!=s_real)
		ERET("Index must be a number!\n",1013)
	if (error) return;
	if (var->type==s_real)
	{	
		result=new_reference(var,"");
	}
	else if (var->type==s_complex)
	{	
		result=new_reference(var,"");
	}
	else if (var->type==s_interval)
	{	
		result=new_reference(var,"");
	}
	else if (var->type==s_matrix)
	{	
		getmatrix(var,&r,&c,&m);
		i=(int)(*realof(hd)+epsilon);
		j=(int)(*realof(hd1)+epsilon);
		if ((i>r && r!=1) || i<1)
		{	
			output1("Index %d out of bounds!\n",i);
			error=1; return;
		}
		if (r==1) i=1;
		if ((j>c && c!=1) || j<1)
		{	
			output1("Index %d out of bounds!\n",j);
			error=1; return;
		}
		if (c==1) j=1;
		result=new_real(m[c*(i-1)+j-1],"");
	}
	else if (var->type==s_cmatrix)
	{	
		getmatrix(var,&r,&c,&m);
		i=(int)(*realof(hd)+epsilon);
		j=(int)(*realof(hd1)+epsilon);
		if ((i>r && r!=1) || i<1)
		{	
			output1("Index %d out of bounds!\n",i);
			error=1; return;
		}
		if (r==1) i=1;
		if ((j>c && c!=1) || j<1)
		{	
			output1("Index %d out of bounds!\n",j);
			error=1; return;
		}
		if (c==1) j=1;
		l=c*(i-1)+j-1;
		result=new_complex(m[2*l],m[2*l+1],"");
	}
	else if (var->type==s_imatrix)
	{	
		getmatrix(var,&r,&c,&m);
		i=(int)(*realof(hd)+epsilon);
		j=(int)(*realof(hd1)+epsilon);
		if ((i>r && r!=1) || i<1)
		{	
			output1("Index %d out of bounds!\n",i);
			error=1; return;
		}
		if (r==1) i=1;
		if ((j>c && c!=1) || j<1)
		{	
			output1("Index %d out of bounds!\n",j);
			error=1; return;
		}
		if (c==1) j=1;
		l=c*(i-1)+j-1;
		result=new_interval(m[2*l],m[2*l+1],"");
	}
	else
	{	
		output1("%s not a variable of proper type for {}!\n",
			name);
		error=1011; return;
	}
	moveresult(st,result);
}

void direct_reference (header *hd)
{	header *hd1=hd;
	while (hd1 && hd1->type==s_reference)
		hd1=referenceof(hd1);
	if (hd1)
		referenceof(hd)=hd1;
}
