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
#include "spell.h"

Flag CaretOn(0); // Global flag for the blinking caret (text cursor)

Text *text=0;

int promptindent=5,outputindent=20;

extern int fixmaximalinel,textwidth;
extern int exportlatexasmathjax,disablelatex,displaymathjaxaslatex;

CriticalSection textcritical("Text");

extern HANDLE CurrentProcess;

int strcontains (char *s, char *str)
{
	char *p=strstr(s,str);
	return p!=0;
}

int isbom (char *s)
{
	if (*s==239-256 && *(s+1)==187-256 && *(s+2)==191-256) return 1;
	else return 0;
}

char *putbom (char *s)
{
	*s++=239-256; *s++=187-256; *s++=191-256;
	return s;
}

extern int htmlutf,utf;

typedef struct
{
	char *name;
	int value;
} entity;

entity ent[] = {
{"Alpha",913},
{"Beta",914},
{"Gamma",915},
{"Delta",916},
{"Epsilon",917},
{"Zeta",918},
{"Eta",919},
{"Theta",920},
{"Iota",921},
{"Kappa",922},
{"Lambda",923},
{"Mu",924},
{"Nu",925},
{"Xi",926},
{"Omicron",927},
{"Pi",928},
{"Rho",929},
{"Sigma",931},
{"Tau",932},
{"Upsilon",933},
{"Phi",934},
{"Chi",935},
{"Psi",936},
{"Omega",937},
{"alpha",945},
{"beta",946},
{"gamma",947},
{"delta",948},
{"epsilon",949},
{"zeta",950},
{"eta",951},
{"theta",952},
{"iota",953},
{"kappa",954},
{"lambda",955},
{"mu",956},
{"nu",957},
{"xi",958},
{"omicron",959},
{"pi",960},
{"rho",961},
{"sigma",963},
{"sigmaf",962},
{"tau",964},
{"upsilon",965},
{"phi",966},
{"chi",967},
{"psi",968},
{"omega",969},
{"thetasym",977},
{"upsih",978},
{"piv",982},
{"bull",8226},
{"hellip",8230},
{"prime",8242},
{"Prime",8243},
{"oline",8254},
{"frasl",8260},
{"weierp",8472},
{"image",8465},
{"real",8476},
{"trade",8482},
{"alefsym",8501},
{"larr",8592},
{"uarr",8593},
{"rarr",8594},
{"darr",8595},
{"harr",8596},
{"crarr",8629},
{"lArr",8656},
{"uArr",8657},
{"rArr",8658},
{"dArr",8659},
{"hArr",8660},
{"forall",8704},
{"part",8706},
{"exist",8707},
{"empty",8709},
{"nabla",8711},
{"isin",8712},
{"notin",8713},
{"ni",8715},
{"prod",8719},
{"sum",8721},
{"minus",8722},
{"lowast",8727},
{"radic",8730},
{"prop",8733},
{"infin",8734},
{"ang",8736},
{"and",8743},
{"or",8744},
{"cap",8745},
{"cup",8746},
{"int",8747},
{"there4",8756},
{"sim",8764},
{"cong",8773},
{"asymp",8776},
{"ne",8800},
{"equiv",8801},
{"le",8804},
{"ge",8805},
{"sub",8834},
{"sup",8835},
{"nsub",8836},
{"sube",8838},
{"supe",8839},
{"oplus",8853},
{"otimes",8855},
{"perp",8869},
{"sdot",8901},
{"lceil",8968},
{"rceil",8969},
{"lfloor",8970},
{"rfloor",8971},
{"lang",9001},
{"rang",9002},
{"loz",9674},
{"spades",9824},
{"clubs",9827},
{"hearts",9829},
{"diams",9830},
{"quot",34},
{"quot",34},
{"amp",38},
{"lt",60},
{"gt",62},
{"OElig",338},
{"oelig",339},
{"Scaron",352},
{"scaron",353},
{"Yuml",376},
{"circ",710},
{"tilde",732},
{"ensp",8194},
{"emsp",8195},
{"thinsp",8201},
{"zwnj",8204},
{"zwj",8205},
{"lrm",8206},
{"rlm",8207},
{"ndash",8211},
{"mdash",8212},
{"lsquo",8216},
{"rsquo",8217},
{"sbquo",8218},
{"ldquo",8220},
{"rdquo",8221},
{"bdquo",8222},
{"dagger",8224},
{"Dagger",8225},
{"permil",8240},
{"lsaquo",8249},
{"rsaquo",8250},
{"euro",8364},
{"iexcl",161},
{"iexcl",161},
{"cent",162},
{"pound",163},
{"curren",164},
{"yen",165},
{"brvbar",166},
{"sect",167},
{"uml",168},
{"copy",169},
{"ordf",170},
{"laquo",171},
{"not",172},
{"shy",173},
{"reg",174},
{"macr",175},
{"deg",176},
{"plusmn",177},
{"sup2",178},
{"sup3",179},
{"acute",180},
{"micro",181},
{"para",182},
{"middot",183},
{"cedil",184},
{"sup1",185},
{"ordm",186},
{"raquo",187},
{"frac14",188},
{"frac12",189},
{"frac34",190},
{"iquest",191},
{"Agrave",192},
{"Aacute",193},
{"Acirc",194},
{"Atilde",195},
{"Auml",196},
{"Aring",197},
{"AElig",198},
{"Ccedil",199},
{"Egrave",200},
{"Eacute",201},
{"Ecirc",202},
{"Euml",203},
{"Igrave",204},
{"Iacute",205},
{"Icirc",206},
{"Iuml",207},
{"ETH",208},
{"Ntilde",209},
{"Ograve",210},
{"Oacute",211},
{"Ocirc",212},
{"Otilde",213},
{"Ouml",214},
{"times",215},
{"Oslash",216},
{"Ugrave",217},
{"Uacute",218},
{"Ucirc",219},
{"Uuml",220},
{"Yacute",221},
{"THORN",222},
{"szlig",223},
{"agrave",224},
{"aacute",225},
{"acirc",226},
{"atilde",227},
{"auml",228},
{"aring",229},
{"aelig",230},
{"ccedil",231},
{"egrave",232},
{"eacute",233},
{"ecirc",234},
{"euml",235},
{"igrave",236},
{"iacute",237},
{"icirc",238},
{"iuml",239},
{"eth",240},
{"ntilde",241},
{"ograve",242},
{"oacute",243},
{"ocirc",244},
{"otilde",245},
{"ouml",246},
{"divide",247},
{"oslash",248},
{"ugrave",249},
{"uacute",250},
{"ucirc",251},
{"uuml",252},
{"yacute",253},
{"thorn",254},
{"yuml",255},
{"",32}
};

/**
Get a Unicode character code and translate to UTF-8 at p.
Return the new end position p.
*/
char *chartoutf (int charcode, char *p)
{
    if (charcode < 128)
    {
		*p++=(char)charcode;
		return p;
    }
    else
    {
		char c[16];
		int cn=0;
        int first_bits = 6; 
        const int other_bits = 6;
        int first_val = 0xC0;
        int t = 0;
        while (charcode >= (1 << first_bits))
        {
            {
                t = 128 | (charcode & ((1 << other_bits)-1));
                charcode >>= other_bits;
                first_val |= 1 << (first_bits);
                first_bits--;
            }
			c[cn++]=(char)t;
        }
        t = first_val | charcode;
        c[cn++]=(char)t;
		for (int i=cn-1; i>=0; i--) *p++=c[i];
    }
    return p;
}

/**
Go through the string at q and replace entitities with their
Unicode value. Result string starts at q. Return value is
0, if illegal entities were found, or the end of the string.
*/
unsigned char *exent (unsigned char *p, unsigned char *q, unsigned char *ramend)
{
	while (*q)
	{
		// print("%d ",(int)(*q));
		if (ramend && p+8>(unsigned char *)ramend) return 0;
		if (*q=='&')
		{
			unsigned char *h=q+1;
			while (*h && *h!=';') h++;
			if (*h==0) return 0;
			if (*(q+1)=='#')
			{
				int k=0;
				unsigned char *hq=q+2;
				while (hq<h)
				{
					if (*hq>='0' && *hq<='9')
					{
						k=k*10+(*hq-'0');
					}
					else
					{
						return 0;
					}
					hq++;
				}
				p=(unsigned char *)chartoutf(k,(char *)p);
				q=h+1;
			}
			else 
			{
				int n=0;
				int found=0;
				while (*(ent[n].name))
				{
					if (strncmp((char *)q+1,ent[n].name,h-(q+1))==0)
					{
						p=(unsigned char *)chartoutf(ent[n].value,(char *)p);
						q=h+1;
						found=1;
						break;
					}
					n++;
				}
				if (!found) return 0;
			}
		}
		else
		{
			*p++=*q++;
		}
	}
	// print("\n");
	*p++=0;
	/*
	unsigned char *c=start;
	while (*c)
	{
		print("%d ",(int)(*c));
		c++;
	}
	print("\n");
	*/
	return p;
}

int entcheck (unsigned char *q)
{
	while (*q)
	{
		if (*q=='&')
		{
			unsigned char *h=q+1;
			while (*h && *h!=';') h++;
			if (*h==0) return 0;
			if (*(q+1)=='#')
			{
				unsigned char *hq=q+2;
				while (hq<h)
				{
					if (!(*hq>='0' && *hq<='9'))
					{
						return 0;
					}
					hq++;
				}
				return 1;
			}
			else
			{
				int n=0;
				while (*(ent[n].name))
				{
					if (strncmp((char *)q+1,ent[n].name,h-(q+1))==0)
					{
						return 1;
					}
					n++;
				}
			}
			q=h+1;
		}
		else *q++;
	}
	return 0;
}



WCHAR uline [MAXLINE*2];
char uline1 [MAXLINE*2];

extern int CODE_PAGE;

/**
Convert a line to UTF.
Uses a global buffer.
*/
char * toutf (char *s)
{
	// DumpWarning("Codepage %d", "Euler", CODE_PAGE);
	MultiByteToWideChar(CODE_PAGE,0,s,-1,uline,MAXLINE*2);
	WideCharToMultiByte(CP_UTF8, 0, uline, -1, uline1, MAXLINE * 2, 0, 0);
	return uline1;
}

char * enttoutf (char *s, int bom=1)
{
	char h[MAXLINE*2];
	MultiByteToWideChar(CODE_PAGE,0,s,-1,uline,MAXLINE*2);
	WideCharToMultiByte(CP_UTF8,0,uline,-1,h,MAXLINE*2,0,0);
	if (bom)
	{
		uline1[0]=239-256; uline1[1]=187-256; uline1[2]=191-256;
		if (!exent((unsigned char *)uline1+3,(unsigned char *)h)) return 0;
	}
	else
	{
		if (!exent((unsigned char *)uline1,(unsigned char *)h)) return 0;
	}
	return uline1;
}

/**
Convert a linte from UTF.
Uses a global buffer.
*/
char * fromutf (char *s)
{
	MultiByteToWideChar(CP_UTF8,0,s,-1,uline,MAXLINE*2);
	WideCharToMultiByte(CODE_PAGE,0,uline,-1,uline1,MAXLINE*2,0,0);
	return uline1;
}

void fs (char *s, FILE *out)
{
	if (s[0]==239-256 && s[1]==187-256 && s[2]==191-256) fputs(s+3,out);
	else if (htmlutf) fputs(toutf(s),out);
	else fputs(s,out);
}

void fshtml (char *s, FILE *out)
{
	char line[MAXLINE];
	char *p=s,*q=line;
	while (*p!=0 && q-line<MAXLINE-20)
	{	
		switch (*p)
		{	case '>' : p++; *q++='&'; *q++='g'; *q++='t'; *q++=';'; break;
			case '<' : p++; *q++='&'; *q++='l'; *q++='t'; *q++=';'; break;
			case '&' : p++; *q++='&'; *q++='a'; *q++='m'; *q++='p'; *q++=';'; break;
			default : *q++=*p++;
		}
	}
	*q=0;
	if (htmlutf) fputs(toutf(line),out);
	else fputs(line,out);
}

void fsen (char *s, FILE *out)
{
	if (utf) fputs(toutf(s),out);
	else fputs(s,out);
}

/**
Convert a textline to HTML for HTML export.
Look for "See: " and add a link to the related function in the index.
Replace <, > and & with proper HTML tags.
*/
char* TextLine::htmltext(int start, int markdown)
{
	static char out[MAXLINE];
	char* p = text() + start, * q = out;
	if ((strstarts(p, "See: ") || strstarts(p, "see: ")) && !strcontains(p, ","))
	{
		p += strlen("See: ");
		String s(p);
		int n = s.find(" | ");
		if (n < 0)
		{
			s.trim();
			String h(s);
			if (!h.startsWith("http") && !h.startsWith("file") &&
				!h.contains("#") && !*(h.extension())) h.cat(".html");
			if (markdown) sprintf(out, "  [%s](%s)", s.text(), h.text());
			else sprintf(out, "  <a href=\"%s\">%s</a>", h.text(), s.text());
		}
		else
		{
			String h(s);
			h.substring(n + 3);
			h.trim();
			s.substring(0, n);
			s.trim();
			if (!s.startsWith("http") && !s.startsWith("file") &&
				!*s.extension()) s.extension(".html");
			if (markdown) sprintf(out, "  [%s](%s)", s.text(), h.text());
			else sprintf(out, "  <a href=\"%s\">%s</a>", s.text(), h.text());
		}
		return out;
	}
	if (strncmp(p, "http", 4) == 0)
	{
		sprintf(out, "  <a href=\"%s\">%s</a>", p, p);
		return out;
	}
	p = displaytext(start);
	while (*p != 0 && q - out < MAXLINE - 20)
	{
		if (markdown)
		{
			switch (*p)
			{
			case '*': p++; *q++ = '\\'; *q++ = '*'; break;
			case '_': p++; *q++ = '\\'; *q++ = '_'; break;
			case '>': p++; *q++ = '\\'; *q++ = '>'; break;
			// case '(': p++; *q++ = '\\'; *q++ = '('; break;
			// case ')': p++; *q++ = '\\'; *q++ = ')'; break;
			// case '[': p++; *q++ = '\\'; *q++ = '['; break;
			// case ']': p++; *q++ = '\\'; *q++ = ']'; break;
			// case '!': p++; *q++ = '\\'; *q++ = '!'; break;
			case '\\': p++; *q++ = '\\'; *q++ = '\\'; break;
			default:
				*q++ = *p++;
			}
		}
		else
		{
			switch (*p)
			{
			case '>': p++; *q++ = '&'; *q++ = 'g'; *q++ = 't'; *q++ = ';'; break;
			case '<': p++; *q++ = '&'; *q++ = 'l'; *q++ = 't'; *q++ = ';'; break;
			case '&': p++; *q++ = '&'; *q++ = 'a'; *q++ = 'm'; *q++ = 'p'; *q++ = ';'; break;
			default:
				*q++ = *p++;
			}
		}
	}
	*q=0;
	return out;
}

extern int foldallmultilines;

/**
Compute, if the text line must be hidden.
*/
int TextLine::hidden ()
{
	if ( Type==prompt && (strcmp(text(),">//")==0 || strcmp(text(),"> //")==0)
			&& ::text->cursorline()!=this && prev() && prev()->type()==TextLine::comment
		)
	return 1;
	TextLine *cl=::text->cursorline();
	if (foldallmultilines && Type==prompt && this!=cl)
	{
		TextLine *h=this;
		while (h->Prev && h->Prev->Type==prompt && h->Prev->endsWith("..."))
		{
			h=h->Prev;
			if (h==cl) return 0;
		}
		if (h==this) return 0;
		h=this;
		while (h->Next && h->Next->Type==prompt && h->endsWith("..."))
		{
			h=h->Next;
			if (h==cl) return 0;
		}
		return 1;
	}
	if (Type==udf && ::udf!=2 && this!=cl)
	{
		TextLine *h=this;
		while (h->Type==udf && h->Prev)
		{
			h=h->Prev;
			if (h==cl) return 0;
		}
		if (h==this) return 0;
		if (foldallmultilines || (h && h->Type==prompt && (h->startsWith(">%+ ") || h->startsWith(">%% "))))
			return 1;
		return 0;
	}
	if (Type==prompt && this!=cl && !startsWith(">%+ "))
	{
		TextLine *h=this;
		while (h->Prev && h->Prev->Type==prompt && h->Prev->endsWith("..."))
		{
			h=h->Prev;
			if (h==cl) return 0;
		}
		if (!(h->startsWith(">%% ") || h->startsWith(">%+ "))) return 0;
		h=this;
		while (h->Next && h->Next->Type==prompt && h->endsWith("..."))
		{
			h=h->Next;
			if (h==cl) return 0;
		}
		return 1;
	}
	return 
		( Type==comment && 
			(
				strstarts(text(),"image: ") 
				|| strstarts(text(),"latex: ")
				|| strstarts(text(),"&latex: ")
				|| strstarts(text(),"maxima: ")
				|| strstarts(text(),"mathjax: ")
				|| strstarts(text(),"&mathjax: ")
			) && next() && next()->Bitmap!=0
		)
		;
}

/**
Compute the height of a text line, including a subsequent interline spacing.
*/
int TextLine::height ()
{	
	if (Type==prompt && hidden() && !startsWith("%+ "))
	{
		if (endsWith("...")) return 0;
		else return rd(T->textheight()*0.5);
	}
	if (hidden()) return 0;
	if (Type==comment && heading) return rd(T->textheight()*2);
	if (Type==comment && startsWith("** ")) return rd(T->textheight()*1.5);
	if (Type==comment && startsWith("%% ")) return 0;
	if (Bitmap && next() && (next()->Bitmap==0 || next()->bitmapline==0))
	{
		int h=rd(Bitmap->height()*factor())-(bitmapline*T->textheight());
		if (next()->type()!=type()) h+=T->textheight()/2;
		else h+=T->textheight()/3;
		return h;
	}
	if (next() && next()->type()!=type()) return T->textheight()*3/2;
	return T->textheight();
}

/**
Find the next command from a text line.
*/
TextLine *TextLine::nextcommand ()
{	
	TextLine *l=this;
	while (l->next())
	{	if (l->next()->type()==TextLine::prompt || l->next()->type()==TextLine::udf)
			return l->next();
		l=l->next();
	}
	return 0;
}

/**
Backup a text line for Ctrl-Z.
*/
void TextLine::backup ()
{	
	if (Backup[MAXBACKUPS-1]) delete Backup[MAXBACKUPS-1];
	for (int i=MAXBACKUPS-2; i>=0; i--)
		Backup[i+1]=Backup[i];
	Backup[0]=new String(text());
	LastBackup=0;
}

/**
Restore a text line from the backup.
*/
void TextLine::restore ()
{
	int j=LastBackup;
	while (j<MAXBACKUPS && Backup[j]!=0)
	{	
		if (strcmp(Backup[j]->text(),text())!=0)
		{	
			copy(Backup[j]->text());
			LastBackup=j;
			return;
		}
		j++;		
	}
}

