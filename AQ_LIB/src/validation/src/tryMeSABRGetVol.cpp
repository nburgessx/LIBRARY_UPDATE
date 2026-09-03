#include "tryMeSABRGetVol.h"

#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "LAMathSwaptionVolUtility.h"
#include "InitializeMLibETrading.h"
#include <algorithm>
#include "RecordMacros.h"

using namespace etrading;

namespace validation_api
{
	/* @brief			validation interface for tryMeSABRGetVol
	*  @param [in]		expiryPoint		Expiry tenor
	*  @param [in]		tenorPoint		Swap tenor tenor
	*  @param [in]		strike			Strike
	*  @param [in]		forwardID		ID for forward rate matrix
	*  @param [in]		alphaID			ID for alpha matrix
	*  @param [in]		betaID			ID for beta matrix
	*  @param [in]		rhoID			ID for rho matrix
	*  @param [in]		approxMethod	Approximation method, HAGAN or ANTONOVE?
	*  @param [in]		shift			shift size of the Shifted SABR
	*  @param [in]		volType			volType of the SABR, default to lognormal
	*/
	double tryMeSABRGetVol(const LAString& expiryPoint,
							const LAString& tenorPoint,
							double strike,
							const LAString& forwardID,
							const LAString& alphaID,
							const LAString& betaID,
							const LAString& nuID,
							const LAString& rhoID,
							const LAString& approxMethod,
							double shift,
							const std::string& volType)
	{
		VALID_EXCEPTION_START
		
		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(expiryPoint, tenorPoint, strike, forwardID, alphaID, betaID, nuID, rhoID, approxMethod, shift, volType);
		
		LAString sabrMethod = approxMethod;
		if (sabrMethod.size() == 0)
		{
			sabrMethod = APPROXIMATION_HAGAN;
		}

		bool isLognormal = (etrading::toVolatilityTypeEnum(volType) != etrading::NORMAL_VOLATILITY);

		double ret = LAMathSwaptionVolUtility::getSABRVol(etrading::InitializeMLibETrading::instance().dataInstance(),
														expiryPoint,
														tenorPoint,
														strike,
														forwardID,
														alphaID,
														betaID,
														nuID,
														rhoID,
														sabrMethod,
														shift,
														isLognormal);
		
		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(ret);

		return ret;

		VALID_EXCEPTION_END
	}
}
