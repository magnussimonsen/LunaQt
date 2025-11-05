// Units

comment
Units in Euler and conversion routines.
endcomment

// * Units in Euler

// Revision by Horst Vogel.

// Units are defined with an ending $, so that they become accessible
// globally. Euler can convert units with the -> syntax.

// It is safest to use the units with the dollar sign to avoid
// confusion with variables.

// See: eulercore.html#Functions_for_Units | Syntax for Conversions

// The following units are known to Euler. You can define own units
// easily in the same manner.

kilometer$:=1000;
km$:=kilometer$;
cm$:=0.01;
mm$:=0.001;
minute$:=60;
min$:=minute$;
minutes$:=minute$;
hour$:=60*minute$;
h$:=hour$;
hours$:=hour$;
day$:=24*hour$;
days$:=day$;
d$:=day$;
year$:=365.2425*day$;
years$:=year$;
y$:=year$;

inch$:=0.0254;
in$:=inch$;
feet$:=12*inch$;
foot$:=feet$;
ft$:=feet$;
yard$:=3*feet$;
yards$:=yard$;
yd$:=yard$;
mile$:=1760*yard$;
miles$:=mile$;
nm$=6080.2*feet$;
kts$=nm$/hour$;
kg$:=1;
gram$=0.001;
tons$=1000;
sec$:=1;
m$=1;
ha$:=10000;
Ar$:=100;
Tagwerk$:=3408;
Acre$:=4046.8564224;
pt$:=0.376mm;

liter$=0.001;
l$=0.001;
L$=l$;
gallon$=3.785411784l;

Euro$:=1.95593;

gammaEuler$:=0.577215664901532860606512090082402431042;
gamma$:=gammaEuler$;
c$:=299792458;
cLight$:=299792458;
lightyear$:=365.25*days$*c$;
ly$:=lightyear$;
astrounit$:=149597870700;
AU$:=astrounit$;
AE$:=AU$;
parsec$:=AU$/tan(1°/3600);
hquantum$:=6.62606957e-34;
mol$:=1;
kmol$=1000;
Rgas$:=8.3144621;
KelvinMol$:=Rgas$/mol$;
Celsius0$:=273.15;
elemcharge$:=1.602176565e-19;
electronmass$:=0.910938291e-30;
protonmass$:=1.672621777e-27;
neutronmass$:=1.674927351e-27;
mumass$:=1e-3/mol$;
molair$:=28.949;
gravconstant$:=6.67384e-11;
calory$:=4.1868;
cal$:=calory$;
calories$:=cal$;
J$=1;
kJ$:=1000;
W$=1;
kW$:=1000;
cal$=1;
kcal$:=1000*cal$;
gearth$:=9.80665;
g$:=gearth$;
PS$:=75*gearth$;
Torr$:=101325/760;
atm$=101325;
K$=1;

pi$=pi();

function FahrenheitToCelsius (x)
## Convert Fahrenheit to Celsius with (x-32)*5/9
return (x-32)*5/9
endfunction

function CelsiusToFahrenheit (x)
## Convert Celsius to Fahrenheit with x*9/5+32
	return x*9/5+32
endfunction

function KelvinToCelsius (x)
## Convert Kelvin to Celsius with x-273.15
	return x-273.15
endfunction

function CelsiusToKelvin (x)
## Convert Kelvin to Celsius with x+273.15
	return x+273.15;
endfunction

function rearth (lat:number, r1=6378137, r2=6356752)
## WSG84 radius of earth at that latitude
	return sqrt(r2^2*sin(lat)^2+r1^2*cos(lat)^2);
endfunction

rEarth$:=rearth(90°); // [m], radius at equator, [rearth(45°)] 
