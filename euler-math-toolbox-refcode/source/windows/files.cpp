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
#include "translate.h"
#include "../mainloop.h"
#include "../globalvars.h"
#include "../aliases.h"
#include "files.h"

// ****************************** File Selectors **************************************

/*****
Euler uses several file selectors with different purposes.
They have different titles and selection wildcards.
Moreover, the file selectors have different start directories.
Window prefers the start directory of a file selector
to the settings of the selector.
*/

FileSelector savenotebook(textwindow,"",FileSelector::save,
	"Save Notebook","en","Euler Notebooks\0*.en\0All Files\0*.*\0\0");
FileSelector savehtml(textwindow,"",FileSelector::save,
	"Export Notebook to HTML","html","HTML Files\0*.html;*.htm\0All Files\0*.*\0\0");
FileSelector loadnotebook(textwindow,"",FileSelector::load,
	"Load Notebook","en","Euler Notebooks\0*.en\0All Files\0*.*\0\0");

FileSelector loadfile(textwindow,"",FileSelector::load,
	"Load EULER File","e","Euler Files\0*.e\0All Files\0*.*\0\0");
FileSelector savebitmap(textwindow,"",FileSelector::save,
	"Save Graphics as PNG","png","PNG Files\0*.png\0All Files\0*.*\0\0");
FileSelector savepostscript(textwindow,"",FileSelector::save,
	"Save Graphics as Postscript","eps","EPS Files\0*.eps\0All Files\0*.*\0\0");
FileSelector savesvgfile(textwindow,"",FileSelector::save,
	"Save Graphics as SVG","svg","SVG Files\0*.svg\0All Files\0*.*\0\0");
FileSelector saveeditor(textwindow,"",FileSelector::save,
	"Save Editor","e","Euler Files\0*.e\0All Files\0*.*\0\0");
FileSelector setbrowser(textwindow,"",FileSelector::load,
	"Set Browser Excecutable","exe","Exectuables\0*.exe\0All Files\0*.*\0\0");

String NotebookName("");
String FileName("",256);
String WindowTitle("",256);

// ******************************** SaveThread ************************

/**** 
This is a mechanism to save notebooks in a separate thread to ensure
proper reaction times of the program in event handling. The thread
is started by the event handler and provides a thread function (e.g.
dosave()) and a continuation function (e.g. nocontinue()). The latter
is called by a message handler of the text window.

During the save thread the Window is blocked, but not between the end
of the thread and the continuation function.

To prevent recursive generation of the thread, this should only be called
from menu functions.

An example would be saving the notebook and loading a new notebook.
The saving takes place in a blocking thread. And the file selection
for the new notebook in the continuation. Note that loading a notebook
is done in the computation thread.

The continuation function gets a result from the thread. The result
is stored in the one instance of SaveThread that the program is allowed
to use.
****/

// empty continuation function
void nocontinue (int res) {}

int SaveThread::savethreadrun ()
{
	textwindow.enable(false);
	graphicswindow->enable(false);
	noevents=true;
	setstatus("Saving. Please wait!");
	int res=threadfunction(); // call thread function
	setstatus("");
	noevents=false;
	textwindow.enable(true);
	graphicswindow->enable(true);
	Res=res;
	textwindow.usermessage(TextWindow::message_continue,this);
	return res;
}

// Called by the message handler of the text window.
// The message type is message_continue
void docontinue (void *p)
{
	SaveThread *st=(SaveThread *)p;
	if (st->continuefunction)
		st->continuefunction(st->result());
}

// instance of save thread to use
SaveThread savethreadinstance;

// generate a thread for f and g
void savethread (Threadfunction f, Continuefunction g)
{
	savethreadinstance.go(f,g);
}

// generate a thread for f 
void savethread (Threadfunction f)
{
	savethread(f,0);
}

// ********************************* Open ***************************************

// forward declarations
void doopenfinish(char *s);
void doopencontinue (int res);

/**
This opens a new notebook with a file name selected by the user.

If the current notebook was changed, the user will be asked to
save it. This is done in a separate thread to prevent blocking
the user interface on long saves. The thread issues a message
which will continue the opening.

Called from the menu to open a notebook.
*/
void doopen ()
{
	// not while running
	if (!editing || nojump || text->type()!=TextLine::prompt) return;

	// ask if changed notebook should be saved
	if (askconfirmations && text->changed() && text->countlines()>1 &&
			Question("Save this Notebook\nbefore deleting it?","Euler",textwindow)
				==Answers::yes
		)
	{	
		// save in separate thread and continue with opening a new notebook
		savethread(dosave,doopencontinue);
		return;
	}
	// continue with opening a new notebook
	doopencontinue(1);
}

