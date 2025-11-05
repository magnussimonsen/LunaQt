#ifndef EXPRESSH
#define EXPRESSH

void equal_params_3 (header **hd1, header **hd2, header **hd3);
void equal_params_2 (header **hd1, header **hd2);
void add (header *, header *);
header * scan_maxima_expression ();
int scan_maxima_expression (char *res, int length);

#endif