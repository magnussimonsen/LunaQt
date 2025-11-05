#include "windows.h"

#include "../header.h"
#include "../sysdep.h"
#include "../graphics.h"
#include "edit.h"
#include "text.h"
#include "main.h"
#include "../help.h"
#include "../maxima.h"

int nojump;

extern int outputing;
extern int maximamode;
extern int yacasmode;

extern int statusalternatives,statusalternative;

void line_help ()
/*
Extend the command at the current cursor position to one of the
possible commands.
We use the help for the status line to get a list of all possible
commands.
*/
{	
	static char *phelpextend=0;
	static char helpextend[MAXLINE],helpstatus[MAXLINE];
	static char search[MAXLINE]="",se[MAXLINE]="";
	int extendsize=0;
	char *s=text->currentline(); int pos=text->cursorpos();
	if (pos<=lefteditlimit) return;
	if (yacasmode) return;
	
	// search the start of the string at the cursor position
	int k=pos;
	while (pos>lefteditlimit && (xisalpha(s[pos-1]) || xisdigit(s[pos-1]))) pos--;
	if (k-pos>128) return;
	extendsize=k-pos;
	if (extendsize==0) return;
	
	// copy the string as the search string
	strncpy(search,s+pos,extendsize);
	search[extendsize]=0;

	// if this is not what we previously inserted
	// we get a new help line
	if (strcmp(search,se)) 
	{	
		if (text->isinmaxima() || maximamode)
		{	if (statusalternatives>0)
			{	statusalternative++;
				statusalternatives=
					getmaximahelp(search,helpextend,helpstatus,statusalternative);
				setstatus(helpstatus);
				return;
			}
			getmaximahelp(search,helpextend,helpstatus,0);
		}
		else 
			gethelp(search,helpextend,helpstatus);
		phelpextend=helpextend;
	}
	
	statusalternatives=0;
	statusalternative=0;
	
	// extract the next string from the help line
	char *q=phelpextend;
	while (*q && *q!=' ') q++;
	if (q-phelpextend>128) return;
	
	// insert the string
	if (q>phelpextend)
	{	
		// delete the previous insert
		text->setcursorpos(pos);
		for (int i=0; i<extendsize; i++) text->delchar();

		// copy the new help string to se
		strncpy(se,phelpextend,q-phelpextend);
		se[q-phelpextend]=0;
		
		// insert se at the text position
		text->inserttext(se);
		text->setcursorpos(pos+(int)strlen(se));
		
		// go the start of the next help string
		while (*q==' ') q++;
		if (*q==0) q=helpextend;
		phelpextend=q;
	}
}

void cpy (char *dest, char *source)
{	memmove(dest,source,strlen(source)+1);
}

#define MAXHIST 32
char history[MAXHIST][MAXLINE];
int act_history=0,hist=0;

void put_history (char *s)
{	int i;
	if (act_history>=MAXHIST)
	{	for (i=0; i<MAXHIST-1; i++) strcpy(history[i],history[i+1]);
		act_history=MAXHIST-1;
		hist--; if (hist<0) hist=0;
	}
	strncpy(history[act_history],s,MAXLINE-2);
	if (hist==act_history) hist++;
	act_history++;
}

void prompt (void)
{	if (!outputing) gprint("\n>");
	if (!udf) output(">");
	else output("$");
}

void fkinsert (int i, int *pos, char *s)
{	char *p;
	p=fktext[i];
	if (strlen(s)+strlen(p)>=254) return;
	cpy(s+*pos+strlen(p),s+*pos); memmove(s+*pos,p,strlen(p));
	output1("%s",p); *pos+=(int)strlen(p);
}

int nomultiline=0,inevaluation=0;