/**
Restore a text line from the backup.
*/
void TextLine::unrestore ()
{
	int j=LastBackup-1;
	while (j>=0)
	{	
		if (Backup[j]!=0 && strcmp(Backup[j]->text(),text())!=0)
		{	
			copy(Backup[j]->text());
			LastBackup=j;
			return;
		}
		j--;		
	}
}

extern int useutf;

/**
Compute the text to be shown to the user.
This can be different form the actual content of the text line.
*/
char *TextLine::displaytext (int start, int useutf)
{
	static String *help=new String();
	char* ret = text() + start;
	if (Type==comment && (startsWith("See: ",start) || startsWith("see: ",start)))
	{
		int n=find(" | ");
		if (n<0)
		{
			help->copy("  ");
			help->cat(text()+5+start);
			ret=help->text();
		}
		else
		{
			help->copy("  ");
			help->cat(text()+n+3+start);
			ret=help->text();
		}		
	}
	else if (Type==comment && startsWith("http",start))
	{
		help->copy("  ");
		help->cat(text()+start);
		ret=help->text();
	}
	else if (Type==comment && startsWith("latex: ",start))
	{
		help->copy("  ");
		help->cat(text()+7+start);
		ret=help->text();
	}
	else if (Type==comment && startsWith("mathjax: ",start))
	{
		help->copy("  ");
		help->cat(text()+9+start);
		ret=help->text();
	}
	else if (Type==comment && startsWith("** ",start))
	{
		ret=text()+3+start;
	}
	else if (Type==comment && startsWith("%% ",start))
	{
		ret="";
	}
	else if (Type==prompt && Prev && Prev->Type==prompt && Prev->endsWith("..."))
	{
		help->copy(" ");
		help->cat(text()+1+start);
		ret=help->text();
	}
	else if (Type==udf)
	{
		help->copy(" ");
		help->cat(text()+1+start);
		ret=help->text();
	}
	else ret=text()+start;
	if (useutf) return toutf(ret);
	else return ret;
}

/**
Compute the text to be displayed in marked lines.
This is different from the usual display, to ease the marking of parts of a
line, and to give the user a way to see the true content of the line.
*/
char *TextLine::marktext ()
{
	if (type()==prompt || type()==udf) return displaytext();
	else return puretext();
}

int TextLine::indent ()
{ 
	if (Type==comment || Type==output || Type==udf) return outputindent; 
	else return promptindent;
}

/**
Destroy all lines in the text.
*/
Text::~Text ()
{	
	textcritical.enter("delete");
	TextLine *l=Start,*n;
	while (l)
	{	n=l->next();
		delete l;
		l=n;
	}
	textcritical.leave();
}

/**
Compute the correct positions of the caret based
on the current cursor position in the text.
Place the caret at this position.
*/
void Text::setcaret (PS &ps)
{	
	textcritical.enter("setcaret");
	TextLine *p=Top;
	if (!p)
	{	
		textcritical.leave();
		return;
	}
	int w,h;
	int c=roffset();
	String Work;
	setfont(ps);
	CW=2; CH=textheight();
	while (c+CH<height())
	{   
		if (Cursor==p)
		{   Work.copy(p->text());
			Work.text()[Cursorpos]=0;
			ps.textextent(Work,w,h);
			CX=w-Offset; CY=c;
			W->setcaretsize(CW,CH);
			W->setcaret(textx+CX+Cursor->indent(),texty+CY);
			textcritical.leave();
			return;
		}
		c+=p->height();
		p=p->next();
		if (!p) break;
	}
	CX=-1000; CY=-1000;
	W->setcaretsize(CW,CH);
	W->setcaret(textx+CX,texty+CY);
	textcritical.leave();
}

/**
Marke the carret for an update and redraw the window.
*/
void Text::updatecaret ()
{	
	CaretUpdate=1;
	W->update();
}

/**
Draw a line of text to the presentation space.
The drawing starts vertically at position y.
*/
void Text::drawline (PS &ps, int y, TextLine *p)
{	
	if (p->hidden()) return;
	textcritical.enter("drawline");
	if (p->Bitmap!=0)
	{	
		ps.clip(0,0,ps.width(),ps.height());
		int xo=20;
		if (p->Bitmap->width()<textwidth)
		{
			xo=(textwidth-p->Bitmap->width())/2;
		}
		if (p->Bitmap!=LastDisplayed)
		{	
			if (enlarge)
			{
				if (p->bitmapmark())
				{
					ps.bar(xo-Offset-5,y-p->bitmapline*textheight()-5,
					(int)(p->Bitmap->width()*enlargefactor)+10,
					(int)(p->Bitmap->height()*enlargefactor+10),
					*textmarked);
				}
				if (p->BitmapLarge)
					p->BitmapLarge->stretch(ps,xo-Offset,y-p->bitmapline*textheight(),
						(int)(p->Bitmap->width()*enlargefactor),
						(int)(p->Bitmap->height()*enlargefactor));
				else
					p->Bitmap->stretch(ps,xo-Offset,y-p->bitmapline*textheight(),
						(int)(p->Bitmap->width()*enlargefactor),
						(int)(p->Bitmap->height()*enlargefactor));
			}
			else
			{
				if (p->bitmapmark())
				{
					ps.bar(xo-Offset-5,y-p->bitmapline*textheight()-5,
						p->Bitmap->width()+10,p->Bitmap->height()+10,*textmarked);
				}
				p->Bitmap->copy(ps,xo-Offset,y-p->bitmapline*textheight());
			}
		}
		LastDisplayed=p->Bitmap;
		textcritical.leave();
		return;
	}
	switch (p->type())
	{	
		case TextLine::prompt : ps.textcolor(*textred); break;
		case TextLine::comment : 
			if (p->startsWith("http") || (p->startsWith("See: ") || p->startsWith("see: ")))
				ps.textcolor(*textblue);
			else
				ps.textcolor((usecolors)?*textgreen:*textblack); 
			if (p->heading)
			{
				ps.font(*courierlarge);
				y+=p->height()/8;
			}
			else if (p->startsWith("** "))
			{
				ps.font(*courierbold);
				y+=p->height()/8;
			}
			break;
		case TextLine::udf : 
			ps.textcolor(*textblue); 
			break;
		default : ps.textcolor(*textblack);
	}
	if (!usecolors) ps.textcolor(*textblack);
	int addoffset=p->indent();
	if (p->type()==TextLine::comment && (p->heading || p->startsWith("** ")))
	{
		int w,h;
		ps.textextent(p->displaytext(0,useutf),w,h);
		addoffset+=textwidth-w;
	}
	if (p->mark())
	{   
		int c=0;
		ps.alignment(Alignment::left,Alignment::top,0);
		ps.move(-Offset+addoffset,y);
		if (p==Markstart)
		{   
			if (Markstartpos>0)
			{	
				ps.backcolor(*textwhite);
				ps.text(p->marktext(),Markstartpos);
			}
			if (p==Markend)
			{	
				c=Markstartpos;
				goto mend;
			}
			if (BracketMark) ps.backcolor(*textbracketmarked);
			else ps.backcolor(*textmarked);
			ps.text(p->marktext()+Markstartpos);
		}
		else if (p==Markend)
		{   
			mend :
			if (BracketMark) ps.backcolor(*textbracketmarked);
			else ps.backcolor(*textmarked);
			ps.text(p->marktext()+c,Markendpos-c);
			ps.backcolor(*textwhite);
			ps.text(p->marktext()+Markendpos);
		}
		else
		{	
			if (BracketMark) ps.backcolor(*textbracketmarked);
			else ps.backcolor(*textmarked);
			ps.text(p->marktext());
		}
		ps.backcolor(*textwhite);
	}
	else 
	{	
		if (p->type()==TextLine::comment && !strncmp(p->text(),"---",3))
		{
			Pen pen(Pen::solid,2,(usecolors)?*textgreen:*textblack);
			ps.line(-Offset+p->indent(),y+(p->height()/2),
				-Offset+p->indent()+textwidth,y+(p->height()/2),pen);
		}
		else ps.text(-Offset+addoffset,y,p->displaytext(0,useutf),
			Alignment::left,Alignment::top);
		if (p->nmisspelled>0)
		{
			char *line=p->displaytext();
			for (int i=0; i<p->nmisspelled; i++)
			{
				Pen pen(Pen::solid,1,(usecolors)?*textred:*textblack);
				int r,c0,c1;
				ps.textextent(line,p->misspelled[i],c0,r);
				ps.textextent(line,p->misspelled[i]+p->misspelledlength[i],c1,r);
				ps.line(-Offset+addoffset+c0,y+(r-2),
					-Offset+addoffset+c1,y+(r-2),pen);
			}
		}
	}
	if (p->heading || p->startsWith("** "))
	{
		Pen pen(Pen::solid,2,(usecolors)?*textgreen:*textblack);
		ps.line(-Offset+p->indent(),y+(p->height()*6/8),
			-Offset+p->indent()+textwidth,y+(p->height()*6/8),pen);
	}
	ps.font(*courier);
	textcritical.leave();
}

extern int second;

/**
Draw everything below the text line to the presentation space.
This was used to avoid drawing too much. Now EMT will draw everything,
which means that always Top=T.
*/
void Text::drawbelow (PS &ps, TextLine *T)
{	
	textcritical.enter("drawbelow");
	if (!Top)
	{	textcritical.leave(); return;
	}
	setfont(ps);
	ps.backmode(PS::opaque);
	ps.backcolor(*textwhite);
	ps.erase(*textwhite);
	int r=roffset();
	int update=0;
	TextLine *p=Top;
	LastDisplayed=0;
	while (1)
	{	
		if (!p || r+p->height()>ps.height()) 
			break;
		if (update || T==p)
		{   
			drawline(ps,r,p);
			update=1;
		}
		r+=p->height();
		p=p->next();
	}
	textcritical.leave();
}

/**
Mark from the marker start to the position in the text line.
This assumes there is already a start marker.
*/
void Text::marktill (char c, TextLine *l, int pos)
{
	Markstartpos=pos;
	Markstart=l;
	l->mark(1);
	char *p=l->text();
	int b1=0,b2=0,b3=0,q1=0;
	int res=0;
	while (true)
	{
		pos++;
		if (!p[pos]) break;
		if (!q1 && strstarts(p+pos," .."))
		{
			if (c!='\"' && l->next() && l->next()->type()==TextLine::prompt)
			{
				l=l->next(); pos=1; l->mark(1); p=l->text();
			}
			else break;
		}
		if (!q1 && p[pos]==c && b1==0 && b2==0 && b3==0) { res=1; break; }
		if (p[pos]=='\"') q1=!q1;
		else if (!q1)
		{
			if (p[pos]=='(') b1++;
			if (p[pos]=='[') b2++;
			if (p[pos]=='{') b3++;
			if (p[pos]==')') b1--;
			if (p[pos]==']') b2--;
			if (p[pos]=='}') b3--;
		}
	}
	Markend=l;
	Markendpos=pos+1;
	if (!p[pos]) Markendpos--;
	if (!res) unmarkall();
}

/**
Mark backwards till the position.
This assumes there is already an end marker.
*/
void Text::markbacktill (char c, TextLine *l, int pos)
{
	Markendpos=pos+1;
	Markend=l;
	l->mark(1);
	char *p=l->text();
	int b1=0,b2=0,b3=0,q1=0;
	int res=0;
	while (true)
	{
		pos--;
		if (pos<0)
		{
			if (c!='\"' && l->prev() && 
				(l->prev()->endsWith(" ..") || l->prev()->endsWith(" ...")))
			{
				l=l->prev(); pos=strlen(l->text()); p=l->text(); l->mark(1);
			}
			else break;
		}
		if (!q1 && p[pos]==c && b1==0 && b2==0 && b3==0) { res=1; break; }
		if (p[pos]=='\"') q1=!q1;
		else if (!q1)
		{
			if (p[pos]==')') b1++;
			if (p[pos]==']') b2++;
			if (p[pos]=='}') b3++;
			if (p[pos]=='(') b1--;
			if (p[pos]=='[') b2--;
			if (p[pos]=='{') b3--;
		}
	}
	Markstart=l;
	if (pos<0) pos=0;
	Markstartpos=pos;
	l->mark(1);
	if (!res) unmarkall();
}

void Text::markback (TextLine *l, int pos)
{
	Markendpos=pos;
	Markend=l;
	l->mark(1);
	char *p=l->text();
	int b1=0,b2=0,b3=0,q1=0;
	while (true)
	{
		pos--;
		if (pos<0) break;
		if (!q1 && 
			(p[pos]=='(' || p[pos]=='[' || p[pos]=='{' || p[pos]==',' || p[pos]==';')
			&& b1==0 && b2==0 && b3==0) { break; }
		if (p[pos]=='\"') q1=!q1;
		else if (!q1)
		{
			if (p[pos]==')') b1++;
			if (p[pos]==']') b2++;
			if (p[pos]=='}') b3++;
			if (p[pos]=='(') b1--;
			if (p[pos]=='[') b2--;
			if (p[pos]=='{') b3--;
		}
	}
	Markstart=l;
	if (pos<0) pos=0;
	Markstartpos=pos+1;
	l->mark(1);
	if (Markstartpos>=Markendpos) unmarkall();
}

/**
Mark the brackets at the cursor position.
*/
void Text::markbracket ()
{
	if (Markstart!=0 && !BracketMark) return;
	TextLine *l=Cursor;
	if (!l) return;
	if (!l->type()==TextLine::prompt) return;
	int i=Cursorpos;
	char *p=l->text();
	int count=0; for (int j=0; j<i; j++) if (p[j]=='\"') count++;
	if (p[i]=='(') marktill(')',l,i);
	else if (p[i]=='[') marktill(']',l,i);
	else if (p[i]=='{') marktill('}',l,i);
	else if (p[i]=='\"' && count%2==0) marktill('\"',l,i);
	else if (i>0 && p[i-1]==')') markbacktill('(',l,i-1);
	else if (i>0 && p[i-1]==']') markbacktill('[',l,i-1);
	else if (i>0 && p[i-1]=='}') markbacktill('{',l,i-1);
	else if (i>0 && p[i-1]=='\"' && count%2==0) markbacktill('\"',l,i-1);
	else if (i>0 && (p[i-1]==',' || p[i-1]==';')) markback(l,i-1);
	BracketMark=(l->mark());
}

/**
Insert a text at the cursor position.
This will always unmark.
*/
void Text::inserttext (char *s)
{   
	textcritical.enter("inserttext");
	CaretUpdate=1;
	unmark();
	if (!*s)
	{	textcritical.leave(); return;
	}
	WindowPS ps(*W);
	TextLine *h;
	int i,n;
	while (*s)
	{   
		switch (*s)
		{   
			case 10 :
				h=Top;
				insertline();
				if (*(s + 1) == 13) s++;
				break;
			case 13:
				h = Top;
				insertline();
				if (*(s + 1) == 10) s++;
				break;
			case 9 :
				n=4-(Cursorpos%4);
				for (i=0; i<n; i++) insertchar(' ');
				break;
			default :
				insertchar(*s);
		}
		h=Top;
		Top=top(Cursor);
		s++;
	}
	textcritical.leave();
	W->update();
}

/**
Compute the top of the page, such that the cursor line is visible.
*/
TextLine * Text::top (TextLine *Cursor)
{	
	textcritical.enter("top");
	CaretUpdate=1;

	if (textheight()<=0) // happens at program start
	{	
		textcritical.leave();
		return Start;
	}

	TextLine *p=Cursor;
	int total=height();

	int havecursor=0;

	if (p->type()==TextLine::prompt && p->next()) 
		// make sure the bitmap and the udf after the plot is visible
	{	
		int lines=0;
		while (p->endsWith("...") && p->next())
		{
			p=p->next();
		}
		if (p->next() && p->next()->Bitmap)
		{	
			while (p->next() && p->next()->Bitmap)
			{	
				p=p->next(); lines++;
			}
		}
		else if (p->next() && p->next()->type()==TextLine::udf)
		{	
			while (p->next() && p->next()->type()==TextLine::udf)
			{	
				p=p->next(); lines++;
			}
		}
		else
		{	
			total=total*7/8;
		}
	}

	int c=p->height()*2;
	int cc=0;

	while (p) // go backwards
	{	
		if (!havecursor && p==Cursor)
		{
			cc=p->height()*2;
			havecursor=1; // found the cursor
			while (p->prev() && p->prev()->type()==TextLine::comment)
			{
				p=p->prev();
				c+=p->height();
				cc+=p->height();
				if (p==Top)
				{
					textcritical.leave();
					return Top;
				}
				if (cc>total)
				{
					textcritical.leave();
					return p->next();
				}
			}
			if (p->prev())
			{
				p=p->prev();
				c+=p->height();
			}
		}
		else
		{
			if (!p->prev()) // we are at the start of the file
			{	
				textcritical.leave();
				return Start;
			}
			p=p->prev();
			c+=p->height();
		}
		if (c>total && havecursor)
		{	
			textcritical.leave();
			return p->next();
		}
		if (p==Top && havecursor)
		{	
			textcritical.leave();
			return Top;
		}
	}

	textcritical.leave();
	return Start;
}

int Text::getheight ()
{	
	WindowPS ps(*W);
	setfont(ps);
	TextHeight=ps.textheight();
	Height=texth;
	return TextHeight;
}

void Text::insertchar (char c)
{	
	textcritical.enter("insertchar");
	if (c==9)
	{
		int k=4-(Cursorpos%4);
		for (int i=0; i<k; i++) 
		{
			Cursor->insert(Cursorpos,' ');
			Cursorpos++;
		}
	}
	else
	{
		Cursor->insert(Cursorpos,c);
		Cursorpos++;
	}
	textcritical.leave();
	changed(1);
}

void Text::clearquestion ()
{	
	textcritical.enter("clear question");
	char *p=Cursor->text()+Cursorpos;
	if (*p=='?' && *(p+1)!='?' && *(p+1)!=' ')
	{	delchar();
		while (xisalpha(*p) || xisdigit(*p) || *p=='_') delchar();
	}
	textcritical.leave();
}

void Text::insertlinebefore (TextLine *cl, int t, char *s)
{   
	textcritical.enter("insertlinebefore");
	int first=(cl->prev()==0);
	TextLine *l=new TextLine(this,cl->prev(),cl);
	if (first) Start=l;
	l->type(t);
	l->cat(s);
	textcritical.leave();
	changed(1);
}

/**
Insert a new line right after the current cursor line.
*/
void Text::insertline ()
{   
	textcritical.enter("insertline");
	CaretUpdate=1;
	Cursor=new TextLine(this,Cursor,Cursor->next());
	Cursorpos=0;
	textcritical.leave();
	Top=top(Cursor);
	changed(1);
}

