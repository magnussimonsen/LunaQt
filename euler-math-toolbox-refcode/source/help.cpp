#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "header.h"
#include "help.h"
#include "builtin.h"
#include "sysdep.h"
#include "stack.h"
#include "udf.h"
#include "maxima.h"
#include "aliases.h"
#include "mainloop.h"

static helpitem *firstitem=0,*lastitem=0;

#define EXTENSION ".e"
extern char *path[32];
extern int npath;

extern char * udf_gethelp (header *hd, int text=1);
extern char * getexternhelp (char *name);

// ************** helper functions *************

char * newline (FILE *in)
	// read a new line from help.txt
{   
	static char line[MAXLINE];
	int l;
	if (feof(in)) return 0;
	if (!fgets(line,MAXLINE-2,in)) return 0;
	if (feof(in)) return 0;
	l=(int)strlen(line);
	if (l>0 && line[l-1]=='\n') line[l-1]=0;

	// Remove UTF-8 marker from the line
	char* p = line, * p1 = line;
	while (*p)
	{
		if (*p != 'Â')
		{
			*p1++ = *p;
		}
		p++;
	}
	*p1 = 0;

	return line;
}

int length_compare (const char **p1, const char **p2)
{	
	int n=strlen(*p1);
	int m=strlen(*p2);
	if (n<m) return -1;
	else if (n>m) return 1;
	else return 0;
}

// ************** functions for help.txt **********

/** 
Load a single help item from the file.
*/
helpitem * loaditem (FILE *in)
{	
	char *l;
	helpline *h,*hn,*hend=0;
	helpitem *hin;
	// read file for line starting with !
	while (1)
	{	
		l=newline(in);
		if (!l) return 0; // end of file, return 0
		if (*l=='!') break;
	}
	// copy line of type "sin,cos,log,tan" to heap
	// add help item to list of help items
	hin=(helpitem *)malloc(sizeof(helpitem));
	hin->next=0;
	hin->item=(char *)malloc(strlen(l+1)+2);
	strcpy(hin->item,l+1);
	// change comma to 0 to separate items
	char *q=hin->item;
	while (*q)
	{
		if (*q==',') *q=0;
		q++;
	}
	q++; *q=0;
	// read help section
	h=0;
	while (1)
	{   
		l=newline(in);
		if (!l || *l=='#') break; // break at file end or line "#"
		if (*l=='/' && *(l+1)=='/') continue; // read over comments
		// add help line to list of help lines
		hn=(helpline *)malloc(sizeof(helpline));
		hn->next=0;
		hn->text=(char *)malloc(strlen(l)+1);
		strcpy(hn->text,l);
		if (!h) h=hend=hn;
		else { hend->next=hn; hend=hn; }
	}
	hin->help=h;
	return hin;
}

/** 
Load the help file help.txt to memory.
*/
void loadhelp (char *filename, int more)
{	
	FILE *in=fopen(filename,"r");
	helpitem *newitem;
	if (!in) return;
	if (!more) 
	{
		lastitem=firstitem=0;
	}
	while (1)
	{	
		newitem=loaditem(in);
		if (!newitem) break;
		if (!firstitem)
		{	lastitem=firstitem=newitem;
		}
		else
		{	lastitem->next=newitem;
		lastitem=newitem;
		}
	}
	fclose(in);
}

/** 
Unload the help file from memory.
*/
void unloadhelp ()
{	
	helpitem *h=firstitem,*hn;
	helpline *l,*ln;
	while (h)
	{	free(h->item);
	l=h->help;
	while (l)
	{	free(l->text);
	ln=l->next;
	free(l);
	l=ln;
	}
	hn=h->next;
	free(h);
	h=hn;
	}
}

/**
Finde a help topic on the help.txt file.
*/
int externhelp (char *origtext)
{   
	char *text=origtext;
	int retry=0;
retry:
	helpitem *h=firstitem;
	helpline *hl;
	char s[256],*p,*q;
	while (h)
	{	
		p=h->item;
		while(*p)
		{   
			q=s;
			while (*p) *q++=*p++;
			*q=0;
			if (strcmp(s,text)==0)
			{   
				if (retry)
				{
					println("%s is an alias for %s",origtext,text);
				}
				// output1("\n%s\n\n",h->item);
				hl=h->help;
				while (hl)
				{	
					println("%s",hl->text);
					hl=hl->next;
				}
				println("");
				return 1;
			}
			p++;
			if (*p==0) break;
		}
		h=h->next;
	}
	if (!retry)
	{
		retry=1;
		text=getalias(text);
		if (text) goto retry;
	}
	return 0;
}

