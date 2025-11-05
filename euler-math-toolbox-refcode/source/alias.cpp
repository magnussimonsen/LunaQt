#include <unordered_map>
#include <string>

using namespace std;

// Mymap stored Euler values in GlobalVar elements by a key.
typedef std::unordered_map <string, string> Mymap;

Mymap *Aliases=new Mymap();

void addalias (char *s1, char *s2)
{
	Aliases->erase(s1);
	Aliases->emplace(s1,s2);
}

void clearaliases ()
{
	Aliases->clear();
}

char *getalias (char *s)
{
	Mymap::iterator it=Aliases->find(s);
	if (it!=Aliases->end()) // have that key
	{
		return (char *)(Aliases->at(s).c_str());
	}
	else return 0;
}

int getaliases (char *start, char **found, int n, int maxn)
{
	if (n>=maxn) return n;
	size_t ln=strlen(start);
	for (auto it = Aliases->begin(); it != Aliases->end(); ++it)
	{
		char *h=(char *)(it->first.c_str());
		if (strncmp(start,h,ln)==0)
		{
			found[n++]=h;
			if (n>=maxn) break;
		}
	}
	return n;
}

