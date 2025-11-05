// Statistics with Euler Math Toolbox

// EMT contains many statistical distributions, tests, plots, and
// functions for reading and writing data. For examples and more
// functions read the following introduction notebook.

// See: ../Programs/12 - Statistics

comment
Statistical functions.
endcomment

// * Random Variables

// Euler has a reliable random number creator. It can be used to
// create random variables for many distributions. If you need a fixed
// sequence, you can set a seed value with seed(x). Otherwise, the
// time value (in seconds) at the start of the current Euler session
// will be used.

// The newer functions for creating random variables start with
// rand...(), replacing older functions with less uniform naming.

function comment seed (x)
## Set the seed for the random numbers
##
## After setting a seed, all random numbers will be determined from
## the seed.
endfunction

function comment random (n,m)
## Uniformly distributed random variables in [0,1]
##
## random() : One random variable
## random(n,m) : Matrix of random variables
## random(n) : Row vector of random variables
## random([n,m]) : Matrix of random variables
##
## The function fastrandom is a quicker, but less reliable,
## alternative.
##
## See: intrandom, normal
endfunction

function randuniform (n : index, m : index, a : number, b : number)
## Random samples uniformly of the interval (a,b)
##
## See: random
    return a + random(n,m)*(b - a)
endfunction

function comment intrandom (n,m,k)
## Integer random variables in {1,...,k}
##
## intrandom(k) : One random variable
## intrandom(n,m,k) : Matrix of random variables
## intrandom(n,k) : Row vector of random variables
## intrandom([n,m],k) : Matrix of random variables
##
## See: random
endfunction

function randint (n,m,k=none)
## Integer random variables in {1,...,k}
##
## randint(n,m,k) : Matrix of random variables
## randint(n,k) : Vector of random variables
	if k==none then return intrandom(n,m)
	else return intrandom([n,m],k);
	endif;
endfunction

function comment normal (n,m)
## 0-1-Gaussian distributed random variables
##
## normal() : One random variable
## normal(n,m) : Matrix of random variables
## normal(n) : Row vector of random variables
## normal([n,m]) : Matrix of random variables
##
## The function fastnormal() is a quicker, but less reliable,
## alternative.
endfunction

function randnormal (n : index, m : index, 
	mean : number = 0, stdev : nonnegative number = 1)
## Random samples from a normal (Gaussian) distribution
##
## See: normal
	return normal(n,m)*stdev+mean;
endfunction

// The following distributions are based on Julia code by John D. Cook.

