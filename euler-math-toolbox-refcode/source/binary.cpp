#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>

#include "sysdep.h"
#include "header.h"
#include "stack.h"
#include "getvalue.h"
#include "mainloop.h"

// routines for binary input and output to files

#define no_file(function) { error=800; output1("No file open in function %s\n",function); return; }

FILE *fa=0,*faout;
#define MAXPATH 512
char filename[MAXPATH];

void mopen (header *hd)
// open a file with name and mode
{   header *st=hd,*hd1,*result;
	hd1=nextof(hd);
	hd=getvalue(hd); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	if (hd->type!=s_string || hd1->type!=s_string) need_arg_in("open","filename, mode (r,w,a)");
	if (strcmp(stringof(hd1),"r")==0 || strcmp(stringof(hd1),"rb")==0) 
	{
		if (fa) fclose(fa);
		fa=open_file_in_path(stringof(hd),stringof(hd1),1);
		if (!fa)
		{	error=1;
			output1("Could not open the file\n%s\nfor reading!\n",stringof(hd));
			return;
		}
		result=new_real((double)ferror(fa),""); if (error) return;
	}
	else
	{
		if (faout) fclose(faout);
		faout=fopen(stringof(hd),stringof(hd1));
		if (!faout)
		{	error=1;
			output1("Could not open the file\n%s\nfor writing!\n",stringof(hd));
			return;
		}
		result=new_real((double)ferror(faout),""); if (error) return;
	}
	moveresult(st,result);
}

void mopen1 (header *hd)
// open a file with name only
{   header *st=hd,*result;
	if (fa) fclose(fa);
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) need_arg_in("open","filename, [mode (r,w,a)]");
	fa=open_file_in_path(stringof(hd),"r",1);
	if (!fa)
	{	error=1;
		output1("Could not open the file %s!\n",stringof(hd));
		return;
	}
	result=new_real((double)ferror(fa),""); if (error) return;
	moveresult(st,result);
}

void mclose (header *hd)
// close a file
{
	int err=0;
	if (fa) err=ferror(fa);
	if (faout && !err) err=ferror(faout);
	if (!fa && !faout) err=-1;
	new_real(err,"");
	if (fa) fclose(fa); fa=0;
	if (faout) fclose(faout); faout=0;
}

void mwrite (header *hd)
// write a string to a file
{   
	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) need_arg_in("write","string");
	if (!faout) output1("%s",stringof(hd));
	else fprintf(faout,"%s",stringof(hd));
	if (faout && ferror(faout))
	{
		error=10; print("Error in writeln.\n");
		return;
	}
	result=new_string("\01",1,""); if (error) return;
	moveresult(st,result);
}

void mwriteln (header *hd)
// write a string to a file with a carriage return
{   
	mwrite(hd);
	if (!error)
	{
		if (!faout) output("\n");
		else fprintf(faout,"\n");
	}
}

void mwriteln0 (header *hd)
// write a string to a file with a carriage return
{   
	if (!faout) output("\n");
	else fprintf(faout,"\n");
	new_string("\01",1,"");
}

// put several types of data to a file

void mputuchar (header *hd)
{   header *st=hd,*result;
	int n,i;
	unsigned char *p,*start=(unsigned char *)newram;
	double *m;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real && hd->type!=s_matrix)
		need_arg_in("putchar","real or vector");
	getmatrix(hd,&i,&n,&m);
	if (i!=1 || n<1) need_arg_in("putuchar","real or vector");
	if (faout)
	{	if (!freeramfrom(start,n))
		{	output("Stack overflow in putuchar.\n");
			error=1; return;
		}
		for (p=start,i=0; i<n; i++) *p++=(unsigned char)*m++;
		fwrite(start,1,n,faout);
	}
	else no_file("putuword");
	result=new_real(ferror(faout),""); if (error) return;
	moveresult(st,result);
}

void mputuword (header *hd)
{	header *st=hd,*result;
	int n,i;
	unsigned short *p,*start=(unsigned short *)newram;
	double *m;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real && hd->type!=s_matrix)
		need_arg_in("putchar","integer or vector of integers");
	getmatrix(hd,&i,&n,&m);
	if (i!=1 || n<1) need_arg_in("putchar","integer or vector of integers");
	if (faout)
	{	if (!freeramfrom(start,n*sizeof(unsigned short)))
		{	output("Stack overflow in putuword.\n");
			error=1; return;
		}
		for (p=start,i=0; i<n; i++) *p++=(unsigned short)*m++;
		fwrite(start,sizeof(unsigned short),n,faout);
	}
	else no_file("putuword");
	result=new_real(ferror(faout),""); if (error) return;
	moveresult(st,result);
}

