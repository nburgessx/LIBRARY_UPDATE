// tryMeUtilityAppend.h

/*
 * @brief			Functions to appendMatrixData
 * @Created:		22 September 2017
 * @Author:			Nicholas Burgess
 * @Department:		ISD Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once
#include "DataUtilities.h"

namespace validation
{
    using etrading::VariantMatrix;

    /* @brief			Function to append matrix data
    *  @param [in]		appendByRow	                Boolean: TRUE = Append to Bottom by Row, FALSE = Append to the Right by Column
    *  @param [in]		matrix1		                input matrix 1
    *  @param [in]		matrix2		                input matrix 2
    *  @param [in]		matrix3		                input matrix 3
    *  @param [in]		matrix4		                input matrix 4
    *  @param [in]		matrix5		                input matrix 5
    *  @param [in]		matrix6		                input matrix 6
    *  @param [in]		matrix7		                input matrix 7
    *  @param [in]		matrix8		                input matrix 8
    *  @param [in]		matrix9		                input matrix 9
    *  @param [in]		matrix10	                input matrix 10
    *  @return			Returns an augmented matrix
    */
    VariantMatrix tryMeUtilityAppend( const bool        appendByRow,
                                      const VariantMatrix &   matrix1,
                                      const VariantMatrix &   matrix2,
                                      const VariantMatrix &   matrix3 = VariantMatrix(),
                                      const VariantMatrix &   matrix4 = VariantMatrix(),
                                      const VariantMatrix &   matrix5 = VariantMatrix(),
                                      const VariantMatrix &   matrix6 = VariantMatrix(),
                                      const VariantMatrix &   matrix7 = VariantMatrix(),
                                      const VariantMatrix &   matrix8 = VariantMatrix(),
                                      const VariantMatrix &   matrix9 = VariantMatrix(),
                                      const VariantMatrix &   matrix10 = VariantMatrix() );

}