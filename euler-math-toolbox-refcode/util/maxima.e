// Maxima Functions for Euler

comment
Function for Maxima functions.
endcomment

// Euler extends Maxima with some useful functions. The details are
// explained in the documentation of the Euler core.

// See: ../reference/eulercore.html#Extensions_of_Maxima_in_Euler | Extensions of Maxima

// Usually, Maxima is used in Euler seamlessly via symbolic
// expressions. This file contains some other ways to exchange data
// with Maxima.

// The file also contains some functions, which use Maxima in the
// background.

// * Maxima Constants

function %i := I
function %pi := pi
function %e := E
function %phi := (1+sqrt(5))/2

function expt (a,b)
## Maxima expression for a^b.
	return a^b
endfunction

function overwrite mxmstart (directory:string="")
## Start Maxima in the specified directory.
##
## The default directory is the Euler start directory. Euler can also
## start Maxima automatically at program start or if needed (see the
## Options menu).
##
## mxmstop: Stops Maxima.
##
## See: maxima
	if (directory==""); directory=start(); endif;
	return _mxmstart(directory);
endfunction

// * Exchange Values with Maxima

function mxmeval (s:string)
## Evaluate the Maxima expression with additional parameters.
##
## The expression will go through the Maxima interpreter in direct
## mode. Thus this function should only be used for simple term
## evaluations. In the case of a Maxima error, an Euler error will
## occur. After the string is interpreted by Maxima, it is evaluated
## in Euler. At this point any additional assigned parameters and all
## global variables are visible, and can be used.
##
## The function returns an Euler value.
##
## See: maxima
	useglobal;
	return evaluate(mxm(s));
endfunction

function mxmget (s:string)
## Gets a value from Maxima.
##
## This is the same as mxmeval.
##
## See: mxmeval
	return mxmeval(s);
endfunction

function mxmset (var, value="none", large:integer=1)
## Set a variable var to the value x in Maxima.
##
## Can be used with either a variable only, or a name and and a
## value. In the first case, the variable name in Maxima will be the
## same as the name of the Euler variable.
##
## Also, "variable::=value" sets a symbolic expression in Maxima and
## Euler.
##
## See: ::=
	if value=="none" then x=var; name=_name(var); 
	else x=value; name=var;
	endif;
	if typeof(x)==8 then
		mxm(name|'':"''|x|+''"'');
	elseif typeof(x)==13 then
		n=cols(x);
		mxm(name|": zeromatrix(1,@n)");
		for i=1 to n;
			mxm(name|"[1,@i]:"+''"''+x[i]+''"'');
		end;	
	elseif rows(x)>1 and large then
		n=rows(x); m=cols(x);
		mxm(name|": zeromatrix(@n,@m)");
		for i=1 to n;
			for j=1 to m;
				v=x[i,j];
				mxm(name|"[@i,@j]:@v");
			end;
		end;
	else
		mxm(name|":"|convertmxm(x));
	endif
endfunction

function evalinmxm
## Evaluate a Maxima expression in Maxima with values.
##
## In contrast to "mxmeval", this function evaluates the expression in
## Maxima, optionally using additional local values for parameters. To
## set such an additional parameter, add its name as a string, and its
## value as two parameter to the function.
##
## See: mxmeval
	expr=args(1);
	for i=2 to argn() step 2
		t=args(i+1);
		expr=expr|","|args(i)|":"|convertmxm(t);
	end;
	return mxmeval(expr);
endfunction

