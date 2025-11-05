/*

Files to run Maxima from Euler.

It is assumed that Maxima is installed in the Euler start directory.
This is the preferred and default location for the "Maxima for Euler" 
installer. If Maxima starts implicitely the default location will
be used. The Maxima process will run in the background, and communicate
via pipes. It is killed, when Euler ends, or
on user demand with the "mxmstop" command. However, the user can also 
specify an own path to Maxima using "mxmstart".

The Maxima communication is text based and asynchronous. Euler has
to use threads to take care of blocks from Maxima. There is currently
no way to stop a Maxima computation from Euler by the user.
Communication to Maxima is via pipes.

*/


#include "windows.h"
#include "../sysdep.h"
#include "../header.h"
#include "../stack.h"
#include "../maxima.h"
#include "../help.h"
#include "main.h"
#include "Shlwapi.h"

STARTUPINFO si; // structure to start a process
PROCESS_INFORMATION pi; // structure to get back process information

HANDLE eulerin,maximaout,eulerout,maximain;

int maximarunning=0;
char mxmbuffer[MAXMXOUTPUT+100]; // Hopefully large enough buffer for communication
int killo=1;

extern int saveprofile; // Is 1, if Euler is installed on portable medium

/*
The startup thread for Maxima. We need to use a thread, since Maxima
might not answer. The user can escape the startup this way and continue
to use Euler.
*/

int runstartup ();

class MaximaStartupThread : public Thread
{	public :
	char Name[512];
	MaximaStartupThread () : Thread(runstartup) {}
	void set (char *name)
	{	strncpy(Name,name,512);
	}
} maximastartup;

extern Profile profile;

extern String smaximacallstring,smaximacalldir;

char foundfile[MAX_PATH];

int FindFilesRecursively (LPCTSTR lpFolder, LPCTSTR lpFilePattern)
{
    TCHAR szFullPattern[MAX_PATH];
    WIN32_FIND_DATA FindFileData;
    HANDLE hFindFile;
    // first we are going to process any subdirectories
    PathCombine(szFullPattern, lpFolder, "*");
    hFindFile = FindFirstFile(szFullPattern, &FindFileData);
    if(hFindFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
				if (*(FindFileData.cFileName)!='.') 
				{
					// found a subdirectory; recurse into it
					PathCombine(szFullPattern, lpFolder, FindFileData.cFileName);
					if (FindFilesRecursively(szFullPattern, lpFilePattern))
					{
						FindClose(hFindFile);
						return 1;
					}
				}
            }
        } 
		while (FindNextFile(hFindFile, &FindFileData));
        FindClose(hFindFile);
    }
    // now we are going to look for the matching files
    PathCombine(szFullPattern, lpFolder, lpFilePattern);
    hFindFile = FindFirstFile(szFullPattern, &FindFileData);
    if(hFindFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            if(!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                // found a file; do something with it
                PathCombine(szFullPattern, lpFolder, FindFileData.cFileName);
				strcpy(foundfile,szFullPattern);
		        FindClose(hFindFile);
                return 1;
            }
        } 
		while(FindNextFile(hFindFile, &FindFileData));
        FindClose(hFindFile);
    }
	return 0;
}

