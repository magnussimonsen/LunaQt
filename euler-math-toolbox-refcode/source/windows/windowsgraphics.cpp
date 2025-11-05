#include "windows.h"
#include "colors.h"
#include "../sysdep.h"
#include "text.h"
#include "windowsgraphics.h"
#include "graphicswindow.h"
#include "../header.h"
#include "../sysdep.h"

extern int usecolors,scalelines,markerfactor,fillcolor1,fillcolor2;

//************* Output transforms **************************

// Compute the screen x-coordinate from an Euler coordinate.
// Euler coordinates go from 0 to 1024 (excluding).
int col (PS &ps, double x)
{	
	return (int)((double)ps.width()*x/1024.0);
}
int row (PS &ps, double y)
{	
	return (int)((double)ps.height()*y/1024.0);
}

/******************* Graphics ***************************/

extern PS *ops;

BitmapPS *hatchbitmap;

void gclear (PS &ps)
{	
	ps.erase(*mycolors.c(0));
	if (&ps==ops)
	{	
		if (cropleft>0) ps.bar(0,0,rd(cropleft*ps.width()),ps.height(),lightgray);
		if (cropright>0) ps.bar(rd(ps.width()*(1-cropright)),0,
			rd(cropright*ps.width()),ps.height(),lightgray);
		if (croptop>0) ps.bar(0,0,ps.width(),rd(croptop*ps.height()),lightgray);
		if (cropbottom>0) ps.bar(0,rd(ps.height()*(1-cropbottom)),
			ps.width(),rd(cropbottom*ps.height()),lightgray);
	}
}

extern int fatterlines,fontlines,gscreenlines,linux;

double linewidth (int w, double width, int thinner)
{
	double factor=(double)w/(fatterlines?300:500);
	if (!scalelines || factor<1) factor=1;
	if (fontlines) factor=factor*(40.0/(gscreenlines+1));
	return width*factor;
}
double linewidth (PS &ops, double width, int thinner)
{
	return linewidth(ops.width(),width,thinner);
}

void gline (PS &ops, double c, double r, double c1, double r1, int color,
	int st, double width)
/***** gline
	draw a line.
	col is the color, where 0 should be white and 1 black.
	st is a style from linetyp.
	width is the linewidth, where 0 or 1 are equal defaults.
*****/
{   
	if (!usecolors) color=1;
	int w=rd(linewidth(ops.width(),width));
	Pen *pen=0;
	switch (st)
	{	case line_dotted :
			pen=new Pen(Pen::dot,w,MyColor(color)); break;
		case line_dashed :
			pen=new Pen(Pen::dash,w,MyColor(color)); break;
		case line_dashdot :
			pen=new Pen(Pen::dashdot,w,MyColor(color)); break;
		case line_dashdotdash :
			pen=new Pen(Pen::dashdotdash,w,MyColor(color)); break;
		case line_dotdashdot :
			pen=new Pen(Pen::dotdashdot,w,MyColor(color)); break;
		case line_none : return;
		default :
			pen=new Pen(Pen::solid,w,MyColor(color)); break;
		
	}
	if (!pen) return;
	int col0=col(ops,c),row0=row(ops,r),
		col1=col(ops,c1),row1=row(ops,r1);
	ops.line(col0,row0,col1,row1,*pen);
	if (st==line_arrow)
	{	
		double hx,hy,rr;
		hx=c1-c; hy=r1-r; rr=sqrt(hx*hx+hy*hy);
		if (rr>1)
		{	hx*=1024.0/markerfactor/rr;
			hy*=1024.0/markerfactor/rr;
			ops.line(col(ops,c1),row(ops,r1),
				col(ops,c1-hx-hy),row(ops,r1-hy+hx),*pen);
			ops.line(col(ops,c1),row(ops,r1),
				col(ops,c1-hx+hy),row(ops,r1-hy-hx),*pen);
		}
	}
	pen->remove();
	delete pen;
}

void gclip (PS &ops, double c, double r, double c1, double r1)
/***** gclip
	clip graphics to a specified region.
**********/
{	ops.clip(col(ops,c),row(ops,r),col(ops,c1-c)+2,row(ops,r1-r)+2);
}

