#pragma once

#include "Variant.h"

#include <string>
#include <vector>
#include "AQLCoreTemplateType.h"

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
	void tryAqObjCurvesEngineJacobianDisplay(DoubleMatrix& matrix,
											AQLStringMatrix& labelMatrix,
											const AQLString& curveEngineObject,
											const AQLString& curveCollection,
											bool displayLabels,
											bool displayInverseMatrix);

	/* @brief Display the jacobian matrix of one yield curve 
	* @param [out] matrix							Matrix being returned and displayed
	* @param [in] curveCollection					Curve collection name
	* @param [in] curveName							Name of yield curve
	* @param [in] displayInverseMatrix				Display inverse Jacobian matrix or not
	*/
	void tryAqObjCurvesJacobianDisplay(DoubleMatrix& matrix,
											const AQLString& curveCollection,
											const AQLString& curveName,
											bool displayInverseMatrix);
}
