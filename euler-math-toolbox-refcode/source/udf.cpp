#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>

#ifdef WIN32
#include "windows\windows.h"
int getstacksize();
#ifdef WIN64
int maxstacksize=64000000;
#else
int maxstacksize=16000000;
#endif
#endif

#ifndef MAXUDF
#ifdef WIN64
#define MAXUDF 1600
#else
#define MAXUDF 400
#endif
#endif

#include "header.h"
#include "sysdep.h"
#include "stack.h"
#include "help.h"
#include "yacas.h"
#include "maxima.h"
#include "builtin.h"
#include "getvalue.h"
#include "udf.h"
#include "express.h"
#include "globalvars.h"

#include "python.h"

// ************** error output *****************

/**
Print the number of needed and received arguments for a builtin function.
*/
void function_error (char *name, int argn)
{	
	builtintyp *h=find_builtin(name,-1);
	if (h)
	{	
		if (h->nargs==1) 
			output1("Built-in function %s needs 1 argument (got %d)!\n",
				name,argn);
		else 
			output1("Built-in function %s needs %d arguments (got %d)!\n",
				name,h->nargs,argn);
		return;
	}
	output1("Function %s not found.\nTry list ... to find functions!\n",name);
}

// ***************** UDF handling ****************

header *running;

#define NTYPES 18
int typeflags[NTYPES]={ParamReal,ParamComplex,ParamInterval,ParamString,
ParamMatrix,ParamVector,ParamColumn,ParamScalar,ParamCPX,ParamInteger,
ParamPositive,ParamNonNegative,
ParamInteger+ParamPositive+ParamScalar,ParamInteger+ParamPositive+ParamVector,
ParamInteger+ParamNonNegative+ParamScalar,ParamReal+ParamScalar,ParamNone,
ParamCall};

char *typenames[NTYPES]={"real","complex","interval","string",
"numerical","vector","column","scalar","cpx","integer",
"positive","nonnegative",
"index","indices","natural","number","none",
"call"};

extern int alwaysrelax;
int defineasrelax=0;

// Format a UDF line for user output
char *type_udfline (char *start, int compact)
{	
	static char outline[MAXLINE];
	char *p=start,*q;
	double x;
	int comn;
	q=outline;
	int cont=0;
	if (compact)
		while (*p==' ' || *p==9) p++;
	while (*p)
	{	
		if (*p==2)
		{	p++;
			x=*((double *)p);
			p+=sizeof(double);
			sprintf(q,"%g",x);
			q+=strlen(q);
		}
		else if (*p==3)
		{	p++;
			comn=*((int *)p);
			p+=2*sizeof(int);
			sprintf(q,"%s",command_list[comn].name);
			q+=strlen(q);
		}
		else *q++=*p++;
		if (q>outline+60)
		{	
			strcpy(q," ...");
			q+=strlen(q);
			cont=1;
			break;
		}
	}
	*q=0;
	output(outline); output("\n");
	if (cont) return p;
	else return p+1;
}

char *format_udfline (char *start)
{	
	static char outline[MAXLINE];
	char *p=start,*q;
	double x;
	int comn;
	q=outline;
	while (*p)
	{	if (*p==2)
		{	p++;
			x=*((double *)p);
			p+=sizeof(double);
			sprintf(q,"%g",x);
			q+=strlen(q);
		}
		else if (*p==3)
		{	p++;
			comn=*((int *)p);
			p+=2*sizeof(int);
			sprintf(q,"%s",command_list[comn].name);
			q+=strlen(q);
		}
		else *q++=*p++;
		if (q>outline+MAXLINE-2)
		{	q=outline+MAXLINE-1;
			break;
		}
	}
	*q=0;
	return outline;
}

void minput (header *);

void trace_udfline (char *next)
{	
	int scan,oldtrace;
	extern header *running;
	header *hd,*res;
	output1("%s: ",running->name); type_udfline(next,1);
again: 
	wait_key(&scan);
	switch (scan)
	{	
		case fk1 :
		case cursor_down :
			break;
		case fk2 :
		case cursor_right :
			trace=2; break;
		case fk3 :
		case cursor_up :
			trace=0; break;
		case fk4 :
		case help :
			hd=(header *)newram;
			oldtrace=trace; trace=0;
			new_string("Expression",12,""); if (error) goto cont;
			minput(hd); if (error) goto cont;
			res=getvalue(hd); if (error) goto cont;
			give_out(res);
			cont : newram=(char *)hd;
			trace=oldtrace;
			goto again;
		case fk9 :
		case escape :
			output("Trace interrupted\n"); error=11010; break;
		case fk10 :
		case cursor_left :
			trace=-1; break;
		default :
			output(
				"\nKeys :\n"
				"cursor_down   Single step\n"
				"cursor_right  Step over subroutines\n"
				"cursor_up     Go until return\n"
				"insert        Evaluate expression\n"
				"escape        Abort execution\n"
				"cursor-left   Stop tracing\n\n");
			goto again;
	}
}

#define MAXYSTRING MAXLINE
char ystring[MAXYSTRING];
char oneline[MAXLINE];

extern int printcomments,allowoverwrite,nextfunctioniscomment;