/**
Join the line with the previous line.
*/
void Text::joinline ()
{
	TextLine *l=Cursor;
	if (!l->prev() || l->prev()->type()!=l->type()) return;
	textcritical.enter("joinline");
	TextLine *p=l->prev();
	Cursor=p;
	if (p->endsWith(" ...")) *(p->text()+p->length()-4)=0;
	p->cat(" ");
	Cursorpos=p->length();
	p->cat(l->text()+1);
	delete l;
	textcritical.leave();
	W->update();
	changed(1);
}

/**
Delete the content of the current line.
*/
void Text::delline ()
{   
	textcritical.enter("delline");
	CaretUpdate=1;
	Cursor->text()[Cursorpos]=0;
	textcritical.leave();
	W->update();
	changed(1);
}

/*
Delete a character at the cursor position
*/
void Text::delchar ()
{   
	textcritical.enter("delchar");
	CaretUpdate=1;
	Cursor->del(Cursorpos,1);
	textcritical.leave();
	W->update();
	changed(1);
}

/*
Move cursor left up to lefteditlimit.
*/
void Text::cursorleft (int lefteditlimit)
{   
	textcritical.enter("cursorleft");
    if (Cursorpos>lefteditlimit) Cursorpos--;
	updatecaret();
	if (Dragline)
	{	mark(Cursor,Cursorpos);
	}
	textcritical.leave();
}

/*
Move cursor right up to end o fline (char 0).
*/
void Text::cursorright ()
{   
	textcritical.enter("cursorright");
    if (Cursor->text()[Cursorpos]) Cursorpos++;
	updatecaret();
	if (Dragline)
	{	mark(Cursor,Cursorpos);
	}
	textcritical.leave();
}

/*
Move cursor right and take care of ?.
*/
void Text::cursorrighttyped ()
{   
	textcritical.enter("cursorright typed");
	// search for ? in the current line
	char *pc=Cursor->text()+Cursorpos;
	char *p=Cursor->text();
	int found=0,inbrackets=0;
	while (*p)
	{	
		if (*p=='\"') inbrackets=!inbrackets;
		if (p>pc && *p=='?' && *(p+1)!='?' && *(p+1)!=' ' &&
			!(inbrackets && *(p-1)!='\"'))
		{	
			found=1; break;
		}
		p++;
	}
	if (found) Cursorpos=p-Cursor->text();
    else if (Cursor->text()[Cursorpos]) Cursorpos++;
	updatecaret();
	if (Dragline)
	{	mark(Cursor,Cursorpos);
	}
	textcritical.leave();
}

/*
Update the top line on the screen depending on the cursor position.
*/
void Text::updatetop ()
{	
	textcritical.enter("updatetop");
	TextLine *l=top(Cursor);
	if (l!=Top)
	{	Top=l;
		W->update();
	}
	textcritical.leave();
}

/*
Update the top line on the screen to show the comment of the top line command.
*/
void Text::showcomment ()
{	
	textcritical.enter("showcomment");
	TextLine *l=Cursor;
	while (l->prev() && l->prev()->type()==TextLine::comment)
	{	
		if (Top==l) Top=Top->prev();
		l=l->prev();
	}
	W->update();
	textcritical.leave();
}

/*
Go one cursor line up.
*/
void Text::lineup ()
{	
	textcritical.enter("line up");
	unmarkall();
	CaretUpdate=1;
	if (udf==2 && Cursor->type()==TextLine::udf && Cursor->prev())
	{
		Cursor=Cursor->prev();
		Cursorpos=1;
		tofirstnonblank();
		if (Cursor->type()!=TextLine::udf) { udf=0; setstatus(""); }
		updatetop();
		updatecaret();
		textcritical.leave();
		return;
	}
	else if (!editing || Cursor->type()!=TextLine::prompt)
	{	textcritical.leave(); return;
	}
	TextLine *l=Cursor;
	while (1)
	{	l=l->prev();
		if (!l)
		{	textcritical.leave(); return;
		}
		if (l->type()==TextLine::prompt) break;
	}
	Cursor=l;
	Cursorpos=1;
	updatetop();
	updatecaret();
	if (Dragline)
	{	mark(Cursor,Cursorpos);
	}
	textcritical.leave();
}

/*
Goto the next cursor prompt (>...).
*/
void Text::gotonextprompt ()
{
	textcritical.enter("line down");
	TextLine *l=Cursor;
	while (1)
	{	l=l->next();
		if (!l)
		{	textcritical.leave(); return;
		}
		if (l->type()==TextLine::prompt) break;
	}
	Cursor=l;
	Cursorpos=1;
	updatetop();
	updatecaret();
	textcritical.leave();
}

/*
Go one line down.
*/
void Text::linedown ()
{	
	textcritical.enter("line down");
	unmarkall();
	CaretUpdate=1;
	if (udf==2 && Cursor->type()==TextLine::udf && Cursor->next())
	{
		if (Cursor->next()->type()!=TextLine::udf)
		{
			textcritical.leave(); return;
		}
		Cursor=Cursor->next();
		Cursorpos=1;
		tofirstnonblank();
		if (Cursor->type()!=TextLine::udf && Cursor->type()!=TextLine::prompt) goto nextprompt;
		updatetop();
		updatecaret();
		textcritical.leave();
		return;
	}
	else if (!editing || Cursor->type()!=TextLine::prompt)
	{	textcritical.leave();
		return;
	}
	nextprompt:
	TextLine *l=Cursor;
	while (1)
	{	l=l->next();
		if (!l)
		{	textcritical.leave(); return;
		}
		if (l->type()==TextLine::prompt) break;
	}
	Cursor=l;
	Cursorpos=1;
	updatetop();
	updatecaret();
	if (Dragline)
	{	mark(Cursor,Cursorpos);
	}
	textcritical.leave();
}

/*
Remove the output of the current command line.
*/
void Text::removeoutput ()
{  	
	textcritical.enter("remove output");
	CaretUpdate=1;
	TextLine *p=Cursor,*h;
	if (p->type()!=TextLine::prompt)
	{	
		textcritical.leave(); return;
	}
	p=p->next();
	while (p)
	{	
		if (p->type()==TextLine::prompt) break;
		else if (p->type()==TextLine::comment) p=p->next();
		else if (p->type()==TextLine::udf && p->type()==TextLine::udf) p=p->next();
		else
		{	
			h=p->next();
			delete p;
			changed(1);
			p=h;
		}
	}
	textcritical.leave();
}

/*
Put cursor to the next command prompt or UDF function line.
*/
void Text::nextprompt ()
{   
	textcritical.enter("next prompt");
	CaretUpdate=1;
	if (Cursor->type()!=TextLine::prompt &&
			Cursor->type()!=TextLine::udf)
		goto stop;
	TextLine *p=Cursor;
	int c=Cursor->type();
	p=p->next();
	while (p)
	{	if (p->type()==c) break;
		if (p->type()==TextLine::prompt ||
			p->type()==TextLine::udf) goto stop;
		p=p->next();
	}
	if (!p) goto stop;
	if (Start==Cursor) Start=Cursor->next();
	delete Cursor;
	Cursor=p; Cursorpos=1;
	updatetop();
	stop :
	textcritical.leave();
	W->update();
}

/*
Window has been sized.
*/
void Text::sized()
{ 
	getheight(); 
	Top = top(Cursor); 
	if (Buffer) delete Buffer;  
	Buffer = 0; 
	W->update(); 
}

/*
Remove all lines.
*/
void Text::clear ()
{	
	textcritical.enter("clear");
	CaretUpdate=1;
	TextLine *l=Start,*h;
	while (l)
	{	h=l->next();
		delete l;
		l=h;
	}
	Cursor=Start=Top=new TextLine(this); 
	Cursorpos=0;
	unmarkall();
	textcritical.leave();
	W->update();
	changed(0);
}

/*
New command prompt after cursor position.
*/
void Text::newprompt ()
{	
	textcritical.enter("new prompt");
	CaretUpdate=1;
	Cursor->copy(">"); Cursorpos=1; Cursor->type(TextLine::prompt);
	changed(1);
	textcritical.leave();
	W->update();
}

/*
Split commands in two lines.
*/
void Text::splitcommand (int adddots)
{	
	textcritical.enter("split command");
	if (Cursor->type()==TextLine::prompt && Cursor->next() && Cursor->next()->type()==TextLine::udf)
	{
		Cursor=Cursor->next();
		Cursorpos=1;
		tofirstnonblank();
		updatecaret();
		showcursor();
		W->update();
		udf=2;
		textcritical.leave();
		return;
	}
	else if (Cursor->type()==TextLine::prompt && Cursor->isFunctionLine())
	{
		if (!Cursor->endsWith(" ...")) Cursor->cat(" ...");
		insertline();
		Cursor->type(TextLine::udf);
		Cursor->copy("$");
		Cursorpos=1;
		updatecaret();
		showcursor();
		W->update();
		udf=2;
		textcritical.leave();
		W->update();
		return;
	}
	else if (Cursor->type()==TextLine::prompt)
	{
		String old(Cursor->text());
		int pos=Cursorpos;
		if (Cursor->endsWith(" ...") && Cursorpos==Cursor->length()) pos-=4;
		*(Cursor->text()+pos)=0;
		if (adddots) Cursor->cat(" ...");
		insertline();
		newprompt();
		if (old.startsWith(">::")) Cursor->copy(">:: ");
		else if (old.startsWith(">:")) Cursor->copy(">: ");
		else if (old.startsWith(">>> ")) Cursor->copy(">");
		else if (old.startsWith(">> ")) Cursor->copy(">> ");
		else Cursor->copy(">");
		Cursorpos=strlen(Cursor->text());
		Cursor->cat(old.text()+pos);
		W->update();
	}
	else if (udf==2 && Cursor->type()==TextLine::udf)
	{
		String old(Cursor->text());
		int pos=Cursorpos;
		char *hc=Cursor->text();
		*(hc+pos)=0;
		while (strlen(hc)>0 && *(hc+strlen(hc)-1)==' ')
		{	*(Cursor->text()+strlen(hc)-1)=0;
		}
		insertline();
		Cursor->type(TextLine::udf);
		Cursor->copy("$");
		Cursorpos=1;
		char *p=old.text()+1;
		while (*p==' ')
		{
			p++; Cursorpos++; Cursor->cat(" ");
		}
		Cursor->cat(old.text()+pos);
		unmarkall();
		W->update();
	}
	changed(1);
	textcritical.leave();
}

extern int maximamode;
extern int yacasmode;
extern int largeimages;
extern int spellcheck;

int utfload;

/*
Load the text from the Euler file.
*/
int Text::load (char *filename)
{   
	textcritical.enter("load");
	FILE *in;
	in=fopen(filename,"r");
	if (!in)
	{	
		textcritical.leave();
		char warning[1024];
		sprintf(warning,"Could not open file\n%s",filename);
		Warning(warning,"Euler",*W);
		return 0;
	}

	clear();
	textwindow.stophelptimer();
	char line[MAXLINE];
	TextLine *l=0;
	int firstline=1;
	int newline=1;
	while (!feof(in))
	{	
		if (!fgets(line,MAXLINE-2,in)) break;

		if (firstline) 
		{
			utfload=((256+line[0])==239 && (256+line[1])==187 && (256+line[2])==191);
			// if (utfload) Warning("UTF","Euler",*W);
			if (utfload) strcpy(line,line+3);
		}

		// if (utfload) dump(line);
		// if (utfload) dump(fromutf(line));
		if (utfload) strcpy(line,fromutf(line));

		if (firstline && strncmp(line,"Maxima mode is on (compatibility mode)",
			strlen("Maxima mode is on (compatibility mode)"))==0) maximamode=1;
		else if (firstline && strncmp(line,"Maxima mode is on (direct mode)",
			strlen("Maxima mode is on (direct mode)"))==0)
			maximamode=2;
		
		firstline=0;
		while (line[0] && line[strlen(line)-1]=='\n') line[strlen(line)-1]=0;
		if (newline)
		{
			if (l) l=new TextLine(this,l,0);
			else l=Start=new TextLine(this,0,0);
		}
		newline=1;
		int commentimage=strstarts(line,"% %image% ");
		if (strstarts(line," %image% ") || commentimage)
		{	
			char *q=line+strlen(" %image% ");
			if (commentimage) q++;
			int tilde=(*q=='~');
			if (tilde) q++;
			String h1(q);
			String h2("images\\"); h2.cat(q);
			String h;
			if (imagesindirectory)
			{	h.copy(h2);
			}
			else
			{	h.copy(h1);
				h1.copy(h2);
			}
			BitmapPS *m=0;
			if (exists(h.text()))
			{	m=new BitmapPS(textwindow,h.text());
			}
			else
			{	h.copy(h1);
				m=new BitmapPS(textwindow,h.text());
			}
			if (m->bitmaphandle()!=0)
			{	
				int lines=rd(m->height()*factor()/text->textheight());
				if (lines>60)
				{	
					m->scale(
						rd(60*text->textheight()/m->width()*m->height()/factor()),
						rd(60*text->textheight()/factor()));
					lines=60;
				}
				if (tilde) { l->copy("~"); l->cat(h.text()); }
				else l->copy("");
				l->Bitmap=m;
				l->bitmapline=0;
				if (commentimage) l->type(TextLine::comment);
				if (h.endsWith(".png"))
				{
					char *p=h.text();
					*(p+strlen(p)-4)=0;
					h.cat("-large.png");
					if (exists(h.text()))
					{
						BitmapPS *ml=new BitmapPS(textwindow,h.text());
						if (ml) l->BitmapLarge=ml;
						else l->BitmapLarge=0;
					}
				}
				for (int i=1; i<lines; i++)
				{	
					l=new TextLine(this,l,0);
					l->copy("");
					l->Bitmap=m;
					l->bitmapline=i;
					if (commentimage) l->type(TextLine::comment);
				}
			}
			else
			{
				TextLine *lp=l->prev();
				delete l;
				l=lp;
			}
		}
		else if (strstarts(line,"% // latex: "))
		{
			TextLine *h=l;
			while (h->prev() && h->prev()->Bitmap) h=h->prev();
			if (h->Bitmap) h->latex(line+strlen("% // latex: "),0);
			newline=0;
		}
		else if (strstarts(line,"% // mathjax: ") && l->prev())
		{
			TextLine *h=l;
			while (h->prev() && h->prev()->Bitmap) h=h->prev();
			if (h->Bitmap) h->latex(line+strlen("% // mathjax: "),1);
			newline=0;
		}
		else if (!strstarts(line,"% // "))
		{	
			l->copy(line);
			switch (*l->text())
			{	case '>' : l->type(TextLine::prompt); break;
				case '$' : l->type(TextLine::udf); break;
				case '%' : 
					l->type(TextLine::comment);
					l->clearcomment();					
					l->makeutf();
					break;
			}
		}
		else newline=0;
	}
	fclose(in);
	if (!Start) newprompt();
	l=Start;
	while (l->type()!=TextLine::prompt)
	{	
		l=l->next();
		if (!l) break;
	}
	if (!l)
	{	
		l=Start;
		while (l->next()) l=l->next();
		l=new TextLine(this,l,0);
		l->type(TextLine::prompt);
		l->copy(">");
	}
	Cursor=l; Cursorpos=1;
	updatetop();
	textcritical.leave();
	if (::spellcheck) spellcheck();
	W->update();
	changed(0);
	return 1;
}

/*
Transfer the currunt line to UTF
*/
void TextLine::makeutf ()
{
	if (!entcheck((unsigned char *)text())) return;
	char *hline=enttoutf(text());
	if (hline)
	{
		setoriginal(text());
		copy(hline);
	}
}

void removeimages (char *filename);

/*
Save current notebook.
*/
int Text::save (char *filename)
{   
	textcritical.enter("save");
	
	FILE *out=fopen(filename,"w");
	if (!out)
	{	textcritical.leave();
		Warning("Could not open this file!","Euler",*W);
		return 0;
	}

	if (utf) { fputc(239,out); fputc(187,out); fputc(191,out); }

	TextLine *l=Start;
	int bitmapcount=1;
	String fname(filename);
	String dirname;
	fname.extension("");
	String name(fname.filename());

	String imgname(fname);
	if (saveimages && imagesindirectory)
	{	
		imgname.stripfilename();
		imgname.cat("images\\");
		dirname.copy(imgname);
		imgname.cat(name.text());
	}

	while (l)
	{	
		if (!l->Bitmap)
		{	
			if (l->type()==TextLine::output &&
				(*l->text()=='>' || *l->text()=='$' || *l->text()=='%'))
			{	
				fputc(' ',out);
			}
			else if (l->type()==TextLine::comment)
			{	
				if (l->newline) fsen("%% ",out);
				else fsen("% ",out);
				if (l->heading) fsen("* ",out);
			}
			if (l->getoriginal()) fsen(l->getoriginal(),out);
			else fsen(l->text(),out);
			fputc('\n',out);
		}
		else if (l->bitmapline==0 && l->Bitmap!=0 && saveimages)
		{	
			if (!exists(dirname.text())) 
			{
				_mkdir(dirname.text());
			}
			char s[MAXLINE];
			if (strlen(l->text())>0) 
				sprintf(s,"%s-%03d-%s.png",imgname.text(),bitmapcount,l->text());
			else
				sprintf(s,"%s-%03d.png",imgname.text(),bitmapcount);
			if (l->FileName) l->FileName->copy(s);
			else l->FileName=new String(s);
			l->Bitmap->savepng(s);
			if (strlen(l->text())>0) 
				sprintf(s,"%s-%03d-%s.png",name.text(),bitmapcount,l->text());
			else
				sprintf(s,"%s-%03d.png",name.text(),bitmapcount);
			if (l->type()==TextLine::comment) fsen("%",out);
			fsen(" %image% ",out);
			fsen(s,out);
			fputc('\n',out);
			if (strlen(l->text())>0) 
				sprintf(s,"%s-%03d-%s-large.png",imgname.text(),bitmapcount,l->text());
			else
				sprintf(s,"%s-%03d-large.png",imgname.text(),bitmapcount);
			if (l->BitmapLarge) 
			{
				l->BitmapLarge->savepng(s);
				if (l->LargeFileName) l->LargeFileName->copy(s);
				else l->LargeFileName=new String(s);
			}
			else remove(s);
			l->bitmapcount=bitmapcount;
			bitmapcount++;
			if (l->latex())
			{
				if (l->ismathjax()) fsen("% // mathjax: ",out);
				else fsen("% // latex: ",out);
				char h[MAXLINE];
				strcpy(h,l->latex());
				char *ph=h;
				while (*ph)
				{
					if (*ph=='\n') *ph=' ';
					ph++;
				}
				fsen(h,out);
				fputc('\n',out);
			}
		}
		l=l->next();
	}
	fclose(out);
	textcritical.leave();
	changed(0);
	return 1;
}

extern int fixlinelength,uselargeimagesforhtml,generatexhtml,noreformat;

/*
Translate blanks to %20 for WWW.
*/
void translateblanks (char *s, char *sh)
{
	char *p=s,*ph=sh;
	while (*p && ph<sh+MAXLINE-10)
	{	
		if (*p==' ') { *ph++='%'; *ph++='2'; *ph++='0'; }
		else *ph++=*p;
		p++;
	}
	*ph++=0;
}

