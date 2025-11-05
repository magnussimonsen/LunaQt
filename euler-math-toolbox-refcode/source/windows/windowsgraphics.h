void gclear (PS &ops);
void gline (PS &ops, double c, double r, double c1, double r1, int color,
	int st, double width);
void gpath (PS &ops, double c[], int n, int color, int st, double width);
void gmarker (PS &ops, double c, double r, int color, int type, double markerfactor);
void gclip (PS &ops, double c, double r, double c1, double r1);
void gfill (PS &ops, double c[], int fillcolor, int wirecolor, int n, int connect[]);
void gpolygon (PS &ops, double c[], int n, int st, int color, int connect);
void gfillh (PS &ops, double c[], int n, double hue, int color, int connect);
void gbar (PS &ops, double c, double r, double c1, double r1, double hue,
	int color, int style);
void gbar1 (PS &ops, double c, double r, double c1, double r1,
	int color, int style);
void gbarRGB (PS &ops, double c1, double r1, double c2, double r2, int cn, int rn,
	int tred, int tblue, int tgreen, unsigned char *colors);
void gtext (PS &ops, double c, double r, char *text, int color, int alignment, double size);
void gvtext (PS &ops, double c, double r, char *text, int color, int alignment, double size);
void gvutext (PS &ops, double c, double r, char *text, int color, int alignment, double size);

extern double cropleft,cropright,croptop,cropbottom;
