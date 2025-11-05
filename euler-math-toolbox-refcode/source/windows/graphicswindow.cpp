#include "windows.h"
#include "../sysdep.h"
#include "text.h"
#include "graphicswindow.h"
#include "main.h"
#include "euler.h"
#include "colors.h"

extern TextWindow textwindow;

extern int antialiasgraphics,cropsquare;
extern double aspect;

GraphicsWindow::GraphicsWindow (char *title, int hidden)
		: StandardWindow(title,eulerg,defaultsize,defaultsize,
			defaultstyle,NULL,hidewindow,hidden)
			,B(0),T(0),Tsized(0)
{
}

void GraphicsWindow::statechanged (int s)
{	
	if (s==StandardWindow::iconic) textwindow.hide();
	else if (s==StandardWindow::normal) textwindow.show();
}

int count=0;

extern int systemscaling,sharper;

BitmapPS *WorkB=0;

/**
Redraw the window by copying the bitmap to it.
If the bitmap is invalid, redraw the bitmap first.
*/
void GraphicsWindow::redraw (PS &ps)
{	
	if (!B)
	{   
		sized(); return;
	}
	critical.enter();
	int gw,gh;
	if (intextwindow)
	{
		gw=textwindow.width();
		gh=textwindow.height();
	}
	else
	{
		gw=Width;
		gh=Height;
	}
	if (cropsquare)
	{
		int x=(gw-DW)/2,y=(gh-DH)/2;
		int w=DW,h=DH;
		if (x>0) ps.bar(0,0,x,gh,MyColor(0));
		if (x+w<gw-1) ps.bar(x+w,0,gw-(x+w),gh,MyColor(0));
		if (y>0) ps.bar(0,0,gw,y,MyColor(0));
		if (y+h<gh-1) ps.bar(0,y+h,gw,gh-(y+h),MyColor(0));
		if (antialiasgraphics && !systemscaling)
		{
			if (!WorkB) 
			{
				WorkB=new BitmapPS(&ps,w,h);
			}
			else if (WorkB->width()!=w || WorkB->height()!=h)
			{
				WorkB=new BitmapPS(&ps,w,h);
			}
			WorkB->copy3(*B,w,h,sharper);
			WorkB->copy(ps,x,y,w,h);
		}
		else B->stretch(ps,x,y,w,h);
	}
	else
	{
		int x=DF/2,y=DF/2;
		int w=gw-DF,h=gh-DF;
		if (x>0) ps.bar(0,0,x,gh,MyColor(0));
		if (x+w<gw-1) ps.bar(x+w,0,gw-(x+w),gh,MyColor(0));
		if (y>0) ps.bar(0,0,gw,y,MyColor(0));
		if (y+h<gh-1) ps.bar(0,y+h,gw,gh-(y+h),MyColor(0));
		if (antialiasgraphics) B->stretch(ps,x,y,w,h);
		else B->copy(ps,x,y);
	}
	critical.leave();
}

/*
Receive timer events.
1 : Timer to copy the current bitmap graphics to the window.
3 : Timer to repaint the resized window.
*/
void GraphicsWindow::timer (int n)
{   
	if (n==3) timedsized();
}

void GraphicsWindow::clear ()
{
	B->erase(*mycolors.c(0));
	StandardWindow::clear(*mycolors.c(0));
}

GraphicsWindow *graphicswindow;

// Functions to compute the width of the bitmap depending on
// anti-aliasing and the size of the graphcis window (or the
// text window, if the graphics goes into it).

extern int cropsquare;

int GraphicsWindow::adjwidth() 
{ 
	int w=Width;
	if (intextwindow) w=textwindow.width();
	return w*(antialiasgraphics==1?antialiasfactor:1); 
}

int GraphicsWindow::adjheight() 
{ 
	int h=Height;
	if (intextwindow) h=textwindow.height();
	return h*(antialiasgraphics==1?antialiasfactor:1); 
}

extern int gfontfixed;

