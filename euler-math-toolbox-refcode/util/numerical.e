// Numerical Algorithms

comment
Various numerical methods.
endcomment

// For demos of some of these algorithms refer to the following
// notebook.

// See: ../Programs/06 - Numerical Analysis

// * Solving Numerical Equations

function bisect (f$:call, a:number, b:number, y:number=0,
	eps=epsilon())
## Bisection method to solve f(x)=y in [a,b]
##
## f is either a function of one real variable, or an expression in x.
## Additional parameters to "bisect" after a semicolon will be passed
## to the function f.
##
## The bisection routine assumes a zero in the interval, i.e., it
## assumes different signs of f(a) and f(b). The routine bisects the
## interval until the desired accuracy is reached. It is a very stable
## function.
##
## It is possible to specify the accuracy epsilon with eps=... as an
## additional parameter. The default is the internal epsilon. The
## absolute error is used.
##
## There is the interval alternative ibisect().
##
## f : function or expression in x
## a,b : interval bounds
## y : target value
## eps : epsilon for the absolute error.
##
## >bisect("x^2",0,2,y=2) // solve x^2=2
##  1.41421356237
## >bisect("x^2",0,2,y=2,eps=0.001)
##  1.41455078125
## 
## >function f(x,a) := exp(-a*x)*cos(x)
## >x=bisect("f",0,pi/2;2,y=0.5) // solve f(x,2)=0.5
##  0.320449786201
## >f(x,2) // test
##  0.5
##
## See: newton, secant, ibisect, call
	if f$(b,args())<y then
		b1=a; a1=b;
		if f$(a,args())<y then error("No zero in interval"); endif;
	else
		a1=a; b1=b;
		if f$(a,args())>y then error("No zero in interval"); endif;
	endif
	repeat
		m=(a1+b1)/2;
		until abs(b1-a1)<eps;
		if f$(m,args())>y then b1=m; else a1=m; endif;
	end;
	return m;
endfunction

function intbisect (f$:call, a:number, b:number, y:number=0)
## Bisection method to find an integer solution for f(x)=y.
##
## This function should only be used for monotone functions. It makes
## sense for functions, which are defined only for integer values.
##
## The solution satisfies f(floor(x))>=y, f(ceil(x))<=y (reversed if
## f is increasing). In the last step, x interpolates f in a linear
## way between floor(x) and ceil(x).
##
## y : target value
##
## >intbisect("chidis(6.5,x)",1,10,y=0.5)
##   7.15896300259
## >chidis(6.5,[7,8])
##   [0.517277,  0.408592]
##
## See: bisect, call
	a=ceil(a); b=floor(b);
	if a==b+1 then error("No integer in interval"); endif;
	if a>b then error("Need an interval a<b for bisection"); endif;
	sign=1; ya=f$(a,args()); yb=f$(b,args());
	if ya<y then
		if yb<y then error("No zero in interval"); endif;
		sign=-1; y=-y; ya=-ya; yb=-yb;
	elseif ya>0 and yb>y then error("No zero in interval");
	endif
	repeat
		m=floor((a+b)/2);
		until m==a;
		ym=f$(m,args())*sign;
		if ym>y then a=m; ya=ym; else b=m; yb=ym; endif;
	end;
	if ya~=yb then return m;
	else return ((ya-y)*b+(y-yb)*a)/(ya-yb);
endfunction

function secant (f$:call, a:complex scalar, b=none, 
	y:number=0, eps=none)
## Secant method to solve f(x)=y in [a,b]
##
## f is either a function of one real variable, or an expression or a
## call collection in x. Additional parameters to "secant" after a
## semicolon will be passed to the function f.
##
## The secant routine uses an approximation of the derivative to find
## a better approximation to the solution. This function is almost as
## good as the Newton method, but does not require the derivative of
## f.
##
## Note that the result is not guaranteed to be within the interval
## [a,b]. For an alternative, see secantin(). Moreover, the function
## may fail, if the approximated derivative is too close to 0.
##
## The parameter b is optional. In this case, the function starts with
## an interval around a.
##
## The method can also be used for complex functions.
##
## It is possible to specify the accuracy with eps=... as last
## parameter.
##
## f : function or expression in x
## a,b : interval bounds, b is optional
## y : target value
##
## >bisect("x^2",0,2,y=2)
##  1.41421356237
##
## >secant("x^2",1,y=2) // expression
##  1.41421356237
## >secant({{"x^2-a",a=2}},1) // call collection
##   1.41421356237
## >function f(x,a) := x^2-a
## >secant("f",5;3), %^2
##  1.73205080757
##  3
## >secant("x^2",1,y=I) // complex equation x^2=I
##  0.707107+0.707107i
##
## See: bisect, newton, secantin, call
	if eps then localepsilon(eps); endif;
	if b==none then x0=a-sqrt(epsilon); x1=a+sqrt(epsilon);
	else x0=a; x1=b; 
	endif;
	y0=f$(x0,args())-y; y1=f$(x1,args())-y;
	repeat
		x2=x1-y1*(x1-x0)/(y1-y0);
		if x2~=x1; break; endif;
		x0=x1; y0=y1; x1=x2; y1=f$(x2,args())-y;
	end;
	return x2
endfunction

function secantin (f$:call, a:real scalar, b:real scalar,
	y:number=0,eps=epsilon())
## Secant method to solve f(x)=y in [a,b]
##
## Works like the secant method. But the result is guaranteed to be
## in [a,b]. Use this for functions, which are not defined outside of
## [a.b]. The parameter "eps" is used for the absolute error.
##
## >secant("sqrt(x)*exp(-x)",0.4,y=0.3) // first leads to x<0
##  Floating point error!
##  Error in sqrt
##  Error in expression: sqrt(x)*exp(-x)
##  secant:         x0=x1; y0=y1; x1=x2; y1=f$(x2,args())-y;
## >secantin("sqrt(x)*exp(-x)",0,0.4,y=0.3) // solve in [0,0.4]
##  0.112769901579
##
## See: bisect, newton, secant, call
	x0=a; x1=b; 
	y0=f$(x0,args())-y; y1=f$(x1,args())-y;
	repeat
		x2=min(max(a,x1-y1*(x1-x0)/(y1-y0)),b);
		if abs(x2-x1)<epsilon then return x2; endif;
		x0=x1; y0=y1; x1=x2; y1=f$(x2,args())-y;
		if y0~=y1 then return bisect(f$,a,b,y,eps;args()); endif;
	end;
endfunction

function solve (f$:call, a:complex scalar, b=none, 
	y:number=0, eps=none)
## Solve f(x)=y near a.
##
## Alias to "secant".
##
## See: secant, call
	if eps then return secant(f$,a,b,y;args(),eps=eps);
	else return secant(f$,a,b,y;args())
	endif;
endfunction

function root (f$:string, %x, eps=none)
## Find the root of an expression f by changing the variable x. 
##
## This is a function for interactive use. It will not work properly
## inside of functions. Use solve() instead.
##
## f must be an expression in several variables. All variables but x
## are constant, and only x is changed to solve f(x,...)=0. All
## variables in the expression must be global variables. Note that the
## actual name of the variable x may be different from "x", so you can
## solve for any variable in the expression f. But the parameter must
## be a variable.
##
## f : expression in several variables
## %x : variable to be solved for
##
## >a=2; x=2; root("x^x-a",x); "a = "+a, "x = "+x,
##  a = 2
##  x = 1.55961046946
## >a=2; x=2; root("x^x-a",a); "a = "+a, "x = "+x,
##  a = 4
##  x = 2
##
## See: secant, solve, newton
	if eps then localepsilon(eps); endif;
	if %x~=0 then x1=0.00001;
	else x1=1.00001*%x;
	endif;
	x0=%x;
	a=f$();
	repeat
		%x=x1; b=f$();
		xn=(x0*b-x1*a)/(b-a);
		if xn~=%x; break; endif;
		x0=x1; x1=xn; a=b;
	end;
	return %x;
endfunction

// * Numerical Iteration

function iterate (f$:call, x:numerical, n:natural=0, 
	till=none, eps=none, best=false)
## Iterates f starting from x to convergence.
##
## This function iterates xnew=f(x), until xnew~=x (using the
## internal epsilon or the optional eps), or the maximal number of
## iterations n is exceeded. If >best is set the iteration will
## continue until abs(x-f(x)) does no longer get smaller. It returns
## the limit for n=0 (the default), or n iterated values. There is a
## "till" expression, which can also stop the iteration.
##
## f is either a function of a scalar returning a scalar, or a
## function of a vector returning a vector. The function will work
## with column or row vectors, and even for matrices, if n==0. f can
## also be an expression or a call collection of a variable x.
##
## The iteration can also be used for intervals or interval vectors.
## In this case, The iteration will stop until the left and right
## interval bounds are closer than epsilon. If >best is set the
## iteration continues until the interval length does not get any
## smaller.
##
## f : function or expression
## x : start point for the algorithm 
## n : optional number of steps (0 = till convergence)
## till : optional end condition
## best : Iterate until best result is achieved (takes long if the
##   limit is 0).
##
## >iterate("cos",1) // solves cos(x)=x, iterating from 1
##  0.739085133216
## >longest iterate("(x+2/x)/2",1,n=7)'
##                        1 
##                      1.5 
##        1.416666666666667 
##         1.41421568627451 
##         1.41421356237469 
##        1.414213562373095 
##        1.414213562373095 
##        1.414213562373095 
## >iterate("x+1",1000,till="isprime(x)")
##  1009
## >function f(x,A,b) := A.x-b
## >A=random(3,3)/2; b=sum(A);
## >iterate("f",zeros(3,1);A,b)
##    -11.7766 
##    -12.4523 
##     -14.938 
## >iterate("(middle(x)+2/x)/2",~1,2~) // interval iteration
##  ~1.414213562372,1.414213562374~
## >iterate("(x+I/x)/2",1) // complex iteration
##  0.707107+0.707107i
##
## Returns {x,n}, where n is the number of iterations needed
##
## See: niterate, call
	if n>0 then
		return niterate(f$,x,n,till;args());
	endif;
	if eps then localepsilon(eps); endif;
	if isinterval(x) then
		count=0;
		repeat
			x=f$(x,args());
			if all(left(x)~=right(x)) then 
				if best then
					err=totalmax(right(x)-left(x));
					repeat
						xn=f$(x,args());
						errn=totalmax(right(xn)-left(xn));
						until errn>=err;
						count=count+1;
						x=xn;
						err=errn;
					end;
					return {x,count+1}
				else				
					return {xn,count+1}; 
				endif;
			endif;
			xn=x;
			count=count+1;
		end;
	else
		count=0;
		repeat
			xn=f$(x,args());
			if till!=none and till(xn) then return {xn,count+1};
			elseif all(x~=xn) then 
				if best then
					err=totalmax(abs(x-xn));
					x=f$(x,args());
					repeat
						xn=f$(x,args());
						errn=totalmax(abs(x-xn));
						until errn>=err;
						count=count+1;
						x=xn;
						err=errn;
					end;
					return {x,count+1};
				else
					return {xn,count+1}; 
				endif;
			endif;
			x=xn;
			count=count+1;
		end;
	endif;
endfunction

