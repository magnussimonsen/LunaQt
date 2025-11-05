#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <float.h>
#include <stdarg.h>

#include "header.h"
#include "sysdep.h"
#include "funcs.h"
#include "graphics.h"
#include "interval.h"
#include "builtin.h"
#include "stack.h"
#include "yacas.h"
#include "maxima.h"
#include "getvalue.h"
#include "express.h"
#include "help.h"
#include "globalvars.h"
#include "assign.h"
#include "loops.h"
#include "aliases.h"
#include "mainloop.h"
#include "maxima.h"
#include "python.h"

char 
	*ramstart, // tobal start of RAM for the Euler stack
	*ramend, // total end of RAM for the Euler stack
	*udfend, // end of user defined functions (starts at ramstart)
	*startlocal, // start of current local variables
	*endlocal, // end of current local variables
	*newram, // start of new ram for new variables on stack
	*startglobal, // remembers the start of the global variables
	*endglobal; // end of the global variables
char *next,*udfline;
char* path[32]={"."};
char* currentdir=".";
int npath=0;

int usematlab=0; // copy of the matlab flag, reset after each command line
int printlargematrices=0; // print larger matrices, default are eclipses

char *s_names[] = {
	"real","complex","real matrix","complex matrix",
	"reference","command","submatrix","complex submatrix","string","function",
	"interval","interval matrix","interval submatrix","string matrix",
	"compress sparse matrix","string subvector","collection","binary"
	};

int ctest (header *hd);

int issymbolic (header *hd)
{
	return hd->type==s_string && hd->flags&SYMBOLIC;
}

char * gettype (header *hd)
{
	if (issymbolic(hd)) return "symbolic expression";
	else return s_names[hd->type];
}

void wrongarg (header *hd) 
{ 
	error=10; 
	if (hd->type==s_string && hd->flags&SYMBOLIC)
	{	
		output1("Wrong argument!\n\n"
			"Cannot combine a symbolic expression here.\n"
			"Did you want to create a symbolic expression?\n"
			"Then start with &.\n\n");
	}
	else
		output1("Wrong argument.\n"
			"Cannot use a %s here.\n\n",s_names[hd->type]); 
}

extern int conditionvectors;

int searchglobal=0;

FILE *metafile=0;

double epsilon,changedepsilon;

int error,quit,surpressed,udf=0,errorout,outputing=1,stringon=0,
	trace=0,traceerrors=0;
char input_line[2*MAXLINE];

long loopindex=0,returnloopindex;

int fieldw=16,linew=5,ilinew=1;
double maxexpo=1.0e6,minexpo=1.0e-5;
int outputlength=14,ioutputlength=42;
char expoformat[16]="%0.12g";
char fixedformat[16]="%0.6g";
char goodformat[16]="%0.12g";
int scalardigits=12,usescalarformat=1;
double scalarminexpo=1e-10,scalarmaxexpo=1e10;
char scalarformat[16]="%12g";
char useroutput[32]="";
int iformat=42;
double fraceps=0;

// **********************************************


int printcomments=1;

int nosubmref=0;

FILE *infile=0,*outfile=0;

header commandheader;

int promptnotebook=1,booktype=0;

extern commandtyp command_list[];

char *outputbuffer=0,*outputbufferend;
int outputbuffererror=0;

int commandtype;

void do_end (void);
void do_loop (void);
void do_repeat (void);
void do_for (void);

extern int runcommands;
extern int stacksize;

size_t size;

extern int second;

int memory_init (void)
/***** memory_init
	get memory for stack.
*****/
{   
	size=(size_t)stacksize*1024*1024;
	ramstart=(char *)malloc(size);

    if (!ramstart)
    {   
		char w[1024];
		if (!second)
		{
			sprintf(w,"Could not allocate %d MB for the stack.\n"
				"Using 1024 MB default size.",stacksize);
			Warning(w,"Euler");
		}
		stacksize=1024;
		size=(size_t)stacksize*1024*1024;
        ramstart=(char *)malloc(size);
    }

	ramend=ramstart+size;

	return ramstart!=0;
}

int skip_space (void)
{	
	int havespace=(*next==' ' || *next==TAB);	
	while (*next==' ' || *next==TAB) next++;
	return havespace;
}

extern int inevaluation;

int scan_space (int skiplineends)
{	
	int havespace=(*next==' ' || *next==TAB || (!udfon && *next=='.' && *(next+1)=='.'));
start: 
	while (*next==' ' || *next==TAB) next++;
	if (udfon!=3 && !inevaluation && *next=='.' && *(next+1)=='.')
		{	next_line(); if (error) return 2; goto start; }
	if (udfon && udfon!=3 && skiplineends && *next==0)
		{	next_line(); goto start; }
	if (!udfon && *next=='/' && *(next+1)=='/')
		{	next+=2; while (*next) next++; }
	if (udfon && *next=='#' && *(next+1)=='#')
		{	next+=2; while (*next) next++; }
	return havespace;
}

void do_endif (void);
void do_else (void);
void do_elseif (void);
void do_if (void);
void do_and (void);
void do_then (void);

void scan_endif (void)
/***** scan_endif
	scan for "endif".
*****/
{	
	commandtyp *com;
	int comn;
	char *oldline=udfline;
	while (1)
	{	switch (*next)
		{	case 1 :
				output("\"endif\" missing!\n");
				error=110; udfline=oldline; return;
			case 0 : 
				if (udfon) { udfline=next+1; next++; }
				else
				{	output("\"endif\" missing!\n");
					error=110; udfline=oldline; return;
				}
				break;
			case 2 : next+=1+sizeof(double); break;
			case 3 : next++;
				memmove((char *)(&comn),next,sizeof(int));
				next+=2*sizeof(int);
				com=command_list+comn;
				if (com->nr==c_endif)
				{	
					if (trace>0) trace_udfline(udfline);
					return;
				}
				else if (com->nr==c_if)
				{	
					scan_endif(); if (error) return; 
				}
				break;
			default : 
				int l;
				if ((*next=='#' && *(next+1)=='#') ||
					(*next=='.' && *(next+1)=='.'))
				{	
					next+=2;
					while (*next!=0 && *next!=1) next++;
				}
				else if (!udfon && xisalpha(*next) && !xisalpha(*(next-1)) &&
					(com=preview_command(&l))!=0)
				{	
					next+=l;
					if (com->nr==c_endif)
					{	
						return;
					}
					else if (com->nr==c_if)
					{	
						scan_endif(); 
						if (error) return; 
					}
				}
				else next++;
		}
	}
}

int scan_else (char *here, int *extra)
/***** scan_else
	scan for "else".
	return 1, if "elseif" or "or" was found.
*****/
{	
	commandtyp *com;
	int comn;
	char *oldline=udfline;
	while (1)
	{	
		switch (*next)
		{	
			case 1 :
				output("\"endif\" missing, searching for \"else\"!\n");
				error=110; udfline=oldline; return 0;
			case 0 : 
				if (udfon) 
				{ 
					udfline=next+1; next++; 
				}
				else
				{
					output("\"endif\" missing, searching for \"else\"!\n");
					error=110; udfline=oldline; return 0;
				}
				break;
			case 2 : next+=1+sizeof(double); break;
			case 3 : next++;
				memmove((char *)(&comn),next,sizeof(int));
				next+=2*sizeof(int);
				com=command_list+comn;
				if (com->nr==c_endif || com->nr==c_else)
				{	
					if (trace>0) trace_udfline(udfline);
					if (extra) *extra=(int)(next-here);
					return 0;
				}
				else if (com->nr==c_elseif)
				{	if (extra) *extra=(int)(next-here);
					return 1;
				}
				else if (com->nr==c_if)
				{	scan_endif(); if (error) return 0; 
				}
				break;
			default : 
				int l;
				if ((*next=='#' && *(next+1)=='#') ||
					(*next=='.' && *(next+1)=='.'))
				{	next+=2;
					while (*next!=0 && *next!=1) next++;
				}
				else if (!udfon && xisalpha(*next) && !xisalpha(*(next-1)) &&
					(com=preview_command(&l))!=0)
				{	
					next+=l;
					if (com->nr==c_endif || com->nr==c_else)
					{	
						return 0;
					}
					else if (com->nr==c_elseif)
					{	
						return 1;
					}
					else if (com->nr==c_if)
					{	
						scan_endif(); if (error) return 0; 
					}
				}
				else next++;
		}
	}
}

int scan_then (void)
/***** scan_then
	scan for "then".
*****/
{	
	commandtyp *com;
	int comn;
	char *oldline=udfline;
	while (1)
	{	switch (*next)
		{	
			case 1 :
				output("\"then\" missing!\n");
				error=110; udfline=oldline; return 0;
			case 0 :
				if (udfon) { udfline=next+1; next++; }
				else
				{	output("\"then\" missing!\n");
					error=110; udfline=oldline; return 0;
				}
				break;
			case 2 : // precompiled number
				next+=1+sizeof(double); break;
			case 3 : // precompiled command
				next++;
				memmove((char *)(&comn),next,sizeof(int));
				next+=2*sizeof(int);
				com=command_list+comn;
				if (com->nr==c_endif || com->nr==c_else)
				{	
					output("\"then\" missing!\n");
					error=110; udfline=oldline; return 0;
				}
				else if (com->nr==c_then)
				{	
					return 1;
				}
				else if (com->nr==c_if)
				{	
					output("\"then\" missing!\n");
					error=110; udfline=oldline; return 0;
				}
				break;
			default : 
				int l;
				if ((*next=='#' && *(next+1)=='#') ||
					(*next=='.' && *(next+1)=='.'))
				{	next+=2;
					while (*next!=0 && *next!=1) next++;
				}
				else if (!udfon && xisalpha(*next) && !xisalpha(*(next-1)) &&
					(com=preview_command(&l))!=0)
				{	next+=l;
					if (com->nr==c_endif || com->nr==c_else)
					{	output("\"then\" missing!\n");
						error=110; udfline=oldline; return 0;
					}
					else if (com->nr==c_then)
					{	return 1;
					}
					else if (com->nr==c_if)
					{	output("\"then\" missing!\n");
						error=110; udfline=oldline; return 0;
					}				
				}
				else next++;
		}
	}
}

void scan_filenamemax (char *name, int lmax)
{	int count=0;
	if (*next=='\"')
	{	
		next++;
		while (*next!='\"' && *next)
		{	*name++=*next++; count++;
			if (count>=lmax-1)
			{	output("Name too long!\n");
				error=11; break;
			}
		}
		if (*next=='\"') next++;
	}
	else if (*next<=32 || *next==',' || *next==';' || isspace(*next))
	{   
		output1("Name expected at: %s!\n",next);
		error=11; *name=0; return;
	}
	else
	{	
		while (!(*next<=32 || *next==',' || *next==';' || isspace(*next)))
		{	
			*name++=*next++; count++;
			if (count>=lmax-1)
			{	output("Name too long!\n");
				error=11; break;
			}
		}
	}
	*name=0;
}

void scan_namemax (char *name, int lmax, int printerror=1)
{	
	int count=0;
	if (*next=='\"')
	{	
		next++;
		while (*next!='\"' && *next)
		{	*name++=*next++; count++;
			if (count>=lmax-1)
			{	
				if (printerror) output("Name too long!\n");
				error=11; break;
			}
		}
		if (*next=='\"') next++;
	}
	else if (!xisalpha(*next) && *next!='_' && *next!='%' && *next!='\\')
	{   
		if (printerror) output1("Name expected at: %s!\n",next);
		error=11; *name=0; return;
	}
	else
	{	
		if (*next=='_' && *(next+1)=='_') { *name++=*next++; next++; count++; }
		if (*next=='_' || *next=='%') { *name++=*next++; count++; }
		while (1)
		{	
			char c=*next;
			if (xisalpha(c) || xisdigit(c) || c=='$')
			{
				*name++=c; next++; count++;
			}
			else if (c=='_' && *(next+1)=='_')
			{
				*name++='_'; next+=2; count++;
			}
			else if (c=='\\' && !xisalpha(*(next+1)) && *(next+1)!='(' && *(next+1)!='[' && *(next+1)!=' ')
			{
				next++; *name++=*next++; count++;
			}
			else break;
			if (count>=lmax-1)
			{	
				if (printerror) output("Name too long!\n");
				error=11; break;
			}
		}
	}
	*name=0;
}

void scan_name (char *name, int printerror)
{	
	scan_namemax (name,maxname,printerror);
}

