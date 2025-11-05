// Povray with Euler

comment
Functions to generate Povray files for the open Raytracer. 
endcomment

// * Functions for Povray

// The Povray interface of Euler generates Povray files in the home
// directory of the user, and calls Povray to parse these files. The
// default file name is current.pov, and the default directory is
// eulerhome(), usually c:\Users\Username\Euler. Povray generates a
// PNG file, which can be loaded by Euler into a notebook. To clean up
// these files, use povclear().

// You need to load the Povray commands with "load povray". There is
// an introduction notebook demonstrating the package.

// See: ../Programs/03 - Povray in Euler | Demonstration

// For more information on the Povray povray project, see

// See: http://www.povray.org/ | Home page of Povray

// Moreover, you need to install Povray (32bit or 64bit), and put the
// sub-directory "bin" of Povray into the environment path, or set the
// variable "defaultpovray" with full path pointing to
// "pvengine.exe".

// The pov3d function is in the same spirit as plot3d. It can generate
// the graph of a function f(x,y), or a surface with coordinates X,Y,Z
// in matrices, including optional level lines. This function starts
// the raytracer automatically, and loads the scene into the Euler
// notebook.

// Besides pov3d(), there are many functions, which generate Povray
// objects. These functions return strings, containing the Povray code
// for the objects. To use these functions, start the Povray file with
// povstart(). Then use writeln(...) to write the objects to the
// scene file. Finally, end the file with povend(). By default, the
// raytracer will start, and the PNG will be inserted into the Euler
// notebook. 

// The object functions have a parameter called "look", which needs a
// string with Povray code for the texture and the finish of the
// object. The function povlook() can be used to produce this string.
// It has parameters for the color, the transparency, Phong
// Shading etc.

// Note that the Povray universe has another coordinate system. This
// interface translates all coordinates to the Povray system. So you
// can keep thinking in the Euler coordinate system with z pointing
// vertically upwards,a nd x,y,z axes in right hand sense.

// For vectors you can always use a instead of [a,a,a], and the
// vector "povx" is defined as [1,0,0] etc.

defaulthome=eulerhome();
defaultfile=eulerhome()+"current";
defaultmeshcolor=green;
defaultambient=0.2;
defaultpovray="pvengine.exe";
defaultpointsize=0.02;

nl$=char(10); // new line
apo$=char(34); // apostrophe

function %form (x)
## Format x to six digits accuracy
##
## This is used to reduce the file size of Povray files using
## the necessary accuracy. Returns a string.
	if x==0 then x=0; endif;
	return printf("%0.6g",x)
endfunction

function povrgb (c:integer, transparent=0)
## Color in rgb for Povray
##
## This function returns a string containing a color definition for
## Povray using "color rgb <r,g,b>". If transparent>0, "rgbf" will be
## used instead of "rgb".
##
## The function works for the 16 Euler colors or colors generated with
## rgb(r,g,b).
##
## See: povlook
	if transparent>epsilon then
		return "color rgbf <"+%form(getred(c))+"," ..
			+%form(getgreen(c))+","+%form(getblue(c)) ..
			+","+min(transparent,1)+">";
	else
		return "color rgb <" ..
		+%form(getred(c))+","+%form(getgreen(c)) ..
		+","+%form(getblue(c))+">";
	endif;
endfunction

function povxyz ()
## povxyz(x,y,z), povxyz(v), povxyz(r) compute coordinates for Povray.
##
## This function is used by many of the Povray functions to translate
## coordinates to a string for Povray. Note that Povray coordinates
## are different from Euler coordinates. The horizontal plane is x-z
## in Povray, and x-y in Euler. This function takes care of that
## transformation.
##
## With one real argument a the vector [a,a,a] is used. Also the
## functions povx, povy, povz can be used for the coordinate axes, and
## povc for the coordinate center.
##
## The functions returns a string.
##
## See: povx, povy, povz, povc
	if argn==1 then 
		if typeof(arg1)==2 and all(size(arg1)==[1,3]) then
			return "<"+%form(arg1[1])+","+%form(arg1[2])+","+%form(arg1[3])+">";
		elseif typeof(arg1)==0 then
			return "<"+%form(arg1)+","+%form(arg1)+","+%form(arg1)+">";
		endif
	elseif argn==3 then
		return "<"+%form(arg1)+","+%form(arg2)+","+%form(arg3)+">";
	endif;
	error("povxyz needs a number, a vector, or three numbers");
endfunction

function povx() := [1,0,0]; // x-axis
function povy() := [0,0,1]; // y-axis
function povz() := [0,1,0]; // z-axis
function povc() := [0,0,0]; // center

