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

extern int largeimages,enlarge;

extern int neverantialiasinsimg,cropsquare,boldfont,gfontfixed,textwidth,generatesvg,systemscaling,sharper;

int svgnumber=1;
extern String FileName;

void setstyles ();
extern int fatterlines;

// Insert the current image into the text window.
void insert_pixels (int n, char *name, double *m, int c, int r)
{	
	BitmapPS *bitmap=new BitmapPS(textwindow,c,r);
	BYTE *b=bitmap->getBits();
	BYTE *x=b;
	int d=c%4;
	for (int i=0; i<r; i++)
	{	for (int j=0; j<c; j++)
		{	unsigned long k=(unsigned long)(*m++);
			*x++=(byte)(k&0x000000FF);
			*x++=(byte)((k&0x0000FF00)>>8);
			*x++=(byte)((k&0x00FF0000)>>16);
		}
		x+=d;
	}
	bitmap->setBits(b);
	delete(b);

	if (n<5) n=5;
	if (n>60) n=60;
	int lines=(bitmap->height()-1)/text->textheight()+1;
	if (lines>n)
	{	bitmap->scale(n*text->textheight()*bitmap->width()/bitmap->height(),
			n*text->textheight());
		lines=n;
	}

	text->showupdate(0);
	for (int i=0; i<lines; i++)
	{	text->cursorline()->Bitmap=bitmap;
		text->cursorline()->bitmapline=i;
		if (i==0) text->inserttext(name);
		text->inserttext("\n");
	}
	text->showupdate(1);
}

/** 
Load an image from a text file.
*/
int load_image (int n, char *filename, double scale, int reduce, char *latex, int mathjax)
{	
	String name(filename);
	if (*name.extension()==0) name.cat(".png");
	String h(name);
	String h1("images\\"); h1.cat(name);
	if (imagesindirectory)
	{	if (exists(h1.text())) h.copy(h1);
	}
	else
	{	if (!exists(h.text())) h.copy(h1);
	}
	BitmapPS *m=new BitmapPS(textwindow,h.text());
	if (m->bitmaphandle()==0) return 0;

	BitmapPS *ml=0;
	if (reduce==2)
	{
		ml=new BitmapPS(textwindow,m->width(),m->height());
		m->copy(*ml,0,0);
	}

	if (scale==0) scale=1.0;
	if (scale<0.1) scale=0.1;
	if (scale>10) scale=10;

	if (reduce) m->scale(m->width()/antialiasfactor,m->height()/antialiasfactor);
	else if (scale!=1.0) m->scale((int)(m->width()*scale),(int)(m->height()*scale));

	if (n<5) n=5;
	if (n>60) n=60;

	if (m->width()>textwidth*0.8)
		m->scale(rd(textwidth*0.8/factor()),rd(textwidth*0.8*m->height()/m->width()/factor()));

	int lines=(int)(m->height()*factor()/text->textheight())+1;
	if (lines>n)
	{	
		m->scale(rd(n*text->textheight()*m->width()/m->height()/factor()),
			rd(n*text->textheight()/factor()));
		lines=n;
	}

	text->showupdate(0);
	for (int i=0; i<lines; i++)
	{	
		text->cursorline()->Bitmap=m;
		text->cursorline()->bitmapline=i;
		if (i==0 && ml) text->cursorline()->BitmapLarge=ml;
		if (i==0 && latex) text->cursorline()->latex(latex,mathjax);
		text->inserttext("\n");
	}
	text->showupdate(1);
	return 1;
}

