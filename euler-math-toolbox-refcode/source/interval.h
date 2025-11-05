#ifndef INTERVALH
#define INTERVALH

#include "header.h"

double round_up (double x);
double round_down (double x);

void minterval (header *hd);
void minterval1 (header *hd);

void make_interval (header *hd);

void interval_add (double *, double *, double *, double *, double *, double *);
void interval_sub (double *, double *, double *, double *, double *, double *);
void interval_mult (double *, double *, double *, double *, double *, double *);
void interval_div (double *, double *, double *, double *, double *, double *);

void imax (double *, double *, double *, double *, double *, double *);
void imin (double *, double *, double *, double *, double *, double *);

void interval_invert (double *, double *, double *, double *);

void icos (double *, double *, double *, double *);
void isin (double *, double *, double *, double *);
void itan (double *, double *, double *, double *);
void iatan (double *, double *, double *, double *);
void iasin (double *, double *, double *, double *);
void iacos (double *, double *, double *, double *);
void iexp (double *, double *, double *, double *);
void ilog (double *, double *, double *, double *);
void iabs (double *, double *, double *, double *);
void isqrt (double *, double *, double *, double *);
void idegree (double *, double *, double *, double *);
void ipercent (double *, double *, double *, double *);
void ipow (double *, double *, double *, double *, double *, double *);
void ior (double *a, double *b, double *a1, double *b1,
	double *ar, double *br);
void iintersects (double *a, double *b, double *a1, double *b1,
	double *ar);
void iand (double *a, double *b, double *a1, double *b1,
	double *ar, double *br);


void miless (header *hd);
void milesseq (header *hd);

void copy_interval (double *, double *);

void i_solvesim (double *a, int n, double *b, int m, double *c);

void ipeval (double *, double *, double *, double *);
void iddeval (double *, double *, double *, double *);

typedef double interval[2];

void i_add (interval x, interval y, interval z);
void i_sub (interval x, interval y, interval z);
void i_mult (interval x, interval y, interval z);
void i_div (interval x, interval y, interval z);
double i_abs (interval x);
void i_copy (interval x, interval y);

void accuinit (void);

void plusminus (header *, header *);

#endif