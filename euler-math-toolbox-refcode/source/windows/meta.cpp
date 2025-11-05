/*** meta.cpp
Metafile Stuff. This is for all versions of Euler (since 1.50).
Euler now needs a C++ compiler for all platforms.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <float.h>
#include <time.h>

#include "meta.h"
#include "../psgraph.h"
#include "../svg.h"
#include "windows.h"
#include "text.h"
#include "graphicswindow.h"

double getcolor (int i, int j);
extern PS *ops;
extern int fillcolor1,fillcolor2,gscreenlines,usecolors,markerfactor;

double pswidth=15,psheight=15;

void Meta::write (void *l, int n)
{   
	if ((size_t)((End+n)-Start)>=Size) Full=1;
	if (!Active || Full) return;
	memmove(End,l,n);
	End+=n;
}

void Meta::commandwrite (int i)
{   
	if (End-Start>(int)(Size-512)) Full=1;
	if (!Active || Full) return;
	*End++=(char)i;
}

void Meta::longwrite (double n)
/***** write a double to the metafile as long
*****/
{	long k;
	k=(long)(n*1000.0);
	write(&k,sizeof(long));
}

void Meta::shortwrite (int n)
/***** write an int to the metafile
*****/
{	write(&n,sizeof(int));
}

void Meta::stringwrite (char *s)
/***** write a string to the metafile
*****/
{	write(s,strlen(s)+1);
}

int Meta::nextcommand (char * &p)
{	int k=*p; p++; return k;
}

double Meta::nextlong (char * &p)
{	long x;
	memmove(&x,p,sizeof(long));
	p+=sizeof(long);
	return x/1000.0;
}

int Meta::nextint (char * &p)
{   int n;
	memmove(&n,p,sizeof(int));
	p+=sizeof(int);
	return n;
}

int Meta::gline (void *p, double c, double r, double c1, double r1, int color,
	int st, double width)
{   if (!Active) return 1;
	if (fabs(r)>10000.0) return 0;
	if (fabs(c)>10000.0) return 0;
	if (fabs(r1)>10000.0) return 0;
	if (fabs(c1)>10000.0) return 0;
	commandwrite(10);
	longwrite(c); longwrite(r); longwrite(c1); longwrite(r1);
	shortwrite(color); shortwrite(st); longwrite(width);
    return 1;
}

int Meta::gpath (void *p, double c[], int n, int color, int st, double width)
{   
	if (!Active) return 1;
	commandwrite(51);
	shortwrite(n);
	for (int i=0; i<n; i++)
	{	longwrite(c[2*i]); longwrite(c[2*i+1]);
	}
	shortwrite(color); shortwrite(st); longwrite(width);
    return 1;
}

int Meta::gmarker (void *p, double c, double r, int color, int type, double size)
{	if (!Active) return 1;
	commandwrite(20);
	longwrite(c); longwrite(r);
	shortwrite(color); shortwrite(type); longwrite(size);
    return 1;
}

int Meta::gfill (void *p, double c[], int fillcolor, int wirecolor, int n, int connect[])
{	if (!Active) return 1;
	int i;
	for (i=0; i<2*n; i++) if (fabs(c[i])>10000.0) return 0;
	commandwrite(30);
	shortwrite(n);
	for (i=0; i<n; i++)
	{	longwrite(c[2*i]); longwrite(c[2*i+1]);
		shortwrite(connect[i]);
	}
	shortwrite(fillcolor);
	shortwrite(wirecolor);
    return 1;
}

int Meta::gpolygon (void *p, double c[], int n, int st, int color, int connect)
{	if (!Active) return 1;
	int i;
	for (i=0; i<2*n; i++) if (fabs(c[i])>10000.0) return 0;
	commandwrite(50);
	shortwrite(n);
	for (i=0; i<n; i++)
	{	longwrite(c[2*i]); longwrite(c[2*i+1]);
	}
	shortwrite(st);
	shortwrite(color);
	shortwrite(connect);
    return 1;
}

int Meta::gfillh (void *p, double c[], int n, double hue, int color, int connect)
{	if (!Active) return 1;
	int i;
	for (i=0; i<2*n; i++) if (fabs(c[i])>10000.0) return 0;
	hue-=floor(hue);
	commandwrite(31);
	shortwrite(n);
	for (i=0; i<2*n; i+=2)
	{	longwrite(c[i]); longwrite(c[i+1]);
	}
	longwrite(hue);
	shortwrite(color); shortwrite(connect);
    return 1;
}

