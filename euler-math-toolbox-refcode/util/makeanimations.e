// Animated GIF or MPEG

// Load this file with "load makeanimations".

comment
Functions to make an animated GIF or an MPEG movie.
endcomment

function makegif (f$:string, x:real vector, 
	filename:string="animation", show=true,
	w:index=none, h:index=none, antialias=true)
## Makes an animated GIF
##
## The procedure calls f$(x[i]) for the values in the row vector x. f$
## should be a plot function depending on this variable. In the
## current directory, images with names im000.png, im001.png etc will
## be generated. These images are deleted later.
##
## The function calls the program convert to produce an animated GIF
## from the images. This function is part of the suite by ImageMagick.
## After the work is done, the PNG files are deleted and the default
## viewer for GIF files is called.
##
## http://www.imagemagick.org/
##
## filename : Filename for images and animation.
## show : Show the GIF in the default viewer after it has been
##    generated.
##
## >function oneplot (a) ...
## $  plot2d("sin(x+a)";a,a=0,b=2pi,c=-1,d=1,color=red,thickness=3);
## $  endfunction
## >load makeanimations
## >makegif("oneplot",(0:99)/100*2pi,"animation");
## >fileremove("animation.gif");
## 
## See: makempeg
	count=1;
	if w!=none and h==none then h=w/getaspect(); endif;
	if w==none and h!=none then w=h*getaspect(); endif;
	for t=x;
		f$(t;args());
		wait(0.01);
		if w==none then
			savepng(filename+printf("%03d",count)+".png",=antialias);
		else
			savepng(filename+printf("%03d",count)+".png",w,h=antialias);
		endif;
		count=count+1;
	end;
	exec("magick","convert +antialias "+ ..
		filename+"*.png "+filename+".gif");
	exec("cmd","/c del "+filename+"*.png");
	exec("cmd","/c start "+filename+".gif");
endfunction

function makempeg (f$:string, x:real vector, 
	filename:string="animation", show=true,
	w:index=none, h:index=none, antialias=true,
	quality=100)
## Makes an animated GIF
##
## This functions works in the same way as makegif().
##
## filename : Filename for images and animation.
## show : Show the GIF in the default viewer after it has been
##    generated.
##
## >function oneplot (a) ...
## $  plot2d("sin(x+a)";a,a=0,b=2pi,c=-1,d=1,color=red,thickness=3);
## $  endfunction
## >load makeanimations
## >makegif("oneplot",(0:99)/100*2pi,"animation");
## >fileremove("animation.gif");
## 
## See: makempeg
	count=1;
	if w!=none and h==none then h=w/getaspect(); endif;
	if w==none and h!=none then w=h*getaspect(); endif;
	for t=x;
	   f$(t;args());
	   wait(0.01);
		if w==none then
			savepng(filename+printf("%03d",count)+".png",=antialias);
		else
			savepng(filename+printf("%03d",count)+".png",w,h=antialias);
		endif;
	   count=count+1;
	end;
	exec("magick convert", ..
		"-quality "+printf("%d",quality)+" "+ ..
		"+antialias "+ ..
		filename+"*.png "+filename+".mpeg");
	exec("cmd","/c del "+filename+"*.png");
	exec("cmd","/c start "+filename+".mpeg");
endfunction
