// Geometry in Euler

comment
Numerical and symbolic geometry.
endcomment

// This file contains functions for analytic geometry in the plane.
// There are numerical Euler functions, and symbolic Maxima functions. 

// Load this file with "load geometry". See the example notebook for
// this package.

// * Geometry Plots

defaultd:=textheight()*1.5;

function plotLabel (text:string, p:real vector, ..
	v:real vector=[1,1], d:number=defaultd)
## Plot a label at position p
## Plot a label to a screen position 
## with offset vector v (normalized, default is lower right)
## and distance d (in screen units)
	h=norm(v);
	c=toscreen(p);
	oc=color(1);
	color(oc);
	textcolor(oc);
	ctext(text,c+[v[1],-v[2]]*d/h-[0,textheight()/2]);
	textcolor(1);
endfunction

function plotPoint (p:real vector, name:string="default", .. 
	v:real vector=[1,-1], ..
	d:number=defaultd, value:integer=0)
## Plot a point [with a name]
## Plot a point at position x
##
## See: plotLabel
	hold on; 
	mark(p[1],p[2]);
	if name=="default" then name=name(p); endif;
	if value then
		if name<>"" then name=name|"="; endif;
		name=name|printf("(%0.2g,",p[1])|printf("%0.2g)",p[2]);
	endif;
	if name<>"" then 
		plotLabel(name,p,v,d);
	endif;
	hold off;
endfunction

function plotSegment (A:real vector, B:real vector, ..
	name:string="default", d:number=defaultd)
## Plot a segment from A to B
## d is the distance of the label
	hold on;
	plot([A[1],B[1]],[A[2],B[2]]);
	if name=="default" then name=name(A)|name(B); endif;
	if name<>"" then 
		v=B-A;
		plotLabel(name,A+v/2,[-v[2],v[1]],d);
	endif;
	hold off;
endfunction

function plotLine (g:real vector, name:string="default", ..
	d:number=defaultd)
## Plot a line through A to B
## d is the distance of the label
	p=plot();
	A=[p[1],p[3]]; B=[p[2],p[4]];
	diag=distance(A,B);
	P=projectToLine((A+B)/2,g);
	v=getLineDirection(g);
	v=v/norm(v);
	plotSegment(P,P-diag*v,"");
	plotSegment(P,P+diag*v,"");
	if name=="default" then name=name(p); endif;
	if name<>"" then 
		plotLabel(name,P,turnLeft(v),d);
	endif;
endfunction

// * Numerical Geometry

function turn (v:real vector, phi:number)
## Turn a vector with angle phi
	return [v[1]*cos(phi)-v[2]*sin(phi),v[1]*sin(phi)+v[2]*cos(phi)]
endfunction

function turnLeft (v:real vector)
## Turn a vector orthogonally to the left
	return [-v[2],v[1]]
endfunction

function turnRight (v:real vector)
## Turn a vector orthogonally to the right
	return [v[2],-v[1]]
endfunction

function lineThrough (A:real vector, B:real vector)
## Compute a line through A and B
## returns [a,b,c] s.t. ax+by=c is an equation of the line.
	v=turnLeft(B-A);
	return v|scalp(v,A);
endfunction	

function lineWithDirection (A:real vector, v:real vector)
## lineWithDirection(A,v) compute a line through A into direction v.
## See: lineThrough
	return lineThrough(A,A+v);
endfunction

function getLineDirection (g:real vector)
## Returns a direction vector of the line.
	return turnRight(g[1:2]);
endfunction

function getNormal (g:real vector)
## Return a perpendicular vector on g
	return g[1:cols(g)-1];
endfunction

function getPointOnLine (g:real vector)
## Return a point on the line.
	v=getNormal(line);
	return g[3]/norm(v)^2*v;
endfunction

function perpendicular (A:real vector, g:real vector)
## Compute a perpendicular line through A to g
## See: lineThrough, parallel
	return lineWithDirection(A,g[1:2]);
endfunction

function parallel (A:real vector, g:real vector)
## Compute a parallel line through A to g
## See: perpendicular
	return lineWithDirection(A,lineDirection(g));
endfunction

function lineIntersection (g:real vector, h:real vector)
## Compute the intersection of two lines
	return ([g[1],g[2];h[1],h[2]]\[g[3];h[3]])';
endfunction

function projectToLine (A:real vector, g:real vector)
## Project a point to a line.
	return lineIntersection(perpendicular(A,g),g);
endfunction

function areaTriangle (A:real vector, B:real vector, C:real vector)
## Compute the area of the triangle A,B,C
	if cols(A)==2 then return abs(det((B-A)_(C-A)))/2;
	else return norm(crossproduct(B-A,C-A))/2;
	endif;
endfunction

