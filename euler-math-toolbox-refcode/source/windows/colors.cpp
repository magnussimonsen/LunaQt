#include "windows.h"
#include "colors.h"

MyColors mycolors;

//******************** Color ******************

// The 16 Euler colors
int red[16]=  {255, 0,144, 16, 16, 50,150,180,120, 80,240,100,100,100,240,240};
int green[16]={255, 0, 16,144, 16,150,150,180,120, 80,130,240,150,240,180,240};
int blue[16]= {255, 0, 16, 16,144,150, 50,180,120, 80,100,100,240,240,100,100};

// compute the percentage of red, green or blue for a color
double getcolor (int i, int j)
{	
	int r,g,b;
	getcolor(i,&r,&g,&b);

	switch (j)
	{	case 0 : return r/256.0;
    	case 1 : return g/256.0;
        case 2 : return b/256.0;
    }
    return 0;
}

// initiealze with the default colors
MyColors::MyColors ()
{	
	for (int i=0; i<16; i++)
		C[i]=new Color(red[i],green[i],blue[i]);
}

// destructor
MyColors::~MyColors ()
{	
	for (int i=0; i<16; i++) delete C[i];
}

void getmycolor (int i, int *red, int *green, int *blue);

// get the color, and by default the black color
Color MyColors::color (int i)
{
	int red,green,blue;
	if (i>=0 && i<16)
	{
		red=c(i)->getRed();
		blue=c(i)->getBlue();
		green=c(i)->getGreen();
	}
	else
	{
		getmycolor(i,&red,&green,&blue);
	}
	act.set(red,green,blue);
	return act;
}

Color * MyColors::c (int i)
{
	return C[i];
}

// get the color, and by default the black color
Color MyColors::fillcolor (int i)
{
	int red,green,blue;
	getfillcolor(i,&red,&green,&blue);
	act.set(red,green,blue);
	return act;
}

extern int usecolors;

/**
Get the red, green, blue part of color i.
The parts are intetegers from 0 to 255.
The color can be a rgb-int or a pre-defined color.
An rgb-int is a positive integer >0x01000000.
*/
void getcolor (int i, int *r, int *g, int *b)
{
	if (i>=0x01000000)
	{	
		if (!usecolors)
		{
			*r=*g=*b=1;
		}
		else
		{
			*r=(i&0x00FF0000)>>16;
			*g=(i&0x0000FF00)>>8;
			*b=(i&0x000000FF);
		}
	}
	else
	{	
		if (!usecolors)
		{
			if (i==0)
				*r=*g=*b=255;
			else
				*r=*g=*b=0;
		}
		else
		{
			if (i>15 || i<0) i=1;
			Color *c=mycolors.c(i);
			*r=c->getRed();
			*b=c->getBlue();
			*g=c->getGreen();
		}
	}
}

/**
Compute an rgb-color from the red, green, blue values.
The values must be between 0 and 255.
*/
int getcolor (int r, int g, int b)
{
	int k=1;
	k=(k<<8)|(r&0x000000FF);
	k=(k<<8)|(g&0x000000FF);
	k=(k<<8)|(b&0x000000FF);
	return k;
}

void getmycolor (int i, int *r, int *g, int *b)
{
	if (i>15 || i<0) i=1;
	Color *c=mycolors.c(i);
	*r=c->getRed();
	*b=c->getBlue();
	*g=c->getGreen();
}

void getfillcolor (int i, int *r, int *g, int *b)
{
	if (i>=0x01000000)
	{	
		*r=(i&0x00FF0000)>>16;
		*g=(i&0x0000FF00)>>8;
		*b=(i&0x000000FF);
	}
	else
	{	if (i>15 || i<0) i=1;
		Color *c=mycolors.c(i);
		*r=c->getRed();
		*b=c->getBlue();
		*g=c->getGreen();
	}
	if (!usecolors)
	{
		int h=(*r+*g+*b)/3;
		h=(255+2*h)/3;
		*r=*g=*b=h;
	}
}