/*
Export notebook to HTML.
*/
void Text::exporthtml (char *filename)
{	
	textcritical.enter("export HTML");

	// Ermittle, ob mathjax: ... vorkommt
	int mathjax=0;
	TextLine *lh=Start;
	while (lh->next())
	{
		if (lh->ismathjax() 
			|| (exportlatexasmathjax && (lh->latex() || strstarts(lh->text(),"latex: ")))
			|| strstarts(lh->text(),"mathjax: "))
		{
			mathjax=1; break;
		}
		lh=lh->next();
	}

	FILE *out=fopen(filename,"w");
	if (!out)
	{	textcritical.leave();
		Warning("Could not open this file!","Euler",*W);
		return;
	}
	String fname(filename);
	fname.extension("");
	String name(fname.filename());
	String title(fname.filename());

	String path(fname);
	path.stripfilename();
	path.cat("\\euler.css");
	int externalcss=exists(path.text());

	path.copy(fname);
	path.stripfilename();
	path.cat("\\pattern.html");
	FILE *pin;
	pin=fopen(path.text(),"r");

	if (!pin && htmlutf) { fputc(239,out); fputc(187,out); fputc(191,out); }

	String imgname(fname);
	String imgpath("");
	if (saveimages && imagesindirectory)
	{	imgname.stripfilename();
		imgname.cat("images\\");
		if (!exists(imgname.text())) _mkdir(imgname.text());
		imgname.cat(name.text());
		imgpath.copy("images/");
	}

	TextLine *l=Start;
	if (l->type()==TextLine::comment && l->heading)
	{	
		title.copy(l->text());
		l=l->next(); l=l->next();
	}

	char pline[MAXLINE];
	if (pin)
	{	
		while (pin)
		{	
			if (!fgets(pline,MAXLINE-2,pin)) break;
			if (*pline=='#') break;
			if (strstarts(pline,"<body") && mathjax)
			{
				fs("<script type=\"text/javascript\"\n",out);
				fs("src=\"http://cdn.mathjax.org/mathjax/latest/MathJax.js?config=TeX-AMS-MML_HTMLorMML\">\n",out);
				fs("</script>\n",out);
			}
			fputs(pline,out);
		}
	}
	else
	{
		if (generatexhtml)
		{
			if (htmlutf) fs("<?xml version='1.0' encoding='utf-8'?>\n",out);
			else fs("<?xml version='1.0'?>\n",out);
			fs("<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\">\n",out);
		}
		else
		{
			fs("<!DOCTYPE html>\n",out);
			fs("<html>\n",out);
		}
		fs("\n",out);
		fs("<head>\n",out);
		fs("<title>",out); fs(title,out); fs("</title>\n",out);
		if (externalcss)
		{	
			String path(fname);
			path.stripfilename();
			path.cat("\\euler.css");
			if (!exists(path.text()))
			{	Warning("euler.css could not be found!","Euler",*W);
			}
			fs("<LINK REL=\"stylesheet\" TYPE=\"text/css\" HREF=\"euler.css\">\n",out);
		}
		else
		{	
			int wbody=650;
			if (!fixlinelength)
			{
				wbody=textwindow.width()+100;
			}
			fs("<style type=\"text/css\">\n",out);
			char st[1024];
			sprintf(st,"body { width : %dpx; font-family: arial, sans-serif; font-size: 10pt; margin: 20px; margin-left: auto; margin-right: auto; }\n",wbody+50);
			fs(st,out);
			fs("pre { font-family : \"Courier New\", monospace; background: #EEE; padding: 10px; margin-left: 15%; }\n",out);
			fs("pre.output { color: #000; background:none; margin-left: 5%; }\n",out);
			if (usecolors)
			{
				fs("pre.udf { color : #006; margin-left: 0; }\n",out);
				fs("pre.prompt { color : #700; margin-left: 0; }\n",out);
				fs("pre.comment { color : #040; background: none; margin-left: 7%; }\n",out);
				fs("p.comment { color : #040; margin-left: 5%; margin-right: 5%; }\n",out);
				fs("p.indentedcomment { color : #040; margin-left: 10%; }\n",out);
				fs("h1.title { font-family : arial, sans-serif; text-align : right; font-size : 24pt; padding-bottom: 5pt; }\n",out);
				fs("h1.comment, h1.comment a { font-family : arial, sans-serif; text-align : right; font-size : 16pt; color : #000; }\n",out);
				fs("h1.comment, h2.comment a { font-family : arial, sans-serif; text-align : right; font-size : 12pt; color : #000; }\n",out);
				fs("h1.title, h1.comment, h2.comment { border-bottom: 2px solid #e9b06e; }\n",out);
				fs("ul.comment { font-family : arial, sans-serif; font-size : 10pt; color : #040; margin-left: 10%; margin-right: 10%; padding-left:0; }\n",out);
			}
			else
			{
				fs("pre.udf { color : #000; margin-left: 0; }\n",out);
				fs("pre.prompt { color : #000; margin-left: 0; }\n",out);
				fs("pre.comment { color : #000; background: none; margin-left: 7%; }\n",out);
				fs("p.comment { color : #000; margin-left: 5%; margin-right: 5%; }\n",out);
				fs("p.indentedcomment { color : #000; margin-left: 10%; }\n",out);
				fs("h1.title { font-family : arial, sans-serif; text-align : right; font-size : 24pt; padding-bottom: 5pt; }\n",out);
				fs("h1.comment, h1.comment a { font-family : arial, sans-serif; text-align : right; font-size : 16pt; color : #000; }\n",out);
				fs("h1.comment, h2.comment a { font-family : arial, sans-serif; text-align : right; font-size : 12pt; color : #000; }\n",out);
				fs("h1.title, h1.comment, h2.comment { border-bottom: 2px solid #000; }\n",out);
			}
			fs("img.euler { }\n",out);
			fs("p.image { text-align:center; margin-left:50px; }\n",out);
			fs("a:link { color:#448; font-weight:bolder; text-decoration:none; }\n",out);
			fs("a:visited { color:#446; font-weight:bolder; text-decoration:none; }\n",out);
			if (mathjax) 
			{
				if (usecolors) fs("p.mathjax { color:#040; font-size: 9pt; text-align: center; padding-left:50px; }\n",out);
				else fs("p.mathjax { color:#000; font-size: 9pt; text-align: center; padding-left:50px; }\n",out);
			}
			fs("</style>\n",out);
		}
		if (mathjax)
		{
			fs("<script type=\"text/javascript\"\n",out);
			fs("src=\"http://cdn.mathjax.org/mathjax/latest/MathJax.js?config=TeX-AMS-MML_HTMLorMML\">\n",out);
			fs("</script>\n",out);
		}
		fs("</head>\n",out);
		fs("\n",out);
		fs("<body>\n",out);
	}

	fs("<h1 class=\"title\">",out);
	fs(title,out);
	fs("</h1>\n",out);
	
	int firstline=1;

	while (l)
		// go through all lines
	{
start:
		if (!l) break;

		if (l->Bitmap)
			// found a bitmap line
		{	
			if (l->ismathjax() || (l->latex() && exportlatexasmathjax))
				// export MathJax formula
			{
				fs("<p class=\"mathjax\">\n",out);
				fs("$$",out);
				fs(l->latex(),out);
				fs("$$",out);
				fs("</p>\n",out);
				firstline=1;
			}
			else if (l->bitmapline==0)
				// bitmap, exports only once in first line
			{
				fs("<p class=\"image\">\n",out);
				char s[MAXLINE];
				if (l->svg() && exists(l->svg()))
				{
					sprintf(s,"<object data=\"%s\" type=\"image/svg+xml\" width=\"%d\" height=\"%d\">\n",
						l->svg(),l->Bitmap->width(),l->Bitmap->height());
					fs(s,out);
				}
				char *large="";
				if (l->BitmapLarge && uselargeimagesforhtml) large="-large";
				if (strlen(l->text())>0) 
					sprintf(s,"%s%s-%03d-%s%s.png",imgpath.text(),name.text(),l->bitmapcount,l->text(),large);
				else
					sprintf(s,"%s%s-%03d%s.png",imgpath.text(),name.text(),l->bitmapcount,large);
				char sh[MAXLINE];
				translateblanks(s,sh);
				char t[MAXLINE];
				if (l->BitmapLarge)
				{
					if (strlen(l->text())>0) 
						sprintf(s,"%s%s-%03d-%s%s.png",imgpath.text(),name.text(),l->bitmapcount,l->text(),"-large");
					else
						sprintf(s,"%s%s-%03d%s.png",imgpath.text(),name.text(),l->bitmapcount,"-large");
					char shl[MAXLINE];
					translateblanks(s,shl);
					if (uselargeimagesforhtml)
					{
						sprintf(t,"<img class=\"euler\" src=\"%s\" "
							"srcset=\"%s 1x, %s 3x\" width=\"%d\" height=\"%d\" alt=\"%s\" />\n",
							shl,sh,shl,l->Bitmap->width(),l->Bitmap->height(),name.text());
					}
					else
					{
						sprintf(t,"<img class=\"euler\" src=\"%s\" "
							"srcset=\"%s 1x, %s 3x\" width=\"%d\" height=\"%d\" alt=\"%s\" />\n",
							sh,sh,shl,l->Bitmap->width(),l->Bitmap->height(),name.text());
					}
				}
				else
				{
					sprintf(t,"<img class=\"euler\" src=\"%s\" width=\"%d\" height=\"%d\" alt=\"%s\" />\n",
						sh,l->Bitmap->width(),l->Bitmap->height(),name.text());
				}
				fs(t,out);
				if (l->svg())
				{
					fs("</object>\n",out);
				}
				fs("</p>\n",out);
				while (l->next() && l->Bitmap && l->bitmapline>0) l=l->next();
			}
			l=l->next(); goto start;
		}
		else if (!l->hidden())
		{
			switch (l->type())
			{	
				case TextLine::output :
					fs("<pre class=\"output\">",out);
					while (l)
					{
						if (l->type()!=TextLine::output || l->Bitmap) break;
						fs(l->htmltext(),out);
						fs("\n",out);
						l=l->next();
					}
					fs("</pre>\n",out);
					goto start;
				case TextLine::prompt :
					if (l->length()>1 && strcmp(l->text(),">//")!=0 && strcmp(l->text(),"> //")!=0 && l->next())
					{
						if (l->next() && l->next()->type()==TextLine::udf) 
						{
							fs("<pre class=\"udf\">",out);
							fs(l->htmltext(),out);
							goto udf;
						}
						fs("<pre class=\"prompt\">",out);
						while (l)
						{
							if (l->type()!=TextLine::prompt || l->Bitmap ||
								!(l->length()>1 && strcmp(l->text(),">//")!=0 && strcmp(l->text(),"> //")!=0 && l->next())) 
									break;
							fs(l->htmltext(),out);
							fs("\n",out);
							l=l->next();
						}
						fs("</pre>\n",out);
						firstline=1; // first line of comment to come
						goto start;
					}
					break;
				case TextLine::udf :
					fs("<pre class=\"udf\">",out);
					while (l)
					{
						if (l->type()!=TextLine::udf || l->Bitmap) break;
						fs(" ",out);
						fs(l->htmltext()+1,out);
					udf:
						fs("\n",out);
						l=l->next();
					}
					fs("</pre>\n",out);
					goto start;
				case TextLine::comment :
					if (l->heading)
					{	
						String h(l->text());
						h.replace(" ","_");
						fs("<h1 class=\"comment\">",out);
						fs("<a name=\"",out);
						fs(h.text(),out);
						fs("\">",out);
						fs(l->htmltext(),out);
						fs("</a>",out);
						fs("</h1>\n",out);
						firstline=1;
					}
					else if (!strncmp(l->text(),"---",3))
					{
						fs("<h1 class=\"comment\">",out);
						fs("</h1>\n",out);
						firstline=1;
					}
					else if (!strncmp(l->text(),"** ",3))
					{
						fs("<h2 class=\"comment\">",out);
						fs(l->displaytext(),out);
						fs("</h2>\n",out);
						firstline=1;
					}
					else if (strstarts(l->text(),"mathjax: ") || strstarts(l->text(),"latex: "))
						goto special;
					else if (l->strippedsize()>0)
					{	
						if (!noreformat)
						{
							if (strstarts(l->text(),"- "))
							{
								fs("<ul class=\"comment\">\n",out);
								while (1)
								{
									if (!strstarts(l->text(),"- ")) 
									{
										fs("</ul>\n",out);
										goto start;
									}
									fs("<li>\n",out);
									fs(l->htmltext(2),out);
									fs("\n",out);
									l=l->next();
									while (l)
									{
										if (l->type()!=TextLine::comment || l->Bitmap) break;
										if (l->strippedsize()==0 || l->startsWith("- ")) break;
										fs(l->htmltext(),out);
										fs("\n",out);
										l=l->next();
									}
									fs("</li>\n",out);
								}
								fs("</ul>\n",out);
								goto start;
							}
							if (strstarts(l->text(),"See: ") || strstarts(l->text(),"see: ") ||
								strstarts(l->text(),"http:"))
							{
								fs("<p class=\"indentedcomment\">",out);
								fs(l->htmltext(),out);
								fs("\n",out);
								l=l->next();
								while (l)
								{
									if (l->type()!=TextLine::comment || l->Bitmap) break;
									if (!(strstarts(l->text(),"See: ") || strstarts(l->text(),"see: ") ||
											strstarts(l->text(),"http:"))) break;
									fs("<br />",out);
									fs(l->htmltext(),out);
									fs("\n",out);
									l=l->next();
								}
								fs("</p>",out);
								goto start;
							}
							if (strstarts(l->text()," ") && l->strippedsize()>0)
							{
								fs("<pre class=\"comment\">",out);
								fs(l->htmltext(),out);
								fs("\n",out);
								l=l->next();
								while (l)
								{
									if (l->type()!=TextLine::comment || l->Bitmap) break;
									if (!(strstarts(l->text()," ") && l->strippedsize()>0)) break;
									fs(l->htmltext(),out);
									fs("\n",out);
									l=l->next();
								}
								fs("</pre>",out);
								goto start;
							}
							fs("<p class=\"comment\">",out);
							while (l)
							{
								if (l->type()!=TextLine::comment || l->Bitmap) break;
								if (l->heading || !strncmp(l->text(),"---",3) ||
									!strncmp(l->text(),"** ",3) || 
									strstarts(l->text(),"mathjax: ") || strstarts(l->text(),"latex: ")) break;
								if (l->strippedsize()==0)
								{
									l=l->next(); break;
								}
								else
								{
									if (l->newline) fs("<br />",out);
									fs(l->htmltext(),out);
									fs("\n",out);
								}
								l=l->next();
							}
							fs("</p>\n",out);
						}
						else
						{
							fs("<pre class=\"comment\">",out);
							int haveempty=0;
							while (l)
							{
								if (l->type()!=TextLine::comment || l->Bitmap) break;
								if (l->heading || !strncmp(l->text(),"---",3) ||
									!strncmp(l->text(),"** ",3) || 
									strstarts(l->text(),"mathjax: ") || strstarts(l->text(),"latex: ")) break;
								if (haveempty) 
								{
									fs("\n",out);
									haveempty=0;
								}
								if (l->strippedsize()>0)
								{
									fs(l->htmltext(),out);
									fs("\n",out);
									haveempty=0;
								}
								else haveempty++;
								l=l->next();
							}
							fs("</pre>\n",out);
						}
						goto start;
					}
					else break;
			}
		}
		else if (l->type()==TextLine::comment)
		{
			special:
			if (strstarts(l->text(),"mathjax: "))
			{
				fs("<p class=\"mathjax\">\n",out);
				fs("$$",out);
				fshtml(l->text()+9,out);
				fs("$$",out);
				fs("</p>\n",out);
				firstline=1;
				if (l->next() && l->next()->type()==TextLine::comment && l->next()->Bitmap) l=l->next();
			}
			else if (strstarts(l->text(),"latex: ") && exportlatexasmathjax)
			{
				fs("<p class=\"mathjax\">\n",out);
				fs("$$",out);
				fshtml(l->text()+7,out);
				fs("$$",out);
				fs("</p>\n",out);
				firstline=1;
				if (l->next() && l->next()->type()==TextLine::comment && l->next()->Bitmap) l=l->next();
			}
		}
		l=l->next();
	}

	if (pin)
	{	while (pin)
		{	if (!fgets(pline,1022,pin)) break;
			fputs(pline,out);
		}
		fclose(pin);
	}
	else
	{	fputs("</body>\n",out);
		fputs("</html>\n",out);
	}

	fclose(out);
	textcritical.leave();
}

