#pragma once

#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"

namespace validation
{


    /* @brief		validation interface for the meSABRCalibrate method
    */
    const AQLString tryMeSABRCalibrate(const AQLString& approxMethod,
											const AQLStringVector& calibFlg, 
											const AQLString& calibMethod, 
											const AQLString& curveSetID, 
											const AQLString& alphaID, 
											const AQLString& betaID,
											const AQLString& nuID, 
											const AQLString& rhoID, 
											const AQLString& convID,
											const AQLString& capConvID, 
											const AQLStringVector& swapVolID, 
											const AQLString& target, 
											const DoubleVector& weight, 
											const IntVector& sgn, 
											const AQLString& forwardID, 
											const double forwardShiftValue,
											const AQLString& numeraireID,
											const AQLStringMatrix& curveMat,
											const std::string& volType = "");

}