void mputulongword (header *hd)
{	header *st=hd,*result;
	int n,i;
	unsigned long *p,*start=(unsigned long *)newram;
	double *m;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real && hd->type!=s_matrix)
		need_arg_in("putchar","integer or vector of integers");
	getmatrix(hd,&i,&n,&m);
	if (i!=1 || n<1) need_arg_in("putchar","integer or vector of integers");
	if (faout)
	{	if (!freeramfrom(start,n*sizeof(unsigned long)))
		{	output("Stack overflow in putulongword.\n");
			error=1; return;
		}
		for (p=start,i=0; i<n; i++) *p++=(unsigned long)*m++;
		fwrite(start,sizeof(unsigned long),n,faout);
	}
	else no_file("putulongword");
	result=new_real(ferror(faout),""); if (error) return;
	moveresult(st,result);
}

void mputchar (header *hd)
{   header *st=hd,*result;
	int n,i;
	char *p,*start=(char *)newram;
	double *m;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real && hd->type!=s_matrix)
		need_arg_in("putchar","integer or vector of integers");
	getmatrix(hd,&i,&n,&m);
	if (i!=1 || n<1) need_arg_in("putchar","integer or vector of integers");
	if (!freeramfrom(start,n+1))
	{	output("Stack overflow in putchar.\n");
		error=1; return;
	}
	for (p=start,i=0; i<n; i++) *p++=(char)*m++;
	*p++=0;
	if (faout) fwrite(start,1,n,faout);
	else output(start);
	result=new_real(faout?ferror(faout):0,""); if (error) return;
	moveresult(st,result);
}

void mputword (header *hd)
{	header *st=hd,*result;
	int n,i;
	short *p,*start=(short *)newram;
	double *m;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real && hd->type!=s_matrix)
		need_arg_in("putword","integer or vector of integers");
	getmatrix(hd,&i,&n,&m);
	if (i!=1 || n<1) need_arg_in("putword","integer or vector of integers");
	if (!faout) no_file("putword");
	if (!freeramfrom(start,n*sizeof(short)))
	{	output("Stack overflow in putword.\n");
		error=1; return;
	}
	for (p=start,i=0; i<n; i++) *p++=(short)*m++;
	fwrite(start,sizeof(short),n,faout);
	result=new_real(ferror(faout),""); if (error) return;
	moveresult(st,result);
}

void mputlongword (header *hd)
{	header *st=hd,*result;
	int n,i;
	long *p,*start=(long *)newram;
	double *m;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real && hd->type!=s_matrix)
		need_arg_in("putlongword","integer or vector of integers");
	getmatrix(hd,&i,&n,&m);
	if (i!=1 || n<1) need_arg_in("putlongword","integer or vector of integers");
	if (!faout) no_file("putlongword");
	if (!freeramfrom(start,n*sizeof(long)))
	{	output("Stack overflow in putlongword.\n");
		error=1; return;
	}
	for (p=start,i=0; i<n; i++) *p++=(long)*m++;
	fwrite(start,sizeof(long),n,faout);
	result=new_real(ferror(faout),""); if (error) return;
	moveresult(st,result);
}

// read several types of data from a file

void mgetuchar (header *hd)
{	new_real(fa?getc(fa):-1,"");
}

void mgetuchar1 (header *hd)
{   header *st=hd,*result;
	long n,count=0;
	unsigned char *start=(unsigned char *)newram;
	double *m;
	hd=getvalue(hd); if (error) return;
	if (!hd->type==s_real) need_arg_in("getchar","integer");
	n=(long)*realof(hd);
	if (n<=0) need_arg_in("getchar","nonnegative integer");
	if (fa)
	{	if (!freeramfrom(start,n))
		{	output("Stack overflow in getuchar.\n");
			error=1; return;
		}
		newram+=n;
		count=(long)fread(start,1,n,fa);
	}
	else no_file("getuchar");
	result=new_matrix(1,count,""); if (error) return;
	m=matrixof(result);
	for (n=0; n<count; n++) *m++=*start++;
	moveresult(st,result);
}

void mgetuword (header *hd)
{	unsigned short n;
	double x=-1;
	if (fa)
	{	if (fread(&n,sizeof(unsigned short),1,fa)==1)
		{	x=n;
		}
	}
	else no_file("gutuword");
	new_real(x,"");
}

void mgetuword1 (header *hd)
{   header *st=hd,*result;
	long n,count=0;
	unsigned short *start=(unsigned short *)newram;
	double *m;
	hd=getvalue(hd); if (error) return;
	if (!hd->type==s_real) need_arg_in("getuword","integer");
	n=(long)*realof(hd);
	if (n<=0) need_arg_in("getuword","nonnegative integer");
	if (fa)
	{	if (!freeramfrom(start,n*sizeof(unsigned short)))
		{	output("Stack overflow in getuword.\n");
			error=1; return;
		}
		newram=(char *)(start+n);
		count=(long)fread(start,sizeof(unsigned short),n,fa);
	}
	else no_file("getuword");
	result=new_matrix(1,count,""); if (error) return;
	m=matrixof(result);
	for (n=0; n<count; n++) *m++=*start++;
	moveresult(st,result);
}

