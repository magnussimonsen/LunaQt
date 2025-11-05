#ifndef METAH
#define METAH

//********** Internal meta file *************************

extern int graphicssize;

/*
Meta files are used to keep the Euler graphics in an internal representation.
They can replay the graphics content to a presentation space, scaling to
the correct size.
*/
class Meta
{   public :
	char *Start,*End;
	size_t Size;
	int Active,Full;
	Meta (int size) : Active(1),Full(0)
	{   
		Size=size*1024l*1024l;
		End=Start=(char *)malloc(Size);
		while (!End)
		{
			graphicssize=size=size/2;
			Size=size*1024l*1024l;
			End=Start=(char *)malloc(Size);
		}
	}
   Meta (Meta *meta) : Active(1),Full(1)
   {	
		Size=meta->End-meta->Start;
		Start=new char[Size];
		memcpy(Start,meta->Start,Size);
  		End=Start+Size;
   }
	void clear () { End=Start; Full=0; }
	void write (void *l, int n);
	void commandwrite (int n);
    void longwrite (double s);
    void shortwrite (int n);
    void stringwrite (char *s);
	int nextcommand (char * &p);
	double nextlong (char * &p);
	int nextint (char * &p);
	virtual int gclear (void *p);
	virtual int gclip (void *p, double c, double r, double c1, double r1);
 	virtual int gline (void *p, double c, double r, double c1, double r1,
    	int color, int st, double width);
 	virtual int gpath (void *p, double c[], int n, int color, int st, double width);
	virtual int gmarker (void *p, double c, double r, int color, int st, double markerfactor);
	virtual int gfill (void *p, double c[], int fillcolor, int wirecolor, int n, int connect[]);
	virtual int gpolygon (void *p, double c[], int n, int st, int color, int connect);
	virtual int gfillh (void *p, double c[], int n, double hue,
    	int color, int connect);
	virtual int gbar (void *p, double c, double r, double c1, double r1,
    	double hue,	int color, int connect);
	virtual int gbar1 (void *p, double c, double r, double c1, double r1,
		int color, int connect);
	virtual int gbarRGB (void *p, double c1, double r1, double c2, double r2, int c, int r,
		int tred, int tgreen, int tblue, unsigned char *colors);
	virtual int gtext (void *p, double c, double r, char *text,
    	int color, int centered, double size);
	virtual int gvtext (void *p, double c, double r, char *text,
    	int color, int centered, double size);
	virtual int gvutext (void *p, double c, double r, char *text,
    	int color, int centered, double size);
    virtual void replay (void *p, int clip=1);
    virtual void dump (FILE *out);
    virtual void postscript (FILE *out);
    virtual void svg (FILE *out, int w, int h);
};

extern Meta *meta;

#endif