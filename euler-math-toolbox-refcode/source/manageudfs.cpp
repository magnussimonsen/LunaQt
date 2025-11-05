#include <unordered_map>
#include <string>

#include "header.h"
#include "stack.h"
#include "udf.h"
#include "sysdep.h"
#include "aliases.h"

using namespace std;

typedef std::unordered_map <string, header *> Mymap;

/* 
Global hash table containg all names of functions.
Note that some names of functions are in the builtin list more than
once with different counts of arguments.
*/
Mymap *UDFunctions=new Mymap();
int udfneedsupdate=1;

/**
Store the index of the name in the builtin list.
*/
void globalputudf (char *name, header *hd)
{
	UDFunctions->erase(name);
	UDFunctions->emplace(name,hd);
}

void invalidate_udflist ()
{	
	udfneedsupdate=1;
}

void make_udflist ()
{	
	UDFunctions->clear();

	header *hd=(header *)ramstart;
	while ((char *)hd<udfend && hd->type==s_udf)
	{	
		globalputudf(hd->name,hd);
		hd=nextof(hd);
	}

	udfneedsupdate=0;
}

header *searchudf (char *name)
/***** searchudf
	search a udf, named "name".
	return 0, if not found.
*****/
{	
	int retry=0;
startover:
	if (udfneedsupdate) make_udflist();
	Mymap::iterator it=UDFunctions->find(name);
	if (it!=UDFunctions->end()) // have that key
	{
		header *h=UDFunctions->at(name);
		// print("searching for %s found %s\n",name,h->name);
		if (h->flags&COMMENT) return 0;
		else return h;
	}
	if (!retry)
	{
		retry=1;
		name=getalias(name);
		if (name) goto startover;
	}
	return 0;
}

header *searchalludf (char *name)
/***** searchudf
	search a udf, named "name".
	return 0, if not found.
*****/
{	
	int retry=0;
startover:
	if (udfneedsupdate) make_udflist();
	Mymap::iterator it=UDFunctions->find(name);
	if (it!=UDFunctions->end()) // have that key
	{
		return UDFunctions->at(name);
	}
	if (!retry)
	{
		retry=1;
		name=getalias(name);
		if (name) goto startover;
	}
	return 0;
}

header *searchudfmap (char *s)
/***** searchudf
	search a udf, named "name".
	return 0, if not found.
*****/
{	
	int n=strlen(s);
	if (n>maxname) return 0;
	char name[maxname+2];
	strcpy(name,s);
	if (n>3 && strncmp(name+n-3,"map",3)==0)
	{	*(name+n-3)=0;
	}
	else return 0;
	return searchudf(name);
}
