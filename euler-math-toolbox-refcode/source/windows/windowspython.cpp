#include "windows.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <float.h>
#include <time.h>
#include <mmsystem.h>
#include <io.h>
#include <direct.h>
#include <Shlwapi.h>

#include "../header.h"
#include "../sysdep.h"
#include "euler.h"
#include "../help.h"
#include "meta.h"
#include "../yacas.h"
#include "../maxima.h"
#include "colors.h"
#include "text.h"
#include "windowsmeta.h"
#include "graphicswindow.h"
#include "main.h"
#include "edit.h"
#include "windowsframes.h"
#include "windowsgraphics.h"
#include "images.h"
#include "helpwindow.h"
#include "spell.h"

#include "windowspython.h"

#include "../builtin.h"
#include "../help.h"


#include "../python.h"
#include "../udf.h"

/************ New Code ***************/

HMODULE pyhandle=0;

extern int askuser (char *text);
extern int askexec,allowpythononce;

typedef void (*f_print) (char *, ...);
typedef void (*f_stack) (char **newram, char **ramend);
typedef int (*f_eval) (header *hd, char **newram);
typedef int (*f_testkey) ();
typedef int (*f_eulerprint) (header *hd, char **newram);
typedef int (*f_py_init) (f_print f, f_stack fs, 
	f_eval fe, f_testkey ftk, f_eulerprint fep);
typedef void (*f_py_exit) (void);
typedef char * (*f_py_run) (char *);

typedef char * (*f_py_error) (void);

f_py_exit pyexit;
f_py_run pyrun;
f_py_run pyrunfile;
f_py_error pyerror;

typedef int (*f_py_get) (char *name, char **xnewram, char *ramend);
f_py_get pyget;

typedef int (*f_py_set) (char *name, header *hd);
f_py_set pyset;

typedef int (*f_py_eval) (char *name, header *hd[], int n, char **xnewram, char *xramend);
f_py_eval pyeval;

void fstack (char **xnewram, char **xramend)
{
	*xnewram=newram;
	*xramend=ramend;
}

int exec_dll (char *name, int n, header *hd);

int feval (header *hd, char **xnewram)
{
	header *st=hd;

	char *oldnewram=newram;
	newram=*xnewram;

	if (hd->type!=s_string)
	{
		print("Python called Euler without a function name\n");
		error=1; return 0;
	}
	char *name=stringof(hd);
	hd=nextof(hd);

	int n=0;
	if ((char *)hd<newram)
	{
		n++;
		header *h=hd;
		while (next_param(h)) { h=next_param(h); n++; }
	}

	if (strstarts(name,"py$"))
	{
		python_call(name+3,n,hd);
		if (error) return 0;
		goto after;
	}

	if (name[0]=='_') 
		// search for a builtin command primarily
	{	
		if (exec_builtin(name+1,n,hd)) {}
#ifdef DLL
		else if (exec_dll(name+1,n,hd)) {}
#endif
		else
		{	
			function_error(name,n);
			error=1;
			return 0;
		}
        goto after;
	}

	header *var=searchudf(name);

	if (var)
	{
		if (var->flags&MAP)
		{
			map1f(hd,var->name);
		}
		else
		{
			interpret_udf(var,hd,n,0);
		}
	}
	else
	{
		if (exec_builtin(name,n,hd));
#ifdef DLL
		else if (exec_dll(name,n,hd));
#endif
		else
		{	
			print("Function %s called from Python not found.\n",name);
			*xnewram=newram;
			newram=oldnewram;
			return 0;
		}
	}

	if (!error) moveresult(st,hd);

after:

	*xnewram=newram;
	newram=oldnewram;
	return error==0;
}

int feulerprint (header *hd, char **xnewram)
{
	if ((char *)hd<*xnewram)
		give_out(hd);
	return 1;
}

extern int python27;

