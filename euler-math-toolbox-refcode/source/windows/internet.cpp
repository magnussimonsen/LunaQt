#include <Windows.h>
#include <WinInet.h>

#include "../sysdep.h"
#include "../header.h"

HINTERNET hinternet=0,hfile=0;

int startnet ()
{
	if (hinternet) return 1;
	hinternet=InternetOpen("EMT",INTERNET_OPEN_TYPE_DIRECT,0,0,0);
	return hinternet!=0;
}

void endnet ()
{
	if (hinternet) InternetCloseHandle(hinternet);
	hinternet=0;
}

void closenetfile ()
{
	if (hinternet && hfile) InternetCloseHandle(hfile);
	hfile=0;
}

char netline[MAXLINE];
char netbuffer[MAXLINE];
int netavailable;
int netcurrent;

int opennetfile (char *url)
{
	if (!hinternet) return 0;
	closenetfile();
	hfile=InternetOpenUrl(hinternet,url,0,0,0,0);
	netcurrent=0;
	netavailable=0;
	return hfile!=0;
}

int readnetfile (char *buffer, int size)
{
	if (!hinternet || !hfile) return -1;
	DWORD n;
	int res=(int)InternetReadFile(hfile,buffer,size,&n);
	if (!res) return -2;
	return (int)n;
}

char *netgetline ()
{
	if (!hinternet || !hfile)
	{
		netline[0]=0;
		return netline;
	}
	char *p=netline;
	while (true)
	{
		if (netcurrent>=netavailable)
		{
			int n=readnetfile(netbuffer,MAXLINE);
			if (n<=0)
			{
				closenetfile();
				*p=0;
				return netline;
			}
			netcurrent=0;
			netavailable=n;
		}
		if (netbuffer[netcurrent]==13) netcurrent++;
		if (netbuffer[netcurrent]==10)
		{
			netcurrent++;
			*p=0;
			return netline;
		}
		*p++=netbuffer[netcurrent++];
		if (p>netline+MAXLINE-2) break;
	}
	*p=0;
	return netbuffer;
}

int neteof ()
{
	return hfile==0;
}