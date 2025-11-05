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
#include "elementary.h"
#include "getvalue.h"
#include "help.h"
#include "otherfunctions.h"
#include "mainloop.h"
#include "globalvars.h"
#include "udf.h"
#include "list.h"

#include "python.h"

/**
These are the routines to interpret Euler expressions.
The interpreter uses a recursive approach which mimics the
recursive definition of the language.
*/

extern int nosubmref;
extern int trace; // trace flag
extern int reportindex; // flag to check indices

int udfon=0; // running a user defined function
int easyunits=0; // allows 1cm etc.

int actargn=0; // current argument number
int actsp=0; // current semicolon position
int builtinsp=0;

int roundbrackets=0; // allow round brackets v(i)
int relax=0; // relax all restrictions

extern char mxmin1[]; // exernal space for Maxima commands

/********** Symbolic Scanning ******************/

/**
Called to read in a symbolic expression &...
*/
int scan_maxima_expression (char *res, int length)
{
	char *q=res;
	int level=0,clevel=0;
	scan_space();
	if (*next=='\"')
		// We allow the expression in quotes &"..."
	{
		int string = 0;
		next++;
		if (*next=='\"')
			// Allows to pass a string to Maxima &""...""
		{
			*q++='\"'; next++;
			string = 1;
		}
		while (*next!='\"')
		{
			if (!*next)
			{
				output("String not closed.\n");
				error=10; return 0;
			}
			else if (q-res>length-2)
			{
				*res=0; error=10; return 0;
			}
			*q++=*next++;
		}
		next++;
		if (*next=='\"')
		{
			*q++='\"'; next++;
			string = 0;
		}
		if (string) // String for Maxima not closed
		{
			output("&\"\"...\"\" string for Maxima not closed!\n");
			error = 10; return 0;
		}
		*q=0;
	}
	else
	{
		int instring=0;
		while (*next)
		{
			if (*next=='\"') 
			{
				instring=!instring;
			}
			else if (!instring)
			{
				if (*next=='(') level++;
				else if (*next==')') 
				{
					if (level==0) break;
					level--;
					if (level==0 && *(next+1)=='(')
					{
						*q++=*next++; break;
					}
				}
				else if (*next=='[') clevel++;
				else if (*next==']') 
				{
					if (clevel==0) break;
					clevel--;
				}
				else if (strncmp(next,"..",2)==0)
				{
					int oldudf=udf; udf=0; next_simple_line(); udf=oldudf;
					scan_space(); continue;
				}
				else if (q-res>length-2)
				{
					*res=0; error=10; return 0;
				}
				else if ((*next==',' || *next==';' || strncmp(next,"//",2)==0 
						|| strcmp(next,":")==0 || strncmp(next,": // ",5)==0) 
					&& level==0 && clevel==0)
				{
					break;
				}
				else if (*next==2)
				{
					print("Use quotes &\"...\" for symbolic expressions in functions!\n");
					error=10; return 0;
				}
			}
			*q++=*next++;
		}
		*q=0;
		if (instring)
		{
			print("String not closed in symbolic expression!\n");
			error=10; return 0;
		}
		if (level>0 || clevel>0)
		{
			output("Closing bracket missing.\n");
			output1("Found: %s\n",res);
			output1("Brackets: %d (, %d [\n",level,clevel);
			error=10; return 0;
		}
	}

	return (int)strlen(res);
}

/**
Scan a symbolic expression and make a string value.
*/
header *scan_maxima_expression (void)
{
	char res[MAXLINE];
	scan_maxima_expression(res,MAXLINE);
	if (!error)
	{
		header *result=new_string(res,strlen(res)+1,"");
		return result;
	}
	return 0;
}

/**
Scan a Yacas expression and make a string value.
*/
header *scan_yacas (void)
{
	char res[MAXLINE];
	scan_maxima_expression(res,MAXLINE);
	if (!error)
	{
		header *result=new_string(res,strlen(res)+1,"");
		return result;
	}
	return 0;
}

/**
Scan a symbolic expression and interpret it.
*/
header *scan_symbolic (void)
{
	header *result=scan_maxima_expression();
	if (!result) 
	{
		print("Symbolic expression expected.\n");
		error=1;
		return result;
	}
	if (!error) mxm(result);
	if (!error) result->flags|=SYMBOLIC;
	if (strcmp(next,":")==0 || strncmp(next,": // ",5)==0)
	{
		// Insert the Gnu Image to the Notebook (as with the colon after a command line)
		next++;
		newram=(char *)result;
		result=new_none();
		char s[MAXLINE];
		sprintf(s,"%sgnuout.png",getusereulerdir());
		load_image(35,s,0,0,0);
	}
	return result;
}


/***************** Scanning ***************************/

/**
Search for then.
>if logical and logical or logical then ...
*/
void skip_logical (void)
{	
	commandtyp *com;
	int comn;
	int bracket=0;
	while (1)
	{	
		switch (*next)
		{	
			case 0 :
			case 1 :
				return;
			case 2 : // precompiled number
				next+=1+sizeof(double); break;
			case 3 : // precompiled command
				next++;
				memmove((char *)(&comn),next,sizeof(int));
				next+=2*sizeof(int);
				com=command_list+comn;
				if (com->nr==c_then)
				{	
					return;
				}
				break;
			case '\"' :
				while (*next && *next!=1 && *next!='\"') next++;
				if (*next=='\"') next++;
				break;
			case ';' :
			case ',' :
				if (bracket==0) return;
				next++;
				break;
			case '(' :
				bracket++;
				next++;
				break;
			case ')' :
				bracket--;
				next++;
				break;
			case '[' :
				bracket++;
				next++;
				break;
			case ']' :
				bracket--;
				next++;
				break;
			default : 
				if ((*next=='#' && *(next+1)=='#') ||
					(*next=='.' && *(next+1)=='.'))
				{	
					next+=2;
					while (*next!=0 && *next!=1) next++;
				}
				else if (strncmp(next,"then ",5)==0) return;
				else next++;
		}
	}
}

