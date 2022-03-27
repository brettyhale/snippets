/*******************************************************************************
*
* minv4.c : efficient [4x4] matrix inverse (scalar)
*
* Copyright (c) 2022 Brett Hale.
* SPDX-License-Identifier: BSD-2-Clause
*
*******************************************************************************/

#include <float.h>
#include <math.h>

/******************************************************************************/

/* efficient [4x4] matrix inverse (scalar) : */

/* return false if the matrix is determined to be singular, in which
 * case the matrix argument is unmodified. precision limits imply that
 * the result (and the test for invertibility) will be affected by the
 * conditioning of the matrix.

 * note: running sums of signed terms are used in this implementation,
 * therefore, the results are subject to cancellation errors. */


int invert_4x4 (float m[4][4])
{
    /* use the first row of co-factors to compute the determinant: */

    float *m3 = m[3];
    float m30 = m3[0], m31 = m3[1], m32 = m3[2], m33 = m3[3];
    float *m2 = m[2];
    float m20 = m2[0], m21 = m2[1], m22 = m2[2], m23 = m2[3];

    float d01 = m20 * m31 - m30 * m21;
    float d12 = m21 * m32 - m31 * m22;
    float d23 = m22 * m33 - m32 * m23;
    float d30 = m23 * m30 - m33 * m20;
    float d02 = m20 * m32 - m30 * m22;
    float d13 = m21 * m33 - m31 * m23;

    float *m1 = m[1];
    float m10 = m1[0], m11 = m1[1], m12 = m1[2], m13 = m1[3];

    float c00 = + (m11 * d23 - m12 * d13 + m13 * d12);
    float c01 = - (m12 * d30 + m13 * d02 + m10 * d23);
    float c02 = + (m13 * d01 + m10 * d13 + m11 * d30);
    float c03 = - (m10 * d12 - m11 * d02 + m12 * d01);

    float *m0 = m[0];
    float m00 = m0[0], m01 = m0[1], m02 = m0[2], m03 = m0[3];

    float det = m00 * c00 + m01 * c01 + m02 * c02 + m03 * c03;

    if (fabsf(det) < FLT_EPSILON)
        return (0);

    /* 28 fmuls required for the determinant. the intermediate
     * values and co-factors are reused for the first two columns
     * of the inverse: */

    det = 1.0f / det;

    m0[0] = c00 * det;
    m1[0] = c01 * det;
    m2[0] = c02 * det;
    m3[0] = c03 * det;

    m0[1] = - (m01 * d23 - m02 * d13 + m03 * d12) * det;
    m1[1] = + (m02 * d30 + m03 * d02 + m00 * d23) * det;
    m2[1] = - (m03 * d01 + m00 * d13 + m01 * d30) * det;
    m3[1] = + (m00 * d12 - m01 * d02 + m02 * d01) * det;

    /* 48 fmuls for the first two columns of the inverse (and a
     * floating-point divide / reciprocal operation). compute the
     * final two columns of the inverse: */

    d01 = (m00 * m11 - m10 * m01) * det;
    d12 = (m01 * m12 - m11 * m02) * det;
    d23 = (m02 * m13 - m12 * m03) * det;
    d30 = (m03 * m10 - m13 * m00) * det;
    d02 = (m00 * m12 - m10 * m02) * det;
    d13 = (m01 * m13 - m11 * m03) * det;

    m0[2] = + (m31 * d23 - m32 * d13 + m33 * d12);
    m1[2] = - (m32 * d30 + m33 * d02 + m30 * d23);
    m2[2] = + (m33 * d01 + m30 * d13 + m31 * d30);
    m3[2] = - (m30 * d12 - m31 * d02 + m32 * d01);

    m0[3] = - (m21 * d23 - m22 * d13 + m23 * d12);
    m1[3] = + (m22 * d30 + m23 * d02 + m20 * d23);
    m2[3] = - (m23 * d01 + m20 * d13 + m21 * d30);
    m3[3] = + (m20 * d12 - m21 * d02 + m22 * d01);

    /* 42 fmuls for the final two columns of the inverse, or 90
     * fmuls in total. this is a *very* good (branch-free) result. */

    return (1);
}

/******************************************************************************/