int Meta::gbar (void *p, double c, double r, double c1, double r1, double hue,
	int color, int style)
{	if (!Active) return 1;
	commandwrite(32);
	longwrite(c); longwrite(r);
	longwrite(c1); longwrite(r1);
	longwrite(hue);
	shortwrite(color);
    shortwrite(style);
    return 1;
}

int Meta::gbar1 (void *p, double c, double r, double c1, double r1,
	int color, int style)
{	if (!Active) return 1;
	commandwrite(33);
	longwrite(c); longwrite(r);
	longwrite(c1); longwrite(r1);
	shortwrite(color);
	shortwrite(style);
    return 1;
}

int Meta::gbarRGB (void *p, double c1, double r1, double c2, double r2, int c, int r,
	int tred, int tgreen, int tblue, unsigned char *colors)
{	if (!Active) return 1;
	commandwrite(34);
	longwrite(c1); longwrite(r1);
	longwrite(c2); longwrite(r2);
	shortwrite(c); shortwrite(r); 
	shortwrite(tred); shortwrite(tgreen); shortwrite(tblue);
	write(colors,3*c*r*sizeof(unsigned char));
	return 1;
}

int Meta::gtext (void *p, double c, double r, char *text, int color, int alignment, double size)
{	if (!Active) return 1;
	commandwrite(40); longwrite(c);
	longwrite(r); shortwrite(color); shortwrite(alignment); longwrite(size);
	stringwrite(text);
    return 1;
}

int Meta::gvtext (void *p, double c, double r, char *text, int color, int alignment, double size)
{	if (!Active) return 1;
	commandwrite(41); longwrite(c);
	longwrite(r); shortwrite(color); shortwrite(alignment); longwrite(size);
	stringwrite(text);
    return 1;
}

int Meta::gvutext (void *p, double c, double r, char *text, int color, int alignment, double size)
{	if (!Active) return 1;
	commandwrite(42); longwrite(c);
	longwrite(r); shortwrite(color); shortwrite(alignment); longwrite(size);
	stringwrite(text);
    return 1;
}

int Meta::gclip (void *p, double c, double r, double c1, double r1)
{
	if (!Active) return 1;
	if (fabs(r)>10000.0) return 0;
	if (fabs(c)>10000.0) return 0;
	if (fabs(r1)>10000.0) return 0;
	if (fabs(c1)>10000.0) return 0;
	commandwrite(2);
	longwrite(c);
	longwrite(r);
	longwrite(c1);
	longwrite(r1);
	return 1;
}
 
int Meta::gclear (void *p)
{   if (!Active) return 1;
	clear();
	commandwrite(1);
    return 1;
}

extern double *cc;
extern int ccsize;
extern void check_cc_size (int n);