/*
Export notebook to Markdown format.
*/
void Text::exportmarkdown (char* filename)
{
	textcritical.enter("export markdown");

	FILE* out = fopen(filename, "w");
	if (!out)
	{
		textcritical.leave();
		Warning("Could not open this file!", "Euler", *W);
		return;
	}
	String fname(filename);
	fname.extension("");
	String name(fname.filename());
	String title(fname.filename());

	if (htmlutf) { fputc(239, out); fputc(187, out); fputc(191, out); }

	String imgname(fname);
	String imgpath("");
	if (saveimages && imagesindirectory)
	{
		imgname.stripfilename();
		imgname.cat("images\\");
		if (!exists(imgname.text())) _mkdir(imgname.text());
		imgname.cat(name.text());
		imgpath.copy("images/");
	}

	TextLine* l = Start;
	if (l->type() == TextLine::comment && l->heading)
	{
		title.copy(l->text());
		l = l->next(); l = l->next();
	}

	fs("# ", out);
	fs(title, out);
	fs("\n", out);

	int firstline = 1;

	while (l)
		// go through all lines
	{
	start:
		if (!l) break;

		if (l->Bitmap)
			// found a bitmap line
		{
			if (l->latex())
			{
				fs("$$", out);
				fs(l->latex(), out);
				fs("$$", out);
				firstline = 1;
			}
			else if (l->bitmapline == 0)
				// bitmap, exports only once in first line
			{
				char s[MAXLINE];
				if (strlen(l->text()) > 0)
					sprintf(s, "%s%s-%03d-%s.png", imgpath.text(), name.text(), l->bitmapcount, l->text());
				else
					sprintf(s, "%s%s-%03d.png", imgpath.text(), name.text(), l->bitmapcount);
				char sh[MAXLINE];
				translateblanks(s, sh);
				char t[MAXLINE];
				sprintf(t, "![%s](%s)\n", sh, sh);
				fs(t, out);
				fs("\n", out);
				while (l->next() && l->Bitmap && l->bitmapline > 0) l = l->next();
			}
			l = l->next(); goto start;
		}
		else if (!l->hidden())
		{
			switch (l->type())
			{
			case TextLine::output:
				while (l)
				{
					if (l->type() != TextLine::output || l->Bitmap) break;
					fs("    ",out);
					fs(l->htmltext(), out);
					fs("\n", out);
					l = l->next();
				}
				fs("\n",out);
				goto start;
			case TextLine::prompt:
				if (l->length() > 1 && strcmp(l->text(), ">//") != 0 && strcmp(l->text(), "> //") != 0 && l->next())
				{
					if (l->next() && l->next()->type() == TextLine::udf)
					{
						fs(l->htmltext(0,1), out);
						fs("\n\n", out);
						goto udf;
					}
					while (l)
					{
						if (l->type() != TextLine::prompt || l->Bitmap ||
							!(l->length() > 1 && strcmp(l->text(), ">//") != 0 && strcmp(l->text(), "> //") != 0 && l->next()))
							break;
						fs(l->htmltext(0,1), out);
						if (l->endsWith("..."))
						{
							fs("  \n\\>  ", out);
						}
						else fs("\n\n", out);
						l = l->next();
					}
					fs("\n", out);
					firstline = 1; // first line of comment to come
					goto start;
				}
				break;
			case TextLine::udf:
				fs("<pre class=\"udf\">", out);
				while (l)
				{
					if (l->type() != TextLine::udf || l->Bitmap) break;
					fs("    ", out);
					fs(l->text() + 1, out);
				udf:
					fs("\n", out);
					l = l->next();
				}
				fs("</pre>\n", out);
				goto start;
			case TextLine::comment:
				if (l->heading)
				{
					fs("# ", out);
					fs(l->htmltext(), out);
					fs("\n\n", out);
					firstline = 1;
				}
				else if (!strncmp(l->text(), "---", 3))
				{
					fs("---\n\n", out);
					firstline = 1;
				}
				else if (!strncmp(l->text(), "** ", 3))
				{
					fs("## ", out);
					fs(l->displaytext(), out);
					fs("\n\n", out);
					firstline = 1;
				}
				else if (strstarts(l->text(), "mathjax: ") || strstarts(l->text(), "latex: "))
					goto special;
				else if (l->strippedsize() > 0)
				{
						if (strstarts(l->text(), "- "))
						{
							while (1)
							{
								if (!strstarts(l->text(), "- "))
								{
									fs("\n", out);
									goto start;
								}
								fs("* \n", out);
								fs(l->htmltext(2), out);
								fs("\n", out);
								l = l->next();
								while (l)
								{
									if (l->type() != TextLine::comment || l->Bitmap) break;
									if (l->strippedsize() == 0 || l->startsWith("- ")) break;
									fs("* ", out);
									fs(l->htmltext(), out);
									fs("\n", out);
									l = l->next();
								}
								fs("\n", out);
							}
							goto start;
						}
						if (strstarts(l->text(), "See: ") || strstarts(l->text(), "see: ") ||
							strstarts(l->text(), "http:"))
						{
							fs(l->htmltext(),out);
							fs("  \n\n",out);
							l = l->next();
							goto start;
						}
						if (strstarts(l->text(), " ") && l->strippedsize() > 0)
						{
							fs(l->htmltext(), out);
							fs("  \n", out);
							l = l->next();
							while (l)
							{
								if (l->type() != TextLine::comment || l->Bitmap) break;
								if (!(strstarts(l->text(), " ") && l->strippedsize() > 0)) break;
								fs(l->htmltext(), out);
								fs("  \n", out);
								l = l->next();
							}
							fs("\n", out);
							goto start;
						}
						while (l)
						{
							if (l->type() != TextLine::comment || l->Bitmap) break;
							if (l->heading || !strncmp(l->text(), "---", 3) ||
								!strncmp(l->text(), "** ", 3) ||
								strstarts(l->text(), "mathjax: ") || strstarts(l->text(), "latex: ")) break;
							if (l->strippedsize() == 0)
							{
								l = l->next(); break;
							}
							else
							{
								if (l->newline) fs("\n\n", out);
								fs(l->htmltext(), out);
								fs("\n", out);
							}
							l = l->next();
						}
						fs("\n\n", out);
					goto start;
				}
				else break;
			}
		}
		else if (l->type() == TextLine::comment)
		{
		special:
			if (strstarts(l->text(), "mathjax: "))
			{
				fs("$$", out);
				fshtml(l->text() + 9, out);
				fs("$$", out);
				fs("\n", out);
				firstline = 1;
				if (l->next() && l->next()->type() == TextLine::comment && l->next()->Bitmap) l = l->next();
			}
			else if (strstarts(l->text(), "latex: ") && exportlatexasmathjax)
			{
				fs("$$", out);
				fshtml(l->text() + 7, out);
				fs("$$", out);
				fs("\n", out);
				firstline = 1;
				if (l->next() && l->next()->type() == TextLine::comment && l->next()->Bitmap) l = l->next();
			}
		}
		l = l->next();
	}

	fclose(out);
	textcritical.leave();
}

void fsl (char *s, FILE *out)
{
	fputs(s,out);
}

char * fsput (char *s, char *p)
{
	while (*s)
	{
		*p++=*s++;
	}
	return p;
}

void fsltext (char *s, FILE *out)
{
	static char h[MAXLINE];
	char *p=h;
	while (*s)
	{
		switch (*s)
		{
			case '{' :
				*p++='\\'; *p++='{'; break;
			case '}' :
				*p++='\\'; *p++='}'; break;
			case '�' :
				p=fsput("\\textpm{}",p); break;
			case '$' :
				p=fsput("\\textdollar{}",p); break;
			case '\\' :
				p=fsput("\\textbackslash{}",p); break;
			case '^' :
				p=fsput("\\textasciicircum{}",p); break;
			case '_' :
				p=fsput("\\_",p); break;
			case '|' :
				p=fsput("\\textbar{}",p); break;
			case '<' :
				p=fsput("\\textless{}",p); break;
			case '>' :
				p=fsput("\\textgreater{}",p); break;
			case '&' :
				p=fsput("\\&",p); break;
			case '%' :
				p=fsput("\\%",p); break;
			default :
				*p++=*s;
		}
		s++;
	}
	*p++=0;
	fsl(h,out);
}

void fsltt (char *s, FILE *out)
{
	static char h[MAXLINE];
	char *p=h;
	while (*s)
	{
		switch (*s)
		{
			case '{' :
				*p++='\\'; *p++='{'; break;
			case '}' :
				*p++='\\'; *p++='}'; break;
			case '�' :
				p=fsput("\\(\\pm\\)",p); break;
			case '\\' :
				p=fsput("\\(\\backslash\\)",p); break;
			default :
				*p++=*s;
		}
		s++;
	}
	*p++=0;
	fsl(h,out);
}

char * getstartdir ();

/*
Export a notebook for Latex to generate a PDF.
*/
void Text::exportlatex (char *filename)
{	
	textcritical.enter("export Latex");

	FILE *out=fopen(filename,"w");
	if (!out)
	{	textcritical.leave();
		Warning("Could not open this file!","Euler",*W);
		return;
	}

	TextLine *l=Start;

	char sty[MAXLINE];
	if (!exists("eumat.sty"))
	{
		sprintf(sty,"%seumat.sty",getstartdir());
		if (exists(sty)) file_copy(sty,"eumat.sty");
		else
		{
			Warning("Could not find eumat.sty!","Euler Math Toolbox");
			return;
		}
	}	
	FILE *in=fopen("eumat.sty","r");
	if (in)
	{
		if (fgets(sty,MAXLINE-32,in) && strstarts(sty,"% ")) fsl(sty+2,out);
		else fsl("\\documentclass{article}\n\n",out);
		fclose(in);
	}
	else fsl("\\documentclass{article}\n\n",out);
	fsl("\\usepackage{eumat}\n",out);
	
	fsl("\n\\begin{document}\n",out);

	fsl("\\begin{eulernotebook}\n",out);

	int type=-1;
	int firstline=1;

	String FN("");

	static char line[MAXLINE];

	while (l)
	{	
		char *text=l->text();
		if (isbom(text)) text+=3;
		else text=toutf(text);
		if (l->Bitmap)
		{	
			if (l->latex())
			{
				fsl("\\begin{eulerformula}\n",out);
				fsl("\\[\n",out);
				fsl(l->latex(),out);
				fsl("\n\\]\n",out);
				fsl("\\end{eulerformula}\n",out);
				type=-1;
				firstline=1;
			}
			else if (l->bitmapline==0)
			{
				TextLine *lh=l;
				int count=0;
				while (lh->next() && lh->next()->Bitmap==l->Bitmap) { count++; lh=lh->next(); }
				sprintf(line,"\\eulerimg{%d}{%s%s}\n",
					count,
					imagesindirectory?"images/":"",
					(l->BitmapLarge)?l->LargeFileName->filename():l->FileName->filename());
				fsl(line,out);
			}
		}
		else if (!l->hidden())
		{
			switch (l->type())
			{	
				case TextLine::output :
					if (type!=TextLine::output) fsl("\\begin{euleroutput}\n",out);
					type=TextLine::output;
					fsl("  ",out);
					fsltt(text,out);
					fsl("\n",out);
					break;
				case TextLine::prompt :
					if (l->length()>1 && strcmp(text,">//")!=0 && strcmp(text,"> //")!=0 && l->next())
					{
						if (type!=TextLine::prompt) fsl("\\begin{eulerprompt}\n",out);
						type=TextLine::prompt;
						fsltt(text,out);
						fsl("\n",out);
						firstline=1; // first line of comment to come
					}
					break;
				case TextLine::udf :
					if (type!=TextLine::udf) fsl("\\begin{eulerudf}\n",out);
					type=TextLine::udf;
					fsl("  ",out);
					fsltt(text+1,out);
					fsl("\n",out);
					break;
				case TextLine::comment :
					if (l->startsWith(" "))
					{
						if (type==TextLine::comment) fsl("\\end{eulercomment}\n",out);
						if (type!=20) fsl("\\begin{eulerttcomment}\n",out);
						fsltt(text,out);
						fsl("\n",out);
						type=20;
					}
					else if (l->heading)
					{	
						if (type==TextLine::comment) fsl("\\begin{eulercomment}\n",out);
						fsl("\\eulerheading{",out);
						fsltt(text,out);
						fsl("}\n",out);
						type=-1;
						firstline=1;
					}
					else if (!strncmp(text,"---",3))
					{
						if (type==TextLine::comment) fsl("\\end{eulercomment}\n",out);
						fsl("\\eulersubheading{}\n",out);
						type=-1;
						firstline=1;
					}
					else if (!strncmp(text,"** ",3))
					{
						if (type==TextLine::comment) fsl("\\end{eulercomment}\n",out);
						fsl("\\eulersubheading{",out);
						fsl(text+3,out);
						fsl("}\n",out);
						type=-1;
						firstline=1;
					}
					else if (strstarts(text,"mathjax: ") || strstarts(text,"latex: "))
						goto special;
					else
					{	
						if (type!=TextLine::comment)
						{
							fsl("\\begin{eulercomment}\n",out);
						}
						type=TextLine::comment;
						char *lout=text;
						if (*lout || !firstline)
						{
							fsltext(lout,out);
							if (l->next() && l->next()->type()==TextLine::comment && l->next()->newline) 
								fsl("\\\\\n",out);
							else fsl("\n",out);
							firstline=0;
						}
						
					}
					break;
			}
		}
		else if (l->type()==TextLine::comment)
		{
			special:
			char *p=text;
			if (strstarts(text,"mathjax: ")) p+=9;
			else if (strstarts(text,"latex: ")) p+=7;
			else p=0;
			if (p)
			{
				if (type==TextLine::comment) fsl("\\end{eulercomment}\n",out);
				fsl("\\begin{eulerformula}\n",out);
				fsl("\\[\n",out);
				fsl(p,out);
				fsl("\n\\]\n",out);
				fsl("\\end{eulerformula}\n",out);
				type=-1;
				firstline=1;
				if (l->next() && l->next()->type()==TextLine::comment && l->next()->Bitmap) l=l->next();
			}
		}
		l=l->next();
		int newtype=0;
		if (l)
		{
			newtype=l->type();
			if (newtype==TextLine::comment && l->startsWith(" ")) newtype=20;
		}
		if (l==0 || newtype!=type || l->Bitmap)
		{	switch (type)
			{	
				case TextLine::output :
					fsl("\\end{euleroutput}\n",out);
					break;
				case TextLine::udf :
					fsl("\\end{eulerudf}\n",out);
					break;
				case TextLine::comment :
					fsl("\\end{eulercomment}\n",out);
					break;
				case TextLine::prompt :
					fsl("\\end{eulerprompt}\n",out);
					break;
				case 20 :
					fsl("\\end{eulerttcomment}\n",out);
					break;
			}
			type=-1;
		}
	}

	fsl("\\end{eulernotebook}\n",out);

	fsl("\\end{document}\n",out);

	fclose(out);
	textcritical.leave();
}

/*
Determine and set the right type for the cursor line from the first character.
*/
void Text::settype ()
{   
	textcritical.enter("settype");
	if (*Cursor->text()=='>') Cursor->type(TextLine::prompt);
	else if (*Cursor->text()=='$') Cursor->type(TextLine::udf);
	W->update();
	textcritical.leave();
}

/*
Insert an image into a comment.
*/
void Text::insertcommentimage (char *filename, int reduce, char *largeimage,
	char *latex, int mathjax)
{
	int maxlines=25;
	while (*filename==' ') filename++;
	if (*filename=='(')
	{
		filename++;
		maxlines=0;
		while ('0' <= *filename && *filename <= '9')
		{
			maxlines=maxlines*10+(*filename-'0');
			filename++;
		}
		if (*filename==')') filename++;
		while (*filename==' ') filename++;
		if (maxlines<5) maxlines=5;
		if (maxlines>60) maxlines=60;
	}
	String name(filename);
	if (*name.extension()==0) name.cat(".png");
	String h(name);
	String h1("images\\"); h1.cat(name);
	if (imagesindirectory)
	{	
		if (exists(h1.text())) h.copy(h1);
	}
	else
	{	
		if (!exists(h.text())) h.copy(h1);
	}

	BitmapPS *m=new BitmapPS(textwindow,h.text());
	if (m->bitmaphandle()==0)
	{
		char error[1024];
		sprintf(error,"Could not open\n%s",filename);
		Warning(error,"Euler");
		return;
	}

	if (reduce) m->scale(m->width()/reduce,m->height()/reduce);

	if (m->width()>textwidth*0.8)
		m->scale(rd(textwidth*0.8/factor()),rd(textwidth*0.8*m->height()/m->width()/factor()));

	int lines=(int)(m->height()*factor()/text->textheight())+1;
	if (lines<1) lines=1;
	if (lines>maxlines)
	{	
		m->scale(rd(maxlines*text->textheight()*m->width()/m->height()/factor()),
			rd(maxlines*text->textheight()/factor()));
		lines=maxlines;
	}
	for (int i=0; i<lines; i++)
	{	
		TextLine *l=new TextLine(this,Cursor->prev(),Cursor);
		l->Bitmap=m;
		l->bitmapline=i;
		l->type(TextLine::comment);
		if (i==0 && largeimage)
		{
			BitmapPS *ml=new BitmapPS(textwindow,largeimage);
			l->BitmapLarge=ml;
		}
		if (i==0 && latex)
		{
			l->latex(latex,mathjax);
		}
	}
}

/*
Try to remove a file.
*/
int tryremove (char *filename)
{
	if (!exists(filename)) return 1;
	return !remove(filename);
}

/*
Try to find an exectuable in the path.
*/
int findexe (char *path, char *exe, char *exepath)
{
	if (strlen(path)>0)
	{
		strcpy(exepath,path);
		strcat(exepath,exe);
		strcat(exepath,".exe");
		if (exists(exepath)) return 1;
		strcpy(exepath,path);
		strcat(exepath,exe);
		if (exists(exepath)) return 1;
	}
	char *pcmd=searchcmd(exe);
	if (pcmd) { strcpy(exepath,pcmd); return 1; }
	return 0;
}

extern int latexams,latexlarger,
	latexmagnify,latexgreen,latexcleanup,latexsilent,latexnoerrors,
	latextransparent,latexkeeplarge,latexantialiasfactor,linux,latexantialias;
extern String slatexbin,slatexdir;

String pngfile("");