void get_udf ()
/***** get_udf
	defines a user defined function from the input.
*****/
{	
	char name[maxname],argu[maxname],type[maxname],retvar[maxname],
		*p,*firstchar,*startp;
	static char mxmline[MAXLINE]; // in case of ::=
	int haveparam=0;
	int *ph,*phn,*phh,*psemicolon,count=0,n;
	int l,comment;
	header *var,*result,*hd;
	FILE *actfile=infile;
	commandtyp *com;
	int comn;
	char *oldnext=0;
	char *onelinehelp=0;
	double x;
	int matlab=0;
	int allowassigned=0;

	// switch on the simple udf mode for the interface.
	udf=1;

	// do not define a function inside a function!
	if (udfon==1)
	{	
		output("Cannot define a function in a function!\n");
		error=60; clearudfinput(); return;
	}
	// check line length for oneline functions
	if (strlen(next)+strlen(oneline)>MAXLINE-16)
	{	
		output("Line too long!\n");
		error=60; clearudfinput(); return;
	}
	// note the content of the command line for oneline functions
	strcpy(oneline,next);

	// parse function name
	skip_space(); scan_name(name);
	if (error) 
	{	clearudfinput(); return;
	}

	int tinyc=0,commentonly=nextfunctioniscomment,overwrite=0,strong=0,
		infix=0,prefix=0,prefixline=0,postfix=0,map=0,args=0,python=0;
	
	skip_space();
	if (strcmp(name,"tinyc")==0)
		// This is a tinyc function.
		// Generate a comment function and start tinyc.
	{
		tinyc=1; skip_space(); scan_name(name);
		if (error) 
		{	clearudfinput(); return;
		}
		goto tinyc1;
	}
	else if (strcmp(name,"python")==0)
	{
		skip_space();
		if (*next==0 || strstarts(next,".."))	
		{
			get_python();
			if (error) clearudfinput();
			udf=0;
			return;
		}
		python=1; scan_name(name);
		goto python1;
	}

	// Scan comment, overwrite, strong, operator etc.

	if (strcmp(name,"comment")==0)
	{	
		commentonly=1; skip_space(); scan_name(name); 
		if (error) 
		{	clearudfinput(); return;
		}
	}
	
	if (strcmp(name,"overwrite")==0)
	{	
		overwrite=1; skip_space(); scan_name(name); 
		if (error) 
		{	clearudfinput(); return;
		}
	}
	
	if (strcmp(name,"strong")==0)
	{	
		strong=1; skip_space(); scan_name(name);
		if (error) 
		{	clearudfinput(); return;
		}
	}

	if (strcmp(name,"operator")==0)
	{	
		infix=1; skip_space(); scan_name(name);
		if (error) 
		{	clearudfinput(); return;
		}
	}
	
	if (strcmp(name,"postfix")==0)
	{	
		if (infix)
		{
			print("\"operator\" and \"postfix\" not possible.\n");
			error=1; clearudfinput(); return;
		}
		postfix=1; skip_space(); scan_name(name);
		if (error) 
		{	clearudfinput(); return;
		}
	}

	if (strcmp(name,"prefix")==0)
	{	
		if (infix || postfix || strong)
		{
			print("\"prefix\" cannot be used with \"operator\", \"prefix\", or \"strong\".\n");
			error=1; clearudfinput(); return;
		}
		prefix=1; skip_space(); scan_name(name);
		if (error) 
		{	clearudfinput(); return;
		}
	}

	if (strcmp(name,"prefixline")==0)
	{	
		if (infix || postfix || strong)
		{
			print("\"prefix\" cannot be used with \"operator\", \"prefix\", or \"strong\".\n");
			error=1; clearudfinput(); return;
		}
		prefixline=1; skip_space(); scan_name(name);
		if (error) 
		{	clearudfinput(); return;
		}
	}

tinyc1:

	if (strcmp(name,"map")==0)
	{	
		map=1; skip_space(); scan_name(name);
		if (error) 
		{	clearudfinput(); return;
		}
	}
	
	if (strcmp(name,"args")==0)
	{	
		args=1; skip_space(); scan_name(name);
		if (error) 
		{	clearudfinput(); return;
		}
	}

	if (strcmp(name,"allowassigned")==0)
	{	
		allowassigned=1; skip_space(); scan_name(name);
		if (error) 
		{	clearudfinput(); return;
		}
	}

python1:

	// function names cannot end with $
	int nl=strlen(name);
	if (nl>0 && name[nl-1]=='$')
	{
		print("Cannot name a function %s\n",name);
		clearudfinput(); error=1; return;
	}
	
	// Scan argument list:

	strcpy(mxmline,name); strcat(mxmline,"(");

	skip_space();
	if (*next=='=' && usematlab)
	{
		strcpy(retvar,name);
		next++;
		skip_space();
		scan_name(name);
		if (error) 
		{	
			clearudfinput(); return;
		}
		skip_space();
		matlab=1;
	}
	skip_space();
	if (*next=='=')
	{	
		clearudfinput(); 
		output1("Error \"=\" in definition of function %s\n",name);
		error=60; return;
	}

	// Kill global string variable with the same name
	header *hds=globalfind(name);
	if (hds && hds->type==s_string && (hds->flags&=SYMBOLIC)) 
	{
		globalremove(name);
	}

	if (nextfunctioniscomment)
	{
		char hname[maxname+1]="&";
		strcat(hname,name);
		strcpy(name,hname);
	}

	// look up, if there is already such a function.
	// care for protected functions.
	if (!overwrite && !allowoverwrite)
	{	
		header *old=searchudf(name);
		// if so, check for protected functions
		if (old  && old->flags&PROTECT)
		{	
			clearudfinput();
			output1("\nTrying to overwrite protected function %s!\n",name);
			error=1; return;
		}
		else if (!commentonly && find_builtin(name)) // or builitin functions
		{	
			clearudfinput();
			output1("\nTrying to overwriting builtin function %s!\n",name);
			error=1; return;
		}
	}

	kill_udf(name); // kill the old function definition

	var=new_reference(0,name); if (error) return; // create an empty reference
	result=new_udf(""); if (error) return; // create a new function
	p=udfof(result);  // start of funciton header and body
	int newline=scan_space();
	
	// leave space for the parameter count
	ph=(int *)p; *ph=0; p+=sizeof(inttyp);
	// leave space for the number of needed parameters
	phn=(int *)p; *phn=0; p+=sizeof(inttyp);
	// leave space for the position of the semicolon
	psemicolon=(int *)p; *psemicolon=0; p+=sizeof(inttyp);
	
	int havedefault=0;
	int dcount=0;
	int varargs=0;

	// scan parameters
	int invector=0;
	if (*next=='(')
	{	
		while(1)
		{	
			next++; // skip "(", or later ","
			scan_space();
			if (*next==')')
			{	
				if (invector)
				{	
					clearudfinput();
					output("Parameter vector not closed!\n");
					error=1; goto aborted;
				}
				break; // end of parameter list found
			}
			// leave space for parameter flags (like ParamHasValue)
			phh=(int *)p; *phh=0; p+=sizeof(inttyp);
			// check for a vector parameter
			if (*next=='[')
			{	
				*phh|=ParamStartsVector; invector=1;
				next++; dcount++;
			}
			// scan argument name
			scan_space(); 
			if (*next==0) next_simple_line();
			scan_space();
			scan_name(argu); if (error) { clearudfinput(); goto aborted; }
			if (haveparam) strcat(mxmline,",");
			haveparam=1;
			strcat(mxmline,argu);
			// increase parameter count
			count++;
			if (!havedefault && !invector) dcount++;
			// denote the parameter name
			strcpy(p,argu); p+=maxname;
			// denote the xor'ed parameter name
			*((int *)p)=xor(argu); p+=sizeof(inttyp);
			// look for next parameter or default value
			test : scan_space();
			if (*next==')')
			{	
				if (invector)
				{	
					clearudfinput();
					output("Parameter vector not closed!\n");
					error=1; goto aborted;
				}
				strcat(mxmline,")");
				break; // end of parameter list found
			}
			else if (strstarts(next,", ...)"))
			{	
				if (invector)
				{	
					clearudfinput();
					output("Parameter vector not closed!\n");
					error=1; goto aborted;
				}
				strcat(mxmline,")");
				next+=5;
				varargs=1;
				break; // end of parameter list found
			}
			else if (strstarts(next,",...)"))
			{	
				if (invector)
				{	
					clearudfinput();
					output("Parameter vector not closed!\n");
					error=1; goto aborted;
				}
				strcat(mxmline,")");
				next+=4;
				varargs=1;
				break; // end of parameter list found
			}
			else if (*next=='=' || !strncmp(next,":=",2)) // default value
			{	
				if (invector)
				{	
					clearudfinput();
					output("Vector parameter cannot have default value!\n");
					error=1; goto aborted;
				}
				if (!havedefault) dcount--;
				havedefault=1; 
				if (*next=='=') next++;
				else next+=2;
				// include ParamHasValue into parameter flags
				*phh|=ParamHasValue;
				// set newram here
				newram=p;
				hd=(header *)p;
				// scan the default value
				scan_value(); 
				if (error) { clearudfinput(); goto aborted; }
				// copy the parameter name into the value header
				strcpy(hd->name,argu);
				hd->xor=xor(argu);
				// Is the value none?
				if (hd->type==s_string && *stringof(hd)==1 && *(stringof(hd)+1)==0)
					*phh|=ParamNone;
				// update function position to the end of the value
				p=newram;
				// look for next parameter (not elegant)
				goto test;
			}
			else if (*next==',')
			{	continue; // another argument
			}
			else if (*next==';')
			{	if (!map)
				{	
					clearudfinput();
					output("Semicolon parameters only in map functions!\n");
					error=1; goto aborted;
				}
				if (*psemicolon>0)
				{	
					clearudfinput();
					output("Only one semicolon in parameter list allowed!\n");
					error=1; goto aborted;
				}
				*psemicolon=count;
				continue;
			}
			else if (*next==':')
			// scan parameter type
			{	
				if (invector)
				{	
					clearudfinput();
					output("Vector parameter is always scalar!\n");
					error=1; goto aborted;
				}
				next++; // skip ":"
				scan_space();
				while (1)
				{	
					if (*next==',' || *next==';' || *next==')' || *next=='=') 
						goto test;
					scan_name(type); 
					if (error) 
					{ 
						clearudfinput(); goto aborted; 
					}
					int found=0;
					for (int k=0; k<NTYPES; k++)
					{	
						if (!strcmp(type,typenames[k]))
						{	*phh|=typeflags[k]; found=1;
							break;
						}
					}
					if (!found)
					{	
						clearudfinput();
						output1("Illegal parameter type %s!\n",type); error=701;
						goto aborted;
					}
					scan_space();
				}
			}
			else if (*next==']')
			{	next++;
				if (!invector)
				{	
					clearudfinput();
					output("Closing ] found!\n");
					error=1; goto aborted;
				}
				*phh|=ParamEndsVector;
				invector=0;
				goto test;
			}
			else // syntax error
			{	
				clearudfinput();		
				output("Syntax error in parameter list!\n"); error=701;
				goto aborted;
			}
		}
		next++; // skip ")"
	}
	else if (!newline && *next!=0 && strncmp(next,":=",2) && strncmp(next,"&=",2))
	{
		clearudfinput();
		print("Error in definition of %s.\n",name);
		error=1; goto aborted;
	}
	// denote the parameter count
	*ph=count;
	*phn=dcount;

	// read in the function body
	scan_space();
	if (*next==0) // no one line function
	{	next_simple_line(); 
	}
	else 
	{	
		if (!strncmp(next,"::=",3))	// Euler and Maxima function
		{	
			next+=3;
			// skip unnecessary return as in "function f(x) := return x^2"
			if (strncmp(next," return ",8)==0) next+=8;
			// check maximal length
			if (strlen(next)>980)
			{	
				output("One line function too long!\n");
				error=1; goto aborted;
			}
			// chop of comments //... and help line ##...
			char *p=next;
			while (*p)
			{	
				if (!strncmp(p,"//",2) || !strncmp(p,"##",2)
					 || (usematlab && !strncmp(p,"% ",2)))
				{	*p=0; 
					// remember the position of the help line
					onelinehelp=p+2; 
					break;
				}
				p++;
			}
			static char parsed[MAXLINE];
			scan_space();
			int nparsed=scan_maxima_expression(parsed,MAXLINE);
			if (nparsed==0 || error)
			{
				error=10; goto aborted;
			}
			char *mxmresult=parsed;
			if (error) goto aborted;
			// update the one line function to look like a usual function
			// add useglobal and return
			strcpy(oneline,"useglobal; return ");
			// add return command
			strcat(oneline,mxmresult);
			// add endfunction
			strcat(oneline," endfunction");
			// skip the one line definition in the input line
			oldnext=next;
			// set the input line to the updated oneline
			next=oneline;
			// define the function body in Maxima
			// the definition will look like "f(x) ::= x^2$"
			// strcat(oneline,"$");
			strcat(mxmline," := ");
			strcat(mxmline,mxmresult);
			strcat(mxmline,"$");
			// output1("%s\n",mxmline);
			runmaxima(mxmline,4,1);
		}
		else if (!strncmp(next,"&=",2)) // symbolic function
		{
			result->flags|=SYMBOLIC;
			next+=2;
			// skip unnecessary return as in "function f(x) := return x^2"
			if (strncmp(next," return ",8)==0) next+=8;
			// check maximal length
			if (strlen(next)>980)
			{	
				output("One line function too long!\n");
				error=1; goto aborted;
			}
			// chop of comments //... and help line ##...
			char *p=next;
			while (*p)
			{
				if (!strncmp(p,"//",2) || 
					!strncmp(p,"##",2) || (usematlab && !strncmp(p,"% ",2)))
				{	
					*p=0; 
					// remember the position of the help line
					onelinehelp=p+2;
					break;
				}
				p++;
			}
			static char parsed[MAXLINE];
			scan_space();
			int nparsed=scan_maxima_expression(parsed,MAXLINE);
			// print("parsed: %s\n",parsed);
			if (nparsed==0 || error)
			{
				output("Function body missing!\n");
				error=10; goto aborted;
			}
			char *mxmresult=callmxm(parsed,5,1);
			if (*next!=';' && strlen(mxmresult)<MAXLINE-2)
			{
				static char sm[MAXLINE];
				strcpy(sm,mxmresult);
				strcat(sm,";");
				callmaxima(sm,4);
			}
			if (error) goto aborted;
			// update the one line function to look like a usual function
			// add useglobal and return
			strcpy(oneline,"useglobal; return ");
			// add return command
			strcat(oneline,mxmresult);
			// add endfunction
			strcat(oneline," endfunction");
			// skip the one line definition in the input line
			oldnext=next;
			// set the input line to the updated oneline
			next=oneline;
			// define the function body in Maxima
			// the definition will look like "f(x) := x^2$"
			// strcat(oneline,"$");
			strcat(mxmline," := ");
			strcat(mxmline,mxmresult);
			strcat(mxmline,"$");
			// print("%s\n",mxmline);
			static char kill[MAXLINE];
			sprintf(kill,"kill(\"%s\")$",name);
			runmaxima(kill,4,0); // kill function
			runmaxima(mxmline,4,1); // define function
			if (infix)
			{
				if (strong) sprintf(mxmline,"infix(\"%s\",190,190)$",name);
				else sprintf(mxmline,"infix(\"%s\",10,10)$",name);
				// print("%s\n",mxmline);
				runmaxima(mxmline,4,0);
			}
			else if (postfix)
			{
				if (strong) sprintf(mxmline,"postfix(\"%s\",190)$",name);
				else sprintf(mxmline,"postfix(\"%s\",10)$",name);
				// print("%s\n",mxmline);
				runmaxima(mxmline,4,0);
			}
			else if (prefix)
			{
				sprintf(mxmline,"prefix(\"%s\",10)$",name);
				// print("%s\n",mxmline);
				runmaxima(mxmline,4,0);
			}
		}
		else if (!strncmp(next,"&&=",3)) // purely symbolic function
		{
			next+=3;
			// skip unnecessary return as in "function f(x) := return x^2"
			if (strncmp(next," return ",8)==0) next+=8;
			// check maximal length
			if (strlen(next)>980)
			{	
				output("One line function too long!\n");
				error=1; goto aborted;
			}
			// chop of comments //... and help line ##...
			char *p=next;
			while (*p)
			{
				if (!strncmp(p,"//",2) || 
					!strncmp(p,"##",2) || (usematlab && !strncmp(p,"% ",2)))
				{	
					*p=0; 
					// remember the position of the help line
					onelinehelp=p+2;
					break;
				}
				p++;
			}
			static char parsed[MAXLINE];
			scan_space();
			int nparsed=scan_maxima_expression(parsed,MAXLINE);
			if (nparsed==0 || error)
			{
				if (!error) output1("Function body for &&= missing!\n%s\n",next);
				error=10; goto aborted;
			}
			// define the function body in Maxima
			// the definition will look like "f(x) := x^2$"
			strcat(mxmline," := ");
			char *hparsed=replaceexpressions(parsed,1);
			strcat(mxmline,hparsed);
			strcat(mxmline,"$");
			if (*next!=';' && strlen(parsed)<MAXLINE-2)
			{
				static char sm[MAXLINE];
				strcpy(sm,"'(");
				strcat(sm,hparsed);
				strcat(sm,");");
				callmaxima(sm,0);
			}
			// print("%s\n",mxmline);
			static char kill[MAXLINE];
			sprintf(kill,"kill(\"%s\")$",name);
			runmaxima(kill,4,0); // kill function
			runmaxima(mxmline,4,1); // define function
			if (infix)
			{
				if (strong) sprintf(mxmline,"infix(\"%s\",190,190)$",name);
				else sprintf(mxmline,"infix(\"%s\",10,10)$",name);
				// print("%s\n",mxmline);
				runmaxima(mxmline,4,0);
			}
			else if (postfix)
			{
				if (strong) sprintf(mxmline,"postfix(\"%s\",190)$",name);
				else sprintf(mxmline,"postfix(\"%s\",10)$",name);
				// print("%s\n",mxmline);
				runmaxima(mxmline,4,0);
			}
			else if (prefix)
			{
				sprintf(mxmline,"prefix(\"%s\",10)$",name);
				// print("%s\n",mxmline);
				runmaxima(mxmline,4,0);
			}
			goto aborted;
		}
		// one line function for Euler
		else if (!strncmp(next,":=",2))
		{	
			next+=2;
			// skip unnecessary return as in "function f(x) := return x^2"
			if (strncmp(next," return ",8)==0) next+=8;
			// check maximal length
			if (strlen(next)>980)
			{	
				output("One line function too long!\n");
				error=1; return;
			}
			// chop of comments //... and help line ##...
			char *p=next;
			while (*p)
			{	if (!strncmp(p,"//",2) || (usematlab && !strncmp(p,"% ",2)))
				{	*p=0; 
					// remember the position of the help line
					onelinehelp=p+2;
					break;
				}
				p++;
			}
			static char parsed[MAXLINE];
			scan_space();
			char *q=parsed;
			int b1=0,b2=0;
			while (true)
			{
				if (*next==';' && b1<=0 && b2<=0)
				{
					char *p=next+1;
					while (*p==' ' || *p==9) p++;
					if (strcmp(p,"...")==0) break;
				}
				if (!strncmp(" ..",next,3) || !strncmp("..",next,2))
				{	
					int oldudf=udf; udf=0; next_simple_line(); udf=oldudf; 
					scan_space();
				}
				if (q-parsed>MAXLINE-2)
				{
					print("One-line function too long.\n");
					error=10; goto aborted;
				}
				if (*next=='(') b1++;
				else if (*next=='[') b2++;
				else if (*next==']') b2--;
				else if (*next==')') b1--;
				*q++=*next++;
				if (*next==0) break;
			}
			*q=0;
			if (q==parsed)
			{
				if (!error) output("Function body missing!\n");
				error=10; goto aborted;
			}
			// print("%s\n",parsed);
			// define the function body in Maxima
			// the definition will look like "f(x) ::= x^2$"
			// update the one line function to look like a usual function
			// add useglobal and return
			strcpy(oneline,"useglobal; return ");
			// add return command
			strcat(oneline,parsed);
			// add endfunction
			strcat(oneline," endfunction");
			// skip the one line definition in the input line
			oldnext=next;
			// set the input line to the updated oneline
			next=oneline;
		}
		else if (*next=='=')
		{
			next++;
			print("Use :=, ::=, or &= in one-line function!\n");
			error=1; goto aborted;
		}
	}

	// add a comment to report the loading position of the function
	if (*currentfilename!=0) // from a file
	{	
		strcpy(p,"## Function in file : ");
		strcat(p,currentfilename);
		p+=strlen(p)+1;
		strcpy(p,"## ");
		p+=strlen(p)+1;
	}
	else // from the comman dline
	{	
		strcpy(p,"## Entered from command line. ");
		if (python) strcat(p,"Python function.");
		p+=strlen(p)+1;
		strcpy(p,"## ");
		p+=strlen(p)+1;
	}

	// if there was a help for the one line function, add it
	if (onelinehelp)
	{	
		if (!freeramfrom(p,strlen(onelinehelp)+10))
		{	
			clearudfinput();
			output("Memory overflow while defining a function!\n");
			error=210; goto stop;
		}
		*p++='#'; *p++='#';
		strcpy(p,onelinehelp);
		p+=strlen(p);
		*p++=0;
	}

	// scan the help section of the function
	while (1)
	{	
		if (!strncmp(next,"##",2) || (usematlab && !strncmp(next,"% ",2)))
		{	// add the help line to the function body
			while (*next)
			{	*p++=*next++;
				if (!freeramfrom(p,maxname))
				{	
					clearudfinput();
					output("Memory overflow while defining a function!\n");
					error=210; goto stop;
				}
			}
			*p++=0; next_simple_line();
		}
		else break; // last help comment added
		// check, if the function did not end within the file
		if (actfile!=infile)
		{	
			clearudfinput();
			output("End of file reached in function definition!\n");
			error=2200; goto stop;
		}
	}

	// denote the start of the statements in the function body
	*udfstartof(result)=(p-(char *)result);

	// remember the start of the statements in the function body
	startp=p;
	*p=0; *(p+1)=1;

	// remember the first character of the line
	firstchar=next;
	comment=0; // flag for being in a /* ... */ comment?

	if (tinyc) goto tinyc2;
	if (python) goto python2;

	// read in the function statements
	while (1)
	{	
		if (error) { clearudfinput(); goto stop; }
		// check for endfunction
		if (!comment && !strncmp(next,"endfunction",strlen("endfunction")))
		{	
			endfunction :
			if (matlab)
			{
				com=find_command("return");
				*p++=3;
				comn=com-command_list;
				*((int *)p)=comn;
				p+=sizeof(int);
				*((int *)p)=0;
				p+=sizeof(int);
				*p++=' ';
				strcpy(p,retvar);
				p+=strlen(p);
				strcpy(p,";");
				p+=strlen(p);
			}
			// end the statement line
			if (p==startp || *(p-1)) *p++=0;
			// denote the function end with char=1
			*p++=1; 
			// end the function definition
			next+=strlen("endfunction"); break;
		}
		// check, if the function did not complete within the file
		if (actfile!=infile)
		{	
			if (usematlab)
			{
				goto endfunction;
			}
			else
			{
				clearudfinput();
				output("End of file reached in function definition!\n");
				error=2200; goto stop;
			}
		}
		// read the statement
		if (comment) // in a comment, do nothing
		{	
			// but check for end of the comment
			if (*next=='*' && *(next+1)=='/')
			{	comment=0; next+=2;
			}
			else
			{	if (*next) next++;
				else { next_simple_line(); firstchar=next; }
			}
		}
		else if (!strncmp(next,"## ",3)) // skip comments
		{	
			while (*next) *p++=*next++;
			*p++=0;
			next_simple_line();
			firstchar=next;
		}
		else if (*next) // other things
		{	
			if (*next=='"' && !((*(next+1)=='@' || *(next+1)=='&') && *(next+2)==':'))
			// ordinary string (not Maxima string "@:...")
			{	
				*p++=*next++;
				while (*next!='"' && *next) *p++=*next++;
				if (*next=='"') *p++=*next++;
				else 
				{ 
					clearudfinput();
					output("\" missing.\n"); error=2200; goto stop; 
				}
			}
			else if (*next=='\'' && *(next+1)=='\'')
			// alternative form of a string
			{	
				*p++=*next++; *p++=*next++;
				while (*next && (*next!='\'' || *(next+1)!='\''))
					*p++=*next++;				
				if (*next=='\'') { *p++=*next++; *p++=*next++; }
				else 
				{ 
					clearudfinput();
					output("\'\' missing.\n"); error=2200; goto stop; 
				}
			}
			else if (xisdigit(*next) ||				     	
				(*next=='.' && xisdigit(*(next+1))) )
			// precompile numbers
			{	if (next!=firstchar && xisalpha(*(next-1)))
				{	
					*p++=*next++;
					while (xisdigit(*next)) *p++=*next++;
				}
				else
				{	// align for even positition (not necessary)
					// if ((p-(char *)result)%2==0) *p++=' ';
					// write 2 to signal a precompiled float
					*p++=2;
					// scan a number using sscanf
					sscanf(next,"%lg%n",&x,&n);
					// skip the number
					next+=n;
					// move the number to the function body
					*((double *)p)=x;
					p+=sizeof(double);
				}
			}
			else if (*next=='/' && *(next+1)=='*')
			// start a comment
			{	next+=2;
				comment=1;
			}
			else if (xisalpha(*next) &&
				(next==firstchar || !xisalpha(*(next-1))) &&
				(com=preview_command(&l))!=0 && com->nr!=c_udf)
			// try to find a builtin command
			{	
				*p++=3;
				comn=com-command_list;
				// move the pointer to the command description here
				*((int *)p)=comn;
				p+=sizeof(int);
				*((int *)p)=0;
				p+=sizeof(int);
				next+=l;
			}
			else if (*next=='.' && *(next+1)=='.')
			// skip the rest of the line
			{	
				*p++='.'; *p++='.'; *p++=0; 
				next_simple_line(); firstchar=next;
			}
			else if (!strncmp(next,"//",2) || (usematlab && !strncmp(next,"% ",2)))
			// one line comment, save it into the function body
			{	
				if (firstchar!=next) *p++=0; 
				next_simple_line(); firstchar=next;
			}
			else if (*next=='$' && next==firstchar)
			// read over "$" in first position of the line
			// this may happen with copy and paste
			{	
				next++;
			}
#ifdef YACAS
			else if (*next=='@' && *(next+1)=='>' && *(next+2)=='"')
			// precompile a string with Yacas
			{	next+=3;
				char *q=ystring;
				while (*next!='"' && *next && (q-ystring)<MAXYSTRING-1) *q++=*next++;
				if (*next=='"') next++;
				else 
				{ 
					clearudfinput();
					output("\" missing.\n"); 
					error=2200; goto stop; 
				}
				*q++=0;
				q=call_yacas(ystring);
				strcpy(p,q);
				p+=strlen(p);
			}
#endif
			else if ((*next=='@' || *next=='&') && *(next+1)==':')
			// precompile a string with Maxima, insert the Maxima result
			{	
				next+=2;
				if (scan_maxima_expression(ystring,MAXLINE)>0)
				{
					char *q=callmxm(ystring);
					strcpy(p,q);
					p+=strlen(p);
				}
				else 
				{
					clearudfinput();
					goto stop; 
				}
			}
			else if (*next=='"' && (*(next+1)=='@' || *(next+1)=='&') && *(next+2)==':')
			// precompile a string with Maxima, insert the result as string
			{	
				next+=3;
				char *q=ystring;
				while (*next!='"' && *next && (q-ystring)<255) *q++=*next++;
				if (*next=='"') next++;
				else 
				{ 
					clearudfinput();
					output("\" missing.\n"); error=2200; goto stop; 
				}
				*q++=0;
				q=callmxm(ystring);
				*p++='"';
				strcpy(p,q);
				p+=strlen(p);
				*p++='"';
			}
			else if (*next=='@')
			// replace @var with the content of var
			{	
				// scan the name of the variable
				char name[maxname];
				next++;
				scan_name(name);
				// search the variable
				header *hd=searchvar(name);
				if (!hd) // not found
				{
					clearudfinput();
					output1("Variable %s not found.\n",name); 
					if (udfon) output("Can only see global variables.\n");
					error=2200; goto stop;
				}
				// convert the content to the Maxima form 
				// (which Euler can also understand)
				char *qh=tomxm(hd,0,0);
				// denote the result into the function
				if (freeramfrom(p,strlen(qh)))
				{	
					strcpy(p,qh);
					p+=strlen(p);
				}
				else
				{	
					clearudfinput();
					output1("Wrong variable or memory overflow in @%s!\n",name); 
					error=2200; goto stop;
				}
			}
			else *p++=*next++; // simply a character
		}
		else // new line 
		{	
			*p++=0; next_simple_line(); 
			firstchar=next; 
		}
		// function was too long
		if (!freeramfrom(p,80))
		{	
			clearudfinput();
			output("Memory overflow while defining a function!\n");
			error=210; goto stop;
		}
	}

tinyc2:
python2:
stop:
	// for one line functions
	if (oldnext) next=oldnext;
	// switch off the udf flag for the interface
	if (error) { udf=0; return; }
	udf=0; 
	// use a good alignment for the header
	result->size=(((p-(char *)result)-1)/ALIGNMENT+1)*ALIGNMENT;
	// define this as a relaxed function
	if (defineasrelax || alwaysrelax) result->flags|=RELAX;
	// in Matlab mode, define a matlab function
	if (matlab) result->flags|=MATLAB;
	// set as mapped function
	if (map) result->flags|=MAP;
	// set comment only function
	if (tinyc || commentonly) result->flags|=COMMENT;
	// set variable argument flag
	if (varargs) result->flags|=VARARGS;
	// set allow assigned parameters
	if (allowassigned) result->flags|=ALLOWASSIGNED;
	// set infix and postfix
	if (infix) 
	{
		if (count!=2)
		{
			clearudfinput();
			print("Infix function %s must take two parameters.\n",name);
			error=1; goto aborted;
		}
		result->flags|=INFIX;
	}
	if (postfix)
	{
		if (count!=1)
		{
			clearudfinput();
			print("Postfix function %s must take one parameter.\n",name);
			error=1; goto aborted;
		}
		result->flags|=POSTFIX;
	}
	if (prefix)
	{
		if (count!=1)
		{
			clearudfinput();
			print("Prefix function %s must take one parameter.\n",name);
			error=1; goto aborted;
		}
		result->flags|=PREFIX;
	}
	if (prefixline)
	{
		if (count!=1)
		{
			clearudfinput();
			print("Prefix function %s must take one parameter.\n",name);
			error=1; goto aborted;
		}
		result->flags|=PREFIXLINE;
	}
	if (strong) result->flags|=STRONG;
	if (args) result->flags|=ARGS;

	// update the newram pointer
	newram=(char *)result+result->size;
	// assign the udf to the variable with the function "assign"
	if (tinyc)
	{
		get_tc(name,count);
	}
	if (python)
	{
		get_python_function(name,result);
		result->flags=result->flags|=PYTHONFLAG;
	}
	assign(var,result);
aborted:
	udf=0;
}