/**
Scan arguments of a function call or a matrix indexing.
bracket: type of bracket.
*/
int scan_arguments (int *scount, char bracket, int &totalcount)
{	
	int count=0,olds=nosubmref,nocount=0;
	header *hd;
	*scount=0;
	totalcount=0;
	nosubmref=1;
	int havesemicolon=0;
	if (bracket==']' && *next==',')
	{
		new_command(c_allv);
		next++;
		count++;
		totalcount++;
	}
	while (1)
	{	
		scan_space(1);
		if (*next==bracket) break;
		if (*next==',') // leave out that parameter
		{	
			hd=new_reference(0,"");
		}
		else // scan one parameter
		{
			if (bracket==')' && *next=='>')
			{
				next++;
				char name[maxname];
				scan_name(name); if (error) return 0;
				hd=new_real(1.0,name);
				nocount=1;
			}
			else if (bracket==')' && *next=='<')
			{
				next++;
				char name[maxname];
				scan_name(name); if (error) return 0;
				hd=new_real(0.0,name);
				nocount=1;
			}
			else if (bracket==')' && *next=='=')
			{
				next++;
				char name[maxname];
				scan_name(name); if (error) return 0;
				header *var=searchvar(name);
				if (!var)
				{
					print("Variable %s not found.\n",name);
					error=1; return 0;
				}
				hd=new_reference(var,name);
				hd->flags|=FLAGVAR;
				nocount=1;
			}
			else
			{
				hd=scan(); 
				if (!hd || error)
				{
					if (*next == ';') output("Closing bracket missing?\n");
					return 0;
				}
				if (!error && newram>(char *)hd && !(hd->flags&ARGS))
				{
					newram=(char *)nextof(hd);
				}
				if (havesemicolon) hd->flags|=SEMICOLON;
				scan_space();
				if (*next=='=' || !strncmp(next,":=",2)) // extra parameter with value
				{	
					if (hd->type!=s_reference)
					{
						print("Cannot use expression here, use name=...\n");
						error=1; return 0;
					}
					int additional=0;
					if (*next=='=') next++;
					else { additional=1; next+=2; }
					char name[maxname];
					strcpy(name,hd->name);
					int xor=hd->xor;
					newram=(char *)hd;
					scan_value();
					if (!error)
					{	
						// create a variable with that value and the name
						strcpy(hd->name,name);
						hd->xor=xor;
						if (additional) hd->flags|=ADDITIONAL;
						nocount=1;
					}
				}
				else if (hd->type==s_reference)
				{
					header *hdvar=hd;
					while (hdvar && hdvar->type==s_reference) hdvar=referenceof(hdvar);
					if (!hdvar)
					{
						header *hdf=searchudf(hd->name);
						if (hdf && hdf->flags&SYMBOLIC)
						{
							char *p=helpof(hdf);
							int nargu=*((int *)p);
							if (nargu>0)
							{
								newram=(char *)hd;
								new_string(hdf->name,"");
							}
						}
						else if (hdf)
						{
							newram=(char *)hd;
							interpret_udf(hdf,hd,0,0);
							if (error) return 0;
						}
						else if (exec_builtin(hd->name,0,hd));
#ifdef DLL
						else if (exec_dll(hd->name,0,hd));
#endif
						else
						{
							print("Variable or function %s not found.\n",hd->name);
							error=1; return 0;
						}
					}
				}
				else if (nocount)
				{	
					output("Illegal parameter after named parameter!\n");
					error=2700;
				}
			}
		}
		if (error)
		{	
			nosubmref=olds; 				
			return 0;
		}
		while (hd<(header *)newram) // in case of multiple returns
		{	
			if (!nocount) count++;
			totalcount++;
			hd=nextof(hd);
		}
		if (count>=50)
		{	
			print("Too many arguments for any function (%d found)!\n",count); 
			error=56; nosubmref=olds; 
			return 0; 
		}
		if (*next==';')
		{	
			if (havesemicolon)
			{	
				output("Only one semicolon allowed in argument list.\n");
				error=2700; nosubmref=olds; 
				return 0;
			}
			*scount=count;
			havesemicolon=1;
		}
		else if (*next!=',') break;
		next++;
		if (*next==']' && bracket==']')
		{
			if (count!=1)
			{
				output(",] only allowed with two indices");
				error=2700; nosubmref=olds; 
				return 0;
			}
			new_command(c_allv);
			count=2;
			totalcount=2;
			*scount=count;
			break;
		}
	}
	if (*next!=bracket)
	{	
		output("Closing bracket missing in function call!\n"); 
		nosubmref=olds;
		error=19; 
		return 0; 
	}
	next++;
	nosubmref=olds;
	if (*scount==count) *scount=0;
	return count;
}

