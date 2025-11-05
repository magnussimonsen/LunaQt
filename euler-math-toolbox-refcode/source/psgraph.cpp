/*************************************************************************
 *	PSGraph
 *
 *	ps output for euler meta file
 *
 *	Eric Boucharé, 08/08/1999
 *
 *************************************************************************/

#include "psgraph.h"
#include <time.h>
#include <string.h>

// The 16 Euler colors
extern int red[16];
extern int green[16];
extern int blue[16];

/*****************************************************************************
 *
 *	postscript device methods
 *
 *****************************************************************************/


/*************
 *
 *	color selection postscript macros
 *
 *************/

//	set color (0 =< num =< 15)
//	num sc
static char setcolormacro[] = "/sc {\n\
	/i exch def\n\
	/idx i 3 mul def\n\
	colors idx get colors idx 1 add get colors idx 2 add get setrgbcolor\n\
} def\n";

//	set hue color (0 < hue < 1)
//	color hue shc
static char sethuecolormacro[] = "/shc {\n\
	/hue exch def\n\
	/i exch def\n\
	/idx i 3 mul def\n\
	/r colors idx get 255 mul def\n\
	/g colors idx 1 add get 255 mul def\n\
	/b colors idx 2 add get 255 mul def\n\
	r 2 div 128 add 2 mul hue mul 255 div\n\
	g 2 div 128 add 2 mul hue mul 255 div\n\
	b 2 div 128 add 2 mul hue mul 255 div\n\
	setrgbcolor\n\
} def\n";

/*************
 *
 *	clipping postscript macros
 *
 *************/
//	rectangle clipping
//	x1 y1 x2 y2 setclip
static char setclipmacro[]= "/setclip {\n\
gsave\n\
	/y2 exch def\n\
	/x2 exch def\n\
	/y1 exch def\n\
	/x1 exch def\n\
	newpath x1 y1 moveto x2 y1 lineto x2 y2 lineto x1 y2 lineto closepath clip\n\
} def\n";

/*************
 *
 *	line postscript macros
 *
 *************/

//	solid line postscript macro
//	[x1 y1 x2 y2 ... ] color width l
static char linemacro[]= "/l {\n\
gsave\n\
	/w exch def\n\
	/c exch def\n\
	/pts exch def\n\
	c sc\n\
	w setlinewidth\n\
	newpath\n\
	pts 0 get pts 1 get moveto\n\
	2 2 pts length 1 sub\n\
	{\n\
		/i exch def\n\
		pts i get pts i 1 add get lineto\n\
	} for\n\
	stroke\n\
grestore\n\
} def";

//	dashed line postscript macro
//	[ x1 y1 x2 y2 ... ] color width ld
static char dashedlinemacro[] = "/ld {\n\
gsave\n\
    /dash exch def\n\
	/w exch def\n\
	/c exch def\n\
	/pts exch def\n\
	c sc\n\
	w setlinewidth\n\
	dash 0 setdash newpath\n\
	pts 0 get pts 1 get moveto\n\
	2 2 pts length 1 sub\n\
	{\n\
		/i exch def\n\
		pts i get pts i 1 add get lineto\n\
	} for\n\
	stroke [] 0 setdash\n\
grestore\n\
} def";

//	arrow line postscript macro (arrow at the end of the line)
//	[ x1 y1 x2 y2 ... ] color width ratio la
static char arrowlinemacro[] = "/sin20 20 sin def\n\
/drawarrow {\n\
	/ratio exch def\n\
	/y2 exch def\n\
	/x2 exch def\n\
	/y1 exch def\n\
	/x1 exch def\n\
	/dx x2 x1 sub  def\n\
	/dy y2 y1 sub  def\n\
	/norme dx dx mul dy dy mul add sqrt def\n\
	/cs dx norme div def\n\
	/sn dy norme div def\n\
	/ah 10 ratio mul def\n\
	newpath x1 y1 moveto x2 ah cs mul sub y2 ah sn mul sub lineto stroke\n\
	norme 1.e-8 ge\n\
	{\n\
		/k0 x1 norme cs mul add def\n\
		/k1 y1 norme sn mul add def\n\
		newpath x2 y2 moveto\n\
		k0 ah cs sn sin20 mul add mul sub k1 ah sn cs sin20 mul sub mul sub lineto\n\
		k0 ah cs sn sin20 mul sub mul sub k1 ah sn cs sin20 mul add mul sub lineto\n\
		closepath\n\
		fill\n\
	} {} ifelse\n\
} def\n\
/la {\n\
gsave\n\
	/r exch def\n\
	/w exch def\n\
	/c exch def\n\
	/pts exch def\n\
	c sc\n\
	w setlinewidth\n\
	newpath\n\
	0 2 pts length 3 sub\n\
	{\n\
		/i exch def\n\
		pts i get pts i 1 add get pts i 2 add get pts i 3 add get r drawarrow\n\
	} for\n\
	stroke\n\
grestore\n\
} def";

/*************
 *
 *	marker postscript macros
 *
 *************/

//	cross marker postscript macro
//	x y msize color mcrs
static char markercrossmacro[] = "/mcrs {\n\
gsave\n\
	/c exch def\n\
	/ms exch def\n\
	/y exch def\n\
	/x exch def\n\
	c sc\n\
	newpath\n\
	x ms sub y ms sub moveto\n\
	ms 2 mul ms 2 mul rlineto\n\
	x ms sub y ms add moveto\n\
	ms 2 mul ms 2 mul neg rlineto\n\
	stroke\n\
grestore\n\
} def";

