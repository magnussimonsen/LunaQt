/********************************
Routines for global variables in Euler.
********************************/

#include <unordered_map>
#include <string>

#include "header.h"
#include "stack.h"
#include "getvalue.h"
#include "globalvars.h"
#include "sysdep.h"

using namespace std;

/**
Element to store in the hash table.
Allocates space for an Euler value on the heap.
*/
class GlobalVar
{
public:
	header *h; // the value
	int size;
	/**
	Allocate space and copy the value of hd to it.
	This constructor is called by unorderd_map.
	*/
	GlobalVar (header *hd)
	{
		size=hd->size*5/4+2*sizeof(double);
		h=(header *)malloc(size);
		if (!h) 
		{
			print("Out of RAM for global variable.\n");
			error=1; return;
		}
		memcpy(h,hd,hd->size);
	}
	/**
	This destructor is called by unordered_map.
	*/
	~GlobalVar ()
	{
		free(h);
	}
};

// Mymap stored Euler values in GlobalVar elements by a key.
typedef std::unordered_map <string, GlobalVar> Mymap;

Mymap *GV=new Mymap();

/**
Put the value to global variable
*/
header *globalput (char *name, header *hd, int flags)
{
	Mymap::iterator it=GV->find(name);
	if (it!=GV->end()) // have that key
	{
		GlobalVar *var=&(GV->at(name));
		header *h=var->h;
		if (hd->size<var->size)
		{
			memmove((char *)h,(char *)hd,hd->size);
			h->flags|=flags;
			setname(h,name);
			// print("reused %s for %d %d %d size %d\n",name,hd->type,hd->size,global,var->size);
			return h;
		}
	}
	GV->erase(name);
	std::pair<Mymap::iterator,bool> res=GV->emplace(name,hd);
	// print("new %s for %d %d %d\n",name,hd->type,hd->size,global);
	if (!res.second)
	{
		error=1; print("Could not generate global variable %s!",name);
	}
	it=res.first;
	if (it!=GV->end()) // have that key
	{
		header *h=it->second.h;
		h->flags|=flags;
		setname(h,name);
		return h;
	}
	else
	{
		error=1; print("Could not generate global variable %s!",name);
	}
	return 0;
}

/**
Define a global variable with name and value.
>gset("a",4);
*/
void mglobalput (header *hd)
{
	header *st=hd,*hd1,*res;
	hd1=nextof(hd);
	hd=getvalue(hd);
	hd1=getvalue(hd1);
	if (error) return;
	if (hd->type!=s_string)
		need_arg_in("gvar","variable name and value");
	globalput(stringof(hd),hd1);
	res=new_none();
	moveresult(st,res);
}

/**
Copy a global variable to the stack.
*/
header *globalget (char *name)
{
	header *res;
	if (!GV) return 0;
	Mymap::iterator it=GV->find(name);
	if (it!=GV->end()) // have that key
	{
		// copy variable from the hash element to the Euler stack
		GlobalVar *var=&(GV->at(name));
		header *h=var->h;
		if (newram+h->size>=ramend)
		{
			print("Out of stack space for global variable %s\n",name);
			error=1; return 0;
		}
		res=(header *)newram;
		memcpy(newram,h,h->size);
		res->flags&=(~GLOBAL);
		newram+=h->size;
		return res;
	}
	return 0;
}

/**
Copy a global variable to the stack.
*/
header *globalfind (char *name)
{
	if (!GV) return 0;
	Mymap::iterator it=GV->find(name);
	if (it!=GV->end()) // have that key
	{
		// copy variable from the hash element to the Euler stack
		GlobalVar *var=&(GV->at(name));
		return var->h;
	}
	return 0;
}

/**
Retrieve a global variable by name
>gvar("a")
*/
void mglobalget (header *hd)
{
	header *st=hd,*res;
	hd=getvalue(hd);
	if (error) return;
	if (hd->type!=s_string)
		need_arg_in("gvar","variable name");
	Mymap::iterator it=GV->find(stringof(hd));
	res=globalget(stringof(hd));
	if (res==0)
	{
		print("Global variable %s not found\n",stringof(hd));
		error=1; return;
	}
	if (res) moveresult(st,res);
}

void globalremove (char *name)
{
	GV->erase(name);
}

/**
Remove a global variable by name.
>gremove("a")
*/
void mglobalrem (header *hd)
{
	header *st=hd,*res;
	hd=getvalue(hd);
	if (error) return;
	if (hd->type!=s_string)
		need_arg_in("gvar","variable name");
	GV->erase(stringof(hd));
	res=new_none();
	moveresult(st,res);
}

extern void listvar_output (header *hd);
extern int contains (const char *s, const char *sub);