/**
Scan a matrix [...,...;...,...]
The elements are scanned with scan_value().
*/
void scan_matrix (void)
{	
	header *hd,*result;
	dims *d;
	int c,r,count,i,j,complex=0,interval=0,n;
	ULONG ic;
	ULONG allcount;
	double *m,*ms,cnull[2]={0,0},*mr;
	hd=new_matrix(0,0,""); /* don't know how big it will be! */
	hd->size=0;
	if (error) return;
	count=0;
	getmatrix(hd,&r,&c,&m); ms=m;
	r=0; c=0;
	scan_space();
	int utf=0;
	while (1)
	{	
		scan_space();
		int hadbreak=0;
		if (*next==0 && (infile || udfon)) 
		{
			hadbreak=1;
			next_line(); scan_space(); 
		}
		if (*next==';' || *next==']')
			/* new column starts */
		{	
			hadbreak=0;
			if (*next==';') next++;
			if (!freeramfrom(ms,count*(long)(r+1)*sizeof(double)))
				ERET("Memory overflow!\n",18)
			if (count>c) /* this column is to long */
			{	
				if (r>0) /* expand matrix */
				{	for (j=count-1; j>=0; j--)
					{	if (complex)
							copy_complex(cmat(ms,count,r,j),
									cmat(ms,c,r,j));
						else if (interval)
							copy_interval(imat(ms,count,r,j),
									imat(ms,c,r,j));
						else *mat(ms,count,r,j)=*mat(ms,c,r,j);
					}
					for (i=r-1; i>=0; i--)
					{	if (i>0)
						for (j=c-1; j>=0; j--)
						{	if (complex)
								copy_complex(cmat(ms,count,i,j),
									cmat(ms,c,i,j));
							else if (interval)
								copy_interval(imat(ms,count,i,j),
									imat(ms,c,i,j));
							else *mat(ms,count,i,j)=*mat(ms,c,i,j);
						}
						for (j=c; j<count; j++)
						if (complex)
							copy_complex(cmat(ms,count,i,j),cnull);
						else if (interval)
							copy_interval(imat(ms,count,i,j),
								cnull);
						else *mat(ms,count,i,j)=0.0;
					}
				}
				c=count;
			}
			else if (count<c)
			{	for (j=count; j<c; j++)
					if (complex) copy_complex(cmat(ms,c,r,j),
							cnull);
					else if (interval) copy_interval(
							imat(ms,c,r,j),cnull);
					else *mat(ms,c,r,j)=0.0;
			}
			r++; newram=
				(char *)(ms+(complex||interval?2l:1l)*(LONG)c*r);
			m=(double *)newram;
			count=0;
		}
		int hadcomma=(count==0);
		if (*next==']') break;
		if (count>0 && (*next==',' || hadbreak))
		{
			if (*next==',') next++;
			hadcomma=1;
		}
		if (!hadcomma && !udfon && commas && !matlab)
		{	
			output("Need , or ; between matrix elements in strict mode\n");
			error=1; return;
		}
		scan_space();
		if (*next==0 && (infile || udfon)) next_line();
		if (xisalpha(*next)) // skip "x = " for Maxima results
		{
			char *q=next+1;
			while (xisalpha(*q)) q++;
			while (*q==' ') q++;
			if (*q=='=') 
			{
				next=q+1; scan_space();
			}
		}
		result=scan_value(); if (error) return;
		newram=(char *)result;
		if (!complex &&
			(result->type==s_complex || result->type==s_cmatrix))
		{   
			if (interval)
				ERET("Cannot make an interval matrix complex.\n",16)
			complex=1;
			/* make matrix complex up to now (oh boy!) */
			allcount=((char *)m-(char *)ms)/sizeof(double);
			if (!freeram(allcount*sizeof(double)+result->size))
				ERET("Memory overflow!\n",16)
			if (allcount)
			{	memmove(newram+allcount*sizeof(double),newram,result->size);
				result=(header *)((char *)result+allcount*sizeof(double));
				for (ic=allcount-1; ic>0; ic--)
				{   *(ms+(LONG)2*ic)=*(ms+ic);
					*(ms+(LONG)2*ic+1)=0.0;
				}
				*(ms+1)=0.0;
				newram=(char *)(ms+(LONG)2*allcount);
				m=(double *)newram;
			}
			hd->type=s_cmatrix;
		}
		else if (!interval &&
			(result->type==s_interval || result->type==s_imatrix))
		{   
			if (complex)
				ERET("Cannot convert a complex matrix to an interval.\n",16)
			interval=1;
			/* make matrix complex up to now (oh boy!) */
			allcount=((char *)m-(char *)ms)/sizeof(double);
			if (!freeram(allcount*sizeof(double)+result->size))
				ERET("Memory overflow!\n",16)
			if (allcount)
			{	memmove(newram+allcount*sizeof(double),newram,result->size);
				result=(header *)((char *)result+allcount*sizeof(double));
				for (ic=allcount-1; ic>0; ic--)
				{   *(ms+(LONG)2*ic)=*(ms+ic);
					*(ms+(LONG)2*ic+1)=*(ms+ic);
				}
				*(ms+1)=*ms;
				newram=(char *)(ms+(LONG)2*allcount);
				m=(double *)newram;
			}
			hd->type=s_imatrix;
		}
		if (result->type==s_real)
		{	
			if (interval) *m++=*aof(result);
			else *m++=*realof(result);
			count++;
			if (complex) *m++=0.0;
			else if (interval) *m++=*aof(result);
		}
		else if (result->type==s_matrix)
		{	
			if (dimsof(result)->r!=1 || dimsof(result)->c<1)
			{
				if (r!=0) ERET("Illegal matrix element\n",16)
				goto concatmode;
			}
			n=dimsof(result)->c; mr=matrixof(result);
			if (complex)
			{	for (j=0; j<n; j++)
				{	*m++=*mr++;
					*m++=0.0;
					count++;
				}
			}
			else if (interval)
			{	for (j=0; j<n; j++)
				{	*m++=*mr;
					*m++=*mr++;
					count++;
				}
			}
			else
			{	n=dimsof(result)->c; mr=matrixof(result);
				for (j=0; j<n; j++)
				{	*m++=*mr++;
					count++;
				}
			}
		}
		else if (result->type==s_cmatrix)
		{	
			if (dimsof(result)->r!=1 || dimsof(result)->c<1)
			{
				if (r!=0) ERET("Illegal matrix element\n",16)
				goto concatmode;
			}
			n=dimsof(result)->c; mr=matrixof(result);
			if (complex)
			{	for (j=0; j<n; j++)
				{	*m++=*mr++;
					*m++=*mr++;
					count++;
				}
			}
		}
		else if (result->type==s_imatrix)
		{	
			if (dimsof(result)->r!=1 || dimsof(result)->c<1)
			{
				if (r!=0) ERET("Illegal matrix element\n",16)
				goto concatmode;
			}
			n=dimsof(result)->c; mr=matrixof(result);
			if (interval)
			{	for (j=0; j<n; j++)
				{	*m++=*mr++;
					*m++=*mr++;
					count++;
				}
			}
		}
		else if (result->type==s_complex && complex)
		{	*m++=*realof(result);
			*m++=*imagof(result);
			count++;
		}
		else if (result->type==s_interval && interval)
		{	*m++=*aof(result);
			*m++=*bof(result);
			count++;
		}
		else if (result->type==s_string || result->type==s_smatrix)
		{	
			utf|=result->flags&UTF;
			if (r==0 && count==0)
			{	
				newram=(char *)hd;
				char *str=newram;
				if (result->type==s_string)	
				{
					hd=new_string(stringof(result),strlen(stringof(result))+1,"");
					str=stringof(hd);
					str+=strlen(str)+1;
				}
				else
				{
					hd=new_smatrix("");
					char *p=stringof(result);
					str=stringof(hd);
					while (*p!=1)
					{
						strcpy(str,p); str+=strlen(str)+1; p+=strlen(p)+1;
					}
					newram=str;
				}
				while (1)
				{	
					scan_space();
					if (*next==0) { next_line(); scan_space(); }
					if (*next==']') 
					{	
						next++; 
						*str++=1;
						size_t length=str-(char *)hd;
						hd->size=sizeof(header)+((int)(length+1)/ALIGNMENT+1)*ALIGNMENT;
						newram=(char *)nextof(hd);
						hd->type=s_smatrix;
						if (utf) hd->flags|=UTF;
						return; 
					}
					if (*next==',')
					{	
						next++;
						result=scan_value(); if (error) return;
						if (result->type==s_string)	
						{
							utf|=result->flags&UTF;
							strcpy(str,stringof(result));
							str+=strlen(str)+1;
							newram=str;
						}
						else if (result->type==s_smatrix)
						{
							char *p=stringof(result);
							utf|=result->flags&UTF;
							while (*p!=1)
							{
								strcpy(str,p); str+=strlen(str)+1; p+=strlen(p)+1;
							}
							newram=str;
						}
						else
							ERET("Illegal string vector!\n",16);
					}
					else
						ERET("Illegal string vector!\n",16);
				}
			}
			else
				ERET("Number matrix cannot contain a string!\n",16);
		}
		else
			ERET("Illegal vector!\n",16);
		if (count>=INT_MAX)
		{	output1("Matrix has more than %d columns!\n",INT_MAX);
			error=17; return;
		}
		newram=(char *)m;
		if (!freeram(maxname))
			ERET("Memory overflow!\n",16)
	}
	next++;
	d=(dims *)(hd+1);
	if (c==0) r=1;
	d->c=c; d->r=r;
	if (r>=INT_MAX)
	{	output1("Matrix has more than %d rows!\n",INT_MAX);
		error=18; return;
	}
	if (complex) hd->size=cmatrixsize(c,r);
	else if (interval) hd->size=imatrixsize(c,r);
	else hd->size=matrixsize(c,r);
	newram=(char *)hd+hd->size;
	return;
concatmode:
	dimsof(hd)->r=1; dimsof(hd)->c=count;
	if (complex) hd->size=cmatrixsize(count,1);
	else if (interval) hd->size=imatrixsize(count,1);
	else hd->size=matrixsize(count,1);
	newram=(char *)nextof(result);
	mhconcat(hd);
	if (error) return;
	while (*next)
	{
		scan_space();
		if (*next==';') ERET("Semicolon not allowed in matrix of column vectors or matrices.",1);
		if (*next==']') { next++; return; }
		if (commas && *next!=',') ERET("Comma missing.",1);
		if (*next==',') next++;
		scan_space();
		result=scan_value();
		if (error) return;
		mhconcat(hd);
		if (error) return;
	}
	ERET("] missing",1);
}