void getmatrix (header *hd, int *r, int *c, double **m)
/***** getmatrix
	get rows and columns from a matrix.
*****/
{	
	dims *d;
	if (hd->type==s_real || hd->type==s_complex
		|| hd->type==s_interval)
	{	*r=*c=1;
		*m=realof(hd);
	}
	else if (hd->type==s_matrix || hd->type==s_cmatrix
		|| hd->type==s_imatrix)
	{	d=dimsof(hd);
		*m=matrixof(hd);
		*r=d->r; *c=d->c;
	}
	else 
	{	output1("Matrix expected!\n");
		error=1;
	}
}

void getmatrix (header *hd, int *r, int *c, double **m, char *name)
/***** getmatrix
	get rows and columns from a matrix.
*****/
{	dims *d;
	if (hd->type==s_real || hd->type==s_complex
		|| hd->type==s_interval)
	{	*r=*c=1;
		*m=realof(hd);
	}
	else if (hd->type==s_matrix || hd->type==s_cmatrix
		|| hd->type==s_imatrix)
	{	d=dimsof(hd);
		*m=matrixof(hd);
		*r=d->r; *c=d->c;
	}
	else 
	{	output1("Matrix expected in %s!\n",name);
		error=1;
	}
}

/***** searchvar
Search a variable by name.
Return 0, if not found.
*****/
header *searchvar (char *name)
{	
	header *hd;
	int r;
	r=xor(name);

	int lname=(int)strlen(name);

	if (lname>2 && strcmp(name+lname-2,"$$")==0)
	{
		return globalfind(name);
	}

	// search for global variables in global statements
	if (!udfon)
	{
		hd=globalfind(name);
		if (hd) return hd;

	}

	// search through the local variable stack
	hd=(header *)startlocal;
	while ((char *)hd<endlocal)
	{	
		if (r==hd->xor && !strcmp(hd->name,name)) return hd;
		hd=nextof(hd);
	}
	
	// search references to variables.
	// this is active in evaluate(string),
	if (searchglobal)
	{	
		hd=(header *)startlocal;
		while ((char *)hd<endlocal)
		{	
			if (*hd->name==0 && hd->type==s_reference)
			{	
				header *hq=hd;
				while (hq && *hq->name==0 && hq->type==s_reference)
				{	hq=referenceof(hq);
				}
				if (hq && r==hq->xor && !strcmp(hq->name,name)) return hd;
			}
			hd=nextof(hd);
		}
	}

	// in functions, search global variables, if useglobal is present.
	// also searches for variables ending with $ globally.
	int dollar=(name[strlen(name)-1]=='$');
	if (udfon && (searchglobal || dollar))
	{
		hd=globalfind(name);
		if (hd) return hd;
		hd=(header *)udfend;
		while ((char *)hd<endglobal)
		{	
			if (r==hd->xor && !strcmp(hd->name,name)) return hd;
			if (hd==nextof(hd)) break;
			hd=nextof(hd);
		}
	}
	
	// search for global variables with the GLOBAL flag.
	if (udfon)
	{   
		hd=globalfind(name);
		if (hd && hd->flags&GLOBAL) return hd;
		hd=(header *)udfend;
		while ((char *)hd<endglobal)
		{	
			if (r==hd->xor && hd->flags&GLOBAL && !strcmp(hd->name,name)) return hd;
			if (hd==nextof(hd)) break;
			hd=nextof(hd);
		}
	}

	// search for units like cm$, in expressions like 1cm
	if (easyunits && (!udfon || udfon==3) && name[lname-1]!='$')
	{
		char name1[maxname+2];
		strcpy(name1,name);
		strcat(name1,"$");
		return searchvar(name1);
	}



	return 0;
}

void kill_local (char *name)
/***** kill_local
	kill a loal variable name, if there is one.
*****/
{	
	if (!udfon) globalremove(name);
	ULONG size,rest;
	header *hd=(header *)startlocal;
	while ((char *)hd<endlocal)
	{	
		if (!strcmp(hd->name,name)) /* found! */
		{	
			size=hd->size;
			rest=(unsigned long)(newram-(char *)hd-size);
			if (size) memmove((char *)hd,(char *)hd+size,rest);
			endlocal-=size; newram-=size;
			return;
		}
		hd=(header *)((char *)hd+hd->size);
	}
}

/**
get the next parameter on the stack, if hd!=0 and there
is another parameter on the stack.
*/
header *next_param (header *hd)
{	
	if (!hd) return 0;
	hd=(header *)((char *)hd+hd->size);
	if ((char *)hd>=newram) return 0;
	else return hd;
}

/********************* output routines **************************/

char boms[] = {239-256,187-256,191-256,0};

char *addbom (char *str)
{
	char *s=newram;
	strcpy(s,boms);
	strcat(s,str);
	// print("%s\n",newram);
	return newram;
}

double frac (double v, double *n, double *d, double errmax)
{
	int sign=1;
	if (v<0) { sign=-1; v=-v; }
	double p=floor(v),q=1,p0=1,q0=0,b=p;
	double x=v;
	int count=0;
	double e=v-p;
	while (e>0)
	{
		if (e<=errmax) break;
		x=1/(x-b);
		b=floor(x);
		double p1=b*p+p0;
		double q1=q*b+q0;
		if (q1==0) break;
		double e1=fabs(p1/q1-v);
		if (e1>=e) break;
		e=e1;
		p0=p;
		q0=q;
		p=p1;
		q=q1;
		if (e<=errmax) break;
		count++;
		if (count>50) break;
	}
	*n=sign*p; *d=q;
	return 1;
}

int frac_out (double x)
{   
	double n,d;
	if (isnan(x)) output1hold(0,"NAN"); 
	else if (x==0.0) output1hold(0,"0");
	else
	{	
		if (frac(fabs(x),&n,&d,fraceps)<0) return 0;
		if (x<0) output1hold(0,"-");
		else output1hold(0,"");
		if (d>1) output1hold(-1,"%0.0f/%0.0f",n,d);
		else output1hold(-1,"%0.0f",n);
	}
	return 1;
}

int frac_out_string (double x, char *s)
{   
	double n,d;
	if (isnan(x)) strcpy(s,"NAN");
	else if (x==0.0) strcpy(s,"0");
	else
	{	if (frac(fabs(x),&n,&d,fraceps)<0) return 0;
		if (x<0) strcpy(s++,"-");
		else strcpy(s,"");
		if (d>1) sprintf(s,"%0.0f/%0.0f",n,d);
		else sprintf(s,"%0.0f",n);
	}
	return 1;
}

int frac_out0 (double x)
{   
	double n,d;
	if (isnan(x)) output1hold(-1,"NAN"); 
	else if (x==0.0) output1hold(-1,"0");
	else
	{	if (frac(fabs(x),&n,&d,fraceps)<0) return 0;
		if (x<0) output1hold(-1,"-");
		else output1hold(-1,"");
		if (d>1) output1hold(-1,"%0.0f/%0.0f",n,d);
		else output1hold(-1,"%0.0f",n);
	}
	return 1;
}

void tofrac (double x, char *s)
{	
	double n,d;
	if (isnan(x)) { strcpy(s,"NAN"); return; }
	else if (fabs(x)<epsilon) { strcpy(s,"0"); return; }
	if (frac(fabs(x),&n,&d,epsilon*fabs(x))<0)
	{	
		error=1; return;
	}
	if (x<0) strcpy(s,"-");
	else strcpy(s,"");
	if (d>1) sprintf(s+(int)strlen(s),"%0.0f/%0.0f",n,d);
	else sprintf(s+(int)strlen(s),"%0.0f",n);
}

void mfrac (header *hd)
{	
	header *st=hd,*result;
	double *m;
	int r,c;
	hd=getvalue(hd); if (error) return;
	static char s[1044];
	s[0]=0;
	if (hd->type==s_real)
	{	
		tofrac(*realof(hd),s);
	}
	else if (hd->type==s_complex)
	{	
		char *p=s;
		tofrac(*realof(hd),p); p+=(int)strlen(p);
		strcpy(p,"+I*"); p+=(int)strlen(p);
		tofrac(*(realof(hd)+1),p);
	}
	else if (hd->type==s_matrix)
	{	
		getmatrix(hd,&r,&c,&m);
		if (r==1)
		{	char *p=s;
			strcpy(p++,"[");
			for (int i=0; i<c; i++)
			{	
				tofrac(*m++,p); p+=(int)strlen(p);
				if (i<c-1) strcpy(p++,",");
				p+=(int)strlen(p);
				if (p-s>1024)
				{	output("String too long!\n"); error=1;
					return;
				}
			}
			strcpy(p,"]"); p+=(int)strlen(p);
		}
		else
		{	char *p=s;
			strcpy(p,"matrix("); p+=(int)strlen(p);
			for (int j=0; j<r; j++)
			{	strcpy(p++,"[");
				for (int i=0; i<c; i++)
				{	tofrac(*m++,p); p+=(int)strlen(p);
					if (i<c-1) strcpy(p++,",");
					if (p-s>1024)
					{	output("String too long!\n"); error=1;
						return;
					}
				}
				if (j<r-1) strcpy(p,"],");
				else strcpy(p,"]"); 
				p+=(int)strlen(p);
			}
			strcpy(p,")"); p+=(int)strlen(p);
		}
	}
	else if (hd->type==s_cmatrix)
	{	
		getmatrix(hd,&r,&c,&m);
		if (r==1)
		{	char *p=s;
			strcpy(p++,"[");
			for (int i=0; i<c; i++)
			{	tofrac(*m++,p); p+=(int)strlen(p);
				strcpy(p,"+I*"); p+=(int)strlen(p);
				tofrac(*m++,p); p+=(int)strlen(p);
				if (i<c-1) strcpy(p++,",");
				if (p-s>1024)
				{	output("String too long!\n"); error=1;
					return;
				}
			}
			strcpy(p,"]"); p+=(int)strlen(p);
		}
		else
		{	char *p=s;
			strcpy(p,"matrix("); p+=(int)strlen(p);
			for (int j=0; j<r; j++)
			{	strcpy(p++,"[");
				for (int i=0; i<c; i++)
				{	tofrac(*m++,p); p+=(int)strlen(p);
					strcpy(p,"+I*"); p+=(int)strlen(p);
					tofrac(*m++,p); p+=(int)strlen(p);
					if (i<c-1) strcpy(p++,",");
					if (p-s>1024)
					{	output("String too long!\n"); error=1;
						return;
					}
				}
				if (j<r-1) strcpy(p,"],");
				else strcpy(p,"]"); 
				p+=(int)strlen(p);
			}
			strcpy(p,")"); p+=(int)strlen(p);
		}
	}
	else
	{	
		output("Cannot convert this to frac!\n"); error=1;
		return;
	}
	result=new_string(s,(int)(int)strlen(s),"");
	moveresult(st,result);
}

extern int zerorounding,dense;

void double_out (double x)
/***** double_out
	print a double number.
*****/
{   
	if ((int)strlen(useroutput)>0)
	{	
		header *var=searchudf(useroutput);
		if (!var) 
		{	error=1; 
			output("Euler function \"");
			output(useroutput); 
			output("\" not found for user output!"); 
			return; 
		}
		header *hd=(header *)newram;
		new_real(x,"");
		interpret_udf(var,hd,1,1);
		if (hd->type!=s_string || error)
		{	newram=(char *)hd;
			goto one;
		}
		output1hold(0,stringof(hd));
		newram=(char *)hd;
	}
	else if (fraceps>0)
	{	
		if (zerorounding && fabs(x)<1e-12) x=0;
		if (!frac_out(x)) goto one;
	}
	else
	{   
		one :
			if (isnan(x)) output1hold(0,"NAN");
			else
			{
				if (zerorounding && fabs(x)<minexpo) x=0;
				if (x==0.0) output1hold(0,fixedformat,0.0);
				else if (fabs(x)>maxexpo || fabs(x)<minexpo)
					output1hold(0,expoformat,x);
				else output1hold(0,fixedformat,x);
			}
	}
	output1hold(outputlength," ");
}

int block_userformat=0;

