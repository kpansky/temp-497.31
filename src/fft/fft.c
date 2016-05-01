/* File which contains FFT implementation.
   The radix-2 decimation in time algorithm is used.
   Maximum supported FFT size is 256 (bounded by bit_reverse_order()).  */

#include "fft.h"

complex** Wn_k;   //Arrays containing the complex Wn^k required for FFT
complex unity = {.Re = 1, .Im = 0};
complex negative_unity = {.Re = -1, .Im = 0};

const unsigned char lookup[16] = {  //Lookup table for bit-reversed order
	0x0, 0x8, 0x4, 0xC, 0x2, 0xA, 0x6, 0xE,
	0x1, 0x9, 0x5, 0xD, 0x3, 0xB, 0x7, 0xF,
};


/* Initializes the complex Wn^k that will be used to compute the FFT.
 * Each iteration of the FFT requires a different Wn array.  For example,
 * the size 2 iteration requires a size 1 Wn^k array, the size 4 iteration
 * requires a size 2 Wn^k array, and so on.  The size of the Wn^k array is half
 * the size of the FFT since we can take advantage of the symmetry of the Wns.
 * Parameters: none
 * Returns: void
 */
void init_Wn()
{
	int num_Wn_arrays = logTwo(MAX_FFT_SIZE);
	Wn_k = malloc(sizeof(complex*)*num_Wn_arrays);
	for(int i=0; i<num_Wn_arrays; ++i)
	{
		int N = powTwo(i+1);                       //N = FFT size
		Wn_k[i] = malloc(sizeof(complex)*N/2);     //Wn^k = exp(-j*2PI*k/N), 0<=k<=N-1

		for(int k=0; k<N/2; ++k)
		{
			//Each Wn^k is a complex value with a real (cosine) and imaginary (sine) component
			Wn_k[i][k].Re = twoTermTrigApprox((-1)*2*PI*k/N, COS);
			Wn_k[i][k].Im = twoTermTrigApprox((-1)*2*PI*k/N, SINE);
		}
	}
}

/* Frees the memory associated with the Wn^k arrays
 * Parameters: none
 * Returns: void
 */
void teardown_Wn()
{
	int num_Wn_arrays = logTwo(MAX_FFT_SIZE);
	for(int i=0; i<num_Wn_arrays; ++i)
	{
		free(Wn_k[i]);
	}
	free(Wn_k);
}

/* Computes an FFT
 * Parameters: samples - an array of complex numbers containing the sample values
 *             size - the size of the sample array.  Must be a power of 2
 * Returns: the FFT result overwrites the samples array. A pointer to this array is
 *          returned, or NULL if the parameters are invalid
 */
complex * fft(complex * samples, int size)
{
	if(samples == NULL || logTwo(size) == -1 || size > MAX_FFT_SIZE)
		return NULL;

	//Put the samples array into bit-reversed order. This is required to
	//break down the larger FFT into smaller FFTs on the even and odd samples
	bit_reverse_order(samples, size);

	//Calculate the FFT recursively
	int recursion_depth = logTwo(size);
	calculate_fft(samples, recursion_depth);

	return samples;
}

/* Iterative helper function used to calculate an FFT
 * Parameters: input - a pointer to an array containing 2^levels samples
 *             levels - the depth of the binary tree containing the N/2 size FFTs
 * Returns: void - this is an "in place" FFT. The input array is overwritten with the results
 */
void calculate_fft(complex * input, int levels)
{
	int N, num_FFTs_per_level;
	int fft_size = powTwo(levels);
	complex * inputPtr = input;
	complex in0, in1, Wn;

	for(int i=0; i<levels; ++i)
	{
		N = powTwo(i+1);
		num_FFTs_per_level = fft_size/N;

		for(int j=0; j<num_FFTs_per_level; ++j)
		{
			for(int k=0; k<N/2; ++k)
			{
				in0 = *inputPtr;
				in1 = *(inputPtr+N/2);
				Wn = Wn_k[i][k];

				//Butterfly
				*inputPtr = complexAdd(in0, complexMultiply(Wn, in1));
				*(inputPtr+N/2) = complexAdd(in0, complexMultiply(complexNegate(Wn), in1));

				inputPtr++;
			}
			//Jump to the next N/2 FFT in this level
			inputPtr += N/2;
		}

		//Reset the pointer to the top of the input array for the next FFT level
		inputPtr = input;
	}
}

/* Computes the base 2 logarithm of the argument
 * Parameters: a power of 2
 * Returns: the base 2 logarithm, or -1 if the argument was not a power of 2
 */
int logTwo(int arg)
{
	//Base case
	if(arg == 1)
		return 0;
	else if (arg < 2 || arg % 2 != 0)
		return -1;
	else if(arg == 2)
		return 1;

	//Recursive case
	else
	{
		int val = logTwo(arg/2);
		return (val < 0) ? -1 : val+1;
	}
}

/* Computes a power of 2
 * Parameters: the exponent
 * Returns: 2^exponent
 */
int powTwo(int exponent)
{
	int result = 1;
	for(int i=0; i<exponent; ++i)
	{
		result = result * 2;
	}
	return result;
}

/* Takes an array of complex numbers and bit-reverses the order
 * Parameters: samples - the array to bit reverse. This array will be
 *                       overwritten with the bit reversed array
 *             size - the size of the samples array. Maximum is 256.
 * Returns: void
 */
void bit_reverse_order(complex * samples, int size)
{
	//Maximum fft size is 256, so we can always represent the array index using a uint8
	uint8_t index, reversed_index, size_uint8;
	int shift_size = 8 - logTwo(size);
	size_uint8 = (uint8_t)(size-1);

	for(index=0; index <= size_uint8; ++index)
	{
		//Reverse top and bottom half of the byte and then swap them
		reversed_index = (lookup[index & 0b1111] << 4 | lookup[index >> 4]) >> shift_size;
		if(index < reversed_index)
		{
			complex temp = samples[reversed_index];
			samples[reversed_index] = samples[index];
			samples[index] = temp;
		}

		//This check is required to prevent infinite looping when size = 256
		if(index == 255)
			break;
	}
	return;
}



