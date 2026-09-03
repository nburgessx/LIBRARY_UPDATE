#include "tryMeLWOCurveCalibrateHedge.h"

#include "CreateDataFile.h"
#include "LWOUtilities.h"
#include "StructuredExceptionHandler.h"
#include "TypeHelpers.h"
#include "RecordMacros.h"

using etrading::CreateDataFile;
using etrading::decorateFilename;


namespace validation
{
    /* @brief			Validation interface for the meLWOCurveCalibrateHedge method. This method calibrates a hedge curve to a pricing curve
	*  @param [in]		oisCurveObjectName			LWO Object name of the ois hedge curve
	*  @param [in]		swapCurveObjectName			LWO Object name of the swap hedge curve
    *  @param [in]		pricingCurveCollection		The CurveCollection containing the swap curve used for pricing
    *  @param [in]		hedgeCurveCollection		The CurveCollection in which the generated hedge curve will be placed
	*  @param [in]		oisCurveGeneratorName		The CurveGenerator used to build the OIS curve in the hedge curve collection
    *  @param [in]		oisCurveMarketDataName		The Curve MarketData used to build the OIS curve
	*  @param [in]		swapCurveGeneratorName		The CurveGenerator used to build the Swap hedge curve in the hedge curve collection
    *  @param [in]		swapCurveMarketDataName		The Curve MatketData used to build the Swap hedge curve. Specifies the swap tenors which will appear in the hedge curve
	*  @param [out]		swapGeneratorName			Swap generator containing the config used to reprice swap calibration instruments
	*  @returns			The HedgeCurveInfo struct containing all the Hedge curve names and curve index values
    */
	etrading::HedgeCurveInfo tryMeLWOCurveCalibrateHedge( const std::string& oisCurveObjectName,
											              const std::string& swapCurveObjectName,
											              const std::string& pricingCurveCollection,
											              const std::string& hedgeCurveCollection,
											              const std::string& oisCurveGeneratorName,
											              const std::string& oisCurveMarketDataName,
											              const std::string& swapCurveGeneratorName,
											              const std::string& swapCurveMarketDataName,
											              const std::string& swapGeneratorName )
    {
        VALID_EXCEPTION_START

		RECORD_INPUTS( oisCurveObjectName, swapCurveObjectName, pricingCurveCollection, hedgeCurveCollection, oisCurveGeneratorName, oisCurveMarketDataName, swapCurveGeneratorName, swapCurveMarketDataName, swapGeneratorName );

		if (oisCurveObjectName.empty() )
		{
			throw LACoreInvalidData("#Error: No OIS curve object name has been provided.",__FILE__,__LINE__);
		}

		if (swapCurveObjectName.empty() )
		{
			throw LACoreInvalidData("#Error: No Swap curve object name has been provided.",__FILE__,__LINE__);
		}

		// Initialise the hedge curve engine with curve generators and curve market data
		etrading::HedgeCurveEngine hedgeCurveEngine( oisCurveGeneratorName, oisCurveMarketDataName, swapCurveGeneratorName, swapCurveMarketDataName );

		// Use the pricing curves to reprice the swap calibration instruments in the hedge curve
		hedgeCurveEngine.repriceHedgeInstruments( pricingCurveCollection, swapGeneratorName );		

		// Build the hedge curve containing repriced swap calibration instruments, which returns the HedgeCurveInfo struct results
		etrading::HedgeCurveInfo hedgeResults = hedgeCurveEngine.buildHedgeCurve( oisCurveObjectName, swapCurveObjectName, hedgeCurveCollection );

        // Record the Hedge Curve Info Results, which is a vector of hedge curve names and curve index values
        std::vector< std::string > hedgeCurveInfoVector;
        hedgeCurveInfoVector.push_back( hedgeResults.oisCurveName );
        hedgeCurveInfoVector.push_back( hedgeResults.oisCurveIndex );
        hedgeCurveInfoVector.push_back( hedgeResults.swapCurveName );
        hedgeCurveInfoVector.push_back( hedgeResults.swapCurveIndex );
        RECORD_OUTPUTS( hedgeCurveInfoVector );
        
        // Return the HedgeCurveInfo struct result
        return hedgeResults;

        VALID_EXCEPTION_END
    }

}


