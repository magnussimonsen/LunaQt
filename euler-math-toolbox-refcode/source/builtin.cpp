#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unordered_map>
#include <string>

#include "header.h"
#include "builtin.h"
#include "stack.h"
#include "graphics.h"
#include "cpx.h"
#include "media.h"
#include "otherfunctions.h"
#include "find.h"
#include "maxima.h"
#include "alglibfuncs.h"
#include "stats.h"
#include "globalvars.h"
#include "aliases.h"
#include "spread.h"

// routines for built-in functions

int builtin_count; // will be determined later in sort_builtin

/**
List of Euler builtin functions contains the Euler name, 
the number of arguments (-1 for any number) and the pointer to the handling routine.
The list is sorted in the initialization.
The functions are then stored in a hash table for fast access.
*/
builtintyp builtin_list[] =
{
	{"index",0,mindex},
	{"pi",0,mpi},
	{"true",0,mtrue},
	{"false",0,mfalse},
	{"E",0,mE},
	{"I",0,mI},
	{"time",0,mtime},
	{"hardspace",0,mhardspace},
	{"epsilon",0,mepsilon},
	{"sin",1,msin},
	{"cos",1,mcos},
	{"tan",1,mtan},
	{"atan",1,matan},
	{"acos",1,macos},
	{"asin",1,masin},
	{"arctan",1,matan},
	{"arccos",1,macos},
	{"arcsin",1,masin},
	{"exp",1,mexp},
	{"log",1,mlog},
	{"ln",1,mlog},
	{"sqrt",1,msqrt},
	{"re",1,mre},
	{"im",1,mim},
	{"complex",1,mcomplex},
	{"iscomplex",1,miscomplex},
	{"isinterval",1,misinterval},
	{"isreal",1,misreal},
	{"isfunction",1,misfunction},
	{"iscall",1,miscall},
	{"isvar",1,misvar},
	{"isempty",1,misempty},
	{"round",2,mround},
	{"round",1,mround},
	{"arg",1,marg},
	{"abs",1,mabs},
	{"sum",1,msum},
	{"colsum",1,mcolsum},
	{"prod",1,mprod},
	{"conj",1,mconj},
	{"size",-1,msize},
	{"rows",1,mrows},
	{"cols",1,mcols},
	{"zeros",1,mzerosmat},
	{"zeros",2,mzerosmat},
	{"linspace",3,mlinspace},
	{"ones",1,mones},
	{"ones",2,mones},
	{"diag",1,mdiag2},
	{"diag",2,mdiag2},
	{"getdiag",1,mdiag2},
	{"getdiag",2,mdiag2},
	{"diag",3,mdiag},
	{"diag",4,mdiag},
	{"band",3,mband},
	{"orthogonal",1,morthogonal},
	{"extrema",1,mextrema},
	{"mesh",1,mmesh},
	{"meshbar",1,mmeshflat},
	{"view",1,mview},
	{"view",0,mview0},
	{"wait",0,mwait0},
	{"wait",1,mwait},
	{"wait",2,mwaitstatus},
	{"fastrandom",1,mfastrandom},
	{"random",0,mrandom},
	{"random",1,mrandom},
	{"random",2,mrandom},
	{"intrandom",1,mintrandom1},
	{"intrandom",2,mintrandom2},
	{"intrandom",3,mintrandom2},
	{"normal",0,mnormalnew},
	{"normal",1,mnormalnew},
	{"normal",2,mnormalnew},
	{"fastnormal",1,mnormal},
	{"seed",1,mseed},
	{"shuffle",1,mshuffle},
	{"text",2,mtext},
	{"ctext",2,mctext},
	{"rtext",2,mrtext},
	{"vtext",2,mvtext},
	{"vctext",2,mvctext},
	{"vrtext",2,mvrtext},
	{"vutext",2,mvutext},
	{"vcutext",2,mvcutext},
	{"vrutext",2,mvrutext},
	{"text",3,mtext},
	{"ctext",3,mctext},
	{"rtext",3,mrtext},
	{"vtext",3,mvtext},
	{"vctext",3,mvctext},
	{"vrtext",3,mvrtext},
	{"vutext",3,mvutext},
	{"vcutext",3,mvcutext},
	{"vrutext",3,mvrutext},
	{"textsize",0,mtextsize},
	{"wire",3,mwire},
	{"wirerows",3,mwirerows},
	{"plot",2,mplot},
	{"plotarea",2,mplotarea},
	{"plot",0,mplot1},
	{"fullplotarea",0,mplot2},
	{"pixel",0,mpixel},
	{"mark",2,mmark},
	{"markwithcolor",3,mmarkcolor},
	{"contour",2,mcontour},
	{"dup",2,mdup},
	{"mod",2,mmod},
	{"format",1,mformat},
	{"goodformat",1,mgformat},
	{"expformat",1,meformat},
	{"fixedformat",1,mfformat},
	{"iformat",1,miformat},
	{"fracformat",1,mfracformat},
	{"scalarformat",1,mscalarformat},
	{"setscalarformat",1,msetscalarformat},
	{"userformat",1,muserformat},
	{"getformat",0,mgetformat},
	{"setformat",1,msetformat},
	{"denseoutput",1,mdense},
	{"setcolor",4,msetcolor},
	{"resetcolors",0,mresetcolors},
	{"color",1,mcolor},
	{"color",0,mcolor0},
	{"framecolor",1,mfcolor},
	{"framecolor",0,mfcolor0},
	{"frame3dcolor",1,mf3dcolor},
	{"frame3dcolor",0,mf3dcolor0},
	{"wirecolor",1,mwcolor},
	{"wirecolor",0,mwcolor0},
	{"contourcolor",1,mcontourcolor},
	{"contourcolor",0,mcontourcolor0},
	{"textcolor",1,mtcolor},
	{"textcolor",0,mtcolor0},
	{"fillcolor",1,mfillcolor},
	{"fillcolor",2,mfillcolor},
	{"fillcolor",0,mfillcolor0},
	{"style",1,mstyle},
	{"markerstyle",1,mmstyle},
	{"linestyle",1,mlstyle},
	{"linewidth",1,mlinew},
	{"window",1,mwindow},
	{"window",0,mwindow0},
	{"normaldis",1,mgauss},
	{"invnormaldis",1,minvgauss},
	{"tdis",2,mtdis},
	{"invtdis",2,minvtdis},
	{"fdis",3,mfdis},
	{"fcdis",3,mfdisc},
	{"invfcdis",3,minvfdis},
	{"chidis",2,mchidis},
	{"chicdis",2,mchicdis},
	{"invchicdis",2,minvchidis},
	{"erf",1,merf},
	{"erfc",1,merfc},
	{"fac",1,mfak},
	{"bin",2,mbin},
	{"logfac",1,mlogfak},
	{"logbin",2,mlogbin},
	{"max",2,mmax},
	{"min",2,mmin},
	{"sort",1,msort},
	{"unique",1,munique},
	{"lexsort",1,mlexsort},
	{"nonzeros",1,mnonzeros},
	{"mnonzeros",1,mmnonzeros},
	{"mset",3,mmset},
	{"mget",2,mmget},
	{"count",2,mstatistics},
	{"floor",1,mfloor},
	{"ceil",1,mceil},
	{"cummax",1,mcummax},
	{"cummin",1,mcummin},
	{"cumsum",1,mcumsum},
	{"cumprod",1,mcumprod},
	{"free",0,mfree},
	{"input",1,minput},
	{"max",1,mmax1},
	{"min",1,mmin1},
	{"firstnonzero",1,mfirstnonzero},
	{"lastnonzero",1,mlastnonzero},
	{"eval",-1,mdo},
	{"polyval",2,polyval},
	{"polyadd",2,polyadd},
	{"polymult",2,polymult},
	{"polydiv",2,polydiv},
	{"interp",2,dd},
	{"divdif",2,dd},
	{"interpval",3,ddval},
	{"divdifeval",3,ddval},
	{"polytrans",2,polydd},
	{"polycons",1,polyzeros},
	{"polytrunc",1,polytrunc},
	{"char",1,mchar},
	{"strtochar",1,mstrtochar},
	{"chartostr",1,mchartostr},
	{"chartoutf",1,mchartoutf},
	{"utf",1,mutf},
	{"fromutf",1,mfromutf},
	{"toutf",1,mtoutf},
	{"ascii",1,mascii},
	{"tolower",1,mtolower},
	{"toupper",1,mtoupper},
	{"substring",3,msubstring},
	{"strlen",1,mstrlen},
	{"strfind",3,mstrfind},
	{"strfind",2,mstrfind},
	{"strxfind",3,mstrxfind},
	{"strxfind",2,mstrxfind},
	{"strrepl",3,mstrrepl},
	{"strxrepl",3,mstrxrepl},
	{"strtokens",1,mstrtokens},
	{"strtokens",2,mstrtokens},
	{"lu",1,mlu},
	{"lusolve",2,mlusolve},
	{"fft",1,mfft},
	{"ifft",1,mifft},
	{"fht",1,mfht},
	{"ifht",1,mifht},
	{"polysolve",1,mzeros},
	{"error",1,merror},
	{"printf",2,mprintf},
	{"print",5,mprint},
	{"printstr",2,mprintstr},
	{"printstr",3,mprintstr},
	{"printstr",4,mprintstr},
	{"sign",1,msign},
	{"mouse",0,mmouse},
	{"mouse",1,mmousestatus},
	{"mousedrag",1,mmousedrag},
	{"mousedrag",0,mmousedrag},
	{"mousestate",0,mmousestate},
	{"mousepos",0,mmousepos},
	{"hb",1,mtridiag},
	{"charpoly",1,mcharpoly},
	{"stringcompare",2,mscompare},
	{"find",2,mfind},
	{"indexof",2,mindexof},
	{"indexofsorted",2,mindexofsorted},
	{"multofsorted",2,mmultofsorted},
	{"setdiag",3,msetdiag},
	{"polyroot",2,mzeros1},
	{"argn",0,margn},
	{"setkey",2,msetkey},
	{"any",1,many},
	{"changedir",-1,mcd},
	{"makedir",1,mcreatedir},
	{"home",0,mhome},
	{"start",0,mstart},
	{"eulerhome",0,meulerhome},
	{"userhome",0,muserhome},
	{"userdir",0,muserdir},
	{"searchfile",1,msearchfile},
	{"searchfile",0,msearchnextfile},
	{"dir",1,mdir},
	{"bandmult",2,wmultiply},
	{"symmult",2,smultiply},
	{"project",3,mproject},
	{"getarg",1,mgetarg},
	{"args",1,margs},
	{"args",0,margs0},
	{"setplot",1,msetplot},
	{"scaling",1,mscaling},
	{"holding",1,mholding},
	{"keepsquare",1,mkeepsquare},
	{"holding",0,mholding0},
	{"lineinput",1,mlineinput},
	{"evaluate",1,mevaluate},
	{"name",1,mname},
	{"twosides",1,mtwosides},
	{"meshfactor",1,mmeshfactor},
	{"setepsilon",1,msetepsilon},
	{"localepsilon",1,mlocalepsilon},
	{"flipx",1,mflipx},
	{"flipy",1,mflipy},
	{"rotleft",1,mrotleft},
	{"rotright",1,mrotright},
	{"shiftleft",1,mshiftleft},
	{"shiftright",1,mshiftright},
	{"jacobi",1,mjacobi},
	{"frame",0,mframe},
	{"key",0,mkey},
	{"key",1,mkeystatus},
	{"errorlevel",1,merrlevel},
	{"density",1,mdensity},
	{"huecolor",1,mdcolor},
	{"huegrid",1,mdgrid},
	{"solid",-1,mxsolidh},
	{"redim",2,mredim},
	{"redim",3,mredim},
	{"open",2,mopen},
	{"open",1,mopen1},
	{"close",0,mclose},
	{"putchar",1,mputchar},
	{"putword",1,mputword},
	{"putlongword",1,mputlongword},
	{"getchar",0,mgetchar},
	{"getchar",1,mgetchar1},
	{"getword",0,mgetword},
	{"getword",1,mgetword1},
	{"getlongword",0,mgetlongword},
	{"getlongword",1,mgetlongword1},
	{"putuchar",1,mputchar},
	{"putuword",1,mputword},
	{"putulongword",1,mputlongword},
	{"getuchar",0,mgetuchar},
	{"getuchar",1,mgetuchar1},
	{"getuword",0,mgetuword},
	{"getuword",1,mgetuword1},
	{"getulongword",0,mgetlongword},
	{"getulongword",1,mgetlongword1},
	{"getstring",1,mgetstring},
	{"write",1,mwrite},
	{"writeln",1,mwriteln},
	{"writeln",0,mwriteln0},
	{"getvector",1,mgetvector},
	{"getvectorline",1,mgetvectorline},
	{"eof",0,meof},
	{"simplex",3,msimplex},
	{"simplex",4,msimplex},
	{"simplex",5,msimplex},
	{"simplex",6,msimplex},
	{"left",1,mleft},
	{"right",1,mright},
	{"middle",1,mmiddle},
	{"diameter",1,mdiameter},
	{"interval",2,minterval},
	{"interval",1,minterval1},
	{"accuload",1,maccuload},
	{"accuadd",1,maccuadd},
	{"accuload",2,maccuload2},
	{"accuadd",2,maccuadd2},
	{"residuum",3,mresiduum},
	{"accu",0,maccu1},
	{"accure",0,maccu1},
	{"accua",0,maccu1},
	{"accuim",0,maccu2},
	{"accub",0,maccu2},
	{"expand",2,mexpand},
	{"intersects",2,mintersects},
	{"typeof",1,mtype},
	{"issymbolic",1,missymbolic},
	{"printtype",1,mprinttype},
	{"markersize",1,mmarkersize},
	{"markersize",0,mmarkersize0},
	{"bar",1,mbar},
	{"barRGB",4,mbarRGB},
	{"barcolor",1,mbarcolor},
	{"barstyle",1,mbarstyle},
	{"map",-1,mmap1},
	{"gammaln",1,mgammaln},
	{"gamma",1,mgamma},
	{"gammai",2,mgammai},
	{"gammaic",2,mgammaic},
	{"brent",-1,mbrent},
	{"nelder",-1,mnelder},
	{"runge1",-1,mrunge1},
	{"runge2",-1,mrunge2},
	{"testkey",0,mcode},
	{"svd",1,msvd},
	{"toeplitz",1,mtoeplitz},
	{"toeplitzsolve",2,msolvetoeplitz},
	{"beta",2,mbeta},
	{"betai",3,mbetai},
	{"invbetai",3,minvbetai},
	{"besselj",2,mbesselj},
	{"bessely",2,mbessely},
	{"besselallr",2,mbesselall},
	{"besseli",2,mbesseli},
	{"besselk",2,mbesselk},
	{"besselmodallr",2,mbesselmodall},
	{"pswindow",1,mpswindow},
	{"playwave",1,mplaywav},
#ifdef DLL
	{"dll",3,mdll},
	{"closedll",1,mclosedll},
#endif
	{"addpage",0,maddframe},
	{"addpage",2,maddframe2},
	{"showpage",1,mshowframe},
	{"copytopage",1,mcopyto},
	{"copyfrompage",1,mcopyfrom},
	{"pages",0,mframes},
	{"deletepages",0,mdeleteframes},
	{"clip",1,mclip},
	{"clip",0,mclip0},
	{"antialiasing",0,mantialiasing0},
	{"antialiasing",1,mantialiasing},
#ifdef YACAS
	{"yacas",1,myacas},
	{"yacasclear",0,myacasclear},
	{"yacaseval",1,myacaseval},
#endif
	{"mxmstart",1,mxmstart},
	{"mxmstop",0,mxmstop},
	{"mxm",1,mxm},
	{"insimg",4,minsertimage},
	{"insimglines",1,minsertimagelines},
	{"loadimg",3,mloadimage},
	{"loadanaglyph",3,mloadagimages},
	{"getline",0,mgetline},
	{"real",1,mreal},
	{"atan2",2,mtan2},
	{"arctan2",2,mtan2},
	{"zerorounding",1,mzerorounding},
	{"setstatus",1,msetstatus},
	//{"plotimg",2,mplotimg},
	{"NAN",0,mnan},
	{"isNAN",1,misnan},
	{"frac",1,mfrac},
	{"convertmxm",1,mconvertmxm},
	{"polygon",3,mpolygon},
	{"givensrot",5,mgivensrot},
	{"givensqr",2,mgivensqr},
	{"cpx",1,mcompress},
	{"decpx",1,mdecompress},
	{"cpxmult",2,mcompressmult},
	{"cpxseidel",4,mcpxseidel},
	{"cpxset",2,mcpxset},
	{"cpxget",1,mcpxget},
	{"cpxzeros",1,mcpxzeros},
	{"cpxzeros",2,mcpxzeros},
	{"cpxtranspose",1,mcpxtranspose},
	{"loadrgb",1,mloadpixels},
	{"savergb",2,msavepixels},
	{"texpng",5,mtexpng},
	{"getblue",1,mgetblue},
	{"getgreen",1,mgetgreen},
	{"getred",1,mgetred},
	{"putblue",1,mputblue},
	{"putgreen",1,mputgreen},
	{"putred",1,mputred},
	{"insrgb",3,minsertpixels},
	{"fold",2,mfold},
	{"bitand",2,mbitand},
	{"bitor",2,mbitor},
	{"bitxor",2,mbitxor},
	{"bitnot",1,mbitnot},
	{"totalmax",1,mtotalmax},
	{"totalmin",1,mtotalmin},
	{"relerror",2,mrelerror},
	{"none",0,mnone},
	{"scalp",2,mscalp},
	{"setanaglyph",1,msetanaglyph},
	{"savepng",4,msavepng},
	{"savesvg",3,msavesvg},
	{"saveps",1,msaveps},
	{"center",1,mcenter},
	{"center",0,mcenter0},
	{"all",1,mall},
	{"toscreen",1,mtoscreen},
	{"fromscreen",1,mfromscreen},
	{"margin",1,mmargin},
	{"lsoda",-1,mlsoda},
	{"magic",1,mmagic},
	{"filter",4,mfilter},
	{"plotcubes",1,mplotcubes},
	{"plotcubes",2,mplotcubes},
	{"tex",1,mtex},
	{"exec",6,mexec},
	{"execread",0,mexecread},
	{"execwrite",1,mexecwrite},
	{"execwriteln",1,mexecwriteln},
	{"execkill",0,mexeckill},
	{"message",1,mmessage},
	{"eulerarg",1,meulerarg},
	{"subgrid",1,msubgrid},
	{"setfont",2,msetfont},
	{"setfont",0,msetfont0},
	{"getaspect",0,maspect0},
	{"aspect",1,maspect},
	{"getnow",1,mgetnow},
	{"printnow",1,mprintnow},
	{"filecopy",2,mfilecopy},
	{"fileexists",1,mfileexists},
	{"fileremove",1,mfileremove},
	{"tic",0,mtic},
	{"toc",0,mtoc},
	{"python",1,mpython},
	{"pyload",1,mloadpython},
	{"pyload",1,mloadpython},
	{"pyget",1,mpyget},
	{"pyset",2,mpyset},
	{"pycall",-1,mpyeval},
	{"hatchgridsize",1,mhatchgridsize},
	{"drop",2,mrdrop},
	{"alsolve",3,malsolve},
	{"aleigen",2,maleigen},
	{"quantile",2,mquantile},
	{"bindis",3,mbindis},
	{"bincdis",3,mbincdis},
	{"invpbindis",3,minvpbindis},
	{"setdecimaldot",1,msetdecimaldot},
	{"alintegrate",-1,malintegrate},
	{"alsingular",-1,malsingular},
	{"win64",0,mwin64},
	{"stacksize",0,mgetstacksize},
	{"gvar",1,mglobalget},
	{"gset",2,mglobalput},
	{"gremove",1,mglobalrem},
	{"gclear",0,mglobalclear},
	{"glist",1,mglist},
	{"glistadd",2,mglistadd},
	{"glistlength",1,mglistlength},
	{"glistget",2,mglistget},
	{"glistput",3,mglistput},
	{"glistinsert",3,mglistinsert},
	{"glistdelete",2,mglistdelete},
	{"glistremove",1,mglistrem},
	{"urlopen",1,murlopen},
	{"urlgetline",0,murlgetline},
	{"urlclose",0,murlclose},
	{"urleof",0,murleof},
	{(char *)0,0,0} 
};

