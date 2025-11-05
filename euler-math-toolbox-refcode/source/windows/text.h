extern Flag CaretOn; // Global flag for the blinking caret (text cursor)

class Text;

#define MAXBACKUPS 8
#define MAXMISSPELLED 16
extern int outputindent;

extern int enlarge;
extern double enlargefactor;
#define factor() (enlarge?enlargefactor:1.0)
#define rd(c) ((int)(c+0.5))

/*

Important class, holding a line of text for the Euler text window.
The text lines are connected in a chained list.

There four types of screen lines.
output - for Euler output
prompt - for Euler input
udf - for programs written in the Euler text window
comment - for Euler comment

Bitmaps are special output lines. They are represented by a number of
output lines with the bitmapline flag set. The first line contains
a pointer to the Bitmap.

*/
class TextLine : public String
{   
	TextLine *Prev,*Next; // for the list structure
	int Type; // for the type of the line
	int Mark; // for a marked (inverted) text line.
	int BitmapMark; // for a marked (inverted) bitmap.
	Text *T; // the text display this line belongs to
	String *Backup[MAXBACKUPS]; // backup of this line
	int LastBackup;
	String *Latex;
	String *SVG;
	int mathjax;

public :
	short misspelled[MAXMISSPELLED];
	short misspelledlength[MAXMISSPELLED];
	int nmisspelled;
	
	BitmapPS *Bitmap,*BitmapLarge; // Bitmap or null pointer
	String *FileName,*LargeFileName,*BitmapName,*BitmapPath; // file names for bitmaps
	String *Original; // original content in case of UTF translation
	int bitmapcount; // count of the bitmap in the file
	int bitmapline; // line is covered by a bitmap above it
	int newline; // comments should be broken here
	enum { output,prompt,udf,comment }; // enumeratin of types
	int heading; // for comment lines, starting with "* "

	// Initialiize the line with an empty string
	TextLine (Text *t, TextLine *prev=0, TextLine *next=0, char *s="")
		:	Prev(prev),Next(next),String(s,16),
			Type(output),Mark(0),Bitmap(0),newline(0),T(t),heading(0),BitmapMark(0),
			BitmapLarge(0),Latex(0),mathjax(0),SVG(0),
			FileName(0),LargeFileName(0),BitmapName(0),BitmapPath(0),bitmapcount(0),
			nmisspelled(0), Original(0), LastBackup(0)
	{	
		if (prev) prev->Next=this;
		if (next) next->Prev=this;
		for (int i=0; i<MAXBACKUPS; i++) Backup[i]=0;
	}

	// destructor
	~TextLine ()
	{	
		if (Prev) Prev->Next=Next;
		if (Next) Next->Prev=Prev;
		if (Bitmap && bitmapline==0) delete Bitmap;
		if (BitmapLarge && bitmapline==0) delete BitmapLarge;
		Bitmap=0;
		for (int i=0; i<MAXBACKUPS; i++)
			if (Backup[i]) delete Backup[i];
		if (Latex) delete Latex;
		if (SVG) delete SVG;
		if (FileName) delete FileName;
		if (LargeFileName) delete LargeFileName;
		if (BitmapName) delete BitmapName;
		if (BitmapPath) delete BitmapPath;
		if (Original) delete Original;
	}

	// for the list structure:
	TextLine *next () { return Next; }
	TextLine *nextcommand ();
	TextLine *prev () { return Prev; }
	
	// getters and setters
	int type () { return Type; }
	void type (int t) { Type=t; }
	int mark () { return Mark; }
	void mark (int f) { Mark=f; }
	int bitmapmark () { return BitmapMark; }
	void bitmapmark (int f) { BitmapMark=f; }
	
	// return the line in HTML coding
	char *htmltext (int start=0, int markdown=0);

	// return the indentation of the line.
	int indent ();

	// comments are saved with % in Euler files.
	// this function removes the %.
	void clearcomment ()
	{	if (*(text()+1)=='%') 
		{	copy(text()+1);
			newline=1;
		}
		if (*(text()+1)==' ') copy(text()+2);
		else copy(text()+1);
		if (*(text())=='*' && *(text()+1)==' '
			&& (!prev() || prev()->type()!=comment || strlen(prev()->text())==0 || newline))
		{	copy(text()+2); heading=1;
		}
	}
	
	// return a marker for the type of line 
	// (% for comments, blank for output)
	char *markertext ()
	{	
		if (Type==comment)
		{	
			if (newline) return "%%";
			else if (heading) return "% * ";
			else return "% ";
		}
		else if (Type==output) return " ";
		else if (Type==udf) return "$";
		else if (Type==prompt) return ">";
		else return "";
	}

