/************* graphics.c ***************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "header.h"
#include "sysdep.h"
#include "funcs.h"
#include "graphics.h"
#include "stack.h"
#include "spread.h"
#include "getvalue.h"

double x_min=-1,x_max=1,y_min=-1,y_max=1,meshfactor=1;
double x_min_old=-1,x_max_old=1,y_min_old=-1,y_max_old=1;
int dgrid=0;
double margin=0.1;

static int upperclipc=0,upperclipr=0,lowerclipc=1023,lowerclipr=1023;
int upperc=10,upperr=30,lowerc=1010,lowerr=1010;
int connected[4]={1,1,1,1};
int tconnected[3]={1,0,0};
int keepsquare=0;
double xcenter=0,ycenter=0,zcenter=0;
int iskip=1,jskip=1;

int fillcolor1=11,fillcolor2=3,markerfactor=140,hchar=1,wchar=1,insimglines=35;

int linetype=line_solid,linecolor=1,contourcolor=8,lines=1,holding=0,
	framecolor=3,frame3dcolor=2,
	wirecolor=9,textcolor=2,markertype=marker_square_hollow,scaling=1,
	twosides=1,densitycolor=1,antialiasing=0,
	bartype=bar_framed,barcolor=3,hatchgridsize=16;

#define MAXLINECOLORS 256
int linecolors[MAXLINECOLORS];
int uselinecolors=0;

double linewidth=1;

double distance=5,tele=2.6,a_left=2,a_up=0.4;

#define max(c,r) (c>r?c:r)

extern int usecomplex;

void resetall (void)
{	
	x_min=-1; x_max=1; y_min=-1; y_max=1; 
	x_min_old=-1; x_max_old=1; y_min_old=-1; y_max_old=1;
	upperclipc=0; upperclipr=0; lowerclipc=1023; lowerclipr=1023;
	upperc=10; upperr=30; lowerc=1010; lowerr=1010;
	pnan=0; usecomplex=0;
	resetgraphics();
	trace=0; traceerrors=0; error=0;
}

void resetgraphics ()
{
	meshfactor=1;
	dgrid=0;
	keepsquare=0;
	linetype=line_solid; 
	linecolor=1; 
	contourcolor=8; 
	wirecolor=9; 
	densitycolor=1; 
	lines=1; 
	holding=0; 
	framecolor=52042266; 
	frame3dcolor=52042266;
	textcolor=52042266; 
	markertype=marker_square_hollow; 
	scaling=1;
	twosides=1; 
	linewidth=1; 
	antialiasing=0;
	fillcolor1=11; 
	fillcolor2=3;
	distance=5; 
	tele=2.6; 
	a_left=2; 
	a_up=0.4;
	bartype=bar_framed; 
	barcolor=3;
	xcenter=0; ycenter=0; zcenter=0;
	pnan=0; 
	underflows=0;
	iskip=1; 
	jskip=1;
	hatchgridsize=16;
	insimglines=35;
}

double scrcol (double x)
{	return (upperc+(x-x_min)/(x_max-x_min)*((double)lowerc- (double)upperc));
}

double scrrow (double y)
{	return (lowerr-(y-y_min)/(y_max-y_min)*((double)lowerr- (double)upperr));
}

double invscrcol (double col)
{	return ((lowerc-col)*x_min+(col-upperc)*x_max)/((double)lowerc- (double)upperc);
}

double invscrrow (double row)
{	return ((upperr-row)*y_min+(row-lowerr)*y_max)/((double)upperr- (double)lowerr);
}

void frame (void)
{	if (framecolor==0) return;
	gline(upperc,upperr,upperc,lowerr,framecolor,line_solid,1);
	gline(upperc,lowerr,lowerc,lowerr,framecolor,line_solid,1);
	gline(lowerc,lowerr,lowerc,upperr,framecolor,line_solid,1);
	gline(lowerc,upperr,upperc,upperr,framecolor,line_solid,1);
}

void plot_marks (double *x, double *y, int nx, int ny)
/***** plot_marks
	plots n pairs (x,y).
*****/
{	double c0,r0,c1,r1;
	int i;
	if (nx<=0 || ny<=0) return;
	int n=max(nx,ny);
	if (!(isnan(*x) || isnan(*y)))
	{	c0=scrcol(*x); r0=scrrow(*y);
		gmarker(c0,r0,linecolor,markertype,markerfactor);
	}
	for (i=1; i<n; i++)
	{	if (i<nx) x++; 
		if (i<ny) y++;
		if (!isnan(*x) && !isnan(*y))
		{	c1=scrcol(*x); r1=scrrow(*y);
			gmarker(c1,r1,linecolor,markertype,markerfactor);
		}
	}
}

void plot_marks_color (double *x, double *y, double *col, int nx, int ny, int ncol)
/***** plot_marks
	plots n pairs (x,y).
*****/
{	double c0,r0,c1,r1;
	int i;
	if (nx<=0 || ny<=0 || ncol<=0) return;
	int n=max(nx,ny);
	if (!(isnan(*x) || isnan(*y)))
	{	c0=scrcol(*x); r0=scrrow(*y);
		gmarker(c0,r0,(int)(*col),markertype,markerfactor);
	}
	for (i=1; i<n; i++)
	{	if (i<nx) x++; 
		if (i<ny) y++;
		if (i<ncol) col++;
		if (!isnan(*x) && !isnan(*y))
		{	c1=scrcol(*x); r1=scrrow(*y);
			gmarker(c1,r1,(int)(*col),markertype,markerfactor);
		}
	}
}

double *cc=0;
int ccsize=0;

void check_cc_size (int n)
{
	if (!cc || ccsize<=n)
	{
		if (cc) delete[] cc;
		int k=n;
		if (k<MAXPOLYGON) k=MAXPOLYGON;
		cc = new double[2*k];
		ccsize=k;
	}
}

void plot_vector (double *x, double *y, int nx, int ny)
/***** plot_vector
	plots n pairs (x,y).
*****/
{	
	int i;
	if (nx<=0) return;
	int ccn=0;
	if (lines)
	{	
		int n=max(nx,ny);
		check_cc_size(n);
		for (i=0; i<n; i++)
		{	
			double c0=scrcol(*x); 
			double c1=scrrow(*y);
			if (!isnan(*x) && !isnan(*y) &&
				c0>-1000 && c0<2000 && c1>-1000 && c1<2000
				&& ccn<MAXPOLYGON)
			{	
				cc[2*ccn]=c0; 
				cc[2*ccn+1]=c1;
				ccn++;
			}
			else if (ccn>0)
			{
				gpath(cc,ccn,linecolor,linetype,linewidth);
				ccn=0;
			}
			if (i<nx-1) x++; 
			if (i<ny-1) y++;
		}
		if (ccn>0) gpath(cc,ccn,linecolor,linetype,linewidth);
	}
}

int setrange (int expand)
{
	if (isnan(x_max) || isnan(y_max)) return 0;
	x_min_old=x_min; y_min_old=y_min;
	x_max_old=x_max; y_max_old=y_max;
	if (expand && margin!=0.0)
	{
		double dx=(x_max-x_min)*(1.0+margin)/2,mx=(x_max+x_min)/2;
		x_max=mx+dx; x_min=mx-dx;
		double dy=(y_max-y_min)*(1.0+margin)/2,my=(y_max+y_min)/2;
		y_max=my+dy; y_min=my-dy;
	}
	if (isnan(x_max) || isnan(y_max)) return 0;
	if (x_max>DBL_MAX) x_max_old=x_max=DBL_MAX;
	if (x_min<-DBL_MAX) x_min_old=x_min=-DBL_MAX;
	if (y_max>DBL_MAX) y_max_old=y_max=DBL_MAX;
	if (y_min<-DBL_MAX) y_min_old=y_min=-DBL_MAX;
	if (x_min>=x_max) x_max_old=x_max=x_min+1;
	if (y_min>=y_max) y_max_old=y_max=y_min+1;
	return 1;
}

void do_plot (header *hdx, header *hdy)
{	
	int cx,rx,cy,ry,i,ix,iy;
	double *x,*y,h;
	getmatrix(hdx,&rx,&cx,&x); getmatrix(hdy,&ry,&cy,&y);
	int r=max(rx,ry),c=max(cx,cy);
	if (r==0 || c==0) return;
	if ((cx>1 && cx!=c) || (rx>1 && rx!=r) || (cy>1 && cy!=c) || (ry>1 && ry!=r))
	{	
		error=22; 
		print("Matrices must fit for plot!\nGot %dx%d and %dx%d\n",rx,ry,cx,cy);
		return;
	}
	if (scaling)
	{	
		minmax(x,(LONG)cx*rx,&x_min,&x_max,&ix,&iy);
		minmax(y,(LONG)cy*ry,&y_min,&y_max,&ix,&iy);
		if (keepsquare)
		{	h=(y_max+y_min)/2;
			y_max=h+(x_max-x_min)/2;
			y_min=h-(x_max-x_min)/2;
		}
		if (!setrange(1)) return;
	}
	graphic_mode();
	// if (!holding) frame();
	int old=linecolor;
	for (i=0; i<r; i++)
	{	
		if (i<uselinecolors) linecolor=linecolors[i];
		plot_vector(mat(x,cx,(i>=rx)?0:i,0),mat(y,cy,(i>=ry)?0:i,0),cx,cy);
		if (test_key()==escape) break;
	}
	linecolor=old;
	gflush();
}

void do_mark (header *hdx, header *hdy, header *hdc=0)
{	
	int cx,rx,cy,ry,i,ix,iy,ccol=0,rcol=0;
	double *x,*y,h,*col=0;
	getmatrix(hdx,&rx,&cx,&x); 
	getmatrix(hdy,&ry,&cy,&y);
	if (hdc) getmatrix(hdc,&rcol,&ccol,&col);
	int r=max(rx,ry),c=max(cx,cy);
	if (r==0 || c==0) return;
	if ((cx>1 && cx!=c) || (rx>1 && rx!=r) || (cy>1 && cy!=c) || (ry>1 && ry!=r))
	{	
		error=22; 
		print("Matrices must fit for mark!\nGot %dx%d and %dx%d\n",rx,ry,cx,cy);
		return;
	}
	if (scaling)
	{	
		minmax(x,(LONG)cx*rx,&x_min,&x_max,&ix,&iy);
		minmax(y,(LONG)cy*ry,&y_min,&y_max,&ix,&iy);
		if (keepsquare)
		{	h=(y_max+y_min)/2;
			y_max=h+(x_max-x_min)/2;
			y_min=h-(x_max-x_min)/2;
		}
		if (!setrange(1)) return;
	}
	graphic_mode();
	// if (!holding) frame();
	int old=linecolor;
	for (i=0; i<r; i++)
	{	
		if (hdc && rcol>0 && ccol>0) 
			plot_marks_color(mat(x,cx,(i>=rx)?0:i,0),mat(y,cy,(i>=ry)?0:i,0),
				mat(col,ccol,(i>=rcol)?0:i,0),cx,cy,ccol);
		else
		{
			if (i<uselinecolors) linecolor=linecolors[i];
			plot_marks(mat(x,cx,(i>=rx)?0:i,0),mat(y,cy,(i>=ry)?0:i,0),cx,cy);
		}
		if (test_key()==escape) break;
	}
	linecolor=old;
	gflush();
}

void mplot (header *hd)
{	
	header *hd1=0,*st=hd,*result;
	double *x;
	hd=getvalue(hd); if (error) return;
	if (hd) /* parameters given */
	{	
		if (hd->type!=s_matrix && hd->type!=s_real)
		{	error=21;
			output("Plot needs a real vector or matrix!\n");
			return;
		}
		hd1=next_param(st);
		if (hd1) hd1=getvalue(hd1); if (error) return;
		if (hd1->type!=s_matrix && hd1->type!=s_real)
		{	error=11001; output("Wrong arguments for plot!\n");
			return;
		}
	}
	do_plot(hd,hd1);
	result=new_matrix(1,4,""); if (error) return;
	x=matrixof(result);
	*x++=x_min_old; *x++=x_max_old; *x++=y_min_old; *x=y_max_old;
	moveresult(st,result);
}

void mplotarea (header *hd)
{	header *hd1=0,*st=hd,*result;
	double *x,*y,h;
	int cx,rx,cy,ry,ix,iy;
	hd=getvalue(hd); if (error) return;
	if (hd) /* parameters given */
	{	if (hd->type!=s_matrix && hd->type!=s_real)
		{	error=21;
			output("Plot needs a real vector or matrix!\n");
			return;
		}
		hd1=next_param(st);
		if (hd1) hd1=getvalue(hd1); if (error) return;
		if (hd1->type!=s_matrix && hd1->type!=s_real)
		{	error=11000; output("Wrong arguments for plotarea!\n"); return;
		}
	}
	getmatrix(hd,&rx,&cx,&x); getmatrix(hd1,&ry,&cy,&y);
	int r=max(rx,ry),c=max(cx,cy);
	if ((cx>1 && cx!=c) || (rx>1 && ry!=r) || (cy>1 && cy!=c) || (ry>1 && ry!=r))
	{	error=22; output("Matrices must fit for plotarea!\n");
		return;
	}
	if (scaling)
	{	minmax(x,(LONG)cx*rx,&x_min,&x_max,&ix,&iy);
		minmax(y,(LONG)cy*ry,&y_min,&y_max,&ix,&iy);
		if (keepsquare)
		{	h=(y_max+y_min)/2;
			y_max=h+(x_max-x_min)/2;
			y_min=h-(x_max-x_min)/2;
		}
		setrange(1);
	}
	scaling=0;
	result=new_matrix(1,4,""); if (error) return;
	x=matrixof(result);
	*x++=x_min_old; *x++=x_max_old; *x++=y_min_old; *x++=y_max_old;
	moveresult(st,result);
}

void mpixel (header *hd)
{	double x,y;
	hd=new_matrix(1,2,""); if (error) return;
	getpixelsize(&x,&y);
	x*=(x_max-x_min)/(lowerc-upperc);
	y*=(y_max-y_min)/(lowerr-upperr);
	*(matrixof(hd))=x; *(matrixof(hd)+1)=y;
}

void mmark (header *hd)
{	header *hd1,*st=hd,*result;
	double *x;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix && hd->type!=s_real)
	{	error=21; output("Mark needs a vector or matrix!\n");
		return;
	}
	hd1=next_param(st);
	if (hd1) hd1=getvalue(hd1); if (error) return;
	if (hd1->type!=s_matrix && hd1->type!=s_real)
	{	error=-1; output("Illegal arguments for mark!\n"); return;
	}
	do_mark(hd,hd1);
	result=new_matrix(1,4,""); if (error) return;
	x=matrixof(result);
	*x++=x_min_old; *x++=x_max_old; *x++=y_min_old; *x=y_max_old;
	moveresult(st,result);
	gflush();
}

void mmarkcolor (header *hd)
{	header *hd1,*st=hd,*result,*hdc;
	double *x;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix && hd->type!=s_real)
	{	error=21; output("Mark needs a vector or matrix!\n");
		return;
	}
	hd1=next_param(st);
	hdc=next_param(hd1);
	if (hd1) hd1=getvalue(hd1); if (error) return;
	if (hd1->type!=s_matrix && hd1->type!=s_real)
	{	error=-1; output("Illegal arguments for mark!\n"); return;
	}
	if (hdc) hdc=getvalue(hdc); if (error) return;
	if (hdc->type!=s_matrix && hdc->type!=s_real)
	{	error=-1; output("Illegal arguments for mark!\n"); return;
	}
	do_mark(hd,hd1,hdc);
	result=new_matrix(1,4,""); if (error) return;
	x=matrixof(result);
	*x++=x_min_old; *x++=x_max_old; *x++=y_min_old; *x=y_max_old;
	moveresult(st,result);
	gflush();
}

void ghold (void)
/**** hold
	toggles holding of the current plot.
****/
{	
	static int oldhold=-1;
	scan_space();
	if (!strncmp(next,"off",3))
	{	oldhold=-1; holding=0; next+=3;
	}
	else if (!strncmp(next,"on",2))
	{	oldhold=-1; holding=1; next+=2;
	}
	else
	{	
		if (oldhold!=-1) {	holding=oldhold; oldhold=-1; }
		else { oldhold=holding; holding=1; }
	}
	scaling=!holding;
}

void show_graphics (void)
{	
	// int scan;
	graphic_mode(); // wait_key(&scan); text_mode();
}