//	circle marker postscript macro
//	x y msize color mcir
static char markercirclemacro[] = "/mcir {\n\
gsave\n\
	/c exch def\n\
	/ms exch def\n\
	/y exch def\n\
	/x exch def\n\
	c sc\n\
	newpath\n\
	x y ms 0 360 arc\n\
	stroke\n\
grestore\n\
} def\n";

//	diamond marker postscript macro
//	x y msize color mdia
static char markerdiamondmacro[] = "/mdia {\n\
gsave\n\
	/c exch def\n\
	/ms exch def\n\
	/y exch def\n\
	/x exch def\n\
	c sc\n\
	newpath\n\
	x ms sub y moveto\n\
	ms ms rlineto\n\
	ms ms neg rlineto\n\
	ms neg ms neg rlineto\n\
	closepath\n\
	stroke\n\
grestore\n\
} def\n";

//	dot marker postscript macro
//	x y msize color mdot
static char markerdotmacro[] = "/mdot {\n\
gsave\n\
	/c exch def\n\
	/y exch def\n\
	/x exch def\n\
	c sc\n\
	newpath x y moveto 1 0 rlineto stroke\n\
grestore\n\
} def\n";

//	plus marker postscript macro
//	x y msize color mpls
static char markerplusmacro[] = "/mpls {\n\
gsave\n\
	/c exch def\n\
	/ms exch def\n\
	/y exch def\n\
	/x exch def\n\
	c sc\n\
	newpath\n\
	x ms sub y moveto\n\
	ms 2 mul 0 rlineto\n\
	x y ms sub moveto\n\
	0 ms 2 mul rlineto\n\
	stroke\n\
grestore\n\
} def\n";

//	square marker postscript macro
//	x y msize color msqr
static char markersquaremacro[] = "/msqr {\n\
gsave\n\
	/c exch def\n\
	/ms exch def\n\
	/y exch def\n\
	/x exch def\n\
	c sc\n\
	newpath\n\
	x ms sub y ms add moveto\n\
	ms 2 mul 0 rlineto\n\
	0 ms 2 mul neg rlineto\n\
	ms 2 mul neg 0 rlineto\n\
	closepath\n\
	stroke\n\
grestore\n\
} def\n";

//	star marker postscript macro
//	x y msize color mstr
static char markerstarmacro[] = "/mstr {\n\
gsave\n\
	/c exch def\n\
	/ms exch def\n\
	/y exch def\n\
	/x exch def\n\
	c sc\n\
	newpath\n\
	x ms sub y ms sub moveto\n\
	ms 2 mul ms 2 mul rlineto\n\
	x ms sub y ms add moveto\n\
	ms 2 mul ms 2 mul neg rlineto\n\
	x ms sub y moveto\n\
	ms 2 mul 0 rlineto\n\
	x y ms sub moveto\n\
	0 ms 2 mul rlineto\n\
	stroke\n\
grestore\n\
} def\n";


/*************
 *
 *	bar postscript macros
 *
 *************/

//	bar pattern postscript macro
//	x y w h color hue bar
static char barmacro[] = "/bar {\n\
gsave\n\
	/hue exch def\n\
	/c  exch def\n\
	/h exch def\n\
	/w exch def\n\
	/y exch def\n\
	/x exch def\n\
	c 0 eq {\n\
		1.0 hue sub 255.0 mul 360.0 div 0.5 0.9 sethsbcolor\n\
	}{\n\
		c hue shc\n\
	} ifelse\n\
	newpath x y moveto w 0 rlineto 0 h neg rlineto w neg 0 rlineto closepath fill\n\
grestore\n\
} def\n";

//	framed bar pattern postscript macro
//	x y w h color hue fbar
static char fbarmacro[] = "/fbar {\n\
gsave\n\
	/hue exch def\n\
	/c  exch def\n\
	/h exch def\n\
	/w exch def\n\
	/y exch def\n\
	/x exch def\n\
	c 0 eq {\n\
		1.0 hue sub 255.0 mul 360.0 div 0.5 0.9 sethsbcolor\n\
	}{\n\
		c hue shc\n\
	} ifelse\n\
	newpath x y moveto w 0 rlineto 0 h neg rlineto w neg 0 rlineto closepath fill\n\
	0 0 0 setrgbcolor\n\
	newpath x y moveto w 0 rlineto 0 h neg rlineto w neg 0 rlineto closepath stroke\n\
grestore\n\
} def\n";


/*************
 *
 *	bar postscript macros
 *
 *************/

//	solid pattern colored bar postscript macro
//	x y w h color bar1
static char bar1macro[] = "/bar1 {\n\
gsave\n\
	/c  exch def\n\
	/h exch def\n\
	/w exch def\n\
	/y exch def\n\
	/x exch def\n\
	c 0 eq {\n\
		1.0 0.5 sub 255.0 mul 360.0 div 0.5 0.9 sethsbcolor\n\
	}{\n\
		c 0.5 shc\n\
	} ifelse\n\
	newpath x y moveto w 0 rlineto 0 h neg rlineto w neg 0 rlineto closepath fill\n\
grestore\n\
} def\n";

