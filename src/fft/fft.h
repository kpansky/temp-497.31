#ifndef FFT_H_
#define FFT_H_

#include <stdlib.h>
#include <stdint.h>
#include "complex_numbers.h"
#include "trig_approximations.h"

#define MAX_FFT_SIZE 256

extern complex** Wn_k;   //Arrays containing the complex Wn^k required for FFT
extern complex unity;
extern complex negative_unity;

extern const unsigned char lookup[16];

void init_Wn();
void teardown_Wn();
complex * fft(complex * samples, int size);
void calculate_fft(complex * input, int levels);
int logTwo(int arg);
int powTwo(int exponent);
void bit_reverse_order(complex * samples, int size);

#endif /* FFT_H_ */