void mmesh (header *hd)
{	double *screen_col,*screen_row;
	LONG col,size;
	double *m,ymin,ymax,xxscale,xyscale,yxscale,yyscale;
	int imin,imax,c,r,i,j;
	double cc[8];
	header *st=hd;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix || dimsof(hd)->c<2 || dimsof(hd)->r<2)
	{	output("Illegal parameter for mesh!\n"); error=80; return;
	}
	getmatrix(hd,&r,&c,&m); col=r;
	minmax(m,(LONG)c*r,&ymin,&ymax,&imin,&imax);
	if (ymin==ymax) ymax=ymin+1;
	size=(LONG)c*r*sizeof(double);
	screen_col=(double *)newram;
	screen_row=(double *)(newram+size);
	if (!freeram(2*size))
	{	output("Out of memory in mesh!\n");
		error=85; return;
	}

	xxscale=0.6*(lowerc-upperc)/c; xyscale=0.4*(lowerc-upperc)/r;
	yxscale=0.2*(lowerr-upperr)/r;
	yyscale=0.8*meshfactor*(lowerr-upperr)/(ymax-ymin);
	for (i=0; i<c; i++)
		for (j=0; j<r; j++)
		{	screen_col[col*i+j]=upperc+(xxscale*i+xyscale*j);
			screen_row[col*i+j]=lowerr-(yxscale*j+
				yyscale*(*mat(m,c,j,i)-ymin));
			if (test_key()==escape)
			{	output("User interrupted!\n");
				error=1; return;
			}
		}
	if (!holding) gclear();
	graphic_mode();
	for (i=0; i<c-1; i++)
		for (j=r-2; j>=0; j--)
		{  	cc[4]=( screen_col[col*i+j] + screen_col[col*(i+1)+j] +
		   	     screen_col[col*(i+1)+j+1] + screen_col[col*i+j+1])/4;
			cc[5]=( screen_row[col*i+j] + screen_row[col*(i+1)+j] +
			     screen_row[col*(i+1)+j+1] + screen_row[col*i+j+1])/4;
			cc[0]=screen_col[col*(i+1)+j+1]; cc[1]=screen_row[col*(i+1)+j+1];
			cc[2]=screen_col[col*i+j+1]; cc[3]=screen_row[col*i+1+j];
			if (twosides &&
				((cc[2]-cc[0])*(cc[5]-cc[1])-
					(cc[3]-cc[1])*(cc[4]-cc[0]))>=0
				)
				gfill(cc,fillcolor1,wirecolor,3,tconnected);
			else gfill(cc,fillcolor2,wirecolor,3,tconnected);
			cc[0]=screen_col[col*i+j+1]; cc[1]=screen_row[col*i+j+1];
			cc[2]=screen_col[col*i+j]; cc[3]=screen_row[col*i+j];
			if (twosides &&
				((cc[2]-cc[0])*(cc[5]-cc[1])-
					(cc[3]-cc[1])*(cc[4]-cc[0]))>=0
				)
				gfill(cc,fillcolor1,wirecolor,3,tconnected);
			else gfill(cc,fillcolor2,wirecolor,3,tconnected);
			cc[0]=screen_col[col*(i+1)+j]; cc[1]=screen_row[col*(i+1)+j];
			cc[2]=screen_col[col*(i+1)+j+1]; cc[3]=screen_row[col*(i+1)+j+1];
			if (twosides &&
				((cc[2]-cc[0])*(cc[5]-cc[1])-
					(cc[3]-cc[1])*(cc[4]-cc[0]))>=0
				)
				gfill(cc,fillcolor1,wirecolor,3,tconnected);
			else gfill(cc,fillcolor2,wirecolor,3,tconnected);
			cc[0]=screen_col[col*i+j]; cc[1]=screen_row[col*i+j];
			cc[2]=screen_col[col*(i+1)+j]; cc[3]=screen_row[col*(i+1)+j];
			if (twosides &&
				((cc[2]-cc[0])*(cc[5]-cc[1])-
					(cc[3]-cc[1])*(cc[4]-cc[0]))>=0
				)
				gfill(cc,fillcolor1,wirecolor,3,tconnected);
			else gfill(cc,fillcolor2,wirecolor,3,tconnected);
			if (test_key()==escape)
			{	output("User interrupted!\n");
				error=1; gflush(); return;
			}
		}
	hd=new_matrix(1,2,"");
	*matrixof(hd)=ymin; *(matrixof(hd)+1)=ymax;
	moveresult(st,hd);
	gflush();
}

void mmeshflat (header *hd)
{	double *m,ymin,ymax,xxscale,xyscale,yxscale,yyscale;
	int imin,imax,c,r,i,j,c1;
	double cc[8];
	double sc,sr;
	header *st=hd;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix || dimsof(hd)->c<1 || dimsof(hd)->r<1)
	{	output("Illegal parameter for meshbar!\n"); error=80; return;
	}
	getmatrix(hd,&r,&c,&m);
	minmax(m,(LONG)c*r,&ymin,&ymax,&imin,&imax);
	if (ymin==ymax) ymax=ymin+1;
	if (c>r) c1=c;
	else c1=r;

	xxscale=0.6*(lowerc-upperc)/c1;
	xyscale=0.4*(lowerc-upperc)/c1;
	yxscale=0.2*(lowerr-upperr)/c1;
	yyscale=0.8*meshfactor*(lowerr-upperr)/(ymax-ymin);
	if (!holding) gclear();
	graphic_mode();
	for (i=0; i<c; i++)
		for (j=r-1; j>=0; j--)
		{   sc=upperc+(xxscale*i+xyscale*j);
			sr=lowerr-(yxscale*j+yyscale*(*mat(m,c,j,i)-ymin));
			cc[0]=sc; cc[1]=sr;
			cc[2]=sc+xxscale; cc[3]=sr;
			cc[4]=sc+xxscale+xyscale; cc[5]=sr-yxscale;
			cc[6]=sc+xyscale; cc[7]=sr-yxscale;
			gfill(cc,fillcolor1,wirecolor,4,connected);
			cc[0]=sc; cc[1]=sr;
			cc[2]=sc+xxscale; cc[3]=sr;
			cc[4]=sc+xxscale; cc[5]=lowerr-yxscale*j;
			cc[6]=sc; cc[7]=lowerr-yxscale*j;
			gfill(cc,fillcolor1,wirecolor,4,connected);
			cc[0]=sc+xxscale; cc[1]=sr;
			cc[2]=sc+xxscale+xyscale; cc[3]=sr-yxscale;
			cc[4]=sc+xxscale+xyscale; cc[5]=lowerr-yxscale*(j+1);
			cc[6]=sc+xxscale; cc[7]=lowerr-yxscale*j;
			gfill(cc,fillcolor1,wirecolor,4,connected);
			if (test_key()==escape)
			{	output("User interrupted!\n");
				error=1; gflush(); return;
			}
		}
	hd=new_matrix(1,2,"");
	*matrixof(hd)=ymin; *(matrixof(hd)+1)=ymax;
	moveresult(st,hd);
	gflush();
}

double cos_up,sin_up,cos_left,sin_left,sin_d,cos_d,anaglyph_d=0;

void turn (double *x, double *y, double cs, double sn)
{	double h;
	h=*x*cs-*y*sn;
	*y=*x*sn+*y*cs;
	*x=h;
}

double project (double x, double y, double z, double *c, double *r)
/***** project
	3D-projection onto the screen.
*****/
{	
	x+=xcenter; y+=ycenter; z+=zcenter;
	turn(&y,&x,cos_left,sin_left);
	turn(&y,&z,cos_up,sin_up);
	turn(&y,&x,cos_d,sin_d);
	if (y<-0.9*distance) y=-0.9*distance;
	x/=(y+distance); z/=(y+distance);
	*c=((upperc+lowerc)/2+(lowerc-upperc)/2*x*tele);
	double asp=getaspect();
	if (asp!=1)
	{
		*c=512+(*c-512)/asp;
	}
	*r=((upperr+lowerr)/2-(lowerr-upperr)/2*z*tele);
	return y;
}

void mcenter (header *hd)
{	
	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (!isnone(hd) && (hd->type!=s_matrix || dimsof(hd)->r!=1 || dimsof(hd)->c!=3))
		need_arg_in("center","vector [x,y,z]");
	result=new_matrix(1,3,""); if (error) return;
	double *m=matrixof(result); 
	*m++=xcenter; *m++=ycenter; *m++=zcenter;
	if (!isnone(hd))
	{
		m=matrixof(hd); 
		xcenter=*m++; ycenter=*m++; zcenter=*m++;
	}
	moveresult(st,result);
}

void mcenter0 (header *hd)
{	header *result=new_matrix(1,3,""); if (error) return;
	double *m=matrixof(result); 
	*m++=xcenter; *m++=ycenter; *m++=zcenter;
}

void mproject (header *hd)
{	LONG col;
	double *mx,*my,*mz,*screen_col,*screen_row,*screen_d;
	int c,r,i,j;
	header *st=hd,*hd1,*hd2,*result1,*result2,*result3;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real)
	{
		hd1=next_param(st); hd2=next_param(hd1);
		hd1=getvalue(hd1); hd2=getvalue(hd2);
		if (error) return;
		if (hd->type!=s_real || hd1->type!=s_real || hd2->type!=s_real)
			need_arg_in("project","two matrices of same size, or two reals");
		double *x=realof(hd),*y=realof(hd1), *z=realof(hd2);
		
		cos_left=cos(a_left); sin_left=sin(a_left);
		cos_up=cos(a_up); sin_up=sin(a_up);
		cos_d=cos(anaglyph_d); sin_d=(anaglyph_d);

		double c,r;
		double d=project(*x,*y,*z,&c,&r);

		result1=new_real(c,"");
		result2=new_real(r,"");
		result3=new_real(d,"");
	}
	else
	{
		if (hd->type!=s_matrix)
			need_arg_in("project","two matrices of same size, or two reals");
		getmatrix(hd,&r,&c,&mx); col=c;
		hd1=next_param(st); hd2=next_param(hd1);
		hd1=getvalue(hd1); hd2=getvalue(hd2);
		if (error) return;
		if (hd1->type!=s_matrix || hd2->type!=s_matrix ||
			dimsof(hd1)->r!=r || dimsof(hd2)->r!=r ||
			dimsof(hd1)->c!=c || dimsof(hd2)->c!=c)
				need_arg_in("project","two matrices of same size, or two reals");
		my=matrixof(hd1); mz=matrixof(hd2);
		result1=new_matrix(r,c,""); if (error) return;
		result2=new_matrix(r,c,""); if (error) return;
		result3=new_matrix(r,c,""); if (error) return;
		screen_col=matrixof(result1);
		screen_row=matrixof(result2);
		screen_d=matrixof(result3);
		
		cos_left=cos(a_left); sin_left=sin(a_left);
		cos_up=cos(a_up); sin_up=sin(a_up);
		cos_d=cos(anaglyph_d); sin_d=(anaglyph_d);
		for (i=0; i<r; i++)
			for (j=0; j<c; j++)
			{	*screen_d++=project(*mat(mx,c,i,j),*mat(my,c,i,j),*mat(mz,c,i,j),
					screen_col++,screen_row++);
				if (test_key()==escape)
				{   output("User interrupted!\n");
					error=1; return;
				}
			}
	}

	moveresult1(st,result1);
}


typedef struct { int i,j; double z; } recttyp;

int compare (const recttyp **r1, const recttyp **r2)
{	if ((*r1)->z > (*r2)->z) return -1;
	else if ((*r1)->z < (*r2)->z) return 1;
	else return 0;
}

void wire_or_rows (header *hd, int rows)
{	double *screen_col,*screen_row;
	LONG col,size;
	double *mx,*my,*mz;
	int c,r,i,j,cx,cy,cz,rx,ry,rz;
	header *st=hd,*hd1,*hd2;
	hd1=next_param(st); hd2=next_param(hd1);
	hd=getvalue(hd);
	hd1=getvalue(hd1); hd2=getvalue(hd2);
	if (error) return;
	if (!isreal(hd) || !isreal(hd1) || !isreal(hd2))
		need_arg_in("wire","three compatible matrices x,y,z");
	getmatrix(hd,&rx,&cx,&mx);
	getmatrix(hd1,&ry,&cy,&my);
	getmatrix(hd2,&rz,&cz,&mz);
	r=rx; if (ry>r) r=ry; if (rz>r) r=rz;
	c=cx; if (cy>c) c=cy; if (cz>c) r=rz;
	col=c;
	if (r<1 || c<1)
		need_arg_in("wire","three compatible matrices x,y,z");
	if ((rx>1 && rx!=r) || (ry>1 && ry!=r) || (rz>1 && rz!=r)
		|| (cx>1 && cx!=c) || (cy>1 && cy!=c) || (cz>1 && cz!=c))
		need_arg_in("wire","three compatible matrices x,y,z");
	size=(LONG)c*r*sizeof(double);
	screen_col=(double *)newram;
	screen_row=(double *)(newram+size);
	if (!freeram(2*size))
	{	output("Out of memory in wire!\n");
		error=85; return;
	}

	cos_left=cos(a_left); sin_left=sin(a_left);
	cos_up=cos(a_up); sin_up=sin(a_up);
	cos_d=cos(anaglyph_d); sin_d=(anaglyph_d);
	for (i=0; i<r; i++)
		for (j=0; j<c; j++)
		{	project(
				*mat(mx,cx,(rx==1)?0:i,(cx==1)?0:j),
				*mat(my,cy,(ry==1)?0:i,(cy==1)?0:j),
				*mat(mz,cz,(rz==1)?0:i,(cz==1)?0:j),
				&screen_col[col*i+j],&screen_row[col*i+j]);
			if (test_key()==escape)
			{   output("User interrupted!\n");
				error=1; return;
			}
		}
	if (!holding) gclear();
	graphic_mode();
	if (c>=2) 
		for (i=0; i<r; i++)
		{	for (j=0; j<c-1; j++)
			{	if (!isnan(screen_col[col*i+j]) && !isnan(screen_col[col*i+j+1])) 
					gline(screen_col[col*i+j],screen_row[col*i+j],
					screen_col[col*i+j+1],screen_row[col*i+j+1],
					wirecolor,linetype,linewidth);
			}
		}
	if (r>=2 && !rows) 
		for (j=0; j<c; j++)
		{	for (i=0; i<r-1; i++)
			{	if (!isnan(screen_col[col*i+j]) && !isnan(screen_col[col*(i+1)+j]))
					gline(screen_col[col*i+j],screen_row[col*i+j],
					screen_col[col*(i+1)+j],screen_row[col*(i+1)+j],
					wirecolor,linetype,linewidth);
			}
		}
	hd=new_real(0.0,"");
	moveresult(st,hd);
	gflush();
}

void mwire (header *hd)
{	wire_or_rows(hd,0);
}

void mwirerows (header *hd)
{	wire_or_rows(hd,1);
}

typedef struct { int i,j,type; double z; } triangletyp;

int comparetriangle (const triangletyp **r1, const triangletyp **r2)
{	if ((*r1)->z > (*r2)->z) return -1;
	else if ((*r1)->z < (*r2)->z) return 1;
	else return 0;
}

