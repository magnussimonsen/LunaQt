// Linear Programming 

comment
Simplex and integer Simplex routines.
endcomment

// * Simplex Algorithms

// The Simplex algorithm is a core function of Euler. It can handle
// inequalities and equalities, restricted and unrestricted variables.
// There is a utility function to simplify the handling and the error
// checking.

// Euler contains the optimization library LPSOLVE, which can be used
// alternatively. E.g., the function ilpsolve() is more efficient than
// the simple branch and bound method intsimplex() for integer programs.

function overwrite simplex (A:real, b:real column, c:real vector,
	eq=-1, restrict=1, max:integer=0, min:integer=1, check:integer=1)
## Minimize c.x with respect to A.x<=b and x>=0 or other restrictions.
##
## Simplex method for maximization or minimization of a linear
## function with linear constrains, and restricted or unrestricted
## variables. The function calls the built-in Simplex algorithm.
##
## Minimizes or maximizes c.x with the conditions A.x <= b, or A.x >=
## b, or A.x == b, and optionally x>=0. The type of the condition is
## contained in a column vector eq. It is in each row -1 for <=, 1 for
## >=, 0 for equality. The column vector restrict determines, which
## variables should be restricted to be non-negative (1), or
## non-positive (-1), or unrestricted (0). If max=1, the function
## maximizes the target function.
##
## A : real matrix (nxm)
## b : real column vector (nx1)
## c : real row vector (1xm)
## eq : real column vector with entries -1, 0, or 1 (nx1)
## restrict : real row vector with entries 0, 1, or -1 (1xm)
## max,min : flags to maximize and minimize (set either of them)
## check: If true, an error will occur for unbounded or not feasible
##   problems. If false, you need to check the r flag for the result.
##
## The return value is {x,r,i}, where x is the solution, and r has the
## following meaning:
##
##   r=0 : success, 
##   r=-1 : no feasible point, 
##   r=1 : the problem is not bounded.
##
## i is a column vector, which is 1 for each active side condition.
##
## It must be remarked, that an equality condition generates two
## conditions. So i might be longer than the original number of
## conditions.
##
## >A=[1,1;4,5]; b=[1000;4500]; // x+y<=1000, 4x+5y<=4500
## >eq=[-1;-1]; // both equations <= (the default)
## >c=[5,6]; // 4x+5y -> Max.
## >restr=[1,1]; // x,y>=0 (the default)
## >simplex(A,b,c,eq,restr,>max,>check) // check for errors
##                  500 
##                  500 
##
## This function calls the built-in simplex() algorithm, which always
## minimizes and always returns a result flag.
##
## See: simplex, intsimplex
	opt=-1;
	if max then opt=1; endif;
	if not min then opt=1; endif;
	{x,r,i}=_simplex(A,b,c,eq,restrict,opt);
	if check then
		if r==1 then error("Unbounded problem!"); return x;
		elseif r==-1 then error("No feasible point!"); return x;
		endif;
	endif
	return {x,r,i'};
endfunction

function feasibleArea (A:real, b:real column, eq=-1, restrict=1)
## Computes the corners of the set with A.x<=b
##
## A can have two columns only. Returns a 2xn matrix with rows x and
## y. These vectors can be used to plot the feasible area with the
## polt2d function and the parameter polygon=1.
##
## >A=[1,1;4,5]; b=[1000;4500]; // x+y<=1000, 4x+5y<=4500
## >eq=[-1;-1]; // both equations <= (the default)
## >c=[5,6]; // 4x+5y -> Max.
## >restr=[1,1]; // x,y>=0 (the default)
## >X=feasibleArea(A,b,eq,restr); // determine polygon
## >plot2d(X[1],X[2],>filled,style="/");
## >x=simplex(A,b,c,eq,restr,>max,>check);
## >plot2d(x[1],x[2],>points,>add);
##
	if cols(A)<>2 then error("Only in two variables!"); endif;
	t=linspace(0,2pi,100);
	x=simplex(A,b,[cos(t[1]),sin(t[1])],eq,restrict);
	loop 2 to cols(t)
		xn=simplex(A,b,[cos(t[#]),sin(t[#])],eq,restrict);
		if any(!(x[:,-1]~=xn)) then x=x|xn; endif;
	end
	return x;
endfunction

function %intsimplex (A,b,c,eq,re,i,%alpha)
	{x,r}=_simplex(A,b,c,eq,re,-1);
	n=cols(A);
	a=c.x;
	if r==0;
		// no hope for a better integer solution:
		if a>%alpha then return {x,-2,a}; endif;
		// look for an non-integer index i:
		loop 1 to cols(i);
			if !(x[i[#]]~=round(x[i[#]],0)) then
				// branch and bound
				ei=((1:n)==i[#]);
				{x1,r1,a1}=%intsimplex(A_ei,b_floor(x[i[#]]),c,eq_-1,re,i,%alpha);
				if r1==0 and a1<%alpha then %alpha=a1; endif;
				{x2,r2,a2}=%intsimplex(A_-ei,b_-ceil(x[i[#]]),c,eq_-1,re,i,%alpha);
				if r2==0 and a2<%alpha then %alpha=a2; endif;
				if r1==0 then
					if r2<>0 or a1<a2 then return {x1,r1,a1}; endif;
				endif;
				return {x2,r2,a2};
			endif;
		end;
		// no non-integer value
		if a<%alpha then %alpha=a; endif;
	endif;
	return {x,r,a}
endfunction

function intsimplex (A:real, b:real column, c:real vector,
	eq=-1, restrict=1, max:integer=0, min:integer=1, i="all",
	check=false, ire:integer vector=none)
## Minimize c.x=b under the conditions A.x<=b, x integer, x>=0.
##
## This is the branch and bound algorithm for integer linear problems.
## It is implemented in the Euler language. There is a faster and more
## sophisticated algorithm ilpsolve().
##
## Minimizes or maximizes c.x with the conditions A.x <= b, or A.x >=
## b, or A.x == b, x integer. and optionally x>=0. The parameter i is a
## row vector of indices of variables, which should be positive. If
## i="all", all variables should be integer. This is the default.
##
## A : real matrix (nxm)
## b : real column vector (nx1)
## c : real row vector (1xm)
## eq : real column vector with entries -1, 0, or 1 (nx1)
## restrict : vector with non-negative restricted variables.
## max,min : flags to maximize and minimize 
## i : real row vector with indices of integer variables 
## ire : Alternative to i. Row vector with 0,1.
##
## Note that i and restrict work in different ways. This is due to the
## compatibility with ilpsolve().
##
## check : If true, the function will throw an error, if the problem
## is unbounded, or has no feasible point.
##
## The function returns {x,r,a}, where x is the solution of the problem
## if r=0. a is the optimal value.
##
## >A=random(20,3)+1; b=ones(20,1)*1000; // random problem
## >c=ones(1,3);
## >intsimplex(A,b,c,>max,>check)
##                  193 
##                  348 
##                   41 
##
## See: simplex, ilpsolve
	if i=="all" then in=ones(size(c));
	else 
		in=zeros(size(c)); 
		if cols(i)>0 then in[i]:=1; endif;
	endif;
	if ire!=none then
		in=ire;
	endif;
	alpha=1e30;
	if max then c=-c;
	elseif not min then c=-c;
	endif;
	if typeof(eq)==0 then eq=eq*ones(rows(A),1); endif;
	{x,r}=%intsimplex(A,b,c,eq,restrict,nonzeros(in),alpha);
	if check then
		if r==0 then return x;
		elseif r==1 then error("Unbounded problem!"); return x;
		elseif r==-1 then error("No feasible point!"); return x;
		endif;
		return x;
	endif
	return {x,r};
endfunction

// For the LPSOLVE package has been ported by Peter Notebaert for
// Euler. The full documentation is available on 

// See: http://lpsolve.sourceforge.net/5.5/Euler.htm | LPSolve on Sourceforge

defaultilpsolvedll:=0;

function startlpsolve ()
## Start the LPSOLVE DLL.
  global defaultilpsolvedll;
  if !defaultilpsolvedll then 
  	dll("eulpsolve.dll", "eulpsolve", -1);
  	defaultilpsolvedll=1;
  endif;
endfunction

function ilpsolve (A, b, c, eq = -1,
	vlb = [], vub = [], i = "all", 
	scalemode = 0, keep = 0, max = 0, ire = none)
## Minimize f.x subject a.x<=b, vlb<=x<=vub, and integer x.
##
## This routine is contained in the lpsolve DLL, which is loaded, if
## you load the lpsolve package with "load lpsolve". The routines have
## been ported to Euler by Peter Notebaert. 
##
## The function solves the linear integer problem
##
##             max v = f'*x
##               a*x <> b
##                 vlb <= x <= vub
##                 x[i] are integer
##
##  Arguments: The first four arguments are required.
##
##           f: n vector of coefficients for a linear objective function.
##           a: m by n matrix representing linear constraints.
##           b: m vector of right sides for the inequality constraints.
##           e: m vector that determines the sense of the inequalities:
##                     e(i) = -1  ==> Less Than
##                     e(i) =  0  ==> Equals
##                     e(i) =  1  ==> Greater Than
##         vlb: n vector of lower bounds. If empty or omitted,
##              then the lower bounds are set to zero.
##         vub: n vector of upper bounds. May be omitted or empty.
##        xint: vector of integer variables. May be omitted or empty.
##   scalemode: scale flag. Off when 0 or omitted.
##        keep: Flag for keeping the lp problem after it's been solved.
##              If omitted, the lp will be deleted when solved.
##
##  Output: The function returns {x,obj,duals}
##
##           x: Optimal value of the decision variables.
##         obj: Optimal value of the objective function.
##       duals: solution of the dual problem.
##
##  Loads the DLL once, when it is called.
##
## >A=random(50,5)+1; b=ones(50,1)*1000; // random problem
## >c=ones(1,5);
## >load lpsolve;
## >ilpsolve(A,b,c,i=[],>max,>check) // no integer problem
##        61.7789496937 
##         156.76464272 
##        76.0735082666 
##        60.3330148438 
##        232.179101403 
## >ilpsolve(A,b,c,>max,>check)
##                   62 
##                  157 
##                   76 
##                   60 
##
## See: intsimplex, lpsolve
	startlpsolve();

	m = rows(A);
	n = cols(A);
	if !max then c:=-c; endif;
	if i=="all" then xint=1:length(c); else xint=i; endif;
	if ire!=none then xint=nonzeros(ire); endif;
	if typeof(eq)==0 then eq=eq*ones(rows(A),1); endif;
	
	lp = eulpsolve("make_lp", m, n);
	eulpsolve("set_verbose", lp, 3);
	eulpsolve("set_mat", lp, A);
	eulpsolve("set_rh_vec", lp, b);
	eulpsolve("set_obj_fn", lp, c);
	eulpsolve("set_maxim", lp); // default is solving minimum lp.

	for j = 1 to length(eq)
		if eq[j] < 0 then
					contype = 1;
		elseif eq[j] == 0 then
					contype = 3;
		else
					contype = 2;
		endif
		eulpsolve("set_constr_type", lp, j, contype);
	end

	for j = 1 to length(vlb)
		eulpsolve("set_lowbo", lp, j, vlb[j]);
	end

	for j = 1 to length(vub)
		eulpsolve("set_upbo", lp, j, vub[j]);
	end

	for j = 1 to length(xint)
		eulpsolve("set_int", lp, xint[j], 1);
	end

	if scalemode <> 0 then
		eulpsolve("set_scaling", lp, scalemode);
	endif

	result=eulpsolve("solve", lp);
	if result == 0 || result == 1 || result == 11 || result == 12 then
		{obj, x, duals} = eulpsolve("get_solution", lp);
		stat = result;
	else
		obj = [];
		x = [];
		duals = [];
		stat = result;
	endif
	
	if !max then obj=-obj; endif;

	if keep == 0 then
		eulpsolve("delete_lp", lp);
	endif

	return {x,obj,duals,stat}
endfunction

// * Newton-Barrier Method

// This method minimizes f(x) under conditions g_i(x)<=0 using a
// global minimizer for f(x)-c*sum(log(-g_i(x)),i), and letting c to
// 0. The method for general convex f need the gradient and the
// Hessian for a good global minimizer.

function map %nbeval (lambda;f$,x,v,cnb,A,b)
    diff = b-A.(x+lambda*v)';
    if all(diff>0) then 
    	return f$(x+lambda*v)-cnb*sum(log(diff)');
    else 
    	return 1e20;
    endif;
endfunction

function newtonbarrier (f$:string, df$:string, Hf$:string,
	A:real, b:real column,
	x:real vector, lambda:positive real=1, 
	c:positive real=0.1, cfactor:positive real=0.5,
	history=false, eps=epsilon())
## Newton-Barrier method starting from interior x.
##
## This function can minimize a convex function function f(x) subject
## to conditions Ax <= b. It needs the gradient df and the Hessian Hf
## of f as functions. The start point x must be an interior point with
## Ax < b. 
##
## Note: f must be convex.
##
## f,df,Hf : Functions or call collections taking a row vector as a
##   parameter.
##
## A,b : Conditions Ax<=b
## x : Start point with Ax<b
##
## lambda,c,cfactor : Parameters for the algorithms. lambda is the
##   first step size for a minimization into a direction of descent. c
##   is the starting value in the Newton-Barrier function, and cfactor
##   is the factor at which c is reduced in each step.
##
## history : If true, the function returns a matrix with one step x
##   in each column. Else it returns the last x only.
##
## eps : The absolute accuracy of the solution. The algorithm returns,
##   when |x-xnew|<epsilon.
	localepsilon(eps);
	if history then X=x; endif;
	fold=%nbeval(0,f$,x,x,c,A,b);
	repeat // refinement of the barrier
		f1=fold;
		repeat // Newton Algorithm
			h=b-A.x';
			until any(h~=0);
			d=1/(b-A.x');
			H=A'.(d^2*A);
			v=-fit(Hf$(x)+c*H,df$(x)'+c*sum((A*d)'))';
			lnew=fmin({{"%nbeval",f$,x,v,c,A,b}},0,d=lambda);
			xnew=x+lnew*v; lambda=lnew;
			fnew=%nbeval(0,f$,xnew,xnew,c,A,b);
			until sum(abs(x-xnew))<c;
			x=xnew; fold=fnew;
		end;
		if history then X=X_x; endif;
		c=c*cfactor;
		until c~=0;
	end;
	if history then return X;
	else return x;
	endif;
endfunction
