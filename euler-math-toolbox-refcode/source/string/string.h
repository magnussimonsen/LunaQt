class String
{	
	char *P;
	int Size;
	public :
	int Error;
	static int defaultsize;
	String ();
	String (char *text);
	String (char *text, int size);
	String (String &s);
	~String ();
	char *text () { return P; }
	long size () { return Size; }
	long strippedsize ();
	void copy (char *text, int size);
	void copy (char *text);
	void cat (char *text); // concat with the text.
	void prepend (char *text); // prepend with this string
	void cat (char *text, int length); // concat with the text at most length characters
	void stripapostroph (); // remove outer "..."
	char *filename ();
	void stripfilename ();
	void appendseparator (); // Add \, if necessary
	char *extension (); // return file extension
	void extension (char *ext); // set file extension
	int testextension (char *ext); // test, if ext is extension
	operator char * () { return P; }
	int todouble (double &x);
	int tolong (long &n);
	int empty () { return *P==0; }
	void clear () { *P=0; }
	char * operator = (char *s) { copy(s); return s; }
    void getprinter ();
	int length () { return (int)strlen(P); }
	void insert (int pos, char c);
	void insert (int pos, char *c);
	void del (int pos, int length);
	int endsWith (char *s);
	int startsWith (char *s, int start=0);
	int find (char *s);
	int contains (char *s) { return find(s)>=0; }
	void replace (char *s1, char *s2);
	void trim ();
	void substring (int from, int length);
	void substring (int from);
	int equals (char *s) { return strcmp(P,s)==0; }
	int compare (char *s) { return strcmp(P,s); }
};