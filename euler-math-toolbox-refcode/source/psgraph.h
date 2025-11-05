/*************************************************************************
 *	PSGraph
 *
 *	ps output for euler meta file
 *
 *	Eric Boucharé, 08/08/1999
 *
 *************************************************************************/

#ifndef PSGRAPH_H
#define PSGRAPH_H

#include <stdio.h>
#include "sysdep.h"

class PSGraph {
	double color[16][3];
	int fillcolor1,fillcolor2;
	int screenlines;
	int usecolor;
	
	int clipped;

	int lineop, lcolor, lstyle, lwidth;
	double x, y;
public:
	PSGraph(FILE *output, int width, int height);
	~PSGraph();

	void init ();
	void setcolor (int i, double r, double g, double b)
	{	color[i][0]=r; color[i][1]=g; color[i][2]=b;
	}
	void setfillcolors (int i1, int i2)
    {	fillcolor1=i1; fillcolor2=i2;
    }
    void setscreenlines (int n)
    {	screenlines = n;
    }
    void setusecolor (int f)
    {	usecolor=f;
    }

	void clear ();
	void clip (double c, double r, double c1, double r1);
	void	line(double c, double r, double c1, double r1, int color, int style, double width);
	void	path(double c[], int n, int color, int style, double width);
	void	lineto(double c, double r, int color, int style, int width);
	void	linestroke(int color, int style, int width);
	
	void	marker(double c, double r, int color, int type, double markerfactor);
	
	void	bar(double c, double r, double c1, double r1, double hue, int color, int framed);
	void	bar1(double c, double r, double c1, double r1, int color, int framed);
	void fillh (double c[], int n, double hue, int color, int connect);
	void fill (double c[], int fillcolor, int wirecolor, int n, int connect[]);
	
	void	text(double c, double r, char *s, int color, int just);
	void	vtext(double c, double r, char *s, int color, int just);
	void	vutext(double c, double r, char *s, int color, int just);
    	
	void	scale(double s);
	
private:
	int	scantext(char c);
	void	lineend(int color, int style, int width);
	double col (double c) {return (double)(c*pswidth)/1024.0;};
	double row (double r) {return (double)(r*psheight)/1024.0;};

protected:
	FILE *	out;
	int		pswidth, psheight;
	struct {
		int	sethuecolor:1;
		int	setclip:1;
		int	linesolid:1;
		int	linedotted:1;
		int	linedashed:1;
		int	linearrow:1;
		int	markercross:1;
		int	markercircle:1;
		int	markerdiamond:1;
		int	markerdot:1;
		int	markerplus:1;
		int	markersquare:1;
		int	markerstar:1;
		int	bar:1;
		int	fbar:1;
		int	bar1:1;
		int	fbar1:1;
		int	fbar2:1;
		int	fb1d1:1;
		int	fb1d2:1;
		int	fb1c:1;
		int	fill:1;
		int	fillh:1;
		int	ffillh:1;
		int	reencode:1;
		int	text:1;
		int	rtext:1;
		int	ctext:1;
	} firstuse;
};

#endif