function computeAngle (A:real vector, B:real vector, C:real vector)
## Compute the angle at B.
	return acos(scalp(A-B,C-B)/(norm(A-B)*norm(C-B)));
endfunction

function setPlotRange (a:number, b=none, c:number=0, d:number=0)
## Set the plot range
## setPlotRange(r) sets the range to a rectangle around (0,0).
	clg;
	if typeof(b)==8 then setplot(-a,a,-a,a);
	else setplot(a,b,c,d); 
	endif;
	return xplot();
endfunction

function circleWithCenter (A:real vector, r:number)
## Return a circle around A with radius r.
## See: circleTrough
	return A|r
endfunction

function getCircleCenter (c:real vector)
## Return the center of a circle.
	return c[1:2]
endfunction

function getCircleRadius (c:real vector)
## Return the radius of a circle.
	return c[3]
endfunction

function middlePerpendicular (A:real vector, B:real vector)
## Return the middle perpendicular of AB.
	return lineWithDirection((A+B)/2,turnLeft(B-A));
endfunction

function circleThrough (A,B,C)
## Compute a circle through three points.
##
## See: circle
	M=lineIntersection(middlePerpendicular(A,B),middlePerpendicular(B,C));
	return circleWithCenter(M,norm(M-A));
endfunction

function plotCircle (c,name="default",v=[1,-1],d=defaultd)
## Plot a circle.
## The label will attach to the circle in direction v (default is lower right).
##
## See: circle
	hold on;
	t=linspace(0,2pi,100*floor(max(1,c[3])));
	plot(c[1]+cos(t)*c[3],c[2]+sin(t)*c[3]);
	if name=="default" then name=name(c); endif;
	if name<>"" then
		h=norm(v);
		plotLabel(name,getCircleCenter(c)+v/h*c[3],v,d);
	endif;
endfunction

function angleBisector (A:real vector, B:real vector, C:real vector)
## Compute the middle perpendicular of the angle ABC
## See: line
	return middlePerpendicular(A,B+(C-B)/norm(C-B)*norm(A-B));
endfunction

function distance (A:real vector, B:real vector)
## Compute the distance of two points.
## See: norm
	return norm(B-A)
endfunction

function distanceSquared (A:real vector, B:real vector)
## Compute the distance of two points, squared.
## See: d
	return scalp(B-A,B-A)
endfunction

function quadrance (A:real vector, B:real vector)
## Compute the distance of two points, squared.
## See: d
	return scalp(B-A,B-A)
endfunction

function lineCircleIntersections (l:real vector, c:real vector)
## Intersection a line with a circle.
## Return both intersection points, and the number
## of intersections.
	M=getCircleCenter(c);
	P=projectToLine(M,l);
	d=distance(M,P);
	r=getCircleRadius(c);
	if d>r then return {P,P,0}
	elseif d~=r then
		return {P,P,1}
	else
		h=sqrt(r^2-d^2);
		v=getLineDirection(l);
		v=v/norm(v);
		return {P+v*h,P-v*h,2}
	end
endfunction

function circleCircleIntersections (c1:real vector, c2:real vector)
## Intersection of two circles.
## Return both intersection points, and the number
## of intersections.
	M1=getCircleCenter(c1); r1=getCircleRadius(c1);
	M2=getCircleCenter(c2); r2=getCircleRadius(c2);
	d=distance(M1,M2);
	v=normalize(M2-M1);
	if d>r1+r2 then
		return {M1+v*r1,M2-v*r2,0};
	elseif d~=r1+r2 then
		return {M1+v*r1,M2-v*r2,1};
	else
		a=(d^2+r1^2-r2^2)/(2*d);
		P=M1+a*v;
		h=sqrt(r1^2-a^2);
		w=turnLeft(v);
		return {P+h*w,P-h*w,2}
	endif;
endfunction

function normalize (v:real vector)
## Return the normalized vector v.
	return v/norm(v);
endfunction

function crossProduct (v:real vector, w:real vector)
## Compute the cross product of v and w.
	return crossproduct(v,w)
endfunction

function planeThrough (A:real vector, B:real vector, C:real vector)
## Compute a plane through three points.
	v=crossProduct(B-A,C-A);
	return v|scalp(v,A);
endfunction

// * Symbolic Geometry

maximafunction norm (x)
## The norm of a point x=[a,b]
endfunction

maximafunction normalize (x)
## The normalized vector x/norm(x)
endfunction

maximafunction scalp (a,b)
## The scalar product a*b
endfunction

maximafunction distance (A,B)
## The distance of the points A and B
endfunction

maximafunction distanceSquared (A,B)
## The distance of the points A and B squared
## See: distanceSquared, quad
endfunction

maximafunction turnRight (v)
## The vector v turned 90� to the right
endfunction