function randmatrix (n:index, m:index=none, f$:string)
## Apply the random generator f$ to generate a matrix.
	if m==none then m=n; n=1; endif;
	M=zeros(n,m);
	loop 1 to prod(size(M)); 
		M{#} = f$(args());
	end;
	return M
endfunction

function %randexponential (mean : positive number)
## Random matrix from an exponential distribution
    return -mean*log(random())
endfunction

function randexponential (n : index, m : index=none, 
	mean : positive number = 1)
## Random matrix from an exponential distribution
##
## randexponential(n,m) : mean=1
## randexponential(n,m,mean) : nxm matrix
## randexponential(n,mean=v) : vector with mean=v
##
## See: randnormal, randuniform
    return randmatrix(n,m,"%randexponential";mean);
endfunction

function %randgamma (shape : nonnegative number, scale : nonnegative number)
## Random samples from a gamma distribution
##
## Implementation based on "A Simple Method for Generating Gamma Variables"
## by George Marsaglia and Wai Wan Tsang.
## ACM Transactions on Mathematical Software
## Vol 26, No 3, September 2000, pages 363-372.
    if shape >= 1.0 then
        d = shape - 1.0/3.0;
        c = 1.0/sqrt(9.0*d);
        repeat
            x = normal();
            v = 1.0 + c*x;
            repeat
	            while v<=0
                x = normal();
                v = 1+c*x;
            end
            v = v*v*v;
            u = random();
            xsq = x*x;
            if u<1-0.0331*xsq*xsq or log(u)<0.5*xsq+d*(1-v+log(v)) then
                return scale*d*v
            endif
        end
    else
        g = %randgamma(shape+1.0, 1.0);
        w = random();
        return scale*g*w^(1.0/shape)
    endif
endfunction

function randgamma (n : index, m : index = none,
	shape : nonnegative number=1, scale : nonnegative number=1)
## Random samples from a gamma distribution
##
## Implementation based on "A Simple Method for Generating Gamma Variables"
## by George Marsaglia and Wai Wan Tsang.
## ACM Transactions on Mathematical Software
## Vol 26, No 3, September 2000, pages 363-372.
##
## Example:
##
## >k=10; theta=2;
## >x=randgamma(10000,shape=k,scale=theta);
## >plot2d(x,>distribution); ...
## >plot2d("x^(k-1)*exp(-x/theta)/theta^k/gamma(k)", ...
## >   >add,color=blue,thickness=2):
    return randmatrix(n,m,"%randgamma";shape,scale);
endfunction

function randchi (n : index, m : index, dof : index)
## Random samples from a chi square distribution
    return randgamma(n,m,0.5*dof,2)
endfunction

function randinversegamma (n : index, m : index, 
	shape : positive number, scale : positive number)
## return a random matrix from an inverse gamma random variable
    return 1/randgamma(n,m,shape,1/scale)
endfunction

function randweibull (n : index, m : index,
	shape : positive number, scale : positive number)
## Random samples from a Weibull distribution
    return scale * (-log(random(n,m)))^(1/shape)
endfunction

function randcauchy (n : index, m : index,
	mean : number=0, scale : positive number=1)
## Random samples from a Cauchy distribution
	p=random(n,m);
    return mean + scale*tan(pi*(p - 0.5));
endfunction

function randt (n : index, m : index,
	dof : positive integer)
## Random samples from a Student-t distribution
##
## See Seminumerical Algorithms by Knuth
	y1=normal(n,m);
	y2=randchi(n,m,dof);
	return y1 / sqrt(y2/dof);
endfunction

function %randlaplace (mean : number, scale : positive number)
## Random samples from a Laplace distribution
## The Laplace distribution is also known as the double exponential distribution.
    u = random();
    if u < 0.5 then
        return mean + scale*log(2.0*u);
    else
        return mean - scale*log(2*(1-u));
    endif
endfunction

function randlaplace (n : index, m : index,
	mean : number, scale : positive number)
## Random samples from a Laplace distribution
## The Laplace distribution is also known as the double exponential distribution.
    return randmatrix(n,m,"%randlaplace";mean,scale);
endfunction

function randlognormal (n : index, m : index,
	mu : number, sigma : positive number)
## Random samples from a log-normal distribution
    return exp(randnormal(n,m,mu,sigma));
endfunction

function randbeta (n : index, m : index,
	a: positive number, b : positive number)
## Random samples from a Beta distribution
##
## There are more efficient methods for generating beta samples.
## However such methods are little more efficient and much more complicated.
    u = randgamma(n, m, a, 1.0);
    v = randgamma(n, m, b, 1.0);
    return u / (u + v);
endfunction

// * Statistical Distributions

// Euler has a lot of routines to generate random numbers (named
// "rand..."), like the built-in functions random and normal. Moreover,
// Euler has functions for distributions ("...dis") and their
// densities ("q..."). For examples, see the following introduction
// notebook.

// See: ../Programs/12 - Statistics

// This file provides more distributions, random numbers, and tests.

function comment bindis (k:natural, n:natural, p:number)
## Cumulative binomial distribution 
##
## Binomial distribution for i<=k out of n with probability p.
##
## From AlgLib.
endfunction

function map binsum (k:natural, n:natural, p:number)
## Binomial sum for getting k<=i out of n runs with probability p.
##
## Uses an actual summation to compute the binomial sum. binsum() is
## faster.
##
## See: bindis, normalsum, invbinsum
	if k>=n; return 1; endif;
	d=sqrt(n*p*(1-p));
	i1=n*p-10*d;
	if i1<0; i1=0; endif;
	if k<i1; return 0; endif;
	if k>n*p+10*d; return 1; endif;
	s=n*log(1-p);
	if k>=1 then
		j=1:k+1;
		s=cumsum(s|(log((n-j+1)/j*p/(1-p))));
	endif;
	return sum(exp(s[max(1,floor(i1)):k+1]));
endfunction

function map invbindis (px:number, n:natural, p:number)
## Inverse cumulative binomial distribution
##
## Finds k such that the probability of i<=k out of n is just more
## than px. The result may not be integer. Then k=floor(result). A
## binary intersection method is used.
##
## >bindis(4,10,0.6), invbindis(%,10,0.6)
##   0.1662386176
##   4
	return intbisect("bindis",0,n,px;n,p);
endfunction

function comment bincdis (k,n,p)
## Complementary cumulative binomial distribution
##
## Inverse of the binomial distribution for i<=k out of n with
## probability p.
##
## From AlgLib.
endfunction

function comment invpbindis (k,n,px)
## Inverse (for p) cumulative binomial distribution
##
## Solves px=bindis(k,n,p) for p. Assumes integer k and n.
##
## From AlgLib.
##
## See: clopperpearson
endfunction

function overwrite normaldis (x : real, mean : real = 0, dev : real = 1)
## Cumulative normal distribution
##
## This function calls the built-in _normaldis(x) with adjusted mean
## and standard deviation.
	return _normaldis((x-mean)/dev);
endfunction

function overwrite invnormaldis (p : real, mean : real = 0, dev : real = 1)
## Inverse of cumulative normal distribution
##
## This function calls the built-in _invnormaldis(x) and adjusts the
## mean and the standard deviation.
	return _invnormaldis(p)*dev+mean;
endfunction

function comment erf (x)
## Gauss error function
##
## This is the integral of exp(-t^2)/sqrt(pi) from -x to x (from
## AlgLib). It is connected to normaldis() via
## 2*normaldis(sqrt(2)*x)-1=erf(x). 
endfunction

function comment erfc (x)
## Complementary Gauss error function
##
## 1-erf(x)
endfunction

function normalsum (i:natural, n:natural, p:number)
## Probability of getting i or less hits in n runs.
##
## Works like binsum, but is much faster for large n and medium p.
##
## See: binsum
	return normaldis((i+0.5-n*p)/sqrt(n*p*(1-p)));
endfunction

function map hypergeomsum (i:natural, n:natural, itot:natural, ntot:natural)
## Hypergemotric sum.
##
## This is the probability to get i or less hits, if n are picked
## randomly in an urn containing ntot objects, with itot good objects.
##
## i : we want i or less hits in n picked objects
## n : number of randomly picked objects
## itot : total number of positive objects
## ntot : total number of objects
##
## Examples:
## >1-hypergeomsum(7,13,13,52) // 8 or more spaces in Bridge
##   0.00126372228099
## >columnsplot(hypergeomsum(0:20,20,20,40),lab=0:20):
## >hypergeomsum(4,20,20,40), 1-hypergeomsum(15,20,20,40)
##   0.000179983683393
##   0.000179983683393
	if i<itot-(ntot-n); return 0; endif;
	if i>=itot; return 1; endif;
	if itot<=ntot-n;
		s=logbin(ntot-n,itot)-logbin(ntot,itot);
		if i>=1;
			j=1:i;
			s=cumsum(s|(log(n-j+1)-log(j)+log((itot-j+1))-log((ntot-n-itot+j))));
		endif;
	else
		s=log(bin(n,itot-(ntot-n)))-log(bin(ntot,itot));
		if i>itot-(ntot-n);
			j=(itot-(ntot-n)+1):i;
			s=cumsum(s|(log(n-j+1)-log(j)+log(itot-j+1)-log(ntot-n-itot+j)));
		endif;
	endif
	return sum(exp(s));
endfunction

function qnormal (x, m=0, d=1)
## Density (DPF) of the m-d-normal distribution
##
## This is the density function the Gauss normal distribution with
## mean m and standard deviation 1.
##
## See: normaldis, erf
	return 1/(d*sqrt(2*pi))*exp(-(x-m)^2/2/d^2);
endfunction

function map gammarestr (x)
## Special Gamma function, works only for 2x natural
##
## See: gamma
	if floor(x)~=x; return fac(x-1); endif;
	return prod(0.5:1:x-1)*sqrt(pi());
endfunction

function qchidis (x, n)
## Density (DPF) of the chi-squared distribution
	return 1/(2^(n/2)*gammarestr(n/2))*x^(n/2-1)*exp(-x/2);
endfunction

function comment chidis (x,n)
## Chi-squared distribution with n degrees of freedom
##
## Algorithm from AlgLib.
endfunction

function comment chicdis (x,n)
## Complementary chi-squared distribution with n degrees of freedom
##
## Algorithm from AlgLib.
##
## See: chidis, invchidis, invchicdis
endfunction

function invchidis (x, n)
## Inverse of of the chi-squared distribution
##
## See: invchicdis
##
## See: chidis
	return invchicdis(1-x,n);
endfunction

function comment invchicdis (x, n)
## Inverse of of the complentary chi-squared distribution
##
## Algrithm from AlgLib.
##
## See: chicdis
endfunction

function qtdis (t:real, n:nonnegative integer)
## Density (DPF) of the student t distribution
##
## See: tdis
	return gammarestr((n+1)/2)/(sqrt(pi()*n)* ..
			gammarestr(n/2))*(1+t*t/n)^(-(n+1)/2);
endfunction

function comment tdis (x:real, n:natural)
## Student T distribution with n degrees of freedom
##
## Algrithm from AlgLib.
##
## See: invtdis
endfunction

function comment invtdis (x:nonnegative, n:natural)
## Inverse Student T distributio with n degrees of freedom
##
## Algrithm from AlgLib.
##
## See: tdis
endfunction

function qfdis (x, n, m)
## Denisity (DPF) of the F-distribution
	return n^(n/2)*m^(m/2)*gammarestr((n+m)/2)/ ..
			(gammarestr(n/2)*gammarestr(m/2))* ..
			x^(n/2-1)*(m+n*x)^(-(n+m)/2);
endfunction

function overwrite map fdis (x, a, b)
## F distribution
##
## Vectorizes the built-in function _fdis(x,a,b).
	return _fdis(x,a,b);
endfunction

function overwrite map fcdis (x, a, b)
## F distribution
##
## Vectorizes the built-in function _fcdis(x,a,b).
	return _fcdis(x,a,b);
endfunction

function overwrite  map invfcdis (x, a, b)
## Complementary F distribution 
##
## Vectorizes the built-in function _invfcdis(x,a,b)
	return _invfcdis(x,a,b);
endfunction

function map invfdis (x, a, b)
## Inverse of of the F distribution
##
## See: invcfdis
	return _invfcdis(1-x,a,b);
endfunction

// * Descriptive Statistical Functions

function meandev (x:numerical, v=none)
## Mean value and statistical standard deviation of [x1,...,xn]
##
## An optional additional parameter v contains the multiplicities of
## x. m=mean(x) will assign the mean value only! If x is a matrix the
## function works on each row.
##
## x : data (1xm or nxm)
## v : multiplicities (1xn or nxm)
##
## See: mean, dev
	if v<>none then
		n=sum(v);
		m=sum(x*v)/n;
		if n>1 then d=sqrt(sum(v*(x-m)^2)/(n-1)); else d=0; endif;
	else
		n=cols(x);
		m=sum(x)/n;
		if n>1 then d=sqrt(sum((x-m)^2)/(n-1)); else d=0; endif;
	endif;
	return {m,d};
endfunction

function mean (x:numerical, v:real vector=none)
## Mean value of x.
##
## An optional additional parameter contains multiplicities.
##
## See: meandev, median
	{m,d}=meandev(x,v); return m;
endfunction

function dev (x:numerical, v:real vector=none)
## Experimental standard deviation of x
##
## An additional parameter may contain multiplicities.
##
## See: meandev
	{m,d}=meandev(x,v); return d;
endfunction

function median (x, v=none, p:real vector=0.5)
## Quantile such that p of the x[i] are less equal.
##
## v are optional multiplicities for the values. If x is a matrix, the
## function works on all rows of x.
##
## x : data (1xm or nxm)
## v : multiplicities (1xm or nxm)
## p : desired percentage (real or row vector)
##
## See: mean, quartiles, quantile
	m=rows(x);
	if m>1 then
		y=zeros(m,1);
		loop 1 to m;
			y[#]=median(x[#],v,p);
		end;
		return y;
	elseif cols(p)>1 then
		m=cols(p);
		y=zeros(1,m);
		loop 1 to m;
			y[#]=median(x,v,p[#]);
		end;
		return y;
	else
		if v<>none then
			{xs,i}=sort(x); vsh=v[i];
			n=cols(xs);
			ns=sum(vsh);
			i=1+p*(ns-1); i0=floor(i);
			vs=cumsum(vsh);
			loop 1 to n
				if vs[#]>i0 then 
					return xs[#]; 
				elseif vs[#]+1>i0 then
					k=#+1; 
					repeat; 
						if vsh[k]>0 or k>n then break; endif; 
						k=k+1;
					end;
					return (1-(i-i0))*xs[#]+(i-i0)*xs[k]+0; 
				endif;
			end;
			return xs[n];
		else
			xs=sort(x); 
			n=cols(x);
			i=1+p*(n-1); i0=floor(i);
			if i0==n then return xs[n]; endif;
			return (i-i0)*xs[i+1]+(1-(i-i0))*xs[i];
		endif;
	endif;
endfunction

function pfold (v: real vector, w: real vector)
## Distribution of the sum of two distributions
##
## v[i], w[i] contain the probabilities that each random variable is
## equal to i-1. result[i] contains the probability that the sum of
## the two random variables is i-1.
##
## See: fold, fftfold
	z=zeros(1,length(w)-1);
	return fold(z|flipx(v)|z,w);
endfunction

function comment quantile (v:vector,p:real)
## Compute the p-quantile of the elements in v
##
## Function from AlgLib. This functions takes care of multiplicities
## of the two values closest to the quantile. For the lower, upper or
## middle quantile, use the median function.
##
## >quantile([1,2],20%)
##  1.2
## >quantile([1,2,2],20%)
##  1.4
endfunction

function covar (x:real vector, y:real vector)
## Empirical covariance of x and y
##
## The covariance is the scalar product of x and y after
## centralization (x-mean(x),y-mean(y)) divided by the n-1, where n is
## the length of x and y.
##
## See: covarmatrix
	return sum((x-mean(x))*(y-mean(y)))/(cols(x)-1);
endfunction

function covarmatrix (x:real)
## Empirical covariance matrix of the rows of x
##
## The covariance matrix contains the empirical covariances of the rows
## of x, i.e., the scalar products of the centralized rows divided by
## the number columns of x minus 1.
##
## See: covar
	x=x-mean(x);
	return (x.x')/(cols(x)-1);
endfunction

function sphering (X)
## Sphering of the matrix X.
##
## The matrix X contains samples of random variables in its rows. The
## sphering of X is a linear transformation Y=T.(X-m), such that the
## rows of B have mean 0 and an identity correlation matrix.
##
## Returns {Y,T,m)
##
## See: covarmatrix
	m=mean(X); X=X-m;
	S=X.X'/(cols(X)-1); {O,d,V}=svd(S);
	T=(O*(1/sqrt(d))).V';
	return {T.X,T,m};
endfunction

function correl (x:real vector, y:real vector)
## Correlation of x and y
##
## The correlation is the salar product of the centralized and
## normalized vectors x and y.
	x=x-mean(x); x=x/sqrt(sum(x^2));
	y=y-mean(y); y=y/sqrt(sum(y^2));
	return x.y';
endfunction

function correlmatrix (x:real)
## Correlation matrix of the rows of x
##
## See: correl, covar
	x=x-mean(x);
	x=x/sqrt(sum(x^2));
	return x.x';
endfunction

function ranks (x)
## Ranks of the elements of x in x.
##
## This is the number i of the item x[i] in the vector x. With
## multiplicities, the rank is the mean rank of the equal elements.
##
## Works for reals, real vectors, or string vectors x.
##
## See: rankcorrel
	{x,i}=sort(x);
	r=indexofsorted(x,x)-(multofsorted(x,x)-1)/2;
	{i,i2}=sort(i);
	return r[i2];
endfunction

function rankcorrel (x:real vector, y:real vector)
## Correlation of x and y
##
## See: ranks
	x=ranks(x); x=x-mean(x); x=x/sqrt(sum(x^2));
	y=ranks(y); y=y-mean(y); y=y/sqrt(sum(y^2));
	return x.y';
endfunction

function empdist (x:real vector, vsorted:real vector)
## Empirical distribution
##
## The vector vsorted contains empirical data. Then we compute the
## empirical cumulative distribution (CPF) of the data at the points
## x[i].
##
## x : vector of values, usually sorted
## vsorted : sorted(!) vector of empirical values.
##
## >short empdist(1:6,sort(intrandom(1,6000,6)))
##  [ 0.16283  0.33083  0.49317  0.662  0.832  1 ]
	return find(vsorted,x)/length(vsorted);
endfunction

function randpint (n:index, m:index, p:vector)
## nxm random numbers with probabilities in p
##
## Generates nxm random numbers from 1 to k based on the vector of
## probabilities p[1],...,p[k].
##
## See: reandmultinomial
	return find(0|cumsum(p),random(n,m));
endfunction

function randmultinomial (n:index, m:index, p: vector)
## n mulitnomial random numbers based on a density
##
## This generates n outcomes of m throws with probabilities
## p[1],...,p[k]. The result is a nxk matrix
##
## See: randpint, chitest
	v=randpint(n,m,p);
	k=cols(p);
	w=zeros(n,k);
	loop 1 to n;
		w[#] = getmultiplicities(1:k,v[#]);
	end;
	return w;
endfunction

// * Statistical Tests

function chitest (x:real vector, y:positive vector, 
	montecarlo=false, nmontecarlo=1000, p=false)
## Perform a chi^2 test, if x has the expected frequency y
##
## This function tests an observed frequency x against an expected
## frequency y. E.g., if 40 men are found sampling 100 persons, then
## [40,60] has to be tested against [50,50]. The result of the test is
## too small, which means that the sample does not obey the expected
## frequency with an error less than 5%.
##
## For a meaningful test, sum(x) should be equal to sum(y), unless
## p=true. In this case, y is interpreted as a vector of probabilities
## not a vector of events.
##
## To get frequencies of data from the data, use "getfrequencies",
## "count", or "histo".
##
## montecarlo : If montecarlo is not zero, the method uses a
## Monte Carlo simulation. It generates nmontecarlo random events of
## sum(x) data with the distribution in y, and counts how often the
## statistics sum((x-y)^2/y) is larger than the observed statistics.
##
## x,y : two real row vectors (1xn)
##
## Returns the error level for rejecting the hypothesis that the
## observed frequency x has the expected frequency y.
##
## >load statistics;
## >x=[100,90]; y=[0.5,0.5]*sum(x); chitest(x,y)
##  0.468159909854
## >chitest(x,y,>montecarlo)
##  0.43
## >chitest(x,[0.5,0.5],>p)
##  0.468159909854
##
## See: getfrequencies, count, histo
	if p then y=y*sum(x); endif;
	if montecarlo then
		if montecarlo==1 then montecarlo=nmontecarlo; endif;
		X=randmultinomial(montecarlo,sum(x),y/sum(y));
		s=sum((X-y)^2/y)';
		return sum(s>sum((x-y)^2/y))/montecarlo;
	else
		return 1-chidis(sum((x-y)^2/y),cols(x)-1);
	endif;
endfunction

function testnormal (r:real vector, n:integer, v:real vector, ..
	m:number, d:number)
## Test an observed frequency for normal distribution.
##
## Test the number of data v[i] in the ranges r[i],r[i+1] against the
## normal distribution with mean m and deviation d, using the chi^2
## method.
##
## r : ranges (sorted 1xm vector)
## n : total number of data
## v : number of data in the ranges (1x(m-1) vector)
## m : expected mean value
## d : expected deviation
##
## Return the error we get, if we reject the normal distribution.
	t=normaldis((r-m)/d);
	k=cols(v); p=(t[2:k+1]-t[1:k])*n;
	return chitest(v,p);
endfunction

function ttest (m:number, d:real scalar, n:natural, mu:number)
## T student test
##
## Test, if the measured mean m with measured deviation d of n data
## comes from a distribution with mean value mu. 
##
## m : mean value of data
## d : standard deviation of data
## n : number of data
## mu : mean value to test for
##
## Returns the error alpha, if we reject that the data come from a
## distribution with mean mu.
	return 1-tdis(abs(m-mu)/d*sqrt(n),n-1);
endfunction

function tcompare (m1:number, d1:number, n1:natural, ..
	m2:number, d2:number, n2:natural)
## Test, if two measured data agree in mean.
##
## The data must be normally distributed. Returns the error you make,
## if you reject that both data are from the same normal distribution.
##
## m1,m2 : means of the data
## d1,d2 : standard deviation of the data
## n1,n2 : number of data
##
## Returns the error alpha, if we reject that the data come from a
## distribution with the same expected mean.
	h1=d1^2/n1; h2=d2^2/n2;
	return 1-tdis(abs(m1-m2)/sqrt(h1+h2), ..
		ceil((h1+h2)^2/(h1^2/(n1+1)+h2^2/(n2+1))-2));
endfunction

function tcomparedata (x:real vector, y:real vector)
## Compare x and y for same mean
##
## Calls "tcompare" to compare the two observations for the same mean.
##
## Returns the error we make, if we reject that both data come from a
## distribution with the same expected mean.
##
## See: tcompare
	return tcompare(mean(x),dev(x),cols(x),mean(y),dev(y),cols(y));
endfunction

function tabletest (A:real)
## Chi^2-Test the results a[i,j] for independence of the rows from the columns.
##
## The table test test for indepence of the rows of the tables
## from the column. E.g., if some items are observed [40,50] times
## for men, and [50,30] times for woman, we can ask, if the
## observations depend on the gender. In this case we can reject
## independece with 1.8% error level.
##
## This test should only be used for large table entries.
##
## Return the error you make, if you reject independence.
	c=sum(A); r=sum(A')';
	E=c*r/sum(r);
	return 1-chidis(totalsum((A-E)^2/E),(cols(A)-1)*(rows(A)-1));
endfunction

function expectedtable (A:real)
	c=sum(A); r=sum(A')'; t=totalsum(A);
	return (c*r)/t;
endfunction

function contingency (A:real, correct=1)
## Contigency Coefficent of a matrix A.
##
## If the coefficient is close to 0, we tend to say that the rows and
## the colums are independent.
##
## correct : Correct the coefficient, so that it is between 0 and 1
	n=cols(A); m=rows(A);
	B=expectedtable(A);
	chi=totalsum((A-B)^2/B);
	k=sqrt(chi/(chi+totalsum(A)));
	if !correct return k; endif;
	return k/sqrt(min(n-1,m-1)/min(n,m));
endfunction

function varanalysis
## varanalysis(x1,x2,x3,...) test for same mean.
##
## Test the data sets for the same mean, assuming normal distributed
## data sets. This is also known as one of the ANOVA tests.
##
## Returns the error we make, if we reject same mean.
##
## Example:
## >seed(0.5); v=normal(1,10)+1; w=normal(1,12)+2; u=normal(1,5);
## >varanalysis(v,w,u)
##   0.000556414242764 // reject same mean!
	md=[mean(arg1),cols(arg1)];
	s=sum((arg1-md[1])^2);
	loop 2 to argn();
		x=args(#); mx=mean(x);
		md=md_[mx,cols(x)];
		s=s+sum((x-mx)^2);
	end;
	md=md'; n=sum(md[2]); mt=sum(md[1]*md[2])/n;
	s1=sum(md[2]*(md[1]-mt)^2);
	return 1-fdis((s1/(argn()-1))/(s/(n-argn())),argn()-1,n-argn());
endfunction

function mediantest (a:real vector, b:real vector)
## Median test for equal mean.
##
## Test the two distributions a and b on equal mean value. For this,
## both distributions are checked on exceeding the median of the
## cumulative distribution.
##
## Returns the error we make, if we reject that a and b can have the
## same mean.
	c=sort(a|b); n=cols(c);
	if mod(n,2)==0; m=(c[n/2]+c[n/2+1])/2;
	else m=c[n/2+1];
	endif;
	return 1-hypergeomsum(sum(a<m)-1,cols(a),floor(n/2),n);
endfunction

function ranktest (a:real vector, b:real vector, eps=epsilon())
## Mann-Whitney test tests a and b on same distribution
##
## Return the error we make, if we reject the same distribution.
	n1=cols(a); n2=cols(b); n=n1+n2;
	{c,i}=sort(a|b); 
	R=ranks(c);
	R1=sum(R*(i<=n1)); R2=sum(R*(i>n1));
	return 1-normaldis(abs((min(R1-n1*(n1+1)/2,R2-n2*(n2+1)/2)-n1*n2/2)/ ..
		sqrt(n1*n2*(n1+n2+1)/12)));
endfunction

function signtest (a:real vector, b:real vector)
## Test, if the expected mean of a is not better than b
##
## Assume a(i) and b(i) are results of a treatment. Then we can ask,
## if a is better than b.
##
## a,b : row vectors of same size
##
## Return the error we make, if we decide that a is better
## than b.
	n=cols(a); i=sum(a>b);
	return 1-binsum(i-1,n,0.5);
endfunction

function wilcoxon (a:real vector, b:real vector, eps=sqrt(epsilon()))
## Test, if the expected mean of a is not better than b
##
## This is a sharper test for the same problem as in "signtest".
##
## Returns the error you make, if you decide that a is better
## than b.
##
## See: signtest
	d=a-b; n=cols(d);
	{c,i}=sort(abs(d)); R=1:n;
	{c1,i1}=sort(c+eps*(n:-1:1));
	R=(R+R[i1])/2;
	R1=sum(R*(d[i]<0)); R2=sum(R*(d[i]>=0));
	W=R2;
	return 1-normaldis((W-n*(n+1)/4)/ ..
		sqrt(n*(n+1)*(2*n+1)/24));
endfunction

// * Statistical Plots

function quartiles (x, outliers=1.5)
## Quartiles for each row of x.
##
## This computes [Min,Q1,M,Q2,Max], where M is the median, Q1
## the median of the lower half and Q2 the median of the upper half.
##
## outliers : If none, Min and Max are the minimal and maximal values
## of the data. Otherwise, Min is the least data value, which is not
## smaller than Q1-outliers*range, where range=Q2-Q1. Similar for
## Max.
##
## See: boxplot, 
	v=zeros(rows(x),5);
	m=cols(x);
	if m<3 then error("Need at least 3 data for quartiles."); endif;
	if mod(m,2)==0 then k1=m/2; k2=k1+1; 
	else k1=floor(m/2); k2=k1+2; endif;
	for i=1 to rows(x);
		w=sort(x[i,:]);
		v[i,3]=quantile(w,0.5);
		v[i,2]=quantile(w[1:k1],50%);
		v[i,4]=quantile(w[k2:m],50%);
		if outliers!=none then
			d=(v[i,4]-v[i,2])*outliers;
			v[i,1]=w[min(nonzeros(w>=v[i,2]-d))];
			v[i,5]=w[max(nonzeros(w<=v[i,4]+d))];
		else
			v[i,1]=w[1];
			v[i,5]=w[m];
		endif;
	end
	return v;
endfunction

function boxplot (data:real, lab=none, style="0#",
	textcolor=none, outliers=1.5, pointstyle="o",
	range=none)
## Summary of the quartiles in graphical form.
##
## data : vector or matrix. In case of a matrix, the rows are used.
## style : If present, it is used as fill style, the default is "O#"
## lab : Labels for each row of the data (vector of strings)
## textcolor : Color of the labels (vector of colors)
## outliers : Factor for the maximal whisker length or none
## pointstyle : Point style for outliers
## range : 1x2 vector for the plot range (or none)
##
## >x=normal(1000)*10+1000; boxplot(x):
## >x=randnormal(5,1000,100,10); boxplot(x,outliers=none):
##
## See: quartiles, barstyle
	x=quartiles(data,outliers);
	m1=totalmin(data); m2=totalmax(data);
	m=(m1+m2)/2; d=(m2-m1)/2*1.05;
	if !holding() then clg; endif;
	h=holding(1);
	if range!=none then
		setplot(0.5,rows(x)+0.5,range[1],range[2]);
	else
		setplot(0.5,rows(x)+0.5,m-d,m+d);
	endif
	frame();
	if lab==none then
		xgrid(1:rows(x),grid=4);
	else
		w=window();
		ht=textheight();
		loop 1 to length(lab);
			ct=textcolor(textcolor{#});
			{col,row}=toscreen(#,0);
			_ctext(""+lab[#],[col,w[4]+0.4*ht]);
			textcolor(ct);
		end;	
	endif;
	if range!=none then
		ygrid(ticks(range[1],range[2]));
	else
		ygrid(ticks(m1,m2));
	endif;
	loop 1 to rows(x);
		m1=x[#,1]; m2=x[#,5];
		st=style("--"); plot([#,#],[m1,m2]); style(st);
		st=style("-"); 
		plot([#-0.2,#+0.2],[m1,m1]); 
		plot([#-0.2,#+0.2],[m2,m2]); 
		style(st);
		st=barstyle(style);
		plotbar(#-0.4,x[#,2],0.8,x[#,4]-x[#,2]);
		barstyle(st);
		m=x[#,3];
		st=style("-");
		plot([#-0.4,#+0.4],[m,m]);
		style(st);
		if outliers!=none then
			k=nonzeros((data[#,]<x[#,1])||(data[#,]>x[#,5]));
			st=markerstyle(pointstyle);
			if cols(k)>0 then
				mark(#,data[#,k]);
			endif;
			markerstyle(st);
		endif;
	end
	holding(h);
endfunction

function columnsplot (x:vector, lab=none,
	style="O#", color=green, textcolor=none, 
	width=0.4, frame=true, grid=true)
## Plot the elements of x as columns.
##
## x : vector of values
## lab : a string vector with one label for each element of x.
## style,color : fill style and color for the bars
## textcolor : color for the labels
## 
## See: style, color, plot2d
	if textcolor==none then textcolor=defaulttextcolor; endif;
	m1=min(min(x),0); m2=max(max(x),0);
	m=(m1+m2)/2; d=(m2-m1)/2*1.05;
	if !holding() then clg; endif;
	h=holding(1);
	setplot(0.5,length(x)+0.5,m-d,m+d);
	if frame then frame(); endif;
	if lab!=none then
		w=window();
		ht=textheight();
		loop 1 to length(lab);
			ct=textcolor(textcolor{#});
			{col,row}=toscreen(#,0);
			_ctext(""+lab[#],[col,w[4]+0.4*ht]);
			textcolor(ct);
		end;
	else
		xgrid(1:length(x),grid=4);
	endif;
	if grid then ygrid(ticks(m1,m2)); endif;
	loop 1 to cols(x);
		cl=barcolor(color{#});
		st=barstyle(style);
		if x[#]>=0 then
			plotbar(#-width,0,2*width,x[#]);
		else
			plotbar(#-width,x[#],2*width,-x[#]);
		endif;
		barstyle(st);
		barcolor(cl);
	end;
	holding(h);
endfunction

function dataplot (x:real, y:real, style="[]w", color=1)
## Plot the data (x,y) with point and line plots.
##
## x : real row vector
## y : real row vector or matrix (one row for each data).
## style : a style or a vector of styles
## color : a color or a vector of colors
##
## You can use a vector of styles and a vector of colors. These
## vectors must contain as many elements as there are rows of y.
##
## See: statplot
	if !holding() then clg; endif;
	h=holding(1);
	setplot(totalmin(x),totalmax(x),totalmin(y),totalmax(y));
	xplot();
	loop 1 to rows(y);
		cl=_color(color{#});
		plot(x,y[#,:]);
		_color(cl);
	end;
	loop 1 to rows(y);
		cl=_color(color{#});
		ms=markerstyle(style{#});
		mark(x,y[#,:]);
		markerstyle(ms);
		_color(cl);
	end;
	holding(h);
endfunction

function piechart (x:real vector, style="0#", 
	color=green, lab:string vector=none, r=1.5, textcolor=red)
## plot the data x in a pie chart.
##
## x : the vector of data
## color : a color or a vector of colors (same length as x)
## style : a style or a vector of styles
## lab : a vector of labels (same length as x)
## r : The piechart has radius 1. To leave space use r=1.5.
	if !holding() then clg; endif;
	w=window(); window(0,0,1024,1024);
	h=holding(1);
	setplot(-r,r,-r,r);
	tot=sum(x);
	phi=0|cumsum(x/sum(x))*2*pi;
	loop 1 to cols(x);
		t=linspace(phi[#],phi[#+1],100);
		xs=0|sin(t); ys=0|cos(t);
		cl=barcolor(color{#}); st=barstyle(style{#});
		polygon(xs,ys,1);
		barcolor(cl); barstyle(st);
		if lab!=none then
			tc=textcolor(textcolor{#});
			ts=(phi[#]+phi[#+1])/2;
			xts=sin(ts)*(1+r)/2; yts=cos(ts)*(1+r)/2;
			{col,row}=toscreen(xts,yts);
			_ctext(""+lab[#],[col,row-textheight()/2]);
			textcolor(tc);
		endif;
	end;
	holding(h);	
	window(w);
endfunction

function starplot (v, style="/", color=green, lab:string=none, 
	rays:integer=0, pstyle="[]w", textcolor=red, r=1.5)
## A star like plot with a filled star or with rays and dots only
	if !holding() then clg; endif;
	w=window(); window(0,0,1024,1024);
	h=holding(1);
	vr=max(abs(v))*r;
	setplot(-vr,vr,-vr,vr);
	n=cols(v); t=linspace(0,2pi,n);
	v=v|v[1]; c=v*cos(t); s=v*sin(t);
	loop 1 to n
		if rays then
			cp=color(color{#});
			plot([0,c[#]],[0,s[#]]);
			ms=markerstyle(pstyle);
			mark(c[#],s[#]);
			markerstyle(ms);
			color(cp);
		else
			cl=barcolor(color{#}); st=barstyle(style{#});
			polygon([0,c[#],c[#+1]],[0,s[#],s[#+1]],1);
			barcolor(cl); barstyle(st);
		endif;
		if lab!=none then
			rlab=v[#]+vr*(r-1)/2;
			{col,row}=toscreen(cos(t[#])*rlab,sin(t[#])*rlab);
			tc=textcolor(textcolor{#});
			ctext(""+lab[#],col,row-textheight()/2);
			textcolor(tc);
		endif;
	end;
	holding(h);
	window(w);
endfunction

function logimpulseplot (x, y=none, style="O#", color=green, d=0.1)
## Logarithmic impulse plot of y.
	if y==none then y=x; x=1:cols(y); endif;
	if !holding() then clg; endif;
	h=holding(1);
	frame();
	yl=log(y)/log(10);
	setplot(min(x)-d,max(x)+d,min(min(yl),0),max(max(yl),0));
	bs=barstyle(style);
	loop 1 to cols(x);
		bc=barcolor(color{#});
		plotbar(x[#]-d,0,2d,yl[#]);
		barcolor(bc);
	end;
	barstyle(bs); 
	xgrid(ticks(x));
	p=plot();
	for i=-50 to 50;
		if i<=p[4] and i>=p[3] then
			 ygrid(i,yt="10^"+i);
		endif;
	end;
	holding(h);
endfunction

function columnsplot3d (z:real, srows=none, scols=none,
	angle=30°, height=40°, zoom=2.5, distance=5,
	crows:vector=none, ccols:vector=none, positive:integer=false)
## Plot 3D columns from the matrix z.
##
## This function shows a 3D plot of columns with heights z[i,j] in
## a rectangular array. z can be any real nxm matrix.
##
## z : the values to be displayed
## srows : labels for the rows
## scols : labels for the columns
## crows : colors of the rows
## ccols : colors of the columns (alternatively)
## positive : plot only positive columns
##
## Example
## >x=normal(1,1000); y=normal(1,1000);
## >v=-6:6; z=find2(x,y,v,v);
## >columnsplot3d(z,v,v,>positive):
##
## See: find2
	mz=totalmax(z);
	z=z/abs(mz);
	n=rows(z); m=cols(z);
	M=zeros(n*m,6);
	d=1.5/max(n,m);
	x=-n*d:d:(n-1)*d; x=x+d/2;
	y=-m*d:d:(m-1)*d; y=y+d/2;
	k=1;
	c=zeros(1,n*m);
	for i=1 to n;
		for j=1 to m;
			if !positive or (z[i,j]>0) then
				if z[i,j]>=0 then
					M[k]=[y[2*j-1],y[2*j],x[2*i-1],x[2*i],0,z[i,j]];
				else
					M[k]=[y[2*j-1],y[2*j],x[2*i-1],x[2*i],z[i,j],0];
				endif;
				if ccols!=none then c[k]=ccols[j]; endif;
				if crows!=none then c[k]=crows[i]; endif;
			endif;
			k=k+1;
		end;
	end;
	if positive then M=M[1:k-1]; endif;
	if crows==none and ccols==none then c=none; endif;
	if !holding() then clg; endif;
	h=holding(1);
	oldcenter=center([0,0,-0.2]);
	oldview=view();
	viewangle(angle);
	viewheight(height);
	zoom(zoom);
	viewdistance(distance);
	plotcubes(M,c);
	th=textheight();
	signum=-sign(cos(angle));
	if scols!=none then
		loop 1 to m
			{col,row}=project((y[2*#-1]+y[2*#])/2,signum*(n*d+d/2),0);
			ctext(""+scols[#],col,row-th/2);
		end
	endif;
	signum=sign(sin(angle));
	if srows!=none then
		loop 1 to n
			{col,row}=project(signum*(m*d+d/2),(x[2*#-1]+x[2*#])/2,0);
			ctext(""+srows[#],col,row-th/2);
		end
	endif;
	view(oldview);
	center(oldcenter);
	holding(h);
endfunction

function mosaicplot (z: real, srows=none, scols=none,
	textcolor=red, color=green, style="O#")
## Moasaic plot of the data in z.
##
## z : matrix with values
## srows, scols : label strings for the rows and columns (string
## vectors)
## color : a color or a vector of colors for the columns of the plot.
## style : a style or a vector of styles.
##
## For an example see the introduction to statistics.
	if !holding() then clg; endif;
	h=holding(1);
	setplot(0,1,0,1);
	n=rows(z); m=cols(z);
	sc=sum(z')';
	cc=zeros(1,n);
	space=0.1/max(n,m);
	xtotal=1-m*space; ytotal=1-n*space;
	dx=1/m;
	for j=1 to m;
		y=0;
		for i=1 to n;
			dy=z[i,j]/sc[j]*ytotal;
			bc=barcolor(color{i});
			st=barstyle(style{i});
			plotbar((j-1)*dx,y,dx-space,dy);
			barstyle(st);
			barcolor(bc);
			if j==1 then cc[i]=y+dy/2; endif;
			y=y+dy+space;
		end;
	end;
	if srows!=none then
		w=window();
		ht=textheight();
		loop 1 to cols(z);
			ct=textcolor(textcolor);
			{col,row}=toscreen((#-0.5)/m,-0.05);
			_ctext(""+srows[#],[col,row-ht/2]);
			textcolor(ct);
		end;
	endif;
	if scols!=none then
		w=window();
		ht=textheight();
		loop 1 to rows(z);
			ct=textcolor(textcolor);
			{col,row}=toscreen(-0.04,cc[#]);
			_rtext(""+scols[#],[col,row-ht/2]);
			textcolor(ct);
		end;
	endif;
	holding(h);
endfunction

function scatterplots (M:real, lab=none,
	ticks=1, grid=4, style="..")
## Plot all rows of M against all rows of M.
##
## The labels are shown in the diagonal of the plot.
##
## lab : labels for the rows.
	n=rows(M);
	figure(n,n);
	for i=1 to n;
		for j=1 to n;
			figure((i-1)*n+j);
			if i==j then
				if lab!=none then
					setplot(-1,1,-1,1);
					{col,row}=toscreen(0,0);
					ctext(lab[i],col,row-textheight()/2); 
				endif;
			else
				plot2d(M[i],M[j],>points, ..
					style=style,grid=grid,ticks=ticks);
			endif;
		end;
	end;
	figure(0);
endfunction

function statplot (x, y=none, plottype="b", 
	pstyle="[]w", lstyle="-", fstyle="O#",
	xl="", yl="", color=none, vertical=0)
## Plots x against y.
##
## This is a simple form of using plot2d with point, line or bar
## options.
##
## The available plotplottypes are
##
## 'p' : point plot
## 'l' : line plot
## 'b' : both
## 'h' : histgram plot
## 's' : surface plot
##
## pstyle, lstyle, fstyle : Styles for the points, lines and bars
##
## color : color or color array
## vertical : vertical labels
##
## See: style
	if !holding() then clg; endif;
	if color==none then color=_color(); endif;
	h=holding(1);
	if y==none then y=x; x=1:rows(y); endif;
	if plottype=="p" then
		plot2d(x,y,xl=xl,yl=yl,,>points,style=pstyle,color=color',vertical=vertical);
	elseif plottype=="l" then
		plot2d(x,y,xl=xl,yl=yl,,style=lstyle,color=color,vertical=vertical);
	elseif plottype=="b" then
		plot2d(x,y,style=lstyle,xl=xl,yl=yl,color=color,vertical=vertical);
		plot2d(x,y,>points,>add,style=pstyle,color=color');
	elseif plottype=="h" then
		plot2d(makeimpulse(x,y),>bar,style=fstyle,color=color,vertical=vertical);
	elseif plottype=="s" then
		plot2d(x,y,xl=xl,yl=yl,>bar,style=fstyle,color=color,vertical=vertical);
	else
		error("Unknown plottype (use p,l,b,h,s).");
	endif;
	holding(h);
endfunction

function getspectral (x)
## Get a spectral color for 0<=x<=1.
##
## The scheme runs from blue (0) to red (1)
	x=(x-0.15)/1.2+0.1;
	co=cos(2*pi*x)/sqrt(2); si=sin(2*pi*x)/sqrt(1.5);
	r=1+co+si/2; g=1-co+si/2; b=1-si; f=1/3;
	return rgb(r*f,g*f,b*f);
endfunction


function colormap (A, spectral=0, color=white)
## Plot a color map of the matrix A.
##
## Color have a color scale on the right. The color is either a fixed
## color (white by default) or spectral colors.
##
## Example
## >colormap(randexponential(50,50),color=yellow); ...
## >title("Exponential distribution"); ...
## >xlabel("n"); ylabel("m"):
	w=window(); m=margin(0);
	if !holding() then h=holding(1); endif; 
	clg;
	window([w[1],w[2],w[3]*0.85+w[1]*0.15,w[4]]);
	setplot(1,cols(A),1,rows(A));
	min=totalmin(A); max=totalmax(A); A=(A-min)/(max-min);
	cr=getred(color); cb=getblue(color); cg=getgreen(color);
	if spectral then 
		C=getspectral(A); 
	else 
		C=rgb(A*cr,A*cg,A*cb); 
	endif;
	plotrgb(C);
	xplot(grid=4,<frame);
	window([w[3]*0.9+w[1]*0.1,w[2],w[3],w[4]]);
	setplot(1,2,min,max);
	v=linspace(0,1,500)';
	if spectral then 
		C=getspectral(v); 
	else 
		C=rgb(v*cr,v*cg,v*cb); 
	endif;
	plotrgb(C);
	ygrid(ticks(min,max));
	window(w); margin(m); holding(h);
endfunction

// * Data Tables in Statistics

function writetable (x, 
	fixed:integer=0, wc:index=10, dc:nonnegative integer=2, 
	labc=none, labr=none, wlabr=none, lablength=1, 
	NA=".", NAval=NAN,
	ctok:index=none, tok:string vector=none,
	file=none, separator=none, comma=false,
	date=none, time=none)
## Write a table x of statistical values
##
## wc : default width for all columns or vector of widths. This is
##      used only if the separator is not set.
## dc : default decimal precision for all columns or vector of
##      precision values. 
## fixed : use fixed number of decimal digits
##         (boolean or vector of boolean).
##
## labc : labels for the columns (string or real vector)
## lablength : increase the width of the columns, if labels are wider.
## labr : labels for the rows (string or real vector)
##
## NA, NAval : Token string and value to represent "Not Available". By
##             default "." and NAN is used.
##
## comma : write with decimal comma instead of dot.
## separator : use this separator string instead of the default
##             blanks. Note that the number of blanks is determined
##             by wc, if no separator is given.
##
## date : vector of columns, which should be written as dates.
## time : vector of columns, which should be written as times.
##
## Write a table with labels for the columns and rows and formats for
## each row. A typical table looks like this
##
##         A     B     C
##  G   1.02     2     f
##  H   3.05     5     m 
##
## Each number in the table can be translated into a token string.
## This translation can be set with a global variable tok (string
## vector) which applies to all columns with indices in ctok (index
## vector). Or it can be set in each column with an assigned variable
## tok? (string vector), where ? is the number of the column. Note
## that these assigned variables need to be declared with :=, since
## they are not in the parameter list of readtable().
##
## See the introduction to statistics for an example.
##
## See: readtable
	if typeof(x)==16 then
		if length(x)>1 then labc=x[2]; endif;
		if length(x)>2 then tok=x[3]; endif;
		if length(x)>3 then labr=x[4]; endif;
		x=x[1];
	endif;
	if file then open(file,"w"); endif;
	if wlabr==none then wlabr=wc; endif;
	if typeof(wc)==0 then wc=wc*ones(1,cols(x)); endif;
	if !isempty(labc) and lablength then
		for j=1 to cols(x)
			wc[j]=max(wc[j],strlen(""+labc[j])+1);
		end;
	endif;
	if !isempty(labc) then
		if !isempty(labr) then write(printstr(" ",wlabr)); endif;
		for j=1 to length(labc)
			if separator==none then write(printstr(""+labc[j],wc{j}));
			else 
				if j>1 then write(separator); endif;
				write(printstr(""+labc[j],1));
			endif;
		end;
		writeln;
	endif;
	for i=1 to rows(x)
		if !isempty(labr) then 
			write(printstr(""+labr[i],wlabr));
		endif; 
		for j=1 to cols(x)
			w=wc{j}; d=dc{j};
			if x[i,j]==NAval then
				s=printstr(NA,w);
			elseif isvar("tok"+j) then
				t=evaluate("tok"+j);
				if x[i,j]<1 or x[i,j]>length(t) then
					error("Unknown token number "+x[i,j]);
				endif;
				s=printstr(t[x[i,j]],w);
			elseif !isempty(ctok) and any(ctok==j) then
				if x[i,j]<1 or x[i,j]>length(tok) then
					error("Unknown token number "+x[i,j]);
				endif;
				s=printstr(tok[x[i,j]],w);
			elseif !isempty(date) and any(date==j) then
				s=printstr(printdate(date(x[i,j]),<time),max(w,11));
			elseif !isempty(time) and any(time==j) then
				s=printstr(printdate(date(x[i,j]),>timeonly),max(w,6));
			else
				if time!=none then
					t=x[i,j];
					s=""+t;
				else
					t=round(x[i,j],d); 
					if t~=0 then t=0; endif;
					if (fixed{j}) then
						s=printf("%"+w+"."+d+"f",t);
					else
						s=printf("%"+w+"g",t);
					endif
				endif;
			endif;
			if comma then s=strrepl(s,".",","); endif;
			if separator!=none and j>1 then
				write(separator+s);
			else
				write(s);
			endif;
		end
		writeln();
	end;
	if file then close(); endif;
endfunction

function readtable (filename:string, clabs=1, rlabs=0,
	NA=".", NAval=NAN,
	ctok:index=none, tokens=[none],
	separator=none, comma=false,
	date=none, list=false)
## Read a table from a file.
##
## filename: readtable(none,...) will used an open file.
## clabs : The table has a line with headings
## rlabs : Each line has a heading label.
## NA, MAval : Sets the string and the returned value for NA (not
##             available).
## ctok : Indices of the columns, where tokens are to be collected.
## tok1=..., tok2=... : Individual string arrays for columns.
## separator : Optional separating characters.
## comma : Use decimal commas instead of dots.
## date : vector of columns which contain a date.
##
## The table can have a header line (clabs=1) and row labels
## (rlabs=1). The entries of the table can be numbers (by default with
## decimal dots) or strings. In case of strings, these tokens are
## translated to unique numbers. The translation can either be set for
## each column separately in string vectors with names tok1, tok2
## etc., or for the complete table in the tokens parameter. 
##
## The tokens are collected from the columns with indices in the ctok
## vector. If a column has a tok? parameter (tok1, tok2, etc.), tokens
## are not collected automatically from that column but the
## translation in tok? is used.
##
## Note that your have to write tok1:=... since the token parameters
## are not pre-defined parameters in the parameter list.
##
## The table can also contain expressions with units or global
## variables.
##
## "Not Available" can be represented by a special string. The
## default is ".". In the numerical table, it is represented by
## default as NAN. If you do not like this, simply let NAN be
## represented by any other string and translate ti into a numerical
## token.
##
## Dates are converted to a unique day number.
##
## See the introduction for statistics for an example.
##
## The default separator is a comma, semicolon, blank or tabulator. If
## you have a file with semicolons and decimal commas, just enable
## >comma. This will replace all commas with dots before the
## evaluation.
##
## Returns {table, heading string, token strings, rowlabel strings}
##
## See: writetable, date, day
	if filename!=none then open(filename,"r"); endif;
	if clabs then heading=strtokens(getline()); 
	else heading=[none]; 
	endif;
	rowlabels=[none];
	M=[];
	repeat
		until eof();
		line=getline();
		if comma then line=strrepl(line,",","."); endif;
		if strlen(line)<1 then continue; endif;
		if separator!=none then line=strtokens(line,separator);
		else line=strtokens(line);
		endif;
		v=[];
		start=0;
		if rlabs then 
			rowlabels=rowlabels|line[1]; start=1;
		endif; 
		for j=1 to length(line)-start;
			if line[start+j]==NA then 
				v=v|NAval;
			else
				if isvar("tok"+j) then
					t=evaluate("tok"+j);
					k=indexof(t,line[start+j]);
					v=v|k;
				elseif date!=none and any(date==j) then
					x=day(line[start+j]);
					v=v|x;
				elseif ctok!=none and any(ctok==j) then
					k=indexof(tokens,line[start+j]);
					if k==0 then
						tokens=tokens|line[start+j];
						v=v|length(tokens);
					else
						v=v|k;
					endif;
				else
					x=line[start+j]();
					v=v|x;
				endif;
			endif;
		end;
		M=M_v;
	end;
	if filename!=none then close(); endif;
	if list then return {{M,heading,tokens,rowlabels}};
	else return {M,heading,tokens,rowlabels};
	endif;
endfunction

function tablecol (M:real, j:nonnegative vector, NAval=NAN)
## The non-NAN values in the columns j of the table M.
##
## To access a table column, you could simply use M[,j], where j is a
## row vector of indices or a single index. But this function skips
## any NAN values in any of the columns j. It returns the columns
## as rows (transposed) and the indices of the rows.
##
## NANval : The value that should be treated as "Not Available"
##
## Returns {colums as rows, indices of non-NAN rows}
	c=M[,j];
	i=nonzeros(!(sum(c==NAval)>0)');
	return {c[i]',i};
endfunction

function selectrows (M:real, j:index, v:real vector, NAval=NAN)
## Select the rows indices i with M[i,j] in v and not-NAN.
	{c,i0}=tablecol(M,j,NAval);
	i=indexof(v,c);
	return i0[nonzeros(i)];
endfunction

function sortedrows (M:real, j:nonnegative integer vector)
## Index of rows for sorted table with respect to columns in j
##
## The table gets sorted in lexicographic order.
##
## Returns : {sorted table, index of sorted values}
	{B,i}=lexsort(M[,j]);
	return i;
endfunction

// * Shuffle, Sort and Find

// For statistical purposes and many other applications, Euler has
// efficient functions to find values in a vector.

function comment shuffle (v)
## Shuffle the vector v
##
## See: sort
endfunction

function comment sort (v)
## Sort the vector v
##
## The function returns {x,i}, where x is the sorted vector, and i is
## the vector of indices, which sort the vector.
##
## >v=shuffle(1:10)
##  [6,  3,  1,  5,  10,  4,  9,  8,  2,  7]
## >{vx,i}=sort(v); vx,
##  [1,  2,  3,  4,  5,  6,  7,  8,  9,  10]
## >v[i]
##  [1,  2,  3,  4,  5,  6,  7,  8,  9,  10]
##
## See: shuffle
endfunction

function comment lexsort (A)
## Lexicographic sort of the rows of A
##
## Returns {Asorted,i}, where i is the vector of indices, which sorts
## the rows of A.
##
## >A=intrandom(5,5,3)
##        2       1       2       1       2 
##        1       3       3       1       2 
##        3       3       2       1       2 
##        3       1       3       2       2 
##        3       2       1       1       1 
## >lexsort(A)
##        1       3       3       1       2 
##        2       1       2       1       2 
##        3       1       3       2       2 
##        3       2       1       1       1 
##        3       3       2       1       2 
##
## See: sort
endfunction

function overwrite unique (v)
## Unique elements in v
##
## >v=intrandom(10,12)
##  [6,  2,  3,  9,  6,  5,  7,  7,  10,  2]
## >unique(v)
##  [2,  3,  5,  6,  7,  9,  10]
	if typeof(v)==13 then
		{vs,ind}=sort(v);
		i=_unique(indexofsorted(vs,v));
		return vs[i];
	else
		return _unique(v);
	endif;
endfunction

function comment find (v,x)
## Find x in the intervals of the sorted vector v
##
## Returns the index i such that v(i) <= x < v(i+1). It returns 0 for
## elements smaller than v[0], and length(v) for elements larger or equal
## the last element of v. The function maps to x. 
##
## The function works for sorted vectors of strings v, and strings or
## string vectors x using alphabetic (ASCII) string comparison.
##
## >s=random(10)
##  [0.270906,  0.704419,  0.217693,  0.445363,  0.308411,  0.914541,
##  0.193585,  0.463387,  0.095153,  0.595017]
## >v=0.2:0.2:0.8
##  [0.2,  0.4,  0.6,  0.8]
## >find(v,s)
##  [1,  3,  1,  2,  1,  4,  0,  2,  0,  2]
##
## See: indexof, indexofsorted
endfunction

function comment count (v,n)
## Counts v[i] in integer intervals [i-1,i] up to n
##
## Returns a vector n, where n[i] is the number of elements of v in
## the interval [i-1,i[ for 1<=i<=n.
## >count([0,0.1,0.2,1,1.5,2],2)
##  [3,  2]
endfunction

function comment indexof (v,x)
## Find x in the vector v
##
## Find the first occurence of x in the vector v. Maps to x.
##
## >v=intrandom(10,4)
##  [6,  5,  2,  2,  3,  8,  5,  4,  4,  2]
## >indexof(v,1:10)
##  [0,  3,  5,  8,  2,  1,  0,  6,  0,  0]
## >indexof(["This","is","a","test"],"a")
##  3
##
## See: indexofsorted, find
endfunction

function comment indexofsorted (v,x)
## Find x in the sorted vector v
##
## Find the last occurence of x in the vector v. Note that indexof
## returns the first occurence. Maps to x.
##
## >v=sort(intrandom(10,4))
##  [3,  4,  5,  5,  5,  6,  8,  8,  9,  10]
## >indexofsorted(v,1:10)
##  [0,  0,  1,  2,  5,  6,  0,  8,  9,  10]
##
## See: indexof, find
endfunction

function comment multofsorted (v, x)
## Counts x in the sorted vector v
##
## The function maps to x.
##
## >v=intrandom(1000,10); multofsorted(sort(v),1:10), sum(%)
##  [88,  84,  126,  86,  110,  104,  86,  103,  113,  100]
##  1000
##
## See: getmultiplicities, getfrequencies
endfunction

function getfrequencies (x:real vector, r: real vector)
## Count the number of x in the intervals of the sorted vector r.
##
## The function returns the number of x[j] in the intervals r[i-1] to
## r[i].
##
## x : real row vector (1xn)
## r : real sorted row vector (1xm)
##
## Returns the frequencies f as a row vector (1x(m-1)) 
##
## See: count, histo, multofsorted, getmultiplicities
	return multofsorted((sort(find(r,x))),1:cols(r)-1);
endfunction

function getmultiplicities (x, y, sorted=0)
## Counts how often the elements of x appear in y.
##
## This works for string vectors and for real vectors.
##
## sorted : if true, then y is assumed to be sorted.
##
## See: count, getfrequencies, multofsorted
	if typeof(x)==8 then x=[x]; endif;
	if !sorted then y=sort(y); endif;
	return multofsorted(y,x);
endfunction

function getstatistics (x:real vector, y:real vector=none)
## Return a statics of the values in the vector x.
##
## If y is none, the function returns {xu,mu}, where xu are the
## unique elements of x, and mu are the multiplicities of these
## values.
##
## Else the function returns {xu,yu,m}, where xu are the unique
## elements of x, yu the unique elements of y, and M is a table of
## multiplicities of pairs (xu[i],yu[j]) in (x[k],y[k]), k=1...n.
	if y==none then
	    xu=unique(x);
		return {xu,getmultiplicities(xu,x)};
	else
		if cols(x)!=cols(y) then 
			error("x and y must have same length");
		endif;
		xu=unique(x);
		yu=unique(y);
		i1=indexofsorted(xu,x);
		i2=indexofsorted(yu,y);
		M=zeros(length(xu),length(yu));
		loop 1 to cols(x);
			M[i1[#],i2[#]]=M[i1[#],i2[#]]+1;
		end;
		return {xu,yu,M};
	endif;
endfunction

function args histo (d:real vector, n:index=10, 
	integer:integer=0, even:integer=0, v:real vector=none,
	bar=1)
## Computes {x,y} for histogram plots.
##
## d : 1xm vector of data
##
## Returns {x,y} whith
##
## x - End points of the intervals (equispaced n+1 points)
## y - The number of data in the subintervals (frequencies)
##
## integer : flag for distributions on integers
## even : flag for evenly spaced discrete distributions
##        This is used by plot2d for bar styles.
##
## v : optional interval boundaries (ordered).
##
## bar : If true, the function returns two vectors for >bar in plot2d.
## If false, it returns a sawtooth function for plot2d.
##
## The plot function plot2d has parameters distribution=1, histogram=1
## to achieve the same effect.
##
## See: plot2d
	if v!=none then
		xx=v;
		d=d[nonzeros(d>=min(v) && d<=max(v))];
		y=count(find(v,d),length(v));
		y=y[2:length(y)];
	else	
		mi=min(d); ma=max(d);
		if even then dc=(ma-mi)/(2*(n-1)); mi=mi-dc; ma=ma+dc; endif;
		if mi~=ma then ma=mi+1; endif;
		if integer then n=floor(ma-mi)+1; ma=ma+0.9999; endif;
		x=zeros(1,n); y=zeros(1,n); h=(d-mi)/(ma-mi)*n;
		c=count(h,n+1); c[n]=c[n]+c[n+1]; y=c[1:n];
		xx=linspace(mi,ma,n);
	endif;
	if !bar then
	    i=2*ones(size(xx));
		xx=multdup(xx,i);
	    i=2*ones(size(y));
		y=multdup(y,i); y=0|y|0;
	endif;
	return {xx,y};
endfunction;

function find2 (x:vector, y:vector, 
	vx:vector=none, vy:vector=none, n:integer=none)
## Matrix count for pairs x[i],y[i] in the bounds.
##
## x,y : Vectors of same size.
## vx,vy : Sorted vector of bounds, if present (must enclose x resp. y)
## n : If vx or vy is not present, number of intervals between the bounds of x.
##
## Returns a matrix with counts.
##
## See: find, columnsplot3d
	if vx==none then vx=linspace(min(x),max(x)+epsilon,n); endif;
	if vy==none then vy=linspace(min(y),max(y)+epsilon,n); endif;
	z=zeros(cols(vx),cols(vy));
	loop 1 to cols(x)
		zi=find(vx,x[#]); zj=find(vy,y[#]);
		z[zi,zj]=z[zi,zj]+1;
	end;
	return z;
endfunction

// * Confidence Intervals

function cinormal (mean:numerical, sigma:numerical, alpha=0.05)
## Confidence interval for known mean and standard deviation.
##
## See: cimean
	d=invnormaldis(1-alpha/2);
	return mean+[-sigma*d,sigma*d];
endfunction

function cimean (data: real vector, alpha=0.05)
## Confidence interval for the mean of normal distributed data
##
## This is a symmetric interval around the mean value of the data
## containing the true mean of the random experiment in 95% (default
## alpha=0.05) of the cases. The data are assumed to be
## from identically normal distributed independent random variables.
##
## See: clopperpearson
  n=cols(data); {m,d}=meandev(data);
  f=invtdis(1-alpha/2,n-1);
  return m+[-f*d/sqrt(n),f*d/sqrt(n)];
$endfunction

function clopperpearson (k:natural, n:natural, alpha=0.05)
## Clopper-Pearson confidence interval for k hits in n.
##
## The upper bound of the interval is such that P(X<=k,p)=alpha/2, the
## lower bound such that P(X>=k,p)=alpha/2. In other words, if p is
## outside the interval then k is an event which is less likely then
## alpha. This interval estimator yields an interval which contains
## the true p in 95% (default alpha=0.05) of the cases.
##
## >clopperpearson(20,400)
##   [0.0308831,  0.076167]
##
## See: cimean
	a=invbetai(alpha/2,k,n-k+1);
	b=invbetai(1-alpha/2,k+1,n-k);
	return [a,b];
endfunction
