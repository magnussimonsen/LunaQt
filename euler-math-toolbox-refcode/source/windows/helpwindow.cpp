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
#include <windowsx.h>

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

#define HARDSPACE -96
#define MAXITEMLENGTH 256

/*
This file contains classes and functions for the help window in EMT.
The window is opened with F1 or with a double click on a command.

It contains a search line, an "Open in Browser" button and a text area
for the help text. This area contains the breadcrumbs at the top, a
separator line with dashes and the help topic. It displays help
text from a lot of sources.

The help text is provided by the functions in help.c which are part of
the core in EMT, and not system related. The main function for this
is getwindowhelp(). See help.h.
*/

// default sizes, will be stored at program end for the next session.
int helpx = 200, helpy = 200, helpw = 700, helph = 500;

class HelpWindow;
HelpWindow* helpwindow = 0; // pointer to the current help

// This is a link to the command line in the help window.
// See: windows32.h
class HelpCommandItem : public StringItem
{
	MultilineItem* txt;
public:
	HelpCommandItem(int id, Dialog& d, MultilineItem* t)
		: StringItem(id, d, ""), txt(t)
	{
	}
	int command(LPARAM p)
	{
		if (p == EN_UPDATE)
		{
			static char help[MAXHELP];
			static char help1[MAXHELP];
			static char line[MAXHELP];
			strcpy(line, text());

			// get the help text for this command line:
			getwindowhelp(line, help);

			// reformat the line separators
			char* p = help, * p1 = help1;
			int havereturn = 0;
			int linepos = 0;
			while (*p && (p1 - help1) < MAXHELP - 2)
			{
				if (*p == '\r') havereturn = 1;
				else if (*p == '\n')
				{
					if (!havereturn) *p1++ = '\r';
					havereturn = 0;
					linepos = 0;
				}
				else if (*p == '\t')
				{
					p++;
					*p1++ = ' '; linepos++;
					for (int i = 0; i < linepos % 4; i++)
					{
						*p1++ = ' '; linepos++;
					}
					continue;
				}
				else if (*p == '_' && *(p + 1) == '_' && *(p + 2) == '_')
				{
					*p1++ = HARDSPACE;
					p += 3;
				}
				else linepos++;
				*p1++=*p++;
			}
			*p1=0;

			// set the result to the text area in the help window:
			txt->set(help1);
		}
		return 1;
	}
};

void dobrowserhelp (); // defined in below
void opencommandinbrowser (); // defined below

// A link to the button in the help window
class HelpOpenInBrowser : public ButtonItem
{
	StringItem *Cmd;
	public :
	HelpOpenInBrowser (int id, Dialog &d, StringItem *cmd)
		: ButtonItem(id,d), Cmd(cmd)
	{}
	int command (LPARAM p)
	{	
		opencommandinbrowser();
		return 1;
	}
};

// We need to change the handlers for the text area
// to be able to process double clicks.
// Here, we store the default handlers.
WNDPROC DefEditProc,DefTextProc;
 
extern Font *helpfont; // defined in main.c

// This MESSAGE seems to be missing in windows.h sometimes.
#ifndef GWL_WNDPROC
#define GWL_WNDPROC -4
#endif

// The link to the help dialog.
class HelpWindow : public Dialog
{
public :
	MultilineItem *text; // help text
	HelpCommandItem *cmd; // search line
	HelpOpenInBrowser *open; // browser button

	HelpWindow (Window &w, int id) : Dialog(w,id)
	{
		text = new MultilineItem(ID_HelpText,*this,"");
		text->setfont(*helpfont);
		cmd = new HelpCommandItem(ID_HelpCommand,*this,text);
		open = new HelpOpenInBrowser(ID_HelpOpenInBrowser,*this,cmd);
	}
	