/*
Plots a solid 3D surface defined on a mesh with edges.
The function takes x,y,z matrices, forming the quadrilaterals
of a surface. Each quadrilateral is split in two triangles.
The triangles are sorted and drawn from back to front.
*/
void msolid (header *hd)
{	
	double *screen_col,*screen_row,*screen_z;
	ULONG col,size,n,ind;
	double *mx,*my,*mz,z; // coordinates
	int c,r,i,j,rx,cx,ry,cy,rz,cz; // sizes
	double cc[8];
    int connect1[]={1,0,1};
	header *st=hd,*hd1,*hd2;
	triangletyp *trp; // to hold a triangle
	triangletyp **trpp,**trps; // pointers to triangles
	// get parameters
	hd=getvalue(hd);
	hd1=next_param(st); hd2=next_param(hd1);
	hd1=getvalue(hd1); hd2=getvalue(hd2);
	if (error) return;
	if (!isreal(hd) || !isreal(hd1) || !isreal(hd2))
		need_arg_in("sold","three compatible matrices x,y,z");
	// get matrices
	getmatrix(hd,&rx,&cx,&mx);
	getmatrix(hd1,&ry,&cy,&my);
	getmatrix(hd2,&rz,&cz,&mz);
	// get maximum size
	r=rx; if (ry>r) r=ry; if (rz>r) r=rz;
	c=cx; if (cy>c) c=cy; if (cz>c) r=rz;
	col=c;
	if (r<2 || c<2)
		need_arg_in("solid","three compatible matrices x,y,z");
	// check if each column is c or 1, each row is r or 1
	if ((rx>1 && rx!=r) || (ry>1 && ry!=r) || (rz>1 && rz!=r)
		|| (cx>1 && cx!=c) || (cy>1 && cy!=c) || (cz>1 && cz!=c))
		need_arg_in("solid","three compatible matrices x,y,z");
	// compute necessary sizes
	size=(LONG)c*r*sizeof(double);
	n=(LONG)(r-1)*(c-1);
	// place to hold the screen coordinates of the mesh corners
	screen_col=(double *)newram;
	screen_row=(double *)(newram+size);
	// place to hold the depth of each coordinate
    screen_z=(double *)(newram+2*size);
	// place for the triangles
	trp=(triangletyp *)(newram+3*size);
	// place for pointers to the triangles (for sorting)
	trpp=trps=(triangletyp **)(newram+3*size+2*n*sizeof(triangletyp));
	// check available space
	if (!freeram(3*size+(sizeof(triangletyp)+sizeof(triangletyp *))*2*n))
	{	
		output("Out of memory in solid!\n");
		error=85; return;
	}
	// fresh up coefficients of turn matrix (according to view)
	cos_left=cos(a_left); sin_left=sin(a_left);
	cos_up=cos(a_up); sin_up=sin(a_up);
	cos_d=cos(anaglyph_d); sin_d=(anaglyph_d);
	// project the corners and store in above places
	for (i=0; i<r; i++)
		for (j=0; j<c; j++)
		{	
			screen_z[col*i+j]=project(
				*mat(mx,cx,(rx==1)?0:i,(cx==1)?0:j),
				*mat(my,cy,(ry==1)?0:i,(cy==1)?0:j),
				*mat(mz,cz,(rz==1)?0:i,(cz==1)?0:j),
				&screen_col[col*i+j],&screen_row[col*i+j]);
			// takes too long?
			if (test_key()==escape)
			{   output("User interrupted");
				error=1; return;
			}
		}
	// setup the list of triangles
	int unsigned nind=0;
	for (i=0; i<r-1; i++)
		for (j=0; j<c-1; j++)
		{   
			// compute average depth in each upper left triangle
			z=(screen_z[col*i+j]+screen_z[col*(i+1)+j]
        		+screen_z[col*i+j+1])/3;
			if (!isnan(z))
			{	
				// store the triangle
        		trp->i=i; trp->j=j; trp->type=0; trp->z=z;
				// store the pointer
				*trpp++=trp; trp++;
				// same for lower right triangle in quadrilateral
				z=(screen_z[col*(i+1)+(j+1)]+screen_z[col*(i+1)+j]
        			+screen_z[col*i+j+1])/3;
        		trp->i=i; trp->j=j; trp->type=1; trp->z=z;
				*trpp++=trp; trp++;
				nind++;
			}
			// takes too long?
			if (test_key()==escape)
			{   output("User interrupted");
				error=1; return;
			}
		}
	// sort the triangles
	qsort(trps,2*nind,sizeof(triangletyp *),
		(int (*)(const void *, const void *))comparetriangle);
	// plot everything
	if (!holding) gclear();
	graphic_mode();
	trpp=trps;
	int skip=((iskip>1)||(jskip>1));
	for (ind=0; ind<2*nind; ind++)
	{	
		i=(*trpp)->i; j=(*trpp)->j;
		// need different drawing for triangles, since the
		// sides are drawn with edges only around for the
		// quadrilateral
    	if ((*trpp)->type==0) // upper left trangle
		{	
			if (skip)
			{
				connect1[0]=((j%jskip)==0);
				connect1[2]=((i%iskip)==0);
			}
			cc[0]=screen_col[col*i+j]; cc[1]=screen_row[col*i+j];
			cc[2]=screen_col[col*(i+1)+j]; cc[3]=screen_row[col*(i+1)+j];
			cc[4]=screen_col[col*i+(j+1)]; cc[5]=screen_row[col*i+(j+1)];
			int nan=0;
			for (int k=0; k<6; k++)
			{	if (isnan(cc[k])) { nan=1; break; }
			}
			if (!nan)
			{	// back side of triangle is darker
				// determine front side with sign of determinant
				double side=((cc[2]-cc[0])*(cc[5]-cc[1])-(cc[3]-cc[1])*(cc[4]-cc[0]));
				if (!twosides || side>=0)
				{	
					gfill(cc,fillcolor2,wirecolor,3,connect1);
				}
				else
				{	
					gfill(cc,fillcolor1,wirecolor,3,connect1);
				}
			}
        }
        else // lower right triangle
		{	
			if (skip)
			{
				connect1[0]=(((i+1)%iskip)==0);
				connect1[2]=(((j+1)%jskip)==0);
			}
			cc[0]=screen_col[col*(i+1)+(j+1)];
        	cc[1]=screen_row[col*(i+1)+(j+1)];
			cc[2]=screen_col[col*(i+1)+j]; cc[3]=screen_row[col*(i+1)+j];
			cc[4]=screen_col[col*i+(j+1)]; cc[5]=screen_row[col*i+(j+1)];
			int nan=0;
			for (int k=0; k<6; k++)
			{	if (isnan(cc[k])) { nan=1; break; }
			}
			if (!nan)
			{	double side=((cc[2]-cc[0])*(cc[5]-cc[1])-(cc[3]-cc[1])*(cc[4]-cc[0]));
				if (!twosides || side<=0)
				{	
					gfill(cc,fillcolor2,wirecolor,3,connect1);
				}
				else
				{	
					gfill(cc,fillcolor1,wirecolor,3,connect1);
				}
			}
        }
		if (test_key()==escape)
		{   output("User interrupted!\n");
			error=1; gflush(); return;
		}
		trpp++;
	}
	// uninteresting result
	hd=new_real(0.0,"");
	moveresult(st,hd);
	gflush();
}

/*
Some help functions for the hue contour plots with niveau lines.
*/

void shcline (double x1, double y1, double z1, 
	  double x2, double y2, double z2)
{	
	double sc1,sr1,sc2,sr2;
	project(x1,y1,z1,&sc1,&sr1);
	project(x2,y2,z2,&sc2,&sr2);
	gline(sc1,sr1,sc2,sr2,contourcolor,line_solid,linewidth);
}

void shcgridline (double x1, double y1, double z1, 
	  double x2, double y2, double z2)
{	
	double sc1,sr1,sc2,sr2;
	project(x1,y1,z1,&sc1,&sr1);
	project(x2,y2,z2,&sc2,&sr2);
	gline(sc1,sr1,sc2,sr2,wirecolor,line_solid,linewidth);
}

double bary (double h, double z1, double z2)
	// compute the t s.t. z1+t*(z2-z1)=h.
{	
	if (z1==z2)
	{	
		if (h==z1) return 0.5;
		else return 1e20;
	}
	return (h-z2)/(z1-z2);
}

void shchelp1 (double x0, double y0, double z0, double w0,
			double x1, double y1, double z1, double w1,
			double x2, double y2, double z2, double w2,
			int n, double *v)
	// help function for solidhuecontour.
	// draws level lines for a triangle for the values in the vector
	// v of length n.
{	
	double eps;
	for (int i=0; i<n; i++)
	{	
		eps=1e-8;
		double h=*v++;
		if (fabs(w0-w1)<eps && fabs(h-w0)<eps)
		{	
			shcline(x0,y0,z0,x1,y1,z1);
		}
		else if (fabs(w1-w2)<eps && fabs(h-w1)<eps)
		{	
			shcline(x1,y1,z1,x2,y2,z2);
		}
		else if (fabs(w2-w0)<eps && fabs(h-w2)<eps)
		{	
			shcline(x2,y2,z2,x0,y0,z0);
		}
		else
		{	
			double t1=bary(h,w0,w1);
			double t2=bary(h,w1,w2);
			double t3=bary(h,w2,w0);
			eps=0.1;
			if (t1>=-eps && t1<=1+eps)
			{	if (t2>=-eps && t2<=1+eps)
				{	shcline(t1*x0+(1-t1)*x1,t1*y0+(1-t1)*y1,t1*z0+(1-t1)*z1,
						t2*x1+(1-t2)*x2,t2*y1+(1-t2)*y2,t2*z1+(1-t2)*z2);
				}
				if (t3>=-eps && t3<=1+eps)
				{	shcline(t1*x0+(1-t1)*x1,t1*y0+(1-t1)*y1,t1*z0+(1-t1)*z1,
						t3*x2+(1-t3)*x0,t3*y2+(1-t3)*y0,t3*z2+(1-t3)*z0);
				}
			}
			else if (t2>=-eps && t2<=1+eps)
			{	if (t3>=-eps && t3<=1+eps)
				{	shcline(t2*x1+(1-t2)*x2,t2*y1+(1-t2)*y2,t2*z1+(1-t2)*z2,
						t3*x2+(1-t3)*x0,t3*y2+(1-t3)*y0,t3*z2+(1-t3)*z0);
				}
			}
		}
	}
}

void shchelp (double x0, double y0, double z0, double w0,
			double x1, double y1, double z1, double w1,
			double x2, double y2, double z2, double w2,
			double x3, double y3, double z3, double w3,
			int n, double *v)
	// help function for solidhuecontour.
	// splits a quadiralateral into four triangles,
	// and draws level lines on these triangles.
{	
	double x=(x0+x1+x2+x3)/4;
	double y=(y0+y1+y2+y3)/4;
	double z=(z0+z1+z2+z3)/4;
	double w=(w0+w1+w2+w3)/4;
	shchelp1(x0,y0,z0,w0,x1,y1,z1,w1,x,y,z,w,n,v);
	shchelp1(x1,y1,z1,w1,x2,y2,z2,w2,x,y,z,w,n,v);
	shchelp1(x2,y2,z2,w2,x3,y3,z3,w3,x,y,z,w,n,v);
	shchelp1(x3,y3,z3,w3,x0,y0,z0,w0,x,y,z,w,n,v);
}

double xbary (double h, double z1, double z2)
	// compute the t s.t. z1+t*(z2-z1)=h.
{	
	if (z1==z2)
	{	
		if (h==z1) return 0.5;
		else return 1e20;
	}
	return (h-z1)/(z2-z1);
}

void sxhchelp3 (double x0, double y0, double z0, double w0,
			double x1, double y1, double z1, double w1,
			double x2, double y2, double z2, double w2,
			double h1, double h2,
			int h, double hue, int color)
	// help function for xsolidhuecontour
	// here, w0 <= w1 <= w2 and h1 <= h2
{	
	double c[10];
	static int cc[]={0,0,0,0,0,0};
	if (w2<=h2)
	{
		if (w0>=h1) // h1 <= w0 <= w1 <= w2 <= h2
		{
			project(x0,y0,z0,c,c+1);
			project(x1,y1,z1,c+2,c+3);
			project(x2,y2,z2,c+4,c+5);
			if (hue) gfillh(c,3,hue,color,0);
			else gfill(c,contourcolor,0,3,cc);
		}
		else if (w1>h1) // w0 < h1 < w1 <= w2 <= h2
		{
			double k1=xbary(h1,w0,w1);
			double k2=xbary(h1,w0,w2);
			project(x0+k1*(x1-x0),y0+k1*(y1-y0),z0+k1*(z1-z0),c,c+1);
			project(x0+k2*(x2-x0),y0+k2*(y2-y0),z0+k2*(z2-z0),c+2,c+3);
			project(x2,y2,z2,c+4,c+5);
			project(x1,y1,z1,c+6,c+7);
			if (hue) gfillh(c,4,hue,color,0);
			else gfill(c,contourcolor,0,4,cc);
		}
		else if (w2>h1) // w0 <= w1 <= h1 < w2 <= h2
		{
			double k1=xbary(h1,w1,w2);
			double k2=xbary(h1,w0,w2);
			project(x1+k1*(x2-x1),y1+k1*(y2-y1),z1+k1*(z2-z1),c,c+1);
			project(x0+k2*(x2-x0),y0+k2*(y2-y0),z0+k2*(z2-z0),c+2,c+3);
			project(x2,y2,z2,c+4,c+5);
			if (hue) gfillh(c,3,hue,color,0);
			else gfill(c,contourcolor,0,3,cc);
		}
	}
	else if (w1<=h2)
	{
		if (w0>=h1) // h1 <= w0 <= w1 <= h2 < w2
		{
			double k1=xbary(h2,w0,w2);
			double k2=xbary(h2,w1,w2);
			project(x0+k1*(x2-x0),y0+k1*(y2-y0),z0+k1*(z2-z0),c,c+1);
			project(x1+k2*(x2-x1),y1+k2*(y2-y1),z1+k2*(z2-z1),c+2,c+3);
			project(x1,y1,z1,c+4,c+5);
			project(x0,y0,z0,c+6,c+7);
			if (hue) gfillh(c,4,hue,color,0);
			else gfill(c,contourcolor,0,4,cc);
		}
		else if (w1>h1) // w0 < h1 < w1 <= h2 < w2
		{
			double k1=xbary(h2,w0,w2);
			double k2=xbary(h2,w1,w2);
			project(x0+k1*(x2-x0),y0+k1*(y2-y0),z0+k1*(z2-z0),c,c+1);
			project(x1+k2*(x2-x1),y1+k2*(y2-y1),z1+k2*(z2-z1),c+2,c+3);
			project(x1,y1,z1,c+4,c+5);
			double k3=xbary(h1,w0,w1);
			double k4=xbary(h1,w0,w2);
			project(x0+k3*(x1-x0),y0+k3*(y1-y0),z0+k3*(z1-z0),c+6,c+7);
			project(x0+k4*(x2-x0),y0+k4*(y2-y0),z0+k4*(z2-z0),c+8,c+9);
			if (hue) gfillh(c,5,hue,color,0);
			else gfill(c,contourcolor,0,5,cc);

		}
		else // w0 <= w1 <= h1 < h2 < w2
		{
			double k1=xbary(h2,w0,w2);
			double k2=xbary(h2,w1,w2);
			project(x0+k1*(x2-x0),y0+k1*(y2-y0),z0+k1*(z2-z0),c,c+1);
			project(x1+k2*(x2-x1),y1+k2*(y2-y1),z1+k2*(z2-z1),c+2,c+3);
			double k3=xbary(h1,w1,w2);
			double k4=xbary(h1,w0,w2);
			project(x1+k3*(x2-x1),y1+k3*(y2-y1),z1+k3*(z2-z1),c+4,c+5);
			project(x0+k4*(x2-x0),y0+k4*(y2-y0),z0+k4*(z2-z0),c+6,c+7);
			if (hue) gfillh(c,4,hue,color,0);
			else gfill(c,contourcolor,0,4,cc);
		}
	}
	else if (w0<h2)
	{
		if (w0>=h1) // h1 <= w0 < h2 < w1 <= w2
		{
			double k1=xbary(h2,w0,w1);
			double k2=xbary(h2,w0,w2);
			project(x0+k1*(x1-x0),y0+k1*(y1-y0),z0+k1*(z1-z0),c,c+1);
			project(x0+k2*(x2-x0),y0+k2*(y2-y0),z0+k2*(z2-z0),c+2,c+3);
			project(x0,y0,z0,c+4,c+5);
			if (hue) gfillh(c,3,hue,color,0);
			else gfill(c,contourcolor,0,3,cc);
		}
		else // w0 < h1 < h2 < w1 <= w2
		{
			double k1=xbary(h2,w0,w1);
			double k2=xbary(h2,w0,w2);
			project(x0+k1*(x1-x0),y0+k1*(y1-y0),z0+k1*(z1-z0),c,c+1);
			project(x0+k2*(x2-x0),y0+k2*(y2-y0),z0+k2*(z2-z0),c+2,c+3);
			double k3=xbary(h1,w0,w2);
			double k4=xbary(h1,w0,w1);
			project(x0+k3*(x2-x0),y0+k3*(y2-y0),z0+k3*(z2-z0),c+4,c+5);
			project(x0+k4*(x1-x0),y0+k4*(y1-y0),z0+k4*(z1-z0),c+6,c+7);
			if (hue) gfillh(c,4,hue,color,0);
			else gfill(c,contourcolor,0,4,cc);
		}
	}
}

void sxhchelp2 (double x0, double y0, double z0, double w0,
			double x1, double y1, double z1, double w1,
			double x2, double y2, double z2, double w2,
			int n, double *v,
			int h, double hue, int color)
	// help function for solidhuecontour.
	// draws level lines for a triangle for the values in the vector
	// v of length n.
{	
	for (int i=0; i<n; i++)
	{	
		double h1=v[i],h2=v[n+i];
		if (h2>h1)
			sxhchelp3(x0,y0,z0,w0,x1,y1,z1,w1,x2,y2,z2,w2,h1,h2,h,hue,color);
	}
}

