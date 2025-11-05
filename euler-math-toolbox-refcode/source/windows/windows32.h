#ifndef EASY32_H
#define EASY32_H

//#define WM_NOTIFY				0x004E

class StatusBar
{   Window *W;
	String Text;
	HWND Handle;
	int Panes,Id;
	double *F;
	public :
	enum { popout=SBT_POPOUT, noborders=SBT_NOBORDERS };
	StatusBar (StandardWindow &w, int id=50000);
	StatusBar (StandardWindow &w, int panes, double *f, int id=50000);
	~StatusBar () { if (F) delete F; }
	void settext (char *s, int align=Alignment::left);
	void settext (int pane, char *s, int align=Alignment::left,
		int style=0);
	int setpanes ();
	void setbackground (COLORREF color);
	HWND handle () { return Handle; }
};

typedef struct
{	int iBitmap;
	int idCommand;
	BYTE fsState;
	BYTE fsStyle;
	BYTE bReserved[2];
	DWORD dwData;
	int iString;
} TBBUTTON5;
typedef const TBBUTTON FAR* LPCTBBUTTON;

class Buttons
{	TBBUTTON5 B[256];
	String Tip[256];
	int N,Nb;
	public :
	Buttons () : N(0),Nb(0) {}
	void addbutton (int id, char *tip);
	void addcheck (int id, char *tip);
	void addbutton (int id);
	void addcheck (int id);
	void addseparator ();
	TBBUTTON5 * buttons () { return B; }
	int size () { return N; }
	int nbuttons () { return Nb; }
	char * tip (int id);
	char * tipnumber (int i);
	TBBUTTON5 * button (int i);
};

class ToolBar
{   HWND Handle;
	int Id;
	StandardWindow *W;
	Buttons *B;
	public :
	ToolBar (StandardWindow &w, int bitmapid, Buttons &b,
		int id=6000);
	void press (int id);
	void release (int id);
	void check (int id);
	void uncheck (int id);
	HWND handle () { return Handle; }
	int height ();
	int notify (NMHDR *h);
};

#include <process.h>

typedef int (*Threadfunction) ();

void threadstart (void *parameter);

class Thread
{   int Stacksize;
	int Result;
	Threadfunction F;
	HANDLE Handle;
	public :
	Flag Active;
	Thread (Threadfunction f, int stacksize=4096) :
		F(f),Stacksize(stacksize),Active(0)
	{}
	void start ()
	{   Active.set();
		Handle=(HANDLE)_beginthread(threadstart,Stacksize,this);
	}
	void result (int n) { Result=n; }
	int result () { return Result; }
	virtual int call () { return F(); }
	void suspend () { SuspendThread(Handle); }
	void resume () { ResumeThread(Handle); }
	int active () { return Active; }
	void kill (int k=0) { TerminateThread(Handle,k); Active.clear(); }
	void priority (int h) { SetThreadPriority(Handle,h); }
	HANDLE handle () { return Handle; }
};

class CriticalSection
// Implements a semaphore (lock object) using Windows critical sections.
// Can be called and left in a nested way.
{	
	CRITICAL_SECTION C;
	int Count;
	String Name;
	String Last;
	public :
	CriticalSection (char *name) : Count(0), Name(name), Last("none") { InitializeCriticalSection(&C); }
	~CriticalSection () { DeleteCriticalSection(&C); }
	void enter ();
	void enter (char *text);
	void leave ();
	void leave (char *text);
	int tryenter ();
};

#endif
