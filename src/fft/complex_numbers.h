#ifndef COMPLEX_NUMBERS_H_
#define COMPLEX_NUMBERS_H_

typedef struct complex_number {
	float Re;
	float Im;
}complex;

complex complexAdd(complex arg1, complex arg2);
complex complexMultiply(complex arg1, complex arg2);
complex complexNegate(complex arg);
float complexMagnitudeSquared(complex arg);

#endif /* COMPLEX_NUMBERS_H_ */
