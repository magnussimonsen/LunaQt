// Mathematical Functions

comment
Special Functions.
endcomment

// * Core Functions

// Many basic mathematical functions (operators, trigonometric
// functions, exponential functions etc.) are listed in the
// documentation of the core.

// See: eulercore.html#Operators_and_Functions | Core Functions

// This file contains special functions like hyperbolic
// functions, elliptic functions, beta functions etc. It does also
// contain some polynomial functions.

// * Hyperbolic Functions

function sinh (x:numerical)
## Computes (exp(x)-exp(-x))/2
	h=exp(x);
	return (h-1/h)/2;
endfunction

function cosh (x:numerical)
## Computes (exp(x)+exp(-x))/2
	h=exp(x);
	return (h+1/h)/2;
endfunction

function asinh (z:numerical)
## Computes log(z+sqrt(z^2+1))
	return log(z+sqrt(z*z+1))
endfunction

function acosh (z:numerical)
## Computes log(z+(z^2-1))
	return log(z+sqrt(z*z-1))
endfunction

function map sinc (z:numerical)
## Computes sin(x)/x
##
## Takes care of x~=0, returning 1 in this case.
## For intervals containing 0, the function fails.
	if z~=0 then return 1;
	else return sin(z)/z; 
	endif;
endfunction

function sec (x:numerical)
## Computes 1/cos(x)
	return 1/cos(x)
endfunction

function cosec (x:numerical)
## Computes 1/sin(x)
	return 1/sin(x)
endfunction

function cot (x:numerical)
## Computes 1/tan(x)
	return 1/tan(x)
endfunction

// * Polar Coordinates

function args polar
## polar(x,y), polar(x,y,z) compute polar coordinates.
##
## This function computes the polar coordinates in two or three
## dimensions.
##
## Returns {phi,r} or {phi,psi,r}
##
## >{phi,r}=polar(1,2); degprint(phi), r,
##  63°26'5.82''
##  2.2360679775
##
## See: rect
	if argn==2 then return {atan2(arg1,arg2),sqrt(arg1*arg1+arg2*arg2)}; 
		elseif argn==3 then
		return {atan2(arg1,arg2),atan2(sqrt(arg1*arg1+arg2*arg2),arg3), ..
			sqrt(arg1*arg1+arg2*arg2+arg3*arg3)}
	else error("Illegal arguments for topolar");
	endif
endfunction

function args rect
## rect(phi,r), rect(phi,psi,r) compute rectangle coordinates.
##
## Computes rectangle coordinates in two or three dimensions.
##
## >{x,y}=rect(45°,sqrt(2)); [x,y]
##  [ 1  1 ]
##
## Returns {x,y} or {x,y,z}
##
## See: polar
	if argn==2 then return {arg2*cos(arg1),arg2*sin(arg1)};
	elseif argn==3 then return {arg3*cos(arg1)*cos(arg2), ..
		arg3*sin(arg1)*cos(arg2),arg3*sin(arg2)};
	else error("Illegal argumtenst for rect");
	endif;
endfunction

// * Logarithms

function logbase (x:numerical, a:numerical)
## Computes the logarithm to base a
	return log(x)/log(a);
endfunction

function log10 (x:numerical)
## Computes the logarithm to base 10
	return log(x)/log(10)
endfunction

// * Polynomials

// There are many built-in functions for polynomials, like polyval,
// polyadd etc.

// See: eulercore.html#Polynomials | Core Functions for Polynomials

function overwrite polysolve (p:vector, 
	usematrix=0, check=1)
## All roots of the polynomial p.
##
## Uses the Baurhuber method to find all zeros using the built-in
## function polysolve(o) by default.
##
## If usematrix=1 and p is not complex, the companion matrix is used
## and the algorithm from AlgLib and LAPACK. This is slower, but more
## accurate for huge degrees.
	if usematrix and !iscomplex(p) then
		p=polytrunc(p); n=cols(p)-1; p=p/p[n+1];
		K=(0_id(n-1))|-p[1:n]';
		return eigenvalues(K,<usecharpoly,check=check);
	else
		return _polysolve(p);
	endif;
endfunction

function polydif (p:vector)
## Returns the polynomial p'.
##
## Polynomials are stored in Euler starting with the constant
## coefficient.
##
## >polydif([0,0,1])
##  [ 0  2 ]
	n=cols(p);
	if (n==1); return 0; endif;
	return p[2:n]*(1:n-1);
