/*
Instance of an Euler graphics meta file. These meta files are used
to keep Euler content and to replay the kept content to screen. 
The WindowsMeta class does store the content AND present it
to the screen.
*/
class WindowsMeta : public Meta
{	public :
	int Draw;
	WindowsMeta (int size=32) : Meta(size),Draw(1) {}
 	WindowsMeta (Meta *meta) : Meta(meta),Draw(1) {}
	virtual int gclear (void *p);
	virtual int gclip (void *p, double c, double r, double c1, double r1);
	virtual int gline (void *p, double c, double r, double c1, double r1,
    	int color, int st, double width);
	virtual int gpath (void *p, double c[], int n, int color, int st, double width);
	virtual int gmarker (void *p, double c, double r, int color, int st, double size);
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
};

