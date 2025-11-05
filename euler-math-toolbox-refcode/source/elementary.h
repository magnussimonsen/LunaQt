#ifndef ELEMENTARY
#define ELEMENTARY

void equal_params_3 (header **hd1, header **hd2, header **hd3);
void equal_params_2 (header **hd1, header **hd2);
void real_add (double *x, double *y, double *z);
void complex_add (double *x, double *xi, double *y, double *yi,
	double *z, double *zi);
void add (header *hd, header *hd1);
void real_subtract (double *x, double *y, double *z);
void complex_subtract (double *x, double *xi, double *y, double *yi,
	double *z, double *zi);
void subtract (header *hd, header *hd1);
void real_multiply (double *x, double *y, double *z);
void complex_multiply (double *x, double *xi, double *y, double *yi,
	double *z, double *zi);
void dotmultiply (header *hd, header *hd1);
void real_divide (double *x, double *y, double *z);
void complex_divide (double *x, double *xi, double *y, double *yi,
	double *z, double *zi);
void dotdivide (header *hd, header *hd1);
void cscalp (double *s, double *si, double *x, double *xi,
	double *y, double *yi);
void iscalp (double *s, double *si, double *x, double *xi,
	double *y, double *yi);
void ccopy (double *y, double *x, double *xi);
void icopy (double *y, double *x, double *xi);
void multiply (header *hd, header *hd1);
void divide (header *hd, header *hd1);
void real_invert (double *x, double *y);
void complex_invert (double *x, double *xi, double *y, double *yi);
void invert (header *hd);
void copy_complex (double *x, double *y);

#endif