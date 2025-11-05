#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <float.h>
#include <stdarg.h>

#include "header.h"
#include "sysdep.h"
#include "funcs.h"
#include "graphics.h"
#include "interval.h"
#include "builtin.h"
#include "stack.h"
#include "yacas.h"
#include "maxima.h"
#include "getvalue.h"
#include "express.h"
#include "help.h"
#include "globalvars.h"
#include "loops.h"

int inloop=0;

int ctest (header *hd);
void kill_local (char *name);

void scan_end (void)
/***** scan_end
	scan for "end".
*****/
{	int comn;
	commandtyp *com;
	char *oldline=udfline;
	while (1)
	{	switch (*next)
		{	case 1 :
				output("\"end\" missing!\n");
				error=110; udfline=oldline; return;
			case 0 : 
				if (udfon) { udfline=next+1; next++; }
				else
				{	output("\"end\" missing!\n");
					error=110; udfline=oldline; return;
				}
				break;
			case 2 : 
				next+=1+sizeof(double); break;
			case 3 : 
				next++;
				memmove((char *)(&comn),next,sizeof(int));
				next+=2*sizeof(int);
				com=command_list+comn;
				if (com->nr==c_end)
				{	if (trace>0) trace_udfline(udfline);
				    return;
				}
				else if (com->nr==c_repeat || com->nr==c_loop ||
					com->nr==c_for)
				{	scan_end(); 
					if (error) return;
				}
				break;
			default :
				int l;
				if ((*next=='#' && *(next+1)=='#') ||
					(*next=='.' && *(next+1)=='.'))
				{	next+=2;
					while (*next!=0 && *next!=1) next++;
				}
				else if (!udfon && xisalpha(*next) && !xisalpha(*(next-1)) &&
					(com=preview_command(&l))!=0)
				{	next+=l;
					if (com->nr==c_end) return;
					else if (com->nr==c_repeat || com->nr==c_loop ||
						com->nr==c_for)
					{	scan_end(); 
						if (error) return;
					}
				}
				else next++;
		}
	}
}

void do_break (void)
{	if (!inloop)
	{	output("\"break\" only allowed in loops!\n"); error=57;
	}
}

void do_until (void)
{	if (!inloop)
	{	output("\"until\" only allowed in loops!\n"); error=57;
	}
}

void do_while(void)
{	if (!inloop)
	{	output("\"while\" only allowed in loops!\n"); error=57;
	}
}

