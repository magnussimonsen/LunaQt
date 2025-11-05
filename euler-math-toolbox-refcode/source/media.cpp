#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "header.h"
#include "sysdep.h"
#include "stack.h"
#include "spread.h"
#include "getvalue.h"

/*
This module contains Euler built-in functions for sound
and graphics.
Graphics include decomposition of RGB values.
*/

void mplaywav (header *hd)
// call the system to play a sound file
{	header *st=hd;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) wrong_arg_in("playwave");
	sys_playwav(stringof(hd));
	moveresult(st,hd);
}

// Load an image from a file and insert it into
// three matrices of rgb values
void mloadpixels (header *hd)
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string)
	{	output("getpixels needs a file name!\n"); error=1; return;
	}
	result=new_matrix(0,0,"");
	int c,r;
	if (!get_pixels(stringof(hd),matrixof(result),&r,&c))
	{	output1("Loading file %s failed!\n",stringof(hd));
		error=1; return;
	}
	dimsof(result)->c=c;
	dimsof(result)->r=r;
	result->size=matrixsize(c,r);
	newram=(char *)result+result->size;
	moveresult(st,result);
}

// Save an image to a file with three 
// three matrices of rgb values and a file name
void msavepixels (header *hd)
{	header *st=hd,*hd1,*result;
	hd=getvalue(hd); if (error) return;
	hd1=nextof(st); hd1=getvalue(hd1); if (error) return;
	if (hd->type!=s_matrix || hd1->type!=s_string)
	{	output("savepixels needs an RGB matrix and a file name!\n"); error=1; return;
	}
	if (!put_pixels(stringof(hd1),matrixof(hd),dimsof(hd)->r,dimsof(hd)->c))
	{	output1("Saving file %s failed!\n",stringof(hd1));
		error=1; return;
	}
	result=new_string(stringof(hd1),strlen(stringof(hd1))+1,"");
	moveresult(st,result);
}

// Plot an image at a screen position.
void mplotimg (header *hd)
{	header *st=hd,*hd1,*result;
	hd=getvalue(hd); if (error) return;
	hd1=next_param(st);
	if (hd1) hd1=getvalue(hd1); if (error) return;
	if (hd->type!=s_matrix || dimsof(hd)->r!=1 || dimsof(hd)->c!=2 || hd1->type!=s_matrix)
	{	output("plotimg needs a 1x2 vector and a matrix!\n"); error=1; return;
	}
	result=new_string("",32,"");
	moveresult(st,result);
}


double getred (double x)
{	
	unsigned long k=(unsigned long)x;
	if (k>=0 && k<=15)
	{
		int r,g,b;
		getcolor(k,&r,&g,&b);
		k=r;
	}
	else k=(k&0x0000FF0000)>>16;
	return k/255.0;
}

void mgetred (header *hd)
{	spread1(getred,0,hd);
	test_error("getred");
}

double getgreen (double x)
{	unsigned long k=(unsigned long)x;
	if (k>=0 && k<=15)
	{
		int r,g,b;
		getcolor(k,&r,&g,&b);
		k=g;
	}
	else k=(k&0x0000FF00)>>8;
	return k/255.0;
}

void mgetgreen (header *hd)
{	spread1(getgreen,0,hd);
	test_error("getgreen");
}

double getblue (double x)
{	unsigned long k=(unsigned long)x;
	if (k>=0 && k<=15)
	{
		int r,g,b;
		getcolor(k,&r,&g,&b);
		k=b;
	}
	else k=(k&0x000000FF);
	return k/255.0;
}

void mgetblue (header *hd)
{	spread1(getblue,0,hd);
	test_error("getblue");
}

double putred (double x)
{	unsigned long k=(unsigned long)(x*255+0.5);
	return ((k&0x000000FF)<<16)|0x01000000;
}

void mputred (header *hd)
{	spread1(putred,0,hd);
	test_error("putred");
}

double putgreen (double x)
{	unsigned long k=(unsigned long)(x*255+0.5);
	return ((k&0x000000FF)<<8)|0x01000000;
}

void mputgreen (header *hd)
{	spread1(putgreen,0,hd);
	test_error("putgreen");
}

double putblue (double x)
{	unsigned long k=(unsigned long)(x*255+0.5);
	return (k&0x000000FF)|0x01000000;
}

void mputblue (header *hd)
{	spread1(putblue,0,hd);
	test_error("putblue");
}
