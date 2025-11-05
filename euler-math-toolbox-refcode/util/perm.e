// Permutations

comment
Functions for permutations.
endcomment

// * Permutations

// Load this file with "load perm".

pn := 10;

function pid (n)
## Identity permutation of n elements.
	return 1:n
endfunction

function pmult (p,q)
## Multiply two permutations.
	return p[q]
endfunction

function operator pm (p,q)
## Multiply two permutations.
##
## >pprint ptranspose(2,3,10) pm ptranspose(3,4,10)
##   (2 3 4)
	return p[q]
endfunction

function ptranspose (i,j,n=none)
## Transposition of i and j in n elements.
	global pn;
	if n==none then n=pn; endif;
	p=pid(n);
	p[i]=j; p[j]=i;
	return p
endfunction

function prefix pprint (p)
## Print a permutation as product of cycles.
	q=zeros(size(p));
	s="";
	loop 1 to cols(p)
		if !q[#] and p[#]<># then
			s=s|"(";
			k=#;
			s=s|print(k,0,0);
			repeat
				q[k]=1;
				k=p[k];
				if k==# then break; endif;
				s=s|" "|print(k,0,0);
			end;
			s=s|")";
		endif
	end;
	if !any(q==1) then s="()"; endif;
	return s;
endfunction

function pcycle (q,n=none)
## Transform a cycle q into a permutation of n elements.
	global pn;
	if n==none then n=pn; endif;
	p=pid(n);
	if cols(q)==0 then return p; endif;
	k=cols(q);
	loop 1 to k-1
		p[q[#]]=q[#+1];
	end
	p[q[k]]=q[1];
	return p;
endfunction

function pinverse (p)
## Inverse of the permutation p.
	{h,i}=sort(p);
	return i;
endfunction

function pmultcycles
## Multiply all cycles in the argument list (right to left).
	k=args(argn);
	p=pid(k);
	loop 1 to argn-1
		p=pmult(p,pcycle((args(#)),k));
	end;
	return p;
endfunction

function %forAllPermRek (p,k,f,n)
	if k>n then f(p);
	else
		%forAllPermRek(p,k+1,f,n);
		loop k+1 to n
			h=p[#]; p[#]=p[k]; p[k]=h;
			%forAllPermRek(p,k+1,f,n);
			h=p[#]; p[#]=p[k]; p[k]=h;
		end	
	endif;
endfunction

function forAllPerm (f,n)
## Run f(p) for all permutations p of n elements.
	p=1:n;
	%forAllPermRek(p,1,f,n);
endfunction

function %forAllChoicesRek (p,k,f,n,m)
	if k>m then f(p[1:m]);
	else
		%forAllChoicesRek(p,k+1,f,n,m);
		loop k+1 to n
			h=p[#]; p[#]=p[k]; p[k]=h;
			%forAllChoicesRek(p,k+1,f,n,m);
			h=p[#]; p[#]=p[k]; p[k]=h;
		end	
	endif;
endfunction

function forAllChoices (f,n,m)
## Run f(p) for all choices p of m elements out of 1:n
	p=1:n;
	%forAllChoicesRek(p,1,f,n,m);
endfunction

function %forAllTripsRek (p,k,f,n)
	if k>n then f(p);
	else
		loop 1 to k-1
			p[k]=p[#]; p[#]=k;
			%forAllTripsRek(p,k+1,f,n);
			p[#]=p[k]; p[k]=k;
		end	
	endif;
endfunction

function forAllTrips (f,n)
## Run f(p) for all trips p of n elements.
	p=1:n;
	%forAllTripsRek(p,2,f,n);
endfunction

function %forAllOrderedChoicesRek (p,k,f,n,m)
	if k>=m then f(p);
	else
		loop p[k]+1 to n-m+k+1
			p[k+1]=#;
			%forAllOrderedChoicesRek(p,k+1,f,n,m);
		end	
	endif;
endfunction

function forAllOrderedChoices (f,n,m)
## Run f(p) for all ordered tuples of m elements in 1 to n.
	p=1:m;
	loop 1 to n-m+1
		p[1]=#;
		%forAllOrderedChoicesRek(p,1,f,n,m);
	end;
endfunction