// compute read, green and blue from hue and saturation
static void hsv_to_rgb (double h,double s,double v,
						double *r, double *g,double *b)
{
	int i;
	double f, w, q, t;

	if (s == 0.0) s = 0.000001;

	if (h == -1.0) {
		*r = v;
		*g = v;
		*b = v;
	} else {
		if (h == 360.0) h = 0.0;
		h = h / 60.0;
		i = (int)h;
		f = h - i;
		w = v * (1.0 - s);
		q = v * (1.0 - (s * f));
		t = v * (1.0 - (s * (1.0 - f)));

		switch (i) {
			case 0:
				*r = v;
				*g = t;
				*b = w;
				break;
			case 1:
				*r = q;
				*g = v;
				*b = w;
				break;
			case 2:
				*r = w;
				*g = v;
				*b = t;
				break;
			case 3:
				*r = w;
				*g = q;
				*b = v;
				break;
			case 4:
				*r = t;
				*g = w;
				*b = v;
				break;
			case 5:
				*r = v;
				*g = w;
				*b = q;
				break;
		}
    }
}

// text colors
Color deftextblack(0,0,0);
Color deftextred(160,0,0);
Color deftextgreen(0,64,0);
Color deftextblue(0,0,140);
Color deftextwhite(255,255,255);
Color deftextmarked(250,100,100);

// light gray for crop
Color lightgray(220,220,220);

Color *textblack=new Color(0,0,0);
Color *textred=new Color(160,0,0);
Color *textgreen=new Color(0,64,0);
Color *textblue=new Color(0,0,140);
Color *textwhite=new Color(255,255,255);
Color *textmarked=new Color(250,100,100);
Color *textbracketmarked=new Color(200,200,200);
Color *textback=new Color(200,200,200);

// set the color with that number to the specified red green blue values.
int setcolor (int i, double red, double green, double blue)
{	
	if (i<0 || i>=16) return 1;
	if (red<0 || red>1) return 1;
	if (green<0 || green>1) return 1;
	if (blue<0 || blue>1) return 1;
	mycolors.set(i,
		*new Color(
			(int)(red*255+0.5),
			(int)(green*255+0.5),
			(int)(blue*255+0.5)));
	return 0;
}

MyColors savecolors; // holds the default colors to restore from

// reset all colors to the saved colors.
void resetcolors ()
{	
	// dump("%d",savecolors.color(0).getBlue());
	for (int i=0; i<16; i++)
		mycolors.set(i,savecolors.color(i));
}

#define PI 3.14159265358979323846

void gethuecolor (int c, int *r, int *g, int *b, double hue)
{
	if (c<=-2 && c>=-10)
	{
		double factor=1.5;
		double offset=1.0;
		switch (c)
		{
			case -3 : factor=1.0; offset=0.0; break;
			case -4 : factor=1.0; offset=0.5; break;
			case -5 : factor=1.0; offset=1.0; break;
			case -6 : factor=1.0; offset=1.5; break;
			case -7 : factor=-1.0; offset=1.0; break;
			case -8 : factor=-1.0; offset=1.5; break;
			case -9 : factor=-1.0; offset=2.0; break;
			case -10 : factor=-1.0; offset=2.5; break;
		}
		double t=hue*factor*PI+offset*PI;
		double st=sin(t);
		double ct=cos(t);
		double s=sqrt(2.0);
		*g=(int)((1+(ct-st)/s)/2*255);
		*r=(int)((1+(ct+st)/s)/2*255);
		*b=(int)((1+(-ct)/s)/2*255);
	}
	else if (c==-1)
	{
		getcolor((int)hue,r,g,b);
	}
	else if (c)
	{
		hue=hue-floor(hue);
		getcolor(c,r,g,b);
		*r = (int)(hue*255+(1-hue)* *r);
		*g = (int)(hue*255+(1-hue)* *g);
		*b = (int)(hue*255+(1-hue)* *b);
	} 
	else 
	{
		double cr,cg,cb;
		hsv_to_rgb((1.0-hue)*255.0,0.5,0.9,&cr,&cg,&cb);
		*r = (int)(255.0*cr);
		*g = (int)(255.0*cg);
		*b = (int)(255.0*cb);
	}
	if (!usecolors)
	{
		int h=(*r+*g+*b)/3;
		h=(255+2*h)/3;
		*r=*g=*b=h;
	}
}

MyColor::MyColor (int c, double hue)
{
	int r, g, b;

	gethuecolor(c,&r,&g,&b,hue);

	set(r>255 ? 255:r, g>255 ? 255:g, b>255 ? 255:b); // the color component must be <= 255.
}
