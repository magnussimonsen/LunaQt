// Sets

comment
Functions for sets.
endcomment

function union ()
## Union of the argument vectors
	a=unique(getarg(1));
	loop 2 to argn();
		a=unique(a|getarg(#));
	end
	return a;
endfunction

function intersection ()
## Intersection of the argument vectors
	a=unique(getarg(1));
	loop 2 to argn();
		i=indexof(a,getarg(#));
		a=unique(a[i[nonzeros(i)]]);
	end;
	return a;
endfunction

function difference (universe, ...)
## Difference of universe and all argument vectors
	a=unique(universe);
	loop 2 to argn();
		i=indexof(a,getarg(#));
		a=drop(a,i);
	end;
	return a;
endfunction
	

