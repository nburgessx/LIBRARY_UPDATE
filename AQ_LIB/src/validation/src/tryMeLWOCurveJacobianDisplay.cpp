/*
* @brief			validation interface for the meLWOCurveEngineJacobianDisplay method
* @Created:			26 April 2018
* @Author:			Joseph Ye
* @Department:		ISD Front Office Development
*
* The copyright to the computer program(s) herein is the property of Mizuho International.
*/

#include "tryMeLWOCurveJacobianDisplay.h"

#include "RecordMacros.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "CurveUtilities.h"
#include "ExceptionMacros.h"

using etrading::CreateDataFile;
using etrading::decorateFilename;

namespace validation_api
{
	/* @brief Display the yield curve engine jacobian matrix
	* @param [out] matrix							Matrix being returned and displayed
	* @param [out] labelMatrix						Matrix that has label for each jacobian matrix element
	* @param [in] curveEngineObject					The name of the dual-bootstrapped object
	* @param [in] curveCollection					Curve collection name
	* @param [in] displayLabels						Display labels of the matrix
	* @param [in] displayInverseMatrix				Display inverse Jacobian matrix or not
	*/
	void tryMeLWOCurveEngineJacobianDisplay(DoubleMatrix& matrix,
											LAStringMatrix& labelMatrix,
											const LAString& curveEngineObject,
											const LAString& curveCollection,
											bool displayLabels,
											bool displayInverseMatrix)
    {
		VALID_EXCEPTION_START
	
		// Record Inputs for logs, tests and playback
        RECORD_INPUTS(curveEngineObject, displayLabels, displayInverseMatrix);
		
		if (curveCollection.size() == 0)
		{
			throw LACoreInvalidData("#Error: Curve collection name has not been provided.", __FILE__, __LINE__);
		}

		if (curveEngineObject.size() == 0)
		{
			throw LACoreInvalidData("#Error: Curve engine object name has not been provided.", __FILE__, __LINE__);
		}

		etrading::displayCurveEngineJacobian(matrix,
											labelMatrix,
											curveEngineObject,
											curveCollection,
											displayLabels,
											displayInverseMatrix);

		VALID_EXCEPTION_END
    };

	/* @brief Display the jacobian matrix of one yield curve
	* @param [out] matrix							Matrix being returned and displayed
	* @param [in] curveCollection					Curve collection name
	* @param [in] curveName							Name of yield curve
	* @param [in] displayInverseMatrix				Display inverse Jacobian matrix or not
	*/
	void tryMeLWOCurveJacobianDisplay(DoubleMatrix& matrix,
										const LAString& curveCollection,
										const LAString& curveName,
										bool displayInverseMatrix)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(curveCollection, curveName, displayInverseMatrix);

		if (curveCollection.size() == 0)
		{
			throw LACoreInvalidData("#Error: Curve collection name has not been provided.", __FILE__, __LINE__);
		}

		if (curveName.size() == 0)
		{
			throw LACoreInvalidData("#Error: Curve name has not been provided.", __FILE__, __LINE__);
		}

		etrading::displayCurveJacobian(matrix,
										curveCollection,
										curveName,
										displayInverseMatrix);

		VALID_EXCEPTION_END
	}
}