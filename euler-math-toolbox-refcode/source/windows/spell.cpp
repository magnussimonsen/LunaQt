#include "windows.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <float.h>
#include <time.h>
#include <mmsystem.h>
#include <io.h>
#include <direct.h>
#include <Shlwapi.h>

#include "../header.h"
#include "../sysdep.h"
#include "euler.h"
#include "../help.h"
#include "meta.h"
#include "../yacas.h"
#include "../maxima.h"
#include "colors.h"
#include "text.h"
#include "windowsmeta.h"
#include "graphicswindow.h"
#include "main.h"
#include "edit.h"
#include "windowsframes.h"
#include "windowsgraphics.h"
#include "images.h"
#include "helpwindow.h"
#include "spell.h"

#define MAXB 1024

char *words=0;
char *wend=0;
int wlength=0;
char **pw=0;
int wcount=0;
int pwlength=0;

void addword (char *word)
{
	if (!words)
	{
		wlength=50000;
		words=(char *)malloc(wlength);
		if (!words) return;
		wend=words;
		pwlength=5000;
		pw=(char **)malloc(pwlength*sizeof(char *));
		if (!pw)
		{
			words=0; return;
		}
		wcount=0;
	}
	int l=strlen(word);
	if (wend+l+1>=words+wlength)
	{
		char *newwords=(char *)malloc(2*wlength);
		if (!newwords) return;
		int wl=wend-words;
		memmove(newwords,words,wl);
		wlength=2*wlength;
		wend=newwords+wl;
		for (int i=0; i<wcount; i++)
		{
			pw[i]=newwords+(pw[i]-words);
		}
		delete words;
		words=newwords;
	}
	if (wcount>=pwlength)
	{
		char **newpw=(char **)malloc(2*pwlength*sizeof(char *));
		memmove(newpw,pw,wcount*sizeof(char *));
		pwlength=2*pwlength;
		delete pw;
		pw=newpw;
	}
	pw[wcount++]=wend;
	strcpy(wend,word);
	wend+=l+1;
}

int string_compare (const char **s, const char **t)
{
	return strcmp(*s,*t);
}

void sortwords ()
{
	qsort(pw,wcount,sizeof(char *),
		(int (*) (const void *, const void *))string_compare);
}

extern int english,german;

void loadwords ()
{
	char line[MAXLINE];
	line[0]=0;
	
	if (english)
	{
		sprintf(line,"%senglish.words",getstartdir());
		FILE *in=fopen(line,"r");
		if (in)
		{
			while (1)
			{
				char *s=fgets(line,MAXB,in);
				if (!s) break;
				if (!*s) continue;
				int n=strlen(s);
				if (s[n-1]=='\n') { s[n-1]=0; n--; }
				if (!*s) continue;
				addword(s);
			}
		}
		fclose(in);
	}
	if (german)
	{
		sprintf(line,"%sgerman.words",getstartdir());
		FILE *in=fopen(line,"r");
		if (in)
		{
			while (1)
			{
				char *s=fgets(line,MAXB,in);
				if (!s) break;
				if (!*s) continue;
				int n=strlen(s);
				if (s[n-1]=='\n') { s[n-1]=0; n--; }
				if (!*s) continue;
				addword(s);
			}
		}
		fclose(in);
	}

	sprintf(line,"%suser.words",getusereulerdir());
	FILE *in=fopen(line,"r");
	if (in)
	{
		while (1)
		{
			char *s=fgets(line,MAXB,in);
			if (!s) break;
			if (!*s) continue;
			int n=strlen(s);
			if (s[n-1]=='\n') { s[n-1]=0; n--; }
			if (!*s) continue;
			addword(s);
		}
		fclose(in);
	}
	sortwords();
}

int findword (char *w)
{
	if (!pw) loadwords();
	if (!pw) return 1;
	char **s=(char **)bsearch(&w,pw,wcount,sizeof(char *),
		(int (*) (const void *, const void *))string_compare);
	if (!s)
	{
		*w=(char)tolower(*w);
		s=(char **)bsearch(&w,pw,wcount,sizeof(char *),
			(int (*) (const void *, const void *))string_compare);
	}
	return s!=0;
}

void resetspell ()
{
	if (pw) delete pw;
	if (words) delete words;
	pw=0;
	words=0;
}