/** 
Find a help topic.
*/
char * getexternhelp (char *text)
{   
	helpitem *h=firstitem;
	helpline *hl;
	char s[256],*p,*q;
	while (h)
	{	
		p=h->item;
		while(*p)
		{   
			q=s;
			while (*p) *q++=*p++;
			*q=0;
			if (strcmp(s,text)==0)
			{   
				hl=h->help;
				while (hl)
				{	
					char *hh=hl->text;
					while (*hh==' ') hh++;
					if (!strncmp(text,hh,strlen(text)) &&
						hh[strlen(text)]=='('
						) 
						return hh;
					hl=hl->next;
				}
			}
			p++;
			if (*p==0) break;
		}
		h=h->next;
	}
	h=firstitem;
	while (h)
	{	
		p=h->item;
		while(*p)
		{   q=s;
		while (*p) *q++=*p++;
		*q=0;
		if (strcmp(s,text)==0)
		{   hl=h->help;
		while (hl)
		{	if (hl->text) return hl->text;
		hl=hl->next;
		}
		return "";
		}
		p++;
		if (*p==0) break;
		}
		h=h->next;
	}
	return "";
}

// ************* Last Topic of the Status Line *********

char laststatustopic[64]="";

void putstatustopic (char *s, int maxima)
{
	if (strlen(s)<62) 
	{
		char *l=laststatustopic;
		if (maxima) *l++='&';
		strcpy(l,s);
	}
}

char *getstatustopic ()
{
	return laststatustopic;
}

// **************** help for the status line *********

#define MAXFOUND 128

/** 
Get help for the status line.
*/
void gethelp (char *xstart, char *extend, char *help)
{	
	*extend=0; *help=0;
	header *hd=(header *)ramstart;
	builtintyp *b=builtin_list;
	commandtyp *c=command_list;

	int n=0; // number of found items
	char *found [MAXFOUND];

	char start[MAXLINE];
	char *h=start;
	while (*xstart && h-start<MAXLINE-2)
	{
		if (*xstart=='\\' && *(xstart+1))
		{
			xstart++; *h++=*xstart++;
		}
		else if (*xstart=='_' && *(xstart+1)=='_')
		{
			xstart++; *h++=*xstart++;
		}
		else *h++=*xstart++;
	}
	*h=0;
	int ln=(int)strlen(start);

	// collect builtin functions
	while (b->name)
	{	
		if (!strncmp(start,b->name,ln) && n<MAXFOUND)
		{	found[n++]=b->name;
		}
		b++;
	}

	// collet udf functions
	while (hd<(header *)udfend && n<MAXFOUND)
	{	
		if (!strncmp(start,hd->name,ln))
		{	found[n++]=hd->name;
		}
		hd=nextof(hd);
	}

	// collect builtin commands
	while (c->name)
	{	
		if (!strncmp(start,c->name,ln) && n<MAXFOUND)
		{	found[n++]=c->name;
		}
		c++;
	}

	char sq[MAXLINE];

	// collect aliases
	strncpy(sq,start,ln);
	start[ln]=0;
	n=getaliases(sq,found,n,MAXFOUND);

	// no match found
	if (n==0)
	{	
		char *q=start;
		while (*q && *q!='(') q++;
		if (*q=='(')
		{
			ln=q-start;
			strncpy(sq,start,ln);
			sq[ln]=0;
			char *name=getalias(sq);
			if (name) strcpy(sq,name);
			header *hd=(header *)ramstart;
			while (hd<(header *)udfend)
			{	
				if (!strcmp(sq,hd->name))
				{	
					strcpy(help,udf_gethelp(hd));
					strcpy(extend,hd->name);
					putstatustopic(hd->name);
					return;
				}
				hd=nextof(hd);
			}
			builtintyp *b=builtin_list;
			while (b->name)
			{	
				if (!strcmp(sq,b->name))
				{	
					strcpy(help,getexternhelp(b->name));
					strcpy(extend,b->name);
					putstatustopic(b->name);
					return;
				}
				b++;
			}
		}
	}

	// one match found
	if (n==1)
	{	
		strcpy(sq,found[0]);
		char *name=getalias(sq);
		if (name) strcpy(sq,name);
		header *hd=(header *)ramstart;
		while (hd<(header *)udfend)
		{	
			if (!strcmp(sq,hd->name))
			{	
				strcpy(help,udf_gethelp(hd));
				strcpy(extend,hd->name);
				putstatustopic(hd->name);
				return;
			}
			hd=nextof(hd);
		}
		builtintyp *b=builtin_list;
		while (b->name)
		{	
			if (!strcmp(sq,b->name))
			{	
				char *h=getexternhelp(b->name);
				strcpy(help,h);
				strcpy(extend,b->name);
				putstatustopic(b->name);
				return;
			}
			b++;
		}

	}

	// sort the found help topics:
	qsort(found,n,sizeof(char *),
		(int (*) (const void *, const void *))length_compare);

	// collect into output line
	int l=0;
	for (int i=0; i<n; i++)
	{	
		unsigned int k=strlen(found[i]);
		int dbl=0;
		// check for double entries
		for (int j=i-1; j>=0; j--)
		{	
			if (!strcmp(found[j],found[i]))
			{	
				dbl=1; break;
			}
			if (strlen(found[j])<k) break;
		}
		// append to help line, if not double entry
		if (!dbl)
		{	
			l+=k+1;
			if (l>MAXLINE) break; // too many items
			strcat(help,found[i]);
			strcat(help," ");
			strcat(extend,found[i]);
			strcat(extend," ");
		}
	}
}

