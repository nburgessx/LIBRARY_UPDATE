// tryAqToolsAppend.cpp

/*
 * @brief			Functions to append matrix data
 * @Created:		22 September 2017
 * @Author:			Nicholas Burgess
 * @Department:		ISD Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "tryAqToolsAppend.h"
#include "ExceptionMacros.h"
#include "DataUtilities.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "RecordMacros.h"


namespace validation
{

    using etrading::CreateDataFile;
    using etrading::decorateFilename;


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
    VariantMatrix tryAqToolsAppend( const bool        appendByRow,
                                      const VariantMatrix &   matrix1,
                                      const VariantMatrix &   matrix2,
                                      const VariantMatrix &   matrix3,
                                      const VariantMatrix &   matrix4,
                                      const VariantMatrix &   matrix5,
                                      const VariantMatrix &   matrix6,
                                      const VariantMatrix &   matrix7,
                                      const VariantMatrix &   matrix8,
                                      const VariantMatrix &   matrix9,
                                      const VariantMatrix &   matrix10 )
    {
         VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( appendByRow, matrix1, matrix2, matrix3, matrix4, matrix5, matrix6, matrix7, matrix8, matrix9, matrix10 );

        VariantMatrix result;

        if ( appendByRow )
        {
            result = etrading::appendRows( matrix1, matrix2, matrix3, matrix4, matrix5, matrix6, matrix7, matrix8, matrix9, matrix10 );
        }
        else
        {
            result = etrading::appendColumns( matrix1, matrix2, matrix3, matrix4, matrix5, matrix6, matrix7, matrix8, matrix9, matrix10 );
        }

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }

}