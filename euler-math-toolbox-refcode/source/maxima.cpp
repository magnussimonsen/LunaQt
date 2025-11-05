#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "header.h"
#include "stack.h"
#include "maxima.h"
#include "help.h"
#include "sysdep.h"
#include "getvalue.h"

static helpline *maximastartupfirstline=0;

char mxmin[MAXLINE];
char mxmin1[MAXLINE+16];

extern char *currentfunction;

int fixmaximalinel=0;

void do_maxima (void)
{	
	char s[MAXLINE];
	scan_space();
	char *p=s;
	while (*next!=0 && *next!=1)
	{	*p++=*next++;
		if (p-s>1023)
		{	output("Maxima line too long!\n");
			error=1;
			return;
		}
	}
	char *q=s;
	while (*q)
	{	if (*q=='/' && *(q+1)=='/')
		{	*q=0; p=q;
		}
		q++;
	}
	while (*(p-1)==' ' && p>s) p--;
	if (p>s)
	{	if (*(p-1)!=';' && *(p-1)!='$' && (p<s+2 || *(p-1)!='.' || *(p-2)!='.')) *p++=';';
	}
	*p=0;
	// output1("|%s|\n",s);
	callmaxima(s,0);
}

int bracketlevel=0;
int needbracket=0;

void do_compatible_maxima ()
/*
Does a lot of conversions. 
The line is converted to s and then runs through Maxima.
*/
{	
	char s[MAXLINE];
	scan_space();
	char *p=s;
	int instring=0;
	int infunction=0;
	int commandstart=1;
	int assignment=1;
	while (*next!=0 && *next!=1)
		// not end of line (command line or function)
	{	
		if (!instring) // we don't convert in strings
		{	
			if (bracketlevel==0 && *next==',')
				// convert commas to semicolons outside brackets
				// semicolons print in Maxima
			{	
				if (needbracket)
					// we will sometimes introduce "(", which must be closed
				{	*p++=')';
					needbracket=0;
				}
				next++; *p++=';';
				commandstart=1;
				assignment=1;
			}
			else if (bracketlevel==0 && *next==';')
				// convert semicolon to dollar (no print in Maxima)
			{	
				if (needbracket)
				{	*p++=')';
					needbracket=0;
				}
				next++; *p++='$';
				commandstart=1;
				assignment=1;
			}
			else if (!strncmp(next,"\\:",2))
			{
				next+=2;
				*p++=':';
			}
			else if (!strncmp(next,":=",2))
				// convert := to : (with exceptions, see code)
			{	
				next+=2; *p++=':';
				if (infunction)
					// we need := for function definitions
				{	
					*p++='=';
				}
				else if (!assignment)
					// we need := ''... for functions, which are not assigments
					// allows: "function f(x) := diff(x^x,x)"
					//         without evaluating the function body
				{	
					*p++='=';
					*p++=' ';
					while (*next==' ') next++;
					if (strncmp(next,"block",(int)strlen("block")))
					{	
						*p++='\'';
						*p++='\'';
						*p++='(';
						needbracket=1;
					}
				}
				infunction=0;
				commandstart=0;
			}
			else if (*next=='(' || *next=='[')
				// adjust the bracket level
			{	
				if (*next=='(') assignment=0;
				bracketlevel++;
				*p++=*next++;
				commandstart=0;
			}
			else if (*next==')' || *next==']')
				// adjust the bracket level
			{	
				bracketlevel--;
				*p++=*next++;
				commandstart=0;
			}
			else if (*next==' ')
			{	
				*p++=*next++;
			}
			else if (commandstart && bracketlevel==0 && 
				strncmp(next,"function ",(int)strlen("function "))==0)
				// we have a function keyword at command start outside brackets
				// set infunction to remember this
			{	
				infunction=1;
				next+=(int)strlen("function ");
			}
			else if (*next=='\"')
				// we go inside a string
			{	
				instring=1;
				*p++=*next++;
				commandstart=0;
			}
			else if (*next=='/' && *(next+1)=='/')
				// ignore anything past "//"
			{	
				break;
			}
			else
			{	
				*p++=*next++;
				commandstart=0;
			}
		}
		else if (*next=='\"')
			// we are inside string, and leave the string
		{	
			instring=0;
			*p++=*next++;
			commandstart=0;
		}
		else
			// we are inside string, and get another character of the string
		{	
			*p++=*next++;
			commandstart=0;
		}
		if (p-s>MAXLINE-16)
			// oops, line too long
		{	
			output("Maxima line too long!\n");
			error=1;
			return;
		}
	}
	if (needbracket && (p<s+2 || *(p-1)!='.' || *(p-2)!='.'))
		// our Maxima string did not end with "..", and we need ")"
	{	
		*p++=')';
		needbracket=0;
	}				
	*p=0; // finish the string
	// find the last non-blank:
	while (*(p-1)==' ' && p>s) p--;
	if (p>s)
		// the string is not blank
		// add a semicolon, if none is found, and no "$" and now ".."
	{	
		if (*(p-1)!=';' && *(p-1)!='$' && (p<s+2 || *(p-1)!='.' || *(p-2)!='.')) *p++=';';
	}
	*p=0; // finish the string again.
	// remember bracket level:
	if (p<s+2 || *(p-1)!='.' || *(p-2)!='.') 
	{
		bracketlevel=0;
	}
	// print("%s\n",s);
	callmaxima(s,1);
}

