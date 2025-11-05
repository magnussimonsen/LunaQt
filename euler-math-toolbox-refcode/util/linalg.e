// Linear Algebra

comment
Linear algebra and regression routines.
endcomment

// This file contains basic linear algebra routines, like solutions of
// linear systems, linear fit, polynomial fit, eigenvalues and
// eigenspaces, decompositions, singular values, norms, and special
// matrices.

// It does also contain methods to solve systems by hand (pivotize).

// For an introduction and the core functions see the following links.

// See: ../Programs/04 - Linear Algebra | Tutorial
// See: eulercore.html#Functions_for_Vectors_and_Matrices | Euler Core

// * Linear Systems

function id (n:index, m:index=none)
## Creates a nxn identity matrix.
##
## See: diag, eye
	if m==none then return diag([n,n],0,1);
	else return diag([n,m],0,1);
	endif;
endfunction

function eye (n:positive integer, m:index=none)
## Creates a nxm matrix with 1 on the diagonal
##
## If m=none, then it creates an nxn matrix. This is a Matlab
## function.
##
## See: id
	if m==none then 
		if cols(n)>1 then return diag(n,0,1);
		else return diag([n,n],0,1);
		endif;
	else return diag([n,m],0,1);
	endif;
endfunction

function diagmatrix (v:vector, k:integer=0)
## Create a diagonal square matrix with v on the k-th diagonal
##
## The size of the square matrix is chosen, such that the vector v
## fits into the matrix.
##
## diagmatrix(1:3)
## diag(3,3,0,1:3) // the same
## diagmatrix(1:3,1)
## diag(4,4,1,1:3) // the same
##
## See: diag
	n=cols(v)+abs(k);
	return diag(n,n,k,v);
endfunction

function transpose (A)
## returns A'
## Alias for Maxima
	return A';
endfunction

function image (A:complex, eps=none)
## Computes the image of the quadratic matrix A.
##
## Returns a matrix, containing a basis of the image of A in the
## columns.
##
## The image of a matrix is the space spanned by its columns. This
## functions uses an LU decomposition with the Gauss algorithm to
## determine a base of the columns. The function "svdimage" returns an
## orthogonal basis of the image with more effort.
##
## >A=redim(1:9,3,3)
##                    1                   2                   3 
##                    4                   5                   6 
##                    7                   8                   9 
## >rank(A)
##  2
## >image(A)
##      0.0592348877759      0.118469775552 
##       0.236939551104       0.29617443888 
##       0.414644214431      0.473879102207 
## >svdimage(A)
##      -0.214837238368     -0.887230688346 
##      -0.520587389465     -0.249643952988 
##      -0.826337540561       0.38794278237 
## >kernel(A)
##                    1 
##                   -2 
##                    1 
## >svdkernel(A)
##       0.408248290464 
##      -0.816496580928 
##       0.408248290464 
##
## See: svdimage
	if eps localepsilon(eps); endif;
	a=A/norm(A);
	{aa,r,c,d}=lu(a);
	return a[:,nonzeros(c)];
endfunction

function kernel (A:complex, eps=none)
## Computes the kernel of the matrix A.
##
## Returns a matrix M, containing a basis of the kernel of A. I.e.,
## A.M=0.
##
## This function uses an LU decomposition with the Gauss algorithm. The
## function "svdkernel" does the same with more effort, but it returns
## an orthogonal base of the kernel.
##
## Add eps=..., if A is almost regular. Use a larger epsilon in this
## case than the default epsilon(). 
##
## See: image, svdkernel
	if eps localepsilon(eps); endif;
	no=norm(A);
	if no==0 then return id(cols(A)); endif;
	a=A/no;
	{aa,r,c,d}=lu(a);
	n=size(a); nr=size(r);
	if nr[2]==n[2]; return zeros([1,n[2]])'; endif;
	if nr[2]==0; return id(n[2]); endif;
	c1=nonzeros(c); c2=nonzeros(!c);
	b=lusolve(aa[r,c1],a[r,c2]);
	m=size(b);
	x=zeros([n[2] m[2]]);
	x[c1,:]=-b;
	x[c2,:]=id(cols(c2));
	return x
endfunction

function rank (A:complex)
## Computes the rank of the matrix A.
##
## The rank is the dimension of the image, the space spanned by the
## columns of A.
##
## See: image
	return cols(image(A));
endfunction

// * Matrix Functions

function overwrite max
## max(A) or max(x,y,...)
##
## Returns the maximum of the rows of A for one argument,
## or the maximum of x,y,z,... for several arguments.
##
## See: max, extrema
	if argn==1 then return _max(arg1);
	elseif argn==2 then return _max(arg1,arg2);
	else return _max(arg1,max(args(2)));
	endif;
endfunction

function overwrite min
## min(A) or min(x,y,...)
##
## Returns the minimum of the rows of A for one argument,
## or the minimum of x,y,z,... for several arguments.
##
## See: min, extrema
	if argn==1 then return _min(arg1);
	elseif argn==2 then return _min(arg1,arg2);
	else return _min(arg1,min(args(2)));
	endif;
endfunction

function comment extrema(A)
## Extremal values [min,imin,max,imax] in each row of A
##
## >x=random(5)
##  [0.866587,  0.536137,  0.493453,  0.601344,  0.659461]
## >extrema(x)
##  [0.493453,  3,  0.866587,  1]
##
## See: min, max
endfunction

