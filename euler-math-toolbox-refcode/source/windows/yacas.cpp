#include "windows.h"
#include "../header.h"
#include "../stack.h"
#include "../getvalue.h"

#include "../yacas.h"

HMODULE YacasHandle;
int YacasStarted;

typedef void (*VoidFunction) ();
typedef void (*CharFunction) (char *);
typedef char * (*ResFunction) ();

void init_yacas ()
{	if (YacasHandle) return;
	YacasHandle=LoadLibrary("yacasdll.dll");
	if (!YacasHandle) return;
	VoidFunction init=(VoidFunction)GetProcAddress(YacasHandle,"yacas_init");
	init();
	YacasStarted=1;
	flog("Yacas started");
}

void yacas_test (int print)
{	if (!YacasHandle) init_yacas();
	if (!YacasHandle)
	{	error=1;
		if (print) output("Yacas not found!\n");
	}
}

void myacas (header *hd)
{	yacas_test(1);
	if (error) return;
	header *st=hd,*result;
	char *arg;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) need_arg_in("yacas","string");
	arg=stringof(hd);
	CharFunction exec=(CharFunction)GetProcAddress(YacasHandle,"yacas_eval");
	exec(arg);
	char *res;
	ResFunction yerror=(ResFunction)GetProcAddress(YacasHandle,"yacas_error");
	ResFunction yres=(ResFunction)GetProcAddress(YacasHandle,"yacas_result");
	res=yerror();
	if (res)
	{	output("Yacas Error:\n");
		output(res);
		error=1;
		return;
	}

	// Print output:
	ResFunction youtput=(ResFunction)GetProcAddress(YacasHandle,"yacas_output");
	res=youtput();
	output(res);
	
	res=yres();
	int n=strlen(res);
	if (n>0 && res[n-1]==';') res[n-1]=0;
	result=new_string(res,strlen(res),"");
	if (error) return;
	moveresult(st,result);
}

void myacaseval (header *hd)
{	yacas_test(1);
	if (error) return;
	header *st=hd,*result;
	char *arg;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) need_arg_in("yacaseval","string");
	arg=stringof(hd);
	CharFunction exec=(CharFunction)GetProcAddress(YacasHandle,"yacas_eval");
	exec(arg);
	char *res;
	ResFunction yerror=(ResFunction)GetProcAddress(YacasHandle,"yacas_error");
	ResFunction yres=(ResFunction)GetProcAddress(YacasHandle,"yacas_result");
	res=yerror();
	if (res)
	{	output("Yacas Error:\n");
		output(res);
		error=1;
		return;
	}
	res=yres();
	int n=strlen(res);
	if (n>0 && res[n-1]==';') res[n-1]=0;
	result=new_string(res,strlen(res),"");
	if (error) return;
	moveresult(st,result);
}

char *call_yacas (char *s)
{	yacas_test(1);
	CharFunction exec=(CharFunction)GetProcAddress(YacasHandle,"yacas_eval");
	exec(s);
	char *res;
	ResFunction yerror=(ResFunction)GetProcAddress(YacasHandle,"yacas_error");
	ResFunction yres=(ResFunction)GetProcAddress(YacasHandle,"yacas_result");
	res=yerror();
	if (res)
	{	error=1;
		return res;
	}
	res=yres();
	int n=strlen(res);
	if (n>0 && res[n-1]==';') res[n-1]=0;
	return res;
}

void exit_yacas ()
{	if (YacasHandle) FreeLibrary(YacasHandle);
}

void interrupt_yacas ()
{	yacas_test(0);
	if (error)
	{	error=0; return;
	}
	VoidFunction inter=(VoidFunction)GetProcAddress(YacasHandle,"yacas_interrupt");
	inter();
}

void start_yacas ()
{	if (YacasStarted) return;
	yacas_test(1);
	if (error) return;
	VoidFunction init=(VoidFunction)GetProcAddress(YacasHandle,"yacas_init");
	init();
	YacasStarted=1;
}

void end_yacas ()
{	if (!YacasStarted) return;
	yacas_test(0);
	if (error) return;
	VoidFunction end=(VoidFunction)GetProcAddress(YacasHandle,"yacas_exit");
	end();
	YacasStarted=0;
}

void myacasclear (header *hd)
{	yacas_test(0);
	if (error) return;
	if (YacasStarted) end_yacas();
	if (error) return;
	start_yacas();
	new_real(0,"");
}