/*
Startup thread routine
*/
int runstartup ()
{		
	// get maxima call string
	char param[MAXLINE];
	int havecall=smaximacallstring.length()>0;

	if (!havecall || smaximacallstring.endsWith("maxima") || smaximacallstring.endsWith("maxima.exe"))
	{
		sprintf(param,"\"%s\" -eval \"(cl-user::run)\" -f --",
			havecall?smaximacallstring.text():"maxima");
	}
	else if (smaximacallstring.endsWith("sbcl.exe"))
	{
		if (!FindFilesRecursively(smaximacalldir.text(),"maxima.core"))
		{
			Warning("Could not find maxima.core for sbcl.exe\nAborting Maxima.","Euler");
			error=1; return 0;
		}
		sprintf(param,"\"%s\" --core \"%s\" --noinform --end-runtime-options --eval \"(cl-user::run)\"",
			smaximacallstring.text(),
			foundfile
			);
	}
	else
	{
		Warning("Can only call maxima, maxima.exe or sbcl.exe for Maxima!","Euler");
		error=1; return 0;
	}

	// get call directory for Maxima
	String calldir(maximastartup.Name);
	if (smaximacalldir.length()>0) calldir.copy(smaximacalldir.text());

	if (!SetEnvironmentVariable("maxima_prefix",calldir.text()))
	{
		Warning("Could not set the environment!\n"
			"Maxima could not be started!","Euler");
		error=1; return 0;
	}

	// get user directory for Maxima
	String userdir("",1024);
	if (!GetEnvironmentVariable("userprofile",userdir.text(),1024))
	{
		Warning("Could not get the environment!\nMaxima could not be started!","Euler");
		error=1; return 0;
	}
	if (saveprofile) userdir.copy(calldir);

	String dir(userdir);
	if (!SetEnvironmentVariable("maxima_tempdir",dir.text()))
	{
		Warning("Could not set the environment!\nMaxima could not be started!","Euler");
		error=1; return 0;
	}
	
	dir.copy(userdir); dir.cat("\\maxima");
	if (!SetEnvironmentVariable("maxima_userdir",dir.text()))
	{
		Warning("Could not set the environment!\nMaxima could not be started!","Euler");
		error=1; return 0;
	}

	String newpath(getenv("path"));
	String toadd("");
	toadd.cat(";");
	toadd.cat(calldir);
	toadd.cat(";");
	toadd.cat(calldir);
	toadd.cat("\\gnuplot;");
	toadd.cat(calldir);
	toadd.cat("\\gnuplot\\bin;");
	toadd.cat(calldir);
	toadd.cat("\\bin;");

	if (!newpath.contains(toadd.text()))
	{
		newpath.cat(toadd.text());
		if (!SetEnvironmentVariable("path",newpath.text()))
		{
			Warning("Could not set the environment!\nMaxima could not be started!","Euler");
			error=1; return 0;
		}
	}
	
	// start the Maxima process

	SECURITY_ATTRIBUTES saAttr; 
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
	saAttr.bInheritHandle = TRUE; 
	saAttr.lpSecurityDescriptor = NULL; 

	ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );
	
	if (!CreatePipe(&eulerin,&maximaout,&saAttr,MAXLINE))
	{	error=1; return 0;
	}
	if (!CreatePipe(&maximain,&eulerout,&saAttr,MAXLINE))
	{	error=1; return 0;
	}
	si.hStdError=maximaout;
	si.hStdOutput=maximaout;
	si.hStdInput=maximain;
	si.dwFlags |= STARTF_USESTDHANDLES;

	if (!CreateProcess(0,param,0,0,TRUE,CREATE_NO_WINDOW,0,0,&si,&pi)) // start Maxima
	{
		Warning("Could not start Maxima!\n"
			"Setup Maxima in the menu.","Euler");
		error=1; return 0;
	}
	WaitForInputIdle(pi.hProcess,10000); // wait until it is ready

	DWORD l;

	while (1) // receive the startup message from Maxima and discard it.
	{	
		if (!ReadFile(eulerin,mxmbuffer,MAXMXOUTPUT,&l,0))
		{	error=1; return 0;
		}
		mxmbuffer[l]=0;
		if (l>2 && mxmbuffer[l-1]==' ' && mxmbuffer[l-2]==')') break;
	}

	// send a lisp command to change the prompt prefix and suffix:
	char *s=":lisp (setq *prompt-prefix* \"--->\" *prompt-suffix* \"<---\");\n";
	if (!WriteFile(eulerout,s,strlen(s),&l,0))
	{	
		Warning("Could not send commands to Maxima!","Euler");
		error=1; return 0;
	}
	
	// again receive Maxima answer and discard it:
	while (1)
	{	
		if (!ReadFile(eulerin,mxmbuffer,MAXMXOUTPUT,&l,0))
		{	
			Warning("Could not receive results from Maxima!","Euler");
			error=1; return 0;
		}
		mxmbuffer[l]=0;
		if (l>2 && mxmbuffer[l-1]==' ' && mxmbuffer[l-2]==')') break;
	}


	// end thread:
	maximarunning=1;
	maximastartup.Active.clear();

	return 0;
}

