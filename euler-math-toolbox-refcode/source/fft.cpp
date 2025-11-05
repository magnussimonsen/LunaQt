#include <stdio.h>
#include <math.h>

#include "header.h"
#include "matheh.h"
#include "sysdep.h"


/************** fft *****************/

int nn;
complex *ff,*zz,*fh;
extern char *ram;

void rfft (long m0, long p0, long q0, long n)
/***** rfft 
	make a fft on x[m],x[m+q0],...,x[m+(p0-1)*q0] (p points).
	one has xi_p0 = xi_n^n = zz[n] ; i.e., p0*n=nn.
*****/
{	long p,q,m,l;
	long mh,ml;
	int found=0;
	complex sum,h;
	if (test_key()==escape) { error=301; return; }
	if (p0==1) return;
	if (p0%2==0) { p=p0/2; q=2; }
	else
	{	q=3;
		while (q*q<=p0)
		{	if (p0%q==0) 
			{	found=1; break; }
			q+=2;
		}
		if (found) p=p0/q;
		else { q=p0; p=1; }
	}
	if (p>1) 
		for (m=0; m<q; m++) 
		{	rfft((m0+m*q0)%nn,p,q*q0,nn/p);
			if (error) return;
		}
	mh=m0;
	for (l=0; l<p0; l++)
	{	ml=l%p;
		c_copy(sum,ff[(m0+ml*q*q0)%nn]);
		for (m=1; m<q; m++)
		{	c_mult(ff[(m0+(m+ml*q)*q0)%nn],zz[(n*l*m)%nn],h);
			c_add(sum,h,sum);
		}
		if (test_key()==escape) { error=301; return; }
		sum[0]/=q; sum[1]/=q;
		c_copy(fh[mh],sum);
		mh+=q0; if (mh>=nn) mh-=nn;
	}
	for (l=0; l<p0; l++)
	{	c_copy(ff[m0],fh[m0]);
		m0+=q0;
	}
}

void fft (double *a, int n, int signum)
{	complex z;
	double h;
	int i;
	
	if (n==0) return;
	nn=n;

	ram=newram;
	if (!freeram(2*(1+n)*sizeof(complex)))
	{	output1("Stack overflow in fft!\n");
		error=1; return;
	}
	ff=(complex *)a;
	zz=(complex *)ram;
	ram+=n*sizeof(complex);
	fh=(complex *)ram;
	ram+=n*sizeof(complex);

	/* compute zz[k]=e^{-2*pi*i*k/n}, k=0,...,n-1 */
	h=2*M_PI/n; z[0]=cos(h); z[1]=signum*sin(h);
	zz[0][0]=1; zz[0][1]=0;
	for (i=1; i<n; i++)
	{	if (i%16) { zz[i][0]=cos(i*h); zz[i][1]=signum*sin(i*h); }
		else c_mult(zz[i-1],z,zz[i]);
	}
	rfft(0,n,1,1);
	if (signum==1)
		for (i=0; i<n; i++)
		{	ff[i][0]*=n; ff[i][1]*=n;
		}
}

#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr

void fftn (double data[], unsigned long nn[], int ndim, int isign)
{
	int idim;
	unsigned long i1,i2,i3,i2rev,i3rev,ip1,ip2,ip3,ifp1,ifp2;
	unsigned long ibit,k1,k2,n,nprev,nrem,ntot;
	double tempi,tempr;
	double theta,wi,wpi,wpr,wr,wtemp;

	for (ntot=1,idim=0; idim<ndim; idim++)
		ntot *= nn[idim];
	nprev=1;
	for (idim=ndim-1; idim>=0; idim--) {
		n=nn[idim];
		nrem=ntot/(n*nprev);
		ip1=nprev << 1;
		ip2=ip1*n;
		ip3=ip2*nrem;
		i2rev=1;
		for (i2=1; i2<=ip2; i2+=ip1) {
			if (i2 < i2rev) {
				for (i1=i2; i1<=i2+ip1-2; i1+=2) {
					for (i3=i1; i3<=ip3; i3+=ip2) {
						i3rev=i2rev+i3-i2;
						SWAP(data[i3],data[i3rev]);
						SWAP(data[i3+1],data[i3rev+1]);
					}
				}
			}
			if (test_key()==escape) { error=301; return; }
			ibit=ip2 >> 1;
			while (ibit >= ip1 && i2rev > ibit) {
				i2rev -= ibit;
				ibit >>= 1;
			}
			i2rev += ibit;
		}
		ifp1=ip1;
		while (ifp1 < ip2) {
			ifp2=ifp1 << 1;
			theta=isign*6.28318530717959/(ifp2/ip1);
			wtemp=sin(0.5*theta);
			wpr = -2.0*wtemp*wtemp;
			wpi=sin(theta);
			wr=1.0;
			wi=0.0;
			for (i3=1; i3<=ifp1; i3+=ip1) {
				for (i1=i3; i1<=i3+ip1-2; i1+=2) {
					for (i2=i1; i2<=ip3; i2+=ifp2) {
						k1=i2;
						k2=k1+ifp1;
						tempr=(double)wr*data[k2]-(double)wi*data[k2+1];
						tempi=(double)wr*data[k2+1]+(double)wi*data[k2];
						data[k2]=data[k1]-tempr;
						data[k2+1]=data[k1+1]-tempi;
						data[k1] += tempr;
						data[k1+1] += tempi;
					}
				}
				if (test_key()==escape) { error=301; return; }
				wr=(wtemp=wr)*wpr-wi*wpi+wr;
				wi=wi*wpr+wtemp*wpi+wi;
			}
			ifp1=ifp2;
		}
		nprev *= n;
	}
}

#undef SWAP