int maximamode=0;

extern int usedirectmode;

void do_maximamode ()
{	char name[maxname];
	scan_space();
	if (!*next || *next==';' || *next==',')
	{	maximamode=!maximamode;
		if (maximamode && usedirectmode) maximamode=2;
	}
	else
	{	scan_name(name);
		if (strcmp(name,"on")==0)
			maximamode=1;
		if (strcmp(name,"off")==0)
			maximamode=0;
		if (strcmp(name,"direct")==0)
			maximamode=2;
	}
	if (*next!=';')
	{	switch (maximamode)
		{	case 0 : output1("Maxima mode is off\n"); break;
			case 1 : output1("Maxima mode is on (compatibility mode)\n"); break;
			case 2 : output1("Maxima mode is on (direct mode)\n"); break;
		}
	}
	if (*next==',' || *next==';') next++;
}


/*
Replace [...;...;...] with matrix([...],[...],[...]).
*/
char *replacematrix (char *s)
{
	strcpy(mxmin1,s);
	s=mxmin1;
	char *p=mxmin;

	int rbl=0,sbl=0; // bracket levels (round and square)
	int inmatrix=0;

	while (*s)
	{
		if (*s=='[')
		{
			sbl++;
			if (sbl==1)
			{
				// look ahaed for matrix [...;
				char *h=s+1;
				int sblh=1;
				inmatrix=0;
				while (*h)
				{
					if (*h=='[') sblh++;
					else if (*h==']') sblh--;
					else if (*h==';' && sblh==1)
					{
						inmatrix=1; break;
					}
					h++;
				}
			}
			if (inmatrix)
			{
				strcpy(p,"matrix(["); p+=strlen(p);
				s++;
			}
			else *p++=*s++;
		}
		else if (*s=='(')
		{
			rbl++;
			*p++=*s++;
		}
		else if (*s==')')
		{
			rbl--;
			*p++=*s++;
		}
		else if (*s==']')
		{
			sbl--;
			if (inmatrix && sbl==0)
			{
				strcpy(p,"])"); p+=strlen(p);
				inmatrix=0;
				s++;
			}
			else *p++=*s++;
		}
		else if (*s==';' && inmatrix && sbl==1)
		{
			strcpy(p,"],["); p+=strlen(p);
			s++;
		}
		else *p++=*s++;
		if (p-mxmin>MAXLINE-16)
		{
			print("Maxima command too long!\n");
			error=1; return mxmin;
		}
	}
	*p=0;

	return mxmin;
}

