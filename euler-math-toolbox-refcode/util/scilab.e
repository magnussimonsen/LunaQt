// Scilab Interface

comment
Interface for the SciLab program.
Currently, this is not operative due to changes in Scilab.
endcomment

// If you have Scilab for Windows installed, you can call commands in
// Scilab with this interface. Start the Scilab process with
// "slstart", and stop it with "slend" (or by exiting EMT).

// A command is entered with the sl() function.

// See: slstart, slend, sl

// Needs to be loaded with "load scilab"

stripstart=true;
stripend=true;
stripans=true;
stripansstring="ans  =";
slstarted=false;
setglobal stripstart,stripend,stripans,stripansstring,slstarted;

function %slget (min=1, expect=none)
## Read an answer string from Scilab
	s=execread();
	repeat
		wait(0);
		until testkey();
		res=execread();
		if length(s)>min and length(res)==1 strlen(res[1])==0 then 
			break;
		endif;
		if length(res)>1 or strlen(res[1])>0 then 
			s=s|res; 
		endif;
	end;
	// for k=1 to length(s); strtochar(s[k]), end;
	// s,
	return s;
endfunction

function %slfind ()
## Find WScilex-cli.exe and return full path
		scilabdir=exec("cmd","/c echo %ProgramFiles%",>print);
		if length(scilabdir)==0 then error("Program Files not found"); endif;
		version=dir(scilabdir[1]+"/scilab*");
		if length(version)==0 then error("SciLab not found"); endif;
		scilab=scilabdir[1]+"/"+version[1]+"bin/scilex.exe";
		return strrepl(scilab,"/","\");
endfunction

function slstart (scilab:string=none, check=false)
## Start the Scilab process
##
## The function searches the current version of Scilab in the program
## directory. Alternatively you can enter the path to "scilex.exe" as
## a parameter.
##
## Since Scilab is not signalling that it waits for input, we are
## relying on the format of the start string. This function needs to
## be fixed whenever the start string changes.
##
## See: slend, sl
	if slstarted then slend(); endif;
	if scilab==none then
		scilabdir=exec("cmd","/c echo %ProgramFiles%",>print);
		if length(scilabdir)==0 then error("Program Files not found"); endif;
		version=dir(scilabdir[1]+"/scilab*");
		if length(version)==0 then error("SciLab not found"); endif;
		scilab=scilabdir[1]+"/"+version[1]+"bin/WScilex-cli.exe";
		scilab=strrepl(scilab,"/","\");
		if check then "Found SciLab in "+scilab, endif;
	endif;
	exec(scilab,"",<wait,>print,>hidden);
	execwrite(""+char(10));
	repeat
		s=%slget();
		// s, "---",
		sc=false;
		for k=1 to length(s);
			if strfind(s[k],"Scilab")>0 then sc=true; break; endif;
		end;
		until sc;
	end;
	execwrite(''"Test"''+char(10));
	repeat
		s=%slget();
		// s, "---",
		sc=false;
		for k=1 to length(s);
			if strfind(s[k],"Test")>0 then sc=true; break; endif;
		end;
		until sc;
	end;
	slstarted=true;
endfunction

function slend ()
## Kill the Scilab process.
	execkill();
	slstarted=false;
endfunction

function slstrip (s: string vector)
## Strips the result of Scilab
##
## The global flags stripstart, stripend, stripans are used.
##
## stripstart : strip emtpy lines and optionally "ans  =".
## stripend : strip empty lines at the end.
##
## See: sl
	// s,
	n=length(s);
	k1=1;
	if stripstart then
		if stripans then
			repeat
				if k1<=n then
					if strfind(s[k1],stripansstring)==0 then k1=k1+1;
					else k1=k1+1; break;
					endif;
				else break;
				endif;
			end
		endif;
		repeat
			if k1<=n and strlen(s[k1])<2 then k1=k1+1;
			else break;
			endif;
		end
	endif;
	k2=n;
	if stripend then
		repeat
			if k2>k1 and strlen(s[k2])<2 then k2=k2-1;
			else break;
			endif;
		end
	endif;
	// [k1,k2],
	if k1<=k2 then return s[k1:k2]; 
	else return [none];
	endif;
endfunction

function prefixline sl (s)
## Send a command to Scilab
##
## Scilab must be started with slstart() first. 
##
## Returns a vector of strings containing the answer of Scilab.
## The string is stripped from empty lines at start and end,
## and the line "ans  =" by default.
##
## Examples:
##
## >sl sum(1:1000)/1000
## >sl("exp(1.2)")
## >res=sl("1:20");
## >res,
##
## See: slstart, slend, slstrip
	if !slstarted then slstart(); endif;
	execwrite(s+char(10));
	s=%slget();
	return slstrip(s);
endfunction

function slget (s:string)
## Try to interpret the result of Scilab.
##
## See: sl
	s=sl(s)[1];
	return s();
endfunction

slend();