//	frame bar postscript macro
//	x y w h color fbar2
static char fbar2macro[] = "/fbar2 {\n\
gsave\n\
	/c  exch def\n\
	/h exch def\n\
	/w exch def\n\
	/y exch def\n\
	/x exch def\n\
	c sc\n\
	newpath x y moveto w 0 rlineto 0 h neg rlineto w neg 0 rlineto closepath stroke\n\
grestore\n\
} def\n";

//	framed bar postscript macro
//	x y w h color fbar1
static char fbar1macro[] = "/fbar1 {\n\
gsave\n\
	/c  exch def\n\
	/h exch def\n\
	/w exch def\n\
	/y exch def\n\
	/x exch def\n\
	c 0 eq {\n\
		1.0 0.5 sub 255.0 mul 360.0 div 0.5 0.9 sethsbcolor\n\
	}{\n\
		c 0.5 shc\n\
	} ifelse\n\
	newpath x y moveto w 0 rlineto 0 h neg rlineto w neg 0 rlineto closepath fill\n\
	1 sc\n\
	newpath x y moveto w 0 rlineto 0 h neg rlineto w neg 0 rlineto closepath stroke\n\
grestore\n\
} def\n";
	
//	diagonal 1 pattern bar postscript macro
//	x y w h color fbd1
static char fb1d1macro[] = "/fb1d1 {\n\
gsave\n\
	/c  exch def\n\
	/h exch def\n\
	/w exch def\n\
	/y exch def\n\
	/x exch def\n\
	c sc\n\
	/ww 1024 def\n\
	/hh 1024 def\n\
	/s 20 def\n\
	newpath x y moveto w 0 rlineto 0 h neg rlineto w neg 0 rlineto closepath clip\n\
	newpath\n\
	/nmax ww  hh add s div def\n\
	0 1 nmax {\n\
		/i exch def\n\
		0 i s mul ww sub moveto ww hh rlineto\n\
	} for stroke\n\
	newpath\n\
	1 sc\n\
	2 setlinewidth\n\
	x y moveto w 0 rlineto 0 h neg rlineto w neg 0 rlineto closepath stroke\n\
grestore\n\
} def\n";


//	diagonal2 pattern bar postscript macro
//	x y w h color fb1d2
static char fb1d2macro[] = "/fb1d2 {\n\
gsave\n\
	/c  exch def\n\
	/h exch def\n\
	/w exch def\n\
	/y exch def\n\
	/x exch def\n\
	c sc\n\
	/ww 1024 def\n\
	/hh 1024 def\n\
	/s 20 def\n\
	newpath x y moveto w 0 rlineto 0 h neg rlineto w neg 0 rlineto closepath clip\n\
	newpath\n\
	/nmax ww  hh add s div def\n\
	0 1 nmax {\n\
		/i exch def\n\
		0 i s mul moveto ww hh neg rlineto\n\
	} for stroke\n\
	newpath\n\
	1 sc\n\
	2 setlinewidth\n\
	x y moveto w 0 rlineto 0 h neg rlineto w neg 0 rlineto closepath\n\
	stroke\n\
grestore\n\
} def\n";

//	cross pattern bar postscript macro
//	x y w h color fb1c
static char fb1cmacro[] = "/fb1c {\n\
gsave\n\
	/c  exch def\n\
	/h exch def\n\
	/w exch def\n\
	/y exch def\n\
	/x exch def\n\
	c sc\n\
	/ww 1024 def\n\
	/hh 1024 def\n\
	/s 20 def\n\
	newpath x y moveto w 0 rlineto 0 h neg rlineto w neg 0 rlineto closepath clip\n\
	newpath\n\
	/nmax ww  hh add s div def\n\
	0 1 nmax {\n\
		/i exch def\n\
		0 i s mul ww sub moveto ww hh rlineto\n\
		0 i s mul moveto ww hh neg rlineto\n\
	} for stroke\n\
	newpath\n\
	1 sc\n\
	2 setlinewidth\n\
	x y moveto w 0 rlineto 0 h neg rlineto w neg 0 rlineto closepath\n\
	stroke\n\
grestore\n\
} def\n";


/*************
 *
 *	filled with hue color pattern postscript macros
 *
 *************/

//	filled and framed region (defined by 4 points)
//	x1 y1 x2 y2 x3 y3 x4 y4 color hue ffillh
static char ffillhmacro[] = "/ffillh {\n\
gsave\n\
	/h  exch def\n\
	/c  exch def\n\
	/y4 exch def\n\
	/x4 exch def\n\
	/y3 exch def\n\
	/x3 exch def\n\
	/y2 exch def\n\
	/x2 exch def\n\
	/y1 exch def\n\
	/x1 exch def\n\
	c 0 eq {\n\
		1.0 h sub 255.0 mul 360.0 div 0.5 0.9 sethsbcolor\n\
	}{\n\
		c h shc\n\
	} ifelse\n\
	newpath x1 y1 moveto x2 y2 lineto x3 y3 lineto x4 y4 lineto closepath fill\n\
	0 0 0 setrgbcolor\n\
	newpath x1 y1 moveto x2 y2 lineto x3 y3 lineto x4 y4 lineto closepath stroke\n\
grestore\n\
} def\n";