void Meta::replay (void *ps, int clip)
/* Redraw the graphics as noted in notespace */
{	
	Active=0;
	char *p=Start;
	double c,r,c1,r1,c2,r2,hue,width,size;
	int col,col1,st,n,i,co[5],cn,rn,con,red,green,blue;
	while (p<End)
	{   
		int command=nextcommand(p);
    	switch(command)
		{	case 1 :
				gclear(ps);
				break;
			case 2 :
				c=nextlong(p);
				r=nextlong(p);
				c1=nextlong(p);
				r1=nextlong(p);
				if (clip) gclip(ps,c,r,c1,r1);
				break;
         	case 10 :
				c=nextlong(p);
				r=nextlong(p);
				c1=nextlong(p);
				r1=nextlong(p);
				col=nextint(p);
				st=nextint(p);
				width=nextlong(p);
				gline(ps,c,r,c1,r1,col,st,width);
				break;
			case 20 :
				c=nextlong(p);
				r=nextlong(p);
				col=nextint(p);
				st=nextint(p);
				size=nextlong(p);
				gmarker(ps,c,r,col,st,size);
				break;
			case 30 :
				n=nextint(p);
				check_cc_size(n);
				for (i=0; i<n; i++)
				{	cc[2*i]=nextlong(p);
					cc[2*i+1]=nextlong(p);
					co[i]=nextint(p);
				}
				col=nextint(p);
				col1=nextint(p);
				gfill(ps,cc,col,col1,n,co);
				break;
			case 31 :
				n=nextint(p);
				check_cc_size(2*n);
				for (i=0; i<2*n; i++) cc[i]=nextlong(p);
				hue=nextlong(p);
				col=nextint(p);
				st=nextint(p);
				gfillh(ps,cc,n,hue,col,st);
				break;
			case 32 :
				c=nextlong(p);
				r=nextlong(p);
				c1=nextlong(p);
				r1=nextlong(p);
				hue=nextlong(p);
				col=nextint(p);
				st=nextint(p);
				gbar(ps,c,r,c1,r1,hue,col,st);
				break;
			case 33 :
				c=nextlong(p);
				r=nextlong(p);
				c1=nextlong(p);
				r1=nextlong(p);
				col=nextint(p);
				st=nextint(p);
				gbar1(ps,c,r,c1,r1,col,st);
				break;
			case 34 :
				c1=nextlong(p);
				r1=nextlong(p);
				c2=nextlong(p);
				r2=nextlong(p);
				cn=nextint(p);
				rn=nextint(p);
				red=nextint(p);
				green=nextint(p);
				blue=nextint(p);
				gbarRGB(ps,c1,r1,c2,r2,cn,rn,red,green,blue,(unsigned char *)p);
				p+=3*cn*rn*sizeof(unsigned char);
				break;
			case 40 :
				c=nextlong(p);
				r=nextlong(p);
				col=nextint(p);
				st=nextint(p);
				size=nextlong(p);
				gtext(ps,c,r,p,col,st,size);
				p+=strlen(p)+1;
				break;
			case 41 :
				c=nextlong(p);
				r=nextlong(p);
				col=nextint(p);
				st=nextint(p);
				size=nextlong(p);
				gvtext(ps,c,r,p,col,st,size);
				p+=strlen(p)+1;
				break;
			case 42 :
				c=nextlong(p);
				r=nextlong(p);
				col=nextint(p);
				st=nextint(p);
				size=nextlong(p);
				gvutext(ps,c,r,p,col,st,size);
				p+=strlen(p)+1;
				break;
			case 50 :
				n=nextint(p);
				check_cc_size(n);
				for (i=0; i<n; i++)
				{	cc[2*i]=nextlong(p);
					cc[2*i+1]=nextlong(p);
				}
				st=nextint(p);
				col=nextint(p);
				con=nextint(p);
				gpolygon(ps,cc,n,st,col,con);
				break;
			case 51 : // gpath
				n=nextint(p);
				check_cc_size(n);
				for (i=0; i<n; i++)
				{	cc[2*i]=nextlong(p);
					cc[2*i+1]=nextlong(p);
				}
				col=nextint(p);
				st=nextint(p);
				width=nextlong(p);
				gpath(ps,cc,n,col,st,width);				
			case 60 :
				break;
			default :
            	Active=1;
				return;
		}
	}
	Active=1;
}

void Meta::dump (FILE *out)
{	fwrite(Start,1,End-Start,out);
}

