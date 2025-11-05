#include "windows.h"
#include "../header.h"
#include "../sysdep.h"
#include "../stack.h"
#include "../getvalue.h"
#include "vector.h"
#include <Windows.h>

#include "../builtin.h"
#include "../help.h"
#include "../python.h"
#include "../udf.h"

class Lib
{	
	HMODULE Handle;
	String Name;
	public :
	Lib (char *name, char *myname) : Name(myname)
	{	Handle=LoadLibrary(name);
	}
	~Lib ()
	{	FreeLibrary(Handle);
	}
	char *name () { return Name; }
	HMODULE handle () { return Handle; }
};

Vector<Lib *> *libs=new Vector<Lib *>(32);

typedef char * (*LibF) (header * params [], int nparam,
	char *newram, char *ramend);

class LibFunction
{	
public :
	LibF F;
	String Name;
	int NArgs;
	Lib *lib;
	public :
	LibFunction (char *name, LibF f, int nargs, Lib *l) :
		Name(name),F(f),NArgs(nargs),lib(l)
	{}
	char *name () { return Name; }
	LibF f () { return F; }
	int nargs () { return NArgs; }
};

extern int evaleuler (char *name, header *header, char **xnewram);

typedef void (*InitFunction) (
	void (*output) (char *), 
	void (*output1) (char *,...), 
	int (*wait_key) (int *), 
	int (*test_key) (),
	int (*evaleuler) (char *, header *, char **));

typedef void (*ExitFunction) ();

Vector<LibFunction *> *libfunctions=new Vector<LibFunction *>(32);

LibFunction * libfind (char *s, int n)
{	
	for (int i=0; i<libfunctions->n; i++)
	{
		LibFunction *p=libfunctions->h[i];
		if (!strcmp(p->name(),s) && (p->nargs()==-1 || p->nargs()==n)) return p;
	}
	return 0;
}

extern char *search_file_in_path (char *filename, char *extension);

extern int askexec,askexecinsession;

extern int askuser (char *text);
extern int allowdllonce;

void mdll (header *hd)
{	
	header *st=hd,*hd1=nextof(hd),*hd2=nextof(hd1);
	hd=getvalue(hd); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	hd2=getvalue(hd2); if (error) return;
	if (hd->type!=s_string || hd1->type!=s_string || hd2->type!=s_real)
		wrong_arg_in("dll");
	char *p=stringof(hd);
	int i,n=(int)*realof(hd2);
	if (n<-1 || n>16)
		ERET("DLL functions must have variable number (-1) or 0 to 16 parameters.\n",1);
	Lib *lib=0;
	for (i=0; i<libs->n; i++)
		if (strcmp(p,libs->h[i]->name())==0)
		{    lib=libs->h[i]; break;
		}
	HMODULE h=0;
	if (!lib)
	{
		char *pp=search_file_in_path(p,".dll");
		if (!pp) pp=p;
		if (askexec && !allowdllonce)
		{
			String ask("");
			ask.cat("Euler wants to load the DLL\n");
			ask.cat(p);
			ask.cat("\n\nAllow this DLL?");

			int res=askuser(ask);

			if (res==2)
			{ 
				print("DLL could not be started!\n");
				error=1; return; 
			}
			else if (res==3) allowdllonce=1;
		}
		Lib *newlib=new Lib(pp,p);
		if (!newlib->handle())
			ERET("Could not open the DLL library!\n",1)
		h=newlib->handle();
		libs->add(newlib);
		InitFunction init;
		init=(InitFunction)GetProcAddress(h,"init");
		if (!init)
			ERET("Could not find the init function in DLL!\n",1)
		else
		{	init(output,output1,wait_key,test_key,evaleuler);
		}
		lib=newlib;
	}
	else
	{
		h=lib->handle();
	}

	char *name=stringof(hd1);
	int nl=(int)strlen(name);
	if (nl>0 && name[nl-1]=='$')
	{
		print("Cannot name a function %s\n",name);
		error=1; return;
	}

	LibF f=(LibF)GetProcAddress(h,stringof(hd1));
	if (!f)
		ERET("Could not find this DLL function!\n",1)
	if (!libfind(stringof(hd1),n))
		libfunctions->add(new LibFunction(stringof(hd1),f,n,lib));
	else hd1=new_string("",4,"");
	moveresult(st,hd1);
}