/** 
Load an image from a text file.
*/
int load_ag_images (int n, char *filename1, char *filename2)
{	
	String name(filename1);
	if (*name.extension()==0) name.cat(".png");
	String h(name);
	String h1("images\\"); h1.cat(name);
	if (imagesindirectory)
	{	if (exists(h1.text())) h.copy(h1);
	}
	else
	{	if (!exists(h.text())) h.copy(h1);
	}
	BitmapPS *m=new BitmapPS(textwindow,h.text());
	if (m->bitmaphandle()==0) return 0;

	name.copy(filename2);
	if (*name.extension()==0) name.cat(".png");
	h.copy(name);
	h1.copy("images\\"); h1.cat(name);
	if (imagesindirectory)
	{	if (exists(h1.text())) h.copy(h1);
	}
	else
	{	if (!exists(h.text())) h.copy(h1);
	}
	BitmapPS *mn=new BitmapPS(textwindow,h.text());
	if (mn->bitmaphandle()==0) return 0;

	int bw=m->width(),bh=m->height();

	BitmapPS *bitmap=new BitmapPS(*graphicswindow,bw,bh);

	if (bw<16 || bw!=mn->width() || bh<16 || bh!=mn->height())
	{
		dump(
			"Need two images of the same size for anaglyphs.\n"
			"Got %s with %d x %d\n"
			"and %s with %d x %d",
			filename1,bw,bh,filename2,mn->width(),mn->height());
		return 0;
	}

	BYTE *b=m->getBits();
	BYTE *bn=mn->getBits();
	if (!b || !bn)
	{	return 0;
	}
	BYTE *x=b,*y=bn;
	int d=bw%4;
	int g1,g2;
	for (int i=0; i<bh; i++)
	{	for (int j=0; j<bw-1; j++)
		{	g1=(*(x+1)*3+*(x+2)*5+*(x+3)*4)/12;
			g2=(*(y+1)*3+*(y+2)*5+*(y+3)*4)/12;
			*x++=(BYTE)((*y+g2)/2); *x++=(BYTE)((*(y+1)+g2)/2); *x++=(BYTE)g1;
			y+=3;
		}
		x+=d+3; y+=d+3;
	}
	bitmap->setBits(b);
	delete b;
	delete bn;

	delete m;
	delete mn;

	m=bitmap;
	
	if (n<5) n=5;
	if (n>60) n=60;

	int lines=rd(m->height()*factor()/text->textheight());
	if (lines>n)
	{	
		m->scale(rd(n*text->textheight()*m->width()/m->height()/factor()),
			rd(n*text->textheight()/factor()));
		lines=n;
	}
	text->showupdate(0);
	for (int i=0; i<lines; i++)
	{	text->cursorline()->Bitmap=m;
		text->cursorline()->bitmapline=i;
		text->inserttext("\n");
	}
	text->showupdate(1);
	return 1;
}

extern int savew,saveh;
extern String SVGName;
extern int savecompressedsvg,linux;

void savesvg (char *filename, int w, int h)
{   
	if (w<10) w=graphicswindow->windowwidth();
	if (h<10) h=w*graphicswindow->windowheight()/graphicswindow->windowwidth();
	savew=w; saveh=h;

	if (!savecompressedsvg)
	{
		SVGName.copy(filename);	
		if (*SVGName.extension()==0) SVGName.cat(".svg");

		FILE *out=fopen(SVGName,"w");
		if (!out) return;
		meta->svg(out,savew,saveh);
		fclose(out);
	}
	else
	{
		FILE *out=fopen("temp.svg","w");
		if (!out) return;
		meta->svg(out,savew,saveh);
		fclose(out);

		remove("temp.svz");
		char line[MAXLINE];
		sprintf(line,"\"%s7z.exe\" -tgzip a temp.svz temp.svg",getstartdir());

		STARTUPINFO si; // structure to start a process
		PROCESS_INFORMATION pi; // structure to get back process information
	
		ZeroMemory(&si, sizeof(si) );
		si.cb = sizeof(si);
	
		ZeroMemory(&pi, sizeof(pi) );

		if (!CreateProcess(0,line,0,0,0,CREATE_NO_WINDOW,0,0,&si,&pi))
		{
			char ws[MAXLINE+255];
			sprintf(ws,"Could not call the 7z cmmand!\nCommand Line\n%s",line);
			Warning(ws,"Euler");
			return;
		}
		if (WaitForSingleObject(pi.hProcess,INFINITE))
		{
			if (!linux)
			{
				DumpWarning(
					"Waiting for the 7z process failed!\n"
					"%s\n"
					"Please retry.",
					"Euler",line);
				return;
			}
			else
			{
				int tries=0;
				while (!exists("temp.svz") && tries<100) { tries++; Sleep(200); }
			}
		}

		CloseHandle( pi.hProcess );
	    CloseHandle( pi.hThread );

		dump(filename);
		SVGName.copy(filename);	
		if (*SVGName.extension()==0) SVGName.cat(".svgz");
		CopyFile("temp.svz",SVGName.text(),FALSE);
		remove("temp.svz");
		remove("temp.svg");
	}
}

extern DWORD dot_style[];
extern DWORD dash_style[];
extern DWORD dashdot_style[];
extern DWORD dashdotdash_style[];
extern DWORD dotdashdot_style[];

extern BitmapPS *hatchbitmap;

extern int hatchgridsize;

void set_styles (int antialias, double f)
{
	if (hatchbitmap) delete hatchbitmap;

	if (antialias) f=f*antialiasfactor;
	dot_style[0]=rd(1*f); dot_style[1]=rd(10*f);
	dash_style[0]=rd(6*f); dash_style[1]=rd(12*f);
	dashdot_style[0]=rd(1*f); dashdot_style[1]=rd(12*f);
	dashdot_style[2]=rd(6*f); dashdot_style[3]=rd(12*f);
	dashdotdash_style[0]=rd(6*f); dashdotdash_style[1]=rd(12*f);
	dashdotdash_style[2]=rd(1*f); dashdotdash_style[3]=rd(12*f);
	dashdotdash_style[4]=rd(6*f); dashdotdash_style[5]=rd(12*f);
	dotdashdot_style[0]=rd(1*f); dotdashdot_style[1]=rd(12*f);
	dotdashdot_style[2]=rd(6*f); dotdashdot_style[3]=rd(12*f);
	dotdashdot_style[4]=rd(1*f); dotdashdot_style[5]=rd(12*f);

	hatchbitmap=new BitmapPS(*graphicswindow,rd(hatchgridsize*f),rd(hatchgridsize*f));
}

