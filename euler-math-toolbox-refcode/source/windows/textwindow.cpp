/**
The text window is the notebook window of EMT.
It manages a data structure of type Text, which contains
all the text and images of a notebook.
*/

#include "windows.h"
#include "../sysdep.h"
#include "colors.h"
#include "text.h"
#include "euler.h"
#include "../yacas.h"
#include "../header.h"
#include "../maxima.h"
#include "graphicswindow.h"
#include "main.h"
#include "helpwindow.h"
#include "spell.h"
#include "translate.h"
#include "../help.h"

// We use this critical section to avoid problems with multitasking
// when the text is changed.
extern CriticalSection textcritical;

extern HANDLE CurrentProcess;

extern int fixmaximalinel,textwidth,fixlinelength;

extern void docontinue (void *p);

/**
Handle user messages. 
These messages are delivered to the text window by the Windows
message system. They are initiated by other functions in the
code of Euler.
*/
void TextWindow::user (int message, Parameter p)
{	
	if (message==message_quit) 
	{
		doexit();
	}
	else if (message==message_caret && waiting)
	{	
		if (CaretOn) showcaret();
		else hidecaret();
	}
	else if (message==message_continue)
	{
		docontinue(p);
	}
	else if (message == message_size)
	{
		textwindow.sized();
	}
}

/**
Handle a resizing of the text window.
*/
void TextWindow::sized ()
{   
	textcritical.enter("sized");
	delete courier;
	courier=new Font(TextFontName,rd(tfontheight*factor()),
		Font::normal,Font::fixed);
	delete courierbold;
	courierbold=new Font(TextFontName,rd((tfontheight+1)*factor()),
		Font::bold,Font::fixed);
	delete courierlarge;
	courierlarge=new Font(TextFontName,
		rd(tfontheight*factor()*4/3),Font::bold,Font::fixed);
	WindowPS ps(*this);
	ps.font(*courier);
	int oldlinelength=linelength;
	if (fixlinelength) linelength=70;
	else linelength=(ps.width()-outputindent*2)/ps.textavewidth();
	if (linelength!=oldlinelength) fixmaximalinel=linelength;
	int w,h;
	ps.textextent("m",w,h);
	textwidth=w*linelength;
	if (text) text->sized();
	if (intextwindow) graphicswindow->timedsized(1);
	textcritical.leave();
}

/**
Handle scroll events by translating them to page events.
*/
void TextWindow::scroll (int flags, int type, int pos)
{   
	if (!editing && !waiting) return;
	if (flags==vscroll)
	{	switch (type)
		{	case pageup :
				text->pageup();
				break;
			case pagedown :
				text->pagedown();
				break;
			case lineup :
				text->scrollup();
				break;
			case linedown :
				text->scrolldown();
				break;
			case scrollposition :
				text->setscroll(pos);
				break;
		}
	}
	else if (flags==hscroll)
	{	int o=text->getoffset();
		int factor=10;
		switch (type)
		{	case lineup :
				if (o==0) return;
				o-=1*factor;
				if (o<0) o=0;
				text->setoffset(o);
				break;
			case linedown :
				o+=1*factor;
				text->setoffset(o);
				break;
			case pageup :
				if (o==0) return;
				o-=10*factor;
				if (o<0) o=0;
				text->setoffset(o);
				break;
			case pagedown :
				o+=10*factor;
				text->setoffset(o);
				break;
			case scrollposition :
				text->sethscroll(pos);
				break;
		}
		text->sethscroll();
		update();
	}
}

int exit ();
extern int wantquit;

int TextWindow::close ()
{   
	if (wantquit) return 1;
	critical.enter();
	wantquit=exit();
	critical.leave();
	return wantquit;
}

void editkey (char ch, int scan);
void stopcrop ();
void endcrop();

int noevents=0;

extern int screenchanged;