/*
Call Latex to produce a Latex PNG from a formula.
*/
char *Text::makelatexpng (char *formula,
		int alias, int textheight, int transparent, int color)
{
	String userdir("",MAXLINE);
	if (slatexdir.length()>0) userdir.copy(slatexdir);
	else userdir.copy(getusereulerdir());

	pngfile.copy(userdir);
	pngfile.cat("eulertemp.log");
	remove(pngfile);

	pngfile.copy(userdir);
	pngfile.cat("eulertemp.png");
	remove(pngfile);

	// make string for Latex
	pngfile.copy(userdir);
	pngfile.cat("eulertemp.tex");
	
	// create file eulertemp.tex
	FILE *out=fopen(pngfile.text(),"w");
	if (!out)
	{
		Warning("Could not open temporary file!\n"
			"Set the latex directory in the menu","Euler");
		return 0;
	}
	if (latexlarger) fputs("\\documentclass[12pt]{article}\n",out);
	else fputs("\\documentclass[11pt]{article}\n",out);
	if (latexams)
	{
		fputs("\\usepackage{amsmath}\n",out);
		fputs("\\usepackage{amsfonts}\n",out);
	}
	if (exists("eupng.sty"))
	{	
		pngfile.copy(userdir);
		pngfile.cat("eupng.sty");
		CopyFile("eupng.sty",pngfile.text(),0);
		fputs("\\usepackage{eupng}\n",out);
	}
	fputs("\\thispagestyle{empty}\n",out);
	fputs("\\begin{document}\n",out);
	while (*formula==' ') formula++;
	int env=(strstarts(formula,"\\begin{align*}") 
		|| strstarts(formula,"\\begin{multline*}") || strstarts(formula,"\\begin{gather*}"));
	if (!env) fputs("\\[\n",out);
	fputs(formula,out);
	fputs("\n",out);
	if (!env) fputs("\\]\n",out);
	fputs("\\end{document}\n",out);
	fclose(out);

	STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si,sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi,sizeof(pi));
	
	// Run Latex command
	char exepath[MAXLINE];
	if (!findexe(slatexbin,"latex",exepath))
	{	
		DumpWarning("Latex could not be found.\n"
			"Please configure Latex in the Menu!","Euler");
		return 0;
	}
	String command("",MAXLINE);
	sprintf(command,"\"%s\" %seulertemp.tex",
		exepath,latexsilent?"-interaction=nonstopmode ":"");
	if (!CreateProcess(0,command.text(),
		0,0,TRUE,latexsilent?CREATE_NO_WINDOW:0,0,userdir.text(),&si,&pi))
	{
		char ws[MAXLINE+255];
		sprintf(ws,
			"Could not start the Latex process!\n"
			"Command Line was:\n%s\n%s",
			command.text(),getsystemerror());
		Warning(ws,"Euler");
		return 0;
	}
	CurrentProcess=pi.hProcess;
	if (WaitForSingleObject(pi.hProcess,10000))
	{
		if (!linux)
		{
			if (Question("Waiting for Latex takes longer than expected.\n"
				"Maybe Latex has to install packages?\nContinue Waiting?","EMT",textwindow))
			{	
				WaitForSingleObject(pi.hProcess,INFINITE);
			}
			else
			{
				DumpWarning(
					"Waiting for the Latex process failed!\n"
					"%s\n"
					"Please retry.",
					"EMT",command.text());
				latexsilent=0;
				CurrentProcess=0;
				return 0;
			}
		}
		else
		{
			pngfile.copy(userdir);
			pngfile.cat("eulertemp.log");
			int tries=0;
			while (!exists(pngfile) && tries<100) { tries++; Sleep(200); }
		}
	}
	CurrentProcess=0;
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );

	pngfile.copy(userdir);
	pngfile.cat("eulertemp.log");
	if (!exists(pngfile))
	{
		Warning("Latex did not produce log file!\n"
			"Maybe setup Latex in the menu!","Euler");
		return 0;
	}
	FILE *in=fopen(pngfile,"r");
	char line[1024];
	while (fgets(line,1020,in))
	{
		if (!strncmp(line,"! ",2))
		{
			String error(line);
			while (fgets(line,1020,in) && strlen(line)>2)
			{
				error.cat(line);
			}
			fclose(in);
			Warning(error,"Euler");
			return 0;
		}
	}
	fclose(in);

    ZeroMemory(&si,sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi,sizeof(pi));

	// Run dvipng command
	if (!findexe(slatexbin,"dvipng",exepath))
	{	
		DumpWarning("Program dvipng could not be found.\n"
			"Please configure Latex in the Menu!","Euler");
		return 0;
	}
	sprintf(command,"\"%s\"",exepath);
	command.cat(" -o eulertemp.png -D ");
	char magn[32];
	int magf=latexantialiasfactor*100;
	if (!latexantialias) magf=100;
	if (alias) sprintf(magn,"%d",rd(magf*textheight/16));
	else sprintf(magn,"%d",magf);
	command.cat(magn);
	command.cat(" -T tight");
	if (transparent>0) command.cat(" -bg Transparent");
	else 
	{
		char s[256];
		int red,green,blue;
		if (transparent==0)
		{
			Color *c=textwhite;
			red=c->getRed(); blue=c->getBlue(); green=c->getGreen();
		}
		else if (transparent==-1) getcolor(0,&red,&green,&blue);
		sprintf(s," -bg \"rgb %g %g %g\"",red/255.0,green/255.0,blue/255.0);
		command.cat(s);
	}
	if (color!=0) 
	{
		char s[256];
		int red,green,blue;
		getcolor(color,&red,&green,&blue);
		sprintf(s," -fg \"rgb %g %g %g\"",red/255.0,green/255.0,blue/255.0);
		command.cat(s);
	}
	command.cat(" eulertemp.dvi");
	if (!CreateProcess(0,command.text(),
		0,0,TRUE,latexsilent?CREATE_NO_WINDOW:0,0,userdir.text(),&si,&pi))
	{
		char ws[MAXLINE+255];
		sprintf(ws,"Could not start the dvipng process!\n"
			"Setup Latex in the menu!\nCommand Line was:\n%s\n%s",
			command.text(),getsystemerror());
		Warning(ws,"Euler");
		return 0;
	}
	CurrentProcess=pi.hProcess;
	if (WaitForSingleObject(pi.hProcess,INFINITE))
	{
		if (!linux)
		{
			Warning("Waiting for the dvipng process failed!","Euler");
			latexsilent=0;
			CurrentProcess=0;
			return 0;
		}
		else
		{
			int tries=0;
			pngfile.copy(userdir);
			pngfile.cat("eulertemp.png");			
			while (!exists(pngfile) && tries<100) { tries++; Sleep(200); }
		}
	}
	CurrentProcess=0;
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );

	if (latexcleanup)
	{
		pngfile.copy(userdir);
		pngfile.cat("eulertemp.log");
		if (!tryremove(pngfile))
		{
			DumpWarning("Could not remove\n%s","Euler",pngfile.text());
			return 0;
		}

		pngfile.copy(userdir);
		pngfile.cat("eulertemp.aux");
		if (!tryremove(pngfile))
		{
			DumpWarning("Could not remove\n%s","Euler",pngfile.text());
			return 0;
		}

		pngfile.copy(userdir);
		pngfile.cat("eulertemp.dvi");
		if (!tryremove(pngfile))
		{
			DumpWarning("Could not remove\n%s","Euler",pngfile.text());
			return 0;
		}

		pngfile.copy(userdir);
		pngfile.cat("eulertemp.tex");
		if (!tryremove(pngfile))
		{
			DumpWarning("Could not remove\n%s","Euler",pngfile.text());
			return 0;
		}
	}

	pngfile.copy(userdir);
	pngfile.cat("eulertemp.png");

	return pngfile.text();
}

/*
Run the formula through Latex, convert to image, and 
insert the image into the comment area at the cursor
position.
*/
void Text::insertlatex (char *formula, int mathjax)
{
	Color *c=textgreen;
	int col=0;
	if (latexgreen) col=getcolor(c->getRed(),c->getGreen(),c->getBlue());
	char *p=makelatexpng(formula,latexmagnify,textheight(),latextransparent,col);
	if (!p) return;

	insertcommentimage(p,latexantialias?latexantialiasfactor:1,
		latexkeeplarge?p:0,formula,mathjax);
}

/*
Insert a Maxima formula via Latex as a PNG image.
*/
void Text::insertmaxima (char *formula, int mathjax)
{
	char line[MAXLINE+20];
	strcpy(line,"tex(");
	strcat(line,formula);
	strcat(line,");");
	runmaxima(line,2,0);

	// try to find tex output
	char *pout=maxima_output();
	char *p=pout;
	if (error) 
	{
		error=0;
		return;
	}

	char *q=line;
	*q=0;
	int found=0;
	while (*p)
	{
		if (*p=='$' && *(p+1)=='$') 
		{
			if (found) *q=0;
			p+=2; found=!found;
			if (!found) break;
		}
		else
		{
			if (found) *q++=*p++;
			else p++;
		}
	}

	if (*line) // found a string
	{
		insertlatex(line,mathjax);
	}
	else
	{
		Warning(pout,"Euler");
		return;
	}
}

/*
Place the comment into the buffer.
*/
void Text::commenttobuffer (char *comment)
{
	*comment=0;
	textcritical.enter("comment");
	TextLine *l=Cursor;
	char *p,*q;
	while (l->prev() && l->prev()->type()==TextLine::comment)
	{	
		l=l->prev();
	}

	int linestart=1; // next line starts a new comment line
	while (l!=Cursor && l->type()==TextLine::comment)
	{	
		if (!(l->Bitmap))
		{
			p=l->text(); // p points to text of line
			if (*p==239-256 && *(p+1)==187-256 && *(p+2)==191-256)
			{
				p=l->getoriginal();
				if (!p) p=(l->text())+3;
			}
			if (*p==0) // it is an empty line (breaking comment lines)
			{	
				if (!linestart) strcat(comment,"\r\n"); // finish previous line
				strcat(comment,"\r\n"); // add empty line in any case
				linestart=1; // next line is a starting line
			}
			else // not an empty line (breaks only if l->newline or l->heading)
			{
				if (!linestart && (l->newline || l->heading || l->startsWith("%% "))) strcat(comment,"\r\n"); 
					// newline or heading flag force a new comment line here
				else if (!linestart) strcat(comment," ");
					// add a blank, unless this is a starting line
				if (l->heading) strcat(comment,"* ");
					// add a heading marker for a heading line
				strcat(comment,p); // add the text of the line
				linestart=0;
			}
			// skip blanks at the end of the current comment
			q=comment+strlen(comment)-1;
			while (*q==' ') { *q=0; q--; }
		}
		l=l->next();
	}
	textcritical.leave();
}

#define TABSIZE 4

void tabextendcopy (char *dest, char *source, int length, int maxdest)
{
	char *deststart=dest,*destend=dest+maxdest;
	for (int i=0; i<length; i++)
	{
		if (*source=='\t')
		{
			*dest++=' ';
			int k=(dest-deststart)%TABSIZE;
			if (k>0)
				for (int j=0; j<TABSIZE-k; j++) 
					if (dest<destend-1) *dest++=' ';
			source++;
		}
		else *dest++=*source++;
		if (dest>=destend-1) break;
	}
	*dest++=0;
}

void Text::buffertocomment (char *comment)
{
	static char line[1024];
	textcritical.enter("comment end");

	// delete old comment
	TextLine *l=Cursor,*h;
	while (l->prev() && l->prev()->type()==TextLine::comment)
	{	
		l=l->prev();
	}
	while (l!=Cursor)
	{	
		h=l->next();
		if (Start==l) Start=h;
		delete l;
		l=h;
	}

	// insert new comment
	char *p;
	p=comment; // p points to the new comment (containing \r and \n)
	int newline=0; // remember to force new line in next line
	while (*p)
	{   
		char *startline=p;
		while (*p==' ') p++; // skip blanks

		char *linebreak=p; // remember last chance to break the line
		int breakline=1;
		if (strstarts(startline,"latex: ") 
			|| strstarts(startline,"maxima: ") || strstarts(startline,"&latex: ") 
			|| strstarts(startline,"image: ") || strstarts(startline,"http") 
			|| strstarts(startline,"See: ") || strstarts(startline,"See: ")
			|| strstarts(startline,"mathjax: ")
			|| strstarts(startline,"%% ")) breakline=0;
		while (1) // search for a break point 
		{
			while (*p!=0 && *p!='\n' && *p!='\r' && *p!=' ') p++; // search next break
			if (breakline && p-startline>linelength)
			{
				if (linebreak>startline) p=linebreak; // break at last good spot
				break;
			}
			if (*p==0 || *p=='\n' || *p=='\r') break;
			linebreak=p;
			while (*p==' ') p++;
		}
		
		// copy new comment line to line
		tabextendcopy(line,startline,p-startline,1024);
		
		// insert it into the notebook
		l=new TextLine(this,Cursor->prev(),Cursor);
		l->type(TextLine::comment);
		if (*line) l->newline=newline;

		if (!strncmp(line,"image: ",7))
		{
			l->copy(line);
			insertcommentimage(line+7);
		}
		else if (!disablelatex && strstarts(line,"latex: "))
		{
			l->copy(line);
			insertlatex(line+7,0);
		}
		else if (!disablelatex && displaymathjaxaslatex && strstarts(line,"mathjax: "))
		{
			l->copy(line);
			insertlatex(line+8,1);
		}
		else if (!disablelatex && displaymathjaxaslatex && strstarts(line,"&mathjax: "))
		{
			l->copy(line);
			insertmaxima(line+9,1);
		}
		else if (!strncmp(line,"maxima: ",8))
		{
			l->copy(line);
			insertmaxima(line+8,0);
		}
		else if (!strncmp(line,"&latex: ",8))
		{
			l->copy(line);
			insertmaxima(line+8,0);
		}
		else if (!strncmp(line,"* ",2))
		{
			l->copy(line+2);
			l->heading=1;
		}
		else
		{
			l->copy(line);
			l->makeutf();
		}
		
		if (Start==Cursor) Start=l; // update top of text window
		
		while (*p==' ') p++; // skip blanks after break

		newline=0; 

		if (*p=='\n' || *p=='\r') 
		{
			p+=2; // skip line break
			if (*line && !l->heading) newline=1; // force new line in next line
		}
	}

	// Top=Cursor;
	updatetop();
	showcomment();
	changed(1);
	textcritical.leave();
	W->update();
}

void Text::updatecomment ()
{
	if (text->cursorline()->prev() 
		&& text->cursorline()->prev()->type()==TextLine::comment)
	{
		static char comment[16*1024l];
		text->commenttobuffer(comment);
		text->buffertocomment(comment);
	}
}

extern int smallscreen;

class CommentDialog : public Dialog
{
public :
	CommentDialog (Window &w, int id) : Dialog(w,id)
	{}
	void layout ()
	{
		positionelement(ID_OK,10,-1,-1,10);
		int w=elementwidth(ID_OK);
		positionelement(ID_CANCEL,w+30,-1,-1,10);
		positionelement(ID_Help,-1,10,-1,10);
		int h=elementheight(ID_OK);
		sizeelement(ID_Comment,10,10,10,h+20);
	}
};

extern int commentx,commenty,commentw,commenth,wscreen,hscreen;

// Edit the current comment in the current line.
void Text::comment ()
{	
	CaretUpdate=1;
	
	CommentDialog d(*W,IDD_Comment);

	static char comment[16*1024l];
	static char line[1024];
	strcpy(comment,"");
	
	commenttobuffer(comment);

	// start modal dialog
	MultilineItem edit(ID_Comment,d,comment);
	edit.setfont(*courier);
	HelpButton help(d,"documentation/gui.html#Comment");

	d.fitsize(wscreen,hscreen,commentx,commenty,commentw,commenth,400,300);
	d.carryout();

	// back from dialog
	d.getsize(commentx,commenty,commentw,commenth);
	
	strcpy(comment,edit);
	if (d.result()!=Dialog::ok) return;

	buffertocomment(comment);
}

int Text::endfunction ()
{  	
	textcritical.enter("endfunction");
	CaretUpdate=1;
	if (udf==1 && editing && Cursor->type()==TextLine::udf &&
			strlen(*Cursor)<=1)
	{	
		Cursor->copy("$endfunction"); Cursorpos=1;
		W->update();
		textcritical.leave();
		return 1;
	}
	textcritical.leave();
	return 0;
}

void Text::setscroll ()
{	
	textcritical.enter("setscroll");
	int na=0,nt=0;
	TextLine *h=Start;
	while (h)
	{	if (Top==h) na=nt;
		nt++;
		h=h->next();
	}
	if (nt>0) W->setscroll(StandardWindow::vscroll,na*100/nt);
	else W->setscroll(StandardWindow::vscroll,0);
	textcritical.leave();
}

void Text::setscroll (int pos)
{   
	textcritical.enter("setscroll pos");
	int nt=0;
	TextLine *h=Start;
	while (h)
	{	nt++; h=h->next();
	}
	if (nt==0) goto stop;
	nt=nt*pos/100;
	h=Start;
	while (h && nt>0)
	{	nt--;
		h=h->next();
	}
	if (h)
	{   Top=h;
		W->update();
	}
stop : textcritical.leave();
}

void Text::sethscroll ()
{	
	textcritical.enter("sethscroll");
	int pos=100*Offset/(W->width()*3);
	if (pos<0) pos=0;
	if (pos>100) pos=100;
	W->setscroll(StandardWindow::hscroll,pos);
	textcritical.leave();
}

void Text::sethscroll (int pos)
{	
	textcritical.enter("sethscroll pos");
	Offset=pos*(W->width()*3)/100;
	if (Offset<0) Offset=0;
	CaretUpdate=0;
	textcritical.leave();
}

void Text::pageup (int untilbreak)
{   
	textcritical.enter("page up");
	CaretUpdate=1;
	if (!editing  || inudf())
	{	textcritical.leave(); return;
	}
	WindowPS ps(*W);
	getheight();
	int lines=texth/ps.textheight()/2;
	if (untilbreak) lines=500;
	while (lines>0 && Top->prev())
	{	
		Top=Top->prev();
		if (Top->type()==TextLine::comment && 
			Top->heading || Top->isSubHeading() || Top->isCommentBreak()) break;
		lines--;
	}
	TextLine *l=Top;
	while (l && l->type()!=TextLine::prompt) l=l->next();
	if (l)
	{	Cursor=l; Cursorpos=1;
	}
	if (Dragline)
	{	mark(Cursor,Cursorpos);
	}
	textcritical.leave();
	W->update();
}

void Text::pagedown (int untilbreak)
{   
	textcritical.enter("page end");
	CaretUpdate=1;
	if (!editing || inudf())
	{	textcritical.leave(); return;
	}
	WindowPS ps(*W);
	getheight();
	int lines=texth/ps.textheight()/2;
	if (untilbreak) lines=500;
	while (lines>0 && Top->next())
	{	
		Top=Top->next();
		if (Top->type()==TextLine::comment && 
			Top->heading || Top->isSubHeading() || Top->isCommentBreak()) break;
		lines--;
	}
	TextLine *l=Top;
	while (l && l->type()!=TextLine::prompt) l=l->next();
	if (l)
	{	Cursor=l; Cursorpos=1;
	}
	if (Dragline)
	{	mark(Cursor,Cursorpos);
	}
	textcritical.leave();
	W->update();
}

void Text::gototextend ()
{	
	textcritical.enter("goto text end");
	CaretUpdate=1;
	if (!editing || inudf())
	{	textcritical.leave(); return;
	}
	WindowPS ps(*W);
	while (Top->next())
	{	Top=Top->next();
	}
	TextLine *l=Top;
	while (l && l->type()!=TextLine::prompt) l=l->prev();
	if (l)
	{	Cursor=l; Cursorpos=1;
	}
	getheight();
	int lines=texth/ps.textheight()/2;
	while (lines>0 && Top->prev())
	{	
		Top=Top->prev();
		lines--;
	}
	textcritical.leave();
	W->update();
}

void Text::gototextstart()
{	
	textcritical.enter("goto text start");
	CaretUpdate=1;
	if (!editing || inudf())
	{	textcritical.leave(); return;
	}
	WindowPS ps(*W);
	while (Top->prev())
	{	Top=Top->prev();
	}
	TextLine *l=Top;
	while (l && l->type()!=TextLine::prompt) l=l->next();
	if (l)
	{	Cursor=l; Cursorpos=1;
	}
	textcritical.leave();
	W->update();
}

int Text::isinmaxima ()
{
	char *start=currentline()+lefteditlimit;
	if (*start==':') return 1;
	char *q=currentline()+Cursorpos;
	while (q>=start)
	{
		if (*q==' ') break;
		if (*q=='&') return 1;
		q--;
	}
	while (q>=start)
	{
		if (*q=='&' && *(q+1)=='=') return 1;
		q--;
	}
	return 0;
}

void Text::setcursorline (TextLine *l, int pos)
{	
	textcritical.enter("setcursorline");
	Cursor=l; Cursorpos=pos;
	textcritical.leave();
}

void Text::setcursorpos (int pos)
{	
	textcritical.enter("setcursorpos");
	Cursorpos=pos;
	if (Dragline)
	{	mark(Cursor,Cursorpos);
	}
	textcritical.leave();
}

void Text::scrollup ()
{   
	textcritical.enter("scroll up");
	CaretUpdate=1;
	if (Top->prev()) Top=Top->prev();
	textcritical.leave();
	W->update();
}

void Text::scrolldown ()
{   
	textcritical.enter("scroll down");
	CaretUpdate=1;
	if (Top->next()) Top=Top->next();
	textcritical.leave();
	W->update();
}