endfunction

// * Chebychev Polynomials

function map cheb (x:complex, n:nonnegative integer)
## Computes the Chebyshev polynomial T_n(x).
##
## The functions used T(x,n) = cos(n*acos(x)), if possible. For x<-1
## and x>1, it uses T(x,n) = (w+1/w)/2, where w is the n-th power of
## the inverse Joukowski transform.
##
## >x=-1.1:0.01:1.1; y=x'; z=x+I*y;
## >plot2d(abs(cheb(z,5)),niveau=exp(0:10))
##
## See: chebpoly, chebrek
	if iscomplex(x) then
		return cos(n*acos(x));
	elseif x>1 then
		w=(x+sqrt(x^2-1))^n;
		return (w+1/w)/2;
	elseif x<-1 then
		signum=-mod(n,2)*2+1;
		w=(-x+sqrt(x^2-1))^n;
		return signum*(w+1/w)/2;
	else
		return cos(n*acos(x));
	endif;
endfunction

function chebrek (x:vector, n:nonnegative integer)
## Computes the Chebyshev polynomial via the recursion formula.
##
## >chebrek(-1:0.1:1,10)
##  [ 1  -0.2007474688  0.9884965888  -0.0998400512  -0.9884965888  -0.5
##  0.5623462912  0.9955225088  0.4284556288  -0.5388927488  -1
##  -0.5388927488  0.4284556288  0.9955225088  0.5623462912  -0.5
##  -0.9884965888  -0.0998400512  0.9884965888  -0.2007474688  1 ]
##
## Works for vector x and scalar n.
	if (n==0); return ones(size(x)); endif;
	if (n==1); return x; endif;
	z=ones(size(x))_x;
	loop 3 to n+1;
		z=z_(2*x*z[#-1]-z[#-2]);
	end;
	return z[n+1];
endfunction

function chebpoly (n:nonnegative integer)
## Computes the coefficients of the n-th Chebyshev polynomial.
##
## >p=chebpoly(10)
##  [ -1  0  50  0  -400  0  1120  0  -1280  0  512 ]
## >plot2d("polyval(p,x)",-1,1);
##
## >x=-1.1:0.01:1.1; y=x'; z=x+I*y;
## >p=chebpoly(10); w=abs(polyval(p,z));
## >plot2d(log(w+0.2),niveau=0:10,>hue,>spectral)
##
## The function uses a loop.
	t0=[1]; t1=[0,1];
	if (n==0); return t0; endif;
	if (n==1); return t1; endif;
	loop 2 to n
		t=(0|2*t1[1:#])-(t0|[0,0]);
		t0=t1; t1=t;
	end;
	return t;
endfunction

function chebfit (xp:vector, yp:vector, n:nonnegative integer, ..
	a:real scalar=-1, b:real scalar=1)
## Fits Chebyshev polynomials on [a,b] to (xp,yp).
##
## Returns the vector of coefficients p(x) = sum a_[i] T(x,i)
##
## See: chebval
	return fit(cheb((xp'-(a+b)/2)/((b-a)/2),0:n),yp')';
endfunction

function map chebval (x:number; alpha:vector, ..
	a:real scalar=-1, b:real scalar=1)
## Evaluates sum a[i]*T(x,i).
##
## See: chebfit
	return sum(alpha*cheb((x-(a+b)/2)/((b-a)/2),0:cols(alpha)-1));
endfunction

// * Beta, Gamma and Elliptical Functions

// Note that all these functions have the main argument x as first
// argument, and the order arguments as second arguments. In general,
// each of these functions has many different versions depending on
// the software or the web site.

function comment gamma (x)
## Gamma function at real or complex x
##
## See: gammai, gammaln
endfunction

function comment gammaln (x)
## Logarithm of the Gamma function for real x
endfunction

function comment gammai (x,a)
## Incomplete normalized Gamma function of order a.
##
## The result is normalized, so that the total integral over x is 1.
## It is also the lower integral, so that it is a distribution (see
## gammaic() for the upper integral). To get the ordinary incomplete
## gamma function multiply by gamma(a). To get the upper gamma
## function compute (1-gamma(x,a))*gamma(a).
##
## Note that the order is the second argument in Euler.
##
## Maxima has incomplete gamma functions with reversed arguments, and
## it is using the upper integral.
##
## >gammai(3,4)
##  0.352768111218
## >&float(1-gamma_incomplete_regularized(4,3))
##                             0.35276811121777
##
## See: gamma, gammaic
endfunction

function comment gammaic (x,a)
## Upper incomplete, regularized gamma function of order a
##
## See: gammai
endfunction

function comment beta (a,b)
## Beta function B(a,b).
endfunction

function comment betai (x:real, a:real, b:real)
## Computes the normalized incomplete beta function.
endfunction

function comment invbetai (x:real, a:real, b:real)
## Computes the inverse of the normalized incomplete beta function.
endfunction

function map ellrf (x:real nonnegative scalar, ..
	y:real nonnegative scalar, z:real nonnegative scalar, eps=none)
## Carlson's elliptic integral of the first kind RF (x; y; z). 
##
## The iteration is implemented in the Euler language.
##
## x, y, and z must be non-negative, and at most one can be zero.
## Computes
##
##  1/2 integrate(1/sqrt((t+x)*(t+y)*(t+z)),t=0..inf)
##
## See: ellrd, ellf
	if eps then localepsilon(eps); endif;
	repeat
		mu=(x+y+z)/3;
		xn=1-x/mu; yn=1-y/mu; zn=1-z/mu;
		if max(max(xn,yn),zn)~=0 then break endif;
		lambda=sqrt(x*y)+sqrt(x*z)+sqrt(y*z);
		x=(x+lambda)/4; y=(y+lambda)/4; z=(z+lambda)/4;
	end;
	return 1/sqrt(mu);
	e2=xn*yn-zn^2;
	e3=xn*yn*zn;
	return (1.0+(e2/24-0.1-3*e3/44)*e2+e3/14)/sqrt(mu);
endfunction

function map ellf (phi: real scalar, k: real scalar)
## Elliptic integral of the first kind F(phi,k)
	s=sin(phi);
	k=sqrt(k);
	return s*ellrf(cos(phi)^2,(1.0-s*k)*(1.0+s*k),1.0);
endfunction

function map ellrd (x:real nonnegative scalar,
	y:real nonnegative scalar, z:real nonnegative scalar,
	eps=none)
## Carlson's elliptic integral of the second kind RD(x; y; z). 
##
## x and y must be non-negative, and at most one can be zero. z must
## be positive. Computes
##
##  3/2 integrate(1/(t+z)*sqrt((t+x)*(t+y)*(t+z))),t=0..inf)
##
##
## See: ellrf, elle
	if eps then localepsilon(eps); endif;
	sum=0.0;
	fac=1.0;
	repeat
		mu=0.2*(x+y+3.0*z);
		xn=(mu-x)/mu; yn=(mu-y)/mu; zn=(mu-z)/mu;
		if max(max(abs(xn),abs(yn)),abs(zn))~=0 then break endif;
		lambda=sqrt(x*y)+sqrt(x*z)+sqrt(y*z);
		sum=sum+fac/(sqrt(z)*(z+lambda));
		fac=0.25*fac;
		x=0.25*(x+lambda);
		y=0.25*(y+lambda);
		z=0.25*(z+lambda);
	end;
	ea=xn*yn;	
	eb=zn*zn;
	ec=ea-eb;
	ed=ea-6.0*eb;
	ee=ed+ec+ec;
	C1=(3.0/14.0);
	C2=(1.0/6.0);
	C3=(9.0/22.0);
	C4=(3.0/26.0);
	C5=(0.25*C3);
	C6=(1.5*C4);
	return 3.0*sum+fac*(1.0+ed*(-C1+C5*ed-C6*zn*ee) ..
		+zn*(C2*ee+zn*(-C3*ec+zn*C4*ea)))/(mu*sqrt(mu));
endfunction

function map elle (phi: real scalar, k: real scalar)
## Elliptic integral of the second kind E(phi,k).
##
## See: ellrd
	s=sin(phi);
	cc=cos(phi)^2;
	k=sqrt(k);
	q=(1.0-s*k)*(1.0+s*k);
	return s*(ellrf(cc,q,1.0)-((s*k)^2)*ellrd(cc,q,1.0)/3.0);
endfunction

// * Bessel Functions

// Note that the following functions have the order as the second
// argument, and the x-value as the first argument.
//
// Bessel functions are canonical solutions of the cylinder function.
// They are used to compute many other useful functions.

function comment besselj (x,a)
## BesselJ function of the first kind at x with order a
##
## See: bessely, besseli, besselk
endfunction

function comment bessely (x,a)
## Bessel function of the second kind at x with order a
##
## See: besselj, besseli, besselk
endfunction

function comment besseli (x,a)
## Modified Bessel function of the first kind at x with order a
##
## See: besselj, bessely, besselk
endfunction

function comment besselk (x,a)
## BesselJ function of the second kind at x with order a
##
## See: besselj, bessely, besseli
endfunction

function comment besselallr (x,a)
## BesselJ and BesselY and their derivatives
##
## See: besselj, besselmodallr
endfunction

function comment besselmodallr (x,a)
## Modified BesselJ and BesselY and their derivatives
##
## See: besselj, besselallr
endfunction

// * Integer Arithmetic with Euler

// Euler uses double arithmetic for this. If you need infinite integer
// arithmetic, use symbolic expressions or Python.

function map gcd (a:integer, b:integer)
## Computes the greatest common divisor of a and b.
##
## >factor(gcd(11^4*123*1237,11^2*123^2))
##  [3,  11,  11,  41]
## 
## See: lcm, gcdext
	if a>b;
		u=a; v=b;
	else
		u=b; v=a;
	endif;
	repeat;
		if v==0; break; endif;
		r=mod(u,v);
		u=v;
		v=r;
	end;
	return u;
endfunction

function lcm (a,b)
## Returns least common multiple of a and b.
##
## See: gcd
	return a*b/gcd(a,b);
endfunction;

function gcdext (x:integer, y:integer)
## Return {g,a,b} such that ax+by=g.
##
## >p=23423429; isprime(p)
##  1
## >x=3423; {g,a,b}=gcdext(x,p); g, a*x+b*p,
##  4831125
## >mod(a*x,p)
##  1
##
## See: gcd
	if x<y then {g,a,b}=gcdext(y,x); return {g,b,a}; endif;
	if y~=1 then return {1,0,1}; endif;
	m=mod(x,y);
	if m~=0 then return {y,0,1}; endif;
	{g,aa,bb}=gcdext(y,m);
	return {g,bb,aa-bb*floor(x/y)};
endfunction

function map invmod (x:integer, m:integer)
## Return y such that x*y is 1 modulo m.
##
## >p=23423429; invmod(3423,p)
	{g,a,b}=gcdext(x,m);
	if !(g~=1); error("no inverse!"); endif;
	if a<0; return m+a; endif;
	return a;
endfunction

function primes (n:integer)
## Return all primes up to n using the Sieve of Eratosthenes.
##
## >length(primes(1000000))
##  78498
##
	if n>=3
		len = floor((n-1)/2);
		sieve = ones ([1,len]);
		for i=1 to (sqrt(n)-1)/2;
			if (sieve[i]) then
				sieve[3*i+1:2*i+1:len] = 0; .. do it
			endif
		end
		return [2, 1+2*nonzeros(sieve)];
	elseif n>=2
		return 2;
	else
		return [];
	endif
endfunction

function map isprime (n:integer)
## Returns true if n is a prime number, false otherwise.
##
## >nonzeros(isprime(1:100))
##  [2,  3,  5,  7,  11,  13,  17,  19,  23,  29,  31,  37,  41,  43,  47,
##  53,  59,  61,  67,  71,  73,  79,  83,  89,  97]
## >primes(100)
##  [2,  3,  5,  7,  11,  13,  17,  19,  23,  29,  31,  37,  41,  43,  47,
##  53,  59,  61,  67,  71,  73,  79,  83,  89,  97]
## >sum(isprime(1000000:1100000))
##  7216
##
## See: factor
	if n < 2 then
		return 0;
	elseif n==2 or n == 3 or n == 5 or n== 7then
		return 1;
	else
		if mod(n,2)==0 then return 0; endif;
		return !any(mod(n,3:2:sqrt(n))==0);
	endif
endfunction

function factor (n:integer, p=none)
## Returns prime factorization of n. 
##
## >fraction factor(344234237)
##  [41,  8395957]
##
## See: isprime
	if n < 4
		return n;
	else
		if p==none then p = primes(floor(sqrt(n))+1); endif;
		q = n/p;
		idx = nonzeros (q == ceil(q));
		if cols(idx)==0 then
			return [n];
		else
			r = p[idx];
			q = factor(n/prod(r),p);
			if all(q != 1) then
				return (sort([r, q]));
			else
				return r;
			endif
		endif
	endif
endfunction