/**
create a new bitmap, and paint the graphics to it.
*/
void GraphicsWindow::timedsized (int forced)
{   
	if (Tsized) delete Tsized;
	Tsized=0;
	BitmapPS *Bnew,*Bold;

	// frames.remove();

	int aw=adjwidth(),ah=adjheight();	
	int w=aw,h=ah;

	DF=rd(0.05*min(aw,ah));
	if (cropsquare || intextwindow) 
	{
		int D=min(rd((w-DF)/::aspect),h-DF);
		DH=D; DW=rd(D*::aspect);
		if (antialiasgraphics && !systemscaling)
		{
			DH=((DH+1)/3)*3;
			DW=((DW+1)/3)*3;
		}
		h=DH; w=DW;
	}
	if (antialiasgraphics) 
	{ 
		DW=DW/antialiasfactor; 
		DH=DH/antialiasfactor; 
		DF=DF/antialiasfactor; 
	}

	if (!forced && B && w==B->width() && h==B->height()) return;

	critical.enter();
	
	Bnew=new BitmapPS(*this,w,h);
	
	ops=(PS *)Bnew;
	Bold=B; B=Bnew;
	if (Bold) delete Bold;

	if (usecolors) B->erase();
	else B->bar(0,0,B->width(),B->height(),MyColor(0));

	if (couriersmall) delete couriersmall;

	Font *font=new Font(GraphicsFontName,
		max(B->width(),B->height())/gscreenlines,
		boldfont?Font::bold:Font::normal,gfontfixed);
	couriersmall=font;
	B->font(*couriersmall);

	couriersmallup=new Font(GraphicsFontName,
		max(B->width(),B->height())/gscreenlines,
		boldfont?Font::bold:Font::normal,gfontfixed,900);
    B->rememberfont(1,*couriersmallup);

	couriersmalldown=new Font(GraphicsFontName,
		max(B->width(),B->height())/gscreenlines,
		boldfont?Font::bold:Font::normal,gfontfixed,2700);
    B->rememberfont(2,*couriersmalldown);

	if (B->height()>0)
		hchar=(int)((double)B->textheight()/B->height()*1024);
	if (B->width()>0)
		wchar=(int)((double)B->textavewidth()/B->width()*1024);

	critical.leave();

	//graphicsreplay.start();
	doreplay();

}

int doreplay();
void endcrop();

void GraphicsWindow::getcr (int x, int y, double *c, double *r)
{
	if (intextwindow)
	{
		int gw=textwindow.width();
		int gh=textwindow.height();
		*c=(double)(x-(gw-DW)/2.0)/DW;
		*r=(double)(y-(gh-DH)/2.0)/DH;
	}
	else if (cropsquare)
	{
		*c=(double)(x-(Width-DW)/2.0)/DW;
		*r=(double)(y-(Height-DH)/2.0)/DH;
	}
	else
	{
		*c=(double)x/Width;
		*r=(double)y/Height;
	}
}

int statusalternatives=0,statusalternative=0;

void GraphicsWindow::clicked (int xm, int ym, int type)
/*
The user has clicked into the graphics window.
Handle only, if we are waiting for a mouse click!
*/
{	
	double x,y;
	getcr(xm,ym,&x,&y);
	if (cropmode)
	{	
		switch (type)
		{	
			case ClickType::button1down :
				cropleft=x; croptop=y;
				cropright=1-x; cropbottom=1-y;
				Dragging=1;
				break;
			case ClickType::mousemove :
				if (!Dragging) break;
				if (x<cropleft) cropleft=x;
				else cropright=1-x;
				if (y<croptop) croptop=y;
				else cropbottom=1-y;
				break;
			case ClickType::button1up :
				if (!Dragging) break;
				if (cropleft<0) cropleft=0;
				if (cropright<0) cropright=0;
				if (cropright+cropleft>1) cropright=cropleft=croptop=cropbottom=0;
				if (croptop<0) croptop=0;
				if (cropbottom<0) cropbottom=0;
				if (cropbottom+croptop>1) cropright=cropleft=croptop=cropbottom=0;
				Dragging=0;
				return;
		}
		doreplay();
		if (intextwindow) textwindow.update();
		else graphicswindow->update();
	}
	else if (mousewaiting==1 && type==ClickType::button1down)
	{	
		X=x*1024.0;
		Y=y*1024.0;
		Time=(long)(myclock()*1000);
		computation.resume();
	}
	else
	{	
		X=x*1024.0;
		Y=y*1024.0;
		Time=(long)(myclock()*1000);
		switch (type)
		{
			case ClickType::button1down :
				Type=1; Dragging=1;
				if (mousewaiting==2) computation.resume();
				break;
			case ClickType::mousemove :
				if (Dragging)
				{
					Type=2;
					if (mousewaiting==2) computation.resume();
				}
				else
				{
					if (mousewaiting==3) 
					{
						Type=4;
						computation.resume();
					}
				}
				break;
			case ClickType::button1up :
				Type=3; Dragging=0;
				if (mousewaiting==2) computation.resume();
				break;
		}
	}
}