maximafunction turnLeft (v)
## The vector v turned 90� to the left
endfunction

maximafunction lineThrough (A,B)
## The line through A and B in the form [a,b,c]
## with equation ax+bx=c
endfunction

maximafunction getLineEquation (line,x,y)
## Equation of the line with variables x,y
endfunction

maximafunction getHesseForm (line,x,y,A)
## Hesseform of the line with the point A on the
## positive side.
endfunction

maximafunction getLineDirection (line)
## Direction vector of the line
endfunction

maximafunction perpendicular (A,line)
## Perpendicular line through A.
endfunction

maximafunction parallel (line)
## Parallel line through A.
endfunction

maximafunction lineIntersection (g,h)
## Intersection point of the two lines.
endfunction

maximafunction projectToLine (A,line)
## Orthogonal projection of A to the line.
endfunction

maximafunction areaTriangle (A,B,C)
## Area of the triangle.
endfunction

maximafunction computeAngle (A,B,C)
## Angle with corner at A.
endfunction

maximafunction circleWithCenter (A,r)
## Circle around A with radius r.
endfunction

maximafunction circleThrough (A,B,C)
## Circle through three points.
endfunction

maximafunction getCircleCenter (circle)
## Center of the circle.
endfunction

maximafunction getCircleRadius (circle)
## Radius of the circle.
endfunction

maximafunction angleBisector (A,B,C)
## Angle bisector for angle with corner at B.
endfunction

maximafunction quad (A,B)
## Distance AB squared
endfunction

maximafunction spread (a,b,c)
## Spread of the of the triangle a,b,c.
## The spread is sin(alpha)^2. alpha is opposite to a.
endfunction

maximafunction crosslaw (a,b,c,sa)
## Equation for three quads and one spread in a triangle.
## The spread is sa=sin(alpha)^2. alpha is opposite to a.
endfunction

maximafunction triplespread (sa,sb,sc)
## Equation for three spreads sa,sb,sc forming a triangle.
## The spreads are sin(phi)^2 for each angle.
endfunction

maximafunction doublespread (sa)
## Spread of the doubled angle.
## Spread of 2*phi, if sa=sin(phi)^2 is the spread of a.
endfunction

// The following definitions are sent to Maxima in direct mode when the
// file is loaded.

maximamode direct;
norm (x) := sqrt(x.x) $$
normalize (x) := x/norm(x) $$
scalp (a,b) := a.b $$
distance (A,B) := norm(B-A) $$
distanceSquared (A,B) := scalp(B-A,B-A) $$
turnLeft (v) := [-v[2],v[1]] $$
turnRight (v) := [v[2],-v[1]] $$
lineThrough (A,B) := block([v],v:turnLeft(B-A),[v[1],v[2],scalp(v,A)]) $$
lineWithDirection (A,v) := lineThrough(A,A+v) $$
getLineEquation (g,x,y) := g[1]*x+g[2]*y=g[3] $$
getHesseForm (g,x,y,p) := block ( [eq], eq : g[1]*x+g[2]*y-g[3], ..
 eq : eq/sqrt(g[1]^2+g[2]^2), ..
 if at(eq,[x=p[1],y=p[2]])<0 then -eq else eq ) $$
getLineDirection (g) := turnRight([g[1],g[2]]) $$
parallel (A,g) := lineWithDirection(A,getLineDirection(g)) $$
perpendicular (A,g) := lineWithDirection(A,turnRight(getLineDirection(g))) $$
lineIntersection (g,h) := at([x,y], ..
  solve([getLineEquation(g,x,y),getLineEquation(h,x,y)],[x,y])[1]) $$
projectToLine (A,g) := lineIntersection(perpendicular(A,g),g) $$
circleWithCenter (A,r) := [A[1],A[2],r] $$
middlePerpendicular (A,B) := lineWithDirection((A+B)/2,turnLeft(B-A)) $$
circleThrough (A,B,C) := block ( [M], ..
  M:lineIntersection(middlePerpendicular(A,B),middlePerpendicular(A,C)), ..
  circleWithCenter(M,distance(M,A))) $$
getCircleRadius (c) := c[3] $$
getCircleCenter (c) := [c[1],c[2]] $$
computeAngle (A,B,C) := acos(scalp(A-B,C-B)/(norm(A-B)*norm(C-B))) $$
angleBisector (A,B,C) := middlePerpendicular(A,B+(C-B)/norm(C-B)*norm(A-B)) $$
getCircleEquation (c,x,y) := (x-c[1])^2+(y-c[2])^2=c[3]^2 $$
lineCircleIntersections (g,c) := block ( [M,P,d,r,h,v], ..
 M:[c[1],c[2]], P:projectToLine(M,l), d:distance(M,P), r:c[3], ..
 h:sqrt(r^2-d^2), v:normalize([l[2],-l[1]]), [P+v*h,P-v*h]) $$