function comment totalmax (A)
## Maximal entry in the matrix A
endfunction

function comment totalmin (A)
## Minimal entry in the matrix A
endfunction

function comment flipx (A)
## flips a matrix vertically
endfunction

function comment flipy (A)
## flips a matrix horizontally
endfunction

function flipud (A:numerical)
## flips a matrix vertically.
	return flipy(A)
endfunction

function fliplr (A:numerical)
## flips a matrix horizontally.
	return flipx(A)
endfunction

function comment rotleft (A)
## rotates the rows of A to the left
endfunction

function comment rotright (A)
## rotates the rows of A to the right
endfunction

function comment shiftleft (A)
## shifts the rows of A to the left, last column to 0
endfunction

function comment rotright (A)
## shifts the rows of A to the right, first column to 0
endfunction

function rot90 (A:numerical)
## rotates a matrix counterclockwise
	return flipx(A)'
endfunction

function rot (A:numerical, k:integer=1)
## rotates a matrix counterclockwise k times 90 degrees
    k=mod(k,4);
	if k==0 then return A
	elseif k==1 then return rot90(A)
	elseif k==2 then return flipx(flipy(A))
	elseif k==3 then return flipx(A')
	endif;
endfunction

function comment drop (v,i)
## Drop elements i from a row vector v
##
## See: droprows, dropcols, butfirst, butlast
endfunction

function droprows (A, i)
## Drop rows i from a matrix A
##
## see: drop, dropcols, butfirst, butlast
	i = i; // because drop will sort i
	return A[_drop(1:rows(A),i),:];
	endif;
endfunction 

function dropcols (A, i)
## Drop columns i from a matrix A
##
## see: drop, droprows, butfirst, butlast
	i = i; // because drop will sort i
	return A[:,_drop(1:cols(A),i)];
	endif;
endfunction 

function butfirst (v)
## Returns the vector v without its first element
##
## For a matrix this will return the matrix without the first row.
##
## See: drop, dropcols, droprows, butlast
	return v[2:length(v)]
endfunction

function butlast (v)
## Returns the vector v without its last element
##
## For a matrix this will return the matrix without the last row.
##
## See: drop, dropcols, droprows, butlast
	return v[1:length(v)-1]
endfunction

// For sums and products of vectors and rows of matrices have a look
// at the core functions.

// See: eulercore.html#Functions_for_Vectors_and_Matrices | Vectors and Matrices

function totalsum (A)
## Computes the sum of all elements of A.
	return sum(sum(A)');
endfunction

function flatten (A)
## Reshapes A to a vector
	return redim(A,1,prod(size(A)));
endfunction

// * Solving Linear Systems

// The simple Gauss algorithm is implemented in the A\b operator in
// Euler and in an algorithm from AlgLib.

// The Gauss-Seidel method and the conjugate gradient methods are
// implemented for full and sparse matrices. Moreover, there are
// decompositions in LU or LL', and routines to solve systems based on
// the decompositions.

// Euler has functions for sparse systems, exact functions, and
// interval routines for linear systems.

// See: numerical.html#Sparce_Matrices | Sparse Matrices
// See: numerical.html#Interval_Solvers_and_Guaranteed_Solutions | Interval Routines

function overwrite alsolve (A:real, B:real, 
	exact=0, check=1)
## Solve A.X=B
##
## A must be square, and B can have more than one column. The function
## uses an implementation of the Gauss algorithm from AlgLib. The
## result is of the same order as the Euler operator A\B.
##
## exacter : If set, refinements are tried to get a better solution.
## check : If set, low condition numbers issue an error exception.
##
## Returns {X,info,condition}
## info : If -3, the matrix was found to be singular
## condition : If close to 0, the matrix should be singular (inverse
## condition)
##
## >A=normal(200,200); b=sum(A);
## >longest totalmax(abs(alsolve(A,b)-1))
##   1.321165399303936e-014 
## >longest totalmax(abs(A\b-1))
##   3.708144902248023e-014 
##
	if check then
		{X,info,cond} = _alsolve(A,B,exact);
		if info!=1 or cond~=0 then
			error("Singular matrix found in alsolve");
		endif;
		return X;
	else
		return _alsolve(A,B,exact);
	endif;
endfunction

function inv (A:complex)
## Computes the inverse of A.
##
## See: xinv, svdsolve, xlgs, fit, pinv
	return A\id(cols(A));
endfunction

function invert (A)
## Computes the inverse of A.
##
## Alias to inv.
	return inv(A);
endfunction

function comment lu (A)
## LU-decomposition of A.
## 
## The result is {B,r,c,det}. 
## 
## B is the result of the Gauss algorithm with the factors written
## below the diagonal, r is the index rearrangement of the rows of A,
## c is 1 for each linear independent column and det is the
## determinant of A. 
##
## There is a utility function LU, which computes the LU-decomposiotn
## of a matrix (L,R,P such that LR=PA).
##
## To get a true LU-decomposition using lu for a non-singular square
## A, take the lower part of B (plus the identity-matrix) as L and the
## upper part as U. Then A[r] is L.U, i.e. for quadratic A, A[r] is
##
## >(band(B[r],-n,-1)+id(n)).band(B[r],0,n) 
##
## To solve A.x=b for quadratic A and an x quickly, use
## lusolve(B[r],b[r]).
##
## >A=normal(20,20); b=sum(A);
## >{B,r,c,det}=lu(A); det,
##  -2.61073e+008
## >longest totalmax(abs((band(B[r],-20,-1)+id(20)).band(B[r],0,20)-A[r]))
##   2.220446049250313e-015 
## >longest totalmax(abs(lusolve(B[r],b[r])-1))
##    1.06581410364015e-014 
##
## See: lusolve
endfunction

function comment lusolve (R,b)
## Solve linear system LUx=b.
##
## L has ones on the diagonal. It is stored in the lower left part of
## R. U is stored in the upper right part of R. This is the way that
## lu returns the matrices.
##
## >R=[1,2,3;0,4,5;0,0,6]
##           1          2          3 
##           0          4          5 
##           0          0          6 
## >L=[0;1;1,2,0]
##           0          0          0 
##           1          0          0 
##           1          2          0 
## >A=(L+id(3)).R
##           1          2          3 
##           1          6          8 
##           1         10         19 
## >A\sum(A)
##           1 
##           1 
##           1 
## >lusolve(R+L,sum(A))
##           1 
##           1 
##           1 
##
## See: lu
endfunction

function seidel (A:real, b:real column, x=0, om=1.2, eps=none)
## Solve Ax=b using the Gauss-Seidel method.
##
## A must be diagonal dominant, at least not have 0 on the diagonal.
## The method will converge for all positive definite matrices.
## However, larger dominance in the diagonal speeds up the
## convergence. For thin matrices use seidelX().
##
## om : 
## This is a relaxation parameter between 1 and 2. The default is 1.2,
## and works in many cases. 
##
## x : start value
##
## You can specify the accuracy with an optional parameter eps.
##
## See: seidelX
	if eps then localepsilon(eps); endif;
	if x==0 then x=zeros(size(b)); endif;
	n=cols(A);
	D=band(A,0,0);
	M1=band(A,1,n-1)+(1-1/om)*D;
	M2=flipx(flipy(band(A,-n+1,-1)+D/om));
	repeat
		xn=flipy(lusolve(M2,flipy(b-M1.x)));
		if all(xn~=x) then return xn; endif;
		x=xn;
	end;
endfunction

function cg (A:real, b:real column, x0:column=none, 
	f:index=10, eps=none)
## Conjugate gradient method to solve Ax=b.
##
## This function is the function of choice for large matrices. There
## is a variant "cgX" for large, sparse matrices.
##
## Stops as soon as the error gets larger.
##
## See: cgX, cpxfit
	if eps then localepsilon(eps); endif;
	n=cols(A);
	if x0==none then x=zeros(size(b));
	else; x=x0;
	endif;
	loop 1 to 10
		r=b-A.x; p=r; fehler=r'.r;
		loop 1 to f*n
			if sqrt(fehler)~=0 then return x; endif;
			Hp=A.p;
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

function solvespecial (A:complex, b:complex)
## Special solution of Ax=b using the LU decomposition.
##
## Returns a vector, such that A.x=b, even for singular, or non-square
## matrices A.
##
## This function uses the Gauss algorithm to determine the LU
## decomposition. It then extracts a base from the columns, and
## uses this base to get a special solution. Works only, if the matrix
## has maximal rank, i.e., the rows are linear independent.
##
## For matrices, which have a solution, the functions "svdsolve" or
## "fit" provide the same functionality with more effort.
##
## See: svdsolve, fit, kernel
	{a,r,c,d}=lu(A); x=zeros(cols(A),1); cc=nonzeros(c);
	s=A(:,cc)\b; x(cc)=s;
	return x
endfunction

function det (A:complex, eps=none)
## Determinant of A.
##
## Uses the Gauss algorithm to compute the determinant of A.
	s=size(A); 
	if s[1]!=s[2] then 
		error("Non square matrix in det()");
	endif;
	if eps then localepsilon(eps); endif;
	r=maxrowsum(A);
	{aa,rows,c,d}=lu(A/r);
	return d*r^cols(A);
endfunction

function cholesky (A)
## Decompose a real matrix A, such that A=L.L'. Returns L.
##
## A must be a positive definite symmetric and at least 2x2 matrix.
## A is not checked for symmetry,
##
## See: LU, lu
	n=cols(A);
	L=zeros(size(A));
	L[1,1]=sqrt(A[1,1]); L[2,1]=A[1,2]/L[1,1];
	L[2,2]=sqrt(A[2,2]-L[2,1]^2);
	loop 3 to n;
		c=flipy(lusolve(flipx(flipy(L[1:#-1,1:#-1])),flipy(A[1:#-1,#])));
		L[#,1:#-1]=c';
		L[#,#]=sqrt(A[#,#]-c'.c);
	end;
	return L;
endfunction

function lsolve (L,b)
## Solve L.L'.x=b
##
## See: cholesky
	return lusolve(L',flipy(lusolve(flipx(flipy(L)),flipy(b))));
endfunction

function pivotize (A:numerical, i:index, j:index)
## Make A[i,j]=1 and all other elements in column j equal to 0.
##
## The function is using elementary operations on the rows of A. Use
## this functions for the demonstration of the Gauss algorithm.
##
## The function modifies A, but returns A, so that it can be printed
## easily.
##
## See: normalizeRow, swapRows, gjstep
	k=1:rows(A); k=nonzeros(k<>i);
	A[k]=A[k]-A[k,j]/A[i,j]*A[i];
	A[i]=A[i]/A[i,j];
	return A;
endfunction

function pivotizeRows (A:numerical, i:index, j:index)
## Alias to pivotize
##
## See: pivotize
	return pivotize(A,i,j);
endfunction

function normalizeRow (A:numerical, i:index, j:index)
## Divide row j by A[i,j].
##
## The function is using elementary operations. Use this functions for
## the demontration of the Gauss algorithm.
##
## See: pivotizeRows, swapRows
	A[i]:=A[i]/A[i,j];
	return A;
endfunction

function swapRows (A:numerical, i1:index, i2:index)
## Swap rows i1 and i2 of A.
##
## The function is using elementary operations. Use this functions for
## the demontration of the Gauss algorithm.
##
## See: pivotizeRows, normalizeRow
	A[[i1,i2]] := A[[i2,i1]];
	return A
endfunction

hilbertfactor:=3*3*3*5*5*7*11*13*17*19*23*29;

function hilbert (n:index, f=hilbertfactor)
## Scaled nxn Hilbert matrix.
##
## The Hilbert matrix is the matrix (1/(i+j-1)). This function
## multplies all entries with a factor, so thatthe matrix can
## accurately be stored up to the 30x30 Hilbert matrix.
    {i,j}=field(1:n,1:n);
    return f/(i+j-1);
endfunction

function hilb (n:integer, f=hilbertfactor)
## Scaled nxn Hilbert matrix.
##
## Alias for hilbert(n).
##
## See: hilbert
	return hilbert(n,f);
endfunction

function comment givensrot (j,i1,i2,A,B)
## One Givens rotation of A and B
##
## Returns (X=GA,Y=GB), such that X[l2,j]=0, and G is an orthogonal
## Matrix, changing only the rows i1 and i2 of A and B.
##
## >shortformat; A=random(3,3),
##      0.77473     0.89357      0.5985 
##      0.70968     0.36746     0.80751 
##      0.59486     0.67635     0.26406 
## >B=id(3)
##            1           0           0 
##            0           1           0 
##            0           0           1 
## >{X,Y}=givensrot(1,1,2,A,B); X
##      -1.0506    -0.90712    -0.98678 
##            0     0.33263    -0.19118 
##      0.59486     0.67635     0.26406 
## >Y.Y'
##            1           0           0 
##            0           1           0 
##            0           0           1 
##
## See: givensqr
endfunction

function comment givensqr (A,B)
## QR decomposition of A and B
##
## Returns {R=GA,Y=GB,c}, such that R is upper triangle, and c shows
## the linear independent columns of A.
##
## See: qrdecomp
endfunction

function qrdecomp (A:real)
## Decompose QA=R using Givens rotations.
##
## The function returns {R,Q,c}. Q is an orthogonal matrix, and R is
## an upper triangle matrix. c is a vector with ones in the basis
## columns of A.
##
## This function uses orthogonal transformations to compute Q and R.
## It works for non-square matrices A.
##
## >A=normal(5,5);
## >{R,Q}=qrdecomp(A);
## >longest totalmax(abs(Q.A-R))
##   4.440892098500626e-016 
##
## see: givensqr, givensrot
	B=id(rows(A));
	return givensqr(A,B);
endfunction

function comment orthogonal (A)
## Compute an orthogonal basis of the columns of A
##
## This functions uses the Gram-Schmid method to compute an orthogonal
## basis of the columns of A. c is a row vector with 1 for the indices
## of the basis columns of A.
##
## >shortformat; A=random(3,5)
##   0.883227  0.270906  0.704419  0.217693  0.445363 
##   0.308411  0.914541  0.193585  0.463387  0.095153 
##   0.595017  0.431184   0.72868  0.465164  0.323032 
## >{B,c}=orthogonal(A); c
##  [1,  1,  1,  0,  0]
## >O=B[,nonzeros(c)]
##   0.796621 -0.370762 -0.477421 
##   0.278169   0.92606  -0.25502 
##   0.536672 0.0703505  0.840853 
## >O.O'
##          1         0         0 
##          0         1         0 
##          0         0         1 
##
## See: svdimage
endfunction

// * Fits and Regression Analysis

// There are functions for linear regression using the normal
// equation, orthogonal transformations, and singular value
// decomposition.

function fitnormal (A:complex, b:complex)
## Computes x such that ||A.x-b|| is minimal using the normal equation.
##
## A is an nxm matrix, and b is a nx1 vector.
##
## The function uses the normal equation A'.A.x=A'.b. This works only,
## if A has full rank. E.g., if n>m, then the m columns of A must be
## linear independent. The function "fit" provides a faster and better
## solution. "svdsolve" has the additional advantage that it produces
## the solution with minimal norm. For sparse matrices use "cpxfit"
##
## For badly conditioned A, you should use svdsolve instead.
##
## A : real or complex matrix (nxm)
## b : column vector or matrix (mx1 or mxk)
##
## See: fit, svdsolve, cpxfit
	At=conj(A');
	return symmult(At,A)\(At.b)
endfunction

function fit (A:real, b:real)
## Computes x such that ||A.x-b||_2 is minimal using Givens rotations.
##
## A is a nxm matrix, and b is a nxk vector (usually k=1).
##
## This function works even if A has no full rank. Since it uses
## orthogonal transformations, it us also quite stable. The function
## "svdsolve" does the same, but minimizes the norm of the solution
## too.
##
## A : real matrix (nxm)
## b : real column vector or matrix  (mx1 or mxk)
##
## See: fitnormal, svdsolve
	{x,y,c}=givensqr(A,b);
	c=nonzeros(c); n=1:cols(c);
	xr=lusolve(x[n,c],y[n,:]);
	x=zeros(cols(A),cols(b)); x[c,:]=xr;
	return x;
endfunction

function divideinto (A: real, B: real)
## Divide A into B yielding fit(B',A')'
##
## This function is used in Matlab mode for A/B.
##
## See: fit
	return fit(B',A')'
endfunction

function polyfit (x:complex vector, y:complex vector, ..
	n:nonnegative integer scalar, w:real vector=none)
## Fits a polynomial in L_2-norm to data.
##
## Given data x[i] and y[i], this function computes a polynomial p of
## given degree such that the sum over the squared
## errors (p(x[i])-y[1])^2 is minimal.
##
## The fit uses the normal equation solved with an orthogonal
## decomposition.
##
## w : Optional weights (squared) for the fit.
##
## See: fit
	A=ones(size(x))_dup(x,n); A=cumprod(A');
	if w!=none then A=w'*A; y=w*y; endif;
	return fit(A,y')';
endfunction

// * Norms

function norm (A:numerical, p:nonnegative real=2)
## Euclidean norm of the vector A.
##
## For p=2, the function computes the square root of the sum of
## squares of all elements of A, if A is a matrix. For p>0 it computes
## the p-norm. For p=0 it computes the sup-norm.
##
## Note: The function does not compute the matrix norm.
##
## See: maxrowsum
	if p==2 then return sqrt(sum(sum(abs(A)^2)'));
	elseif p==0 then return totalmax(abs(A));
	elseif p==1 then return totalsum(abs(A));
	else return totalsum(A^p)^(1/p);
	endif;
endfunction

function maxrowsum (A:numerical)
## Maximal row sum of A
	return max(sum(abs(A))');
endfunction

function comment scalp (v,w)
## Scalar product of v an w
endfunction

function crossproduct (v:real, w:real)
## Cross product between two 3x1 or 1x3 vectors.
##
## The function will work for two column or row vectors. The result is
## of the same form as v. It will also work for a matrix of 1x3 columns
## v, and a matrix of 1x3 columns w.
##
## See: vectorproduct, scalp
	if rows(v)==1 then
		return [v[2]*w[3]-v[3]*w[2],-v[1]*w[3]+w[1]*v[3],v[1]*w[2]-v[2]*w[1]];
	else if rows(v)==3 then
		return (v[2]*w[3]-v[3]*w[2])_(-v[1]*w[3]+w[1]*v[3])_(v[1]*w[2]-v[2]*w[1]);
	else
		error("Wrong arguments for crossproduct().");	
	endif;		
endfunction

function vectorproduct (A:real)
## Multiplication of the columns of an nx(n-1) matrix A.
##
## Generalized cross product of n-1 vectors of length n. The result is
## perpendicular to the vectors, and its length is the volume of the
## polyhedron spanned by the vectors squared.
##
## A : real matrix of size n times (n-1)
##
## See: crossproduct, scalp, gramdet
	B=(A|1)'; b=zeros(cols(A),1)_1;
	return (B\b)*det(B);
endfunction

function gramdet (A)
## Gram determinant of the columns of A.
##
## The Gram determinant is the volume of the polyhedron spanned by the
## columns of A. 
##
## A : real matrix of size kxn (k<n)
##
## See: vectorproduct 
	return sqrt(det(A'.A));
endfunction

function %pospower (A,n)
	if n==1 then return A; endif;
	m=floor(n/2); y=%pospower(A,m);
	if (2*m==n) then return y.y;
	else return y.y.A;
	endif;
endfunction

// * Matrix Functions

function power (A:numerical, n:integer scalar)
## A^n for integer n for square matrices A.
##
## The function uses a binary recursive algorithm. For n<0, it will
## compute the inverse matrix first. For n<0, the matrix must be real
## or complex, not interval.
##
## For scalar values, use the faster x^n. Moreover, power(x,n) will
## not map to the elements of x or n.
##
## See: matrixpower
	if n>0 then return %pospower(A,n);
	elseif n==0 then return id(cols(A)); 
	elseif not isinterval(A) then return %pospower(inv(A),-n);
	else error("power(A,n) does not work for interval matrices and n<0"); 
	endif;
endfunction

function matrixpospower (A:numerical, n:natural)
## Returns A^n for natural n>=0 and a matrix A.
##
## Alias for power(A,n)
	return %pospower(A,n)
endfunction

function matrixpower (A:complex, n:integer scalar)
## Returns A^n for integer n and a matrix A.
##
## Alias for power(A,n)
	return power(A,n)
endfunction

function matrixfunction (f:string, A:complex)
## Evaluates the function f for a matrix A.
##
## The function f must be an analytic function in the spectrum of A.
## A must have a basis of eigenvectors. This function will decompose A
## into M.D.inv(M), where D is a diagonal matrix, and apply f to the
## diagonal of D.
##
## See: matrixpower
	{d,M}=eigen(A);
	R=M.diag(size(A),0,f(d;args())).inv(M);
	if all(R~=re(R)) then return real(R)
	else return R;
	endif;
endfunction

function LU (A)
## Returns {L,R,P} such that L.R=P.A
##
## This function uses the Gauss-Algorithm in lr(A) to find the LR
## decomposition of a regular square matrix A. L is a lower triangle
## matrix with 1 on the diagonal, R an upper right triangle matrix,
## and P a permutation matrix.
##
## See: lu
	{B,r,c,det}=lu(A);
	if det~=0 then error("Determinant too small!"); endif;
	n=cols(A);
	return {band(B[r],-n+1,-1)+id(n),band(B[r],0,n-1),id(n)[r]}
endfunction

function echelon (A)
## Forms A into echelon form using the Gauss algorithm
##
## See: LU, lu
	{B,r,c,det}=lu(A);
	j=nonzeros(c);
	i=1:length(j);
	C=A[r];
	return inv(C[i,j]).C[i];
endfunction

// * Eigenvalues and Singular Values

// For eigenvalues and eigenvectors, Euler has algorithms based on the
// AlgLib library, and algorithms based on the characteristic
// polynomial.

function comment charpoly (A)
## Characteristic polynomial of A
##
## Euler uses an orthogonal transformation to a simpler matrix, and a
## recursion to compute the characteristic polynomial. Note that
## polynomials in Euler start with the constant coefficient.
endfunction

function eigenvalues (A:complex, 
	usecharpoly=0, check=1)
## Eigenvalues of A.
##
## Returns a complex vector of eigenvalues of A.
##
## For real matrices, the functions uses an algorithm from AlgLib and
## LAPACK to find the eigenvalues, unless usecharpoly is true.
##
## If usecharpoly is true or the matrix is complex, this function
## computes the characteristic polynomial using orthogonal
## transformations to an upper diagonal matrix, with additional
## elements below the diagonal. It then computes the zeros of this
## polynomial with "polysolve".
##
## See: charpoly, svd, jacobi, svdeigenvalues, mxmeigenvalues, aleigen
	if usecharpoly or iscomplex(A) then
		return polysolve(charpoly(A));
	else
		{v,flag}=_aleigen(A,0);
		if check and !flag then
			error("aleigen did not converge");
		endif;
		if !check then return {v,flag};
		else return v;
		endif;
	endif;
endfunction

function eigenspace (A:complex, l:complex vector)
## Returns the eigenspace of A to the eigenvaue l.	
##
## The eigenspace is computed with kernel(A-l*id(n)). Since the
## eigenvalue may be inaccurate, the function tries several epsilons
## to find a non-zero kernel. A more stable function is
## "svdeigenspace".
##
## See: eigenvalues, svdeigenspace
	k=kernel(A-l*id(cols(A)));
	if all(k==0) then k=kernel(A-l*id(cols(A)),eps=1e-8); endif;
	if all(k==0) then k=kernel(A-l*id(cols(A)),eps=1e-4); endif;
	if all(k==0) then error("No eigenvalue found!"); endif;
	si=size(k);
	loop 1 to si[2];
		x=k[:,index()];
		k[:,index()]=x/norm(x);
	end;
	return k;
endfunction

function %eigenremove (l,n)
	{ll,i}=sort(abs(l[1]-l)); h=l[i];
	return l[n+1:cols(l)];
endfunction

function eigen (A:complex,
	usekernel=0, usecharpoly=0, check=1)
## Eigenvectors and a basis of eigenvectors of A.
##
## Returns {l,x}, where l is a vector of eigenvalues, x is a basis
## of eigenvectors.
##
## For real matrices A an algorithm from AlgLib and LAPACK is used,
## unless usekernel is true.
##
## usekernel : Compute the eigenvalues, then the kernels.
## usecharpoly : Use the characteristic polynomial for the
##    eigenvalues.
## check : Check the return code of AlgLib.
##
## See: eigenvalues, eigenspace, mxmeigenvectors
	if usekernel or iscomplex(A) then
		l=eigenvalues(A,usecharpoly);
		s=eigenspace(A,l[1]);
		si=size(s); v=dup(l[1],si[2])'; vv=l[1];
		l=%eigenremove(l,si[2]);
		repeat;
			if min(size(l))==0; break; endif;
			ll=l[1]; sp=eigenspace(A,ll);
			si=size(sp); l=%eigenremove(l,si[2]);
			s=s|sp; v=v|dup(ll,si[2])'; vv=vv|ll;
		end;
		return {vv,s}
	else
		{v,flag,w}=_aleigen(A,1);
		if check and !flag then
			error("aleigen did not converge");
		endif;
		if !check then return {v,w,flag};
		else return {v,w};
		endif;
	endif;
endfunction

function comment aleigen (A,flag)
## AlgLib routine to compute the eigenvalues of A
##
## This function returns the eigenvalues {v,res}, of the eigenvalues
## and the eigenvectors {v,res,W}, if flag is true. A must be real
## vector. The res flag is true, if the algorithm succeeds (from
## AlgLib and LAPACK).
##
## It is easier to use the functions eigenvalues() and eigen().
##
## See: eigenvalues, eigen
endfunction

function cjacobi (A:complex, eps=sqrt(epsilon))
## Jacobi method to compute the eigenvalus of a Hermitian matrix A.
##
## Returns a vector of eigenvalues.
##
## This function computes the eigenvalues of a real or complex matrix
## A, using an iteration with orthogonal transformations. It works
## only for Hermitian matrices A, i.e., A=conj(A').
##
## See: jacobi
	if not all(A==conj(A')) then error("A not Hermitian!"); endif;
	A0=A;
	n=cols(A0);
	repeat;
		H=band(A0,-n,-1);
		E=extrema(abs(H));
		J=extrema(E[:,3]');
		i=J[4];
		j=E[i,4];
		if J[3]<eps; break; endif;
		A2=A0[[j,i],[j,i]];
		Q2=%rotation2(A2);
		Q=id(n);
		Q[[j,i],[j,i]]=Q2;
		A0=conj(Q').A0.Q;
	end;
	return diag(A0,0);
endfunction

function overwrite jacobi (A:real, eps=none)
## Jacobi method to compute the eigenvalus of a symmetric matrix A.
##
## Returns a vector of eigenvalues.
##
## This function computes the eigenvalues of a real matrix A, using an
## iteration with orthogonal transformations. It works only for
## symmetric matrices A, i.e., A=A'.
##
## See: cjacobi
	if eps then localepsilon(eps); endif;
	if not all(A~=A') then error("Matrix not symmetric!"); endif;
	return _jacobi((A+A')/2)
endfunction

function choleskyeigen (A, eps=none)
## Iterates the Cholesky-iteration, until the diagonal converges.
##
## A must be positive definite symmetric and at least 2x2.
##
## See: cholesky
	if eps localepsilon(eps); endif;
	L=cholesky(A);
	B=L'.L;
	d=diag(B,0);
	repeat
		L=cholesky(B);
		B=L'.L;
		dnew=diag(B,0);
		if all(dnew~=d) then break; endif;
		d=dnew;
	end;
	return dnew;
endfunction

// The singular value decomposition of Euler is based on the builtin
// function svd. It is used to compute an orthogonal basis of the
// kernel and the image of a matrix, the condition of a matrix, or the
// pseudo-inverse.

function svdkernel (A:real)
## Computes the kernel of the quadratic matrix A
##
## This function is using the singular value decomposition, and works
## for real matrices only. 
##
## Returns an orthogonal basis of the kernel as columns of a matrix.
##
## See: svd, kernel
	{B,w,V}=svd(A);
	i=nonzeros(w~=0);
	if cols(i)==0; return zeros(cols(V),1);
	else return V[:,i];
	endif;
endfunction

function svdimage (A:real)
## Computes the image of the quadratic matrix A
##
## This function is using the singular value decomposition, and works
## for real matrices only.
##
## Returns an orthogonal basis of the image as columns of a matrix.
## This can be used to compute an orthogonal basis of vectors.
##
## >shortformat; A=random(3,2)
##   0.493453  0.601344 
##   0.659461  0.967468 
##   0.193151  0.935921 
## >svdimage(A)
##  -0.441518  -0.45828 
##  -0.357159  -0.69891 
##   0.823103 -0.549094 
##
## See: orthogonal, svd, kernel, image, svdkernel
	{B,w,V}=svd(A);
	i=nonzeros(!(w~=0));
	if cols(i)==0; return zeros(1,cols(A));
	else return B[:,i];
	endif;
endfunction

function svdcondition (A:real)
## Condition number based on a singular value decomposition of A
##
## Returns the quotient of the largest singular value divided by the
## smallest. 0 means singularity.
##
## A : real matrix
	{B,w,V}=svd(A);
	if any(w~=0); return 0; endif;
	return max(abs(w))/min(abs(w));
endfunction

function svddet (A:real)
## Determinant based on a singular value decomposition of A
##
## A : real matrix
##
## See: svd, det
	{B,w,V}=svd(A);
	return prod(w);
endfunction

function svdeigenvalues (A:real)
## Eigenvalues or singular values of A
##
## For a symmetric A, this returns the eigenvalues of A For a
## non-symmetric A, the singular values.
##
## A : real matrix
##
## See: eigenvalues
	{B,w,V}=svd(A);
	return w;
endfunction

function svdeigenspace (A:real,l:real)
## Returns the eigenspace of A to the eigenvalue l
##
## See: eigenspace
	k=svdkernel(A-l*id(cols(A)));
	if all(k~=0) then error("No eigenvalue found!"); endif;
	si=size(k);
	loop 1 to si[2];
		x=k[:,index()];
		k[:,index()]=x/sqrt(x'.x);
	end;
	return k;
endfunction

function svdsolve (A:real,b:real)
## Minimize |A.x-b| with smallest norm for x
##
## The singular value decomposition is one way to solve the fit
## problem. It has the advantage, that the result will be the result
## with smallest norm, if there is more than one solution.
##
## A : real matrix
##
## See: fit
	{B,w,V}=svd(A);
	i=nonzeros(!(w~=0));
	if (cols(i)>0); w[i]=1/w[i]; endif;
	return V.diag(size(V),0,w).(B'.b);
endfunction

function svdinv (A:real)
## The pseudo-inverse of A.
##
## The pseudo-inverse B of a matrix solves the fit problem to minimize
## |Ax-b| by x=B.b. It is computed in this function using an svd
## decomposition.
##
## See: pseudoinv
	return svdsolve(A,id(rows(A)))
endfunction

function pinv (A:real)
## The pseudo-inverse of A.
##
## Alias to svdinv
##
## See: svdinv
	return svdsolve(A,id(rows(A)))
endfunction

function ginv (A:real)
## Gilbert inverse of a matrix A
##
## This inverse has the property A.G.A=A
	i=shuffle(1:rows(A));
	H=A[i];
	j=shuffle(1:cols(A));
	H=H[:,j];
	{B,C,c}=qrdecomp(A);
	H=H[:,nonzeros(c)]';
	{B,C,c}=qrdecomp(A);
	H=H[:,nonzeros(c)]';
	n=min(size(H));
	H=inv(H[1:n,1:n])';
	Z=zeros(size(A));
	Z[i[1:n],j[1:n]]=H;
	return Z';
endfunction

// * Gauss-Jordan Scheme

// Some functions for the demonstration of the Gauss-Jordan algorithm.
// These functions can be used for linear systems or for the simplex
// algorithm.

// See: ../Programs/17 - Optimization.html#The_Gauss-Jordan_Scheme | Example

defaultgjdigits:=3;
defaultgjlength:=10;

function gjprint (A : real, n : positive integer vector,
	v : string vector, digits:nonnegative integer=none,
	length:index=none)
## Print the matrix A in Gauss-Jordan form.
##
## The scheme A is printed in the form
##
##    x  y ...
## a  1  2 ...
## b  3  4 ...
## ...
##
## n : index vector with a permumatin of the variables
## v : variable names with the column variables first
## digits : number of digits for each number
## length : output length of each number
##
## See: gjstep
	global defaultgjdigits,defaultgjlength;
	if digits==none then digits=defaultgjdigits; endif;
	if length==none then length=defaultgjlength; endif;
	ma=rows(A); na=cols(A); nv=cols(n)-ma;
	s=printstr("",length);
	for j=1 to nv;
		s=s|printstr(v[n[ma+j]],length);
	end;
	s,
	for i=1 to ma;
		s=printstr(v[n[i]],length);
		for j=1 to na;
			s=s|print(A[i,j],digits,length);
		end;
		s,
	end;
endfunction

function gjstep (A:numerical, i:index, j:index,
	n:positive integer vector,
	v:string vector=none, digits:nonnegative integer=none,
	length:index=none)
## Make A[i,j]=1 and all other elements in column j equal to 0.
##
## The function is using elementary operations on the rows of A. Use
## this functions for the demonstration of the Gauss_Jordan algorithm.
##
## The function modifies A and n.
##
## n : A row vector of indices, the function assumes the Gauss-Jordan
## form. n contains the indices of the variables, first the indices of
## the variables in the rows, and then the indices of the variables in
## the columns. The row variable i is exchanged with the column
## variable j. This is the same as making the j-th column to an
## canonical vector e[i], and inserting in the j-th column the result
## of this operation applied to e[i].
##
## You can add a vector v of strings, which contains the names of the
## variables. The problem will then be printed using gjprint.
##
## See: pivotize, gjprint
	global defaultgjdigits,defaultgjlength;
	if digits==none then digits=defaultgjdigits; endif;
	if length==none then length=defaultgjlength; endif;
	m=rows(A);
	b=zeros(m,1); b[i]=1;
	k=1:m; k=nonzeros(k<>i);
	b[k]=b[k]-A[k,j]/A[i,j]*b[i];
	A[k]=A[k]-A[k,j]/A[i,j]*A[i];
	b[i]=b[i]/A[i,j];
	A[i]=A[i]/A[i,j];
	A[:,j]=b;
	h=n[i]; n[i]=n[m+j]; n[m+j]=h;
	if v<>none then
		gjprint(A,n,v,digits,length);
	endif
endfunction

function gjsolution (M : real, n : positive integer vector)
## Read the values of the solution from the Gauss-Jordan scheme.
##
## For this, we assume that the last column contains the values, and
## the variables in the top row are set to zero.
##
## See: gjstep
	m=rows(M);
	x=M[:,-1]'|zeros(1,cols(n)-m); x=x[n];
	return x[(m+1):cols(n)];
endfunction

function gjvars (M : real, simplex=false)
## Generate variable names for M
##
## simplex : If true the last row variable will be named ""
##           and the last column will name will be missing.
##
## Returns {v,n}
##    where n=1:(rows+cols), v=["s1"...,"x1",...]
##
## See: gjprint
	n=rows(M);
	m=cols(M); if simplex then m=m-1; endif;
	nres=1:(n+m);
	v=["s1"];
	loop 2 to n-1; v=v|("s"+#); end;
	if simplex then v=v|""; else v=v|("s"+n); endif;
	loop 1 to m; v=v|("x"+#); end;
	return {v,nres}
endfunction

function gjaddcondition (M:real, 
	n:positive integer vector, v:string vector,
	line:real vector, varname:string)
## Add a condition to the Gauss-Jordan Form
##
## The line for the condition (left side <= right side) and the
## variable name must be provided. The condition uses the current top
## row variables.
##
## Return {M,n,v}
	r=rows(M)-1; c=cols(M);
	M=M[1:r]_line_M[-1];
	v=v|varname;
	n=n[1:r]|(cols(n)+1)|n[r+1:cols(n)];
	return {M,n,v};
endfunction

// * Additional Matrix Functions

function comment magic (n)
## Magic square of size nxn.
endfunction