char * print_defaults (int defaults)
{	int hastype=0;
	static char out[512];
	out[0]=0;
	for (int k=NTYPES-1; k>=0; k--)
	{	if ((defaults&typeflags[k])==typeflags[k])
		{	if (!hastype)
			{	strcat(out,":"); hastype=1;
			}
			strcat(out," ");
			strcat(out,typenames[k]);
			defaults&=(0xFFFFFFFF^typeflags[k]);
		}
	}
	return out;
}

void do_type (void)
// type functions and variables
{	
	char name[maxname];
	scan_space();
	scan_name(name); if (error) return;
	if (*next==';' || *next==',') next++;

	do_type_name(name);
}

void do_type_name (char *name)
// type functions and variables
{	
	header *hd;
	char *p,*pnote;
	int i,count,defaults,argcount;
	builtintyp *b;
	hd=searchudf(name);
	b=find_builtin(name);
	int found=0;
	if (b)
	{   if (b->nargs>=0)
			output1(
				"%s is a builtin function with %d argument(s).\n"
				,name,b->nargs);
		else
			output1(
				"%s is a builtin function.\n"
				,name);
		found=1;
	}
	if (hd && hd->type==s_udf)
	{   
		found=1;
		if (b) output1("\n%s is also an Euler function.\n\n",name);
		if (hd->flags&MAP) output1("function map %s (",name);
		else output1("function %s (",name);
		p=helpof(hd);
		count=*((int *)p);
		p+=3*sizeof(inttyp);
		pnote=p;
		argcount=0;
		int ol=0;
		if (hd->flags&MAP) ol=strlen("function map  (")+strlen(name);
		else ol=strlen("function  (")+strlen(name);

		// print parameter
		for (i=0; i<count; i++)
		{	defaults=*((int *)p);
			p+=sizeof(inttyp);
			if (defaults&ParamStartsVector) output("[");
			output1("%s",p);
			ol+=strlen(p);
			p+=maxname+sizeof(inttyp);
			if (defaults&ParamHasValue)
			{	p=(char *)nextof((header *)p);
			}
			char *sd=print_defaults(defaults);
			output(sd); ol+=strlen(sd);
			if (defaults&ParamEndsVector) output("]");
			if (i!=count-1) { output(", "); ol+=2; }
			argcount++;
			if (ol>linelength-15)
			{	output(" ..\n");
				ol=0;
			}
		}
		output(")\n");

		p=pnote;
		int havedefaults=0;
		for (i=0; i<count; i++)
		{	defaults=*((int *)p);
			p+=sizeof(inttyp);
			if (defaults&ParamHasValue)
			{	if (!havedefaults) output("\n");
				output1("## Default for %s : ",p);
				havedefaults=1;
			}
			p+=maxname+sizeof(inttyp);
			if (defaults&ParamHasValue)
			{	
				header *hp=(header *)p;
				if (isnone(hp))	output("none\n");
				else
				{	
					if (hp->type==s_matrix || hp->type==s_cmatrix ||
						hp->type==s_imatrix || hp->type==s_smatrix ||
						hp->type==s_cpxmatrix) output("\n");
					give_out(hp);
				}
				p=(char *)nextof((header *)p);
			}
		}
		if (havedefaults) output("\n");
		p=udfof(hd);
		while (*p!=1 && p<(char *)nextof(hd))
			p=type_udfline(p,0);
		output("endfunction\n");
	}
	another:
	hd=searchvar(name);
	if (hd!=0)
	{	if (found) output("\n");
		found=1;
		output1("%s = ",name);
		if (hd->type!=s_real && hd->type!=s_complex && hd->type!=s_string
			&& hd->type!=s_interval) output("\n");
		hd=getvalue(hd); if (error) return;
		give_out(hd);
		if (*next==',')
		{	next++;
			scan_space();
			scan_name(name);
			if (error) return;
			found=0;
			goto another;
		}
	}
	if (!found) 
	{	output("No such function or variable!\n"); 
		error=173;
	}
}

