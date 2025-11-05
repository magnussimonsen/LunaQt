#include <time.h>
#include <string.h>
#include <io.h>
#include <math.h>

#include "svg.h"
#include "sysdep.h"
#include "header.h"

extern int fatterlines,boldfont;

char * SVGGraph::html (char *text)
{	
	static char out[MAXLINE];
	char *p=text,*q=out;
	while (*p!=0 && q-out<MAXLINE-20)
	{	
		switch (*p)
		{	case '>' : p++; *q++='&'; *q++='g'; *q++='t'; *q++=';'; break;
			case '<' : p++; *q++='&'; *q++='l'; *q++='t'; *q++=';'; break;
			case '&' : p++; *q++='&'; *q++='a'; *q++='m'; *q++='p'; *q++=';'; break;
			default : *q++=*p++;
		}
	}
	*q=0;
	return out;
}

double lw (double width)
{
	if (fatterlines) return width*1.5;
	else return width;
}

void SVGGraph::init ()
{	
	fprintf(out,"<?xml version=\"1.0\"?>\n");
	fprintf(out,"<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"");
	fprintf(out," \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");
	fprintf(out,"<svg %s=\"%s\" width=\"%d\" height=\"%d\">\n",
		"xmlns","http://www.w3.org/2000/svg",w,h);
	clip(0,0,1024,1024);
	nhatches=0;
}

SVGGraph::~SVGGraph ()
{	
	fprintf(out,"</svg>");
}

void SVGGraph::clear ()
{
	simplerectangle(0,0,1024,1024,0);
}

double cx,cy,cx1,cy1;
int cn=1;

void SVGGraph::clip (double c, double r, double c1, double r1)
{	
	if (cx==c && cy==r && cx1==c1 && cy1==r1) return;

	cn++;
	fprintf(out,"<clipPath id=\"clip%d\"><rect x=\"%0.2f\" y=\"%0.2f\""
		" width=\"%0.2f\" height=\"%0.2f\" /></clipPath>\n",
		cn,col(c),row(r),col(c1)-col(c),row(r1)-row(r));
	cx=c; cy=r; cx1=c1; cy1=r1;
}

void print_style (FILE *out, int style, int red, int green, int blue, double w)
{
	switch (style)
	{
		case line_dashed :
			fprintf(out," style=\"fill:none;stroke:rgb(%d,%d,%d);"
				"stroke-linecap:round;stroke-width:%0.2f;stroke-dasharray:6,10\"",
				red,green,blue,w);
			break;
		case line_dashdot :
			fprintf(out," style=\"fill:none;stroke:rgb(%d,%d,%d);"
				"stroke-linecap:round;stroke-width:%0.2f;stroke-dasharray:1,12,6,12\"",
				red,green,blue,w);
			break;
		case line_dashdotdash :
			fprintf(out," style=\"fill:none;stroke:rgb(%d,%d,%d);"
				"stroke-linecap:round;stroke-width:%0.2f;stroke-dasharray:6,12,1,12,6,12\"",
				red,green,blue,w);
			break;
		case line_dotdashdot :
			fprintf(out," style=\"fill:none;stroke:rgb(%d,%d,%d);"
				"stroke-linecap:round;stroke-width:%0.2f;stroke-dasharray:1,12,6,12,1,12\"",
				red,green,blue,w);
			break;
		case line_dotted :
			fprintf(out," style=\"fill:none;stroke:rgb(%d,%d,%d);stroke-linecap:round;"
				"stroke-width:%0.2f;stroke-dasharray:1,10\"",
				red,green,blue,w);
			break;
		default :
			fprintf(out," style=\"fill:none;stroke:rgb(%d,%d,%d);"
				"stroke-linecap:round;stroke-width:%0.2f\"",
				red,green,blue,w);
			break;
	}
}

void SVGGraph::line (double c, double r, double c1, double r1, 
					 int color, int style, double width)
{	
	if (style==line_none) return;

	double w=lw(width);

	fprintf(out,"<line x1=\"%0.2f\" y1=\"%0.2f\" x2=\"%0.2f\" y2=\"%0.2f\"",
		col(c),row(r),col(c1),row(r1));
	int red,green,blue;
	getcolor(color,&red,&green,&blue);
	print_style(out,style,red,green,blue,w);
	fprintf(out," clip-path=\"url(#clip%d)\"",cn);
	fprintf(out,"/>\n");

	if (style==line_arrow)
	{	
		double hx,hy,rr;
		hx=c1-c; hy=r1-r; rr=sqrt(hx*hx+hy*hy);
		if (rr>1)
		{	hx*=1024.0/markerfactor/rr;
			hy*=1024.0/markerfactor/rr;
			line(c1,r1,c1-hx-hy,r1-hy+hx,color,line_solid,width);
			line(c1,r1,c1-hx+hy,r1-hy-hx,color,line_solid,width);
		}	
	}
}


void SVGGraph::path (double c[], int n, int color, int style, double width)
{	
	if (style==line_none) return;

	if (style==line_arrow)
	{
		for (int i=0; i<n-1; i++)
			line(c[2*i],c[2*i+1],c[2*i+2],c[2*i+3],color,style,width);
		return;	
	}

	double w=lw(width);
	fprintf(out,"<polyline points=\"");
	for (int i=0; i<n; i++)
	{	fprintf(out,"%0.2f,%0.2f",col(c[2*i]),row(c[2*i+1]));
		if (i<n-1) fprintf(out," ");
	}
	fprintf(out,"\"");
	int red,green,blue;
	getcolor(color,&red,&green,&blue);
	print_style(out,style,red,green,blue,w);
	fprintf(out," clip-path=\"url(#clip%d)\"",cn);
	fprintf(out," />\n");
}

void SVGGraph::simplerectangle(double c, double r, double c1, double r1, int color)
{	
	if (c1<0) { c=c+c1; c1=-c1; }
	if (r1<0) { r=r+r1; r1=-r1; }
	fprintf(out,"<rect x=\"%0.2f\" y=\"%0.2f\" width=\"%0.2f\" height=\"%0.2f\"",
		c,r,c1,r1);
	int red,green,blue;
	getfillcolor(color,&red,&green,&blue);
	fprintf(out," style=\"fill:rgb(%d,%d,%d)\"",red,green,blue);
	fprintf(out,"/>\n");
}

void SVGGraph::simpleellipse(double x, double y, double rx, double ry, 
	int color, int filled)
{	
	double w=lw(1);
	fprintf(out,"<ellipse cx=\"%0.2f\" cy=\"%0.2f\" rx=\"%0.2f\" ry=\"%0.2f\"",
		col(x),row(y),col(rx),row(ry));
	int red,green,blue;
	if (filled) 
	{
		getcolor(color,&red,&green,&blue);
		fprintf(out," style=\"stroke:none;fill:rgb(%d,%d,%d)\"",red,green,blue);
	}
	else 
	{
		getfillcolor(color,&red,&green,&blue);
		fprintf(out," style=\"stroke:rgb(%d,%d,%d);stroke-width:%0.2f;fill:none\"",
			red,green,blue,w);
	}
	fprintf(out," clip-path=\"url(#clip%d)\"",cn);
	fprintf(out,"/>\n");
}

void SVGGraph::simpleline (double c, double r, double c1, double r1, int color)
{	
	double w=lw(1);
	fprintf(out,"<line x1=\"%0.2f\" y1=\"%0.2f\" x2=\"%0.2f\" y2=\"%0.2f\"",
		col(c),row(r),col(c1),row(r1));
	int red,green,blue;
	getcolor(color,&red,&green,&blue);
	fprintf(out," style=\"stroke:rgb(%d,%d,%d);stroke-width:%0.2f\"",
			red,green,blue,w);
	fprintf(out," clip-path=\"url(#clip%d)\"",cn);
	fprintf(out,"/>\n");
}

void SVGGraph::marker (double c, double r, int color, int type, double markerfactor)
{	
	double w=lw(1);
	double d=w*0.8;
	double size=1024/markerfactor;
	int dsize=(int)(size*1.4+0.5);
	double cp[8];
	switch (type)
	{	
		case marker_cross :
			simpleline(c-size,r-size,c+size,r+size,color);
			simpleline(c-size,r+size,c+size,r-size,color);
			break;
		case marker_circle :
			simpleellipse(c,r,size,size,color);
			break;			
		case marker_circle_hollow :
			simpleellipse(c,r,size,size,0,1);
			simpleellipse(c,r,size,size,color);
			break;			
		case marker_filledcircle :
			simpleellipse(c,r,size,size,color,1);
			break;			
		case marker_diamond : 
			simpleline(c-dsize,r,c,r+dsize,color);
			simpleline(c,r+dsize,c+dsize,r,color);
			simpleline(c+dsize,r,c,r-dsize,color);
			simpleline(c,r-dsize,c-dsize,r,color);
			break;
		case marker_diamond_hollow : 
			cp[0]=c-dsize; cp[1]=r;
			cp[2]=c; cp[3]=r+dsize;
			cp[4]=c+dsize; cp[5]=r;
			cp[6]=c; cp[7]=r-dsize;
			polygon(cp,4,bar_solid,0,0);
			simpleline(c-dsize,r,c,r+dsize,color);
			simpleline(c,r+dsize,c+dsize,r,color);
			simpleline(c+dsize,r,c,r-dsize,color);
			simpleline(c,r-dsize,c-dsize,r,color);
			break;
		case marker_filleddiamond : 
			dsize++;
			cp[0]=c-dsize; cp[1]=r;
			cp[2]=c; cp[3]=r+dsize;
			cp[4]=c+dsize; cp[5]=r;
			cp[6]=c; cp[7]=r-dsize;
			polygon(cp,4,bar_solid,color,0);
			break;
		case marker_dot :
			simplerectangle(col(c-d),row(r-d),
				col(c+d)-col(c-d),row(r+d)-row(r-d),color);
			break;
		case marker_largedot :
			size=1;
			simpleline(c-size,r-size,c+size,r-size,color);
			simpleline(c-size,r-size,c-size,r+size,color);
			simpleline(c+size,r+size,c+size,r-size,color);
			simpleline(c+size,r+size,c-size,r+size,color);
			break;
		case marker_filledlargedot :
			size=2;
			cp[0]=c-size; cp[1]=r-size;
			cp[2]=c-size; cp[3]=r+size;
			cp[4]=c+size; cp[5]=r+size;
			cp[6]=c+size; cp[7]=r-size;
			polygon(cp,4,bar_solid,color,0);
			break;
		case marker_plus :
			simpleline(c,r-size,c,r+size,color);
			simpleline(c-size,r,c+size,r,color);
			break;
		case marker_square :
			simpleline(c-size,r-size,c+size,r-size,color);
			simpleline(c-size,r-size,c-size,r+size,color);
			simpleline(c+size,r+size,c+size,r-size,color);
			simpleline(c+size,r+size,c-size,r+size,color);
			break;
		case marker_square_hollow :
			cp[0]=c-size; cp[1]=r-size;
			cp[2]=c-size; cp[3]=r+size;
			cp[4]=c+size; cp[5]=r+size;
			cp[6]=c+size; cp[7]=r-size;
			polygon(cp,4,bar_solid,0,0);
			simpleline(c-size,r-size,c+size,r-size,color);
			simpleline(c-size,r-size,c-size,r+size,color);
			simpleline(c+size,r+size,c+size,r-size,color);
			simpleline(c+size,r+size,c-size,r+size,color);
			break;
		case marker_filledsquare :
			size++;
			cp[0]=c-size; cp[1]=r-size;
			cp[2]=c-size; cp[3]=r+size;
			cp[4]=c+size; cp[5]=r+size;
			cp[6]=c+size; cp[7]=r-size;
			polygon(cp,4,bar_solid,color,0);
			break;
		case marker_star :
			simpleline(c-size,r-size,c+size,r+size,color);
			simpleline(c-size,r+size,c+size,r-size,color);
			simpleline(c,r-size,c,r+size,color);
			simpleline(c-size,r,c+size,r,color);
			break;
		case marker_vertical :
			simpleline(c,r-size,c,r+size,color);
			break;
		case marker_minus :
			simpleline(c-size,r,c+size,r,color);
			break;
	}
}

void SVGGraph::bar (double c, double r, double c1, double r1, 
					double hue, int color, int framed)
{	
	double w=lw(1);
	if (c1<c) { double h=c1; c1=c; c=h; }
	if (r1<r) { double h=r1; r1=r; r=h; }
	fprintf(out,"<rect x=\"%0.2f\" y=\"%0.2f\" width=\"%0.2f\" height=\"%0.2f\"",
		col(c),row(r),col(c1-c),row(r1-r));
	int red,green,blue;
	hue-=floor(hue);
	gethuecolor(color,&red,&green,&blue,hue);
	int red1,green1,blue1;
	getcolor(color,&red1,&green1,&blue1);
	if (framed)
		fprintf(out," style=\"fill:rgb(%d,%d,%d);stroke:rgb(%d,%d,%d);stroke-width:%0.2f\"",
			red,green,blue,red1,green1,blue1,w);
	else
		fprintf(out," style=\"fill:rgb(%d,%d,%d);stroke:rgb(%d,%d,%d);stroke-width:%0.2f\"",
			red,green,blue,red,green,blue,w);
	fprintf(out," clip-path=\"url(#clip%d)\"",cn);
	fprintf(out,"/>\n");
}

/*
<pattern id="hatch00" patternUnits="userSpaceOnUse"
                          x="0" y="0" width="10" height="10">
  <g style="fill:none; stroke:black; stroke-width:1">
    <path d="M0,0 l10,10"/>
    <path d="M10,0 l-10,10"/>
  </g>
</pattern>
*/

extern int scalelines,fontlines,gscreenlines,hatchgridsize;

double linewidth (int w)
{
	double factor=(double)w/(fatterlines?300:500);
	if (!scalelines || factor<1) factor=1;
	if (fontlines) factor=factor*(40.0/(gscreenlines+1));
	return factor;
}

int SVGGraph::create_hatch (int color, int style)
{	
	for (int i=0; i<nhatches; i++)
		if (hcolor[i]==color && hstyle[i]==style) return i;
	fprintf(out,"<pattern id=\"hatch%02d\" patternUnits=\"userSpaceOnUse\"",nhatches);
	int hg=(int)(hatchgridsize*linewidth(w)/2+0.5);
	if (hg%2==1) hg++;
	fprintf(out," x=\"0\" y=\"0\" width=\"%d\" height=\"%d\">\n",hg,hg);
	int red,green,blue;
	getcolor(color,&red,&green,&blue);
	fprintf(out," <g style=\"fill:none; stroke:rgb(%d,%d,%d); stroke-width:%d\">\n",
		red,green,blue,(int)(linewidth(w)+0.5));
	switch (style)
	{	case bar_hhatch :
			fprintf(out,"  <path d=\"M0,%d L%d,%d\"/>\n",hg/2,hg,hg/2);
			break;
		case bar_vhatch :
			fprintf(out,"  <path d=\"M%d,0 L%d,%d\"/>\n",hg/2,hg/2,hg);
			break;
		case bar_diagonal1 :
			fprintf(out,"  <path d=\"M%d,0 L0,%d\"/>\n",hg,hg);
			fprintf(out,"  <path d=\"M%d,%d L%d,%d\"/>\n",-hg,hg,hg,-hg);
			fprintf(out,"  <path d=\"M%d,0 L0,%d\"/>\n",2*hg,2*hg);
			break;
		case bar_diagonal2 :
			fprintf(out,"  <path d=\"M0,0 L%d,%d\"/>\n",hg,hg);
			fprintf(out,"  <path d=\"M0,%d L%d,%d\"/>\n",-hg,2*hg,hg);
			fprintf(out,"  <path d=\"M%d,0 L%d,%d\"/>\n",-hg,hg,2*hg);
			break;
		case bar_plus :
			fprintf(out,"  <path d=\"M0,%d L%d,%d\"/>\n",hg/2,hg,hg/2);
			fprintf(out,"  <path d=\"M%d,0 L%d,%d\"/>\n",hg/2,hg/2,hg);
			break;
		case bar_cross :
			fprintf(out,"  <path d=\"M%d,0 L0,%d\"/>\n",hg,hg);
			fprintf(out,"  <path d=\"M0,0 L%d,%d\"/>\n",hg,hg);
			break;	
	}
	fprintf(out," </g>\n</pattern>\n");
	if (nhatches<MAXHATCHES)
	{	hcolor[nhatches]=color; hstyle[nhatches]=style;
		nhatches++;
	}
	return nhatches-1;
}

void SVGGraph::bar1 (double c, double r, double c1, double r1, int color, int style)
{	
	double w=lw(1);
	int red0,green0,blue0;
	getcolor(0,&red0,&green0,&blue0);
	if (c1<c) { double h=c1; c1=c; c=h; }
	if (r1<r) { double h=r1; r1=r; r=h; }
	if (style!=bar_transparent && style!=bar_solid)
	{
		fprintf(out,"<rect x=\"%0.2f\" y=\"%0.2f\" width=\"%0.2f\" height=\"%0.2f\"",
			col(c),row(r),col(c1-c),row(r1-r));
		fprintf(out," style=\"fill:rgb(%d,%d,%d);stroke:none\"",
			red0,green0,blue0);	
		fprintf(out," clip-path=\"url(#clip%d)\"",cn);
		fprintf(out,"/>\n");
	}
	int hatch=create_hatch(color,style);
	fprintf(out,"<rect x=\"%0.2f\" y=\"%0.2f\" width=\"%0.2f\" height=\"%0.2f\"",
		col(c),row(r),col(c1-c),row(r1-r));
	int red,green,blue;
	getcolor(color,&red,&green,&blue);
	int red1,green1,blue1;
	getcolor(1,&red1,&green1,&blue1);
	switch (style)
	{	
		case bar_solid :
			fprintf(out," style=\"fill:rgb(%d,%d,%d);stroke:rgb(%d,%d,%d);stroke-width:%0.2f\"",
				red,green,blue,red,green,blue,w);
			break;
		case bar_frame :
			fprintf(out," style=\"fill:none;stroke:rgb(%d,%d,%d);stroke-width:%0.2f\"",
				red,green,blue,w);
			break;
		case bar_framed :
			fprintf(out," style=\"fill:rgb(%d,%d,%d);stroke:rgb(%d,%d,%d);stroke-width:%0.2f\"",
				red,green,blue,red1,green1,blue1,w);
			break;
		case bar_hhatch :
		case bar_vhatch :
		case bar_diagonal1 :
		case bar_diagonal2 :
		case bar_plus :
		case bar_cross :
			fprintf(out," style=\"fill:url(#hatch%02d);stroke:rgb(%d,%d,%d);stroke-width:%0.2f\"",
				hatch,red,green,blue,w);	
			break;	
		case bar_transparent :
			fprintf(out," style=\"fill:none;stroke:rgb(%d,%d,%d);stroke-width:%0.2f\"",
				red,green,blue,w);	
			break;	
		default :
			fprintf(out," style=\"fill:rgb(%d,%d,%d);stroke:none\"",
				red1,green1,blue1);
	}
	fprintf(out," clip-path=\"url(#clip%d)\"",cn);
	fprintf(out,"/>\n");
}

void SVGGraph::barRGB (double c1, double r1, double c2, double r2, int c, int r,
	int tred, int tgreen, int tblue, unsigned char *colors)
{	
	double w=lw(1)/2;
	double dx=(c2-c1)/c;
	double dy=(r2-r1)/r;
	for (int i=0; i<r; i++)
		for (int j=0; j<c; j++)
		{
			double x1=col(c1+j*dx), y1=row(r1+i*dy);
			double x2=col(c1+(j+1)*dx), y2=row(r1+(i+1)*dy);
			int red=*colors++,green=*colors++,blue=*colors++;
			if (red!=tred || green!=tgreen || blue!=tblue)
			{
				fprintf(out,"<rect x=\"%0.4f\" y=\"%0.4f\" width=\"%0.4f\" height=\"%0.4f\"",
					x1,y1,x2-x1,y2-y1);
				fprintf(out," style=\"fill:rgb(%d,%d,%d);stroke:rgb(%d,%d,%d);stroke-width:%0.2f\"",
					red,green,blue,red,green,blue,w);
				fprintf(out,"/>\n");
			}

		}
}

void SVGGraph::fill (double c[], int fillcolor, int wirecolor, int n, int connect[])
{
	fprintf(out,"<polygon points=\"");
	for (int i=0; i<n; i++)
	{	fprintf(out,"%0.2f,%0.2f",col(c[2*i]),row(c[2*i+1]));
		if (i<n-1) fprintf(out," ");
	}
	fprintf(out,"\"");
	int red,green,blue;
	getcolor(fillcolor,&red,&green,&blue);
	fprintf(out," style=\"fill:rgb(%d,%d,%d);stroke:rgb(%d,%d,%d);stroke-width:%0.2f\"",
		red,green,blue,red,green,blue,0.5);
	//fprintf(out," style=\"fill:rgb(%d,%d,%d)\"",
	//	red,green,blue);
	fprintf(out," />\n");
	for (int i=0; i<n-1; i++)
	{	if (connect[i])
		{	simpleline(c[2*i],c[2*i+1],c[2*i+2],c[2*i+3],
				wirecolor);
		}
	}
	if (connect[n-1])
	{	simpleline(c[0],c[1],c[2*(n-1)],c[2*(n-1)+1],
			wirecolor);
	}
}

void SVGGraph::fillh (double c[], int n, double hue, int color, int connect)
{	
	fprintf(out,"<polygon points=\"");
	for (int i=0; i<n; i++)
	{	fprintf(out,"%0.2f,%0.2f",col(c[2*i]),row(c[2*i+1]));
		if (i<n-1) fprintf(out," ");
	}
	fprintf(out,"\"");
	int red,green,blue;
	hue-=floor(hue);
	gethuecolor(color,&red,&green,&blue,hue);
	/*
	if (connect)
		fprintf(out," style=\"fill:rgb(%d,%d,%d);stroke:black;stroke-width:%0.2f\"",
			red,green,blue,w);
	else
	*/
		fprintf(out," style=\"fill:rgb(%d,%d,%d);stroke:rgb(%d,%d,%d);stroke-width:%0.2f\"",
			red,green,blue,red,green,blue,0.5);
	//	fprintf(out," style=\"fill:rgb(%d,%d,%d)\"",
	//		red,green,blue);
	fprintf(out," clip-path=\"url(#clip%d)\"",cn);
	fprintf(out," />\n");
}

#define rd(c) ((int)(c+0.5))
double svgfontfactor=7.0/8.0;

void SVGGraph::text (double c, double r, char *s, int color, int alignment)
{	
	int fontsize=rd(svgfontfactor*h/screenlines);
	fprintf(out,"<text x=\"%0.2f\" y=\"%0.2f\"",col(c),row(r)+fontsize);
	switch (alignment)
	{	case 0 : fprintf(out," text-anchor=\"begin\""); break;
		case 1 : fprintf(out," text-anchor=\"middle\""); break;
		case 2 : fprintf(out," text-anchor=\"end\""); break;
	}
	int red,green,blue;
	getcolor(color,&red,&green,&blue);
	fprintf(out," font-family=\"%s\"",font);
	fprintf(out," font-size=\"%d\"",fontsize);
	if (boldfont) fprintf(out," font-weight=\"bold\"");
	fprintf(out," fill=\"rgb(%d,%d,%d)\"",red,green,blue);
	fprintf(out,">");
	fprintf(out,html(s));
	fprintf(out,"</text>\n");
}

void SVGGraph::vtext (double c, double r, char *s, int color, int alignment)
{	
	int fontsize=rd(svgfontfactor*h/screenlines);
	fprintf(out,"<text x=\"%0.2f\" y=\"%0.2f\"",col(c)-fontsize,row(r));
	switch (alignment)
	{	case 0 : fprintf(out," text-anchor=\"begin\""); break;
		case 1 : fprintf(out," text-anchor=\"middle\""); break;
		case 2 : fprintf(out," text-anchor=\"end\""); break;
	}
	int red,green,blue;
	getcolor(color,&red,&green,&blue);
	fprintf(out," font-family=\"%s\"",font);
	fprintf(out," font-size=\"%d\"",fontsize);
	if (boldfont) fprintf(out," font-weight=\"bold\"");
	fprintf(out," fill=\"rgb(%d,%d,%d)\"",red,green,blue);
	fprintf(out," transform=\"rotate(90 %0.2f %0.2f)\"",col(c)-fontsize,row(r));
	fprintf(out,">");
	fprintf(out,html(s));
	fprintf(out,"</text>\n");
}

void SVGGraph::vutext (double c, double r, char *s, int color, int alignment)
{	
	int fontsize=rd(svgfontfactor*h/screenlines);
	fprintf(out,"<text x=\"%0.2f\" y=\"%0.2f\"",col(c),row(r)+fontsize);
	switch (alignment)
	{	case 0 : fprintf(out," text-anchor=\"begin\""); break;
		case 1 : fprintf(out," text-anchor=\"middle\""); break;
		case 2 : fprintf(out," text-anchor=\"end\""); break;
	}
	int red,green,blue;
	getcolor(color,&red,&green,&blue);
	fprintf(out," font-family=\"%s\"",font);
	fprintf(out," font-size=\"%d\"",fontsize);
	if (boldfont) fprintf(out," font-weight=\"bold\"");
	fprintf(out," fill=\"rgb(%d,%d,%d)\"",red,green,blue);
	fprintf(out," transform=\"rotate(270 %0.2f %0.2f)\"",col(c),row(r));
	fprintf(out,">");
	fprintf(out,html(s));
	fprintf(out,"</text>\n");
}

void SVGGraph::scale (double s)
{
}

void SVGGraph::polygon (double c[], int n, int style, int color, int connect)
{	
	double w=lw(1);
	int red,green,blue;
	int red1,green1,blue1;
	if (style!=bar_transparent && style!=bar_solid)
	{
		getcolor(0,&red,&green,&blue);
		fprintf(out,"<polygon points=\"");
		for (int i=0; i<n; i++)
		{	fprintf(out,"%0.2f,%0.2f",col(c[2*i]),row(c[2*i+1]));
			if (i<n-1) fprintf(out," ");
		}
		fprintf(out,"\"");
			fprintf(out," style=\"fill:rgb(%d,%d,%d);stroke:none\"",
				red,green,blue);
		fprintf(out," clip-path=\"url(#clip%d)\"",cn);
		fprintf(out," />\n");
	}
	int hatch=create_hatch(color,style);
	fprintf(out,"<polygon points=\"");
	for (int i=0; i<n; i++)
	{	fprintf(out,"%0.2f,%0.2f",col(c[2*i]),row(c[2*i+1]));
		if (i<n-1) fprintf(out," ");
	}
	fprintf(out,"\"");
	getcolor(color,&red,&green,&blue);
	switch (style)
	{	
		case bar_transparent :
			fprintf(out," style=\"fill:none;stroke:rgb(%d,%d,%d);stroke-width:%0.2f\"",
				red,green,blue,w);
			break;
		case bar_frame :
			getcolor(1,&red,&green,&blue);
			fprintf(out," style=\"fill:none;stroke:rgb(%d,%d,%d);stroke-width:%0.2f\"",
				red,green,blue,w);
			break;
		case bar_framed :
			getcolor(1,&red1,&green1,&blue1);
			fprintf(out," style=\"fill:rgb(%d,%d,%d);stroke:rgb(%d,%d,%d);stroke-width:%0.2f\"",
				red,green,blue,red1,green1,blue1,w);
			break;
		case bar_hhatch :
		case bar_vhatch :
		case bar_plus :
		case bar_diagonal1 :
		case bar_diagonal2 :
		case bar_cross :
			if (connect)
				fprintf(out," style=\"fill:url(#hatch%02d);stroke:rgb(%d,%d,%d);stroke-width:%0.2f\"",
					hatch,red,green,blue,w);
			else
				fprintf(out," style=\"fill:url(#hatch%02d);stroke:none\"",
					hatch);
			break;
		default :
			fprintf(out," style=\"fill:rgb(%d,%d,%d);stroke:none\"",
				red,green,blue);
	}
	fprintf(out," clip-path=\"url(#clip%d)\"",cn);
	fprintf(out," />\n");
}

double SVGGraph::row (double r)
{	return (r/1024)*h;
}

double SVGGraph::col (double c)
{	return (c/1024)*w;
}