/**
"for" loop in functions and on the command line.
for i=1 to 4; ...; end;
for i=vector; ...; end;
*/
void do_for (void)
{	
	int h,signum,inudf;
	char name[maxname],*jump;
	header *hd,*init,*end,*step;
	double vend,vstep;
	double *v=0;
	int type,ind,indmax;
	int *extra=0;
	char *here=next;

	// compute the space to store a jump after the loop end (only in functions).
	// this space is left after the for command by the compiler.
	if (udfon) extra=((int *)next)-1;

	// scan the name of the variable
	scan_space(); scan_name(name); if (error) return;

	// make a new variable with that name
	kill_local(name);
	newram=endlocal;
	hd=new_complex(0.0,0.0,name); if (error) return;

	// check if we are running on the command line
	int changeglobal=!udfon;
	endlocal=newram=(char *)hd+hd->size;
	if (changeglobal) endglobal=endlocal;

	// expect =
	scan_space(); if (*next!='=')
	{	output("Syntax error in for!\n"); error=71; goto end;
	}

	// scan the start value or the vector of values
	next++; init=scan(); if (error) goto end;
	init=getvalue(init); if (error) goto end;

	// check the type of input
	if (init->type==s_real)
	{	
		type=s_real;
		hd->type=s_real;
		*realof(hd)=*realof(init);
	}
	else if (init->type==s_matrix && (dimsof(init)->r==1 || dimsof(init)->c==1))
	{
		type=s_matrix;
		hd->type=s_real;
		indmax=dimsof(init)->r*dimsof(init)->c;
		if (indmax==0) { scan_end(); goto end; }
		ind=0;
		v=(double *)malloc(indmax*sizeof(double));
		if (v==0)
		{	
			print("Out of RAM in for loop!\n"); 
			error=1; return;
		}
		memcpy(v,matrixof(init),indmax*sizeof(double));
		*realof(hd)=v[ind];
	}
	else if (init->type==s_cmatrix && (dimsof(init)->r==1 || dimsof(init)->c==1))
	{
		type=s_cmatrix;
		hd->type=s_complex;
		indmax=dimsof(init)->r*dimsof(init)->c;
		if (indmax==0) { scan_end(); goto end; }
		ind=0;
		v=(double *)malloc(2*indmax*sizeof(double));
		if (v==0)
		{	
			print("Out of RAM in for loop!\n"); 
			error=1; return;
		}
		memcpy(v,matrixof(init),2*indmax*sizeof(double));
		*realof(hd)=v[2*ind];
		*(realof(hd)+1)=v[2*ind+1];
	}
	else if (init->type==s_complex)
	{
		type=s_cmatrix;
		hd->type=s_complex;
		indmax=1;
		ind=0;
		v=(double *)malloc(2*indmax*sizeof(double));
		if (v==0)
		{	
			print("Out of RAM in for loop!\n"); 
			error=1; return;
		}
		memcpy(v,realof(init),2*indmax*sizeof(double));
		*realof(hd)=v[2*ind];
		*(realof(hd)+1)=v[2*ind+1];
	}
	else if (init->type==s_imatrix && (dimsof(init)->r==1 || dimsof(init)->c==1))
	{
		type=s_imatrix;
		hd->type=s_interval;
		indmax=dimsof(init)->r*dimsof(init)->c;
		if (indmax==0) { scan_end(); goto end; }
		ind=0;
		v=(double *)malloc(2*indmax*sizeof(double));
		if (v==0)
		{	
			print("Out of RAM in for loop!\n"); 
			error=1; return;
		}
		memcpy(v,matrixof(init),2*indmax*sizeof(double));
		*realof(hd)=v[2*ind];
		*(realof(hd)+1)=v[2*ind+1];
	}
	else if (init->type==s_interval)
	{
		type=s_imatrix;
		hd->type=s_interval;
		indmax=1;
		ind=0;
		v=(double *)malloc(2*indmax*sizeof(double));
		if (v==0)
		{	
			print("Out of RAM in for loop!\n"); 
			error=1; return;
		}
		memcpy(v,realof(init),2*indmax*sizeof(double));
		*realof(hd)=v[2*ind];
		*(realof(hd)+1)=v[2*ind+1];
	}
	else
	{
		output("Illegal start value for loop!\n"); error=72; goto end;
	}

	// if this was a real start value, check for "to"
	if (type==s_real)
	{
		scan_space(); 
		if (strncmp(next,"to",2))
			// no "to" found
		{	
			vend=*realof(hd);
			vstep=1; signum=1;
		}
		else
			// "to" found
		{
			next+=2;

			// scan for the end value
			end=scan(); if (error) goto end;
			end=getvalue(end); if (error) goto end;
			if (end->type!=s_real)
			{	
				output("Endvalue must be real!\n"); error=73; goto end;
			}

			// note the end falue
			vend=*realof(end);
			scan_space();

			// check for "step"
			if (!strncmp(next,"step",4))
				// "step" found
			{	
				next+=4;
				step=scan(); if (error) goto end;
				step=getvalue(step); if (error) goto end;
				if (step->type!=s_real)
				{	output("Stepvalue must be real!\n"); error=73; goto end;
				}
				vstep=*realof(step);
			}
			else // "step" not found 
				vstep=1.0;

			// denote backward or forwar loop
			signum=(vstep>=0)?1:-1;
			// for inaccuracy in end value
			vend=vend+signum*epsilon;

			// check for empty loop
			if (signum>0 && *realof(hd)>vend) { scan_end(); goto end; }
			else if (signum<0 && *realof(hd)<vend) { scan_end(); goto end; }
		}
		newram=endlocal;
	}

	// skip semicolon or comma
	scan_space(); if (*next==';' || *next==',') next++;

	// remember jump position for loops
	jump=next;

	// remember, if we are in a function
	inudf=udfon;

	// loop:
	while (!error)
	{	
		if (*next==1 || (!udfon && *next==0))
			// end of function or end of command line
		{	
			output("\"end\" missing!\n");
			error=401; goto end;
		}

		// turn inloop on and remember old state
		int oldinloop=inloop;
		inloop=1;

		// do a command and denote the return value in h (command number)
		h=command();

		// adjust endglobal after the command
		if (changeglobal) endglobal=endlocal;

		// break the look on errors
		if (error) break;

		// somehow the command line broke the loop
		if (!inudf && !inloop) break;
		inloop=oldinloop;

		// return statement met
		if (udfon!=inudf) break;

		if (h==c_return)
			// return statement met (can only be in functions)
		{
			break;
		}
		else if (h==c_break)
			// break statement met (maybe inside if)
		{	
			if (udfon && *extra) 
				// then we know the end of the loop already
			{
				// jump to the end
				next=here+(*extra);
			}
			else 
			{	
				// scan for "end"
				scan_end(); 
				// denote the jump position
				if (extra) *extra=(int)(next-here);
			}
			break; 
		}
		else if (h==c_while)
			// while statement met
		{
			// scan condition
			scan_space();
			header *cond=scan(); if (error) goto end;
			if (*next=='=')
				// this is a frequent error. assignment instead of check for equality!
			{	
				output("Want to use == for conditions?\n");
				error=1; return;
			}
			// test condition
			int flag=ctest(cond); if (error) goto end;
			// skip semicolon
			if (*next==';') next++;
			if (!flag)
				// loop needs to be stopped (see break)
			{
				if (udfon && *extra) next=here+(*extra);
				else 
				{	
					scan_end(); 
					if (extra) *extra=(int)(next-here);
				}
				break;
			}
		}
		else if (h==c_until)
			// until statement met. (reverse of "while").
		{
			scan_space();
			header *cond=scan(); if (error) goto end;
			if (*next=='=')
			{	output("Want to use == for conditions?\n");
				error=1; return;
			}
			int flag=ctest(cond); if (error) goto end;
			if (*next==';') next++;
			if (flag)
			{
				if (udfon && *extra) next=here+(*extra);
				else 
				{	scan_end(); 
					if (extra) *extra=(int)(next-here);
				}
				break;
			}
		}
		if (h==c_end || h==c_continue)
			// "end" or "continue" met.
		{	
			if (h==c_end && extra) 
				// denote the jump position after the end of the loop
				*extra=(int)(next-here);

			// set loop variable to next value
			hd=searchvar(name); if (error) break;
			int finish=0;
			switch (type)
			{
				case s_real:
					if (hd->type!=s_real)
					{
						print("Loop variable %s has been changed.\n",name);
						error=1; return;
					}
					*realof(hd)+=vstep;
					if (signum>0 && *realof(hd)>vend) finish=1;
					else if (signum<0 && *realof(hd)<vend) finish=1;
					break;
				case s_matrix:
					if (hd->type!=s_real)
					{
						print("Loop variable %s has been changed.\n",name);
						error=1; return;
					}
					ind++;
					if (ind>=indmax) finish=1;
					*realof(hd)=v[ind];
					break;
				case s_cmatrix:
					if (hd->type!=s_complex)
					{
						print("Loop variable %s has been changed.\n",name);
						error=1; return;
					}
					ind++;
					if (ind>=indmax) finish=1;
					*realof(hd)=v[2*ind];
					*(realof(hd)+1)=v[2*ind+1];
					break;
				case s_imatrix:
					if (hd->type!=s_interval)
					{
						print("Loop variable %s has been changed.\n",name);
						error=1; return;
					}
					ind++;
					if (ind>=indmax) finish=1;
					*realof(hd)=v[2*ind];
					*(realof(hd)+1)=v[2*ind+1];
					break;
			}
			// break at last index
			if (finish) break;

			// jump back to start of loop
			next=jump;
			
			// check for escape key
			if (test_key()==escape)
			{   
				output("User interrupted!\n");
				error=1; break;
			}
		}
	}

end : 
	// free the area for the loop vectors, if used.
	if (v!=0) free(v);
	// for command lines adjust endglobal.
	if (changeglobal) endglobal=endlocal;
}