void sxhchelp1 (double x0, double y0, double z0, double w0,
			double x1, double y1, double z1, double w1,
			double x2, double y2, double z2, double w2,
			int n, double *v,
			int h, double hue, int color)
{
	if (w0<w1) 
	{
		if (w2<w0)
			sxhchelp2(x2,y2,z2,w2,x0,y0,z0,w0,x1,y1,z1,w1,n,v,h,hue,color);
		else if (w2<w1)
			sxhchelp2(x0,y0,z0,w0,x2,y2,z2,w2,x1,y1,z1,w1,n,v,h,hue,color);
		else
			sxhchelp2(x0,y0,z0,w0,x1,y1,z1,w1,x2,y2,z2,w2,n,v,h,hue,color);
	}
	else 
	{
		if (w2<w1)
			sxhchelp2(x2,y2,z2,w2,x1,y1,z1,w1,x0,y0,z0,w0,n,v,h,hue,color);
		else if (w2<w0)
			sxhchelp2(x1,y1,z1,w1,x2,y2,z2,w2,x0,y0,z0,w0,n,v,h,hue,color);
		else 
			sxhchelp2(x1,y1,z1,w1,x0,y0,z0,w0,x2,y2,z2,w2,n,v,h,hue,color);
	}
}

void sxhchelp (double x0, double y0, double z0, double w0,
			double x1, double y1, double z1, double w1,
			double x2, double y2, double z2, double w2,
			double x3, double y3, double z3, double w3,
			int n, double *v,
			int h=1, double hue=0, int color=0)
	// help function for solidhuecontour.
	// splits a quadiralateral into four triangles,
	// and draws level lines on these triangles.
{	
	double x=(x0+x1+x2+x3)/4;
	double y=(y0+y1+y2+y3)/4;
	double z=(z0+z1+z2+z3)/4;
	double w=(w0+w1+w2+w3)/4;
	sxhchelp1(x0,y0,z0,w0,x1,y1,z1,w1,x,y,z,w,n,v,h,hue,color);
	sxhchelp1(x1,y1,z1,w1,x2,y2,z2,w2,x,y,z,w,n,v,h,hue,color);
	sxhchelp1(x2,y2,z2,w2,x3,y3,z3,w3,x,y,z,w,n,v,h,hue,color);
	sxhchelp1(x3,y3,z3,w3,x0,y0,z0,w0,x,y,z,w,n,v,h,hue,color);
}

void mxsolidh (header *hdx)
/*
Solid plots with hues and optional level lines.
*/
{	
	header *st=hdx; // remember stack

	header *hdy=next_param(hdx); // x value
	header *hdz=next_param(hdy); // y value
	if (!hdz) need_arg_in("solid","at least x,y,z matrices");
	header *hdrows=next_param(hdz); // disconnected rows
	if (!hdrows) { msolid(hdx); return; }
	header *hdhue=next_param(hdrows); // hue value
	if (!hdhue) { msolid1(hdx); return; }
	header *hdw=next_param(hdhue); // values for levels
	header *hdlev=next_param(hdw); // levels
	header *hdlimits=next_param(hdlev); // upper and lower limits
	
	hdx=getvalue(hdx);
	hdy=getvalue(hdy); 
	hdz=getvalue(hdz); 
	hdrows=getvalue(hdrows);
	if (isnone(hdrows)) hdrows=0;
	hdhue=getvalue(hdhue);
	
	if (hdw) hdw=getvalue(hdw);
	if (hdlev) hdlev=getvalue(hdlev);
	if (isnone(hdlev)) hdlev=0;
	if (hdlimits) hdlimits=getvalue(hdlimits);
	if (isnone(hdlimits)) hdlimits=0;

	if (error) return; // may happen at any of the getvalues

	double *screen_col,*screen_row;
	ULONG col,size,n,ind;
	double *mx,*my,*mz,*mw,*mh,*mv,*mrows,*mlimits=0; // pointers to the matrices
	double z;
	int c,r,i,j,rx,cx,ry,cy,rz,cz,rh,ch,rv,cv,rw,cw,rrows,crows,rlimits,climits; // sizes

	double cc[8];
	
	recttyp *rectp;
	recttyp **rectpp,**rectps;
	
	if (error) return;
	if (!isreal(hdx) || !isreal(hdy) || !isreal(hdz) || !isreal(hdhue))
	{	
		need_arg_in("solid","real x, y, z, hue");
	}
	if (hdlev && !isreal(hdlev)) need_arg_in("solid","real levels")
	if (hdw && !isreal(hdw)) need_arg_in("solid","real values")
	if (hdlimits && !isreal(hdlimits)) need_arg_in("solid","real limits")

	int hue=1;
	if (hdhue->type==s_real && *realof(hdhue)==0) hue=0;
	
	getmatrix(hdx,&rx,&cx,&mx);
	getmatrix(hdy,&ry,&cy,&my);
	getmatrix(hdz,&rz,&cz,&mz);
	if (hdrows) getmatrix(hdrows,&rrows,&crows,&mrows);
	getmatrix(hdhue,&rh,&ch,&mh);
	
	if (hdlev) getmatrix(hdlev,&rv,&cv,&mv);
	if (hdw) getmatrix(hdw,&rw,&cw,&mw);
	if (hdlimits) getmatrix(hdlimits,&rlimits,&climits,&mlimits);

	r=rx; if (ry>r) r=ry; if (rz>r) r=rz; if (rh>r) r=rh; // get maximal row size
	c=cx; if (cy>c) c=cy; if (cz>c) r=rz; if (ch>c) ch=c; // get maximal col size
	col=c;

	if (r<2 || c<2)
	{	
		print("Size %d x %d not acceptable in solid plots\n",r,c);
		error=1; return;
	}

	if ((rx>1 && rx!=r) || (ry>1 && ry!=r) || (rz>1 && rz!=r)
		|| (cx>1 && cx!=c) || (cy>1 && cy!=c) || (cz>1 && cz!=c)
		|| (rh>1 && rh!=r && rh!=r-1) || (ch>1 && ch!=c && ch!=c-1))
	{	
		need_arg_in("solid","compatible matrices x, y, z, hue")
	}

	if ((hdw && rw>1 && rw!=r) || (hdw && cw>1 && cw!=c))
	{	
		need_arg_in("solid","compatible matrices x, y, z, hue and values")
	}

	if (hdlev && rv!=1 && rv!=2)
	{
		need_arg_in("solid","one or two rows of levels");
	}

	if (hdlimits && rlimits!=1 && climits!=2)
	{
		need_arg_in("solid","1x2 vector for limits");
	}
	if (hdlimits && *mlimits >= *(mlimits+1)) hdlimits=0;

	size=(LONG)c*r*sizeof(double);
	n=(LONG)(r-1)*(c-1);
	// place for screen coordinates
	screen_col=(double *)newram;
	screen_row=(double *)(newram+size);
	// place for list of quadrilaterals
	rectp=(recttyp *)(newram+2*size);
	// place for a list of pointers to quadrilaterals
	rectpp=rectps=(recttyp **)(newram+2*size+n*sizeof(recttyp));
	// check available space
	if (!freeram(2*size+(sizeof(recttyp)+sizeof(recttyp *))*n))
	{	
		output("Out of memory in solidhue!\n");
		error=85; return;
	}
	// determine matrix for projection (using view angles)
	cos_left=cos(a_left); sin_left=sin(a_left);
	cos_up=cos(a_up); sin_up=sin(a_up);
	cos_d=cos(anaglyph_d); sin_d=(anaglyph_d);
	// compute screen coordinates and list of quadrilaterals
	unsigned int nind=0;
	for (i=0; i<r; i++)
		for (j=0; j<c; j++)
		{	
			// project, getting the distance z, 
			// storing screen coordinates
			z=project(
				*mat(mx,cx,(rx==1)?0:i,(cx==1)?0:j),
				*mat(my,cy,(ry==1)?0:i,(cy==1)?0:j),
				*mat(mz,cz,(rz==1)?0:i,(cz==1)?0:j),
				&screen_col[col*i+j],&screen_row[col*i+j]);
			if (!isnan(z))
			{	// store the quadrilaterals in the list and the pointer to it
				if (i<r-1 && j<c-1)
				{	rectp->i=i; rectp->j=j; rectp->z=z;
					*rectpp++=rectp; rectp++; nind++;
				}
			}
			// takes too long?
			if (test_key()==escape)
			{	output("User interrupted!\n");
				error=1; return;
			}
		}
	// sort the rectangles, using the rectangle compare function
	qsort(rectps,nind,sizeof(recttyp *),
		(int (*)(const void *,const void *))compare);
	// plot the quadrilaterals from back to front
	if (!holding) gclear();
	graphic_mode();
	rectpp=rectps;
	for (ind=0; ind<nind; ind++)
	{	
		i=(*rectpp)->i; j=(*rectpp)->j;
		cc[0]=screen_col[col*i+j]; cc[1]=screen_row[col*i+j];
		cc[2]=screen_col[col*(i+1)+j]; cc[3]=screen_row[col*(i+1)+j];
		cc[4]=screen_col[col*(i+1)+j+1]; cc[5]=screen_row[col*(i+1)+(j+1)];
		cc[6]=screen_col[col*i+j+1]; cc[7]=screen_row[col*i+j+1];
		int nan=0;
		for (int k=0; k<8; k++)
			// see if any corner is not a number (nan).
		{	
			if (isnan(cc[k])) 
			{ 
				nan=1; break; 
			}
		}
		if (!nan)
		{	
			// call the gfillh system function
			if (hue && densitycolor!=255)
			{
				if (hdlimits)
				{
					sxhchelp(
						*mat(mx,cx,(rx==1)?0:i,(cx==1)?0:j),
						*mat(my,cy,(ry==1)?0:i,(cy==1)?0:j),
						*mat(mz,cz,(rz==1)?0:i,(cz==1)?0:j),
						*mat(mw,cw,(rw==1)?0:i,(cw==1)?0:j),
						*mat(mx,cx,(rx==1)?0:(i+1),(cx==1)?0:j),
						*mat(my,cy,(ry==1)?0:(i+1),(cy==1)?0:j),
						*mat(mz,cz,(rz==1)?0:(i+1),(cz==1)?0:j),
						*mat(mw,cw,(rw==1)?0:(i+1),(cw==1)?0:j),
						*mat(mx,cx,(rx==1)?0:(i+1),(cx==1)?0:(j+1)),
						*mat(my,cy,(ry==1)?0:(i+1),(cy==1)?0:(j+1)),
						*mat(mz,cz,(rz==1)?0:(i+1),(cz==1)?0:(j+1)),
						*mat(mw,cw,(rw==1)?0:(i+1),(cw==1)?0:(j+1)),
						*mat(mx,cx,(rx==1)?0:i,(cx==1)?0:(j+1)),
						*mat(my,cy,(ry==1)?0:i,(cy==1)?0:(j+1)),
						*mat(mz,cz,(rz==1)?0:i,(cz==1)?0:(j+1)),
						*mat(mw,cw,(rw==1)?0:i,(cw==1)?0:(j+1)),
						1,mlimits,
						1,*mat(mh,ch,(rh==1)?0:i,(ch==1)?0:j),densitycolor);
				}
				else
				{
					gfillh(cc,4,
						*mat(mh,ch,(rh==1)?0:i,(ch==1)?0:j),
						densitycolor,0);
				}
			}
			if (hdlev)
			{
				if (rv==2)
				{
					sxhchelp(
						*mat(mx,cx,(rx==1)?0:i,(cx==1)?0:j),
						*mat(my,cy,(ry==1)?0:i,(cy==1)?0:j),
						*mat(mz,cz,(rz==1)?0:i,(cz==1)?0:j),
						*mat(mw,cw,(rw==1)?0:i,(cw==1)?0:j),
						*mat(mx,cx,(rx==1)?0:(i+1),(cx==1)?0:j),
						*mat(my,cy,(ry==1)?0:(i+1),(cy==1)?0:j),
						*mat(mz,cz,(rz==1)?0:(i+1),(cz==1)?0:j),
						*mat(mw,cw,(rw==1)?0:(i+1),(cw==1)?0:j),
						*mat(mx,cx,(rx==1)?0:(i+1),(cx==1)?0:(j+1)),
						*mat(my,cy,(ry==1)?0:(i+1),(cy==1)?0:(j+1)),
						*mat(mz,cz,(rz==1)?0:(i+1),(cz==1)?0:(j+1)),
						*mat(mw,cw,(rw==1)?0:(i+1),(cw==1)?0:(j+1)),
						*mat(mx,cx,(rx==1)?0:i,(cx==1)?0:(j+1)),
						*mat(my,cy,(ry==1)?0:i,(cy==1)?0:(j+1)),
						*mat(mz,cz,(rz==1)?0:i,(cz==1)?0:(j+1)),
						*mat(mw,cw,(rw==1)?0:i,(cw==1)?0:(j+1)),
						cv,mv);
				}
				else
				{
					shchelp(
						*mat(mx,cx,(rx==1)?0:i,(cx==1)?0:j),
						*mat(my,cy,(ry==1)?0:i,(cy==1)?0:j),
						*mat(mz,cz,(rz==1)?0:i,(cz==1)?0:j),
						*mat(mw,cw,(rw==1)?0:i,(cw==1)?0:j),
						*mat(mx,cx,(rx==1)?0:(i+1),(cx==1)?0:j),
						*mat(my,cy,(ry==1)?0:(i+1),(cy==1)?0:j),
						*mat(mz,cz,(rz==1)?0:(i+1),(cz==1)?0:j),
						*mat(mw,cw,(rw==1)?0:(i+1),(cw==1)?0:j),
						*mat(mx,cx,(rx==1)?0:(i+1),(cx==1)?0:(j+1)),
						*mat(my,cy,(ry==1)?0:(i+1),(cy==1)?0:(j+1)),
						*mat(mz,cz,(rz==1)?0:(i+1),(cz==1)?0:(j+1)),
						*mat(mw,cw,(rw==1)?0:(i+1),(cw==1)?0:(j+1)),
						*mat(mx,cx,(rx==1)?0:i,(cx==1)?0:(j+1)),
						*mat(my,cy,(ry==1)?0:i,(cy==1)?0:(j+1)),
						*mat(mz,cz,(rz==1)?0:i,(cz==1)?0:(j+1)),
						*mat(mw,cw,(rw==1)?0:i,(cw==1)?0:(j+1)),
						cv,mv);
				}
			}
			if (dgrid)
				// grid lines should be drawn
			{
				if ((i%iskip)==0)
				{
					shcgridline(
						*mat(mx,cx,(rx==1)?0:i,(cx==1)?0:j),
						*mat(my,cy,(ry==1)?0:i,(cy==1)?0:j),
						*mat(mz,cz,(rz==1)?0:i,(cz==1)?0:j),
						*mat(mx,cx,(rx==1)?0:i,(cx==1)?0:j+1),
						*mat(my,cy,(ry==1)?0:i,(cy==1)?0:j+1),
						*mat(mz,cz,(rz==1)?0:i,(cz==1)?0:j+1));
				}
				if ((j%jskip)==0)
				{
					shcgridline(
						*mat(mx,cx,(rx==1)?0:i,(cx==1)?0:j),
						*mat(my,cy,(ry==1)?0:i,(cy==1)?0:j),
						*mat(mz,cz,(rz==1)?0:i,(cz==1)?0:j),
						*mat(mx,cx,(rx==1)?0:i+1,(cx==1)?0:j),
						*mat(my,cy,(ry==1)?0:i+1,(cy==1)?0:j),
						*mat(mz,cz,(rz==1)?0:i+1,(cz==1)?0:j));
				}
				if (((j+1)%jskip)==0)
				{
					shcgridline(
						*mat(mx,cx,(rx==1)?0:i,(cx==1)?0:j+1),
						*mat(my,cy,(ry==1)?0:i,(cy==1)?0:j+1),
						*mat(mz,cz,(rz==1)?0:i,(cz==1)?0:j+1),
						*mat(mx,cx,(rx==1)?0:i+1,(cx==1)?0:j+1),
						*mat(my,cy,(ry==1)?0:i+1,(cy==1)?0:j+1),
						*mat(mz,cz,(rz==1)?0:i+1,(cz==1)?0:j+1));
				}
				if (((i+1)%iskip)==0)
				{
					shcgridline(
						*mat(mx,cx,(rx==1)?0:i+1,(cx==1)?0:j),
						*mat(my,cy,(ry==1)?0:i+1,(cy==1)?0:j),
						*mat(mz,cz,(rz==1)?0:i+1,(cz==1)?0:j),
						*mat(mx,cx,(rx==1)?0:i+1,(cx==1)?0:j+1),
						*mat(my,cy,(ry==1)?0:i+1,(cy==1)?0:j+1),
						*mat(mz,cz,(rz==1)?0:i+1,(cz==1)?0:j+1));
				}
			}
			// takes too long?
			if (test_key()==escape)
			{	output("User interrupted!\n");
				error=1; gflush(); return;
			}
		}
		rectpp++;
	}
	header *hd=new_none(); // uninteresting result
	moveresult(st,hd);
	gflush();
}