	// To enable a flexible layout of the help window, we need to
	// do it ourselves.
	void layout ()
	{
		positionelement(ID_HelpOpenInBrowser,-1,10,10,-1);
		int w=elementwidth(ID_HelpOpenInBrowser);
		positionelement(ID_HelpCommand,10,w+20,10,-1);
		int h=elementheight(ID_HelpCommand);
		sizeelement(ID_HelpText,10,10,h+20,10);
	}
	
	int closing ()
	{
		// save sizes for the next session:
		dogetsize(helpx,helpy,helpw,helph);
		// hide window:
		helpwindow->hide();
		// set default handlers:
		SetWindowLongPtr(cmd->handle(),GWL_WNDPROC,(LONG_PTR)DefEditProc);
		helpwindow=0; // so we know it has been closed
		return 0;
	}
	
	int askok ()
	{
		return 1;
	}
	
	// For external use. Sets the search line for that topic.
	void topic (char *topic)
	{
		dogetsize(helpx,helpy,helpw,helph);
		String s(topic);
		// A search for a topic must end with a blank.
		// We add a blank besides for some exceptions.
		if (s.length()>0 && !s.endsWith(" ") && !s.endsWith("(") && !s.startsWith("?") 
				&& !s.endsWith(".e")) 
			s.cat(" ");

		// Set and handle the topic, start searching and display results.
		// Issues the command for the HelpCommandItem (see its definition above).
		cmd->set(s);
		
		helpwindow->show();
		helpwindow->cmd->setfocus();
	}
};

extern char firsttopic[256];

// New keyboard handler for the input line in the help window.
LRESULT EditProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_KEYDOWN :
			if (wParam == VK_F1)
			{
				textwindow.setfocus();
				return 0;
			}
			else if (wParam == VK_F2)
			{
				opencommandinbrowser();
				return 0;
			}
			break;
		case WM_CHAR :
			if (wParam == VK_RETURN)
			{
				helpwindow->topic(firsttopic);
				return 0;
			}
			else if (wParam == VK_ESCAPE) 
			{
				if (strlen(helpwindow->cmd->text())==0) 
				{	
					helpwindow->dogetsize(helpx,helpy,helpw,helph);
					helpwindow->hide();
				}
				else helpwindow->cmd->set("");
				return 0;
			}
	}
	return CallWindowProc(DefEditProc, hwnd, uMsg, wParam, lParam);
}

// New event handler for the text display in the help window.
LRESULT TextProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char c;

	switch (uMsg)
	{
		case WM_KEYDOWN :
			if (wParam == VK_F1) opencommandinbrowser();
			return 0;
		case WM_KEYUP :
			return 0;
		case WM_CHAR :
			if (wParam == VK_ESCAPE) 
			{
				if (strlen(helpwindow->cmd->text())==0) helpwindow->hide();
				else helpwindow->cmd->set("");
			}
			c=(char)GET_WM_VKEYTOITEM_CODE(wParam,lParam);
			if (c==3 || c==22) break;
			return 0;
		case WM_LBUTTONDBLCLK: // double click
			static char line[MAXITEMLENGTH]="";
			int pos=(int)LOWORD(SendMessage(helpwindow->text->handle(),EM_CHARFROMPOS,wParam,lParam));
			char *t=helpwindow->text->text();
			if (xisalpha(*(t+pos)))
			{
				char *start=(t+pos),*end=(t+pos);
				// We go back and forth from the current position in the text:
				while (start>t && 
					(xisalpha(*(start-1)) || *(start-1)=='&' || *(start-1)=='_' 
					|| *(start-1)=='$' || *(start-1)=='%' || *(start-1)==HARDSPACE || *(start-1)=='?'))
				{
					start--;
				}
				while (*(end+1) && 
					(xisalpha(*(end+1)) || xisdigit(*(end+1)) || *(end+1)=='$' || *(end+1)=='_'
					|| *(end+1)==HARDSPACE))
				{
					end++;
				}
				if (*(end+1)=='.' && *(end+2)=='e') end+=2;
				char c=*helpwindow->cmd->text();
				char *l=line;
				*l=0;
				if (*start!='&' && start>l && *(start-1)!='>' && c=='&' || c==' ' 
					|| *helpwindow->cmd->text()=='&' || *helpwindow->cmd->text()==' ') 
				{
					if (*start!=' ' && *start!='&') strcat(l++,"&");
				}
				if (end>start && end-start<MAXITEMLENGTH-1)
				{
					char* pl = l;
					for (char *p = start; p <= end; p++)
					{
						*pl++ = *p;
					}
					*pl++ = 0;
					if (*helpwindow->cmd->text()=='?') putlasttopic(helpwindow->cmd->text(),2);
					helpwindow->topic(line);
				}
			}
			return 0;
	}
	return CallWindowProc(DefTextProc, hwnd, uMsg, wParam, lParam);
}