/**
Simple integer loop.
loop 1 to n; ...; end;
*/
void do_loop (void)
{	
	int h,inudf;
	char *jump;
	int *extra=0;
	char *here=next;
	header *init,*end;
	long vend,oldindex;
	if (udfon) extra=((int *)next)-1;
	init=scan(); if (error) return;
	init=getvalue(init); if (error) return;
	if (init->type!=s_real)
	{	output("Startvalue must be real!\n"); error=72; return;
	}
	oldindex=loopindex;
	loopindex=(long)*realof(init);
	scan_space(); if (error) goto end;
	if (strncmp(next,"to",2))
	{	output("Endvalue missing in loop!\n"); error=73; goto end;
	}
	next+=2;
	end=scan(); if (error) goto end;
	end=getvalue(end); if (error) goto end;
	if (end->type!=s_real)
	{	output("Endvalue must be real!\n"); error=73; goto end;
	}
	vend=(long)*realof(end);
	if (loopindex>vend) { scan_end(); goto end; }
	newram=endlocal;
	scan_space(); if (error) goto end;
	if (*next==';' || *next==',') next++;

	// check if we are running on the command line
	int changeglobal=!udfon;
	endlocal=newram;
	if (changeglobal) endglobal=endlocal;

	jump=next;
	inudf=udfon;
	while (!error)
	{	
		if (*next==1 || (!udfon && *next==0))
		{	
			output("End missing in loop!\n");
			error=401; goto end;
		}
		int oldinloop=inloop;
		inloop=1;

		// run a command
		h=command();

		// adjust endglobal after the command
		if (changeglobal) endglobal=endlocal;

		if (!inudf && !inloop) break;
		inloop=oldinloop;
		if (error) break;
		if (udfon!=inudf || h==c_return)
		{	returnloopindex=loopindex; // in case return uses #
			break;
		}
		if (h==c_break)
		{	if (udfon && *extra) next=here+(*extra);
			else 
			{	scan_end(); 
				if (extra) *extra=(int)(next-here);
			}
			break; 
		}
		else if (h==c_while)
		{
			scan_space();
			header *cond=scan(); if (error) return;
			if (*next=='=')
			{	output("Want to use == for conditions?\n");
				error=1; return;
			}
			int flag=ctest(cond); 
			if (error) return;
			if (error) return;
			if (*next==';') next++;
			if (!flag)
			{
				if (udfon && *extra) next=here+(*extra);
				else 
				{	scan_end(); 
					if (extra) *extra=(int)(next-here);
				}
				break;
			}
		}
		else if (h==c_until)
		{
			scan_space();
			header *cond=scan(); if (error) return;
			if (*next=='=')
			{	output("Want to use == for conditions?\n");
				error=1; return;
			}
			int flag=ctest(cond); 
			if (error) return;
			if (error) return;
			if (*next==';') next++;
			if (flag)
			{
				if (udfon && *extra) next=here+(*extra);
				else 
				{	scan_end(); 
					if (extra) *extra=(int)(next-here);
				}
				break;
			}
		}
		else if (h==c_end || h==c_continue)
		{	if (h==c_end && extra) *extra=(int)(next-here);
			loopindex++;
			if (loopindex>vend) break;
			else next=jump;
			if (test_key()==escape)
			{	output("User interrupted!\n");
				error=1; break;
			}
		}
	}
	end : loopindex=oldindex;
}