void double_out_string (char *s, double x)
/***** double_out
	print a double number to a string.
*****/
{   
	if ((int)strlen(useroutput)>0)
	{	
		if (block_userformat)
		{
			error=1;
			output("User format called recursively.\n");
			return;
		}
		header *var=searchudf(useroutput);
		if (!var) 
		{	
			error=1; 
			output("Euler function \"");
			output(useroutput); 
			output("\" not found for user output!"); 
			return; 
		}
		header *hd=(header *)newram;
		new_real(x,"");
		block_userformat=1;
		interpret_udf(var,hd,1,1);
		block_userformat=0;
		if (hd->type!=s_string || error)
		{	newram=(char *)hd;
			goto one;
		}
		char *p=stringof(hd);
		while (*p==' ') p++;
		strcpy(s,p);
		newram=(char *)hd;
	}
	else if (fraceps>0)
	{	
		if (zerorounding && fabs(x)<1e-12) x=0;
		if (!frac_out_string(x,s)) goto one;
	}
	else
	{   
one :
		char work[128];
		if (isnan(x)) strcpy(work,"NAN");
		else
		{
			if (zerorounding && fabs(x)<minexpo) x=0;
			if (x==0.0) sprintf(work,fixedformat,0.0);
			else if (fabs(x)>maxexpo || fabs(x)<minexpo)
				sprintf(work,expoformat,x);
			else sprintf(work,fixedformat,x);
		}
		char *p=work;
		while (*p==' ') p++;
		strcpy(s,p);
	}
}

void out_matrix (header *hd)
/***** out_matrix
   print a matrix.
*****/
{	
	int c,r,i,j,c0,cend;
	double *m,*x;
	getmatrix(hd,&r,&c,&m);
	if (error) return;
	int lines=0;
	if (r==1 && c==0) output("[]\n");
	else if (c==0 || r==0) output1("Empty matrix of size %dx%d\n",r,c);
	else if (r==1 && dense)
	{	
		int ol=0;
		char work[128];
		char line[MAXLINE];
		*line=0;
		char noteformat[16],noteeformat[16];
		strcpy(noteformat,fixedformat);
		strcpy(noteeformat,expoformat);
		double oldminexpo=minexpo,oldmaxexpo=maxexpo;
		if (c==1 && usescalarformat)
		{
			strcpy(fixedformat,scalarformat);
			strcpy(expoformat,scalarformat);
			minexpo=scalarminexpo; maxexpo=scalarmaxexpo;
		}
		if (c!=1) output("[");
		for (i=0; i<c && !error; i++)
		{	
			double_out_string(work,*m++);
			if (i<c-1) strcat(work,",");
			if ((int)((int)strlen(work)+ol+((ol>0)?dense:0))>linelength) 
			{	
				output(line); output("\n"); ol=0; *line=0; lines++;
			}
			if (lines>=20 && !printlargematrices)
			{
				strcat(line," ... "); ol+=5;
				break;
			}
			if (ol>0) 
			{	
				for (int k=0; k<dense; k++) 
				{	strcat(line," "); ol++; 
				}
			}
			strcat(line,work); ol+=(int)(int)strlen(work);
			if (test_key()==escape) break;
		}
		strcpy(fixedformat,noteformat);
		minexpo=oldminexpo; maxexpo=oldmaxexpo;
		if (ol>0)
		{	
			output(line); 
			if (c!=1) output("]\n"); 
			else output("\n");
			ol=0; *line=0;
		}
	}
	else if (!printlargematrices && (c>linew || r>20))
	{
		print("Real %d x %d matrix\n\n",r,c);
		for (i=0; i<(r>20?20:r) && !error; i++)
		{	
			x=mat(m,c,i,0);
			for (j=0; j<linew-1 && j<c && !error; j++) double_out(*x++);
			if (j!=c) print("    ...\n");
			else print("\n");
		}
		if (r>20) print("    ...\n");
	}
	else 
	{
		for (c0=0; c0<c; c0+=linew)
		{	
			cend=c0+linew-1;
			if (cend>=c) cend=c-1;
			if (c>linew) 
			{	
				if (cend>c0) output1("Column %d to %d:\n",c0+1,cend+1);
				else output1("Column %d:\n",c0+1);
			}
			for (i=0; i<r && !error; i++)
			{	x=mat(m,c,i,c0);
				for (j=c0; j<=cend && !error; j++) double_out(*x++);
				output("\n");
				if (test_key()==escape) return;
			}
		}
	}
}

void complex_out (double x, double y)
/***** complex_out
	print a complex number.
*****/
{	
	if ((int)(int)strlen(useroutput)>0)
	{	
		header *var=searchudf(useroutput);
		if (!var) 
		{	error=1; 
			output("Euler function \"");
			output(useroutput); 
			output("\" not found!"); 
			return; 
		}
		header *hd=(header *)newram;
		new_complex(x,y,"");
		interpret_udf(var,hd,1,1);
		if (hd->type!=s_string || error)
		{	newram=(char *)hd;
			goto one;
		}
		output1hold(0,stringof(hd));
		newram=(char *)hd;
		output1hold(outputlength*2," ");
		return;
	}
	else if (fraceps>0)
	{   if (!frac_out(x)) goto one;
		if (y>=0) output1hold(-1,"+");
		else if (y<0) output1hold(-1,"-");
		if (!frac_out0(fabs(y))) goto two;
	}
	else
	{   
		one :
			if (zerorounding && fabs(x)<1e-12) x=0.0;
			if ((fabs(x)>maxexpo || fabs(x)<minexpo) && x!=0.0)
				output1hold(0,expoformat,x);
			else output1hold(0,fixedformat,x);
			if (zerorounding && fabs(y)<1e-12) 
			{	output1hold(outputlength*2,"+0i ");
				return;
			}
			if (y>=0) output1hold(-1,"+");
			else output1hold(-1,"-");
			y=fabs(y);
		two :
			if ((y>maxexpo || y<minexpo) && y!=0.0)
				output1hold(-1,expoformat,y);
			else 
				output1hold(-1,fixedformat,y);
	}
	output1hold(outputlength*2,"i ");
}

void complex_out_string (double x, double y, char *s)
/***** complex_out
	print a complex number.
*****/
{	
	if ((int)(int)strlen(useroutput)>0)
	{	
		header *var=searchudf(useroutput);
		if (!var) 
		{	error=1; 
			output("Euler function \"");
			output(useroutput); 
			output("\" not found!"); 
			return; 
		}
		header *hd=(header *)newram;
		new_complex(x,y,"");
		interpret_udf(var,hd,1,1);
		if (hd->type!=s_string || error)
		{	newram=(char *)hd;
			goto one;
		}
		strcpy(s,stringof(hd));
		newram=(char *)hd;
		return;
	}
	else if (fraceps>0)
	{   
		if (!frac_out_string(x,s)) goto one;
		if (y>=0) strcat(s,"+");
		else if (y<0) strcat(s,"-");
		if (!frac_out_string(fabs(y),s+(int)(int)strlen(s))) goto two;
	}
	else
	{   
		one :
			if (zerorounding && fabs(x)<1e-12) x=0.0;
			if ((fabs(x)>maxexpo || fabs(x)<minexpo) && x!=0.0)
			sprintf(s,expoformat,x);
			else sprintf(s,fixedformat,x);
			if (zerorounding && fabs(y)<1e-12) 
			{	strcat(s,"+0i ");
				return;
			}
			if (y>=0) strcat(s,"+");
			else strcat(s,"-");
			y=fabs(y);
		two :
			if ((y>maxexpo || y<minexpo) && y!=0.0)
				sprintf(s+(int)(int)strlen(s),expoformat,y);
			else 
				sprintf(s+(int)(int)strlen(s),fixedformat,y);
	}
	if (fraceps>0) strcat(s,"*I");
	else strcat(s,"i");
}

void out_cmatrix (header *hd)
/***** out_matrix
   print a complex matrix.
*****/
{	
	int c,r,i,j,c0,cend;
	double *m,*x;
	getmatrix(hd,&r,&c,&m);
	int lines=0;
	if (error) return;
	if (r==1 && dense)
	{	int ol=0;
		char work[128];
		char line[MAXLINE];
		*line=0;
		if (hd->type!=s_complex) output("[ ");
		for (i=0; i<c && !error; i++)
		{	
			complex_out_string(*m,*(m+1),work);
			if (i<c-1) strcat(work,",");
			m+=2;
			if ((int)((int)(int)strlen(work)+ol+((ol>0)?dense:0))>linelength) 
			{	
				output(line); output("\n"); ol=0; *line=0; lines++;
			}
			if (lines>=20 && !printlargematrices)
			{
				strcat(line," ... "); ol+=5;
				break;
			}
			if (ol>0) 
			{	for (int k=0; k<dense; k++) 
				{	strcat(line," "); ol++; 
				}
			}
			strcat(line,work); ol+=(int)strlen(work);
			if (test_key()==escape) return;
		}
		if (ol>0)
		{	output(line); 
			if (hd->type!=s_complex) output(" ]\n");
			else output("\n"); 
			ol=0; *line=0;
		}
	}
	else if (!printlargematrices && (c>linew/2 || r>20))
	{
		print("Complex %d x %d matrix\n\n",r,c);
		for (i=0; i<(r>20?20:r) && !error; i++)
		{	
			x=cmat(m,c,i,0);
			for (j=0; j<linew/2 && j<c && !error; j++) 
			{ 
				complex_out(*x,*(x+1)); 
				x+=2;
			}
			if (j!=c) print("    ...\n");
			else print("\n");
		}
		if (r>20) print("    ...\n");
	}
	else for (c0=0; c0<c; c0+=linew/2)
	{	cend=c0+linew/2-1;
		if (cend>=c) cend=c-1;
		if (c>linew/2)
		{	if (cend>c0) output1("Column %d to %d:\n",c0+1,cend+1);
			else output1("Column %d:\n",c0+1);
		}
		for (i=0; i<r && !error; i++)
		{	x=cmat(m,c,i,c0);
			for (j=c0; j<=cend && !error; j++) 
			{ 
				complex_out(*x,*(x+1));
				x+=2; 
			}
			output("\n");
			if (test_key()==escape) return;
		}
	}
}

void interval_out (double x, double y)
/***** double_out
	print a complex number.
*****/
{   
	int d1,d2,l;
	char form[16];
	if ((int)strlen(useroutput)>0)
	{	header *var=searchudf(useroutput);
		if (!var) 
		{	error=1; 
			output("Euler function \"");
			output(useroutput); 
			output("\" not found!"); 
			return; 
		}
		header *hd=(header *)newram;
		new_interval(x,y,"");
		interpret_udf(var,hd,1,1);
		if (hd->type!=s_string || error)
		{	newram=(char *)hd;
			goto one;
		}
		output1hold(0,stringof(hd));
		output1hold(ioutputlength," ");
		newram=(char *)hd;
		return;
	}
	one : if (iformat>0 && (x>0 || y<0) && x!=y)
	{   if (x>0)
		{	d1=(int)log10(y); d2=(int)log10(y-x);
		}
		else
		{	d1=(int)log10(-x); d2=(int)log10(-x+y);
		}
		l=d1-d2+4;
		if (l>DBL_DIG+3) l=DBL_DIG+3;
		sprintf(form,"%%0.%dg",l);
		if (y!=floor(y))
		    y+=pow(10.0,floor(log10(fabs(y)))-l+1)/2.00000001;
		if (x!=floor(x))
			x-=pow(10.0,floor(log10(fabs(x)))-l+1)/2.00000001;
		output1hold(0,"~");
		output1hold(-1,form,x);
		output1hold(-1,",");
		output1hold(-1,form,y);
		output1hold(ioutputlength,"~ ");
		return;
	}
	output1hold(0,"~");
	output1hold(-1,"%0.2g",x);
	output1hold(-1,",");
	output1hold(-1,"%0.2g",y);
	output1hold(ioutputlength,"~ ");
}

void interval_out_string (double x, double y, char *s)
/*
	print an interval [x,y].
	The result ist stored to s.
*/
{   
	int d1,d2,l;
	char form[16];
	if ((int)strlen(useroutput)>0) // user output is active
	{	
		header *var=searchudf(useroutput);
		if (!var) 
		{	error=1; 
			output("Euler function \"");
			output(useroutput); 
			output("\" not found!"); 
			return; 
		}
		header *hd=(header *)newram;
		new_interval(x,y,"");
		interpret_udf(var,hd,1,1);
		if (hd->type!=s_string || error || (int)strlen(stringof(hd))>MAXWORK)
		{	
			newram=(char *)hd;
			goto one;
		}
		strcpy(s,stringof(hd));
		newram=(char *)hd;
		return;
	}
	one : 
	if (iformat>0 && (x>0 || y<0) && x!=y)
	{   
		if (x>0)
		{	d1=(int)log10(y); d2=(int)log10(y-x);
		}
		else
		{	d1=(int)log10(-x); d2=(int)log10(-x+y);
		}
		l=d1-d2+2;
		if (l>DBL_DIG+3) l=DBL_DIG+3;
		sprintf(form,"%%0.%dg",l);
		if (y!=floor(y))
		    y+=pow(10.0,floor(log10(fabs(y)))-l+1)/2.00000001;
		if (x!=floor(x))
			x-=pow(10.0,floor(log10(fabs(x)))-l+1)/2.00000001;
		strcpy(s,"~");
		sprintf(s+(int)strlen(s),form,x);
		strcat(s,",");
		sprintf(s+(int)strlen(s),form,y);
		strcat(s,"~");
		return;
	}
	strcpy(s,"~");
	sprintf(s+(int)strlen(s),"%0.2g",x);
	strcat(s,",");
	sprintf(s+(int)strlen(s),"%0.2g",y);
	strcat(s,"~");
}