void msolid1 (header *hd)
	// Solid with disconnected rows
{	
	double *screen_col,*screen_row;
	ULONG col,size,n,ind;
	double *mx,*my,*mz,z,*mult;
	int c,r,rx,cx,ry,cy,rz,cz,i,j,multc,multr,multi,multn,norectp=0;
	double cc[8];
	header *st=hd,*hd1,*hd2,*hdmult;
	recttyp *rectp;
	recttyp **rectpp,**rectps;
	hd=getvalue(hd); if (error) return;
	hd1=next_param(st); hd2=next_param(hd1); hdmult=next_param(hd2);
	hd1=getvalue(hd1); hd2=getvalue(hd2); hdmult=getvalue(hdmult);
	if (error) return;
	if (hd->type!=s_matrix || hd1->type!=s_matrix || hd2->type!=s_matrix)
	{	output("Illegal parameter for solid!\n");
		error=83; return;
	}
	// get matrices
	getmatrix(hd,&rx,&cx,&mx);
	getmatrix(hd1,&ry,&cy,&my);
	getmatrix(hd2,&rz,&cz,&mz);
	// get maximum size
	r=rx; if (ry>r) r=ry; if (rz>r) r=rz;
	c=cx; if (cy>c) c=cy; if (cz>c) r=rz;
	if (r<2 || c<2)
	{	output("Illegal parameter for solid!\n");
		error=83; return;
	}
	col=c;
	if (hdmult->type!=s_real &&
		(hdmult->type!=s_matrix || dimsof(hdmult)->r!=1
			|| dimsof(hdmult)->c<1)
		)
	{	output("4th parameter for solid must be a real vector!\n");
		error=83; return;
	}
	getmatrix(hdmult,&multr,&multc,&mult);
	multn=0; multi=(int)(*mult)-1;
	size=(LONG)c*r*sizeof(double);
	n=(LONG)(r-1)*(c-1);
	screen_col=(double *)newram;
	screen_row=(double *)(newram+size);
	
	if (!freeram(2*size+(sizeof(recttyp)+sizeof(recttyp *))*n))
	{	
		output("Out of memory in solid!\n");
		error=85; return;
	}	
	
	rectp=(recttyp *)(newram+2*size);
	rectpp=rectps=(recttyp **)(newram+2*size+n*sizeof(recttyp));
	
	cos_left=cos(a_left); sin_left=sin(a_left);
	cos_up=cos(a_up); sin_up=sin(a_up);
	cos_d=cos(anaglyph_d); sin_d=(anaglyph_d);

	for (i=0; i<r; i++)
	{	
		if (multi==i)
		{	mult++; multn++;
			if (multn>=multc) multi=-1;
			else multi=(int)(*mult)-1;
			n-=c-1;
			norectp=1;
		}
		for (j=0; j<c; j++)
		{	
			z=project(
				*mat(mx,cx,(rx==1)?0:i,(cx==1)?0:j),
				*mat(my,cy,(ry==1)?0:i,(cy==1)?0:j),
				*mat(mz,cz,(rz==1)?0:i,(cz==1)?0:j),
				&screen_col[col*i+j],&screen_row[col*i+j]);
			if (i<r-1 && j<c-1 && !norectp)
			{	rectp->i=i; rectp->j=j; rectp->z=z;
				*rectpp++=rectp; rectp++;
			}
			if (test_key()==escape)
			{   output("User interrupted!\n");
				error=1; return;
			}
		}
		norectp=0;
	}
	qsort(rectps,n,sizeof(recttyp *),
		(int (*)(const void *,const void *))compare);
	if (!holding) gclear();
	graphic_mode();
	rectpp=rectps;
	for (ind=0; ind<n; ind++)
	{	
		i=(*rectpp)->i; j=(*rectpp)->j;
		cc[0]=screen_col[col*i+j]; cc[1]=screen_row[col*i+j];
		cc[2]=screen_col[col*(i+1)+j]; cc[3]=screen_row[col*(i+1)+j];
		cc[4]=screen_col[col*(i+1)+j+1]; cc[5]=screen_row[col*(i+1)+(j+1)];
		cc[6]=screen_col[col*i+j+1]; cc[7]=screen_row[col*i+j+1];
		if (!twosides ||
			((cc[2]-cc[0])*(cc[5]-cc[1])-(cc[3]-cc[1])*(cc[4]-cc[0]))>=0)
			gfill(cc,fillcolor2,wirecolor,4,connected);
		else gfill(cc,fillcolor1,wirecolor,4,connected);
		if (test_key()==escape)
		{	output("User interrupted!\n");
			error=1; gflush(); return;
		}
		rectpp++;
	}
	hd=new_real(0.0,"");
	moveresult(st,hd);
	gflush();
}

void hxproject (double c0, double r0, double c1, double r1, 
	double k, double *cc, double *cr)
{
	*cc=c0+k*(c1-c0);
	*cr=r0+k*(r1-r0);
}

void hxcontour1 (
	double w0, double c0, double r0,
	double w1, double c1, double r1,
	double w2, double c2, double r2,
	double h1, double h2)
	// now x1 <= x2 <= x3
{
	double c[10];

	if (w2<=h2)
	{
		if (w0>=h1) // h1 <= w0 <= w1 <= w2 <= h2
		{
			c[0]=c0; c[1]=r0;
			c[2]=c1; c[3]=r1;
			c[4]=c2; c[5]=r2;
			gpolygon(c,3,bartype,contourcolor,0);
		}
		else if (w1>h1) // w0 < h1 < w1 <= w2 <= h2
		{
			double k1=xbary(h1,w0,w1);
			double k2=xbary(h1,w0,w2);
			hxproject(c0,r0,c1,r1,k1,c,c+1);
			hxproject(c0,r0,c2,r2,k2,c+2,c+3);
			c[4]=c2; c[5]=r2;
			c[6]=c1; c[7]=r1;
			gpolygon(c,4,bartype,contourcolor,0);
		}
		else if (w2>h1) // w0 <= w1 <= h1 < w2 <= h2
		{
			double k1=xbary(h1,w1,w2);
			double k2=xbary(h1,w0,w2);
			hxproject(c1,r1,c2,r2,k1,c,c+1);
			hxproject(c0,r0,c2,r2,k2,c+2,c+3);
			c[4]=c2; c[5]=r2;
			gpolygon(c,3,bartype,contourcolor,0);
		}
	}
	else if (w1<=h2)
	{
		if (w0>=h1) // h1 <= w0 <= w1 <= h2 < w2
		{
			double k1=xbary(h2,w0,w2);
			double k2=xbary(h2,w1,w2);
			hxproject(c0,r0,c2,r2,k1,c,c+1);
			hxproject(c1,r1,c2,r2,k2,c+2,c+3);
			c[4]=c1; c[5]=r1; 
			c[6]=c0; c[7]=r0;
			gpolygon(c,4,bartype,contourcolor,0);
		}
		else if (w1>h1) // w0 < h1 < w1 <= h2 < w2
		{
			double k1=xbary(h2,w0,w2);
			double k2=xbary(h2,w1,w2);
			hxproject(c0,r0,c2,r2,k1,c,c+1);
			hxproject(c1,r1,c2,r2,k2,c+2,c+3);
			c[4]=c1; c[5]=r1;
			double k3=xbary(h1,w0,w1);
			double k4=xbary(h1,w0,w2);
			hxproject(c0,r0,c1,r1,k3,c+6,c+7);
			hxproject(c0,r0,c2,r2,k4,c+8,c+9);
			gpolygon(c,5,bartype,contourcolor,0);
		}
		else // w0 <= w1 <= h1 < h2 < w2
		{
			double k1=xbary(h2,w0,w2);
			double k2=xbary(h2,w1,w2);
			hxproject(c0,r0,c2,r2,k1,c,c+1);
			hxproject(c1,r1,c2,r2,k2,c+2,c+3);
			double k3=xbary(h1,w1,w2);
			double k4=xbary(h1,w0,w2);
			hxproject(c1,r1,c2,r2,k3,c+4,c+5);
			hxproject(c0,r0,c2,r2,k4,c+6,c+7);
			gpolygon(c,4,bartype,contourcolor,0);
		}
	}
	else if (w0<h2)
	{
		if (w0>=h1) // h1 <= w0 < h2 < w1 <= w2
		{
			double k1=xbary(h2,w0,w1);
			double k2=xbary(h2,w0,w2);
			hxproject(c0,r0,c1,r1,k1,c,c+1);
			hxproject(c0,r0,c2,r2,k2,c+2,c+3);
			c[4]=c0; c[5]=r0;
			gpolygon(c,3,bartype,contourcolor,0);
		}
		else // w0 < h1 < h2 < w1 <= w2
		{
			double k1=xbary(h2,w0,w1);
			double k2=xbary(h2,w0,w2);
			hxproject(c0,r0,c1,r1,k1,c,c+1);
			hxproject(c0,r0,c2,r2,k2,c+2,c+3);
			double k3=xbary(h1,w0,w2);
			double k4=xbary(h1,w0,w1);
			hxproject(c0,r0,c2,r2,k3,c+4,c+5);
			hxproject(c0,r0,c1,r1,k4,c+6,c+7);
			gpolygon(c,3,bartype,contourcolor,0);
		}
	}
}

void hxcontour (
	double x1, double c1, double r1,
	double x2, double c2, double r2,
	double x3, double c3, double r3,
	double v0, double v1)
{
	if (x2<x1)
	{
		if (x3<x2) hxcontour1(x3,c3,r3,x2,c2,r2,x1,c1,r1,v0,v1);
		else if (x3<x1) hxcontour1(x2,c2,r2,x3,c3,r3,x1,c1,r1,v0,v1);
		else hxcontour1(x2,c2,r2,x1,c1,r1,x3,c3,r3,v0,v1);
	}
	else
	{
		if (x3<x1) hxcontour1(x3,c3,r3,x1,c1,r1,x2,c2,r2,v0,v1);
		else if (x3<x2) hxcontour1(x1,c1,r1,x3,c3,r3,x2,c2,r2,v0,v1);
		else hxcontour1(x1,c1,r1,x2,c2,r2,x3,c3,r3,v0,v1);
	}
}
	
void xcontour (double x[], int i, int j, int rows, int cols, 
	double v[], int nv)
/***** contour
	x1 is lower left edge, x2 upper left, x3 upper right, x4 lower
	right value at a square. 
	does contour plot of the nv values in v.
	r and c is needed to compute the position of the square.
*****/
{	
	int k;
	double sr[5],sc[5];
	double mr=(lowerr+upperr)/2,dr=(upperr-lowerr)/2/(1+margin);
	double lr=mr-dr,ur=mr+dr;
	double mc=(lowerc+upperc)/2,dc=(upperc-lowerc)/2/(1+margin);
	double lc=mc-dc,uc=mc+dc;
	sr[4]=sr[0]=sr[3]=(lr-((LONG)i*(lr-ur))/cols);
	sr[1]=sr[2]=(lr-((LONG)(i+1)*(lr-ur))/cols);
	double srm=(sr[0]+sr[1])/2; 
	sc[4]=sc[0]=sc[1]=(uc+((LONG)j*(lc-uc))/rows);
	sc[2]=sc[3]=(uc+((LONG)(j+1)*(lc-uc))/rows);
	double scm=(sc[0]+sc[2])/2; 
	double xm=0;
	for (i=0; i<4; i++) xm+=x[i]; 
	xm/=4.0;
	for (k=0; k<nv; k++)
	{	
		for (i=0; i<4; i++)
			hxcontour(x[i],sc[i],sr[i],x[i+1],sc[i+1],sr[i+1],xm,scm,srm,v[k],v[k+nv]);
	}
}

void hcontour (double val, int n, int m, 
	double x[], double r[], double c[])
/**** hcontour
	helping function to contour.
****/
{	double f1,f2;
	if ((val>=x[n] && val<=x[n+1]) || (val>=x[n+1] && val<=x[n]))
	if ((val>=x[m] && val<=x[m+1]) || (val>=x[m+1] && val<=x[m]))
	{	
		if (x[n+1]==x[n]) f1=0;
		else f1=(val-x[n])/(x[n+1]-x[n]);
		if (x[m+1]==x[m]) f2=0;
		else f2=(val-x[m])/(x[m+1]-x[m]);
		gline((c[n]+f1*(c[n+1]-c[n])),
			(r[n]+f1*(r[n+1]-r[n])),
			(c[m]+f2*(c[m+1]-c[m])),
			(r[m]+f2*(r[m+1]-r[m])),
			contourcolor,line_solid,linewidth);
	}
}

void contour (double x[], int i, int j, int rows, int cols, 
	double v[], int nv)
/***** contour
	x1 is lower left edge, x2 upper left, x3 upper right, x4 lower
	right value at a square. 
	does contour plot of the nv values in v.
	r and c is needed to compute the position of the square.
*****/
{	
	int k,n,m;
	double sr[5],sc[5];
	double val;
	double mr=(lowerr+upperr)/2,dr=(upperr-lowerr)/2/(1+margin);
	double lr=mr-dr,ur=mr+dr;
	double mc=(lowerc+upperc)/2,dc=(upperc-lowerc)/2/(1+margin);
	double lc=mc-dc,uc=mc+dc;
	sr[4]=sr[0]=sr[3]=(lr-((LONG)i*(lr-ur))/cols);
	sr[1]=sr[2]=(lr-((LONG)(i+1)*(lr-ur))/cols);
	sc[4]=sc[0]=sc[1]=(uc+((LONG)j*(lc-uc))/rows);
	sc[2]=sc[3]=(uc+((LONG)(j+1)*(lc-uc))/rows);
	for (k=0; k<nv; k++)
	{	
		val=v[k];
		for (n=0; n<3; n++)
			for (m=n+1; m<4; m++)
				hcontour(val,n,m,x,sr,sc);
	}
}

void mcontour (header *hd)
/***** mcontour
	contour plot with matrix and vector intput.
*****/
{	header *st=hd,*result,*hd1;
	double *m,*mv,x[5];
	int r,c,rv,cv,i,j;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix || dimsof(hd)->c<2 || dimsof(hd)->r<2)
	{	output("Contour needs a real matrix!\n"); error=81; return;
	}
	hd1=next_param(st); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	if (hd1->type!=s_real)
		if (hd1->type!=s_matrix || (dimsof(hd1)->r!=1 && dimsof(hd1)->r!=2))
		{	output("Second parameter of contour must be a vector!\n");
			error=82; return;
		}
	getmatrix(hd,&r,&c,&m); getmatrix(hd1,&rv,&cv,&mv);
	if (!holding) gclear();
	graphic_mode();
	// frame();
	for (i=0; i<r-1; i++)
	{	for (j=0; j<c-1; j++)
		{	
			x[0]=*mat(m,c,i,j); x[1]=*mat(m,c,i+1,j);
			x[2]=*mat(m,c,i+1,j+1); x[3]=*mat(m,c,i,j+1);
			x[4]=x[0];
			if (rv==1) contour(x,i,j,c-1,r-1,mv,cv);
			else xcontour(x,i,j,c-1,r-1,mv,cv);
		}
		if (test_key()==escape)
		{	output("User interrupted!\n");
			error=1; gflush(); return;
		}
	}
	result=new_real(cv,"");
	moveresult(st,result);
	gflush();
}

