// Plot Functions

comment
The plot functions of EMT.
endcomment

// * Reset

white:=0;
black:=1;
red:=2;
green:=3;
blue:=4;
cyan:=5;
olive:=6;
lightgray:=7;
gray:=8;
darkgray:=9;
orange:=10;
lightgreen=11;
turquoise:=12;
lightblue:=13;
lightorange:=14;
yellow:=15;
transparent=255;

spectral=-2;
yellowblue=-3;
purplegreen=-4;
blueyellow=-5;
greenred=-6;
blueyellow=-7;
greenpurple=-8;
yellowblue=-9;
redgreen=-10;

setglobal white,black,red,green,blue, ..
	cyan,olive,lightgray,gray, ..
	darkgray,orange,lightgreen,turquoise, ..
	lightblue,lightorange,yellow;

ResetEpsilon:=epsilon();
ResetView:=_view();

function reset
## Resets many internal settings.
##
## This function resets the default settings for colors, line width,
## the default view, and other basic settings for Euler graphics. It
## does also unclip the window, and release the hold flag.
##
## Moreover, the function resets the default format to long, and
## restores the default epsilon.
##
## See: restart
	matlab off;
	global ResetEpsilon,ResetView;
	_setepsilon(ResetEpsilon);
	resetgraphics;
	resetcolors;
	unclip();
	_style(""); hold off;
	_color(1);
	_setfont();
	_aspect(0);
	shrinkwindow();
	defformat();
	_linewidth(1);
	_view(ResetView);
	_huecolor(1);
	_twosides(1);
	_fillcolor([11,3]);
	_contourcolor(9);
	_wirecolor(9);
	_keepsquare(0);
	_resetcolors();
	defaultgrid1="--";
	defaultgrid2=".";
	defaultgridcolor=rgb(0.5,0.5,0.5);
	gridcolor(defaultgridcolor);
	defaulttextcolor=rgb(0.1,0.1,0.1);
	defaultgrid=2;
	_textcolor(rgb(0.1,0.1,0.1));
	_frame3dcolor(rgb(0.1,0.1,0.1));
	_framecolor(rgb(0.1,0.1,0.1));
	_twosides(1);
	_setanaglyph(0);
	_margin(0.1);
	_markersize(6);
	_subgrid([1,1]);
	_huegrid(0);
	clipping(true);
	userformat$="%0.10g";
	setdecimaldot(".");
	_insimglines(35);
	return 0;
endfunction

// * Plots in 2D

// plot2d() handles all plotting in the plane. This includes plots of
// functions of one variable, implicit plots of functions of two
// variables, plots of curves and polygons in the plane, data and bar
// plots of two vectors, or clouds of points in the plane.

// plot3d() handles all plotting in 3D stereographic view. This includes
// plots of functions of two variables, implicit plots of functions of
// three variables, plots of surfaces, bar plots of matrices, or clouds
// of points in the space. There is an option to view the plot in
// anaglyph mode with red/cyan glasses.

// For a demonstration of 2D plots in Euler, see the following
// introduction notebook.

// See: ../Programs/02 - 2D Graphics | 2D Plots
// See: ../Programs/03 - 3D Graphics | 3D Plots

defaultanaglyphseparation=0.2;
anaglyphseparation=0.2;
defaultnc=10;

function plot2d (xv,yv=none,btest=none,
	a=none,b=2,c=none,d=2,
	xmin=none, xmax=1, r=none, n=none, logplot=0,
	grid=none, frame=none, framecolor=none,
	square=0, color=none, thickness=1, style=none, auto=1, add=0,
	user=0, delta=0.1, points=0, addpoints=0, pointstyle=none,
	bar=0, histogram=0, distribution=0, even=0, steps=0,
	own=0, adaptive=1,
	hue=0, level=none, contour=0, nc=defaultnc,
	filled=0, fillcolor=none, outline=1,
	title="",xl=none,yl=none,
	maps=0,
	contourcolor=none, contourwidth=1,
	ticks=1, margin=none, clipping=true,
	cx=0, cy=0,
	insimg=0, spectral=0,
	cgrid=none, vertical=1, smaller=none,
	dl=0,
	niveau=none, levels=none)
