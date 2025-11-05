// Elementary File Input and Output

comment
Formatted input and output to files. 
endcomment

// See also the Euler core functions for file input and output in
// @"eulercore".

// See: eulercore.html#File_Input_and_Output | Core Functions for File Input and Output

// For files of unknown sources getmatrix() or getvector() should be
// used to read the numerical content. For output, writematrix() is
// the easiest choice. 

// The functions work either for files opened with open(), or for a
// file name provided as a parameter, or for terminal output.

// For Latex, writelatex() can be used.

// * Reading and Writing Matrices and Vectors

function comment setdecimaldot (s)
## Set the decimal dot.
##
## Returns the current value of the decimal dot.
##
## >dd=setdecimaldot(",");
## >...
## >setdecimaldot(dd);
endfunction

function decimalcomma ()
## Use a decimal comma for matrix input
##
## Returns the previous value.
	return setdecimaldot(",");
endfunction

function overwrite getvector (n, comma=0)
## Read at most numbers from an input file
##
## A file must be opened for this to work. The function returns a
## vector of numbers. The numbers in the file can be separated by any
## character.
##
## See: getvectorline, open, close
	if comma then dd=setdecimaldot(","); endif;
	{v,n}=_getvector(n);
	if comma then setdecimaldot(dd); endif;
	return v[1:n];
endfunction

function overwrite getvectorline (n, comma=0)
## Read at most numbers from an input line
##
## A file must be opened for this to work. The function returns a
## vector of numbers. The numbers in the file can be separated by any
## character.
##
## See: getvector, getmatrix, readmatrix
	if comma then dd=setdecimaldot(","); endif;
	{v,n}=_getvectorline(n);
	if comma then setdecimaldot(dd); endif;
	return v[1:n];
endfunction

function getmatrix (n:real scalar, m:real scalar, 
	filename:string="", comma=0)
## Read a real nxm Matrix from the file.
##
## If the filename is "", the file must be opened before. Else the
## file will open and close for read.
##
## Matrix values must be decimal fix point numbers with a dot (or a
## comma, if set). They must be stored row after row.
##
## comma : use decimal comma
##
## See: getvector, open, readmatrix
	if comma then dd=setdecimaldot(","); endif;
	if filename<>"" then open(filename,"r"); endif;
	{v,N}=_getvector(n*m);
	if filename<>"" then close(); endif;
	if (N<n*m) then 
		if comma then setdecimaldot(dd); endif;
		error("Incorrect values in file"); 
	endif;
	v=v[1:n*m];
	if comma then setdecimaldot(dd); endif;
	return redim(v,n,m);
endfunction

function readmatrix (filename:string="", 
	max=1000, expand=false, comma=false)
## Read a real matrix from the file.
##
## The matrix is stored row by row in lines, with an empty line ending
## the matrix. If the filename is not empty, the function will open
## the file for reading, and close it after reading. Otherwise, it
## will use the provided file name.
##
## max : the maximimal number of columns.
## expand : incomplete lines will filled with 0.
## comma : use decimal comma instead of dot
##
## To import matrices from Excel, export to CSV (comma separated
## values). These files can be read directly. On German systems,
## enable the decimal comma with >comma.
##
## See: writematrix, open, close, getmatrix
	if comma then dd=setdecimaldot(","); endif;
	if filename<>"" then open(filename,"r"); endif;
	A=[];
	r=0;
	repeat;
		{s,n}=_getvectorline(max);
		if n==0 then break; endif;
		if cols(A)==0 or rows(A)==0 then 
			A=s; r=1;
		elseif n==cols(A) then 
			r=r+1;
			if r>rows(A) then A=A_A; endif;
			A[r]=s;
		elseif expand then
			if n>cols(A) then A=(A|zeros(rows(A),n-cols(A))); endif;
			r=r+1;
			if r>rows(A) then A=A_A; endif;
			A[r]=s|zeros(1,cols(A)-n);
		else
			if comma then setdecimaldot(dd); endif;
			"Error in line "+(rows(A)+1),
			error("Matrix rows do not agree");
		endif;
	end;
	if filename<>"" then close(); endif;
	if comma then setdecimaldot(dd); endif;
	return A[1:r,:];
endfunction

function getmatrixlines (filename:string="", max=1000, expand=0)
## Obsolete. Use readmatrix.
##
## See: readmatrix
	return readmatrix(filename,max,expand)
endfunction
	
function writematrix (v:real, 
	file:string=none, format:string="%0.16g", 
	separator=none, comma=false)
## Write a real nxm Matrix to the file row after row.
##
## v : real matrix
## file : filename
## format : C type format, see printf
## separator : Separator string
## comma : >comma uses a decimal comma.
##
## If the filename is not "", the file will be created (beware!),
## opened, and closed after writing. If the filename is "", a file
## opened with open(filename,"w") will be used, or the terminal.
##
## For cvs output (comma separated values), set the separator to ","
## and use a filename with extension "csv". For German files, set the
## use ";" and enable >comma. These files can be opened directly in
## Excel.
##
## See: readmatrix, open, printf
	if typeof(v)<>2 then error("Can only write real matrices"); endif;
	if file!=none then open(file,"w"); endif;
	if comma and separator==none then separator=";"; endif;
	if separator==none then separator="," endif;
	for i=1 to rows(v);
		for j=1 to cols(v);
			s=printf(format,v[i,j]);
			if comma then s=strrepl(s,".",","); endif;
			if j>1 then write(separator); endif;
			write(s);
		end;
		putchar(10);
	end;
	putchar(10);
	if file!=none then close(); endif;
endfunction

// * Formatted Output to Files