int listglobal (char *name)
{
	int count=0;
	for (auto it = GV->begin(); it != GV->end(); ++it)
	{
		header *hd=it->second.h;
		string sname=it->first;
		const char *hname=sname.c_str();
		if (
			(*name && contains(hname,name)) || 
			(!*name &&
			!(hd->flags&PROTECT) && strncmp(hname,"default",7) 
			&& strncmp(hname,"%",1)))
		{
			// print("%4x ",hd->flags);
			listvar_output(hd);
			count++;
		}
		if (test_key()==escape) break;
	}
	return count;
}

void clear_global ()
{
	again :
	for (auto it = GV->begin(); it != GV->end(); ++it)
	{
		header *hd=it->second.h;
		string sname=it->first;
		const char *hname=sname.c_str();
		if (
			(!(hd->flags&PROTECT) && strncmp(hname,"default",7) 
			&& strncmp(hname,"%",1)))
		{
			// print("%4x ",hd->flags);
			GV->erase(it);
			goto again;
		}
	}
}

void protectglobal ()
{
	for (auto it = GV->begin(); it != GV->end(); ++it)
	{
		header *hd=it->second.h;
		hd->flags|=PROTECT;
	}
}

/**
Element to store in the hash table.
It contains an array of pointers to Euler values.
The array can grow whenever needed.
*/
class GlobalList
{
public :

	header **h; // array of Euler elements on the heap
	int n; // current content
	int ncap; // maximal content

	GlobalList (int cap)
	{
		ncap=cap;
		h=(header **)malloc(ncap*sizeof(header *));
		n=0;
	}
	~GlobalList ()
	{
		removeAll();
		free(h);
	}
	int add (header *p); // append a new list element
	int insert (header *p, int pos); // insert at position pos
	int put (header *p, int pos); // insert at position pos
	int remove (int i); // remove element at position i
	int removeAll (); // remove all elements
	int checkAdd (); // helper functions to make sure one element can be added
};

int GlobalList::add (header *p)
{
	if (checkAdd()) return 1;
	header *hnew=(header *)malloc(p->size);
	if (!hnew) 
	{
		print("Out of RAM in globalput.\n");
		error=1; return 1;
	}
	memcpy(hnew,p,p->size);

	h[n++]=hnew;
	return 0;
}

int GlobalList::insert (header *p, int pos)
{
	if (checkAdd()) return 1;
	header *hnew=(header *)malloc(p->size);
	if (!hnew) 
	{
		print("Out of RAM in globalput.\n");
		error=1; return 1;
	}
	memcpy(hnew,p,p->size);

	if (pos>=n) h[n++]=hnew;
	else
	{
		if (pos<0) pos=0;
		for (int i=n; i>pos; i--)
		{
			h[i]=h[i-1];
		}
		n++;
		h[pos]=hnew;
	}
	return 0;
}

int GlobalList::put (header *p, int pos)
{
	if (pos>=n && checkAdd()) return 1;
	header *hnew=(header *)malloc(p->size);
	if (!hnew) 
	{
		print("Out of RAM in globalput.\n");
		error=1; return 1;
	}
	memcpy(hnew,p,p->size);

	if (pos>=n) h[n++]=hnew;
	else
	{
		if (pos<0) pos=0;
		delete h[pos];
		h[pos]=hnew;
	}
	return 0;
}

int GlobalList::remove (int i)
{
	if (i<0 || i>=n) return 1;
	free(h[i]);
	for (int j=i; j<n-1; j++)
	{
		h[j]=h[j+1];
	}
	n--;
	return 0;
}

int GlobalList::removeAll ()
{
	for (int i=0; i<n; i++) free(h[i]);
	n=0;
	return 0;
}


/**
Make sure, there is room for one more element.
If not, grow array by double its size and copy entries.
*/
int GlobalList::checkAdd ()
{
	if (n==ncap-1)
	{
		header **hnew=(header **)malloc(2*ncap*sizeof(header *));
		if (!hnew) 
		{
			print("Out of RAM in globalput.\n");
			error=1; return 1;
		}
		for (int i=0; i<n; i++) hnew[i]=h[i];
		free(h);
		h=hnew;
		ncap=ncap*2;
	}
	return 0;
}


// hashtable type to store global lists
typedef unordered_map <string, GlobalList> Mymaplist;

Mymaplist *GL=new Mymaplist;

/**
Create a new list of default size 16.
>glist("a")
*/
void mglist (header *hd)
{
	header *st=hd,*res;
	hd=getvalue(hd);
	if (error) return;
	if (hd->type!=s_string)
		need_arg_in("glist","list name");
	GL->erase(stringof(hd));
	GL->emplace(stringof(hd),16);
	res=new_none();
	moveresult(st,res);
}

/**
Find a list by name.
*/
GlobalList *findlist (char *name)
{
	Mymaplist::iterator it=GL->find(name);
	if (it!=GL->end())
	{
		return &(GL->at(name));
	}
	else
	{
		print("Global list %s not found\n",name);
		error=1; return 0;
	}
}