function writeCamera (aspect:positive number=1, anaglyph=0)
## Set the camera for Povray.
##
## This function need not be used directly. Use povstart() instead.
## The function writes the camera position to the output file for
## Povray.
##
## The current view is used to position the camera. To change the view
## use the view() function, or the zoom(), viewangle(), or
## viewheight(), viewdistance() service functions.
##
## The camera looks at the current center. To change this use the
## center() function.
##
## aspect : 
## The image is assumed to be square. If not, change the aspect.
##
## anaglyph : 
## If this is true the scene will be tilted with the default anaglyph
## separation times the anaglyph value. This is used in pov3d() and
## povanaglyph().
##
## See: povstart, pov3d, povanaglyph
	v=view();
	cn=center();
	a=-v[3];
	T=[cos(a),-sin(a),0;sin(a),cos(a),0;0,0,1];
	a=v[4];
	T=[1,0,0;0,cos(a),-sin(a);0,sin(a),cos(a)].T;
	if anaglyph then
		global defaultanaglyphseparation;
		da=-defaultanaglyphseparation/max(v[1],4)/2*anaglyph;
		T=[cos(da),-sin(da),0;sin(da),cos(da),0;0,0,1].T;
	endif
	S=inv(T)';
	cam=[0,-v[1],0];
	cam=cam.S;
	sky=[0,0,1];
	sky=sky.S;
	writeln("camera {"); 
	writeln("  location " + povxyz(cam));
	writeln(" angle "+deg(2*arctan(1.2/v[2])));
	writeln(" right "+povxyz(aspect,0,0));
	writeln(" up "+povxyz(0,0,1));
	writeln(" sky "+povxyz(sky));
	writeln(" look_at " + povxyz(cn));
	writeln("}");
endfunction

function writeAxis (t1:number, t2:number,
	delta:real vector=none, d:number=0.02,
	c:integer=gray, axis:integer=1, add:number=0.1)
## Axis for Povray
##
## Creates an axis in form of an arrow. This functions is used in
## pov3d to create all three axes.
##
## Since pov3d packs the scene into the unit cube, and scales the
## function, there is a translation vector delta.
##
## t1, t2 : left and right end of the axis
## delta : translate the axis using this vector (in Euler coordinates)
## d : thickness of the axis
## c : color of the axis
## axis : which axis 1=x, 2=y, 4=z
## add : additional length exceeding t1 an t2 on both sides
##
## See: pov3d
	t1=t1-add; t2=t2+add;
	writeln("union {");
    writeln("  cylinder { "+povxyz(t1,0,0)+","+povxyz(t2,0,0)+","+d+" }");
    writeln("  cone { ");
    writeln("    "+povxyz(t2+6*d,0,0)+",0");
    writeln("    "+povxyz(t2,0,0)+","+4*d);
    writeln("  }");
    if axis==2 then writeln("  rotate 90*z");
    elseif axis>2 then writeln("  rotate -90*y"); 
    endif;
    if delta!=none then
    	writeln("  translate "+povxyz(delta));
    endif;
	writeln("  texture { pigment { "+povrgb(c)+" } }");
	writeln("}");
endfunction

function writeAxes (xmin=-1,xmax=1,ymin=-1,ymax=1,zmin=-1,zmax=1,
	delta:real vector=none, d:number=0.02,
	c:integer=gray, add:number=0.1)
## Write an axis System.
##
## The function expects the start and the end of each axis. For the
## other parameters see writeAxis(). It writes to the Povray file
## directly.
##
## Example:
##
## >povstart(zoom=4,center=[0,0,1/2],height=45?);
## >writeAxes(-1,1,-1,1,0,1.4,d=0.01);
## >writeln(povsphere([0,0,1/2],1/2,povlook(green,0.5)));
## >writeln(povpoint([0,0,1],povlook(gray),size=0.02));
## >povend();
##
## See: writeAxis
	writeAxis(xmin,xmax,delta,d,c,1,add);
	writeAxis(ymin,ymax,delta,d,c,2,add);
	writeAxis(zmin,zmax,delta,d,c,4,add);	
endfunction

function writeGlobal (c:integer=white, gamma=1.4)
## Write the global header.
##
## This is called automatically with povstart().
##
## Sets the color of the ambient light and the assumed gamma.
	writeln("global_settings {");
    writeln("  ambient_light "+povrgb(c));
    writeln("  assumed_gamma "+gamma);
    writeln("}");
endfunction

function writeBackground (c:integer=white)
## Write the background for the scene.
##
## This is called automatically with povstart().
##
## Sets the background color.
	writeln("background {"+povrgb(c)+" }");
endfunction

function writeLight (v:real vector=[2,3,4], c:integer=white,
	distance=none, shadow=true, fade=0)
## Write the light source.
##
## This is called automatically with povstart().
##
## The light source is in direction of the vector v. The distance of
## the light source is the distance in view (which is the distance of
## the camera).
##
## shadow=0 : produces no shadow in the scene.
	if distance==none then distance=view()[1]; endif;
    v=v/norm(v)*distance;
	writeln("light_source {");
	writeln("  "+povxyz(v[1],v[2],v[3]));
	writeln("  "+povrgb(c));
	if fade then
		writeln("  fade_distance "+distance);
		writeln("  fade_power "+fade);
	endif;
	if !shadow then writeln("  shadowless"); endif;
	writeln("}");
endfunction

function povray (filename=none, width:index=450, height:index=450,
	aspect:positive number=1, exit:integer=true)
## Start the Povray process.
##
## By default, this is called in povend().
##
## Start the Povray program. The program name is in the global
## variable "defaultpovray". The default filename current.pov in the
## user home directory is used if file==none.
##
## exit=false allows to keep the Povray window open. Euler will wait,
## until you close it.
##
## filename : The filename to be used (without .pov)
	close();
	global defaultpovray;
	program=defaultpovray;
	if aspect!=1 then width=height*aspect; endif;
	global defaultfile;
	if filename==none then filename=defaultfile; endif;
	params="+H"+height+" +W"+width+" +FN24 +A -D /RENDER " ..
		+apo$+filename+".pov"+apo$;
	if exit then params="/NR /EXIT "+params; endif;
	global defaulthome;
	pngfile=filename+".png";
	fileremove(pngfile);
	exec(program,params,defaulthome);
	png=dir(pngfile);
	if cols(png)==0 then
		error("Povray error!");
	endif;