## Multipurpose plot function for plots in the plane.
## 
## Multi-Purpose function for 2D plots. For an overview of Graphics
## and Plots in EMT, double click the following link.
##
## See: Plots
##
## This function can do plots of functions of one variables, data
## plots, curves in the plane, bar plots, grids of complex numbers, and
## implicit plots of functions of two variables.
##
## Parameters
##
## x,y : 
##   Equations, functions or data vectors.
##   One equation is a simple function plot. Two equations is a curve.
##   In this case xmin,xmax need to be used, because a,b,c,d determine
##   the plot area in this case. If x and y are row vectors, the points
##   will be used for the plot. If x and y are matrices, a plot for each
##   rwo will be produced. For implicit plots, x must be an expression
##   in x and y. xmin,xmax : Range for curve plots with two functions.
## a,b,c,d : 
##   Plot area (default a=-2,b=2)
## r : 
##   If r is set, then a=cx-r, b=cx+r, c=cy-r, d=cy+r,
##   r can be a vector [rx,ry] or a vector [rx1,rx2,ry1,ry2].
## xmin,xmax : 
##   Range of the parameter for curves.
## auto : 
##   Determine y-range automatically (default is >auto).
## square : 
##   If true, try to keep square x-y-ranges.
## n : 
##   Number of intervals (default is adaptive).
## adaptive : 
##   Use adaptive plots (n is the minimal number of steps).
##   This is used by default. In steep regions, more plot points
##   will be generated, i.e., a finer grid.
## grid : 
##   0 = no grid and labels, 
##   1 = axis only, 
##   2 = normal grid (see below for the number of grid lines)
##   3 = inside axis
##   4 = no grid
##   5 = full grid including margin
##   6 = ticks at the frame
##   7 = axis only
##   8 = axis only, sub-ticks
## frame: 
##   Draw a frame around the plot (default >frame)
## framecolor: 
##   Color of the frame and the grid.
## margin : 
##   Number between 0 and 0.4 for the margin around the plot.
##
## color : 
##   Color of curves. If this is a vector of colors, it will be used
##   for each row of a matrix of plots. In the case of point plots, it
##   should be a column vector. If a row vector or a full matrix of
##   colors is used for point plots, it will be used for each data
##   point.
##
## thickness :
##   Line thickness for curves This value can be smaller than 1 for
##   very thin lines.
##
## style : 
##   Plot style for lines, markers, and fills.
##   For points use 
##      "[]", "<>", ".", "..", "...",
##      "*", "+", "|", "-", "o"
##      "[]#", "<>#", "o#" (filled shapes)
##      "[]w", "<>w", "ow" (non-transparent)
##   For lines use 
##      "-", "--", "-.", ".", ".-.", "-.-", "->"
##   For filled polygons or bar plots use 
##      "#", "#O", "O", "/", "\", "\/",
##      "+", "|", "-", "t"
##
## points : 
##   Plot single points instead of line segments (>points).
## addpoints : 
##   If true, plots line segments and points (>addpoints).
##
## add : 
##   Add the plot to the existing plot. This will not redraw the grid
##   or the frame, just add another plot. It will use the same range
##   values.
##
## user :
##   Enable user interaction for functions (>user) for plots of
##   functions.
##     left, right : move the interval
##     +/- : enlarge the viewpoint.
##     mouse clicks : select region of interest.
##     space : end interaction.
## delta : 
##   Step size for user interaction.
##
## bar : 
##   Bar plot (x are the interval bounds, y the interval values) (>bar).
##   The package stat.e contains column plots too.
## histogram : 
##   Plots the frequencies of x in n subintervals. x is a vector of
##   random values.
## distribution : 
##   Plots the distribution of x with n subintervals.
## even : 
##   Use integer values for automatic histograms.
##
## steps : 
##   Plots the function as a step function (steps=1,2).
##
## level : 
##   Plot level lines of an implicit function of two variables.
##   If the level value is a 2xn matrix, ranges of levels will be drawn
##   in the color using the given fill style. If outline is true, it
##   will be drawn in the contour color. Using this feature, regions of
##   f(x,y) can be marked. Instead of a 2xn matrix, a simple vector of
##   levels can be used and dl can be set to the thickness of these
##   levels.
## outline : 
##   Draws boundary of level ranges.
## hue : 
##   Aadd hue color to the level plot to indicate the function
##   value 
## contour : 
##   Use level plot with automatic levels.
## nc : 
##   Number of automatic level lines
## contourcolor : 
##   Color of contour lines
## contourwidth :
##   Width of contour lines
##
## title : 
##   Plot title (default "").
##   This can be used to describe the plot. The title will appear
##   above the plot. Moreover, a label for the x and y axis can be added
##   with xl="string" or yl="string". Other labels can be added with the
##   functions label() or labelbox(). The title can be a unicode string
##   or an image of a Latex formula.
##
## xl, yl : 
##   Labels for the x- and y-axis
## smaller :
##   If >0, there will be more space to the left for labels.
##
## vertical : 
##   Turns vertical labels on or off. This changes the global variable
##   verticallabels locally for one plot. The value 1 sets only vertical
##   text, the value 2 uses vertical numerical labels on the y axis.
##
## filled : 
##   Fill the plot of a curve. A curve is simply a plot with non-increasing
##   x values. The fill style is determined by the parameter style.
## fillcolor :
##   Fill color for bar and filled curves
## outline :
##   Boundary for filled polygons
##
## logplot : set logarithmic plots
##   1 = logplot in y, 
##   2 = logplot in xy, 
##   3 = logplot in x
##
## own : 
##   A string, which points to an own plot routine. With >user, you get
##   the same user interaction as in plot2d. The range will be set
##   before each call to your function.
##
## maps : 
##   Map expressions (0 is faster), functions are always mapped.
##
## clipping :
##   Sets clipping (default is true).
##
## cgrid : 
##   Determines the number of grid lines for plots of complex grids.
##   Should be a divisor of the the matrix size minus 1 (number of
##   subintervals). cgrid can be a vector [cx,cy].
##
## Typical Examples:
##
## >plot2d("x^3-x",-1,2,title="y=x^3-x",yl="y",xl="x");
## >plot2d("x^3-x",a=0,b=2,c=-1,d=2,grid=3,<frame);
## >plot2d({{"x^3-a*x",a=1}},>user,title="Press any key!");
##
## >function f(x) &= x^x;
## >plot2d(f,r=1,cx=1,cy=1,color=blue,thickness=2);
## >plot2d(&diff(f(x),x),>add,color=red,style="-.-");
##
## >x=linspace(0,2pi,1000);
## >plot2d(sin(5x),cos(7x));
## >plot2d(sin(x),cos(x)*0.5,r=1,>filled,style="/");
##
## >k=0:10; 
## >plot2d(k,bin(10,k),>bar);
## >plot2d(k,bin(10,k)); plot2d(k,bin(10,k),>points,>add);
##
## >plot2d(normal(1,1000),>distribution,style="O");
## >plot2d("qnormal",0,5;2.5,0.5,>filled);
##
## >plot2d("x^2",0,1,steps=1,color=red,n=10);
## >plot2d("x^2",>add,steps=2,color=blue,n=10);
##
## >plot2d("x^3-y^2",>contour,>hue,>spectral);
## >plot2d("x^3-y^2",level=0,contourwidth=3,>add,contourcolor=red);
##
## For more information refer to the overview in Plots or the tutorials.
##
## See: Plots, plot3d, label, rgb, yaxis, xgrid, xlabel, call
	
	if grid==none then grid=defaultgrid; endif;
	
	if frame==none then
		if grid==0 or grid==3 or grid==7 or grid==8 then frame=0;
		else frame=1;
		endif;
	endif;
	
	if niveau!=none then level=niveau; endif;
	if levels!=none then 
		if typeof(levels)==0 and levels==true then level="thin";
		else level=levels; 
		endif;
	endif;

	oldwin=none;
	if smaller then
		oldwin=_window();
		shrinkwindow(smaller);
	endif;

	errors off; // functions may not be defined everywhere
	
	global usesubwindows;

	oldclipping=clipping(clipping);
	
	// contour or level plots (calls fcontour or datacontour)
	if contour or level!=none then
		if dl>0 and level!=none then level=(level-dl)_(level+dl); endif;
		if spectral then color=-1-spectral;	endif;
		oldframe=_framecolor(framecolor);
		if  not add then
			if r==none then r=1; endif;
			if a==none then a=cx-r; b=cx+r; endif;
			if c==none then c=cy-r; d=cy+r; endif;
			setplot(a,b,c,d);
		endif;
		if typeof(xv)==8 then
			fcontour(xv,a,b,c,d;args(), ..
				=hue,=level,=title,=n,=nc, ..
				=add,=color,=maps,=grid, ..
				=contourcolor,=contourwidth, ..
				=style,=outline,=frame);
		else
			datacontour(xv,a,b,c,d, ..
				=hue,=level,=title,=nc, ..
				=add,=color,=grid, ..
				=contourcolor,=contourwidth, ..
				=style,=outline,=frame);
		endif;
		errors on;
		_framecolor(oldframe);
		if margin<>none then _margin(marginold); endif;
		clipping(oldclipping);
		if insimg then insimg(); endif;
		if oldwin then _window(oldwin); endif;
		return;
	endif;

	if margin<>none then marginold=_margin(margin); endif;
	if fillcolor==none then fillcolor=fillcolor()[2]; endif;
	if color==none then color=color(); endif;
	if framecolor==none then framecolor=framecolor(); endif;
	
	if n==none then n=100; endif;

	oldhold=holding();
	
	// hold, if add=1 or subwindows are used
	if add then hold on; 
	elseif !oldhold then hold off; 
	endif;
	
	global verticallabels;
	oldvertical=verticallabels;
	verticallabels=vertical;
	
	aa=a; bb=b; cc=c; dd=d; xx=xv; yy=yv;
	
	// take care of plot("f(x)",a,b)
	if typeof(btest)==0 and typeof(yy)==0 then
		aa=yv; bb=btest; yy=none;
		if typeof(a)==0 then cc=a; dd=b; endif;
	endif;
	
	// set the range if r is specified
	if r!=none then
		if cols(r)==2 then r=[r[1],r[1],r[2],r[2]]; endif;
		aa=cx-r{1}*getaspect(); bb=cx+r{2}*getaspect(); 
		cc=cy-r{3}; dd=cy+r{4};
	endif;
	
	// plot a complex matrix
	if iscomplex(xx) then
		if points then
			plot2d(re(xx),im(xx),a=aa,b=bb,c=cc,d=dd,points=1, ..
				grid=grid,frame=frame,framecolor=framecolor, ..
				square=square,color=color,thickness=thickness, ..
				style=style,add=add,title=title,user=user,maps=maps, ..
				xl=xl,yl=yl,r=r,ticks=ticks,margin=margin,grid=grid,
				filled=filled,style=style,=cx,=cy);
		elseif rows(xx)==1 then
			plot2d(re(xx),im(xx),a=aa,b=bb,c=cc,d=dd,grid=grid, ..
				frame=frame,framecolor=framecolor, ..
				square=square,color=color,thickness=thickness,style=style, ..
				add=add,title=title,user=user,thickness=thickness, ..
				maps=maps,xl=xl,yl=yl,r=r,ticks=ticks,margin=margin,
				filled=filled,style=style,=cx,=cy);
		else
			if cgrid!=none then
				ci=linspace(1,rows(xx),cgrid{1});
				cj=linspace(1,cols(xx),cgrid{2});
			else
				ci=1:rows(xx); cj=1:cols(xx);
			endif;
			plot2d(re(xx[ci]),im(xx[ci]),a=aa,b=bb,c=cc,d=dd,grid=grid, ..
				frame=frame,framecolor=framecolor, ..
				square=square,color=color,thickness=thickness,style=style, ..
				add=add,title=title,user=user,thickness=thickness, ..
				maps=maps,xl=xl,yl=yl,r=r,ticks=ticks,margin=margin,=cx,=cy);
			plot2d(re(xx'[cj]),im(xx'[cj]),add=1,grid=grid, ..
				frame=frame,framecolor=framecolor, ..
				square=square,color=color,thickness=thickness,style=style, ..
				add=add,title=title,user=user,thickness=thickness, ..
				maps=maps,xl=xl,yl=yl,r=r,ticks=ticks,margin=margin,=cx,=cy);
		endif;
		errors on;
		if margin<>none then _margin(marginold); endif;
		clipping(oldclipping);
		if insimg then insimg(); endif;
		holding(oldhold);
		if oldwin then _window(oldwin); endif;
		return;
	endif;
	
	// use old plot range, if add=1
	if add then
		p=_plot();
		if aa==none then aa=p[1]; bb=p[2]; endif;
		cc=p[3]; dd=p[4];
	// if a,b are not specified, and c,d not, compute range
	// automatically
	elseif typeof(aa)<>8 and typeof(cc)==8 then
		auto=1;
	// if both are specified, use them
	elseif typeof(aa)<>8 and typeof(cc)<>8 then
		auto=0;
	endif;
	
	// if histogram>0 then compute the xx,yy from the data
	if histogram then
		if histogram==1 then histogram=20; endif;
		{xx,yy}=histo(xx,histogram,even=even);
		bar=1;
	endif;
	
	// if distribution>0 then compute xx, yy from the data
	if distribution then
		if distribution==1 then distribution=20; endif;
		ny=cols(xx);
		{xx,yy}=histo(xx,distribution,even=even);
		yy=yy/(ny*(xx[2]-xx[1]));
		bar=1;
	endif;
	
	if !add then unclip(); endif;
	if typeof(aa)==8 then aa=-2; endif;
	if typeof(cc)==8 then cc=-2; endif;
	
	// determine xmin, xmax from a and b
	allx=0;
	if typeof(xmin)==8 then xmin=aa; xmax=bb; allx=1; endif;
	ks=keepsquare(square);
	
	olds=[aa,bb,cc,dd];
	
	oldframe=_framecolor(framecolor);
	
	// loop for user interaction
	repeat
		// set the range
		if !add then
			if !auto then setplot(aa,bb,cc,dd); 
			else setplot();
			endif;
		else
			hold on;
		endif;
		
		if allx then xmin=aa; xmax=bb; endif;
		
		// own plotting routine
		if own then
			if (!add) then clg; endif;
			ccc=_color(color); l=_linewidth(thickness);
			xx(args());
			_color(ccc); _linewidth(1);
		elseif iscall(xx) and iscall(yy) then
		// curve with one or two parameter functions
			if yy<>none then
			// two functions specified
				if adaptive then
					dw=abs(xmax-xmin);
					{s,u}=adaptiveeval(xx,yy,xmin,xmax, ..
						dw/n,dw/n^2,dw/n;args());
				else
					t=linspace(xmin,xmax,n);
					s=%ploteval(xx,t;args());
					u=%ploteval(yy,t;args());
				endif;
				ccc=_color(color); l=_linewidth(thickness);
				if style!=none then oldst=_linestyle(style); endif;
				if logplot then 
					if logplot==2 then 
						xylogplot(t,u,color=color,points=points,frame=frame,grid=grid);
					elseif logplot==3 then 
						xlogplot(t,u,=points,=color,=frame,=grid,=ticks);
					else 
						ylogplot(t,u,=color,=points,=frame,=grid,=ticks);
					endif;
				elseif filled then
					if (!add) then 
						if auto then 
							setplot(totalmin(s),totalmax(s),totalmin(u),totalmax(u));
						endif;
						clg; 
					endif;
					if !add then 
						xplot(grid=grid,ticks=ticks,frame=frame); 
					endif;
					ccf=_barcolor(fillcolor);
					ccc=_color(color);
					if style!=none then oldst=_barstyle(style); endif;
					if !add then
						if auto then plotarea(s,u); endif;
						if !holding() then clg; endif;
						xplot(grid=grid,ticks=ticks,frame=frame); 
					endif;
					h1=holding(1); polygon(s,u,outline); holding(h1);
					if style!=none then _barstyle(oldst); endif;
					_barcolor(ccf);
				else 
					if steps then {s,u}=%makesteps(s,u,steps==2); endif;
					if !add then
						if auto then plotarea(s,u); endif;
						if !holding() then clg; endif;
						xplot(grid=grid,ticks=ticks,frame=frame); 
					endif;
					h1=holding(1); plot(s,u); holding(h1);
				endif;
				if style!=none then _linestyle(oldst); endif;
				_color(ccc); _linewidth(1);
			elseif xv==none then
			// function xv is none
				if !add then 
					if !auto then clg; endif;
					xplot(grid=grid,ticks=ticks,frame=frame); 
				endif;
			else
			// one function specified
				if adaptive then
					dw=abs(xmax-xmin);
					{t,u}=adaptiveevalone(xx,xmin,xmax, ..
						dw/n,dw/n^2,dw/n,auto;args());
				else
					t=linspace(xmin,xmax,n);
					u=%ploteval(xx,t;args());
					if cols(u)<>cols(t) then
						error("Cannot plot this result!"); 
					endif;
				endif;
				ccc=_color(color); l=_linewidth(thickness); 
				if style!=none then oldst=_linestyle(style); endif;
				if logplot then 
					if logplot==2 then 
						xylogplot(t,u,=points,=color,=frame,=grid,=ticks);
					elseif logplot==3 then 
						xlogplot(t,u,=points,=color,=frame,=grid,=ticks);
					else 
						ylogplot(t,u,=points,=color,=frame,=grid,=ticks);
					endif;
				else 
					if steps then {t,u}=%makesteps(t,u,steps==2); endif;
					if !add then
						if auto then plotarea(t,u); endif;
						if !holding() then clg; endif;
						xplot(grid=grid,ticks=ticks,frame=frame); 
					endif;
					h1=holding(1); 
					if filled then 
						bst=barstyle(style);
						bcl=barcolor(fillcolor);
						polygon(t[1]|t|t[-1],0|u|0,1); 
						barcolor(bcl);
						barstyle(bst);
					else plot(t,u); 
					endif;
					holding(h1);
				endif;
				if style!=none then _linestyle(oldst); endif;
				_color(ccc); _linewidth(l);
			endif;
		elseif typeof(xx)==13 then
			// vector of functions specified
			if adaptive then
				dw=abs(xmax-xmin);
				{t,u}=adaptiveevalone(xx[1],xmin,xmax, ..
					dw/n,dw/n^2,dw/n,auto;args());
			else
				t=linspace(xmin,xmax,n);
				u=%ploteval(xx[1],t;args());
				if cols(u)<>cols(t) then
					error("Cannot plot this result!"); 
				endif;
			endif;
			loop 2 to length(xx)
				u=u_(%ploteval(xx[#],t;args()));
			end;
			ccc=_color(-1); l=_linewidth(1); 
			if steps then {t,u}=%makesteps(t,u,steps==2); endif;
			oldst=_linestyle("");
			if !add then
				if auto then plotarea(t,u); endif;
				if !holding() then clg; endif;
				xplot(grid=grid,ticks=ticks,frame=frame); 
			endif;
			h1=holding(1); 
				loop 1 to rows(u);
					_color(color{#});
					_linewidth(thickness{#});
					if style!=none then _linestyle(style{#}); endif;
					plot(t,u[#]); 
				end;
			holding(h1);
			if style!=none then _linestyle(oldst); endif;
			_color(ccc); _linewidth(l);
		// data plots:
		else
			if yy==none then
				yy=xx; xx=1:cols(yy);
			elseif typeof(yy)==11 then
				xx=xx|flipx(xx); yy=left(yy)|flipx(right(yy));
				filled=1; polygon=1;
			endif;
			if points then
				if pointstyle!=none then style=pointstyle; endif;
				if style!=none then oldst=_markerstyle(style); endif;
				if square then setplot(aa,bb,totalmin(yy),totalmax(yy)); endif;
				if logplot then
					if logplot==2 then 
						xylogplot(xx,yy,=points,=color,=frame,=grid,=ticks);
					elseif logplot==3 then 
						xlogplot(xx,yy,=points,=color,=frame,=grid,=ticks);
					else 
						ylogplot(xx,yy,=points,=color,=frame,=grid,=ticks);
					endif; 
				else
					if !add then
						if auto then plotarea(xx,yy); endif;
						if !holding() then clg; endif;
						xplot(grid=grid,ticks=ticks,frame=frame); 
					endif;
					h1=holding(1); 
					if cols(color)>1 then
						mark(xx,yy,color);
					else
						ccc=_color(color);
						mark(xx,yy); 
						_color(ccc);
					endif;
					holding(h1);
				endif;
				if style!=none then _markerstyle(oldst); endif;
			elseif bar then
				ccc=_barcolor(fillcolor);
				if style!=none then oldst=_barstyle(style); endif;
				if cols(xx)==cols(yy);
					xx=xx|(2*xx[cols(xx)]-xx[cols(xx)-1]);
				endif;
				if auto and !add then
					setplot(min(xx),max(xx),1.1*min(0|yy),1.1*max(0|yy)); 
				endif;
				xxx=xx[1:cols(xx)-1];
				if !add then
					if !holding() then clg; endif;
					xplot(grid=grid,ticks=ticks,frame=frame); 
				endif;				
				h=holding(1);
				plotbar(xxx,0,xx[2:cols(xx)]-xxx,yy[1:cols(xx)-1]);
				holding(h);
				if style!=none then _barstyle(oldst); endif;
				_barcolor(ccc);
			elseif filled then
				if !add then
					if auto then plotarea(xx,yy); endif;
					if !holding() then clg; endif;
					xplot(grid=grid,ticks=ticks,frame=frame); 
				endif;
				ccf=_barcolor(fillcolor);
				ccc=_color(color);
				if style!=none then oldst=_barstyle(style); endif;
				h1=holding(1); polygon(xx,yy,outline); holding(h1);
				if style!=none then _barstyle(oldst); endif;
				_barcolor(ccf);
				_color(ccc);
			else
				ccc=_color(color); l=_linewidth(thickness); 
				if style!=none then oldst=_linestyle(style); endif;
				if logplot then
					if logplot==2 then 
						xylogplot(xx,yy,=points,=color,=frame,=grid,=ticks);
					elseif logplot==3 then 
						xlogplot(xx,yy,,=points,=color,=frame,=grid,=ticks);
					else 
						ylogplot(xx,yy,,=points,=color,=frame,=grid,=ticks);
					endif;
				else 
					if square then setplot(aa,bb,totalmin(yy),totalmax(yy)); endif;
					if steps then {xx,yy}=%makesteps(xx,yy,steps==2); endif;
					if !add then
						if auto then plotarea(xx,yy); endif;
						if !holding() then clg; endif;
						xplot(grid=grid,ticks=ticks,frame=frame); 
					endif;
					h1=holding(1); 
					plot(xx,yy); 
					if addpoints then 
						mst=_markerstyle(pointstyle);
						mark(xx,yy); 
						_markerstyle(mst);
					endif;
					holding(h1);
				endif;
				if style!=none then _linestyle(oldst); endif;
				_color(ccc); _linewidth(l);
			endif;
		endif;
		settitle(title);
		if xl<>none then xlabel(xl); endif;
		if yl<>none then ylabel(yl); endif;
		if add then holding(oldhold); endif;
		if !user then break; endif;
		gotkey=0;
		k1=_mouse("Press return, cursor key, +/-, space, click with mouse");
		if cols(k1)==1 then
			gotkey=k1;
		elseif add then
			gotkey=13;
		else
			hl=holding(1); mark(k1[1],k1[2]); holding(hl);
			k2=_mouse("Click with mouse");
			if auto then k2[2]=k1[2]; endif;
			if (cols(k2)==1) then
				gotkey=k2;
			else
				hl=holding(1); 
				mark(k2[1],k2[2]);
				{kl,k2}={min(k1,k2),max(k1,k2)};
				c=color(2);
				plot([kl[1],kl[1],k2[1],k2[1],kl[1]],[kl[2],k2[2],k2[2],kl[2],kl[2]]);
				color(c);
				holding(hl);			
				wait(0.5);
				aa=kl[1]; bb=k2[1]; cc=kl[2]; dd=k2[2];
			endif;
		endif;
		if gotkey then
			k=gotkey;
			if k==13 or k==9 then break;
			elseif k==27 then error("Interrupt"); break;
			elseif !add then
				if k==43 then
					{aa,bb}={(1-delta)*aa+delta*bb,(1-delta)*bb+delta*aa};
					{cc,dd}={(1-delta)*cc+delta*dd,(1-delta)*dd+delta*cc};
				elseif k==45 then
					{aa,bb}={(1+delta)*aa-delta*bb,(1+delta)*bb-delta*aa};
					{cc,dd}={(1+delta)*cc-delta*dd,(1+delta)*dd-delta*cc};
				elseif k==3 then
					{aa,bb}={(1+delta)*aa-delta*bb,(1-delta)*bb+delta*aa};
				elseif k==4 then
					{aa,bb}={(1-delta)*aa+delta*bb,(1+delta)*bb-delta*aa};
				elseif k==2 then
					{cc,dd}={(1+delta)*cc-delta*dd,(1-delta)*dd+delta*cc};
				elseif k==1 then
					{cc,dd}={(1-delta)*cc+delta*dd,(1+delta)*dd-delta*cc};
				elseif k==32 then
					aa=olds[1]; bb=olds[2]; cc=olds[3]; dd=olds[4];
				endif;
			endif;
		endif;
	end;
	_framecolor(oldframe);
	if margin<>none then _margin(marginold); endif;
	clipping(oldclipping);
	keepsquare(0);
	errors on;
	if insimg then insimg(); endif;
	verticallabels=oldvertical;
	if oldwin!=none then window(oldwin); endif;
	return;
endfunction

// * Plots in 3D

function plot3d (x, y=none, z=none,
	xmin=none, xmax=none, ymin=-1, ymax=1, n=60,
	a=none, b=none, c=none, d=none, r=none,
	scale=1, fscale=-1, frame=1,
	angle=none, height=none, zoom=none, 
	distance=none, view=none, 
	center=none, cx=0, cy=0, cz=0,
	xlabel="x", ylabel="y", zlabel="z", 
	sframe=none, grid=none,
	polar=false, sliced=false, disconnect=none,
	hue=false, light=[2,3,4], amb=0.1, max=0.9,
	spectral=false, xhue=0, yhue=0, zhue=0, hues=none,
	color=none, framecolor=none, transparent=0,
	anaglyph=0,	user=0, duser=0.1,
	wire=false, wirecolor=none, points=false, style=".",
	lines=false, contour=false, rotate=false, title="", 
	bar=false, own=false,
	level=none, dl=0, nc=defaultnc, values=none, contourcolor=none, contourwidth=1,
	implicit=false, zmin=-1, zmax=-1,
	maps=false, insimg=false,
	limits=none, add=false,
	cp=false, cpcolor=none, cplevel=none, cpdelta=0.05,
	levels=none, niveau=none,
	linewidth=none, fillcolor=none,
	fullwindow=true,
	zscale=false,zlim=none)
## General function for 3D plots.
##
## Multi-Purpose function for 3D plots. For an overview of Graphics
## and Plots in EMT, double click the following link.
##
## See: Plots
##
## This function can plot 3D plots with graphs of functions of two
## variables, parameterized surfaces, space curves, clouds of points,
## solutions of an equation of three variables. All 3D plots can be
## shown as anaglyphs.
##
## Parameters
##
## x : expression in x and y
## x,y,z : matrices of the coordinates of a surface
## x,y,z : expressions in x and y for a parametric surface
## x,y,z : expressions in x to plot a space curve
##
## xmin,xmax,ymin,ymax : 
##   x,y bounds for expressions
## a,b,c,d : 
##   Alternative for xmin,xmax,ymin,ymax.
## r : 
##   Can be used instead of xmin,xmax,ymin,ymax.
##   r can be a vector [rx,ry] or [rx,ry,rz].
##   r is also used in polar plots.
## cx, cy, cr : 
##   Center of plot, if r is used.
##
## n : 
##   Accuracy, i.e. the number of sub-intervals. If n is a
##   vector, it is used in each direction.
## grid : 
##   Determines the grid lines. For plots of functions, n can
##   be larger than grid, but it will be adjusted to be
##   a multiple of grid. For x-y-z-plots the user has to
##   take care of this. A 1x2 vector can be used for each
##   direction.
##
## scale : 
##   Scale plot to this size (0 = no scaling, 1 = default).
##   If scale is a 1x3 vector, it will scale in each direction.
##   Function plots will be auot-scaled. But x-y-z-plots often
##   need to be scaled in some direction.
##
## fscale : 
##   Scales the z-values for a function to this range 
##  (0=no, -1=auto)
##
## zscale : 
##   Scales data matrices in z-direction. Can be combined with
##   scale=[sx,sy,sz]. The value of zscale is used as a scaling
##   factor. It should be between 0.1 and 1.
##
## wire : Generates a wire plot (>wire).
## polar : Generats a polar plot (>polar)
## points : Plots a cloud of points. (>points)
##
## sliced : 
##   Plot a sliced version (0=no, 1=x-direction, 2=y-direction).
## hue : 
##   Compute shading using a light source.
## light, amb, max : 
##   Controls the shading setting the light point, ambient and 
##   maximum.
## contour : 
##   Show thick level lines (with automatic levels).
## levels: 
##   If true, set thin level lines (automatic levels).
## level : 
##   Level lines (1xn vector) or level ranges (2xn vector).
## dl : 
##   Expand level lines (1xn) to level ranges (2xn) with this 
##  thickness.
##
## spectral : 
##   Use spectral colors instead of monochrome hue.
##   There are spectral schemes from spectral=1 (>spectral) 
##   to spectral=9. >spectral defaults to >zhue.
##   In fact, this is equivalent to color=-2 to color=-10.
##
## xhue,yhue,zhue : 
##   Use these coordinates instead of a light source.
## hues : 
##   A matrix of hue values from 0 to 1 for the shading
##   for x-y-z-plots. The matrix must have a size compatible 
##   to x,y,z.
##
## values : 
##   Values to be used for contour plots (default: z-values)
##   in x-y-z-plots with hue. The levels and the limits are 
##   computedusing these values. The matrix must be compatible
##   to the matrices x, y, z.
## contourcolor : 
##   Color of contour lines.
## contourwidth : 
##   Width of contour lines.
## fillcolor : 
##   Fill color for 3d surfaces with no hue.
##
## limits : 
##   Clip the plot outside an interval of values.
##   This is a 1x2 matrix [min,max]. The values taken from
##   the parameter values (by default the z-values). In case
##   of a function plot, you may want to set zlim to restrict 
##   the frame (a 1x2 matrix too).
##
## user : 
##  The user can turn the plot around with the keyboard.
##  left,right,up,down turns the plot. +,- zooms the plot.
##  Space resets the plot. Return ends the user interaction.
##  The key a generates an anaglyph plot. The key l toggles
##  the movement of the light source for hue plots. The key
##  c moves the plot up, down, left or right.
## duser : 
##  Controls the accuracy of the user action (default 0.1).
##
## lines : 
##   Plot a line in space given by three expressions in x. 
##   Use xmin, xmax for the bounds of the parameter x.
##
## rotate : 
##   Rotation plot of a funciton in one expression in x.
##   Use xmin, xmax for the bounds of the parameter x.
##
## anaglyph : 
##   Generate an anaglyph 3d plot (>anaglyph). This plot
##   needs red-dyan glasses to be viewed properly.
##
## viewangle :
##   The default angle of view, rotated around the z-axis.
##   The value is in radians, but 10° etc. can be used.
##   0 is in the direction of the x-axis. 
## viewheight : 
##   The height of view above the x-y-axis (-pi/2 to pi/2).
##   Use -90° to 90°.
## zoom : 
##   The zoom of view. Default is around 2.6.
## distance : 
##   Distance of view. Default is 5. Note that the plot is
##   usually scaled unless <scale is set. The distance applies
##   to the scaled plot.
## view : 
##   The complete view, a 1x4 vector conaining distance, zoom,
##   viewangle, viewheight.
## center :
##   This vector moves the center of the plot. It is necessary
##   if the plot should not be centered in (0,0,0) automatically.
##   For x-y-z-plots with an own frame, it is essential.
##
## implicit : 
##   Creates an implict plots of f(x,y,z)=0. The plot consists of
##   grid lines on the surface cut by planes parallel to an axis.
##   Use 0=off, 1=x, 2=y, 4=z to select the axis. These values can
##   be added.
##
## style : 
##   Plot style for markers (see plot2d for available styles).
##
## maps : 
##   Expressions should be mapped (faster, if 0). Functions
##   are always mapped.
##
## color : 
##   Color for markers and for shaded surfaces
## wirecolor : 
##   Color for wire plots
##
## frame : 
##   If 0 (<frame), no frame is drawn. If this is a vector, it must 
##   be of the form [xmin,xmax,ymin,ymax,zmin,zmax]. Make sure that
##   the plot fits into the frame. The vector is only applied to
##   x-y-z-plots, not to plot of functions.
## framecolor : 
##   The color of the 3D frame.
## sframe : 
##   Sets the ranges for the values on the frame. This is a vector of the
##   same form as frame. Use this, if the true ranges are different from
##   the values.
##
## cp : 
##   Draws a contour plane below the plot (>cp).
## cpcolor : 
##   The color for the contour plane.
## cplevel : 
##   The level lines for the contour plane.
## cpdelta : 
##   The distance (relative) to the z-range of the contour plane.
##
## Typical Examples:
##
## >plot3d("x*y",r=1,title="z=x*y"):
## >plot3d("x*y^2",>user,r=1,title="Press cursor keys or return!"):
## >plot3d("x^2*y^3",r=0.9,zlabel="x^2*y^3",angle=30°,height=20°,zoom=3, ...
## >  >cp,cplevel="thin",cpcolor=green):
## >plot3d("x^2+y^3",angle=0°,>contour,>spectral):
## >plot3d("x^y-y^x",a=0,b=4,c=0,d=4,angle=40°,level=0, ...
## >  contourwidth=4,contourcolor=red,n=100):
## >plot3d("x^2+y^2",>wire,>anaglyph,title="Use Red/Cyan Glasses!",n=10):
## >plot3d("x^3+y^2",0,2,0,10,scale=[5,1,2],zoom=3.2,grid=10,>transparent):
## >plot3d("x^2+y^3+sin(z)^2-1",r=pi,implicit=4,zoom=3):
## >x=-1:0.05:1; y=x'; plot3d(x,x*y^2,y,>hue,angle=20°):
## >h=x^2+y^2; ...
## >plot3d(x,y,(y-x)/2,level=-2:0.1:2,values=h,hues=h/2,>spectral):
## >X=normal(3,1000); plot3d(X[1],X[2],X[3],>points,zoom=3,>user):
## >Y=cumsum(X); plot3d(Y[1],Y[2],Y[3],>wire,>user):
## >t=linspace(0,2,1000);  ...
## >plot3d(sin(2pi*t),cos(2pi*t),t,>wire,>anaglyph):
## >plot3d("min(1/(x^2+y^2),3)",r=1.5,>hue,limits=[0,2.9]):
##
## For more information see the overview in Plots or refer to the tutorials.
##
## See: Plots, plot2d, call
	
	if levels!=none then 
		if typeof(levels)==0 and levels==true then level="thin";
		else level=levels; 
		endif;
	endif;
	if niveau!=none then level=niveau; endif;
	
	if wirecolor==none and anaglyph then wirecolor=black; endif;
	
	oldwc=_wirecolor(wirecolor); 
	oldfc=_frame3dcolor(framecolor);
	fc=_fillcolor(fillcolor);
	
	global usesubwindows;
	oldwindow=none;
	if fullwindow && !usesubwindows then 
		oldwindow=window(); fullwindow(title=(title!=""));
	endif;
	
	errors off; // allow plot areas out of the definition set of the function
	
	global defaultanaglyphseparation;
	
	oldcenter=_center(center);
	resetcenter=_center();

	// handle plot3d(expr,a,b)
	if typeof(x)==8 and typeof(y)==0 and ..
				typeof(z)==0 and xmin!=none and xmax!=none then
		ymin=xmin; ymax=xmax; xmin=y; xmax=z;
	endif;	
	
	if r!=none then 
		if cols(r)==3 then r=[r[1],r[1],r[2],r[2],r[3],r[3]]; 
		elseif cols(r)==2 then r=[r[1],r[1],r[2],r[2],r[2],r[2]];
		endif;
		xmin=cx-r{1}; xmax=cx+r{2}; 
		ymin=cy-r{3}; ymax=cy+r{4}; 
		zmin=cz-r{5}; zmax=cz+r{6};
		xx=x; yy=y; zz=z;
	else
		if xmin==none then xmin=-1; endif;
		if xmax==none then xmax=1; endif;
		xx=x; yy=y; zz=z;
		if a<>none then xmin=a; 
			if b<>none then xmax=b;
				if c<>none then ymin=c;
					if d<>none then ymax=d;
					endif;
				endif;
			endif;
		endif;
	endif;
	
	if polar and r==none then r=1; endif;
	
	if fscale==-1 then 
		if typeof(r)==0 then fscale=r;
		else fscale=min(xmax-xmin,xmax-xmin)*0.5; 
		endif;
		fscale=fscale/getaspect();
	endif
	
	if contour and level==none then level="auto"; endif;
	if level!=none and hue==none then hue=-4; endif;
	if contour==2 and hue==none then hue=-1; endif;
	if user==1 then hold off; endif;
	wwire=wire;
	
	if typeof(x)==8 and typeof(y)==8 and y<>none and typeof(z)==8 and z<>none then
		if lines then
			t=linspace(xmin,xmax,n{1});
			xx=%ploteval(xx,t;args());
			yy=%ploteval(yy,t;args());
			zz=%ploteval(zz,t;args());
			wwire=1;
		else
			lx=linspace(xmin,xmax,n{1});
			ly=linspace(ymin,ymax,n{2});
			{xa,ya}=field(lx,ly);
			xx=%ploteval2(xx,xa,ya;args());
			yy=%ploteval2(yy,xa,ya;args());
			zz=%ploteval2(zz,xa,ya;args());
		endif;	
	endif
	
	if iscall(xx) then
		f=xx;
	else
		f=none;
	endif;
	
	if rotate then
		xxt=linspace(xmin,xmax,n{1});
		r=%ploteval(f,xxt;args());
		t=linspace(0,2*pi,n{2})';
		if rotate==1 then 
			xx=xxt; yy=cos(t)*r; zz=sin(t)*r;
		else
			zz=r; xx=cos(t)*xxt; yy=sin(t)*xxt;
		endif;
		f=none;
	endif;
	
	if transparent then color=255; hue=1; endif;
	
	vold=view();
	view(view); zoom(zoom); viewdistance(distance); 
	viewangle(angle); viewheight(height);
	
	vreset=view();
	v=view();	
	movelight=0;
	movecenter=0;
	{phil,psil,rl}=polar(light[1],light[2],light[3]);
	philorg=phil; psilorg=psil; rlorg=rl;
	
	if anaglyph then 
		agmode=1; deletepages(); addpage(); addpage();
	else 
		agmode=0; 
	endif;
	
	if spectral then
		if hue==false then hue=-4; endif;
		if color==none then color=-1-spectral; endif;
	endif;
	
	if xhue then hue=-2; endif;
	if yhue then hue=-3; endif;
	if zhue then hue=-4; endif;
	if hues!=none then hue=1; endif;
	
	xyzframe=none;
	if length(frame)>1 then xyzframe=frame; frame=1; endif;
	
	if f!=none and grid==none then 
		if !rotate and !contour and level==none and !hue and !implicit then 
			grid=10;
		endif;
	endif;
	if grid!=none and all(grid>0) then
		if typeof(xx)==2 and typeof(yy)==2 and typeof(zz)==2 then
			n=size(xx,yy,zz)-1;
		endif;
		vg=round(n/grid);
		oldgrid=subgrid(vg);
		n=floor(n/vg)*vg;
		oldhuegrid=huegrid(1);
	endif;
	
	if cpcolor==none then cpcolor=color; endif;
	if cplevel==none then cplevel=level; endif;
	if cplevel==none then cplevel="auto"; endif;
	
	if contour or level!=none then 
		if hue==none or hue==0 then hue=-4; endif;
	endif;
	
	illkey=false;
	
	repeat
		if add then h=holding(1); endif;
		if own then // user wants own plotting routine
			f(args());
		elseif f==none then // no function for x provided
			if yy==none and zz==none then // only one matrix
				zz=xx; xx=1:cols(zz); yy=(1:rows(zz))';
			endif;
			if zscale>0 then
				mx=(totalmax(zz)-totalmin(zz)) ..
					/max(totalmax(xx)-totalmin(xx),totalmax(yy)-totalmin(yy),epsilon);
				scale=[mx,mx,zscale]*scale;
			endif;
			if hue
				hc=huecolor(color);
				framedsolidhue(xx,yy,zz,hues,
					scale,1,frame,1,xlabel,ylabel,zlabel,level,values,
					light,amb,max,hue,contourcolor,contourwidth,
					frame=xyzframe,=sframe,=dl,=limits,
					=cp,=cplevel,=cpcolor,=cpdelta,=linewidth);
				huecolor(hc);
			else
				if wwire or lines then
					framedwire(xx,yy,zz,scale,frame,1,xlabel,ylabel,zlabel, ..
						wirecolor,lines,frame=xyzframe,=sframe,=linewidth);
				elseif points then
					ms=markerstyle(style);
					framedmark3(xx,yy,zz,scale,frame,1,xlabel,ylabel,zlabel,color, ..
						frame=xyzframe,=sframe);
					markerstyle(ms);
				else
					framedsolid(xx,yy,zz,scale,frame,1,disconnect, ..
						xlabel,ylabel,zlabel,sliced,bar, ..
						frame=xyzframe,=sframe, ..
						=cp,=cplevel,=cpcolor,=cpdelta,=linewidth);
				endif;
			endif
		else // function for x provided
			if hue
				if polar
					hc=huecolor(color);
					f3dplotpolarl(f,r,n,scale,fscale,light, ..
						amb,max,frame,xlabel,ylabel,zlabel,level,hue,maps;args(),
						=cp,=cplevel,=cpcolor,=cpdelta);
					huecolor(hc);
				else
					hc=huecolor(color);
					f3dplotlight(f,xmin,xmax,ymin,ymax,n,scale,fscale,light, ..
						amb,max,frame,xlabel,ylabel,zlabel,level,hue,maps, ..
						contourcolor,contourwidth,dl,limits;args(), ..
						=nc,=cp,=cplevel,=cpcolor,=cpdelta,=zlim);
					huecolor(hc);
				endif;
			else
				if polar
					f3dplotpolar(f,r,n,scale,fscale,frame, ..
						wire,xlabel,ylabel,zlabel,maps;args(), ..
						=cp,=cplevel,=cpcolor,=cpdelta);
				elseif implicit then
					xv=linspace(xmin,xmax,n{1});
					yv=linspace(ymin,ymax,n{2});
					if zmin==none then zmin=-1; endif;
					if zmax==none then zmax=1; endif;
					zv=linspace(zmin,zmax,n{3});
					f3dimplicit(f,xv,yv,zv,frame,
						xlabel,ylabel,zlabel,scale,implicit,maps;args(),
						=contourcolor,=contourwidth);
				else
					f3dplot(f,xmin,xmax,ymin,ymax,n,scale,fscale,frame, ..
						wire,xlabel,ylabel,zlabel,sliced,maps,wirecolor;args(), ..
						=cp,=cplevel,=cpcolor,=cpdelta);
				endif;
			endif;
		endif;
		settitle(title);
		if add then holding(h); endif;
		v=view();
		if agmode==1 then
			copytopage(1);
			setanaglyph(defaultanaglyphseparation/max(v[1],4));
			agmode=2;
		else
			if agmode==2 then
				copytopage(2); showpage(-1);
				setanaglyph(0);
				if anaglyph then agmode=1; else agmode=0; endif;
			endif;
			if !user then break; endif;
			if !illkey then
				pr=" angle="+print(deg(v[3]),0,0)+
					" height="+print(deg(v[4]),0,0)+
					" distance="+print(v[1],1,0)+
					" zoom="+print(v[2],1,0);
				ce=center();
				if any(ce!=0) then
					pr=pr+" center=["+print(ce[1],1,0)+
						"."+print(ce[2],1,0)+"."+print(ce[3],1,0)+"]"; 
				endif;
				if hue then 
					pr=pr+" light=["+print(light[1],1,0)+
						"."+print(light[2],1,0)+"."+print(light[3],1,0)+"]"; 
				endif;
			endif;
			illkey=false;

			k=key(pr);

			if k==13 or k==9 then break; endif;
			oc=center();
			if k==2 then
				if movelight then psil=psil-duser;
				elseif movecenter then oc[3]=oc[3]-duser;
				else v[4]=v[4]-duser;
				endif;
			elseif k==1 then 
				if movelight then psil=psil+duser;
				elseif movecenter then oc[3]=oc[3]+duser;
				else v[4]=v[4]+duser;
				endif;
			elseif k==4 then 
				if movelight then phil=phil+duser;
				elseif movecenter then
					oc[1]=oc[1]+cos(v[3])*duser;
					oc[2]=oc[2]+sin(v[3])*duser;
				else v[3]=v[3]+duser;
				endif
			elseif k==3 then
				if movelight then phil=phil-duser;
				elseif movecenter then
					oc[1]=oc[1]-cos(v[3])*duser;
					oc[2]=oc[2]-sin(v[3])*duser;
				else v[3]=v[3]-duser;
				endif
			elseif k==32 then 
				v=vreset;
				if hue then
					phil=philorg; psil=psilorg; rl=rlorg;
					{xl,yl,zl}=rect(phil,psil,rl);
					light=[xl,yl,zl];
				endif;
				movelight=0;
				movecenter=0;
				oc=resetcenter; _center(resetcenter);
			elseif k==7 then v[3]=0;
			elseif k==6 then v[4]=0;
			elseif k==43 then 
				v[1]=v[1]-duser; 
				if (v[1]<1) then v[1]=1; endif;
			elseif k==45 then v[1]=v[1]+duser;
			elseif k==ascii("a") and !anaglyph then
				agmode=1; deletepages(); addpage(); addpage(); 
			elseif k==ascii("l") then
				if hue then movelight=!movelight; endif;
			elseif k==ascii("c") then
				movecenter=!movecenter;
			elseif k==13 then 
				break;
			else
				illkey=true;
				pr="Press return to end, space to reset, "+
					"+/-, cursor keys, a(naglyph), c(enter), l(ight)";
				setstatus(pr);
			endif;
			if movelight then
				{xl,yl,zl}=rect(phil,psil,rl);
				light=[xl,yl,zl];
			elseif movecenter then
				center(oc);
			endif
		endif;
		view(v);
	end;
	if grid!=none and all(grid>0) then
		subgrid(oldgrid);
		huegrid(oldhuegrid);
	endif;

	if agmode then setanaglyph(0); endif;

	_center(oldcenter);
	view(vold);
	_wirecolor(oldwc);
	_frame3dcolor(oldfc);
	_fillcolor(fc);
	
	errors on;
	
	if oldwindow!=none then window(oldwindow); endif;
	
	if insimg then insimg(); endif;
endfunction

// * Support functions for Plots

// It should not be necessary to call these functions directly, unless
// a special effect is needed. You can use plot2d or plot3d instead.
// Have a look at the tutorials.

// There are also some core functions, which are not explained here.
// Look at the following documentation, if you need those.

// See: eulercore.html#Functions_for_Plots | Core Functions for Plots

// Moreover, some of the following functions link to core functions
// for further details.

logredraw=1;
subwindows=zeros([100,4]);
subwindowsfull=zeros([100,4]);
usesubwindows=0;

function overwrite window
## window(c1,r1,c2,r2) sets a plotting window.
## 
## The coordinates must be screen coordinates.
## window() returns the active window coordinates.
##
## See: coordinates
	global logredraw;
	if argn()==4 then 
		logredraw=1; return _window([arg1,arg2,arg3,arg4]);
	elseif argn()==1 then 
		logredraw=1; return _window(arg1);
	else 
		return _window();
	endif;
endfunction

function squarewindow ()
## Make the plot window square.
##
## This function is called by fullwindow() and shrinkwindow() with the
## >square parameter. It makes the window square in screen coordinates
## by retracting its larger dimension towards the middle of the
## window.
##
## See: fullwindow, shrinkwindow
	w=window();
	x0=w[1]; y0=w[2]; x1=w[3]; y1=w[4];
	h=min(y1-y0,x1-x0);
	my=(y0+y1)/2; y0=my-h/2; y1=my+h/2;
	mx=(x0+x1)/2; x0=mx-h/2; x1=mx+h/2;
	return _window([x0,y0,x1,y1]);
endfunction

function fullwindow (title=false, square=true)
## Takes the full size for the plots
##
## The plot window is the part that is actually used by plot2d and
## plot3d for graphs. For plot2d() it does not include the labels, the
## tick marks, or the heading.
##
## See: allwindow, shrinkwindow, reset, plotRGB
	global logredraw;
	h=textheight();
	logredraw=1;
	if title then
		w=_window([0,h*1.5,1023,1023]);
	else
		w=_window([0,0,1023,1023]);
	endif;
	if square then w=squarewindow(); endif;
	return w;
endfunction

function shrinkwindow (smaller=false, square=true)
## shrinkwindow() shrinks the window to allow labels.
##
## This sets the default plot window, leaving place for the labels at
## the left and the button, and for the title line. For more space,
## use fullwindow(). If you need larger labels at the y-axis, set
## >smaller. If you want to have an exact 1:1 square plot use >square.
##
## See: plotRGB, fullwindow, reset
	global logredraw;
	h=textheight(); b=textwidth();
	logredraw=1;
	if smaller then 
		x0=(14+smaller)*b; y0=2*h; x1=1023-0.5*b; y1=1023-2.5*h; 
	else 
		x0=10*b; y0=1.5*h; x1=1023-0.5*b; y1=1023-3*h;
	endif;
	w=_window([x0,y0,x1,y1]);
	if square then w=squarewindow(); endif;
	return w;
endfunction

function allwindow
## Takes all of the window for the plot
##
## See: fullwindow, shrinkwindow
	global logredraw;
	logredraw=1;
	return _window([0,0,1024,1024]);
endfunction

function overwrite setplot
## setplot(xmin xmax ymin ymax) sets the plot coordinates.
##
## Also setplot([xmin,xmax,ymin,ymax]).
##
## setplot() resets it. 
	if argn()==4 then 
		return _setplot([arg1,arg2,arg3,arg4]);
	elseif argn()==1 and typeof(arg1)==0 then
		return _setplot([-arg1,arg1,-arg1,arg1]);
	elseif argn()==0 then 
		scaling(1); 
	else
		_setplot(arg1);
	endif;
	return _plot();
endfunction

function overwrite aspect (w:positive number=none, 
	h:positive number=1, reset=true)
## Set the window aspect to w times h.
##
## This function reduces the plot window to a new window with
## proportions w times h. The font size remains the same as in the
## full window.
##
## If an aspect with larger width is active, insimg() (or the :) will
## insert only the active portion of the window.
##
## Without parameters, the function resets to the default window.
##
## Returns the current window ratio.
##
## Example:
##
## >aspect(2);
## >plot2d("sin(x)",0,2pi): 
## aspect();
##
## See: insimg
	if h==0 then error("Illegal Aspect"); endif;
	if w==none then _aspect(0);
	else _aspect(w/h);
	endif;
	if reset then shrinkwindow(); endif;
endfunction

function unclip
## Clips to the complete graphics screen
##
## See: clip
	return _clip([0,0,1023,1023]);
endfunction	

function overwrite clip
## clip(c1,r1,c2,r2) sets the clipping window. 
##
## The coordinates must be screen coordinates. clip() returns the
## active clipping window coordinates. The function can also be used
## with a 1x4 vector. It returns a 1x4 vector. Calls the built-in
## function _clip().
##
## See: unclip
	if argn()==4; return _clip([arg1,arg2,arg3,arg4]);
	else if argn()==1; return _clip(arg1);
	else return _clip();
	endif;
endfunction

function clipmore (x,more=0)
## Sets the clipping window.
## 
## The coordinates must be screen coordinates. There is some extra
## space around the area x clip() returns the active clipping window
## coordinates.
##
## See: clip
	return _clip([x[1]-more,x[2]-more,x[3]+more,x[4]+more]);
endfunction

function overwrite args toscreen (x,y=none)
## Convert the x-y-coordinates to screen coordinates
##
## Plot coordinates are generated by the last plot or by setplot().
## They reflect the x-y-positions of the points in the plot. Screen
## coordinates range from 0..1024 (even for non-quadratic aspect
## rations).
##
## The function calls the built-in function _toscreen([x,y]), which
## accepts and returns 1x2 vectors. With only one parameter, the
## function calls _toscreen() directly assuming a 1x2 vector. With two
## parameters, it assumes x and y separately, and returns {c,r}.
##
## See: aspect, setplot
	if y==none then return _toscreen(x);
	else
		c=x; r=y;
		loop 1 to prod(size(x))
			cr=_toscreen([x{#},y{#}]);
			c{#}=cr[1]; r{#}=cr[2];
		end;
		return {c,r};
	else error("Need x,y for toscreen");
	endif;
endfunction

function overwrite args fromscreen (c,r=none)
## Convert the screen coordinates v[1],v[2] to x-y-coordinates.
##
## fromscreen(c,r) : column and row in screen coordinates,
##   returns {x,y}
## [c,r] : use function with vectors, returns [x,y]
##
## Calls the built-in function _fromscreen(). For more explanation
## about coordinates see the function toscreen().
##
## See: toscreen
	if r==none then return _fromscreen(c);
	else
		x=c; y=c;
		loop 1 to prod(size(c))
			xy=_fromscreen([c{#},r{#}]);
			x{#}=xy[1]; y{#}=xy[2];
		end;
		return {x,y};
	else error("Need c,r for fromscreen");
	endif;
endfunction

function label (text, x:number, y:number, offset=15, ..
	color=none, ypos="l", xpos="r", pos=none, 
	scale=1/3, tcolor=0)
## Label the x-y-value with the text t.
##
## Labels a point in plot coordinates. The position of the label is
## by default at the lower right of the point. The label can be simple
## text or an RGB matrix, e.g. parsed from a Latex formula.
##
## text : String, vector of strings or RGB matrix.
##    A vector of strings is line by line. To get an RGB matrix of
##    color values from a Latex formula, use latex(expr).
##
## x,y : Position in plot coordinates.
## offset : In screen coordinates 0..1024.
## color : Color of the label.
## ypos : One of "u", "c", "l" (upper, center, lower)
## xpos : One of "l", "c", "r" (left, center, right)
## pos : Combination of ypos and xpos, such as "ur"
##
## Examples:
## >plot2d("x^2",r=2); label(["Ursprung","(0,0)"],0,0):
## 
## >function f(x) := x^3-x; ...
## >plot2d("f",r=2); ...
## >label("Min",1/sqrt(3),f(1/sqrt(3)),pos="lc"); ...
## >label("Max",-1/sqrt(3),f(-1/sqrt(3)),pos="uc"); ...
## >label(latex("y=x^3-x",factor=2),-1,1,pos="ur"):
##
## See: labelbox, latex
	c=_toscreen([x,y]); 
	yoffset=0;
	if strlen(pos)==2 then
		ypos=substring(pos,1,1);
		xpos=substring(pos,2,2);
	endif;
	if typeof(text)==8 or typeof(text)==13 then
	// label with text.
		if ypos=="u" then
			yoffset=cols(text)*(textheight()*1.2)+1.5*offset;
		elseif ypos=="c" then
			yoffset=cols(text)*(textheight()*1.2)/2+offset-textheight()*0.1;
		endif;
		if xpos=="r" then
			c[1]=c[1]+textwidth()/4+offset; 
		elseif xpos=="l" then
			c[1]=c[1]-textwidth()/4-offset; 
		endif;
		c[2]=c[2]-yoffset+offset;
		old=textcolor(color);
		loop 1 to length(text);
			if xpos=="r" then
				text(text{#},c);
			elseif xpos=="c" then
				ctext(text{#},c);
			else
				rtext(text{#},c);
			endif;
			c[2]=c[2]+textheight()*1.2;
		end;
		textcolor(old);
	elseif typeof(text)==2 then
	// label with a RGB image from a Latex formula
		px=pixel();
		w=abs(_toscreen([px[1]*cols(text), ..
			px[2]*rows(text)])-_toscreen([0,0]))*scale;
		if ypos=="u" then c[2]=c[2]-w[2]-offset;
		elseif ypos=="c" then c[2]=c[2]-w[2]/2;
		else c[2]=c[2]+offset;
		endif;
		if xpos=="l" then c[1]=c[1]-w[1]-offset;
		elseif xpos=="c" then c[1]=c[1]-w[1]/2;
		else c[1]=c[1]+offset;
		endif;
		c1=c+w;
		hd=holding(1); 
		plotrgb(text,[c[1],c[2],c1[1],c1[2]],=tcolor);
		holding(hd);
	else 
		error("Need string or rgb-matrix for label.");
	endif;
endfunction

function labelbox (labels, styles="-", colors:real=1,
	x=0.98, y=0.02, style="O#", color=0, 
	w=none, points=0, tcolor=none, left=0, wt=0.3,
	latex=0, scale=1, latexwidth=5)
## Plot a box with labels and line styles for the functions.
##
## A label box is a list of labels for each function in the plot. The
## box shows a string and a line in the style and color of the
## function for each function. For point plots the box can show point
## styles.
##
## labels : string, or vector of strings
## styles : string or string of vector, styles of the functions
## colors : real vector, colors of the functions
## w : width of the box (fraction of plot window)
## x, y : upper right corner of label box (fractions)
## points : flag or vector of flags.
## color : background color
## style : background style
## left : align at the top left corner (default is top right)
## wt : fraction of the width for the text
## latex : parse each label through Latex and plot the formulas
## scale : take higher lines than the normal text height
## latexwidth : We assume that Latex formulas are 5 characters wide
##
## For a transparent box set style="t" and color=black for a black
## boundary. Set color=transparant for no boundary.
##
## >plot2d("x+x^3",-1,1); plot2d("x^4",color=blue,style="--",>add);
## >labelbox(["x+x^3","x^4"],colors=[black,blue],styles=["-","--"]):
##
## See: label

	ts=textsize(); 
	win=window(); ww=win[3]-win[1]; wh=win[4]-win[2];
	th=ts[2]*scale; h=length(labels)*(th*1.2)+th*0.8;
	
	if latex then
		twidth=ts[1]*latexwidth;
	else
		lmax=0;
		loop 1 to length(labels); 
			lmax=max(lmax,strlen(labels{#}));
		end;
		twidth=(ts[1]+0.5)*lmax;
	endif;	
	
	if w==none then
		if all(points) then w=twidth+ts[1]*6;
		else 
			w=twidth+ts[1]*15;
			twidth=w*wt;
		endif;
	else
		w=ww*w;
		if all(points) then twidth=w-6*ts[1];
		else twidth=w*wt;
		endif;
	endif;
		
	if left
		x=win[1]+ww*x; y=win[2]+wh*y;
	else
		x=win[1]+ww*x-w; y=win[2]+wh*y;
	endif;

	global transparent;
	if color!=transparent then	
		bst=barstyle(style); bcol=barcolor(color);
		bar([x,y,w,h]);
		barstyle(bst); barcolor(bcol);
	endif;
	
	xt=x+w-ww/60-twidth; yt=y+th/2;
	loop 1 to length(labels);
		tc=textcolor(tcolor{#});
		if latex then 
			form=latex(labels{#});
			px=abs(_toscreen(pixel())-_toscreen([0,0]));
			hl=px[2]*rows(form)/3;
			text(form,[xt,yt+(th-hl)/2]);
		else 
			text(labels{#},[xt,yt]);
		endif;
		textcolor(tc);
		yt=yt+th*1.2;
	end;
	
	xt=x+ww/60; yt=y+th;
	loop 1 to length(labels);
		if points{#} then
			lc=color(colors{#}); ms=markerstyle(styles{#});
			v=fromscreen([x+w-ww/30-twidth-ts[1],yt]); 
			hold on; mark(v[1],v[2]); hold off; 
			color(lc); markerstyle(ms); 
			yt=yt+th*1.2;
		else
			lc=color(colors{#}); ls=linestyle(styles{#});
			v1=fromscreen([xt,yt]);
			v2=fromscreen([x+w-ww/30-twidth-ts[1],yt]); 
			hold on; plot([v1[1],v2[1]],[v1[2],v2[2]]); hold off; 
			color(lc); linestyle(ls); 
			yt=yt+th*1.2;
		endif;
	end;
endfunction

function textbox (s, x=0.98, y=0.02, 
	style="O#", color=0, tcolor=none, w:real=none, 
	left=0, spaces=2, scale=1/3, center=false)
## Plot a box with text.
##
## x,y : 
## An anchor for the upper left corner of the box (fractions of plot
## area)
##
## s : A string or a vector of strings, one for each line of text
## tcolor : Color or an array of colors for the text lines
## w : an optional width of the box (fraction of plot area)
## left : align at the left edge (default is right edge)
## spaces : add some space to adjust for short strings
## center : center text in its box
##
## For styles and colors see labelbox().
##
## >function f(x) &= x^3-x; ...
## >plot2d(f,r=2); ...
## >textbox(latex(&f(x),factor=2,color=green),0.4,0.2):
## 
## See: label, labelbox
	if typeof(s)==8 or typeof(s)==13 then
		lmax=0;
		loop 1 to length(s); lmax=max(lmax,strlen(s{#})); end;
		ts=textsize(); twidth=(ts[1]+0.5)*(lmax+spaces);
		
		win=window(); ww=win[3]-win[1]; wh=win[4]-win[2];
		th=ts[2]; h=length(s)*(th*1.2)+th*1.2;
		
		if w==none then w=twidth; 
		else w=ww*w;
		endif;
		
		if left
			x=win[1]+ww*x; y=win[2]+wh*y;
		else
			x=win[1]+ww*x-w; y=win[2]+wh*y;
		endif;
	
		global transparent;
		if color!=transparent then		
			bst=barstyle(style); bcol=barcolor(color);
			bar([x,y,w,h]);
			barstyle(bst); barcolor(bcol);
		endif
		
		xt=x+th*0.6; yt=y+th*0.6;
		loop 1 to length(s);
			tc=textcolor(tcolor{#});
			if center then
				text(s{#},[x+w/2,yt],align=0);
			else
				text(s{#},[xt,yt]);
			endif
			textcolor(tc);
			yt=yt+th*1.2;
		end;
	elseif typeof(s)==2 then
		
		win=window(); ww=win[3]-win[1]; wh=win[4]-win[2];		
		x=win[1]+ww*x; y=win[2]+wh*y;
		
		px=abs(_toscreen(pixel())-_toscreen([0,0]));
		margin=textheight()*0.6;
		if w==none then w=px[1]*cols(s)*scale+2*margin;
		else w=ww*w;
		endif;
		h=px[2]*rows(s)*scale+2*margin;		

		if not left then x=x-w; endif;

		global transparent;
		if color!=transparent then		
			bst=barstyle(style); bcol=barcolor(color);
			bar([x,y,w,h]);
			barstyle(bst); barcolor(bcol);
		endif
		
		if center then
			text(s,x+margin+ww/2,y+margin,align=-1,=scale,align=0);
		else
			text(s,x+margin,y+margin,align=-1,=scale);
		endif;
		
	else
		error("String, string vector or RGB image expected in labelbox()");
	endif;		
endfunction

function plotbar1 (x:number,y:number,w:number,h:number)
## Plots a single bar rectangle with x,y,w,h in plot coordinates.
##
## Use barstyle() and barcolor() to modify the bar.
##
## See: bar
	a=_toscreen([x,y+h]);
	b=_toscreen([x+w,y]);
	bar(a|(b-a));
	return 0;
endfunction

function plotbar (x,y,w,h,color=none,st=none)
## Plots bars with x,y,w,h in plot coordinates.
##
## The parameters can be vectors.
##
## x,y : lower left coordinate
## w,h : width and height
## color : color of bars
## style : style of bars
##
## See: bar
	if !holding() then clg; endif;
	if color!=none then bc=barcolor(color); endif;
	if st!=none then bs=barstyle(st); endif;
	oldclip=clipmore(window());
	map("plotbar1",x,y,w,h);
	if color!=none then barcolor(bc); endif;
	if st!=none then barstyle(bs); endif;
	clip(oldclip);
	return plot();
endfunction

function barclear ([x,y,w,h],color=0,style="O")
## Clear a region in screen coordinates
##
## This can also be used to draw rectangles with specific style and
## color. The styles and colors will be reset after the plot.
##
## See: bar
	bc=barcolor(color); bs=barstyle(style);
	bar([x,y,w,h]);
	bc=barcolor(bc); bs=barstyle(bs);
endfunction

function xplotbar (x,y,w,h,st:string="#O")
## Plots a bar with x,y,w,h in plot coordinates.
##
## Uses the plot coordinates from the last plot! Use setplot() to set
## new plot coordinates. Draws coordinates below the bar. use
## barstyle() and barcolor() to modify the bar.
##
## See: plot2d
	if !holding() then clg; endif;
	frame(); xplot();
	stold=barstyle(st);
	hold; plotbar(x,y,w,h); hold;
	barstyle(stold);
	return plot();
endfunction;

function xplotrange (r:real vector, v:real vector)
## Plots a bar plot of the multiplicities v[i] in the ranges r[i],r[i+1]. 
##
## Obsolete. Use plot2d with the histogram=1 option.
##
## See: plot2d
	s=scaling(1); 
	if s;	
		setplot(min(r),max(r),0,max(v));
	endif;
	scaling(s);
	n=cols(v);
	xplotbar(r[1:n],0,r[2:n+1]-r[1:n],v);
	return plot();
endfunction

function scalematrix (A)
## Scales a matrix A, so that its value are in the range from 0 to 1.
	e=extrema(A)'; mi=min(e[1]); ma=max(e[3]);
	if ma~=mi; ma=mi+1; endif;
	return (A-mi)/(ma-mi);
endfunction	

function args select
## Coordinates {x,y} of mouse clicks
##
## Stops if user clicks above the window. The function returns vectors
## {x,y} of the clicked coordinates.
##
## See: mouse
	p=plot();
	x=[];
	repeat
		m=mouse();
		until m[2]>p[4];
		h=holding(1); mark(m[1],m[2]); holding(h);
		x=x_m;
	end;
	x=x'; 
	return {x[1],x[2]}
endfunction

// * Plotting Text

function textheight
## Height of a letter.
	h=textsize();
	return h[2];
endfunction

function textwidth
## Width of a letter.
	h=textsize();
	return h[1];
endfunction

defaultnticks=5;

function ticks (a=0, b=none, finer=false, vertical=false, n=none)
## Ticks to be used for intervals [a,b].
##
## This function is used to compute the ticks for plots. It tries to
## show ticks of major decimal values, but not too many or too few.
## The target number of ticks is 5, but for small windows this
## number is reduced.
##
## a,b : Plot range
## finer : compute for the fine grid in grid=8
## vertical : compute grid for vertical plots (respect aspect ratio)
## n : target number of ticks
##
## The default for n is in the variable defaultnticks. Start value is
## 5. 
##
## See: xplot, ygrid, xgrid
	w=window();
	global defaultnticks;
	if n==none then n=defaultnticks; endif;
	if vertical and max(w[4]-w[2])<600*getaspect() then n=3; endif;
	if !vertical and max(w[3]-w[1])<600 then n=3; endif;
	if vertical and max(w[4]-w[2])<400*getaspect() then n=2; endif;
	if !vertical and max(w[3]-w[1])<400 then n=2; endif;
	if b==none and cols(a)>1 then b=max(a); a=min(a); endif;
	if b>1e30 then b=1e30; endif;
	if a<-1e30 then a=-1e30; endif;
	if a>=b then b=a+1; endif;
	tick=10^floor(log(b-a)/log(10)-0.3);
	nticks=floor((b-a)/tick);
	semi=false;
	if nticks<n then tick=tick/2; semi=true;
	elseif nticks>2*n then tick=tick*5; semi=true;
	endif
	if finer then 
		if semi then 
			tick=tick/5; 
		else
			tick=tick/5; 
		endif;
	endif;
	tick1=tick;
	ma=max(abs(a),abs(b));
	if ma>1e-4 and ma<1e6 then
		tick=1; 
	else
		if semi then tick1=tick1*2; tick=tick*2; endif;
	endif;
	res=(round(a/tick1,0):round(b/tick1,0))*tick1;
	return {res[nonzeros(res>=a-tick1*0.1 && res<=b+tick1*0.1)],tick};
endfunction

function xplot (x=0,y=0,grid=2,ticks=1,frame=1)
## Works like plot, but shows axis ticks.
##
## This function is called by plot2d(). The user should prefer to call
## plot2d(). xplot() without parameters shows only axis ticks and the
## grid.
##
## See: plot, plot2d
	if grid==0 then
		if frame then frame(); endif;
		return;
	endif;
	if argn()>0 then
		if argn()==1 then
			if iscomplex(x) then y=im(x); xh=re(x);
			else y=x; xh=1:cols(y);
			endif;
		else; xh=x;
		endif;
		p=plotarea(xh,y);
		if !holding() and frame then clg; frame(); endif;
	else
		p=plot();
	endif;
	lw=linewidth(1);
	if grid==7 or grid==8 then
		{t,f}=ticks(p[1],p[2],<vertical);
		uy=0; pl=plot(); top=false;
		if uy<pl[3] then uy=pl[3]-(pl[4]-pl[3])/100;
		elseif uy>pl[4] then uy=pl[4]+(pl[4]-pl[3])/100; top=true;
		endif;
		ux=0; left=true;
		if ux<pl[1] then ux=pl[1]-(pl[2]-pl[1])/100;
		elseif ux>pl[2] then ux=pl[2]+(pl[2]-pl[1])/100; left=true;
		endif;
		xaxis(uy,t,style="-",ticks=3,=top);
		{t,f}=ticks(p[3],p[4],>vertical);
		yaxis(ux,t,style="-",ticks=3,=left);
		if grid==8 then
			{t,f}=ticks(p[1],p[2],>finer,<vertical);
			xaxis(uy,t,style="-",ticks=1,<labels,=top);
			{t,f}=ticks(p[3],p[4],>finer,>vertical);
			yaxis(ux,t,style="-",ticks=1,<labels,=left);
		endif;			
	else
		ylevel=none;
		xlevel=none;
		if grid==3 then
			grid=1;
			plevel=_toscreen([0,0]);
			xlevel=plevel[1];
			ylevel=plevel[2];
			w=window();
			if xlevel<w[1] or xlevel>w[3] then xlevel=none; endif;
			if ylevel<w[2] or ylevel>w[4] then ylevel=none; endif;
		endif;
		{t,f}=ticks(p[1],p[2],<vertical);
		xgrid(t,f,grid,ticks,ylevel=ylevel);
		{t,f}=ticks(p[3],p[4],>vertical);
		ygrid(t,f,grid,ticks,xlevel=xlevel);
	endif;
	linewidth(lw);
	if argn()>0 then
		ho=holding(1); plot(xh,y); holding(ho);
	endif;
	if frame then frame(); endif;
	return p;
endfunction

function xmark (x=0,y=0,grid=2,ticks=1)
## Works like mark, but shows axis ticks.
##
## xmark() shows only axis ticks and the grid.
##
## See: mark, plot2d
	if !holding(); clg; frame(); endif;
	if argn()==1;
		if iscomplex(x); y=im(x); xh=re(x);
		else; y=x; xh=1:cols(y);
		endif;
	else; xh=x;
	endif;
	p=plotarea(xh,y);
	{t,f}=ticks(p[1],p[2],<vertical);
	xgrid(t,f,grid,ticks);
	{t,f}=ticks(p[3],p[4],>vertical);
	ygrid(t,f,grid,ticks);
	ho=holding(1); p=mark(xh,y); holding(ho);
	return p;
endfunction

function setplotm
## The user can choose the plotting coordinates with the mouse.
##
## Returns plot coordinates.
	h=holding(1);
	k1=mouse(); mark(k1[1],k1[2]);
	k2=mouse(); mark(k2[1],k2[2]);
	kl=min(k1,k2); ku=max(k1,k2);
	c=color(2);
	plot([kl[1],kl[1],ku[1],ku[1],kl[1]],[kl[2],ku[2],ku[2],kl[2],kl[2]]);
	color(c);
	setplot(kl[1],ku[1],kl[2],ku[2]);
	holding(h);
	return plot();
endfunction

function %printscale (x)
	if (abs(x)>10000) || (abs(x)<0.00001);
		return printf("%12.5e",x);
	else
		return printf("%10.5f",x);
	endif;
endfunction

function niceform (x,n=10,f=1)
## Return a string, containing a nicely formatted of x
	if isstring(x) then return x; endif;
	y=round(x/f,n);
	if abs(y)<10^(-10); y=0; endif;
	return printf("%g",y);
endfunction

defaultgrid=2;
defaultgrid1="--";
defaultgrid2=".";
defaultgridcolor=3;
defaulttextcolor=2;
setglobal defaultgrid,defaultgrid1,defaultgrid2;
setglobal defaultgridcolor,defaulttextcolor;

function gridstyle (st1:string=none, st2:string=none, 
	color=none, textcolor=none, framecolor=none, grid=none)
## Set the grid styles and colors and the frame color.
##
## st1 : axis lines
## st2 : other grid lines
## color : grid color
## textcolor : default text color for plots
## framecolor : frame color
## grid : Number for grid styles (see plot2d)
##
## See: linestyle, rgb, plot2d
	if st1!=none then defaultgrid1=st1; endif;
	if st2!=none then defaultgrid2=st2; endif;
	if color!=none then defaultgridcolor=color; endif;
	if textcolor!=none then defaulttextcolor=textcolor; endif;
	if framecolor!=none then _framecolor(framecolor); endif;
	if grid!=none then defaultgrid=grid; endif;
endfunction

function gridcolor (c:integer=3)
## Set the grid color.
## Returns the current color
## See plot2d,rgb
	res=defaultgridcolor;
	defaultgridcolor=c;
	return res;
endfunction

function comment fillcolor ([color1, color2])
## Set the fill color for both sides of 3D plots.
##
## fillcolor(none) or fillcolor() simply returns the old colors as a
## 1x2 vector of colors.
endfunction

function xgrid (xx, f=1, grid=2, ticks=1, color=none, ..
	xt=none, ylevel=none, textcolor=none)
## Draws vertical grid lines on the plot window at x0,x1,...
##
## xgrid([x0,x1,...],f) additionally writes x0/f to the axis.
##
## f : factor (like 10^5)
## grid : 2 for normal grid (see plot2d)
## ticks : write labels below the axis
## color : tick color
## xt : use these numbers for the grid
## ylevel : draw the grid at this y level
##
## The default values for the grid styles are defaultgrid1,
## defaultgrid2,defaultgridcolor.
##
## See: ygrid, plot2d
	if color==none then color=defaultgridcolor; endif;
	if textcolor==none then textcolor=defaulttextcolor; endif;
	if grid==5 then c=_fullplotarea();
	else c=_plot(); 
	endif;
	n=cols(xx); s=scaling(0); h=holding(1);
	w=window();
	st=linestyle(defaultgrid2); 
	wi=linewidth(1); 
	oldcolor=color(color); oldtextcolor=textcolor(textcolor);
	ht=textheight();
	if xt==none then xt=xx; endif;
	str="";
	ee=(c[2]-c[1])/100;
	loop 1 to n;
		x=xx[#];
		col=toscreen([x,c[3]])[1];
		if x<=c[2]+ee and x>=c[1]-ee then
			if grid<>4 then
				if grid!=9 and abs(x)<1e-10 then
					linestyle(defaultgrid1); 
					_plot([x,x],[c[3],c[4]]); 
					linestyle(defaultgrid2);
				elseif grid==2 or grid==5 then
					_plot([x,x],[c[3],c[4]]); 					
				endif
				if grid==6 or grid==9 then
					linestyle("-");
					{xw,yw}=fromscreen(col,w[2]);
					_plot([x,x],[yw,yw-(c[4]-c[3])/50]);
					{xw,yw}=fromscreen(col,w[4]);
					_plot([x,x],[yw,yw+(c[4]-c[3])/50]);
					linestyle(defaultgrid2);
				endif;				
			endif;		
			if ticks and grid then
				stra=niceform(xt[#],=f);
				if stra!=str or #==n then
					if ylevel==none	then
						_ctext(stra,[col,w[4]+0.4*ht]);
					else
						_ctext(stra,[col+0.5*ht,ylevel+0.4*ht]);
					endif;
					str=stra;
				endif
			endif;
		endif;
	end;
	if ticks and grid and !(f~=1) then
		_rtext("* "|printf("%0.0e",f),[w[3],w[4]+1.5*ht]);
	endif;
	linestyle(st); linewidth(wi); color(1);
	color(oldcolor); textcolor(oldtextcolor);
	holding(h); scaling(s);
	return 0;
endfunction

function xtick (x:real vector, s=none, latex=false)
## Set non-conformal ticks to the x-axis
##
## x : real or real vector
## s : string, string vector or RGB image (if x is real)
## latex : parse string in s through LaTeX
##
## See: xgrid, xaxis, latex
	loop 1 to cols(x)
		if s==none then str=niceform(x{#}); d=0.4;
		elseif latex then str=latex(s{#}); d=0.6;
		else str=s{#}; d=0.4;
		endif;
		{c,r}=toscreen(x{#},0);
		wi=window();
		text(str,c,wi[4]+d*textheight(),>center,scale=1/3);
	end;
endfunction

function ygrid (yy, f=1, grid=2, ticks=1, color=none, ..
	yt=none, xlevel=none, textcolor=none, vertical=none)
## Draws horizontal grid lines on the plot window at y0,y1,...
##
## ygrid([x0,x1,...],f) additionally writes x0/f to the axis.
## Parameters similar to xgrid().
##
## >plot2d("x^3-x",<grid); plot()
##  [-2,  2,  -6,  6]
## >ygrid(ticks(-6,6,n=4));
## >xgrid(-2:2);
## 
## See: xgrid, plot2d
	if color==none then color=defaultgridcolor; endif;
	if textcolor==none then textcolor=defaulttextcolor; endif;
	global verticallabels;
	if vertical==none then vertical=verticallabels; endif;
	if grid==5 then c=_fullplotarea();
	else c=_plot(); 
	endif;
	n=cols(yy); s=scaling(0); h=holding(1);
	st=linestyle(defaultgrid2); 
	wi=linewidth(1); 
	oldcolor=color(color); oldtextcolor=textcolor(textcolor);
	w=window(); wd=textwidth(); ht=textheight();
	if yt==none then yt=yy; endif;
	ee=(c[4]-c[3])/100;
	loop 1 to n;
		y=yy[#];
		row=toscreen([c[1],y])[2];
		if y>=c[3]-ee  and y<=c[4]+ee then
			if grid<>4 then
				if grid!=9 and abs(y)<1e-10 then
					linestyle(defaultgrid1); 
					_plot([c[1],c[2]],[y,y]); 
					linestyle(defaultgrid2);
				elseif grid==2 or grid==5 then
					_plot([c[1],c[2]],[y,y]); 
				endif
				if grid==6 or grid==9 then
					linestyle("-");
					{xw,yw}=fromscreen(w[1],row);
					_plot([xw,xw+(c[2]-c[1])/50],[y,y]); 	
					{xw,yw}=fromscreen(w[3],row);
					_plot([xw,xw-(c[2]-c[1])/50],[y,y]); 	
					linestyle(defaultgrid2);
				endif;
			endif;		
			if ticks and grid then
				if typeof(yt)==8 or typeof(yt)==13 then str=yt{#};
				else str=niceform(yt[#],=f);
				endif;
				if xlevel==none then
					if vertical==2 then
						_vcutext(str,[w[1]-1.5*ht,row]);
					else
						_rtext(str,[w[1]-wd,row-ht/2]);
					endif;
				elseif not y~=0 then
					if vertical==2 then
						_vctext(str,[xlevel-0.5*ht,row]);
					else
						_text(str,[xlevel+0.5*ht,row-ht/2]);
					endif;
				endif
			endif;
		endif;
	end;
	if ticks and grid and !(f~=1) then
		_text("* "|printf("%0.0e",f),[w[1]-6*wd,0]);
	endif;
	linestyle(st); linewidth(wi); color(1); 
	color(oldcolor); textcolor(oldtextcolor);
	holding(h); scaling(s);
	return 0;
endfunction

function yaxis (x, yy=none, yt=none, 
	color=none, textcolor=none, style=none, gridstyle=none,
	grid=false, vertical=false, left=true, axis=true,
	zero=false, ticks=1, labels=true)
## Draws a y-axis to the plot window
##
## This function manually draws an axis with labels and grid lines
## vertical to the axis.
##
## x : x-coordinate of the axis
## yy : y-axis ticks of the axis (none: no ticks)
## yt : labels of axis ticks (real or string vector, none: take yy)
## color,style : color and style of axis
## textcolor : color of labels
## gridstyle : style of grid lines (rectangular to axis)
## grid : draw grid lines (true or false)
## vertical : draw labels vertically
## left : draw labels left of axis
## axis : draw axis line (true or false)
## zero : draw the label for 0
## ticks : draw small ticks at the labels
##
## >fullwindow(); setplot(0,1.05,0,1.1); clg;
## >plot2d("exp(-x)",0,1,>add);
## >xaxis(0,0:0.1:1,style="->");
## >yaxis(0,0.1:0.1:1,style="->",>left);
## >yaxis(1,style=".",>grid);
## >yaxis(1.05,[1,0.5,0.25],>grid,<axis);
##
## See: xaxis
	if color==none then color=defaultgridcolor; endif;
	if textcolor==none then textcolor=defaulttextcolor; endif;
	if style==none then style=defaultgrid1; endif;
	if gridstyle==none then gridstyle=defaultgrid2; endif;
	if grid==5 then c=_fullplotarea();
	else c=_plot(); 
	endif;
	s=scaling(0); h=holding(1);
	st=linestyle(style); 
	wi=linewidth(1); 
	oldcolor=color(color); 
	oldtcolor=textcolor(textcolor);
	w=window(); wd=textwidth(); ht=textheight();
	if yt==none then yt=yy; endif;
	ee=(c[4]-c[3])/1000;
	xlevel=toscreen([x,0])[1];
	if axis then
		linestyle(style); 
		_plot([x,x],[c[3],c[4]]); 
	endif
	if yy<>none then
		n=cols(yy); 
		tickl=(c[2]-c[1])/100*ticks/getaspect();
		loop 1 to n;
			y=yy[#];
			row=toscreen([c[1],y])[2];
			if y>=c[3]-ee and y<=c[4]+ee and (zero || !(y~=0)) then
				if grid then
					linestyle(gridstyle);
					_plot([c[1],c[2]],[y,y]); 
				endif;		
				if typeof(yt)==8 or typeof(yt)==13 then str=yt{#};
				elseif typeof(yt)==1 then str=yt;
				else str=niceform(yt[#]);
				endif;
				if vertical then
					disp=0.5; if left then disp=-1.5; endif;
					text(str,xlevel+disp*ht,row,>vertical,>center);
					if ticks then
						ls=linestyle("-");
						plot([x,x+tickl],[y,y]);
						linestyle(ls);
					endif;
				elseif not left
					if labels then 
						text(str,xlevel+0.5*ht,row-ht/2); 
					endif;
					if ticks then
						ls=linestyle("-");
						plot([x-tickl,x],[y,y]);
						linestyle(ls);
					endif;
				else
					if labels then 
						text(str,xlevel-0.5*ht,row-ht/2,>right); 
					endif;
					if ticks then
						ls=linestyle("-");
						plot([x,x+tickl],[y,y]);
						linestyle(ls);
					endif;
				endif;
			endif;
		end;
	endif;
	linestyle(st); linewidth(wi); color(1);
	color(oldcolor); textcolor(oldtcolor);
	holding(h); scaling(s);
endfunction

function xaxis (y, xx=none, xt=none, 
	color=none, textcolor=none, style=none, gridstyle=none,
	grid=false, top=false, axis=true,
	zero=false, ticks=1, labels=true)
## Draws a y-axis to the plot window
##
## This function is similar to yaxis().
##
## top : labels on top of the axis
##
## See: yaxis
	if color==none then color=defaultgridcolor; endif;
	if textcolor==none then textcolor=defaulttextcolor; endif;
	if style==none then style=defaultgrid1; endif;
	if gridstyle==none then gridstyle=defaultgrid2; endif;
	if grid==5 then c=_fullplotarea();
	else c=_plot(); 
	endif;
	s=scaling(0); h=holding(1);
	st=linestyle(style); 
	wi=linewidth(1); 
	oldcolor=color(color); 
	oldtcolor=textcolor(textcolor);
	w=window(); wd=textwidth(); ht=textheight();
	if xt==none then xt=xx; endif;
	ee=(c[2]-c[1])/1000;
	ylevel=toscreen([0,y])[2];
	if axis then
		linestyle(style); 
		_plot([c[1],c[2]],[y,y]); 
	endif;
	if xx<>none then
		n=cols(xx); 
		tickl=(c[4]-c[3])/100*ticks;
		loop 1 to n;
			x=xx[#];
			row=toscreen([x,c[3]])[1];
			if x>=c[1]-ee and x<=c[2]+ee and (zero || !(x~=0)) then
				if grid then
					linestyle(gridstyle);
					_plot([x,x],[c[3],c[4]]); 
				endif;		
				if typeof(xt)==8 or typeof(xt)==13 then str=xt{#};
				elseif typeof(xt)==1 then str=xt;
				else str=niceform(xt[#]);
				endif;
				if not top
					if labels then 
						text(str,row,ylevel+0.4*ht,align=0); 
					endif;
					if ticks then
						ls=linestyle("-");
						plot([x,x],[y,y+tickl]);
						linestyle(ls);
					endif;
				else
					if labels then _ctext(str,[row,ylevel-1.4*ht]); endif;
					if ticks then
						ls=linestyle("-");
						plot([x,x],[y-tickl,y]);
						linestyle(ls);
					endif;
				endif;
			endif;
		end;
	endif;
	linestyle(st); linewidth(wi); color(1);
	color(oldcolor); textcolor(oldtcolor);
	holding(h); scaling(s);
endfunction

function xrange()
## Writes the range of x below the x axis
	c=plot(); s=scaling(0); h=holding(1);
	w=window();
	ht=textheight();
	text(printf("%0.10g",c[1]),[w[1],w[4]+0.2*ht]);
	rtext(printf("%0.10g",c[2]),[w[3],w[4]+0.2*ht]);
	holding(h); scaling(s);
endfunction

function xlabel (s, x:real=none, color=none, tcolor=0)
## Puts the label text at the x-axis
##
## s : string, string vector or RGB image
## x : if present, x position of the label
##
## >plot2d("sin(2pi*x)",0,1); ...
## >xlabel(u"&phi;"); ...
## >ylabel(u"sin(&phi;)"):
##
## See: text, ylabel, plot2d
	w=window();
	ht=textheight();
	c=(w[1]+w[3])/2;
	if x!=none then {c,r}=toscreen(x,0); endif;
	text(s,c,w[4]+1.5*ht,align=0,=color,=tcolor);
endfunction

function yrange()
## Writes the range of y besides the y axis.
	c=plot(); s=scaling(0); h=holding(1);
	w=window();
	wd=textwidth();
	ht=textheight();
	rtext(niceform(c[4]),[w[1]-wd/2,w[2]]);
	rtext(niceform(c[3]),[w[1]-wd/2,w[4]-ht]);
	holding(h); scaling(s);
endfunction

function ylabel (s,y=none,edge=none,vertical=none,
	color=none,tcolor=0)
## Puts the label text at the y-axis
##
## edge : align with left of screen window
## vertical : vertical text (on by default)
##
## See: xlabel
	global usesubwindows,subwindowsfull;
	w=window();
	wd=textwidth();
	ht=textheight();
	global verticallabels;
	if vertical==none then vertical=verticallabels; endif;
	if edge==none then edge=true; endif;
	r=(w[2]+w[4])/2;
	if y!=none then {c,r}=toscreen(0,y); endif;
	col=5;
	if usesubwindows then col=subwindowsfull[usesubwindows,1]; endif;
	if vertical then
		if edge then
			text(s,col,r,vertical=1,align=0,=color,=tcolor);
		else
			text(s,w[1]-8*wd+5,r,vertical=1,align=0,align=0,=color,=tcolor);
		endif
	else
		if edge then
			text(s,col,r,align=-1,=color,=tcolor);
		else
			text(s,w[1]-7*wd+5,r,align=0,=color,=tcolor);
		endif;
	endif;
endfunction

function plotwindow
## Sets the plot window to the screen coordinates.
##
## This function makes the screen coordinates and the plot coordinates
## agree.
	w=window();
	setplot(w[1],w[3],w[2],w[4]);
	return plot()
endfunction

function getwindowfromplot
## Get the window coordinates from the plot coordinates
##
## This is useful for plotrgb.
##
## >getwindowfromplot()
	if argn()==0 then v=plot();
	elseif argn()==4 then v=[arg1,arg2,arg3,arg4];
	else v=arg1
	endif;
	{c1,r1}=toscreen(v[1],v[4]); 
	{c2,r2}=toscreen(v[2],v[3]); 
	return [c1,r1,c2,r2];
endfunction

// Windows

function upperwindow (title="")
## Select the upper half of the plot window for a plot
## See: figure
	window(150,860,75,500);
	title(title);
	return title
endfunction

function lowerwindow (title="")
## Select the lower half of the plot window for a plot
## See: figure
	window(150,860,575,1010);
	ctext(title,512,510);
	return title
endfunction

// * Adaptive Evaluation for Plots

function args adaptiveeval (f$:call, g$:call, a, b,
	eps=0.01, amin=1e-5, amax=1e5)
## Compute f$(x), g$(x) for t in [a,b] with adaptive step size.
##
## eps is the target accuracy.
## amin and amax are the minimal and maximal step size.
## amax is also the initial step size.
## f and g can be expressions in x or user defined functions.
## Returns {x,y}
##
## See: plot2d, adaptiveevalone, adaptive
// other direction
	if b<a then return adaptiveeval(f$,g$,b,a,eps,amin,amax); endif;
	t=a;
	sx=f$(t;args());
	sy=g$(t;args());
	x=zeros(rows(sx),1000); y=zeros(rows(sy),1000); 
	if cols(sx)>1 then 
		error(f$|" does not produce a real or column vector"); 
	endif
	if cols(sy)>1 then 
		error(g$|" does not produce a real or column vector"); 
	endif
	n=1; h=1;
	x[:,1]=sx; y[:,1]=sy;
	repeat
		repeat
			tn=t+h;
			if tn>b then tn=b; endif;
			sxn=f$(tn;args());
			syn=g$(tn;args());
			d=sqrt((sxn-sx)^2+(syn-sy)^2);
			if any(isNAN(d)) then break; endif;
			if d>eps and h>amin then h=max(h/2,amin);
			elseif d<eps/2 then h=min(h*2,amax); break;
			else break;
			endif;
		end;
		n=n+1;
		if n>cols(x) then
			x=x|x;
			y=y|y;
		endif;
		x[:,n]=sxn; y[:,n]=syn;
		sx=sxn; sy=syn;
		t=tn;
		if t>=b; then break; endif;
	end;
	return {x[:,1:n],y[:,1:n]};
endfunction

function %ploteval (f$:call, x:vector)
	y0=f$(x[1],args());
	if cols(y0)>1 then 
		error(f$|" does not produce a real or column vector"); 
	endif
	y=zeros(rows(y0),cols(x));
	y[:,1]=y0;
	loop 2 to cols(x)
		y[:,#]=f$(x[#],args());
	end
	return y;
endfunction
	
function map %mapexpression2 (x:scalar, y:scalar; expr:string)
	return expr(x,y;args());
endfunction
	
function %ploteval2 (f$:call, x:numerical, y:numerical, maps=0)
	if isfunction(f$) then 
		return map(f$,x,y;args());
	else 
		if maps then return %mapexpression2(x,y,f$;args());
		else return f$(x,y,args());
		endif;
	endif;
endfunction

function map %mapexpression3 (x:scalar, y:scalar, z:scalar; expr:string)
	return expr(x,y,z;args());
endfunction

function %ploteval3 (f$:call, ..
	x:numerical, y:numerical, z:numerical, maps=0)
	if isfunction(f$) then 
		return map(f$,x,y,z;args());
	else 
		if maps then return %mapexpression3(x,y,z,f$;args());
		else return f$(x,y,z,args());
		endif;
	endif;
endfunction
	
function args adaptiveevalone (g$:call, a:number, b:number, ..
	eps=0.01, amin=0.00001, amax=0.01, square=1)
## Compute g(x) for t in [a,b] with adaptive step size.
##
## eps is the target accuracy.
## amin and amax are the minimal and maximal step size.
## amax is also the initial step size.
## f and g can be expressions in x or user defined functions.
## Returns {x,y}
##
## See: plot2d, adaptiveeval
	// other direction
	if b<a then return
		adaptiveevalone(g$,b,a,eps,amin,amax,square;args()); 
	endif;
	// determine saling of g-values (assume a nearly square plot)
	scale=1;
	if square and b<>a then
		t=linspace(a,b,20);
		s=%ploteval(g$,t;args());
		scale=(totalmax(s)-totalmin(s))/(b-a);
		if scale==0 then scale=1; endif;
	else scale=1;
	endif;
	// walk through [a,b]
	t=a;
	sx=t;
	sy=g$(t,args());
	if cols(sy)>1 then 
		error(g$|" does not produce a real or column vector"); 
	endif
	x=zeros(rows(sx),1000); y=zeros(rows(sy),1000); 
	n=1;
	x[:,1]=sx; y[:,1]=sy;
	h=amax;
	repeat
		repeat
			tn=t+h;
			if tn>b then tn=b; endif;
			sxn=tn;
			syn=g$(tn,args());
			d=sqrt((sxn-sx)^2+((syn-sy)/scale)^2)[1];
			if isNAN(d) then break; endif;
			if d>eps and h>amin; then h=max(h/2,amin);
			elseif d<eps/4;	then h=min(h*2,amax); break;
			else break;
			endif;
		end;
		n=n+1;
		if n>cols(x) then
			x=x|x;
			y=y|y;
		endif;
		x[:,n]=sxn; y[:,n]=syn;
		sx=sxn; sy=syn;
		t=tn;
		if t>=b; then break; endif;
	end;
	return {x[:,1:n],y[:,1:n]};
endfunction

// * Colors

// Euler can use any color with the rgb function. 

// But there are 16 pre-defined functions, which use the integer
// indices 0-15: white=0,black=1,red=2,green=3,blue=4, cyan=5,
// olive=6, lightgray=7, gray=8, darkgray=9, orange=10, lightgreen=11,
// turquoise=12, lightblue=13, lightorange=14, yellow=15.

// Moreover, there are special colors: 255 for transparent grid plots,
// -1, -2 for spectral colors in hue plots.

function rgb (r,g,b)
## Computes a RGB color from [0,1]-values
##
## Returns an integer encryption of the color.
## Use to set rgb colors instead of a color index,
## and to define an image to be saved on disk.
##
## See: plot2d, putred, putgreen, putblue, savergb
	return putred(r)+putgreen(g)+putblue(b);
endfunction

function args getrgb (x)
## Computes the red, green and blue parts from an rgb color.
##
## Used to decompose image pixels into colors.
## Returns {r,g,b}
##
## See: rgb, getred, getblue, getgreen, loadrgb
	return {getred(x),getgreen(x),getblue(x)};
endfunction

function computehue (x,y,z,L=[0,0,1],amb=0.1,max=0.9,hue=1)
## Computes the shading for a surface
##
## This is used in various plot functions.
##
## x,y,z : coordinates of the surface
## l : direction of light
## amb : ambient light
## max : maximal hue value
## hue : -2, -3, -4 for x, y, z direction (default 1)
##
	if hue==-1; return 0; endif;
	// used in direction x, y or z
	if hue==-2; return (x-totalmin(x))/(totalmax(x)-totalmin(x))*0.999; endif;
	if hue==-3; return (y-totalmin(y))/(totalmax(y)-totalmin(y))*0.999; endif;
	if hue==-4; return (z-totalmin(z))/(totalmax(z)-totalmin(z))*0.999; endif;
	// compute shading depending on the light
	O=ones(size(x,y,z));
	X=x*O; Y=y*O; Z=z*O;
	m=rows(O); n=cols(O);
	X1=X[1:m-1,1:n-1]; Y1=Y[1:m-1,1:n-1]; Z1=Z[1:m-1,1:n-1];
	DX1=(X[2:m,1:n-1])-X1;
	DY1=(Y[2:m,1:n-1])-Y1;
	DZ1=(Z[2:m,1:n-1])-Z1;
	DX2=(X[1:m-1,2:n])-X1;
	DY2=(Y[1:m-1,2:n])-Y1;
	DZ2=(Z[1:m-1,2:n])-Z1;
	HX=DY1*DZ2-DY2*DZ1;
	HY=-DX1*DZ2+DX2*DZ1;
	HZ=DX1*DY2-DY1*DX2;
	R=sqrt(HX*HX+HY*HY+HZ*HZ)+epsilon;
	H=-(HX*L[1]+HY*L[2]+HZ*L[3])/R;
	if hue>1; H=abs(H); endif;
	H=(H-totalmin(H))/(totalmax(H)-totalmin(H))*(max-amb)+amb;
	return H;
endfunction

// * Save Images to Files

// This can should be done in the menu. But there are also functions
// for Euler scripts.

function overwrite savepng (filename:string, w:integer=0, h:integer=0, ..
	antialias:integer=1)
## Save the current graphics as PNG file in current directory.
##
## Saves the current graphics in PNG format. If antialias=1, then
## the function takes more time, and needs more memory, but the output
## looks smoother. If w=0, "savepng" uses the screen width. If h=0,
## the current aspect ratio with the current width is used.
##
## The current directory is set by the "cd" command, or, if a
## notebook is saved or opened.
##
## See: savesvg, saveps
	_savepng(filename,w,h,antialias);
endfunction

function overwrite savesvg (filename:string, w:integer=0, h:integer=0)
## Save the current graphics as SVG file in current directory.
##
## Saves the current graphics in the format SVG (scalable vector
## format). If h=0, the current aspect ratio with the current width is
## used.
##
## The current directory is set by the "cd" command, or, if a
## notebook is saved or opened.
##
## See: savepng, saveps
	_savesvg(filename,w,h);
endfunction

function overwrite saveps (filename:string)
## Save the current graphics as EPS file in current directory.
##
## Saves the current graphics in EPS (encapsulated postscript) format.
##
## The current directory is set by the "cd" command, or, if a
## notebook is saved or opened.
##
## See: savepng, savesvg
	_saveps(filename);
endfunction

function fcontour (f$,xmin=none,xmax=1,ymin=-1,ymax=1,r=none,
	n=40,nc=defaultnc,hue=0,level="auto",
	grid=2,title="",add=0,color=none,hcolor=0,maps=0,
	contourcolor=none, contourwidth=1,style="#",outline=1,frame=1)
## Draw contour lines of a function or expression in x and y.
##
## Obsolete. Use plot2d.
##
## See: plot3d
	if color==none then color=1; endif;
	if r!=none then xmin=-r; xmax=r; ymin=-r; ymax=r; endif;
	if xmin==none then
		if add then 
			p=plot(); 
			xmin=p[1]; xmax=p[2]; ymin=p[3]; ymax=p[4];
		else
			xmin=-1; xmax=1; ymin=-1; ymax=1;
		endif;
	endif;
	if n==none then n=40; endif;
	x=linspace(xmin,xmax,n); y=linspace(ymin,ymax,n);
	{X,Y}=field(x,y);
	Z=%ploteval2(f$,X,Y,maps;args());
	datacontour(Z,xmin,xmax,ymin,ymax, ..
		nc,hue,level,grid,title,add,color,hcolor, ..
		contourcolor,contourwidth,style,outline,frame);
endfunction

function datacontour (Z:real, 
	a:number=0, b:number=1, c:number=0, d:number=1,
	nc=defaultnc, hue=0, level="auto",
	grid=2, title="", add=0, color=1, hcolor=0,
	contourcolor=none, contourwidth=none, style="#", outline=1,
	frame=1)
## Draw contour lines of a data matrix.
##
## Obsolete. Use plot3d.
##
## See: plot3d
	if !add and !holding() then clg; endif;
	h=holding(1);
	if add then 
		p=_plot(); 
		m0=margin(0); w0=window();
		window(toscreen(a,d),toscreen(b,c));
		setplot(p);
	elseif not hue then
		xplot(=grid,=frame);
	endif;
	oldcw=linewidth(contourwidth);
	oldcol=contourcolor(contourcolor);
	if hue then
		col=huecolor(color);
		density(Z,0.99); 
		huecolor(col);
		if level==none or level=="auto" or level=="thick" then
			level=autolevel(totalmin(Z),totalmax(Z),nc,0.1);
			contourcolor(contourcolor);
			contour(Z,level);
		elseif level=="thin" then
			level=autolevel(totalmin(Z),totalmax(Z),nc,0);
			contourcolor(contourcolor);
			contour(Z,level); 
		else
			if rows(level)==2 then
				bst=barstyle(style);
				if color then contourcolor(color);
				else contourcolor(green);
				endif;
			else
				contourcolor(contourcolor);
			endif;
			contour(Z,level);
			if rows(level)==2 then
				barstyle(bst);
				if outline then
					contourcolor(contourcolor); 
					contour(Z,redim(level,1,cols(level)*2));
				endif;
			endif;
		endif;
	else
		if level==none or level=="auto" or level=="thick" then
			level=autolevel(totalmin(Z),totalmax(Z),nc,0.1);
			contourcolor(contourcolor);
			contour(Z,level); 
		elseif level=="thin" then
			level=autolevel(totalmin(Z),totalmax(Z),nc,0);
			contourcolor(contourcolor);
			contour(Z,level); 
		else
			if rows(level)==2 then
				bst=barstyle(style);
				if color!=none then 
					contourcolor(color);
					if style=="#" then contourcolor=color; endif;
				else contourcolor(green); endif;
			else
				contourcolor(contourcolor);
			endif;
			contour(Z,level); 
			if rows(level)==2 then
				barstyle(bst);
				if outline then
					if contourcolor!=none then contourcolor(contourcolor); 
					else contourcolor(wirecolor()); 
					endif;
					contour(Z,redim(level,1,cols(level)*2));
				endif;
			endif;
		endif;
	endif;
	contourcolor(oldcol);
	linewidth(oldcw);
	if add then 
		margin(m0); window(w0);
		setplot(p);
	else 
		setplot(a,b,c,d); 
	endif;
	if hue then xplot(grid=grid); endif;
	settitle(title);
	holding(h);
endfunction

function flevel (f$,level=0,xmin=-1,xmax=1,ymin=-1,ymax=1,n=50,maps=0)
## Draw contour lines of a function or expression in x and y
##
## Obsolete, since plot3d should be used now.
##
## See: plot3d
	x=linspace(xmin,xmax,n); y=linspace(ymin,ymax,n);
	{X,Y}=field(x,y);
	Z=%ploteval2(f$,X,Y,maps;args());
	z1=totalmin(Z); z2=totalmax(Z);
	contour(Z,level);
	setplot(xmin,xmax,ymin,ymax);
endfunction

function frotate (f$,a,b,n=40,nt=40,scale=1,frame=1, ..
	xlabel="x",ylabel="y",zlabel="z")
## Show a rotated plot of a function
##
## Obsolete, since plot3d with user=1 should be used now.
##
## See: plot3d
	x=linspace(a,b,n);
	if isfunction(f$);
		r=map(f$,x;args);
	else
		r=f$(x,args());
	endif;
	t=linspace(0,2*pi,nt)';
	framedsolid(x,cos(t)*r,sin(t)*r,scale,frame);
endfunction

function triangle
## Obsolete function
	return 	1
endfunction

function args %makesteps (x,y,lower=0)
## Expand {x,y}, such that it plots in steps.
	m=cols(x);
	i=redim(dup(1:m,2)',[1,2*m]);
	if lower then return {x[:,i[2:2*m]],y[:,i[1:2*m-1]]};
	else return {x[:,i[1:2*m-1]],y[:,i[2:2*m]]};
	endif;
endfunction

function args %makesteps3d (x,y,z)
## Expand {x,y}, such that it plots in steps.
	m=max(cols(x),cols(y));
	i=redim(dup(1:m,2)',[1,2*m]);
	if cols(x)>1 then x=x[:,i[2:2*m]]; x=x|(2*x[:,-1]-x[:,-3]); endif;
	if cols(y)>1 then y=y[:,i[2:2*m]]; y=y|y[:,-1]; endif;
	if cols(z)>1 then z=z[:,i[1:2*m]]; endif;
	n=max(rows(x),rows(y));
	i=redim(dup(1:n,2)',[1,2*n]);
	if rows(x)>1 then x=x[i[2:2*m]]; x=x_x[-1]; endif;
	if rows(y)>1 then y=y[i[2:2*m]]; y=y_(2*y[-1]-y[-3]); endif;
	if cols(z)>1 then z=z[i[1:2*m]]; endif;
	return {x,y,z}
endfunction

function f3dimplicit (f$:string, x:real vector, y:real vector, z:real vector, ..
	fr:integer=1, xlabel:string="x", ylabel:string="y", zlabel:string="z", ..
	scale:real=1, direction:integer=4, maps=0, ..
	contourcolor=1, contourwidth=1)
## Implicit 3D plot contours at z-values.
	frame=getframe(x,y,z);
	if !holding() then clg; endif;
	h=holding(1);
	oldframe=frame;
	if scale then
		s=max([frame[2]-frame[1],frame[4]-frame[3],frame[6]-frame[5]]);
		xm=(frame[2]+frame[1])/2;
		ym=(frame[4]+frame[3])/2;
		zm=(frame[6]+frame[5])/2;
		ff=scale/s*2;
		frame=[frame[1]-xm,frame[2]-xm,frame[3]-ym, ..
			frame[4]-ym,frame[5]-zm,frame[6]-zm]*ff;
	else
		xm=0; ym=0; zm=0; ff=1;
	endif;
	if fr then color(2); %frame1(frame,fr); color(1); endif;
	if bitand(direction,1) then
		loop 1 to cols(x)
			W=%ploteval3(f$,x[#],y,z',maps;args())+x[#];
			oldc=contourcolor(contourcolor);
			lw=linewidth(contourwidth);
			solidhuecontour((W-xm)*ff,(y-ym)*ff,(z'-zm)*ff,0,(x[#]-xm)*ff,(W-xm)*ff);
			linewidth(lw);
			contourcolor(oldc);
		end;
	endif;
	if bitand(direction,2) then
		loop 1 to cols(y)
			W=%ploteval3(f$,x,y[#],z',maps;args())+y[#];
			oldc=contourcolor(contourcolor);
			lw=linewidth(contourwidth);
			solidhuecontour((x-xm)*ff,(W-ym)*ff,(z'-zm)*ff,0,(y[#]-xm)*ff,(W-ym)*ff);
			linewidth(lw);
			contourcolor(oldc);
		end;
	endif;
	if bitand(direction,4) then
		loop 1 to cols(z)
			W=%ploteval3(f$,x,y',z[#],maps;args())+z[#];
			oldc=contourcolor(contourcolor);
			lw=linewidth(contourwidth);
			solidhuecontour((x-xm)*ff,(y'-ym)*ff,(W-zm)*ff,0,(z[#]-zm)*ff,(W-zm)*ff);
			linewidth(lw);
			contourcolor(oldc);
		end;
	endif;
	if fr then 
		color(2); %frame2(frame,fr);
		if fr<2 then %frameticks(frame,oldframe,xlabel,ylabel,zlabel,fr); endif;
		color(1); 
	endif;
	holding(h);
endfunction

function solidhue (x:real, y:real, z:real, h:real,
	f:number=1, level=none, nc:index=10, values=none,
	contourcolor=1, contourwidth=1, 
	dl=none, breaks=none, limits=none,
	linewidth=none)
## Shaded solid 3D-plot of x,y,z
##
## h is the shading and should run between 0 and 1.
## f determines, if h is scaled to fit in between 0 and f.
##
## See: solid, solidhuecontour, plot3d
	oldlw=_linewidth(linewidth);
	if values==none then values=z; endif;
	if level==none then
		if limits!=none and limits[1]<limits[2] then
			_solid(x,y,z,breaks,h,values,none,limits);
		else
			_solid(x,y,z,breaks,h);
		endif;
	else 	
		if level=="auto" or level=="thick" then
			level=autolevel(totalmin(z),totalmax(z),nc,0.1);
		elseif level=="thin" then
			level=autolevel(totalmin(z),totalmax(z),nc,0);
		endif;
		oldc=contourcolor(contourcolor);
		lw=linewidth(contourwidth);
		if dl==none then dl=0.01; endif;
		if dl>0 and rows(level)==1 then
			level=level+[-dl;dl];
		endif
		_solid(x,y,z,breaks,h,values,level,limits);
		linewidth(lw);
		contourcolor(oldc);
	endif;
	linewidth(oldlw);
endfunction

function solidhuecontour (x:real, y:real, z:real, h:real, 
	v:real, w:real, breaks=none, limits=none)
## Shaded solid 3D-plot with contour lines.
##
## This function is just for compatibility with older Euler files. It
## calls the built-in solid function.
##
## See: solid, plot3d
	return solid(x,y,z,breaks,h,w,v,limits);
endfunction

function drawbutton (text:string, x:real scalar, y:real scalar, w:real scalar)
## Draw a button with the text at x,y with w character width. x
## centered and y top of the label. Return the button rectangle. This
## function can be used to program user interaction with clicks onto
## the screen.
##
## See: inbutton
	st=barstyle("#O");
	bc=barcolor(0);
	tw=textwidth(); th=textheight();
	b=[x-tw*w/2,y-th/3,tw*w,th*5/3];
	bar(b);
	ctext(text,[x,y]);
	barstyle(st);
	barcolor(bc);
	return b
endfunction

function inbutton (b:real vector, s:real vector)
## Test if the screen coordinates s are with in the button rectangle b. 
## Get s with toscreen(mouse()) to ask the user for a click.
##
## See: drawbutton, mouse
	return s[1]>b[1] && s[1]<b[1]+b[3] && s[2]>b[2] && s[2]<b[2]+b[4];
endfunction

%currentaspect=1;

function fplot (f$,a=0,b=0,n=200,grid=2)
## Plots the function f in [a,b]
##
## fplot("f") or fplot("f",,,n,...) plots f in the old interval.
## fplot uses map for the evaluation of the function "f".
## f$ may be an expression in x.
##
## Deprecated!
##
## See: plot2d
	if a~=b; s=plot(); a=s[1]; b=s[2]; endif;
	t=linspace(a,b,n);
	if isfunction(f$) then 
		s=map(f$,t;args());
	else 
		s=f$(t);
		if all(size(s)==[1,1]) then s=s[1,1]*ones(size(t)); endif;
	endif;
	if holding || !grid; return plot(t,s);
	else return xplot(t,s);
	endif;
endfunction

function f3dplotlight (f$,
	xmin=-1,xmax=1,ymin=-1,ymax=1,n=40,
	scale=1,fscale=1,light=[0,0,1],amb=0.1,max=0.9,fr=1,
	xlabel="x",ylabel="y",zlabel="z",level=none,hue=1,
	maps=0,contourcolor=1,contourwidth=1,
	dl=none,limits=none, nc=defaultnc,
	cp=0, cpcolor=1, cplevel="auto", cpdelta=0.05, zlim=none)
## Plots a function f(x,y,...) in a square.
##
## Also f3dplot("f",xmin,xmax,ymin,ymax,n;...).
## f3dplot uses map for the evaluation of the function "f".
## "f" may be an expression in x and y.
##
## Obsolete. Use plot3d.
##
## See: plot3d
	lx=linspace(xmin,xmax,n{1});
	ly=linspace(ymin,ymax,n{2});
	{x,y}=field(lx,ly);
	z=%ploteval2(f$,x,y,maps;args());
	h=1; z0=0;
	if fscale!=0 then 
		{z,h,z0}=scalez0(z,fscale); 
		if typeof(level)!=8 then
			level=(level-z0)*h;
		endif;
		if typeof(cplevel)!=8 then
			cplevel=(cplevel-z0)*h;
		endif;
		if dl==none then dl=0.01*h; endif;
		if limits!=none then
			limits=(limits-z0)*h;
		endif;
		if zlim!=none then
			zlim=(zlim-z0)*h;
		endif;
	endif;
	frame=none;
	framedsolidhue(x,y,z,computehue(x,y,z,light,amb,max,hue), ..
		scale,1,fr,h,xlabel,ylabel,zlabel,level,none, ..
		=contourcolor,=contourwidth,=z0, ..
		=nc,=dl,=limits,=cp,=cpcolor,=cplevel,=cpdelta,=zlim);
endfunction

function f3dplotpolar (f$,rmax=1,n=40,scale=1,fscale=1, ..
	frame=1,wire=0,xlabel="x",ylabel="y",zlabel="z",maps=0, 
	cp=0, cpcolor=1, cplevel="auto", cpdelta=0.05)
## Draw a function defined on the circle with radius rmax.
##
## f3d uses map for the evaluation of the function "f".
## n is the spacing.
##
## Obsolete. Use plot3d.
## See: plot3d
	t=fliplr(linspace(0,2*pi,2*n{1}))'; 
	r=linspace(rmax/100,rmax,n{2}); 
	x=cos(t)*r; y=-sin(t)*r;
	Z=%ploteval2(f$,x,y,maps;args());
	h=1; z0=0;
	if fscale!=0 then {Z,h,z0}=scalez0(Z,fscale); endif;
	if wire then
		framedwire(x,y,Z,scale,frame,h,xlabel,ylabel,zlabel,z0=z0, ..
		cp=cp,cpcolor=cpcolor,cplevel=cplevel,cpdelta=cpdelta);
	else
		framedsolid(x,y,Z,scale,frame,h,,xlabel,ylabel,zlabel,z0=z0, ..
		cp=cp,cpcolor=cpcolor,cplevel=cplevel,cpdelta=cpdelta,
		=cp,=cpcolor,=cplevel,=cpdelta);
	endif;	
endfunction

function f3dplotpolarl (f$,rmax=1,n=20,scale=1,fscale=1,
	light=[0,0,1],amb=0.2,max=0.8,frame=1,
	xlabel="x",ylabel="y",zlabel="z",
	level="auto",hue=1,maps=0, dl=none, limits=none,
	cp=0, cpcolor=1, cplevel="auto", cpdelta=0.05, nc=defaultnc)
## Draw a function defined on the circle with radius rmax.
##
## f3d uses map for the evaluation of the function "f".
## n is the spacing.
##
## Obsolete. Use plot3d.
## See: plot3d
	t=fliplr(linspace(0,2*pi,n{1}))'; 
	r=linspace(rmax/100,rmax,n{2}); 
	x=cos(t)*r; y=-sin(t)*r;
	Z=%ploteval2(f$,x,y,maps;args());
	h=1; z0=0;
	if fscale!=0 then {Z,h,z0}=scalez0(Z,fscale); endif;
	framedsolidhue(x,y,Z,computehue(x,y,Z,light,amb,max,hue), ..
		scale,1,frame,h,xlabel,ylabel,zlabel,level,=z0, ..
		=nc,=cp,=cpcolor,=cplevel,=cpdelta);
endfunction

function mark3 (x,y,z)
## Plots points in three dimensions. 
##
## x,y,z must be 1xn vectors.
##
## Obsolete. Use plot3d.
##
## See: plot3d
	{c0,r0}=project(x,y,z);
	{x,y}=fromscreen(c0,r0);
	return mark(x,y);
endfunction

defaultclipping=true;

function clipping (flag:integer=true)
## Sets clipping off or on globally for 2D plot functions.
##
## See: plot2d
	global defaultclipping;
	res=defaultclipping;
	defaultclipping=flag;
	return res;
endfunction

function overwrite plot (x=0, y=0)
## Plots the values (x(i),y(i)) with the current style.
## If x is a matrix, y must be a matrix of the same size.
## The plot is then drawn for all rows of x and y.
## The plot is scaled automatically, unless hold is on.
## plot(x,y) and plot() return [x1,x2,y1,y2], where [x1,x2] is the range
## of the x-values and [y1,y2] of the y-values.
## plot(x) is the same as plot(1:cols(x),x).
##
## To turn off the clipping set defaultclipping=false
##
## See: plot2d
	global defaultclipping;
	if !holding() && argn()>0; clg; endif;
	if argn()==1 then
		if iscomplex(x) then
			if defaultclipping then oldclip=clipmore(window()); endif;
			res = _plot(re(x),im(x));
			if defaultclipping then clip(oldclip); endif;
			return res;
		elseif isreal(x) then
			if defaultclipping then oldclip=clipmore(window()); endif;
			res = _plot(1:cols(x),x);
			if defaultclipping then clip(oldclip); endif;
			return res;
		else
			error("Illegal x for plot");
		endif;
	else if argn()==2 then
		if isreal(x) && isreal(y) then
			if defaultclipping then oldclip=clipmore(window()); endif;
			res = _plot(x,y);
			if defaultclipping then clip(oldclip); endif;
			return res;
		else
			error("Illegal x or y in plot");
		endif;			
	else return _plot();
	endif;
endfunction

function overwrite mark (x=0,y=0,c=1)
## Plots markers at (x(i),y(i)) according the the actual marker style.
## Works like plot.
## Obsolete. Use plot2d instead.
##
## See: plot2d
	if !holding() && argn()>0; clg; endif;
	if argn()==1 then
		if iscomplex(x) then
			oldclip=clipmore(window());
			res = _mark(re(x),im(x));
			clip(oldclip);
			return res;
		else
			oldclip=clipmore(window());
			res = _mark(1:cols(x),x);
			clip(oldclip);
			return res;
		endif;
	else if argn()==2 then
			oldclip=clipmore(window());
			res = _mark(x,y);
			clip(oldclip);
			return res;
	else if argn()==3 then
			oldclip=clipmore(window());
			res = _markwithcolor(x,y,c);
			clip(oldclip);
			return res;
	else return _plot();
	endif;
endfunction

function cplot (z)
## Plots a grid of complex numbers.
## Obsolete. Use plot2d instead.
##
## See: plot2d
	plot(re(z),im(z)); s=scaling(0); h=holding(1);
	w=z'; plot(re(w),im(w)); holding(h); scaling(s);
	return plot();
endfunction

function scale (z:real, scale:number=1,	center:number=0,
	retscale:integer=0)
## Scale the matrix z into [0,scale]
##
## z : real matrix
## scale : maximal absolute value of the returned matrix
## retscale : returns the scaling factor f and center c too.
##
## center : Scales into [-scale,scale]
##
## If >retscale, the function returns {znew,f,c}.
##
## See: plot3d
	z1=totalmin(z); z2=totalmax(z);
	z0=(z1+z2)/2; dz=(z2-z1)/2+epsilon();
	if not center then z0=z0-dz; dz=dz*2; endif;
	if retscale then return {(z-z0)/dz*scale,scale/dz,z0};
	else return  (z-z0)/dz*scale;
endfunction

function scalez (z,scale)
## Scale the matrix z to [-scale,scale].
##
## This is a function used by plot3d with parameter scale=1.
## Returns the scaled matrix and and the scaling factor.
##
## See: plot3d
	if scale==0 then return {z,0}; endif;
	h=totalmax(abs(z))+epsilon();
	return {z/h*scale,scale/h};
endfunction

function scalez0 (z:real, scale:number, center:integer=1)
## Scale the matrix z to [-scale,scale] symmetrical to 0.
##
## This is a function used by plot3d.
## Returns the scaled matrix and the scale.
##
## See: plot3d
	if scale==0; return {z,0}; endif;
	z1=totalmin(z); z2=totalmax(z);
	if center then
		z0=(z1+z2)/2; dz=(z2-z1)/2+epsilon();
	else
		z0=0; dz=(z2-z1)+epsilon();
	endif;
	return {(z-z0)/dz*scale,scale/dz,z0}
endfunction

function f3d (f,m=40)
	return plot3d(f,n=m);
endfunction

function f3dplot (f$,xmin=-1,xmax=1,ymin=-1,ymax=1,n=40, ..
	scale=1,fscale=1,frame=1,wire=0,xlabel="x",ylabel="y",zlabel="z", ..
	sliced=0,maps=0,wirecolor=1, 
	cp=0, cpcolor=1, cplevel="auto", cpdelta=0.05, nc=defaultnc)
## f3dplot("f") plots a function f(x,y,...) in a square.
##
## Obsolete function. Use plot3d instead.
##
## f3dplot uses map for the evaluation of the function "f".
## "f" may be an expression in x and y.
##
## See: plot3d
	lx=linspace(xmin,xmax,n{1});
	ly=linspace(ymin,ymax,n{2});
	{x,y}=field(lx,ly);
	z=%ploteval2(f$,x,y,maps;args());
	h=1; z0=0;
	if fscale!=0 then {z,h,z0}=scalez0(z,fscale,!sliced); endif;
	if wire;
		framedwire(x,y,z,scale,frame,h,xlabel,ylabel, ..
			zlabel,wirecolor,0,z0, ..
			cp=cp,cpcolor=cpcolor,
			cplevel=cplevel,cpdelta=cpdelta);
	else
		framedsolid(x,y,z,scale,frame,h,, ..
			xlabel,ylabel,zlabel,sliced,0,z0, ..
			cp=cp,cpcolor=cpcolor,
			cplevel=cplevel,cpdelta=cpdelta);
	endif;	
endfunction

function overwrite density (x,f=0.9,amb=0.1)
## Makes density plot of the values in the matrix x
## scaled to fit into [0,f].
## See: plot2d
	if f==0;
		_density(x);
		return x;
	else;
		ma=max(max(x)'); mi=min(min(x)'); h=ma-mi;
		if h~=0; h=1; endif;
		xx=amb+(x-mi)/h*(f-amb)*0.99;
		_density(xx);
	return xx;
	endif;
endfunction

function %plot2dcomplex (z)
	oldclip=clipmore(window());
	_plot(re(z),im(z));
	_plot(re(z'),im(z'));
	clip(oldclip);
	xplot();
endfunction

function plot3dbars (x : real vector, y : real vector, z : real, ..
	z0 : real scalar)
## Plot bars at x[i],y[j] with height z[i,j].
##
## The bars have limits x[i],x[i+1] and y[j],y[j+1] and use
## the values z[i,j]. z must be at least (n-1)x(m-1), if
## x is 1xn and y is 1xm. The bars are sorted correctly before
## they are plotted by plotcubes.
##
## A comfortable way to use is is via plot3d and bar=true.
##
## See: plotcubes, plot3d
	n=cols(x); m=cols(y);
	M=zeros((n-1)*(m-1),6);
	k=1;
	for i=1 to n-1
		for j=1 to m-1
			if x[i+1]<x[i] or y[j+1]<y[j] then
				error("x and y must be sorted for plot3dbar.");
			endif;
			if z0<z[j,i] then
				M[k]=[x[i],x[i+1],y[j],y[j+1],z0,z[j,i]];
			else
				M[k]=[x[i],x[i+1],y[j],y[j+1],z[j,i],z0];
			endif;
			k=k+1;
		end;
	end;
	plotcubes(M,>xy);
endfunction

function overwrite plotcubes (M, color=none, 
	xy=false, midpoints=false)
## Plot cubes (parallel to the axes)
##
## The rows of M contain [x1,x2,y1,y2,z2,z2] with lower and upper
## limits for the sides of the cubes. The cubes are sorted either by
## distance of the midpoint to the eye, or by distance of the
## projection to the x-y-plane.
##
## See: plot3dbars
	v=view();
	x=sin(v[3])*cos(v[4])*v[1]; 
	y=-cos(v[3])*cos(v[4])*v[1]; 
	z=sin(v[4])*v[1];
	d=zeros(1,rows(M));
	for k=1 to rows(M)
		if xy then
			if x>M[k,1] and x<M[k,2] and y>M[k,3] and y<M[k,4] then
				d[k]=0;
			endif;
			h=(x-M[k,1])^2+(y-M[k,3])^2;
			h=min(h,(x-M[k,1])^2+(y-M[k,4])^2);
			h=min(h,(x-M[k,2])^2+(y-M[k,3])^2);
			h=min(h,(x-M[k,2])^2+(y-M[k,4])^2);
			d[k]=h;
		elseif midpoints then
			d[k]=(x-(M[k,1]+M[k,2])/2)^2+ ..
				(y-(M[k,3]+M[k,4])/2)^2+ ..
				(z-(M[k,5]+M[k,6])/2)^2;
		else
			d1=x-M[k,1]; d2=x-M[k,2];
			if d1*d2<0 then dx=0; else dx=min(abs(d1),abs(d2));	endif;
			d1=y-M[k,3]; d2=y-M[k,4];
			if d1*d2<0 then dy=0; else dy=min(abs(d1),abs(d2));	endif;
			d1=z-M[k,5]; d2=z-M[k,6];
			if d1*d2<0 then dx=0; else dz=min(abs(d1),abs(d2));	endif;
			d[k]=dx^2+dy^2+dz^2;
		endif;
	end;
	{ds,i}=sort(d);
	i=flipx(i);
	if color!=none then _plotcubes(M[i],color[i]);
	else _plotcubes(M[i]);
	endif;
endfunction

noantialias=0;

function overwrite insimg (lines:index=35, name:string="", ..
	antialias=1, crop=none)
## Insert the graphics into the notebook.
##
## Insert the graphics with default size and name into the notebook.
## The image appears in the notebook with the given height in lines.
## If the name is not empty, the image will be saved under this name
## when the notebook is saved. If the image is too small it will take
## less lines.
##
## antialiased : looks good for most plots, but not for all.
##
## crop : inserts only a top portion of the graphics. The crop can be
## a vector [top,bottom,left,right] of numbers from 0 to 1, or it can
## be [top,bottom], or only bottom. The defaults are [0,1,0,1].
##
## The crop works well with the aspect function. In fact, if an aspect
## is active with larger width than height, only the active portion of
## the image is inserted automatically.
##
## This function is used by the : at the end of command lines.
##
## >plot2d("x^3"); insimg();
## >plot2d("x^3"): // simpler alternative
##
## See: insimg, loadimg, aspect
	global noantialias;
	if crop==none then crop=[0,1,0,1];
	elseif typeof(crop)==0 then crop=[0,crop,0,1];
	elseif typeof(crop)==2 and cols(crop)==2 then crop=crop|[0,1];
	endif;
	_insimg(lines,name,antialias&&(!noantialias),crop);
endfunction

function overwrite insrgb (x:real, lines:integer=25, name:string="")
## Insert an rgb image matrix into the notebook.
##
## The image appears in the notebook with the given height
## in lines. If the name is not empty, the image will
## be saved under this name when the notebook is saved.
## If the image is too small it will take less lines.
## 
## >M=ones(1000,1000)*linspace(0,1,999);
## >insrgb(rgb(M,0.5,M),10);
##
## See: insimg, rgb
	return _insrgb(lines,name,x);
endfunction

function plotrgb (x:real, window:real vector=none, tcolor=none)
## Plot an rgb image into the plot window.
##
## The window can be a vector with screen coordinates or none. For
## sync with the plot coordinates use getwindowfromplot();
##
## >s=linspace(0,1,100); t=s'; c=rgb(s,t,0);
## >plot2d(none,0,1,0,1); plotrgb(c);
##
## See: rgb, getwindowfromplot
	if tcolor==none then tcolor=-1; endif;
	if window<>none then w=window(); _window(window); endif;
	hd=holding(1);
	res=_barRGB(getred(x),getgreen(x),getblue(x),tcolor);
	holding(hd);
	if window<>none then _window(w); endif;
	return res;
endfunction

function overwrite loadimg (filename:string, lines:index=35, 
	scale:positive=1)
## Insert an image from file.
##
## The image appears in the notebook with the given height
## in lines.
##
## filename : filename including extension (jpg or png)
## n : maximal number of lines to be used
## scale : scale with this factor
##
## See: insimg, loadimg
	return _loadimg(filename,lines,scale);
endfunction

function overwrite loadanaglyph
## loadanaglyph("left","right") or loadanaglyph(n,"left","right")
##
## Loads two images, combines them to one anaglyph, and inserts the
## images into the notebook.
##
## n : maximal number of lines to be used (default is 40)
##
## See: loadanaglyph, loadimg
	if argn==3 then return _loadanaglyph(arg1,arg2,arg3);
	else if argn==2 then return _loadanaglyph(40,arg1,arg2);
	else error("Need to file names for loadanaglyph.");
	endif;
endfunction

function figure (n:natural, m:index=none, smaller=0)
## figure (nc,nr) or figure(nc), figure(0) for sub-windows
##
## figure(nr,nc) divides the graph window into nr rows x nc columns of subplots
## figure(n) sets the current drawing window to the nth subplot counting
## by rows.
##
## figure(0) sets the plot back to the normal window
##
## smaller : if set, there is more space for vertical labels.
##
## >figure(3,1,>smaller); ...
## >figure(1); plot2d("sin(x)",0,2pi,yl="sin"); ...
## >figure(2); plot2d("cos(x)",0,2pi,yl="cos"); ...
## >figure(3); plot2d("sinc(x)",0,2pi,yl="sinc"); ...
## >figure(0):
##
	global subwindows,logredraw,usesubwindows,subwindowsfull;
	if m<>none then
		xf=8; if smaller then xf=12; endif;
		if m>0 then nr=n; nc=m;
		else nr=n[1]; nc=n[2];
		endif;
		w=window();
		wd=textwidth();
		ht=textheight();
		hr=floor(1024/nr);
		wc=floor(1024/nc);
		for r=1 to nr
			for c=1 to nc
				w=[xf*wd+(c-1)*wc,1.5*ht+(r-1)*hr,c*wc-2*wd,r*hr-3*ht];
				subwindows[(r-1)*nc+c]=w;
				subwindowsfull[(r-1)*nc+c]=[(c-1)*wc,(r-1)*hr,c*wc,r*hr];
			end;
		end;
		hold off;
		logredraw=1;
		clg;
		return _window(subwindows[1]);
	else
		hold on;
		logredraw=1;
		if n==0 or n>rows(subwindows) then
			hold off;
			usesubwindows=0;
			return shrinkwindow();
		else 
			usesubwindows=n;
			return _window(subwindows[n]); 
		endif;
	endif
endfunction

function title (s, color=none, x:real=none)
## Plots a title to the graphics window.
##
## See: plot2d, plot3d
	if color==none then color=defaulttextcolor; endif;
	cold=textcolor(color);
	global usesubwindows;
	w=window();
	if usesubwindows then c=(w[3]+w[1])/2;
	else c=512;
	endif;
	r=max(1,w[2]-textheight()*3/2);
	if x!=none then {c,r}=_toscreen([x,0]); endif;
	text(s,c,r,align=0);
	color(cold);
endfunction

function settitle (s, color=none, x:real=none)
## Plots a title to the graphics window.
##
## See: plot2d, plot3d
	title(s,color,x);
endfunction

function fwebplot (f$:string, a:real scalar, b: real scalar, ..
	xstart: real scalar, n: index, color=black)
## Plots an iteration of fff on [a,b] starting from xstart, n steps
## This function can be used to study, demonstrate or understand
## fixed point iteration of the type x(n+1) = f(x(n)).
## See: iterate, sequence
	plot2d("x",a,b,style="--");
	hold on; color(color); fplot(f$,a,b;args()); color(1); hold off;
	y=niterate(f$,xstart,n+1;args())';
	v=xstart|redim(dup(y[1:n],4),[1,4*n])|[y[n+1]];
	v=redim(v,[cols(v)/2,2])';
	hold on; color(10); plot(v[1],v[2]); color(1); hold off;
	return y;
endfunction

function args vectorfield (f$:string, 
	x1: real scalar=-2, x2: real scalar=2, ..
	y1:real scalar=-2, y2: real scalar=2, 
	r=none,	cx=0, cy=0, 
	nx:index=20, ny:index=20,
	thickness=1, color=black, frame=1,
	plot=true)
## Draw the vector field of a differential equation in x and y.
##
## expr : expression "f(x,y)", which computes the
##        derivative of y(x) at x.
## x1,x2,y1,y2 : bounds of the plot
## r,cx,xy : alternative way to enter the bounds
## nx, ny : number of arrows in each direction
## thickness,color,frame : see plot2d
## plot : do not plot but return x,y for later plots with plot2d().
##
## >vectorfield("x*y",r=1,cx=1,cy=1):
##
## See: vectorfield2, ode
	if r!=none then x1=cx-r; x2=cx+r; y1=cy-r; y2=cy+r; endif;
	x=linspace(x1,x2,nx-1);
	y=linspace(y1,y2,ny-1);
	{X,Y}=field(x,y);
	V=f$(X,Y);
	n=prod(size(X));
	h=(x2-x1)/nx/4;
	v=redim(V,[n,1]);
	r=sqrt(v*v+1);
	x=redim(X,[n,1]); x=(x-h/r)|(x+h/r);
	y=redim(Y,[n,1]); y=(y-h*v/r)|(y+h*v/r);
	if plot then
		plot2d(x,y,color=color,thickness=thickness, ..
			style="->",frame=frame);
	else
		return {x,y};
	endif;
endfunction

function args vectorfield2 (f1$:string, f2$:string, ..
	x1: real scalar=-2,x2: real scalar=2,
	y1: real scalar=-2,y2: real scalar=2,
	nx:index=20, ny:index=20,
	r=none,	cx=0, cy=0, 
	nx:index=20, ny:index=20,
	scale=1, normalize=0,
	thickness=1, color=black, frame=1,
	plot=true)	
## Draw the vector field of a differential equation in x and y.
##
## expr1 and expr2 : expressions "f(x,y)", which compute the
##                   x- and y-value of the derivative of y(x) at x.
## scale : scaling factor for the arrows
## x1,x2,y1,y2 : bounds of the plot
## r,cx,xy : alternative way to enter the bounds
## nx, ny : number of arrows in each direction
## thickness,color,frame : see plot2d
## normalize : derivative vectors are normalized
## plot : do not plot but return x,y for later plots with plot2d().
##
## >>vectorfield2("-y","x",r=1):
## >vectorfield2("x","x*y",>normalize):
##
## See: vectorfield, ode
	if r!=none then x1=cx-r; x2=cx+r; y1=cy-r; y2=cy+r; endif;
	setplot(x1,x2,y1,y2);
	x=linspace(x1,x2,nx-1);
	y=linspace(y1,y2,ny-1);
	{X,Y}=field(x,y); V1=zeros(size(X)); V2=zeros(size(X));
	loop 1 to prod(size(X));
		V1{#}=f1$(X{#},Y{#})*scale;
		V2{#}=f2$(X{#},Y{#})*scale;
		if normalize then
			L=sqrt(V1{#}^2+V2{#}^2);
			if not L~=0 then
				V1{#}=V1{#}/L*scale;
				V2{#}=V2{#}/L*scale;
			endif;
		endif;
	end;
	n=prod(size(X));
	h=(x2-x1)/nx/4;
	v1=redim(V1,[n,1]);
	v2=redim(V2,[n,1]);
	x=redim(X,[n,1]); x=(x-h*v1)|(x+h*v1);
	y=redim(Y,[n,1]); y=(y-h*v2)|(y+h*v2);
	if plot then
		plot2d(x,y,color=color,thickness=thickness, ..
			style="->",frame=frame);
	else
		return {x,y};
	endif;
endfunction

function overwrite view
## view(distance, tele, angle1, angle2) sets the perspective for solid and view. 
## distance is the eye distance, tele a zooming factor.
## angle1 is the angle from the negativ y-axis to the positive x-axis.
## angle2 is the angle to the positive z-axis (the height of the eye).
## view() returns the current value of view in a vector.
## view is used in plot3d as the default view.
## See: plot3d, zoom
	if argn()==4 then return _view([arg1,arg2,arg3,arg4]);
	else if argn()==1 then 
		if arg1==none then return _view();
		else return _view(arg1);
		endif;
	else return _view();
	endif;
endfunction

function zoom (f:positive=none)
## Set the zoom distance to d.
## See: view
   v=_view();
   if f!=none then _view([v[1],f,v[3],v[4]]); endif;
   return v;
endfunction

function viewangle (a:real scalar=none)
## Set the view angle to a.
## See: view
   v=_view();
   if a!=none then _view([v[1],v[2],a,v[4]]); endif;
   return v;
endfunction

function viewheight (a:real scalar=none)
## Set the view height angle to a.
## See: view
   v=_view();
   if a!=none then _view([v[1],v[2],v[3],a]); endif;
   return v;
endfunction

function viewdistance (d:positive=none)
## Set the view distance to d.
## See: view
   v=_view();
   if d!=none then _view([d,v[2],v[3],v[4]]); endif;
   return v;
endfunction

function f3daxis (f$:string, n:index=25)
## Draw a function defined on [-1,1]^2 with x-, y- and z-axis.
## f3daxis uses map for the evaluation of the function "f".
## 2n is the spacing.
## Fixed view, cannot be rotated.
## See: plot3d
	x=-1:1/n:1; y=x;
	{X,Y}=field(x,y);
	Z=%ploteval2(f$,X,Y;args());
	v=view(5,3,0.4,0.2);
	z1=totalmin(Z); z2=totalmax(Z); h=(z2+z1)/2; Z=Z-h;
	i=n+1:2*n+1; j=1:n+1;
	if !holding() then clg; endif;
	hl=holding(1);
	solid(X[i,j],Y[i,j],Z[i,j]);
	linewidth(3); 
	wire([0,0],[0,0],[-1.5,1.5]);
	wire([-1.5,0],[0,0],[-h,-h]);
	wire([0,0],[0,1.5],[-h,-h]);
	linewidth(1);
	solid(X[i,i],Y[i,i],Z[i,i]);
	solid(X[j,j],Y[j,j],Z[j,j]);
	linewidth(3);
	wire([0,1.5],[0,0],[-h,-h]);
	wire([0,0],[-1.5,0],[-h,-h]);
	linewidth(1);
	solid(X[j,i],Y[j,i],Z[j,i]);
	holding(hl);
	view(v);
endfunction

function f3daxispolar (f$:string, n:index=20)
## Draw a function defined on [-1,1]^2 with x-, y- and z-axis.
## 2n is the spacing.
## f3daxispolar uses map for the evaluation of the function "f".
## Fixed view, cannot be rotated.
## See: plot3d
	r=flipx(0:1/n:1)'; p=linspace(0,2*pi,8*n);
	X=r*cos(p); Y=r*sin(p);
	Z=%ploteval2(f$,X,Y;args());
	v=view(5,3,0.4,0.2);
	z1=totalmin(Z); z2=totalmax(Z); h=(z2+z1)/2; Z=Z-h;
	i=1:n+1;
	j=2*n:4*n+1;
	if !holding() then clg; endif;
	solid(X[i,j],Y[i,j],Z[i,j]); 
	hl=holding(1);
	linewidth(3);
	wire([0,0],[0,0],[-1.5,1.5]);
	wire([-1.5,0],[0,0],[-h,-h]);
	wire([0,0],[0,1.5],[-h,-h]);
	linewidth(1);
	j=1:2*n+1;
	solid(X[i,j],Y[i,j],Z[i,j]);
	j=4*n:6*n+1;
	solid(X[i,j],Y[i,j],Z[i,j]);
	linewidth(3);
	wire([0,1.5],[0,0],[-h,-h]);
	wire([0,0],[-1.5,0],[-h,-h]);
	linewidth(1);
	j=6*n:8*n+1;
	solid(X[i,j],Y[i,j],Z[i,j]);
	holding(hl);
	view(v);
endfunction

function stereo (f$:string, a:number=-0.05, s:index=500, ..
	d:number=1, parallel:natural=0)
## Stereo plot of the function f from two viewpoints
## Calls the function f, which must be the name of
## a 3D plot command, and does it twice in two different
## windows. Many people are able to view a 3D picture.
##
## There are two ways: parallel view and squint view.
## Most people find the latter easier. It is the default
## for this function.
	if parallel then a=-a; endif;
	clg;
	m=512;
	win=window();
	window(m-s,m-s/2,m,m+s/2);
	setanaglyph(-d*a);
	f$(args());
	window(m,m-s/2,m+s,m+s/2);
	setanaglyph(0);
	hold on;
	f$(args());
	hold off;
	window(win);
	setanaglyph(0);
	hold off;
endfunction

function parameterplot (f:string, a:number, d:number, ..
	display=1, c=800, r=100, ..
	amax=1e30, amin=-1e30)
## Call the plot function f with parameter a adjustable by user.
##
## Animate a paramter with the up/down keys, and show a plot depending
## on that parameter. The space key resets the default parameter, and
## the return key ends the function.
##
## f : function of a (and additional semicolon parameters)
## a : starting value parameter a
## d : increment with one keystroke
## display : flag for the display of the parameter value
## c,r : coordinates for the display
## amax,amin : limits for the display
##
## See: twoparameterplot
	astart=a;
	repeat
		f(a,args());
		if display then
			text("a = "|a,c,r);
		endif;
		k=key("up/down, space or return");
		if k==1 then a=a+d;
		elseif k==2 then a=a-d;
		elseif k==32 then a=astart;
		elseif k==13 then return a;
		else "up/down, space or return",
		endif;
		if a<amin then a=amin; endif;
		if a>amax then a=amax; endif;
		a=round(a,12);
	end;
endfunction

function twoparameterplot (f$:string, a:number, b:number, d:number, ..
	display=1, c=800, r=100, ..
	amax=1e30, amin=-1e30)
## Call the plot function f with parameter a adjustable by user.
##
## Animate a parameter a with the up/down keys, and a parameter b with
## left/right keys, and show a plot depending on that parameter. The
## space key resets the default parameter, and the return key ends the
## function.
##
## f : function of a,b (and additional semicolon parameters)
## a : starting value of parameter a
## b : second parameter b
## d : increment with one keystroke
## display : flag for the display of the parameter values
## c,r : coordinates for the display
## amax,amin : limits for the display
##
## See: twoparameterplot
	astart=a;
	bstart=b;
	repeat
		f$(a,b,args());
		if display then
			text("a = "|a,c,r);
			text("b = "|b,c,r+40);
		endif;
		k=key("up/down, left/right, space or return");
		if k==1 then a=a+d;
		elseif k==2 then a=a-d;
		elseif k==4 then b=b+d;
		elseif k==3 then b=b-d;
		elseif k==32 then a=astart; b=bstart;
		elseif k==13 then return a;
		else "up/down, left/right, space or return",
		endif;
		a=round(a,12);
		if a<amin then a=amin; endif;
		if a>amax then a=amax; endif;
	end;
endfunction

function args makeimpulse (x:real vector,y:real vector,d=0.1)
## Compute {x1,y1} for impulse bar plots.
##
## Computes the correct input for an impulse bar plot
## at points x with values y.
## 
## x : sorted x parameters
## y : values
## d : radius of the plot intervals
	n=cols(x);
	x1=redim((x'-d)|(x'+d),[1,2*n]);
	y1=zeros(1,cols(x1)-1); y1[1:2:2*n-1]=y;
	return {x1,y1}
endfunction

// * Functions for Animated Graphics

function dragpoints (f$ : string, x : real, y : real, 
	status="Drag one of the points!")
## Lets the user drag points on a user defined plot.
##
## This function calls a function f(x,y), which draws some plot
## depending on the points at coordinates x[i] and y[i]. Then it lets
## the user drag one of the points with the mouse.
##
## f : f(x,y[,select]) a plot function. select is the index of the
## selected point, if not select==0.
##
## Additional arguments are passed to f.
##
## Returns {x,y}
##
## Example:
## >function f(x,y) := plot2d(x,y,>points,a=0,b=1,c=0,d=1);
## >{x,y}=dragpoints("f",random(1,10),random(1,10));
	xreset=x; yreset=y;
	select=0;
	repeat
		f$(x,y,select,args());
		{flag,m,t}=mousedrag(status);
		if flag==0;
			if m==32 then x=xreset; y=yreset; flag=3;
			else break;
			endif;
		endif
		if flag==1 then // mouse down
			{c,r}=toscreen(x,y);
			{cm,rm}=toscreen(m[1],m[2]);
			e=extrema(max(abs(c-cm),abs(r-rm)));
			if e[1]<markersize() then 
				select=e[2];
				x[select]=m[1]; y[select]=m[2];
			endif;
		elseif flag==2 then // mouse moved
			if select>0 then 
				x[select]=m[1]; y[select]=m[2];
			endif;
		elseif flag==3 then // mouse up
			select=0;
		endif;
	end
	return {x,y};
endfunction

function dragvalues (f$, names, values, ranges, stops=100,
	x=0.98, y=0.02, w=0.4, tcolor=none, digits=none,
	heading=none, hcolor=none, status="")
## Let the user drag parameters for a plot.	
##
## This function lets the user drag values for plots. A box with
## values appears, by default in the upper right, with an optional
## heading. The values can be dragged to the right or left, and change
## within given ranges and steps.
##
## f : A plot function depending on a vector of parameters.
## names : The names of the parameters for the display.
## values : Initial values of the parameters (string vector of length
##   n or string) 
## ranges : A nx2 vector of ranges for the values
## stops : A vector with the number of subintervals for each range
## x,y,w : Position anchored in the top right (fraction of plot window)
## tcolor : A vector with colors for the values or one scalar color
## digits : A vector of digits for rounding the values or a scalar
## heading : An optional heading string
## hcolor : The color of the heading
## status : An optional string for the status line
##
## Example:
## >function f([a,b]) := plot2d("a*x^2+b*x";a,b,r=1);
## >dragvalues("f",["a","b"],[0,0],[-1,1;-1,1],digits=2, ...
## >  heading="a*x^2+b*x");
	vreset=values;
	
	win=window(); ww=win[3]-win[1]; wh=win[4]-win[2];
	
	ts=textsize();
	w=w*ww;
	x=win[1]+ww*x-w; y=win[2]+ww*y;
	h=(length(names)*1.2+1)*ts[2];
	if heading!=none then h=h+ts[2]*1.2; endif;
	
	select=0; outside=0;
	repeat
		if digits!=none then
			vv=round(values,digits);
		else
			vv=values;
		endif;
		f$(vv,args());
		
		bst=barstyle("O#"); bcol=barcolor(0);
		bar([x,y,w,h]);
		barstyle(bst); barcolor(bcol);
		
		xt=x+w/2; yt=y+ts[2]/2;
		if heading!=none then
			tc=textcolor(hcolor);
			ctext(heading,[xt,yt]);
			textcolor(tc);
			yt=yt+ts[2]*1.2;
		endif;
		loop 1 to length(names);
			tc=textcolor(tcolor{#});
			if digits==none then text=names{#}+" = "+values{#};
			else text=names{#}+" = "+print(values{#},digits{#},0);
			endif;
			ctext("< "+text+" >",[xt,yt]);
			textcolor(tc);
			yt=yt+ts[2]*1.2;
		end;
		if select and outside then
			wait(0); m=mousepos(); flag=2;
			if mousestate()==3 then flag=3; endif;
		else
			{flag,m,t}=mousedrag(status);
		endif;
		if flag==0 then
			if m==32 then values=vreset; flag=3;
			else break;
			endif;
		endif;
		if flag==1 then // mouse down
			{cm,rm}=toscreen(m[1],m[2]);
			if cm>x and cm<x+w then
				select=floor((rm-(y+ts[2]/2))/(ts[2]*1.2))+1;
				if heading!=none then select=select-1; endif;
				if select>length(names) then select=0; endif;
				if select<0 then select=0; endif;
			endif;
		elseif select and flag==2 then // mouse moved
			{cm1,rm1}=toscreen(m[1],m[2]);
			wait(0.1);
			if outside then sg=sign(cm1-(x+w/2));
			else sg=sign(cm1-cm);
			endif
			vnew=values[select]+ ..
				sg*(ranges[select,2]-ranges[select,1])/stops{select};
			values[select]=min(max(vnew,ranges[select,1]),ranges[select,2]);
			cm=cm1; rm=rm1; 
			if cm>=x+w or cm<=x then outside=1; 
			else outside=0; 
			endif;
		elseif flag==3 then // mouse up
			select=0;
		endif;	
	end;
	return vv;	
endfunction

// The following functions in this section are obsolete, since plot3d
// can now animate 3D graphics under user control. E.g. plot2d and
// plot3d have the parameter >user.

function animate (d:real scalar=0.1)
## Animate pre-defined pages with delay d.
##
## This animates predefined images. The function was designed for
## slower computers or for complicated plots. If the computer is fast
## enough, plots can also be generated directly. Then wait() will
## issue the display of the current plot.
##
## For animate(), the pages must be generated with calls to addpage(),
## as in the following example.
##
## deletepages(); 
## title("Creating Animation"); 
## addpage(); 
## showpage(1);
## loop ...
##   ... // graph something
##   addpage();
## end;
##
## See: addpage, showpage, wait, rotate
	testkey();
	repeat
		loop 2 to pages();
			showpage(#); 
			k=testkey();
			if k==32; key();
			elseif k==13; clg; showpage(0); return pages(); 
			elseif k;
				"Press space to stop, return to end",
				if key()==13; break; endif;
			endif;
			dd=wait(d);
			if (dd<d); showpage(0); return pages(); endif;
		end;
	end;
	showpage(0);
	return pages();
endfunction;	

function rotate (f$:string, d:real scalar=0.01, n:integer=120)
## Shows a rotating animation of the plot f$, 
##
## The plotting function f$ must produce a 3D plot. This functions
## used addpage() to pre-compute all plots. Seen the remarks in
## animate() for an easier method.
##
## Obsolete, since the plot can now be rotated with plot3d under user
## control. 
##
## See: plot3d, animate
	deletepages();
	clg;
	title("Creating Animation");
	addpage();
	showpage(1);
	t=linspace(0,2*pi,n);
	v=view();
	vold=v;
	loop 1 to cols(t);
		v[3]=t[#]; view(v);
		f$(args());
		addpage();
	end;
	animate(d);
	view(vold);
endfunction

function view3d (f$:string, d:real scalar=0.1)
## Calls the plot f$ and waits for key strokes to rotate it.
##
## Use the cursor keys for rotation, del and backspace, or space, to
## reset, return to abort. Returns the current view.
##
## Obsolete, use plot3d().
##
## See: plot3d
	vold=view();
	repeat
		f$(args());
		k=key();
		if k==13; break; endif;
		v=view();
		if (k==1); v[4]=v[4]-d;
		elseif (k==2); v[4]=v[4]+d;
		elseif (k==3); v[3]=v[3]-d;
		elseif (k==4); v[3]=v[3]+d;
		elseif (k==32); v=vold;
		elseif (k==7); v[3]=0;
		elseif (k==6); v[4]=0;
		elseif (k==43); v[1]=max(v[1]-d,1);
		elseif (k==45); v[1]=v[1]+d;
		else
			"Press return to end, cursorkeys to turn,",
			"+, - to zoom, or space to reset.",
			if key()==13; break; endif;
		endif;
		view(v);
	end;
	v=view();
	view(vold);
	return v;
endfunction

function fanimate (f$:string, t:vector, d:real scalar=0.01)
## Animate the function f$
##
## f$ should display some graphics depending on t. Additional
## parameters are passed to f. This function used addpage() to
## pre-compute the plots. See the remarks in animate() for an easier
## method.
##
## See: animate
	deletepages();
	clg;
	title("Creating Animation");
	addpage();
	showpage(1);
	loop 1 to cols(t);
		f$(t[#],args());
		addpage();
	end;
	animate(d);
	return pages();
endfunction

// Most of the following functions need never be used, since 3D
// graphics should be done with plot3d.

function getframe (x,y,z)
## Gets a box around all points in (x,y,z).
##
## Returns [xmin,xmax,ymin,ymax,zmin,zmax]
	return [totalmin(x),totalmax(x), ..
		totalmin(y),totalmax(y), ..
		totalmin(z),totalmax(z)]
endfunction

function %framez0 (f)
	wire([f[1],f[2],f[2],f[1],f[1]], ..
		[f[3],f[3],f[4],f[4],f[3]],dup(f[5],5)');
	return 0	
endfunction

function %framez1 (f)
	wire([f[1],f[2],f[2],f[1],f[1]], ..
		[f[3],f[3],f[4],f[4],f[3]],dup(f[6],5)');
	return 0
endfunction

function %framexpyp (f)
	wire([f[2],f[2]],[f[4],f[4]],[f[5],f[6]]);
	return 0
endfunction

function %framexpym (f)
	wire([f[2],f[2]],[f[3],f[3]],[f[5],f[6]]);
	return 0
endfunction

function %framexmyp (f)
	wire([f[1],f[1]],[f[4],f[4]],[f[5],f[6]]);
	return 0
endfunction

function %framexmym (f)
	wire([f[1],f[1]],[f[3],f[3]],[f[5],f[6]]);
	return 0
endfunction

function %frame1 (f,fr=1)
## Draws the back part of the frame (considering view)
	if fr==0 or fr==3 then return; endif;
	v=view();
	wc=wirecolor(frame3dcolor()); c=color(frame3dcolor());
	x=sin(v[3])*cos(v[4])*v[1];
	y=-cos(v[3])*cos(v[4])*v[1]; 
	z=sin(v[4])*v[1];
	if z>f[5] then %framez0(f); endif;
	if z<f[6] then %framez1(f); endif;
	if x<=f[2] then %framexpyp(f); %framexpym(f); endif;
	if x>=f[1] then %framexmyp(f); %framexmym(f); endif;
	if y<=f[4] then %framexmyp(f); %framexpyp(f); endif;
	if y>=f[3] then %framexmym(f); %framexpym(f); endif;
	wirecolor(wc); color(c);
	return 0
endfunction

function %frame2 (f,fr=1)
## Draws the front part of the frame (considering view).
	if fr==0 or fr==3 then return; endif;
	v=view();
	wc=wirecolor(frame3dcolor()); c=color(frame3dcolor());
	x=sin(v[3])*cos(v[4])*v[1]; 
	y=-cos(v[3])*cos(v[4])*v[1]; 
	z=sin(v[4])*v[1];
	if z<=f[5] then %framez0(f); endif;
	if z>=f[6] then %framez1(f); endif;
	if x>=f[2] then %framexpyp(f); %framexpym(f); endif;
	if x<=f[1] then %framexmyp(f); %framexmym(f); endif;
	if y>=f[4] then %framexmyp(f); %framexpyp(f); endif;
	if y<=f[3] then %framexmym(f); %framexpym(f); endif;
	wirecolor(wc); color(c);
	return 0
endfunction

function %scaleframe (x,y,z,%f,m)
## Scales the frame and the x,y,z matrix
	s=max([%f[2]-%f[1],%f[4]-%f[3],%f[6]-%f[5]]);
	xm=(%f[2]+%f[1])/2;
	ym=(%f[4]+%f[3])/2;
	zm=(%f[6]+%f[5])/2;
	ff=m/s*2;
	%f=[(%f[1]-xm)*ff{1},(%f[2]-xm)*ff{1}, ..
		(%f[3]-ym)*ff{2},(%f[4]-ym)*ff{2}, ..
		(%f[5]-zm)*ff{3},(%f[6]-zm)*ff{3}];
	return {(x-xm)*ff{1},(y-ym)*ff{2},(z-zm)*ff{3}, ..
		[xm,ym,zm],[ff{1},ff{2},ff{3}]}
endfunction

function %frametick (h1,h2,x1,y1,z1,x2,y2,z2,xz,yz,zz,s,fr)
## Draws a border line of the frame with frame ticks.
	wc=wirecolor(frame3dcolor()); c=color(frame3dcolor());
	if fr==3 then
		wire([x1,x2],[y1,y2],[z1,z2]);
	endif;
	ti=ticks(h1,h2,n=2);
	a=0.025;
	dx=sign((x1+x2)/2-xz);
	dy=sign((y1+y2)/2-yz);
	dz=sign((z1+z2)/2-zz);
	r=max(abs([(x1+x2)/2-xz,(y1+y2)/2-yz,(z1+z2)/2-zz]));
	for t=ti;
		c=(t-h1)/(h2-h1);
		x=(1-c)*x1+c*x2; y=(1-c)*y1+c*y2; z=(1-c)*z1+c*z2;
		af=a;
		if t~=0 then af=2*a; endif;
		xa=x+af*dx*r; 
		ya=y+af*dy*r; 
		za=z+af*dz*r;
		wire([x,xa],[y,ya],[z,za]);
	end;
	wirecolor(wc); color(c);
	a=0.15; cr=0;
	s1=niceform(h1,2); s2=niceform(h2,2);
	h=max(max(strlen(s1),strlen(s2)),strlen(s));
	if h>4 then a=(h+1)*(a/4); endif;
	th=textheight();
	xa=x1+a*dx*r; 
	ya=y1+a*dy*r; 
	za=z1+a*dz*r;
	{cc1,r1}=project(xa,ya,za);
	xb=x2+a*dx*r; 
	yb=y2+a*dy*r; 
	zb=z2+a*dz*r;
	{cc2,r2}=project(xb,yb,zb);
	_ctext(s1,[cc1,r1-th/2]);
	_ctext(s2,[cc2,r2-th/2]);
	{cc,r}=project((xa+xb)/2,(ya+yb)/2,(za+zb)/2);
	_ctext(s,[cc,r-th/2]);
endfunction

function %frameticks (f,of,xlabel="x",ylabel="y",zlabel="z",fr=1)
## Draws all border lines of the frame with frame ticks.
	if fr==2 then return; endif;
	v=view();
	x=sin(v[3])*cos(v[4])*v[1]; 
	y=-cos(v[3])*cos(v[4])*v[1]; 
	z=sin(v[4])*v[1];
	xz=(f[1]+f[2])/2;
	yz=(f[3]+f[4])/2;
	zz=(f[5]+f[6])/2;
	if fr==3 then
		if z>=f[5] then
			if x>=f[2] and y<=f[4] then 
				xl=f[2]; yl=f[4]; xl1=f[2]; yl1=f[3];
				%frametick(of[5],of[6],xl,yl,f[5],xl,yl,f[6],xz,yz,zz,zlabel,fr);
				%frametick(of[3],of[4],xl1,yl1,f[5],xl,yl,f[5],xz,yz,zz,ylabel,fr);
				if y<=f[3] then
					xl2=f[1]; yl2=f[3];
					%frametick(of[1],of[2],xl2,yl2,f[5],xl1,yl1,f[5],xz,yz,zz,xlabel,fr);
				elseif z>f[6] then
					xl2=f[1]; yl2=f[4];
					%frametick(of[1],of[2],xl2,yl2,f[6],xl,yl,f[6],xz,yz,zz,xlabel,fr);
				endif;
			elseif x<=f[2] and y<=f[3] then 
				xl=f[2]; yl=f[3]; xl1=f[1]; yl1=f[3];
				%frametick(of[5],of[6],xl,yl,f[5],xl,yl,f[6],xz,yz,zz,zlabel,fr);
				%frametick(of[1],of[2],xl1,yl1,f[5],xl,yl,f[5],xz,yz,zz,xlabel,fr);
				if x<=f[1] then
					xl2=f[1]; yl2=f[4];
					%frametick(of[3],of[4],xl1,yl1,f[5],xl2,yl2,f[5],xz,yz,zz,ylabel,fr);
				elseif z>f[6] then
					xl2=f[2]; yl2=f[4];
					%frametick(of[3],of[4],xl,yl,f[6],xl2,yl2,f[6],xz,yz,zz,ylabel,fr);
				endif;
			elseif x<=f[1] and y>=f[3] then 
				xl=f[1]; yl=f[3]; xl1=f[1]; yl1=f[4];
				%frametick(of[5],of[6],xl,yl,f[5],xl,yl,f[6],xz,yz,zz,zlabel,fr);
				%frametick(of[3],of[4],xl,yl,f[5],xl1,yl1,f[5],xz,yz,zz,ylabel,fr);
				if y>=f[4] then
					xl2=f[2]; yl2=f[4];
					%frametick(of[1],of[2],xl1,yl1,f[5],xl2,yl2,f[5],xz,yz,zz,xlabel,fr);
				elseif z>f[6] then
					xl2=f[2]; yl2=f[3];
					%frametick(of[1],of[2],xl,yl,f[6],xl2,yl2,f[6],xz,yz,zz,xlabel,fr);
				endif;
			else 
				xl=f[1]; yl=f[4]; xl1=f[2]; yl1=f[4];
				%frametick(of[5],of[6],xl,yl,f[5],xl,yl,f[6],xz,yz,zz,zlabel,fr);
				%frametick(of[1],of[2],xl,yl,f[5],xl1,yl1,f[5],xz,yz,zz,xlabel,fr);
				if x>=f[2] then
					xl2=f[2]; yl2=f[3];
					%frametick(of[3],of[4],xl2,yl2,f[5],xl1,yl1,f[5],xz,yz,zz,ylabel,fr);
				elseif z>f[6] then
					xl2=f[1]; yl2=f[3];
					%frametick(of[3],of[4],xl2,yl2,f[6],xl,yl,f[6],xz,yz,zz,ylabel,fr);
				endif;
			endif;
		else
			if x>=f[2] and y<=f[4] then 
				xl=f[2]; yl=f[4];
				%frametick(of[5],of[6],xl,yl,f[5],xl,yl,f[6],xz,yz,zz,zlabel,fr);
				xl2=f[1]; yl2=f[4];
				%frametick(of[1],of[2],xl2,yl2,f[5],xl,yl,f[5],xz,yz,zz,xlabel,fr);
				xl1=f[1]; yl1=f[3];
				%frametick(of[3],of[4],xl1,yl1,f[5],xl2,yl2,f[5],xz,yz,zz,ylabel,fr);
			elseif x<=f[2] and y<=f[3] then 
				xl=f[2]; yl=f[3];
				%frametick(of[5],of[6],xl,yl,f[5],xl,yl,f[6],xz,yz,zz,zlabel,fr);
				xl2=f[2]; yl2=f[4];
				%frametick(of[3],of[4],xl,yl,f[5],xl2,yl2,f[5],xz,yz,zz,ylabel,fr);
				xl1=f[1]; yl1=f[4];
				%frametick(of[1],of[2],xl2,yl2,f[5],xl1,yl1,f[5],xz,yz,zz,xlabel,fr);
			elseif x<=f[1] and y>=f[3] then 
				xl=f[1]; yl=f[3];
				%frametick(of[5],of[6],xl,yl,f[5],xl,yl,f[6],xz,yz,zz,zlabel,fr);
				xl2=f[2]; yl2=f[3];
				%frametick(of[1],of[2],xl,yl,f[5],xl2,yl2,f[5],xz,yz,zz,xlabel,fr);
				xl1=f[2]; yl1=f[4];
				%frametick(of[3],of[4],xl2,yl2,f[5],xl1,yl1,f[5],xz,yz,zz,ylabel,fr);
			else 
				xl=f[1]; yl=f[4]; 
				%frametick(of[5],of[6],xl,yl,f[5],xl,yl,f[6],xz,yz,zz,zlabel,fr);
				xl2=f[1]; yl2=f[3];
				%frametick(of[3],of[4],xl2,yl2,f[5],xl,yl,f[5],xz,yz,zz,ylabel,fr);
				xl1=f[2]; yl1=f[3];
				%frametick(of[1],of[2],xl2,yl2,f[5],xl1,yl1,f[5],xz,yz,zz,xlabel,fr);
			endif;
		endif;
	else
		if x>=f[2] and y<=f[4] then
			%frametick(of[5],of[6],f[2],f[4],f[5],f[2],f[4],f[6],xz,yz,zz,zlabel,fr);
		elseif y>=f[4] and x>=f[1] then
			%frametick(of[5],of[6],f[1],f[4],f[5],f[1],f[4],f[6],xz,yz,zz,zlabel,fr);
		elseif x<=f[1] and y>=f[3] then
			%frametick(of[5],of[6],f[1],f[3],f[5],f[1],f[3],f[6],xz,yz,zz,zlabel,fr);
		elseif y<=f[3] and x<=f[2] then
			%frametick(of[5],of[6],f[2],f[3],f[5],f[2],f[3],f[6],xz,yz,zz,zlabel,fr);
		endif;
		if x>=f[2] and z<=f[6] then
			%frametick(of[3],of[4],f[2],f[3],f[6],f[2],f[4],f[6],xz,yz,zz,ylabel,fr);
		elseif x>=f[1] and z>=f[6] then
			%frametick(of[3],of[4],f[1],f[3],f[6],f[1],f[4],f[6],xz,yz,zz,ylabel,fr);
		elseif x<=f[1] and z>=f[5] then
			%frametick(of[3],of[4],f[1],f[3],f[5],f[1],f[4],f[5],xz,yz,zz,ylabel,fr);
		elseif x<=f[2] and z<=f[5] then
			%frametick(of[3],of[4],f[2],f[3],f[5],f[2],f[4],f[5],xz,yz,zz,ylabel,fr);
		endif;
		if y>=f[4] and z<=f[6] then
			%frametick(of[1],of[2],f[1],f[4],f[6],f[2],f[4],f[6],xz,yz,zz,xlabel,fr);
		elseif y>=f[3] and z>=f[6] then
			%frametick(of[1],of[2],f[1],f[3],f[6],f[2],f[3],f[6],xz,yz,zz,xlabel,fr);
		elseif y<=f[3] and z>=f[5] then
			%frametick(of[1],of[2],f[1],f[3],f[5],f[2],f[3],f[5],xz,yz,zz,xlabel,fr);
		elseif y>=f[4] and z<=f[5] then
			%frametick(of[1],of[2],f[1],f[4],f[5],f[2],f[4],f[5],xz,yz,zz,xlabel,fr);
		endif;
	endif;
endfunction

function %slice (x,y,z,s)
	n=max(rows(x),rows(y));
	i=redim(dup(1:n,2)',[1,2*n]);
	if rows(x)>1 then x1=x[i,:]; else x1=x; endif;
	if rows(y)>1 then y1=y[i,:]; else y1=y; endif;
	if rows(z)>1 then 
		z1=z[i,:]; z1[1:2:2*n-1,:]=0;
	else 
		z1=z|0; 
	endif;
	return {x1,y1,z1}
endfunction

function cubescale (x,y,z)
## return a scaling that makes (x,y,z) fit to the unit cube.
	xs = 1/(totalmax(x)-totalmin(x));
	ys = 1/(totalmax(y)-totalmin(y));
	zs = 1/(totalmax(z)-totalmin(z));
	return [xs,ys,zs];
endfunction

function framedplot (f$:string, frame:real vector, 
	sframe:real vector=none, 
	xl:string="x", yl:string="y", zl:string="z",
	ticks:integer=1,
	view=none, zoom=none, distance=none, angle=none, height=none,
	center=none,fr=1)
## General function for framed plots.
##
## This function does the same as the functions called by plot3d. It
## draws a frame around a plot. The frame is drawn in two parts, one
## in the back and one in the front. It can display ticks, which may
## have nothing to do with the frame itself (sframe parameter).
##
## A function using this function can be used in plot3d with parameter
## >own to allow anaglyphs or turning.
##
## f : A 3D plot function. This can also be plot3d with parameter
## <frame. Semicolon parameters are passed to f.
##
## frame : The frame. take care that the plot fits into the frame. The
## frame is a vector of the form [xmin,xmax,ymin,ymax,zmin,zmax]
##
## sframe : The tick values for the frame. If sframe=none then frame
## is used.
##
## See: plot3d
	wi=fullwindow();
	if !holding() then clg; endif;
	h=holding(1);
	vold=view();
	if view<>none then view(view); endif;
	if zoom<>none then zoom(zoom); endif;
	if distance<>none then viewdistance(distance); endif;
	if angle<>none then viewangle(angle); endif;
	if height<>none then viewheight(height); endif;	
	oldcenter=center();
	if center!=none then center(center); endif;
	%frame1(frame,fr);
	f$(args());
	%frame2(frame,fr);
	if sframe==none then sframe=frame; endif;
	if ticks then %frameticks(frame,sframe,xl,yl,zl,fr); endif;
	view(vold);
	center(oldcenter);
	holding(h);	
	window(wi);
endfunction

function framedsolid (x:real, y:real, z:real,
	scale:real vector=1, fr:integer=1, hf:number=1,
	disconnect=none,
	xlabel:string="x", ylabel:string="y", zlabel:string="z",
	sliced:integer=0, bar:integer=0, z0:real=0,
	frame:real vector=none, sframe:real vector=none,
	cp=0, cpcolor=1, cplevel="auto", cpdelta=0.05, nc=defaultnc,
	linewidth=none)
## 3D plot with frame.
##
## x, y, and z must be compatible matrices. The determine the
## coordinates of the 3D images of a parameter matrix.
##
## If scale is specified, then the plot is scaled to fit into a cube
## of side length 2*scale centered at 0.
##
## If fr=0 then no frame will be drawn around the plot.
##
## disconnect is a vector of indices containing the rows of the
## matrix, which should not be connected.
##
## sliced is a parameter, which produces sliced plots into the x, or
## the y direction. Should only be used for regular x,y grids.
##
## bar is a parameter, which makes the plot into steps. Should only be
## used for regular x,y grids.
##
## This function should only be used via plot3d.
##
## See: plot3d
	if sliced==1 then 
		{x,y,z}=%slice(x,y,z,sliced);
		disconnect=2:2:max(rows(x),rows(y))-2;
	elseif sliced==2 then
		{x,y,z}=%slice(x',y',z',sliced);
		disconnect=2:2:max(rows(x),rows(y))-2;
	endif;
	ownframe=0;
	if frame==none then 
		frame=getframe(x,y,z); 
	else 
		ownframe=1;
		if sframe==none then sframe=frame; endif;
		scale=0; 
	endif;
	if !holding() then clg; endif;
	h=holding(1);
	if sframe==none then
		sframe=frame; 
		sframe[5]=sframe[5]/hf+z0; sframe[6]=sframe[6]/hf+z0;
	endif;
	niv=0;
	if all(scale>0) then 
		{x,y,z,m,ff}=%scaleframe(x,y,z,frame,scale); 
		niv=(niv-m[3])*ff[3];
	endif;
	lw=_linewidth(linewidth);
	if cp && vieweye()[3]>totalmin(z) then
		plotcontourplane(x,y,z,cp, ..
			cpcolor,cplevel,cpdelta,nc);
	endif;
	_linewidth(lw);
	%frame1(frame,fr);
	lw=_linewidth(linewidth);
	if bar then
		plot3dbars(x,y',z,min(max(niv,totalmin(z)),totalmax(z)));
	elseif typeof(disconnect)==8 then 
		solid(x,y,z); 
	else 
		solid(x,y,z,disconnect);
	endif;
	_linewidth(lw);
	if fr then 
		color(2); 
		%frame2(frame,fr);
		%frameticks(frame,sframe,xlabel,ylabel,zlabel,fr); 
		color(1); 
	endif;
	if cp && vieweye()[3]<totalmin(z) then
		plotcontourplane(x,y,z,cp, ..
			cpcolor,cplevel,cpdelta,nc);
	endif;
	holding(h);
	return frame
endfunction

function %scaleframelevel (x,y,z,values,level,%f,m,hf,z0=0)
## Scales the frame, the x,y,z and the level values
	s=max([%f[2]-%f[1],%f[4]-%f[3],%f[6]-%f[5]]);
	xm=(%f[2]+%f[1])/2;
	ym=(%f[4]+%f[3])/2;
	zm=(%f[6]+%f[5])/2;
	ff=m/s*2;
	%f=[(%f[1]-xm)*ff{1},(%f[2]-xm)*ff{1}, ..
		(%f[3]-ym)*ff{2},(%f[4]-ym)*ff{2}, ..
		(%f[5]-zm)*ff{3},(%f[6]-zm)*ff{3}];
	return {(x-xm)*ff{1},(y-ym)*ff{2},(z-zm)*ff{3}, ..
		(values-zm)*ff{3},((level-zm-z0)*hf)*ff{3}}
endfunction

function autolevel (a,b,nc,d=0.1)
## Compute automatic levels between a and b
	if d>0 then
		s=linspace(a,b,nc);
		d=(b-a)/nc*0.1;
		return (s-d)_(s+d);
	else
		return linspace(a,b,nc);
	endif;
endfunction

function plotcontourplane (x, y, z, style=1, color=1,
	level=none, delta=0.05, nc=defaultnc, grid=false,
	contourcolor=none, scale=false, linewidth=none)
## Plot a contour plane below the 3D plot
##
## This is called by plot2d() if >cp is set. The functions projects
## the contours of the plane with coordinates x,y,z to a plane below
## the plot.
##
## x,y,z : coordinates of the surface
## style : 1=level lines only, 2=shading and level lines
## level : "auto", "thick", "thin" or level values
##   If level is a 2xn matrix it plots ranges
## cpcolor : color of the shading
## cpdelta : distance from below the plot
## nc : number of lines for automatic levels
## grid : grid values for an additional grid.
##
## See: plot2d
	if style==0 then return; endif;
	
	if scale then 
		frame=getframe(x,y,z); 
		niv=0;
		if all(scale>0) then 
			{x,y,z,m,ff}=%scaleframe(x,y,z,frame,scale); 
			niv=(niv-m[3])*ff[3];
		endif;	
	endif;
	
	planedelta=(totalmax(z)-totalmin(z))*delta;
	zhue=scalematrix(z)*0.99999;
	
	if level=="off" then level=none; endif;
	
	if level=="auto" or level=="thick" then
		level=autolevel(totalmin(z),totalmax(z),nc,0.1);
	elseif level=="thin" then
		level=autolevel(totalmin(z),totalmax(z),nc,0);
	endif;

	hg=huegrid(grid);	
	if grid then
		subgrid(length(x)/grid);
	endif;
	wc=contourcolor(contourcolor);
	col=huecolor(color);
	lw=_linewidth(linewidth);
	
	if style==2 then
		_solid(x,y,totalmin(z)-planedelta,none,0,z,level,none);	
	elseif style==3 then
		_solid(x,y,totalmin(z)-planedelta,none,zhue);	
	else
		hc=huecolor(color);
		_solid(x,y,totalmin(z)-planedelta,none,zhue,z,level,none);
		huecolor(hc);
	endif;
	
	huecolor(col);
	contourcolor(wc);
	_linewidth(lw);
	huegrid(hg);
endfunction

function vieweye ()
## The position of the eye depending on center and view
	v=view();
	return center()+
		v[1]*[cos(v[3])*cos(v[4]),cos(v[3])*cos(v[4]),sin(v[4])];
endfunction

function framedsolidhue (x:real, y:real, z:real, hue, ..
	scale:real vector=1, f:number=1, fr:integer=1, hf:number=1, ..
	xlabel:string="x", ylabel:string="y", zlabel:string="z", ..
	level=none, values=none,
	light=[0.5,0.5,1], amb=0.1, max=0.9, huecolor=0, ..
	contourcolor=1, contourwidth=1, z0=0,
	frame=none, sframe=none,
	dl=none, limits=none, 
	nc=defaultnc, cp=0, cpcolor=1, cplevel="auto", cpdelta=0.05,
	linewidth=none, zlim=none)
## 3D plot with hue, and optional level lines.
##
## x, y, and z must be compatible matrices. They determine the
## coordinates of the 3D images of a parameter matrix. hue is an
## additional compatible matrix, which determines the color shade of
## the matrix at each point. E.g., plot3d computes the hue using a
## light source.
##
## level is a row vector of level levels, which are to be drawn.
## values is a matrix of the same size as x, y, and z which contains
## the values, which determine the level. By default, the value will
## be z (values=none).
##
## If scale is specified, then the plot is scaled to fit into a cube
## of side length 2*scale centered at 0.
##
## If fr=0 then no frame will be drawn around the plot.
##
## disconnect is a vector of indices containing the rows of the
## matrix, which should not be connected.
##
## This function should only be used via plot3d.
##
## See: plot3d
	if dl and dl>0 and !isstring(level) and rows(level)==1 then
		level=level+[-dl;dl];
		dl=none;
	endif;
	if frame==none then 
		frame=getframe(x,y,z); 
		if zlim!=none then
			frame[5]=zlim[1]; frame[6]=zlim[2];
		endif;
	else 
		frame=frame;
		if sframe==none then sframe=frame; endif;
	endif;
	if values==none then values=z; endif;
	if !holding(); clg; endif;
	h=holding(1);
	if sframe==none then
		sframe=frame; 
		sframe[5]=sframe[5]/hf+z0; sframe[6]=sframe[6]/hf+z0;
	endif;
	if all(scale>0) then 
		{x1,y1,z1}=%scaleframe(x,y,z,frame,scale);
	else 
		{x1,y1,z1}={x,y,z}; 
	endif;
	if hue==none then 
		hue=computehue(x1,y1,z1,light,amb,max,huecolor); 
	endif;
	if cp && vieweye()[3]>totalmin(z1) then
		plotcontourplane(x1,y1,z1,cp, ..
			cpcolor,cplevel,cpdelta,=nc,=contourcolor,
			=linewidth);
	endif;
	if fr then 
		%frame1(frame,fr); 
	endif;
	if f then
		if typeof(level)==8 then 
			solidhue(x1,y1,z1,hue,f,level, ..
				=contourcolor,=contourwidth,=nc,=dl,=limits,=linewidth); 
		else 
			solidhue(x1,y1,z1,hue,f,=level,=values, ..
			 	=contourcolor,=contourwidth,=nc,=dl,=limits,=linewidth); 
		endif;
	else
		if level==none then
			_solidhue(x1,y1,z1,hue,=limits);
		elseif typeof(level)==8 then 
			solidhue(x1,y1,z1,hue,"auto", ..
				=contourcolor,=contourwidth,=nc,=dl,=limits,=linewidth); 
		else 
			solidhue(x1,y1,z1,hue,level=level,values=values, ..
				=contourcolor,=contourwidth,=nc,=dl,=limits,=linewidth); 
		endif;
	endif;
	if fr then
		color(2); %frame2(frame,fr);
		%frameticks(frame,sframe,xlabel,ylabel,zlabel,fr);
		color(1);	
	endif;
	if cp && vieweye()[3]<totalmin(z1) then
		plotcontourplane(x1,y1,z1,cp, ..
			cpcolor,cplevel,cpdelta,=nc,=contourcolor);
	endif;
	holding(h);
	return frame
endfunction

function framedwire (x:real, y:real, z:real,
	scale:real vector=1, fr:integer=1, hf:number=1,
	xlabel:string="x", ylabel:string="y", zlabel:string="z",
	wirecolor:number=none, rowsonly:integer=0, z0:real=0,
	frame:real vector=none, sframe:real vector=none,
	cp=0, cpcolor=1, cplevel="auto", cpdelta=0.05,
	linewidth=none)
## 3D wire plot with frame.
##
## x, y, and z must be compatible matrices. They determine the
## coordinates of the 3D images of a parameter matrix.
##
## If scale is specified, then the plot is scaled to fit into a cube
## of side length 2*scale centered at 0.
##
## If fr=0 then no frame will be drawn around the plot.
##
## With rowsonly=1 the wire will be drawn only in one direction. 
##
## The wirecolor can be specified with the variable wirecolor. Use any
## of the 16 predefined colors, or an rgb value.
##
## This function should only be used via plot3d.
##
## See: plot3d,rgb
	if frame==none then 
		frame=getframe(x,y,z); 
	else 
		if sframe==none then sframe=frame; endif;
		scale=0; 
	endif;	
	if !holding(); clg; endif;
	h=holding(1);
	if sframe==none then
		sframe=frame; 
		sframe[5]=sframe[5]/hf+z0; sframe[6]=sframe[6]/hf+z0;
	endif;
	if all(scale>0) then 
		{x1,y1,z1}=%scaleframe(x,y,z,frame,scale);
	else 
		{x1,y1,z1}={x,y,z}; 
	endif;
	if cp && vieweye()[3]>totalmin(z1) then
		plotcontourplane(x1,y1,z1,cp, ..
			cpcolor,contourlevel,cpdelta,nc,=linewidth);
	endif;
	if fr then %frame1(frame,fr); endif;
	wc=wirecolor(wirecolor); 
	lw=_linewidth(linewidth);
	if rowsonly then wirerows(x1,y1,z1); else wire(x1,y1,z1); endif;
	wirecolor(wc);
	_linewidth(lw);
	if fr then
		color(2); %frame2(frame,fr);
		%frameticks(frame,sframe,xlabel,ylabel,zlabel,fr); 
		color(1);
	endif;
	if cp && vieweye()[3]<totalmin(z1) then
		plotcontourplane(x1,y1,z1,cp, ..
			cpcolor,contourlevel,cpdelta,nc);
	endif;
	holding(h);
	return frame
endfunction

function framedmark3 (x:real, y:real, z:real,
	scale:real vector=1, fr:integer=1, hf:number=1,
	xlabel:string="x", ylabel:string="y", zlabel:string="z",
	color=1, z0:number=0, 
	frame:real vector=none, sframe:real vector=none)
## 3D point plot with frame.
##
## x, y, and z must be row vectors. They determine the coordinates of
## the 3D images of the points.
##
## This function should only be used via plot3d.
##
## See: plot3d,rgb
	if frame==none then 
		frame=getframe(x,y,z); 
	else 
		if sframe==none then sframe=frame; endif;
		scale=0; 
	endif;	
	if !holding(); clg; endif;
	h=holding(1);
	if sframe==none then
		sframe=frame; 
		sframe[5]=sframe[5]/hf+z0; sframe[6]=sframe[6]/hf+z0;
	endif;
	if all(scale>0) then 
		{x1,y1,z1}=%scaleframe(x,y,z,frame,scale);
	else
		{x1,y1,z1}={x,y,z}; 
	endif
	if fr then %frame1(frame,fr); endif;
	{c0,r0,dist}=project(x1,y1,z1);
	{x0,y0}=fromscreen(c0,r0);
	if length(color)>1 and rows(x)==1 and rows(y)==1 then
		{dist,id}=sort(-dist);
		markwithcolor(x0[id],y0[id],color[id]);
	else
		oc=_color(color);
		mark(x0,y0);
		color(oc);
	endif;
	if fr then
		color(2); %frame2(frame,fr);
		%frameticks(frame,sframe,xlabel,ylabel,zlabel,fr);
		color(1);
	endif;
	holding(h);
	return frame;
endfunction

function overwrite subgrid (v)
## Sets the sub-grid for grid plots
##
## This is usually called by plot2d() automatically. The sub-grid
## determines the number of grid lines which are drawn in a 3D plot.
## This allows a finer plot than the visible grid lines indicate. In
## plot2d(), the parameter grid= controls the sub-grids.
##
## Calls the built-in function _subgrid, which works with 1x2 vectors.
##
## >plot3d("x^2-y^3",grid=[10,5]):
##
## See: plot2d
	if typeof(v)==0 then return _subgrid([v,v]) endif;
	return _subgrid(v);
endfunction

// * MatPlotLib

// MatPlotLib is a very nice plotting library for Python. It has to be
// installed separately into Python. Consult the remarks about the
// installation of EMT for more details.

function pyins (lines=35, close=true)
## Insert a Python plot into the notebook
##
## This saves a plot done with MatPlotLib in the working directory of
## EMT, and loads the saved file to the current notebook. By default,
## the plot is closed. Unclosed plots can be displayed with show() in
## a separate window or saved in other formats too.
##
## lines : number of lines for the plot in the notebook
## close : close the plot
##
## See: pymatplotib
	file=eulerhome()+"pyout.png";
	pyset("pyout",file);
	python("plt.savefig(pyout)");
	if close then python("plt.close()"); endif;
	loadimg(file,lines);
	pyout$$=file;
endfunction

// * Logarithmic Plots

// These function should not be called directly. Use plot2d to produce
// log plots.

function logticks (aa, bb=none, base=10)
## Logarithmic ticks
##
## See: xlogplot
	if bb!=none then a=aa; b=bb;
	else a=min(aa); b=max(aa);
	endif;
	if (b>1e30); b=1e30; endif;
	if (a<-1e30); a=-1e30; endif;
	if (a>=b); b=a+1; endif;
	tick=10^floor(log(b-a)/log(10)-0.4);
	if b-a>10*tick then tick=tick*2; endif;
	if b-a<5*tick then tick=tick/2; endif;
	if tick<1 then tick=1; endif;
	res=(floor(a/tick))*tick:tick:(ceil(b/tick))*tick;
	return res[nonzeros(res>=aa && res<=bb)];
endfunction;

function xlogplot (x:positive, y, xbase=10, color=1, 
	points=false, frame=1, grid=2, ticks=1)
## Logarithmic plot
##
## Instead of this, plot2d() should be used with the parameter
## logplot=1.
##
## See: plot2d, ylogplot, xylogplot

	global logredraw;
	if !holding() then clg; endif;
	
	xl=logbase(x,xbase);
	
	c=_color();
	
	global defaultgridcolor;
	_color(defaultgridcolor);
	
	if holding() && !logredraw then
		hd=holding();
		p=plot();
	else
		hd=holding(1);
		
		expiy=totalmin(y);
		expfy=totalmax(y);

		expix=floor(logbase(totalmin(x),xbase));
		expfx=ceil(logbase(totalmax(x),xbase));
		
		xt=logticks(expix,expfx,xbase);
		yt=ticks(expiy,expfy);

		p=setplot([expix,expfx,expiy,expfy]);
		pf=fullplotarea();

		h=textheight(); w=textwidth();
		
		xgrid(xt,1,grid,<ticks);
		ygrid(yt,1,grid,>ticks);

		dh=4*textwidth();
		if ticks then
			for k=1 to length(xt);
				i=floor(xt[k]+0.2);
				d=toscreen([i,pf[3]]);
				rtext(printf("%g",xbase),[d[1]+w,d[2]+0.75*h]);
				text(printf("%g",i),[d[1]+w,d[2]+h/4]);
			end;
		endif;
				
		if frame then frame(); endif;
	endif;
	
	_color(color);
	
	if points then mark(xl,y);
	else plot(xl,y);
	endif;
	
	_color(c);

	holding(hd);
	
	logredraw=0;
	return p;
endfunction;

function ylogplot (x, y:positive, ybase=10, color=1, points=false, 
	frame=1, grid=2, ticks=1)
## Logarithmic plot
##
## Instead of this, plot2d() should be used with the parameter
## logplot=2.
##
## See: plot2d, xlogplot, xylogplot

	global logredraw;
	if !holding() then clg; endif;
	
	yl=logbase(y,ybase);
	
	c=_color();
	
	global defaultgridcolor;
	_color(defaultgridcolor);
	
	if holding() && !logredraw then
		hd=holding();
		p=plot();
	else
		hd=holding(1);
		
		expix=totalmin(x);
		expfx=totalmax(x);
		
		expiy=floor(logbase(totalmin(y),ybase));
		expfy=ceil(logbase(totalmax(y),ybase));

		xt=ticks(expix,expfx);
		yt=logticks(expiy,expfy,ybase);

		p=setplot([expix,expfx,expiy,expfy]);
		pf=fullplotarea();

		h=textheight(); w=textwidth();
		
		xgrid(xt,1,grid,>ticks);
		ygrid(yt,1,grid,<ticks);

		dh=4*textwidth();
		if ticks then
			for k=1 to length(yt);
				i=floor(yt[k]+0.2);
				d=toscreen([pf[1],i]);
				rtext(printf("%g",ybase),[d[1]-dh,d[2]-h/2]);
				text(printf("%g",i),[d[1]-dh,d[2]-h]);
			end;
		endif;
				
		if frame then frame(); endif;
	endif;
	
	_color(color);
	
	if points then mark(x,yl);
	else plot(x,yl);
	endif;
	
	_color(c);

	holding(hd);
	
	logredraw=0;
	return p;
endfunction;

function xylogplot(x:positive, y:positive, 
	xbase=10, ybase=10, color=1, points=0, 
	frame=1, grid=2, ticks=1)
## Logarithmic plot
##
## Instead of this, plot2d() should be used with the parameter
## logplot=3.
##
## See: plot2d, xlogplot, ylogplot

	global logredraw;
	if !holding() then clg; endif;
	
	xl=logbase(x,xbase);
	yl=logbase(y,ybase);
	
	c=_color();
	
	global defaultgridcolor;
	_color(defaultgridcolor);
	
	if holding() && !logredraw then
		hd=holding();
		p=plot();
	else
		hd=holding(1);
		
		expix=floor(logbase(totalmin(x),xbase));
		expfx=ceil(logbase(totalmax(x),xbase));
		expiy=floor(logbase(totalmin(y),ybase));
		expfy=ceil(logbase(totalmax(y),ybase));
		
		xt=logticks(expix,expfx,xbase);
		yt=logticks(expiy,expfy,ybase);

		p=setplot([expix,expfx,expiy,expfy]);
		pf=fullplotarea();

		h=textheight(); w=textwidth();
		
		xgrid(xt,1,grid,<ticks);
		ygrid(yt,1,grid,<ticks);

		dh=4*textwidth();
		if ticks then
			for k=1 to length(xt);
				i=floor(xt[k]+0.2);
				d=toscreen([i,pf[3]]);
				rtext(printf("%g",xbase),[d[1]+w,d[2]+0.75*h]);
				text(printf("%g",i),[d[1]+w,d[2]+h/4]);
			end;
			for k=1 to length(yt);
				i=floor(yt[k]+0.2);
				d=toscreen([pf[1],i]);
				rtext(printf("%g",ybase),[d[1]-dh,d[2]-h/2]);
				text(printf("%g",i),[d[1]-dh,d[2]-h]);
			end;
		endif;
				
		if frame then frame(); endif;
	endif;
	
	_color(color);
	
	if points then mark(xl,yl);
	else plot(xl,yl);
	endif;
	
	_color(c);

	holding(hd);
	
	logredraw=0;
	return p;
endfunction;

// * LaTeX

// LaTeX needs to be installed separately. I recommend MiKTeX for
// Windows. If you do not install the full package, you may have to
// wait a bit, when MiKTeX installs necessary packages. EMT may issue
// a warning.

function comment tex (expr:string)
## LaTeX code of the expression.
##
## This calls Maxima to for the LaTeX code of the expression. The
## result is a string.
##
## See: texprint
endfunction

function overwrite texpng (latex:string,
	usealias=true, factor=1.0, backcolor=-1, color=none)
## PNG file containing the scanned LaTeX code.
##
## LaTeX must be installed for this. The string will be inserted as a
## display formula into a LaTeX document in the working directory of
## EMT. Then LaTeX will be called to translate the document. Finally
## the DVI file will be transformed into a PNG with "dvipng".
##
## The text color is the current text color as set with textcolor(),
## unless set as parameter color. The background color is the color
## number 0 of the graphics, usually white. The font size is the
## current font size. The scan is always tree times larger than
## necessary to allow for anti-aliasing.
##
## usealias : dvipng should alias the formula
## factor : make the formula larger by this factor
## transparent : use -1 for the current background color of the
##    graphics, and 0 for the background color of the text.
##    1 yields a transparent graphics. Note that loadrgb()
##    ignores transparency.
## color : use another color for the text
##
## The output is C:\Users\Username\Euler\eulertemp.png. It is usually
## loaded by loadrgb() and drawn into the graphics with plotrgb(). The
## function label() does this.
##
## This is calling the built-in function texpng() with the same
## arguments.
##
## See: loadrgb, label, xlabel, ylabel, setfont, latex:
	if color==none then color=defaulttextcolor; endif;
	return _texpng(latex,usealias,factor,backcolor,color);
endfunction

function latex (s:string, color=none, factor=1.0)
## RGB matrix containing the scanned LaTeX code.
##
## s : LaTeX code
## color : the text color; if none, the default text color is used.
## factor: scale the Latex output by this factor.
##
## This combines texpng() and loadrgb() to scan a LaTeX formula to an
## RGB matrix. With plotrgb() such a matrix can be plotted into a plot
## window. For more details, see texpng().
##
## A symbolic expression can be transformed to Latex by Maxima with
## tex(expr).
##
## Returns an image as a matrix of RGB values.
##
## >expr &= integrate(x^2*sin(x),x)
## >plot2d(expr,0,2pi,grid=6); label(latex(tex(expr)),1,-10):
## 
## See: text, texpng, plotrgb, label, xlabel, ylabel, tex
	if color==none then color=defaulttextcolor; endif;
	return loadrgb(texpng(s,=color,=factor));
endfunction

function overwrite text (s, c:real, r:real=none, color=none, 
	align:integer=-1, vertical=0,
	left=none, center=none, right=none, up=none, down=none,
	scale=1/3, tcolor=0) 
## Put text or RGB matrix on the screen.
##
## This function works for a single string, a vector of strings, or an
## RGB matrix of an image, e.g. a LaTeX formula. It can align the text
## left, center, or right, or vertically up or down.
##
## RGB images can be from the latex() function. This functions returns
## three times enlarged images. Thus the default scale factor is 1/3.
##
## s : string, string vector, or RGB image
## c : column in screen coordinates or vector [c,r]
## r : row in screen coordinates
## color : text color
## align : -1,0,1 for left, center, right
## vertical : 0,1,2 for not vertical, up, down
## left, center, right : sets align
## up, down : sets vertical
## scale : scale for the RGB image
## tcolor : transparency color for the RGB image
##
## >plot2d(''integrate("x^x",1,x)'',0,2,grid=6); ...
## >text(latex("f(x) = 'integrate(t^t,t,1,x)"),toscreen(0.5,1)); ...
## >g &= integrate(taylor(t^t,t,1,2),t,1,x); ...
## >plot2d(g,color=red,>add); ...
## >text(latex("g(x) = T_2f(x)",color=red),toscreen(0.5,0.7)):
## 
## See: latex, plotrgb, label, text
	if r==none then
		if cols(c)==2 then r=c[2]; c=c[1];
		else error("Illegal position in text().");
		endif;
	endif;
	if left then align=-1; endif;
	if right then align=1; endif;
	if center then align=0; endif;
	if up then vertical=1; endif;
	if down then vertical=2; endif;
	if typeof(s)==8 or typeof(s)==13 then
		if color then tc=textcolor(color); endif;
		th=textheight();
		pos=[c,r];
		if vertical then
			if vertical==1 then
				loop 1 to length(s);
					if align==-1 then
						_vutext(s{#},pos);
					elseif align==0 then
						_vcutext(s{#},pos);
					else
						_vrutext(s{#},pos);
					endif;
					pos[1]=pos[1]+th*1.2;
				end
			else
				loop 1 to length(s);
					if align==-1 then
						_vtext(s{#},pos);
					elseif align==0 then
						_vctext(s{#},pos);
					else
						_vrtext(s{#},pos);
					endif;
					pos[1]=pos[1]-th*1.2;
				end
			endif;
		else
			loop 1 to length(s);
				if align==-1 then
					_text(s{#},pos);
				elseif align==0 then
					_ctext(s{#},pos);
				else
					_rtext(s{#},pos);
				endif;
				pos[2]=pos[2]+th*1.2;
			end
		endif;
		if color then textcolor(tc); endif;	
	else
		px=abs(_toscreen(pixel())-_toscreen([0,0]));
		w=px[1]*cols(s)*scale;
		h=px[2]*rows(s)*scale;
		if vertical==1 then
			s=flipy(s)';
			if align==0 then r=r-w/2;
			elseif align==-1 then r=r-w;
			endif;
			plotrgb(s,[c,r,c+h,r+w],=tcolor);
		elseif vertical==2 then
			s=flipx(s)';
			c=c-h;
			if align==0 then r=r-w/2;
			elseif align==1 then r=r-w;
			endif;
			plotrgb(s,[c,r,c+h,r+w],=tcolor);
		else
			if align==0 then c=c-w/2;
			elseif align==1 then c=c-w;
			endif;
			plotrgb(s,[c,r,c+w,r+h],=tcolor);
		endif;
	else
		error("Text needs a string, string vector, or RGB matrix!");
	endif
endfunction