// Insert the current image into the text window.
void insert_image (int n, char *name, int antialiased, 
	double crop1, double crop2, double crop3, double crop4)
{	
	if (neverantialiasinsimg) antialiased=0;
	int factor=1;
	if (antialiased) factor=antialiasfactor;

	int saveh=0,savew=0;
	double asp=graphicswindow->aspect();
	while (n>2)
	{
		saveh=text->textheight()*n;
		savew=rd(saveh*asp);
		if (savew<=textwidth*0.8) break;
		n--;
	}

	if (enlarge)
	{
		saveh=rd(saveh/enlargefactor);
		savew=rd(savew/enlargefactor);
	}

	savew*=factor;
	saveh*=factor;

	
	BitmapPS *m=new BitmapPS(*graphicswindow,savew,saveh);
	Font mfont(GraphicsFontName,
		max(savew,saveh)/gscreenlines,boldfont?Font::bold:Font::normal,gfontfixed);
	m->font(mfont);
	Font mufont(GraphicsFontName,
		max(savew,saveh)/gscreenlines,boldfont?Font::bold:Font::normal,gfontfixed,900);
	m->rememberfont(1,mufont);
	Font mdfont(GraphicsFontName,
		max(savew,saveh)/gscreenlines,boldfont?Font::bold:Font::normal,gfontfixed,2700);
	m->rememberfont(2,mdfont);

	critical.enter();
	int al=antialiasgraphics;
	antialiasgraphics=antialiased;
	m->bar(0,0,savew,saveh,MyColor(0));
	if (!agmode || !frames.makeag(m))
	{
		set_styles(0,linewidth(savew,1,!fatterlines)/2.0);
		meta->replay(m);
		setstyles();
	}
	antialiasgraphics=al;
	critical.leave();
	
	if (crop1<0) crop1=0;
	if (crop2>1) crop2=1;
	if (crop2<crop1) crop2=crop1;
	if (crop3<0) crop3=0;
	if (crop4>1) crop4=1;
	if (crop4<crop3) crop4=crop3;
	BitmapPS *mnew=new BitmapPS(*graphicswindow,rd(savew*(crop4-crop3)),rd(saveh*(crop2-crop1)));
	m->copy(*mnew,-rd(crop3*savew),-rd(crop1*saveh));
	delete m;
	m=mnew;
	savew=rd(savew*(crop4-crop3));
	saveh=rd(saveh*(crop2-crop1));
	n=rd(n*(crop2-crop1));
	
	BitmapPS *ml=0;

	if (factor!=1) 
	{
		if (largeimages||enlarge)
		{
			ml=new BitmapPS(*graphicswindow,m->width(),m->height());
			m->copy(*ml,0,0);
		}
		if (factor==3 && !systemscaling) m->scale3(savew/factor,saveh/factor,sharper);
		else m->scale(savew/factor,saveh/factor);
	}

	static char svgname[MAX_PATH];
	if (generatesvg)
	{
		String name(FileName.filename());
		name.extension("");
		String imgname(FileName);
		imgname.stripfilename();
		if (imagesindirectory)
		{	imgname.cat("images\\");
		}
		imgname.cat(name.text());
		sprintf(svgname,"%s-%03d.svg%s",imgname.text(),svgnumber,savecompressedsvg?"z":"");
		savesvg(svgname,m->width(),m->height());
		if (imagesindirectory) sprintf(svgname,"images/%s-%03d.svg%s",name.text(),svgnumber,savecompressedsvg?"z":"");
		else sprintf(svgname,"%s-%03d.svg%s",name.text(),svgnumber,savecompressedsvg?"z":"");
		svgnumber++;
	}


	text->showupdate(0);
	for (int i=0; i<n; i++)
	{	
		text->cursorline()->Bitmap=m;
		text->cursorline()->bitmapline=i;
		if (i==0 && ml) text->cursorline()->BitmapLarge=ml;
		if (i==0 && generatesvg) text->cursorline()->svg(svgname);
		if (i==0) text->inserttext(name);
		text->inserttext("\n");
	}
	text->showupdate(1);
	text->showcursor();

}

extern int textcolor;

char *texpng (char *formula, int alias, double factor, int transparent, int tc)
{
	if (tc==-1) tc=textcolor;
	return text->makelatexpng(formula,alias,rd(hchar*0.6*factor),transparent,tc);
}