// ************* Last Topics List for the Help Window *************

#define MAXLASTTOPICS 16
char lasttopic[MAXLASTTOPICS][64];
int nlasttopics=0;

char *getlasttopics ()
	// return a string with last topics, separated by comma
{
	static char s[MAXLINE];
	*s=0;
	int length=0;
	for (int i=0; i<nlasttopics; i++)
	{
		if (i>0) 
		{	
			int n=strlen(s);
			if (n>0 && s[n-1]==' ') s[n-1]=0;
			strcat(s,", "); length+=2;
		}
		if (length>60) 
		{
			strcat(s,"\r\n"); length=0;
		}
		strcat(s,lasttopic[i]);
		length+=(int)strlen(lasttopic[i])+2;
	}
	return s;
}

void putlasttopic (char *s, int type)
	// put a topic to the list of last topics
{
	if (strlen(s)>63) return;
	char t[64]="";
	if (type==1) strcat(t,"&");
	strcat(t,s);
	if (type==2)
	{
		int n=strlen(t);
		if (n>1 && t[n-1]==' ') t[n-1]=0;
	}
	s=t;
	for (int i=0; i<nlasttopics; i++)
	{
		if (!strcmp(lasttopic[i],s)) return;
	}
	if (nlasttopics>=MAXLASTTOPICS)
	{
		for (int i=0; i<nlasttopics-1; i++)
			strcpy(lasttopic[i],lasttopic[i+1]);
		nlasttopics--;
	}
	strcpy(lasttopic[nlasttopics++],s);
}

// ************** Routines for help in the help window *****************

char *currentout;

void addoutx (char *s, ...)
{	
	char text [MAXLINE];
	va_list v;
	va_start(v,s);
	vsprintf(text,s,v);
	strcpy(currentout,text);
	currentout+=strlen(currentout);
}

void addoutxln (char *s, ...)
{	
	char text [MAXLINE];
	va_list v;
	va_start(v,s);
	vsprintf(text,s,v);
	strcpy(currentout,text);
	strcat(currentout,"\r\n");
	currentout+=strlen(currentout);
}

void addout (char *s)
{	
	strcpy(currentout,s);
	currentout+=strlen(currentout);
}

void addoutln (char *s="")
{
	addout(s);
	addout("\r\n");
}

/** 
Print parametes of UDF.
*/
void udf_forhelpwindow (header *hd)
{
	char *name=hd->name;
	char *co=currentout;
	addoutx("function ");
	if (hd->flags & COMMENT) addoutx("comment ");
	if (hd->flags & MAP) addoutx("map ");
	if (hd->flags & ARGS) addoutx("args ");
	if (hd->flags & STRONG) addoutx("strong ");
	if (hd->flags & PREFIX) addoutx("prefix ");
	if (hd->flags & INFIX) addoutx("operator ");
	if (hd->flags & POSTFIX) addoutx("postfix ");
	if (hd->flags & MATLAB) addoutx("matlab ");
	if (hd->flags & PYTHONFLAG) addoutx("python ");
	addoutx("%s (",name);
	char *p=helpof(hd);
	int count=*((int *)p);
	p+=3*sizeof(inttyp);
	int argcount=0;
	int ol=0;
	ol+=(int)(currentout-co);

	// print parameter
	for (int i=0; i<count; i++)
	{	
		int defaults=*((int *)p);
		p+=sizeof(inttyp);
		if (defaults&ParamStartsVector) addout("[");
		addoutx("%s",p);
		ol+=strlen(p);
		p+=maxname+sizeof(inttyp);
		if (defaults&ParamHasValue)
		{	
			p=(char *)nextof((header *)p);
		}
		// char *sd=print_defaults(defaults);
		// addout(sd); ol+=strlen(sd);
		if (defaults&ParamEndsVector) addout("]");
		if (i!=count-1) { addout(", "); ol+=2; }
		argcount++;
		if (ol>linelength-15)
		{	addout(" ..\r\n");
		ol=0;
		}
	}
	addout(")\r\n");
	if (hd->flags&COMMENT) addoutx("\r\nComment function.\r\n");
}