/**
Continue opening a notebook with file selection. The save
thread start this using a Windows message.

If the save thread fails, res will be zero. Then the opening
aborts.
*/
void doopencontinue (int res)
{
	if (!res) return;
	String Name("",256);
	Name.copy(loadnotebook.select());
	if (Name.empty()) return;
	if (alwaysrestart)
	{	
		wantload.copy(Name.text());
		dorestart(0);
	}
	else
	{	
		doopenfinish(Name.text());
	}
	return;
}

extern int noevents;
void setwindowname ();

/**
Finish opening a notebook with a known name.

While opening events are blocked.
*/
void doopenfinish (char *Name)
{	
	noevents=1;

	// extract new home dir from the file name
	String dir(Name);
	dir.stripfilename();
	homedir.copy(dir);
	if (!homedir.endsWith("\\")) homedir.cat("\\");
	sethomedir();

	// open
	FileName.copy(Name);
	if (text->load(FileName))
	{	
		setwindowname();
		text->changed(0);
		addrecent(Name);
	}

	noevents=0;
}

void doopenindircontinue (int res);
String opendir("");

/**
Open a file in the specifed directory, e.g. the toturials and examples, or
the Euler Files in the home directory of the user.

To do this, a special file selector is used. Otherwise, the code copies
doopen().
*/
void doopenindir (char *dir)
{	
	opendir.copy(dir);
	if (!editing || nojump || text->type()!=TextLine::prompt) return;
	if (text->changed() && text->countlines()>1 &&
			(!askconfirmations || Question("Save this Notebook\nbefore deleting it?",
				"Euler",textwindow)==Answers::yes))
	{	
		savethread(dosave,doopenindircontinue);
		return;
	}
	doopenindircontinue(1);
}

void doopenindircontinue (int res)
{
	if (!res) return;
	char *dir=opendir;
	String Name("",MAX_PATH);
	FileSelector loadspecialnotebook(textwindow,"",FileSelector::load,
		"Load Notebook","en","Euler Notebooks\0*.en\0All Files\0*.*\0\0");
	String s(dir); s.cat("*.en");
	loadspecialnotebook.setFile(s);
	loadspecialnotebook.setDir(dir);
	Name.copy(loadspecialnotebook.select());
	if (Name.empty()) return;
	if (alwaysrestart)
	{	
		dorestart(0);
		wantload.copy(Name.text());
	}
	else
	{	
		doopenfinish(Name.text());
	}
}

/**
Open a tutorial or example. Called from the menu.
*/
void doopenintroduction ()
{	
	String Name(getstartdir());
	Name.cat("docs\\Programs\\");
	doopenindir(Name.text());
}

/**
Open a notebook in "Euler Files" in the user home directory.
*/
void doopenown ()
{	
	doopenindir(geteulerfiles());
}


// ********************************************* Save **********************************

// forward declaration
void removeimages (char *filename);

/**
Save a notebook.
Call "Save As ..." if the notebook is new.
This function already runs in a separate thread.
*/
int dosave ()
{	
	int res=0;
	if (*NotebookName==0)
	{	
		// call dosaveas so the user must give the file a name
		return dosaveas();
	}
	else
	{
		res=text->save(FileName);
	}
    text->changed(0);
	return res;
}

/**
Save under the known name or ask for name calling "Save as ..."

This is already in a separate thread.
*/
int doremoveandsave ()
{	
	int res=0;
	if (*NotebookName==0)
	{	
		// call dosaveas so the user must give the file a name
		return dosaveas();
	}
	else
	{
		removeimages(FileName);
		res=text->save(FileName);
	}
    text->changed(0);
	return res;
}

/**
Save a file under the known name. If no name is known,
"Save As ..." is used instead.

Called from the menu.
*/
void dosavemenu ()
{	
	savethread(doremoveandsave);
}

/**
Service routine to quickly save a file under a file name
in the global string FileName.
*/
int saveit ()
{
	text->changed(0);
	return text->save(FileName);
}