/*
Replace all @var in the expression with the content of the variable var.
Replace all __ to _.
percent=1: replace % with content of this variable outside functions
*/
char *replaceexpressions (char *s, int percent)
{	
	// print("Replace \"%s\" flag=%d\n",s,percent);

	if (strlen(s)>MAXLINE-2) return s;
	if (*s=='!') 
	{
		if (*(s+1)!='!') return s+1;
		else s++;
	}

	strcpy(mxmin1,s);
	s=mxmin1;
	char *p=mxmin;

	while (*s)
	{	
		int found=0;
		if (*s=='\\')
		{
			if (*(s+1)) s++;
			*p++=*s++;
		}
		else if (!strncmp(s,"__",2))
		{
			*p++='_';
			s+=2;
		}
		else if (*s=='|')
		{
			*p++=',';
			s++;
		}
		else if ((*s=='@' && xisalpha(*(s+1))) || 
			(percent && *s=='%' && !xisalpha(*(s+1))))
		{	
			char name[maxname];
			char *q=s+1;
			if (*s=='%' && !udfon)
			{
				strcpy(name,"%");
			}
			else
			{
				char *qn=name;
				while ((xisalpha(*q) || xisdigit(*q) || *q=='%') 
						&& qn<name+maxname-1) *qn++=*q++;
				*qn++=0;
			}
			if (*name)
			{	
				int old=searchglobal; searchglobal=1;
				header *hd=searchvar(name);
				searchglobal=old;
				if (hd)
				{	
					hd=getvalue(hd); if (error) return mxmin;
					char *qh=tomxm(hd,0,0);
					if (strlen(qh)+(p-mxmin)>MAXLINE-2)
					{	
						print("Maxima command too long!\n");
						error=1; return mxmin;
					}
					*p++='(';
					strcpy(p,qh);
					strcat(p,")");
					p+=strlen(p);
				}
				else
				{	
					output1("Variable %s not found!\n",name); 
					if (udfon)
						output1("Can only see global and local variables in %s.\n",
							currentfunction);
					error=1; return mxmin;
				}
				found=1;
				s=q;
			}
		}
		else if (percent && !strncmp(s,":=",2))
		{
			*p++=':';
			s+=2;
		}
		if (!found) *p++=*s++;
		if (p-mxmin > MAXLINE-16)
		{
			print("Maxima command too long!\n");
			error=1; return mxmin;
		}
	}
	*p=0;

	// print("Result \"%s\"\n",mxmin);

	return mxmin;
}

/*
Replace all __ to _.
*/
char *replaceunderscore (char *s)
{	
	if (!strchr(s,'_')) return s;

	char *p=mxmin;

	while (*s)
	{
		if (*s=='_' && *(s+1)=='_') s++;
		*p++=*s++;
	}
	*p=0;
	return mxmin;
}

/*
Euler command mxm to send a string to Maxima and get the result
in 1D string form.
*/
void mxm (header *hd)
{	
	// get the string from the stack:
	header *st=hd,*result;
	char *arg;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) need_arg_in("mxm","string");
	arg=stringof(hd);
	if (*arg==':' && *(arg+1)==':') arg+=2;
	else if (*arg=='@' && *(arg+1)==':') arg+=2;
	else if (*arg=='&') arg++;

	char *res=callmxm(arg,5,1);
	if (error) return;

	// return result:
	result=new_string(res,strlen(res),"");

	if (error) return;
	moveresult(st,result);
}

/*
Feed mxmstartup.txt to Maxima
*/
void mxmstartup ()
{
	char param[MAXLINE];
	sprintf(param,"gnuout:\"%sgnuout.png\"$\n",getusereulerdir());
	char *p=param;
	while (*p) 
	{
		if (*p=='\\') *p='/';
		p++;
	}
	runmaxima(param,4,3);

	helpline *first=maximastartupfirstline;
	while (first)
	{	
		runmaxima(first->text,4,3);
		first=first->next;
	}
}

int display2d=1; // Flag for the Maxima output mode


/*
Send the command to Maxima, and get the result in 2D fashion.
*/
int callmaxima (char *s, int replace)
{	
	// print("callmaxima(%s,%d)\n",s,replace);

	if (!display2d) // if not in 2D state, switch to it.
	{	
		runmaxima("display2d:true;",4,0);
		display2d=1;
	}

	if (fixmaximalinel)
	{
		char s[32];
		sprintf(s,"linel:%d;\n",fixmaximalinel);
		runmaxima(s,4,0);
		fixmaximalinel=0;
	}

	int output=1,n=strlen(s);

	if (n>4 && s[n-1]=='.' && s[n-2]=='.' && s[n-3]=='.')
	{	s[n-3]=0;
		output=3;
	}
	else if (n>3 && s[n-1]=='.' && s[n-2]=='.')
	{	s[n-2]=0;
		output=3;
	}
	else if (n>3 && s[n-1]=='$' && s[n-2]=='$')
	{	s[n-1]=0;
		output=4;
	}
	// output1("|%s|\n",s);

	// now, send the Maxima command, and expect and print output.
	return runmaxima(s,output,replace);
}

