int fcompute ();

extern int usecolors,gscreenlines,agmode;

extern PS *ops;

extern String GraphicsFontName;

class ComputationThread : public Thread
{	int Argc;
	char **Argv;
	public :
	ComputationThread () : Thread(fcompute) {}
	void set (int n, char **a) { Argc=n; Argv=a; }
	int argc () { return Argc; }
	char ** argv () { return Argv; }
};

extern ComputationThread computation;

extern int lefteditlimit;

void addrecent (char *s);
void makerecent ();

extern int mousewaiting,maximamode;
extern CriticalSection critical;

extern String wantload;

extern int intextwindow,textmode,showgraphics,wchar,hchar,boldfont;
extern Font *couriersmall,*couriersmallup,*couriersmalldown;
extern int statusalternatives,statusalternative;
extern double cropleft,croptop,cropright,cropbottom;
extern int automatic,runcommands,udfon;
extern int noevents;

int doreplay ();

#include "euler.h"

class HelpButton : public ButtonItem
{	
	String S;
	public :
	HelpButton (Dialog &d, String s) :
		ButtonItem(ID_Help,d),S(s) {}
	virtual int command (LPARAM p);
};

extern int askconfirmations,alwaysrestart;

extern String homedir;

int doremoveandsave ();
void dorestart (int ask);
char *geteulerfiles ();