#ifndef SVG_H
#define SVG_H

#include <stdio.h>

#include "sysdep.h"

#define MAXHATCHES 256

class SVGGraph 
{
	double color[16][3];
	int fillcolor1,fillcolor2;
	int screenlines;
	int usecolor;
	int markerfactor;
	
	int clipped;
	
	char clipstring[256];

	int lineop, lcolor, lstyle, lwidth;
	double x, y;
	
	FILE *out;
	int w,h;
	
	int hcolor[MAXHATCHES],hstyle[MAXHATCHES];
	int nhatches;
	char *font;
	
public:
	SVGGraph(FILE *output, int width, int height) : out(output),w(width),h(height) {}
	~SVGGraph();

	void init ();
	void setcolor (int i, double r, double g, double b)
	{	color[i][0]=r; color[i][1]=g; color[i][2]=b;
	}
	void setfillcolors (int i1, int i2)
    {	fillcolor1=i1; fillcolor2=i2;
    }
    void setscreenlines (char *fontname, int n)
    {	
		font=fontname;
		screenlines = n;
    }
    void setusecolor (int f)
    {	usecolor=f;
    }
    void setmarkerfactor (int f)
    {	markerfactor=f;
    }

	char *html (char *text);

	void clear ();
	void clip (double c, double r, double c1, double r1);
	void line(double c, double r, double c1, double r1, int color, int style, double width);
	void path(double c[], int n, int color, int style, double width);
	void lineto(double c, double r, int color, int style, int width);
	void linestroke(int color, int style, int width);
	
	void simpleline(double c, double r, double c1, double r1, int color);
	void simplerectangle(double c, double r, double w, double h, int color);
	void simpleellipse(double x, double y, double rx, double ry, int color, int filled=0);

	void marker(double c, double r, int color, int type, double size);
	
	void bar(double c, double r, double c1, double r1, double hue, int color, int framed);
	void bar1(double c, double r, double c1, double r1, int color, int framed);
	void barRGB(double c1, double r1, double c2, double r2, int c, int r,
		int tred, int tgreen, int tblue, unsigned char *colors);
	void fillh (double c[], int n, double hue, int color, int connect);
	void fill (double c[], int fillcolor, int wirecolor, int n, int connect[]);
	
	void text(double c, double r, char *s, int color, int just);
	void vtext(double c, double r, char *s, int color, int just);
	void vutext(double c, double r, char *s, int color, int just);
    	
	void polygon(double c[], int n, int style, int color, int connect);

	void scale(double s);
	
	double row (double r);
	double col (double c);
	
	int create_hatch (int color, int style);
};

#endif