// ****************** handle maxima.txt ***********

maximahelpline *maximafirstline=0;

/*
read maxima.txt into a list
*/
void loadmaximahelp (char *filename)
{	
	char line[MAXLINE];
	FILE *in=fopen(filename,"r");
	if (!in) return;
	maximahelpline *hl=0;
	while (!feof(in))
	{	char *lin=newline(in);
		if (!lin) break;
		strcpy(line,lin);
		int l=(int)strlen(line);
		char *buf=(char *)malloc(l+1);
		strcpy(buf,line);
		char *q=line;
		while (*q && *q!=' ') q++;
		*q=0;
		char *bufitem=(char *)malloc(strlen(line)+1);
		strcpy(bufitem,line);
		maximahelpline *nexthl=(maximahelpline *)malloc(sizeof(maximahelpline));
		nexthl->next=0;
		nexthl->text=buf;
		nexthl->item=bufitem;
		if (maximafirstline==0) maximafirstline=nexthl;
		else hl->next=nexthl;
		hl=nexthl;
	}
	fclose(in);
}


char mhelpline[MAXLINE];
#define MAXHLINES 256
maximahelpline *hlines[MAXHLINES];
int hmore[MAXHLINES];

int helpline_compare (const maximahelpline **p1, const maximahelpline **p2)
{	int n=strlen((*p1)->item);
	int m=strlen((*p2)->item);
	if (n<m) return -1;
	else if (n>m) return 1;
	else return 0;
}

/*
Search for help through the help items.
*/
int getmaximahelp (char *stext, char *extend, char *status, int number)
{	
	*extend=0; *status=0;
	maximahelpline *hl=maximafirstline;
	char text[MAXLINE];
	strcpy(text,stext);
	int n=strlen(text);
	if (n<1) return 0;
	if (text[n-1]=='(')
	{	
		text[n-1]=0; n--;
		while (hl)
		{	
			if (!strcmp(hl->item,text))
			{	
				strcpy(extend,"");
				strcpy(status,hl->text);
				putstatustopic(hl->item,1);
				return 0;
			}
			hl=hl->next;
		}
		strcpy(extend,"");
		strcpy(status,"");
		return 0;
	}
	int count=0;
	maximahelpline *prev=0;
	while (hl)
	{	
		if (!strncmp(hl->item,text,n))
		{	
			if (prev)
			{	
				if (strcmp(prev->item,hl->item))
				{	
					hmore[count]=0;
					hlines[count++]=hl;
					prev=hl;
				}
				else
				{	
					if (count>0) hmore[count-1]++;
					if (hmore[count-1]<=number) hlines[count-1]=hl;
				}
			}
			else
			{	
				hmore[count]=0;
				hlines[count++]=hl;
				prev=hl;
			}
		}
		hl=hl->next;
		if (count>=MAXHLINES) break;
	}
	if (count==0) return 0;
	else if (count==1) 
	{	
		strcpy(status,hlines[0]->text);
		if (hmore[0]>0)
		{	sprintf(status,"%s ... and %d more (press ins)",hlines[0]->text,hmore[0]);
		}
		strcpy(extend,hlines[0]->item);
		return hmore[0];
	}
	else
	{	
		qsort(hlines,count,sizeof(helpline *),
			(int (*) (const void *, const void *))helpline_compare);
		for (int i=0; i<count; i++)
		{	if (strlen(extend)+strlen(hlines[i]->item)+1>MAXLINE-2) break;
			strcat(extend,hlines[i]->item);
			strcat(extend," ");
			strcat(status,hlines[i]->item);
			strcat(status," ");
		}
	}
	return 0;
}