/**
.digit...
digit...
precompiled double
name{element1}
name(arguments)
name[arguments]
#
+elementary
-elementary
(expression)
[matrix]
"..."
''...''
~value,value~
&symbolic
::symbolic
@:maxima
&:maxima
@value (value must be string)
elementary'
elementary°
elementary%
elementary[arguments] (matrix element)
elementary(arguments) (evaluation)
elementary postfix
elementary infix elementary
*/
int scan_elementary (void)
{	
	double x;
	int nargs,hadargs=0,sp,totalargs;
	header *hd=(header *)newram,*var=0,*hd1;
	char name[maxname],*s;
	scan_space();
	int automult=0;
	int zerobased=0;
	if ((*next=='.' && xisdigit(*(next+1))) || xisdigit(*next))
	{   
		char s[MAXLINE];
		char *sh=s;
		int havedot=0;
		while (*next && sh<s+MAXLINE-4)
		{
			if (*next==hardspace) next++;
			else if (xisdigit(*next)) *sh++=*next++;
			else if (*next=='.' && !isalpha(*(next+1)))
			{
				if (havedot)
				{
					*sh=0;
					print("Error in number %s\n",s);
					error=1; return 0;
				}
				havedot=1; *sh++=*next++;
			}
			else break;
		}
		if (*next=='e' || *next=='E')
		{
			*sh++=*next++;
			if (*next=='-' || *next=='+') *sh++=*next++;
			if (xisdigit(*next))
				while (xisdigit(*next) && sh-s<MAXLINE-2) *sh++=*next++;
			else
			{
				print("Invalid exponential format 'e'.\n");
				error=1; return 0;
			}
		}
		*sh=0;
		sscanf(s,"%lf",&x);
		if (*next=='i' && !xisalpha(*(next+1))) /* complex number! */
		{	next++;
			new_complex(0,x,"");
		}
		else new_real(x,"");
		automult=1;
	}
	else if (*next==2) /* a double stored in binary form */
	{	
		next++;
#ifdef SPECIAL_ALIGNMENT
		memmove((char *)(&x),next,sizeof(double));
#else
		x=*((double *)next);
#endif
		next+=sizeof(double);
		if (*next=='i' && !xisalpha(*(next+1))) /* complex number! */
		{	next++;
			new_complex(0,x,"");
		}
		else new_real(x,"");
		automult=1;
	}
	else if (*next==3) // command stored in binary form
	{	
		commandtyp *p;
		int comn;
		next++;
		memmove((char *)(&comn),next,sizeof(int));
		p=command_list+comn;
		output1("Command name \"%s\" used as variable!\n",p->name);
		error=5000; return 0;
	}
	else if (*next=='u' && *(next+1)=='\"') // unicode string
	{   
		next+=2;
		s=next;
		while (*next!='\"' && *next!=0) next++;
		if (*next!='\"') ERETVAL("\" missing\n",1,0)
		hd=new_string(s,next-s,"");
		next++;
		mutf(hd);
		if (error) return 0;
	}
	else if (xisalpha(*next) || *next=='_' || *next=='%') // name
	{   
		scan_name(name); if (error) return 0;
		int havespace=(*next==' ');
		scan_space(); nargs=0; totalargs=0;
		if (*next=='{')
		{	
			next++; 
			scan(); if (error) return 0; 
			scan_space();
			if (*next==',')
			{
				next++; 
				scan(); if (error) return 0;
				if (*next!='}')
					ERETVAL("} missing!\n",1010,0)
				next++;
				get_element2(name,hd,nextof(hd));
			}
			else
			{
				if (*next!='}')
				ERETVAL("} missing!\n",1010,0)
					next++;
				get_element1(name,hd);
			}
			goto after;
		}
		if (havespace)
			// check for a prefix function if "f " is given
		{
			header *var=searchudf(name);
			if (var && (var->flags&PREFIX)!=0)
			{
				hd=scan(); if (error) return 0;
				newram=(char *)nextof(hd);
				interpret_udf(var,hd,1,0);
				return 1;
			}
			if (var && (var->flags&PREFIXLINE)!=0)
			{
				size_t n=strlen(next);
				hd=new_string(next,n+1,""); if (error) return 0;
				next+=n;
				newram=(char *)nextof(hd);
				interpret_udf(var,hd,1,0);
				return 1;
			}
		}
		if (*next=='(' || *next=='[') /* arguments or indices */
			// scan the argument list
		{	
			// remember bracket type 0=none, 1=(, 2=[
			hadargs=(*next=='[')?2:1;
			next++; 
			// scan all arguments
			nargs=scan_arguments(&sp,hadargs==2?']':')',totalargs);
			if (error) return 0;
		}
		if (name[0]=='_') 
			// search for a builtin command primarily
		{	
			builtinsp=sp;
			if (hadargs==1 && exec_builtin(name+1,totalargs,hd));
#ifdef DLL
			else if (hadargs==1 && exec_dll(name+1,nargs,hd));
#endif
			else
			{	
				function_error(name,totalargs);
				error=1;
				return 0;
			}
            goto after;
		}
		int map=0;
		if (hadargs==2)
			// for [...] look for a variable
		{	
			var=searchvar(name);
			if (var)
			{
				if (var->flags&ZEROBASED) zerobased=1;
				var=getvalue(var);
				if (var->type==s_list && nargs==1)
				{
					getlistelement(var,hd);
					if (error) return 0;
					return 1;
				}
			}
		}
		else if (hadargs==1)
			// for (...) look for a variable to interpret
		{	
			if (strstarts(name,"py$"))
			{
				python_call(name+3,nargs,hd);
				if (error) return 0;
				goto after;
			}
			var=searchvar(name);
			if (var)
				// found a variable
			{	
				if (var->flags&ZEROBASED) zerobased=1;
				var=getvalue(var);
				if (error) return 0;
				// branch to the evaluation of a string
				if (var->type==s_string && !isnone(var)) 
				{
					// print("Evaluate %s\n",stringof(var));
					goto contvar;
				}
				else if (var->type==s_list && nextof(var)>var+1 && (var+1)->type==s_string)
				{
					header *vh=nextof(var+1);
					size_t vhsize=(char *)nextof(var)-(char *)vh;
					if (vhsize>0) memmove(newram,(char *)vh,vhsize);
					newram+=vhsize;
					while (vh<nextof(var) && *(vh->name)==0)
					{
						nargs++;
						vh=nextof(vh);
					}
					sp=0;
					totalargs=nargs;
					var=var+1;
					goto contvar;
				}
			}
			// search for udf
			var=searchudf(name);
			if (!var) 
				// found no updf, search for udf called with ...map
			{	
				var=searchudfmap(name);
				if (var) map=1;
			}
			else
				// remember map flag in udf
			{	
				if (var->flags&MAP) map=1;
			}
			if (!var) 
				// still not found a udf
				var=searchvar(name);
		}
		else // no arguments
		{	
			var=searchvar(name);
			// print("Search %s yields %d\n",name,var!=0);
		}

contvar:
		// we should have a variable now
		// if (var) print("%s %d\n",var->name,var->type);
		if (var && var->type==s_udf && hadargs==1)
			// we had a variable, which is a udf
		{	
			if (map)
			{	
				builtinsp=sp;
				map1f(hd,var->name);
				builtinsp=0;
			}
			else 
			{
				interpret_udf(var,hd,nargs,sp); 
			}
			if (error) return 0;
		}
		else if (!var && hadargs)
			// we did not find any variable but had arguments
			// try builtin stuff and DLL functions
		{	
			builtinsp=sp;
			if (hadargs==1 && exec_builtin(name,totalargs,hd));
#ifdef DLL
			else if (hadargs==1 && exec_dll(name,nargs,hd));
#endif
			else
			{	
				error=24;
				if (hadargs==2)
				{
					if (!var) print("%s is not a variable!\n",name);
					else print("%s is not a variable you can index with []!\n",name);
				}
				else
				{	
					function_error(name,totalargs);
				}
				return 0;
			}
		}
		else if (var && hadargs)
			// we have a variable, but it is no udf, and we have arguments
		{	
			builtinsp=sp;
			// print("searching for %s (found %s) totalargs=%d zerobased=%d\n",name,var->name,totalargs,zerobased);
			if (hadargs==1 && exec_builtin(name,totalargs,hd));
#ifdef DLL
			else if (hadargs==1 && exec_dll(name,nargs,hd));
#endif
			else
			{	
				// print("Get element of for %s with zerobased=%d\n",var->name,zerobased);
				get_element(nargs,var,hd,hadargs,sp,zerobased);
			}
		}
		else hd=new_reference(var,name);
	}
	else if (*next=='#' && *(next+1)!='#')
	{	
		next++; 
		mindex(hd);
	}
	else if (*next=='+')
	{	
		next++;
		automult=scan_elementary();
	}
	else if (*next=='-' && *(next+1)!='>') // minus, but no unit conversion!
	{	next++; automult=scan_elementary();
		if (!error) invert(hd);
	}
	else if (*next=='(')
	{	
		next++;
		scan(1); if (error) return 0;
		scan_space(1);
		if (*next!=')')
		ERETVAL("Closing bracket ) missing!\n",5,0)
		newram=(char *)nextof(hd);
		next++;
		automult=1;
	}
	else if (*next=='[')
	{	
		next++;
		scan_matrix();
	}
	else if (*next=='~' && *(next+1)!='=')
	{	
		next++;
		scan_value();
		if (error) return 0;
		newram=(char *)nextof(hd);
		hd1=(header *)newram;
		scan_space();
		if (*next=='~')
		{	next++;
			minterval1(hd);
		}
		else
		{	if (*next!=',') ERETVAL("\",\" expected!\n",1,0)
			next++;
			scan_value();
			if (error) return 0;
			newram=(char *)nextof(hd1);
			scan_space();
			if (*next!='~') ERETVAL("\"~\" expected!\n",1,0)
			next++;
			minterval(hd);
		}
	}
	else if (*next=='&' && *(next+1)!=':' && *(next+1)!='>')
	{
		next++;
		hd=scan_symbolic();
	}
#ifdef YACAS
	else if (strncmp(next,"&>",2)==0)
	{
		next+=2;
		hd=scan_yacas();
		myacas(hd);
	}
#endif
	else if (*next=='$' && *(next+1)!='>')
	{
		next++;
		hd=scan_symbolic();
		if (!error) hd->flags|=LATEX;
	}
#ifdef YACAS
	else if (strncmp(next,"$>",2)==0)
	{
		next+=2;
		hd=scan_yacas();
		myacas(hd);
		if (!error) hd->flags|=LATEX;
	}
#endif
	else if (strncmp(next,"::",2)==0)
	{
		next+=2;
		hd=scan_symbolic();
	}
	else if (*next=='\"') // string
	{   
		next++;
		s=next;
		while (*next!='\"' && *next!=0) next++;
		hd=new_string(s,next-s,"");
		if (*next!='\"') ERETVAL("\" missing\n",1,0)
		/*
		char *sh=stringof(hd);
		if ((*sh==':' && *(sh+1)==':') ||
			(*sh=='@' && *(sh+1)==':') ||
			(*sh=='&')) mxm(hd);
		*/
		next++;
		if (error) return 0;
	}
	else if (!strncmp(next,"@:",2)) // maxima string
	{	
		next+=2;
		scan_maxima_expression();
		if (error) return 0;
		mxm(hd); if (error) return 0;
		evaluate(hd,nextof(hd),0,0); if (error) return 0;
		moveresult(hd,nextof(hd));
	}
	else if (*next=='@')
	{	
		next++;
		scan_value();
		if (hd->type!=s_string)
		{
			output("@ not followed by a string!\n");
			error=ERROR_ARGUMENT; return 0;
		}
		hd->flags&=!SYMBOLIC;
	}
	else if (!strncmp(next,"&:",2) && *(next+2)!='=')
	{	
		next+=2;
		scan_maxima_expression();
		if (error) return 0;
		mxm(hd); if (error) return 0;
		evaluate(hd,nextof(hd),0,0); if (error) return 0;
		moveresult(hd,nextof(hd));
	}
	else if (!strncmp(next,"\'\'",2))
	{   next+=2;
		s=next;
		while (strncmp(next,"\'\'",2) && *next!=0) next++;
		hd=new_string(s,next-s,"");
		if (*next!='\'' || *(next+1)!='\'')
			ERETVAL("\'\' missing\n",1,0)
		next+=2;
	}
	else 
	{	output1("Syntax error in expression, or unfinished expression!\n"); 
		error=1;
	}
	
	after: if (error) return 0;
	
	/* for things, that come after an elementary expression */
	redo:
	if (*next=='\'') 
	{ 
			next++; transpose(hd); goto redo; 
	}
	else if (*next=='°') 
	{ 
		next++; mdegree(hd); goto redo; 
	}
	else if (*next=='%') 
	{ 
			next++; mpercent(hd); goto redo; 
	}
	else if (*next=='[' || *next=='(')
	{	
		hadargs=(*next=='[')?2:1;
		next++; 
		header *args=nextof(hd);
		nargs=scan_arguments(&sp,hadargs==2?']':')',totalargs);
		if (error) return 0;
		get_element(nargs,hd,args,hadargs,sp,0);
		if (error) return 0;
		args=getvalue(args);
		if (error) return 0;
		moveresult(hd,args);
		goto redo;
	}
	automult=automult&&(!scan_space());
	scan_space();
	if (*next=='!' && *(next+1)!='=')
	{	next++;
		mfak(hd);
		goto redo;
	}
	else if (xisalpha(*next))
	{
		char name[maxname];
		char *p=next;
		scan_name(name); if (error) return 0;
		header *var=searchudf(name);
		if (var && var->flags&INFIX && var->flags&STRONG)
		{
			scan_elementary(); 
			if (error)
			{
				print("Operator %s expects second argument.\n",name);
				return 0;
			}
			interpret_udf(var,hd,2,2);
		}
		else if (var && var->flags&POSTFIX  && var->flags&STRONG)
		{
			interpret_udf(var,hd,1,1);
		}
		else next=p;
	}

	automult=automult && (xisalpha(*next) || *next=='$' || *next=='(');
	scan_space();
	return automult;
}