int runmaxima (char *s, int output, int replace);

CriticalSection maximacritical("Maxima");

/**
Function to start Maxima. Waits for Maxima, but can be interrupted.
*/
void start_maxima (char *s)
{	
	if (maximarunning) return;

	flog("Starting Maxima");

	if (maximastartup.active())
	{
		Sleep(100);
		for (int i=0; i<600; i++)
		{	
			if (error>0 || !maximastartup.active()) return;
			if (test_key()==escape) 
			{	
				error=1; return;
			}
			Sleep(100);
		}
		return;
	}

	maximacritical.enter();

	display2d=1;
	maximastartup.set(s);
	maximastartup.start();

	// Wait for the process to finish
	Sleep(100);
	for (int i=0; i<600; i++)
	{	if (error>0 || !maximastartup.active()) break;
		if (test_key()==escape) 
		{	error=1; break;
		}
		Sleep(100);
	}

	// if error, clean up:
	if (error>0 || !maximarunning)
	{	
		if (maximastartup.active()) maximastartup.kill();
		error=1;
		maximacritical.leave();
		return;
	}

	mxmstartup();
	maximacritical.leave();
}

/*
Stop Maxima by killing the process.
*/
void end_maxima ()
{	if (maximarunning && pi.hProcess>0) 
		if (!TerminateProcess(pi.hProcess,0))
			dump("Could not end the Maxima thread!");
	maximarunning=0;
	CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
}

/**
The thread to get results from Maxima.
This assumes that maxima sends a prompt to the user. The
prompt is recognized by its special prefix and suffix.
There are three output modes:
0 : all output is discarded
1 : output is printed to the Euler text window
2 : output is kept in the out String
*/

int getmxmresult ();

class MaximaThread : public Thread
{	public :
	int output,question;
	String out;
	MaximaThread () : Thread(getmxmresult),out("") {}
} maximathread;

/*
Threaded function to get Maxima results.
*/
int getmxmresult ()
{	
	int end=0;
	maximathread.question=0;
	DWORD l;

	if (maximathread.output==1) output("\n");

	int outfound=0,first=1;

	/*
	Maxima output comes in chunks (not line	oriented).
	*/
	while (1)
	{	
		if (!ReadFile(eulerin,mxmbuffer,MAXMXOUTPUT,&l,0))
		{	
			error=1; return 0;
		}
		
		// Null terminate the output string (need not include the 0):
		mxmbuffer[l]=0;
		
		// Test output of Maxima output:
		// output("--------------------\n");
		// output(mxmbuffer);
		// output("\n--------------------\n");

		if (first) 
			// remove empty lines that Maxima might send
		{
			char *start=mxmbuffer;
			while (true)
			{
				char *h=start;
				while (*h==' ') h++;
				if (*h!=10 && *h!=13) break;
				while (*h==10 || *h==13) h++;
				start=h;
			}
			if (start>mxmbuffer) memmove(mxmbuffer,start,strlen(start)+1);
		}
		first=false;


		// Look for prompts:
		char *p=mxmbuffer+l-4;
		while (p>=mxmbuffer)
		{	
			if (strncmp(p,"--->(%i",7)==0) // input prompt
			{	
				*p=0; break; // prompt found, crop off prompt
			}
			else if (strncmp(p,"<---",4)==0) // prompt suffix
			{	
				*p=0; end=1; // found a prompt, end communication
			}
			else if (strncmp(p,"--->",4)==0) // other prompt
			{	
				strcpy(p,p+4);
				maximathread.question=1; break; // other prompt found
			}
			p--;
		}

		if (killo==1 && maximathread.output<=1)
		{	
			p=mxmbuffer;
			while (!outfound && p<mxmbuffer+l)
			{
				if (strncmp(p,"(%o",3)==0)
				{	
					outfound=1;
					char *q=p+3; 
					while (*q && *q!=')') q++;
					if (*q==')')
					{	
						char *p1=p;
						while (p1<=q) *p1++=' ';
					}
					p=q;
					break;
				}
				p++;
			}
		}

		if (maximathread.output==1) output(mxmbuffer);
		else if (maximathread.output==2) maximathread.out.cat(mxmbuffer);
		if (maximathread.out.Error) { Warning("Maxima output too long.","Euler"); break; }
		if (end) break;
	}

	// output new line or not:
	if (maximathread.output==1) output("\n");

	maximathread.Active.clear();
	return 0;		
}