int printdefaults=1;

char udfhelp[MAXLINE];

char * udf_gethelp (header *hd, int text=1)
// get a line of help from a udf
{	
	int count,defaults;
	char *p=helpof(hd);
	count=*((int *)p);
	p+=3*sizeof(inttyp);
	strcpy(udfhelp,"");
	if (hd->flags&COMMENT) strcat(udfhelp,"comment ");
	if (hd->flags&MAP) strcat(udfhelp,"map ");
	if (hd->flags&STRONG) strcat(udfhelp,"strong ");
	if (hd->flags&PREFIX) strcat(udfhelp,"prefix ");
	if (hd->flags&INFIX) strcat(udfhelp,"operator ");
	if (hd->flags&POSTFIX) strcat(udfhelp,"postfix ");
	if (hd->flags&ARGS) strcat(udfhelp,"args ");
	if (hd->flags&MATLAB) strcat(udfhelp,"matlab ");
	if (hd->flags&PYTHONFLAG) strcat(udfhelp,"python ");
	strcat(udfhelp,hd->name); 
	strcat(udfhelp," (");
	int havedefaults=0;
	for (int i=0; i<count; i++)
	{	
		defaults=*((int *)p);
		p+=sizeof(inttyp);
		if (defaults&ParamHasValue)
		{	if (!havedefaults) strcat(udfhelp," {");
			havedefaults=1;
		}
		if (i>0 && i<6) strcat(udfhelp,", ");
		if (i<5)
		{	
			if (defaults&ParamStartsVector) strcat(udfhelp,"[");
			strcat(udfhelp,p); 
			if (defaults&ParamEndsVector) strcat(udfhelp,"]");
			char *sd=print_defaults(defaults);
			strcat(udfhelp,sd);
		}
		else if (i==5) strcat(udfhelp,"...");
		p+=maxname+sizeof(inttyp);
		if (defaults&ParamHasValue)
		{	
			p=(char *)nextof((header *)p);
		}
	}
	if (havedefaults) strcat(udfhelp,"}");
	strcat(udfhelp,") ");
	if (text)
	{
		if (*p && p<udfof(hd))
		{	p+=strlen(p); p++;
		}
		if (*p && p<udfof(hd))
		{	p+=strlen(p); p++;
		}
		if (*p && p<udfof(hd)) 
		{	if (*(p+2))
			{	strcat(udfhelp,p+2); 
				return udfhelp;
			}
		}
		strcat(udfhelp,"Euler function has no help line!");
	}
	return udfhelp;
}