/**
Remove all images of the notebook, small and large, including SVG images.
*/
void removeimages (char *name)
{
	String Name(name);
	String imgname(name);
	imgname.stripfilename();
	if (saveimages && imagesindirectory) 
		imgname.cat("images\\");
	String Path(imgname);
	imgname.cat(Name.filename());
	*(imgname.extension())=0;
	int i=1;
	while (true)
	{
		char s[MAX_PATH];
		sprintf(s,"%s-%03d.png",imgname.text(),i);
		int ex=exists(s);
		remove(s);
		sprintf(s,"%s-%03d-large.png",imgname.text(),i);
		ex=ex || exists(s);
		if (exists(s)) remove(s);
		sprintf(s,"%s-%03d.svg",imgname.text(),i);
		ex=ex || exists(s);
		remove(s);
		sprintf(s,"%s-%03d.svgz",imgname.text(),i);
		ex=ex || exists(s);
		remove(s);
		if (!ex) break;
		i++;
	}
	if (imagesindirectory) RemoveDirectory(Path.text());
}

/**
Save the notebook under a name chosen by a file selector.
Ask to delete old notebook.

This is already in a separate thread.
*/
int dosaveas ()
{   
	String OldName(FileName);

	// Select a file name
	String Name("",MAX_PATH);
	Name.copy(savenotebook.select());
	if (Name.empty()) return 0;

	// Set the home directory for the destination
	String dir(Name);
	dir.stripfilename();
	homedir.copy(dir);
	homedir.cat("\\");
	sethomedir();

	// Update the recent file list
	FileName.copy(Name);
	addrecent(Name);

	// Update the window name
	setwindowname();

	// save it
	int ret=text->save(FileName);
    text->changed(0);

	// Ask to delete the old notebook
	if (ret && strcmp(FileName,OldName)!=0 
		&& OldName.length()>0 && exists(OldName.text()))
	{	
		char q[1024];
		sprintf(q,"Keep the old notebook\n%s\nand all its images?",OldName.text());
		if (Question(q,"Keep File?",textwindow)==Answers::no)
		{	
			remove(OldName.text());
			String html(OldName);
			*(html.extension())=0;
			html.cat(".html");
			remove(html.text());
			removeimages(OldName);
		}
	}

	return ret;
}

/**
Save the file under a new name. The user is asked to delete
the old notebook and its images.

Called from the menu.
*/
void dosaveasmenu ()
{	
	savethread(dosaveas);
}

/**
Save the notebook in "Euler Files".

Called from the menu.
*/
void dosaveown ()
{	
	savenotebook.setDir(geteulerfiles());
	String s(geteulerfiles());
	s.cat("*.en");
	savenotebook.setFile(s);
	dosaveasmenu();
}

/**
Delete a notebook and all its images from the disk.

Called from the menu.
*/
void dodeletenotebook ()
{
	if (!editing || nojump || text->type()!=TextLine::prompt) return;

	// File name empty. This would delete all files.
	if (FileName.length()<3) return;

	// first the images
	removeimages(FileName);

	// build a pattern to find all files.

	String Path(FileName);
	*Path.filename()=0;
	if (Path.length()<3) return;

	String File;

	String Name(FileName);
	Name.extension(".*");
	char *pattern=Name.text();

	static struct _finddata_t ff;
	static int another=0;
	static LONG handle;

	// delete all files matching the pattern
	handle=(LONG)_findfirst(pattern,&ff);
	if (handle!=-1)
	{	
		another=1;
		File.copy(Path); File.cat(ff.name);
		remove(File);
		while (true)
		{	
			if (_findnext(handle,&ff)==-1) break;
			File.copy(Path); File.cat(ff.name);
			remove(File);
		}
	}

}

/**
Zips a notebook, its images and all files starting with the file name.
Needs 7z.exe to be installed in the start directory of EMT.

Called from the menu.
*/
int docreatezipgo ()
{
	char line[MAXLINE];

	dosave();

	char *name=NotebookName.text();
	if (*name==0)
	{
		Warning("Notebook is not yet saved.","Euler");
		return 0;
	}

	sprintf(line,"\"%s7z.exe\" a \"%s.zip\" \"%s*\" \"images\\%s*\" -x!\"*.zip\"",
		getstartdir(),name,name,name);

	STARTUPINFO si; // structure to start a process
	PROCESS_INFORMATION pi; // structure to get back process information
	
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	
	ZeroMemory( &pi, sizeof(pi) );

	if (!CreateProcess(0,line,0,0,0,CREATE_NO_WINDOW,0,0,&si,&pi))
	{
		char ws[MAXLINE+255];
		sprintf(ws,"Could not call the 7z cmmand!\nCommand Line\n%s",line);
		Warning(ws,"Euler");
		return 0;
	}

	CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
	return 1;
}

// function for the menu
void docreatezip ()
{	
	savethread(docreatezipgo);
}
