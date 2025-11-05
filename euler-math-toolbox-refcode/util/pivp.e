// Print Functions for Simplex in Gauss-Jordan Form

comment
This file contains functions to print a Simplex scheme in Gauss-Jordan
form. I.e. we print only the columns of M|b that are not in the basis
J. The columns in the basis J must contains the identity matrix.

This is nice if the problem starts with Ax<=b, and the scheme is
M=A|I.

The target function is printed underneath the scheme as in

>A=[1,4,3;4,5,1]; b=[5,3]'; c=[1,2,3];
>fraction simplex(A,b,c,>max)
      4/11 
         0 
     17/11 
>M=(A|id(2)|b)_c; J=[4,5];
>load pivp;
>pivp(M,J,8)
            1       2       3
 
    4       1       4       3       5
    5       4       5       1       3
 
            1       2       3       0
>piv(M,2,1,J,8)
            2       3       5
 
    4    11/4    11/4    -1/4    17/4
    1     5/4     1/4     1/4     3/4
 
          3/4    11/4    -1/4    -3/4
>piv(M,2,2,J,8)
            1       3       5
 
    4   -11/5    11/5    -4/5    13/5
    2     4/5     1/5     1/5     3/5
 
         -3/5    13/5    -2/5    -6/5
>piv(M,1,3,J,8)
            1       4       5
 
    3      -1    5/11   -4/11   13/11
    2       1   -1/11    3/11    4/11
 
            2  -13/11    6/11  -47/11
>piv(M,2,1,J,8)
            2       4       5
 
    3       1    4/11   -1/11   17/11
    1       1   -1/11    3/11    4/11
 
           -2      -1       0      -5
>fraction pivsol(M,J)'
      4/11 
         0 
     17/11 
         0 
         0 
>
endcomment

// Load with "load hondt".

function pfrac (x,n)
## Print a row vector in fractional format.
##
## n : Number of digits for each element.
	s="";
	for i=1 to cols(x)
		s=s+printf("%"+n+"s",frac(x[i]));
	end;
	return s;
endfunction

function pivp (H,b,l=6)
## Print the Simplex scheme in Gauss-Jordan fractional Form
##
## H : Scheme Mx=b with a target function in last row.
## b : Indices of Basis columns (must contain identity matrix)
## l : length of element output
##
## >A=[1,4,3;4,5,1]; b=[5,3]'; c=[1,2,3];
## >fraction simplex(A,b,c,>max)
##       4/11 
##          0 
##      17/11 
## >M=(A|id(2)|b)_c; J=[4,5];
## >load pivp;
## >pivp(M,J,8)
##             1       2       3
##  
##     4       1       4       3       5
##     5       4       5       1       3
##  
##             1       2       3       0
##
## See: piv, pivsol
	n=cols(H)-1;
	m=drop(1:n,b);
	s="    ";
	for j=1 to cols(m);
		s=s+pfrac(m[j],l); 
	end;
	s,
	"", 
	for i=1 to rows(H)-1;
		pfrac(b[i],4)+pfrac(H[i,m|(-1)],l),
	end;
	"",
	"    "+pfrac(H[-1,m]|H[-1,-1],l),
endfunction

function piv (H,i,j,b,l=6)
## Pivotize Simplex Scheme in Gauss-Jordan form
##
## H : Scheme Mx=b with a target function in last row.
## b : Indices of Basis columns (must contain identity matrix)
## i : Basis element to be excluded (row of H)
## j : Basis element to be included (column of H)
## l : length of element output
##
## >pivp(M,J,8)
##             1       2       3
##  
##     4       1       4       3       5
##     5       4       5       1       3
##  
##             1       2       3       0
## >piv(M,2,1,J,8)
##             2       3       5
##  
##     4    11/4    11/4    -1/4    17/4
##     1     5/4     1/4     1/4     3/4
##  
##           3/4    11/4    -1/4    -3/4
##
## See: pivp, pivsol, addcut
	pivotize(H,i,j);
	b[i]=j;
	pivp(H,0+b,l);
endfunction

function pivsol (M,b)
## Compute the solution from Gauss-Jordan form
##
## M : Scheme Mx=b with a target function in last row.
## b : Indices of Basis columns (must contain identity matrix)
##
## >piv(M,2,1,J,8)
##             2       4       5
##  
##     3       1    4/11   -1/11   17/11
##     1       1   -1/11    3/11    4/11
##  
##            -2      -1       0      -5
## >fraction pivsol(M,J)'
##       4/11 
##          0 
##      17/11 
##          0 
##          0 
##
## See: piv, pivp
	x = zeros(cols(M)-1);
	x[b] = M[1:rows(M)-1,-1]';
	return x
endfunction

function addcut (M,b,c)
## Add a Gomory Cut to the Simplex in Gauss-Jordan form
##
## M : Scheme Mx=b with a target function in last row.
## b : Indices of Basis columns (must contain identity matrix)
##
## The cut is derived from the c-th row of the scheme and added
## as one line of M.
##
## See: piv, pivp, pivsol
	cut=floor(M[c])-M[c];
	m=rows(M)-1;
	n=cols(M)-1;
	Mnew=(M[1:m,1:n]|0|M[1:m,-1]) ..
		_(cut[1:n]|1|cut[-1])_(M[-1,1:n]|0|M[-1,-1]);
	bnew=b|(n+1); 
	return {Mnew,bnew};
endfunction
