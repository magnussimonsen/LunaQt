// Sounds

comment
Routines for sound.
endcomment

// * Sound with Euler

// Euler can generate sound with its matrix language. You need a time
// vector t to generate a sound, and then generate the sound with
// trigonometric functions or other functions. Sound can be played,
// saved or loaded in WAV format. Moreover, there some functions to
// analyzes sound via windowed FFT graphically.

defaultsoundrate=44100;
setglobal defaultsoundrate;

function soundsec (n:number, rate=none)
## n seconds of parameter t with rate/sec ticks.
##
## >t=soundsec(2); playwave(sin(440*t));
##
## See: playwave, savewave
	if rate==none then rate=defaultsoundrate; endif;
	return (0:n*rate-1)*2*pi/rate;
endfunction

function savewave (filename:string, v:real, rate=none, 
	bits=16, normalize=true)
## Save a vector of sound data in WAV format.
##
## This functions saves a vector of sound amplitudes into a file.
## It returns the length of the sample in seconds, which does of
## course depend on the rate. For simple application, use the default
## rate with rate=none.
##
## rate : If not none, it determines the number of sound items per
##        second. 
## bits : should be 8 or 16
## normalize : prevents overload
##
## >t=soundsec(2); s=sin(440*t)*(4+sin(t^2/3));
## >file=eulerhome()+"test.wav"; savewave(file,s);
## >playwave(file);
##
## See: soundsec, playwave, loadwave
	if rate==none then rate=defaultsoundrate; endif;
	open(filename,"wb");
	write("RIFF");
	if bits==8 then
		n=cols(v);
		putlongword(n+36);
		write("WAVEfmt ");
		putlongword(16); putword(1); putword(1);
		putlongword(rate); putlongword(rate);
		putword(1); putword(8);
		write("data");
		putlongword(n);
		m=max(abs(v)); vv=v/m*64+128;
		putuchar(vv);
		putchar(0);
	elseif bits==16 then
		n=cols(v);
		channels=rows(v);
		if channels==2 
			then v=redim(v',[1,2*cols(v)]);
			n=cols(v);
		endif; 
		putlongword(2*n+36);
		write("WAVEfmt ");
		putlongword(16); // offset
		putword(1); // always 1
		putword(channels); // channels
		putlongword(rate); // rate
		putlongword(2*channels*rate); // ???
		putword(2*channels); // word format
		putword(16); // 16 bits
		write("data");
		putlongword(2*n); // length
		if normalize then m=max(abs(v)); else m=1; endif;
		vv=v/m*64*256; putword(vv);
	else error("Bits must be 8 or 16");
	endif
	close();
	return cols(v)/rate;
endfunction

function loadwave (filename:string, saverate=true)
## Read a WAV file.
##
## This very raw function can read mono or stereo wav files with any
## sample rate. It can handle 8 or 16 bit per channel.
##
## Returns {w,rate}
##
## w : 1xn or 2xn (for stereo) sound.
## saverate : remember the rate as defaultsoundrate.
##
## See: savewave, playwave
	open(filename,"rb");
	if getstring(4)<>"RIFF" then error("No Wave file!"); endif;
	getlongword(); // This is the total file length minus 8
	if getstring(8)<>"WAVEfmt " then error("No Wave file!"); endif;
	offset=getlongword(); // Is always 16, sometimes 18
	getword(); // Is always 1.
	channels=getword(); // channels (1 or 2)
	rate=getlongword();  // usually 22050 or 44100
	getlongword(); // ???
	byte=getword(); // bytes per sound item
	bits=getword(); // bit rate (16)
	if offset>16 then getuchar(offset-16); endif;
	if getstring(4)<>"data" then error("No Wave file!"); endif;
	if byte==channels then
		w=getuchar(getlongword());
	elseif byte==2*channels then
		w=getword(getlongword()/2);
	else error("Not 8 or 16 bit!");
	endif;
	w=redim(w,[cols(w)/channels,channels])';
	if cols(w)>0 then 
		w=w-sum(w)/cols(w); 
		w=w/max(abs(w));
	endif;
	close();
	if saverate then defaultsoundrate=rate; endif;
	return {w,rate};
endfunction

function analyzesound (w:real vector, fmin=10, fmax=1500, 
	rate=none, points=2^16)
## Make a frequency plot of the signal w with sampling rate.
##
## The function makes an FFT of the first points of f, by default 2^16
## points. If there are less points, the program takes a power of 2,
## if the parameter points is not specified. 
##
## fmin, fmax : range of frequencies to show in the plot
## spectral : use spectral colors instead of grays
##
## Example:
## >t=soundsec(0.8);
## >s=soundpulse(440*t);
## >analyzesound(s,1,4000):
##
## See: mapsound
	if rate==none then rate=defaultsoundrate; endif;
	points=min(points,2^floor(log(cols(w))/log(2)));
	v=w[1:points];
	f=abs(fft(v));
	i=(fmin/rate*points:fmax/rate*points)+1;
	setplot(fmin,fmax,0,max(f[i]));
	fr=(i-1)/points*rate;
	plot2d(fr,f[i]);
endfunction

function mapsound (w:real vector, dt=0.1, fmin=100, fmax=1500,
	rate=none, spectral=true)
## Plots a sound map for a sound.
##
## It does an FFT at time increments dt. rate is the sampling rate.
##
## fmin, fmax : range of frequencies to show in the plot
## spectral : use spectral colors instead of grays
##
## Example:
## >t=440*soundsec(1);
## >s=soundpulse(t)|soundsaw(t)|soundtriangle(t);
## >mapsound(s,0.1,1,2400):
##
## See: analyzesound
	if rate==none then rate=defaultsoundrate; endif;
	n=cols(w);
	dp=dt*rate;
	points=2^floor(log(dp)/log(2));
	ind=(fmin/rate*points:fmax/rate*points)+1;
	f=abs(fft(w[1:points]));
	M=f[ind];
	i=1;
	repeat
		i=i+dp;
		if i+points>n; break; endif;
		f=abs(fft(w[i:i+points-1]));
		M=M_f[ind];
	end;
	setplot(fmin,fmax,0,n/rate);
	if spectral then cl=huecolor(-2); density(M,0.99); huecolor(cl);
	else density(-M,0.99);
	endif;
	xplot();
endfunction

function overwrite playwave (s, rate=none, normalize=true)
## Plays either a file with name s, or a vector of sound s.
##
## This calls a built-in function playwave(file), which uses system
## routine to play the wave. The format is "*.wav" in stereo and 16
## bit.
##
## rate : Sound rate. The default rate is 44100.
## normalize : Normalize the volume.
##
## See: soundsec
	if rate==none then rate=defaultsoundrate; endif;
	if typeof(s)==8 then return _playwave(s);
	else
		file=userhome()|"eulertemp.wav";
		savewave(file,s,rate=rate,normalize=normalize);
		_playwave(file);
		fileremove(file);
		return file;
	endif;
endfunction

function soundpulse (t) := sign(sin(t)); // Pulse sound
function soundsaw (t) := pi-mod(t,2*pi); // Saw wave sound
function soundtriangle (t) := 2*abs(pi-mod(t,2*pi))-pi; // Triangle wave sound
