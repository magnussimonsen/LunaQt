#include <malloc.h>

template <class type> class Vector
{
public :

	type *h;
	int n,ncap;

	Vector (int cap)
	{
		ncap=cap;
		h=(type *)malloc(ncap*sizeof(type));
		n=0;
	}
	~Vector ()
	{
		free(h);
	}
	int add (type p);
	int remove (int i);
	int removeAll ();
};

template <class type> int Vector<type>::add (type p)
{
	if (n==ncap-1)
	{
		type *hnew=(type *)malloc(2*ncap*sizeof(type));
		for (int i=0; i<n; i++) hnew[i]=h[i];
		free(h);
		h=hnew;
		ncap=ncap*2;
	}
	h[n++]=p;
	return 0;
}

template <class type> int Vector<type>::remove (int i)
{
	for (int j=i; j<n-1; j++)
	{
		h[j]=h[j+1];
	}
	n--;
	return 0;
}

template <class type> int Vector<type>::removeAll ()
{
	n=0;
	return 0;
}