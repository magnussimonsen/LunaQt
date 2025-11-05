#include "windows.h"
#include "meta.h"
#include "../sysdep.h"
#include "colors.h"
#include "windowsmeta.h"
#include "windowsgraphics.h"
#include "text.h"
#include "graphicswindow.h"

extern PS *ops;

extern int usecolors;

extern CriticalSection critical;

/*************** Metafile replay **********************/

int WindowsMeta::gclear (void *p)
/***** clear the graphics screen
*****/
{   PS *ps=(PS *)p;
	if (ps==0) ps=ops;
	if (!Meta::gclear(0)) return 0;
	if (Draw==0) return 1;
	::gclear(*ps);
	return 1;
}

int WindowsMeta::gclip (void *p, double c, double r, double c1, double r1)
{
	PS *ps=(PS *)p;
	if (ps==0) ps=ops;
	if (!Meta::gclip(0,c,r,c1,r1)) return 0;
	if (Draw==0) return 1;
	critical.enter();
	::gclip(*ps,c,r,c1,r1);
	critical.leave();
	return 1;
}

int WindowsMeta::gline (void *p, double c, double r, double c1, double r1, int color,
	int st, double width)
{   
	PS *ps=(PS *)p;
	if (ps==0) ps=ops;
	if (!Meta::gline(0,c,r,c1,r1,color,st,width)) return 0;
	if (Draw==0) return 1;
	critical.enter();
	::gline(*ps,c,r,c1,r1,color,st,width);
	critical.leave();
    return 1;
}

int WindowsMeta::gpath (void *p, double c[], int n, int color, int st, double width)
{   
	PS *ps=(PS *)p;
	if (ps==0) ps=ops;
	if (!Meta::gpath(0,c,n,color,st,width)) return 0;
	if (Draw==0) return 1;
	critical.enter();
	::gpath(*ps,c,n,color,st,width);
	critical.leave();
    return 1;
}

int WindowsMeta::gmarker (void *p, double c, double r, int color, int st, double markerfactor)
{   PS *ps=(PS *)p;
	if (ps==0) ps=ops;
	if (!Meta::gmarker(0,c,r,color,st,markerfactor)) return 0;
	if (Draw==0) return 1;
	critical.enter();
	::gmarker(*ps,c,r,color,st,markerfactor);
	critical.leave();
    return 1;
}

int WindowsMeta::gfill (void *p, double c[], int fillcolor, int wirecolor, int n, int connect[])
{   PS *ps=(PS *)p;
	if (ps==0) ps=ops;
   	if (!Meta::gfill(0,c,fillcolor,wirecolor,n,connect)) return 0;
	if (Draw==0) return 1;
	critical.enter();
	::gfill(*ps,c,fillcolor,wirecolor,n,connect);
	critical.leave();
    return 1;
}

int WindowsMeta::gpolygon (void *p, double c[], int n, int st, int color, int connect)
{   PS *ps=(PS *)p;
	if (ps==0) ps=ops;
   	if (!Meta::gpolygon(0,c,n,st,color,connect)) return 0;
	if (Draw==0) return 1;
	critical.enter();
	::gpolygon(*ps,c,n,st,color,connect);
	critical.leave();
    return 1;
}

int WindowsMeta::gfillh (void *p, double c[], int n, double hue, int color, int connect)
{   PS *ps=(PS *)p;
	if (ps==0) ps=ops;
	if (!Meta::gfillh(0,c,n,hue,color,connect)) return 0;
	if (Draw==0) return 1;
	critical.enter();
	::gfillh(*ps,c,n,hue,color,connect);
	critical.leave();
    return 1;
}

int WindowsMeta::gbar (void *p, double c, double r, double c1, double r1, double hue,
	int color, int connect)
{   PS *ps=(PS *)p;
	if (ps==0) ps=ops;
	if (!Meta::gbar(0,c,r,c1,r1,hue,color,connect)) return 0;
	if (Draw==0) return 1;
	critical.enter();
	::gbar(*ps,c,r,c1,r1,hue,color,connect);
	critical.leave();
    return 1;
}

int WindowsMeta::gbar1 (void *p, double c, double r, double c1, double r1,
	int color, int connect)
{   PS *ps=(PS *)p;
	if (ps==0) ps=ops;
	if (!Meta::gbar1(0,c,r,c1,r1,color,connect)) return 0;
	if (Draw==0) return 1;
	critical.enter();
	::gbar1(*ps,c,r,c1,r1,color,connect);
	critical.leave();
    return 1;
}

int WindowsMeta::gbarRGB (void *p, double c, double r, double c1, double r1, int cn, int rn,
	int tred, int tgreen, int tblue, unsigned char *colors)
{   PS *ps=(PS *)p;
	if (ps==0) ps=ops;
	if (!Meta::gbarRGB(0,c,r,c1,r1,cn,rn,tred,tgreen,tblue,colors)) return 0;
	if (Draw==0) return 1;
	critical.enter();
	::gbarRGB(*ps,c,r,c1,r1,cn,rn,tred,tgreen,tblue,colors);
	critical.leave();
    return 1;
}

int WindowsMeta::gtext (void *p, double c, double r, char *text, int color, int centered, double size)
{   PS *ps=(PS *)p;
	if (ps==0) ps=ops;
	if (!Meta::gtext(0,c,r,text,color,centered,size)) return 0;
	if (Draw==0) return 1;
	critical.enter();
	::gtext(*ps,c,r,text,color,centered,size);
	critical.leave();
    return 1;
}

int WindowsMeta::gvtext (void *p, double c, double r, char *text, int color, int centered, double size)
{   PS *ps=(PS *)p;
	if (ps==0) ps=ops;
	if (!Meta::gvtext(0,c,r,text,color,centered,size)) return 0;
	if (Draw==0) return 1;
	critical.enter();
	::gvtext(*ps,c,r,text,color,centered,size);
	critical.leave();
    return 1;
}

int WindowsMeta::gvutext (void *p, double c, double r, char *text, int color, int centered, double size)
{	PS *ps=(PS *)p;
	if (ps==0) ps=ops;
	if (!Meta::gvutext(0,c,r,text,color,centered,size)) return 0;
	if (Draw==0) return 1;
	critical.enter();
	::gvutext(*ps,c,r,text,color,centered,size);
	critical.leave();
    return 1;
}