void Meta::postscript (FILE *out)
{	Active=0;
	char *p=Start;
	double c,r,c1,r1,hue,con;
	int col,col1,st,n,i,co[16];
	double width,size;
	PSGraph	*ps=new PSGraph(out,graphicswindow->bitmapwidth(),graphicswindow->bitmapheight());
	for (i=0; i<16; i++)
		ps->setcolor(i,getcolor(i,0),getcolor(i,1),getcolor(i,2));
	ps->setfillcolors(fillcolor1,fillcolor2);
	ps->setscreenlines(gscreenlines);
	ps->setusecolor(usecolors);
	ps->init();
	while (p<End) 
	{
		if (test_key()==27) break;
		int command=nextcommand(p);
	    	switch(command) {
			case 1 :
				ps->clear();
				break;
			case 2 :
				c=nextlong(p);
				r=nextlong(p);
				c1=nextlong(p);
				r1=nextlong(p);
				ps->clip(c,r,c1,r1);
				break;
         	case 10 :
				c=nextlong(p);
				r=nextlong(p);
				c1=nextlong(p);
				r1=nextlong(p);
				col=nextint(p);
				st=nextint(p);
				width=nextlong(p);
				ps->line(c,r,c1,r1,col,st,width);
				break;
			case 20 :
				c=nextlong(p);
				r=nextlong(p);
				col=nextint(p);
				st=nextint(p);
				size=nextlong(p);
				ps->marker(c,r,col,st,size);
				break;
			case 30 :
				n=nextint(p);
				check_cc_size(n);
				for (i=0; i<n; i++)
				{	cc[2*i]=nextlong(p);
					cc[2*i+1]=nextlong(p);
					co[i]=nextint(p);
				}
				col=nextint(p);
				col1=nextint(p);
				ps->fill(cc,col,col1,n,co);
				break;
			case 31 :
				n=nextint(p);
				check_cc_size(2*n);
				for (i=0; i<2*n; i++) cc[i]=nextlong(p);
				hue=nextlong(p);
				col=nextint(p);
				st=nextint(p);
				ps->fillh(cc,n,hue,col,st);
				break;
			case 32 :
				c=nextlong(p);
				r=nextlong(p);
				c1=nextlong(p);
				r1=nextlong(p);
				hue=nextlong(p);
				col=nextint(p);
				st=nextint(p);
				ps->bar(c,r,c1,r1,hue,col,st);
				break;
			case 33 :
				c=nextlong(p);
				r=nextlong(p);
				c1=nextlong(p);
				r1=nextlong(p);
				col=nextint(p);
				st=nextint(p);
				ps->bar1(c,r,c1,r1,col,st);
				break;
			case 40 :
				c=nextlong(p);
				r=nextlong(p);
				col=nextint(p);
				st=nextint(p);
				size=nextlong(p);
				ps->text(c,r,p,col,st);
				p+=strlen(p)+1;
				break;
			case 41 :
				c=nextlong(p);
				r=nextlong(p);
				col=nextint(p);
				st=nextint(p);
				size=nextlong(p);
				ps->vtext(c,r,p,col,st);
				p+=strlen(p)+1;
				break;
			case 42 :
				c=nextlong(p);
				r=nextlong(p);
				col=nextint(p);
				st=nextint(p);
				size=nextlong(p);
				ps->vutext(c,r,p,col,st);
				p+=strlen(p)+1;
				break;
			case 50 :
				n=nextint(p);
				check_cc_size(n);
				for (i=0; i<n; i++)
				{	cc[2*i]=nextlong(p);
					cc[2*i+1]=nextlong(p);
				}
				st=nextint(p);
				col=nextint(p);
				con=nextint(p);
				// !!! todo: filled polygon
				break;
			case 51 :
				n=nextint(p);
				check_cc_size(n);
				for (i=0; i<n; i++)
				{	cc[2*i]=nextlong(p);
					cc[2*i+1]=nextlong(p);
				}
				col=nextint(p);
				st=nextint(p);
				width=nextlong(p);
				ps->path(cc,n,col,st,width); // !!! todo: polyline
			case 60 :
				break;
			default :
            	Active=1;
				delete ps;
				return;
		}
	}
	delete ps;
	Active=1;
}

extern String GraphicsFontName;