void gmarker (PS &ops, double c, double r, int color, int type, double markerfactor)
/***** gmarker
	plot a single marker on screen.
	col is the color.
	type is a type from markertyp.
*****/
{	
	int t=Markers::square;
	if (!usecolors) color=1;

	double factor=ops.width()/500;
	if (!scalelines || factor<1) factor=1;

	ops.markersize(rd((double)ops.width()/markerfactor));
	double w=linewidth(ops);

	switch (type)
	{	case marker_cross : t=Markers::cross; break;
		case marker_circle : t=Markers::circle; break;
		case marker_circle_hollow : 
			ops.color(MyColor(0));
			ops.mark(col(ops,c),row(ops,r),Markers::filledcircle,rd(w));
			t=Markers::circle; break;
		case marker_filledcircle : t=Markers::filledcircle; break;
		case marker_diamond : t=Markers::diamond; break;
		case marker_diamond_hollow : 
			ops.color(MyColor(0));
			ops.mark(col(ops,c),row(ops,r),Markers::filleddiamond,rd(w));
			t=Markers::diamond; break;
		case marker_filleddiamond : t=Markers::filleddiamond; break;
		case marker_dot : t=Markers::dot; break;
		case marker_largedot : t=Markers::dot; w=w*2; break;
		case marker_filledlargedot : t=Markers::dot; w=w*3; break;
		case marker_plus : t=Markers::plus; break;
		case marker_square : t=Markers::square; break;
		case marker_square_hollow : 
			ops.color(MyColor(0));
			ops.mark(col(ops,c),row(ops,r),Markers::filledsquare,rd(w));
			t=Markers::square; break;
		case marker_filledsquare : t=Markers::filledsquare; break;
		case marker_star : t=Markers::star; break;
		case marker_vertical : t=Markers::vertical; break;
		case marker_minus : t=Markers::minus; break;
	}
	ops.color(MyColor(color));
	int ww=rd(w); if (ww<1) ww=1;
	ops.mark(col(ops,c),row(ops,r),t,ww);
}

void gfill (PS &ops, double c[], int fillcolor, int wirecolor, int n, int connect[])
/***** gfill
	fill an area given by n pairs of points (in c: x,y,x,y,...) with the fillcolor.
	connect pairs of points indicated in connect by a line with the wirecolor.
*****/
{	
	int i;
	static Points p(5); // initally place for 5 points. can grow.
	p.empty();
	for (i=0; i<n; i++)
	{	
		p.add(col(ops,c[2*i]),row(ops,c[2*i+1]));
	}
	p.add(col(ops,c[0]),row(ops,c[1]));
	ops.fillpolygon(p,MyColor(fillcolor,1));
	Pen *pen=new Pen(Pen::solid,rd(linewidth(ops,1,1)),MyColor(wirecolor));
	for (i=0; i<n; i++)
	{	
		if (connect[i])
		{	
			ops.line(p.x(i),p.y(i),p.x(i+1),p.y(i+1),*pen);
		}
	}
	pen->remove();
	delete pen;
	ops.color(MyColor(1));
}

Points gpoints(MAXPOLYGON);