void Text::showcursor ()
{	
	textcritical.enter("showcursor");
	TextLine *h=top(Cursor);
	if (h==Top)
	{	textcritical.leave();
		return;
	}
    Top=h;
	textcritical.leave();
	W->update();
}

void Text::showcursorline ()
// make sure to show the line of the cursor
{	
	textcritical.enter("showcursorline");
	TextLine *h=top(Cursor);
	if (h==Top)
	{	
		textcritical.leave(); return;
	}
    Top=h;
	textcritical.leave();
	W->update();
}

void Text::computecursor (int x, int y, TextLine * &l, int &pos)
// compute the cursor line and position at x,y
{  
	if (!editing) return;
	textcritical.enter("computecursor");
	if (x<0) x=0;
	if (y<0) y=0;
	WindowPS ps(*W);
	setfont(ps);
	TextLine *h=Top;
	int c=roffset();
	while (1)
	{   
		if (!h->next()) break;
		c+=h->height();
		if (c>y) break;
		h=h->next();
	}
	l=h;
	pos=strlen(l->text());
	String Work(h->text());
	int w,ht;
	x+=Offset-l->indent();
	while (pos>0)
	{   
		Work.text()[pos]=0;
		ps.textextent(Work,w,ht);
		if (w<=x) break;
		pos--;
	}
	if (l->type()==TextLine::comment && (l->heading || l->isCommentBreak())) pos=0;
	textcritical.leave();
}

void Text::selectall ()
{	
	textcritical.enter("select all");
	Markend=Markstart=Start; Markstartpos=0;
	while (Markend->next())
	{   Markend->mark(1);
		Markend=Markend->next();
	}
	Markendpos=strlen(Markend->text());
	textcritical.leave();
	W->update();
}

int Text::cursorinmark ()
{	return Cursor==Markstart && Cursor==Markend;
}

void Text::deletemark ()
{	
	textcritical.enter("delete mark");
	if (!waiting || Markstart==0) { textcritical.leave(); return; }
	if (!editing || udf==1) { textcritical.leave(); return; }
	if (Markstart==Markend)
	{	if (Cursor==Markstart && Markstartpos==0) Markstartpos=1;
		Markstart->del(Markstartpos,Markendpos-Markstartpos);
		if (Cursor==Markstart && Cursorpos>Markstartpos)
		{	Cursorpos=Markstartpos;
		}
	}
	else
	{	
		changed(1);
		TextLine *l=Markstart;
		TextLine *last=Markstart;
		if (Markstartpos>0) 
		{	l->del(Markstartpos,l->length()-Markstartpos);
			l=l->next();
		}
		else last=Markstart->prev();
		int lostcursor=0;
		while (l)
		{	int end=(l==Markend);
			if (end && l->length()>Markendpos) break;
			if (l==Cursor) lostcursor=1;
			TextLine *h=l->next();
			TextLine *hp=l->prev();
			delete l;
			l=h;
			if (!hp) Start=l;
			if (end) break;
		}
		if (l && l==Markend)
		{	if (l==Cursor)
			{	l->del(1,Markendpos);
				Cursorpos=1;
			}
			else l->del(0,Markendpos);
		}
		if (lostcursor)
		{	if (last)
			{	Cursor=last;
				Cursorpos=1;
				insertline();
				newprompt();
			}
			else
			{	Cursor=Start;
				Cursorpos=1;
				if (Cursor==0) clear();
				else insertline();
				newprompt();
			}
		}
		updatetop();
	}
	unmarkall();
	textcritical.leave();
	W->update();
}

void Text::shift (int flag)
{	
	textcritical.enter("shift");
	if (flag)
	{	if (!Dragline)
		{	Dragline=Cursor; Dragpos=Cursorpos;
		}
	}
	else Dragline=0;
	textcritical.leave();
}

int Text::startdrag (int x, int y)
{	
	if (!waiting || !editing)
		return 0;
	textcritical.enter("start drag");
	computecursor(x,y,Dragline,Dragpos);
	if (Dragline->Bitmap)
	{
		TextLine *p=Dragline;
		while (p->prev() && p->prev()->Bitmap) p=p->prev();
		while (p && p->Bitmap)
		{
			p->bitmapmark(1);
			p=p->next();
		}
		Dragline=0;
		textwindow.update();
		textcritical.leave();
		return 0;
	}
	textcritical.leave();
	return 1;
}

void Text::drag (int x, int y)
{	
	textcritical.enter("drag");
	TextLine *l;
	int pos;
	computecursor(x,y,l,pos);
	mark(l,pos);
	textcritical.leave();
}

void Text::enddrag (int x, int y)
	// Called at mouse-up after dragging with the mouse
	// or clicking and not dragging.
{   
	textcritical.enter("end drag");

	if (!Dragline) // unknown mouse up event
	{ 
		textcritical.leave(); return; 
	}

	// not clicked into a prompt
	if (Dragline->type()!=TextLine::prompt) 
	{
		if (udf!=1 && Markstart==0 || Markstart==Markend && Markstartpos==Markendpos)
			// nothing was marked (simple click).
			// ignored, if in simple udf input mode.
		{
			if (Dragline->type()==TextLine::udf)
				// The user clicked into an udfline.
			{
				udf=2; // edit udf mode
				TextLine *l;
				int pos;
				computecursor(x,y,l,pos);
				Cursor=l;
				Cursorpos=pos;
				markbracket();
				W->update();
			}
			else
				// The user clicked not on an udf line.
			{
				udf=0;
				TextLine *l=Dragline;
				// goto the next prompt
				while (l->next() && l->next()->type()!=TextLine::prompt)
					l=l->next();
				l=l->next();
				if (l)
				{
					Cursor=l; Cursorpos=1;
					Markstart=Markend=0;
					Dragline=0;
					showcursorline();
					W->update();
				}
			}
		}
		textcritical.leave(); return; 
	}
	TextLine *l;
	int pos;
	computecursor(x,y,l,pos);
	if (l==Dragline && pos==Dragpos && udf!=1)
	{	
		udf=0;
		Cursor=Dragline; Cursorpos=Dragpos;
		if (Cursorpos<1) Cursorpos=1;
		W->update();
		textcritical.leave();
		markbracket();
		return;
	}
	mark(l,pos);
	if (!inudf() && Markstart && Markstart==Markend 
		&& Markstart->type()==TextLine::prompt)
	{	
		Cursor=Markstart; Cursorpos=Markstartpos;
		W->update();
	}
	BracketMark=0;
	textcritical.leave();
}

void Text::unmarkall ()
{   
	textcritical.enter("unmark all");
	TextLine *h=Start;
	while (h)
	{	h->mark(0); h->bitmapmark(0); h=h->next();
	}
	Markstart=Markend=0;
	textcritical.leave();
}

void Text::unmark ()
{   if (!Markstart) return;
	unmarkall();
	W->update();
}

void Text::redraw () 
{	
	textcritical.enter("redraw");
	// if (!textcritical.tryenter()) return;
	WindowPS ps(*W);
	// check, if the graphics is in the text window, and visible.
	if (showgraphics) 
		graphicswindow->redraw(ps);
	else 
		redraw(ps);
	textcritical.leave();
}

extern int second;

void Text::redraw (PS &ps)
{	
	textcritical.enter("redraw");
	if (!Buffer)
	{   
		if (onewindow)
		{
			Buffer=new BitmapPS(*W,textw,texth);
		}
		else
		{
			if (second)
			{
				texth=W->height()-20; 
				textw=W->width()-20;
				Buffer=new BitmapPS(*W,textw,texth);
				textx=texty=10;
			}
			else
			{
				texth=W->height();
				textw=W->width();
				Buffer=new BitmapPS(*W,textw,texth);
				textx=texty=0;
			}
		}
	}
	drawbelow(*Buffer,Top);
	if (CaretOn) W->hidecaret();
	if (second) ps.erase(*textback);
	if (ShowUpdate) Buffer->copy(ps,textx,texty);
	setcaret(ps);
	if (CaretUpdate && (CX>W->width()*9/10 || (CX<W->width()/10 && Offset>0)))
	{	
		Offset=CX+Offset-W->width()/2;
		if (Offset<0) Offset=0;
		drawbelow(*Buffer,Top);
		Buffer->copy(ps,textx,texty);
		setcaret(ps);
		sethscroll();
	}
	if (CaretOn) W->showcaret();
	textcritical.leave();
}

/*
Mark the text from Dragline and Dragpos to the line and pos.
*/
void Text::mark (TextLine *l, int pos)
{	
	textcritical.enter("mark");
	if (l==Dragline && pos==Dragpos) { textcritical.leave(); return; }
	TextLine *h=Start;
	if (l==Dragline)
	{   unmarkall();
		if (Dragpos==pos) { textcritical.leave(); return; }
		Markstart=Markend=Dragline;
		if (pos<Dragpos)
		{	Markstartpos=pos; Markendpos=Dragpos;
		}
		else
		{	Markstartpos=Dragpos; Markendpos=pos;
		}
		l->mark(1);
		textcritical.leave();
		W->update();
        return;
	}
	int m=0;
	while (h)
	{	if (m==0 && h==l)
		{	m=1; h->mark(m);
			Markstart=l; Markstartpos=pos;
		}
		else if (m==0 && h==Dragline)
		{	m=1; h->mark(m);
			Markstart=Dragline; Markstartpos=Dragpos;
		}
		else if (m==1 && h==l)
		{	h->mark(m); m=0;
			Markend=l; Markendpos=pos;
		}
		else if (m==1 && h==Dragline)
		{	h->mark(m); m=0;
			Markend=Dragline; Markendpos=Dragpos;
		}
		else h->mark(m);
		h=h->next();
	}
	textcritical.leave();
	W->update();
}

int isnameletter (char c)
{
	return xisalpha(c) || c=='%' || c=='$' || xisdigit(c);
}

int xisdigitletter (char c)
{
	return xisdigit(c) || c=='.';
}

void Text::markcommand (int x, int y)
{	
	TextLine *l;
	int pos;
	computecursor(x,y,l,pos);
	if (!l) return;
	int left=lefteditlimit;
	if (l->type()!=TextLine::prompt) lefteditlimit=0;
	Markstart=Markend=l; 
	BracketMark=0;
	char *s=l->text();
	if (l->type()==TextLine::comment && 
		(l->startsWith("http") || l->startsWith("See:") || l->startsWith("see:")))
	{
		Markstartpos=0; Markendpos=l->length();
	}
	else if (l->type()==TextLine::prompt && l->startsWith(">load "))
	{
		Markstartpos=1; Markendpos=l->length();
	}
	else
	{
		int p1=pos,p2=pos;
		if (xisalpha(s[pos]))
		{
			while (p1>left && isnameletter(s[p1-1])) p1--;
			while (p2<l->length() && isnameletter(s[p2])) p2++;
		}
		else if (s[pos]==0 || pos==0)
		{
			p1=left; p2=l->length();
		}
		else if (xisdigitletter(s[pos]))
		{
			while (p1>left && xisdigitletter(s[p1-1])) p1--;
			while (p2<l->length() && xisdigitletter(s[p2])) p2++;		
		}
		else
		{
			while (p1>left && !xisalpha(s[p1-1])) p1--;
			while (p2<l->length() && !xisalpha(s[p2])) p2++;		
		}
		Markstartpos=p1; Markendpos=p2;
	}
	Markend->mark(1);
	W->update();
	lefteditlimit=left;
}

extern void docomment();
extern void doedit();

int Text::rightclicked (int x, int y)
{
	TextLine *l;
	int pos;
	computecursor(x,y,l,pos);
	if (l->type()==TextLine::udf)
		// The user clicked into an udfline.
	{
		while (l->prev() && l->prev()->type()==TextLine::udf)
			l=l->prev();
		l=l->prev();
		if (l)
		{
			Cursor=l; Cursorpos=1;
			W->update();
			doedit();
		}
	}
	else
		// The user clicked not on an udf line.
	{
		udf=0;
		// goto the next prompt
		while (l->next() && l->next()->type()!=TextLine::prompt)
			l=l->next();
		l=l->next();
		if (l)
		{
			Cursor=l; Cursorpos=1;
			showcursorline();
			W->update();
			docomment();
		}
	}
	return 0;
}

char * Text::getlinemark ()
{	static char t[256];
	if (Markstart && Markstart==Markend && Markendpos-Markstartpos<255)
	{	strncpy(t,Markstart->text()+Markstartpos,Markendpos-Markstartpos);
		t[Markendpos-Markstartpos]=0;
		return t;
	}
	else return 0;
}

void Text::deleteoutput ()
{   
	CaretUpdate=1;
	if (!waiting) return;
	textcritical.enter("delete output");
	TextLine *h=Markstart,*n;
	while (h)
	{   
		n=h->next();
		if (h->type()==TextLine::output)
		{   
			if (Start==h) Start=n;
			delete h;
			changed(1);
		}
		h=n;
		if (h==Markend) break;
	}
	unmarkall();
	Top=top(Cursor);
	textcritical.leave();
	W->update();
}

void Text::execute (char *s)
{	
	textcritical.enter("execute");
	CaretUpdate=1;
	Cursor->copy(">"); 
	Cursor->cat(s); 
	Cursorpos=1;
	textcritical.leave();
	W->update();
	computation.resume();
}

void Text::execute ()
{	
	computation.resume();
	changed(1);
}

void Text::set (char *s, int pos)
{	
	textcritical.enter("set");
	CaretUpdate=1;
	Cursor->copy(">"); Cursor->cat(s); Cursorpos=pos;
	W->update();
	textcritical.leave();
}

void Text::setinput (char *s, int pos)
{	
	textcritical.enter("set input");
	CaretUpdate=1;
	if (Cursor->length()<lefteditlimit)
	{	set(s,pos); textcritical.leave(); return;
	}
	*(Cursor->text()+lefteditlimit)=0; 
	if (pos+lefteditlimit>(int)strlen(s)) pos=strlen(s)-lefteditlimit+1;
	Cursor->cat(s); Cursorpos=pos+lefteditlimit;
	W->update();
	textcritical.leave();
}

void Text::deleteudf ()
{   
	textcritical.enter("delete udf");
	CaretUpdate=1;
	if (!waiting || !editing || Cursor->type()!=TextLine::udf)
	{	textcritical.leave(); return; }
	if (!Cursor->next() || Cursor->next()->type()!=TextLine::udf)
	{	textcritical.leave(); return; }
	TextLine *h=Cursor->next();
	if (Cursor==Start) Start=h;
	delete Cursor;
	Cursor=h;
	Top=top(Cursor);
	changed(1);
	textcritical.leave();
	W->update();
}

TextLine* LastUndo = 0;

/**
This is called by Alt-Back and adds the current
command to the undo buffer, removing it from the
notebook.
*/
void Text::deletecommand ()
{   
	if (Cursor != LastUndo)
		UndoBuffer.clear();

	textcritical.enter("delete command");

	CaretUpdate=1;

	if (Cursor->type()==TextLine::udf)
	{	
		if (udf==2)
		{
			UndoBuffer.cat(Cursor->text());
			UndoBuffer.cat("\n");
		}
		deleteudf(); textcritical.leave(); return;
	}

	if (!waiting || !editing || Cursor->type()!=TextLine::prompt)
	{	textcritical.leave(); return; }

	TextLine *l=Cursor,*h,*k;

	// Go back in multi-line commands
	while (l->type()==TextLine::prompt && l->prev() && l->prev()->type()==TextLine::prompt && l->prev()->endsWith(" ..."))
		l=l->prev();

	// find all comment that belongs to this command:
	while (l->prev() && l->prev()->type()==TextLine::comment)
		l=l->prev();
	
	k=l->prev(); // last line of previous commands or 0

	// delete all that comment:
	while (l->type()==TextLine::comment)
	{   
		if (Start==l) Start=l->next(); // do not lose the lines!
		h=l->next();
		if (!l->Bitmap)
		{
			if (l->newline)	UndoBuffer.cat("%");
			UndoBuffer.cat("% ");
			if (l->heading) UndoBuffer.cat("* ");
			UndoBuffer.cat(l->text());
			UndoBuffer.cat("\n");
		}
		delete l;
		l=h;
	}

	// delete the command itself:
	while (true)
	{
		int cont=(l->type()==TextLine::prompt && l->endsWith(" ...") && l->next() && l->next()->type()==TextLine::prompt);
		if (Start==l) Start=l->next();
		h=l->next();
		UndoBuffer.cat(l->text());
		UndoBuffer.cat("\n");
		delete l;
		l=h;
		if (!cont) break;
	}
	
	// delete all output after the command:
	while (l && (l->type()==TextLine::output ||
		l->type()==TextLine::udf))
	{   
		if (Start==l) Start=l->next();
		h=l->next();
		char c=*(l->text());
		if (!l->Bitmap)
		{
			if (l->type()==TextLine::output && 
				(c=='%' || c=='$' || c=='*')) 
				UndoBuffer.cat(" ");
			UndoBuffer.cat(l->text());
			UndoBuffer.cat("\n");
		}
		delete l;
		l=h;
	}

	// set the cursor:
	if (!l)
	{	if (k)
		{	TextLine *kp=k;
			while (kp && kp->type()!=TextLine::prompt) kp=kp->prev();
			if (kp) Cursor=kp;
			else Cursor=new TextLine(this,k,0,">");
		}
		else Start=Cursor=new TextLine(this,0,0,">");
	}
	else
	{   k=l;
		while (k && k->type()!=TextLine::prompt)
		{	k=k->next();
		}
		if (k) Cursor=k;
		else
		{	if (l==Start)
			{	Start=Cursor=new TextLine(this,0,l,">");
			}
			else Cursor=new TextLine(this,l->prev(),l,">");
		}
	}
	Cursor->type(TextLine::prompt);
	Cursorpos=1;
	Top=top(Cursor);
	LastUndo = Cursor;
	changed(1);
	textcritical.leave();
	W->update();
}

void Text::undodelete ()
{   
	CaretUpdate=1;
	if (!editing || !waiting) return;

	if (Cursor->type()==TextLine::prompt)
	{
		textcritical.enter("undo delete");
		TextLine *l=Cursor;
		while (l->prev() && l->prev()->type()==TextLine::comment) l=l->prev();
		char *s=UndoBuffer.text();
		if (*s==0) { textcritical.leave(); return; }
		char *p=s;
		while (1)
		{	while (*p && *p!='\n') p++;
			char c=*p;
			*p=0;
			TextLine *h=new TextLine(this,l->prev(),l);
			if (Start==l) Start=h;
			if (*s=='%' && *(s+1)=='%')
			{	h->copy(s+1);
				h->newline=1;
			}
			else h->copy(s);
			if (*s=='>')
			{	h->type(TextLine::prompt);
				Cursor=h;
			}
			else if (*s=='%')
			{	h->type(TextLine::comment);
				h->clearcomment();
			}
			else if (*s=='$')
			{	h->type(TextLine::udf);
			}
			*p=c;
			if (!*p) break;
			p++;
			if (!*p) break;
			s=p;
		}
		Top=top(Cursor);
		changed(1);
		textcritical.leave();
	}
	else if (udf==2)
	{
		textcritical.enter("undo delete");
		TextLine *l=Cursor;
		char *s=UndoBuffer.text();
		if (*s==0) { textcritical.leave(); return; }
		char *p=s;
		while (*s=='$')
		{	
			while (*p && *p!='\n') p++;
			char c=*p;
			*p=0;
			TextLine *h=new TextLine(this,l->prev(),l);
			if (Start==l) Start=h;
			h->copy(s);
			h->type(TextLine::udf);
			*p=c;
			if (!*p) break;
			p++;
			if (!*p) break;
			s=p;
		}
		Top=top(Cursor);
		changed(1);
		textcritical.leave();
	}
	W->update();
	UndoBuffer.clear();
}