void out_imatrix (header *hd)
/***** out_matrix
   print a complex matrix.
*****/
{	
	int c,r,i,j,c0,cend;
	double *m,*x;
	getmatrix(hd,&r,&c,&m);

	int lines=0;
	if (error) return;
	if (r==1 && dense)
	{	int ol=0;
		char work[MAXWORK];
		char line[MAXLINE];
		*line=0;
		if (hd->type!=s_interval) output("[ ");
		for (i=0; i<c && !error; i++)
		{	
			interval_out_string(*m,*(m+1),work);
			if (i<c-1) strcat(work,",");
			m+=2;
			if ((int)((int)strlen(work)+ol+((ol>0)?dense:0))>linelength) 
			{	output(line); output("\n"); ol=0; *line=0; lines++;
			}
			if (lines>=20 && !printlargematrices)
			{
				strcat(line," ... "); ol+=5;
				break;
			}
			if (ol>0) 
			{	
				for (int k=0; k<dense; k++) 
				{	strcat(line," "); ol++; 
				}
			}
			strcat(line,work); ol+=(int)strlen(work);
			if (test_key()==escape) return;
		}
		if (ol>0)
		{	
			output(line); 
			if (hd->type!=s_interval) output(" ]\n");
			else output("\n"); 
			ol=0; *line=0;
		}
	}
	else if (!printlargematrices && (c>ilinew || r>20))
	{
		print("Interval %d x %d matrix\n\n",r,c);
		for (i=0; i<(r>20?20:r) && !error; i++)
		{	
			x=imat(m,c,i,0);
			for (j=0; j<(ilinew>1?ilinew-1:1) && j<c && !error; j++) 
			{ 
				interval_out(*x,*(x+1)); 
				x+=2;
			}
			if (j!=c) print("    ...\n");
			else print("\n");
		}
		if (r>20) print("    ...\n");	
	}
	else for (c0=0; c0<c; c0+=ilinew)
	{	
		cend=c0+ilinew-1;
		if (cend>=c) cend=c-1;
		if (c>ilinew)
		{	if (cend>c0) output1("Column %d to %d:\n",c0+1,cend+1);
			else output1("Column %d:\n",c0+1);
		}
		for (i=0; i<r && !error; i++)
		{	x=imat(m,c,i,c0);
			for (j=c0; j<=cend && !error; j++) 
			{ 
				interval_out(*x,*(x+1));
				x+=2; 
			}
			output("\n");
			if (test_key()==escape) return;
		}
	}
}

void out_smatrix (header *hd)
{	
	char *str=stringof(hd);
	while (*str!=1)
	{	
		if (hd->flags&UTF) output(addbom(str));
		else output(str); 
		output("\n");
		if (test_key()) break;
		str+=(int)strlen(str)+1;
	}
}

void out_list (header *hd)
{	
	header *hd1=listof(hd);
	while (((char *)hd1-(char *)hd)<hd->size)
	{
		give_out(hd1);
		hd1=nextof(hd1);
	}
}

void out_binary (header *hd)
{
	output1("Binary data of %d bytes\n",*binsize(hd));
}

void out_cpxmatrix (header *hd)
{	
	int n=*nelementsof(hd);
	xelement *m=elementsof(hd);
	output1("Compressed %dx%d matrix\n",dimsof(hd)->r,dimsof(hd)->c);
	for (int i=0; i<n; i++)
	{	
		output1("%5d %5d %20g\n",m->r+1,m->c+1,m->x);
		m++;
		if (test_key()==escape) return;
	}
}

extern int useutf;

void give_out (header *hd)
/***** give_out
	print a value.
*****/
{	
	int oldpnan=pnan;
	pnan=1;
	switch(hd->type)
	{	case s_real : out_matrix(hd); break;
		case s_complex : out_cmatrix(hd); break;
		case s_matrix : out_matrix(hd); break;
		case s_cmatrix : out_cmatrix(hd); break;
		case s_imatrix : out_imatrix(hd); break;
		case s_string : 
			if ((int)strlen(stringof(hd))>1 || !isnone(hd))	
			{	
				if ((hd->flags&LATEX) && (int)strlen(stringof(hd))<MAXLINE-2)
				{
					char s[MAXLINE];
					strcpy(s,stringof(hd));
					char *res=tex(s);
					insert_latex(res);
				}
				else if ((hd->flags&SYMBOLIC) && (int)strlen(stringof(hd))<MAXLINE-2)
				{
					char s[MAXLINE];
					strcpy(s,stringof(hd));
					strcat(s,";");
					callmaxima(s,4);
				}
				else
				{
					if (hd->flags & UTF)
					{
						if (useutf) output(fromutf(stringof(hd)));
						else output(addbom(stringof(hd)));
					}
					else output(stringof(hd)); 
					output("\n"); 
				}
			}
			break;
		case s_smatrix : out_smatrix(hd); break;
		case s_interval : out_imatrix(hd); break;
		case s_cpxmatrix : out_cpxmatrix(hd); break;
		case s_list : out_list(hd); break;
		case s_binary : out_binary(hd); break;
		default : print("Unknown type %d\n",hd->type);
	}
	pnan=oldpnan;
}

/***************** some builtin commands *****************/

#define EXTENSION ".e"
#define BOOKEXTENSION ".en"

char currentfilename[1024]="";

extern int alwaysrelax;

FILE *open_file_in_path (char *filename, char *mode, int currentfirst, int eulerfile)
{	
	char fn[FILENAME_MAX];
	int pn=-2;
	// print("open %s in mode %s with currentfirst=%d\n",filename,mode,currentfirst);
	if (currentfirst && !strchr(filename,'\\'))
	{
		strcpy(fn,".\\");
		strcat(fn,filename);
		// print("trying %s\n",fn);
		FILE *infile=fopen(fn,mode);
		if (infile) return infile;
		if (eulerfile)
		{
			strcat(fn,EXTENSION);
			infile=fopen(fn,mode);
			if (infile) return infile;
		}
	}
	retry :
	if (pn>=0)
	{	if (*path[pn]=='\\')
		{	strcpy(fn,getstartdir());
			strcat(fn,path[pn]+1);
		}
		else strcpy(fn,path[pn]);
		strcat(fn,PATH_DELIM_STR);
		strcat(fn,filename);
	}
	else if (pn==-1)
	{	strcpy(fn,filename);
	}
	else if (pn==-2)
	{	strcpy(fn,getstartdir());
		strcat(fn,filename);
	}
	// output1("%s\n",fn);
	FILE *infile=fopen(fn,mode);
	if (!infile)
	{   
		if (eulerfile)
		{
			strcat(fn,EXTENSION);
			infile=fopen(fn,mode);
		}
		if (!infile)
		{	pn++;
			if (pn>=npath) return 0;
			else goto retry;
		}
	}
	return infile;
}

char *search_file_in_path (char *filename, char *extension)
{	
	static char fn[FILENAME_MAX];
	int pn=-2;
	retry :
	if (pn>=0)
	{	
		if (*path[pn]=='\\')
		{	strcpy(fn,getstartdir());
			strcat(fn,path[pn]+1);
		}
		else strcpy(fn,path[pn]);
		strcat(fn,PATH_DELIM_STR);
		strcat(fn,filename);
	}
	else if (pn==-1)
	{	strcpy(fn,filename);
	}
	else if (pn==-2)
	{	strcpy(fn,getstartdir());
		strcat(fn,filename);
	}
	// output1("%s\n",fn);
	if (file_exists(fn)) return fn;
	else
	{   
		strcat(fn,extension);
		if (file_exists(fn)) return fn;
		else
		{	pn++;
			if (pn>=npath) return 0;
			else goto retry;
		}
	}
}

void addsubmenu();
extern char *py_run_file (char *s);
extern int allowoverwrite;

void load_file (void)
/***** load_file
	interpret a file.
*****/
{	
	char filename[1024];
	char oldline[MAXLINE],*oldnext;
	int oldbooktype=booktype;
	header *hd;
	FILE *oldinfile;
	int old=allowoverwrite;
	if (udfon)
	{	output("Cannot load a file in a function!\n");
		error=221; goto end;
	}
	scan_space();
	if (strstarts(next,"over "))
	{
		allowoverwrite=1;
		next+=5;
	}
	if (*next=='(')
	{   hd=scan_value();
		if (error) goto end;
		if (hd->type!=s_string)
		{	output("String value expected!\n");
			error=1; goto end;
		}
		if ((int)strlen(stringof(hd))>255)
		{	output("File name too long!\n");
			error=1; goto end;
		}
		strcpy(filename,stringof(hd));
	}
	else
	{	scan_filenamemax(filename,255);
	}
	if (error) goto end;

	int fl=(int)strlen(filename);
	if (fl>3 && !strcmp(filename+fl-3,".py"))
	{
		char *h=py_run_file(filename);
		if (!error && *h && *next!=';') print("%s\n",h);
		if (*next==';') next++;
		goto end;
	}

	addsubmenu(filename);
	defineasrelax=alwaysrelax;
	int silent=(*next==';');
	if (*next==';' || *next==',') next++;
	oldinfile=infile;
	infile=open_file_in_path(filename,"r",1,1);
	if (!infile)
	{	output1("Could not open %s!\n",filename);
		error=53; infile=oldinfile;
		goto end;
	}
	flog("load %s",filename);
	strcpy(oldline,input_line); oldnext=next;
	*input_line=0; next=input_line;
	strcpy(currentfilename,filename);
	addloadedfile(filename);
	booktype=0;
	int oldshowcomments=printcomments;
	if (silent) printcomments=0;

	// loop
	while (!error && infile && !quit)
	{	startglobal=startlocal;
		endglobal=endlocal;
		command();
	}
	
	defineasrelax=0;
	printcomments=oldshowcomments;
	booktype=oldbooktype;
	if (infile) fclose(infile);
	strcpy(currentfilename,"");
	infile=oldinfile;
	strcpy(input_line,oldline); next=oldnext;
end:
	allowoverwrite=old;
}

void stopload_file ()
{
	infile=0;
}

void do_tcc ()
{
	char name[MAXLINE];
	scan_space();
	if (*next=='(')
	{   
		header *hd=scan_value();
		if (error) return;
		if (hd->type!=s_string)
		{	output("String value expected!\n");
			error=1; return;
		}
		if ((int)strlen(stringof(hd))>MAXLINE-1)
		{	output("String too large!\n");
			error=1; return;
		}
		strcpy(name,stringof(hd));
	}
	else
	{	
		scan_namemax(name,MAXLINE,1);
	}
	if (error) return;
#ifdef DLL
	closedll(name);
#endif
	tcc(name);
}

void load_book (void)
/***** load_book
	interpret a notebook file.
*****/
{	header *hd;
	char name[MAXLINE];
	char oldline[MAXLINE],fn[256],*oldnext;
	int oldbooktype=booktype;
	FILE *oldinfile;
	if (udfon)
	{	output("Cannot load a notebook in a function!\n");
		error=221; return;
	}
	 scan_space();
	if (*next=='(')
	{   hd=scan_value();
		if (error) return;
		if (hd->type!=s_string)
		{	output("String value expected!\n");
			error=1; return;
		}
		if ((int)strlen(stringof(hd))>MAXLINE-1)
		{	output("String too large!\n");
			error=1; return;
		}
		strcpy(name,stringof(hd));
	}
	else
	{	scan_namemax(name,MAXLINE);
	}
	if (error) return;
	oldinfile=infile;
	infile=fopen(name,"r");
	if (!infile)
	{	strcpy(fn,name);
		strcat(fn,BOOKEXTENSION);
		infile=fopen(fn,"r");
		if (!infile)
		{	output1("Could not open %s!\n",stringof(name));
			error=53; infile=oldinfile; return;
		}
	}
	flog("Open %s",name);
	strcpy(oldline,input_line); oldnext=next;
	*input_line=0; next=input_line;
	booktype=1;
	while (!error && infile && !quit)
	{	startglobal=startlocal; endglobal=endlocal;
		command();
	}
	booktype=oldbooktype;
	if (infile) fclose(infile);
	infile=oldinfile;
	strcpy(input_line,oldline); next=oldnext;
}