int printmaximahelp (char *text)
{	maximahelpline *hl=maximafirstline;
	int count=0;
	while (hl)
	{	if (!strcmp(hl->item,text))
		{	if ((int)strlen(hl->text)<linelength) output1("%s\n",hl->text);
			else
			{	char *p=hl->text;
				while (*p && p-hl->text<linelength-10) p++;
				while (*p && *p!=' ') p++;
				if (*p==' ')
				{	*p=0; output1("%s\n",hl->text); output1("%s\n",p+1);
					*p=' ';
				}
				else output1("%s\n",hl->text);
			}
			count++;
		}
		hl=hl->next;
	}
	if (count>0)
		output1("\nFor more information try: mxmhelp %s\n",text);
	return count;
}

// ******************* Maxima start ************************

/*
Load the maximastartup.txt file to memory.
*/
void loadmaximastartup (char *filename)
{	
	char line[MAXLINE];
	FILE *in=fopen(filename,"r");
	if (!in) return;
	helpline *hl=0;
	while (!feof(in))
	{	if (!fgets(line,MAXLINE-2,in)) break;
		int l=(int)strlen(line);
		char *buf=(char *)malloc(l+1);
		strcpy(buf,line);
		helpline *nexthl=(helpline *)malloc(sizeof(helpline));
		nexthl->next=0;
		nexthl->text=buf;
		if (maximastartupfirstline==0) maximastartupfirstline=nexthl;
		else hl->next=nexthl;
		hl=nexthl;
	}
	fclose(in);
}

/*
Euler command to start Maxima. Gets the directory string, and the
port on the Euler stack.
*/
void mxmstart (header *hd)
{	
	if (maxima_running())
	{	output("Maxima already running!\n");
		error=1; return;
	}
	header *st=hd,*result;
	hd=getvalue(hd);
	if (error) return;
	if (hd->type!=s_string)
	{	output("Need a path to Maxima!\n");
		error=4000; return;
	}

	// start Maxima:
	start_maxima(stringof(hd));

	result=new_string("Maxima started",32,"");
	moveresult(st,result);
}

/**
Euler function mxmstop to kill Maxima.
*/
void mxmstop (header *hd)
{	header *st=(header *)newram,*result;
	end_maxima();
	result=new_string("Maxima stopped!",32,"");
	moveresult(st,result);
}

extern int answermaxima;

/*
Called to parse a string through Maxima.
*/
char * callmxm (char *arg, int autoanswer, int fixunderscore)
{	
	// print("callmxm(%s)\n",arg);

	if (!maxima_running()) // Maxima not started
	{	
		start_maxima(getstartdir());
	}
	if (display2d) // make sure, we are not in 2d Maxima mode.
	{	
		runmaxima("display2d:false;",4,0);
		display2d=0;
	}
	
	if (strlen(arg)<1) return "";
	if (strlen(arg)>MAXMXOUTPUT)
	{	
		output("Maxima input too long!\n");
		error=1; return "";
	}

	// Build the Maxima command from the string
	static char s[MAXMXOUTPUT+100];
	char c=arg[strlen(arg)-1];
	if (c==';' || c=='$') sprintf(s,"%s",arg);
	else sprintf(s,"%s;",arg);

	// Send it, collect output, dont print:
	runmaxima(s,2,2);

	// Got a maxima prompt, which is a Maxima question?
	if (maxima_question())
	{	
		if (!autoanswer) 
		{
			maxima_break(); // break questions
			print("Maxima is asking\n%s\nUse assume!\n",maxima_output());
			error=1; return "";
		}
		char *q=maxima_output();
		char *p=q+strlen(q);
		char answer[MAXLINE];
		*answer=0;
		int question=0;
		int or=0;
		while (p>q)
		{
			p--;
			if (*p=='?' && !question) { *p=0; question=1; }
			if (*p==' ' && question && !*answer) strcpy(answer,p+1);
			if (question && strncmp(p,"nonzero",7)==0) strcpy(answer,"nonzero");
			if (question && strncmp(p,"positive",8)==0) strcpy(answer,"positive");
			if (question && strncmp(p,"equal",5)==0) strcpy(answer,"no");
			if (question && strncmp(p,"or ",3)==0) or=1;
			if (*answer && (*p==10 || *p==13)) 
			{
				p++; break;
			}
		}
		if (question && *answer)
		{
			// if (or) strcpy(answer,"unknown");
			if (!answermaxima) print("Answering \"%s?\" with \"%s\"\n",p,answer);
			return callmxm(answer,autoanswer-1);
		}
		else
		{
			print("Cannot answer \"%s\"\n",p);
			error=1; return "";
		}
	}

	// analyse the Maxima output and get a string from it:

	char *in=maxima_output();
	char *p=in;

	if (strlen(in)<3) return "";

	s[0]=0;
	
	int foundoutput=0;
	
	// skip all blanks and new line characters and collect in out.
	while (1)
	{	
		if (!foundoutput && strncmp(p,"(%o",3)==0) // is there a Maxima output? skip it.
		{	
			while (*p && *p!=' ') p++; // to next blank
			foundoutput=true;
		}
		while (*p==' ') p++;
		char *q=p;
		while (*q && *q!='\n') q++;
		if (q>p && foundoutput)
		{	
			if (strlen(s)+(q-p)>MAXMXOUTPUT)
			{	error=1;
				output("\nMaxima output too long!\n");
				s[0]=0; return s;
			}
			if (*(q-1)=='\\') strncat(s,p,q-p-1); // add to the out string
			else strncat(s,p,q-p); // add to the out string
		}
		if (*q==0) break;
		p=q+1;
	}

	if (!foundoutput)
	{
		print("Maxima said:\n");
		print("%s\n",in);
		error=10;
	}

	static char su[MAXMXOUTPUT+100];
	if (fixunderscore)
	{
		char *p=s;
		char *q=su;
		while (*p && q-su<MAXMXOUTPUT+98)
		{
			if (*p=='_') *q++='_';
			*q++=*p++;
		}
		*q=0;
		return su;
	}
	else return s;
}