void gpolygon (PS &ops, double c[], int n, int style, int color, int connect)
/***** Draw a filled polygon.
	style is the fill style, color is the fill color.
	connect determines, if an outline should be drawn.
******/
{	
	int i;
	gpoints.empty();
	for (i=0; i<n; i++)
	{	
		gpoints.add(col(ops,c[2*i]),row(ops,c[2*i+1]),1);
	}
	gpoints.add(col(ops,c[0]),row(ops,c[1]));
	Color col;
	col=MyColor(color);
	Brush *b;
	Pen *pen;
	switch (style)
	{	
		case bar_solid :
		case bar_framed :
			b=new SolidBrush(MyColor(color,1)); 
			ops.fillpolygon(gpoints,b);
			b->remove();
			delete b;
			break;

		case bar_frame :
			b=new SolidBrush(MyColor(0,1)); 
			ops.color(col);
			ops.fillpolygon(gpoints,b);
			b->remove();
			delete b;
			break;

		case bar_hhatch :
			hatchbitmap->erase();
			pen=new Pen(Pen::solid,rd(linewidth(ops,1)),col);
			hatchbitmap->line(0,hatchbitmap->height()/2,
				hatchbitmap->width(),hatchbitmap->height()/2,*pen);
			pen->remove();
			delete pen ;
			goto draw;
		
		case bar_vhatch :
			hatchbitmap->erase();
			pen=new Pen(Pen::solid,rd(linewidth(ops,1)),col);
			hatchbitmap->line(hatchbitmap->width()/2,0,
				hatchbitmap->width()/2,hatchbitmap->height(),*pen);
			pen->remove();
			delete pen;
			goto draw;

		case bar_diagonal2 :
			hatchbitmap->erase();
			pen=new Pen(Pen::solid,rd(linewidth(ops,1)),col);
			hatchbitmap->line(0,0,hatchbitmap->width(),hatchbitmap->height(),*pen);
			hatchbitmap->line(0,-hatchbitmap->width(),
				2*hatchbitmap->width(),hatchbitmap->height(),*pen);
			hatchbitmap->line(-hatchbitmap->width(),0,
				hatchbitmap->width(),2*hatchbitmap->height(),*pen);
			pen->remove();
			delete pen;
			goto draw;

		case bar_diagonal1 :
			hatchbitmap->erase();
			pen=new Pen(Pen::solid,rd(linewidth(ops,1)),col);
			hatchbitmap->line(0,hatchbitmap->height(),hatchbitmap->width(),0,*pen);
			hatchbitmap->line(hatchbitmap->width(),-hatchbitmap->width(),
				-hatchbitmap->width(),hatchbitmap->height(),*pen);
			hatchbitmap->line(2*hatchbitmap->width(),0,
				0,2*hatchbitmap->height(),*pen);
			pen->remove();
			delete pen;
			goto draw;

		case bar_plus :
			hatchbitmap->erase();
			pen=new Pen(Pen::solid,rd(linewidth(ops,1)),col);
			hatchbitmap->line(hatchbitmap->width()/2,0,
				hatchbitmap->width()/2,hatchbitmap->height(),*pen);
			hatchbitmap->line(0,hatchbitmap->height()/2,
				hatchbitmap->width(),hatchbitmap->height()/2,*pen);
			pen->remove();
			delete pen;
			goto draw;

		case bar_cross :
			hatchbitmap->erase();
			pen=new Pen(Pen::solid,rd(linewidth(ops,1)),col);
			hatchbitmap->line(0,0,hatchbitmap->width(),hatchbitmap->height(),*pen);
			hatchbitmap->line(0,hatchbitmap->height(),hatchbitmap->width(),0,*pen);
			pen->remove();
			delete pen;

			draw :
			b=new HatchBrush(hatchbitmap);
			ops.fillpolygon(gpoints,b);
			b->remove();
			delete b;
			break;
	}
	if (connect)
	{	
		ops.color(MyColor(connect));
		Pen *pen=new Pen(Pen::solid,rd(linewidth(ops,1)),
			MyColor(style==bar_framed||style==bar_frame?1:color));
		for (i=0; i<gpoints.size()-1; i++)
		{	ops.line(gpoints.x(i),gpoints.y(i),gpoints.x(i+1),gpoints.y(i+1),*pen);
		}
		pen->remove();
		delete pen;
	}
}

void gpath (PS &ops, double c[], int n, int color, int style, double width)
/***** Draw a path.
	style is the path style.
******/
{	
	if (style==line_arrow)
	{
		for (int i=0; i<n-1; i++)
			gline(ops,c[2*i],c[2*i+1],c[2*i+2],c[2*i+3],color,style,width);
		return;
	}

	int w=rd(linewidth(ops,width));

	int i;
	if (!usecolors) color=1;

	gpoints.empty();
	for (i=0; i<n; i++)
	{	
		gpoints.add(col(ops,c[2*i]),row(ops,c[2*i+1]),1);
	}

	Pen *pen=0;
	switch (style)
	{	case line_dotted :
			pen=new Pen(Pen::dot,w,MyColor(color)); break;
		case line_dashed :
			pen=new Pen(Pen::dash,w,MyColor(color)); break;
		case line_dashdot :
			pen=new Pen(Pen::dashdot,w,MyColor(color)); break;
		case line_dashdotdash :
			pen=new Pen(Pen::dashdotdash,w,MyColor(color)); break;
		case line_dotdashdot :
			pen=new Pen(Pen::dotdashdot,w,MyColor(color)); break;
		case line_none : return;
		default :
			pen=new Pen(Pen::solid,w,MyColor(color,1)); break;
		
	}
	if (!pen) return;

	ops.path(gpoints.points(),gpoints.size(),*pen);
	pen->remove();
	delete pen;
}