header *returnresult=0;

void do_return (void)
{	if (!udfon)
	{	output("Use return only in functions!\n");
		error=56; return;
	}
	udfon=2;
}

void do_else (void)
{	
	scan_endif();
}

void do_elseif (void)
{	
	scan_endif();
}

void do_endif (void)
{	
}

int ctest (header *hd)
/**** ctest
	test, if a matrix contains nonzero elements.
****/
{	
	double *m;
	LONG n,i;
	hd=getvalue(hd); if (error) return 0;
	if (hd->type==s_string) 
	{
		char *p=stringof(hd);
		return (*p!=1 && *p!=0);
	}
	else if (hd->type==s_real) return (*realof(hd)!=0.0);
	else if (hd->type==s_complex) return (*realof(hd)!=0.0 &&
		*imagof(hd)!=0.0);
	else if (hd->type==s_matrix)
	{	
		if (!conditionvectors && !relax)
		{	output("Cannot use vectors for conditions, use all(...)!\nMaybe you need to vectorize the function with \"map\".\n");
			error=1; return 0;
		}
		n=(LONG)(dimsof(hd)->r)*dimsof(hd)->c;
		m=matrixof(hd);
		for (i=0; i<n; i++) if (*m++==0.0) return 0;
		return 1;
	}
	else if (hd->type==s_cmatrix)
	{	
		if (!conditionvectors && !relax)
		{	output("Cannot use vectors for conditions, use all(...)!\nMaybe you need to vectorize the function with \"map\".\n");
			error=1; return 0;
		}
		n=(LONG)(dimsof(hd)->r)*dimsof(hd)->c;
		m=matrixof(hd);
		for (i=0; i<n; i++) 
		{	if (*m==0.0 && *(m+1)==0.0) return 0; m+=2; }
		return 1;
	}
	else
	{	output("Cannot use this type as if condition.\n");
		error=1; return 0;
	}
}

void do_then (void)
{
}

void do_if (void)
{	
	header *cond;
	int flag;
	int target=0;
	char *here;
	int *extra=0;

another :
	here=next;
	if (udfon)
	{	
		extra=((int *)next)-1;
		target=*extra;
	}
	scan_space();
	cond=scan(); if (error) return;
	if (*next=='=')
	{	
		output("Want to use == for conditions?\n");
		error=1; return;
	}
	flag=ctest(cond); 
	if (error) return;
	if (!flag)
	{	
		if (target) // already noted the jump target
		{	
			if (target>0) next=here+target;
			else 
			{	
				next=here-target;
				goto another;
			}
		}
		else if (scan_else(here,extra)) 
		{	
			if (extra) *extra=-(*extra); // means "elseif" or "or".
			goto another;
		}
	}
}

void do_clg (void)
{	gclear(); gflush();
	checkcommand("clg");
}

void do_cls (void)
{	text_mode(); clear_screen();
	checkcommand("cls");
}

void do_clear (void)
{	
	if (udfon)
	{	output("Cannot use \"clear\" in a function!\n");
		error=120; return;
	}
	clear();
	checkcommand("clear");
}

void do_remvalue (void)
{	remvalue();
}

void do_clearall (void)
{	
	header *hd=(header *)startlocal;
	while ((char *)hd<endlocal)
	{	
		if (!(hd->flags&PROTECT) && strncmp(hd->name,"default",7)!=0 && *hd->name!='%') 
		{
			kill_local(hd->name);
			hd=(header *)startlocal;
		}
		else hd=nextof(hd);
	}
	clear_global();
	checkcommand("clearall");
}


void do_alias ()
{
	char s1[maxname],s2[maxname];
	skip_space(); scan_name(s1); if (error) return;
	skip_space(); scan_name(s2); if (error) return;
	addalias(s1,s2);
}

void do_quit (void)
{	quit=1;
}

void do_reset (void)
{	resetall();
}

void do_restart (void)
{	quit=2;
}

void mexec (header *hd)
{	
	header *st=hd,*hdparam,*hddir,*hdoutput,*hdhidden,*hdwait;
	hdparam=nextof(hd);
	hddir=nextof(hdparam);
	hdoutput=nextof(hddir);
	hdhidden=nextof(hdoutput);
	hdwait=nextof(hdhidden);
	hd=getvalue(hd); if (error) return;
	hdparam=getvalue(hdparam); if (error) return;
	hddir=getvalue(hddir); if (error) return;
	hdoutput=getvalue(hdoutput); if (error) return;
	hdhidden=getvalue(hdhidden); if (error) return;
	hdwait=getvalue(hdwait); if (error) return;
	if (hd->type!=s_string || hdparam->type!=s_string || hddir->type!=s_string
		|| hdoutput->type!=s_real || hdhidden->type!=s_real || hdwait->type!=s_real)
		need_arg_in("exec","program, args, directory, print flag, hidden flag, wait flag");

	int output=(int)*realof(hdoutput),hidden=(int)*realof(hdhidden),
		wait=(int)*realof(hdwait);
	char buffer[32000];
	int ex=execute(stringof(hd),stringof(hdparam),stringof(hddir),
		output,hidden,wait,buffer,32000);
	if (error) return;
	
	if (output && wait)
	{
		header *hds=new_smatrix("");
		char *p=buffer;
		char *q=stringof(hds);
		while (*p)
		{
			char c=*p++;
			if (c==13) continue;
			if (c==10) { *q++=0; continue; }
			*q++=c;
		}
		*q++=1;
		newram=q;
		hds->size=(long)(newram-(char *)hds);
		new_real(ex,"");
		moveresult1(st,hds);
	}
	else
	{
		header *res=new_real(ex,"");
		moveresult(st,res);
	}
}

void mexecread (header *hd)
{
	char buffer[32000];
	execread(buffer,32000);
	header *hds=new_smatrix("");
	char *p=buffer;
	char *q=stringof(hds);
	while (true)
	{
		char c=*p++;
		if (c==13) continue;
		if (c==10) { *q++=0; continue; }
		*q++=c;
		if (!c) break;
	}
	*q++=1;
	newram=q;
	hds->size=(long)(newram-(char *)hds);
}

void mexecwrite (header *hd)
{
	header *st=hd;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) need_arg_in("execwrite","string");
	int res=execwrite(stringof(hd),0);
	header *rs=new_real(res,"");
	moveresult(st,rs);
}

void mexecwriteln (header *hd)
{
	header *st=hd;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) need_arg_in("execwrite","string");
	int res=execwrite(stringof(hd),1);
	header *rs=new_real(res,"");
	moveresult(st,rs);
}

void mexeckill (header *hd)
{
	execkill();
	new_none();
}

void mmessage (header *hd)
{
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) need_arg_in("message","string");
	Warning(stringof(hd),"Euler");
}

void do_forget (void)
{	
	char name[maxname];
	header *hd;
	int r;
	if (udfon)
	{	
		output("Cannot forget functions in a function!\n");
		error=720; return;
	}
	while (1)
	{	
		scan_space();
		scan_name(name);
		r=xor(name);
		hd=(header *)ramstart;
		while ((char *)hd<udfend)
		{	
			if (r==hd->xor && !strcmp(hd->name,name)) break;
			hd=nextof(hd);
		}
		if ((char *)hd<udfend)
		{
			int symbolic=hd->flags&&symbolic;
			kill_udf(name);
			if (symbolic)
			{
				char line[MAXLINE];
				sprintf(line,"remfunction(%s)",name);
				callmxm(line);
			}
		}
		scan_space();
		if (*next!=',') break;
		else next++;
	}
}

/**
Create a reference to a global variable in a function.
>global a
*/
void do_global (void)
{	
	char name[maxname];
	int r;
	header *hd;

	if (!udfon)
	{
		output("It makes no sense to use \"global\" outside of functions.\n");
		error=500; return;
	}

	while (1)
	{	
		scan_space(); scan_name(name); r=xor(name);
		scan_space();

		int heap=0;
		hd=globalfind(name);
		if (hd)
		{
			heap=1;
		}
		else
		{
			hd=(header *)startglobal;
			while ((char *)hd<endglobal)
			{	
				if (r==hd->xor && !strcmp(hd->name,name)) break;
				hd=nextof(hd);
			}
			if ((char *)hd>=endglobal)
			{	
				if (*next=='=' || strstarts(next,":="))
				{
					if (*next=='=') next++;
					else next+=2;
					header *h=scan_value();
					if (error) return;
					globalput(name,h);
					hd=globalfind(name);
					if (!hd)
					{
						print("Could not allocate the global variable %s\n",name);
						error=1; return;
					}
				}
				else
				{
					output1("Global variable %s not found in \"global\" command.\n",name);
					error=160; return;
				}
			}
		}
		
		newram=endlocal;
		header *h=new_reference(hd,name);
		h->flags|=HEAP;
		h->flags|=GLOBALREF;
		h->flags|=hd->flags;

		newram=endlocal=(char *)nextof(h);
		
		if (*next=='=' || strstarts(next,":="))
		{
			if (*next=='=') next++;
			else next+=2;
			header *hh=scan_value();
			if (error) return;
			hh=globalput(name,hh);
			referenceof(h)=hh;
		}
		if (*next!=',') break;
		else next++;
	}
	checkcommand("global");
}

void do_useglobal (void)
{   
	searchglobal=1;
	checkcommand("useglobal");
}

void do_base0 (void)
{

	char name[maxname];
	header *hd;

	while (1)
	{	
		scan_space(); scan_name(name);
		scan_space();

		hd=searchvar(name);
		if (hd)
		{
			// while (hd && hd->type==s_reference) hd=referenceof(hd);
			if (hd) hd->flags|=ZEROBASED;
		}
		else
		{
			print("Variable %s not found.\n",name);
			error=1; return;
		}
		
		if (*next!=',') break;
		else next++;
	}

	checkcommand("base0");
}

void print_commands (char *name);

int name_compare (const char **p1, const char **p2)
{	return strcmp(*p1,*p2);
}

extern int builtin_count,command_count;

#define MAXNAMES 10000

int contains (const char *s, const char *p)
// check if one string contains another
{	
	return strstr(s,p)!=0;
}

int do_list_name (char *name)
{	
	// Functions
	int count=0,lcount=0,totalcount=0;
	char *names[MAXNAMES+1];
	header *hd=(header *)ramstart;
	while ((char *)hd<udfend)
	{	
		if (hd->type!=s_udf || count>=MAXNAMES) break;
		if (contains(hd->name,name) && !(*hd->name=='%'))
		{	names[count++]=hd->name;
		}
		hd=nextof(hd);
	}
	for (int i=0; i<builtin_count; i++)
	{	
		if (contains(builtin_list[i].name,name))
		{	names[count++]=builtin_list[i].name;
		}
	}
	qsort(names,count,sizeof(char *),(int (*) (const void *, const void *))name_compare);
	if (count>0)
	{	
		output("\n  *** Functions:\n");
		char *prev=0;
		for (int i=0; i<count; i++)
		{	char *name=names[i];
			if (prev!=0 && strcmp(prev,name)==0) continue;
			if (lcount+(int)(int)strlen(name)+2>=linelength)
				{ lcount=0; output("\n"); }
			output1("%s ",name);
			lcount+=(int)(int)strlen(name)+1;
			prev=name;
		}
		output("\n");
	}
	totalcount+=count;
	// Variables
	count=0;
	hd=(header *)startlocal;
	while ((char *)hd<newram)
	{	
		if (hd->type==s_udf) continue;
		if (count>=MAXNAMES) break;
		if (contains(hd->name,name) && !(*hd->name=='%'))
		{	names[count++]=hd->name;
		}
		hd=nextof(hd);
	}
	qsort(names,count,sizeof(char *),(int (*) (const void *, const void *))name_compare);
	if (count>0)
	{	
		output("\n  *** Variables :\n");
		char *prev=0;
		for (int i=0; i<count; i++)
		{	char *name=names[i];
			if (prev!=0 && strcmp(prev,name)==0) continue;
			if (lcount+(int)(int)strlen(name)+2>=linelength)
				{ lcount=0; output("\n"); }
			output1("%s ",name);
			lcount+=(int)(int)strlen(name)+1;
			prev=name;
		}
		output("\n");
	}
	totalcount+=count;
	// Commands
	count=0;
	for (int i=0; i<command_count; i++)
	{	
		if (contains(command_list[i].name,name))
		count++;
	}
	if (count>0)
	{	
		output("\n  *** Commands:\n");
		int linel=0;
		for (int i=0; i<command_count; i++)
		{	if (contains(command_list[i].name,name))
			{	
				if (linel+(unsigned int)strlen(command_list[i].name)+2>
						(unsigned int)linelength)
					{ output("\n"); linel=0; }
				output1("%s ",command_list[i].name);
				linel+=(int)(int)strlen(command_list[i].name)+1;
			}
		}
		output("\n");
	}
	totalcount+=count;
	char extend[MAXLINE];
	char status[MAXLINE];
	char *mh=extend;
	getmaximahelp(name,mh,status,0);
	if (mh && *mh) 
	{	
		output("\n  *** Maxima:\n\n");
		char *p=mh;
		while (*p!=0)
		{	int lcount=0;
			while ((lcount<linelength-16 || *p!=' ') && *p) 
			{	
				if (*p==' ') totalcount++;
				p++; lcount++;
			}
			char c=*p;
			*p=0;
			output1("%s\n",mh);
			*p=c;
			while (*p==' ') p++;
			mh=p;
		}
	}
	return totalcount;
}

