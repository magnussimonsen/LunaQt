#include <stdio.h>
#include <string.h>

#include "header.h"
#include "sysdep.h"

extern int trace,nojump,booktype,promptnotebook;
extern int inloop,nomultiline;

void read_line (char *line)
{	
	int count=0,input,scan=0;
	char *p;
	start :
	p=line;
	while(1)
	{	input=getc(infile);
		if (input==EOF)
		{	fclose(infile);
			if (p>line) break;
			else *p++=1;
			infile=0;
			break;
		}
		if (input=='\n') break;
		if (count>=1023)
		{	output("Line too long!\n"); error=50; *line=0; return;
		}
		if ((char)input>=' ' || (signed char)input<0
			|| (char)input==TAB)
		{	*p++=(char)input; count++;
		}
	}
	*p=0;
	if (booktype)
	{	
		switch (*line)
		{   
		case '$' :
			case '>' :
				output1("%s\n",line);
				if (promptnotebook && line[1]) wait_key(&scan);
				if (scan==escape)
				{	
					fclose(infile); infile=0; *line=0;
					break;
				}
				*line=' '; break;
			case '%' :
				output1("%s\n",line);
				goto start;
			default :
				if (!infile) break;
				goto start;
		}
	}
}

extern int pythonmode;

/**
Detect, if the line at s is a mult-line and return the end of
the commands in the line. This allows for the following cases
command ...
command ... // comment
command // comment ...
*/
char *ismultiline (char *s)
{
	if (strstarts(s,"py: ") || strstarts(s,">") || strstarts(s,":") || pythonmode) return 0;
	char *mark=0;
	while (*s)
	{
		if (*s=='\"')
		{
			s++;
			while (*s && *s!='\"') s++;
		}
		else if (strncmp(s," // ",4)==0) mark=s+1;
		else if (strcmp(s," ...")==0) 
		{
			if (mark) return mark;
			else return s+1;
		}
		else if (strstarts(s,"function ")) return 0;
		s++;
	}
	return 0;
}

void next_line (int noml)
/**** next_line
	read a line from keyboard or file.
	noml: no multilines
****/
{	
	if (nomultiline)
	{
		print(".. not allowed here.\n");
		error=1; return;
	}
	if (udfon)
	{	
		while (*next) next++;
		next++;
		if (*next==1) 
		{	udfon=0; 
		}
		else 
			udfline=next;
		if (trace>0) 
			trace_udfline(next);
	}
	else
	{	
		if (trace==-1) trace=1;
		if (stringon)
		{	
			error=2300; print("Line ended inside a string!\n");
			return;
		}
		char *nextinput=input_line;
		while (true)
			// collect multi-lines
		{
			nojump=0;
			if (!infile) 
			{
				nomultiline=noml;
				edit(nextinput);
				nomultiline=0;
			}
			else read_line(nextinput);
			if (noml) break;
			char *p=ismultiline(nextinput);
			if (!p) break;
			nextinput=p;
		}
		next=input_line;
	}
}

void next_line ()
{
	next_line(0);
}

void next_simple_line ()
{
	while (true)
		// until line is not empty
	{
		next_line(1);
		char *h=next;
		// skip empty lines
		while (*h==' ' || *h==9) h++;
		if (*h==0 || strncmp(h,"//",2)==0) continue;
		else break;
	}
}

