// Spherical Functions

comment
Spherical functions for Euler. 
endcomment

// See: ../Programs/Examples/Geometry on the Earth

// * Spherical Geometry

// Load this file with "load spherical"

function spoint (pos:vector)
## Get point on the sqhere from [lat,long] with N>0, S<0, E>0, W<0
## Returns a 1x3 vector
##
## File: spherical
	c=cos(pos[1]);
	return [c*cos(pos[2]),c*sin(pos[2]),sin(pos[1])]
endfunction

function sposprint (pos:vector, sec:integer=0)
## Print [lat,long] in usual form
## sec=1 : use DMS
## Returns a string
##
## File: spherical
	lat=pos[1]; long=pos[2];
	if lat>=0 then s="N "; else s="S "; lat=-lat; endif;
	s=s|degprint(lat,sec);
	if long>=0 then s=s|" E "; else s=s|" W "; long=-long; endif;
	return s|degprint(long,sec);
endfunction

function sposition (point:vector)
## Convert a point on the sphere to a position [lat,long]
## Returns a 1x2 vector
##
## File: spherical
	lat=polar(norm([point[1],point[2]]),point[3]);
	long=polar(point[1],point[2]);
	return [lat,long];
endfunction

function svector (frompos:vector, topos:vector)
## Difference vector of two positions [lat,long] on an ideal ball.
## Note that following the direction will not reach the destination
## exactly.
## Returns [direction,distance], both in radians.
##
## File: spherical
## See: saddvector
	xfrom=spoint(frompos); 
	northpole=[0,0,1]; 
	v1=crossproduct(northpole,xfrom); v2=-crossproduct(v1,xfrom);
	xto=spoint(topos);
	delta=xto-xfrom;
	v1=v1/norm(v1); v2=v2/norm(v2);
	a=scalp(delta,v2); b=scalp(delta,v1);
	p=polar(a,b);
	return [p[1],2*asin(norm(delta)/2)];
endfunction

function sangle (A:vector, B:vector, C:vector)
## Compute the angle ABC at B on a perfect sphere.
##
## File: spherical
	a=svector(B,A)[1]-svector(B,C)[1];
	if a<0 then a=a+2*pi;
	elseif a>=2*pi then a=a-2*pi;
	endif;
	if a>pi then a=2*pi-a; endif;
	return a;
endfunction

function strianglearea (A:vector, B:vector, C:vector)
## Compute the area of a triangle in space.
##
## File: spherical
	return norm(crossproduct(spoint(A)-spoint(B),spoint(A)-spoint(C)))/2;
endfunction

function sarea (A:vector, B:vector, C:vector)
## Compute the area of a triangle on a perfect sphere.
##
## File: spherical
	a=sangle(A,B,C)+sangle(B,C,A)+sangle(C,A,B)-pi;
	if abs(a)<1e-6 then return strianglearea(A,B,C);
	else return a;
	endif;
endfunction

function esarea (A:vector, B:vector, C:vector)
## Compute the area of a triangle on the earth
## For small triangles this is not very accurate.
##
## File: spherical
	return sarea(A,B,C)*rearth((A[1]+B[1]+C[1])/3)^2;
endfunction

function esdist (frompos:vector, topos:vector)
## Distance of two positions [lat,long] on the earth.
## This functions uses an average earth radius between the two points.
## esdist (earth spherical distance).
## Return distance in m
##
## File: spherical
	r1=rearth(frompos[1]); 
	r2=rearth(topos[1]);
	xfrom=spoint(frompos)*r1; 
	xto=spoint(topos)*r2;
	delta=xto-xfrom;
	return asin(norm(delta)/(r1+r2))*(r1+r2);
endfunction
	
function esdir (frompos:vector, topos:vector)
## Heading between two positions [lat,long] on the earth.
## esdirection (earth spherical direction).
## Return distance in m
##
## File: spherical
	r1=rearth(frompos[1]); 
	r2=rearth(topos[1]);
	xfrom=spoint(frompos)*r1; 
	xto=spoint(topos)*r2;
	northpole=[0,0,1]; 
	v1=crossproduct(northpole,xfrom); v2=-crossproduct(v1,xfrom);
	v1=v1/norm(v1); v2=v2/norm(v2);
	delta=xto-xfrom; delta=delta/norm(delta);
	a=scalp(delta,v2); b=scalp(delta,v1);
	p=(polar(a,b))[1];
	if p<0 then return p+2*pi else return p; endif;
endfunction

function skmprint (x) := print(x/km$,digits=3,unit="km")
function sdegprint (x) := print(deg(x),digits=2,unit="°")

