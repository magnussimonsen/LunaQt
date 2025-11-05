// D'Hondt and Hare-Niemeyer

comment
D'Hondt and Hare-Niemeyer algorithm.
endcomment

// Load with "load hondt".

function hondt (v,n)
## Distributions of seats with d'Hondt
##
## Compute the distribution of seats s[1],...s[k], which corresonds
## to v[1],...,v[k], such that sum s[i] = n, using d'Hondt.
##
## See: hnbest
	k=length(v);
	t=1/(1:n); S=[]; I=[];
	loop 1 to k;
		S=S|(v[#]*t); I=I|(dup(#,n)');
	end;
	{S,i}=sort(-S);
	I=I[i]; I=I[1:n];
	return count(I-0.5,k);
endfunction

function hnbest (v,n)
## Best distribution of seats with Hare-Niemeyer
##
## Compute the distribution of seats s[1],...s[k], which corresonds
## to v[1],...,v[k], such that sum s[i] = n, using closest fit.
## The method minimizes the sum of absolute errors and is
## often called Hare-Niemeyer method.
##
## See: hondt
	s=v/sum(v)*n; S=floor(s);
	d=n-sum(S);
	if d>0;
		{f,i}=sort(S-s); i=i[1:d];
		S[i]=S[i]+1;
	endif;
	return S;
endfunction

submenu d'Hondt
addmenu hondt(?v,?n)
addmenu hnbest(?v,?n)