/** 
Print defaults of UDF.
*/
void udf_defaults_forhelpwindow (header *hd)
{
	char *co=currentout;
	char *p=helpof(hd);
	int count=*((int *)p);
	p+=3*sizeof(inttyp);
	char *pnote=p;
	int ol=0;
	ol+=(int)(currentout-co);

	// print parameter
	for (int i=0; i<count; i++)
	{	
		p+=sizeof(inttyp);
	}
	p=pnote;
	int havedefaults=0;
	for (int i=0; i<count; i++)
	{	
		int defaults=*((int *)p);
		p+=sizeof(inttyp);
		if (defaults&ParamHasValue)
		{	if (!havedefaults) addout("\r\n");
		addoutx("Default for %s : ",p);
		havedefaults=1;
		}
		p+=maxname+sizeof(inttyp);
		if (defaults&ParamHasValue)
		{	
			header *hp=(header *)p;
			if (isnone(hp))	addout("none\r\n");
			else
			{	
				if (hp->type==s_matrix || hp->type==s_cmatrix ||
					hp->type==s_imatrix || hp->type==s_smatrix ||
					hp->type==s_cpxmatrix) addout("\r\n");
				outputbuffer=currentout; outputbufferend=currentout+1024;
				give_out(hp);
				outputbuffer=0;
				currentout+=strlen(currentout);
			}
			p=(char *)nextof((header *)p);
		}
	}
	if (havedefaults) addout("\r\n");
}

/**
Print the help lines of a function, including the defaults.
*/
void putudfhelp (header *hd, char *out, char *alias=0)
{
	currentout=out;

	putlasttopic(hd->name);

	if (alias)
	{
		addout(alias);
		addout(" is an alias to ");
	}
	addout(hd->name);
	addout("\r\n\r\n");

	udf_forhelpwindow(hd);
	addout("\r\n");

	char *end=udfof(hd);
	char *p=helpof(hd);
	int count=*((int *)p);
	p+=3*sizeof(inttyp);

	for (int i=0; i<count; i++)
	{	
		int defaults=*((int *)p);
		p+=sizeof(inttyp);
		p+=maxname+sizeof(inttyp);
		if (defaults&ParamHasValue)
		{	
			p=(char *)nextof((header *)p);
		}
	}

	// print help lines
	while (*p!=1 && p<end)
	{	
		if (*(p+2)==' ') addout(p+3); 
		else addout(p+2);
		addout("\r\n");
		p+=strlen(p); p++;
	}
	udf_defaults_forhelpwindow(hd);
}

/**
Print help for a built-in function.
*/
int putbuiltinhelp (char *origtext, char *out)
{   
	char *text=origtext;
	int retry=0;
retry:

	currentout=out;

	if (strlen(text)==0) return 0;
	char *ph=text+strlen(text)-1;
	if (*ph==' ') *ph=0;

	helpitem *h=firstitem;
	helpline *hl;
	while (h)
	{	
		char *p=h->item;
		while(*p)
		{   
			if (strcmp(p,text)==0)
			{   
				if (retry)
				{
					addout(origtext);
					addout(" is an alias to ");
				}
				addout(text);
				addout("\r\n");
				putlasttopic(text);
				hl=h->help;
				while (hl)
				{	
					addout(hl->text);
					addout("\r\n");
					hl=hl->next;
				}
				return 1;
			}
			while (*p) p++;
			p++;
			if (*p==0) break;
		}
		h=h->next;
	}
	if (!retry)
	{
		retry=1;
		text=getalias(text);
		if (text) goto retry;
	}
	return 0;
}

extern maximahelpline *maximafirstline;

char firsttopic[512]="";