void gfillh (PS &ops, double c[], int n, double hue, int color, int connect)
/***** Draw a filled polygon.
	Works like gfill, but uses hue.
*****/
{	
	int i;
	static Points p(32);
	p.empty();
	for (i=0; i<n; i++)
	{	
		p.add(col(ops,c[2*i]),row(ops,c[2*i+1]));
	}
	p.add(col(ops,c[0]),row(ops,c[1]));
	if (connect)
		ops.polygon(p,MyColor(color,hue),MyColor(1));
	else
    	ops.fillpolygon(p,MyColor(color,hue));
	ops.color(MyColor(1));
}

void gbar (PS &ops, double c, double r, double c1, double r1, double hue,
	int color, int style)
/***** Draw a rectangle.
	hue is a hue intensity from 0 to 1.
	style determines, if a black boundary should be drawn.
******/
{	
	int x=col(ops,c),y=row(ops,r);
	int w=col(ops,c1)-x+1,h=row(ops,r1)-y+1;
	Color col;
	col=MyColor(color,hue);
	ops.bar(x,y,w,h,col);
    if (style!=0)
	{	
		Pen *pen=new Pen(Pen::solid,rd(linewidth(ops,1)),col);
	 	ops.rectangle(x,y,w,h,*pen);
		pen->remove();
		delete pen;
    }
}

void gbar1 (PS &ops, double c, double r, double c1, double r1,
	int color, int style)
/***** Draw a rectangle.
	style determines, if a black boundary should be drawn.
******/
{	
	int x=col(ops,c),y=row(ops,r);
	int w=col(ops,c1)-x+1,h=row(ops,r1)-y+1;
	Color col;
	col=MyColor(color);
	Brush *b;
	Pen *pen;
	switch (style)
	{	
		case bar_solid :
			ops.bar(x,y,w,h,MyColor(color,1)); 
			break;

		case bar_framed :
			if (!usecolors) goto hatch;
			ops.bar(x,y,w,h,MyColor(color,1));
			pen=new Pen(Pen::solid,rd(linewidth(ops,1)),MyColor(1));
			ops.rectangle(x,y,w,h,*pen);
			pen->remove();
			delete(pen);
			break;

		case bar_frame :
			if (!usecolors) goto hatch;
			ops.bar(x,y,w,h,MyColor(0));
			pen=new Pen(Pen::solid,rd(linewidth(ops,1)),MyColor(color));
			ops.rectangle(x,y,w,h,*pen);
			pen->remove();
			delete(pen);
			break;

		case bar_transparent :
			pen=new Pen(Pen::solid,rd(linewidth(ops,1)),col);
			ops.rectangle(x,y,w,h,*pen);
			pen->remove();
			delete(pen);
			break;

		case bar_hhatch :
			hatchbitmap->erase();
			pen=new Pen(Pen::solid,rd(linewidth(ops,1)),col);
			hatchbitmap->line(0,hatchbitmap->height()/2,
				hatchbitmap->width(),hatchbitmap->height()/2,*pen);
			delete(pen);
			b=new HatchBrush(hatchbitmap);
			goto draw;
		
		case bar_vhatch :
			hatchbitmap->erase();
			pen=new Pen(Pen::solid,rd(linewidth(ops,1)),col);
			hatchbitmap->line(hatchbitmap->width()/2,0,
				hatchbitmap->width()/2,hatchbitmap->height(),*pen);
			pen->remove();
			delete(pen);
			b=new HatchBrush(hatchbitmap);
			goto draw;

		case bar_diagonal2 :
			hatchbitmap->erase();
			pen=new Pen(Pen::solid,rd(linewidth(ops,1)),col);
			hatchbitmap->line(0,0,hatchbitmap->width(),hatchbitmap->height(),*pen);
			hatchbitmap->line(0,-hatchbitmap->width(),
				2*hatchbitmap->width(),hatchbitmap->height(),*pen);
			hatchbitmap->line(-hatchbitmap->width(),0,
				hatchbitmap->width(),2*hatchbitmap->height(),*pen);
			pen->remove();
			delete(pen);
			b=new HatchBrush(hatchbitmap);
			goto draw;

		case bar_diagonal1 :
			hatchbitmap->erase();
			pen=new Pen(Pen::solid,rd(linewidth(ops,1)),col);
			hatchbitmap->line(0,hatchbitmap->height(),hatchbitmap->width(),0,*pen);
			hatchbitmap->line(hatchbitmap->width(),-hatchbitmap->width(),
				-hatchbitmap->width(),hatchbitmap->height(),*pen);
			hatchbitmap->line(2*hatchbitmap->width(),0,
				0,2*hatchbitmap->height(),*pen);
			pen->remove();
			delete(pen);
			b=new HatchBrush(hatchbitmap);
			goto draw;

		case bar_plus :
			hatchbitmap->erase();
			pen=new Pen(Pen::solid,rd(linewidth(ops,1)),col);
			hatchbitmap->line(hatchbitmap->width()/2,0,
				hatchbitmap->width()/2,hatchbitmap->height(),*pen);
			hatchbitmap->line(0,hatchbitmap->height()/2,
				hatchbitmap->width(),hatchbitmap->height()/2,*pen);
			pen->remove();
			delete(pen);
			b=new HatchBrush(hatchbitmap);
			goto draw;

		case bar_cross :
			hatch :
			
			hatchbitmap->erase();
			pen=new Pen(Pen::solid,rd(linewidth(ops,1)),col);
			hatchbitmap->line(0,0,hatchbitmap->width(),hatchbitmap->height(),*pen);
			hatchbitmap->line(0,hatchbitmap->height(),hatchbitmap->width(),0,*pen);
			pen->remove();
			delete(pen);
			b=new HatchBrush(hatchbitmap);

			draw :
			ops.backmode(PS::opaque);
			if (!usecolors) ops.backcolor(MyColor(0));
			ops.bar(x,y,w,h,*b);
			pen=new Pen(Pen::solid,rd(linewidth(ops,1)),col);
			ops.rectangle(x,y,w,h,*pen);
			pen->remove();
			delete pen;
			ops.backmode(PS::transparent);
			if (!usecolors) ops.backcolor(Color::background());
			b->remove();
			delete b;
			break;
	}
}