function writeform (x, format="%0.16g")
## Converts x in the specified format to a string
	if isreal(x) then 
		return printf(format,x);
	elseif iscomplex(x) then
		return printf(format,re(x))|"+"|printf(format,im(x));|"i";
	elseif isinterval(x) then
		return "~"|printf(format,left(x))|","|printf(format,right(x))|"~";
	elseif isstring(x) then
		return x;
	endif;
	error("Cannot print this!");
endfunction

function writevar (x,s="",filename="",newlines=0)
## Write a variable Euler syntax. 
##
## If a filename is given, or a file is open, it will be used,
## otherwise the function will write to the command line.
##
## s : the name of the variable
## filename : a file to write to
## newlines : flag for newlines to separate elements in a row
	if filename<>"" then open(filename,"w"); endif;
	if s=="" then s=name(x); endif;
	si=size(x);
	if max(si)==1 then 
		writeln(s|" = "|writeform(x)|";");
	else
		write(s|" = [ .."); putchar(10);
		for i=1 to si[1];
			for j=1 to si[2]-1;
				write(writeform(x[i,j])|", ");
				if newlines then putchar(10); endif;
			end;
			if i==si[1]; writeln(writeform(x[i,si[2]])|"];");
			else; writeln(writeform(x[i,si[2]])|";");
			endif;
		end;
	endif;
	if filename<>"" then close(); endif;
endfunction

function %writeformat(x,f,func,sep)
	if func then write(f(x));
	else write(printf(f,x));
	endif;
	write(sep);
endfunction

function writeformat (x,f:string="%g",sep1=", ",sep2=";",
	writereal=1,ibracket1="~",ibracket2="~",perline=100)
## Print the vector x formatted with format f.
##
## The format is applied to real and imaginary part, or lower and
## upper part of the elements of x. 
##
## writereal : Complex and interval values become two real values. 
##   Rows are separated with sep2.
##
## If a file is open, the function will write to that file.
##
## See: writevar
	s=size(x);
	func=isfunction(f);
	loop 1 to s[1];
		i=#;
		count=1;
		loop 1 to s[2]
			sep=sep1;
			if #==s[2] then 
				if i==s[1] then sep=""; else sep=sep2; endif;
			endif;
			if iscomplex(x) then
				if writereal then
					%writeformat(re(x[i,#]),f,func,sep1);
					%writeformat(im(x[i,#]),f,func,sep);
				else
					%writeformat(re(x[i,#]),f,func,"");
					if !(im(x[i,#])~=0) then
						if im(x[i,#])>0 then write("+"); else write("-"); endif;
						%writeformat(im(x[i,#]),f,func,"");
						write("i"); write(sep);
					else
						write(sep);
					endif;
				endif;
			elseif isinterval(x) then
				if writereal then
					%writeformat(left(x[i,#]),f,func,sep1);
					%writeformat(right(x[i,#]),f,func,sep);
				else
					write(ibracket1);
					%writeformat(left(x[i,#]),f,func,"");
					write(",");
					%writeformat(right(x[i,#]),f,func,"");
					write(ibracket2);
					write(sep);
				endif;
			else
				%writeformat(x[i,#],f,func,sep);
			endif;
			count=count+1;
			if count>perline and #<s[2] then 
				count=0; putchar(10);
			endif;
		end;
		putchar(10);
	end;
endfunction

// Latex is a text system for mathematical texts. Sometimes, you may
// wish to output Euler matrices in Latex. Symbolic matrices can be
// printed using the latex command of Maxima.

function writelatex (x,f="%g",environment="pmatrix",perline=100)
## Write x for AMS latex matrix environment
##
## This function can output an Euler matrix for the AMS Latex
## environment pmatrix, or any other environment. The open file is
## used, or the terminal.
##
## perline: Maximal number of items per line
##
## See: writeformat, open, latex
	write("\begin{"|environment|"}"); putchar(10);
	writeformat(x,f,sep1=" & ",sep2=" \\", ..
		writereal=0,ibracket1="[",ibracket2="]",perline=perline);
	write("\end{"|environment|"}"); putchar(10);
endfunction

// Use the dir command to get a listing of the files in the active
// directory. This is the directory, where the notebook is stored. It
// can be changed using the changedir command in @"eulercore".

function overwrite dir (pattern:string=none, dirs:integer=false)
## vector of strings with matching file names or subdirectories.
##
## The pattern can be something like "*.en" or "file?.dat"
## If dirs==true, then only directories will be listed.
## Directories are listed with a "/" at the end.
##
## See: changedir, searchfile
	if pattern==none then pattern="*"; endif;
	s=_dir(pattern);	
	if dirs then
		res=none;
		loop 1 to length(s)
			f=s[#];
			if substring(f,-1,-1)=="/" then
				if res==none then res=[f]
				else res=res|f;
				endif;
			endif;
		end;
		return res;
	endif;
	return s;
endfunction

function printfile (filename:string, lines:index=100)
## Print the first lines of a file
##
## See: open, close
	open(filename,"r");
	loop 1 to lines
		s=getline();
		if eof() then 
			if strlen(s)>0 then s, endif;
			break; 
		endif;
		s,
	end
	close();
endfunction

function printhexfile (filename:string, n:index=1024)
## Print the first bytes of a file in hex
	open(filename);
	loop 1 to n/16;
		v=getuchar(16);
		s=printstr((#-1)*16,8);
		loop 1 to 16; s=s+printstr(printhex(v[#],true),3); end;
		s=s+"  ";
		loop 1 to 16
			if v[#]>=32 then s=s+char(v[#]);
			else s=s+"?";
			endif;
		end;
		s,
	end;
	close();
endfunction