/**
Handle key events in the text window.
*/
void TextWindow::key (int flags, int code, int scan)
{  	
	if (screenchanged)
		// F11 was pressed and sized() is necessary to fix cursor
	{
		textwindow.sized();
		screenchanged = 0;
	}


	if (noevents) return; // the text window does not accept keys now
	
	 // see, if the text window is busy (e.g. load or save notebook)
	if (!textcritical.tryenter()) return;
	textcritical.leave();

	static int alternate=0;

	// control pressed and realeased
	if (flags&Keycode::up && scan==VK_CONTROL)
	{
		alternate=!alternate;
	}

	// react on key down only
	if (!(flags&Keycode::down)) return;

	// if in crop mode, interpret return and escape
	if (cropmode)
	{	
		alternate=0;
		if (flags&Keycode::charkey)
		{	if (code==VK_ESCAPE) { stopcrop(); return; }
			else if (code==VK_RETURN) { endcrop(); return; }
			else { stopcrop(); return; }
		}
		graphicswindow->top();
		return;
	}

	// dump("%d %d %d\n",flags,code,scan);

	// do not react on naked control keys
	if (scan==VK_CONTROL) { return; }
	if (scan==VK_SHIFT) { alternate=0; return; }
	
	int shift=0; // shift was pressed along with key

	if (flags&Keycode::charkey)
		// A character key was pressed.
		// Note that enter, escape and tab also return as character keys.
	{	
		Code=code; Scan=0;
		if (alternate) Code=translate_code((char)Code);
		alternate=0;
		switch (Code)
		{	
			case 13 : 
				Scan=enter;
				break;
			case 32 :
				if (Control()) Code=hardspace;
				break;
			case 27 :
				if (udf!=2 || !editing)
				{
					if (text->endfunction())
					{	Scan=enter; break;
					}
					else if (editing && text->inudf())
					{	break;
					}
#ifdef YACAS
					interrupt_yacas();
#endif
					if (CurrentProcess!=0)
					{
						TerminateProcess(CurrentProcess,0);
						CurrentProcess=0;
					}
				}
				Scan=escape; Code=27; break;
			case 9 : 
				Scan=switch_screen; Code=9; break;
		}
	}
	else if (flags&Keycode::virtualkey)
		// A virtual key was pressed. 
		// These are cursor keys and function keys etc.
	{   
		Code=0;
		switch (scan)
		{	
			case VK_CANCEL :
				Scan=escape; break;
			case VK_UP :
				if (!editing || Control() || code==VK_CONTROL) Code=Scan=cursor_up;
				else if (Shift() && editing) { text->shift(1); text->lineup(); return; }
				else { text->shift(0); text->lineup(); text->markbracket(); return; }
				break;
			case VK_DOWN :
				if (!editing || Control() || code==VK_CONTROL) Code=Scan=cursor_down;
				else if (Shift() && editing) { text->shift(1); text->linedown(); return; }
				else { text->shift(0); text->linedown(); text->markbracket(); return; }
				break;
			case VK_LEFT :
				if (Control()) 
				{	if (Shift() && editing) shift=1;
					Scan=word_left;
				}
				else 
				{	if (Shift() && editing) shift=1;
					Scan=cursor_left;
				}
				break;
			case VK_RIGHT :
				if (Control()) 
				{	if (Shift() && editing) shift=1;
					Scan=word_right;
				}
				else 
				{	if (Shift() && editing) shift=1;
					Scan=cursor_right;
				}
				break;
			case VK_DELETE :
				if (editing && text->cursorinmark() && !text->bracketmarked())
				{	text->deletemark(); return;
				}
				Scan=deletekey; 
				break;
			case VK_INSERT :
				Scan=help; break;
			case VK_HOME :
				if (Control()) 
				{	Scan=0; text->gototextstart(); return;
				}
				else if (Shift())
				{	if (editing) shift=1;
				}
				Scan=line_start; 
				break;
			case VK_END :
				if (Control()) 
				{	Scan=0; text->gototextend(); return;
				}
				else if (Shift())
				{	if (editing) shift=1;
					if (!text->endfunction()) Scan=line_end; 
				}
				else 
				{	if (!text->endfunction()) Scan=line_end; 
				}
				break;
			case VK_F1 :
				// F1 is caught by a menu entry. Needs Shift-F1, or Ctrl-F1
				Scan=fk1; break;
			case VK_F2 :
				Scan=fk2; break;
			case VK_F3 :
				Scan=fk3; break;
			case VK_F4 :
				Scan=fk4; break;
			case VK_F5 :
				Scan=fk5; break;
			case VK_F6 :
				Scan=fk6; break;
			case VK_F7 :
				Scan=fk7; break;
			case VK_F8 :
				Scan=fk8; break;
			case VK_F9 :
				Scan=fk9; break;
			case VK_F10 :
				Scan=fk10; break;
			case VK_PRIOR :
				if (editing)
				{
					text->shift(Shift());
					if (Control()) text->pageup(1);
					else text->pageup();
					return;
				}
				Scan=page_up;
				break;
			case VK_NEXT :
				if (editing)
				{
					text->shift(Shift());
					if (Control()) text->pagedown(1);
					else text->pagedown();
					return;
				}
				Scan=page_down;
				break;
			case VK_BACK :
				if (editing && text->cursorinmark() && !text->bracketmarked())
				{	text->deletemark(); return;
				}
				Scan=backspace;
				break;
			default : 
				return;
		}
	}
	else 
	{
		return; // ignore all other keys (composite codes etc.)
	}

	// update the dragged, marked text
	text->shift(shift);
	
	// if the program was only waiting for a mouse click with the mouse() function,
	// resume the computation thread.
	if (mousewaiting)
	{	
		computation.resume();
		return; 
	}

	// dump("%d %d",Code,Scan);

	if (editing)
		// we are in the editing mode
	{	
		editkey((char)Code,Scan); // parse to the editkey function
		starthelptimer(); // start the time to update the status line
	}
	else if (waiting)
		// we are waiting with the wait() function.
	{	
		computation.resume();
		Sleep(0);
	}

	// kill the wait taimer
	if (T)
	{	T->kill(); T=0;
	}
}