char * mxmp1 (double x, int fractional)
{	static char s[32];
	if (fractional) tofrac(x,s);
	else sprintf(s,"%0.16g",x);
	return s;
}

char * mxmp2 (double x, int fractional)
{	static char s[32];
	if (fractional) tofrac(x,s);
	else sprintf(s,"%0.16g",x);
	return s;
}

/*
Translate an Euler item into a Maxima string
*/
char * tomxm (header *hd, char *start, int fr)
{	
	hd=getvalue(hd);
	static char s[MAXLINE];
	double *m;
	int r,c;
	if (start) strcpy(s,start);
	else s[0]=0;
	char *p=s+strlen(s);
	if (hd->type==s_real)
	{	sprintf(p,"%s",mxmp1(*realof(hd),fr));
	}
	else if (hd->type==s_complex)
	{	sprintf(p,"%s+I*%s",mxmp1(*realof(hd),fr),mxmp2(*(realof(hd)+1),fr));
	}
	else if (hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		if (r==1)
		{	strcpy(p,"["); p+=strlen(p);
			for (int i=0; i<c; i++)
			{	if (i<c-1) sprintf(p,"%s,",mxmp1(*m++,fr));
				else sprintf(p,"%s",mxmp1(*m++,fr));
				p+=strlen(p);
				if (p-s>MAXLINE-20)
				{	output("String too long for Maxima!\n"); error=1;
					return s;
				}
			}
			strcpy(p,"]"); p+=strlen(p);
		}
		else
		{	strcpy(p,"matrix("); p+=strlen(p);
			for (int j=0; j<r; j++)
			{	strcpy(p,"["); p+=strlen(p);
				for (int i=0; i<c; i++)
				{	if (i<c-1) sprintf(p,"%s,",mxmp1(*m++,fr));
					else sprintf(p,"%s",mxmp1(*m++,fr));
					p+=strlen(p);
					if (p-s>MAXLINE-20)
					{	output("String too long for Maxima!\n"); error=1;
						return s;
					}
				}
				if (j<r-1) strcpy(p,"],");
				else strcpy(p,"]"); 
				p+=strlen(p);
			}
			strcpy(p,")"); p+=strlen(p);
		}
	}
	else if (hd->type==s_cmatrix)
	{	getmatrix(hd,&r,&c,&m);
		if (r==1)
		{	strcpy(p,"["); p+=strlen(p);
			for (int i=0; i<c; i++)
			{	if (i<c-1) sprintf(p,"%s+I*%s,",mxmp1(*m,fr),mxmp2(*(m+1),fr));
				else sprintf(p,"%s+I*%s",mxmp1(*m,fr),mxmp2(*(m+1),fr));
				m+=2;
				p+=strlen(p);
				if (p-s>984)
				{	output("String too long for Maxima!\n"); error=1;
					return s;
				}
			}
			strcpy(p,"]"); p+=strlen(p);
		}
		else
		{	strcpy(p,"matrix("); p+=strlen(p);
			for (int j=0; j<r; j++)
			{	strcpy(p,"["); p+=strlen(p);
				for (int i=0; i<c; i++)
				{	if (i<c-1) sprintf(p,"%s+I*%s,",mxmp1(*m,fr),mxmp2(*(m+1),fr));
					else sprintf(p,"%s+I*%s",mxmp1(*m,fr),mxmp2(*(m+1),fr));
					m+=2;
					p+=strlen(p);
					if (p-s>984)
					{	output("String too long for Maxima!\n"); error=1;
						return s;
					}
				}
				if (j<r-1) strcpy(p,"],");
				else strcpy(p,"]"); 
				p+=strlen(p);
			}
			strcpy(p,")"); p+=strlen(p);
		}
	}
	else if (hd->type==s_string)
	{	
		strcpy(p,stringof(hd));
	}
	else
	{	output("Cannot convert this for Maxima!\n"); error=1;
	}
	return s;
}

