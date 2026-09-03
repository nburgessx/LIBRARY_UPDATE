#include "tryMeSABRCalibrate.h"
#include "RecordMacros.h"

#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "LAMathSwaptionVolUtility.h"
#include "InitializeMLibETrading.h"
#include "LACoreUtility.h"
#include "ScheduleValidation.h"
#include "ParameterValidation.h"

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation_api
{

    /* @brief	validation interface for the meSABRCalibrate method
    */
    const LAString tryMeSABRCalibrate(const LAString& approxMethod,
											const LAStringVector& calibFlag,
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
											const IntVector& sign,
											const LAString& forwardID, 
											const double forwardShiftValue,
											const LAString& numeraireID, 
											const LAStringMatrix& curveMat,
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
			calibFlag_bool.push_back(LAString(b).toUpper() == "TRUE" ? true : false);
		}

		if (forwardID != LAString("") && convID != LAString(""))
		{
			AQ_THROW("Do not input ForwardID and Convention ID at the same time")
		}

		LADataInstance* dataInstance = etrading::InitializeMLibETrading::instance().dataInstance();

		LAString curveID("");
		if (forwardID == LAString(""))
		{
			if (curveMat.size() == 0)
			{
				AQ_THROW("Must provide curve matrix when forwardID is not given.")
			}
			else
			{
				LAStringMatrix temp(curveMat);
				curveID = searchbyrow(temp, "CurveID", 1, true);
				LAMathSwaptionVolUtility::setCurveID2(dataInstance, curveID, temp);
			}
		}

		bool isLognormal = (etrading::toVolatilityTypeEnum(volType) != etrading::NORMAL_VOLATILITY);

		LAString msg;
		LAMathSwaptionVolUtility::calibrateSABRMatrix(dataInstance,
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

		if (msg == LAString(""))
		{
			msg = "Successfully calibrate SABR Matrix";
		}
		
		return msg;

        VALID_EXCEPTION_END
    }

}
