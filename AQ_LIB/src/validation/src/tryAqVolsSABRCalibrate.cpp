#include "tryAqVolsSABRCalibrate.h"
#include "RecordMacros.h"

#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "AQLMathSwaptionVolUtility.h"
#include "InitializeETrading.h"
#include "AQLCoreUtility.h"
#include "ScheduleValidation.h"
#include "ParameterValidation.h"

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation
{

    /* @brief	validation interface for the aqVolsSABRCalibrate method
    */
    const AQLString tryAqVolsSABRCalibrate(const AQLString& approxMethod,
											const AQLStringVector& calibFlag,
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
											const IntVector& sign,
											const AQLString& forwardID, 
											const double forwardShiftValue,
											const AQLString& numeraireID, 
											const AQLStringMatrix& curveMat,
											const std::string& volType )
    {
		VALID_EXCEPTION_START

		if (CreateDataFile::recordEnabled())
		{
			CreateDataFile file("tryMeCalibrateSABRMatrix");
			file.write("generatorFunction", "tryMeSABRCalibrate");
			file.write("approxMethod", approxMethod);
			file.write("calibFlag", calibFlag);
			file.write("calibMethod", calibMethod);
			file.write("curveSetID", curveSetID);
			file.write("alphaID", alphaID);
			file.write("betaID", betaID);
			file.write("nuID", nuID);
			file.write("rhoID", rhoID);
			file.write("convID", convID);
			file.write("swapVolID", swapVolID);
			file.write("target", target);
			file.write("weight", weight);
			file.write("sign", sign);
			file.write("forwardID", forwardID);
			file.write("forwardShiftValue", forwardShiftValue);
			file.write("numeraireID", numeraireID);
			file.write("curveMat", curveMat);
			file.write("volType", volType);
		}

		std::vector<bool> calibFlag_bool;
		for (const auto& b : calibFlag)
		{	
			calibFlag_bool.push_back(AQLString(b).toUpper() == "TRUE" ? true : false);
		}

		if (forwardID != AQLString("") && convID != AQLString(""))
		{
			AQ_THROW("Do not input ForwardID and Convention ID at the same time")
		}

		AQLDataInstance* dataInstance = etrading::InitializeETrading::instance().dataInstance();

		AQLString curveID("");
		if (forwardID == AQLString(""))
		{
			if (curveMat.size() == 0)
			{
				AQ_THROW("Must provide curve matrix when forwardID is not given.")
			}
			else
			{
				AQLStringMatrix temp(curveMat);
				curveID = searchbyrow(temp, "CurveID", 1, true);
				AQLMathSwaptionVolUtility::setCurveID2(dataInstance, curveID, temp);
			}
		}

		bool isLognormal = (etrading::toVolatilityTypeEnum(volType) != etrading::NORMAL_VOLATILITY);

		AQLString msg;
		AQLMathSwaptionVolUtility::calibrateSABRMatrix(dataInstance,
													approxMethod,
													calibFlag_bool,
													calibMethod,
													curveID,
													alphaID,
													betaID,
													nuID,
													rhoID,
													convID,
													capConvID,
													swapVolID,
													target,
													weight,
													sign,
													forwardID,
													forwardShiftValue,
													numeraireID,
													msg,
													0, //Use the default value
													isLognormal);

		if (msg == AQLString(""))
		{
			msg = "Successfully calibrate SABR Matrix";
		}
		
		return msg;

        VALID_EXCEPTION_END
    }

}
