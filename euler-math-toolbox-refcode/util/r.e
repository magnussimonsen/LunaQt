// Some R Commands in Euler

comment
Implements some R Commands in Euler (very incompletely).
endcomment

// This is not meant to be an R environment. It just steals some
// functions from R, which I consider useful for Euler for users
// coming from R.

// Some elements contradict the Euler philosophy. E.g., the function
// c() might easily confuse with variables c, or T,F for true and
// false might be better used for other variables. Other functions
// would overwrite Euler functions and destroy the functionality of
// Euler. These functions have been renamed. E.g., plot is called
// rplot.

// Load this with "load r". Help with "help r.e".

T:=1;
F:=0;

function c ()
## Make a vector from the elements of the arguments.
##
## The arguments can be scalars, vectors or matrices. Matrices are
## reformed to a vector. So just all elements of the matrices are
## added.
	v=[];
	loop 1 to argn;
		A=args(#);
		k=prod(size(A));
		if k==1 then v=v|A;
		else v=v|redim(A,1,k);
		endif;
	end;
	return v;
endfunction

function rep (v,n)
## Repeat the vector v n times
##
## n : If it is a number the vector n is repeated n times. If it is a
## vector it must match the size of v, and the elements of v are
## repeated as often as the correspoding element of n says.
	if typeof(n)==0 then return redim(dup(v,n),1,n*cols(v));
	else 
		w=[];
		loop 1 to cols(n);
			w=w|dup(v[#],n[#])';
		end;
		return w;
	endif;
endfunction

function seq (a, b, by=1, length:positive integer=none)
## Make a sequence with step size by or total length.
##
## by : an optional step size
## length : if this is specified the vector will contain n=length
## equispaced points between a and b.
	if length!=none then
		return linspace(a,b,length-1);
	else
		return a:by:b;
	endif;
endfunction

function matr (A, ncol=none, nrow=none, byrow=F)
## Make a matrix from the elements of A
##
## The matrix A is reformed to a matrix with ncol columns or nrow
## rows.
##
## byrow : If true, the elements are inserted row by row, else column
## by column
	if byrow then
		if nrow!=none then
			return redim(A,nrow,prod(size(A))/nrow);
		elseif ncol!=none then
			return redim(A,prod(size(A))/ncol,ncol);
		else
			return A;
		endif;
	else
		if nrow!=none then
			return redim(A',prod(size(A))/nrow,nrow,)';
		elseif ncol!=none then
			return redim(A',ncol,prod(size(A))/ncol)';
		else
			return A;
		endif;
	endif;
endfunction

function cbind ()
## Add matrices horizontally (binding columns)
	v=[];
	loop 1 to argn;
		A=args(#);
		if rows(A)==1 then v=v|A';
		else v=v|A;
		endif;
	end;
	return v;
endfunction

function rbind ()
## Add matrices vertically (binding rows)
	m=0;
	loop 1 to argn;
		A=args(#);
		m=max(m,cols(A));
	end;
	v=zeros(1,m);
	loop 1 to argn;
		A=args(#);
		v=v_A;
	end;
	return v[2:rows(v)];
endfunction

function summary (A)
## Summary of quantiles and mean of A.
##
## The minimal and maximal value, the 0.25,0.5,0.75 quantiles and the
## mean are printed.
	if rows(A)>1 then A=A'; endif;
	write("Min: "); min(A)',
	write("1st: "); median(A,p=0.25)',
	write("Med: "); median(A,p=0.5)',
	write("Mea: "); mean(A)',
	write("3rd: "); median(A,p=0.75)',
	write("Max: "); max(A)',
endfunction

function t (A)
## Transpose of A.
	return A'
endfunction

function apply (A:real, where:integer, what:string)
## Apply function "what" to where=1,2 (col or row)
	if where==1 then
		v=zeros(1,rows(A));
		loop 1 to rows(A);
			v[#]=what(A[#];args());
		end;
		return v;
	elseif where==2 then
		v=zeros(1,cols(A));
		loop 1 to cols(A);
			v[#]=what(A[:,#]';args());
		end;
		return v;
	else
		error("Row=1 or Column=2 operation only");
	endif;
endfunction

// * Distributions

function dnorm (x, m=0, sigma=1)
## Gauß density with mean m and deviation sigma
	return qnormal(x,m,sigma);
endfunction

function pnorm (x, m=0, sigma=1)
## Gauß distribution with mean m and deviation sigma
	return normaldis((x-m)/sigma);
endfunction

function qnorm (x, m=0, sigma=1)
## Gauß distribution with mean m and deviation sigma
	return invnormaldis(x)*sigma+m;
endfunction

function rnorm (n:integer, m=0, sigma=1)
## Simulate the Gauß normal distribution n times.
	return m+normal(1,n)*sigma;
endfunction

function rplot (x,y=none,rtype="p",style=none)
## Plots x and y.
##
## 'p' : point plot
## 'l' : line plot
## 'b' : both
## 'h' : histogram plot
## 's' : surface plot
##
## See: statplot
	mfadvance();
	if style!=none then fillstyle=style; else fillstyle="O"; endif;
	if y==none then y=x; x=1:rows(y); endif;
	if rtype=="p" then
		plot2d(x,y,>points);
	elseif rtype=="l" then
		plot2d(x,y);
	elseif rtype=="b" then
		plot2d(x,y);
		plot2d(x,y,>points,>add);
	elseif rtype=="h" then
		plot2d(makeimpulse(x,y),>bar,style=fillstyle);
	elseif rtype=="s" then
		plot2d(x,y,>bar,style=fillstyle);
	else
		error("Unknown type (use p,l,b).");
	endif;
endfunction

mf:=[1,1,1,1];

function par (mfrow=none, mfcol=none)
## Sets plot parameters.
##
## The following splits the screen to plot fields. The plot functions
## will automatically advance to the next field. If write an own plot
## routine, use mfadvance() to advance to the next field. Use
## h=holding(1) to stop the plots from beeing deleted, and holding(h)
## to restore the hold state.
##
## mfrow : multi frames with [n,m] fields filled by rows
## mfcol : multi frames with [n,m] fields filled bycols
	if mfrow!=none then
		global mf;
		mf[1:2]=mfrow;
		mf[3]=1;
		mf[4]=1;
		if mf[1]>1 or mf[2]>1 then
			figure(mf[1],mf[2]);
		else
			figure(0);
		endif;
	elseif mfcol!=none then
		global mf;
		mf[1:2]=mfcol;
		mf[3]=1;
		mf[4]=2;
		if mf[1]>1 or mf[2]>1 then
			figure(mf[1],mf[2]);
		else
			figure(0);
		endif;
	endif;
endfunction

function mfadvance ()
## Advance to the next plot field.
	global mf;
	if mf[1]==1 and mf[2]==1 then return; endif;
	if mf[3]>mf[1]*mf[2] then mf[3]=1; endif;
	if mf[4]==1 then
		figure(mf[3]);
	else
		col=floor((mf[3]-1)/mf[1]);
		row=mf[3]-col*mf[1]-1;
		figure(row*mf[2]+col+1);
	endif;
	mf[3]=mf[3]+1;
endfunction

function rhist (x,style="O")
## Histogram of the data in x.
	{t,s}=histo(x);
	mfadvance();
	plot2d(t,s,>bar,style=style);
endfunction

function rboxplot ()
## Summary of the data in graphical form.
##
## Shows the 0.25,0.5,0.75 quantiles.
##
## style : If present, it is used as fill style, the default is "O",
## an unfilled style. Try "0#", "/", "\", "\/", "|".
##
## See: boxplot
	mfadvance();
    m1=1e30; m2=-1e30;
	loop 1 to argn
		v=getarg(#);
		m1=min(min(v),m1);
		m2=max(max(v),m2);
	end
	fillstyle="O";
	m=(m1+m2)/2; d=(m2-m1)/2*1.05;
	h=holding(1);
	setplot(0,argn+1,m-d,m+d);
	frame();
	xgrid(1:argn,grid=4);
	ygrid(ticks(m1,m2));
	loop 1 to argn
		v=args(#);
		m1=min(v); m2=max(v);
		st=style("--"); plot([#,#],[m1,m2]); style(st);
		st=style("-"); 
		plot([#-0.2,#+0.2],[m1,m1]); 
		plot([#-0.2,#+0.2],[m2,m2]); 
		style(st);
		st=barstyle(fillstyle);
		plotbar(#-0.4,median(v,p=0.25),0.8,median(v,p=0.75)-median(v,p=0.25));
		barstyle(st);
		m=median(v);
		st=style("-"); plot([#-0.4,#+0.4],[m,m]); style(st);
	end
	holding(h);
endfunction

function abline (a,b=none,color=1)
## Add a regression line to the plot
	if b==none then b=a[2]; a=a[1]; endif;
	h=holding(1); cl=color(color); 
	pl=plot(); plot(pl[1:2],polyval([a,b],pl[1:2])); 
	holding(h); color(cl);
endfunction