void Meta::svg (FILE *out, int w, int h)
{	Active=0;
	char *p=Start;
	double c,r,c1,c2,r1,r2,hue;
	int col,col1,st,n,i,j,co[16],con,red,green,blue,cn,rn;
	double width,size;
	SVGGraph *svg=new SVGGraph(out,w,h);
	for (i=0; i<16; i++)
		for (j=0; j<3; j++)
			svg->setcolor(i,getcolor(i,0),getcolor(i,1),getcolor(i,2));
	svg->setfillcolors(fillcolor1,fillcolor2);
	svg->setscreenlines(GraphicsFontName.text(),gscreenlines);
	svg->setusecolor(usecolors);
	svg->setmarkerfactor(markerfactor);
	svg->init();
	while (p<End) {
		int command=nextcommand(p);
	    	switch(command) {
			case 1 :
				svg->clear();
				break;
			case 2 :
				c=nextlong(p);
				r=nextlong(p);
				c1=nextlong(p);
				r1=nextlong(p);
				svg->clip(c,r,c1,r1);
				break;
         	case 10 :
				c=nextlong(p);
				r=nextlong(p);
				c1=nextlong(p);
				r1=nextlong(p);
				col=nextint(p);
				st=nextint(p);
				width=nextlong(p);
				svg->line(c,r,c1,r1,col,st,width);
				break;
			case 20 :
				c=nextlong(p);
				r=nextlong(p);
				col=nextint(p);
				st=nextint(p);
				size=nextlong(p);
				svg->marker(c,r,col,st,size);
				break;
			case 30 :
				n=nextint(p);
				check_cc_size(n);
				for (i=0; i<n; i++)
				{	cc[2*i]=nextlong(p);
					cc[2*i+1]=nextlong(p);
					co[i]=nextint(p);
				}
				col=nextint(p);
				col1=nextint(p);
				svg->fill(cc,col,col1,n,co);
				break;
			case 31 :
				n=nextint(p);
				check_cc_size(2*n);
				for (i=0; i<2*n; i++) cc[i]=nextlong(p);
				hue=nextlong(p);
				col=nextint(p);
				st=nextint(p);
				svg->fillh(cc,n,hue,col,st);
				break;
			case 32 :
				c=nextlong(p);
				r=nextlong(p);
				c1=nextlong(p);
				r1=nextlong(p);
				hue=nextlong(p);
				col=nextint(p);
				st=nextint(p);
				svg->bar(c,r,c1,r1,hue,col,st);
				break;
			case 33 :
				c=nextlong(p);
				r=nextlong(p);
				c1=nextlong(p);
				r1=nextlong(p);
				col=nextint(p);
				st=nextint(p);
				svg->bar1(c,r,c1,r1,col,st);
				break;
			case 34 :
				c1=nextlong(p);
				r1=nextlong(p);
				c2=nextlong(p);
				r2=nextlong(p);
				cn=nextint(p);
				rn=nextint(p);
				red=nextint(p);
				green=nextint(p);
				blue=nextint(p);
				svg->barRGB(c1,r1,c2,r2,cn,rn,red,green,blue,(unsigned char *)p);
				p+=3*cn*rn*sizeof(unsigned char);
				break;
			case 40 :
				c=nextlong(p);
				r=nextlong(p);
				col=nextint(p);
				st=nextint(p);
				size=nextlong(p);
				svg->text(c,r,p,col,st);
				p+=strlen(p)+1;
				break;
			case 41 :
				c=nextlong(p);
				r=nextlong(p);
				col=nextint(p);
				st=nextint(p);
				size=nextlong(p);
				svg->vtext(c,r,p,col,st);
				p+=strlen(p)+1;
				break;
			case 42 :
				c=nextlong(p);
				r=nextlong(p);
				col=nextint(p);
				st=nextint(p);
				size=nextlong(p);
				svg->vutext(c,r,p,col,st);
				p+=strlen(p)+1;
				break;
			case 50 :
				n=nextint(p);
				check_cc_size(n);
				for (i=0; i<n; i++)
				{	cc[2*i]=nextlong(p);
					cc[2*i+1]=nextlong(p);
				}
				st=nextint(p);
				col=nextint(p);
				con=nextint(p);
				svg->polygon(cc,n,st,col,con);
				break;
			case 51 : // gpath
				n=nextint(p);
				check_cc_size(n);
				for (i=0; i<n; i++)
				{	cc[2*i]=nextlong(p);
					cc[2*i+1]=nextlong(p);
				}
				col=nextint(p);
				st=nextint(p);
				width=nextlong(p);
				svg->path(cc,n,col,st,width);
			case 60 :
				break;
			default :
            	Active=1;
				delete svg;
				return;
		}
	}
	delete svg;
	Active=1;
}

/**************** The externally visible things ****************/
// Called from graphics, mainloop etc.

extern int agmode;

extern CriticalSection critical;

void gclear (void)
{	
	critical.enter();
	agmode=0;
	meta->gclear(0);
	critical.leave();
	// graphicswindow->update();
}

void gclip (double c, double r, double c1, double r1)
{
	critical.enter();
	meta->gclip(0,c,r,c1,r1);
	critical.leave();
}

void gline (double c, double r, double c1, double r1, int color,
	int st, double width)
