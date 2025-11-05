#ifndef _FILESH

typedef void (*Continuefunction) (int);

/**
Thread to do something outside of the main thread.
Originally written for saving notebooks.
*/
class SaveThread : Thread
{
public :
	Threadfunction threadfunction;
	Continuefunction continuefunction;
	int Res;
	SaveThread () : Thread (0) {}
	void go (Threadfunction f)
	{
		threadfunction=f;
		continuefunction=0;
		start();
	}
	void go (Threadfunction f, Continuefunction g)
	{
		threadfunction=f;
		continuefunction=g;
		start();
	}
	int result ()
	{
		return Res;
	}
	int call ()
	{
		return savethreadrun();
	}
	int savethreadrun ();
};

void savethread (Threadfunction f, Continuefunction g);
void savethread (Threadfunction f);

extern FileSelector loadfile,savebitmap,savepostscript,savesvgfile,saveeditor,setbrowser;
extern String NotebookName,FileName,WindowTitle;

int dosave ();
int dosaveas ();
void doopen ();
void dosavemenu();
void dosaveasmenu();
void doopenintroduction();
void doopenown ();
void dosaveown ();
void docreatezip ();
void dodeletenotebook ();

#define _FILESH
#endif