	// return a nice pretext for the text line
	// (> for prompts, blanks else)
	char *formattedtext ()
	{	
		if (Type==comment) return "   ";
		else if (Type==output) return "     ";
		else if (Type==udf)
		{	
			return "  $  ";
			/*
			char *t=text();
			if (strcmp(t,"$endfunction")==0) return "$";
			else if (*(t+1)==' ' && *(t+2)==' ') return "$";
			else if (*(t+1)==' ') return "$ ";
			else return "$  ";
			*/
		}
		else if (Type==prompt) return "  >";
		else return "  ";
	}

	// return the text without > or $
	char *puretext ()
	{	if (Type==udf) return text()+1;
		else if (Type==prompt) return text()+1;
		else if (Original) return Original->text();
		else return text();
	}

	TextLine *nextprompt ()
	{	TextLine *l=this;
		while (l->Next && l->Next->Type!=prompt) l=l->Next;
		return l->Next;
	}

	int height ();
	void draw (PS &ps, int y, int Offset);

	void backup ();
	void restore ();
	void unrestore ();

	int hidden ();

	int isCommentBreak ()
	{
		return Type==comment && strncmp(text(),"---",3)==0;
	}

	int isSubHeading ()
	{
		return Type==comment && strncmp(text(),"** ",3)==0;
	}

	char *displaytext (int start=0, int useutf = 0);

	char *marktext ();
	
	char *latex ()
	{
		if (Latex) return Latex->text();
		else return 0;
	}
	int ismathjax () { return mathjax; }
	void latex (char *latex_, int mathjax_)
	{
		Latex=new String(latex_);
		mathjax=mathjax_;
	}

	char *svg ()
	{
		if (SVG) return SVG->text();
		else return 0;
	}
	void svg (char *name)
	{
		SVG=new String(name);
	}

	int isFunctionLine ()
	{
		return (startsWith(">function ") || startsWith(">%+ function ") || startsWith(">%% function"))
			 && 
		!(contains(":=") || contains("&=") || contains("&&="));
	}

	void addmisspelled (int p, int n)
	{
		if (nmisspelled>=MAXMISSPELLED) return;
		misspelled[nmisspelled]=(short)p;
		misspelledlength[nmisspelled++]=(short)n;
	}

	void clearmisspelled ()
	{
		nmisspelled=0;
	}

	void setoriginal (char *s)
	{
		Original=new String(s);
	}
	char *getoriginal ()
	{
		if (Original) return Original->text();
		else return 0;
	}
	void makeutf ();
};

extern Font *courier,*courierlarge,*courierbold;

/**
This class contains the text as a list of text lines. It is able
to draw the text onto the text window. Text can be inserted and
deleted from the text. Note, that text lines can also contain
images!
*/
class Text
{   
	TextLine *Start,*Top,*Cursor; // first line, top of page, and cursor line
	int Cursorpos,Offset,TextHeight,Height; 
		// cursor position, hor. offset, text size, and page size
	StandardWindow *W; // pointer to the text window containing the text
	int CX,CY,CW,CH;
	TextLine *Dragline; // line to drag from
	int Dragpos; // and position to drag from
	TextLine *Markstart,*Markend; // start and endline of marked text
	int Markstartpos,Markendpos; // and positions in the lines
	int BracketMark; // The mark comes from a bracket mark
	int Changed; // did the text change recently?
	int CaretUpdate; // flag, if the carret position needs updating
	BitmapPS *LastDisplayed; // pointer to the image covering the current line
	String UndoBuffer; // buffer for deleted lines
	BitmapPS *Buffer;
	int ShowUpdate;
	int ROffset;

