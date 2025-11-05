#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
#include "assign.h"

int sametype (header *hd1, header *hd2)
//	returns true, if hd1 and hd2 have the same type and dimensions.
{	
	dims *d1,*d2;
	if (hd1->type==s_string && hd2->type==s_string)
    	return hd1->size>=hd2->size;
	if (hd1->type==s_smatrix && hd2->type==s_smatrix)
    	return hd1->size>=hd2->size;
	if (hd1->type!=hd2->type || hd1->size!=hd2->size) return 0;
	if (hd1->type==s_matrix || hd1->type==s_cmatrix || hd1->type==s_imatrix
		|| hd1->type==s_cpxmatrix)
	{	d1=dimsof(hd1); d2=dimsof(hd2);
			if (d1->r!=d2->r) return 0;
	}
	return 1;
}

void setname (header *hd, char *name)
{	
	strcpy(hd->name,name);
	hd->xor=xor(name);
}

extern int allowoverwrite,allowvaroverwrite;

int useglobalheap=1;

header *assign (header *var, header *value)
//	assign the value to the variable.
{	
	char name[maxname],*nextvar;
	ULONG size;
	long dif;
	double *m,*mv,*m1,*m2;
	int i,j,c,r,cv,rv,*rind,*cind;
	dims *d;
	header *help,*orig;
	if (error) return 0;
	size=value->size;

	strcpy(name,var->name);

	if (strcmp(name,"%") && value && value->type==s_string && *stringof(value)==2 && *(stringof(value)+1)==0)
	{
		print("A function returned no value. Cannot assign this to a variable.\n");
		error=1; return 0;
	}

	if (var->type==s_reference && !referenceof(var)) // a new variable
	{	
		if (!udfon && !(allowoverwrite || allowvaroverwrite)  && value->type!=s_udf)
		{
			header *hd=searchalludf(name);
			if (hd && hd->flags&PROTECT)
			{
				output1("Cannot change protected function %s!",name);
				error=1; return 0;
			}
			builtintyp *bf=find_builtin(name);
			if (bf && bf->nargs==0)
			{
				print("Cannot overwrite builtin function %s!",name);
				error=1; return 0;
			}
		}

		if (value->type==s_udf) // assign a function to var
		{	
			invalidate_udflist(); // necessary!
			setname(value,name);
			if (!freeram(size)) ERETVALX("Memory overflow assigning to %s\n",name,500,value)
			memmove(ramstart+size,ramstart,newram-ramstart);
			newram+=size; endlocal+=size; startlocal+=size;
			udfend+=size;
			value=(header *)((char *)value+size);
			memmove(ramstart,(char *)value,size);
			return (header *)ramstart;
		}
		else if (strends(name,"$$") || (useglobalheap && !udfon && strcmp(name,"%")!=0))
		{
			header *hh=globalput(name,value);
			return hh;
		}
		else
		{	
			if (!freeram(size)) 
				ERETVALX("Memory overflow assigning to %s\n",name,500,value)
			memmove(endlocal+size,endlocal,newram-endlocal);
			value=(header *)((char *)value+size);
			newram+=size;
			memmove(endlocal,(char *)value,size);
			value=(header *)endlocal;
			setname(value,name); 
			endlocal+=size;
			value->flags=(value->flags)&(~PROTECT);
			return value;
		}
	}
	else // not a new variable
	{	
		if (!var) ERETVAL("Internal variable error!\n",43,0) // shold not happen!

		if (var->type==s_reference && strends(name,"$$"))
		{
			header *hh=globalput(name,value);
			return hh;
		}

		int heapvar=0;

		while (var && var->type==s_reference && referenceof(var))
		{	
			if (var->flags&HEAP) { heapvar=1; break; }
			if (var->flags&FLAGVAR) break; // stop at local variables (non-%-variables)
			var=referenceof(var);
		}
			
		if (!udfon && !(allowoverwrite || allowvaroverwrite) && var->flags&PROTECT)
		{
			output1("Cannot change protected variable %s!",var->name);
			error=1; return 0;
		}

		if (var->type!=s_udf && value->type==s_udf) 
			ERETVAL("Cannot assign a UDF to a variable!\n",320,var)
		
		if (var->type==s_submatrix) // make a matrix and call asign again
		{	
			d=submdimsof(var);
			if (value->type==s_complex || value->type==s_cmatrix)
			{	
				orig=submrefof(var);
				help=new_reference(orig,"");
				if (error) return 0;
				mcomplex(help); if (error) return 0;
				var->type=s_csubmatrix;
				submrefof(var)=help;
				assign(var,value); if (error) return 0;
				submrefof(var)=orig;
				assign(orig,help);
				return orig;
			}
			else if (value->type==s_interval || value->type==s_imatrix)
			{	
				orig=submrefof(var);
				help=new_reference(orig,"");
				if (error) return 0;
				minterval1(help); if (error) return 0;
				var->type=s_isubmatrix;
				submrefof(var)=help;
				assign(var,value); if (error) return 0;
				submrefof(var)=orig;
				assign(orig,help);
				return orig;
			}
			else if (value->type!=s_real && value->type!=s_matrix)
			{	
				switch (value->type)
				{	case s_string : 
					case s_smatrix :
						ERETVAL("Cannot assign a string to real!\n",45,0)
					case s_complex : 
					case s_cmatrix :
						ERETVAL("Cannot assign a complex value to real!\n",45,0)
					default :
						ERETVAL("Cannot assign this value to real!\n",45,0)
				}
			}
			getmatrix(value,&rv,&cv,&mv);
			getmatrix(submrefof(var),&r,&c,&m);
			if (d->r!=rv || d->c!=cv)
			{   if (rv==1 && cv==1)
				{	rind=rowsof(var); cind=colsof(var);
					for (i=0; i<d->r; i++)
					{	m1=mat(m,c,rind[i],0);
						for (j=0; j<d->c; j++)
						{	m1[cind[j]]=*mv;
						}
					}
					return submrefof(var);
				}
				ERETVAL("Illegal assignment!\nRow or column numbers do not agree!\n",45,0)
			}
			rind=rowsof(var); cind=colsof(var);
			for (i=0; i<d->r; i++)
			{	m1=mat(m,c,rind[i],0);
				m2=mat(mv,cv,i,0);
				for (j=0; j<d->c; j++)
				{	m1[cind[j]]=*m2++;
				}
			}
			return submrefof(var);
		}
		else if (var->type==s_csubmatrix) // make a matrix and call asign again
		{	
			d=submdimsof(var);
			if (value->type==s_real || value->type==s_matrix)
			{	help=new_reference(value,""); if (error) return 0;
				mcomplex(help); if (error) return 0;
				assign(var,help);
				return submrefof(var);
			}
			if (value->type!=s_complex && value->type!=s_cmatrix)
				ERETVAL("Illegal assignment!\n",45,0)
			getmatrix(value,&rv,&cv,&mv);
			getmatrix(submrefof(var),&r,&c,&m);
			if (d->r!=rv || d->c!=cv)
			{   if (rv==1 && cv==1)
				{	rind=rowsof(var); cind=colsof(var);
					for (i=0; i<d->r; i++)
					{	m1=cmat(m,c,rind[i],0);
						for (j=0; j<d->c; j++)
						{	copy_complex(m1+(LONG)2*cind[j],mv);
						}
					}
					return submrefof(var);
				}
				ERETVAL("Illegal assignment!\nRow or column numbers do not agree!\n",45,0)
			}
			rind=rowsof(var); cind=colsof(var);
			for (i=0; i<d->r; i++)
			{	m1=cmat(m,c,rind[i],0);
				m2=cmat(mv,cv,i,0);
				for (j=0; j<d->c; j++)
				{   copy_complex(m1+(LONG)2*cind[j],m2); m2+=2;
				}
			}
			return submrefof(var);
		}
		else if (var->type==s_isubmatrix) // make a matrix and call asign again
		{   
			d=submdimsof(var);
			if (value->type==s_real || value->type==s_matrix)
			{	help=new_reference(value,""); if (error) return 0;
				minterval1(help); if (error) return 0;
				assign(var,help);
				return submrefof(var);
			}
			if (value->type!=s_interval && value->type!=s_imatrix)
				ERETVAL("Cannot assign this type to intervals!\n",45,0)
			getmatrix(value,&rv,&cv,&mv);
			getmatrix(submrefof(var),&r,&c,&m);
			if (d->r!=rv || d->c!=cv)
			{   if (rv==1 && cv==1)
				{	rind=rowsof(var); cind=colsof(var);
					for (i=0; i<d->r; i++)
					{	m1=imat(m,c,rind[i],0);
						for (j=0; j<d->c; j++)
						{	copy_interval(m1+(LONG)2*cind[j],mv);
						}
					}
					return submrefof(var);
				}
				ERETVAL("Illegal assignment!\nRow or column numbers do not agree!\n",45,0)
			}
			rind=rowsof(var); cind=colsof(var);
			for (i=0; i<d->r; i++)
			{	m1=imat(m,c,rind[i],0);
				m2=imat(mv,cv,i,0);
				for (j=0; j<d->c; j++)
				{   copy_interval(m1+(LONG)2*cind[j],m2); m2+=2;
				}
			}
			return submrefof(var);
		}
		else if (var->type==s_ssubmatrix)
		{
			if (value->type!=s_string)
			{
				print("Can only assign a string to an element of the string vector %s!\n",
					referenceof(var)->name);
				error=12; return 0;
			}
			header *hnew=new_string("",1,"");
			hnew->type=s_smatrix;
			int ivar=*indexof(var);
			var=referenceof(var);
			char *str=stringof(var);
			char *hstr=stringof(hnew);
			int count=1,found=0;
			while (*str!=1)
			{
				if (count==ivar) 
				{
					found=1;
					strcpy(hstr,stringof(value));
				}
				else strcpy(hstr,str);
				hstr+=strlen(hstr)+1;
				str+=strlen(str)+1;
				count++;
			}
			if (!found)
			{
				print("Index %d out of range in string vector %s!\n",ivar,var->name);
				error=12; return 0;
			}
			*hstr++=1;
			hnew->size=(long)((hstr-(char *)hnew)/ALIGNMENT+1)*ALIGNMENT;
			newram=(char *)(nextof(hnew));
			value=hnew;
			goto doassign;
		}
		else // not a submatrix
		{	
doassign:
			if (useglobalheap && (!udfon || heapvar))
			{
				// print("Assigning to variable %s flags=%x\n",name,var->flags);
				// give_out(value);
				int flags=var->flags&(~HEAP)&(~SYMBOLIC)&(~ZEROBASED);
				header *hh=globalput(name,value,flags);
				if (heapvar) referenceof(var)=hh;
				return hh;
			}
			else
			{
				if ((char *)var<startlocal || (char *)var>endlocal)
				{	
					if (!sametype(var,value)) // not a local variable
					{	output1("Cannot change type of non-local variable %s!\n",
							var->name);
						error=12; return 0;
					}
					memcpy((char *)(var+1),(char *)(value+1),
						value->size-sizeof(header));
					return var;
				}
				dif=(long)(value->size-var->size);
				if (dif>0 && !freeram(dif))
					ERETVAL("Memory overflow in assignment.\n",501,value)
				nextvar=(char *)var+var->size;
				if (dif!=0)
					memmove(nextvar+dif,nextvar,newram-nextvar);
				newram+=dif; endlocal+=dif;
				value=(header *)((char *)value+dif);
				setname(value,var->name);
				memmove((char *)var,(char *)value,value->size);
			}
		}
	}
	var->flags=var->flags&(~PROTECT);
	return var;
}

