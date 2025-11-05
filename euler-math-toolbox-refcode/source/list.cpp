#include <math.h>

#include "header.h"
#include "stack.h"
#include "getvalue.h"
#include "list.h"

void getlistelement (header *var, header *hd)
{
	header *st=hd,*res;
	hd=getvalue(hd); if (error) return;
	char *name=var->name;
	if (hd->type==s_real)
	{
		int n=(int)(*realof(hd)+0.5);
		int note=n;
		if (n<=0) goto err;
		header *end=nextof(var);
		res=var+1;
		n--;
		while (n>0)
		{
			res=nextof(res);
			if (res>=end)
			{
				if (*name) print("Index %d out of range for collection %s.\n",note,name);
				else print("Index %d out of range for collection.\n",note);
				error=100; return;
			}
			n--;
		}
		moveresult(st,res);
	}
	else
	{
		err:
		print("Need an integer scalar as a list index.\n");
		error=100; return;
	}
}