void mgetulongword (header *hd)
{	unsigned long n;
	double x=-1;
	if (fa)
	{	if (fread(&n,sizeof(unsigned long),1,fa)==1)
		{	x=n;
		}
	}
	else no_file("getulongword");
	new_real(x,"");
}

void mgetulongword1 (header *hd)
{   header *st=hd,*result;
	long n,count=0;
	unsigned long *start=(unsigned long *)newram;
	double *m;
	hd=getvalue(hd); if (error) return;
	if (!hd->type==s_real) need_arg_in("getulongword","integer");
	n=(long)*realof(hd);
	if (n<=0) need_arg_in("getulongword","nonnegative integer");
	if (fa)
	{	if (!freeramfrom(start,n*sizeof(unsigned long)))
		{	output("Stack overflow in getulongword.\n");
			error=1; return;
		}
		newram=(char *)(start+n);
		count=(long)fread(start,sizeof(unsigned long),n,fa);
	}
	else no_file("getulongword");
	result=new_matrix(1,count,""); if (error) return;
	m=matrixof(result);
	for (n=0; n<count; n++) *m++=*start++;
	moveresult(st,result);
}

void mgetstring (header *hd)
{   header *st=hd,*result;
	long n,count=0;
	unsigned char *start=(unsigned char *)newram;
	double *m;
	hd=getvalue(hd); if (error) return;
	if (!hd->type==s_real) need_arg_in("getstring","integer");
	n=(long)*realof(hd);
	if (n<=0) need_arg_in("gestring","nonnegative integer");
	if (fa)
	{	if (!freeramfrom(start,n+1))
		{	output("Stack overflow in getstring.\n");
			error=1; return;
		}
		newram+=n+1;
		count=(long)fread(start,1,n,fa);
		start[n]=0;
	}
	else no_file("getstring");
	result=new_string((char *)start,(ULONG)strlen((char *)start),""); if (error) return;
	m=matrixof(result);
	for (n=0; n<count; n++) *m++=*start++;
	moveresult(st,result);
}

void mgetchar (header *hd)
{	new_real(fa?getc(fa):-1,"");
}

void mgetchar1 (header *hd)
{   header *st=hd,*result;
	long n,count=0;
	char *start=(char *)newram;
	double *m;
	hd=getvalue(hd); if (error) return;
	if (!hd->type==s_real) need_arg_in("getchar","integer");
	n=(long)*realof(hd);
	if (n<=0) need_arg_in("getchar","integer");
	if (fa)
	{	if (!freeramfrom(start,n))
		{	output("Stack overflow in getchar.\n");
			error=1; return;
		}
		newram+=n;
		count=(long)fread(start,1,n,fa);
	}
	else no_file("getchar");
	result=new_matrix(1,count,""); if (error) return;
	m=matrixof(result);
	for (n=0; n<count; n++) *m++=*start++;
	moveresult(st,result);
}

void mgetword1 (header *hd)
{   header *st=hd,*result;
	long n,count=0;
	short *start=(short *)newram;
	double *m;
	hd=getvalue(hd); if (error) return;
	if (!hd->type==s_real) need_arg_in("getword","integer");
	n=(long)*realof(hd);
	if (n<=0) need_arg_in("getword","nonnegative integer");
	if (fa)
	{	if (!freeramfrom(start,n*sizeof(short)))
		{	output("Stack overflow in getword.\n");
			error=1; return;
		}
		newram=(char *)(start+n);
		count=(long)fread(start,sizeof(short),n,fa);
	}
	else no_file("getword");
	result=new_matrix(1,count,""); if (error) return;
	m=matrixof(result);
	for (n=0; n<count; n++) *m++=*start++;
	moveresult(st,result);
}

void mgetlongword1 (header *hd)
{   header *st=hd,*result;
	long n,count=0;
	long *start=(long *)newram;
	double *m;
	hd=getvalue(hd); if (error) return;
	if (!hd->type==s_real) need_arg_in("getlongword","integer");
	n=(long)*realof(hd);
	if (n<=0) need_arg_in("getlongword","nonnegative integer");
	if (fa)
	{	if (!freeramfrom(start,n*sizeof(long)))
		{	output("Stack overflow in getlongword.\n");
			error=1; return;
		}
		newram=(char *)(start+n);
		count=(long)fread(start,sizeof(long),n,fa);
	}
	else no_file("getlongword");
	result=new_matrix(1,count,""); if (error) return;
	m=matrixof(result);
	for (n=0; n<count; n++) *m++=*start++;
	moveresult(st,result);
}

