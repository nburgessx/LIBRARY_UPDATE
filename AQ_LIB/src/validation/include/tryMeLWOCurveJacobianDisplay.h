/*
 * @brief			validation interface for the meLWOCurveEngineJacobianDisplay method
 * @Created:		26 April 2018
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "Variant.h"

#include <string>
#include <vector>
#include "LACoreTemplateType.h"

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
											bool displayInverseMatrix);

	/* @brief Display the jacobian matrix of one yield curve 
	* @param [out] matrix							Matrix being returned and displayed
	* @param [in] curveCollection					Curve collection name
	* @param [in] curveName							Name of yield curve
	* @param [in] displayInverseMatrix				Display inverse Jacobian matrix or not
	*/
	void tryMeLWOCurveJacobianDisplay(DoubleMatrix& matrix,
											const LAString& curveCollection,
											const LAString& curveName,
											bool displayInverseMatrix);
}