void do_list (void)
{	
	char name[maxname];
	while (*next==' ') next++;
	char *p=name;
	while (*next!=0 && *next!=' ')
	{	*p++=*next++;
    	if (p-name>maxname-2) break;
	}
	*p=0;
	do_list_name(name);
}

void listvar1 (char *s, header *hd)
{	output1("%-20sType: %s\n",hd->name,s);
}

void listvar2 (char *s, header *hd)
{	output1("%-20sType: %s (%dx%d)\n",hd->name,s,dimsof(hd)->r,dimsof(hd)->c);
}

void listvar3 (char *s, header *hd)
{	output1("%-20sType: %s (%dx%0d)",hd->name,s,
		submdimsof(hd)->r,submdimsof(hd)->c);
}


void do_listvar (void)
{	
	char name[maxname];
	while (*next==' ') next++;
	char *p=name;
	while (*next!=0 && *next!=' ')
	{	
		*p++=*next++;
    	if (p-name>254) break;
	}
	*p=0;
	do_listvar_name(name);
}

void listvar_output (header *hd)
{
	char s[MAXLINE];
	switch (hd->type)
	{	
		case s_real : 
			print("%-20s%0.15g\n",hd->name,*realof(hd)); 
			break;
		case s_interval :
			print("%-20s~%0.15g,%0.15g~\n",hd->name,
				*realof(hd),*(realof(hd)+1));
			break;
		case s_complex :
			print("%-20s%0.15g+%0.15gi\n",hd->name,
				*realof(hd),*(realof(hd)+1));
			break;
		case s_string :
			strcpy(s,stringof(hd));
			if ((int)strlen(s)>32) strcpy(s+32," ...");
			print("%-20s%s\n",hd->name,s);
			break;
		case s_smatrix : listvar1("String Vector",hd); break;
		case s_matrix : listvar2("Real Matrix",hd); break;
		case s_cmatrix : listvar2("Complex Matrix",hd); break;
		case s_imatrix : listvar2("Interval Matrix",hd); break;
		case s_reference : listvar1("Reference",hd); break;
		case s_submatrix : listvar3("Real Submatrix",hd); break;
		case s_isubmatrix : listvar3("Interval Submatrix",hd); break;
		case s_csubmatrix : listvar3("Complex Submatrix",hd); break;
		case s_cpxmatrix : listvar2("Compressed Matrix",hd); break;
		case s_list : listvar1("Collection",hd); break;
		case s_binary : listvar1("Binary Data",hd); break;
		default: listvar1("Unknown Type",hd); break;
	}	
}

extern int listglobal(char *name);

void do_listvar_name (char *name)
{
	header *hd=(header *)startlocal;
	int count=0;
	while (hd<(header *)endlocal)
	{	
		if ((*name && contains(hd->name,name)) || (!*name &&
			!(hd->flags&PROTECT) && strncmp(hd->name,"default",6) 
			&& strncmp(hd->name,"%",1)))
		{
			listvar_output(hd);
			count++;
		}
		hd=nextof(hd);
		if (test_key()==escape) break;
	}
	if (!udfon)
	{
		count+=listglobal(name);
	}
	if (!udfon)
	{
		count+=list_lists(name);
	}
	if (count==0) 
	{
		if (*name) print("No variable containing %s found.\n",name);
		else print("No user variables defined.\n");
	}
}

void do_listlists (void)
{	
	char name[maxname];
	while (*next==' ') next++;
	char *p=name;
	while (*next!=0 && *next!=' ')
	{	
		*p++=*next++;
    	if (p-name>254) break;
	}
	*p=0;
	int count=list_lists(name);
	if (count==0) 
	{
		if (*name) print("No lists containing %s found.\n",name);
		else print("No lists defined.\n");
	}
}



void do_dump (void)
{	
	header *file;
	if (outfile)
	{	if (fclose(outfile))
		{	output("Error while closing dumpfile.\n");
		}
		outfile=0;
	}
	scan_space();
	if (*next==';' || *next==',' || *next==0)
	{	if (*next) next++; return; }
	file=scan_value();
	if (error || file->type!=s_string)
	{	output("Dump needs a filename!\n");
		error=201; return;
	}
	outfile=fopen(stringof(file),"a");
	if (!outfile)
	{	output1("Could not open %s!\n",stringof(file));
	}
}

#ifdef YACAS
void myacas (header *hd);
void do_yacas (void)
{	char s[MAXLINE];
	scan_space();
	char *p=s;
	strcpy(p,"PrettyForm(");
	p+=(int)strlen(p);
	while (*next!=0 && *next!=1)
	{	*p++=*next++;
		if (p-s>1023)
		{	output("Yacas line too long!\n");
			error=1;
			return;
		}
	}
	strcpy(p,")");
	header *hd=new_string(s,(int)strlen(s),"");
	myacas(hd);
}
#endif

int yacasmode=0;

void do_yacasmode ()
{	
	char name[maxname];
	scan_space();
	if (!*next || *next==';' || *next==',')
	{	
		yacasmode=!yacasmode;
	}
	else
	{	scan_name(name);
		if (strcmp(name,"on")==0)
			yacasmode=1;
		if (strcmp(name,"off")==0)
			yacasmode=0;
	}
	if (*next!=';')
	{	
		switch (yacasmode)
		{	case 0 : output1("Yacas mode is off\n"); break;
			case 1 : output1("Yacas mode is on\n"); break;
		}
	}
	if (*next==',' || *next==';') next++;
}

void do_matlab ()
{	
	char name[maxname];
	scan_space();
	if (!*next || *next==';' || *next==',') 
		matlab=!matlab;
	else
	{	scan_name(name);
		if (strcmp(name,"on")==0)
			matlab=1;
		if (strcmp(name,"off")==0)
			matlab=0;
	}
	if (*next!=';')
	{	switch (matlab)
		{	case 0 : output1("Matlab mode is off\n"); break;
			case 1 : output1("Matlab mode is on\n"); break;
		}
	}
	if (*next==',' || *next==';') next++;
}

static double currenttime;

void mtic (header *hd)
{
	currenttime=myclock();
	new_real(currenttime,"");
}

void mtoc (header *hd)
{
	double t=myclock()-currenttime;
	print("Used %0.6g seconds\n",t);
	new_real(t,"");
}

void mwin64 (header *hd)
{
#ifdef WIN64
	new_real(1.0,"");
#else
	new_real(0.0,"");
#endif
}

void mgetstacksize (header *hd)
{
	new_real(getstacksize(),"");
}

void do_path (void)
{	int i;
	scan_space();
	if (*next==';' || *next==',' || *next==0)
	{   out :
		for (i=0; i<npath; i++)
		{	output1("%s;",path[i]);
		}
		output("\n");
		return;
	}
	header *hd=scan_value();
	if (error) return;
	if (hd->type!=s_string)
	{	output("String value expected!\n");
		error=1; return;
	}
	char s[MAXLINE];
	char *p=stringof(hd);
	for (i=0; i<npath; i++) free(path[i]);
	npath=0;
	while (*p)
	{	char *q=s;
		while (*p && *p!=';') *q++=*p++;
		if (q>s && *(q-1)==PATH_DELIM_CHAR) q--;
		*q=0;
		if (*p==';') p++;
		path[npath]=(char *)malloc((int)strlen(s)+1);
		strcpy(path[npath],s);
		npath++;
	}
	if (npath==0)
	{	path[0]=(char *)malloc(5);
		strcpy(path[0],".");
	}
	if (*next!=';') goto out;
}

void do_cd (void)
{	header *hd;
	char name[MAXLINE];
	char *s;
	scan_space();
	if (*next==';' || *next==',' || *next==0)
	{	s=cd("");
		output1("%s\n",s);
		return;
	}
	if (*next=='(')
	{   hd=scan_value();
		if (error) return;
		if (hd->type!=s_string)
		{	output("String value expected!\n");
			error=1; return;
		}
		if ((int)strlen(stringof(hd))>MAXLINE-1)
		{	output("String too large!\n");
			error=1; return;
		}
		strcpy(name,stringof(hd));
	}
	else
	{	scan_namemax(name,MAXLINE);
	}
	if (error) return;
	s=cd(name);
	if (*next!=';') output1("%s\n",s);
	if (*next==',' || *next==';') next++;
}

void do_postscript (void)
{	
	header *file;
	scan_space();
	file=scan_value();
	if (error || file->type!=s_string)
	{	output("Postscript needs a filename!\n");
		error=201; return;
	}
	FILE *metafile=fopen(stringof(file),"w");
	if (!metafile)
	{	output1("Could not open %s!\n",stringof(file));
	}
    dump_postscript(metafile);
    fclose(metafile);
}

void mfileremove (header *hd)
{	
	header *st=hd;
	hd=getvalue(hd);
	if (error) return;
	if (hd->type!=s_string)
		need_arg_in("fileexists","a string");
	remove(stringof(hd));
	header *result=new_string("\0x01","");
	moveresult(st,result);
}

void mfilecopy (header *hd)
{
	header *st=hd,*hd1;
	hd1=nextof(hd);
	hd=getvalue(hd);
	hd1=getvalue(hd1);
	if (error) return;
	if (hd->type!=s_string || hd1->type!=s_string)
		need_arg_in("filecopy","two strings");
	if (!file_copy(stringof(hd),stringof(hd1)))
	{
		error=1; print("Error while copying\n%s to %s\n",stringof(hd),stringof(hd1));
		return;
	}
	header *result=new_none();
	moveresult(st,result);
}

void mfileexists (header *hd)
{
	header *st=hd;
	hd=getvalue(hd);
	if (error) return;
	if (hd->type!=s_string)
		need_arg_in("fileexists","a string");
	header *result=new_real(file_exists(stringof(hd)),"");
	moveresult(st,result);
}

void do_do (void)
{	int udfold;
	char name[maxname];
	char *oldnext=next,*udflineold;
	header *var;
	scan_space(); scan_name(name); if (error) return;
	var=searchudf(name);
	if (!var || var->type!=s_udf)
	{	output("Need a udf!\n"); error=220; return;
	}
	udflineold=udfline; udfline=next=udfof(var); udfold=udfon; udfon=1;
	while (!error && udfon==1)
	{	command();
		if (udfon==2) break;
		if (test_key()==escape) 
		{	output("User interrupted!\n"); error=58; break;
		}
	}
	if (error) output1("Error in function %s!\n",var->name);
	if (udfon==0)
	{	output1("Return missing in %s!\n",var->name); error=55; }
	udfon=udfold; udfline=udflineold;
	if (udfon) next=oldnext;
	else { next=input_line; *next=0; }
}

void do_mdump (void)
{	header *hd;
	output1("ramstart : 0\nstartlocal : %ld\n",startlocal-ramstart);
	output1("endlocal : %ld\n",endlocal-ramstart);
	output1("newram   : %ld\n",newram-ramstart);
	output1("ramend   : %ld\n",ramend-ramstart);
	hd=(header *)ramstart;
	while ((char *)hd<newram)
	{
		output1("%6ld : %16s, ",(char *)hd-ramstart,hd->name);
		output1("size %6ld ",(long)hd->size);
		output1("type %d\n",hd->type);
		hd=nextof(hd);
	}
}

void hex_out1 (int n)
{	if (n<10) output1("%c",n+'0');
	else output1("%c",n-10+'A');
}