/****************** udf ************************/

void printlocalvars (char *startlocal)
{
	header *hd=(header *)startlocal;
	print("Local variables.\n");
	while ((char *)hd<newram)
	{
		print("\n%s (type=%d)\n",hd->name,hd->type);
		give_out(hd);
		hd=nextof(hd);
	}
	print("\n\n");
}

void minput1 (header *hd);

int udfcount=0;
extern int nosubmref,alwaysrelax;

char *currentfunction="";

void interpret_udf (header *var, header *args, int argn, int sp)
/**** interpret_udf
	interpret a user defined function.
	arg : total number of arguments
	sp : number of first assigned parameter
****/
{	
	// if (!strcmp(var->name,"test")) printlocalvars((char *)args);

	if (var->flags&PYTHONFLAG)
		// this is a Python function, it will not be interpreted by EMT.
	{
		header *st=args;
		header *hds[32];
		int n=0;
		header *hn=args;
		if (argn>0 && hn && (char *)args<newram)
		{
			while (hn && n<32)
			{
				hds[n++]=hn;
				hn=next_param(hn);
			}
		}
		for (int i=0; i<n; i++)
		{
			hds[i]=getvalue(hds[i]); if (error) return;
		}
		header *result=(header *)newram;
		py_eval(var->name,hds,n,&newram,ramend); if (error) return;
		moveresult(st,result);
		return;
	}

	int udfold,nargu,i,oldargn,defaults,oldtrace,oldindex,
		oldsearchglobal,oldsp,n,oldnosubmref,relaxold,oldusematlab;
	char *oldnext=next,*oldstartlocal,*oldendlocal,*udflineold,
		*p,*name;
	header *st=args,*hd=args,*hd1,*hd2,*oldrunning,*result;
	double oldepsilon,oldchanged;

	// find the function body where the arguments start
	p=helpof(var);
	
	// read the number of parameters the function was defined with
	nargu=*((int *)p); p+=sizeof(inttyp);
	
	// read the number of arguments that it needs
	int needargn;
	needargn=*((int *)p); p+=sizeof(inttyp);
	
	// skip position of semicolon (for map functions)
	p+=sizeof(inttyp);
	
	// check for too few arguments
	if (argn<needargn)
		// function got less arguments than it needs
	{	
		if (exec_builtin(var->name,argn,args)) return;
		if (needargn>1)
			output1("Function %s needs at least %d arguments!\n",var->name,needargn);
		else
			output1("Function %s needs at least one argument!\n",var->name);
		output1("Use: %s\n",udf_gethelp(var,0));
		error=1; return;
	}

	// check for too many arguments
	// except if the function had an eclipse ",...)"
	if (!(var->flags & VARARGS))
	{
		int gotn=argn;
		if (sp>0) gotn=sp;
		if (gotn>nargu && nargu>0)
		{
			print("Function %s needs only %d arguments (got %d)!\n",var->name,nargu,gotn);
			print("Use: %s\n",udf_gethelp(var,0));
			error=1; return;
		}
	}


	// remember the position of the first parameter after ";"
	// those parameters are reserved and accessible with args() only
	if (sp!=0) n=sp;
	else n=argn;

	// count the number of parameters of the function already used
	int nused=0;

	// remember the end of the arguments
	header *hdend=(header *)newram;
	
	// in the following loop,
	// give the provided arguments the names of the function parameters
	// hd now points to the provided n arguments
	// p points to the nargu parameters the function wants

	int invector=0;
	int paramflag;
	char *startargs=p;
	for (i=0; i<n; i++)
	{	
		startloop : 
		char *pname=p+sizeof(inttyp);

		if (!invector)
			// parameter was not of the form "...,[a,b],..."
		{	
			// store the value of this parameter to hd1
			if (hd->type==s_reference && !referenceof(hd))
				// empty parameter,
				// happens, if the user skipped this assigned parameter
				// as in "f(x,,y)"
			{	
				if (i<nargu && hd->name[0]==0 && (*((int *)p)&ParamHasValue))
				{	
					p+=maxname+2*sizeof(inttyp);
					nused++;
					moveresult((header *)newram,(header *)p);
					p=(char *)nextof((header *)p);
					hd=nextof(hd);
					continue;
				}
				else
					// normal parameter
				{	
					hd1=getvalue(hd); if (error) return;
					setname(hd,"");
				}
			}
			else hd1=hd; // get the value of the parameter passed to the function
		}

		/**
		if (!strcmp(var->name,"test")) 
			output1("%s hdname=%s type=%d i=%d nargu=%d argn=%d\n",pname,
				hd1->name,hd1->type,i,nargu,argn);
		*/

		if (nused<nargu)
			// function needs that many arguments
		{	
			paramflag=(*(int *)p);
			
			if (!invector) hd2=getvalue(hd1);

			if (paramflag&ParamStartsVector &&
				((hd2->type==s_matrix || hd2->type==s_cmatrix || hd2->type==s_imatrix)
					&& dimsof(hd2)->c>1))
			{	
				invector=1; *(hd->name)=0; hd->xor=0;
			}

			if (invector)
				// vector given to vector parameter "...,[a,b],..."
			{	
				defaults=paramflag&ParamHasValue; p+=sizeof(inttyp);
				if (dimsof(hd2)->c<invector)
				{	
					output1("Vector too short in function call to %s!\n",var->name);
					error=1; return;
				}
				if (hd2->type==s_matrix)
					new_real(matrixof(hd2)[invector-1],p);
				if (hd2->type==s_cmatrix)
					new_complex(matrixof(hd2)[2*(invector-1)],matrixof(hd2)[2*(invector-1)+1],p); 
				if (hd2->type==s_imatrix)
					new_interval(matrixof(hd2)[2*(invector-1)],matrixof(hd2)[2*(invector-1)+1],p); 
				if (error) return;
				invector++;
				p+=maxname+sizeof(inttyp);
				nused++;
				if (defaults) p=(char *)nextof((header *)p);
				if (paramflag&ParamEndsVector)
				{	
					if (dimsof(hd2)->c>=invector)
					{
						print("Vector too long for vector parameter (%d elements expected, got %d).\n"
							,invector-1,dimsof(hd2)->c);
						error=1; return;
					}
					referenceof(hd)=0;
					hd=nextof(hd); // goto next value passed to the function
					invector=0; // end of the vector
					continue; // continue with next parameter
				}
				goto startloop;
			}
			
			// in the following check the type of the argument

			if (paramflag>=ParamReal && 
				!(paramflag&ParamNone && hd2->type==s_string && *stringof(hd2)==1)
				)
			{	
				// scalar, vector or column (exclusive)	
				if (paramflag&ParamScalar)
				{	if (!(hd2->type==s_real || hd2->type==s_complex
						|| hd2->type==s_interval
						|| (hd2->type==s_string && paramflag&ParamString)))
					{	output1("Function %s needs a scalar for %s\n",var->name,pname);
						error=1; return;
					}
				}
				else if (paramflag&ParamVector && !(paramflag&ParamString))
				{	if (!(hd2->type==s_matrix || hd2->type==s_cmatrix || hd2->type==s_imatrix
						|| hd2->type==s_real || hd2->type==s_complex || hd2->type==s_interval))
					{	output1("Function %s needs a vector for %s\n",var->name,pname);
						error=1; return;
					}
					if ((hd2->type==s_matrix || hd2->type==s_cmatrix || hd2->type==s_imatrix)
						&& dimsof(hd2)->r!=1)
					{	output1("Function %s needs a vector for %s\n",var->name,pname);
						error=1; return;
					}
				}
				else if (paramflag&ParamVector && paramflag&ParamString)
				{	if (!(hd2->type==s_smatrix) && !(hd2->type==s_string))
					{	output1("Function %s needs a string vector for %s\n",var->name,pname);
						error=1; return;
					}
				}
				else if (paramflag&ParamColumn)
				{	if (!(hd2->type==s_matrix || hd2->type==s_cmatrix || hd2->type==s_imatrix
						|| hd2->type==s_real || hd2->type==s_complex
						|| hd2->type==s_interval))
					{	output1("Function %s needs a column vector for %s\n",var->name,pname);
						error=1; return;
					}
					if ((hd2->type==s_matrix || hd2->type==s_cmatrix || hd2->type==s_imatrix)
						&& dimsof(hd2)->c!=1)
					{	output1("Function %s needs a column vector for %s\n",var->name,pname);
						error=1; return;
					}
				}
				// integer, real, complex, interval, string, cpx, numerical (exclusive)
				if (paramflag&ParamInteger)
				{	if (hd2->type==s_real)
					{	if (floor(*realof(hd2))!=*realof(hd2))
						{	output1("Function %s needs an integer for %s\n",var->name,pname);
							error=1; return;
						}
					}
					else if (hd2->type==s_matrix)
					{	double *m=matrixof(hd2);
						int n=(dimsof(hd2)->r)*(dimsof(hd2)->c);
						for (int i=0; i<n; i++)
						{	if (floor(*m)!=(*m))
							{	output1("Function %s needs integer values for %s\n",var->name,pname);
								error=1; return;
							}
							m++;
						}
					}
					else
					{	output1("Function %s needs integer values for %s\n",var->name,pname);
						error=1; return;
					}
				}
				else if (paramflag&ParamReal)
				{	if (!(hd2->type==s_real || hd2->type==s_matrix))
					{	output1("Function %s needs a real for %s\n",var->name,pname);
						error=1; return;
					}
				}
				else if (paramflag&ParamComplex)
				{	if (!(hd2->type==s_complex || hd2->type==s_cmatrix || 
						hd2->type==s_real || hd2->type==s_matrix))
					{	output1("Function %s needs a complex for %s\n",var->name,pname);
						error=1; return;
					}
				}
				else if (paramflag&ParamInterval)
				{	if (!(hd2->type==s_interval || hd2->type==s_imatrix ||
						hd2->type==s_real || hd2->type==s_matrix))
					{	output1("Function %s needs an interval for %s\n",var->name,pname);
						error=1; return;
					}
				}
				else if (paramflag&ParamString)
				{	if (!(hd2->type==s_string || 
						(hd2->type==s_smatrix && paramflag&ParamVector)))
					{	output1("Function %s needs a string for %s\n",var->name,pname);
						error=1; return;
					}
				}
				else if (paramflag&ParamCall)
				{	if (!(hd2->type==s_string || (hd2->type==s_list && hd2+1<nextof(hd2) && (hd2+1)->type==s_string)))
					{	output1("Function %s needs a function name, an expression,\n"
							"   or a collection (with a function name) for %s\n",var->name,pname);
						error=1; return;
					}
				}
				else if (paramflag&ParamCPX)
				{	if (!(hd2->type==s_cpxmatrix))
					{	output1("Function %s needs a compressed matrix for %s\n",var->name,pname);
						error=1; return;
					}
				}
				else if (paramflag&ParamMatrix)
				{	if (!(hd2->type==s_matrix || hd2->type==s_cmatrix || hd2->type==s_imatrix
						|| hd2->type==s_real || hd2->type==s_complex
						|| hd2->type==s_interval))
					{	output1("Function %s needs a numerical type for %s\n",var->name,pname);
						error=1; return;
					}
				}
				// positive, nonnegative (exclusive)
				if (paramflag&ParamPositive)
				{	
					if (!isreal(hd2) || (hd2->type==s_real && *realof(hd2)<=0) || isnan(*realof(hd)))
					{	output1("Function %s needs a positive value for %s\n",var->name,pname);
						error=1; return;
					}
					if (hd2->type==s_matrix)
					{	double *m=matrixof(hd2);
						int n=(dimsof(hd2)->r)*(dimsof(hd2)->c);
						for (int i=0; i<n; i++)
						{	if (*m<=0 || isnan(*m))
							{	output1("Function %s needs positive values for %s\n",var->name,pname);
								error=1; return;
							}
							m++;
						}
					}
				}
				else if (paramflag&ParamNonNegative)
				{	if (!isreal(hd2) || (hd2->type==s_real && *realof(hd2)<0))
					{	output1("Function %s needs a positive value for %s\n",var->name,pname);
						error=1; return;
					}
					if (hd2->type==s_matrix)
					{	double *m=matrixof(hd2);
						int n=(dimsof(hd2)->r)*(dimsof(hd2)->c);
						for (int i=0; i<n; i++)
						{	
							if (*m<0 || isnan(*m))
							{	output1("Function %s needs positive values for %s\n",var->name,pname);
								error=1; return;
							}
							m++;
						}
					}
				}
			}

			// check if parameter has defaults
			defaults=paramflag&ParamHasValue; p+=sizeof(inttyp);
			
			// assign the parameter name to the provided parameter
			strcpy(hd1->name,p); hd1->xor=*((int *)(p+maxname));
			
			// mark reference parameters (i.e., parameter can be changed)
			if (*(hd1->name)!='%') hd1->flags|=FLAGVAR;
			
			// goto next parameter in the function
			p+=maxname+sizeof(inttyp);
			nused++;

			// skip the default value of the parameter
			if (defaults) p=(char *)nextof((header *)p);
		}
		else
			// additional provided parameters
		{	
			// give the parameters default names arg1, arg2, ...
			if (i<MAXARGN)
			{
				strcpy(hd1->name,argname[i]);
				hd1->xor=xors[i];
			}
		}

		// go to next argument
		hd=nextof(hd);
	}

	// disable all additional parameters
	for (i=n; i<argn; i++)
	{  
		strcpy(hd->name,""); hd->xor=0;
		hd=nextof(hd);
	}

	header *xhd[MAXARGN];
	int nxhd=0;

	// if (strcmp(var->name,"test")==0)
	while (hd<hdend)
	{
		if (*(hd->name) && !(hd->flags&ADDITIONAL)) 
		{
			// print("\n%s %d\n",hd->name,hd->type);
			xhd[nxhd++]=hd;
			if (nxhd>=MAXARGN) break;
		}
		hd=nextof(hd);
	}

	// this loop runs, if the function has more parameters with default values
	// than have been provided at function call
	for (i=nused; i<nargu; i++)
	{	
		defaults=(*(int *)p)&ParamHasValue;
		name=p+sizeof(inttyp);
		p+=maxname+2*sizeof(inttyp);
		if (defaults)
		{	
			header *h=(header *)p;
			// move the default value to the stack
			moveresult((header *)newram,h);
			// kill the assigned argument from the list of assigned arguments
			for (int j=0; j<nxhd; j++)
			{
				if (xhd[j] && xhd[j]->xor==h->xor && strcmp(xhd[j]->name,h->name)==0)
					xhd[j]=0;
			}
			p=(char *)nextof(h);
		}
		else
			// unprovided parameter has no default value
		{	
			print("Argument %s undefined for function %s.\n",name,var->name);
			error=1; return;
		}
	}

	if (!(var->flags&ALLOWASSIGNED))
	{
		for (int i=0; i<nxhd; i++)
		{
			if (xhd[i])
			{
				print("Argument %s not in parameter list of function %s.\n\n",xhd[i]->name,var->name);
				print("Parameters:\n");

				int ol=0;
				p=startargs;

				// print parameter
				for (i=0; i<nargu; i++)
				{	
					defaults=*((int *)p);
					p+=sizeof(inttyp);
					if (defaults&ParamStartsVector) output("[");
					output1("%s",p);
					ol+=strlen(p);
					p+=maxname+sizeof(inttyp);
					if (defaults&ParamHasValue)
					{	p=(char *)nextof((header *)p);
					}
					// char *sd=print_defaults(defaults);
					// output(sd); ol+=strlen(sd);
					if (defaults&ParamEndsVector) output("]");
					if (i!=nargu-1) { output(", "); ol+=2; }
					if (ol>linelength-15)
					{	output(" ..\n");
						ol=0;
					}
				}
				output("\n\n");

				error=1; return;
			}
		}
	}

	if (sp==0 && argn>nargu) sp=nargu;

	// remember the state of flags
	oldnosubmref=nosubmref; nosubmref=0;
	udflineold=udfline;
	relaxold=relax;
	relax=(var->flags&RELAX);
	oldargn=actargn; oldsp=actsp;
	actargn=argn; actsp=sp;
	udfline=next=udfof(var); udfold=udfon; udfon=1;
	currentfunction=var->name;
	oldstartlocal=startlocal; oldendlocal=endlocal;
	startlocal=(char *)args; endlocal=newram;
	oldrunning=running; running=var;
	oldindex=loopindex;
	oldsearchglobal=searchglobal; searchglobal=0;
	oldepsilon=epsilon; oldchanged=epsilon=changedepsilon;
	oldusematlab=usematlab;
	usematlab=(var->flags&MATLAB);

	if ((oldtrace=trace)>0) // note old trace flag and see of tracing is on.
	{	
		if (trace==2) trace=0; // user pressed cursor key to stop tracing
		if (trace>0) trace_udfline(next);
	}
	else if (var->flags&FLAGTRACE) // functions is marked for tracing
	{	
		trace=1; // switch on tracing
		trace_udfline(next);
	}
	
	udfcount++;

#ifdef WIN32
	if (udfcount%50==0 && getstacksize()>maxstacksize)
	{
		output("Too many recursions!\n");
		error=1;
	}
#else
	if (udfcount>MAXUDF)
	{	
		output("Too many recursions!\n");
		error=1;
	}
#endif

	// if (!strcmp(var->name,"test")) printlocalvars((char *)args);

#ifdef WIN32
	__try
	{
#endif
		while (!error && udfon==1)
		{	
			command();
			if (error) break;
			if (udfon==2)
			{	
				scan_space();
				if (*next==';' || *next==',' || *next==0)
					// no return value
				{	
					result=new_none(2);
				}
				else
				{	
					int oldli=loopindex;
					loopindex=returnloopindex;
					result=scan_value();
					if (!error && newram>(char *)result && var->flags&ARGS)
					{
						result->flags|=ARGS;
					}
					loopindex=oldli;
					scan_space();
					if (!(*next==0 || *next==';' || *next==',' || *next==3 || strncmp(next,"//",2)==0))
					{
						if (!error) print("Error in return result.\n");
						error=1; break;
					}
				}
				if (error) break;
				moveresult1(st,result);
			}
			if (test_key()==escape)
			{	
				output("User interrupted!\n"); error=58; break;
			}
		}
		if (error && error!=12345688 && traceerrors)
		{
			error=0;
			while (true)
			{
				hd=(header *)newram;
				new_string("Expression",12,""); if (error) break;
				minput1(hd); if (error) break;
				header *res1=getvalue(hd); if (error) break;
				give_out(res1);
				newram=(char *)hd;
			}
			error=12345688;
		}
#ifdef WIN32
	}
	__except (GetExceptionCode()==EXCEPTION_STACK_OVERFLOW?
		EXCEPTION_EXECUTE_HANDLER:EXCEPTION_CONTINUE_SEARCH)
	{
		print("Stack overflow. Too many recursions?\n");
		error=8888;
	}
#endif

	if (error)
	{	
		if (!traceerrors && udfcount==1)
		{
			print("Try \"trace errors\" to inspect local variables after errors.\n");
		}
		if (udfcount<20)
		{
			print("%s:\n    ",var->name);
			type_udfline(udfline,1);
		}
		else if (udfcount==20)
		{
			print("...\n");
		}
	}			
	
	udfcount--;
	endlocal=oldendlocal; startlocal=oldstartlocal;
	running=oldrunning;
	relax=relaxold;
	loopindex=oldindex;
	if (oldchanged==changedepsilon) epsilon=oldepsilon;
	else epsilon=changedepsilon;

	if (trace>=0) trace=oldtrace;
	
	if (udfon==0)
	{	
		moveresult1(st,new_none(2));
		// output1("Return missing in %s!\n",var->name); error=55; 
	}
	udfon=udfold; next=oldnext; udfline=udflineold;
	actargn=oldargn; actsp=oldsp;
	searchglobal=oldsearchglobal;
	nosubmref=oldnosubmref; 
	usematlab=oldusematlab;
}
