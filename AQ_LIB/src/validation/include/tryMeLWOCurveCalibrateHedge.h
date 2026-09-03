#pragma once

#include "LACoreTemplateType.h"
#include "LADataInstance.h"
#include "LabelValueBlock.h"
#include "HedgeCurveEngine.h" // Included here to declare the 'HedgeCurveInfo' struct return type

using etrading::LabelValueBlock;

namespace validation_api
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
											              const std::string& swapGeneratorName );
										


}