#define addsize(hd,size) ((header *)((char *)(hd)+(size)))

extern int insimglines,insertsmaller,nosubmref;

/** do_assignment
assign a value to a variable. 
handle multiple assignments and symbolic assignments.
*/
void do_assignment (header *var)
{	
	header *variable[16],*rightside[16],*rs,*v,*mark;
	int rscount,varcount,i,j;
	char *oldendlocal;
	scan_space();

	if (*next=='=' || !strncmp(next,":=",2)
		|| 
		!strncmp(next,"&=",2) ||
		!strncmp(next,"&&=",3) ||
		!strncmp(next,"::=",3) ||
		!strncmp(next,"&:=",3) ||
		!strncmp(next,":&=",3)
		)
	{	
		int assignformaxima=0;
		if (*next=='=') // = was found
		{	
			if (!udfon && assignments)
			{	
				output("Use assignment := in strict mode!\n");
				error=1; return;
			}
			next++;
		}
		else if (!strncmp(next,":=",2)) 
		{
			next+=2;
		}
		else if (!strncmp(next,"&=",2))
		{	
			next+=2;
			assignformaxima=2;
		}
		else if (!strncmp(next,"&&=",3))
		{	
			next+=3;
			assignformaxima=3;
		}
		else // ::= or :&= or &:= was found
		{	
			next+=3;
			assignformaxima=1;
		}
		if (assignformaxima==2 || assignformaxima==3)
		{
			scan_space(1);
			rs=scan_maxima_expression();
			while (*next==' ') next++;
			if (strstarts(next,"//")) 
				while (*next) next++;
		}
		else
		{
			nosubmref=1; scan_space(1); rs=scan_value(); nosubmref=0;
		}
		if (error) return;
		varcount=0;
		/* count the variables, that get assigned something */
		while (var<rs)
		{	
			if (var->type!=s_reference && var->type!=s_submatrix
				&& var->type!=s_csubmatrix && var->type!=s_isubmatrix && var->type!=s_ssubmatrix &&
				var->type!=s_smatrix)
			{	
				output("Cannot assign to this value!\n");
				error=210;
			}
			variable[varcount]=var; var=nextof(var); varcount++;
			if (varcount>=16)
			{	
				print("Too many variables for multiple assignment (%d variables found)!\n",varcount); error=100; return;
			}
		}
		/* count and note the values, that are assigned to the
			variables */
		rscount=0;
		while (rs<(header *)newram)
		{	
			rightside[rscount]=rs;
			if (rs==0)
			{
				print("Illegal assignment!\n"); error=101; return;
			}
			rs=nextof(rs); rscount++;
			if (rscount>=16)
			{	print("Too many values found for multiple assignment (%d found)!\n",rscount); error=101; return;
			}
		}
		/* cannot assign 2 values to 3 variables , e.g. */
		if (rscount>1 && rscount<varcount)
		{	
			output("Illegal multiple assignment!\n"); error=102; return;
		}
		oldendlocal=endlocal;
		/* do all the assignments */
		if (varcount==1)
		{	
			if (assignformaxima==2)
			{
				static char s[MAXLINE];
				strcpy(s,variable[0]->name);
				strcat(s,":");
				strcat(s,stringof(rightside[0]));
				strcat(s,";");
				// print("%s\n",s);
				char *h=callmxm(s,5,1);
				// char *h="test";
				newram=(char *)rightside[0];
				new_string(h,(int)strlen(h),"");
				var=assign(variable[0],rightside[0]);
				if (error) return;
				var->flags|=SYMBOLIC;
			}
			else if (assignformaxima==3)
			{
				static char s[MAXLINE];
				strcpy(s,variable[0]->name);
				strcat(s,":");
				strcat(s,stringof(rightside[0]));
				strcat(s,";");
				// print("%s\n",h);
				callmxm(s);
			}
			else if (assignformaxima==1) 
			{
				static char s[MAXLINE];
				strcpy(s,variable[0]->name);
				var=assign(variable[0],rightside[0]);
				strcat(s,":");
				char *h=tomxm(var,s,1);
				if (error) return;
				strcat(h,";");
				// print("%s\n",h);
				runmaxima(h,4,2);
			}
			else
			{
				var=assign(variable[0],rightside[0]);
			}
		}
		else if (assignformaxima)
		{	
			output("No multiple assignment for Maxima!\n"); error=102; return;
		}
		else
			// multiple assignment
		{
			/*
			if (varcount>rscount)
			{
				if (rscount>1) print("You cannot assign %d values to %d variables (see Syntax Options).\n",rscount,varcount);
				else print("You cannot assign %d value to %d variables (see Syntax Options).\n",rscount,varcount);
				error=102;
				return;
			}
			*/
			int offset=0,oldoffset,dif;
			for (i=0; i<varcount; i++)
			{	
				oldoffset=offset;
				/* assign a variable */
				header *varx=assign(addsize(variable[i],offset),
					addsize(rightside[(rscount>i)?i:0],offset));
				if (i==0) var=varx;
				if (error) return;
				offset=(ptrdiff_t)(endlocal-oldendlocal);
				if (oldoffset!=offset) /* size of variable changed */
				{	
					v=addsize(variable[i],offset);
					if (v->type==s_reference) mark=referenceof(v);
					else mark=submrefof(v);
					if (mark) // not a new variable
							// shift all references of the variabless
						for (j=i+1; j<varcount; j++)
						{	
							v=addsize(variable[j],offset);
							dif=offset-oldoffset;
							if (v->type==s_reference && referenceof(v)>mark)
								referenceof(v)=addsize(referenceof(v),dif);
							else if (submrefof(v)>mark)
								submrefof(v)=addsize(submrefof(v),dif);
						}
				}
			}
			var=addsize(rightside[0],offset);
		}
	}
	else // just an expression which is a variable
	{	
		var=getvalue(var);
	}
	if (error) return;
	int insimg=!udfon && (!strcmp(next,":") || !strcmp(next,": ") 
		|| !strncmp(next,": //",3));

	if (*next==')' || *next==']')
	{	
		print("Found too many closing brackets, excessive %c\n",*next);
		error=1000;
	}
	else if (*next==0 || *next==1 || *next==',')
		// print the result
	{	
		if (*next==',') next++;
		give_out(var);
	}
	else if (*next==';' || insimg)
	{
		next++;
	}
	else if (spaces && *next!=3)
	{	
		print("Commands must be separated by semicolon or comma!\nFound: %s (character %d)\n"
			"You can disable this in the Options menu.\n",next,(int)(*next));
		error=1000; return;
	}
	if (spaces && *next && *next!=' ' && *next!=9 && *next!=3 && strncmp(next,"...",3))
	{	
		print("Space between commands expected!\nFound: %s (character %d)\n"
			"You can disable this in the Options menu.\n",next,(int)(*next));
		error=1000; return;
	}

	if (insimg)
	{
		insert_image(insimglines-insertsmaller*10,"",1,0,1,0,1);
	}
}