int edit_hook ()
/* 
Called before the line is passed to the interpreter.
the input line can be adjusted here.
Currently, this is used to start a multi-line command.
*/
{	
	// are we already in automatic execution mode?
	if (automaticreturn || runcommands>0 || nomultiline) return 0;
	
	// get the cursor line
	TextLine *l=text->cursorline();

	// next line does not exist or is empty?
	if (!l->isFunctionLine()
		&& (!l->nextprompt() || l->nextprompt()->length()<2)
		&& l->endsWith("...")) 
	{	
		return -1; // run only this line
	}

	if (l->next() && l->next()->type()==TextLine::udf && l->endsWith("..."))
	{
		int res=1;
		l=l->next();
		while (l->next() && l->next()->type()==TextLine::udf)
		{	l=l->next(); res++;
		}
		runcommands=res;
		return res;
	}
	else
	{	
		int res=0;
		while (l) // go back and execute from there
		{	
			TextLine *lprev=l->prev();
			// goto the previous prompt (or 0, if there is none)
			while (lprev && lprev->type()!=TextLine::prompt)
			{	
				lprev=lprev->prev();
			}
			// stop if this is not part of a multiline
			if (!lprev || !lprev->endsWith("...") 
					|| lprev->nextcommand()->type()==TextLine::udf
					|| (lprev->isFunctionLine()
						&& lprev->next()->type()==TextLine::udf)) 
						break;
			l=lprev;
			res++;
		}
		if (res==0 && text->cursorline()->endsWith("..."))
		{	
			l=text->cursorline(); res=1;
		}
		if (res)
		{	
			TextLine *last=l;
			text->setcursorline(l,0);
			res=1;
			while (l)
			{	
				l=l->next();
				while (l && l->type()!=TextLine::prompt) l=l->next();
				if (!l || !l->endsWith("...")) break;
				last=l;
				res++;
			}
			if (!l || l->length()<2)
			{	text->setcursorline(last,0);
				return 0;
			}
			runcommands=res; // set number of commands to run automatically
		}
		return res;
	}
}