//	filled region (defined by 4 points)
//	x1 y1 x2 y2 x3 y3 x4 y4 color hue fillh
static char fillhmacro[] = "/fillh {\n\
gsave\n\
	/h  exch def\n\
	/c  exch def\n\
	/y4 exch def\n\
	/x4 exch def\n\
	/y3 exch def\n\
	/x3 exch def\n\
	/y2 exch def\n\
	/x2 exch def\n\
	/y1 exch def\n\
	/x1 exch def\n\
	c 0 eq {\n\
		1.0 h sub 255.0 mul 360.0 div 0.5 0.9 sethsbcolor\n\
	}{\n\
		c h shc\n\
	} ifelse\n\
	newpath x1 y1 moveto x2 y2 lineto x3 y3 lineto x4 y4 lineto closepath fill\n\
	newpath x1 y1 moveto x2 y2 lineto x3 y3 lineto x4 y4 lineto closepath stroke\n\
grestore\n\
} def\n";


/*************
 *
 *	arbitrary filled region postscript macro
 *
 *************/

//	arbitrary filled region macro
//	[x1 y1 x2 y2 ...] color pf\n");
static char fillmacro[] = "/pf {\n\
gsave\n\
	/c exch def\n\
	/pts exch def\n\
	c sc\n\
	newpath\n\
	pts 0 get pts 1 get moveto\n\
	3 3 pts length 1 sub\n\
	{\n\
		/i exch def\n\
		pts i get pts i 1 add get lineto\n\
	} for\n\
	closepath fill\n\
	newpath\n\
	1 sc\n\
	pts 0 get pts 1 get moveto\n\
	2 3 pts length 4 sub\n\
	{\n\
		/i exch def\n\
		pts i get 1 eq\n\
		{\n\
			pts i 1 add get pts i 2 add get lineto\n\
		} if\n\
	} for\n\
	pts pts length 1 sub get 1 eq\n\
	{\n\
		closepath\n\
	} if\n\
	stroke\n\
grestore\n\
} def\n";

/*************
 *
 *	string postscript macros
 *
 *************/
static char *encoding[] = {
"quotesinglbase",
"quotedblbase",
"ellipsis",
"OE",
"oe",
"quotedblleft",
"quotedblright",
"fi",
"fl",
"endash",
"emdash",
"bullet",
"dagger",
"daggerdbl",
"florin",
"fraction",
"dotlessi",
"grave",
"acute",
"circumflex",
"tilde",
"macron",
"breve",
"dotaccent",
"dieresis",
"-none-",
"ring",
"cedilla",
"-none-",
"hungarumlaut",
"ogonek",
"caron",
"space",
"exclamdown",
"cent",
"sterling",
"currency",
"yen",
"brokenbar",
"section",
"dieresis",
"copyright",
"ordfeminine",
"guillemotleft",
"logicalnot",
"hyphen",
"registered",
"macron",
"degree",
"plusminus",
"twosuperior",
"threesuperior",
"acute",
"mu",
"paragraph",
"periodcentered",
"cedilla",
"onesuperior",
"ordmasculine",
"guillemotright",
"onequarter",
"onehalf",
"threequarters",
"questiondown",
"Agrave",
"Aacute",
"Acircumflex",
"Atilde",
"Adieresis",
"Aring",
"AE",
"Ccedilla",
"Egrave",
"Eacute",
"Ecircumflex",
"Edieresis",
"Igrave",
"Iacute",
"Icircumflex",
"Idieresis",
"Eth",
"Ntilde",
"Ograve",
"Oacute",
"Ocircumflex",
"Otilde",
"Odieresis",
"multiply",
"Oslash",
"Ugrave",
"Uacute",
"Ucircumflex",
"Udieresis",
"Yacute",
"Thorn",
"germandbls",
"agrave",
"aacute",
"acircumflex",
"atilde",
"adieresis",
"aring",
"ae",
"ccedilla",
"egrave",
"eacute",
"ecircumflex",
"edieresis",
"igrave",
"iacute",
"icircumflex",
"idieresis",
"eth",
"ntilde",
"ograve",
"oacute",
"ocircumflex",
"otilde",
"odieresis",
"divide",
"oslash",
"ugrave",
"uacute",
"ucircumflex",
"udieresis",
"yacute",
"thorn",
"ydieresis"
};

//	ReEncode postscript macro to handle full Latin1 char set
//	
static char	reencodemacro[] = "/reencsmalldict 12 dict def\n\
/ReEncodeSmall\n\
{\n\
	reencsmalldict begin\n\
	/newcodesandnames exch def\n\
	/newfontname exch def\n\
	/basefontname exch def\n\
	/basefontdict basefontname findfont def\n\
	/newfont basefontdict maxlength dict def\n\
	basefontdict\n\
	{\n\
		exch dup /FID ne\n\
		{\n\
			dup /Encoding eq\n\
			{\n\
				exch dup length array copy\n\
				newfont 3 1 roll put\n\
			}{\n\
				exch newfont 3 1 roll put\n\
			} ifelse\n\
		}{\n\
			pop pop\n\
		} ifelse\n\
	} forall\n\
	newfont /FontName newfontname put\n\
	newcodesandnames aload pop\n\
	newcodesandnames length 2 idiv\n\
	{\n\
		newfont /Encoding get 3 1 roll put\n\
	} repeat\n\
	newfontname newfont definefont pop\n\
	end\n\
} def";