function niterate (f$:call, x:numerical, n:natural, till=none)
## Iterates f n times starting from x.
##
## Works like iterate with n>0. 
##
## f : function or expression
## x : start point for the algorithm 
## n : number of iterations
## till : optional end condition
##
## Returns v, where v is the vector of iterations and n the
## number of iterations.
##
## See: iterate, call
	c=(cols(x)==1);
	if c then 
		y=zeros(rows(x),n+1); 
		y[:,1]=x;
	else 
		y=zeros(n+1,cols(x)); 
		y[1]=x;
	endif
	loop 2 to n+1
		x=f$(x,args());
		if c then y[:,#]=x; else y[#]=x; endif;
		if till!=none and till(x) then
			if c then return y[:,1:#] else return y[1:#] endif;
		endif;
	end;
	return y;
endfunction

function %seqhelp (expr$,x,n)
	useglobal;
	return evaluate(expr$);
endfunction

function sequence (f$:call, x:numerical, n:natural)
## Computes sequences recursively.
##
## This function can be used to compute x[n]=f(x[1],...,x[n-1])
## recursively. It will work for scalar x[i], or for column vectors
## x[i]. Depending on the iteration function, more than one start
## value x[1] to x[k] is needed. These start values are stored in a
## row vector x0 (or in a matrix for vector iteration). The iteration
## counter n can be used in the expression.
##
## f$ must be a function f$(x,n), or an expression or a call
## collection in x an n. The parameter x contains the values
## x[1],...,x[n-1] computed so far as elements of a row vector, or as
## columns in a matrix x. In case of a vector sequence, the function
## can also work with row vectors, depending on the result of the
## function f$. Note, that the start value must fit to the result of
## the function.
##
## f$ : function f$(x,n) where x is m x (n-1) or (n-1) x m, 
##     and the result m x 1 or 1 x m
## x : start values (m x k or k x m)
## n : desired number of values
##
## >sequence("x[n-1]+x[n-2]",[1,1],10)
##  [1,  1,  2,  3,  5,  8,  13,  21,  34,  55]
## >sequence("n*x[n-1]",1,9)
##  [1,  2,  6,  24,  120,  720,  5040,  40320,  362880]
## >A=[1,2;3,4]; sequence("A.x[,n-1]",[1;1],5)
##           1          3         17         91        489 
##           1          7         37        199       1069 
## >matrixpower(A,4).[1;1] // more efficiently
##         489 
##        1069 
## >function f(x,n) := mean(x[n-4:n-1])
## >plot2d(sequence("f",1:4,20)):
## 
## See: iterate, call
	loop cols(x)+1 to n;
		if isfunction(f$) then xnew=f$(x,#;args());
		else xnew=%seqhelp(f$,x,#);
		endif;
		if cols(xnew)==1 then x=x|xnew;
		else x=x_xnew;
		endif;
	end;
	return x;
endfunction	

function steffenson (f$:call, x:scalar, n:natural=0, eps=none)
## Does n steps of the Steffenson operator, starting from x.
##
## This is a similar function as iterate. However, the function
## assumes an asymptotic expansion of the error of the convergence, and
## uses the Steffenson operator to speed up the convergence.
##
## f$ is either a function of a scalar returning a scalar, or a
## function of a column vector returning a column vector. f can also
## be an expression or a call collection of a variable x.
##
## f$ : function or expression of a scalar x
## x : start point
## n : optional number of iterations
##
## See: iterate, call
	if eps then localepsilon(eps); endif;
	if n>0 then
		return nsteffenson(f$,x,n;args())
	endif;
	repeat
		a=f$(x,args()); b=f$(a,args());
		c=b-2*a+x;
		if c~=0; return x; endif;
		xn=(x*b-a*a)/c;
		if xn~=x; return xn; endif;
		x=xn;
	end
endfunction

function nsteffenson (f$:call, x0:scalar, n:natural, eps=none)
## Does n steps of the Steffenson operator, starting from x0.
##
## Works like "steffenson", but returns the history of the iteration.
##
## See: steffenson, niterate, call
	if eps then localepsilon(eps); endif;
	y=zeros(rows(x0),n);
	x=x0;
	loop 1 to n;
		a=f$(x,args()); b=f$(a,args());
		c=b-2*a+x;
		if c~=0 then i=#; return y[:,1:i-1]; endif;
		x=(x*b-a*a)/c;
		y[:,#]=x;
	end
	return y;
endfunction

function aitkens (x: vector)
## Improves the convergence of the sequence x.
##
## The Aitkens operator assumes that a-x[n] has a geometric
## convergence 1/c^n to 0. With this information, it is easy to compute
## the limit a.
##
## x : row vector (1xn, n>2)
##
## Return a row vector (1x(n-2)) with faster convergence.
## 
## >v=iterate("cos",1,40); longest v[-1]
##       0.7390851699445544 
## >longest aitkens(v)[-1]
##       0.7390851332151597 
## >longest iterate("cos",1)
##       0.7390851332157367 
##
## See: steffenson
	n=length(x);
	return x[1:n-2]-(x[2:n-1]-x[1:n-2])^2/(x[3:n]-2*x[2:n-1]+x[1:n-2]);
endfunction

// * Newton Methods

function newton (f$:call, df$:call, x:complex scalar, 
	y:number=0, eps=none)
## Solves f(x)=y with the Newton method.
##
## The Newton method needs the derivative of the function f. This
## derivative must be computed by the function df. The Newton method
## will start in x, and stop if the desired accuracy is reached. It is
## possible to set this accuracy with eps=...
##
## The function will only work for real or complex variables. For
## systems of equations, see "newton2". For intervals, see "inewton".
##
## f and df must be a function of a real or complex scalar, returning a
## scalar, or an expression or call collection of x.
##
## f : function or expression in x (real or complex, scalar)
## df : function or expression in x (real or complex, scalar)
## x : start value
## y : target value
##
## >newton("x^2","2*x",1,y=2)
##  1.41421356237
## >function f(x) &= x^3-x+exp(x);
## >function df(x) &= diff(f(x),x);
## >newton(f,df,1), f(%)
##  -1.13320353316
##  0
## >newton(f,df,I), f(%)
##  0.351537+0.802637i
##  0+0i 
##
## >function f(v) &= [v[1]+v[2]-1,v[1]^2+v[2]^2-10]
## >function Df(v) &= jacobian(f(v),[v[1],v[2]])
## >newton2("f","Df",[4,3]), f(%)
##
## See: secant, bisect, newton2, inewton, mxminewton, call
	if eps then localepsilon(eps); endif;
	repeat
		a=f$(x,args())-y;
		b=df$(x,args());
		xnew=x-a/b;
		if all(xnew~=x) then return xnew; endif;
		x=xnew;
	end;
endfunction

function newton2 (f$:call, df$:call, x:numerical, eps=none)
## Newton method to solve a system of equations.
##
## The multidimensional Newton method can solve the equation f(v)=0
## for vectors v. Here, f must map row vectors to row vectors. This
## function needs the Jacobian matrix of f$, which is provided through
## the function df. The Newton method will fail, if the Jacobian gets
## singular during the computation.
##
## f$ must be a function of a row vector, returning a row vector, or
## an expression of the row vector x. df must be a function of a row
## vector, returning the Jacobian matrix of f. Additional parameters
## after a semicolon are passed to the functions f and df. The
## function f can also use column vectors. In this case, the start
## value must be a column vector.
##
## f$ : function f$(v) (v: 1 x n or n x 1, result 1 x n or n x 1)
## df$ : function df$(v) (v: 1 x n or n x 1, result: n x n)
## x : start point (1xn or nx1)
##
## >function f(x) := [x[1]+x[2]-1,x[1]^2+x[2]^2-10]
## >function Df(x) := [1,1;2*x[1],2*x[2]]
## >newton2("f","Df",[4,3]), f(%)
##  [2.67945,  -1.67945]
##  [0,  0]
## >function f([x,y]) &= [exp(x)-y,y-x-2];
## >function Df([x,y]) &= jacobian(f(x,y),[x,y]);
## >newton2("f","Df",[1,1]), f(%)
##  [1.14619,  3.14619]
##  [0,  0]
##
## See: broyden, neldermin, inewton2, call
	if eps then localepsilon(eps); endif;
	c=(cols(x)==1);
	repeat
		if c then d=df$(x,args())\f$(x,args());
		else d=(df$(x,args())\f$(x,args())')';
		endif; 
		xn=x-d;
		if all(xn~=x) then return xn; endif;
		x=xn;
	end
endfunction

function broyden (f$:call, xstart:real, A:real=none, eps=none)
## Finds a zero of f with the Broyden algorithm.
##
## The Broyden algorithm is an iterative algorithm just like the
## Newton method, which solves the equation f(v)=0 for vectors v.
## It tries to approximate the Jacobian of the function f using the
## previous iteration steps. The algorithm will fail, if the Jacobian
## of f in the zero is singular.
##
## The function f$ must take a vector v, and return a vector.
## Additional parameters after the semicolon are passed to f$. The
## function can work with column or row vectors. The start vector must
## be of the same form.
##
## f$ can be an expression or a call collection.
##
## To change the accuracy, you can specify an optional eps=...
##
## f$ : function of one vector
## xstart : start point, a row vector
## A : optional start for the Jacobian
##
## Returns the solution as a vector.
##
## See: nbroyden, call
	if eps then localepsilon(eps); endif;
	c=(cols(xstart)==1);
	x=xstart; n=length(x);
	if c then x=x'; endif;
	delta=sqrt(epsilon());
	if A==none then
		A=zeros(n,n);
		y=f$(x,args());
		loop 1 to n;
			x0=x; x0[#]=x0[#]+delta;
			x1=x; x1[#]=x1[#]-delta;
			if c then
				A[:,#]=(f$(x0,args())-f$(x1,args()))/(2*delta);
			else
				A[:,#]=(f$(x0,args())-f$(x1,args()))'/(2*delta);
			endif;
		end;
	endif;
	y=f$(x,args());
	if c then y=y'; endif;
	repeat
		d=-A\y'; x=x+d';
		y1=f$(x,args()); 
		if c then y1=y1'; endif;
		q=y1-y;
		if all(d~=0) then break; endif;
		A=A+((q'-A.d).d')/(d'.d);
		y=y1;
	end;
	if c then return x'; else return x; endif;
endfunction

function nbroyden (f$:call, xstart:real vector, nmax:natural, 
	A:real=none, eps=none)
## Do nmax steps of the Broyden algorithm.
##
## This function works like "broyden", but returns the steps of the
## algorithm. But it does at most nmax steps.
##
## f$ : function of one row vector, expression or call collection
## xstart : start point, a row vector
## nmax : maximal number of steps
## A : optional start for the Jacobian
##
## Returns a matrix, with one step of the algorithm in each row,
## and the current approximation of the Jacobian.
##
## See: broyden, call
	if eps then localepsilon(eps); endif;
	x=xstart; n=cols(x); r=x;
	delta=epsilon();
	if A==none then
		A=zeros([n n]);
		y=f$(x,args());
		loop 1 to n;
			x0=x; x0[#]=x0[#]+delta;
			A[:,#]=(f$(x0,args())-y)'/delta;
		end;
	endif;
	y=f$(x,args());
	count=0;
	repeat
		count=count+1;
		d=-A\y'; xn=x+d';
		if all(xn~=x) or count>nmax then r=r_xn; break; endif;
		x=xn;
		y1=f$(x,args()); q=y1-y;
		A=A+((q'-A.d).d')/(d'.d);
		y=y1;
		r=r_xn;
	end;
	return {r,A};
endfunction

// * Numerical Differentiation

function %setupdif (n)
	A=zeros([n,2*n+1]);
	loop 1 to n;
	b=zeros([2*#+1,1]); b[#+1]=fac(#);
	a=((-#:#)^(0:2*#)')\b;
	A[#,1:2*#+1]=a';
	end;
	return A;
endfunction

DifMatrix:=%setupdif(5);

function %dif (x, f$, n=1, h=epsilon()^(1/3))
	global DifMatrix;
	if n==0 then
		return f$(x,args()); 
	elseif n==1 then
		return (-f$(x+2*h,args())/12+2*f$(x+h,args())/3 ..
			-2*f$(x-h,args())/3+f$(x-2*h,args())/12)/h
	elseif n<=5 then
		eps=h^(3/(2*n+1));
		h=(-n:n)*eps; xh=h'+x; a=DifMatrix[n,1:2*n+1];
		return a.f$(xh,args())/eps^n;
	else
		error("Too high a derivative!");
	endif;
endfunction

function diff (f$:call, x:numerical, 
	n:natural=1, h:number=epsilon()^(1/3))
## Compute the n-th (n<6) derivative of f in the points x.
##
## Numerical differentiation is inherently somewhat inaccurate for
## general functions. To get a good approximation, the first
## derivative uses 4 evaluations of the function. There is a more
## accurate function diffc() for functions, which are analytic and
## real valued on the real line.
##
## f is either a function in a scalar, or an expression or call
## collection in x. Additional parameters after a semicolon are passed
## to f.
##
## f$ : function or expression of a scalar x
## x : the point, where the derivative should be computed
## n : degree of derivative
## h : optional delta
##
## >diff("x^x",2)
##  6.77258872224
## >&diff(x^x,x)(2)
##  6.77258872224
## 
## See: diffc, call
	return %dif(x,f$,n,h;args())
endfunction

function diffc (f$:call, x:numerical, h:number=epsilon())
## Computes the first derivative for real analytic functions
##
## This uses the imaginary part of a f(x+ih)/h. Thus it must be
## possible to evaluate f into the complex plane.
##
## f$ : function or expression. The function must evaluate for complex
##   values.
## h : step size
##
## See: diff, call
	return im(f$(x+I*h;args())/h);
endfunction

// * FFT, Folding, Filters

// Most functions here are built-in functions. For an introduction to
// the Fourier Transform, see the following tutorial.
//
// See: ../Programs/15 - Fast Fourier Transform

function comment overwrite fft (v:vector)
## Fast Fourier Transform of v
##
## v : Real or complex row vector. It should have a number of columns
## with many low prime factors, like 2^n. This is the same as
## evaluating a polynomial with coefficients v in all roots of unity
## simultaneously, but much faster.
##
## v : If v is a matrix, the function returns the two dimensional
## FFT. In this case, the number of rows and columns of v must be a
## power of 2.
##
## >sec=1.2; t=soundsec(sec); s=sin(440*t)+cos(660*t)/2;
## >i=1:2000; plot2d(i/sec,abs(fft(s))[i]):
##
## See: fht, fft2
endfunction

function comment overwrite ifft (v:vector)
## Inverse Fast Fourier Transform of v
##
## The inverse of fft.
##
## See: fft
endfunction

function comment overwrite fht (v:real vector)
## Fast Hartley Transform of v
##
## The Hartley transform is similar to the Fourier Transform, but
## works from real vectors to real vectors. It can be used for sound
## analysis in the same way the Fourier transform can be used.
##
## >sec=1.2; t=soundsec(sec); s=sin(440*t)+cos(660*t)/2;
## >i=1:2000; plot2d(i/sec,fht(s)[i]):
##
## Algorithm from AlgLib.
##
## See: fft
endfunction

function comment overwrite ifht (v:real vector)
## Inverse Fast Hartley Transform of v
##
## The inverse of fht().
##
## Algorithm from AlgLib.
##
## See: fht
endfunction

function fftfold (v:vector, w:vector)
## fold v with w using the Fast Fourier Transformation.
##
## The length of the vector v should have small prime factors. For
## large v, this is much faster than fold(v,w). However, the vector v
## is folded as a periodic vector. To get the same result as
## fold(v,w), delete the first cols(w)-1 coefficients.
##
## v : row vector (1xn)
## w : row vector (1xm, m<=n)
##
## >fold(1:10,[1,2,1]/4)
##  [2,  3,  4,  5,  6,  7,  8,  9]
## >tail(fftfold(1:10,[1,2,1]/4),3)
##  [2,  3,  4,  5,  6,  7,  8,  9]
##
## See: fold
	v1=zeros(size(v)); v1[1:cols(w)]=w;
	z=ifft(fft(v)*fft(v1));
	if isreal(v) then return real(z); 
	else return z;
	endif;
endfunction

function comment overwrite fold (v:complex, w:complex)
## fold v with w
##
## v and w can be real or complex vectors or matrices.
##
## In the result R, R[i,j] is the total sum of the pointwise
## multiplication of the submatrix of A with A[i,j] in the upper left
## corner and B. I.e.
##
## The result R is of size (cols(A)-cols(B)+1,rows(A)-rows(B)+1).
## Consequently B must be smaller in size than A.
## 
## In the example, folding with [1/2,1/2] takes the average of two
## neighboring elements of 1:10.
## 
## >fold(1:10,[0.5,0.5])
##  [ 1.5  2.5  3.5  4.5  5.5  6.5  7.5  8.5  9.5 ]
## 
## For a matrix, folding with [1/4,1/4;1/4,1/4] takes the average of
## 2x2-sub-matrices.
## 
## Folding with [1,-1] takes the difference of two elements, and is
## equivalent to differences.
##
## See: fftfold
endfunction

function overwrite filter (b:complex vector, a:complex vector, ..
	x:complex vector, y:complex vector=none, zeros:integer=true)
## Apply a filter with a and b to start values x and y
##
## Computes y[n] recursively using
##
##   a[1]*y[n]+...+a[m]*y[n-m+1] = b[1]*x[n]+...+b[k]*x[n-k+1]
##
## a[1] must not be zero, of course.
##
## The start values on the right hand side are either 0 or the first k
## values of x, depending on the flag zeros (true by default). The
## start values on the left hand side are either 0 (the default), or
## the values of the parameter y.
##
## The size of a must exceed the size of x by 1.
## All input vectors must be real or complex row vectors or scalars.
##
## See: filter, fold, fftfold
	if zeros and cols(b)>1 then x=zeros(1,cols(b)-1)|x; endif;
	if y==none then y=zeros(1,cols(a)); endif;
	return _filter(b,a,x,y);
endfunction

// * Interpolation

// Aliases to function in @"eulercore"

function interpolate (x:vector, y:vector)
## Interpolate the values in y at x
##
## Alias to interp. Return the vector of divided differences.
##
## See: interp, evaldivdif
	return _interp(x,y)
endfunction

function evaldivdif (t:numerical, d:vector, x:vector)
## Evaluate the divided differences d at t
##
## Works like interpval with other order of parameters. The first
## parameter contains the points, where the polynomial should be
## evaluated.
##
## Alias to interpval with t-argument first.
##
## Example:
## >xp=0:0.1:1; yp=sqrt(xp);
## >dp=divdif(xp,yp);
## >plot2d("sqrt(x)-evaldivdif(x,dp,xp)",0,1):
##
## See: divdif
	return _interpval(x,d,t)
endfunction

function evalpoly (t:numerical, p:vector)
## evaluates a polynomial p in t.
##
## Polynomials are stored as a row vector with the constant
## coefficient in front. The function works for complex polynomials
## too.
##
## Alias to polyval with t-argument first.
##
## See: polyval
	return _polyval(p,t);
endfunction

// * Hermite Interpolation

function hermiteinterp (x:vector, y)
## Divided differences for Hermite interpolation.
##
## x : row vector of interpolation points. x can contain double
##     points in immediate succession. These points use derivatives
##     for the interpolation.
## y : Either a function df(x,n), which computes the n-th derivatives
##     of f (including the 0-th derivative), or a vector of values
##     and derivatives in the form f(x),f'(x),f''(x),... for a
##     multiple interpolation point x.
##
## Returns the divided differences, which can be used in interpval()
## (divdifeval()) as usual.
##
## The multiplicities in x can be generated with multdup().
##
## >xp=multdup(1:3,2)
##  [ 1  1  2  2  3  3 ]
## >yp=[1,0,0,0,-1,0]
##  [ 1  0  0  0  -1  0 ]
## >d=hermiteinterp(xp,yp)
##  [ 1  0  -1  2  -1.5  1.5 ]
## >plot2d("interpval(xp,d,x)",0.9,3.1);
##
## >function fh (x,n,f,fd) ...
## $  if n==0 then return f(x);
## $  elseif n==1 then return fd(x);
## $  else error("n=0 or n=1!");
## $  endfunction
## >xp=multdup(chebzeros(0,1,5),2);
## >expr &= sqrt(x);
## >d=hermiteinterp(xp,"fh";expr,&diff(expr,x));
## >plot2d("sqrt(x)-interpval(xp,d,x)",0,1);
##
## See: divdif, divdifeval, multdup
	n=length(x);
	d=zeros(1,n);
	if isstring(y) then
		df$=y;
		for i=1 to n;
			d[i]=df$(x[i],0,args());
		end;
		havevalue=false; value=0;
		for i=1 to n-1;
			for j=n to i+1 step -1;
		    	if x[j]==x[j-i] then 
		    		if not havevalue then
		    			value=df$(x[j],i,args())/i!;
		    			havevalue=true;
		    		endif;
		    		d[j]=value;
		    	else
		    		havevalue=false; 
		    		d[j]=(d[j]-d[j-1])/(x[j]-x[j-i]);
		    	endif;
			end;
		end;
	else
		k=1; ind=ones(1,n);
	    for i=2 to n;
	    	if not x[i]==x[i-1] then k=i; endif;
	    	ind[i]=k;
	    end;
		for i=1 to n;
			d[i]=y[ind[i]];
		end;
		havevalue=false; value=0;
		for i=1 to n-1;
			for j=n to i+1 step -1;
		    	if x[j]==x[j-i] then 
		    		if not havevalue then
		    			value=y[ind[j]+i]/i!;
		    			havevalue=true;
		    		endif;
		    		d[j]=value;
		    	else
		    		havevalue=false; 
		    		d[j]=(d[j]-d[j-1])/(x[j]-x[j-i]);
		    	endif;
			end;
		end;
	endif;
	return d
endfunction

function hermitedivdif (x:vector, y)
## Divided differences for Hermite interpolation.
##
## Alias to hermiteinterp()
##
## See: hermiteinterp
	return hermiteinterp(x,y);
endfunction

function multdup (x:numerical, n:nonnegative integer)
## Repeat the rows or columns of x with multiplicities in n
##
## E.g., if n[1]=2, and n is a row vector, the first column of x is
## duplicated.
##
## If n is a column vector, the rows of x are duplicated, if it is a
## row vector, the columns are duplicated. n can be shorter than the
## number of rows resp. columns of x. 
##
## If n is scalar, it acts on the rows, duplicating the first row n
## times.
##
## See: dup, hermite
	if typeof(n)==0 then n=n*ones(1,cols(x)); endif;
	if rows(n)==1 or cols(n)==1 then
	    m=totalsum(n);
		ind=ones(1,m);
		k=1;
		for i=1 to length(n);
			if n[i]>0 then
				ind[k:k+n[i]-1]=i;
				k=k+n[i];
			endif;
		end;
		if cols(n)==1 then return x[ind,:];
		else return x[:,ind];
		endif;
	else
		error("Multiplicities n must be a row or column vector");
	endif;
endfunction

// * Remez Algorithm

function %remezhelp (x,y,n)
	d1=interp(x,y); d2=interp(x,mod(1:n+2,2)*2-1);
	h=d1[n+2]/d2[n+2];
	d=d1-d2*h;
	return {x[1:n+1],d[1:n+1],h};
endfunction

function remez (x:vector, y:vector, deg:index, tracing:integer=0, 
	remezeps=0.00001)
## Best polynomial approximation.
##
## The Remez algorithm computes the best polynomial approximation to
## the data (x[i],y[1]) of degree deg. This algorithm uses a
## simultaneous exchange, which requires the points x[i] to be sorted.
##
## x : vector of points on the real line, sorted.
## y : vector of values at these points.
## deg : degree of the polynomial.
## tracing : if non-zero, the steps will be plotted.
##
## Returns {t,d,h,r}
##
## t : the alternation points
## d : the divided difference form of the best approximation
## h : the discrete error (with sign)
## r : the rightmost alternation point, which is missing in t. 
##
## To evaluate the polynomial in v use interpval(t,d,v).
##
## >x=equispace(-1,1,500); y=abs(x);
## >{t,d,h,r}=remez(x,y,10);
## >plot2d(x,interpval(t,d,x)-y):
## 
## See: interpval, polyfit
	n=cols(x);
	ind=round(linspace(1,n,deg+1),0); // indices of reference points
	h=0; // discrete error on reference points
	repeat
		// compute new reference
		{t,d,hnew}=%remezhelp(x[ind],y[ind],deg);
		// compute reference error
		r=y-interpval(t,d,x); 
		hh=hnew;
		// plot, if wanted
		if tracing then
			xplot(x,r); 
			xgrid(x[ind],ticks=0,color=red);
			ygrid([-hnew,0,hnew],ticks=0,color=red);
			title("Weiter mit Taste"); wait(180);
		endif;
		indnew=ind;
		// simultanuous exchange
		// extrema between first point of x and second reference point
		e=extrema(r[1:ind[2]]);
		if (hh<0) then
			indnew[1]=e[2];
		else
			indnew[1]=e[4];
		endif;
		// extrema between two reference points
		hh1=hh;
		loop 2 to deg+1;
			e=extrema(r[ind[#-1]:ind[#+1]]);
			if (hh>0);
				indnew[#]=e[2]+ind[#-1]-1;
			else
				indnew[#]=e[4]+ind[#-1]-1;
			endif;
			hh=-hh;
		end;
		// extrema on right interval end
		e=extrema(r[ind[deg+1]:cols(r)]);
		if (hh>0) then
			indnew[deg+2]=e[2]+ind[deg+1]-1;
		else
			indnew[deg+2]=e[4]+ind[deg+1]-1;
		endif;
		hh=-hh;
		// total maximum
		mm=max(abs(r[indnew]));
		if ind[deg+2]<cols(r) then
			e=extrema(r[ind[deg+2]:cols(r)]);
			if hh>0 then
				if abs(r[e[2]+ind[deg+2]-1])>mm then
					indnew=indnew[2:deg+2]|(e[2]+ind[deg+2]-1);
				endif;
			else
				if abs(r[e[4]+ind[deg+2]-1])>mm then
					indnew=indnew[2:deg+2]|(e[4]+ind[deg+2]-1);
				endif;
			endif;
		endif;
		mm=max(abs(r[indnew]));
		if (ind[1]>1) then
			e=extrema(r[1:ind[1]]);
			if (hh1>0) then
				if abs(r[e[2]])>mm then
					indnew=e[2]|indnew[1:deg+1]; 
				endif;
			else
				if abs(r[e[4]])>mm then
					indnew=e[4]|indnew[1:deg+1]; 
				endif;
			endif;
		endif;
		ind=sort(indnew);
		if (abs(hnew)<(1+remezeps)*abs(h)) then break; endif;
		h=hnew;
	end;
	return %remezhelp(x[ind],y[ind],deg);
endfunction

// * Nonlinear Optimization

function fmin (f$:call, a:real scalar, b:real scalar=none, 
	d=0.01, dmax=1e20, eps=none)
## Compute the minimum of the convex function f in [a,b].
##
## Uses the golden cut method, starting with the interval [a,b]. The
## method takes about 60*(b-a) function evaluations for full accuracy.
## If no interval is given (b==none), the function searches left or
## right of a with initial step size d doubling d in each step until
## the function gets larger. Then the usual method is started.
##
## f$ is either an expression or call collection in x, or a function of
## x. Additional parameters are passed to a function f$.
##
## You can specify an epsilon eps with eps=... as last parameter.
##
## >fmin("x^x",0,1), 1/E
##  0.367879431154
##  0.367879441171
##
## >function f(x) := x^x
## >f(fmin("f",0,1))
##  0.692200627555
## >fmin({{"x^2+x-a",a=2}},-2,2)
##  -0.500000015805
##
## >function f(x,a) := x^(a*x)
## >f(fmin("f",0,1;1.1),1.1)
##  0.667198694057
##
## See: fmax, call
	if eps then localepsilon(eps); endif;
	if b==none then
		x0=a;
		y0=f$(x0,args());
		x1=x0+d;
		y1=f$(x1,args());
		if y1>y0 then
			d=-d; x1=x0+d;
			y1=f$(x1,args());
		endif;
		repeat
			while y1<y0;
			d=2*d;
			x0=x1; x1=x0+d;
			y0=y1; y1=f$(x1,args());
			until abs(d)>dmax;
		end;
		a=x0-d; b=x1;
	endif
	x0=a; x3=b;
	y0=f$(x0,args()); y3=f$(x3,args());
	l=(3-sqrt(5))/2;
	x1=x0+l*(x3-x0); x2=x3-l*(x3-x0);
	y1=f$(x1,args()); y2=f$(x2,args());
	repeat
		if y1>y2 then
			x0=x1; x1=x2; x2=x3-l*(x3-x0);
			y0=y1; y1=y2;
			y2=f$(x2,args());
		else
			x3=x2; x2=x1; x1=x0+l*(x3-x0);
			y3=y2; y2=y1;
			y1=f$(x1,args());
		endif;
		if x0~=x3 then return x0; endif;
	end;
endfunction

function fmax (f$:call, a:real scalar, b:real scalar=none, 
	d=0.01, dmax=1e20, eps=none)
## Compute the maximum of the concave function f in [a,b].
##
## Works like fmin().
##
## See: fmin
	if eps then localepsilon(eps); endif;
	if b==none then
		x0=a;
		y0=f$(x0,args());
		x1=x0+d;
		y1=f$(x1,args());
		if y1<y0 then
			d=-d; x1=x0+d;
			y1=f$(x1,args());
		endif;
		repeat
			while y1>y0;
			d=2*d;
			x0=x1; x1=x0+d;
			y0=y1; y1=f$(x1,args());
			until abs(d)>dmax;
		end;
		a=x0-d; b=x1;
	endif
	x0=a; x3=b;
	y0=f$(x0,args()); y3=f$(x3,args());
	l=(3-sqrt(5))/2;
	x1=x0+l*(x3-x0); x2=x3-l*(x3-x0);
	y1=f$(x1,args()); y2=f$(x2,args());
	repeat
		if y1<y2 then
			x0=x1; x1=x2; x2=x3-l*(x3-x0);
			y0=y1; y1=y2;
			y2=f$(x2,args());
		else
			x3=x2; x2=x1; x1=x0+l*(x3-x0);
			y3=y2; y2=y1;
			y1=f$(x1,args());
		endif;
		if x0~=x3 then return x0; endif;
	end;
endfunction

function fextrema (f$:call, a:real scalar, b:real scalar, 
	n:integer=100, eps=none)
## Find all internal extremal points of f in [a,b].
##
## f may be an expression or call collection in x or a function.
## Additional parameters after a semicolon are passed to a function f.
## f must vectorize to its first argument.
##
## n : number of sub-intervals to be scanned
## eps : epsilon for the search of the maximum
##
## Returns {minima,maxima} (vectors, possibly of length 0)
##
## See: fmax, fmin, fzeros
	if eps then localepsilon(eps); endif;
	x=linspace(a,b,n);
	mi=[]; ma=[];
	y=f$(x,args());
	loop 2 to n;
		if y[#]>=y[#-1] && y[#]>y[#+1];
			ma=ma|fmax(f$,x[#-1],x[#+1];args());
		elseif y[#]<=y[#-1] && y[#]<y[#+1]
			mi=mi|fmin(f$,x[#-1],x[#+1];args());
		endif;
	end;
	return {mi,ma}
endfunction

function fzeros (f$:call, a:real scalar, b:real scalar,
	n:integer=100, eps=none)	
## Find all zeros of f in [a,b]
##
## f may be an expression or a call collection in x or a function.
## Additional parameters after a semicolon are passed to a function f.
## f must vectorize to its first argument
##
## The function uses fextrema() to find all local extreme values and
## uses bisectin() to find the zeros between sign changes.
##
## n : number of sub-intervals to be scanned eps : epsilon for
## the search of the maximum
##
## See: fextrema, bisectin
	{mi,ma}=fextrema(f$,a,b,n,eps);
	v=unique(a|sort(mi|ma)|b);
	y=f$(v);
	w=[];
	for k=1 to length(v)-1;
	   if y[k]~=0 then w=w|v[k];
	   elseif y[k]*y[k+1]<0 then w=w|secantin(f$,v[k],v[k+1]);
	   endif;
	end;
	if y[-1]~=0 then w=w|b; endif;
	return unique(sort(w));
endfunction

function %fexprv (x,expr,...)
	return expr(x,args(3));
endfunction

function brentmin (f$:call, a:real scalar, d=0.1, eps=epsilon())
## Compute the minimum of f around a with Brent's method.
##
## This function is no longer recommended. Use fmin() instead.
##
## d is an initial step size to look for a starting interval.
## eps is the final accuracy.
##
## Returns the point of minimal value.
##
## f is an expression or call collection in x, or a function in f.
##
## Return the point of minimum.
##
## >function f(x,a) := x^2+x-a
## >brentmin({{"f",2}},1;2)
##   -0.499999992146
##
## See: fmin, fmax, call
	return _brent("%fexprv",a,d,eps,f$;args());
endfunction

function %nelderf (x,f$:call,...)
	return f$(x,args(3));
endfunction

function neldermin (f$:call, v:real, d=0.1, eps=epsilon(), tries=2)
## Compute the minimum of f around v with the Nelder-Mead method.
##
## The Nelder-Mead method is a stable, but slow method to search for a
## local minimum of a function without using any derivative. It should
## not be used for high dimensions. Of course, it can be applied to
## solve a system of equations by minimizing the norm of the errors.
##
## This function cannot be used for one dimension. Use fmin() instead
## with a proper interval to search for the minimum.
##
## f must be function of a row or column vector x, returning a real
## value. Additional parameters after the semicolon are passed to f.
## f can also be an expression or a call collection depending on a
## vector x.
##
## d is an optional initial step size for a starting simplex.
## eps is the final accuracy.
##
## f : function f(v) (v : 1xn or nx1, result: scalar)
## v : start point for the search (1xn or nx1)
## d : optional size of start simplex
## eps : optional accuracy
## tries : number of restarts of the algorithm
##
## Return the point of minimum (1xn vector).
##
## >function f([x,y],a) &= x^2+y^2+exp(-x)+x*y
##    
##                              2          - x    2
##                             y  + x y + E    + x
##    
## >xmin=nelder({{"f",3}},[1,2])
##    [0.432563,  -0.216282]
## >plot2d(&f(x,y,3),>contour); plot2d(xmin[1],xmin[2],>points,>add):
## 
## See: fmin, nelder, call
	m=f$(v,args());
	loop 1 to tries
		v1=_nelder("%nelderf",v,d,eps;f$,args());
		m1=f$(v,args());
		if m1>m then return v; endif;
		v=v1;
	end;
	return v1;
endfunction

function overwrite nelder (f$:call, v:real, d=0.1, eps=epsilon())
## Compute the minimum of f around v with the Nelder-Mead method.
##
## For more explanations and examples, see neldermin().
##
## See: neldermin, call
	return neldermin(f$,v,d,eps;args());
endfunction

function nlfit (f$:call, Df$:call, v:real)
## Minimizes f(x) from start vector v.
##
## This method is named after Levenberg-Marquardt. It minimizes a
## function from n variables to m variables (m>n) by minimizing the
## linear tangent function. If the norm gets larger it decreases the
## step size, until the step size gets closer than epsilon.
##
## While f can be an expression or a call collection, it is easier to
## use symbolic functions as in the example below.
##
## f(x) maps 1xn vectors to 1xm vectors (m>n)
## Df(x) is the Jacobian of f.
##
## >function f([x,y]) &= [x,y,x^2+2*y^2+1];
## >function Df([x,y]) &= jacobian(f(x,y),[x,y]);
## >nlfit("f","Df",[0.2,0.1])
##   [-9.19403e-011,  -1.07327e-010]
## >function h(v) := norm(f(v))
## >nelder("h",[3,1]) // for comparison
##   [6.33484e-007,  -1.9733e-007]
##
## See: nelder, call
	fn=norm(f$(v));
	c=1;
	repeat
		M=Df$(v,args());
		vnew=(1-c)*v-c*fit(M,f$(v,args())'-M.v')';
		fnnew=norm(f$(vnew));
		if fnnew>=fn then
			if !all(v~=vnew) then c=c/2;
			else return vnew;
			endif;
		endif;
		fn=fnnew;
		v=vnew;
	end;
endfunction

function %evalmodel (param,f$,x,y,p,...)
	return norm(y-f$(x,param,args(6)),p);
endfunction

function modelfit (f$:call, param:vector, x:real, y:vector,
	powell=false, p=2, d=1e-2, eps=epsilon(), tries=2)
## Fit a model f$ to data x,y with initial guess p
##
## This uses the Powell or the Nelder-Mead algorithm to fit a
## non-linear function to data x[i],y[1].
##
## f$ : The model function f$(x,p). 
## The function must vectorize to x. x will be a n x m vector for m
## data of n variables. The result of f$(x,p) must be a 1 x m vector.
## f$ can also be an expression or a call collection.
##
## x,y : The data to be used. x can be m x n, and y a 1 x n vector.
## param : Initial guess of the model parameter.
## x,y : Row vector of data.
## powell : If false, Nelder-Mead will be used.
##
## The following parameters are relevant for the Nelder-Mead method.
## Especially, decreasing d may help for difficult models.
##
## d : initial step size
## eps : accuracy
## tries : number of new starts
##
## >function model(x,p) := p[1]*cos(p[2]*x)+p[2]*sin(p[1]*x);
## >xdata = [-2,-1.64,-1.33,-0.7,0,0.45,1.2,1.64,2.32,2.9];
## >ydata = [0.699369,0.700462,0.695354,1.03905,1.97389,2.41143, ...
## >  1.91091,0.919576,-0.730975,-1.42001];
## >modelfit("model",[1,0.2],xdata,ydata)
##
## See: neldermin, powellmin, call
	if length(param)==1 then
		return _brent("%evalmodel",param,d,eps; ..
			f$,x,y,p,args());
	elseif powell or length(param)==1 then
		return powellmin("%evalmodel",param; ..
			f$,x,y,p,args());
	else
		return neldermin("%evalmodel",param,d,eps,tries; ..
			f$,x,y,p,args());
	endif;
endfunction

function %fv (t, f$, x, v, ...)
	return f$(x+t*v,args(5));
endfunction

function powellmin (f$:call, x:real vector)
## Minimize the function f with Powell's method
##
## f$ : function mapping a row vector to a real number
## x : start vector
##
## This function uses Powell's algorithm to find the minimum of a
## convex function. Make sure that the function is defined everywhere.
## You should return a large value outside the proper range of
## definition of f$. Alternatively, setting "errors off" will often
## work.
##
## Additional semicolon parameters are passed to f$. f$ can be a
## call collection or an expression in the vector xn.
##
## >function f([x,y]) := x^2+x*y+y^2+x
## >powellmin("f",[0,0])
##  [-0.666667,  0.333333]
## >function f(x,a) := x^2-a*x
## >powellmin({{"f",3}},1)
##    1.50000000147
## >function f(x) := log(x)*x
## >errors off; powellmin("f",1)
##    0.367879439331
## >errors on;
##
## See: neldermin, errors
	n=cols(x); U=id(n);
	y=f$(x,args());
	d=zeros(1,n);
	repeat
		xold=x;
		ystart=y;
		loop 1 to n;
			yold=y;
			gamma=fmin("%fv",0;f$,x,U[#],args());
			x=x+gamma*U[#];
			y=f$(x,args());
			d[#]=yold-y;
	   end;
	   ex=extrema(d);
	   U[ex[4]]=x-xold;
	   y=f$(x,args()); until y>=ystart;
	end;
	return x;
endfunction

// * Bezier Curves

function bezier (x,y)
## Compute the Bezier curve to points y in x
##
## The Bezier curve is the sum of y_i*B_i(t). The points y
## form the control polygon for the curve. The curve interpolates
## only the first and last point of y. 
##
## In the following example, we take a square as the control
## polygon. Note that t runs from 0 to 1 along the curve.
##
## >y=[0,0,1,1;0,1,1,0];
## >t=linspace(0,1,500);
## >plot2d(y[1],y[2],>addpoints);
## >p=bezier(t,y); 
## >plot2d(p[1],p[2],>add):
## 
## See: spline
	d=cols(y)-1; n=(0:d)';
	B=bin(d,n)*(1-x)^n*x^(d-n);
	return y.B
endfunction

// * Splines

function spline (x,y)
## Defines the natural Spline at points x[i] with values y[i].
## 
## The natural spline is the spline using cubic polynomials
## between the points x[i], smooth second derivative, and linear
## outside the interval x[1] to x[n]. The points x[i] must be sorted.
##
## The function returns the second derivatives at these points. With
## this information, the spline can be evaluated using "splineval".
##
## >xp=1:10; yp=intrandom(1,10,3); s=spline(xp,yp);
## >plot2d("splineval(x,xp,yp,s)",0,11);
## >plot2d(xp,yp,>points,>add);
##
## See: splineval
	n=cols(x);
	h=x[2:n]-x[1:n-1];
	s=h[2:n-1]+h[1:n-2];
	l=h[2:n-1]/s;
	A=diag([n,n],0,2);
	A=setdiag(A,1,0|l);
	A=setdiag(A,-1,1-l|0);
	b=6/s*((y[3:n]-y[2:n-1])/h[2:n-1]-(y[2:n-1]-y[1:n-2])/h[1:n-2]);
	return (A\(0|b|0)')';
endfunction

function map splineval (t:number; x:vector, y:vector, h:vector)
## Evaluates the cubic interpolation spline for [x(i),y(i)] 
##
## The function needs the second derivatives h[i] at t[i].
##
## See: spline
	n=cols(x);
	if t>=x[n] then
		x1=x[n]; x2=x[n-1];
		y1=y[n]; y2=y[n-1];
		f=(x2-x1)^2;
		h1=h[n]*f; h2=h[n-1]*f;
		b=h1/2; c=(h2-h1)/6;
		return y1+(y2-y1-b-c)*(t-x1)/(x2-x1);
	endif;
	if t<x[1] then
		x1=x[1]; x2=x[2];
		y1=y[1]; y2=y[2];
		f=(x2-x1)^2;
		h1=h[1]*f; h2=h[2]*f;
		b=h1/2; c=(h2-h1)/6;
		return y1+(y2-y1-b-c)*(t-x1)/(x2-x1);
	endif;
	p1=find(x,t); p2=p1+1;
	y1=y[p1]; y2=y[p2];
	x1=x[p1]; x2=x[p2]; f=(x2-x1)^2;
	h1=h[p1]*f; h2=h[p2]*f;
	b=h1/2; c=(h2-h1)/6;
	a=y2-y1-b-c;
	d=(t-x1)/(x2-x1);
	return y1+d*(a+d*(b+c*d));
endfunction

function map evalspline (t:number; x:vector, y:vector, s:vector)
## Evaluates the cubic interpolation spline for (x(i),y(i)) 
## with second derivatives s(i) at t
## 
## See: spline, splineval
	return splineval(t,x,y,s);
endfunction

function map linsplineval (x:number; xp:vector, yp:vector,
	constant:integer=1)
## Evaluates the linear interpolating spline for (xp[i],yp[i]) at x
##
## xp must be sorted. Outside of the points of xp, the spline is
## continued as in the closest interval to x, or as a constant
## function, depending on the value of the parameter "constant".
##
## >xp=1:10; yp=intrandom(1,10,3);
## >plot2d("linsplineval(x,xp,yp,<constant)",0,11);
## >plot2d(xp,yp,>points,>add);
	n=cols(xp);
	if (x>=xp[n]) then
		if constant then return yp[n]; endif;
		x1=xp[n]; x2=xp[n-1];
		y1=yp[n]; y2=yp[n-1];
		return y1+(y2-y1)/(x2-x1)*(x-x1);		
	endif;
	if (x<xp[1]) then
		if constant then return yp[1]; endif;
		x1=xp[1]; x2=xp[2];
		y1=yp[1]; y2=yp[2];
		return y1+(y2-y1)/(x2-x1)*(x-x1);		
	endif;	
	p1=find(xp,x); p2=p1+1;
	y1=yp[p1]; y2=yp[p2];
	x1=xp[p1]; x2=xp[p2];
	return y1+(x-x1)*(y2-y1)/(x2-x1);
endfunction

function map evallinspline (x:number; xp:vector, yp:vector,
	constant:integer=1)
## Evaluates the linear interpolating spline for (xp[i],yp[i]) at x
##
## See: linsplineval
	return linsplineval (x,xp,yp,constant);
endfunction

function %nspline (x,k,t)
	return x>=t[k']' && x<t[k'+1]';
endfunction

function nspline (x:real, k:column integer, n:scalar integer,
	t:real vector)
## Compute the normalized B-spline on t[k,k+n+1] in x.
##
## This spline is the normalized B-spline on the interval
## (t[k],t[k+n+1]) and has knots t[k] to t[k+n+1]. The polynomial
## degree is n. The spline is evaluated in the point x.
##
## This function maps to vectors x, but not to k and n.
##
## Examples:
##
## >plot2d("nspline";1,2,[1,2,3,4],a=1,b=4):
## >plot2d("nspline";(1:3)',2,[1,2,3,4,5,6],a=1,b=6):
##
## See: spline
	if n<=0 then return %nspline(x,k,t); endif;	
	return nspline(x,k,n-1,t)*(x-t[k']')/(t[k'+n]'-t[k']'+epsilon)+ ..
		 nspline(x,k+1,n-1,t)*(t[k'+n+1]'-x)/(t[k'+n+1]'-t[k'+1]'+epsilon);
endfunction

// * Adaptive Evaluation

// There are many adaptive functions in EMT for integration and for
// differential equations. Moreover, the plot2d() function evaluates a
// function adaptively by default using adaptiveeval() and
// adaptiveevalone(). The function adaptive() is for adaptive
// evaluation of a general function.

function adaptive (f$:call, a:number, b:number,
	eps=0.01, dmin=1e-5, dmax=1e5, dinit=0.1, factor=2)
## Compute f$(x) for x in [a,b] with adaptive step size.
##
## The adaptive evaluation takes step sizes of different sizes to be
## able to return the values of the function with spacing not
## exceeding a give accuracy. The function is evaluated on an
## interval, with real or complex scalar or vector values.
##
## f$ is an expressions in x, a function or a call collection. The
## function must return a real or complex scalar or a column vector.
##
## eps is the target accuracy.
## a,b are the interval limits
## dmin and dmax are the minimal and maximal step size.
## dinit is the initial step size.
## factor is the factor the step size is changed with.
##
## Returns {x,y}
## x are the points of evaluation, and y is a row vector of values (or
## a matrix with columns of vector values). 
##
## >{x,y}=adaptive("sqrt(x)*exp(-x)",0,10); plot2d(x,y):
## >plot2d(differences(x),>logplot):
##
## >{x,y}=adaptive("x*sin(1/x)",1,epsilon); plot2d(x,y):
##
## >function f(t:number,a:vector) := sum(exp(a*I*t));
## >{t,z}=adaptive({{"f",[1,2,5]}},0,2*pi); plot2d(z,r=3):
##
## See: adaptiveeval
	sn=sign(b-a);
	t=a;
	sy=f$(t;args());
	if sn==0 then return {a,sy}; endif;
	if cols(sy)>1 then 
		error(f$|" does not produce a real or column vector"); 
	endif
	x=zeros(1,1000); y=zeros(rows(sy),1000); 
	n=1; h=dinit;
	x[:,1]=t;
	y[:,1]=sy;
	repeat
		repeat
			tn=t+sn*h;
			if sn*(tn-b)>=0 then tn=b; endif;
			syn=f$(tn;args());
			d=sqrt(colsum(abs(syn-sy)^2));
			if isNAN(d) then break; endif;
			if d>eps and h>dmin then h=max(h/factor,dmin);
			elseif d<eps/factor then h=min(h*factor,dmax); break;
			else break;
			endif;
		end;
		n=n+1;
		if n>cols(x) then
			x=x|x;
			y=y|y;
		endif;
		x[:,n]=tn; y[:,n]=syn;
		sy=syn;
		t=tn;
		if sn*(t-b)>=0 then break; endif;
	end;
	return {x[:,1:n],y[:,1:n]};
endfunction



// * Numerical Integration

// Numerical integration in EMT can be done with various methods. The
// most accurate methods are the adaptive methods adaptivegauss() or
// alintegrate(). Less accurate, but fast methods are the Gauss
// quadrature or the LSODA integration. The function integrate
// defaults to the adaptive Gauss method.

// See: ../Programs/06 - Numerical Analysis.html | Tutorial on Numerical Analysis

function args simpson (f$:call, a:number, b:number, 
	n:natural=50, maps:integer=0, allvalues=0)
## Integrates f in [a,b] using the Simpson method
##
## The Simpson method is stable, but not fast. The results are not
## accurate. The method is exact for polynomials of degree 3. Use
## gauss() for better results.
##
## f is either a function of one real variable, or an expression in x.
## Additional parameters after a semicolon will be passed to the
## function f.
##
## f : function or expression in x
## a,b : interval bounds
## n : number of points to use
## maps : flag, if the expression needs to be mapped (0 or 1).
##        Functions are always mapped.
##
## See: romberg, gauss
	t=linspace(a,b,2*n);
	s=%evalexpression(f$,t,maps;args());
	ff=4-mod(1:2*n+1,2)*2; ff[1]=1; ff[2*n+1]=1;
	if allvalues then
		res=cumsum(ff*s)/3*(t[2]-t[1]);
		i=1:2:2*n+1;
		return {t[i],res[i]};
	else
		return sum(ff*s)/3*(t[2]-t[1]);
	endif;
endfunction

function simpsonfactors (n)
## Returns the vector 1,4,2,...,2,4,1.
##
## See: simpson
	ff=4-mod(1:2*n+1,2)*2; ff[1]=1; ff[2*n+1]=1;
	return ff;
endfunction

function romberg (f$:call, a:number, b:number, m:natural=10, 
	maps:integer=0, eps=none)
## Romberg integral of f in [a,b]
##
## This method gets very accurate results for many functions. The
## Romberg algorithm uses an extrapolation of the trapezoidal rule for
## decreasing step sizes. 
##
## However, the Gauss method is to be preferred for functions, which
## are close to polynomials. The function may fail, if f is not smooth
## enough.
##
## f is either a function of one real variable, or an expression in x.
## Additional parameters after a semicolon will be passed to the
## function f.
##
## f : function or expression in x
## a,b : interval bounds
## m : number of points to use for the start of the algorithm
##
## maps : 
## Flag, if the expression needs to be mapped. Functions are always
## mapped.
##
## >romberg("x^x",0,1)
##  0.783430510714
## 
## See: simpson, gauss, integrate
	if eps then localepsilon(eps); endif;
	y=%evalexpression(f$,linspace(a,b,m),maps;args());
	if cols(y)==1 then return y*(b-a); endif;
	h=(b-a)/m;
	y[1]=y[1]/2; y[m+1]=y[m+1]/2; I=sum(y);
	S=I*h; H=h^2; Intalt=S;
	repeat;
		I=I+sum(%evalexpression(f$,a+h/2:h:b,maps;args())); h=h/2;
		S=S|I*h;
		H=H|h^2;
		Int=interpval(H,interp(H,S),0);
		if Int~=Intalt; break; endif;
		Intalt=Int;
	end;
	return Intalt
endfunction

function adaptiveint (f$:call, a:real scalar, b:real scalar, ..
	eps=epsilon(), steps=10)
## Returns the integral from a to b with the adaptive Runge method.
##
## f is an expression in x, which must not contain y, or a function
## in x. Make sure, that f is computed at each point with relative
## error eps or less. Additional parameters after a semicolon are
## passed to a function f.
##
## f : function or expression in x
## a,b : interval bounds
## eps : accuracy of the adaptive process
## steps : hint for a good step size
##
## >adaptiveint("x^x",0,1)
##  0.783430510714
##
## See: adaptiverunge, integrate
	if iscall(f$) then
		return (runge2("%adaptintf",a,b,0,eps,(b-a)/steps;f$,args()));
	else
		return (runge2("%fdgleval",a,b,0,eps,(b-a)/steps;f$));
	endif;
endfunction

function adaptiveintlsoda (f$:call, a:real scalar, b:real scalar, ..
	eps=epsilon(), steps=10)
## Returns the integral from a to b with the adaptive Runge method.
##
## f is an expression in x, which must not contain y, or a function
## in x. Make sure, that f is computed at each point with relative
## error eps or less. Additional parameters after a semicolon are
## passed to a function f.
##
## f : function or expression in x
## a,b : interval bounds
## eps : accuracy of the adaptive process
## steps : hint for a good step size
##
## See: adaptiverunge, integrate
	if isfunction(f$) then
		return ode("%adaptintf",linspace(a,b,steps),0;f$,args())[-1];
	else
		return ode("%fdgleval",linspace(a,b,steps),0;f$)[-1];
	endif;
endfunction

function %alevalf (x,f$,...)
	return f$(x,args());
endfunction

function overwrite alintegrate (f$:call, a:number, b:number)
## Adaptive integration
##
## Algorithm  from AlgLib similar to adaptivegauss().
##
## See: integrate
	return _alintegrate("%alevalf",a,b;f$,args());
endfunction

function overwrite alsingular (f$:call, a:number, b:number, 
	alpha:number, beta:number)
## Adaptive integration
##
## Algorithm from AlgLib to integrate a function with singularities
## at the boundaries. The singularities must be of type (x-a)^alpha
## and (b-x)^beta. Either alpha or beta can be 0, which denotes a
## smooth function.
##
## >alsingular("1/sqrt(x)",0,1,-0.5,0)
##  2
##
## See: integrate, alintegrate
	return _alsingular("%alevalf",a,b,alpha,beta;f$,args());
endfunction

function integrate (f$:call, a:real scalar, b:real scalar, ..
	eps=epsilon(), steps=10, method:integer=0, fast:integer=0,
	maps:integer=1)
## Integrates f from a to b with the adaptive Runge method.
##
## Calls an integration method, depending on the method parameter. The
## default is the Gauss integration with 10 subintervals. Available is
## the adaptive Runge, Gauss and the LSODA method. Note, that the
## fastest algorithm is the Gauss algorithm, which is the method of
## choice.
##
## method : 
##   0=adaptive Gauss, (needs vectorized function)
##   1=adaptive Runge, 
##   2=lsoda, 
##   3=AlgLib
##
## fast : Simple Gauss with one sub-interval (needs vectorized
##   function)
##
## >integrate("x^x",0,1)
##  0.783430510712
##
## See: mxmiint, gauss, adaptivegauss, alintegrate, lsoda
	if fast then return gauss(f$,a,b,10;args());
	elseif method==0 then 
		return adaptivegauss(f$,a,b,eps*1000;args(),=maps);
	elseif method==1 then 
		return adaptiveint(f$,a,b,eps,steps;args());
	elseif method==2 then 
		return adaptiveintlsoda(f$,a,b,eps,steps;args());
	elseif method==3 then 
		return alintegrate(f$,a,b;args());
	else error("Use method 0-3 for integrate().");
	endif;
endfunction

// * Gauss-Quadrature

function legendre (n:index)
## Compute the coefficients of the n-th Legendre polynomial.
##
## This is used to compute the Gauss coefficients.
##
## See: gauss
	p0=1; p1=[0,1];
	loop 2 to n;
		p=((0|(2*#-1)*p1)-((#-1)*(p0|0|0)))/#;
		p0=p1; p1=p;
	end;
	return p1
endfunction

function gaussparam (n:index)
## Returns the knots and alphas of gauss integration at n points in [-1,1].
##
## Returns {gaussz,gaussa}.
##
## See: gauss, gauss10
	p=legendre(n);
	z=sort(re(polysolve(p)));
	Z=dup(z,n)^dup((0:n-1)',n);
	a=Z\(2/(1:n)'*mod(1:n,2)');
	return {z,a'}
endfunction

gaussz := [ ..
-9.7390652851717172e-0001,
-8.6506336668898451e-0001,
-6.7940956829902441e-0001,
-4.3339539412924719e-0001,
-1.4887433898163121e-0001,
 1.4887433898163121e-0001,
 4.3339539412924719e-0001,
 6.7940956829902440e-0001,
 8.6506336668898451e-0001,
 9.7390652851717172e-0001];
gaussa := [ ..
 6.6671344308688139e-0002,
 1.4945134915058059e-0001,
 2.1908636251598205e-0001,
 2.6926671930999635e-0001,
 2.9552422471475288e-0001,
 2.9552422471475287e-0001,
 2.6926671930999635e-0001,
 2.1908636251598205e-0001,
 1.4945134915058059e-0001,
 6.6671344308688137e-0002];

function map %mapexpression1 (x:scalar; expr:call)
	return expr(x,args());
endfunction

function %evalexpression (f$:call, x:numerical, maps=0)
	if isfunction(f$) then
		if maps then return map(f$,x;args());
		else return f$(x,args());
		endif;
	else
		if maps then return %mapexpression1(x,f$;args());
		else return f$(x,args());
	endif;
endfunction

function %gauss10 (f$:string, a:real scalar, b:real scalar, ..
	maps:integer=0)
## Evaluates the gauss integration with 10 knots an [a,b].
##
## f is an expression in x, or a function in one scalar variable.
## Additional parameters after the semicolon are passed to f.
##
## See: gauss
	return gauss(f$,a,b,maps;args());
endfunction

function gauss (f$:call, a:real scalar, b:real scalar, n:index=1, ..
	maps:integer=1, xn=gaussz, al=gaussa)
## Gauss integration with 10 knots and n subintervals.
##
## This function is exact for polynomials up to degree 19, even for
## one sub-interval. For other functions, it may be necessary to
## specify n subintervals.
##
## f is an expression in x, or a function in x. Additional parameters
## after the semicolon are passed to f.
##
## maps : Vectorize an expression to the arguments. On by default.
##    Turn off for a bit more performance.
##
## See: gauss10, gauss5, simpson, integrate, romberg
	h=(b-a)/n/2;
	if n==1 then
		if maps then y=%evalexpression(f$,a+h-(h*xn)',maps;args());
		else y=f$(a+h-(h*xn'),args());
		endif;
		return al.y*h
	else
		x=linspace(a+h,b-h,n-1);
		if maps then y=%evalexpression(f$,x-(h*xn)',maps;args());
		else y=f$(x-(h*xn'),args());
		endif;
		return sum(al.y)*h
	endif;
endfunction

function igauss (expr$:call, a:real, b:real, n:index=1, ..
	dexpr20$:string=none, dexpr20est:real=none, xn=gaussz, al=gaussa)
## Compute an interval inclusion for an integral
##
## This works only for expressions, unless you provide an estimate for
## the 20-th derivative. The function will compute this derivative
## with Maxima otherwise.
##
## Optional:
##
## dexpr20: Expression or function for the 20-th derivative
## dexpr20est: Interval inclusion of the 20-th derivative 
##
## >igauss("exp(-x^2)",0,1)
##  ~0.7468241328124256,0.7468241328124285~
## 
## See: gauss, gauss5
	h=(b-a)/n/2;
	if n==1 then
		x=a+h;	
	else
		x=linspace(a+h,b-h,n-1);
	endif;
	X=~x-(h*xn)'~;
	res=sum(al.expr$(X;args()))*h;
	if dexpr20est==none then
		if dexpr20$==none then 
			dexpr20$=mxm("diff("+expr$+",x,20)"); 
		endif
		dexpr20est=ieval(dexpr20$,~a,b~,10*n);
	endif;
	gausserror=2.925590330737589e-006;
	return res+dexpr20est*((b-a)/(2*n))^21*gausserror/20!;
endfunction

gauss5z := [ ..
-0.8611363115940526, ..
0.8611363115940526, ..
-0.3399810435848562, ..
0.3399810435848562];

gauss5a := [ ..
0.3478548451374539, ..
0.3478548451374539, ..
0.6521451548625461, ..
0.6521451548625461];

function gauss5 (f$:call, a:real scalar, b:real scalar, n:index=1, ..
	maps:integer=0, xn=gauss5z, al=gauss5a)
## Gauss integration with 4 knots and n subintervals
##
## This is exact for polynomials to degree 7 (even for n=1).
## The results are usually much better than the Simpson method
## with only twice as many function evaluations.
##
## See: gauss, simpson, integrate, romberg
	return gauss(f$,a,b,n,maps,xn,al;args());
endfunction

function %evalfxy (x,expr$,y)
	return evaluate(expr$);
endfunction

function map %gaussinner(y;f$,a,b,n=1)
	if isfunction(f$) then
		return gauss(f$,a,b,n;y)
	else
		return gauss("%evalfxy",a,b,n;f$,y)
	endif;
endfunction

function gaussfxy (f$:call, a:real scalar, b:real scalar, ..
	c:real scalar, d:real scalar, n:index=1)
## Computes the double integral of f on [a,b]x[c,d].
##
## This function uses Gauss integration for the inner and the outer
## integral. The function is exact for polynomials in x and y up to
## degree 19.
##
## f$ : a function f(x,y) or an expression of x and y.
##
## >gaussfxy("exp(x)*y+y^2*x",0,1,0,2)
##  4.76989699025
## >&integrate(integrate(exp(x)*y+y^2*x,x,0,1),y,0,2)() // exact!
##  4.76989699025
##
## See: gauss
	return gauss("%gaussinner",c,d,n;f$,a,b,n)
endfunction

function adaptivegauss (f$:call, a:real scalar, b:real scalar, ..
	eps=epsilon*100, n=1, maps:integer=1)
## Adaptive Gauss integral of f on [a,b].
##
## This function divides the interval into subintervals, and tries a
## Gauss integration with 1 and 2 intervals on each sub-interval. If
## both do not agree good enough the step size is divided by 2. the
## algorithm tries to double the step size in each step..
##
## n : default number of subintervals
## eps : default accuracy
## maps : Vectorizes the call to f$. Turn off for more performance.
##
## Examples:
## adaptivegauss("sqrt(x)",0,1)
##
## See: gauss
	if b~=a then return 0; endif;
	if b<a then {a,b}={b,a}; sign=-1; else sign=1; endif
	h=(b-a)/n;
	sum=0;
	c=a;
	repeat
		t1=gauss(f$,c,c+h;args(),=maps);
		t2=gauss(f$,c,c+h,2;args(),=maps);
		if abs(t1-t2)>eps*h/abs(b-a) then
			if c==a then h=h/10; else h=h/2; endif;
			continue;
		endif;
		sum=sum+t2;
		until c>=b or c~=b;
		c=c+h;
		h=min(1.5*h,b-c);
	end;
	return sign*sum;
endfunction

// * Differential Equations

// The LSODA method should be the method of choice. It is a very
// efficient adaptive method for ordinary differential equations,
// which takes also care of stiff cases. But the Heun and Runge
// methods are available too, including an adaptive Runge method,
// which can be used for adaptive integration.

// Vector fields can be plotted using the vectorfield() function in
// plot2d.

// There are also the interval algorithms mxmidlg() and mxmiint(). 

// See: interval | Interval solvers and guaranteed solutions
// See: maxima | The function mxmidgl()

// Numerical Solutions of Differential Equations

function heun (f$:string, t:real vector, y0:real vector)
## Solves the differential equation y'=f(x,y) at x=t with y(t[1])=y0.
##
## This function works also for systems of n differential equations.
##
## f must be a function with two parameters f(x,y), where x is scalar,
## and y is scalar or a 1xn row vector in the case of a system of
## differential equations. f must return a scalar, or a 1xn row vector
## respectively. Additional parameters for "heun" after a semicolon
## are passed to a function f. Alternatively, f can be an expression
## in x and y. 
##
## y0 is the initial value y(t[1]), a scalar, or a 1xn row vector. 
## t is the 1xm row vector of points, where the differential equation
## will be solved.
##
## f : function or expression in x and y
## t : row vector of points, where y(t) should be computed
## y0 : initial value, scalar or row vector
##
## The function returns a 1xm row vector of y, or a nxm matrix in the
## case of a system of n equations.
##
## >x=0:0.01:2; plot2d(x,heun("-2*x*y",x,1)):
## >function f(x,y,a) &= -a*x*y;
## >heun("f",x,1;2)[-1]
##  0.0183155649842
## >exp(-4)
##  0.0183156388887
##
## See: runge
	n=cols(t);
	y=dup(y0,n);
	loop 1 to n-1;
		h=t[#+1]-t[#];
		yh=y[#]; xh=t[#];
		k1=f$(xh,yh,args());
		k2=f$(xh+h/2,yh+h/2*k1,args());
		k3=f$(xh+h,yh+2*h*k2-h*k1,args());
		y[#+1]=yh+h/6*(k1+4*k2+k3);
	end;
	return y';
endfunction

function %fdgleval (x,y,expr)
	return expr(x,y);
endfunction

function runge (f$:string, t:real vector, y0:real, ..
	steps:index=1)
## Solves the differential equation y'=f(x,y) at x=t with y(t[1])=y0.
##
## This function works also for systems of n differential equations.
##
## f must be a function with two parameters f(x,y), where x is scalar,
## and y is scalar or a vector in the case of a system of differential
## equations. f must return a scalar, or a vector respectively.
## Additional parameters for "runge" after a semicolon are passed to a
## function f. Alternatively, f can be an expression in x and y. 
##
## y0 is the initial value y(t[1]), a scalar, or a 1xn row vector. 
## t is the 1xm vector of points, where the differential equation will
## be solved.
##
## steps are optional intermediate steps between the t[i]. This
## parameter allows to take extra steps, which will not be stored into
## the solution.
##
## Note that f can work with row or column vectors, but the initial
## value must be of this form too. The return matrix will always
## consist of column vectors.
##
## f : function or expression in x and y
## t : row vector of points, where y(t) should be computed
## y0 : initial value, scalar or row vector
## steps : additional steps between the points of x
##
## Returns a row vector y(t[i]) for the elements of t. Im case of a
## system, the function returns a matrix with columns y(t[i]).
##
## >x=0:0.01:2; plot2d(x,runge("-2*x*y",x,1)):
## >function f(x,y,a) &= -a*x*y;
## >runge("f",x,1;2)[-1]
##  0.018315639424
## >exp(-4)
##  0.0183156388887
## 
## See: heun, adaptiverunge
	if cols(y0)==1 and rows(y0)>1 then
		y=dup(y0,cols(t))';
		if isfunction(f$);
			loop 2 to cols(t);
				y[#]=runge1(f$,t[#-1],t[#],steps,y[#-1]';args())';
			end;
		else
			loop 2 to cols(t);
				y[#]=runge1("%fdgleval",t[#-1],t[#],steps,y[#-1]';f$)';
			end;
		endif;
	else
		y=dup(y0,cols(t));
		if isfunction(f$);
			loop 2 to cols(t);
				y[#]=runge1(f$,t[#-1],t[#],steps,y[#-1];args());
			end;
		else
			loop 2 to cols(t);
				y[#]=runge1("%fdgleval",t[#-1],t[#],steps,y[#-1];f$);
			end;
		endif;
	endif;
	return y';
endfunction

function adaptiverunge (f$:call, x:real vector, y0:real, ..
	eps=epsilon(), initialstep=0.1)
## Solves the differential equation y'=f(x,y) at x=t with y(t[1])=y0
## adaptively.
##
## The function uses an adaptive step size between x[i] and x[i+1] to
## guarantee an accuracy eps. By default the accuracy is the default
## epsilon, but eps=... can be specified as an extra parameter.
##
## This function works also for systems of n differential equations.
##
## f must be a function with two parameters f(x,y), where x is scalar,
## and y is scalar or a vector in the case of a system of differential
## equations. f must return a scalar, or a vector respectively.
## Additional parameters for "adaptiverunge" after a semicolon are
## passed to a function f. Alternatively, f can be an expression in x
## and y.
##
## y0 is the initial value y(t[1]), a scalar or a vector. t is the 1xm
## vector of points, where the differential equation will be solved.
##
## Note that f can work with row or column vectors, but the initial
## value must be of this form too. The return matrix will always
## consist of column vectors.
##
## f : function or expression in x and y
## t : row vector of points, where y(t) should be computed
## y0 : initial value, scalar or row vector
## eps : accuracy of the adaptive method
## initialstep : initial step size
##
## Returns a row vector y(t[i]) for the elements of t. Im case of a
## system, the function returns a matrix with columns y(t[i]).
##
## >longest adaptiverunge("-2*x*y",[0,2],1)
##                        1     0.01831563888880886 
## >longest E^-4
##      0.01831563888873419 
## 
## See: heun, runge
	if cols(y0)==1 and rows(y0)>1 then
		y=dup(y0,cols(x));
		if isfunction(f$);
			loop 2 to cols(x);
				{y[:,#],initialstep}= ..
					runge2(f$,x[#-1],x[#],y[:,#-1],eps,initialstep,args());
			end;
		else
			loop 2 to cols(x);
				{y[:,#],initialstep}= ..
					runge2("%fdgleval",x[#-1],x[#],y[:,#-1],eps,initialstep,f$);
			end;
		endif;
		return y;
	else
		y=dup(y0,cols(x));
		if isfunction(f$);
			loop 2 to cols(x);
				{y[#],initialstep}= ..
					runge2(f$,x[#-1],x[#],y[#-1],eps,initialstep,args());
			end;
		else
			loop 2 to cols(x);
				{y[#],initialstep}= ..
					runge2("%fdgleval",x[#-1],x[#],y[#-1],eps,initialstep,f$);
			end;
		endif;
		return y';
	endif;
endfunction

function %adaptintf (x,y,f$,...)
	useglobal;
	return f$(x,args(4));
endfunction

function ode (f$:call, t:real vector, y:real, ..
	eps:real=epsilon(), warning:integer=false, reset:integer=false)
## Solves the differential equation y'=f(x,y) at x=t with y(t[1])=y0.
##
## This function works also for systems of n differential equations.
## The algorithm used is the LSODA algorithm for stiff equations.
## This algorithm switches between stiff and normal case
## automatically.
##
## The LSODA algorithm is based on work by Linda R. Petzold  and  
## Alan C. Hindmarsh, Livermore National Laboratory. The EMT version
## is based on a C source by Heng Li, MIT.
##
## f must be a function with two parameters f(x,y), where x is scalar,
## and y is scalar or a vector in the case of a system of differential
## equations. f must return a scalar, or a vector respectively.
## Additional parameters for "lsoda" after a semicolon are passed to a
## function f. Alternatively, f can be an expression or a call
## collection in x and y.
##
## This is an adaptive algorithm. But for some functions, it is
## necessary to add intermediate points, especially if the function is
## close to 0 over wide subintervals. Otherwise, it has been observed
## that the algorithm fails. If you are sure that the problem does not
## fall under the problematic category, you can leave reset=false, which
## will interpolate intermediate points instead of resetting the
## algorithm every time. This is a huge benefit in terms of function
## evaluations. By default, reset is false.
##
## y0 is the initial value y(t[1]), a scalar, or a 1xn row vector. 
## t is the 1xm vector of points, where the differential equation will
## be solved.
##
## Note that f can work with row or column vectors, but the initial
## value must be of this form too. The return matrix will always
## consist of column vectors.
##
## f : function or expression in x and y
## t : row vector of points, where y(t) should be computed
## y0 : initial value, scalar or row vector
## warning : toggles warnings from lsoda (but not errors)
## reset : reset the integration between iterations
##
## Returns a row vector y(t[i]) for the elements of t. Im case of a
## system, the function returns a matrix with columns y(t[i]).
##
## >function f(x,y,a) &= -a*x*y;
## >ode({{"f",2}},x,1)[-1]
##  0.0183156388888
## >exp(-4)
##  0.0183156388887
## 
## >ode("x*y",[0:0.1:1],1)[-1], exp(1/2) // solve y'=x*y
##  1.64872127078
##  1.6487212707
##
## >function f(x,y) := -x*y^2/(y^2+1); // y'=-xy/(y^2+1)
## >t=linspace(0,5,1000); plot2d(t,ode("f",t,1)):
##
## >sol &= ode2('diff(y,x)=-x*y^2/(1+y^2),y,x) // check with Maxima
## >&solve(sol with %c=0,y)[2], plot2d(&rhs(%),0,5):
##
## >function f(x,y) := [y[2],-10*sin(y[1])]; // y''=-10*sin(y)
## >t=linspace(0,2pi,1000); plot2d(t,ode("f",t,[0,1])[1]):
##
## See: heun, runge, adaptiverunge, lsoda, call
	if isfunction(f$) then
		return _lsoda(f$,t,y,eps,warning,reset;args());
	else
		return _lsoda("%fdgleval",t,y,eps,warning,reset;f$);
	endif;
	return y;
endfunction

function overwrite lsoda (f$:call, t:real vector, y:real, ..
	eps:real=epsilon(), warning:integer=true, reset:integer=false)
## Solves the differential equation y'=f(x,y) at x=t with y(t[1])=y0.
##
## See: ode
	return ode(f$,t,y,eps,warning,reset;args());
endfunction

// * Sparse Matrices

// Euler has support for a compressed format for thin matrices. This
// format stores non-zero elements only. Operations on such matrices
// are typically very much faster. For examples see the following
// introduction.

// See: ../Programs/19 - Large Systems | Introduction to Large Systems

// To solve large, sparse equations the CG-method implemented in cgX()
// or cpxfit() is the method of choice. The function cpxfit() uses the
// normal equation to fit Ax-b. The Gauss-Seidel method seidelX() is an
// alternative for diagonal dominant matrices.

function comment cpx (A)
## Compress the matrix A
##
## The compressed matrix stores only the non-zero values of A. It
## consists of lines of the form (i,j,x), which means A[i,j]=x. Note
## that compressed matrices are a separate data type in Euler.
##
## >cpx(id(3))
##  Compressed 3x3 matrix
##      1     1                    1
##      2     2                    1
##      3     3                    1
##
## See: decpx
endfunction

function comment decpx (X)
## Decompress the compressed matrix X
##
## >C=cpxzeros(3,3)
##  Compressed 3x3 matrix
## >decpx(cpxset(C,[1,1,0.5;2,3,0.7]))
##            0.5             0             0 
##              0             0           0.7 
##              0             0             0 
##
## See: cpx
endfunction

function comment cpxzeros ([n,m])
## Empty compressed matrix of size nxm
endfunction

function comment cpxset (X,K)
## Set elements in the compressed matrix X to 
##
## K is a matrix containing lines of type (i,j,x). Then X[i,j] will be
## set to x. If the element already exists in X, it is replaced by the
## new value.
##
## >function CI(n) := cpxset(cpxzeros(n,n),(1:n)'|(1:n)'|1)
## >decpx(CI(3))
##              1             0             0 
##              0             1             0 
##              0             0             1 
##
## See: cpx, decpx, cpxget
endfunction

function comment cpxget (X)
## Get the elements of a compressed matrix X
##
## The return value is a matrix containing lines of type (i,j,x).
##
## >H=[1,1,0.1;2,3,0.7]
##              1             1           0.1 
##              2             3           0.7 
## >cpxget(cpxset(cpxzeros(3,3),H))
##              1             1           0.1 
##              2             3           0.7 
## 
## See: cpxset
endfunction

function comment cpxmult (A,B)
## Multiply two compressed matrices
##
## This is a fast algorithm to multiply two compressed, sparse
## matrices. The result is a compressed matrix.
##
## See: cpx
endfunction

// * Gauss-Seidel for Sparse Systems

function comment cpxseidel (C,b,x,om)
## One step of the Gauss-Seidel algorithm for Cx=b.
##
## C must be a compressed matrix, x is the step to go from, and om is
## the relaxation parameter.
##
## See: seidelX, cgX
endfunction

function seidelX (H:cpx, b:real column, x:column=0, ..
	om:real positive scalar=1.2, eps=none)
## Solve Hx=b using the Gauss-Seidel method for compressed matrices H.
##
## The Gauss-Seidel method  with Relaxation is an iterative method,
## which converges for all positive definite matrices. For large
## matrices, it may work well. However, the conjugate gradient method
## "cgX" is the method of choice.
##
## H must be diagonal dominant, at least not have 0 on the diagonal.
## om is the relaxation parameter between 1 and 2. x is start value
## (automatic if 0). It is possible to specify the accuracy with
## eps=...
##
## H : compressed matrix (nxm)
## b : column vector (mx1)
## om : optional relaxation coefficient
##
## Returns the solution x, a column vector.
##
## See: cgX, seidel
	if eps then then localepsilon(eps); endif;
	if x==0 then x=zeros(size(b)); endif;
	repeat
		xn=cpxseidel(H,b,x,om);
		if all(xn~=x) then return xn; endif;
		x=xn;
	end;
endfunction

function cgX (H:cpx, b:real column, x0:real column=none, 
	f:index=10, eps=none)
## Conjugate gradient method to solve Hx=b for compressed H.
##
## This is the method of choice for large, sparse matrices. In most
## cases, it will work well, fast, and accurate.
##
## H must be positive definite. Use cpxfit(), if it is not.
##
## The accuracy can be controlled with an additional parameter
## eps. The algorithm stops, when the error gets smaller then eps, or
## after f*n iterations, if the error gets larger. x0 is an optional
## start vector.
##
## H : compressed matrix (nxm)
## b : column vector (mx1)
## x0 : optional start point (mx1)
## f : number of steps, when the method should be restarted
##
## >X=cpxsetdiag(cpxzeros(1000,1000),0,2);
## >X=cpxsetdiag(cpxsetdiag(X,-1,1),1,1);
## >b=random(1000,1);
## >x=cgX(X,b);
## >totalmax(abs(cpxmult(X,x)-b))
##  0
## 
## See: cpxfit, cg, cgXnormal
	if eps then localepsilon(eps); endif;
	n=cols(H);
	if x0==none then x=zeros(size(b));
	else; x=x0;
	endif;
	loop 1 to 10
		r=b-cpxmult(H,x); p=r; fehler=r'.r;
		loop 1 to f*n
			if sqrt(fehler)~=0 then return x; endif;
			Hp=cpxmult(H,p);
			a=fehler/(p'.Hp);
			x=x+a*p;
			rn=r-a*Hp;
			fehlerneu=rn'.rn;
			p=rn+fehlerneu/fehler*p;
			r=rn; fehler=fehlerneu;
		end;
	end;
	return x;
endfunction

function cgXnormal (H:cpx, Ht:cpx, b:real column, x0:real column=none, ..
	f:index=10, eps=none)
## Conjugate gradient method to solve Ht.H.x=Ht.b for compressed H.
##
## This algorithm is used by cpxfit() to solve the normal equation
## H'Hx=H'b, which minimizes |Hx-b|, i.e., to find an optimal solution
## of an linear system with more equations than unknowns.
##
## Stops, when the error gets smaller then eps, or after f*n
## iterations, when the error gets larger. x0 is an optional start
## vector.
##
## H : compressed matrix (nxm)
## Ht : compressed matrix (mxn)
## b : column vector (mx1)
## x0 : optional start (mx1)
## f : number of steps, when the method should be restarted
##
## See: cgX, cpxfit
	if eps then then localepsilon(eps); endif;
	n=cols(H);
	if x0==none then x=zeros(size(b));
	else; x=x0;
	endif;
	loop 1 to 10
		r=b-cpxmult(H,cpxmult(Ht,x)); p=r; fehler=r'.r;
		loop 1 to f*n
			if sqrt(fehler)~=0 then return x; endif;
			Hp=cpxmult(H,cpxmult(Ht,p));
			a=fehler/(p'.Hp);
			x=x+a*p;
			rn=r-a*Hp;
			fehlerneu=rn'.rn;
			p=rn+fehlerneu/fehler*p;
			r=rn; fehler=fehlerneu;
		end;
	end;
	return x;
endfunction

function cpxfit (H:cpx, b:real column, f:index=10, eps=none)
## Minimize |Hx-b| for a compressed matrix H.
##
## This function uses the conjugate gradient method to solve the
## normal equation H'Hx=H'b for sparse compressed matrices H.
##
## H : compressed matrix (nxm)
## b : column vector (mx1)
## f : number of steps, when the method should be restarted
##
## >X=cpxsetdiag(cpxzeros(1000,1000),0,2);
## >X=cpxsetdiag(X,1,1);
## >b=random(1000,1);
## >x=cpxfit(X,b);
## >totalmax(abs(cpxmult(X,x)-b))
##  0
## 
## See: fit, fitnormal, svdsolve
	if eps then localepsilon(eps); endif;
	Ht=cpxtranspose(H);
	return cgXnormal(Ht,H,cpxmult(Ht,b),f=f,eps=epsilon());
endfunction

function cpxsetdiag (R:cpx, k:integer scalar, d:real vector)
## Set the k-th diagonal of the compressed matrix R to the value d.
##
## k=0 is the main diagonal, k=-1 the diagonal below, and k=1 the
## diagonal above.
##
## Note that this function does not change R, but returns a new matrix
## with the changes.
##
## >function CI(n) := cpxsetdiag(cpxzeros(n,n),0,1);
## >decpx(CI(3))
##              1             0             0 
##              0             1             0 
##              0             0             1 
## 
## See: setdiag
	s=size(R);
	if k>=0 and k<s[2] then
		i=1:min(s[1],s[2]-k);
		j=k+(1:cols(i));
		if rows(d)>1 then d=d[1:cols(i)]; endif;
		return cpxset(R,i'|j'|d');
	else if k<0 && -k<s[1] then
		j=1:min(s[2],s[1]+k);
		i=-k+(1:cols(j));
		if rows(d)>1 then d=d[1:cols(i)]; endif;
		return cpxset(R,i'|j'|d');		
	else return R;
	endif;
endfunction

function cpxmultrows (c:real column, A:cpx)
## Multiply the i-th row of the compressed matrix A by c[i].
##
## Note that this function does not change R, but returns a new matrix
## with the changes.
##
## c : column vector (nx1)
## A : compressed matrix (nxm)
##
## >function CI(n) := cpxsetdiag(cpxzeros(n,n),0,1);
## >decpx(cpxmultrows((1:3)',CI(3)))
##              1             0             0 
##              0             2             0 
##              0             0             3 
## 
## See: cpxsetrow
	n=size(A)[1];
	D=cpxzeros([n,n]); D=cpxsetdiag(D,0,c');
	return cpxmult(D,A);
endfunction

function cpxsetrow (A:cpx, i:index, r:real vector)
## Set the i-th row of the compressed matrix A to r.
##
## Note that this function does not change R, but returns a new matrix
## with the changes.
##
## A : compressed matrix (nxm)
## i : index
## r : row vector (1xm)
##
## See: cpxnultrows
	s=size(A);
	j=1:s[2];
	return cpxset(A,(i|j')|r');
endfunction

function cpxsetcolumn (A:cpx, j:index, c:real column)
## Set the j-th column of the compressed matrix A to c.
##
## Note that this function does not change R, but returns a new matrix
## with the changes.
##
## A : compressed matrix (nxm)
## j : integer
## c : column vector (mx1)
##
## See: cpxsetrow
	s=size(A);
	i=1:s[1];
	return cpxset(A,(i'|j)|c);
endfunction

function cpxsum (A:cpx)
## The sums of all rows of the compressed matrix A.
##
## See: sum
	n=size(A)[2];
	return cpxmult(A,ones(n,1));
endfunction

// * Incidence Matrices

function rectangleIncidenceX (n:index, m:index)
## Incidence matrix of a rectangle grid in compact form.
##
## The incidence matrix of a graph is the matrix H, such that H(i,j)
## contains 1, if node i is connected to node j. In this function, the
## graph consists of the points of a rectangle, and the edges connect
## adjacent points. The points in the rectangle are numbered row by
## row.
##
## Returns a compressed nm x nm matrix.
##
## See: cpxset
	K=zeros(n*(m-1)+m*(n-1),3);
	k=1;
	for i=1 to n;
		for j=1 to m-1;
		K[k,1]=(i-1)*m+j; K[k,2]=(i-1)*m+j+1; K[k,3]=1;
		k=k+1;
		end;
	end;
	for i=1 to n-1;
		for j=1 to m;
		K[k,1]=(i-1)*m+j; K[k,2]=i*m+j; K[k,3]=1;
		k=k+1;
		end;
	end;
	H=cpxzeros([n*m,n*m]);
	H=cpxset(H,K);
	H=cpxset(H,K[:,[2,1,3]]);
	return H;
endfunction

// * Exact Computation

// Euler has a long accumulator, which can compute the scalar product
// and the residuum of a linear equation exactly. On this basis, Euler
// implements a residuum iteration to solve linear systems. It can be
// used to invert matrices more exactly, or to evaluate polynomials.

// See: ../Programs/14 - Exact Computation | Tutorial

// Residuum iterations uses the long accumulator of Euler to compute
// an exact scalar product.

// See: eulercore.html#Numerical_Algorithms | Core Functions for the Long Accumulator

function xlgs (A:complex, b:complex, n:integer=20, eps=none)
## Compute a more exact solution of Ax=b using residuum iteration.
##
## You can specify the relative accuracy with eps=... This epsilon is
## used to determine, if the algorithm should stop.
##
## n : the maximal number of residual iterations.
##
## >H=hilbert(10); 
## >longest totalmax(abs(xlgs(H,sum(H))-1))
##                        0 
## >longest totalmax(abs(H\sum(H)-1))
##   8.198655030211555e-005 
## 
## See: ilgs
	if eps then localepsilon(eps); endif;
	{LU,rows,c,d}=lu(A); LU=LU[rows];
	if d==0 then v=A\b;
	else v=lusolve(LU,b[rows]);
	endif;
	loop 1 to n;
		r=residuum(A,v,b);
		if d==0 then vn=v-A\r;
		else vn=v-lusolve(LU,r[rows]);
		endif;
		if all(vn~=v) then break; endif;
		v=vn;
	end;
	return vn;
endfunction

function xinv (A:complex, n:integer=20, eps=none)
## Compute the inverse of A using residuum iteration.
##
## You can specify the relative accuracy with eps=... as last
## parameter. Additionally, a maximal number of iteration n can be
## set.
##
## See: xlgs
	if eps return xlgs(A,id(cols(A)),eps=eps);
	else return xlgs(A,id(cols(A)));
	endif
endfunction

function xlusolve (A:complex, b:complex, n:integer=20, eps=none)
## Compute the solution of Ax=b for L- or U-matrices A.
##
## Works for lower triangle matrices with diagonal 1, or for upper
## triangle matrices. The function is just a faster version of xlgs.
##
## You can specify the relative accuracy with eps=... as last
## parameter. Additionally, a maximal number of iteration n can be
## set.
##
## See: xlgs, lu, lusolve
	if eps then localepsilon(eps); endif;
	v=lusolve(A,b);
	loop 1 to n;
		r=residuum(A,v,b);
		vn=v-lusolve(A,r);
		if all(vn~=v) then break; endif;
		v=vn;
	end;
	return vn;
endfunction

function xpolyval (p:complex vector, t:complex, n:integer=20, eps=none)
## Evaluate the polynomial at values t using residuum iteration.
##
## Alias to xevalpoly().
##
## See: xevalpoly
	return xevalpoly (t,p,n,eps);
endfunction

function xevalpoly (t:complex, p:complex vector, 
	n:integer=20, eps=none)
## Evaluate the polynomial at values t using residuum iteration.
##
## You can specify the relative accuracy with eps=... as last
## parameter. Additionally, a maximal number of iteration n can be
## set.
##
## >p:=[-945804881,1753426039,-1083557822,223200658]; ...
## >t:=linspace(1.61801916,1.61801917,100); ...
## >plot2d(t-1.61801916,evalpoly(t,p)):
## >plot2d(t-1.61801916,xevalpoly(t,p,eps=1e-17)):
## 
## See: evalpoly, xlgs
	if eps then localepsilon(eps); endif;
	s=t; si=size(t);
	A=id(cols(p));
	b=flipx(p)';
	loop 1 to prod(si);
		A=setdiag(A,-1,-t{#});
		v=lusolve(A,b);
		loop 1 to n;
			r=residuum(A,v,b);
			vn=v-lusolve(A,r);
			if all(vn~=v) then break; endif;
			v=vn;
		end;
		s{#}=v[cols(p)];
	end;
	return s;
endfunction

function xeigenvalue (a,l,x=none)
## Returns an improved eigenvalue of A, starting with the approximation l.
##
## l must be close to a simple eigenvalue, and x close to an
## eigenvector, if x is not 0. This is the inverse iteration due to
## Wielandt.
##
## Returns the eigenvalue and the eigenvector.
##
## >H=hilbert(10);
## >lambda=jacobi(H)[1]
##  255023613680
## >lambda=lambda+1000;
## >x=eigenspace(H,lambda);
## >norm(H.x-lambda*x)
##  7445.4368515
## >{lambda,x}=xeigenvalue(H,lambda,x);
## >norm(H.x-lambda*x)
##  3.7570410926e-005
##
## See: eigenvalues
	l1=l;
	if x==none then x=kernel(a-l1*id(cols(a))); endif;
	if all(x~=0) then error("Eigenvalue generation failed!"); endif;
	repeat;
		eps=setepsilon(0); x=(a-l1*id(cols(a)))\x; setepsilon(eps);
		x=x/sqrt(x'.x);
		l2=residuum(x',residuum(a,x,0),0);
		if l1~=l2 then return {l2,x}; endif;
		l1=l2;
	end;
endfunction

// * Interval Solvers and Guaranteed Solutions

function ieval (f$:string, x:interval scalar, n:integer=10)
## Better evaluation of the expression in f for the interval x.
##
## Evaluating an expression directly assumes that all values in all
## intervals are combined. If the expression is a function of x, this
## is not the way we want to to evaluate the expression. Then we want
## to take the same point whenever x occurs in the expression. So
## the interval is split into subintervals for more accuracy, and then
## the usual evaluation takes place in each subinterval. The results
## are combined.
##
## n : number of subintervals.
##
## >expr &= x^3-x+1/x^2; plot2d(expr,0.8,0.9):
##
## >x=~0.8,0.9~; expr()
##  ~0.84,1.5~
## >ieval(expr,~0.8,0.9~) // better
##  ~0.99,1.3~
## >ieval(&diff(expr,x),~0.8,0.9~)<0 // decreasing function
##  1
## >longest expr(~0.8~)||expr(~0.9~) // correct
##                              ~1.063,1.275~ 
##
## >function f(x) := @expr
## >ieval("f",x) // the same with functions
##  ~0.99,1.3~
##
## See: mxmieval, ievalder
	if n==1 then
		return f$(x);
	else
		ti=splitinterval(x,n);
		s=f$(ti[1]);
		if isreal(s) then s=~s,s~; endif;
		loop 2 to n
			s=s||f$(ti[#]);
		end;
		return s;
	endif;
endfunction

function ievalder (f$:string, fd$:string, xi:interval scalar, n:integer=10)
## Better evaluation of the expression in f for the interval x.
##
## The derivative is used to improve the interval accuracy. The
## interval is split into sub-intervals for more accuracy.
##
## See: ieval, mxmieval
	if n==1 then
		xm=~middle(xi)~;
		der=fd$(x=xi);
		if 0 << der then
			return f$(x=xm)+(xi-xm)*der && f$(x=xi);
		else
			return f$(x=~left(xi)~) || f$(x=~right(xi)~);
		endif;
	else
		ti=splitinterval(xi,n);
		s=ievalder(f$,fd$,ti[1],1);
		loop 2 to n
			s=s||ievalder(f$,fd$,ti[#],1);
		end;
		return s;
	endif;
endfunction

function idgl (f$:string, x:real vector, y0:interval scalar)
## Guaranteed inclusion of y'=f(t,y0;...) at points t with y(t[1])=y0. 
##
## This is a quick inclusion for a differential equation, which avoids
## the use of any Taylor series. The inclusion is very coarse,
## however. The function uses a simple Euler one step method.
##
## The result is an interval vector of values. 
##
## See: mxmidgl, idglder
	n=length(x);
	y=~zeros(1,n)~; y[1]=y0;
	loop 1 to n-1;
		m=f$(x[#],y[#],args());
		i=~x[#],x[#+1]~;
		d=diameter(i);
		repeat
			J=y[#]+m*~0,d~;
			m=expand(f$(i,J,args()),6/5);
			y2=y[#]+m*d;
			if all(y2 <<= J) then break; endif;
		end;
		y[#+1]=y2;
	end;
	return y;
endfunction

function idglder (f$:string, fx$:string, fy$:string, x:real vector, ..
	y0:interval scalar)
## Inclusion for y'=f(t,y0;...) at t with y(t[1])=y0.
##
## This function needs the partial derivatives of f to x and y.
##
## The result is an interval vector of values. The result is a very
## coarse inclusion. For better results, see mxmidgl().
##
## f, fx and fy are functions in f(x,y), or expressions of x and y.
## Additional arguments are passed to the functions.
##
## >function f(x,y) &= -x*sin(y);
## >function fx(x,y) &= diff(f(x,y),x);
## >function fy(x,y) &= diff(f(x,y),y);
## >x=0:0.01:2;
## >idglder("f","fx","fy",x,~1~)[-1]
##  ~0.137,0.158~
## >mxmidgl(&f(x,y),x,~1~)[-1]
##  ~0.14759945743769,0.14759945743821~
## 
## See: mxmidgl
	n=length(x);
	y=~zeros(1,n)~; y[1]=y0;
	loop 1 to n-1;
		i=~x[#],x[#+1]~;
		d=diameter(i);
		JT=y[#]+f$(x[#],y[#],args())*~0,d~;
		J=expand(JT,3/2);
		repeat
			J1=JT+~0,d~^2*(fx$(i,J,args())+fy$(i,J;args())*f$(i,J,args()))/2;
			if all(J1 <<= J) then break; endif;
			J=expand(J,3/2);
		end;
		y[#+1]=y[#]+f$(x[#],y[#],args())*~d~+ ..
				~0,d~^2*(fx$(i,J1,args())+fy$(i,J1,args())*f$(i,J1,args()))/2;
	end;
	return y;
endfunction

function isimpson (f$:string, der$:string, a:number, b:number, ..
	n:index=50)
## Interval Simpson integral of f from a to b.
##
## This function uses the Simpson method and its error estimate to get
## guaranteed inclusions of integrals. Other interval methods like
## igauss or mxmiint provide better estimates.
##
## f : expression (must map to arguments and work for intervals)
## der : expression for fourth derivative (like f)
## a,b : interval bounds
## n : number of subintervals
##
## >isimpson("exp(-x^2)",&diff(exp(-x^2),x,4),0,2)
##  ~0.8820813879,0.8820813936~
## >igauss("exp(-x^2)",0,2)
##  ~0.88208139037,0.88208139115~
## >mxmiint("exp(-x^2)",0,2)
##  ~0.8820813907619,0.882081390763~
## 
## See: mxmisimpson
	a=~a~; b=~b~;
	m=2*n;
	ff=4-mod(1:m+1,2)*2; ff[1]=1; ff[m+1]=1;
	i=~0:m~; t=i/m*a+(m-i)/m*b;
	simps=sum(f$(t)*ff)/3*(b-a)/m;
	t=splitinterval(a||b,n);
	error=sum(der$(t)*ones(size(t)))*((b-a)/m)^5/90;
	return simps-error
endfunction

function ilgs (A:interval, b:interval, R="", steps=100)
## Guaranteed interval inclusion for the solution of A.x=b.
##
## This function uses an interval algorithm, and an exact residuum
## calculation. If the algorithm succeeds, the result is a guaranteed
## inclusion for the solution of the linear system. Note that the
## algorithm can only work for regular A, or interval matrices not
## containing singular A.
##
## A and b may be of interval or real type.
##
## The optional R should be a pseudo inverse to A.
## 
## >H=hilbert(10); b=sum(H);
## >longest max(diameter(ilgs(H,b)'))
##   1.695865670114927e-012 
## >longest max(abs(xlgs(H,b)'-1))
##                        0 
## >longest max(abs((H\b)'-1))
##   8.198655030211555e-005 
##
## See: xlgs
	n=cols(A);
	count=1;
	if R=="" then
		A1=middle(A); R=inv(A1');
		repeat;
			M=residuum(A1',R,id(n));
			rho=max(sum(abs(M))');
			if rho<1; break; endif;
			R=R-A1'\M;
		count=count+1;
		if (count>steps); error("Could not find a pseudo invers."); endif;
		end;
		R=R';
	endif;
	M=~-residuum(R,A,id(n))~;
	rho=right(max(sum(abs(M))'));
	if (rho>=1); error("Pseudo inverse not good enough."); endif;
	f=~-residuum(R,b,0)~;
	if argn()==2 then x=middle(A)\middle(b); else x=residuum(R,b,0); endif;
	if isreal(x) then xn=residuum(M,~x,x~,f); else xn=residuum(M,x,f); endif;
	x=expand(x,max((right(max(abs(xn-x)'))/(1-rho))'));
	count=1;
	repeat;
		xn=residuum(M,x,f)&&x;
		if not all(xn<<x) then break; endif;
		if count>steps then error("Interval inclusion failed."); endif;
		x=xn;
	end;
	return x;
endfunction

function iinv (A:interval)
## Guaranteed interval inverse of the matrix A.
##
## See: inv, xinv
	return ilgs(A,id(cols(A)));
endfunction

function ievalpoly (t:interval, p:interval vector)
## Guaranteed evaluation of a polynomial p(t).
##
## p contains the coefficients of a polynomial. Euler stores
## polynomials starting with the constant coefficient.
##
## See: polyval, xpolyval
	s=t; si=size(t);
	A=id(cols(p));
	b=flipx(p)';
	loop 1 to prod(si);
		A=setdiag(A,-1,-t{#});
		if isreal(A) then v=ilgs(A,b,lusolve(A,id(cols(p))));
		else v=ilgs(A,b);
		endif;
		s{#}=v[cols(p)];
	end;
	return s;
endfunction

function ipolyval (p:interval vector, t:interval)
## Guaranteed evaluation of a polynomial p(t).
##
## See: ievalpoly
	return ievalpoly(t,p);
endfunction

function ibisect (f:string, a:scalar, b:scalar=none, y:scalar=0)
## Interval bisection algorithm to solve f(x)=y
##
## >function f(x) &= x^2+x;
## >x=ibisect("f(x)",0,4,y=2)
##   ~0.99999999999999967,1.0000000000000004~~
## >x=ibisect("f(x)",0,4,y=~1.99,2.01~)
##   ~0.9958,1.005~~
## >function f(x,a) &= x^a-a^x;
## >ibisect("f",1,3;~1.999,2.001~) // a as semicolon parameter, y=0
##   ~1.98,2.02~~
##
## See: bisect, inewton
    if b==none then b=right(a); a=left(a); endif;
	if f(~b,b~,args())<y then
		b1=a; a1=b;
		if not f(~a,a~,args())>y then 
			error("Possibly no zero in interval"); 
		endif;
	else
		a1=a; b1=b;
		if not f(~a,a~,args())<y then 
			error("Possibly no zero in interval"); 
		endif;
	endif;
	repeat
		if a1~=b1 then return  ~a1,a1~||~b1,b1~; endif;
		m=(2*a1+b1)/3;
		fy=f(~m,m~,args());
		if fy>y then 
			b1=m; 
		elseif fy<y then 
			a1=m;
		else
			m=(a1+2*b1)/3;
			fy=f(~m,m~,args());
			if fy>y then 
				b1=m; 
			elseif fy<y then 
				a1=m;
			else
				return ~a1,a1~||~b1,b1~
			endif;
		endif;	
	end;
endfunction

function ibis(f,a,b=none,y=0) ...
    if b==none then b=right(a); a=left(a); endif;
    if f(~b,b~,args())<y then
        b1=a; a1=b;
        if not f(~a,a~,args())>y then 
            error("Possibly no zero in interval"); 
        endif;
    else
        a1=a; b1=b;
        if not f(~a,a~,args())<y then 
            error("Possibly no zero in interval"); 
        endif;
    endif;
    repeat
        m=(2*a1+b1)/3;
        fy=f(~m,m~,args());
        if fy>y then 
            b1=m; 
        elseif fy<y then 
            a1=m;
        else
            m=(a1+2*b1)/3;
            fy=f(~m,m~,args());
            if fy>y then 
                b1=m; 
            elseif fy<y then 
                a1=m;
            else
                return ~a1,a1~||~b1,b1~
            endif;
        endif;
    end;
endfunction

function inewton (f$:string, df$:string , xi: interval, yi:real scalar="", y=0)
## Guaranteed interval inclusion of the zero of f.
##
## df must compute an inclusion of the derivative of f for intervals
## x. f and df must be functions of one scalar variable, or
## expressions in x. Additional parameters after the semicolon are
## passed to both functions.
##
## The initial interval x must already contain a zero. If x is a
## point, and not an interval, the function tries to get an initial
## interval with the usual Newton method.
##
## Returns {x0,f}: the solution and a flag, if the solution is
## guaranteed.
## 
## >expr &= x^2*cos(x);
## >inewton(expr,&diff(expr,x),1,y=0.1)
##  ~-0.32483576255267282,-0.32483576255267244~
## >longest solve(expr,0.3,y=0.1)
##       0.3248357625526727 
##
## See: inewton2, mxminewton, inewton2
	if !(yi=="") then x=~xi,yi~;
	else x=xi;
	endif;
	if !isinterval(x);
		x=expand(~newton(f$,df$,x,y;args())~,1000);
	endif
	valid=0;
	repeat
		m=middle(x);
		a=f$(~m,m~,args())-y;
		b=df$(x,args());
		xnew=(m-a/b);
		if !valid and xnew << x then valid=1; endif;
		xnew=xnew && x;
		if not xnew << x then return {x,valid}; endif;
		x=xnew;
	end;
endfunction

function inewton2 (f$:string, Df$:string, x:interval, check:integer=false)
## Guaranteed inclusion of the zero of f, a function of several parameters.
##
## Works like newton2, starting from a interval vector x which already
## contains a solution. If x is no interval, the function tries to
## find such an interval.
##
## f and Df must be a function of a row vector x, or an expression in
## x. f must return a row vector, and Df the Jacobi matrix of f.
##
## Returns {x,valid}.
##
## If check is false, the result is not checked for a guaranteed
## inclusion. In this case the return value of valid can be checked
## to learn, if the inclusion is a guaranteed inclusion. If checked
## is true valid=0 will throw an error exception.
##
## See: newton2, inewton 
	if !isinterval(x); 
		x=expand(~newton2(f$,Df$,x;args())~,1000);
	endif;
	valid=0;
	y=middle(x)';
// Compute an approximate inverse of Df(x)
	R=inv(Df$(y',args()));
	yi=x';
	repeat
// Compute xm-R.f(xm)+(R.Df(x)-I).(x-xm)
// where xm is the midpoint of x
		y=middle(yi);
		M=~residuum(R,Df$(yi',args()),id(cols(x)))~;
		yn=(y-R.f$(~y,y~',args())'+M.(yi-y));
		if !valid and all(yn<<yi) then valid=1; endif;
		if any(!intersects(yn,yi)) then 
			if check then
					error("Nonempty intersection!");
			endif;
			return {yn,0}; 
		endif;
		yn=yn&&yi;
		if not all(yn<<yi) then
			if check and not valid then
					error("No guaranteed inclusion found!");
			endif;
			return {yn',valid}; 
		endif;
		yi=yn;
	end;
endfunction

function plotintervals (r, style=none)
## Adds plots of two dimensional intervals to a given plot.
##
## r is an nx2 vector of intervals containing the x values in the
## first row, and the y values in the second row. Each value is
## plotted as a bar.
##
## plot2d() can also plot (x,y), where y is a vector of intervals. But
## there the values of y are assumed to be the result of a function
## with errors.
##
## >expr &= x^3-x;
## >plot2d(expr,r=1.1);
## >x=(-1:0.1:1)' + ~-0.02,0.02~;
## >plotintervals(x|expr(x),style="O"):
## 
## See: mxmibisectfxy
	hh=holding(1);
	if style!=none then st=barstyle(style); endif;
	rh=r';
	x=left(rh[1]); y=left(rh[2]);
	w=diameter(rh[1]); h=diameter(rh[2]);
	plotbar(x,y,w,h);
	if style!=none then barstyle(st); endif;
	holding(hh);
endfunction

function arnoldi (A, q ,m)
## Compute an orthonormal basis of the Krylov space
##
## This function computes a matrix Q with orthonormal
## columns which are a basis of the Krylov matrix
##
## b, Ab, A^2.b, ..., A^m.b
##
## A : real nxk matrix
## q : real nx1 vector
## m : integer
##
## Result Q : real nx(m+1) matrix
##
## n=6; A=normal(n,n); q=normal(n,1); ...
## Q = arnoldi(A,q,4)
## Q'.Q // -> id(5)
## Q'.A.Q // -> Hessenberg matrix
##
	Q=q/norm(q);
	for k=2 to m+1;
		w = A.Q[,k-1];
		w = w - Q.Q'.w; // Gram-Schmidt
		if norm(w)~=0 then return Q ; endif;
		Q = Q|(w/norm(w));
	end;
	return Q;
endfunction