void Text::undo ()
{	
	textcritical.enter("undo");
	Cursor->restore();
	Cursorpos=1;
	changed(1);
	textcritical.leave();
}

void Text::redo ()
{	
	textcritical.enter("undo");
	Cursor->unrestore();
	Cursorpos=1;
	changed(1);
	textcritical.leave();
}

void Text::backup ()
{	
	textcritical.enter("backup");
	Cursor->backup();
	textcritical.leave();
}

void Text::insertcommand ()
	// insert a new command line or udf line in front of the current one.
{	
	textcritical.enter("insert command");
	CaretUpdate=1;
	int type=Cursor->type();
	if (!waiting || !editing || (
		type!=TextLine::prompt && type!=TextLine::udf))
	{	
		textcritical.leave(); return; 
	}
	TextLine *l=Cursor;
	while (l->prev() && l->prev()->type()==TextLine::comment)
		l=l->prev();
	Cursor=new TextLine(this,l->prev(),l);
	if (Start==l) Start=Cursor;
	if (type==TextLine::prompt)
	{	
		Cursor->copy(">"); Cursor->type(TextLine::prompt);
	}
	else
	{	
		Cursor->copy("$"); Cursor->type(TextLine::udf);
	}
	Cursorpos=1;
	Top=top(Cursor);
	textcritical.leave();
	W->update();
	changed(1);
}

void Text::copytext ()
{	
	if (!waiting) return;
	if (Markstart==0)
	{
		TextLine *h=Start;
		while (h)
		{	
			if (h->bitmapmark() && h->Bitmap)
			{
				Clipboard c(*graphicswindow);
				BitmapPS *m=new BitmapPS(*graphicswindow,
					h->Bitmap->width(),h->Bitmap->height());
				h->Bitmap->copy(*m,0,0);
				c.copy((HBITMAP)m->bitmaphandle());
				textwindow.setstatus(">>> Bitmap copied to clipboard!");
				return;
			}	
			h=h->next();
		}
		return;
	}
	textcritical.enter("copy text");
	int l=1;
	TextLine *h=Start;
	while (h)
	{	
		if (h->mark()) l+=strlen(h->text())+12;
		h=h->next();
	}
	char *s=new char[l],*p;
	p=s;
	h=Start;
	while (h)
	{	
		if (h->bitmapmark() && h->Bitmap)
		{
			Clipboard c(*graphicswindow);
			c.copy((HBITMAP)h->Bitmap->bitmaphandle());
			delete s;
			textcritical.leave();
			return;
		}
		if (h->mark())
		{	
			if (h==Markstart)
			{   if (h==Markend)
				{	memcpy(p,h->text()+Markstartpos,
						Markendpos-Markstartpos);
					p+=Markendpos-Markstartpos;
				}
				else
				{	strcpy(p,h->markertext());
					strcat(p,h->puretext()+
						max(0,Markstartpos-(h->puretext()-h->text()))); 
					p+=strlen(p);
				}
			}
			else if (h==Markend)
			{	strcpy(p,h->markertext());
				strcat(p,h->puretext()); 
				p+=strlen(p);
				p-=max(0,strlen(h->text())-Markendpos);
			}
			else if (!h->Bitmap)
			{	strcpy(p,h->markertext());
				strcat(p,h->puretext()); p+=strlen(p);
			}
			if (!h->Bitmap)
			{	*p++=13; *p++=10;
			}
		}
		h=h->next();
	}
	*p=0;
	Clipboard c(*W);
	c.copy(s);
	textwindow.setstatus(">>> Text copied to clipboard!");
	delete s;
	textcritical.leave();
}

char * Text::getMarkedWord (char *line)
{
	*line=0;
	if (Markstart==0) return line;
	if (Markstart->Bitmap) return line;
	char *h1=Markstart->text()+Markstartpos;
	if (Markend==Markstart)
	{
		int length=Markendpos-Markstartpos;
		strncpy(line,h1,length);
		line[length]=0;
	}
	else strcpy(line,h1);
	return line;
}

void Text::copycommands ()
{	
	if (!waiting || Markstart==0) return;
	textcritical.enter("copy commands");
	int l=1;
	TextLine *h=Start;
	while (h)
	{	if (h->mark()) l+=strlen(h->text())+4;
		h=h->next();
	}
	char *s=new char[l],*p;
	p=s;
	h=Start;
	while (h)
	{	
		if (h->mark())
		{	
			if (h==Markstart)
			{   if (h==Markend)
				{	memcpy(p,h->text()+Markstartpos,
						Markendpos-Markstartpos);
					p+=Markendpos-Markstartpos;
					*p++=13; *p++=10;
				}
				else
				{	if (h->type()==TextLine::udf || h->type()==TextLine::prompt)
					{	strcpy(p,h->puretext()+
							max(0,Markstartpos-(h->puretext()-h->text()))); 
						p+=strlen(p);
						*p++=13; *p++=10;
					}
				}
			}
			else if (h==Markend)
			{	if (h->type()==TextLine::udf || h->type()==TextLine::prompt)
				{	strcpy(p,h->puretext()); 
					p+=strlen(p);
					p-=max(0,strlen(h->text())-Markendpos);
					*p++=13; *p++=10;
				}
			}
			else
			{	if (h->type()==TextLine::udf)
				{	strcpy(p,"$ "); strcat(p,h->puretext()); p+=strlen(p);
					*p++=13; *p++=10;
				}
				else if (h->type()==TextLine::prompt)
				{	strcpy(p,h->puretext()); p+=strlen(p);
					*p++=13; *p++=10;
				}
			}
		}
		h=h->next();
	}
	*p=0;
	Clipboard c(*W);
	c.copy(s);
	delete s;
	textcritical.leave();
}

void Text::copyformatted ()
{	
	if (!waiting || Markstart==0) return;
	textcritical.enter("copy formatted");
	int l=1;
	TextLine *h=Start;
	while (h)
	{	
		if (h->mark()) l+=strlen(h->text())+8;
		h=h->next();
	}
	char *s=new char[l],*p;
	p=s;
	h=Start;
	while (h)
	{	
		if (h->mark())
		{	
			if (h==Markstart)
			{  
				if (h==Markend)
				{	
					memcpy(p,h->text()+Markstartpos,
						Markendpos-Markstartpos);
					p+=Markendpos-Markstartpos;
				}
				else
				{	
					strcpy(p,h->formattedtext());
					strcat(p,h->puretext()+
						max(0,Markstartpos-(h->puretext()-h->text()))); 
					p+=strlen(p);
				}
			}
			else if (h==Markend)
			{	
				strcpy(p,h->formattedtext());
				strcat(p,h->puretext()); 
				p+=strlen(p);
				p-=max(0,strlen(h->text())-Markendpos);
			}
			else if (!h->Bitmap)
			{	
				strcpy(p,h->formattedtext());
				strcat(p,h->puretext()); p+=strlen(p);
			}
			if (!h->Bitmap)
			{	
				*p++=13; *p++=10;
			}
		}
		h=h->next();
	}
	*p=0;
	Clipboard c(*W);
	c.copy(s);
	delete s;
	textcritical.leave();
}

void strcrepl (char *s, char c1, char c2)
{
	while (*s)
	{
		if (*s==c1) *s=c2;
		s++;
	}
}

void Text::pastecommands ()
{   
	if (!editing) return;
	clearquestion();
	textcritical.enter("paste commands");

	unmarkall();
	Clipboard c(*W);
	char *s=c.gettext();
	if (*s==0) { textcritical.leave(); return; }
	char *p=s,ch;
	while (*p && *p!=13) p++;
	if (strlen(p)<3)
	{	
		*p=0;
		strcrepl(s,9,',');
		Cursor->insert(Cursorpos,s);
	}
	else if (udf!=1)
	{   
		TextLine *current=0;
		TextLine *h=Cursor,*hn=0;
		h=h->prev();
		while (h && h->type()==TextLine::comment)
		{	
			h=h->prev();
		}
		while (1)
		{   
			if (strstarts(s," >")) s++;
			else if (strstarts(s,"  >")) s+=2;
			else if (strstarts(s," $")) s++;
			else if (strstarts(s,"  $")) s+=2;
			ch=*p;
			*p=0;
			int type=(h==0)?TextLine::prompt:h->type();
			if (*s=='$')
			{	s++;
				if (type==TextLine::udf || type==TextLine::prompt)
				{	if (h==0) Start=hn=new TextLine(this,h,Start,"$");
					else hn=new TextLine(this,h,h->next(),"$");
					hn->type(TextLine::udf);
					hn->cat(s);
					h=hn;
				}
			}
			else if (udf==0)
			{
				if (*s=='%')
				{	s++;
					int newline=0;
					if (*s=='%') { newline=1; s++; }
					else if (*s==' ') s++;
					if (type==TextLine::udf || type==TextLine::comment
						|| type==TextLine::output || type==TextLine::prompt)
					{	if (h==0) Start=hn=new TextLine(this,h,Start);
						else hn=new TextLine(this,h,h->next());
						hn->type(TextLine::comment);
						if (!strncmp(s,"* ",2))
						{
							s+=2; hn->heading=1;
						}
						hn->cat(s);
						if (newline) hn->newline=1;
						h=hn;
						h->makeutf();
					}
				}
				else if (*s=='>')
				{	
					s++;
					if (type==TextLine::udf || type==TextLine::comment
						|| type==TextLine::output || type==TextLine::prompt)
					{	if (h==0) Start=hn=new TextLine(this,h,Start,">");
						else hn=new TextLine(this,h,h->next(),">");
						hn->type(TextLine::prompt);
						hn->cat(s);
						h=hn;
					}
					if (!current) current=hn;
				}
				else
				{	
					if (*s==' ') s++;
					if (type==TextLine::output || type==TextLine::prompt)
					{	
						if (h==0) Start=hn=new TextLine(this,h,Start);
						else hn=new TextLine(this,h,h->next());
						hn->type(TextLine::output);
						hn->cat(s);
						h=hn;
					}
				}
			}
			*p=ch; if (*p==0) break;
			p++;
			if (*p!=10) break;
			p++;
			s=p;
			while (*p!=0 && *p!=13) p++;
		}
		if (h->type()!=TextLine::prompt && h->next()==0)
		{	
			hn=new TextLine(this,h,h->next(),">");
			hn->type(TextLine::prompt);
			hn->cat("");
			h=hn;
		}
		if (current)
		{	
			Cursor=current;
		}
	}
	showcursorline();
	textcritical.leave();
	W->update();
	changed(1);
}

void Text::pasteascommands ()
{   
	if (!editing) return;
	textcritical.enter("paste as commands");
	int inudf=Cursor->type()!=TextLine::prompt;
	unmarkall();
	Clipboard c(*W);
	char *s=c.gettext();
	if (*s==0) { textcritical.leave(); return; }
	char *p=s,ch;
	while (*p && *p!=13) p++;
	if (strlen(p)<3)
	{	
		*p=0;
		strcrepl(s,9,',');
		Cursor->insert(Cursorpos,s);
	}
	else if (!inudf)
	{   
		TextLine *current=0;
		TextLine *h=Cursor,*hn=0;
		h=h->prev();
		while (h && h->type()==TextLine::comment)
		{	
			h=h->prev();
		}
		while (1)
		{   
			ch=*p;
			*p=0;
			int type=(h==0)?TextLine::prompt:h->type();
			char *ps=s;
			while (*ps==' ') ps++;
			if (*ps=='>') s=ps;
			if (*s=='>') s++;
			if (type==TextLine::udf || type==TextLine::comment
				|| type==TextLine::output || type==TextLine::prompt)
			{	
				if (h==0) Start=hn=new TextLine(this,h,Start,">");
				else hn=new TextLine(this,h,h->next(),">");
				hn->type(TextLine::prompt);
				strcrepl(s,9,',');
				hn->cat(s);
				h=hn;
			}
			if (!current) current=hn;
			*p=ch; if (*p==0) break;
			p++;
			if (*p!=10) break;
			p++;
			s=p;
			while (*p!=0 && *p!=13 && *p!=10) p++;
		}
		if (h->type()!=TextLine::prompt && h->next()==0)
		{	
			hn=new TextLine(this,h,h->next(),">");
			hn->type(TextLine::prompt);
			hn->cat("");
			h=hn;
		}
		if (current)
		{	Cursor=current;
		}
	}
	showcursorline();
	textcritical.leave();
	W->update();
	changed(1);
}

int Text::countlines ()
{	
	textcritical.enter("count lines");
	TextLine *l=Start;
	int count=0;
	while (l)
	{	if (l->type()==TextLine::prompt) count++;
		l=l->next();
	}
	textcritical.leave();
	return count;
}

void Text::search (char *s)
{	
	textcritical.enter("search");
	TextLine *l=Start;
	while (l)
	{	int n=l->find(s);
		if (n>=0)
		{	Markstart=Markend=l;
			Markstartpos=n; Markendpos=n+strlen(s);
			l->mark(1);
			for (int i=0; i<5; i++)
			{	if (l->prev()) l=l->prev();
			}
			Top=l;
			W->update();
			textcritical.leave();
			return;
		}
		l=l->next();
	}
	textcritical.leave();
}

void Text::searchnext (char *s)
{	
	textcritical.enter("search next");
	TextLine *l=Cursor;
	if (Markstart) l=Markstart->next();
	unmarkall();
	while (l)
	{	int n=l->find(s);
		if (n>=0)
		{	Markstart=Markend=l;
			Markstartpos=n; Markendpos=n+strlen(s);
			l->mark(1);
			for (int i=0; i<5; i++)
			{	if (l->prev()) l=l->prev();
			}
			Top=l;
			textcritical.leave();
			W->update();
			return;
		}
		l=l->next();
	}
	textcritical.leave();
}

void Text::spellcheck ()
{
	textcritical.enter("spellcheck");
	TextLine *l=Start;
	while (l)
	{
		if (l->type()==TextLine::comment && !l->hidden())
		{
			l->clearmisspelled();
			char *s0=l->displaytext();
			if (l->getoriginal()) s0=l->getoriginal();
			char word[32];
			char *s=s0;
			int correct=0;
			while (*s)
			{
				if (*s=='\"')
				{
					s++;
					while (*s && *s!='\"') s++;
					if (*s=='\"') s++;
					continue;
				}
				if (*s=='&')
				{
					s++; correct++;
					while (*s && *s!=';') { correct++; s++; }
					if (*s==';') s++;
					continue;
				}
				if (xisalpha(*s) && (s==s0 || *(s-1)==' '))
				{
					char *s1=s;
					char *p=word;
					*p++=*s++;
					while (*s && p-word<31 && xisalpha(*s))
					{
						*p++=*s++;
					}
					*p++=0;
					if ((*s=='.' || *s==',' || *s==';' || *s==' ' || *s==0) &&
						strlen(word)>3 && *s!='(' && !isdigit(*s) && !findword(word))
					{
						l->addmisspelled(s1-s0-correct,strlen(word));
					}
					continue;
				}
				s++;
			}
		}
		l=l->next();
	}
	textcritical.leave();
	W->update();
}

void Text::addwords (char *file)
{
	if (!Markstart) return;
	FILE *out=fopen(file,"a");
	if (!out) DumpWarning("Could not open\n%s","Euler",file);
	textcritical.enter("spellcheck");
	TextLine *l=Start;
	int inmark;
	while (l)
	{
		if (l==Markstart) inmark=1;
		if (inmark && l->type()==TextLine::comment && !l->hidden())
		{
			l->clearmisspelled();
			char *s0=l->displaytext();
			char word[32];
			char *s=s0;
			while (*s)
			{
				if (*s=='\"')
				{
					s++;
					while (*s && *s!='\"') s++;
					if (*s=='\"') s++;
					continue;
				}
				if (xisalpha(*s))
				{
					char *p=word;
					*p++=*s++;
					while (*s && p-word<31 && xisalpha(*s))
					{
						*p++=*s++;
					}
					*p++=0;
					if (strlen(word)>3 && *s!='(' && !isdigit(*s) && !findword(word))
					{
						fprintf(out,"%s\n",word);
						addword(word);
						sortwords();
					}
					continue;
				}
				s++;
			}
		}
		if (l==Markend) break;
		l=l->next();
	}
	textcritical.leave();
	fclose(out);
	spellcheck();
	W->update();
}

void Text::unmarkspellerrors ()
{
	textcritical.enter("spellcheck");
	TextLine *l=Start;
	while (l)
	{
		l->clearmisspelled();
		l=l->next();
	}
	textcritical.leave();
	W->update();
}

int Text::isAfterBreak (TextLine *l)
{
	if (l->type()!=TextLine::prompt) return false;
	if (!l) return true;
	if (l->length()<2) return true;
	while (l->prev() && l->prev()->type()==TextLine::comment) l=l->prev();
	if (!l) return false;
	if (l->type()!=TextLine::comment) return false;
	if (l->length()==0) return true;
	if (l->heading) return true;
	if (!strncmp(l->text(),"---",3)) return true;
	return false;
}

int Text::isBeforeBreak (TextLine *l)
{
	if (!l) return true;
	if (l->length()<2) return true;
	if (!strncmp(l->text()+1,"//",2)) return true;
	while (l->next() && l->next()->type()!=TextLine::comment) 
	{
		l=l->next();
		if (l->type()==TextLine::prompt) return false;
	}
	if (!l) return false;
	if (l->type()!=TextLine::comment) return false;
	if (l->length()==0) return true;
	if (l->heading) return true;
	if (!strncmp(l->text(),"---",3)) return true;
	return false;
}

int Text::isbreak (TextLine *l)
{
	if (!l) return true;
	if (l->length()<2) return true;
	if (!strncmp(l->text()+1,"//",2)) return true;
	return false;
}

extern int helpintitle;
extern String WindowTitle;
void setwindowname ();

void Text::changed (int i)
{
	Changed=i;
	if (!helpintitle) setwindowname();
}

void Text::clearudfinput ()
{
	TextLine *next=Cursor->next();
	if (Cursor->length()==0 && next) 
	{
		TextLine *old=Cursor;
		Cursor=next;
		delete old;
	}
	while (Cursor->next() && Cursor->next()->type()==TextLine::udf)
	{
		Cursor=Cursor->next();
	}
	insertline();
	Cursorpos=0;
}