void browse (char *line);

void opencommandinbrowser ()
{
	String H(helpwindow->cmd->text());
	char *h=H;
	if (!*h)
	{
		browse("reference/overview.html");
		return;
	}
	if (*h=='*') helpwindow->topic(firsttopic);
	if (*h==' ' || *h=='&') h++;
	size_t l=strlen(h);
	if (l>0 && (h[l-1]==' ' || h[l-1]=='(')) h[l-1]=0;
	static String s;
	s.copy("reference/index.html#");
	s.cat(h);
	browse(s.text());
}

extern int wscreen,hscreen;

// Open the help window. Bind to key F1 in main.c.
void doopenhelpwindow ()
{
	if (helpwindow)
	{
		helpwindow->show();
		return;
	}

	helpwindow=new HelpWindow(textwindow,IDD_HelpWindow);
	helpwindow->fitsize(wscreen,hscreen,helpx,helpy,helpw,helph,500,300);
	helpwindow->cmd->set(getstatustopic());
	helpwindow->create();

	// change the keyboard handling for the edit and text window
	DefEditProc = (WNDPROC)SetWindowLongPtr(helpwindow->cmd->handle(),GWL_WNDPROC,(LONG_PTR)EditProc);	
	DefTextProc = (WNDPROC)SetWindowLongPtr(helpwindow->text->handle(),GWL_WNDPROC,(LONG_PTR)TextProc);	
}

/**
Open the help window with the current topic, or open the browser if the
current line starts with see: or http:.
*/
void doeulerhelp ()
{	
	static char line[MAXLINE]="";
	if (!text->bracketmarked()) text->getMarkedWord(line);

	if (strstarts(line,"http") ||
		strstarts(line,"See: ") || strstarts(line,"see: "))
	{
		browse(line);
	}
	else
	{
		if (!helpwindow) doopenhelpwindow();

		char s[MAXLINE];
		
		if (strlen(line)==0) 
		{
			strcpy(s,getstatustopic());
		}
		else
		{
			if (text->isinmaxima() || maximamode) sprintf(s,"&%s",line);
			else strcpy(s,line);
		}

		helpwindow->topic(s);
	}
}

/**
Open the help window with a specific topic
*/
void doeulerhelp (char *topic)
{
	if (!helpwindow) doopenhelpwindow();
	helpwindow->topic(topic);
}

void dooverview()
{
	doeulerhelp("");
}

/**
Open a the marked help in the browser (could be
a page for in a see:... or http:... line or the reference
at the marked word), or open the main page of the documentation.
*/
void dobrowserhelp ()
{	
	static char line[MAXLINE]="";
	if (!text->bracketmarked()) text->getMarkedWord(line);

	if (strstarts(line,"http") ||
		strstarts(line,"See: ") || strstarts(line,"see: "))
	{
		browse(line);
	}
	else if (*line)
	{
		String s="reference/index.html#";
		if (*line=='&') s.cat(line+1);
		else s.cat(line);
		browse(s);
	}	
	else
	{
		browse("reference/index.html");
	}
}

