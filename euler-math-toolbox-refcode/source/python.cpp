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

#include "python.h"

void mloadpython (header *hd)
{
	header *st=hd,*result;
	hd=getvalue(hd);
	if (error) return;
	if (hd->type==s_string)
	{
		char *h=py_run_file(stringof(hd));
		if (error)
		{
			print("%s\n",h);
			return;
		}
		result=new_string(h,"");
	}
	else need_arg_in("python","string or string vector");
	moveresult(st,result);
}

void mpython (header *hd)
{
	header *st=hd,*result;
	hd=getvalue(hd);
	if (error) return;
	if (hd->type==s_string)
	{
		char *h=py_run(stringof(hd));
		if (error)
		{
			print("%s\n",h);
			return;
		}
		result=new_string(h,"");
	}
	else if (hd->type==s_smatrix)
	{
		static char line[MAXLINE];
		char *p=stringof(hd);
		char *q=line;
		while (*p!=1 && (q-line)<MAXLINE-2)
		{
			if (*p==0) *q='\n';
			else *q=*p;
			p++; q++;
		}
		*q=0;
		char *h=py_run(line);
		if (error)
		{
			print("%s\n",h);
			return;
		}
		result=new_string(h,"");
	}
	else need_arg_in("python","string or string vector");
	moveresult(st,result);
}

void mpyget (header *hd)
{
	header *st=hd,*result=(header *)newram;
	hd=getvalue(hd);
	if (error) return;
	if (hd->type==s_string)
	{
		py_get(stringof(hd),&newram,ramend);
		if (error) return;
	}
	else need_arg_in("pyget","string");
	moveresult(st,result);
}

void mpyset (header *hd)
{
	header *hd1;
	hd1=nextof(hd);
	hd=getvalue(hd); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	if (hd->type==s_string)
	{
		py_set(stringof(hd),hd1);
		if (error) return;
	}
	else need_arg_in("pyset","string and value");
}

void mpyeval (header *hd)
{
	header *st=hd;
	header *hds[32];
	int n=0;
	header *hn=next_param(hd);
	while (hn && n<32)
	{
		hds[n++]=hn;
		hn=next_param(hn);
	}
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) need_arg_in("pyeval","string and arguments");
	for (int i=0; i<n; i++)
	{
		hds[i]=getvalue(hds[i]); if (error) return;
	}
	header *result=(header *)newram;
	py_eval(stringof(hd),hds,n,&newram,ramend); if (error) return;
	moveresult(st,result);
}

void do_python ()
{
	// print("\"%s\"\n",next);

	static char line[MAXLINE];
	char *p=line;

	if (!strcmp(next,"...")) next_simple_line();

	strcpy(p,next);
	int l=strlen(next);
	next+=l;

	while (l>3 && !strcmp(p+l-3,"..."))
	{
		*(p+l-3)='\n';
		*(p+l-2)=0;
		p=p+(l-2);
		next_simple_line();
		strcpy(p,next);
		l=strlen(p);
		next+=l;
	}

	if ((*line == ':') && (*(line+1) == ' ') && (strlen(line)<MAXLINE-8))
	{
		char h[MAXLINE];
		strcpy(h, "print(");
		strcat(h, line+2);
		strcat(h, ")");
		strcpy(line, h);
	}

	// print("\"%s\"\n",line);

	char *h=py_run(line);
	if (error)
	{
		print("%s\n",h);
		return;
	}
	if (*h) print("%s\n",h);
}

void python_call (char *name, int n, header *hd)
{
	header *st=hd;
	header *hds[32];
	n=0;
	header *hn=hd;
	while (hn && n<32)
	{
		hds[n++]=hn;
		hn=next_param(hn);
	}
	for (int i=0; i<n; i++)
	{
		hds[i]=getvalue(hds[i]); if (error) return;
	}

	header *result=(header *)newram;
	py_eval(name,hds,n,&newram,ramend);
	if (error)
	{
		print("Error in Python command\n");
		return;
	}
	moveresult(st,result);
}

void get_python ()
{
	udf=1;
	static char line[32000];
	char *p=line;
	while (1)
	{
		next_line();
		if (strstarts(next,"endfunction"))
		{
			next+=strlen("endfunction");
			break;
		}
		strcpy(p,next);
		p+=strlen(p);
		strcpy(p,"\n");
		p++;
	}
	udf=0;
	char *h=py_run(line);
	if (error)
	{
		print("%s\n",h);
		return;
	}
	if (*h) print("%s\n",h);
}

char * print_defaults (int defaults);

void get_python_function (char *name, header *hd)
{
	FILE *actfile=infile;

	udf=1;
	static char line[32000];

	int count,defaults;
	char *p=helpof(hd);
	count=*((int *)p);
	p+=3*sizeof(inttyp);
	strcpy(line,"def ");
	strcat(line,name); 
	strcat(line,"(");
	int havedefaults=0;
	for (int i=0; i<count; i++)
	{	
		defaults=*((int *)p);
		p+=sizeof(inttyp);
		if (defaults&ParamHasValue)
		{	havedefaults=1;
		}
		if (i>0) strcat(line,",");
		strcat(line,p); 
		p+=maxname+sizeof(inttyp);
		if (defaults&ParamHasValue)
		{	
			p=(char *)nextof((header *)p);
		}
	}
	strcat(line,"):\n");

	p=line+strlen(line);

	while (1)
	{
		if (infile && actfile!=infile)
		{
			print("Function did not complete in current file\n");
			error=1; break;
		}
		if (strstarts(next,"endfunction"))
		{
			next+=strlen("endfunction");
			break;
		}
		strcpy(p,"  ");
		strcat(p,next);
		strcat(p,"\n");
		p+=strlen(p);
		next_simple_line();
	}
	udf=0;

	char *h=py_run(line);
	if (error)
	{
		print("%s\n",h);
		return;
	}
	if (*h) print("%s\n",h);
}

int pythonmode;

void do_pythonmode ()
{	
	char name[maxname];
	scan_space();
	if (!*next || *next==';' || *next==',')
	{	pythonmode=!pythonmode;
	}
	else
	{	scan_name(name);
		if (strcmp(name,"on")==0)
			pythonmode=1;
		if (strcmp(name,"off")==0)
			pythonmode=0;
	}
	if (*next!=';')
	{	switch (pythonmode)
		{	case 0 : output1("Python mode is off\n"); break;
			case 1 : output1("Python mode is on\n"); break;
		}
	}
	if (*next==',' || *next==';') next++;
}