/*
- factor
elementary ^ elementary
elementary ** elementary
elementary .^ elementary
*/
int scan_factor (void)
{	
	header *hd=(header *)newram;
	scan_space();
	int inv=0;
	while (*next=='-') { next++; inv=!inv; }
	int automult=scan_elementary();
	if (error) return 0;
	if (*next=='^')
	{	
		next++;
		scan_space(1);
		newram=(char *)nextof(hd);
		automult=scan_factor();
		if (error) return 0;
		if (usematlab && !isscalar(hd))
		{
			header *var=searchudf("matrixpower");
			if (!var) { error=1; return 0; }
			interpret_udf(var,hd,2,0);
			return 0;
		}
		else 
		{
			mpower(hd);
		}
	}
	else if (!strncmp(next,"**",2) || !strncmp(next,".^",2))
	{	
		next+=2;
		newram=(char *)nextof(hd);
		scan_space(1);
		automult=scan_factor();
		if (error) return 0;
		mpower(hd);
	}
	if (!error && inv) invert(hd);
	return automult;
}

/**
factor * factor
factor .* factor
factor / factor
factor ./ factor
factor . factor
factor \ factor
*/
void scan_summand (void)
{	
	header *hd=(header *)newram,*hd1;
	scan_space();
	int automult=scan_factor();
	if (error) return;
	while (1)
	{	
		if (*next=='*' && *(next+1)!='*')
		{	
			hd1=nextafter(hd);
			next++;
			scan_space(1);
			automult=scan_factor();
			if (!error) 
			{
				if (usematlab && !(isscalar(hd) || isscalar(hd1))) multiply(hd,hd1);
				else dotmultiply(hd,hd1);
			}
		}
		else if (!strncmp(next,".*",2))
		{	
			hd1=nextafter(hd);
			next+=2;
			scan_space(1);
			automult=scan_factor();
			if (!error) 
			{
				dotmultiply(hd,hd1);
			}
		}
		else if (*next=='/')
		{	
			hd1=nextafter(hd);
			next++;
			scan_space(1);
			int autodiv=scan_factor();
			if (!error) 
			{
				if (usematlab && !(isscalar(hd) || isscalar(hd1)))
				{
					header *var=searchudf("divideinto");
					if (!var) { error=1; break; }
					interpret_udf(var,hd,2,2);
				}
				else
				{
					dotdivide(hd,hd1);
					automult=0;
					if (!error && autodiv && (xisalpha(*next) || *next=='$' || *next=='('))
					{	
						int oldeasyunits=easyunits;
						easyunits=1;
						scan_factor();
						easyunits=oldeasyunits;
						if (!error) dotdivide(hd,hd1);
					}
				}
			}
		}
		else if (!strncmp(next,"./",2))
		{	
			hd1=nextafter(hd);
			next+=2;
			scan_space(1);
			int autodiv=scan_factor();
			if (!error) dotdivide(hd,hd1);
			automult=0;
			if (!error && autodiv && (xisalpha(*next) || *next=='$' || *next=='('))
			{	
				int oldeasyunits=easyunits;
				easyunits=1;
				scan_factor();
				easyunits=oldeasyunits;
				if (!error) dotdivide(hd,hd1);
			}
		}
		else if (*next=='.') 
		{	
			hd1=nextafter(hd);
			next++;
			scan_space(1);
			scan_factor();
			automult=0;
			if (!error) multiply(hd,hd1);
		}
		else if (*next=='\\')
		{	
			hd1=nextafter(hd);
			next++;
			newram=(char *)nextof(hd);
			scan_space(1);
			scan_factor();
			automult=0;
			if (!error) 
			{
				if (usematlab) 
				{
					header *var=searchudf("fit");
					if (!var) { error=1; break; }
					interpret_udf(var,hd,2,2);
				}
				else msolve(hd);
			}
		}
		else if (automult && (xisalpha(*next) || *next=='$' || *next=='('))
		{	
			hd1=nextafter(hd);
			int oldeasyunits=easyunits;
			easyunits=1;
			scan_summand();
			automult=0;
			if (!error) dotmultiply(hd,hd1);
			easyunits=oldeasyunits;
		}
		else break;
		if (error) break;
	}
}