//	Left string postscript macro
//	x y angle (str) color Lstr
static char lstrmacro[] = "/Lstr {\n\
gsave\n\
	/c   exch def\n\
	/str exch def\n\
	/ang exch def\n\
	/y  exch def\n\
	/x  exch def\n\
	newpath c sc 0 0 moveto\n\
	x y translate ang rotate 0 fontsize neg moveto str show\n\
grestore\n\
} def";

//	Center string postscript macro
//	x y angle (str) color Cstr
static char cstrmacro[] = "/Cstr {\n\
gsave\n\
	/c   exch def\n\
	/str exch def\n\
	/ang exch def\n\
	/y  exch def\n\
	/x  exch def\n\
	str stringwidth pop\n\
	/width exch def\n\
	newpath c sc 0 0 moveto\n\
	x y translate ang rotate width neg 2 div fontsize neg moveto str show\n\
grestore\n\
} def";

//	Right string postscript macro
//	x y angle (str) color Rstr
static char rstrmacro[] = "/Rstr {\n\
gsave\n\
	/c   exch def\n\
	/str exch def\n\
	/ang exch def\n\
	/y  exch def\n\
	/x  exch def\n\
	str stringwidth pop\n\
	/width exch def\n\
	newpath c sc 0 0 moveto\n\
	x y translate ang rotate width neg fontsize neg moveto str show\n\
grestore\n\
} def";


//#define col(c) ((double)(((c)*getmetawidth())/1024.0))

//#define row(r) ((double)(((r)*getmetaheight())/1024.0))

//////////////////////////////////////////////////////////////

extern int usecolors;

PSGraph::PSGraph(FILE *output, int width, int height)
{
	out = output;
	pswidth = width;
	psheight = height;

	fillcolor1=11; fillcolor2=3;
	screenlines=30;
	usecolor=usecolors;

	clipped = 0;lineop=0;
	x=-1.0;y=-1.0;

}

void PSGraph::init ()
{
	int i;
	char	moment[26];
	time_t	tim, *p_tim;

	if (!out) return;
	
	/* postscript intro */

	firstuse.sethuecolor	= 1;
	firstuse.setclip		= 1;
	firstuse.linesolid		= 1;
	firstuse.linedotted		= 1;
	firstuse.linedashed		= 1;
	firstuse.linearrow		= 1;
	firstuse.markercross	= 1;
	firstuse.markercircle	= 1;
	firstuse.markerdiamond	= 1;
	firstuse.markerdot		= 1;
	firstuse.markerplus		= 1;
	firstuse.markersquare	= 1;
	firstuse.markerstar		= 1;
	firstuse.bar			= 1;
	firstuse.fbar			= 1;
	firstuse.bar1			= 1;
	firstuse.fbar1			= 1;
	firstuse.fbar2			= 1;
	firstuse.fb1d1			= 1;
	firstuse.fb1d2			= 1;
	firstuse.fb1c			= 1;
	firstuse.fill			= 1;
	firstuse.fillh			= 1;
	firstuse.ffillh			= 1;
	firstuse.reencode		= 1;
	firstuse.text			= 1;
	firstuse.rtext			= 1;
	firstuse.ctext			= 1;

	p_tim = &tim;
	tim = time(0);
	sprintf(moment,"%s",ctime(p_tim));
	moment[(strlen(moment)-1)] = 0;	/* enleve le '\n' final */
	fprintf(out,"%%!PS-Adobe-2.0 EPSF-2.0\n");
	fprintf(out,"%%%%BoundingBox: 0 0 %d %d\n",pswidth,psheight);
	fprintf(out,"%%%%CreationDate: %s\n",moment);
	fprintf(out,"%%%%Creator Euler\n");
	fprintf(out,"%%%%EndComments\n");
	
	// Color management		
	fprintf(out,"/colors [\n");
    for (i=0; i<16; i++)
		fprintf(out,"%g %g %g\n",color[i][0],color[i][1],color[i][2]);
	fprintf(out,"] def\n");

	fprintf(out,"%s\n",setcolormacro);
	
	fprintf(out,"%%%%Beginning of the flag page\n");
	fprintf(out,"gsave newpath\n");
	fprintf(out,"/Helvetica findfont %d scalefont setfont\n",psheight/screenlines);
	fprintf(out,"gsave\n	newpath 0 0 moveto (lq) true charpath pathbbox\n	/y2 exch def pop /y1 exch def pop\n	/fontsize y2 y1 sub def\ngrestore\n");
}

PSGraph::~PSGraph()
{
	if (lineop) lineend(lcolor,lstyle,lwidth);
	fprintf(out,"stroke grestore showpage\n");
}

void PSGraph::lineend(int color, int style, int width)
{	
	if (color>=0x01000000) color=0;
		
	switch (style)
	{
		case line_solid:
			fprintf(out,"] %d %d l\n",color,width);
			break;
		case line_dotted:
			fprintf(out,"] %d %d [ 2 2 ] ld\n",color,width);
			break;
		case line_dashdot:
			fprintf(out,"] %d %d [ 5 2 ] ld\n",color,width);
			break;
		case line_dashdotdash:
			fprintf(out,"] %d %d [ 5 2 5 ] ld\n",color,width);
			break;
		case line_dotdashdot:
			fprintf(out,"] %d %d [ 2 5 2 ] ld\n",color,width);
			break;
		case line_dashed:
			fprintf(out,"] %d %d [ 5 5 ] ld\n",color,width);
			break;
		case line_arrow:
			fprintf(out,"] %d %d %g la\n",color,width,psheight/1024.0);
			break;
	}
	lineop = 0;
	x = -1.0;
	y = -1.0;
}