using namespace std;

typedef std::unordered_map <string, int> Mymap;

/* 
Global hash table containg all names of functions.
Note that some names of functions are in the builtin list more than
once with different counts of arguments.
*/
Mymap *BuiltinFunctions=0;

/**
Store the index of the name in the builtin list.
*/
void globalputbuiltin (char *name, int i)
{
	BuiltinFunctions->erase(name);
	BuiltinFunctions->emplace(name,i);
}


/* 
Compare two built-in functions for sorting
*/
int builtin_compare (const builtintyp *p1, const builtintyp *p2)
{	
	int h;
	h=strcmp(p1->name,p2->name);
	if (h) return h;
	else
	{	if (p1->nargs==-1 || p2->nargs==-1) return 0;
		else if (p1->nargs<p2->nargs) return -1;
		else if (p1->nargs>p2->nargs) return 1;
		else return 0;
	}
}

/**
Called at program start.
Sorts the builtin list and puts each function name once
into a hash table. Put the first occurance of the name
in the builtin list into the hash table only!
*/
void sort_builtin (void)
{	
	builtin_count=0;
	while (builtin_list[builtin_count].name) builtin_count++;
	qsort(builtin_list,builtin_count,sizeof(builtintyp),
		(int (*) (const void *, const void *))builtin_compare);

	if (BuiltinFunctions!=0) delete BuiltinFunctions;
	BuiltinFunctions=new Mymap();
	for (int i=0; i<builtin_count; i++)
	{
		if (i==0 || strcmp(builtin_list[i].name,builtin_list[i-1].name)!=0)
			globalputbuiltin(builtin_list[i].name,i);
	}
}

/**
Find a builtin function in the hash table and the builtin list.
*/
builtintyp *find_builtin (char *name, int nargs)
{	
	int retry=0;
startover:
	Mymap::iterator it=BuiltinFunctions->find(name);
	if (it!=BuiltinFunctions->end()) // have that key
	{
		int i=BuiltinFunctions->at(name);
		// Search for correct number of arguments.
		// Note: the builtin-list is sorted by name.
		while (i<builtin_count)
		{
			if (builtin_list[i].nargs==-1 || nargs==-1 || builtin_list[i].nargs==nargs) 
				return builtin_list+i;
			i++;
			if (strcmp(builtin_list[i].name,name)!=0) return 0;
		}
	}
	if (!retry)
	{
		retry=1;
		name=getalias(name);
		if (name) goto startover;
	}
	return 0;
}

/**
Execute a builtin command. Return 0 if it was not found.
*/
int exec_builtin (char *name, int nargs, header *hd)
{	
	builtintyp *b=find_builtin(name,nargs);
	if (b)
	{	
		if (nargs==0) newram=(char *)hd;
		if (nargs==0) hd=0;
		b->f(hd); return 1;
	}
	else return 0;
}