circleCircleIntersections (c1,c2) := block ( [M1,M2,r1,r2,d,v,a,P,h], ..
 M1:[c1[1],c1[2]], M2:[c2[1],c2[2]], r1:c1[3], r2:c2[3], ..
 d:distance(M1,M2), v:normalize(M2-M1), ..
 a:(d^2+r1^2-r2^2)/(2*d), P:M1+a*v, h:sqrt(r1^2-a^2), v:turnLeft(v), ..
 [P+h*v,P-h*v] ) $$
areaTriangle(A,B,C) := determinant(matrix(B-A,B-C))/2 $$
crossProduct(v,w) := [v[2]*w[3]-v[3]*w[2], ..
	-v[1]*w[3]+w[1]*v[3],v[1]*w[2]-v[2]*w[1]] $$
planeThrough(A,B,C) := block( [v], v:crossProduct(A-B,A-C), ..
	append(v,[scalp(v,A)]) ) $$
getPlaneEquation(F,x,y,z) := x*F[1]+y*F[2]+z*F[3]=F[4] $$
getPlaneHesseForm(F,x,y,z,P) := block ( [eq], eq : F[1]*x+F[2]*y+F[3]*z-F[4], ..
 eq : eq/sqrt(F[1]^2+F[2]^2+F[3]^2), ..
 if at(eq,[x=P[1],y=P[2],z=P[3]])<0 then -eq else eq ) $$
quad(A,B) := scalp(B-A,B-A) $$
crosslaw(a,b,c,sa) := (b+c-a)^2=4*b*c*(1-sa) $$
triplespread(a,b,c) := (a+b+c)^2=2*(a^2+b^2+c^2)+4*a*b*c $$
spread(a,b,c) := ratsimp(-(c^2+(-2*b-2*a)*c+b^2-2*a*b+a^2)/(4*b*c)) $$
doublespread(a) :=  4*(1-a)*a $$
maximamode off;

submenu Plot Geometry
addmenu setPlotRange(?r)
addmenu setPlotRange(?a,?b,?c,?d)
addmenu plotPoint(?P)
addmenu plotPoint(?P,"?name",value=?flag)
addmenu plotSegment(?A,?B)
addmenu plotSegment(?A,?B,"?name",value=?flag)
addmenu plotLine(?l)
addmenu plotLine(?l,"?name")
addmenu plotLabel("?text",?p)
addmenu plotCircle(?c)
addmenu plotCircle(?c,"?name")

submenu Euler Geometry
addmenu ?A:=[?x,?y]
addmenu ?A::=[?x,?y]
addmenu lineThrough(?A,?B)
addmenu lineWithDirection(?A,?v)
addmenu getLineDirection(?g)
addmenu getNormal(?g)
addmenu perpendicular(?A,?g)
addmenu parallel(?A,?g)
addmenu lineIntersection(?g,?h)
addmenu projectToLine(?A,?g)
addmenu areaTriangle(?A,?B,?C)
addmenu computeAngle(?A,?B,?C)
addmenu circleWithCenter(?A,?r)
addmenu circleThrough(?A,?B,?C)
addmenu getCircleCenter(?c)
addmenu getCircleRadius(?c)
addmenu angleBisector(?A,?B,?C)
addmenu distance(?A,?B)
addmenu quadrance(?A,?B)
addmenu {?A,?B}:=lineCircleIntersections(?g,?c)
addmenu {?A,?B}:=circleCircleIntersections(?ca,?cb)

submenu Maxima Geometry
addmenu :: ?A=[?x,?y]
addmenu :: lineThrough(?A,?B)
addmenu :: lineWithDirection(?A,?B)
addmenu :: getLineEquation(?g,?xvar,?yvar)
addmenu :: getHesseForm(?g,?xvar,?yvar.?Point)
addmenu :: getLineDirection(?g)
addmenu :: perpendicular(?A,?g)
addmenu :: parallel(?A,?g)
addmenu :: lineIntersection(?ga,?gb)
addmenu :: projectToLine(?A,?g)
addmenu :: areaTriangle(?A,?B,?C)
addmenu :: computeAngle(?A,?B,?C)
addmenu :: circleWithCenter(?A,?r)
addmenu :: circleThrough(?A,?B,?C)
addmenu :: getCircleCenter(?c)
addmenu :: getCircleRadius(?c)
addmenu :: angleBisector(?A,?B,?C)
addmenu :: distance(?A,?B)
addmenu :: quad(?A,?B)
addmenu :: crosslaw(?a,?b,?c,?sa)
addmenu :: triplespread(?sa,?sb,?sc)
addmenu :: spread(?a,?b,?c)
addmenu :: doublespread(?sa)