/**
Handle clicks into the text window.
*/
void TextWindow::clicked (int x, int y, int type)
{   
	if (intextwindow && showgraphics) 
	{
		graphicswindow->clicked(x,y,type);
		return;
	}
	if (noevents) return;
	if (!editing || !wantload.empty()) return;
	if (!textcritical.tryenter()) return;
	static int dragging=0;
	static Timer *t=0;
	switch (type)
	{   
		case ClickType::button1down :
    		text->unmarkall();
			dragging=text->startdrag(x,y);
			if (dragging) capture(1);
			break;
		case ClickType::button1double :
			text->markcommand(x,y);
			doeulerhelp();
			break;	
		case ClickType::mousemove :
			if (!dragging) break;
			if (y<0 || y>height())
			{	
				if (!t) t=new Timer(*this,20,2);
				Dragscroll=(y<0)?-1:1;
				break;
			}
			if (t) delete t;
			t=0; Dragscroll=0;
			text->drag(x,y);
			break;
		case ClickType::button1up :
			if (t) delete t;
			t=0; Dragscroll=0;
			if (dragging)
			{   text->enddrag(x,y); capture(0); dragging=0;
			}
			break;
		case ClickType::button2down :
			text->rightclicked(x,y);
			break;
	}
	textcritical.leave();
}