void PSGraph::clear ()
{
	if (lineop) lineend(lcolor,lstyle,lwidth);
}

void PSGraph::clip (double c, double r, double c1, double r1)
{
	if (lineop) lineend(lcolor,lstyle,lwidth);

	if (!clipped) {
		if (firstuse.setclip) {
			fprintf(out,"%s",setclipmacro);
			firstuse.setclip = 0;
		}
		fprintf(out,"%g %g %g %g setclip\n",
			col(c),row(1024.0-r),col(c1),row(1024.0-r1));
		clipped = 1;
	} else {
		fprintf(out,"grestore\n");
		clipped = 0;
	}
}

void PSGraph::line (double c, double r, double c1, double r1, int color, int style, double width)
{
	if (color>=0x01000000) color=0;
		
	if ((lineop && (c!=x || r!=y || color != lcolor || style!=lstyle || width!=lwidth)) || (!lineop)) 
	{
		if (lineop) lineend(lcolor,lstyle,lwidth);
		
		switch (style) {
			case line_solid:
	 			if (firstuse.linesolid) {
					fprintf(out,"%s\n",linemacro);
					firstuse.linesolid = 0;
				}
				break;
			case line_dotted:
			case line_dashed:
				if (firstuse.linedotted) {
					fprintf(out,"%s\n",dashedlinemacro);
					firstuse.linedotted = 0;
				}
				break;
			case line_arrow:
				if (firstuse.linearrow) {
					fprintf(out,"%s\n",arrowlinemacro);
					firstuse.linearrow = 0;
				}
				break;
		}

		fprintf(out,"[ %g %g %g %g ",col(c),row(1024.0-r),col(c1),row(1024.0-r1));
		lcolor = color;
		lstyle = style;
		lwidth = (int)(width);
		x = c1;
		y = r1;
		lineop = 1;
	} 
	else 
	{
		fprintf(out,"%g %g ",col(c1),row(1024.0-r1));
		x = c1;y=r1;
	}
}

void PSGraph::path (double c[], int n, int color, int style, double width)
{
	for (int i=0; i<n-1; i++)
	{
		line(c[2*i],c[2*i+1],c[2*i+2],c[2*i+3],color,style,width);
	}
}


void PSGraph::marker (double c, double r, int color, int type, double size)
{
	double x=col(c);
	double y=row(1024.0-r);
	double ms = 10*psheight/1024.0;
	
	if (color>=0x01000000) color=0;
		
	if (lineop) lineend(lcolor,lstyle,lwidth);

	switch (type)
	{
		case marker_cross:
			if (firstuse.markercross) {
				fprintf(out,"%s\n",markercrossmacro);
				firstuse.markercross = 0;
			}
			fprintf(out,"%g %g %g %d mcrs\n",x,y,ms,color);
			break;
		case marker_circle:
			if (firstuse.markercircle) {
				fprintf(out,"%s\n",markercirclemacro);
				firstuse.markercircle = 0;
			}
			fprintf(out,"%g %g %g %d mcir\n",x,y,ms,color);
			break;
		case marker_diamond:
			if (firstuse.markerdiamond) {
				fprintf(out,"%s\n",markerdiamondmacro);
				firstuse.markerdiamond = 0;
			}
			fprintf(out,"%g %g %g %d mdia\n",x,y,ms,color);
			break;
		case marker_dot:
			if (firstuse.markerdot) {
				fprintf(out,"%s\n",markerdotmacro);
				firstuse.markerdot = 0;
			}
			fprintf(out,"%g %g %d mdot\n",x,y,color);
			break;
		case marker_plus:
			if (firstuse.markerplus) {
				fprintf(out,"%s\n",markerplusmacro);
				firstuse.markerplus = 0;
			}
			fprintf(out,"%g %g %g %d mpls\n",x,y,ms,color);
			break;
		case marker_square:
			if (firstuse.markersquare) {
				fprintf(out,"%s\n",markersquaremacro);
				firstuse.markersquare = 0;
			}
			fprintf(out,"%g %g %g %d msqr\n",x,y,ms,color);
			break;
		case marker_star:
			if (firstuse.markerstar) {
				fprintf(out,"%s\n",markerstarmacro);
				firstuse.markerstar = 0;
			}
			fprintf(out,"%g %g %g %d mstr\n",x,y,ms,color);
			break;
	}	
}