/*
Send a command to Maxima and get back the results.
output=1 : normal output of Maxima results
output=2 : gather results in the buffer, do not print
output=3 : calling line ended with ".." or "...", which was removed
output=4 : never any output in this call, calling line ended with "$$"
mode=0 : do not replace @var and %
mode=1 : replace @var
mode=2 : replace @var and %
mode=3 : replace nothing, and do not truncate commands
mode=4 : replace __ by _ only
*/
int runmaxima (char *s, int output, int mode)
{	
	// print("Maxima called for: %s\noutput=%d mode=%d\n",s,output,mode);

	if (!maximarunning) // Maxima not started
	{	start_maxima(getstartdir());
	}

	if (mode==1 || mode==2)
	{	
		s=replacematrix(s);
		if (error) return 0;
		s=replaceexpressions(s,mode==2);
		if (error) return 0;
	}
	if (mode==4)
	{
		s=replaceunderscore(s);
		if (error) return 0;
	}
	
	while (*s)
	{
		int outf=output;

		char *end=s;
		if (mode==3)
		{
			end=s+strlen(s);
		}
		else
		{
			// truncate one command
			while (*end)
			{	
				if (*end=='\"')
				{	end++;
					while (*end && *end!='\"') end++;
				}
				else if (*end=='\\' && *(end+1)!=0)
				{	end+=2;
				}
				else if (*end==';' || *end=='$')
				{	
					if (*end=='$') outf=0;
					end++; break;
				}
				end++;
			}
			
			if (strlen(s)<2) return 0; // Do not send short commands
		}

		// print("%s\n",s);

		DWORD l;
		if (!WriteFile(eulerout,s,end-s,&l,0) || l==0)
		{	error=1; return 0;
		}
		if (!WriteFile(eulerout,"\n",1,&l,0) || l==0)
		{	error=1; return 0;
		}

		s=end;

		if (output==3 && *end==0) return 0;

		// Note everything in the Maxima thread object
		maximathread.output=outf;
		if (output==2) maximathread.out.clear();

		maximathread.start(); // start thread to get answer

		// Wait for the thread to complete:
		HANDLE obj=maximathread.handle();
		while (1)
		{	
			if (!maximathread.active()) break;
			if (test_key()==escape) // user can escape!
			{	
				maximathread.kill(); // don't know if this always works
				maximarunning=0;
				break;
			}
			// wait for the thread to end:
			if (WaitForSingleObjectEx(obj,1000,true)!=WAIT_TIMEOUT) break;
		}
	}
	return 0;
}

/*
Break Maxima questions by creating an error message and sending it
to Maxima, discarding any output.
*/
void maxima_break ()
{	
	static char *s="1a;\n"; // 1a; is a Maxima error
	runmaxima(s,0,0);
}

/*
Determine, if Maxima is running.
*/
int maxima_running ()
{	return maximarunning;
}

int maxima_question ()
{	return maximathread.question;
}

char *maxima_output ()
{	return maximathread.out.text();
}