void mdensity (header *hd)
/***** mcontour
	density plot with matrix input.
*****/
{	header *st=hd,*result;
	double *m,x,deltax,deltay;
	int r,c,i,j;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix || dimsof(hd)->c<2 || dimsof(hd)->r<2)
	{	output("Density needs a real matrix!\n"); error=81; return;
	}
	getmatrix(hd,&r,&c,&m);
	if (!holding) gclear();
	graphic_mode();
	double mr=(lowerr+upperr)/2,dr=(upperr-lowerr)/2/(1+margin);
	double lr=mr-dr,ur=mr+dr;
	double mc=(lowerc+upperc)/2,dc=(upperc-lowerc)/2/(1+margin);
	double lc=mc-dc,uc=mc+dc;
	if (antialiasing) 
	{
		deltax=(double)(lc-uc)/(c-1);
		deltay=(double)(lr-ur)/(r-1);
		for (i=0; i<r-1; i++)
		{	for (j=0; j<c-1; j++)
			{
				x=((*mat(m,c,i,j))+(*mat(m,c,i+1,j))+(*mat(m,c,i+1,j+1))+(*mat(m,c,i,j+1)))/4;
				if (!isnan(x))
					gbar(
					uc+j*deltax,lr-(i+1)*deltay,
					uc+(j+1)*deltax,lr-i*deltay,
					x,densitycolor,dgrid);
			}
			if (test_key()==escape)
			{   output("User interrupted!\n");
				error=1; gflush(); return;
			}
		}
	}
	else 
	{
		deltax=(double)(lc-uc)/(c);
		deltay=(double)(lr-ur)/(r);
		for (i=0; i<r; i++)
		{	for (j=0; j<c; j++)
			{
				if (!isnan(*mat(m,c,i,j)))
					gbar(
					uc+j*deltax,lr-(i+1)*deltay,
					uc+(j+1)*deltax,lr-i*deltay,
					*mat(m,c,i,j),densitycolor,dgrid);
			}
			if (test_key()==escape)
			{   output("User interrupted!\n");
				error=1; gflush(); return;
			}
		}
	}
	result=new_real(0,"");
	moveresult(st,result);
	gflush();
}

void mview (header *hd)
{	double *m;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix || dimsof(hd)->r!=1 || dimsof(hd)->c!=4)
	{	output("Arguments for view are [dist tele alpha beta]!\n");
		error=90; return;
	}
	m=matrixof(hd);
	distance=*m++;
	tele=*m++;
	a_left=*m++;
	a_up=*m;
}

void mwindow (header *hd)
{	double *m;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix || dimsof(hd)->r!=1 || dimsof(hd)->c!=4)
	{	output("Arguments for window are [c0 r0 c1 r1]!\n");
		error=90; return;
	}
	m=matrixof(hd);
	upperc=(int)(*m++);
	upperr=(int)(*m++);
	lowerc=(int)(*m++);
	lowerr=(int)(*m);
	if (lowerr<upperr) lowerr=upperr+1;
	if (lowerc<upperc) lowerc=upperc+1;
	scaling=1;
}

void mclip (header *hd)
{	double *m;
	header *stack=hd, *oldclip;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix || dimsof(hd)->r!=1 || dimsof(hd)->c!=4)
	{	output("Arguments for clip are [c0 r0 c1 r1]!\n");
		error=90; return;
	}
	oldclip = new_matrix(1,4,""); if (error) return;
	m=matrixof(oldclip);
	*m++=upperclipc;
	*m++=upperclipr;
	*m++=lowerclipc;
	*m=lowerclipr;
	m=matrixof(hd);
	upperclipc=(int)(*m++);
	upperclipr=(int)(*m++);
	lowerclipc=(int)(*m++);
	lowerclipr=(int)(*m);
	if (upperclipc<0) upperclipc = 0;
	if (upperclipr<0) upperclipr = 0;
	if (lowerclipc>1023) lowerclipc = 1023;
	if (lowerclipc>1023) lowerclipc = 1023;
	if (lowerclipr<upperclipr) lowerclipr=upperclipr+1;
	if (lowerclipc<upperclipc) lowerclipc=upperclipc+1;
	gclip(upperclipc,upperclipr,lowerclipc,lowerclipr);
	moveresult(stack,oldclip);
}

void mclip0 (header *hd)
{	double *m;
	hd=new_matrix(1,4,""); if (error) return;
	m=matrixof(hd);
	*m++=upperclipc;
	*m++=upperclipr;
	*m++=lowerclipc;
	*m=lowerclipr;
}

void mtoscreen (header *hd)
{	header *stack=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix || dimsof(hd)->r!=1 || dimsof(hd)->c!=2)
	{	output("Arguments for fromscreen are [c r]!\n");
		error=90; return;
	}
	double *m=matrixof(hd);
	result=new_matrix(1,2,"");
	double *mr=matrixof(result);
	*mr++=scrcol(*m++);
	*mr++=scrrow(*m++);
	moveresult(stack,result);
}

void mfromscreen (header *hd)
{	header *stack=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix || dimsof(hd)->r!=1 || dimsof(hd)->c!=2)
	{	output("Arguments for fromscreen are [c r]!\n");
		error=90; return;
	}
	double *m=matrixof(hd);
	result=new_matrix(1,2,"");
	double *mr=matrixof(result);
	*mr++=invscrcol(*m++);
	*mr++=invscrrow(*m++);
	moveresult(stack,result);
}

void mpswindow (header *hd)
{	double *m;
	double c,r;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix || dimsof(hd)->r!=1 || dimsof(hd)->c!=2)
	{	output("Arguments for pswindow are [c r]!\n");
		error=90; return;
	}
	m=matrixof(hd);
	c=*m++;
	r=*m;
	if (c<1) c=1;
	if (r<1) r=1;
    pswindow(c,r);
}

void mcolor (header *hd)
{   
	header *st=hd,*res;
	hd=getvalue(hd); if (error) return;
	if (uselinecolors)
	{
		res=new_matrix(1,uselinecolors,"");
		double *m=matrixof(res);
		for (int i=0; i<uselinecolors; i++) *m++=linecolors[i];
	}
	else
	{
		res=new_real(linecolor,"");
	}
	if (!isnone(hd))
	{
		if (hd->type==s_real)
		{
			linecolor=(int)*realof(hd);
			uselinecolors=0;
		}
		else if (hd->type==s_matrix)
		{
			double *m;
			int r,c;
			getmatrix(hd,&r,&c,&m);
			int n=max(r,c);
			if (n>MAXLINECOLORS) n=MAXLINECOLORS;
			if (n>0)
			{
				for (int i=0; i<n; i++) linecolors[i]=(int)(*m++);
				linecolor=linecolors[0];
				uselinecolors=n;
			}
		}
		else
		{	output("Argument for color must be real!\n");
			error=90; return;
		}
	}
	moveresult(st,res);
}

void mcolor0 (header *hd)
{   new_real(linecolor,"");
}

void mcontourcolor (header *hd)
{   header *st=hd;
	int old=contourcolor;
	hd=getvalue(hd); if (error) return;
	if (!isnone(hd))
	{
		if (hd->type!=s_real)
		{	output("Argument for color must be real!\n");
			error=90; return;
		}
		contourcolor=(int)*realof(hd);
	}
	moveresult(st,new_real(old,""));
}

void mcontourcolor0 (header *hd)
{   new_real(contourcolor,"");
}

void mresetcolors (header *hd)
{	resetcolors();
	new_real(0,"");
}

void msetcolor (header *hd)
{   header *st=hd,*hdr,*hdg,*hdb;
	hdr=next_param(st);
	hdg=next_param(hdr);
	hdb=next_param(hdg);
	hd=getvalue(hd);
	hdr=getvalue(hdr);
	hdg=getvalue(hdg);
	hdb=getvalue(hdb);
	if (error) return;
	if (hd->type!=s_real || hdr->type!=s_real || 
			hdg->type!=s_real || hdb->type!=s_real)
	{	output("Argument for setcolor must be real!\n");
		error=90; return;
	}
	int i=(int)*realof(hd);
	int ret=setcolor(i,*realof(hdr),*realof(hdg),*realof(hdb));
	if (ret)
	{	output("Argument error for setcolor!\n");
		error=90; return;
	}
	moveresult(st,new_real(i,""));
}

void mfcolor (header *hd)
{   header *st=hd;
	int old=framecolor;
	hd=getvalue(hd); if (error) return;
	if (!isnone(hd))
	{
		if (hd->type!=s_real)
		{	output("Argument for framecolor must be real!\n");
			error=90; return;
		}
		framecolor=(int)*realof(hd);
	}
	moveresult(st,new_real(old,""));
}

void mf3dcolor0 (header *hd)
{  new_real(frame3dcolor,"");
}

void mf3dcolor (header *hd)
{   header *st=hd;
	int old=frame3dcolor;
	hd=getvalue(hd); if (error) return;
	if (!isnone(hd))
	{
		if (hd->type!=s_real)
		{	output("Argument for frame3dcolor must be real!\n");
			error=90; return;
		}
		frame3dcolor=(int)*realof(hd);
	}
	moveresult(st,new_real(old,""));
}

void mfcolor0 (header *hd)
{  new_real(framecolor,"");
}

extern int fillcolor1,fillcolor2;

void mfillcolor (header *hd)
{   
	header *st=hd,*hd1,*res;
	int o1=fillcolor1,o2=fillcolor2;
	hd1=next_param(hd);
	hd=getvalue(hd); 
	if (hd1) hd1=getvalue(hd1);
	if (error) return;
	if (hd1 && hd->type==s_real && hd1->type==s_real)
	{
		fillcolor1=(int)*realof(hd);
		fillcolor2=(int)*realof(hd1);
	}
	else if (hd->type==s_matrix || dimsof(hd)->r==1 || dimsof(hd)->c==2)
	{
		fillcolor1=(int)*matrixof(hd);
		fillcolor2=(int)*(matrixof(hd)+1);
	}
	else if (!isnone(hd))
	{	
		output("Argument for fillcolor must none, a 1x2 vector or two reals!\n");
		error=90; return;
	}
	res=new_matrix(1,2,""); if (error) return;
	*matrixof(res)=o1; *(matrixof(res)+1)=o2;
	moveresult(st,res);
}

void mfillcolor0 (header *hd)
{   
	header *res=new_matrix(1,2,""); if (error) return;
	*matrixof(res)=fillcolor1; *(matrixof(res)+1)=fillcolor2;
}

extern int markerfactor;

void mmarkersize (header *hd)
{   
	header *st=hd;
	int old,markersize;
	old=1024/markerfactor;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real)
	{	output("Argument for markersize must be integer!\n");
		error=90; return;
	}
	markersize=(int)*realof(hd);
	if (markersize<1) markersize=1;
	moveresult(st,new_real(old,""));
	markerfactor=1024/markersize;
}

void mmarkersize0 (header *hd)
{   
	new_real(1024/markerfactor,"");
}

void mwcolor (header *hd)
{   header *st=hd;
	int old=wirecolor;
	hd=getvalue(hd); if (error) return;
	if (!isnone(hd))
	{
		if (hd->type!=s_real)
		{	output("Argument for wirecolor must be integer!\n");
			error=90; return;
		}
		wirecolor=(int)*realof(hd);
	}
	moveresult(st,new_real(old,""));
}

void mwcolor0 (header *hd)
{   new_real(wirecolor,"");
}

void mtcolor (header *hd)
{   header *st=hd;
	int old=textcolor;
	hd=getvalue(hd); if (error) return;
	if (!isnone(hd))
	{
		if (hd->type!=s_real)
		{	output("Argument for textcolor must be integer!\n");
			error=90; return;
		}
		textcolor=(int)*realof(hd);
	}
	moveresult(st,new_real(old,""));
}

void mtcolor0 (header *hd)
{   
	new_real(textcolor,"");
}

void mdcolor (header *hd)
{   
	header *st=hd;
	int old=densitycolor;
	hd=getvalue(hd); if (error) return;
	if (!isnone(hd))
	{
		if (hd->type!=s_real)
		{	output("Argument for densitycolor must be integer!\n");
			error=90; return;
		}
		densitycolor=(int)*realof(hd);
	}
	moveresult(st,new_real(old,""));
}

void mdgrid (header *hd)
{   
	header *st=hd;
	int old=dgrid;
	hd=getvalue(hd); if (error) return;
	if (!isnone(hd) && hd->type!=s_real)
	{	output("Argument for huegrid must be integer!\n");
		error=90; return;
	}
	if (!isnone(hd)) dgrid=(int)*realof(hd);
	moveresult(st,new_real(old,""));
}

void mstyle (header *hd)
{	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string)
	{	output("Argument style must be a string!\n");
		error=90; return;
	}
	if (!strcmp(stringof(hd),"i")) linetype=line_none;
	else if (!strcmp(stringof(hd),"-")) linetype=line_solid;
	else if (!strcmp(stringof(hd),".")) linetype=line_dotted;
	else if (!strcmp(stringof(hd),"--")) linetype=line_dashed;
	else if (!strcmp(stringof(hd),"-.")) linetype=line_dashdot;
	else if (!strcmp(stringof(hd),".-")) linetype=line_dashdot;
	else if (!strcmp(stringof(hd),"-.-")) linetype=line_dashdotdash;
	else if (!strcmp(stringof(hd),".-.")) linetype=line_dotdashdot;
	else if (!strcmp(stringof(hd),"->")) linetype=line_arrow;
	else if (!strcmp(stringof(hd),"mx")) markertype=marker_cross;
	else if (!strcmp(stringof(hd),"mo")) markertype=marker_circle;
	else if (!strcmp(stringof(hd),"mow")) markertype=marker_circle_hollow;
	else if (!strcmp(stringof(hd),"mo#")) markertype=marker_filledcircle;
	else if (!strcmp(stringof(hd),"m<>")) markertype=marker_diamond;
	else if (!strcmp(stringof(hd),"m<>w")) markertype=marker_diamond_hollow;
	else if (!strcmp(stringof(hd),"m<>#")) markertype=marker_filleddiamond;
	else if (!strcmp(stringof(hd),"m.")) markertype=marker_dot;
	else if (!strcmp(stringof(hd),"m..")) markertype=marker_largedot;
	else if (!strcmp(stringof(hd),"m..#")) markertype=marker_filledlargedot;
	else if (!strcmp(stringof(hd),"m...")) markertype=marker_filledlargedot;
	else if (!strcmp(stringof(hd),"m+")) markertype=marker_plus;
	else if (!strcmp(stringof(hd),"m|")) markertype=marker_vertical;
	else if (!strcmp(stringof(hd),"m-")) markertype=marker_minus;
	else if (!strcmp(stringof(hd),"m[]")) markertype=marker_square;
	else if (!strcmp(stringof(hd),"m[]w")) markertype=marker_square_hollow;
	else if (!strcmp(stringof(hd),"m[]#")) markertype=marker_filledsquare;
	else if (!strcmp(stringof(hd),"m*")) markertype=marker_star;
	else if (!strcmp(stringof(hd),"b/")) bartype=bar_diagonal1;
	else if (!strcmp(stringof(hd),"b\\")) bartype=bar_diagonal2;
	else if (!strcmp(stringof(hd),"bO")) bartype=bar_frame;
	else if (!strcmp(stringof(hd),"b#")) bartype=bar_solid;
	else if (!strcmp(stringof(hd),"bO#")) bartype=bar_framed;
	else if (!strcmp(stringof(hd),"b#O")) bartype=bar_framed;
	else if (!strcmp(stringof(hd),"b|")) bartype=bar_vhatch;
	else if (!strcmp(stringof(hd),"b-")) bartype=bar_hhatch;
	else if (!strcmp(stringof(hd),"b\\/")) bartype=bar_cross;
	else if (!strcmp(stringof(hd),"b/\\")) bartype=bar_cross;
	else if (!strcmp(stringof(hd),"b+")) bartype=bar_plus;
	else if (!strcmp(stringof(hd),"bt")) bartype=bar_transparent;
	else if (!isnone(hd))
	{
		markertype=marker_square_hollow; linetype=line_solid;
		bartype=bar_framed;
	}
}

