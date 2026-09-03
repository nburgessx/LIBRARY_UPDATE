/*
 * @brief			validation interface for the meSABRCalibrate method
 * @Created:		18 October 2018
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */


#pragma once

#include "LACoreTemplateType.h"
#include "LADataInstance.h"

namespace validation_api
{


    /* @brief		validation interface for the meSABRCalibrate method
    */
    const LAString tryMeSABRCalibrate(const LAString& approxMethod,
											const LAStringVector& calibFlg, 
											const LAString& calibMethod, 
											const LAString& curveSetID, 
											const LAString& alphaID, 
											const LAString& betaID,
											const LAString& nuID, 
											const LAString& rhoID, 
											const LAString& convID,
											const LAString& capConvID, 
											const LAStringVector& swapVolID, 
											const LAString& target, 
											const DoubleVector& weight, 
											const IntVector& sgn, 
											const LAString& forwardID, 
											const double forwardShiftValue,
											const LAString& numeraireID,
											const LAStringMatrix& curveMat,
											const std::string& volType = "");

}

