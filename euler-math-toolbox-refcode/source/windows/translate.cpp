#include <stdio.h>

#include "windows.h"
#include "translate.h"

char *translations=0;

char translate_code (char c)
{
	if (translations && c>=32 && c<128 && translations[c])
		return translations[c];
	return c;
}

void translations_init (char *filename)
{
	translations=new char[128];
	for (int i=0; i<128; i++) translations[i]=0;
	
	FILE *in=fopen(filename,"r");
	if (!in) return;
	while (true)
	{
		char s[16];
		if (fgets(s,15,in)==NULL) break;
		if (s[0]<32) break;
		if (s[1]>=32 && s[1]<256) translations[s[0]]=s[1];
	}
	fclose(in);
}

