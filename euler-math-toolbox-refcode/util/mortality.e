// Mortality and Insurance

// Load with "load mortality". Help with "help mortality.e".

comment
Functions to compute life insurances using a table of mortality. The
tables are containted in the files pm.dat and pf.dat.

For more information try "help mortality", see the example notebook,
or see the snippets in the user menu.
endcomment

qm=0; qm0=0; qf=0; // will be initialized with inittables

function %inittables (male="male.dat",female="female.dat")
	global qm,qm0,qf; 
	open(male,"r"); qm=getvector(101); close();
	qm0=qm[1]; qm=tail(qm,2); 
	open(female,"r"); qf=getvector(101); close();
	qf0=qf[1]; qf=tail(qf,2);
endfunction

%inittables();

male=0; female=1;

function map ptodiewithin (agenow:integer, age:integer, gender=male)
## Probablity to between the two ages
##
## agenow : current age
## age : later age (inclusive)
## gender : male (0) or female (1)
	if age>100 then return 1 endif;
	global qm,qf;
	if gender then p=qf; else p=qm; endif;
	if agenow>=age then return p[agenow];
	else return p[agenow]+sum(p[agenow+1:age]*cumprod(1-p[agenow:age-1]))
	endif
endfunction

function map ptodiein (agenow:integer, age:integer, gender=male)
## Probablity to die in a specific age
##
## agenow : current age
## age : age of death
## gender : male (0) or female (1)
	if age>100 then return 0 endif;
	global qm,qf;
	if gender then p=qf; else p=qm; endif;
	if agenow>age then return 0;
	elseif agenow==age then return p[agenow];
	else return p[age]*prod(1-p[agenow:age-1])
	endif
endfunction

function map lifeexpect (age:integer, gender=male)
## Expected life span
##
## age : current age
## gender : male (0) or female (1)
	global qm,qf;
	if gender then p=qf; else p=qm; endif;
	return p[age]*age+ ..
		sum(p[age+1:100]*cumprod(1-p[age:99])*[age+1:100])+ ..
		prod(1-p[age:100])*101;
endfunction

function map pension (agenow:integer, agestart:integer, ..
	interest:number, gender=male)
## Value of an eternal pension
##
## We assume the pension stops at age 100.
##
## agenow : the current age
## agestart : the age, the pension starts
## interest : the interest rate (as a fraction, e.g. 5%)
## gender : male (0) or female (1)
##
## Returns the value of 1 monetary unit
	ps=1-ptodiewithin(agenow,agestart-1:100,gender);
	return sum(ps/(1+interest)^(agestart-agenow:101-agenow));
endfunction

function pensionv (age:integer, v:real vector, ..
	interest:number, gender=male, pay=0)
## Value of payments v[1],...,v[n]
##
## The functions assumes, that the payments start after one year, but
## stop as soon as the person dies. There are at most length(v)
## payments.
##
## age : current age
## v : vector of payments
## interest : the interest rate (as a fraction, e.g. 5%)
## gender : male (0) or female (1)
## pay : last payment will always be payed
	global qm,qf;
	if gender then p=qf; else p=qm; endif;
	n=length(v); 
	if age+n>100 then n=100-age; v=v[1:n]; endif;
	return sum(cumprod(1-p[age:age+n-1])*v ..
		/(1+interest)^(1:n));
endfunction

function lifeinsurance (agenow:integer, ageend:integer, ..
	interest:number, gender=male, pay=1)
## Risk of a life insurance
##
## This computes the value of a life insurance with payment 1,
## starting at an age, and payable at death or an end age (pay=1).
## If pay=0, then the end amount will not be payed, and the
## amount will only be payed, if the person dies BEFORE the
## ending age.
##
## agenow : starting age of the insurance
## ageend : ending age of the insurance
## interest : the interest rate (as a fraction, e.g. 5%)
## gender : male (0) or female (1)
## pay : amount will be payed at end (1) or not (0)
	global qm,qf;
	if gender then p=qf; else p=qm; endif;
	if pay then
		return p[agenow]/(1+interest)+ ..
			sum(cumprod(1-p[agenow:ageend-1])* ..
				(p[agenow+1:ageend-1]|1) ..
				/(1+interest)^(2:ageend+1-agenow));
	else
		return p[agenow]/(1+interest)+ ..
			sum(cumprod(1-p[agenow:ageend-1])* ..
				p[agenow+1:ageend] ..
				/(1+interest)^(2:ageend+1-agenow));
	endif;
endfunction

function payments (agenow:integer, ageend:integer, ..
	interest:number, gender=male)
## Value of payments till end age or death
##
## agenow : start age of payments
## ageend : last payment at that age
## interest : the interest rate (as a fraction, e.g. 5%)
## gender : male (0) or female (1)
	global qm,qf;
	if gender then p=qf; else p=qm; endif;
	return 1+..
		sum(cumprod(1-p[agenow:ageend-1])/ ..
			(1+interest)^(1:ageend-agenow));
endfunction
	
submenu Mortality and Insurance
addmenu ptodiewithin(?startAge,?endIncluding,male)
addmenu ptodiewithin(?startAge,?endIncluding,female)
addmenu ptodiein(?ageNow,?inAge,?male)
addmenu lefeexpect(?ageNow,?male)
addmenu pension(?ageNow,?startAge,?interestRate,?male)
addmenu pensionv(?startAge,?vectorOfPayments,?interestRate,?male)
addmenu lifeinsurance(?ageNow,?ageEnd,?interestRate,?male,?payAtEnd)
addmenu payments(?ageNow,?ageEndIncluded,?interestRate,?male)