/**
- summand ...
summand + summand ...
summand - summand ...
summand +- summand
*/
void scan_summe (void)
{	header *hd=(header *)newram,*hd1;
	scan_space();
	scan_summand();
	if (error) return;
	while (1)
	{	
		scan_space();
		if (*next=='+')
		{	
			hd1=nextafter(hd);
			next++;
			scan_space(1);
			scan_summand();
			if (!error) add(hd,hd1);
		}
		else if (*next=='-' && *(next+1)!='>')
		{	
			hd1=nextafter(hd);
			next++;
			scan_space(1);
			scan_summand();
			if (!error) subtract(hd,hd1);
		}
		else if (*next==-79) // plusminus
		{	
			hd1=nextafter(hd);
			next++;
			scan_space(1);
			scan_summand();
			if (!error) plusminus(hd,hd1);
		}
		else break;
		if (error) break;
	}
}

/**
:
summe : summe
summe : summe : summ
*/
void scan_dp (void)
{	
	header *hd=(header *)newram,*hd1,*hd2;
	scan_space();
	if (*next==':' && *(next+1)!='=' && *(next+1)!=':')
	{	
		next++;
		new_command(c_allv);
		return;
	}
	scan_summe();
	if (*next==':' && *(next+1)!='=' && *(next+1)!=':' 
		&& !(strcmp(next,":")==0 || strcmp(next,": ")==0 
			|| strncmp(next,": //",3)==0)) /* a vector a:b:c or a:c */
	{	
		next++;
		hd1=(header *)newram; scan_summe();
		if (error) return;
		scan_space();
		if (*next==':' && *(next+1)!='=' && *(next+1)!=':')
		{	
			next++; hd2=(header *)newram; 
			scan_summe(); if (error) return;
		}
		else
		{	
			hd2=hd1; hd1=new_real(1.0,"");
		}
		if (error) return;
		vectorize(hd,hd1,hd2);
	}
}