void hex_out (unsigned int n)
{	hex_out1(n/16);
	hex_out1(n%16);
	output(" ");
}

void string_out (unsigned char *p, int k)
{	int i;
	unsigned char a;
	for (i=0; i<k; i++) 
	{	a=*p++;
		output1("%c",(a<' ')?'_':a);
	}
}

void do_hexdump (void)
{	
	char name[maxname];
	unsigned char *p,*end;
	int i=0,j;
	ULONG count=0;
	header *hd;
	scan_space(); scan_name(name); if (error) return;
	hd=searchvar(name);
	if (!hd) hd=searchudf(name);
	if (error || hd==0) return;
	p=(unsigned char *)hd; end=p+hd->size;
	output1("\n%s at %8lx\n",name,p-(unsigned char *)ramstart);
	output1("\n%4lx  ",count);
	while (p<end)
	{	
		hex_out(*p++); i++; count++;
		if (i>=16) 
		{	
			i=0; string_out(p-16,16);
			output1("\n%4lx  ",count);
			if (test_key()==escape) break;
		}
	}
	for (j=i; j<16; j++) output("   ");
	string_out(p-i,16-i);
	output("\n");
}

void do_output (void)
/**** do_output
	toggles output.
****/
{	scan_space();
	if (!strncmp(next,"off",3))
	{	outputing=0; next+=3;
	}
	else if (!strncmp(next,"on",2))
	{	outputing=1; output("\n"); next+=2;
	}
	else outputing=!outputing;
}

void do_errors (void)
/****
	toggles pnan (produce NAN)
****/
{	scan_space();
	if (!strncmp(next,"off",3))
	{	pnan=1; next+=3;
	}
	else if (!strncmp(next,"on",2))
	{	pnan=0; next+=2;
	}
	else pnan=!pnan;
}

void do_easyunits (void)
/****
	toggles easy units like 1cm.
****/
{	
	scan_space();
	if (!strncmp(next,"off",3))
	{	easyunits=0; next+=3;
	}
	else if (!strncmp(next,"on",2))
	{	easyunits=1; next+=2;
	}
	else easyunits=!easyunits;
}

void do_underflows (void)
/****
	toggles underflows (produce underflows)
****/
{	
	scan_space();
	if (!strncmp(next,"off",3))
	{	underflows=0; next+=3;
	}
	else if (!strncmp(next,"on",2))
	{	underflows=1; next+=2;
	}
	else underflows=!underflows;
}

void do_largematrices (void)
/****
	toggles underflows (produce underflows)
****/
{	
	scan_space();
	if (!strncmp(next,"off",3))
	{	printlargematrices=0; next+=3;
	}
	else if (!strncmp(next,"on",2))
	{	printlargematrices=1; next+=2;
	}
	else printlargematrices=!printlargematrices;
}

void do_prompt (void)
/**** do_prompt
	toggles notebook prompt.
****/
{	
	scan_space();
	if (!strncmp(next,"off",3))
	{   promptnotebook=0; next+=3;
	}
	else if (!strncmp(next,"on",2))
	{	promptnotebook=1; output("\n"); next+=2;
	}
	else promptnotebook=!promptnotebook;
}

void do_comments (void)
/**** do_comments
	toggles comments
****/
{	scan_space();
	if (!strncmp(next,"off",3))
	{	printcomments=0; next+=3;
	}
	else if (!strncmp(next,"on",2))
	{	printcomments=1; output("\n"); next+=2;
	}
	else printcomments=!printcomments;
}

void do_relax (void)
{	
	if (!infile || udfon)
	{	output("\"relax\" only in Euler files!\n");
		error=1001; return;
	}
	defineasrelax=1;
}

void do_comment (void)
{	
	FILE *fp=infile;
	if (!fp || udfon)
	{	output("\"comment\" illegal at this place!\n");
		error=1001; return;
	}
	while (1)
	{	next_line();
		if (infile!=fp)
		{	output("\"endcomment\" missing!\n"); error=1002;
			return;
		}
		if (strncmp(next,"endcomment",10))
		{	if (printcomments)
			{	output(input_line); output("\n");
			}
		}
		else break;
	}
	next_line();
}

int nextfunctioniscomment=0;

void do_maximafunction (void)
{	
	nextfunctioniscomment=1;
	get_udf();
	nextfunctioniscomment=0;
}

void do_skipcomment (void)
{	
	FILE *fp=infile;
	if (!fp || udfon)
	{	output("\"skipcomment\" illegal at this place!\n");
		error=1001; return;
	}
	while (1)
	{	next_line();
		if (infile!=fp)
		{	output("\"endcomment\" missing!\n"); error=1002;
			return;
		}
		if (!strncmp(next,"endcomment",10)) break;
		if (!strncmp(next,"*/",2)) break;
	}
	next_line();
}

void do_trace(void)
/**** do_trace
	toggles tracing or sets the trace bit of a udf.
****/
{	
	header *f;
	char name[64];
	scan_space();
	if (!strncmp(next,"off",3))
	{	
		trace=0; traceerrors=0; next+=3;
	}
	else if (!strncmp(next,"alloff",6))
	{	
		next+=6;
		f=(header *)ramstart;
		while ((char *)f<udfend && f->type==s_udf)
		{	f->flags&=~FLAGTRACE;
			f=nextof(f);
		}
		trace=0;
		traceerrors=0;
	}	
	else if (!strncmp(next,"on",2))
	{	
		trace=1; next+=2;
	}
	else if (!strncmp(next,"errors",6))
	{	
		traceerrors=1; next+=6;
	}
	else if (*next==';' || *next==',' || *next==0) trace=!trace;
	else
	{	
		if (*next=='"') next++;
		scan_name(name); if (error) return;
		if (*next=='"') next++;
		f=searchudf(name);
		if (!f || f->type!=s_udf)
		{	
			print("Function %s not found!\n",name);
			error=11021; return;
		}
		f->flags^=FLAGTRACE;
		if (f->flags&FLAGTRACE) output1("Tracing %s\n",name);
		else output1("No longer tracing %s\n",name);
		scan_space();
	}
	if (*next==';' || *next==',') next++;
}

void do_traceif ()
{
	char *oldram=newram;
	header *hd=scan_value();
	if (error) return;
	scan_space();
	if (*next==';' || *next==',') next++;
	hd=getvalue(hd);
	if (error) return;
	newram=oldram;
	if (hd->type==s_real)
	{
		trace=(*realof(hd)!=0.0);
	}
	else if (hd->type==s_string && *stringof(hd)!=1)
	{
		trace=1;
	}
	else
		need_arg_in("traceif","real value");
}

void do_protect ()
{	
	header *hd=(header *)ramstart;
	while ((char *)hd<udfend && hd->type==s_udf)
	{	
		if (strncmp(hd->name,"default",7))
			hd->flags|=PROTECT;
		hd=nextof(hd);
	}
	hd=(header *)startglobal;
	while ((char *)hd<endglobal)
	{	
		if (strncmp(hd->name,"default",7) && *(hd->name)!='%') 
			hd->flags|=PROTECT;
		hd=nextof(hd);
	}
	protectglobal();
}

/**
Set one mor more global variables as globally visible.
*/
void do_setglobal ()
{	
	if (udfon)
	{
		output("Cannot use setglobal inside a function.\n");
	}

	char name[maxname];
	int r;
	header *hd;
	while (1)
	{	
		scan_space(); scan_name(name); r=xor(name);

		if ((hd=globalfind(name))!=0) 
		{
			hd->flags|=GLOBAL;
		}
		else
		{
			hd=(header *)startglobal;
			while ((char *)hd<endglobal)
			{	
				if (r==hd->xor && !strcmp(hd->name,name)) break;
				hd=nextof(hd);
			}
			if ((char *)hd>=endglobal)
			{	
				output1("Global variable %s not found in \"setglobal\" command.\n",name);
				error=160; return;
			}
		
			hd->flags|=GLOBAL;
		}

		scan_space();
		if (*next!=',') break;
		else next++;
	}
}

int command_count;

void do_type (void);
void do_help (void);
void do_demo (void);
void do_example (void);
void do_addmenu (void);
void do_submenu (void);

commandtyp command_list[] =
	{
	{"quit",c_quit,do_quit},
	{"restart",c_noudf,do_restart},
	{"resetgraphics",c_udf,do_reset},
	{"hold",c_udf,ghold},
	{"shg",c_udf,show_graphics},
	{"load",c_noudf,load_file},
	{"stopload",c_noudf,stopload_file},
	{"function",c_noudf,get_udf},
	{"maximafunction",c_noudf,do_maximafunction},
	{"return",c_return,do_return},
	{"for",c_for,do_for},
	{"endif",c_endif,do_endif},
	{"end",c_end,do_end},
	{"continue",c_continue,do_continue},
	{"break",c_break,do_break},
	{"until",c_until,do_until},
	{"while",c_while,do_while},
	{"loop",c_loop,do_loop},
	{"else",c_else,do_else},
	{"elseif",c_elseif,do_elseif},
	{"if",c_if,do_if},
	{"then",c_then,do_then},
	{"repeat",c_repeat,do_repeat},
	{"clear",c_noudf,do_clear},
	{"clearall",c_noudf,do_clearall},
	{"remvalue",c_udf,do_remvalue},
	{"clg",c_udf,do_clg},
	{"cls",c_udf,do_cls},
	{"forget",c_noudf,do_forget},
	{"global",c_global,do_global},
	{"useglobal",c_global,do_useglobal},
	{"zerobase",c_global,do_base0},
	{"list",c_udf,do_list},
	{"listvar",c_udf,do_listvar},
	{"listvars",c_udf,do_listvar},
	{"listlists",c_udf,do_listlists},
	{"type",c_udf,do_type},
	{"dump",c_udf,do_dump},
	{"help",c_udf,do_help},
	{"mxmhelp",c_udf,do_mxmhelp},
	{"hexdump",c_udf,do_hexdump},
	{"output",c_udf,do_output},
	{"errors",c_udf,do_errors},
	{"underflows",c_udf,do_underflows},
	{"comments",c_noudf,do_comments},
	{"comment",c_noudf,do_comment},
	{"skipcomment",c_noudf,do_skipcomment},
	{"traceif",c_udf,do_traceif},
	{"trace",c_global,do_trace},
	{"prompt",c_global,do_prompt},
	{"path",c_udf,do_path},
	{"protect",c_udf,do_protect},
	{"maximamode",c_udf,do_maximamode},
	{"pythonmode",c_udf,do_pythonmode},
	{"yacasmode",c_udf,do_yacasmode},
	{"relax",c_udf,do_relax},
	{"addmenu",c_noudf,do_addmenu},
	{"submenu",c_noudf,do_submenu},
	{"easyunits",c_udf,do_easyunits},
	{"setglobal",c_noudf,do_setglobal},	
	{"matlab",c_udf,do_matlab},
	{"cd",c_udf,do_cd},
	{"largematrices",c_udf,do_largematrices},
	{"alias",c_noudf,do_alias},
#ifdef DLL
	{"tccompile",c_udf,do_tcc},
#endif
//	{"postscript",c_global,do_postscript},
	{0,c_none,0} };

int command_compare (const commandtyp *p1, const commandtyp *p2)
{	return strcmp(p1->name,p2->name);
}

void sort_command (void)
{	
	command_count=0;
	while (command_list[command_count].name) command_count++;
	qsort(command_list,command_count,sizeof(commandtyp),
		(int (*)(const void *, const void *))command_compare);
}

commandtyp *preview_command (int *l)
{	
	commandtyp h;
	char name[16],*a,*n;
	*l=0;
	a=next; n=name;
	while (*l<15 && xisalpha(*a)) { *n++=*a++; *l+=1; }
	*n=0; if (xisalpha(*a)) return 0;
	h.name=name;
	return (commandtyp *)bsearch(&h,command_list,command_count,sizeof(commandtyp),
		(int (*)(const void *, const void *))command_compare);
}

commandtyp *find_command (char *name)
{	
	commandtyp h;
	h.name=name;
	return (commandtyp *)bsearch(&h,command_list,command_count,sizeof(commandtyp),
		(int (*)(const void *, const void *))command_compare);
}

int lastcommand;
char *lastextra;

