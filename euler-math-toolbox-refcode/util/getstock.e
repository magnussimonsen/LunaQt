// Get Stock Data from Yahoo

comment
Downlaods and displays stock data from Yahoo historical courses.

Load with "load getstock". Help with "help getstock.e". See the
snippets in the user menu.
endcomment

function getstock (stock,since,till=none,showstock=false,
	a=none,b=none,c=none,d=none)
## Read the stock data since a date
##
## stock : GOOGL, ALV.DE, SIE.DE etc.
## since : e.g. day(2013,1,1)
## till : e.g. day(2014,1,1)
## >showstock : plots the stock
## a,b,c,d : boundaries for the stock plot in years and prices.
##
## >getstock("ALV.DE",day(2012,1,1),>showstock):
## >getstock("SIE.DE",day(2012,1,1),day(2013,1,1),>showstock):
##
## See: showstock
	s="https://ichart.finance.yahoo.com/table.csv?s="+stock;
	{ye,mo,da}=date(since); da=1; 
	s=s+''&a=''+(mo-1)+''&b=''+da+''&c=''+ye;
	if till then
		{ye,mo,da}=date(till);
	else
		{ye,mo,da}=date(daynow); 
	endif;
	s=s+''&d=''+(mo-1)+''&e=''+da+''&f=''+ye;
	s=s+''&g=d&ignore=.csv'';
	v=[];
	urlopen(s);
	heading=strtokens(urlgetline(),",");
	repeat
	   line=strtokens(urlgetline(),",");
	   if length(line)<2 then break; endif;
	   date=day(line[1]);
	   value=line[5]();
	   v=v_[date,value];
	   until urleof();
	end;
	urlclose();
	v=flipud(v);
	if showstock then showstock(v,=a,=b,=c,=d,title=stock); endif;
	return v;
endfunction

function map getyear (dd)
	{y,m,d}=date(dd);
	return y+(m-1+(d-1)/31)/12;
endfunction

function showstock (v,a=none,b=none,c=none,d=none,title="")
## Display the stock data
##
## >v=getstock("GOOGL",day(2012,1,1));
## >showstock(v,a=2012,b=2015,c=500,d=1500,title="Google Prices"):
##
## See: getstock
	v=v'; y=getyear(v[1]);
	if a==none then plot2d(y,v[2],grid=0);
	else plot2d(y,v[2],=a,=b,=c,=d,grid=0);
	endif;
	frame();
	p=plot();
	xgrid(ticks(p[1],p[2]),grid=6); 
	ygrid(ticks(p[3],p[4]));
	settitle(title);
endfunction

addmenu getstock("?stock",day(?year,?month,?day)):
addmenu showstock(?vector):