void mgetword (header *hd)
{	short n;
	double x=-1;
	if (fa)
	{	if (fread(&n,sizeof(short),1,fa)==1)
		{	x=n;
		}
	}
	else no_file("getword");
	new_real(x,"");
}

void mgetlongword (header *hd)
{	long n;
	double x=-1;
	if (fa)
	{	if (fread(&n,sizeof(long),1,fa)==1)
		{	x=n;
		}
	}
	else no_file("getlongword");
	new_real(x,"");
}

// functions to read data from a file

char decimaldot='.';

void msetdecimaldot (header *hd)
{
	header *st=hd;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string || strlen(stringof(hd))!=1)
		need_arg_in("setdecimaldot","string of length 1");
	char c=*stringof(hd);
	header *result=new_string(".","");
	*stringof(result)=decimaldot;
	decimaldot=c;
	moveresult(st,result);
}

void mgetvector (header *hd)
// get a vector from a file with a maximal number of elements
{   
	header *st=hd,*result;
	int i,c,n,negative;
	double *m;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real) need_arg_in("getvector","integer");
	if (!fa) no_file("getvector");
	n=(unsigned int) *realof(hd);
	result=new_matrix(1,n,""); if (error) return;
	m=matrixof(result);
	for (i=0; i<n; i++)
	{	
		another:
		c=getc(fa);
		if (c==EOF || feof(fa)) break;
		if (c==decimaldot) c='.';
		if (c=='-')
		{	
			negative=1;
			c=getc(fa);
			if (c==EOF) break;
		}
		else negative=0;
		if (!xisdigit(c) && c!='.') goto another;
		ungetc(c,fa);
		fscanf(fa,"%lg",m);
		if (negative) *m=-*m;
		m++;
	}
	c=i;
	dims *d=(dimsof(result));
	d->c=c; d->r=1;
	result->size=matrixsize(1,c);
	moveresult(st,result);
	new_real(c,"");
}

void mgetline (header *hd)
// read a line as a string from the file
{	
	if (!fa) no_file("getline");
	char line[MAXLINE];
	char *p=line;
	while (1)
	{	int c=getc(fa);
		if (c==EOF || feof(fa) || c=='\n') break;
		if (p-line>MAXLINE-1)
		{	output1("Line too long in getline, %d characters!\n",p-line);
			error=1; return;
		}
		*p++=(char)c;
	}
	*p++=0;
	new_string(line,(ULONG)strlen(line)+1,"");
}

void mgetvectorline (header *hd)
// read a line of numbers from a file skipping non numeric characters
// given are a the maximal number of data to be read
// maximal line length is 4096
// returns the vector and the line as a string
{   
	header *st=hd,*result;
	int i,c,n,negative;
	double *m;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real) need_arg_in("getvectorline","nonnegative integer");
	if (!fa) no_file("getvectorline");
	n=(unsigned int) *realof(hd);
	result=new_matrix(1,n,""); if (error) return;
	m=matrixof(result);
	char line[MAXLINE];
	char *p=line;
	while (1)
	{	
		int c=getc(fa);
		if (c==EOF || feof(fa) || c=='\n') break;
		if (c==decimaldot) c='.';
		if (p-line>MAXLINE-1)
		{	output1("Line too long in getvectorline, %d characters!\n",p-line);
			error=1; return;
		}
		*p++=(char)c;
	}
	*p++=0;
	p=line;
	i=0;
	while (1)
	{	
		c=*p;
		if (c==0) break;
		if (c=='-')
		{	negative=1;
			p++;
			c=*p;
			if (c==0) break;
		}
		else negative=0;
		if (xisdigit(c) || c=='.')
		{	int k;
			sscanf(p,"%lg%n",m,&k);
			if (negative) *m=-*m;
			m++; i++;
			p+=k;
		}
		else p++;
	}
	c=i;
	dims *d=(dimsof(result));
	d->c=c; d->r=1;
	result->size=matrixsize(1,c);
	moveresult(st,result);
	new_real(c,"");
	new_string(line,(ULONG)strlen(line)+1,"");
}

void meof (header *hd)
// test for end of file
{	
	new_real(fa?feof(fa):1,""); if (error) return;
}

void murlopen (header *hd)
{
	header *st=hd,*res;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) need_arg_in("urlopen","string");
	int flag=startnet();
	if (flag)
	{
		flag=opennetfile(stringof(hd));
	}
	res=new_real(flag,"");
	moveresult(st,res);

}

void murlgetline (header *hd)
{
	new_string(netgetline(),"");
}

void murlclose (header *hd)
{
	closenetfile();
	new_none();
}

void murleof (header *hd)
{
	new_real(neteof(),"");
}