/***** gline
	draw a line.
	col is the color, where 0 should be white and 1 black.
	st is a style from linetyp.
	width is the linewidth, where 0 or 1 are equal defaults.
*****/
{
	critical.enter();
	meta->gline(0,c,r,c1,r1,color,st,width);
	critical.leave();
}

void gpath (double c[], int n, int color, int st, double width)
/***** gline
	draw a line.
	col is the color, where 0 should be white and 1 black.
	st is a style from linetyp.
	width is the linewidth, where 0 or 1 are equal defaults.
*****/
{
	critical.enter();
	meta->gpath(0,c,n,color,st,width);
	critical.leave();
}

void gmarker (double c, double r, int color, int st, double markerfactor)
/***** gmarker
	plot a single marker on screen.
	col is the color.
	type is a type from markertyp.
*****/
{   
	critical.enter();
	meta->gmarker(0,c,r,color,st,markerfactor);
	critical.leave();
}

void gfill (double c[], int fillcolor, int wirecolor, int n, int connect[])
{	
	critical.enter();
	meta->gfill(0,c,fillcolor,wirecolor,n,connect);
	critical.leave();
}

void gpolygon (double c[], int n, int st, int color, int connect)
{	
	critical.enter();
	meta->gpolygon(0,c,n,st,color,connect);
	critical.leave();
}

void gfillh (double c[], int n, double hue, int color, int connect)
/***** Draw a filled polygon.
	Works like gfill, but uses hue.
*****/
{	
	critical.enter();
	meta->gfillh(0,c,n,hue,color,connect);
	critical.leave();
}

void gbar (double c, double r, double c1, double r1, double hue,
	int color, int connect)
/***** Draw a rectangle.
	hue is a hue intensity from 0 to 1.
	style determines, if a black boundary should be drawn.
******/
{	
	critical.enter();
	meta->gbar(0,c,r,c1,r1,hue,color,connect);
	critical.leave();
}

void gbar1 (double c, double r, double c1, double r1,
	int color, int connect)
/***** Draw a rectangle.
	hue is a hue intensity from 0 to 1.
	style determines, if a black boundary should be drawn.
******/
{	
	critical.enter();
	meta->gbar1(0,c,r,c1,r1,color,connect);
	critical.leave();
}

void gbarRGB (double c1, double r1, double c2, double r2, int c, int r,
	int tred, int tgreen, int tblue,
	double *red, double *green, double *blue)
/***** Draw a rectangle.
	hue is a hue intensity from 0 to 1.
	style determines, if a black boundary should be drawn.
******/
{	
	critical.enter();
	int i,j;
	double x;
	unsigned char *colors= new unsigned char [3*c*r];
	unsigned char *p=colors;
	for (i=r-1; i>=0; i--)
		for (j=0; j<c; j++)
		{	x=red[i*c+j];
			if (x<0) x=0;
			if (x>1) x=1;
			*p++=((unsigned char)(x*255));
			x=green[i*c+j];
			if (x<0) x=0;
			if (x>1) x=1;
			*p++=((unsigned char)(x*255));
			x=blue[i*c+j];
			if (x<0) x=0;
			if (x>1) x=1;
			*p++=((unsigned char)(x*255));
		}
	meta->gbarRGB(0,c1,r1,c2,r2,c,r,tred,tgreen,tblue,colors);
	delete colors;
	critical.leave();
}

void gtext (double c, double r, char *text, int color, int centered, double size)
/***** gtext
	output a graphic text on screen.
	alignment is left=0, centered=1, right=2.
*****/
{	
	critical.enter();
	meta->gtext(0,c,r,text,color,centered,size);
	critical.leave();
}

void gvtext (double c, double r, char *text, int color, int centered, double size)
/***** gvtext
	like gtext downwards
*****/
{	
	critical.enter();
	meta->gvtext(0,c,r,text,color,centered,size);
	critical.leave();
}

void gvutext (double c, double r, char *text, int color, int centered, double size)
/***** gvutext
	like gtext upwards.
*****/
{	
	critical.enter();
	meta->gvutext(0,c,r,text,color,centered, size);
	critical.leave();
}

void dump_postscript (FILE *out)
{	meta->postscript(out);
}

void pswindow (double w, double h)
{	pswidth=w;
	psheight=h;
}

