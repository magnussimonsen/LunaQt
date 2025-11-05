#ifndef GRAPHICSH
#define GRAPHICSH

/* graphics.c */

void mplot (header *hd);
void mplot1 (header *hd);
void mmark (header *hd);
void mmarkcolor (header *hd);
void mcontour (header *hd);
void mwire (header *hd);
void mwirerows (header *hd);
void msolid (header *hd);
void msolid1 (header *hd);
void mview (header *hd);
void mmesh (header *);
void ghold (void);
void show_graphics (void);
void mctext (header *hd);
void mtext (header *hd);
void mtextsize (header *hd);
void mstyle (header *hd);
void mcolor (header *hd);
void mfcolor (header *hd);
void mf3dcolor (header *hd);
void mwcolor (header *hd);
void mtcolor (header *hd);
void mtcolor0 (header *hd);
void mwindow (header *hd);
void mwindow0 (header *hd);
void mmouse (header *hd);
void mmousedrag (header *hd);
void mmousestate (header *hd);
void mmousepos (header *hd);
void mproject (header *hd);
void mview0 (header *hd);
void mholding (header *hd);
void mholding0 (header *hd);
void msetplot (header *hd);
void mscaling (header *hd);
void mclip (header *hd);
void mclip0 (header *hd);
void mantialiasing (header *hd);
void mantialiasing0 (header *hd);
void mbarRGB (header *hd);
void mcontourcolor (header *hd);
void mcontourcolor0 (header *hd);
void msetanaglyph (header *hd);
void msavepng (header *hd);
void msavesvg (header *hd);
void msaveps (header *hd);
void mcenter (header *hd);
void mcenter0 (header *hd);
void resetall (void);
void resetgraphics (void);
void mtoscreen (header *hd);
void mfromscreen (header *hd);
void mmargin (header *hd);
void mplotcubes (header *hd);
void msubgrid (header *hd);
void msetfont (header *hd);
void msetfont0 (header *hd);
void mtriangles (header *hd);
void maspect0 (header *hd);
void maspect (header *hd);
void mhatchgridsize (header *hd);


#endif