function mxm2vector (s:string, large:integer=0)
## Turn a Maxima vector into an Euler vector.
##
## For large vectors (>100 elements) set large to 1. For small
## vectors, mxmget("v") or v() will work faster.
	if large then
		mxm("VVV:"|s);
		n=mxmeval("length(VVV)");
		y=zeros(1,n);
		loop 1 to n;
			y[#]=evaluate(mxm("VVV["|#|"]"));
		end;
	else
		return mxmeval(s);
	endif;
	return y;
endfunction

function mxm2str (s:string)
## Turn a Maxima vector of maxima expressions into a string vector.
##
## This reads a vector of expressions in Maxima, and turns it to a
## vector of strings in Euler (e.g., for plotting).
	mxm("VVV:"|s);
	n=mxmeval("length(VVV)");
	y=[mxm("VVV[1]")];
	loop 2 to n;
		y=y|mxm("VVV["|#|"]");
	end;
	return y;
endfunction

function mxm2matrix (s:string, large:integer=0)
## Turn a Maxima matrix into an Euler matrix.
##
## For large matrixed (>100 columns) set large to 1. For small
## matrices, mxmget("A") or A() will work faster.
	mxm("AAA:"|s);
	n=mxmeval("length(AAA)");
	y=mxm2vector(mxm("AAA[1]"),large);
	loop 2 to n;
		y=y_mxm2vector(mxm("AAA["|#|"]"),large);
	end;
	return y;
endfunction

// * Newton Algorithms with Maxima

function mxmnewton (expression:string, 
	start:complex scalar, y:real scalar=0, eps=none)
## Solve expression=y using Newton's method.
##
## The expression must be an expression in x. Maxima is called to
## compute the derivative of the expression. The accuracy of the
## method can be controlled with eps=...
##
## See: mxminewton, mxmnewtonfxy
	if eps then localepsilon(eps); endif;
	df=mxm("diff(@expression,x)");
	oldx=start;
	x=start;
	repeat
		x=x-(expression(x)-y)/df(x);
		if all(x~=oldx) then return x; endif;
		oldx=x;
	end
endfunction

function mxminewton (expression:string, start:interval, ..
	yi:real scalar="", y:real scalar=0)
## Guaranteed inclusion of the solution of expression=y.
##
## The functions uses an interval version of Newton's method. The
## expression must be an expression in x. Maxima is used to find the
## derivative of the expression.
##
## See: inewton
	df=mxm("diff(@expression,x)");
	return inewton(expression,df,start,yi=yi,y=y);
endfunction

function newtonfxy (f1:string, f2:string, f1x:string, ..
	f1y:string, f2x:string, f2y:string, start:real vector)
## Newton's method to a solution of f1(x,y)=0, f2(x,y)=0.
##
## The derivatives must ge given as expressions or functions. All
## expressions must be expressions in x and y.
##
## This function is a special Newton method for two variables. For
## more variables, use "newton2".
##
## See: mxmnewtonfxy, newton2
	s=start;
	repeat
		x=s[1]; y=s[2];
		D=[evaluate(f1x),evaluate(f1y);evaluate(f2x),evaluate(f2y)];
		d=(D\[evaluate(f1);evaluate(f2)])';
		startn=s-d;
		if all(startn~=s) then return startn; endif;
		s=startn;
	end;
endfunction

function mxmnewtonfxy (f1:string, f2:string, start:real vector)
## Newton's method to a solution of f1(x,y)=0, f2(x,y)=0. 
##
## The expressions f1, f2 are passed to Maxima to compute the
## derivatives of f1 and f2. Both expressions must be expressions in x
## and y.
##
## See: newtonfxy
	f1x=mxm("diff(@f1,x)");
	f1y=mxm("diff(@f1,y)");
	f2x=mxm("diff(@f2,x)");
	f2y=mxm("diff(@f2,y)");
	return newtonfxy(f1,f2,f1x,f1y,f2x,f2y,start);
endfunction

// * Differential Equations with Maxima

function mxmtaylorrest (expr:string, deg:integer=10)
## Compute the Taylor development and rest for the expression expr.
##
## The function computes a Taylor series for y, assuming y'=expr(x,y).
## The series has the form y(x+h)=y+expr(x,y)*h+f2(x,y)*h^2+...
## Moreover, the function produces a term for the error. For the
## actual error, this term must be evaluated at any point between x
## and x+h.
##
## The result is useful for mxmidgl.
##
## Returns {taylor,rest}
##
## See mxmidgl
	mxm("res : dgltaylor("|printf("%g",deg)|",subst(y(x),y,@expr),h)");
	taylor=mxm("subst(y,y(x),res[1])");
	rest=mxm("subst(y,y(x),res[2])");
	return {taylor,rest}	
endfunction

function mxmidgl (expr:string, x:real vector, y0:interval scalar, ..
	deg:integer=10, taylor:string="", rest:string="")
## Guaranteed inclusion of the differential equation y'=expr. 
##
## The function uses Maxima to compute a Taylor series of y to the given
## degree, and uses the error term to find a guaranteed solution. 
##
## The series can be computed in advance using mxmtaylorrest.
##
## expr : expression in x and y. Global variables are allowed
##        as long as they are not defined in Maxima.
##
## deg : degree of the method.
## x : is a vector of places, where the solutions should be found.
##
## y0 : start value at x[1].
##
## See: mxmiint, mxmtaylorrest
	n=length(x);
	y=~zeros(1,n)~; y[1]=y0;
	if taylor=="" or rest=="" then
		res=mxm("dgltaylor(@deg,subst(y(x),y,@expr),h)");
		taylor=mxm("subst(y,y(x),@res[1])");
		rest=mxm("subst(y,y(x),@res[2])");
	endif;
	loop 1 to n-1;
		d=x[#+1]-x[#];
		if d>0 then
			i=~x[#],x[#+1]~;
			h=~0,d~;
		else
			i=~x[#+1],x[#]~;
			h=~d,0~;
		endif;
		JT=taylor(x[#],y[#],h=h); 
		J=expand(JT,3/2);
		repeat
			J1=JT+rest(i,J,h=h);
			if all(J1 <<= J) then break; endif;
			J=expand(J,3/2);
		end;
		y[#+1]=taylor(x[#],y[#],h=~d~)+rest(i,J1,h=h);
	end;
	return y;
endfunction

// * Exact Numerical Integration with Maxima

function mxmiint (expr:string, a:real scalar, b:real scalar, ..
	n:integer=20, deg:integer=10)
## Guaranteed inclusion for the integral of expr from a to b.
##
## This functions uses a Taylor series of high degree, and an
## estimation of the error.
##
## expr : expression (must map to arguments and work for intervals)
##        Global variables are allowed, if they are not defined in
##        Maxima.
##
## a,b : interval bounds
## n : number of subinervals
## deg : degree of the Taylor expansion
##
## See: integrate, mxmintegrate, mxmidgl
	v=mxmidgl(expr,linspace(a,b,n),0,deg);
	return v[-1];
endfunction

function mxmisimpson (expr:string, a:number, b:number, n:index=50)
## Interval Simpson integral of f from a to b.
##
## This function uses the Simpson method and its error estimate to get
## guaranteed inclusions of integrals. The estimate involves the 4-th
## derivative, which is computed by Maxima.
##
## expr : expression (must map to arguments and work for intervals).
##        Global variables are allowed, if they are not defined in
##        Maxima.
##
## a,b : interval bounds
## n : number of subintervals
##
## See: isimpson
	return isimpson(expr,mxm("diff(@expr,x,4)"),a,b,n);
endfunction

// * Interval Evaluation with Maxima

function %mxmevaldiff (expression,x)
	useglobal;
	return evaluate(mxm("diff(@expression,x)"))
endfunction

function splitinterval (xi:interval scalar, n:integer)
## Split the interval into n subintervals
	h=(right(xi)-left(xi))/n;
	return left(xi)+(0:(n-1))*h+~0,h~;
endfunction

function mxmieval (expression:string, xi:interval scalar, n:integer=1)
## Better evaluation of the expression in x for the interval xi.
##
## The function is using the mean value theorem. Maxima is called to
## compute the derivative of the expression. The expression must be
## an expression in x. The interval can also be split into
## sub-intervals for more accuracy.
	if n==1 then
		xm=~middle(xi)~;
		der=%mxmevaldiff(expression,xi);
		if isreal(der) && der>=0 then
			return mxmeval(expression,x:=xm)+(xi-xm)*der && expression(x=xi);		
		elseif 0 << der then
			return mxmeval(expression,x:=xm)+(xi-xm)*der && expression(x=xi);
		else
			return expression(x=~left(xi)~) || expression(x=~right(xi)~);
		endif;
	else
		ti=splitinterval(xi,n);
		s=mxmieval(expression,ti[1]);
		loop 2 to n
			s=s||mxmieval(expression,ti[#]);
		end;
		return s;
	endif;
endfunction

function %ievalfxy (ix,iy,ef,edfx,edfy)
	xm=~middle(ix)~; ym=~middle(iy)~;
	return expreval(ef,xm,y:=ym)+ ..
		(ix-xm)*expreval(edfx,ix,y:=iy)+ ..
		(iy-ym)*expreval(edfy,ix,y:=iy);
endfunction

function mxmievalfxy (expression:string, ix:interval scalar, iy:interval scalar)
## Better evaluation of the expression in x and y on intervals.
##
## This function is using the mean value theorem in two variables. The
## derivatives are computed by Maxima.
##
## See: mxmieval
	return %evalifxy(ix,iy,ef,mxm("diff(@expression,x)"), ..
		mxm("diff(@expression,y)"));
endfunction

function %ibisectfxy (ix,iy,ef,edfx,edfy,eg,edgx,edgy,eps=0.01)
	r=zeros(0,2);
	if !(0 << %ievalfxy(ix,iy,ef,edfx,edfy)) ..
		or !(0 << %ievalfxy(ix,iy,eg,edgx,edgy))
	then 
		return r;
	elseif diameter(ix)<eps and diameter(iy)<eps then 
		return [ix,iy];
	endif;
	x=zeros(0,2);
	if diameter(ix)>=eps then
		if diameter(iy)>=eps then
			x=x_[~left(ix),middle(ix)~,~left(iy),middle(iy)~]_ ..
				[~middle(ix),right(ix)~,~left(iy),middle(iy)~]_ ..
				[~left(ix),middle(ix)~,~middle(iy),right(iy)~]_ ..
				[~middle(ix),right(ix)~,~middle(iy),right(iy)~];
		else
			x=x_[~left(x),middle(x)~,y]_[~middle(x),right(x)~,y];
		endif
	else
		x=x_[ix,~left(iy),middle(iy)~]_[ix,~middle(iy),right(iy)~];
	endif
	loop 1 to rows(x)
		r=r_%ibisectfxy(x[#,1],x[#,2],ef,edfx,edfy,eg,edgx,edgy,eps);
	end;
	return r;
endfunction

function mxmibisectfxy (ef:string, eg:string, ..
	ix:interval scalar, iy:interval scalar, eps=0.01)
## Two dimensional bisection method
##
## Use the two dimensional bisection method to find intervals in a
## given interval, where the function could be zero. The derivatives
## are computed by Maxima. Returns a nx2 vector of intervals. The
## first column is the interval of x values, and the second the
## interval of y values.
##
## ef and eg are expressions in x and y.
##
## See: ibisectfxy.
	return %ibisectfxy(ix,iy, ..
		ef,mxm("diff(@ef,x)"),mxm("diff(@ef,y)"), ..
		eg,mxm("diff(@eg,x)"),mxm("diff(@eg,y)"),eps);
endfunction

// * Utility Functions for Maxima

function map mxmdiff (expr:string,x=none,n=1)
## Differentiate the expression with Maxima
##
## Differentiate the expression and optionally evaluate at x. The
## expression must be an expression in x.
##
## See: diff
	if x==none then return mxm("diff(@expr,x,@n)");
	else return mxm("diff(@expr,x,@n)")(x);
	endif
endfunction

function mxmintegrate (expr:string,a=none,b=0)
## Integrate the expression
##
## Integrate the expression (optionally with limits) using Maxima.
## Maxima is used to find an anti-derivative of the expression. Euler
## is used to evaluate the result in b and a.
##
## See: integrate, mxmiint
	Iexpr:=mxm("integrate(@expr,x)");
	if a==none then return Iexpr;
	else return Iexpr(b)-Iexpr(a);
	endif;
endfunction

function mxmsolve (expr:string,n=0,y=0)
## Solve expr=y im Maxima, optionally get the n-th solution.
##
## Call Maxima to solve expr=y, and evaluate all solutions in Euler.
## If n is not zero, extract the n-th solution as a string.
	if n==0 then
		return mxm("map(rhs,solve(@expr=@y,x))")()
	else
		return mxm("rhs(solve(@expr=@y,x)[@n])");
	endif;
endfunction

function mxmdx (expr:string,x=none,y=0)
## mxmdx(expr), mxmdx(expr,x,y) Partial derivative to x.
##
## Maxima is used to compute the partial derivative to x of an
## expression in two variables. Optionally, this derivative is
## evaluated in (x,y).
##
## Note that this function is inefficient, since it calls Maxima each
## time. It is better to compute the partial derivative once, and
## evaluate it for each x and y in Euler.
##
## See: mxmgradient, mxmdy, mxmdiff
	if x==none or y==none then return mxm("diff(@expr,x)");
	else return mxm("diff(@expr,x)")(x,y);
	endif
endfunction

function mxmdy (expr:string,x=none,y=0)
## mxmdy(expr), mxmdy(expr,x,y) Partial derivative to x.
##
## Maxima is used to compute the partial derivative to y of an
## expression in two variables. Optionally, this derivative is
## evaluated in (x,y).
##
## Note that this function is inefficient, since it calls Maxima each
## time. It is better to compute the partial derivative once, and
## evaluate it for each x and y in Euler.
##
## See: mxmgradient, mxmdx, mxmdiff
	if x==none or y==none then return mxm("diff(@expr,y)");
	else return mxm("diff(@expr,y)")(x,y);
	endif
endfunction

function mxmgradient (expr:string,x=none,y=0)
## Gradient of the expression in x and y.
##
## Maxima is used to compute the gradient. The gradient is evaluated
## in x and y.
##
## Note that this function is inefficient, since it calls Maxima each
## time. It is better to compute the gradient once, and evaluate it
## for each x and y in Euler.
##
## See: mxmdx, mxmdy, mxmdiff, mxmhessian
	if x==none or y==none then return mxm("gradient(@expr,[x,y])");
	else return mxm("gradient(@expr,[x,y])")(x,y)
	endif
endfunction

function mxmhessian (expr:string,x=none,y=0)
## Hessian of the expression in x and y.
##
## Maxima is used to compute the Hessian. The gradient is evaluated
## in x and y.
##
## Note that this function is inefficient, since it calls Maxima each
## time. It is better to compute the Hessian once, and evaluate it
## for each x and y in Euler.
##
## See: mxmdx, mxmdy, mxmdiff, mxmhessian
	if x==none or y==none then return mxm("hessian(@expr,[x,y])");
	else return mxm("hessian(@expr,[x,y])")(x,y)
	endif
endfunction

function mxmeigenvalues (A,n=0)
## Eigenvalues of the Matrix as computed by Maxima.
##
## Returns {lambdas,multiplicities} evaluated in Euler, 
## or {lambda[n],multiplicity[n]} as strings, if n>0.
##
## See: eigenvalues, mxmeigenvectors
	if n==0 then
		M=mxm("eigenvalues(@A)$ matrix(%[1],%[2])")();
		return {M[1,:],M[2,:]};
	else
		res:=mxm("eigenvalues(@A)");
		return {mxm("@res[1][@n]"),mxm("@res[2][@n]")};
	endif;
endfunction

function mxmeigenvectors (A,n=0)
## Eigenvectors of the Matrix as computed by Maxima.
##
## Calls Maxima to compute the Eigenvectors of the Matrix, and
## evaluates the result in Euler.
##
## See: eigenvectors, mxmeigenvalues
	k=cols(A);
	if n==0 then
		return redim(mxm("eigenvectors(@A)[2]")(),k,k)';
	else
		return mxm("eigenvectors(@A)[2][@n]");
	endif;
endfunction

function mxmcharpoly (A)
## Coefficients of the characteristic polynomials by Maxima.
##
## See: charpoly
	n=cols(A);
	return mxm("expand(-charpoly(@A,x))$ makelist(coeff(%,x,i),i,0,@n)")();
endfunction

function mxmgetcoeff (p)
## Coefficients of the symbolic polynomial p(x)
	return mxm("makelist(coeff(@p,x,i),i,0,hipow(@p,x))")();
endfunction

// * Tables of Values with Maxima

function %mxmevtable (expr,var,value) ...
	return mxm("@expr,@var=@value")();
endfunction

function mxmtable (expr,var,a,b,d=1,plot=1,points=1,print=1,frac=0)
## Generate a table of values and a plot of these values.
##
## This function uses Maxima to create a table of values in a simple
## way. The "expression" of the variable "var" is incremented from
## a to b with increments d. If the expression is a vector of
## strings, a matrix of values will be created.
##
## expr : any Maxima expression (string or vector of strings)
## var : the variable name (string)
## a,b,d : range of variable and increment
## plot=1 : plot values and add plot to notebook
## points=1 : plot as point plot
## print=1 : print values to the notebook
## frac=0 : print as fractional values
##
## Returns the values
	x=a:d:b;
	if typeof(expr)==8 then k=1;
	else k=size(expr)[2];
	endif;
	y=zeros(cols(x),k);
	loop 1 to cols(x)
		if typeof(expr)==8 then
			y[#,1]=%mxmevtable(expr,var,x[#]);
		else
			for j=1 to k;
			y[#,j]=%mxmevtable(expr[j],var,x[#]);
			end;
		endif;
	end;
	if print then
		if frac then fracprint(y);
		else y,
		endif
	endif;
	if plot then
		plot2d(x,y',points=points); insimg;
	endif;
	return y;
endfunction

// * Gnu PLot

function mxmsetpng (w=600,h=600)
## Setup for Gnu Plot to PNG
##
## This sets the output of Gnu Plot to a PNG file in the home
## directory of Euler ("C:\Users\Name\Euler\gnuout.png"). When Maxima
## starts a similar routine is called automatically, so that this
## function is only necessary if you need another plot size than the
## default 600x600 pixels.
##
## The produced image can be inserted into the notebook with xmmins()
## or with a colon right at the end of the line after a symbolic plot
## command.
##
## >&plot2d(x^3-x,[x,-2,2]):
## >mxmsetpng(600,400);
## >&plot3d (log (x^2*y^2), [x, -2, 2], [y, -2, 2], ...
## >         [grid, 29, 29], ...
## >         [palette, get_plot_option(palette,5)]);
## >mxmins;
## >plot3d("log(x^2*y^2)",r=2,>spectral,grid=10,zoom=3):
## 
## See: mxmins
	term="pngcairo size "+w+","+h;
	mxm(''set_plot_option([gnuplot_term,"''+term+''"]);'');
	file=strrepl(eulerhome()+"gnuout.png","\","/");
	mxm(''set_plot_option([gnuplot_out_file,"''+file+''"])'');
endfunction

function mxmins (lines=35)
## Load a Gnu Plot image into the notebook
##
## The function inserts the default output file of Gnu Plot into
## the notebook (/Users/Name/Euler/gnuout.png). The output file is set
## for &plot2d() during the start of Maxima. 
##
## Instead of mxmins(), the output can be inserted with a colon after
## a symbolic command, but at the end of the command line.
##
## See: mxmsetpng, mxmsetdraw
	loadimg(lines,eulerhome()+"gnuout.png";);
endfunction

function mxmsetdraw (w=600,h=600)
## Sets the proper output for the draw package.
##
## The package draw has different options than plot2d(). If you load
## this package, you need to set the output in each draw(), or you can
## set it globally with this function.
##
## See: mxmsetpng
	file=strrepl(eulerhome()+"gnuout","\","/"); 
	mxmset(file);
	mxm("set_draw_defaults(file_name=file,terminal='pngcairo,"+ ..
		"dimensions=["+w+","+h+"])");	
endfunction
