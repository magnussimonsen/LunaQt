#ifndef WINDOWSFRAMES
#define WINDOWSFRAMES

#include "windows.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "../header.h"
#include "../sysdep.h"
#include "euler.h"
#include "meta.h"
#include "colors.h"
#include "text.h"
#include "windowsmeta.h"
#include "graphicswindow.h"
#include "main.h"
#include "windowsframes.h"


/*
Routines for the Frame and the Frames classes.
Especially show, copy and delete.
But also Anaglyph generation.
*/

Frame::Frame () : Next(0)
{	copyfrom(meta);
}

Frame::Frame (int w, int h) : Next(0)
{	copyfrom(meta);
}

Frame::~Frame ()
{	delete M;
}

Frames::~Frames ()
{	remove();
}

void Frames::remove ()
{	Frame *p=First,*h;
	while (p)
	{	h=p->next();
		delete p;
		p=h;
	}
	First=0;
}

void Frames::append ()
{	Frame *p=First;
	if (!p)
	{	First=new Frame();
	}
	else
	{	while (p->next()) p=p->next();
		p->next(new Frame());
	}
}

void Frames::append (int w, int h)
{	Frame *p=First;
	if (!p)
	{	First=new Frame(w,h);
	}
	else
	{	while (p->next()) p=p->next();
		p->next(new Frame(w,h));
	}
}

void Frames::show (int n)
{	if (!First) return;
	Frame *p=First;
	int i=0;
	while (p && i<n)
	{	i++; p=p->next();
	}
	if (!p) return;

	critical.enter();
	if (usecolors) ops->erase();
	else ops->bar(0,0,ops->width(),ops->height(),MyColor(0));
	p->getMeta()->replay(ops);
	critical.leave();

	WindowPS ps(*graphicswindow);
	graphicswindow->redraw(ps);
	((WindowsMeta *)meta)->Draw=0;
}

int Frames::makeag (BitmapPS *bitmap)
{	
	Frame *p=First;
	if (!p) return 0;
	Frame *pn=p->next();
	if (!pn) return 0;

	int savew=bitmap->width();
	int saveh=bitmap->height();

	BitmapPS m(*graphicswindow,savew,saveh);
	Font mfont(GraphicsFontName,max(savew,saveh)/gscreenlines);
	m.font(mfont);
	Font mufont(GraphicsFontName,max(savew,saveh)/gscreenlines,Font::normal,0,900);
	m.rememberfont(1,mufont);
	Font mdfont(GraphicsFontName,max(savew,saveh)/gscreenlines,Font::normal,0,2700);
	m.rememberfont(2,mdfont);
	
	BitmapPS mn(*graphicswindow,savew,saveh);
	mn.font(mfont);
	mn.rememberfont(1,mufont);
	mn.rememberfont(2,mdfont);

	critical.enter();
	m.bar(0,0,savew,saveh,MyColor(0));
	p->getMeta()->replay(&m);
	mn.bar(0,0,savew,saveh,MyColor(0));
	pn->getMeta()->replay(&mn);

	BYTE *b=m.getBits();
	BYTE *bn=mn.getBits();
	if (!b || !bn)
	{	critical.leave();
		return 0;
	}
	BYTE *x=b,*y=bn;
	int d=savew%4;
	int g1,g2;
	for (int i=0; i<saveh; i++)
	{	
		for (int j=0; j<savew-1; j++)
		{	
			g1=(*(x+1)*3+*(x+2)*5+*(x+3)*4)/12;
			g2=(*(y+1)*3+*(y+2)*5+*(y+3)*4)/12;
			*x++=(BYTE)((*y+g2)/2); *x++=(BYTE)((*(y+1)+g2)/2); *x++=BYTE(g1);
			y+=3;
		}
		x+=d+3; y+=d+3;
	}
	bitmap->setBits(b);
	delete b;
	delete bn;
	critical.leave();

	return 1;
}

void Frames::copyto (int n)
{	if (!First) return;
	Frame *p=First;
	int i=0;
	while (p && i<n)
	{	i++; p=p->next();
	}
	if (!p) return;
	p->copyfrom(meta);
}

void Frames::copyfrom (int n)
{	if (!First) return;
	Frame *p=First;
	int i=0;
	while (p && i<n)
	{	i++; p=p->next();
	}
	if (!p) return;

	critical.enter();
	p->getMeta()->replay(ops);
	critical.leave();
}

int Frames::nframes ()
{	if (!First) return 0;
	int i=0;
	Frame *p=First;
	while (p) { i++; p=p->next(); }
	return i;
}

Frames frames;

int nframes ()
{	return frames.nframes();
}

void addframe (void)
{   frames.append();
}

void addframe (int w, int h)
{	frames.append(w,h);
}

void deleteframes ()
{	frames.remove();
	showframe(0);
}

int doreplay ();

void showframe (int n)
{	static int currentframe=-2; 
	if (!ops) return; // could be at program start (race condition)
	if (n==currentframe && n>=0) return;
	if (n==0)
	{   agmode=0;

		critical.enter();
		((WindowsMeta *)meta)->Draw=1;
		if (usecolors) ops->erase();
		else ops->bar(0,0,ops->width(),ops->height(),MyColor(0));
		ops->erase();
		meta->replay(ops);
		critical.leave();
	
		graphicswindow->update();
	}
	else if (n>0)
    {	agmode=0;
		frames.show(n-1);
    }
	else if (n==-1)
	{	agmode=1;
		doreplay();
	}
	currentframe=n;
}

void copytoframe (int n)
{	frames.copyto(n-1);
}

void copyfromframe (int n)
{	frames.copyfrom(n-1);
}

#endif