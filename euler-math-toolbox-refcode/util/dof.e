// Depth of Field of an Ideal Lens

// Load with "load dof.e", help with "help dof.e", see the user menu
// for snippets.

comment
Functions for DOF, hyperfocal distance and Bokeh.

startDOF, endDOF: computes depth of field (DOF)
DOF: prints ranges of DOF
hyperfocal: computes hyperfocal distance
bokeh: computes quality of Bokeh (blurriness at infinity)

lens: prints various parameters for a specific lens

For another calculator:

See: https://www.photopills.com/calculators/dof
endcomment

function endDOF (D, L, A, c=0.03mm, ff=1, inf=NAN)
## Far limit of DOF
##
## If the lens is sharp to infinity the result will be inf.
##
## D : focussed distance in m
## L : film equivalent focal length in m
## A : film equivalent aperture (F-stop times ff)
## c=0.03mm : accepted blurriness radius
## ff=1 : form factor
## inf=NAN : default value for inifinity
##
## See: startDOF
	res = ff*D*L^2/(ff*L^2+c*A*L-c*ff*A*D);
	if res<0 then res=inf; endif;
	return res;
endfunction

function startDOF (D, L, A, c=0.03mm, ff=1)
## Near limit of DOF
##
## D : focussed distance in m
## L : film equivalent focal length in m
## A : film equivalent aperture (F-stop times ff)
## c=0.03mm : accepted blurriness radius
## ff=1 : form factor
##
## See: endDOF
	return ff*D*L^2/(ff*L^2-c*A*L+c*ff*A*D);
endfunction

function DOF (D:real vector, L:real vector, A:real vector, 
	c:real vector=0.03mm, ff:real vector=1, inf=NAN)
## DOF interval
##
## Returns a 1x2 vector containing the bounds of the interval. If one
## of the inputs is a vector the result is a nx2 vector of interval
## bounds.
##
## >DOF(4m,[28mm,35mm,50mm,85mm],8)
##        1.80512           NAN 
##        2.25122       17.9225 
##        2.90023        6.4433 
##        3.53967       4.59796 
##
## See: endDOF, startDOF
	s=size(D,L,A,c,ff);
	res=zeros(s[2],2);
	loop 1 to s[2];
		res[#,]=[startDOF(D{#},L{#},A{#},c{#},ff{#}),
			endDOF(D{#},L{#},A{#},c{#},ff{#},inf)];
	end;
	return res;
endfunction

function hyperfocal (L, A, c=0.03mm, ff=1)
## Hyperfocal distance
##
## L : film equivalent focal length of the lens
## A : filrm equivalent aperture (F-stop times ff)

## c : accepted radius of blurriness
## ff : form factor
## 
## The lense will be acceptably sharp from the hyperfocal distance
## divided by 2 to infinity.
##
## >hyperfocal(18mm,1.5,[2,3,8,16])
##  [5.412,  3.612,  1.362,  0.687]
## 
## See: DOF
	return L*(ff*L+c*A)/(c*ff*A);
endfunction

function bokeh (D, L, A, ff=1)
## Computes the Bokeh
##
## Bokeh is the diameter of the circles of diffusion divided
## by the diameter of the image.
	return 2*L^2/(A*(D-L))/sqrt(0.024^2+0.036^2)
endfunction

function %f30(s) := printf("%-30s : ",s);

function lens (D:real, L:real, A:real, c:real=0.03mm, ff:real=1, equiv=false)
## Print various distances for photographers
##
## D : focussed distance
## L : true focal length (if >equiv then times ff)
## A : true aperture F-stop (if >equiv then F-stop times ff)
## c : accepted blurriness
## ff : form factor
## equiv=false : determines if F and A are true or film equivalent
##
## The printed parameters include DOF, hyperfocal distance and Bokeh
## quality.
##
## See: startDOF, endDOF, hyperfocal, bokeh
	if ff!=1 then
		%f30("Crop factor")+printf("%0.2f",ff),
	endif
	"",
	if not equiv then
		L = L*ff;
		A = A*ff;
	endif
	if ff!=1 then
		%f30("True focal length")+printf("%0.1f mm",L/ff*1000),
	endif
	%f30("35mm equival focal length")+printf("%0.1f mm",L*1000),
	if ff!=1 then
		%f30("F-stop")+printf("F %0.1f",A/ff),
	endif
	%f30("35mm equival F-stop")+printf("F %0.1f",A),
	%f30("Focussed distance")+printf("%0.1f m",D),
	"",
	sd = startDOF(D,L,A,c,ff);
	ed = endDOF(D,L,A,c,ff);
	if isNAN(ed) then
		%f30("Start of DOF")+printf("%0.1f m",sd),
		%f30("End of DOF")+"infinity",
	elseif D<1 then
		%f30("Start of DOF")+printf("%0.1f cm",sd*100),
		%f30("End of DOF")+printf("%0.1f cm",ed*100),
		%f30("Total DOF")+printf("%0.1f cm",(ed-sd)*100),
	else
		%f30("Start of DOF")+printf("%0.2f m",sd),
		%f30("End of DOF")+printf("%0.2f m",ed),
		%f30("Total DOF")+printf("%0.2f m",ed-sd),
	endif
	"",
	%f30("Hyperfocal distance")+printf("%0.1f m",hyperfocal(L,A,c,ff)),
	%f30("Sharpness from")+printf("%0.1f m",hyperfocal(L,A,c,ff)/2),
	"",
	if not isNAN(ed) then
		%f30("Bokeh quality")+printf("%0.1f",bokeh(D,L,A,ff)*100)+"%"
	endif
	
endfunction