/**
! compare
not compare
dp < dp
dp <= dp
dp << dp
dp > dp
dp == dp
dp ~= dp
dp != dp
*/
void scan_compare (void)
{	
	header *hd=(header *)newram;
	scan_space();
	if (*next=='!')
	{	
		next++;
		scan_compare(); mnot(hd); return;
	}
	else if (!strncmp(next,"not ",4))
	{	
		next+=4; scan_compare(); if (error) return;
		// test if the result is a real
		header *hd1=getvalue(hd); if (error) return;
		header *result=0;
		if (hd1->type==s_string)
		{
			char *p=stringof(hd1);
			result=new_real((*p!=1 && *p!=0)?0.0:1.0,"");
		}
		else if (hd1->type==s_real)
		{
			// return 0 or 1
			result=new_real(*realof(hd1)!=0.0?0:1,"");
		}
		else need_arg_in("not","real");
		if (error) return;
		moveresult(hd,result);
		return;
	}
	scan_dp(); if (error) return;
	scan_space();
	if (*next=='<')
	{	
		if (*(next+1)!='<')
		{	
			next++;
			newram=(char *)nextof(hd);
			if (*next=='=')
			{	next++; scan_dp(); if (error) return; mlesseq(hd); return;
			}
			else if (*next=='>')
			{	next++; scan_dp(); if (error) return; munequal(hd); return;
			}
			scan_dp(); if (error) return;
			mless(hd);
		}
		else
		{	
			next+=2;
			newram=(char *)nextof(hd);
			if (*next=='=')
			{	next++; scan_dp(); 
				if (error) return; 
				milesseq(hd); 
				return;
			}
			scan_dp(); if (error) return;
			miless(hd);
		}
	}
	else if (*next=='>')
	{	
		next++; 
		newram=(char *)nextof(hd);
		if (*next=='=')
		{	next++; scan_dp(); if (error) return; mgreatereq(hd); return;
		}
		scan_dp(); if (error) return;
		mgreater(hd);
	}
	else if (!strncmp(next,"==",2))
	{	
		next+=2;
		newram=(char *)nextof(hd);
		scan_dp(); if (error) return;
		mequal(hd);
	}
	else if (!strncmp(next,"~=",2) && *(next+2)!='=')
	{	
		next+=2; 
		newram=(char *)nextof(hd);
		scan_dp(); if (error) return;
		maboutequal(hd);
	}
	else if (!strncmp(next,"!=",2))
	{	
		next+=2; 
		newram=(char *)nextof(hd);
		scan_dp(); if (error) return;
		munequal(hd);
	}
}

/**
compare _ compare _ ...
compare | compare | ...
compare _ compare | compare ...
*/
void scan_concat ()
{	
	header *hd=(header *)newram;
	scan_space();
	scan_compare(); if (error) return;
	while (true)
	{	if (*next=='_')
		{	next++;
			newram=(char *)nextof(hd);
			scan_compare(); if (error) return;
			mvconcat(hd);
		}
		else if (*next=='|' && *(next+1)!='|')
		{	next++;
			newram=(char *)nextof(hd);
			scan_compare(); if (error) return;
			mhconcat(hd);
		}
		else break;
	}
}

int ctest (header *hd);

/**
concat && andlogical
*/
void scan_andlogical (void)
{	
	header *hd=(header *)newram;
	scan_concat(); if (error) return;
	scan_space();
	if (!strncmp(next,"&&",2) && strncmp(next,"&&=",3))
	{	
		next+=2;
		newram=(char *)nextof(hd);
		scan_space(1);
		scan_andlogical(); if (error) return;
		mand(hd);
		return;
	}
	else if (!strncmp(next,"and ",4))
	{	
		next+=4;
		int flag=ctest(hd);
		if (error) return;
		if (!flag)
		{	
			skip_logical();
			header *result=new_real(0,"");
			if (error) return;
			moveresult(hd,result);
			return;
		}
		newram=(char *)hd;
		scan_space(1);
		scan_andlogical(); if (error) return;
		flag=ctest(hd);
		if (error) return;
		header *result=new_real(flag,"");
		moveresult(hd,result);
	}
}

