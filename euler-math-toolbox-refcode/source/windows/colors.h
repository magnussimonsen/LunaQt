// The 16 Euler colors
extern int red[16];
extern int green[16];
extern int blue[16];

// class to hold an array for the Euler colors.
class MyColors
{	Color *C[16];
	Color act;
	public :
	MyColors ();
	~MyColors ();
	Color color (int i);
	Color fillcolor (int i);
	Color *c (int i);
	void set (int i, Color c) { *C[i]=c; }
};

extern MyColors mycolors;

// text colorsColor *textblack;
extern Color *textblack;
extern Color *textred;
extern Color *textgreen;
extern Color *textblue;
extern Color *textwhite;
extern Color *textmarked;
extern Color *textbracketmarked;
extern Color *textback;

extern Color deftextblack;
extern Color deftextred;
extern Color deftextgreen;
extern Color deftextblue;
extern Color deftextwhite;
extern Color deftextmarked;

// Class to hold compute a color with a specified hue of an Euler color.
// Used in the 3D hue plots.
class MyColor : public Color
{	public :
	MyColor (int c) : Color(mycolors.fillcolor(c)) {}
	MyColor (int c, int fill) 
		: Color(fill?mycolors.fillcolor(c):mycolors.color(c)) {}
	MyColor (int c, double hue);
};

extern MyColors savecolors;
void resetcolors();

void getcolor (int i, int *red, int *green, int *blue);
int getcolor (int red, int green, int blue);
void getfillcolor (int i, int *red, int *green, int *blue);

extern Color lightgray;