void closedll (char *name)
{
	for (int i=0; i<libs->n; i++)
		if (strcmp(name,libs->h[i]->name())==0)
		{    
			Lib *lib=libs->h[i];
			for (int j=0; j<libfunctions->n; j++)
			{
				if (libfunctions->h[j]->lib==lib)
				{
					delete libfunctions->h[j];
					libfunctions->remove(j--);
				}
			}
			ExitFunction exitf;
			exitf=(ExitFunction)GetProcAddress(lib->handle(),"clear");
			if (exitf) exitf();
			delete lib;
			libs->remove(i);
			break;
		}	
}

void mclosedll (header *hd)
{
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string)
		ERET("Need a string for closedll",1);
	closedll(stringof(hd));
}

void closelibs ()
{	
	int i;
	for (i=0; i<libs->n; i++) 
	{
		ExitFunction exitf;
		exitf=(ExitFunction)GetProcAddress(libs->h[i]->handle(),"clear");
		if (exitf) exitf();
		delete libs->h[i];
	}
	libs->n=0;
	for (i=0; i<libfunctions->n; i++) delete libfunctions->h[i];
	libfunctions->n=0;
}

int exec_dll (char *name, int n, header *hd)
{   
	header *st=hd;
	LibFunction *l=libfind(name,n);
	if (!l) return 0;
	static header *h[32];
	if (n>=32) n=32;
	int i;
	for (i=0; i<n; i++)
	{	
		h[i]=getvalue(hd); if (error) return 1;
		hd=nextof(hd);
	}

	char *ram=0;
	__try
	{
		*newram=0;
		ram=l->f()(h,n,newram,ramend);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		print("DLL function %s crashed.\n",name);
		error=1; return 1;
	}
	
	if (ram==0 || (ram<=newram && *newram==0))
	{   
		newram=(char *)st;
		new_none(2);
	}
	else if (ram>newram)
	{   
		memmove((char *)st,newram,ram-newram);
    	newram=(char *)st+(ram-newram);
	}
	else
	{   
		output1("%s returned an error:\n",l->name());
		output(newram); output("\n");
		error=1;
	}
	return 1;
}

extern HANDLE CurrentProcess;
HANDLE tccin,tccout;

#define MAXTCC 32000
char tccbuffer[MAXTCC];

int gettccout ()
{
	DWORD l;

	if (!ReadFile(tccin,tccbuffer,MAXTCC,&l,0))
	{	
		print("tcc is not responding.\n");
		error=1; return 0;
	}
	tccbuffer[l]=0;

	CancelIo(tccout);

	return 0;
}

Thread tccthread (gettccout);

extern int linux,systemscaling;