void mmstyle (header *hd)
{	
	header *st=hd,*res;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string)
	{	
		output("Argument for markerstyle must be a string!\n");
		error=90; return;
	}
	switch (markertype)
	{	case marker_cross : res=new_string("x",8,""); break;
		case marker_circle : res=new_string("o",8,""); break;
		case marker_circle_hollow : res=new_string("ow",8,""); break;
		case marker_filledcircle : res=new_string("o#",8,""); break;
		case marker_diamond : res=new_string("<>",8,""); break;
		case marker_diamond_hollow : res=new_string("<>w",8,""); break;
		case marker_filleddiamond : res=new_string("<>#",8,""); break;
		case marker_dot : res=new_string(".",8,""); break;
		case marker_largedot : res=new_string("..",8,""); break;
		case marker_filledlargedot : res=new_string("...",8,""); break;
		case marker_plus : res=new_string("+",8,""); break;
		case marker_square : res=new_string("[]",8,""); break;
		case marker_square_hollow : res=new_string("[]w",8,""); break;
		case marker_vertical : res=new_string("|",8,""); break;
		case marker_minus : res=new_string("-",8,""); break;
		case marker_filledsquare : res=new_string("[]#",8,""); break;
		case marker_star : res=new_string("*",8,""); break;
		default : res=new_string("",8,"");
	}
	if (!strcmp(stringof(hd),"x")) markertype=marker_cross;
	else if (!strcmp(stringof(hd),"o")) markertype=marker_circle;
	else if (!strcmp(stringof(hd),"ow")) markertype=marker_circle_hollow;
	else if (!strcmp(stringof(hd),"o#")) markertype=marker_filledcircle;
	else if (!strcmp(stringof(hd),"<>")) markertype=marker_diamond;
	else if (!strcmp(stringof(hd),"<>w")) markertype=marker_diamond_hollow;
	else if (!strcmp(stringof(hd),"<>#")) markertype=marker_filleddiamond;
	else if (!strcmp(stringof(hd),".")) markertype=marker_dot;
	else if (!strcmp(stringof(hd),"..")) markertype=marker_largedot;
	else if (!strcmp(stringof(hd),"..#")) markertype=marker_filledlargedot;
	else if (!strcmp(stringof(hd),"...")) markertype=marker_filledlargedot;
	else if (!strcmp(stringof(hd),"+")) markertype=marker_plus;
	else if (!strcmp(stringof(hd),"[]")) markertype=marker_square;
	else if (!strcmp(stringof(hd),"[]w")) markertype=marker_square_hollow;
	else if (!strcmp(stringof(hd),"|")) markertype=marker_vertical;
	else if (!strcmp(stringof(hd),"-")) markertype=marker_minus;
	else if (!strcmp(stringof(hd),"[]#")) markertype=marker_filledsquare;
	else if (!strcmp(stringof(hd),"*")) markertype=marker_star;
	moveresult(st,res);
}

void mbarstyle (header *hd)
{	
	header *st=hd,*res;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string)
	{	output("Argument for barstyle must be a string!\n");
		error=90; return;
	}
	switch (bartype)
	{	
		case bar_solid : res=new_string("#",8,""); break;
		case bar_framed : res=new_string("O#",8,""); break;
		case bar_frame : res=new_string("O",8,""); break;
		case bar_vhatch : res=new_string("|",8,""); break;
		case bar_hhatch : res=new_string("-",8,""); break;
		case bar_plus : res=new_string("+",8,""); break;
		case bar_diagonal1 : res=new_string("/",8,""); break;
		case bar_diagonal2 : res=new_string("\\",8,""); break;
		case bar_cross : res=new_string("\\/",8,""); break;
		case bar_transparent : res=new_string("t",8,""); break;
		default : res=new_string("",8,"");
	}
	if (!strcmp(stringof(hd),"/")) bartype=bar_diagonal1;
	else if (!strcmp(stringof(hd),"\\")) bartype=bar_diagonal2;
	else if (!strcmp(stringof(hd),"O")) bartype=bar_frame;
	else if (!strcmp(stringof(hd),"#")) bartype=bar_solid;
	else if (!strcmp(stringof(hd),"O#")) bartype=bar_framed;
	else if (!strcmp(stringof(hd),"#O")) bartype=bar_framed;
	else if (!strcmp(stringof(hd),"|")) bartype=bar_vhatch;
	else if (!strcmp(stringof(hd),"-")) bartype=bar_hhatch;
	else if (!strcmp(stringof(hd),"+")) bartype=bar_plus;
	else if (!strcmp(stringof(hd),"\\/")) bartype=bar_cross;
	else if (!strcmp(stringof(hd),"/\\")) bartype=bar_cross;
	else if (!strcmp(stringof(hd),"t")) bartype=bar_transparent;
	moveresult(st,res);
}

void mlstyle (header *hd)
{	
	header *st=hd,*res;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string)
	{	output("Argument for linestyle must be string!\n");
		error=90; return;
	}
	switch (linetype)
	{	case line_none : res=new_string("i",8,""); break;
		case line_solid : res=new_string("-",8,""); break;
		case line_dotted : res=new_string(".",8,""); break;
		case line_dashed : res=new_string("--",8,""); break;
		case line_dashdot : res=new_string("-.",8,""); break;
		case line_arrow : res=new_string("->",8,""); break;
		case line_dashdotdash : res=new_string("-.-",8,""); break;
		case line_dotdashdot : res=new_string(".-.",8,""); break;
		default : res=new_string("",8,"");
	}
	if (!strcmp(stringof(hd),"i")) linetype=line_none;
	else if (!strcmp(stringof(hd),"-")) linetype=line_solid;
	else if (!strcmp(stringof(hd),".")) linetype=line_dotted;
	else if (!strcmp(stringof(hd),"--")) linetype=line_dashed;
	else if (!strcmp(stringof(hd),"-.")) linetype=line_dashdot;
	else if (!strcmp(stringof(hd),".-")) linetype=line_dashdot;
	else if (!strcmp(stringof(hd),".-.")) linetype=line_dotdashdot;
	else if (!strcmp(stringof(hd),"-.-")) linetype=line_dashdotdash;
	else if (!strcmp(stringof(hd),"->")) linetype=line_arrow;
	moveresult(st,res);
}

void mlinew (header *hd)
{	
	header *st=hd,*res;
	double old=linewidth;
	hd=getvalue(hd); if (error) return;
	if (!isnone(hd))
	{
		if (hd->type!=s_real)
		{	output("Argument for linewidth must be a real!");
			error=90; return;
		}
		linewidth=fabs(*realof(hd));
	}
	res=new_real(old,"");
	moveresult(st,res);
}

char *addbom (header *hd)
{
	if (hd->flags&UTF)
	{
		char *s=newram;
		*s++=239-256; *s++=187-256; *s++=191-256;
		strcpy(s,stringof(hd));
		return newram;
	}
	else return stringof(hd);
}

void mtext1 (header *hd, int flag)
{	
	header *hd1,*hd2;
	hd1=next_param(hd);
	hd2=next_param(hd1);
	hd=getvalue(hd); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	if (hd2)  hd2=getvalue(hd2); if (error) return;
	if (hd->type!=s_string || (hd1->type==s_matrix &&
		dimsof(hd1)->r!=1 && dimsof(hd1)->c!=2) ||
		(hd2 && hd2->type!=s_real && hd1->type!=s_real))
	{	output("Need a string, and a position in text!\n");
		error=91; return;
	}
	graphic_mode();
	if (hd2) gtext((int)*realof(hd1),(int)*realof(hd2),
		addbom(hd),textcolor,flag,hchar);
	else gtext((int)*matrixof(hd1),(int)*(matrixof(hd1)+1),
		addbom(hd),textcolor,flag,hchar);
	gflush();
}

void mctext (header *hd)
{   mtext1(hd,1);
}

void mrtext (header *hd)
{   mtext1(hd,2);
}

void mtext (header *hd)
{   mtext1(hd,0);
}

void mvtext1 (header *hd, int flag)
{	header *hd1,*hd2;
	hd1=next_param(hd);
	hd2=next_param(hd1);
	hd=getvalue(hd); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	if (hd2)  hd2=getvalue(hd2); if (error) return;
	if (hd->type!=s_string || (hd1->type==s_matrix &&
		dimsof(hd1)->r!=1 && dimsof(hd1)->c!=2) ||
		(hd2 && hd2->type!=s_real && hd1->type!=s_real))
	{	output("Need a string, and a position in vtext!\n");
		error=91; return;
	}
	graphic_mode();
	if (hd2) gvtext((int)*realof(hd1),(int)*realof(hd2),
		addbom(hd),textcolor,flag,hchar);
	else gvtext((int)*matrixof(hd1),(int)*(matrixof(hd1)+1),
		addbom(hd),textcolor,flag,hchar);
	gflush();
}

void mvtext (header *hd)
{	mvtext1(hd,0);
}

void mvctext (header *hd)
{	mvtext1(hd,1);
}

void mvrtext (header *hd)
{	mvtext1(hd,2);
}

void mvutext1 (header *hd, int flag)
{	header *hd1,*hd2;
	hd1=next_param(hd);
	hd2=next_param(hd1);
	hd=getvalue(hd); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	if (hd2)  hd2=getvalue(hd2); if (error) return;
	if (hd->type!=s_string || (hd1->type==s_matrix &&
		dimsof(hd1)->r!=1 && dimsof(hd1)->c!=2) ||
		(hd2 && hd2->type!=s_real && hd1->type!=s_real))
	{	output("Need a string, and a position in vutext!\n");
		error=91; return;
	}
	graphic_mode();
	if (hd2) gvutext((int)*realof(hd1),(int)*realof(hd2),
		addbom(hd),textcolor,flag,hchar);
	else gvutext((int)*matrixof(hd1),(int)*(matrixof(hd1)+1),
		addbom(hd),textcolor,flag,hchar);
	gflush();
}

void mvutext (header *hd)
{	mvutext1(hd,0);
}

void mvcutext (header *hd)
{	mvutext1(hd,1);
}

void mvrutext (header *hd)
{	mvutext1(hd,2);
}

void mbar (header *hd)
{	header *st=hd,*result;
	double *m,x,y,w,h;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix || dimsof(hd)->r!=1 || dimsof(hd)->c!=4)
	{	output("Bar needs a 1x4 vector!\n");
		error=1; return;
	}
	m=matrixof(hd);
	x=*m; y=*(m+1); w=*(m+2); h=*(m+3);
	w+=x; h+=y;
	graphic_mode();
	gbar1(x,y,w,h,barcolor,bartype);
	result=new_string("",2,"");
	moveresult(st,result);
}

void mbarcolor (header *hd)
{   header *st=hd;
	int old=barcolor;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real)
	{	output("Argument for textcolor must be integer!\n");
		error=90; return;
	}
	barcolor=(int)*realof(hd);
	moveresult(st,new_real(old,""));
}

void msetplot (header *hd)
{	header *st=hd,*result;
	double *m;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix || dimsof(hd)->r!=1 || dimsof(hd)->c!=4)
	{	output("Setplot needs a 1x4 vector!\n");
		error=2200; return;
	}
	result=new_matrix(1,4,""); if (error) return;
	m=matrixof(result);
	*m++=x_min_old; *m++=x_max_old; *m++=y_min_old; *m=y_max_old;
	m=matrixof(hd);
	x_min=*m++; x_max=*m++; y_min=*m++; y_max=*m;
	setrange(1);
	moveresult(st,result);
	scaling=0;
}

void mholding (header *hd)
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real)
	{	output("Holding needs a 1 or 0!\n");
		error=2201; return;
	}
	result=new_real(holding,"");
	holding=(*realof(hd)!=0.0); scaling=!holding;
	moveresult(st,result);
}

void mkeepsquare (header *hd)
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real)
	{	output("Keepsquare needs a 1 or 0!\n");
		error=2201; return;
	}
	result=new_real(keepsquare,"");
	keepsquare=(*realof(hd)!=0.0);
	moveresult(st,result);
}

void mscaling (header *hd)
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real)
	{	output("Scaling needs a 1 or 0!\n");
		error=2201; return;
	}
	result=new_real(scaling,"");
	scaling=(*realof(hd)!=0.0);
	moveresult(st,result);
}

void mtwosides (header *hd)
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real)
	{	output("Twosides needs a 1 or 0!\n");
		error=2201; return;
	}
	result=new_real(twosides,"");
	twosides=(*realof(hd)!=0.0);
	moveresult(st,result);
}

void mmeshfactor (header *hd)
{	
	double oldfactor=meshfactor;
	hd=getvalue(hd);
	if (!isnone(hd))
	{
		if (hd->type!=s_real)
		{	output("Meshfactor needs a real!\n"); error=150; return;
		}
		meshfactor=*realof(hd);
		if (meshfactor<0) meshfactor=0;
		if (meshfactor>1) meshfactor=1;
	}
	*realof(hd)=oldfactor;
}

void mtextsize (header *hd)
{	
	header *result;
	result=new_matrix(1,2,""); if (error) return;
	*matrixof(result)=wchar;
	*(matrixof(result)+1)=hchar;
}

void mmouse (header *hd)
{	header *result;
	double c,r;
	double *m;
	graphic_mode();
	int code=mouse(&c,&r);
	if (code==27) 
	{
		output("Interrupt with Escape key!\n");
		error=1;
	}
	if (code!=0) new_real(code,"");
	else
	{	result=new_matrix(1,2,""); if (error) return;
		m=matrixof(result);
		*m++=invscrcol(c);
		*m=invscrrow(r);
	}
}

void mmousestatus (header *hd)
{	header *result,*st=hd;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) need_arg_in("mouse","status string");
	double c,r;
	double *m;
	graphic_mode();
	int code=mouse(&c,&r,stringof(hd));
	if (code==27) 
	{
		output("Interrupt with Escape key!\n");
		error=1;
	}
	if (code!=0) result=new_real(code,"");
	else
	{	result=new_matrix(1,2,""); if (error) return;
		m=matrixof(result);
		*m++=x_min+(c-upperc)/(double)(lowerc-upperc)*(x_max-x_min);
		*m=y_max-(r-upperr)/(double)(lowerr-upperr)*(y_max-y_min);
	}
	moveresult(st,result);
}

void mmousedrag (header *hd)
{	
	header *result,*st=hd;
	char *status="";
	if (hd)
	{	hd=getvalue(hd); if (error) return;
		if (hd->type!=s_string) need_arg_in("mouse","status string");
		status=stringof(hd);
	}
	double c,r;
	double *m;
	long t;
	int code;
	graphic_mode();
	int res=mousedrag(&c,&r,&t,&code,status);
	if (code==27) 
	{
		output("Interrupt with Escape key!\n");
		error=1;
	}
	if (error) return;
	result=new_real(res,""); if (error) return;
	if (res==0) new_real(code,""); if (error) return;
	else
	{
		header *mm=new_matrix(1,2,""); if (error) return;
		m=matrixof(mm);
		*m++=x_min+(c-upperc)/(double)(lowerc-upperc)*(x_max-x_min);
		*m=y_max-(r-upperr)/(double)(lowerr-upperr)*(y_max-y_min);
	}
	new_real(t,""); if (error) return;
	if (hd) moveresult1(st,result);
}

void mmousestate (header *hd)
{
	double x,y;
	new_real(mousestate(&x,&y),"");
}

void mmousepos (header *hd)
{
	double c,r;
	header *result=new_matrix(2,1,""); if (error) return;
	mousestate(&c,&r);
	double *m=matrixof(result);
	*m++=x_min+(c-upperc)/(double)(lowerc-upperc)*(x_max-x_min);
	*m=y_max-(r-upperr)/(double)(lowerr-upperr)*(y_max-y_min);
}

void mholding0 (header *hd)
{	new_real(holding,"");
}

void mplot1 (header *hd)
{	header *result;
	double *x;
	result=new_matrix(1,4,""); if (error) return;
	x=matrixof(result);
	*x++=x_min_old; *x++=x_max_old; *x++=y_min_old; *x=y_max_old;
}

void mplot2 (header *hd)
{	header *result;
	double *x;
	result=new_matrix(1,4,""); if (error) return;
	x=matrixof(result);
	*x++=x_min; *x++=x_max; *x++=y_min; *x=y_max;
}

void mview0 (header *hd)
{	header *result;
	double *m;
	result=new_matrix(1,4,""); if (error) return;
	m=matrixof(result);
	*m++=distance;
	*m++=tele;
	*m++=a_left;
	*m=a_up;
}

void mwindow0 (header *hd)
{	double *m;
	hd=new_matrix(1,4,""); if (error) return;
	m=matrixof(hd);
	*m++=upperc;
	*m++=upperr;
	*m++=lowerc;
	*m=lowerr;
}

void mframe (header *hd)
{	graphic_mode();
	frame();
	new_real(0,"");
}

void mantialiasing (header *hd)
{   header *st=hd;
	int old=antialiasing;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real)
	{	output("Argument for antialiasing must be real!\n");
		error=90; return;
	}
	antialiasing=(int)*realof(hd);
	moveresult(st,new_real(old,""));
}

void mantialiasing0 (header *hd)
{
	new_real(antialiasing,"");
}

void mbarRGB (header *hd)
/***** mbarRGB
	plot an RGB image, scaled to the graphics window.
*****/
//
//  created by RonL Aug 2006
//

