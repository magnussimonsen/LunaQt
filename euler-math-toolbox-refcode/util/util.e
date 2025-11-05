// Basic Utilities

comment
Basic utility functions.
endcomment

// This file collects some important utility functions.

// * Reset Euler

// Resetting Euler is best done with the menu item, or with Ctrl-N.
// Then Maxima is restarted too. All variables are lost. You will be
// asked to save your notebook first. After reset, Euler calls the
// reset function below.

Epsilon:=epsilon();
I$:=I;
verticallabels:=1;
E$:=E;
eulergamma:=0.577215664901532860606512;

function Pi() := pi();

none$=char(1);

function overwrite setfont (size=none,width=1,name="",
	markersize=0.2)
## Set the font relative to the screen size
##
## Small prints of graphics require a larger font size relative to the
## maximal dimension of the graphics. This function sets the font
## size. The default is about 12 pixel for the HTML export with images
## of maximal dimensions of 400.
##
## The default name is blank. This sets the default font, which is
## "Arial" when Euler starts. But the user can set the default font
## permanently in the Menu.
##
## size : Size of font.
## width : Size of the print in the same unit.
## name : Font name. Default is "Arial".
## markersize : Fraction of the text height for the marker radius.
##
## Uses the internal function _setfont(lines,name).
##
## Examples:
## setfont(11pt,10cm) : 11 point font for a print of 10cm height.
## setfont(12,400) : This is a good font size for web pages.
## setfont() : Resets to the default 1/40 of the screen.
##
## See: setfont
	if size==none then 
		res=_setfont();
		markersize(6);
	else
		res=_setfont(ceil(width/size),name);
		markersize(markersize*textheight());
	endif;
	shrinkwindow();
	return width/res;
endfunction

function resetepsilon
## Reset the default epsilon and return the current epsilon.
	global ResetEpsilon;
	return setepsilon(ResetEpsilon);
endfunction

function overwrite exec (program:string, param:string="", dir:string="", 
	print:integer=0, hidden:integer=0, wait:integer=1)
## Execute an external program
##
## This is a utility function for the built-in exec command. It runs
## an external program, and by default waits for it to finish. The
## result of the program can be returned in form of vector of strings.
## Communication is via standard named pipes. The pipes stdout and
## stderr are read as output from the program.
##
## By default, the builtin function exec will block and wait for user
## confirmation. This prevents security problems with downloaded Euler
## notebooks. The confirmation can be switched off in the options. It
## can be switched off in the first confirmation dialog for one
## session.
##
## If wait is false and print is true, the function starts the
## program, and generates pipes to and from the program. These pipes
## can later be written to with execwrite(string) or
## execwriteln(string) and read from with execread(), returning a
## vector of strings. Only one such program is available. Trying to
## start a second program with open pipes terminates the first one.
##
## program : the name of the program (plus path, if not in the system
## path).
##
## param : Parameters separated by blanks. If the parameters contain
## blanks, they must be enclosed by double quotes. Use strings as in
##
## param=''"first parameter" "second parameter"''
##
## dir : The working directory of the program. This can be used to
## locate the program instead of a full path.
##
## print : If true and wait is false, the function returns the output
## immediately in form of a vector of strings. If true and wait is
## true, the program will print the output of the program.
##
## hidden : If true, no window will be generated.
##
## wait : If true, the function will wait for the command to be
## finished, or for the escape key.
##
## Examples:
##
## >exec("cmd","/c dir /w *.en",home(),>print,>hidden,>wait)
##  Volume in Laufwerk C: hat keine Bezeichnung.
##  Volumeseriennummer: ECF3-71E6
## 
##  Verzeichnis von C:\euler\docs\Programs
##  ...
##
## >filename=eulerhome()+"test.txt"; ...
##  writematrix(random(5,5),filename,"%1.5f"); ...
##  exec("cmd","/c start "+filename,eulerhome(),<wait);
##
## >exec("cmd","",home(),>print,>hidden,<wait);
## >execwriteln("dir /w");
## >execread()
##  Microsoft Windows [Version 6.1.7601]
##  Copyright (c) 2009 Microsoft Corporation. Alle Rechte vorbehalten.
##  ...
## >execkill();
## 
## See: exec, execwrite, execwriteln, execread
	return _exec(program,param,dir,print,hidden,wait);
endfunction

// * Vector and Matrix Functions

// Functions to generate matrices or vectors with values spaced in
// various ways, as well as functions providing informations about
// vectors and matrices.