/**
Print help for the command from the Maxima help files maxima-info-*.
*/
void putmaximahelp (char *text, char *out)
{
	currentout=out;

	int sub=0;
	if (*text=='*')
	{
		sub=1; text++;
	}

	int l=strlen(text);
	if (l==0) return;

	if (text[l-1]==' ' || text[l-1]=='(')
	{
		text[l-1]=0;
onlyone :
		strcpy(firsttopic,text);
		char filename[MAXLINE];

		for (int i=1; i<=12; i++)
		{	
			sprintf(filename,"%s/info/maxima.info-%d",getmaximadir(),i);
			FILE *in=fopen(filename,"r");
			if (!in) return;
			char *p;
			while (!feof(in))
			{	
				char *lin=newline(in);
				if (!lin) break;
next:
				if (!strncmp(lin," -- ",4))
				{	
					p=strstr(lin,text);
					if (p && p>lin && *(p-1)==' ' && 
						(*(p+strlen(text))==' ' || *(p+strlen(text))==0))
					{
						addout(lin);
						addout("\r\n");
						lin=newline(in);
						if (!lin) break;
						addout(lin);
						addout("\r\n");
						lin=newline(in);
						if (!lin) break;
						putlasttopic(text,1);
						while ((*lin==' ' || *lin==9 || *lin==0) && strncmp(lin," -- ",4))
						{	
							addout(lin);
							addout("\r\n");
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
	else
	{
		char *hfound=0;
		int count=0;
		maximahelpline *hl=maximafirstline;
		while (hl)
		{
			if (!strncmp(hl->item,text,l) ||
				(sub && strstr(hl->item,text)!=0))
			{
				hfound=hl->item;
				addout(hl->text);
				addout("\r\n");
				count++;
				if (count==1)
				{
					strcpy(firsttopic,"&");
					strcat(firsttopic,hl->item);
				}
				if (count>32) break;
			}
			hl=hl->next;
		}
		if (count==1) 
		{
			text=hfound;
			goto onlyone;
		}
	}
}

void strtrim (char *s)
{
	int l=strlen(s);
	char *p=s+l-1;
	while (l>0)
	{
		if (*p==' ') 
		{
			*p=0;
			p--; l--;
		}
		else break;
	}
}

char *putlasttopics (char *help)
{
	strcpy(help,getlasttopics());
	strcat(help,"\r\n----------------------------------------------------------------------\r\n\r\n");
	return help+strlen(help);
}

void findhelp (char *text, char *help);

/**
Get help for the help window.
The help item is searched at the position start.
The help result is stored to the position help.
*/
void getwindowhelp (char *start, char *help)
{	
	help=putlasttopics(help);

	if (*start=='?' && strlen(start)>3)
	{
		start++;
		findhelp(start,help);
		return;
	}

	if (strncmp(start,"type ",5)==0 && strlen(start)>5)
	{
		outputbuffer=help; outputbufferend=help+32000;
		strcpy(start,start+5);
		strtrim(start);
		error=0; do_type_name(start);
		outputbuffer=0;
		return;
	}

	if (strncmp(start,"list ",5)==0 && strlen(start)>5)
	{
		outputbuffer=help; outputbufferend=help+32000;
		strcpy(start,start+5);
		do_list_name(start);
		outputbuffer=0;
		return;
	}

	if (strncmp(start,"listvar ",8)==0)
	{
		outputbuffer=help; outputbufferend=help+32000;
		strcpy(start,start+8);
		do_listvar_name(start);
		outputbuffer=0;
		return;
	}

	if (!*start) strcpy(start,"Topics ");

	strcpy(firsttopic,"");

	*help=0;
	header *hd=(header *)ramstart;
	int ln=(int)strlen(start);
	int n=0; // number of found items
	char *found [MAXFOUND];

	int le=strlen(start);
	if (le>1 && start[le-1]=='(') start[le-1]=' ';

	if (le>3 && !strcmp(start+(le-strlen(EXTENSION)),EXTENSION))
		// help on an Euler file
	{
		help_file(start,help);
		return;
	}

	if (strncmp(start,"load ",5)==0)
	{
		help_file(start+5,help);
		return;
	}

	if (*start=='&' || *start==' ') 
		// this marks the search for Maxima commands
	{
		int maximafunction=0;
		if (*start=='&')
		{
			char name[maxname];
			strcpy(name,start);
			int le=strlen(name);
			while (le>0 && name[le-1]==' ') le--;
			name[le]=0;
			maximafunction=(searchalludf(name)!=0);
		}
		putmaximahelp(start+1,help);
		// all done by putmaximahelp
		if (!maximafunction) return;
	}

	int builtinonly=0;
	// marks the search for builtin commands only
	if (*start=='_' && *(start+1)) 
	{
		builtinonly=1; start++;
	}

	int sub=0;
	if (*start=='*')
		// marks the search for Maxima commands only
	{
		sub=1; start++;
	}

	// search through the help items in help.txt
	helpitem *h=firstitem;
	while (h)
	{	
		char *p=h->item;
		// the h->item is sequence of 0-terminated strings.
		// the last string is followed by an empty string.
		while(*p)
		{   
			if ((strncmp(start,p,ln)==0 
				|| (sub && strstr(p,start)!=0))
				&& n<MAXFOUND)
			{   
				found[n++]=p;
			}
			// goto the next string
			while (*p) p++;
			p++;
		}
		h=h->next;
	}

	// collect udf functions
	if (!builtinonly)
	{
		while (hd<(header *)udfend && n<MAXFOUND)
		{	
			if ((!strncmp(start,hd->name,ln) 
				|| (sub && strstr(hd->name,start)!=0))
				&& n<MAXFOUND)
			{	found[n++]=hd->name;
			}
			hd=nextof(hd);
		}
	}

	// collect aliases
	char sq[MAXLINE];
	strncpy(sq,start,ln);
	sq[ln]=0;
	n=getaliases(sq,found,n,MAXFOUND);

	// look for udf help line
	if (n==0)
	{	
		char *q=start;
		if (!builtinonly)
		{
			while (*q && *q!=' ') q++;
			if (*q==' ')
			{	
				ln=q-start;
				header *hd=(header *)ramstart;
				while (hd<(header *)udfend)
				{	
					if ((int)strlen(hd->name)==ln && !strncmp(start,hd->name,ln))
					{	
						putudfhelp(hd,help);
						return;
					}
					hd=nextof(hd);
				}	
				char sq[MAXLINE];
				strncpy(sq,start,ln);
				sq[ln]=0;
				char *sh=getalias(sq);
				if (sh)
				{
					header *hd=(header *)ramstart;
					while (hd<(header *)udfend)
					{	
						if (!strcmp(hd->name,sh))
						{	
							putudfhelp(hd,help,sq);
							return;
						}
						hd=nextof(hd);
					}
				}
			}
			q=start;
		}
		if (!putbuiltinhelp(start,help))
		{
			findhelp(start,help);
			// strcat(help,"No topics found.");
		}
		return;
	}

	// look for udf help line
	if (n==1 && !sub)
	{	
		if (!builtinonly)
		{
			header *hd=(header *)ramstart;
			while (hd<(header *)udfend)
			{	
				if (!strcmp(found[0],hd->name))
				{	
					putudfhelp(hd,help);
					return;
				}
				hd=nextof(hd);
			}
			char *sq=getalias(found[0]);
			if (sq)
			{
				header *hd=(header *)ramstart;
				while (hd<(header *)udfend)
				{	
					if (!strcmp(sq,hd->name))
					{	
						putudfhelp(hd,help,found[0]);
						return;
					}
					hd=nextof(hd);
				}
			}
		}
		putbuiltinhelp(found[0],help);
		return;
	}

	// sort the found help topics:
	qsort(found,n,sizeof(char *),
		(int (*) (const void *, const void *))length_compare);

	if (n>0) strcpy(firsttopic,found[0]);

	// collect into output line
	int l=0;
	if (n>0)
	{
		for (int i=0; i<n; i++)
		{	
			unsigned int k=strlen(found[i]);
			int dbl=0;
			for (int j=i-1; j>=0; j--)
			{	
				if (!strcmp(found[j],found[i]))
				{	dbl=1; break;
				}
				if (strlen(found[j])<k) break;
			}
			if (!dbl)
			{	
				l+=k+1;
				if (l>60)
				{
					strcat(help,"\r\n");
					l=0;
				}
				strcat(help,found[i]);
				strcat(help," ");
			}
		}
	}
}

// ************ print help from commands in Euler file ***********

void stripnl (char *s)
{
	int n=strlen(s);
	if (n>0) s[n-1]=0;
}

void putout (char *s, char *out)
{
	if (out) addoutln(s);
	else print("%s\n",s);
}

int help_file (char *file, char *out)
{	
	if (out) currentout=out;

	FILE *in=open_file_in_path(file,"r",1,1);
	if (!in) return 0;

	putlasttopic(file,0);

	// if (out) addoutx("Euler file %s found.\r\nFile contains the following definitions:\r\n\r\n",fn);
	// else output1("Euler file %s found.\nFile contains the following definitions:\n\n",fn);

	char s[MAXLINE];

	int emptyline=1;
		
	while (!feof(in))
	{	
		if (fgets(s,MAXLINE-2,in)==0) break;
		stripnl(s);
		if (strncmp(s,"comment",strlen("comment"))==0 &&
			strncmp(s,"comments",strlen("comments"))!=0)
		{	
			while (!feof(in))
			{	
				if (fgets(s,MAXLINE-2,in)==0) break;
				stripnl(s);
				if (strncmp(s,"endcomment",strlen("endcomment"))==0) break;
				if (out) addoutln(s);
				else print("%s\n",s);
			}
			emptyline=0;
		}
		else if (strncmp(s,"// ",3)==0)
		{
			// if (out) addoutln(s+3);
			// else print("%s\n",s+3);
		}
		else if (!*s)
		{
			if (!emptyline)
			{
				if (out) addoutln();
				else output("\n");
			}
			emptyline=1;
		}
		else if (strstr(s,":=")!=0 && 
			strncmp(s,"function %",strlen("function %"))!=0 &&
			strstr(s,"?")==0)
		{	
			if (out) addoutln(s);
			else print("%s\n",s);
			emptyline=0;
		}
		else if (strncmp(s,"function %",strlen("function %"))==0)
		{	
			while (1)
			{	
				if (fgets(s,MAXLINE-2,in)==0) break;
				if (strncmp(s,"endfunction",strlen("endfunction"))==0) break;
			}
			emptyline=0;
		}
		else if (strncmp(s,"function",strlen("function"))==0 
			|| strncmp(s,"maximafunction",strlen("maximafunction"))==0)
		{	
			int havecomment=0,header=1;
			while (1)
			{	
				int ns=strlen(s)-1;
				while (s[ns]==' ' && ns>=0) ns--;
				if (header)
				{	
					putout(s,out);
					if (s[0]=='#')
					{	
						havecomment=1; header=0;
					}
				}
				if (s[0]=='#') 
				{	
					if (!havecomment) 
					{
						putout(s,out);
					}
					havecomment=1; 
					header=0;
				}
				else if (s[ns]==')')
				{	
					header=0;
				}
				if (fgets(s,MAXLINE-2,in)==0) break;
				stripnl(s);
				if (strncmp(s,"endfunction",strlen("endfunction"))==0) break;
			}
			emptyline=0;
		}
	}

	putout("",out);
	fclose(in);

	return 1;
}

// *************** help ... command **********

int do_example (char *name, int title);

void do_help (int maxima)
	/*
	Print help for a user defined function.
	*/
{	
	char name[MAXLINE];
	header *hd;
	int count,i,defaults,argcount;
	int found=0;
	char *p,*end,*pnote;
	int ampersand=0;
	builtintyp *b;

	// get help item from command line
	skip_space();
	if (*next=='&')
	{
		name[0]='&'; next++;
		scan_name(name+1);
		ampersand=1;
	}
	else 
		scan_name(name);
	if (!*name) strcpy(name,"");

	// look through udfs
	hd=searchalludf(name);
	int udf=0;
	if (hd && hd->type==s_udf)
	{   
		if (hd->flags&PYTHONFLAG) print("%s is a Python function.\n\n",name);
		else if (hd->flags&COMMENT) print("%s is a comment function or a maxima function.\n\n",name);
		else print("%s is an Euler function.\n\n",name);
		udf=1;
		found=1;
		if (hd->flags&MAP) output1("function map %s (",name);
		else output1("function %s (",name);
		end=udfof(hd);
		p=helpof(hd);
		count=*((int *)p);
		p+=3*sizeof(inttyp);
		pnote=p;
		argcount=0;
		int ol=0;
		if (hd->flags&MAP) ol=strlen("function map  (")+strlen(name);
		else ol=strlen("function  (")+strlen(name);

		// print parameter
		for (i=0; i<count; i++)
		{	
			defaults=*((int *)p);
			p+=sizeof(inttyp);
			if (defaults&ParamStartsVector) output("[");
			output1("%s",p);
			ol+=strlen(p);
			p+=maxname+sizeof(inttyp);
			if (defaults&ParamHasValue)
			{	p=(char *)nextof((header *)p);
			}
			// char *sd=print_defaults(defaults);
			// output(sd); ol+=strlen(sd);
			if (defaults&ParamEndsVector) output("]");
			if (i!=count-1) { output(", "); ol+=2; }
			argcount++;
			if (ol>linelength-15)
			{	output(" ..\n");
			ol=0;
			}
		}
		output(")\n");

		// print defaults
		p=pnote;
		int havedefaults=0;
		for (i=0; i<count; i++)
		{	defaults=*((int *)p);
		p+=sizeof(inttyp);
		if (defaults&ParamHasValue)
		{	if (!havedefaults) output("\n");
		output1("Default for %s : ",p);
		havedefaults=1;
		}
		p+=maxname+sizeof(inttyp);
		if (defaults&ParamHasValue)
		{	if (printdefaults)
		{	header *hp=(header *)p;
		if (isnone(hp))	output("none\n");
		else
		{	if (hp->type==s_matrix || hp->type==s_cmatrix ||
		hp->type==s_imatrix || hp->type==s_smatrix ||
		hp->type==s_cpxmatrix) output("\n");
		give_out(hp);
		}
		}
		p=(char *)nextof((header *)p);
		}
		}
		output("\n");

		// print help lines
		while (*p!=1 && p<end)
		{	
			if (*(p+2)==' ') output(p+3); 
			else output(p+2);
			output("\n");
			p+=strlen(p); p++;
		}
		output("\n");
	}

	// look through builtin functions and help.txt
	b=find_builtin(name);
	if (b)
	{   
		if (udf) output1("%s is also a builtin function.\n",name);
		else output1("%s is a builtin function.\n",name);
		found=1;
	}
	found=found|externhelp(name);

	// print content help for an Euler file
	if (!found && !ampersand)
	{	
		found=help_file(name);
		if (found) return;
	}

	// no help found
	if (!found && !ampersand)
	{	
		output1("No Euler help found for %s.\n",name);
	}

	// list all Maxima commands that fit

	if (maxima || ampersand)
	{
		if (!ampersand) output("\nSearch Maxima for help:\n\n");

		int mfound=printmaximahelp(ampersand?name+1:name);

		if (mfound) { output("\n"); found=1; }

		// try list ...
		if (!found)
		{	output("\nOther Euler Functions:\n");
			output("Try \"help item\" for help.\n");
			int count=do_list_name(name);
			if (count>0) found=1;
		}

		if (!mfound)
		{	
			char extend[MAXLINE];
			char status[MAXLINE];
			char *mh=extend;
			getmaximahelp(name,mh,status,0);
			if (mh && *mh) 
			{	output("\nOther Maxima functions.\n");
				output("Try \":: ??item\" for help.\n\n");
				p=mh;
				while (*p!=0)
				{	int lcount=0;
				while ((lcount<linelength-16 || *p!=' ') && *p) 
				{	p++; lcount++;
				}
				char c=*p;
				*p=0;
				output1("%s\n",mh);
				*p=c;
				while (*p==' ') p++;
				mh=p;
				}
			}
		}
	}

	error=0;
}

void do_help (void)
{
	do_help(0);
}

void do_addmenu (void)
	/*
	Print help for a user defined function.
	*/
{	
	static char name[MAXLINE];
	char *p;

	// get help item from command line
	while (*next==' ') next++;
	p=name;
	while (*next!=0)
	{	*p++=*next++;
	if (p-name>254) break;
	}
	*p=0;
	if (!*name) strcpy(name,"addmenu");
	addmenu(name);
}

void do_submenu (void)
	/*
	Print help for a user defined function.
	*/
{	
	static char name[MAXLINE];
	char *p;

	// get help item from command line
	while (*next==' ') next++;
	p=name;
	while (*next!=0)
	{	*p++=*next++;
	if (p-name>254) break;
	}
	*p=0;
	if (!*name) strcpy(name,"submenu");
	addsubmenu(name);
}


// ********************** Find a text in the help ****************

/**
Print a help line with eclipses.
*/
void print_help_line_eclipsed (char *line, char *text)
{
	addoutxln("  %s",line);
}

/**
Search through UDFs to find the text in the help lines.
Print the help line and the udf name.
*/
void search_in_udf_help (header *hd, char *text)
{
	char *end=udfof(hd);
	char *p=helpof(hd);
	int count=*((int *)p);
	p+=3*sizeof(inttyp);

	for (int i=0; i<count; i++)
	{	
		int defaults=*((int *)p);
		p+=sizeof(inttyp);
		p+=maxname+sizeof(inttyp);
		if (defaults&ParamHasValue)
		{	
			p=(char *)nextof((header *)p);
		}
	}

	int found=0;

	// print help lines
	while (*p!=1 && p<end)
	{	
		p+=2;
		if (*p==' ') p++;
		if (strstr(p,text))
		{
			if (!found) addoutxln("%s :",hd->name);
			if (found<10) print_help_line_eclipsed(p,text);
			found++;
		}
		p+=strlen(p); p++;
	}
}

/**
Search in help.txt for the text in help lines.
*/
void search_in_help_txt (char *text)
{
	helpitem *h=firstitem;
	helpline *hl;
	while (h)
	{	
		int found=0;
		hl=h->help;
		while (hl)
		{	
			if (strstr(hl->text,text))
			{
				if (!found) addoutxln("%s",h->item);
				if (found<10) print_help_line_eclipsed(hl->text,text);
				found++;
			}
			hl=hl->next;
		}
		h=h->next;
	}
}

/**
Search help in Maxima reference.
*/
void search_in_maxima_reference (char *text)
{
	static char h[MAXLINE];

	for (int i=1; i<=12; i++)
	{	
		sprintf(h,"%s/info/maxima.info-%d",getmaximadir(),i);
		FILE *in=fopen(h,"r");
		if (!in) return;
		while (!feof(in))
		{	
			char *lin=newline(in);
			if (!lin) break;
			int found=0;
			if (!strncmp(lin," -- ",4))
			{	
				lin+=4;
				while (*lin && *lin!=':') lin++;
				if (*lin==0) continue;
				lin++;
				while (*lin==' ') lin++;
				strcpy(h,"&");
				strcat(h,lin);
				char *p=h;
				while (*p && *p!=' ') p++;
				*p=0;
				strcat(h," :");
				while (true)
				{	
					lin=newline(in);
					if (!lin) break;
					if (!(*lin==' ' || *lin==9 || *lin==0) || strncmp(lin," -- ",4)==0) break;
					if (strstr(lin,text))
					{
						if (!found) addoutxln("%s",h);
						while (*lin==' ') lin++;
						if (found<20) addoutxln("  %s",lin);
						found++;
					}
				}
			}
		}
		fclose(in);
	}
}

void findhelp (char *text, char *help)
{
	currentout=help;
	addoutxln("Searching for \"%s\"",text);
	addoutln();

	header *hd=(header *)ramstart;
	while (hd<(header *)udfend && hd->type==s_udf)
	{
		search_in_udf_help(hd,text);
		hd=nextof(hd);
	}

	search_in_help_txt(text);
	search_in_maxima_reference(text);
}
