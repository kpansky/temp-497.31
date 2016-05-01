/* File which contains Taylor series approximations for SINE and COSINE. */

#include "trig_approximations.h"

/* Computes a two-term Taylor series approximation of either a sine or a cosine.
 * The center of the approximation is shifted every PI/2 radians so that each approximation
 * is always within PI/4 of its center.
 * Parameters: angleRadians - the argument of the sine or cosine, between -2pi and 2pi
 *             type - 0 for SINE, 1 for COSINE
 * Returns: a floating point approximation of the sine or cosine value
 */
float twoTermTrigApprox(float angleRadians, int type)
{
    float angle, approximation;
    int invertResult = 0;

    //If angle is negative, make it positive
    if(angleRadians < 0)
        angle = angleRadians + 2*PI;
    else
        angle = angleRadians;

    //If input is a COSINE, shift to a SINE
    if(type == COS)
    {
        angle += PI/2;
        //Handle edge case when type is cosine and angleRadians is greater than 3PI/2
        if(angle >= 2*PI)
            angle -= 2*PI;
    }

    //If the angle is greater than PI, take advantage of SINE symmetry
    if(angle > PI)
    {
        angle -= PI;
        invertResult = 1;
    }

    //Taylor series approximations (center is changed every PI/2 to improve accuracy)
    float A_n = 0;   //Taylor series coefficient
    if(angle <= PI/4)
    {
        A_n = angle;
        approximation = A_n - (A_n * A_n * A_n)/6;
    }
    else if (angle <= 3*PI/4)
    {
        A_n = angle - PI/2;
        approximation = 1 - (A_n*A_n)/2;
    }
    else
    {
        A_n = angle - PI;
        approximation = (-1)*A_n + (A_n * A_n * A_n)/6;
    }

    return invertResult ? (-1)*approximation : approximation;
}

/* Converts degrees to radians
 * Parameters: degrees
 * Returns: radians
 */
float convert_to_radians(float degrees)
{
    return degrees*PI/180;
}