{	header *st=hd,*hd1,*hd2,*hdcol,*result;
	double *red, *green, *blue;
	int r,c,r1,c1,r2,c2;
	hd1=nextof(hd);
	hd2=nextof(hd1);
	hdcol=nextof(hd2);
	hd=getvalue(hd); 
	hd1=getvalue(hd1); 
	hd2=getvalue(hd2); 
	hdcol=getvalue(hdcol); 
	if (error) return;

	if (hd->type!=s_matrix)
	{	output("barRGB needs three real matrices of equal size and a color!\n"); error=81; return;
	}
	if (hd1->type!=s_matrix)
	{	output("barRGB needs three real matrices of equal size and a color!\n"); error=81; return;
	}
	if (hd2->type!=s_matrix)
	{	output("barRGB needs three real matrices of equal size and a color!\n"); error=81; return;
	}

	getmatrix(hd,&r,&c,&red);
	getmatrix(hd1,&r1,&c1,&green);
	getmatrix(hd2,&r2,&c2,&blue);

	if (r!=r1 || c!=c1 || r!=r2 || c!=c2)
	{	output("Matrix dimensions for barRGB must agree!\n");
		error=83; return;
	}

	if (!holding) gclear();
	graphic_mode();

	int color=(int)*realof(hdcol);
	int tred,tgreen,tblue;
	if (color==-1) tred=-1;
	else getcolor(color,&tred,&tgreen,&tblue);

	gbarRGB(upperc,upperr,lowerc,lowerr,c,r,tred,tgreen,tblue,red,green,blue);
	
	result=new_real(0,"");
	moveresult(st,result);
	gflush();
}

// Insert the current plot image into the text
// window. Parameters: number of lines to use
// and name.
void minsertimage (header *hd)
{	
	header *st=hd,*hd1,*hd2,*hd3,*result;
	hd1=next_param(hd);
	hd2=next_param(hd1);
	hd3=next_param(hd2);
	hd=getvalue(hd); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	hd2=getvalue(hd2); if (error) return;
	hd3=getvalue(hd3); if (error) return;
	if (hd->type!=s_real || hd1->type!=s_string || hd2->type!=s_real 
		|| hd3->type!=s_matrix || dimsof(hd3)->r!=1 || dimsof(hd3)->c!=4)
	{	output("insertimage needs a number, a string, a flag, and crop limits (1x4 vector)!\n"); error=1; return;
	}
	int n=(int)*realof(hd);
	if (n<5) n=5;
	if (n>60) n=60;
	double *m=matrixof(hd3);
	insert_image(n,stringof(hd1),*realof(hd2)!=0.0,m[0],m[1],m[2],m[3]);
	result=new_real(n,"");
	moveresult(st,result);
}

void minsertimagelines (header *hd)
{   
	header *st=hd;
	int old=insimglines;
	hd=getvalue(hd); if (error) return;
	if (!isnone(hd))
	{
		if (hd->type!=s_real)
		{	output("Argument for color must be real!\n");
			error=90; return;
		}
		insimglines=(int)*realof(hd);
		if (insimglines<10) insimglines=10;
		if (insimglines>100) insimglines=100;
	}
	moveresult(st,new_real(old,""));
}

// Insert an RGB image into the text
// window. Parameters: number of lines to use
// and name, and image.
void minsertpixels (header *hd)
{	header *st=hd,*hd1,*hd2,*result;
	hd=getvalue(hd); if (error) return;
	hd1=next_param(st);
	hd2=next_param(hd1);
	hd1=getvalue(hd1); if (error) return;
	hd2=getvalue(hd2); if (error) return;
	if (hd->type!=s_real || hd1->type!=s_string || hd2->type!=s_matrix)
	{	output("insertpixels needs a number, a string and a matrix!\n"); error=1; return;
	}
	int n=(int)*realof(hd);
	if (n<5) n=5;
	if (n>60) n=60;
	insert_pixels(n,stringof(hd1),matrixof(hd2),dimsof(hd2)->c,dimsof(hd2)->r);
	result=new_real(n,"");
	moveresult(st,result);
}

// Load an image from a file and insert it into
// the notebook. Parameters: numbers of lines to
// use (at most), and filename.
void mloadimage (header *hd)
{	
	header *st=hd,*hdn,*hdscale,*result;
	hdn=next_param(hd);
	hdscale=next_param(hdn);
	hd=getvalue(hd); 
	hdn=getvalue(hdn); 
	hdscale=getvalue(hdscale); 
	if (error) return;
	if (hd->type!=s_string || hdn->type!=s_real || hdscale->type!=s_real)
		need_arg_in("loadimg","string, lines, scale");
	int n=(int)*realof(hdn);
	if (n<5) n=5;
	if (n>60) n=60;
	if (!load_image(n,stringof(hd),*realof(hdscale)))
	{	output("loadimg failed on "); output(stringof(hd)); output("\n"); error=1; return;
	}
	result=new_real(n,"");
	moveresult(st,result);
}

// Load an image from a file and insert it into
// the notebook. Parameters: numbers of lines to
// use (at most), and filename.
void mloadagimages (header *hd)
{	header *st=hd,*hd1,*hd2,*result;
	hd1=next_param(st);
	hd2=next_param(hd1);
	hd=getvalue(hd); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	hd2=getvalue(hd2); if (error) return;
	if (hd->type!=s_real || hd1->type!=s_string || hd2->type!=s_string)
	{	output("loadanaglyph needs a number and two filenames!\n"); error=1; return;
	}
	int n=(int)*realof(hd);
	if (n<5) n=5;
	if (n>60) n=60;
	if (!load_ag_images(n,stringof(hd1),stringof(hd2)))
	{	output("loadimg failed on "); output(stringof(hd)); output("\n"); error=1; return;
	}
	result=new_real(n,"");
	moveresult(st,result);
}


void mpolygon (header *hd)
{	
	header *st=hd,*hd1,*hd2,*result;
	hd1=next_param(st);
	hd2=next_param(hd1);
	if (error) return;
	hd=getvalue(hd);
	if (error) return;
	hd1=getvalue(hd1); 
	if (error) return;
	hd2=getvalue(hd2); 
	if (error) return;
	if (hd->type!=s_matrix || hd1->type!=s_matrix
		|| (dimsof(hd)->r>1 && dimsof(hd1)->r>1 && dimsof(hd)->r!=dimsof(hd1)->r)
		|| (dimsof(hd)->c>1 && dimsof(hd1)->c>1 && dimsof(hd)->c!=dimsof(hd1)->c)
		|| hd2->type!=s_real)
	{	
		print("Need two matrices of equal size and a flag for polygon!"); 
		error=1; return;
	}
	if (dimsof(hd)->c>MAXPOLYGON)
	{	
		print("Polygons are limited to %d points!",MAXPOLYGON); 
		error=1; return;
	}
	int connect=(*realof(hd2)!=0.0);
	double *m1,*m2;
	int r,r1,r2,c;
	getmatrix(hd,&r1,&c,&m1);
	getmatrix(hd1,&r2,&c,&m2);
	r=r1;
	if (r==1) r=r2;

	static double cp[2*MAXPOLYGON];

	if (connect) connect=linecolor;

	graphic_mode();

	for (int i=0; i<r; i++)
	{	double *m=cp;
		for (int j=0; j<c; j++)
		{	*m++=scrcol(*m1++);
			*m++=scrrow(*m2++);
		}
		gpolygon(cp,c,bartype,barcolor,connect);
		if (r1==1) m1=matrixof(hd);
		if (r2==1) m2=matrixof(hd1);	
	}

	gflush();

	result=new_string("",32,"");
	moveresult(st,result);
}

void msetanaglyph (header *hd)
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	double old=anaglyph_d;
	if (hd->type==s_real)
	{	anaglyph_d=*realof(hd);
	}
	else need_arg_in("setanaglyph","real");
	result=new_real(old,"");
	moveresult(st,result);
}

void msavepng (header *hd)
{	header *st=hd,*hd1,*hd2,*hd3,*result;
	hd1=nextof(hd); hd2=nextof(hd1); hd3=nextof(hd2);
	hd=getvalue(hd); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	hd2=getvalue(hd2); if (error) return;
	hd3=getvalue(hd3); if (error) return;
	if (hd->type!=s_string || hd1->type!=s_real || hd2->type!=s_real ||
		hd3->type!=s_real)
			need_arg_in("savesvg","filename, x, y, flag");
	result=new_string(stringof(hd),strlen(stringof(hd))+1,"");
	int w=(int)*realof(hd1);
	int h=(int)*realof(hd2);
	int alias=(int)*realof(hd3);
	if (w>10000) w=10000;
	if (h>10000) h=10000;
	savepng(stringof(hd),w,h,alias);
	moveresult(st,result);
}

void msaveps (header *hd)
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string)
			need_arg_in("saveps","filename");
	result=new_string(stringof(hd),strlen(stringof(hd))+1,"");
	saveps(stringof(hd));
	moveresult(st,result);
}

void msavesvg (header *hd)
{	header *st=hd,*hd1,*hd2,*result;
	hd1=nextof(hd); hd2=nextof(hd1);
	hd=getvalue(hd); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	hd2=getvalue(hd2); if (error) return;
	if (hd->type!=s_string || hd1->type!=s_real || hd2->type!=s_real)
			need_arg_in("savepng","filename, x, y");
	result=new_string(stringof(hd),strlen(stringof(hd))+1,"");
	int w=(int)*realof(hd1);
	int h=(int)*realof(hd2);
	if (w>10000) w=10000;
	if (h>10000) h=10000;
	savesvg(stringof(hd),w,h);
	moveresult(st,result);
}

void mmargin (header *hd)
{	
	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	double old=margin;
	if (hd->type==s_real)
	{	
		margin=*realof(hd);
		if (margin<0.0001) margin=0;
		if (margin>1) margin=1;
	}
	else need_arg_in("margin","real");
	result=new_real(old,"");
	moveresult(st,result);
}

void mplotcubes (header *hd)
{
	header *st=hd,*hd1,*result;
	hd1=nextof(hd); 
	if ((char *)hd1>=newram) hd1=0;
	hd=getvalue(hd); if (error) return;
	if (hd1) hd1=getvalue(hd1); if (error) return;
	double *m,*mcol;
	int c,r,ccol,rcol;
	if (hd->type!=s_matrix) need_arg_in("plotcubes","real nx6 matrix");
	getmatrix(hd,&r,&c,&m); if (error) return;
	if (c!=6) need_arg_in("plotcubes","real nx6 matrix");
	if (hd1) 
	{
		getmatrix(hd1,&rcol,&ccol,&mcol); if (error) return;
		if (rcol!=1 || ccol!=r) need_arg_in("plotcubes","real nx6 matrix and 1xn color vector");
	}
	double cc[8];
	int connect[4];
	for (int i=0; i<4; i++) connect[i]=1;
	graphic_mode();

	cos_left=cos(a_left); sin_left=sin(a_left);
	cos_up=cos(a_up); sin_up=sin(a_up);
	cos_d=cos(anaglyph_d); sin_d=(anaglyph_d);
	double xeye=sin_left*cos_up*distance;
	double yeye=-cos_left*cos_up*distance;
	double zeye=sin_up*distance;

	for (int i=0; i<r; i++)
	{
		int fc=fillcolor1;
		if (hd1) fc=(int)mcol[i];
		double *h=cc;
		if (m[0]>xeye)
		{
			project(m[0],m[2],m[4],h,h+1); h+=2;
			project(m[0],m[3],m[4],h,h+1); h+=2;
			project(m[0],m[3],m[5],h,h+1); h+=2;
			project(m[0],m[2],m[5],h,h+1); h+=2;
			gfill(cc,fc,wirecolor,4,connect);
		}
		if (m[1]<xeye)
		{
			h=cc;
			project(m[1],m[2],m[4],h,h+1); h+=2;
			project(m[1],m[3],m[4],h,h+1); h+=2;
			project(m[1],m[3],m[5],h,h+1); h+=2;
			project(m[1],m[2],m[5],h,h+1); h+=2;
			gfill(cc,fc,wirecolor,4,connect);
		}
		if (m[2]>yeye)
		{
			h=cc;
			project(m[0],m[2],m[4],h,h+1); h+=2;
			project(m[0],m[2],m[5],h,h+1); h+=2;
			project(m[1],m[2],m[5],h,h+1); h+=2;
			project(m[1],m[2],m[4],h,h+1); h+=2;
			gfill(cc,fc,wirecolor,4,connect);
		}
		if (m[3]<yeye)
		{
			h=cc;
			project(m[0],m[3],m[4],h,h+1); h+=2;
			project(m[0],m[3],m[5],h,h+1); h+=2;
			project(m[1],m[3],m[5],h,h+1); h+=2;
			project(m[1],m[3],m[4],h,h+1); h+=2;
			gfill(cc,fc,wirecolor,4,connect);
		}
		if (m[4]>zeye)
		{
			h=cc;
			project(m[0],m[2],m[4],h,h+1); h+=2;
			project(m[0],m[3],m[4],h,h+1); h+=2;
			project(m[1],m[3],m[4],h,h+1); h+=2;
			project(m[1],m[2],m[4],h,h+1); h+=2;
			gfill(cc,fc,wirecolor,4,connect);
		}
		if (m[5]<zeye)
		{
			h=cc;
			project(m[0],m[2],m[5],h,h+1); h+=2;
			project(m[0],m[3],m[5],h,h+1); h+=2;
			project(m[1],m[3],m[5],h,h+1); h+=2;
			project(m[1],m[2],m[5],h,h+1); h+=2;
			gfill(cc,fc,wirecolor,4,connect);
		}
		m+=6;
	}
	gflush();
	result=new_none();
	moveresult(st,result);
}

void msubgrid (header *hd)
{	
	header *st=hd;
	double *m;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix || dimsof(hd)->r!=1 || dimsof(hd)->c!=2)
	{	output("Arguments for subgrid are [i,j]!\n");
		error=90; return;
	}
	m=matrixof(hd);
	int iskipold=iskip,jskipold=jskip;
	iskip=(int)(*m++);
	jskip=(int)(*m++);
	if (iskip<1) iskip=1;
	if (jskip<1) jskip=1;
	header *result=new_matrix(1,2,""); if (error) return;
	m=matrixof(result); *m++=iskipold; *m++=jskipold;
	moveresult(st,result);
}

void msetfont (header *hd)
{
	header *st=hd,*hd1;
	hd1=nextof(hd);
	hd=getvalue(hd); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	if (hd->type!=s_real || *realof(hd)<1 ||
		hd1->type!=s_string)
	{	output("Arguments for setfont must be an integer and a string!\n");
		error=90; return;
	}
	int old=setfont((int)*realof(hd),stringof(hd1));
	moveresult(st,new_real(old,""));
}

void msetfont0 (header *hd)
{
	new_real(setfont(0,""),"");
}

void maspect0 (header *hd)
{
	new_real(getaspect(),"");
}

void maspect (header *hd)
{
	header *st=hd,*res;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real) need_arg_in("aspect","real number");
	res=new_real(setaspect(*realof(hd)),"");
	moveresult(st,res);
}

void mhatchgridsize (header *hd)
{
	header *st=hd,*res;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real) need_arg_in("hatchgridsize","real number");
	int oldhg=hatchgridsize;
	hatchgridsize=(int)(*realof(hd));
	if (hatchgridsize<8) hatchgridsize=8;
	if (hatchgridsize>32) hatchgridsize=32;
	res=new_real(oldhg,"");
	moveresult(st,res);
	doreplay();
}

void mtexpng (header *hd)
{
	header *st=hd,*hdalias,*hdfactor,*hdtransparent,*hdcolor,*res;
	hdalias=next_param(hd);
	hdfactor=next_param(hdalias);
	hdtransparent=next_param(hdfactor);
	hdcolor=next_param(hdtransparent);
	hd=getvalue(hd);
	hdalias=getvalue(hdalias);
	hdfactor=getvalue(hdfactor);
	hdtransparent=getvalue(hdtransparent);
	hdcolor=getvalue(hdcolor);
	if (error) return;
	if (!hdcolor || hd->type!=s_string || hdalias->type!=s_real || hdfactor->type!=s_real || 
		hdtransparent->type!=s_real || hdcolor->type!=s_real)
		need_arg_in("texpng","string, alias (flag), factor (real), transparent (flag), color");
	char *filename=texpng(stringof(hd),*realof(hdalias)!=0,*realof(hdfactor),(int)(*realof(hdtransparent)),(int)(*realof(hdcolor)));
	if (!filename)
	{
		print("Error with Latex formula\n%s\n",stringof(hd));
		error=1; return;
	}
	res=new_string(filename,"");
	moveresult(st,res);
}