/*
Handle timer events for the text window.

1 : Resume computation.
2 : Drag in the window.
3 : Fill status line with help.
4 : Set status line while computing.
5 : Update the content.

*/
void TextWindow::timer (int n)
{   
	if (noevents) return;
	if (n==1)
		// wait timer for the wait() command
	{	
		if (T) T->kill(); 
		T=0;
		Scan=Code=0;
		if (waiting) computation.resume();
	}
	else if (n==2)
		// scroll timer while dragging
	{	
		if (Dragscroll==1)
		{	text->scrolldown();
			text->drag(0,height());
		}
		else if (Dragscroll==-1)
		{	text->scrollup();
			text->drag(0,0);
		}
	}
	else if (n==3) 
		// help timer for the status line
	{	
		if (noevents) return;
		if (H) H->kill();
		if (editing) 
		{	
			char shelp[MAXLINE];
			char sfoundhelp[MAXLINE];
			char sstatus[MAXLINE];

			critical.enter();
			char line[MAXLINE];
			line[0]=0;
			char *p=line;
			if (strlen(text->currentline())<MAXLINE-2)
				strcpy(line,text->currentline());
			char *q=line+text->cursorpos();
			critical.leave();
			
			sfoundhelp[0]=0;
			if (*p=='>') p++;
			if (text->isinmaxima() || maximamode)
			{	
				if (strlen(p)>=1)
				{	char *q1=q;
					if (*(q-1)=='(') q--;
					while (q>p && (xisalpha(*(q-1)) || *(q-1)=='_' 
						|| xisdigit(*(q-1)))) q--;
					if (q1>q)
					{	
						strncpy(shelp,q,q1-q);
						shelp[q1-q]=0;
						static char oldse[MAXLINE]="";
						statusalternatives=getmaximahelp(shelp,sfoundhelp,sstatus,0);
						if (strcmp(oldse,sstatus))
						{	statusalternative=0;
							strcpy(oldse,sstatus);
						}
						else
						{	if (statusalternative>statusalternatives) statusalternative=0;
							getmaximahelp(shelp,sfoundhelp,sstatus,statusalternative);
						}
					}
					else *sstatus=0;
				}
			}
			else if (*p=='>')
			{	
				strcpy(sfoundhelp,">>> Enter a Yacas command!");
			}
			else if (!strncmp(p,"function ",8) && text->cursorpos()<10)
			{
				strcpy(sstatus,">>> Press F9 to edit the function");
			}
			else
			{	
				if (strlen(p)>=1)
				{	
					char *q1=q;
					if (*(q-1)=='(') q--;
					while (1) 
					{
						if (q-1>p && *(q-1)=='_' && *(q-2)=='_') q-=2;
						else if (q-1>p && *(q-2)=='\\') q-=2;
						else if (xisalpha(*(q-1)) || xisdigit(*(q-1))) q--;
						else break;
					}
					if (q1>q)
					{	
						strncpy(shelp,q,q1-q);
						shelp[q1-q]=0;
						gethelp(shelp,sfoundhelp,sstatus);
					}
				}
			}
			setstatus(sstatus);
		}
	}
	else if (n==4)
		// runs once while the computing thread is active
	{	
		if (noevents) return;
		if (!editing) setstatus(">>> Computing. Press ESC to abort.");
		if (CT) CT->kill();
	}
	else if (n==5)
		// update timer, runs all the time to update the graphics window
	{	
		if (NeedUpdate && !(runcommands>0 && runcommands<20))
		{	
			if (showgraphics && !waiting) return;
			if (textcritical.tryenter())
			{
				text->redraw(); // Do the redraw
				text->setscroll();
				textcritical.leave();
				NeedUpdate=0;
			}
			// else update();
		}
		// stopupdatetimer();
	}
}

int lastwidth=0;

extern String StatusText;

/**
The redraw routine of the text window as called by the Windows system.
Unless the graphics is currently shown, it will defer the redraw
for the next timer event.
*/
void TextWindow::redraw (PS &ps)
{	
	if (!textmode && showgraphics)
	{
		textwindow.hidecaret();
		graphicswindow->redraw(ps);
	}
	else needupdate(1);
	setstatus(StatusText);
}

/**
Handle iconization of the text window.
This will iconize or deiconize the graphics window too.
*/
void TextWindow::statechanged (WPARAM s)
{	
	if (!intextwindow)
	{
		if (s==StandardWindow::iconic) graphicswindow->hide();
		else if (s==StandardWindow::normal && !intextwindow) graphicswindow->show();
	}
}