function saddvector (pos:vector, heading:vector)
## Add a vector v to a position, all elements in degrees.
## This would be the position, which you see in that distance
## under that heading from the position pos.
## Returns the new position.
##
## File: spherical
## See: svector,esaddvector
	xpos=spoint(pos); northpole=[0,0,1];
	v1=crossproduct(northpole,xpos); v2=-crossproduct(v1,xpos);
	v1=v1/norm(v1); v2=v2/norm(v2);
	v=v2*cos(heading[1])+v1*sin(heading[1]); v=v/norm(v);
	return sposition(cos(heading[2])*xpos+sin(heading[2])*v);
endfunction

function esadd (pos:vector, direction:number, distance:number)
## Add a distance into the direction on the surface of the earth
## This will only work exactly for small distances.
## Returns the new postion
##
## File: spherical
## See: saddvector
	newpos=saddvector(pos,[direction,distance/rearth(pos[1])]);
	return saddvector(pos,[direction,distance/rearth((pos[1]+newpos[1])/2)]);
endfunction

function navigate (P1:vector, P2:vector, n:index=10)
## Navigate from p1 to p2
## Prints a series of GPS positions and headings to follow.
	v=P1;
	p=P1;
	loop 1 to n;
		p=esadd(p,esdir(p,P2),esdist(p,P2)/(n+1-#));
		v=v_p;
	end;
	return v;
endfunction

function plotearth (lat=-90:30:90, long=0:30:330, labels=1)
## Plots the earth with a grid of coordinates
##
## File: spherical
	mc=wirecolor(3);
	t=linspace(-pi/2,pi/2,100); s=rad(long)';
	wirerows(cos(t)*cos(s),cos(t)*sin(s),sin(t));
	h=holding(1); 
	wirecolor(4);
	t=rad(lat)'; s=linspace(0,2*pi,200);
	wirerows(cos(t)*cos(s),cos(t)*sin(s),sin(t)); 
	lw=linewidth(3);
	wirecolor(3);
	t=rad(-90:90:90)'; s=linspace(0,2*pi,200);
	wirerows(cos(t)*cos(s),cos(t)*sin(s),sin(t));
	wirecolor(4); 
	t=linspace(-pi/2,pi/2,100); s=rad(0:90:270)';
	wirerows(cos(t)*cos(s),cos(t)*sin(s),sin(t));
	linewidth(lw);
	wirecolor(mc);
	holding(h);
	if labels then
		{c,r}=project(1.2,0,0);
		ctext("0°",c,r);
		{c,r}=project(-1.2,0,0);
		ctext("180°",c,r);
		{c,r}=project(0,1.2,0);
		ctext("90°",c,r);
		{c,r}=project(0,-1.2,0);
		ctext("-90°",c,r);
		{c,r}=project(0,0,1.2);
		ctext("N",c,r);
		{c,r}=project(0,0,-1.2);
		ctext("S",c,r);
	endif;
endfunction

function plotpos (pos:vector, label="", style="o", col=1)
## Plot a postion and a label
##
## File: spherical
	h=holding(1);
	c=color(col);
	x=spoint(pos);
	{c,r}=project(x[1],x[2],x[3]);
	{s,t}=fromscreen(c,r);
	st=markerstyle(style); mark(s,t); markerstyle(st);
	x=x*1.3;
	{c,r}=project(x[1],x[2],x[3]);
	ctext(label,c,r);	
	color(c);
	holding(h);
endfunction

function plotposline (pos, thickness=2, color=1)
## Plot a line of positions
##
## File: spherical
	h=holding(1);
	n=rows(pos);
	x=zeros(n,3);
	loop 1 to n; x[#]=spoint(pos[#]); end;
	x=x';
	lw=linewidth(thickness);
	c=wirecolor(color);
	wirerows(x[1],x[2],x[3]);
	wirecolor(c);
	linewidth(lw);
	holding(h);
endfunction

submenu Earth Geometry
addmenu rad(?deg,?min?sec)
addmenu ?location=[?northRad,?eastRad)
addmenu sposprint(?location)
addmenu svector(?location,?location)
addmenu sangle(?location,?location,?location)
addmenu esarea(?location,?location,?location)
addmenu esdist(?location,?location)
addmenu esdir(?location,?location)
addmenu esadd(?location,?esDirection,?esDistance)
addmenu skmprint(?esDistance)
addmenu rearth(?northRad)

submenu Spherical Plots
addmenu plotearth
addmenu plotpos(?location,"?Name")
addmenu ?vector=zeros(0,2)
addmenu v_esadd(?locationVector,esDirection,?esDistance)
addmenu plotposline(?locationVector)
addmenu plot3d("?sphericalPlotFunction",own=1,user=1,zoom=4);

