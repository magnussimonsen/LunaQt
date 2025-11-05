extern WindowClass eulerg;

extern int antialiasgraphics,antialiasfactor,cropsquare;

extern double aspect;

class GraphicsWindow : public StandardWindow
{   
	BitmapPS *B;
	Timer *T,*Tsized;
	int DW,DH,DF; // width, height amd boundaries of the bitmap
	public :

	double X,Y; // to remember mouse coordinates
	int Type; // type of mouse action
	long Time; // time of mouse action
	int Dragging; // flag set by mouse down

	GraphicsWindow (char *title, int hidden);
	virtual void sized ();
	void timedsized (int forced=0);
	virtual void redraw (PS &ps);
	virtual void timer (int n);
	virtual void key (int f, int c, int s)
	{   textwindow.key(f,c,s);
	}
	virtual void clicked (int x, int y, int type);
	void mouse (double &x, double &y) { x=X; y=Y; }
	BitmapPS *bitmap () { return B; }
	virtual void statechanged (int s);
	int adjwidth();
	int adjheight();
	int windowwidth() { return Width; }
	int windowheight() { return Height; }
	int bitmapwidth() { return B->width(); }
	int bitmapheight() { return B->height(); }
	double aspect() { if (cropsquare) return ::aspect; else return (double)B->width()/B->height(); }
	int displaywidth() { return B->width()/(antialiasgraphics==1?antialiasfactor:1); }
	int displayheight() { return B->height()/(antialiasgraphics==1?antialiasfactor:1); }
	void getcr (int x, int y, double *c, double *r);
	void clear ();
};

extern GraphicsWindow *graphicswindow;

extern int cropmode;

double linewidth (int w, double width=1.0, int thinner=0);
double linewidth (PS &ops, double width=1.0, int thinner=0);