	public :
	Text (StandardWindow &w)
		: W(&w),Offset(0),TextHeight(0),Dragline(0),
		Markstart(0),Markend(0),BracketMark(0),CaretUpdate(1),UndoBuffer(""),Buffer(0),
		ShowUpdate(1), ROffset(5)
	{	Cursor=Top=Start=new TextLine(this);
		Cursorpos=0; Changed=0;
	}
	~Text ();
	void delline (); // from Cursor to line end
	void delchar (); // one character
	void inserttext (char *s); // insert in current line
	void insertchar (char c); // insert character in current line
	void clearquestion (); // insert typed character in current line
	void insertline (); // insert a new line
	void insertlinebefore (TextLine *line, int type, char *text); // insert a new line
	void joinline (); // append the current line to the previous
	TextLine *top (TextLine *Cursor); // compute a good top line from the cursor line
	int getheight (); // height of text
	void setfont (PS &ps) { ps.font(*courier); } // set a font for thie text
	int textheight () // compute text height
	{	if (TextHeight==0) getheight();
		return TextHeight;
	}
	int height () { return Height; } // total height of text
	void sized();
		// window was resized
	void clear (); // clear all text
	void cursorleft (int lefteditlimit); // move caret left
	void cursorright (); // move caret right
	void cursorrighttyped (); // move caret right
	void setcaret (PS &ps); // set the caret to its correct position
	void updatecaret (); // update caret position
	void redraw (); // redraw everything and update caret
	void redraw (PS &ps); // redraw the presentation space
	void drawline (PS &ps, int y, TextLine *p); // draw a specific text line
	void drawbelow (PS &ps, TextLine *l); // draw anything below a line
	void markbracket (); // Mark brackets to help the user
	void marktill (char c, TextLine *l, int pos); // Mark until this closing bracket
	void markback (TextLine *l, int pos); // Mark back until comma, semicolon or opening bracket
	void markbacktill (char c, TextLine *l, int pos); // Mark until this closing bracket
	int bracketmarked () { return BracketMark; } // Marked from brackets
	void lineup (); // set cursor to previous line
	void linedown (); // set cursor to next line
	void updatetop (); // get a good top line
	void showcomment (); 
		// make sure the comment above the current line shows on page
	void removeoutput (); // remove all output
	void nextprompt (); // set cursor to next prompt
	void gotonextprompt (); // set the cursor to the next prompt
	void newprompt (); // insert a new prompt line
	int load (char *filename); // load text from file
	int save (char *filename); // save text to file
	void exporthtml (char *s); // export text to HTML
	void exportmarkdown(char* s); // export text to MD
	void exportlatex (char *s); // export text to Latex
	void settype (); // set the type of the cursor line based on content
	int type () { return Cursor->type(); } // get the type of the cursor line
	void comment (); // enter comment in a dialog and update text
	void commenttobuffer (char *buffer); // copy the current comment to the buffer
	void buffertocomment (char *buffer); // insert the comment from the buffer
	void updatecomment (); // update the current comment
	void insertcommentimage (char *filename, int reduce=0, char *largeimage=0, 
		char *latex=0, int mathjax=0); // try to insert a comment image
	char *makelatexpng (char *formula, int alias, int textheight, int transparent, int color); 
		// make a Latex formula (return 0 on failure)
	void insertlatex (char *formula, int mathjax); // try to insert a Latex formula
	void insertmaxima (char *formula, int mathjax); // try to insert a Maxima expression as Latex formula
	int endfunction (); // stop editing a function

	void pageup (int untilbreak=0); // update top line to previous page
	void pagedown (int untilbreak=0); // to next page
	void gototextend (); // to end of page
	void gototextstart (); // to start of page
	void scrollup (); // to previous line
	void scrolldown (); // to next line
	
	void showcursor (); // make sure the cursor line is visible and switch on cursor
	void showcursorline (); // make sure the cursor line is visible
	void computecursor (int x, int y, TextLine * &l, int &pos); 
		// compute cursor position after click

	int startdrag (int x, int y); // start dragging for mark
	void drag (int x, int y); // called from the drag thread
	void enddrag (int x, int y); // end dragging

	int rightclicked (int x, int y); // right click into text window

	void mark (TextLine *l, int pos); // mark till here
	void unmark (); // unmark everything
	void unmarkall (); // unmark everything and redraw
	void selectall (); // mark everything
	int cursorinmark (); // see, if the cursor is in the marked line
	void deletemark (); // delete the marked text (only in cursor line!)
	void markcommand (int x, int y); // mark a command after a doubleclick
	char *getlinemark (); // return a string, if text within a line is marked

	void execute (char *s); // set the cursorline to this text
	void execute (); // execute the current line
	void set (char *s, int pos); // set the cursorline to this text
	void setinput (char *s, int pos); // set the edit text to this text

	void deleteudf (); // delete a line containg a line of a function
	void deletecommand (); // delete a line and copy to buffer
	void insertcommand (); // insert a new prompt
	void undodelete (); // insert all lines from the buffer
	void undo (); // undo change in current line with backup
	void redo (); // undo change in current line with backup
	void backup (); // store backup of current line

