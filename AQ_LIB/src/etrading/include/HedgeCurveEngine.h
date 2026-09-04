#pragma once

#include <string>
#include "LabelValueBlock.h"



namespace etrading
{
    struct HedgeCurveInfo
    {
        std::string oisCurveName;
        std::string oisCurveIndex;
        std::string swapCurveName;
        std::string swapCurveIndex;
    };

	/* @brief	the HedgeCurveEngine class has the ability to construct a hedge curve which is based on
	 * a pricing curve, but potentially contains different calibration instruments.
	 * This is useful for example in viewing the risk of a portfolio expressed in different instruments.
	 */
    class HedgeCurveEngine
    {
    public:
		/* @brief	Constructor for the HedgeCurveEngine.
		 *			Stores the curve generators and curve market data object names used to define the hedge curve.
		 *			
		 * @param[in]	aqObjHedgeOISCurveGeneratorName	OIS Curve Generator name
		 * @param[in]	aqObjHedgeOISCurveMarketDataName	OIS market data object name
		 * @param[in]	aqObjHedgeOISCurveGeneratorName	Swap Curve Generator name: Specifies the hedge curve config
		 * @param[in]	aqObjHedgeOISCurveMarketDataName	Swap market data object name: Specifies the swap tenors to use in the hedge curve
		 */
		HedgeCurveEngine( const std::string& aqObjHedgeOISCurveGeneratorName,
						  const std::string& aqObjHedgeOISCurveMarketDataName,
						  const std::string& aqObjHedgeSwapCurveGeneratorName,
						  const std::string& aqObjHedgeSwapCurveMarketDataName );

		/* @brief	Reprices the swaps in the hedge-curve using the specified pricing curve collection
		 * @param[in]	pricingCurveCollection	The curveCollection containing the curves used for instrument repricing
		 * @param[in]	aqObjSwapGeneratorName	The name of the swap generator which holds the config used to calculate the par-rate for each swap calibration instrument
		 */
		void repriceHedgeInstruments( const std::string& pricingCurveCollection,
									  const std::string& aqObjSwapGeneratorName );

		/* @brief		Builds the hedge curve and stores it in the specified hedgeCurveCollection.
		 * @param[in]	oisCurveObjectName		The name of the AQObj object handle corresponding to the OIS curve which will be built
		 * @param[in]	swapCurveObjectName		The name of the AQObj object handle corresponding to the Swap curve which will be built
		 * @param[in]	hedgeCurveCollection	The CurveCollection which will contain the hedge OIS and Swap curve
		 */
		HedgeCurveInfo buildHedgeCurve( const std::string& oisCurveObjectName,
									    const std::string& swapCurveObjectName,
									    const std::string& hedgeCurveCollection ) const;

        ~HedgeCurveEngine() {}

	private:
		std::string aqObjHedgeOISCurveGeneratorName_;
		std::string aqObjHedgeOISCurveMarketDataName_;
		std::string aqObjHedgeSwapCurveGeneratorName_;
		std::string aqObjHedgeSwapCurveMarketDataName_;
		LabelValueBlock repriceSwapExpressionLVB_;

    };


}