void PSGraph::bar (double c, double r, double c1, double r1, double hue, int color, int framed)
{
	double x=col(c);
	double y=row(1024.0-r);
	double w=col(c1-c);
	double h=row(r1-r);
	
	if (color>=0x01000000) color=0;
		
	if (lineop) lineend(lcolor,lstyle,lwidth);

	if (firstuse.sethuecolor) {
		fprintf(out,"%s\n",sethuecolormacro);
		firstuse.sethuecolor = 0;
	}
	
	switch (framed)
	{
		case 0:
			if (firstuse.bar)
			{
				fprintf(out,"%s\n",barmacro);
				firstuse.bar = 0;
			}
			fprintf(out,"%g %g %g %g %d %g bar\n",x,y,w,h,color,hue);
			break;
		default:
			if (firstuse.fbar)
			{
				fprintf(out,"%s\n",fbarmacro);
				firstuse.fbar = 0;
			}
			fprintf(out,"%g %g %g %g %d %g fbar\n",x,y,w,h,color,hue);
	}
}

void PSGraph::bar1 (double c, double r, double c1, double r1, int color, int style)
{
	double x=col(c);
	double y=row(1024.0-r);
	double w=col(c1-c);
	double h=row(r1-r);

	if (color>=0x01000000) color=0;
		
	if (lineop) lineend(lcolor,lstyle,lwidth);

	switch (style)
	{	case bar_solid :
			if (firstuse.bar1)
			{
				if (firstuse.sethuecolor)
				{
					fprintf(out,"%s\n",sethuecolormacro);
					firstuse.sethuecolor = 0;
				}
				fprintf(out,"%s\n",bar1macro);
				firstuse.bar1 = 0;
			}
			fprintf(out,"%g %g %g %g %d bar1\n",x,y,w,h,color);
			break;
		case bar_frame :
			if (firstuse.fbar2)
			{
				fprintf(out,"%s\n",fbar2macro);
				firstuse.fbar2 = 0;
			}
			fprintf(out,"%g %g %g %g %d fbar2\n",x,y,w,h,color);
			break;
		case bar_framed :
		case bar_hhatch :
		case bar_vhatch :
			if (firstuse.fbar1)
			{
				if (firstuse.sethuecolor)
				{
					fprintf(out,"%s\n",sethuecolormacro);
					firstuse.sethuecolor = 0;
				}
				fprintf(out,"%s\n",fbar1macro);
				firstuse.fbar1 = 0;
			}
			fprintf(out,"%g %g %g %g %d fbar1\n",x,y,w,h,color);
			break;
		case bar_diagonal1 :
			if (firstuse.fb1d1)
			{
				fprintf(out,"%s\n",fb1d1macro);
				firstuse.fb1d1 = 0;
			}
			fprintf(out,"%g %g %g %g %d fb1d1\n",x,y,w,h,color);
			break;
		case bar_diagonal2 :
			if (firstuse.fb1d2)
			{
				fprintf(out,"%s\n",fb1d2macro);
				firstuse.fb1d2 = 0;
			}
			fprintf(out,"%g %g %g %g %d fb1d2\n",x,y,w,h,color);
			break;
		case bar_cross :
			if (firstuse.fb1c)
			{
				fprintf(out,"%s\n",fb1cmacro);
				firstuse.fb1c = 0;
			}
			fprintf(out,"%g %g %g %g %d fb1c\n",x,y,w,h,color);
			break;
	}
}

void PSGraph::fill (double c[], int fillcolor, int wirecolor, int n, int connect[])
{
	double x, y;
	int i;

	if (lineop) lineend(lcolor,lstyle,lwidth);

	if (firstuse.fill)
	{
		fprintf(out,"%s\n",fillmacro);
		firstuse.fill = 0;
	}
	fprintf(out,"[ ");

	for (i=0; i<n; i++)
	{
		x=col(c[2*i]);
		y=row(1024.0-c[2*i+1]);
		fprintf(out,"%g %g %d ",x,y,connect[i]);
	}
	fprintf(out,"] %d pf\n",fillcolor);
}

void PSGraph::fillh (double c[], int n, double hue, int color, int connect)
{
	double x, y;
	int i;
	
	if (color>=0x01000000) color=0;
		
	if (lineop) lineend(lcolor,lstyle,lwidth);
	
	if (firstuse.sethuecolor)
	{
		fprintf(out,"%s\n",sethuecolormacro);
		firstuse.sethuecolor = 0;
	}

	if (connect)
	{
		if (firstuse.ffillh)
		{
			fprintf(out,"%s\n",ffillhmacro);
			firstuse.ffillh = 0;
		}
	}
	else
	{
		if (firstuse.fillh)
		{
			fprintf(out,"%s\n",fillhmacro);
			firstuse.fillh = 0;
		}
	}
	
	for (i=0; i<n; i++)
	{
		x=col(c[2*i]);
		y=row(1024.0-c[2*i+1]);
		fprintf(out,"%g %g ",x,y);
	}
	if (connect)
		fprintf(out,"%d %g ffillh\n",color,hue);
	else
		fprintf(out,"%d %g fillh\n",color,hue);
}

int PSGraph::scantext(char c)
{
	if (((unsigned)c)>127) {
		if (firstuse.reencode) {
			fprintf(out,"%s\n",reencodemacro);
			firstuse.reencode = 0;
		}
		return 1;
	}
	return 0;
}