/**
andlogical || logical
andlogical or logical
*/
void scan_logical (void)
{	
	header *hd=(header *)newram;
	scan_andlogical(); if (error) return;
	scan_space();
	if (!strncmp(next,"||",2))
	{	
		next+=2;
		scan_space(1);
		newram=(char *)nextof(hd);
		scan_logical(); if (error) return;
		mor(hd);
	}
	else if (!strncmp(next,"or ",3))
	{	
		next+=3;
		// test if the result is a real
		int flag=ctest(hd);
		if (error) return;
		if (flag)
		{
			skip_logical();
			header *result=new_real(1,"");
			if (error) return;
			moveresult(hd,result);
			return;
		}
		newram=(char *)hd;
		scan_space(1);
		scan_logical(); if (error) return;
		flag=ctest(hd);
		if (error) return;
		header *result=new_real(flag,"");
		if (error) return;		
		moveresult(hd,result);
		return;
	}
	else if (xisalpha(*next))
	{
		char name[maxname];
		char *p=next;
		scan_name(name); if (error) return;
		header *var=searchudf(name);
		if (var && var->flags&INFIX && !(var->flags&STRONG))
		{
			scan(); 
			if (error)
			{
				print("Operator %s expects second argument.\n",name);
				return;
			}
			interpret_udf(var,hd,2,2);
		}
		else if (var && var->flags&POSTFIX && !(var->flags&STRONG))
		{
			interpret_udf(var,hd,1,1);
		}
		else next=p;
	}
}

/**
logical
logical -> unit conversion
logical ?
*/
void scan_expression (void)
{
	header *hd=(header *)newram,*hd1;
	scan_space();
	scan_logical();
	if (error) return;
	if (!strncmp(next,"->",2))
		// unit conversion
	{	
		next+=2;
		scan_space();
		hd1=(header *)newram;
		int string=0;
		char k[MAXLINE];
		if (*next=='\"')
		{
			string=1; next++;
			char *p=next;
			while (*p)
			{
				if (*p=='\"')
				{
					break;	
				}
				p++;
			}
			if (*p!='\"' || p-next>MAXLINE-2)
			{
				output("End of string missing!\n");
				error=1;
			}
			strncpy(k,next,p-next);
			k[p-next]=0;
		}
		if (!error)
		{
			int oldeasyunits=easyunits;
			easyunits=1;
			if (*next=='%')
			{ 
				next++; new_real(0.01,""); 
			}
			else if (strncmp(next," %",2)==0) 
			{ 
				next+=2; new_real(0.01,""); 
			}
			else if (*next=='°') 
			{ 
				next++; new_real(M_PI/180,""); 
			}
			else if (strncmp(next," °",2)==0) 
			{ 
				next+=2; new_real(M_PI/180,""); 
			}
			else scan_summand();
			if (!error) dotdivide(hd,hd1);
			easyunits=oldeasyunits;
			if (string)
			{
				if (*next=='\"') next++;
				new_string(k,strlen(k),"");
				mhconcat(hd);
			}
		}
	}
}

extern int usecomplex;

/**
Scan functions, scanning the expression at char *next.
Includes multiple values as in {a,b}.
The function leaves the scanned value (or values) on the stack,
at the current newram position. newram is advanced to the end
of the value (values).
Calls scan_expression() once or several times for {...}.
*/
header *scan (int skiplineends)
{	
	header *result=(header *)newram;
	scan_space();
	if (strncmp(next,"complex:",8)==0)
	{
		next+=8;
		usecomplex=1;
		header *result=scan();
		usecomplex=0;
		return result;
	}
	else if (*next=='{' && *(next+1)=='{')
	{	
		result=scan_value();
	}
	else if (*next=='{')
	{	
		next++; scan_expression(); 
		if (error) goto end;
		while (1)
		{	
			scan_space();
			if (*next=='}') 
			{ 
				next++; 	
				goto end;
			}
			if (*next!=',')	
				ERETVAL("Error in {}!\n",104,result)
			next++; scan_expression();
			if (error) goto end;
		}
	}
	else
	{	
		scan_expression();
	}
end:
	return result;
}

/**
Scan functions with a value as a result. 
Includes multiple values {...}.
*/
header *scan_value (void)
{	
	header *result=(header *)newram,*hd,*hd1,*marker,*nextresult,
		*endresults;
	int oldnosubmref;
	char s[maxname];
	ULONG size,hd1size;
	scan_space();
	if (*next=='{' && *(next+1)=='{')
	{
		next+=2; 
		new_list(""); if (error) return result;
		while (1)
		{	
			scan_space();
			*s=0;
			char *nextold=next;
			scan_name(s,0);
			if (!error)
			{
				scan_space();
				if (*next=='=') next++;
				else 
				{
					next=nextold;
					*s=0;
				}
			}
			error=0;
			hd=(header *)newram;
			scan_value();
			if (error) return result;
			hd1=getvalue(hd); if (error) return result;
			setname(hd1,s);
			if (*next=='}' && *(next+1)=='}') 
			{ 
				next+=2; 
				result->size=newram-(char *)result;
				return result; 
			}
			if (*next!=',')
				ERETVAL("Error in {{}}!\n",104,result)
			next++;
			moveresult(hd,hd1);
		}
	}
	else if (*next=='{')
	{	
		next++; 
		oldnosubmref=nosubmref; nosubmref=1; 
		scan_logical(); nosubmref=oldnosubmref;
		hd1=getvalue(result);
		if (error) return result;
		moveresult(result,hd1);
		while (1)
		{	
			scan_space();
			if (*next=='}') { next++; return result; }
			if (*next!=',')
				ERETVAL("Error in {}!\n",104,result)
			next++; hd=(header *)newram; scan_value();
			if (error) return result;
			hd1=getvalue(hd); if (error) return result;
			moveresult(hd,hd1);
		}
	}
	else
	{	
		scan_logical();
		marker=result;
		if (error) return result;
		endresults=(header *)newram;
		while (marker<endresults)
		{	
			hd1=getvalue(marker);
			if (error) return result;
			nextresult=nextof(marker);
			if (hd1!=marker)
			{   
				if (nextresult==endresults)
				{	
					if (hd1>=nextresult && (char *)hd1<newram)
					{   
						hd1size=newram-(char *)hd1;
						size=(char *)hd1-(char *)marker;
						memmove((char *)marker,(char *)hd1,hd1size);
						newram-=size;
						break;
					}
					else
					{	
						memmove((char *)marker,(char *)hd1,hd1->size);
						newram=(char *)nextof(marker);
						break;
					}
				}
				if (hd1>nextresult && (char *)hd1<newram)
					hd1size=(newram-(char *)hd1);
				else hd1size=hd1->size;
				size=hd1size-marker->size;
				memmove((char *)nextresult+size,(char *)nextresult,
					newram-(char *)nextresult);
				if (hd1>nextresult && (char *)hd1<newram)
					hd1=(header *)((char *)hd1+size);
				nextresult=(header *)((char *)nextresult+size);
				endresults=(header *)((char *)endresults+size);
				memmove((char *)marker,(char *)hd1,hd1size);
				newram=(char *)endresults;
			}
			marker=nextresult;
		}
	}
	setname(result,"");
	return result;
}
