 #include "dlldef.h"
 #include <stdio.h>
 
 double hfib (int n)
 {
     if (n<2) return n;
     return hfib(n-1)+hfib(n-2);
 }
 
 EXPORT char * fib (header *hd[], int np, char *ramstart, char *ramend)
 {
     start(ramstart,ramend); // required !!!
 
     int n=getint(hd[0]);
     IFERROR("Need an integer");
     
     new_real(hfib(n));
     
     return newram;
 }
 
 EXPORT char * fibv (header *hd[], int np, char *ramstart, char *ramend)
 {
     start(ramstart,ramend); // required !!!
 
     int n=getint(hd[0]);
     IFERROR("Need an integer");
     CHECK(n>1,"Need an interger >1");
     
     header *res=new_matrix(1,n+1);
     double *m=matrixof(res);
     m[0]=0; m[1]=1;
     int i;
     for (i=2; i<=n; i++) m[i]=m[i-1]+m[i-2];
     
     return newram;
 }