/**
Append an element to a list.
>glistadd("a",1:10)
*/
void mglistadd (header *hd)
{
	header *st=hd,*hd1,*res;
	hd1=nextof(hd);
	hd=getvalue(hd);
	hd1=getvalue(hd1);
	if (error) return;
	if (hd->type!=s_string)
		need_arg_in("glistadd","list name and value");

	GlobalList *L=findlist(stringof(hd));
	if (error) return;

	L->add(hd1);

	res=new_real(L->n,"");
	moveresult(st,res);
}

/**
Get the length of a list
>glistlength("a")
*/
void mglistlength (header *hd)
{
	header *st=hd,*res;
	hd=getvalue(hd);
	if (error) return;
	if (hd->type!=s_string)
		need_arg_in("glistlength","list name");

	GlobalList *L=findlist(stringof(hd));
	if (error) return;

	res=new_real(L->n,"");
	moveresult(st,res);
}

/**
Get an element of a list by position.
>glistvar("a",3)
*/
void mglistget (header *hd)
{
	header *st=hd,*hd1,*res;
	hd1=nextof(hd);
	hd=getvalue(hd);
	hd1=getvalue(hd1);
	if (error) return;
	if (hd->type!=s_string || hd1->type!=s_real)
		need_arg_in("glistvar","list name and index");
	int n=(int)*realof(hd1)-1;
	GlobalList *L=findlist(stringof(hd));
	if (error) return;
	if (n<0 || n>=L->n)
	{
		print("List index %d out of range %d in list %s.\n",
			n+1,L->n,stringof(hd));
		error=1; return;
	}
	checkram(newram+L->h[n]->size);
	header **h=L->h;
	res=(header *)newram;
	memcpy(newram,h[n],h[n]->size);
	newram+=h[n]->size;
	moveresult(st,res);
}

/**
Insert an element into a list at a position.
>glistinsert("a",3,1:10)
*/
void mglistinsert (header *hd)
{
	header *st=hd,*hd1,*hd2,*res;
	hd1=nextof(hd);
	hd2=nextof(hd1);
	hd=getvalue(hd);
	hd1=getvalue(hd1);
	hd2=getvalue(hd2);
	if (error) return;
	if (hd->type!=s_string || hd1->type!=s_real)
		need_arg_in("glistinsert","list name, index, value");
	int n=(int)*realof(hd1)-1;
	GlobalList *L=findlist(stringof(hd));
	if (error) return;
	L->insert(hd2,n);

	res=new_real(L->n,"");
	moveresult(st,res);
}

/**
Replace an element in a list at a position.
>glistput("a",3,1:10)
*/
void mglistput (header *hd)
{
	header *st=hd,*hd1,*hd2,*res;
	hd1=nextof(hd);
	hd2=nextof(hd1);
	hd=getvalue(hd);
	hd1=getvalue(hd1);
	hd2=getvalue(hd2);
	if (error) return;
	if (hd->type!=s_string || hd1->type!=s_real)
		need_arg_in("glistput","list name, index, value");
	int n=(int)*realof(hd1)-1;
	GlobalList *L=findlist(stringof(hd));
	if (error) return;
	L->put(hd2,n);

	res=new_real(L->n,"");
	moveresult(st,res);
}

/**
Delete an element from a list.
>glistdelete("a",3)
*/
void mglistdelete (header *hd)
{
	header *st=hd,*hd1,*res;
	hd1=nextof(hd);
	hd=getvalue(hd);
	hd1=getvalue(hd1);
	if (error) return;
	if (hd->type!=s_string || hd1->type!=s_real)
		need_arg_in("glistvar","list name and index");
	int n=(int)*realof(hd1)-1;
	GlobalList *L=findlist(stringof(hd));
	if (error) return;
	if (n<0 || n>=L->n)
	{
		print("List index %d out of range %d in list %s.\n",
			n+1,L->n,stringof(hd));
		error=1; return;
	}
	L->remove(n);
	res=new_real(L->n,"");
	moveresult(st,res);
}

/**
Delete a list.
>glistdelete("a")
*/
void mglistrem (header *hd)
{
	header *st=hd,*res;
	hd=getvalue(hd);
	if (error) return;
	if (hd->type!=s_string)
		need_arg_in("gvar","variable name");
	GL->erase(stringof(hd));
	res=new_none();
	moveresult(st,res);
}

/**
Called from the main program to initialize the global stuff.
*/
void exit_globalvars ()
{
	GV->clear();
	GL->clear();
}

/**
Called from the main program to release the global stuff.
*/
void init_globalvars ()
{
	exit_globalvars();
}

/**
Remove all global variables.
*/
void mglobalclear (header *hd)
{
	GV->clear();
	GL->clear();
	new_none();
}

int list_lists (char *name)
{
	int count=0;
	for (auto it = GL->begin(); it != GL->end(); ++it)
	{
		string sname=it->first;
		const char *hname=sname.c_str();
		if (*name==0 || contains(hname,name))
		{
			print("%-20sglobal list\n",hname);
			count++;
		}
		if (test_key()==escape) break;
	}
	return count;
}
