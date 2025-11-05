#include <stdio.h>

#include "header.h"
#include "sysdep.h"
#include "stack.h"
#include "getvalue.h"

void mframes (header *hd)
{	new_real(nframes(),"");
}

void mshowframe (header *hd)
{	header *st=hd,*result;
	hd=getvalue(hd);
	if (error) return;
	if (hd->type!=s_real) need_arg_in("showpage","integer");
	graphic_mode();
	showframe((int)*realof(hd));
	result=new_real(nframes(),"");
	moveresult(st,result);
}

void mcopyto (header *hd)
{	header *st=hd,*result;
	hd=getvalue(hd);
	if (error) return;
	if (hd->type!=s_real) need_arg_in("copytopage","integer");
	graphic_mode();
	copytoframe((int)*realof(hd));
	result=new_real(nframes(),"");
	moveresult(st,result);
}

void mcopyfrom (header *hd)
{	header *st=hd,*result;
	hd=getvalue(hd);
	if (error) return;
	if (hd->type!=s_real) need_arg_in("copyfrompage","integer");
	graphic_mode();
	copyfromframe((int)*realof(hd));
	result=new_real(nframes(),"");
	moveresult(st,result);
}

void maddframe (header *hd)
{	addframe();
	new_real(nframes(),"");
}

void maddframe2 (header *hd)
{   header *st=hd,*hd1;
	int w,h;
	hd1=nextof(hd);
	hd=getvalue(hd); hd1=getvalue(hd1); if (error) return;
	if (hd->type!=s_real || hd1->type!=s_real)
		need_arg_in("addpage","optional w, h");
	w=(int)*realof(hd); h=(int)*realof(hd1);
	if (w<0 || w>1000) w=100;
	if (h<0 || h>1000) h=100;
	addframe(w,h);
	moveresult(st,new_real(nframes(),""));
}

void mdeleteframes (header *hd)
{	deleteframes();
	new_real(nframes(),"");
}