	void copytext (); // copy marked text to clipboard
	char *getMarkedWord (char *s); // get the marked string, but only one line
	void copycommands (); // copy marked commands to clipboard
	void copyformatted (); // copy marked commands to clipboard
    void pastecommands (); // paste from clipboard
    void pasteascommands (); // paste clipboard as commands

	void deleteoutput (); // delete marked output

	void setscroll (); // set the scroll bars correctly according to the top line
	void setscroll (int pos); // set the top line according to this scroll position
	void sethscroll (); // set the horizontal scroll bars according to Offset
	void sethscroll (int pos); // set Offset according to this position

	int changed () { return Changed; } // text has changed recently
    void changed (int i); // set the changed flag

	int inudf () { return Cursor->type()==TextLine::udf; } 
		// currently editing a function?
	void setoffset (int c) { Offset=c; CaretUpdate=0; }
		// set the offset to this position
	int getoffset () { return Offset; }
		// get the offset
	char * currentline () { return Cursor->text(); }
		// get the text in the cursor line
	int cursorpos () { return Cursorpos; }
		// get the current cursor position
	int isinmaxima ();
	TextLine *cursorline () { return Cursor; }
		// get the cursor line itself
	TextLine *currenttop () { return Top; }
	void setcursorline (TextLine *l, int pos);
	void setcursorpos (int pos);
	StandardWindow * getWindow () { return W; }
		// get the associated text window
	int countlines ();
		// get the number of text lines
	void shift (int flag);
		// start or stop shift for cursor movements (shift marks text)
	void splitcommand (int adddots=1);
		// search for the text, mark it, and make it visible
	void search (char *s);
	void searchnext (char *s);
	void showupdate (int flag) { ShowUpdate=flag; if (flag) W->update(); }
	void clearudfinput ();
	int roffset () { return ROffset; }
	int isBeforeBreak (TextLine *l);
	int isAfterBreak (TextLine *l);
	int isbreak (TextLine *l);

	void tofirstnonblank ()
	{
		char *p=Cursor->text()+Cursorpos;
		while (*p==' ') { p++; Cursorpos++; }
	}

	void spellcheck();
	void unmarkspellerrors();
	void addwords(char *file);
};

extern Text *text;

extern int onewindow;
extern int textx,texty,textw,texth;

extern WindowClass eulert;
extern int statushelp,editing,waiting,saveimages,imagesindirectory,helpintitle;

class TextWindow : public StandardWindow
{   int Code,Scan,Dragscroll;
	Timer *T,*CT,*H,*UT;
	int NeedUpdate;
	public :
	enum { message_quit,message_caret,message_continue,message_size };
	TextWindow (char *title) :
		StandardWindow(title,eulert,defaultsize,defaultsize,
			onewindow?defaultstyle:defaultstyle|vscroll|hscroll),
		Dragscroll(0),NeedUpdate(0)
	{	text=new Text(*this);
		usecaret();
	}
	virtual void redraw (PS &ps);
	virtual void key (int flags, int code, int scan);
	int code () { int h=Code; Code=0; return h; }
	int scan () { int h=Scan; Scan=0; return h; }
	int scanescape ()
	{   int h=Scan;
		if (h!=escape) return 0;
		Scan=0; return h;
	}
	virtual void user (int message, Parameter p);
	virtual void sized ();
	virtual void timer (int n);
	virtual void starttimer (long msec)
	{	
		T=new Timer(*this,msec,1);
	}
	virtual void starthelptimer ()
	{	
		if (!statushelp) return;
		if (H) H->kill();
		H=new Timer(*this,500,3);
	}
	virtual void stophelptimer ()
	{	
		if (H) H->kill();
		H=0;
	}
	virtual void startupdatetimer ()
	{	
		if (UT) UT->kill();
		UT=new Timer(*this,100,5);
	}
	virtual void stopupdatetimer ()
	{	
		if (UT) UT->kill();
		UT=0;
	}
	virtual void startcomputingtimer ()
	{	
		if (CT) CT->kill();
		CT=new Timer(*this,500,4);
	}
	virtual void stopcomputingtimer ()
	{	
		if (CT) CT->kill();
		CT=0;
	}
	virtual void clicked (int x, int y, int type);
	virtual void scroll (int flags, int type, int pos);
	virtual int close ();
	void needupdate (int flag)
	{	
		if (!UT) startupdatetimer(); 
		NeedUpdate=1;
	}
    virtual void statechanged (WPARAM state);
};

extern TextWindow textwindow;

extern void doexit();
extern String wantload,FileName,TextFontName;
extern int tfontheight,linelength;
void setname (void);
int strcontains (char *s, char *str);
char *fromutf (char *);

