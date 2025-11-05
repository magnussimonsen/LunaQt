// Interest Rates

submenu Interest Rates

comment
Computes interest rates for investments.
endcomment

// Load these functions with "load interest".

// * Interest Rates

function rate (x:vector, n:vector=none)
## Compute the interest rate in % for an investment with returns.
##
## x : Vector of payments (negative) or income (positive) at evenly
##   spaced periods or at times in the vector n.
## n : times of payments (default is yearly).
##
## >load interest;
## >rate([1000,-50,-50,-50,-50,-1050])+"%" // savings
##  5%
## >rate([-1000,0,0,0,0,1200])+"%" // accumulated interest
##  3.71372893366%
## >rate(100000|(-10150*ones(1,20))) // 20 year loan
##  7.95349754169
## >rate([-1200,0,0,0,2100]), rate([-1200,2100],[0,4])
##  15.0163316896
##  15.0163316895
## 
## See: simrate
	if sum(x)<0 then x=-x; endif;
	if n<>none then
	    k2=x*n; n2=n-1;
	    k2=k2[2:length(x)]; n2=n2[2:length(x)];
	    c=1;
	    repeat
	        cnew=c-sum(x*c^n)/sum(k2*c^n2);
	        if c~=cnew; break; endif;
	        c=cnew;
	    end;	
	else
	    q=polydif(x);
	    c=1;
	    repeat
	        cnew=c-polyval(x,c)/polyval(q,c);
	        if c~=cnew; break; endif;
	        c=cnew;
	    end;
	endif;
    return (1/cnew-1)*100;
endfunction

addmenu rate(?vectorOfPayments)
addmenu rate(?vectorOfPayments,?timeVector)

function simrate (v:vector, P:positive real, digits=none)
## Simulates payments in vector v at rate P in %
##
## >v=simrate(100000|(-10150*ones(1,20)),7.96,2)
##  [ 100000  97810  95445.68  92893.16  90137.46  87162.4  83950.53
##  80482.99  76739.44  72697.9  68334.65  63624.09  58538.57  53048.24
##  47120.88  40721.7  33813.15  26354.68  18302.51  9609.39  224.3 ]
## >columnsplot(v);
##
## See: rate
	sum=0; x=v;
	loop 1 to length(x);
		sum=sum*(1+P/100)+v[#];
		if digits<>none then sum=round(sum,digits); endif;
		x[#]=sum;
	end;
	return x;
endfunction

addmenu simrate(?vectorOfPayments,?interestRate)
addmenu simrate(?vectorOfPayments,?interestRate,digits=?digits)

function %rateformula (f,k0,r,k1,n,i0,i1)
	return k0*f^n+r*f^i1*(f^(n-i0-i1+1)-1)/(f-1)+k1
endfunction

function map investment (start, instalment, final, n; i0=1, i1=1)
## Computes the interest rate of a loan and other investments.
##
## Note that payments are negative!
##
## start: start investment at period 0
## instalment: rate payed at the start of periods i0 to n-i1 (by
## default n-1 rates)
## final: that is the final dept after n periods
##
## >print(investment(1000,-50,-1050,10),2,unit="%")
##        5.00%
## >print(investment(1000,0,-1200,5),2,unit="%")
##        3.71%
## >print(investment(100000,-10150,0,21),2,unit="%")
##        7.95%
##
## The result is the interest rate in %.
##
## See: rate
	return (bisect("%rateformula",1.0000001,2;start,instalment, ..
			final,n,i0,i1)-1)*100;
endfunction

addmenu investment(?investment,?returns,?finalReturn,?years,?ystart,?yend)
addmenu investment(?start,?savings,?finalReturn,?years,?ystart,?yend)

function finaldebt (loan,periods,rate,payment,start=1)
## Compute the final dept of a loan after periods of payments.
##
## See: rate
	f=1+rate/100;
	return loan*f^periods-payment*(f^(periods-start+1)-1)/(f-1);
endfunction

addmenu finaldept (?loan,?periods,?rate,?payment)