void tcc (char *filename)
{
	static char line[MAXLINE];
	sprintf(line,"%s.dll",filename);
	remove(line);

#ifdef WIN64
	sprintf(line,"\"%stcc\\tcc\" -shared -DWIN64 -I \"%sdll\" -o \"%s.dll\" \"%s.c\" \"%sdll\\dlldef.c\"",
		getstartdir(),getstartdir(),filename,filename,getstartdir());
#else
	sprintf(line,"\"%stcc\\tcc\" -shared -I \"%sdll\" -o \"%s.dll\" \"%s.c\" \"%sdll\\dlldef.c\"",
		getstartdir(),getstartdir(),filename,filename,getstartdir(),filename);
#endif

	SECURITY_ATTRIBUTES saAttr; 
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
	saAttr.bInheritHandle = TRUE; 
	saAttr.lpSecurityDescriptor = NULL;

	if (!CreatePipe(&tccin,&tccout,&saAttr,MAXLINE))
	{	
		print("Could not create a pipe to tcc.\n");
		error=1; return;
	}

	STARTUPINFO si; // structure to start a process
	PROCESS_INFORMATION pi; // structure to get back process information
	
	// Now start browser process:
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	
	ZeroMemory( &pi, sizeof(pi) );

	si.hStdError=tccout;
	si.hStdOutput=tccout;
	si.hStdInput=tccin;
	si.dwFlags |= STARTF_USESTDHANDLES;

	if (!CreateProcess(0,line,0,0,TRUE,CREATE_NO_WINDOW,0,0,&si,&pi))
	{
		char ws[MAXLINE+255];
		sprintf(ws,"Could not call command\n%s",line);
		Warning(ws,"Euler");
		return;
	}
	CurrentProcess=pi.hProcess;

	if (WaitForSingleObject(pi.hProcess,INFINITE))
	{
		if (!linux)
		{
			DumpWarning(
				"Waiting for the tcc process failed!\n"
				"%s\n"
				"Please retry.",
				"Euler",line);
			CurrentProcess=0;
			return;
		}
		else
		{
			sprintf(line,"%s.dll",filename);
			int tries=0;
			while (!exists(line) && tries<100) { tries++; Sleep(200); }
		}
	}

	tccbuffer[0]=0;
	if (tccthread.active()) tccthread.kill();
	tccthread.start();

	if (tccbuffer[0]==0) Sleep(200);

	sprintf(line,"%s.def",filename);
	remove(line);

	sprintf(line,"%s.dll",filename);
	if (!exists(line))
	{
		sprintf(line,"%s.c:",filename);
		if (strncmp(line,tccbuffer,strlen(line))==0)
		{
			int ln=0;
			sscanf(tccbuffer+strlen(line),"%d",&ln);
			sprintf(line,"%s.c",filename);
			FILE *fin=fopen(line,"r");
			int count=1;
			print("\n...\n");
			if (fin)
			{
				while (true)
				{
					char *res=fgets(line,MAXLINE,fin);
					if (!res) break;
					if (count>ln-5 && count<ln+5)
					{
						print("%d: %s",count,line);
					}
					count++;
					if (count>=ln+5) break;
				}
				print("...\n\n");
				fclose(fin);
			}
		}
		print("%s\n%s.dll was not generated.\n",tccbuffer,filename);
		error=1; return;
	}

	CurrentProcess=0;
	DWORD ex;
	GetExitCodeProcess(pi.hProcess,&ex);
	return;
}

char *tcheader[] = {
"#include \"dlldef.h\"",
"#include <stdio.h>",
"#include <math.h>",
" ",
"EXPORT char *%s (header *hd[], int np, char *ramstart, char *ramend)",
"{",
"\tint argn=0;",
"\tstart(ramstart,ramend);",
" ",
""};

void closedll (char *name); // defined in dll.cpp
void mdll (header *name); // also

void get_tc (char *name, int argn)
	// function tc name ...
{
	FILE *actfile=infile;

	// create and open the *.c file
	char line[MAXLINE];
	sprintf(line,"%s.c",name);
	FILE *out=fopen(line,"w");
	if (!out)
	{
		print("Could not open %s\n",line);
		error=1; return;
	}

	// write the includes and the header
	char **h=tcheader;
	while (**h)
	{
		fprintf(out,*h,name);
		fputs("\n",out);
		h++;
	}
	
	// set udf mode
	udf=1;

	while (!error)
	{
		// scan the next line
		char *pl=line;
		while (*next) *pl++=*next++;
		*pl=0;

		if (strstarts(line,"endfunction")) break;
		if (infile && actfile!=infile)
		{
			print("Function did not complete in current file\n");
			error=1; break;
		}

		// print the line to the file
		fputs("\t",out); fputs(line,out); fputs("\n",out);
		next_simple_line();
	}

	// clear udf mode
	udf=0;

	// print footer and stop
	fputs("\t",out); fputs("return newram;\n",out);
	fputs("}\n",out);
	
	// close the file
	fclose(out);

	// compile it
	closedll(name);
	tcc(name);

	// load the function
	header *st=(header *)newram;
	new_string(name,"");
	new_string(name,"");
	new_real(argn,"");
	mdll(st);
}

int evaleuler (char *name, header *hd, char **xnewram)
	// evaluate an Euler function with arguments
{
	char *oldnewram=newram;
	newram=*xnewram;

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
		if (exec_builtin(name+1,n,hd));
		else if (exec_dll(name+1,n,hd));
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
			print("Function %s called from C not found.\n",name);
			*xnewram=newram;
			newram=oldnewram;
			return 0;
		}
	}

after:

	*xnewram=newram;
	newram=oldnewram;
	return error!=0;
}