void PSGraph::text (double c, double r, char *s, int color, int alignment)
{
	double x=col(c);
	double y=row(1024.0-r);
	char *t=s,ch;
	int beginreencode=0;
	
	if (color>=0x01000000) color=0;
		
	if (lineop) lineend(lcolor,lstyle,lwidth);
	
	while (*t)
	{
		ch =*t;
		if (!beginreencode)
		{
			beginreencode = scantext(*t);
			if (beginreencode)
			{
				fprintf(out,"gsave /scan-vec [\n8#%o /%s\n",256+ch,encoding[128+ch]);
			}
		}
		else if (scantext(*t))
		{
			fprintf(out,"8#%o /%s\n",256+ch,encoding[128+ch]);
		}
		t=t++;
	}
	if (beginreencode)
	{
		fprintf(out,"] def\n/Helvetica /Helvetica-8 scan-vec ReEncodeSmall\n/Helvetica-8 findfont %d scalefont setfont\n",psheight/screenlines);
	}
	
	switch (alignment)
	{
		case 0:
			if (firstuse.text)
			{
				fprintf(out,"%s\n",lstrmacro);
				firstuse.text=0;
			}
			fprintf(out,"%g %g 0 (%s) %d Lstr\n",x,y,s,color);
			break;
		case 1:
			if (firstuse.ctext)
			{
				fprintf(out,"%s\n",cstrmacro);
				firstuse.ctext=0;
			}
			fprintf(out,"%g %g 0 (%s) %d Cstr\n",x,y,s,color);
			break;
		case 2:
			if (firstuse.rtext)
			{
				fprintf(out,"%s\n",rstrmacro);
				firstuse.rtext=0;
			}
			fprintf(out,"%g %g 0 (%s) %d Rstr\n",x,y,s,color);
			break;
	}
	if (beginreencode) fprintf(out,"grestore\n");
}

void PSGraph::vtext (double c, double r, char *s, int color, int alignment)
{
	double x=col(c);
	double y=row(1024.0-r);
	char *t=s,ch;
	int beginreencode=0;
	
	if (color>=0x01000000) color=0;
		
	if (lineop) lineend(lcolor,lstyle,lwidth);

	while (*t)
	{
		ch =*t;
		if (!beginreencode)
		{
			beginreencode = scantext(*t);
			if (beginreencode)
			{
				fprintf(out,"gsave /scan-vec [\n8#%o /%s\n",256+ch,encoding[128+ch]);
			}
		}
		else if (scantext(*t))
		{
			fprintf(out,"8#%o /%s\n",256+ch,encoding[128+ch]);
		}
		t=t++;
	}
	if (beginreencode)
	{
		fprintf(out,"] def\n/Helvetica /Helvetica-8 scan-vec ReEncodeSmall\n/Helvetica-8 findfont %d scalefont setfont\n",psheight/screenlines);
	}

	switch (alignment)
	{
		case 0:
			if (firstuse.text)
			{
				fprintf(out,"%s\n",lstrmacro);
				firstuse.text=0;
			}
			fprintf(out,"%g %g -90 (%s) %d Lstr\n",x,y,s,color);
			break;
		case 1:
			if (firstuse.ctext)
			{
				fprintf(out,"%s\n",cstrmacro);
				firstuse.ctext=0;
			}
			fprintf(out,"%g %g -90 (%s) %d Cstr\n",x,y,s,color);
			break;
		case 2:
			if (firstuse.rtext)
			{
				fprintf(out,"%s\n",rstrmacro);
				firstuse.rtext=0;
			}
			fprintf(out,"%g %g -90 (%s) %d Rstr\n",x,y,s,color);
			break;
	}
	if (beginreencode) fprintf(out,"grestore\n");
}

void PSGraph::vutext (double c, double r, char *s, int color, int alignment)
{
	double x=col(c);
	double y=row(1024.0-r);
	char *t=s,ch;
	int beginreencode=0;
	
	if (color>=0x01000000) color=0;
		
	if (lineop) lineend(lcolor,lstyle,lwidth);

	while (*t)
	{
		ch =*t;
		if (!beginreencode)
		{
			beginreencode = scantext(*t);
			if (beginreencode)
			{
				fprintf(out,"gsave /scan-vec [\n8#%o /%s\n",256+ch,encoding[128+ch]);
			}
		}
		else if (scantext(*t))
		{
			fprintf(out,"8#%o /%s\n",256+ch,encoding[128+ch]);
		}
		t=t++;
	}
	if (beginreencode)
	{
		fprintf(out,"] def\n/Helvetica /Helvetica-8 scan-vec ReEncodeSmall\n/Helvetica-8 findfont %d scalefont setfont\n",psheight/screenlines);
	}
	
	switch (alignment)
	{
		case 0:
			if (firstuse.text)
			{
				fprintf(out,"%s\n",lstrmacro);
				firstuse.text=0;
			}
			fprintf(out,"%g %g 90 (%s) %d Lstr\n",x,y,s,color);
			break;
		case 1:
			if (firstuse.ctext)
			{
				fprintf(out,"%s\n",cstrmacro);
				firstuse.ctext=0;
			}
			fprintf(out,"%g %g 90 (%s) %d Cstr\n",x,y,s,color);
			break;
		case 2:
			if (firstuse.rtext)
			{
				fprintf(out,"%s\n",rstrmacro);
				firstuse.rtext=0;
			}
			fprintf(out,"%g %g 90 (%s) %d Rstr\n",x,y,s,color);
			break;
	}
	if (beginreencode) fprintf(out,"grestore\n");
}

void PSGraph::scale (double s)
{
	if (lineop) lineend(lcolor,lstyle,lwidth);
}