function matrix
## matrix(v1,v2,v3,...) returns a matrix with rows v1,v2,v3,...
	if argn==1 then return arg1; endif;
	v=zeros(argn,cols(arg1));
	loop 1 to argn;
		v[#]=getarg(#);
	end;
	return v;
endfunction

function equispace (a:real scalar, b:real scalar, 
	n: positive integer scalar)
## Generates n+1 arcsin spaced values in [a,b].
##
## The arcsin distribution generates points which are the extremal
## points of the Chebyshev polynomial. These points are well suited
## for discrete approximation on an interval.
##
## See: linspace,chebzeros
	m=(1-cos((0:n)*pi()/n))/2;
	return a+(b-a)*m;
endfunction

function logspace (a:positive real scalar,b:positive real scalar,
	n:positive integer scalar)
## logspace(a,b,n) generates n+1 log spaced points in [a,b]
##
## See: equispace, linspace
	return linspace(log(a),log(b),n);
endfunction

function chebzeros (a: real scalar, b: real scalar, 
	n: positive integer scalar)
## Generate the zeros of the n-th Chebyshev polynomial in [a,b]
##
## See: linspace,equispace
	m=(1-cos((1:2:2*n-1)*pi()/(2*n)))/2;
	return a+(b-a)*m;
endfunction

function length (v)
## Returns the length of a row or column vector.
##
## In fact, the function returns the maximum of the number of rows,
## and the number of columns of a matrix.
	if rows(v)==1 then return cols(v);
	elseif cols(v)==1 then return rows(v);
	else return max(size(v));
	endif;
endfunction

function differences (s)
## Computes vector of the differences s[i+1]-s[i]
##
## See: fold
	return fold(s,[-1,1]);
endfunction

function tail (s, n:integer)
## The rest of the vector s starting from the n-th element.
##
## tail(s,-2) returns the last two elements of s.
##
## See: head
	c=cols(s);
	if n>0 then return s[:,n:c];
	else return s[:,c+n+1:c];
	endif;
endfunction

function head (s, n:integer)
## The vector, up to the n-th element
##
## The function works for negative n too. head(s,-2) returns all
## elements, but the last.
##
## See: tail
	c=cols(s);
	if n>0 then return s[:,1:n];
	else return s[:,1:c+n+1];
	endif;
endfunction

function args field (x:vector, y:vector)
## Returns {X,Y} such that the X+i*Y is a rectangular grid
##
## x and y must be 1xN and 1xM vectors. 
##
## This is not necessary in most cases, since operators in the Euler
## matrix language combine x and y' automatically.
	return {dup(x,cols(y)),dup(y',cols(x))};
endfunction

// * Text Menus

// This is a simple text menu. It can be used for files with a user
// choice of options.

function menu ()
## Displays a menu and returns the number of the chosen item
##
## This is a simple menu in text form inside the text window of Euler.
## The menu items are the arguments to this function. The items should
## be labelled (a), (b), etc. The user can press the keys a, b, etc.,
## or the return key. In this case, the functin returns -1. Else the
## function returns the number of the chosen argument.
	loop 1 to argn();
		char(#+ascii("a")-1)|" : "|args(#),
	end;
	""
	"Please, press return to exit or any of these keys!"
	k=key();
	n=ascii("a");
	if k-n>=0 && k-n<=argn(); return k-n+1; endif;
	if k==13; return -1; endif;
	return 0;
endfunction

// * Number Bases

// Functions for hexadecimal or binary input and conversion.

function baseinput (s:string, b:index=16)
## Read a number in base b.
	x=0;
	dot=false;
	factor=1;
	if ascii(s)==ascii("-") then
		factor=-factor;
		s=substring(s,2,-1);
	endif;
	repeat
		if s=="" then break; endif
		c=ascii(s);
		s=substring(s,2,-1);
		if c>=ascii("0") and c<=ascii("9") then
			z=c-ascii("0"); 
			if z>=b then error("wrong digit "|char(c)); endif;
			x=x*b+z;
			if dot then factor=factor*b; endif;
		elseif c>=ascii("a") then
			z=c-ascii("a")+10; 
			if z>=b then error("wrong digit "|char(c)); endif;
			x=x*b+z;
			if dot then factor=factor*b; endif;
		elseif c>=ascii("A") then
			z=c-ascii("A")+10; 
			if z>=b then error("wrong digit "|char(c)); endif;
			x=x*b+z;
			if dot then factor=factor*b; endif;
		elseif c==ascii(".") and not dot then
			dot=true;
		else
			error("wrong digit "|char(c));
		endif;
	end
	return x/factor;
endfunction

function hex (s:string)
## Convert a hex string.
##
## See: printhex, baseinput
	return baseinput(s);
endfunction

function dual (s:string)
## Convert a hex string.
##
## See: printdual, baseinput
	return baseinput(s,2);
endfunction

function printbase (x:real scalar, base:index=16, digits=13,
	integer=false)
## Prints the real number x with mantissa and exponent to base b.
##
## The output is of the form 1.xxxxx...xxx*2^n with mantissa of
## length digits. If integer is true, the value is assumed to 
## be integer, and it prints in the form xxxxx. x is between 0 and
## base, where A=11, B=12, etc.
##
## >dual("10001001"), printdual(%,>integer)
##  137
##  10001001
## >baseinput("24256",7), printbase(%,base=7,>integer)
##  6313
##  24256
## >6+5*7+2*7^2+4*7^3+2*7^4
##  6313
##
## See: baseinput
	out="";
	if x<0 then x=-x; out=out|"-"; endif;
	if x==0; then return "0"; endif;
	if integer then
		s="";
		repeat
			x=x/base;
			c=round((x-floor(x))*base);
			if c>9 then s=char(floor(c)-10+ascii("A"))|s;
			else s=char(floor(c)+ascii("0"))|s;
			endif;
			x=floor(x);
			if x<1 then break; endif;
		end;
		return out|s;
	endif;
	if x<1 then
		n=-1; a=1/base;
		repeat
			if a<=x then break; endif;
			a=a/base; n=n-1;
		end;
	else
		n=0; a=1;
		repeat
			if a>x/base then break; endif;
			a=a*base; n=n+1;
		end;
	endif;
	x=x/a;
	if floor(x)>9 then out=out|char(floor(x)-10+ascii("A"))|".";
	else out=out|char(floor(x)+ascii("0"))|".";
	endif;
	x=x-floor(x);
	loop 1 to digits
		x=x*base;
		f=floor(x);
		if f>9 then out=out|char(f-10+ascii("A"));
		else out=out|char(f+ascii("0"));
		endif;
		x=x-floor(x); 
		endif;
	end;
	out=out|"*"|printf("%g",base)|"^"|printf("%g",n);
	return out;
endfunction

function printdual (x:real scalar, integer=false, length=1)
## Prints the real number x with dual mantissa.
##
## The output is of the form 1.xxxxx...xxx*2^n. If integer is true,
## then the output is of the form xxxxxx with x=0 or x=1.
##
## >dual("10001001"), printdual(%,>integer)
##  137
##  10001001
## >printdual(0.1)
##  1.1001100110011001100110011001100110011001100110011010*2^-4
##
## See: dual
	return printstr(printbase(x,base=2,digits=52,integer=integer),length);
endfunction

function printhex (x:real scalar, integer=false)
## Prints the real number x with hexadecimal mantissa.
##
## The output is of the form 1.xxxxx...xxx*16^n. If integer is true
## the output is of the form xxxxx where x is 0,...,9,A,B,C,D,E,F.
##
## >hex("DDFFD"), printhex(%,>integer)
##  909309
##  DDFFD
## >printhex(0.1)
##  1.999999999999A*16^-1
##
## See: hex
	return printbase(x,base=16,digits=13,integer=integer);
endfunction

// * Date and Time Functions

englishmonths$=[
	"jan","feb","mar","apr","may","jun","jul","aug","sep","oct","nov","dec"];
	
function day (y, 
	m:integer scalar=1, d:integer scalar=1,
	h:integer scalar=0, min:real scalar=0,
	german=false, us=false)
## Convert a date to a number of days.
##
## The computation is based on the Gregorian calendar. It is intended
## to be used to compute differences between dates.
## 
## The date can alternatively be a formatted string in ISO8601 format
## (yyyy-mm-dd) with an optional time (hh:mm or hh:mm:sec) appended
## with a blank. The function will detect the alternative day format
## (dd.mm.yyyy), and an English month name instead of a numerical
## value. The date order can also be set with >us to "mm-dd-yyyy", and
## with >german to "dd-mm-yyyy".
##
## >day(2020,1,1)-day(2010,1,1)
##  3652
## >day("2012-12-03 16:00")-day("2012-01-01 13:00")
##  337.125
## >day(2012,3,1,12)
##  734993.5
## >day("jan-3-2012 12:00",>us)
##  734935.5
## >day("3.jan.2012") // automatically German
##  734935
## >day("3-1-2012",>german)
##  734935
##
## See: date
	if typeof(y)==8 then
		s=strtokens(y," ");
		if length(s)>2 then error("Illegal day format"); endif
		sd=strtokens(s[1],"-");
		if length(sd)!=3 then 
			sd=strtokens(s[1],".");
			if length(sd)!=3 then
				error("Illegal day format"); 
			endif;
			german=true;
		endif
		if german then sd=[sd[3],sd[2],sd[1]];
		elseif us then sd=[sd[3],sd[1],sd[2]];
		endif;
		y=sd[1](); 
		m=indexof(englishmonths$,substring(tolower(sd[2]),1,3));
		if m==0 then m=sd[2](); endif;
		d=sd[3]();
		if m<1 or m>12 or d<1 or d>31 then 
			error("Illegal day format: m="+m+" d="+d); 
		endif
		if length(s)==2 then
			sd=strtokens(s[2],":");
			if length(sd)<2 or length(sd)>3 then
				error("Illegal time format"); 
			endif
			h=sd[1](); min=sd[2]();
			if length(sd)==3 then
				min=min+sd[3]()/60;
			endif;
		endif;
	endif;
	greg = y*10000 + m*100 + d;
	if m == 1 or m == 2 then
		y = y - 1;
		m = m + 12;
	endif;
	if greg > 15821004 then
	   a = floor(y/100);
	   b = 2 - a  + floor(a/4);
	else;
	   b = 0;
	endif;
	c = floor(365.25 * y);
	d1 = floor(30.6001 * (m + 1));
	return b + c + d1 + d + (h+min/60)/24;
endfunction

function args date (d:number)
## Converts a day number to the date.
##
## Reverse function of day(). Returns the date in multiple return
## values.
##
## >{y,m,d,h,min}=date(day(2012,11,1,23,12)); [y,m,d,h,min]
##  [ 2012  11  1  23  12 ]
##
## See: day, daynow, weekday
	y=floor((d-53)/365.25);
	repeat
		if day(y+1,1,1)<=d then y=y+1;
		else break;
		endif;
	end
	m=floor((d-day(y,1,1))/31);
	repeat
		if day(y,m+1,1)<=d then m=m+1;
		else break;
		endif;
	end;
	md=floor(d-day(y,m,1))+1;
	d=d-day(y,m,md);
	h=floor(d*24);
	return {y,m,md,h,round(d*24*60-h*60)};
endfunction

function daynow (utc=0)
## Date number for the current day and time.
##
## The computation is based on the Gregorian calendar.
##
## utc : Use UTC time.
##
## >daynow()-day(2000,1,1)
## >printdate(date(daynow()))
##  2016-03-17 15:40
## 
## See: day, date, getnow, printnow, weekday
	v=getnow(utc);
	return day(v[1],v[2],v[3],v[4],v[5]+v[6]/60+v[7]/60000);
endfunction

function comment getnow (utc)
## Current date in vector format.
##
## utc : in UTC or local time.
##
## See: daynow, weekday
endfunction

function overwrite printnow (utc=0)
## Current day and time as string.
##
## The functions uses the locale settings of the user.
##
## >printnow
##  Donnerstag, 17. März 2016 15:42:55
## 
## See: getnow, daynow, weekday
	return _printnow(utc);
endfunction

function printdate (y:integer scalar, m:integer scalar, 
	d:integer scalar, h:integer scalar=0, min:integer scalar=0, 
	sec: integer scalar=none, time=true, timeonly=false, 
	german=false, us=false)
## Prints a day.
##
## The output is in ISO6801 standard format "yyyy-mm-dd hh:mm" by
## default. With <time, the output contains only the date, with
## >timeonly it contains only the time. With >german the format of
## the date is "dd.mm.yyyy". With >us, the output is mm-dd-yyyy with
## the month in string format.
##
## >printdate(date(day(2011,9,11,12)+1000))
##   2014-06-07 12:00
## >printdate(date(day("jan-03-2012",>us)),>us)
##   JAN-03-2012 00:00
##
## See: day, date, getnow, weekday
	if timeonly then
		return printf("%02g",round(h))+":"+printf("%02g",round(min))+sec;	
	endif;
	if german then
		sd=printf("%02g",d)+"."+printf("%02g",m)+"."+printf("%g",y);
	elseif us then
		sd=toupper(englishmonths$[m])+"-"+printf("%02g",d)+"-"+printf("%02g",y);
	else
		sd=printf("%g",y)+"-"+printf("%02g",m)+"-"+printf("%02g",d);
	endif;
	if time then
		if sec!=none then sec=":"+printf("%02g",round(sec)); else sec=""; endif;
		return sd+" "+printf("%02g",round(h))+":"+printf("%02g",round(min))+sec;
	else
		return sd;
	endif;
endfunction

function weekday (d:number, name=false)
## Gets the week day of a day (1=monday, 7=sunday), or its name
##
## >weekday(daynow())
##  4
## >weekday(daynow(),>name)
##  Thursday
## >weekday(day(2021,11,11),>name)
##   Thursday
##
## See: day, date
	wd=mod(floor(d)+4,7);
	if name then
		if wd==0 then wd=7; endif;
		weekdaynames = ["Monday","Tuesday","Wednesday", ..
			"Thursday","Friday","Saturday","Sunday"];
		return weekdaynames[wd];
	endif;
	return wd;
endfunction


// * Continued Fractions

function contfrac (x,n=10)
## Compute the continued fraction of x.
## returns [a,b,c,...] with
## x = a + 1 / (b + 1 / (c + 1/(...
	s=x;
	r=floor(s);
	loop 1 to n
		s=1/(s-floor(s));
		r=r|floor(s);
	end
	return r;
endfunction

function contfracval (r)
## Evaluate the continued fraction
## x = a + 1 / (b + 1 / (c + 1/(...
## with r = [a,b,c,...]
## Return an Interval {x1,x2}
	n=cols(r);
	x1=r[n]; x2=r[n]+1;
	loop 1 to n-1
		 x1=1/x1+r[n-#];
		 x2=1/x2+r[n-#];
	end;
	return {x1,x2};
endfunction

function contfracbest (x,n=3)
## Return the best rational
## approximation to x
	{x1,x2}=contfracval(contfrac(x,n));
	if (abs(x-x1)<abs(x-x2)) then return x1;
	else return x2;
	endif
endfunction

function isstring (x)
## Tests, if x is a string.
	return typeof(x)==8;
endfunction

function expreval (expr:string, x)
## Evaluate the expression, using x and global variables
##
## Note that the variable x is given as a parameter. Other variables
## can be set using assigned variables, as in expreval("x*y",3,y=4).
## Moreover, global variables can be used. Local variables in the
## calling functions cannot be used, however.
##
## This works very much line expr(x), or "a*x*y"(3,4,a=5).
	useglobal;
	return evaluate(expr);
endfunction

function map case (b:real, x, y)
## Returns x, if b!=0, else y.
##
## This function works like if, but evaluates both, x and y. It works
## for matrices b, x and y too.
	if b return x; else return y; endif
endfunction

// * Output Formats

// EMT can either set an output format for all numbers of a specific
// type, or print one scalar or vector value in some format, or
// convert a value to a string in that format. E.g., one can select
// fracformat(), printfrac() or frac(). Most settings specify a length
// of output and the accuracy of the output in some form.

// The are some other functions and settings that affect the output.
// The setting denseoutput prints vectors in a tighter, comma
// separated format with brackets around them. The setting
// zerorounding rounds values close to 0 to print exactly as 0.

// Note that the default format for Euler is longformat with 12
// digits of precision. You can set this format with defformat. By
// default, zero rounding is on so that very small numbers print as 0.
// Dense output is on.

function overwrite print (x:real scalar, digits:integer=2, length:integer=10,
	unit:string="", sep=none, dot=none)
## Formats the real number x to a string.
##
## x : real value
## digits : number of digits after the decimal dot
## length : total length of output string
## unit : unit to append after the print
## sep : separator to group large numbers in groups of 3.
## dot : decimal dot
##
## sep and dot default to none, which means no separation and the
## usual decimal dot. If sep==1 (with >sep), then a hard space will be
## used.
##
## This function calls the built-in function _print() which takes the
## parameters _print(x,digits,length,sep,dot).
##
## >print(101/891,unit="%")
##        0.11%
## >print(1234567890.23,2,20,>sep,dot=",",unit=" €")
##      1 234 567 890,23 €
##
## See: printf, prinststr, hardspace
	if x~=0 then x=0; endif;
	if sep==1 then sep=char(hardspace()); endif;
	return _print(x,digits,length,sep,dot)|unit;
endfunction

function comment printstr (s, length, mode, char)
## String with x and n characters according to mode.
##
## s : string
## length : length of output (at least length of string)
## mode : (optional)
##   -1 = left justified
##    0 = centered
##    1 = right justified
## char : (optional)
##   ascii code of fill character or string with one character
##
## >printstr("Test",10,0,"-")
##  ---Test---
## >s=""; for i=1:5; s=s+"|"+printstr("Test",10,0); end; s+"|",
##  |   Test   |   Test   |   Test   |   Test   |   Test   |
## >printstr(""+24,10,1,"0")
##  0000000024
##
## See: print
endfunction;

function prefix show (x$)
## Prints a variable with its name.
##
## This will print x=... if the provided argument is a variable.
## Otherwise, it will print the value only as usual.
##
## >x=pi/2; show x
##   x = 
##   1.57079632679
##
## See: showlarge
	s=name(x$);
	if s!="x$" then s+" = ", endif;
	x$,
endfunction

function prefix showlarge (x$)
## Prints large matrices in full.
##
## By default EMT eclipses lines and rows of large matrices. This can
## be used to see the full matrix. If the parameter is a variable the
## variable name will be printed.
##
## The default can be changes with largematrices on/off.
##
## >x=random(20,5); showlarge(x)
## >showlarge(random(20,5))
##
## See: largematrices, show
	s=name(x$);
	if s!="x$" then s+" = ", endif;
	largematrices on;
	x$,
	largematrices off;
endfunction

function comment printf (format,x)
## Print a value with a C-format.
##
## The format can be decimal (%d, %x) or floating point (%g, %e, %f)
## if x is a number, or a string format (%s) if x is a string. Length
## modifiers (%20g), digits settings (%10.5f) or adjustments (%-20.5f)
## are allowed.
##
## The format string is checked for a valid format. It should not be
## possible to crash Euler via this function. 
##
## x : real number
## format : a format string 
##
## >printf("pi = %0.2f",pi)
##  pi = 3.14
## >printf("left adjusted: --- %-20.10e ---",pi)
##  left adjusted: --- 3.1415926536e+000    ---
## >printf("pi = %0.10g",pi)
##  pi = 3.141592654
## >printf("%20s","Just a test")
##           Just a test
## >printf(''String: "%20s"'',"Just a test")
##  String: "         Just a test"
##
## See: printf, Formats
endfunction

function overwrite format (n, digits=none, all=0)
## format(n,m) sets the output format to m digits and width n.
##
## format(n,m) : Turns zero rounding and dense output off. Then calls
## the builtin function _format, which sets the output width to n
## places, and the precision digits to m.
##
## format(n) : Works like goodformat with n digits of acccuracy. Dense
## output and zero rounding is on.
##
## all : If true, the set scalar format will be disabled.
##
## >format(6,2); (1:10)/3, longformat; // reset to default
##   0.33  0.67  1.00  1.33  1.67  2.00  2.33  2.67  3.00  3.33 
## >format(20,5); (1:4)'/3, longformat; // reset to default
##              0.33333 
##              0.66667 
##              1.00000 
##              1.33333 
## >format(3); pi, // like goodformat
##   3.14
##
## See: goodformat, Formats
	zerorounding(0); denseoutput(0); scalarformat(!all);
	if digits==none then return goodformat(n);
	else return _format([n,digits]);
	endif;
endfunction

function comment zerorounding (f)
## Sets rounding to 0 of small numbers for output.
##
## See: Formats
endfunction

function comment denseoutput (f)
## Sets dense output for row vectors.
##
## If f is not zero, its value is used to set the number of spaces
## after each comma in the vector.
##
## See: Formats
endfunction

function comment scalarformat (f)
## Enables or disables the special output for scalars.
##
## If this is off the same format will be used for vectors and
## scalars. Many specific format command turn this off. The function
## defformat will set this on. Turning it off manually for a format
## makes sense only if setscalarformat(n) is used too.
##
## >format(10,5);
## >1/3
##     0.33333 
## >(1:5)/3
##     0.33333   0.66667   1.00000   1.33333   1.66667 
## >defformat;
## >1/3
##   0.333333333333
## >(1:5)/3
##   [0.333333,  0.666667,  1,  1.33333,  1.66667]
##
## See: setscalarformat, Formats
endfunction

function comment setscalarformat (n)
## Sets the number of digits for the scalar format.
##
## By default, the scalar format is different from the vector format.
## This function can be used to set the number of digits for the
## scalar format.
##
## See: scalarformat, Formats
endfunction

function comment iformat (n)
## Sets the number of digits for interval output.
endfunction

function overwrite goodformat (n,digits=none,all=0)
## goodformat(n,m) sets the output format to m digits and width n.
##
## Selects a nice format. For integers, the decimal dot will not be
## included. For very large or very small numbers, the exponential
## format will be used.
##
## This turn zero rounding on and dense output on. Then it calls the
## builtin function _goodformat, which sets the width to m digits and
## the precession to n digits. The width will only be used for matrix
## output with more than one row unless dense output is off.
##
## goodformat(n,m) : sets m digits of precission and a total length of
## n digits.
##
## goodformat(m) : total length is m+8, which is sufficient for
## exponential formats.
##
## all : if on, the scalar format is disabled.
##
## The difference to format is that the output of integers uses no
## decimal dot. Moreover, the dense output prints row vectors with
## brackets.
##
## >goodformat(6,2); (1:10)/3, defformat; // reset to default
##   [ 0.33  0.67  1  1.3  1.7  2  2.3  2.7  3  3.3 ]
##
## See: Formats
	zerorounding(1); denseoutput(2);  scalarformat(!all);
	if digits==none then return _goodformat(n);
	else return _goodformat([n,digits]);
	endif;
endfunction

function overwrite expformat
## expformat(n,m) sets the output format to m digits and width n.
##
## Turns zero rounding off and dense output on. This works like
## format, but uses exponential output always. It calls the builtin
## function _expformat.
##
## >expformat(6,2); (1:10)/3, longformat; // reset to default
##  [ 3.33e-001  6.67e-001  1.00e+000  1.33e+000  1.67e+000  2.00e+000
##  2.33e+000  2.67e+000  3.00e+000  3.33e+000 ]
##
## See: Formats	
	zerorounding(0); denseoutput(2); scalarformat(1);
	if argn()==2 then return _expformat([arg1,arg2]);
	else return _expformat(arg1);
	endif;
endfunction

function overwrite fixedformat
## fixedformat(n,m) sets the output format to m digits and width n.
##
## This works like expformat(), but always uses fixed point format. It
## calls the builtin function _fixedformat.
##
## Turns zero rounding on. Zero rounding is used to round very small
## numbers to 0.
##
## See: Formats	
	zerorounding(1); denseoutput(2); scalarformat(0);
	if argn()==2 then return _fixedformat([arg1,arg2]);
	else return _fixedformat(arg1);
	endif;
endfunction

function longestformat
## Sets a very long format for numbers.
##
## Turns zero rounding and dense output off. Uses a format, that
## allows to see the internal accuracy of IEEE double precision.
##
## See: Formats
	zerorounding(0); denseoutput(2); scalarformat(0);
	_goodformat([24,16]);
endfunction

function longformat (all=0)
## Sets a long format for numbers
##
## This is the default format for Euler with 12 digits precision.
## Turns zero rounding and dense output on, and uses a good format,
## which does not show a decimal dot if possible.
##
## See: Formats
	zerorounding(1); denseoutput(2); scalarformat(!all);
	_goodformat([20,12]);
endfunction

function defformat
## Sets the default format for Euler
##
## Turns zero rounding and dense output on, and uses a good format,
## which does not show a decimal dot if possible.
##
## See: Formats
	zerorounding(1); denseoutput(2); 
	scalarformat(1); setscalarformat(12);
	_goodformat(6);
endfunction

function shortformat (all=0)
## Sets a short format for numbers.
##
## See: Formats
	zerorounding(1); denseoutput(2);
	if not all then scalarformat(0); endif;
	_goodformat([10,5]);
endfunction;

function shortestformat (all=0)
## Sets a very short format for numbers.
##
## See: Formats
	zerorounding(1); denseoutput(1); scalarformat(!all);
	_goodformat([7,2]);
endfunction;

function overwrite fracformat (n:integer=0, eps=1e-10)
## Sets the fractional format.
##
## Turns zero rounding on. If n>0 it turns dense output off. A
## continued fraction is used to approximate the numbers with
## fractions. The accuracy can be set with eps.
##
## >fracformat; (1:10)/3, longformat;
##  [ 1/3  2/3  1  4/3  5/3  2  7/3  8/3  3  10/3 ]
## >fracformat(10); (1:4)'/3, longformat;
##        1/3 
##        2/3 
##          1 
##        4/3 
##
## See: frac
	zerorounding(1); 
	if n>0 then denseoutput(0);
	else denseoutput(2); n=20;
	endif;
	_fracformat([n,eps]);
endfunction

function fracprint (x:numerical, n:integer=10, eps=1e-10)
## Prints x in fractional format with n places.
##
## Uses a temporary format to print a value in fractional format. The
## format is then reset to the default longformat.
##
## >fracprint(pi,eps=0.01)
##  22/7
##
## See: frac
	f=getformat();
	_fracformat([n,eps]);
	x,
	setformat(f);
endfunction

function overwrite frac (x:numerical, mixed:integer=0, 
	eps:scalar=none)
## Returns a string containing the fractional representation of x.
##
## This can be used to convert a number to a fraction contained in a
## string. If mixed, then the integer part is separated from the
## fractional part. The function uses the builtin function _frac to
## find a continued fraction, which approximates the number.
##
## The function can also be used for vectors and matrices to convert
## decimals into good fractions, e.g. for Maxima.
##
## mixed : split the integer part from the fractional part
## eps : allowed relative accuracy.
##
## >frac(2/3+5/4,>mixed)
##   1+11/12
## >(1:4)/3
##   [0.333333,  0.666667,  1,  1.33333]
## >frac((1:4)/3)
##   [1/3,2/3,1,4/3]
##
## See: fracformat
	if eps!=none then localepsilon(eps); endif;
	if mixed and abs(x)>1 then
		if x<0 then 
			x=-x;
			h=floor(x); 
			if h~=x then return "-"|h;
			else return "-("|h|"+"|_frac(x-h)|")";
			endif;
		else
			h=floor(x); 
			if h~=x then return ""|h;
			else return h|"+"|_frac(x-h);
			endif;
		endif;
	else
		return _frac(x)
	endif;
endfunction

function prefix short (x)
## Print x in short format
##
## >short pi^2
	f=getformat();
	_zerorounding(1);
	_denseoutput(2);
	_goodformat([10,5]);
	_scalarformat(0);
	x, 
	setformat(f);
endfunction

function prefix shortest (x)
## Print x in shortest format
##
## >shortest pi^2
	f=getformat();
	_zerorounding(1);
	_denseoutput(2);
	_goodformat([7,2]);
	_scalarformat(0);
	if typeof(x)==0 then denseoutput(0); endif;
	x, 
	setformat(f);
endfunction

function prefix long (x)
## Print x in long format
##
## >long pi^2
	f=getformat();
	_zerorounding(1);
	_denseoutput(2);
	_goodformat([20,12]);
	_scalarformat(0);
	x, 
	setformat(f);
endfunction

function prefix longest (x)
## Print x in the longest format
##
## >longest pi^2
	f=getformat();
	_zerorounding(0);
	_denseoutput(0);
	_goodformat([24,16]);
	_scalarformat(0);
	x, 
	setformat(f);
endfunction

function rad ()
## rad(d[,min,sec]) converts to radians
##
## rad(x) transfers degree x to radians. If min and sec are present,
## these will be used to set minutes and seconds of the angle.
##
## x° works too instead of rad(x).
##
## See: deg, degprint
	if argn==1; then return arg1/180*pi; endif;
	if argn==2; then return (arg1+arg2/60)/180*pi; endif;
	if argn==3; then return (arg1+arg2/60+arg3/3600)/180*pi; endif;
	error("Wrong argument for rad");
endfunction

function deg (x)
## deg(x) transfers radians x to degrees
##
## See: rad, degprint
	return x/pi*180;
endfunction

function degprint (x : real scalar, dms:integer=1)
## Converts radians x to string in degrees, minutes and seconds.
##
## This function returns a string with a representation of the radial
## angle x in degrees. By default, the function will print minutes and
## seconds. Turn this off with <dms. Returns a string.
##
## See: degformat
	y=round(deg(x),10);
	h=abs(y);
	vz=""; if (y<0); vz="-"; endif;
	g=floor(h); 
	if g~=h; return vz|printf("%g°",g); endif;
	if not dms then 
		return vz|printf("%g°",g)|printf("%2.3f'",(h-g)*60); 
	endif;
	m=floor((h-g)*60); 
	s=(h-g-m/60)*3600; 
	return vz|printf("%g°",g)|printf("%g'",m)|printf("%2.2f''",s);
endfunction

function degformat (on:integer=1)
## Use degree format from now on.
##
## This calls userformat with the degprint function.
##
## See: userformat, degprint
	denseoutput(2);
	if on;
		userformat("degprint");
	else
		userformat("");
	endif;
	return on;
endfunction

function polarprint (x:complex scalar, format="%g")
## Converts a complex x to a string in polar form.
##
## Converts the complex number in the form (r,phi°). The format for r
## can be set with format="...". Returns a string.
##
## >polarprint(1+I,format="%0.12g")
##  (1.41421356237,45°)
## >polarprint(1+I)
##  (1.41421,45°)
##
## See: polarformat, polar
	if iscomplex(x) then
		return "("|printf(format,abs(x))|","|printf(format,deg(arg(x)))|"°)";
	endif
	return x;
endfunction

function polarformat (on:integer=1)
## Sets the format to polar form.
##
## See: userformat, polarprint
	denseoutput(2);
	if on;
		userformat("polarprint");
	else
		userformat("");
	endif;
	return on;
endfunction

function ipmprint (x:interval scalar)
## Print an interval using plus-minus notation.
##
## >ipmprint(~1,2~)
##  1.5±0.5
##
## See: ipmformat
	if isinterval(x);
		return printf("%g",middle(x))|char(177)|printf("%g",diameter(x)/2)
	endif;
	return x;
endfunction

function ipmformat (on:integer=1)
## Set the interval format to plus-minus notation.
##
## See: ipmprint
	denseoutput(2);
	if on then
		userformat("ipmprint");
	else
		userformat("");
	endif;
	return on;
endfunction

function prefix cformat (x)
## Print x in currency format
##
## >long pi^2
## See: cprint
	f=getformat();
	_zerorounding(1);
	_denseoutput(2);
	userformat("cprint");
	x, 
	userformat("");
	setformat(f);
endfunction

function prefix fraction (x)
## Print x in fractional format
##
## >longest pi^2
	f=getformat();
	_zerorounding(1);
	_denseoutput(2);
	_goodformat([10,3]);
	_fracformat([10,1e-10]);
	x, 
	setformat(f);
endfunction

euro$="€";

function printeuro (x,n=0)
## The value in Euro rounded to cent
	return print(x,2,n,euro$,>sep,dot=",");
endfunction

function printdollar (x,n=0)
## The value in Dollar rounded to cent
	return print(x,2,n,"$",>sep);
endfunction

userformat$="%0.10g";

function %userprint (x)
	if isreal(x) then return printf(userformat$,x);
	else return ""+x;
endfunction

function prefix uprint (x)
## Print in the format in userformat$.
##
## >userformat$="%10.2f Yen";
## >uprint 119569*119%
##  142287.11 Yen
##
## See: userformat
	f=getformat();
	_zerorounding(1);
	_denseoutput(2);
	_goodformat([10,3]);
	userformat("%userprint");
	x, 
	userformat("");
	setformat(f);
endfunction

function comment userformat (f$)
## Uses f$(x) for the output of the number x.
##
## f$ : A function to format a number x.
##    The function should be able to detect real, complex or interval
##    values. If f$=="" then the user format is reset to the default
##    format.
##
## >function prefix money (x) := print(x,2,12,>sep,dot=",",unit="Euro");
## >money 12345.678
## 
endfunction

// * Diverse Helper Functions

function isstring (x)
## Tests, if x is a string.
	return typeof(x)==8;
endfunction

function expreval (expr:string, x)
## Evaluate the expression, using x and global variables
##
## Note that the variable x is given as a parameter. Other variables
## can be set using assigned variables, as in expreval("x*y",3,y=4).
## Moreover, global variables can be used. Local variables in the
## calling functions cannot be used, however.
##
## This works very much line expr(x), or "a*x*y"(3,4,a=5).
	useglobal;
	return evaluate(expr);
endfunction

function map case (b:real, x, y)
## Returns x, if b!=0, else y.
##
## This function works like if, but evaluates both, x and y. It works
## for matrices b, x and y too.
	if b return x; else return y; endif
endfunction

function prefix sel (a)
## Prefix version of nonzeros(a)
##
## See: nonzoros
	return nonzeros(a)
endfunction