int builtin (void)
/***** builtin
	interpret a builtin command, return command number.
*****/
{	
	int l;
	commandtyp *p;
	int comn;
	if (udfon && *next==3)
	{	
		next++;
		memmove((char *)(&comn),next,sizeof(int));
		p=command_list+comn;
		lastcommand=comn;
		lastextra=next+sizeof(int);
		l=2*sizeof(int);
	}
	else p=preview_command(&l);
	if (p)
	{	
		if (udfon)
		{
			if (p->nr==c_noudf)
			{
				print("Cannot execute command %s in a function.\n",p->name);
				error=1; return 0;
			}
			else if (p->nr==c_udf)
			{
				char *p=next+l;
				while (*p==' ') p++;
				if (*p=='=' || strstarts(p,":="))
				{
					return 0;
				}
			}
		}
		next+=l;
		p->f();
		commandtype=p->nr;
		if (udfon!=2 && (*next==';' || *next==',')) next++;
		return 1;
	}
	return 0;
}

header *scan_expression (void)
/***** scan_expression
	scans a variable, a value or a builtin command.
*****/
{	if (builtin()) return &commandheader;
	return scan();
}

extern int enablemaxima;
#ifdef YACAS
extern int enableyacas;
#endif

extern int holding,allowsymbolicdp,script;
extern void do_python ();
extern int insimglines,insertsmaller;
extern int pythonmode;
extern char bom[];

void do_help (int maxima);

/**
Scan a command and interpret it.
Return, if the user wants to quit.
*****/
int command (void)
{	
	header *expr;
	int ret=c_none;
	quit=0; error=0; errorout=0; udf=0;
	int atlinestart=(next==input_line);

	// read over space
	while (1)
	{	
		scan_space();
		if (*next) break;
		else 
		{ 
			next_line(); 
			atlinestart=(next==input_line); 
		}
	}

	// end of a function reached
	if (*next==1) return ret;

	if (*next == bom[0] && *(next + 1) == bom[1] && *(next + 2) == bom[2])
	{
		next += 3;
		output1("Cannot interpret UTF-8 commands!\nNext command: %s\n",next);
		error = 1; return 0;
	}
	
#ifdef YACAS
	if (!strncmp(next,"> ",2) && next==input_line)
	{	
		if (!enableyacas)
		{	
			output("Yacas is disabled!\n");
			next=input_line; input_line[0]=0;
			return 0;
		}		
		else if (!udfon)
		{	
			next+=1; 
			if (*next=='>') next+=1;
			do_yacas();
		}
		else 
		{	
			output("Yacas > not allowed in functions!\n");
			error=1;
		}
		return 0;
	}
	else if (yacasmode==1 && next==input_line 
		&& strncmp(next,"yacasmode",9) && strncmp(next,"euler ",6)
		&& strncmp(next,"restart",7) && strncmp(next,"quit",5))
	{	
		if (!udfon)
		{	
			do_yacas();
		}
		else 
		{	
			output("Yacas not allowed in functions!\n");
			error=1;
		}
		return 0;
	}
	else 
#endif

	// skip over folding or invisible line markers
	if (atlinestart && (strncmp("%% ",next,3)==0 || strncmp("%+ ",next,3)==0)) 
		next+=3;

	if (atlinestart && strncmp("::: ",next,4)==0)
	{	
		if (!udfon)
		{	next+=4; do_maxima();
		}
		else 
		{	output("Maxima ::: not allowed in functions!\n");
			error=1;
		}
		return 0;
	}
	else if (atlinestart && 
		((!allowsymbolicdp && strncmp(next,"::",2)==0) ||
		strncmp(next,":: ",3)==0))
	{	
		if (!udfon)
		{	
			next+=2; do_compatible_maxima();
		}
		else 
		{	
			output("Maxima :: not allowed in functions!\n");
			error=1;
		}
		return 0;
	}
	else if (atlinestart && !strncmp(next,"?? ",3))
	{	
		next+=3;
		do_help(1);
		return 0;
	}
	else if (atlinestart && !strncmp(next,"maxima ",7))
	{	
		if (!udfon)
		{	
			next+=7; do_compatible_maxima();
		}
		else 
		{	
			output("Maxima not allowed in functions!\n");
			error=1;
		}
		return 0;
	}
	else if (atlinestart && (pythonmode && !udfon) 
		&& strncmp(next,"pythonmode",10) && strncmp(next,"function ",9) && strncmp(next,"euler ",6))
	{	
		do_python();
		return 0;
	}
	else if (atlinestart && (!strncmp(next,"py: ",4) || !strncmp(next,">> ",3)))
	{	
		if (!udfon)
		{	
			if (*(next+3)==' ') next+=4; 
			else next+=3;
			do_python();
			return 0;
		}
		else 
		{	
			output("Python command lines not allowed in functions!\n");
			error=1;
		}
		return 0;
	}
#ifdef YACAS
	else if (atlinestart && !strncmp(next,"yacas ",6))
	{	
		if (!udfon)
		{	
			next+=6; do_yacas();
		}
		else 
		{	
			output("Yacas not allowed in functions!\n");
			error=1;
		}
		return 0;
	}
#endif
	else if (!strncmp(next,"restart",(int)strlen("restart")) ||
		!strncmp(next,"quit",(int)strlen("quit")))
	{	
		maximamode=0; yacasmode=0; pythonmode=0;
	}
	else if (maximamode==1 && atlinestart
		&& strncmp(next,"maximamode",(int)strlen("maximamode"))
		&& strncmp(next,"euler ",(int)strlen("euler ")))
	{	
		if (!udfon)
		{	
			do_compatible_maxima();
		}
		return 0;
	}
	else if (*next==':' && *(next+1)!=':' && atlinestart)
	{	
		if (!enablemaxima)
		{	output("Maxima direct mode disabled!\n");
			next=input_line; input_line[0]=0;
			error=1;
		}		
		else if (!udfon)
		{	next+=1; do_maxima();
		}
		else 
		{	output("Maxima : not allowed in functions!\n");
			error=1;
		}
		return 0;
	}
	else if (maximamode==2 && atlinestart
		&& strncmp(next,"maximamode",(int)strlen("maximamode"))
		&& strncmp(next,"euler ",(int)strlen("euler ")))
	{	
		if (!udfon)
		{	do_maxima();
		}
		return 0;
	}
	else if (!strncmp(next,"euler ",(int)strlen("euler ")))
	{	
		next+=(int)strlen("euler ");
	}
	else if (!strncmp(next,"usenan ",(int)strlen("usenan ")))
	{
		pnan=1;
		usenan=1;
		next+=(int)strlen("usenan ");
	}
	else if (*next=='/' && *(next+1)=='*')
	{	
		next+=2; do_skipcomment();
		return 0;
	}

	// scan the next expression
	bracketlevel=0;
	needbracket=0;

	expr=scan_expression();
	
	// should not happen
	if (!expr) 
	{	
		newram=endlocal;
		return ret; 
	}

	// scanning returned an error
	if (error)
	{	
		newram=endlocal;
		flog("Error in %s",input_line);
		print_error(next);
		if (script) 
		{
			quit=2;
			// show_script_error();
		}
		next=input_line; input_line[0]=0;
		holding=0; pnan=0;
		inloop=0;
		return ret;
	}

	// command was executed
	if (expr==&commandheader)
	{	
		newram=endlocal;
		if (*next==':' && !udfon && !inloop)
		{
			next++;
			insert_image(insimglines-insertsmaller*10,"",1,0,1,0,1);
		}
		return commandtype;
	}
	
	// test for prefix command
	if ((char *)nextof(expr)==newram 
		&& expr->type==s_reference && !searchvar(expr->name))
	{
		header *hdudf=searchudf(expr->name);
		if (hdudf && hdudf->flags&PREFIX && *next!='=' && !strstarts(next,":="))
		{
			header *hd=scan();
			if (error) return 0;
			interpret_udf(hdudf,hd,1,1);
			if (error) return 0;
			moveresult(expr,hd);
		}
	}

	int insimg=0; // flag for the : after the command

	// handle expression (print or assign)
	switch (expr->type)
	{	
		case s_real :
		case s_complex :
		case s_matrix :
		case s_cmatrix :
		case s_imatrix :
		case s_string :
		case s_interval :
		case s_smatrix :
		case s_cpxmatrix :
		case s_list :
		case s_binary :
			if (*next=='=' || (*next==':' && *(next+1)=='='))
			{	
				print("Cannot assign to a value of type %s.\n",s_names[expr->type]);
				error=1000;
			}
			else
			{	
				insimg=!udfon && (!strcmp(next,":") || !strcmp(next,": ") 
					|| !strncmp(next,": //",3));
				if (*next==')' || *next==']')
				{	
					print("Found too many closing brackets, excessive %c\n",*next);
					error=1000;
				}
				else if (*next==0 || *next==1 || *next==',')
				{	
					if (*next==',') next++;
					give_out(expr);
				}
				else if (*next==';' || insimg)
				{
					next++;
				}
				else if (spaces && *next!=3)
				{	
					print("Commands must be separated by semicolon or comma!\nFound: %s (character %d)\n"
						"You can disable this in the Options menu.\n",next,(int)(*next));
					error=1000; break;
				}
				if (spaces && *next && *next!=' ' && *next!=9 && *next!=3 && strncmp(next,"...",3))
				{	
					print("Space between commands expected!\nFound: %s (character %d)\n"
						"You can disable this in the Options menu.\n",next,(int)(*next));
					error=1000; break;
				}
			}
			if (!udfon && !inloop && !error)
			{	
				header *last=searchvar("%");
				if (last) assign(last,expr);
				else
				{	last=new_reference(0,"%");
					assign(last,expr);
				}
			}
			break;
		case s_reference :
		case s_submatrix :
		case s_csubmatrix :
		case s_isubmatrix :
		case s_ssubmatrix :
			do_assignment(expr);
			break;
		default : break;
	}

	// test for error in command or assignment
	if (error)
	{	
		print_error(next);
		pnan=0; holding=0;
	}
	
	// clear stack
	newram=endlocal;

	// test : after command
	if (insimg)
	{
		insert_image(insimglines-insertsmaller*10,"",1,0,1,0,1);
	}

	if (error)
	{	
		next=input_line; input_line[0]=0; resetgraphics();
	}

	if (usenan)
	{
		pnan=0;
		usenan=0;
	}

	return ret;
}

/******************* main functions ************************/

void clear_fktext (void)
{	int i;
	for (i=0; i<10; i++) fktext[i][0]=0;
}

extern int startinmaximamode,usedirectmode;

int argcount=0;
char **argvector=0;

void meulerarg (header *hd)
{
	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real) need_arg_in("eulerarg","integer");
	int n=(int)(*realof(hd));
	if (n<1) need_arg_in("eulerarg","integer");
	if (n>argcount || !argvector)
	{
		print("Argument %d not known (got %d)!\n",
			n,argcount);
		error=1; return;
	}
	result=new_string(argvector[n-1],(int)strlen(argvector[n-1]),"");
	moveresult(st,result);
}

void main_loop (int argc, char *argv[])
{	
restart :
	flog("EMT restarted (Stack size %ld)",ramend-ramstart);
	clear_screen(); 
	quit=0; path[0]=currentdir;
	setstartdir();
	resetall(); 
	dostartup(); // Maxima and Yacas
	npath=0;
	newram=startlocal=endlocal=ramstart;
	udfend=ramstart;
	invalidate_udflist();
	init_globalvars();
	changedepsilon=epsilon=10000*DBL_EPSILON;
	sort_builtin(); sort_command(); make_xors(); clear_fktext();
	accuinit();
	next=input_line; *next=0;		/* clear input line */
	strcpy(input_line,"load \"euler.cfg\"; ");
	if (argc>1)
	{	strcat(input_line,"cd(home()); ");
	}
	if (argc>=2 && (int)strlen(input_line)+(int)strlen(argv[1])+20<=MAXLINE)
	{
		if (argv[1][0]=='"') strcat(input_line," load ");
		else strcat(input_line," load \"");
		strcat(input_line,argv[1]);
		if (argv[1][0]=='"') strcat(input_line,";");
		else strcat(input_line,"\";");
	}
	if (argc>2) argvector=argv+2;
	else argvector=0;
	argcount=argc-2;
	if (argcount<0) argcount=0;
	argc=1;
	if (!script && startinmaximamode) strcat(input_line," maximamode,");
	if (script) strcat(input_line," quit;");
	while (!quit)
	{   
		startglobal=startlocal;
		endglobal=endlocal;
		command();	// interpret until "quit"
		usematlab=matlab;
		if (trace<0) trace=0;
	}
	if (quit==2) 
	{	
		usematlab=0;
		matlab=0;
		maximamode=0;
		pythonmode=0;
		clear_notebook();
		goto restart;
	}
}