void py_init ()
{
	char* pydll = "py3.dll";
	// char* pyversion = "python27.dll"; // will be loaded to test Python
	if (python27)
	{
		pydll = "py2.dll";
		// pyversion = "python37.dll";
	}
	

	/*
	// Test, if Python is installed at all
	HMODULE test=LoadLibrary(pyversion);
	if (!test)
	{
		DumpWarning("Could not load %s.\nMaybe Python is not installed?","Euler",pyversion);
		error=1; return;
	}
	FreeLibrary(test);
	*/

	if (askexec && !allowpythononce)
	{
		int res=askuser("Euler wants to use Python.\nAllow?");
		if (res==2) 
		{
			error=1; return;
		}
		if (res==3) allowpythononce=1;
	}

	static char line[MAXLINE];
	strcpy(line,pydll);

	pyhandle=LoadLibrary(line);
	if (!pyhandle) 
	{
		DumpWarning("Could not open %s","Euler",line);
		error=1; return;
	}

	pyexit = (f_py_exit)GetProcAddress(pyhandle, "py_exit");
	pyrun = (f_py_run)GetProcAddress(pyhandle, "py_run");
	pyrunfile = (f_py_run)GetProcAddress(pyhandle, "py_run_file");
	pyerror = (f_py_error)GetProcAddress(pyhandle, "py_error");
	pyget = (f_py_get)GetProcAddress(pyhandle, "py_get");
	pyset = (f_py_set)GetProcAddress(pyhandle, "py_set");
	pyeval = (f_py_eval)GetProcAddress(pyhandle, "py_eval");

	f_py_init pyinit;
	pyinit = (f_py_init)GetProcAddress(pyhandle, "py_init");

	if (!pyinit || !pyexit || !pyrun || !pyrunfile || !pyerror || !pyget || !pyset || !pyeval)
	{
		Warning("Could not initialize Python DLL", "Euler");
		error = 1;
		FreeLibrary(pyhandle); pyhandle = 0;
		return;
	}

	if (!pyinit(print,fstack,feval,test_code,feulerprint))
	{
		print(pyerror());
		print("\n");
		Warning("Error when initializing Python (in pyinit())","Euler");
		error=1; 
		FreeLibrary(pyhandle); pyhandle=0; 
		return;
	}

}

void py_exit (void)
{
	if (!pyhandle) return;
	pyexit();
	FreeLibrary(pyhandle); pyhandle=0; 
}

/**
evaluate a Python command.
*/
char *py_run (char *s)
{
	if (!pyhandle) py_init();
	if (!pyhandle) 
	{
		print("Python was not initialized (py_run).\n");
		error=1; return ""; 
	}

	char *res="";
	__try
	{
		res=pyrun(s);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		print("Python crash executing:\n%s\n",s);
		error=1; return "";
	}
	
	if (!res)
	{
		print(pyerror());
		print("\n");
		error=1; return "";
	}

	textwindow.top();

	return res;
}

char *py_run_file (char *s)
{
	if (!pyhandle) py_init();
	if (!pyhandle) 
	{
		print("Python was not initialized (py_run_file).\n");
		error=1; return ""; 
	}

	char *res=0;
	__try
	{
		res=pyrunfile(s);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		print("Python crash executing:\n%s\n",s);
		error=1; return "";
	}

	if (!res)
	{
		print("Python error:\n%s\n",pyerror());
		error=1; return "";
	}
	return res;
}

void py_get (char *name, char **xnewram, char *ramend)
{
	if (!pyhandle) py_init();
	if (!pyhandle) 
	{
		print("Python was not initialized (py_get).\n");
		error=1; return; 
	}

	__try
	{
		pyget(name,xnewram,ramend);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		print("Python crash getting %s\n",name);
		error=1; return;
	}

	char *err=pyerror();
	if (*err)
	{
		print("Python error:\n%s\n",err);
		error=1; return;
	}
}

void py_set (char *name, header *hd)
{
	if (!pyhandle) py_init();
	if (!pyhandle) 
	{
		print("Python was not initialized (py_set).\n");
		error=1; return; 
	}

	__try
	{
		pyset(name,hd);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		print("Python crash setting %s\n",name);
		error=1; return;
	}

	char *err=pyerror();
	if (*err)
	{
		print("Python error:\n%s\n",err);
		error=1; return;
	}
}

/**
evaluate a python function
*/
void py_eval (char *name, header *hd[], int n, char **xnewram, char *xramend)
{
	if (!pyhandle) py_init();
	if (!pyhandle) 
	{
		print("Python was not initialized (py_eval).\n");
		error=1; return; 
	}

	__try
	{
		pyeval(name,hd,n,xnewram,xramend);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		print("Python crash evaluating %s\n",name);
		error=1; return;
	}

	char *err=pyerror();
	if (*err)
	{
		print("Python error:\n%s\n",err);
		error=1; return;
	}
}