void gbarRGB (PS &ops, double c1, double r1, double c2, double r2, int cn, int rn,
	int tred, int tgreen, int tblue, unsigned char *colors)
/***** Draw a rectangle.
	in colour defined by the red, green and blue values.	
******/
//
//  Created by Ron Larham August 2006
//

{	int i,j;
	double dx=(c2-c1)/cn;
	double dy=(r2-r1)/rn;
	for (i=0; i<rn; i++)
		for (j=0; j<cn; j++)
		{	
			int x1=col(ops,c1+j*dx), y1=row(ops,r1+i*dy);
			int x2=col(ops,c1+(j+1)*dx), y2=row(ops,r1+(i+1)*dy);
			int red=*colors++,green=*colors++,blue=*colors++;
			if (red!=tred || green!=tgreen || blue!=tblue)
				ops.bar(x1,y1,x2-x1+1,y2-y1+1,RGB(red,green,blue));
		}
}

void gtext (PS &ops, double c, double r, char *text, int color, int alignment, double size)
/***** gtext
	output a graphic text on screen.
	alignment is left=0, centered=1, right=2.
*****/
{	
	int a;
	if (!usecolors) color=1;
	switch (alignment)
	{	case 0 : a=Alignment::left; break;
		case 1 : a=Alignment::center; break;
		case 2 : a=Alignment::right; break;
		default : a=Alignment::left; break;
	}
	ops.textcolor(MyColor(color));
	ops.text(col(ops,c),row(ops,r),text,a,Alignment::top);
}

void gvtext (PS &ops, double c, double r, char *text, int color, int alignment, double size)
/***** gtext
	output a graphic text on screen.
	alignment is left=0, centered=1, right=2.
*****/
{	
	int a;
	if (!usecolors) color=1;
	switch (alignment)
	{	case 0 : a=Alignment::left; break;
		case 1 : a=Alignment::center; break;
		case 2 : a=Alignment::right; break;
		default : a=Alignment::left; break;
	}
    ops.font(2);
	ops.textcolor(MyColor(color));
	ops.text(col(ops,c),row(ops,r),text,a,Alignment::top);
    ops.font(0);
}

void gvutext (PS &ops, double c, double r, char *text, int color, int alignment, double size)
/***** gtext
	output a graphic text on screen.
	alignment is left=0, centered=1, right=2.
*****/
{	
	int a;
	if (!usecolors) color=1;
	switch (alignment)
	{	case 0 : a=Alignment::left; break;
		case 1 : a=Alignment::center; break;
		case 2 : a=Alignment::right; break;
		default : a=Alignment::left; break;
	}
    ops.font(1);
	ops.textcolor(MyColor(color));
	ops.text(col(ops,c),row(ops,r),text,a,Alignment::top);
    ops.font(0);
}