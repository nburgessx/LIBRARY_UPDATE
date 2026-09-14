#include "tryAqCurveObjectJacobianDisplay.h"

#include "RecordMacros.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "CurveUtilities.h"
#include "ExceptionMacros.h"

using etrading::CreateDataFile;
using etrading::decorateFilename;

namespace validation
{
	/* @brief Display the yield curve engine jacobian matrix
	* @param [out] matrix							Matrix being returned and displayed
	* @param [out] labelMatrix						Matrix that has label for each jacobian matrix element
	* @param [in] curveEngineObject					The name of the dual-bootstrapped object
	* @param [in] curveCollection					Curve collection name
	* @param [in] displayLabels						Display labels of the matrix
	* @param [in] displayInverseMatrix				Display inverse Jacobian matrix or not
	*/
	void tryAqCurveObjectEngineJacobianDisplay(DoubleMatrix& matrix,
											AQLStringMatrix& labelMatrix,
											const AQLString& curveEngineObject,
											const AQLString& curveCollection,
											bool displayLabels,
											bool displayInverseMatrix)
    {
		VALID_EXCEPTION_START
	
		// Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS(curveEngineObject, displayLabels, displayInverseMatrix);
		
		AQ_THROW_IF( curveCollection.size() == 0, "Curve collection name has not been provided." );

		AQ_THROW_IF( curveEngineObject.size() == 0, "Curve engine object name has not been provided." );

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
	void tryAqCurveObjectJacobianDisplay(DoubleMatrix& matrix,
										const AQLString& curveCollection,
										const AQLString& curveName,
										bool displayInverseMatrix)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS(curveCollection, curveName, displayInverseMatrix);

		AQ_THROW_IF( curveCollection.size() == 0, "Curve collection name has not been provided." );

		AQ_THROW_IF( curveName.size() == 0, "Curve name has not been provided." );

		etrading::displayCurveJacobian(matrix,
										curveCollection,
										curveName,
										displayInverseMatrix);

		VALID_EXCEPTION_END
	}
}