void editkey (char ch, int scan)
/*
This function is called from the windows event handler.
If a key is pressed in the text window, and editing=1, the handler
will deliver the key here.
*/
{	
	// dump("editkey(%d,%d)",ch,scan);

	static int last=0; // remembers last key

	// show the text window
	if (!textmode && scan!=switch_screen) text_mode();
	
	if ((ch>=32 || ch<0) && ch!=127)
		// A character was pressed
	{	
		text->unmarkall(); // no marked text after inserted character
		// dump("insert %d",ch);
		text->clearquestion(); // If at a ?variable pattern, delete this
		text->insertchar(ch); // insert the character into the notebook
	}
	else
		// Other keys, which are no characters
	{	
		if (scan==cursor_up && last!=cursor_up)
			// this is control-up, since up is handled by the event handler
		{	
			hist=act_history;
		}

		// see, where we are
		char *s=text->currentline();
		int pos=text->cursorpos();

		TextLine *l=text->cursorline();
		switch (scan)
		{	
			case enter : // enter key (not control-enter etc.)
				if (udf==2)
					// udf edit mode
				{
					if ((l->next() && l->next()->type()==TextLine::udf 
							&& l->next()->length()>1)
						|| l->endsWith("endfunction"))
							// return pressed and the next line is a non-empty udf line.
							// this aborts the edit mode.
					{
						endfunction:
						udf=0;
						while (l->prev() && l->type()==TextLine::udf) l=l->prev();
						if (l->type()!=TextLine::prompt)
							// should not happen
						{
							text->setcursorline(l,1);
							text->insertcommand();
							text->cursorline()->type(TextLine::prompt);
							text->cursorline()->copy(">");
							return;
						}
						text->setcursorline(l,1);
						int res=1;
						l=l->nextcommand();
						while (l->next() && l->next()->type()==TextLine::udf)
						{	l=l->next(); res++;
						}
						runcommands=res;
						computation.resume();
						return;
					}
					else if (!l->next() || l->next()->type()!=TextLine::udf)
						// return pressed, and next line is not an udf line.
					{
						if (l->length()<=1)
						{
							l->copy("$endfunction");
							goto endfunction;
						}
						char *p=l->text()+1;
						int pos=1;
						text->insertline();
						l=text->cursorline();
						l->type(TextLine::udf);
						l->copy("$");
						while (*p==' ') { p++; l->cat(" "); pos++; }
						text->setcursorline(l,pos);
					}
					else text->linedown();
				}
				else if (udf==1)
					// line by line udf mode
				{
					if (!l->next() || l->next()->type()!=TextLine::udf)
						// return in an empty line finishes the function
					{
						if (l->length()<=1) l->copy("$endfunction");
					}
					computation.resume();
				}
				else if (!nojump && edit_hook()<0)
					// line ends with ... and there is no next line
					// note: edit_hook() has a side effect to position the cursor back
					// and set runcommands to automatically run the next commands.
				{	
					text->insertline();
					text->newprompt();
				}
				else 
					// usual reaction: resume computation
				{	
					computation.resume();
					if (outfile) fprintf(outfile,"%s",text->currentline()); // deprecated
				}
				break;
			case cursor_left :
				text->cursorleft(lefteditlimit);
				break;
			case cursor_right :
				text->cursorrighttyped();
				break;
			case backspace :
				if (pos>lefteditlimit)
				{	
					text->cursorleft(lefteditlimit);
					text->delchar();
				}
				break;
			case deletekey :
				text->delchar();
				break;
			case switch_screen :
				if (!udf)
				{
					if (textmode) 
					{
						if (intextwindow) showgraphics=1;
						graphic_mode();
					}
					else text_mode();
				   	return;
				}
				else text->insertchar(9);
				break;
			case line_start :
				text->setcursorpos(lefteditlimit);
				break;
			case line_end :
				text->setcursorpos(text->cursorline()->length());
				break;
			case clear_home :
				if (!udf) text->set("",lefteditlimit);
				break;
			case word_left :
				if (pos>lefteditlimit) pos--;
				while (pos>lefteditlimit && s[pos]!=' ') pos--;
				text->setcursorpos(pos);
				break;
			case word_right :
				s=text->currentline(); pos=text->cursorpos();
				if (s[pos]) pos++;
				while (s[pos] && s[pos]!=' ') pos++;
				text->setcursorpos(pos);
				break;
		    case cursor_up :
		    	if (hist) 
				{	hist--; text->setinput(history[hist],0);
				}
				break;
		    case cursor_down :
		    	if (hist<act_history-1) 
				{	hist++; text->setinput(history[hist],0);
				} 
			    break;
		    case help :
		    	line_help();
		    	break;
			case escape :
				if (udf==2)
					// currently editing a function in the notebook
				{
					udf=0;
					while (l->prev() && l->type()==TextLine::udf) l=l->prev();
					if (l->type()!=TextLine::prompt)
						// should not happen
					{
						text->setcursorline(l,1);
						text->insertcommand();
						text->cursorline()->type(TextLine::prompt);
						text->cursorline()->copy(">");
						return;
					}
					text->setcursorline(l,1);
				}
				else if (udf==1)
				{
					if (l->length()<=1) l->copy("$endfunction");
					computation.resume();
				}
				else if (nojump)
				{	text->setinput("escape",0); computation.resume();
				}
				else
				{	hist=act_history;
				}
				break;
			case fk1 : text->inserttext(fktext[0]); break;
			case fk2 : text->inserttext(fktext[1]); break;
			case fk3 : text->inserttext(fktext[2]); break;
			case fk4 : text->inserttext(fktext[3]); break;
			case fk5 : text->inserttext(fktext[4]); break;
			case fk6 : text->inserttext(fktext[5]); break;
			case fk7 : text->inserttext(fktext[6]); break;
			case fk8 : text->inserttext(fktext[7]); break;
			case fk9 : text->inserttext(fktext[8]); break;
			case fk10 : text->inserttext(fktext[9]); break;
			default : 
				// ignore all other keys!
				return;
		}
		if (text->bracketmarked()) text->unmarkall();
		last=scan;
	}
	text->showcursor();
	text->markbracket();
	textwindow.update();
}
