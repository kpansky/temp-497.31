/* File which contains definition of complex number data type as well as complex math functions */

#include "complex_numbers.h"

/* Adds two complex numbers
 * Parameters: two complex numbers to add
 * Returns: a complex data type containing arg1+arg2
 */
complex complexAdd(complex arg1, complex arg2)
{
	complex retval;
	retval.Re = arg1.Re + arg2.Re;
	retval.Im = arg1.Im + arg2.Im;
	return retval;
}

/* Multiplies two complex numbers
 * Parameters: Two complex numbers
 * Returns: a complex data type containing arg1*arg2
 */
complex complexMultiply(complex arg1, complex arg2)
{
	complex retval;
	retval.Re = (arg1.Re*arg2.Re) + (-1)*(arg1.Im*arg2.Im);
	retval.Im = (arg1.Re*arg2.Im) + (arg1.Im*arg2.Re);
	return retval;
}

/* Multiplies a complex number by -1
 * Parameters: Complex data type
 * Returns: arg*-1
 */
complex complexNegate(complex arg)
{
	complex retval;
	retval.Re = (-1)*arg.Re;
	retval.Im = (-1)*arg.Im;
	return retval;
}

/* Computes the magnitude squared of a complex number
 * Parameters: Complex data type
 * Returns: a floating point number containing the magnitude squared
 */
float complexMagnitudeSquared(complex arg)
{
	return (arg.Re*arg.Re)+(arg.Im*arg.Im);
}

