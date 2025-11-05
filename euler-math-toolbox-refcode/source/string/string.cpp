#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>

#include "string.h"

int String::defaultsize=64;

String::String (char *text, int size)
{   Size=strlen(text);
	if (Size<size) Size=size;
	Error=0;
	P=new char[Size+1];
	if (!P) Error=1; 
	else strcpy(P,text);
}

String::String (char *text)
{	Size=strlen(text);
	Error=0;
	P=new char[Size+1];
	if (!P) Error=1;
	else strcpy(P,text);
}

String::String ()
{   Size=defaultsize;
	Error=0;
	P=new char[Size+1];
	if (!P) Error=1;
	else *P=0;
}

String::String (String &s)
{	Size=s.Size;
	Error=0;
	P=new char[Size+1];
	if (!P) Error=1;
	else strcpy(P,s.P);
}

String::~String ()
{	if (P) delete P;
}

long String::strippedsize ()
{
	char *p=text();
	while (*p && (*p==' ' || *p==9)) p++;
	char *q=p+strlen(p);
	while (q>p && (*q==' ' || *q==9)) q--;
	return q-p;
}

void String::copy (char *text, int size)
{	Size=strlen(text);
	if (Size<size) Size=size;
	char *p=new char[Size+1];
	if (!P) { Error=1; return; }
	strcpy(p,text);
	delete P;
	P=p;
}

void String::copy (char *text)
{   int size=strlen(text);
	if (Size<size)
	{	delete P;
		Size=size;
		P=new char[Size+1];
		if (!P) { Error=1; return; }
	}
	strcpy(P,text);
}

void String::cat (char *text)
{   long total;
	if ((total=strlen(P)+strlen(text))>Size)
	{   char *p=new char[total+1];
		if (!p) { Error=1; return; }
		Size=total;
		strcpy(p,P);
		delete P;
        P=p;
	}
	strcat(P,text);
}

void String::prepend (char *text)
{   long total;
	int n=strlen(text),m=strlen(P);
	if ((total=m+n)>Size)
	{   char *p=new char[total+1];
		if (!p) { Error=1; return; }
		Size=total;
		strcpy(p,P);
		delete P;
        P=p;
	}
	memcpy(P+n,P,m+1);
	memcpy(P,text,n);
}

void String::cat (char *text, int n)
{   long total,l=strlen(P);
	if ((total=l+n+1)>Size)
	{   char *p=new char[total+1];
		if (!p) { Error=1; return; }
		Size=total;
		strcpy(p,P);
		delete P;
        P=p;
	}
	strncpy(P+l,text,n);
	*(P+l+n)=0;
}

void String::stripapostroph ()
{	while (strlen(P)>1)
	{	if (P[0]!='"' || P[strlen(P)-1]!='"') break;
		for (int i=0; i<(int)strlen(P)-1; i++)
			P[i]=P[i+1];
		P[strlen(P)-2]=0;
	}
}

char *String::filename ()
{	char *p=P+strlen(P);
	while (p>P)
	{	if (*p=='\\' || *p==':') return p+1;
		p--;
	}
	return p;
}

void String::stripfilename ()
{	*filename()=0;
}

void String::appendseparator ()
{
	if (length()>0 && !endsWith("\\") && !endsWith("/")) cat("\\");
}

char *String::extension ()
{	char *p=P+strlen(P);
	while (p>P)
	{	if (*p=='\\' || *p=='/' || *p=='.') break;
		p--;
	}
	if (p>=P && *p=='.') return p;
	return P+strlen(P);
}

void String::extension (char *ext)
{   *extension()=0;
	cat(ext);
}

int String::testextension (char *ext)
{	char *p=extension();
	while (*p && *ext)
	{   if (::toupper(*p)!=::toupper(*ext)) return 0;
		p++; ext++;
	}
	if (!*p && !*ext) return 1;
	return 0;
}

int String::todouble (double &x)
{	int n;
	if (sscanf(P,"%lg%n",&x,&n)==0) return 0;
	char *p=P+n;
	if (*p) return 0;
	else return 1;
}

int String::tolong (long &x)
{	int n;
	if (sscanf(P,"%ld%n",&x,&n)==0) return 0;
	char *p=P+n;
	if (*p) return 0;
	else return 1;
}

// insert a character at position pos
void String::insert (int pos, char c)
{   
	int n=strlen(text());
	if (size()<n+1)
	{	copy(text(),n+1);
	}
	memmove(text()+pos+1,text()+pos,n-pos+1);
	text()[pos]=c;
}

// insert a string at position pos
void String::insert (int pos, char *c)
{   int n=strlen(text());
	if ((unsigned int)size()<n+strlen(c))
	{	copy(text(),n+strlen(c));
	}
	memmove(text()+pos+strlen(c),text()+pos,n-pos+1);
	memmove(text()+pos,c,strlen(c));
}

// delete length characters at pos
void String::del (int pos, int length)
{	char *s=text();
	int n=strlen(s);
	if (pos+length==n) *(s+pos)=0;
	else if (pos+length<n)
	{	strcpy(s+pos,s+pos+length);
	}
}

int String::endsWith (char *str)
{	char *s=text();
	int n=strlen(s),m=strlen(str);
	if (n<m) return 0;
	return (strcmp(s+(n-m),str)==0);
}

int String::startsWith (char *str, int start)
{	return (strncmp(text()+start,str,strlen(str))==0);
}

int String::find (char *s)
{	char *p=strstr(text(),s);
	if (p==0) return -1;
	else return (p-text());
}

void String::replace (char *s1, char *s2)
{
	if (!*s1) return;
	String h;
	char *p=text();
	int m=strlen(s2);
	while (*p)
	{
		char *pf=strstr(p,s1);
		int n=-1;
		if (pf) n=pf-p;
		if (n<0)  { h.cat(p); break; }
		*(p+n)=0;
		h.cat(p);
		h.cat(s2);
		p=p+n+m;
	}
	copy(h.text());
}

void String::substring (int pos)
{
	if (pos<=0) return;
	char *p=text();
	if ((unsigned int)pos>strlen(p)) return;
	copy(p+pos);
}

void String::substring (int from, int length)
{
	char *p=text();
	if (from<=0)
	{
		if ((unsigned int)length<strlen(text())) *(p+length)=0;
		return;
	}
	substring(from);
	substring(0,length);
}

void String::trim ()
{
	char *p=text();
	while (*p==' ') p++;
	if (p>text()) copy(p);
	p=text();
	p=p+strlen(p);
	while (p>text() && *(p-1)==' ') *(--p)=0;
}