void do_repeat (void)
/***** do_loop
	do a loop command in a UDF.
	for value to value; .... ; endfor
*****/
{	
	int h;
	char *jump;
	int *extra=0;
	char *here=next;
	if (udfon) extra=((int *)next)-1;
	newram=endlocal;
	scan_space(); if (*next==';' || *next==',') next++;
	jump=next;
	int inudf=udfon;
	while (!error)
	{	
		if (*next==1 || (!udfon && *next==0))
		{	output("\"end\" missing in repeat statement!\n");
			error=401; break;
		}
		int oldinloop=inloop;
		inloop=1;
		h=command();
		inloop=oldinloop;
		if (udfon!=inudf || h==c_return) break;
		if (h==c_break)
		{	
			if (udfon && *extra) next=here+(*extra);
			else 
			{	scan_end(); 
				if (extra) *extra=(int)(next-here);
			}
			break; 
		}
		else if (h==c_until)
		{
			scan_space();
			header *cond=scan(); if (error) return;
			if (*next=='=')
			{	output("Want to use == for conditions?\n");
				error=1; return;
			}
			int flag=ctest(cond); 
			if (error) return;
			if (error) return;
			if (*next==';') next++;
			if (flag)
			{
				if (udfon && *extra) next=here+(*extra);
				else 
				{	scan_end(); 
					if (extra) *extra=(int)(next-here);
				}
				break;
			}
		}
		else if (h==c_while)
		{
			scan_space();
			header *cond=scan(); if (error) return;
			if (*next=='=')
			{	output("Want to use == for conditions?\n");
				error=1; return;
			}
			int flag=ctest(cond); 
			if (error) return;
			if (error) return;
			if (*next==';') next++;
			if (!flag)
			{
				if (udfon && *extra) next=here+(*extra);
				else 
				{	scan_end(); 
					if (extra) *extra=(int)(next-here);
				}
				break;
			}
		}
		else if (h==c_until)
		{
			scan_space();
			header *cond=scan(); if (error) return;
			if (*next=='=')
			{	output("Want to use == for conditions?\n");
				error=1; return;
			}
			int flag=ctest(cond); 
			if (error) return;
			if (error) return;
			if (*next==';') next++;
			if (flag)
			{
				if (udfon && *extra) next=here+(*extra);
				else 
				{	scan_end(); 
					if (extra) *extra=(int)(next-here);
				}
				break;
			}
		}
		else if (h==c_end)
		{	if (extra) *extra=(int)(next-here);
			next=jump;
			if (test_key()==escape)
			{   output1("User interrupted!\n");
				error=1; break;
			}
		}
		else if (h==c_continue)
		{	
			next=jump;
			if (test_key()==escape)
			{   output1("User interrupted!\n");
				error=1; break;
			}
		}
	}
}

void do_end (void)
{	if (!inloop)
	{	output("\"end\" only allowed in loops!\n"); error=57;
	}
}

void do_continue (void)
{	if (!inloop)
	{	output("\"continue\" only allowed in loops!\n"); error=57;
	}
}