void mconvertmxm (header *hd)
{	
	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	char *s=tomxm(hd,0,0); if (error) return;
	result=new_string(s,strlen(s),"");
	moveresult(st,result);
}

/*
Print help for a Maxima function. Search info/maxima.info-1 and
info/maxima.info-2 to find the item.
*/
void do_mxmhelp ()
{	
	char name[256];
	char *p;

	// get help item from command line
	while (*next==' ') next++;
	p=name;
	if (*next=='\"')
	{	
		next++;
		while (*next!=0 && *next!='\"')
		{	
			*p++=*next++;
			if (p-name>254) break;
		}
		if (*next!=0) next++;
	}
	else
	{	
		while (*next!=0 && *next!=' ' && *next!=';')
		{	*p++=*next++;
			if (p-name>254) break;
		}
	}
	*p=0;
	if (!*name)
	{	
		output1("Need a topic in mxmhelp!\n");
	}

	char filename[MAXLINE];

	for (int i=1; i<=12; i++)
	{	
		sprintf(filename,"%s/info/maxima.info-%d",getmaximadir(),i);
		FILE *in=fopen(filename,"r");
		if (!in) 
		{	
			if (i==1)
			{
				output1("Could not open the info file\n");
				error=1; return;
			}
			break;
		}
		while (!feof(in))
		{	
			char *lin=newline(in);
			if (!lin) break;
			next:
			char *p;
			if (!strncmp(lin," -- ",4))
			{	p=strstr(lin,name);
				if (p && p>lin && *(p-1)==' ' && 
					(*(p+strlen(name))==' ' || *(p+strlen(name))==0))
				{
					output1("%s\n\n",lin);
					lin=newline(in);
					if (!lin) break;
					output1("%s\n\n",lin);
					lin=newline(in);
					if (!lin) break;
					while ((*lin==' ' || *lin==9 || *lin==0) && strncmp(lin," -- ",4))
					{	output1("%s\n",lin);
						lin=newline(in);
						if (!lin) break;
					}
					goto next;
				}
			}
		}
		fclose(in);
	}

}

extern char none[]="\x01";
char latexline[MAXLINE+20];

char *tex (char *formula)
{
	strcpy(latexline,"tex(");
	strcat(latexline,formula);
	strcat(latexline,");");
	runmaxima(latexline,2,0);

	// try to find tex output
	char *pout=maxima_output();
	char *p=pout;
	if (error) 
	{
		error=0; print("Maxima error in %s command.\n%s",latexline);
		return none;
	}

	char *q=latexline;
	*q=0;
	int found=0;
	while (*p)
	{
		if (*p=='$' && *(p+1)=='$') 
		{
			if (found) 
			{
				*q=0; break;
			}
			p+=2; found=1;
		}
		else
		{
			if (found) *q++=*p++;
			else p++;
		}
	}

	return latexline;
}

void mtex (header *hd)
{
	header *st=hd;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) need_arg_in("tex","string");
	char *s=tex(stringof(hd)); if (error) return;
	header *res=new_string(s,strlen(s),"");
	moveresult(st,res);
}