endfunction

function povopen (filename)
## Open the povray file for writing.
##
## This is called in povstart().
##
## Do not forget to close it before you use the file. The povend()
## function closes the file automatically.
##
## See: open
	open(filename+".pov","w");
endfunction

function povscalexyz (x,y,z)
## Scale x,y,z to fit into a unit cube.
##
## Return {x,y,z,f,cx,cy,cz}
## where f is the factor used, and cx,cy,cz is the new center of the
## coordinate system.
##
## See: pov3d
	xmin=totalmin(x); xmax=totalmax(x);
	ymin=totalmin(y); ymax=totalmax(y);
	zmin=totalmin(z); zmax=totalmax(z);
	h=max(xmax-xmin,ymax-ymin,zmax-zmin)*0.5;
	cx=(xmin+xmax)/2;
	cy=(ymin+ymax)/2;
	cz=(zmin+zmax)/2;
	return {(x-cx)/h,(y-cy)/h,(z-cz)/h,1/h,cx/h,cy/h,cz/h};
endfunction

function povscalef (x,y)
## Scale x,y to fit into a unit square.
##
## See: povscalexyz
	xmin=totalmin(x); xmax=totalmax(x);
	ymin=totalmin(y); ymax=totalmax(y);
	h=max(xmax-xmin,ymax-ymin)*0.5;
	cx=(xmin+xmax)/2;
	cy=(ymin+ymax)/2;
	return {(x-cx)/h,(y-cy)/h,1/h,cx/h,cy/h};
endfunction

function povscalez (z)
## Scale z to fit into [-1,1]
##
## See: povscalez
	zmin=totalmin(z); zmax=totalmax(z);
	h=(zmax-zmin)*0.5;
	cz=(zmin+zmax)/2;
	return {(z-cz)/h,1/h,cz/h};
endfunction

function povload (file=none, anaglyph=false)
## Load the Povray image into the Notebook.
##
## By default, this is called in povend().
##
## file : If none, use defaultfile
	global defaultfile;
	if file==none then file=defaultfile; endif;
	if anaglyph then loadanaglyph(file,file+1);
	else loadimg(file);
	endif;
endfunction

function povstart (file=none,
	zoom=none, distance=none, angle=none, height=none,
	light:real vector=[2,2,4], lightcolor=white, background=white,
	aspect=1, shadow=true, center=[0,0,0],
	fade=0, anaglyph=0)
## Start a Povray scene.
##
## This function opens the file, and sets the scene, including light,
## camera and background.
##
## zoom, distance, angle, height : Change the view locally in this
## function. The angle is measured from the negative y-axis, as in
## plot3d() and view(). The height is from the x-y-plane. Both angles
## are in radians. Use angle=30? to convert.
##
## light, lightcolor : A vector to the light source and the color of
##                     the light.
## background : The background color
## aspect : The aspect ratio of your window. For square images use 1.
##          Note that you need the same aspect in povend(). 
## shadow : A flag for shadows.
## center : The point, where the camera looks at. The camera itself is
##   determined by the view.
## fade : If nonzero, fading with this exponential rate is used. The
##        fading distance is the distance from the light to the center.
##
## See: povend, view
	global defaultfile,defaultinc;
	if file==none then file=defaultfile; endif;
	defaultinc=1;
	vold=_view();
	cold=_center(center);
	if zoom!=none then zoom(zoom); endif;
	if distance!=none then viewdistance(distance); endif;
	if angle!=none then viewangle(angle); endif;
	if height!=none then viewheight(height); endif;
	povopen(file);
	writeln(''#include "functions.inc"'');
	writeln(''#include "colors.inc"'');
	writeln(''#include "stones.inc"'');
	writeGlobal(lightcolor);
	writeBackground(background);
	writeLight(light,lightcolor,distance,shadow,fade);
	writeCamera(aspect,anaglyph);
	_view(vold);
	_center(cold);
endfunction

function povend (file=none, povray=true, w=450, h=450, 
	aspect=1, exit=true, imgload=true)
## End a Povray scene
##
## Closes the scene file. By default, the Povray process will start,
## and the image will be loaded into the notebook.
##
## w,h,aspect : 
## In combination determine the aspect ratio of your scene. Note that
## you need the same aspect as in povstart().
##
## exit : If false, the Povray window will not close automatically.
## 
## imgload : 
## If false, the image will not be loaded. You can then find the image
## in the Euler subdirectory of your home directory.
##
## >povstart(zoom=3.6,aspect=4/3);
## >writeln(povsphere([0,-1,0],1/2,povlook(gray)));
## >writeln(povsphere([0,1,0],1/2,povlook(gray)));
## >writeln(povsphere([0,0,0],1,povlook(green)));
## >povend(h=300,aspect=4/3);
##
## Or, if you want to insert the image later:
##
## >povend(h=400,aspect=4/3,<imgload); // make image
## >loadimg(defaultfile); // manually insert the image
##
## See: povstart
	close();
	if povray then
		global defaultfile;
		if file==none then file=defaultfile; endif;
		povray(file,w,h,aspect,exit); 
		if imgload then povload(file); endif;
	endif;	
	global defaultinc;
	defaultinc=1;
endfunction

function povanaglyph (scene$:string,
	file=none,
	zoom=none, distance=none, angle=none, height=none,
	light:real vector=[2,2,4], lightcolor=white, background=white,
	aspect=1, shadow=true, center=[0,0,0],
	fade=0,	w=450, h=450, 
	aspect=1, exit=true, imgload=true)
## Create a red/cyan anaglyph
##
## Create an anaglyph with a scene, which is generated in the function
## scene. The Povray command runs twice, and generates two PNG files
## in the user directory. The parameters combine povstart and povend
##
## >function f() ...
## $  writeAxes(-1,1,-1,1,-1,1);
## $  writeln(povcylinder([-1,0,0],[1,0,0],1,povlook(lightblue,0.4)));
## $endfunction
## >povanaglyph("f");
##
## See: povstart, povend
	global defaultfile;
	if file==none then file=defaultfile; endif;
	currentfile=file;
	loop 1 to 2
	    an=-3+#*2;
		povstart(currentfile,zoom,distance,angle,height, ..
			light,lightcolor,background, ..
			aspect,shadow,center,fade,an);
		scene$(args());
		close();
		povray(currentfile,w,h,aspect,exit); 
		currentfile=currentfile+1;
	end;
	if imgload then povload(file,>anaglyph); endif;
	global defaultinc;
	defaultinc=1;
endfunction

function %texture (color=green, transparent=0)
## Texture string
##
## See: finish
	return " texture { pigment { "+povrgb(color,transparent)+" } "+" } ";
endfunction

function %povparam (s:string, v)
## Produces a parameter form, if v is not none.
	if v==none then return "";
	elseif typeof(v)==8 then return v+nl$;
	elseif typeof(v)==2 then return " "+s+" "+povxyz(v)+nl$;
	else return " "+s+" "+v+nl$;
	endif;
endfunction

function %finish (ambient:real=defaultambient,
	phong:real=none, phongsize:real=100,
	reflection=none, diffuse=none)
## Finish string
##
## See: finish
	sphong="";
	if phong!=none then 
		sphong=" phong "+phong+" phong_size "+phongsize;
	endif;
	return " finish { ambient "+ambient+sphong .. 
	+%povparam("reflection",reflection) ..
	+%povparam("diffuse",diffuse) ..
	+" } ";
endfunction

function povlook (color=green, transparent=0, ambient=none,
	phong:real=none, phongsize:real=100,
	reflection=none, diffuse=none)
## Texture and finish of an object.
##
## Returns a string containing the Povray code for this texture and
## finish.
##
## color : the color of the object.
## transparent : If nonzero, the color will be transparent.
## ambient : The amount of ambient light to be used.
## phong, phongsize : If nonzero, a shining finish will be used.
## reflection : Makes the object reflect other objects.
##
## >povlook(green)
## >povlook(gray,0.5) // half transparent
##
## See: rgb, povl
	global defaultambient;
	if ambient==none then ambient=defaultambient; endif;
	return %texture(color,transparent)+nl$ ..
		+%finish(ambient,phong,phongsize,reflection,diffuse)+nl$;
endfunction

function povl () := povlook(args(1));

defaultlook=povlook(gray);

function %writeTrianglesSimple (x:real, y:real, z:real)
## Triangles for the mesh2 object
##
## Writes the vertex vectors and the face indices so that the
## triangles represent the surface in the same way as all other Euler
## plots. 
##
## Two triangles are used for each matrix element.
##
## See: writeTriangleMesh
	n=max(rows(x),rows(y),rows(z));
	m=max(cols(x),cols(y),cols(z));
	writeln("vertex_vectors { "+(n*m)+",");
	for i=1 to n
		for j=1 to m
			write(povxyz(x{i,j},y{i,j},z{i,j}));
			writeln(",");
		end
	end	
	writeln("}");
	writeln("face_indices { "+(2*(n-1)*(m-1))+",");
	for i=1 to n-1
		for j=1 to m-1
			k1=(i-1)*m+j-1;
			k2=k1+m;
			writeln(povxyz(k1,k1+1,k2)+",");
			writeln(povxyz(k2+1,k2,k1+1)+",");
		end
	end	
	writeln("}");
endfunction

function %writeTrianglesAndNormals (x:real, y:real, z:real,
	xv:real, yv:real, zv:real)
## Normals for the mesh2 object
##
## Writes the vertex vectors and the face indices so that the
## triangles represent the surface in the same way as all other Euler
## plots. The functions writes normal vectors too. It assumes one
## normal vector for each vertex.
##
## Two triangles are used for each matrix element.
##
## See: writeTriangleMeshSimple
	n=max(rows(x),rows(y),rows(z));
	m=max(cols(x),cols(y),cols(z));
	writeln("vertex_vectors { "+(n*m)+",");
	for i=1 to n
		for j=1 to m
			write(povxyz(x{i,j},y{i,j},z{i,j}));
			writeln(",");
		end
	end	
	writeln("}");	
	writeln("normal_vectors { "+(n*m)+",");
	for i=1 to n
		for j=1 to m
			write(povxyz(xv{i,j},yv{i,j},zv{i,j}));
			writeln(",");
		end
	end	
	writeln("}");
	writeln("face_indices { "+(2*(n-1)*(m-1))+",");
	for i=1 to n-1
		for j=1 to m-1
			k1=(i-1)*m+j-1;
			k2=k1+m;
			writeln(povxyz(k1,k1+1,k2)+",");
			writeln(povxyz(k2+1,k2,k1+1)+",");
		end
	end	
	writeln("}");
endfunction

function %writeTriangles (x:real, y:real, z:real)
## Triangles for the mesh2 object
##
## Works like WriteTrianglesSimple(). But four triangles are used for
## each matrix element.
##
## See: writeTrianglesSimple
	n=max(rows(x),rows(y),rows(z));
	m=max(cols(x),cols(y),cols(z));
	writeln("vertex_vectors { "+(n*m+(n-1)*(m-1))+",");
	for i=1 to n
		for j=1 to m
			write(povxyz(x{i,j},y{i,j},z{i,j}));
			writeln(",");
		end
	end	
	for i=1 to n-1
		for j=1 to m-1
			hx=(x{i,j}+x{i+1,j}+x{i,j+1}+x{i+1,j+1})/4;
			hy=(y{i,j}+y{i+1,j}+y{i,j+1}+y{i+1,j+1})/4;
			hz=(z{i,j}+z{i+1,j}+z{i,j+1}+z{i+1,j+1})/4;
			write(povxyz(hx,hy,hz));
			writeln(",");
		end
	end	
	writeln("}");
	writeln("face_indices { "+(4*(n-1)*(m-1))+",");
	for i=1 to n-1
		for j=1 to m-1
			k=n*m+(i-1)*(m-1)+j-1;
			k1=(i-1)*m+j-1;
			k2=k1+m;
			writeln(povxyz(k1,k,k2)+",");
			writeln(povxyz(k2,k,k2+1)+",");
			writeln(povxyz(k2+1,k,k1+1)+",");
			writeln(povxyz(k1+1,k1,k)+",");			
		end
	end	
	writeln("}");
endfunction

function writeTriangleMesh (x:real, y:real, z:real, 
	look:string=defaultlook,
	xv:real=none, yv:real=none, zv:real=none, 
	simple:integer=false)
## Mesh object with triangles
##
## Write a mesh object for the surface with coordinates x,y,z.
## The surface is formed of 4 triangles for each rectangular segment,
## connecting the corner to the average midpoint of the 4 points.
## The evaluation of x,y,z follows the matrix language of Euler.
##
## If normal vectors are present, the functions used two triangles in
## each matrix element, but adds the vectors to the scene.
##
## Note that this function does writeln() to the output file. If you
## need an object use povtriangles().
##
## See: povtriangles
	writeln("mesh2 {");
	if xv==none then
		if simple then %writeTrianglesSimple(x,y,z);
		else %writeTriangles(x,y,z);
		endif;
	else
		%writeTrianglesAndNormals(x,y,z,xv,yv,zv);
	endif
	writeln(look);
	writeln("}");
endfunction

defaultinc=1;

function povtriangles (x:real, y:real, z:real, look:string=defaultlook,
	xv:real=none, yv:real=none, zv:real=none, simple:integer=false,
	include=false)
## A mesh object
##
## Mesh objects are formed of hundreds of triangles. They cannot be
## stored in Euler strings. So this function write either to an
## include file and returns a #include statement. Or it writes to the
## scene file, and #declares an object there. Then it returns an
## object. Usually, it is not necessary to use an include file.
##
## For a description of the triangle mesh and the normal vectors, see
## writeTriangleMesh().
##
## See: writeTriangleMesh
	if include then
		global defaultinc,defaulthome;
		filename=defaulthome+"current"+defaultinc+".inc";
		defaultinc=defaultinc+1;
		open(filename,"w");
		writeTriangleMesh(x,y,z,look,xv,yv,zv,simple);
		close();
		return nl$+"#include "+apo$+filename+apo$+nl$
	else
		global defaultinc;
		name="mesh"+defaultinc;
		writeln("#declare "+name+" =");
		writeTriangleMesh(x,y,z,look,xv,yv,zv,simple);
		return "object { "+name+" }";
	endif
endfunction

function %writegrid (x:real,y:real,z:real,
	look:string=defaultlook,
	d:number=none,dballs:number=none,
	skip:positive integer vector=none)
## Write a grid mesh to the current output.
	global defaultpointsize;
	if d==none then d=defaultpointsize; endif;
	if dballs==none then dballs=d; endif;
	s=size(x,y,z);
	n=s[1]; m=s[2];
	writeln("union {"+nl$);
	for i=1 to n
		if skip!=none and mod(i-1,skip{1})!=0 then
			continue;
		endif;
		for j=1 to m
			writeln("sphere { "+ ..
				povxyz(x{i,j},y{i,j},z{i,j})+", "+ ..
				dballs+" }"+nl$);
		end;
		for j=2 to m
			writeln("cylinder { "+ ..
				povxyz(x{i,j-1},y{i,j-1},z{i,j-1})+", "+ ..
				povxyz(x{i,j},y{i,j},z{i,j})+", "+ ..
				d+" }"+nl$);
		end;
	end;
	for j=1 to m
		if skip!=none and mod(j-1,skip{2})!=0 then
			continue;
		endif;
		for i=2 to n
			if skip!=none and mod(i-1,skip{1})!=0 then
				writeln("sphere { "+ ..
					povxyz(x{i,j},y{i,j},z{i,j})+", "+ ..
					dballs+" }"+nl$);
			endif;		
			writeln("cylinder { "+ ..
				povxyz(x{i-1,j},y{i-1,j},z{i-1,j})+", "+ ..
				povxyz(x{i,j},y{i,j},z{i,j})+", "+ ..
				d+" }"+nl$);
		end;
	end;
	writeln(look+" }"+nl$);
endfunction

function povgrid (x:real, y:real, z:real, 
	look:string=defaultlook, d=none, dballs=none,
	include=false, skip:positive integer vector=none)
## A mesh object
##
## Grid objects, like mesh objects, are formed of hundreds of
## triangles. They cannot be stored in Euler strings. So this function
## write either to an include file and returns a #include statement.
## Or it writes to the scene file, and #declares an object there. Then
## it returns an object. Usually, it is not necessary to use an
## include file.
##
## d, dballs : Diameter of grid and balls
## skip : Scalar or 1x2 vector. Skip rows and columns of the matrix.
##        With this parameter, it is possible to generate a fine grid,
##        but show only a few grid lines.
##
##
## Example:
##
## >povstart(center=[0,0,1],zoom=3.5);
## >x=-1:0.05:1; y=x';
## >writeln(povgrid(x,y,x^2+y^2,skip=5,d=0.01,dballs=0.02));
## >povend();
##
## See: povtriangles
	if include then
		global defaultinc,defaulthome;
		filename=defaulthome+"current"+defaultinc+".inc";
		defaultinc=defaultinc+1;
		open(filename,"w");
		%writegrid(x,y,z,look,d,dballs,skip);
		close();
		return nl$+"#include "+apo$+filename+apo$+nl$
	else
		global defaultinc;
		name="grid"+defaultinc;
		writeln("#declare "+name+" =");
		%writegrid(x,y,z,look,d,dballs,skip);
		return "object { "+name+" }";
	endif
endfunction

function povbox (P1: real vector, P2: real vector, 
	look:string=defaultlook)
## A box with these two corners.
##
## The box parallel to the axes. If you need rotated box, use an
## object containing the box, and the rotation parameter.
##
## >povstart();
## >writeln(povobject(povbox([-1,-1,-1],[1,1,1]),rotate=xrotate(-20?)));
## >povend();
##
	return "box { "+povxyz(P1)+", "+povxyz(P2)+nl$+look+" }";
endfunction

function povcylinder (P1: real vector, P2: real vector,
	r: number, look:string=defaultlook, open:integer=false)
## A cylinder with these two ends and radius
	if open then sopen=" open"+nl$; else sopen=""; endif;
	return "cylinder { "+povxyz(P1)+", "+povxyz(P2)+", "+r+nl$+ ..
		sopen+look+" }";
endfunction

function povdisc (P: real vector, v: real vector, r:number,
	d: number=none, look:string=defaultlook)
## A disc with center, normal and radius
##
## If d==none the disc uses defaultpointsize for its thickness.
	global defaultpointsize;
	if d==none then d=defaultpointsize; endif;
	v=v/norm(v);
	return povcylinder(P-v*d/2,P+v*d/2,r,look);
endfunction

function povintersection (v:string vector, look:string=defaultlook)
## Intersection of the objects in v
##
## v : a vector of string, containing the objects.
	s="intersection { ";
	loop 1 to length(v); s=s+v[#]+nl$; end;
	return s+look+" }";
endfunction

function povunion (v:string vector, look:string=defaultlook)
## Union of the objects in v
##
## v : a vector of strings, containing the objects.
	s="union { ";
	loop 1 to length(v); s=s+v[#]+nl$; end;
	return s+look+" }";
endfunction

function povmerge (v:string vector, look:string=defaultlook)
## Merge of the objects in v
##
## Merging removes the inner boundaries
##
## v : a vector of strings, containing the objects.
##
## See: povunion, povintersection
	s="merge { ";
	loop 1 to length(v); s=s+v[#]+nl$; end;
	return s+look+" }";
endfunction

function povdifference (v1:string, v2: string,
	look:string=defaultlook)
## v1 minus v2
	return "difference { "+nl$+v1+nl$+v2+nl$+look+" }";
endfunction

function povcone (P1: real vector, r1: number,
	P2: real vector, r2: number, look:string=defaultlook)
## A cone with these two ends and radii.
	return "cone { "+povxyz(P1)+", "+r1+" "+povxyz(P2)+", "+r2 ..
  		+look+" }";
endfunction

function povsphere (P: real vector, r: number,
	look:string=defaultlook)
## A sphere with midpoint and radius.
	return "sphere { "+povxyz(P)+", "+r+look+nl$+"}";
endfunction

function povtext (s:string, P:real vector, look:string=defaultlook,
	d:number=none, size:number=none, font="timrom.ttf",
	rotate:number=0°)
## Text object at position P
##
## d : thickness of the text
## size : height of the text
## font : text font name
## rotate : rotation around vertical axis
	global defaultpointsize;
	if d==none then d=defaultpointsize*2; endif;
	if size==none then size=defaultpointsize*4; endif;
	t="text { ttf "+apo$+font+apo$+" "+ ..
		apo$+s+apo$+" "+ ..
		d+" ,0 "+look+" }";
	return povobject(t,translate=P,scale=size, ..
		rotate=xrotate(90°)|yrotate(rotate));
endfunction

function povplane (P: real vector, d: number,
	look:string=defaultlook, clippedby:string=none)
## A plane with a'.v=d, a=[x,y,z].
##
## Note that the plane is really a half space, unless it is clipped.
## For clipping use any full object.
	cl="";
	d=d/norm(P);
	if clippedby!=none then cl="clipped_by { "+clippedby+" } "; endif;
	return "plane { "+povxyz(P)+", "+d+nl$+cl+look+nl$+"}";
endfunction

function povpoint (P: real vector,
	look:string=defaultlook, size:number=none)
## A point
##
## The size will be the radius of the sphere. By default it is the
## defaultpointsize.
##
## >povstart(zoom=2,center=[0,0,-1]);
## >X=randnormal(3,100,0,1/2);
## >loop 1 to cols(X); writeln(povpoint([X[1,#],X[2,#],X[3,#]],povlook(red))); end;
## >writeln(povdisc([0,0,-3],[0,0,1],2));
## >writeAxis(-3,1,axis=3);
## >povend();
	global defaultpointsize;
	if size==none then size=defaultpointsize; endif;
	return povsphere(P, size, look);
endfunction

function povsegment (P1: real vector, P2: real vector,
	look:string=defaultlook, size=none)
## A line segment between the two points
	global defaultpointsize;
	if size==none then size=defaultpointsize/2; endif;
	return povcylinder(P1,P2,size,look);
endfunction

function povarrow (x: real vector, v: real vector,
	look:string=defaultlook, d:number=none)
## An arrow
##
## d : The thickness of the arrow. By default, this is the
##     defaultpointsize
	global defaultpointsize;
	if d==none then d=defaultpointsize; endif;
	o1=povcylinder(x,x+v,d);
	o2=povcone(x+v,3*d,x+v+v/norm(v)*5*d,0);
	return povunion([o1,o2],look);
endfunction

function povdefine (a:string, b: string)
## Declares an object in Povray
##
## Sometimes, you wish to define an object in Povray, not in an Euler
## string. You can later use the object by name. You can also define
## other things, like finishes in Povray.
	writeln("#declare "+a+" = "+nl$+b);
endfunction

function xrotate (a) := " rotate "+deg(a)+" *x "
function yrotate (a) := " rotate "+deg(a)+" *z "
function zrotate (a) := " rotate "+deg(a)+" *y "

function povobject (object:string, look:string="",
	translate=none, rotate=none, scale=none)
## Povray object containing another object
##
## scale : A vector or a scalar to stretch the object. If a vector is
##   used its coordinates are the factors in each of the three
##   directions.
## rotate : The rotation parameter. This can be a vector, which
##   contains the rotations around the x, y, and z axis in this order.
##   The values must be in radians. Usually, you want to use the
##   functions xrotate(a), yrotate(a), zrotate(a), which return
##   strings. These strings can be concatenated for the rotation
##   parameter.
## translate : A vector for the translation.
##
## Example:
##
## >povstart();
## >writeAxes(-1,1,-1,1,-1,1);
## >sph=povsphere([0,0,0],1);
## >rot=xrotate(10?)+yrotate(45?);
## >tr=[0,0,-1/2];
## >writeln(povobject(sph,scale=[2,1,0.2],rotate=rot,translate=tr));
## >povend();
##
    if typeof(rotate)==2 then rotate=deg(rotate); endif;
	return "object { "+object+nl$ ..
		+%povparam("scale",scale) ..
		+%povparam("rotate",rotate) ..
		+%povparam("translate",translate) ..
		+look+" }";
endfunction

function povsurface (f:string, look:string=defaultlook,
	container:string=none, maxgradient=5, open=true)
## Implicit surface
##
## The surface is defined by f(x,y,z)=0.
##
## f : Contains the formula f. Povray uses a different syntax for
##   mathematical functions. E.g., pow(x,2) must be used instead
##   of x^2. Please refer to a Povray manual. The file functions.inc is
##   #included in povstart().
##
## maxgradient : If 5 is too low and the surface looks ugly, use a
##   higher value at the cost of increased time.
##
## open : If false, the cube will be filled between the surface and
##   the cube. Effectively, the set where the function <=0 will be
##   visible.
##
## container : A box containing the surface. Default is
##   povbox(-1,1,"").
##
## Example:
##
## >povstart();
## >writeln(povsurface("pow(x,4)+pow(y,6)+pow(z,2)-1"));
## >writeAxes();
## >povend();
	if container==none then container=povbox(-1,1,""); endif;
	if open then sopen="open"+nl$; else sopen=""; endif;
	return "object {"+nl$ ..
		+"isosurface {"+nl$+"function { "+f+" }"+nl$ ..
		+"max_gradient "+maxgradient+nl$ ..
		+sopen ..
		+"contained_by { "+container+" }"+nl$ ..
		+look+"}" ..
		+"}";
endfunction

function povclear ()
## Remove all temporary Povray files.
	global defaulthome;
	d=dir(defaulthome+"current*");
	loop 1 to length(d);
		fileremove(defaulthome+d[#]);
	end;
	d=dir(defaulthome+"mesh*");
	loop 1 to length(d);
		fileremove(defaulthome+d[#]);
	end;
endfunction

function pov3d (x, y=none, z=none,
	xmin:number=-1, xmax:number=1, ymin:number=-1, ymax:number=1,
	r:number=none, n:index=40,
	file=none, axis:integer=7, 
	look:string=none, axiscolor:integer=gray,
	zoom=none, distance=none, angle=none, height=none,
	light:real vector=[2,3,4], lightcolor=white, background=white,
	povray=true, w=450, h=450,
	shadow=true, center=[0,0,0], fade=0,
	xv:real=none, yv:real=none, zv:real=none,
	level:real vector=none, dlevel=0.02, levellook:string=none,
	add=none, loadimg=true, anaglyph=false, fscale=true, scale=true)
## Multi-purpose function to plot 3D in Povray
##
## pov3d("f(x,y)") : plots a function
## pov3d(x,y,z) : plots a surface given by x,y,z
##
## xmin,xmax,ymin,ymax : range x and y
## n : resolution for x and y
## r : alternatively radius for quadratic or polar plots
## cx, cy, cz : center for r
## file : used, if not the defaultfile="current" (without .pov)
## color : color of the plot
## axis : 1,2,4 or sum of these for x, y, z axis
## axiscolor : color of axis
## zoom, distance, angle, height : see view()
## light : direction vector to the light source
## lightcolor : color of the light
## background : color of the background
## povray : if true, the Povray program is called
## w, h : size of png
## loadimg : if false, the image is not loaded into the notebook.
## add : one Povray object to add
##
## >pov3d("x^3+y^2",angle=-80?, ...
## >  level=-2:0.2:2,dlevel=0.01,levellook=povlook(red), ...
## >  add=povtext("z=x^3+y^2",[0,-0.2,1.2],size=0.06,rotate=-90°), ...
## >  zoom=4);
##
## See: view
	global defaultfile;
	if file==none then file=defaultfile; endif;
	if look==none then look=povlook(green); endif;
	if levellook==none then levellook=povlook(gray); endif;
	currentfile=file;
	if typeof(x)==8 then
		f=x;
		if r!=none then xmin=-r; xmax=r; ymin=-r; ymax=r; endif;
		x=linspace(xmin,xmax,n); 
		y=linspace(ymin,ymax,n)';
		z=f(x,y;args());
		cx=0; cy=0; fxy=1;
		if scale then {x,y,fxy,cx,cy}=povscalef(x,y); endif;
		cz=0; fz=1;
		if fscale then {z,fz,cz}=povscalez(z); endif;
	else
	    cx=0; cy=0; cz=0; fz=1;
		if scale then {x,y,z,fz,cx,cy,cz}=povscalexyz(x,y,z); endif;
	endif;	
	loop 1 to 2
		an=0;
		if anaglyph and #==1 then an=-1;
		else an=1;
		endif;
		povstart(currentfile, ..
			zoom,distance,angle,height, ..
			light, lightcolor, background, w/h, shadow, center, ..
			0,an);
		if typeof(x)==8 then
			mesh=povtriangles(x,y,z,"",xv,yv,zv);
		else
			mesh=povtriangles(x,y,z,"",xv,yv,zv);
		endif;
		if level!=none then
			level=(level*fz-cz);
			r=max(totalmax(x),totalmax(y));
			ncl=[povdisc([0,0,level[1]],[0,0,1],r,dlevel)];
			loop 2 to cols(level);
				ncl=ncl|povdisc([0,0,level[#]],[0,0,1],2,dlevel);
			end;
			writeln(povdifference(mesh,povunion(ncl),look));
			writeln(povintersection([povunion(ncl),mesh],levellook));
		else
			writeln(povobject(mesh,look));
		endif;
		if bitand(axis,1) then
			writeAxis(totalmin(x)+cx,totalmax(x)+cx,-[cx,cy,cz],c=axiscolor);
		endif;
		if bitand(axis,2) then
			writeAxis(totalmin(y)+cy,totalmax(y)+cy,-[cx,cy,cz],axis=2,c=axiscolor);
		endif;
		if bitand(axis,4) then
			writeAxis(totalmin(z)+cz,totalmax(z)+cz,-[cx,cy,cz],axis=3,c=axiscolor);
		endif;
		if add!=none then writeln(add); endif;
		close();
		if povray then povray(currentfile,w,h,w/h); endif;
		if !anaglyph then break; endif;
		currentfile=currentfile+1;
	end;
	if povray and loadimg then povload(file,anaglyph); endif;
endfunction

submenu Povray
addmenu pov3d(?fxy,xmin=?xmin,xmax=?xmax,ymin=?ymin,ymax=?ymax)
addmenu pov3d(?fxy,r=?r)
addmenu pov3d(?X,?Y,?Z)
addmenu pov3d(?X,?Y,?Z,level=?n)
addmenu startpov();
addmenu startpov(zoom=?zoom,center=?center);
addmenu endpov();
addmenu endpov(<exit);

submenu Povray Objects
addmenu povlook(?color)
addmenu povlook(?color,?transparancy)
addmenu povlook(?color,?transparancy,>phong)
addmenu povsphere(?point,?radius)
addmenu povcylinder(?point,?point,?radius)
addmenu povcone(?point,?radius,?point,?radius)
addmenu povbox(?point,?point)
addmenu povplane(?vector,?value)
addmenu povdisc(?point,?vector,?radius)
addmenu povpoint(?point)
addmenu povsegment(?point,?point)
addmenu povintersection(?objectlist)
addmenu povunion(?objectlist)
addmenu povdifference(?object,?object)
addmenu povobject(?object,povlook(?color))
addmenu povobject(?object,povlook(?color,?transparency))
addmenu xrotate(?value)
addmenu yrotate(?value)
addmenu zrotate(?value)
addmenu povobject(?object,povlook(?color),scale=?vector,rotate=?string,translate=?vector)
addmenu povdefine(?string,?string)


