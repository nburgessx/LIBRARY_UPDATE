#include <string>
#include <boost/algorithm/string.hpp>

#include "HedgeCurveEngine.h"
#include "ObjectUtilities.h"
#include "AQOUtilities.h"
#include "CommonConstants.h"
#include "InitializeETrading.h"

#include "AQLDateScheduleHelpers.h"
#include "AQLCurveForwardRateHelpers.h"
#include "AQLCurvePricingObject.h"
#include "CurveCalibrationData.h"
#include "AQLCoreComponentManager.h"
#include "AQLDefinitions.h"
#include "AQLDataReference.h"
#include "AQLMathDefine.h"
#include "DataUtilities.h"  // For AQ_TO_STRING macros

namespace
{

	/* @brief: Helper function which examines the swap conventions in the provided CurveGenerator and
	 *         extracts the conventions required to reprice swap instruments
	 *
	 * param[in] curveGenerator		The input swap curve generator
	 * param[out] resetLag			The resetLag specified in the curve generator
	 * param[out] businessDayAdjust	The businessDayAdjust / date sliding rule specified in the curve generator
	 * param[out] calendar			The calendar(s) specified in the curve generator
	 */
	AQLDate getSwapEffectiveDateFromCurve( etrading::CurveGeneratorPtr curveGenerator, AQLDate asOfDate  )
	{
		// Fetch the Swap Conventions block from the Curve Generator
		AQLStringMatrix swapConventions = curveGenerator->toAQLStringMatrix(  etrading::GENERATOR_COMPONENTS::KEY_SWAPS );

		// Iterate through the Swap Conventions looking for specific keys of interest
		AQLString resetLag;
		AQLString businessDayAdjust;
		AQLString calendar;
		for (size_t i=0; i< swapConventions.size(); i++)
		{
			const AQLStringVector& row = swapConventions[i];
			std::string key( row[0].getCString() );
			
			if ( boost::iequals( key, etrading::CURVEGENERATOR_SWAPS_KEY::RESET_LAG ) )
			{
				resetLag = row[1];
			}
			else if ( boost::iequals( key, etrading::CURVEGENERATOR_SWAPS_KEY::SLIDING_RULE ) )
			{
				businessDayAdjust = row[1];
			}
			else if ( boost::iequals(key, etrading::CURVEGENERATOR_SWAPS_KEY::CALENDAR ) )
			{
				calendar = row[1];
			}
		}

		// Append the 'D' tenor string to the resetLag if it appears to be missing
		if ( resetLag.findString( 'D' ) < 0 )
		{
			resetLag += 'D';
		}

		AQLDate effectiveDate = etrading::AQLDateScheduleHelpers::getDate( asOfDate, resetLag, businessDayAdjust, calendar );
		return effectiveDate;
	}
}


namespace etrading
{

	/* @brief	Constructor for the HedgeCurveEngine.
	*			Stores the curve generators and curve market data object names used to define the hedge curve.
	*			
	* @param[in]	lwoHedgeOISCurveGeneratorName	OIS Curve Generator name
	* @param[in]	lwoHedgeOISCurveMarketDataName	OIS market data object name
	* @param[in]	lwoHedgeOISCurveGeneratorName	Swap Curve Generator name: Specifies the hedge curve config
	* @param[in]	lwoHedgeOISCurveMarketDataName	Swap market data object name: Specifies the swap tenors to use in the hedge curve
	*/
	HedgeCurveEngine::HedgeCurveEngine( const std::string& lwoHedgeOISCurveGeneratorName,
										const std::string& lwoHedgeOISCurveMarketDataName,
										const std::string& lwoHedgeSwapCurveGeneratorName,
										const std::string& lwoHedgeSwapCurveMarketDataName )
										  : lwoHedgeOISCurveGeneratorName_( lwoHedgeOISCurveGeneratorName ),
											lwoHedgeOISCurveMarketDataName_( lwoHedgeOISCurveMarketDataName ),
											lwoHedgeSwapCurveGeneratorName_( lwoHedgeSwapCurveGeneratorName ),
											lwoHedgeSwapCurveMarketDataName_( lwoHedgeSwapCurveMarketDataName )
    {
		// Set up the Swap Expression LVB used for repricing swap calibration instruments
		AQLStringVector keys; 
		AQLStringVector values;
		bool validateKeys = true;

		// We standardize these trade keys because we are only interested in par rate calculations 
	    keys.push_back( etrading::IRS_KEY::PAY_RECEIVE.c_str() );					values.push_back( AQLString( "PAY" ) );
		keys.push_back( etrading::IRS_KEY::NOTIONAL.c_str() );						values.push_back( AQLString( "1.0" ) );
		keys.push_back( etrading::IRS_KEY::EFFECTIVE_DATE.c_str() );				values.push_back( AQLString( "" ) );
		keys.push_back( etrading::IRS_KEY::MATURITY_DATE.c_str() );					values.push_back( AQLString( "" ) );
		keys.push_back( etrading::SWAP_EXPRESSION_KEY::RATE_OR_SPREAD1.c_str() );	values.push_back( AQLString( "0.0" ) );
		keys.push_back( etrading::SWAP_EXPRESSION_KEY::RATE_OR_SPREAD2.c_str() );	values.push_back( AQLString( "0.0" ) );

		repriceSwapExpressionLVB_ = etrading::populateLabelValueBlock( keys, values );
	}

	/* @brief		Reprices the swaps in the hedge-curve using the specified pricing curve collection
	*  @param[in]	pricingCurveCollection	The curveCollection containing the curves used for instrument repricing
	*  @param[in]	lwoSwapGeneratorName	The name of the swap generator which holds the config used to calculate the par-rate for each swap calibration instrument
	*/
	void HedgeCurveEngine::repriceHedgeInstruments( const std::string& pricingCurveCollection,
												    const std::string& lwoSwapGeneratorName )
	{

		CurveGeneratorPtr lwoHedgeCurveGenerator  = getCurveGenerator(  lwoHedgeSwapCurveGeneratorName_ );
		CurveMarketDataPtr lwoHedgeCurveMarketData = getCurveMarketData( lwoHedgeSwapCurveMarketDataName_ );

		// Get the asOf date from the hedge curve, compute an effective date
		const LabelValueBlock marketDataPropertiesLVB = lwoHedgeCurveMarketData->toLabelValueBlock( GENERATOR_COMPONENTS::KEY_MARKETDATAPROPERTIES );
		const std::string marketDataAsOfDate          = marketDataPropertiesLVB.getCompulsoryValue( "AsOfDate" );

		AQLDate asOfDate( AQLDateScheduleHelpers::getAQLDate( marketDataAsOfDate.c_str() ) );
		AQLDate effectiveDate = getSwapEffectiveDateFromCurve( lwoHedgeCurveGenerator, asOfDate );

		/* 
		*  This next section Reprices all of the calibration swaps using the pricing curve
		*/
		LabelValueBlock fixingTables;
		LabelValueBlock swapPropertiesLVB;
		const bool isXccySwap = false;

		LabelValueBlock curveCollections( pricingCurveCollection, StandardString("") );

		// Get all of the calibration swap maturity dates
		AQLStringMatrix swapTenorsAndRates  = lwoHedgeCurveMarketData->toAQLStringMatrix( GENERATOR_COMPONENTS::KEY_SWAPS );
		const size_t numSwaps = swapTenorsAndRates.size();

		// Iterate over all of the calibration swaps and calculate a par-rate for each swap using the pricing curve
		for ( size_t i=0; i<numSwaps; i++)
		{
            AQLString swapTenor = swapTenorsAndRates[i][0];
            
            StandardStringVector tradeDateKeys(2);
            tradeDateKeys[0] = IRS_KEY::EFFECTIVE_DATE;
            tradeDateKeys[1] = IRS_KEY::MATURITY_DATE;

            StandardStringVector tradeDateValues(2);
            tradeDateValues[0] = AQ_TO_STRING_FROM_INT( AQLDateScheduleHelpers::getExcelDate(effectiveDate) );
            tradeDateValues[1] = swapTenor.c_str();

            repriceSwapExpressionLVB_ = LabelValueBlock( repriceSwapExpressionLVB_, tradeDateKeys, tradeDateValues );

			auto swapInstrument = createSwapFromGenerator( GENERATOR_COMPONENTS::KEY_SWAPS, lwoSwapGeneratorName, repriceSwapExpressionLVB_, swapPropertiesLVB, isXccySwap );
			
			double parRate = swapInstrument->parRate(curveCollections, fixingTables  );
			// Now clear /delete the swap
			swapTenorsAndRates[i][1] = AQLString( parRate );
		}

		// Set the updated swap parRates back into the Swap MarketData Local Cache
        // *** Important Note *** This is a local market data area in memory, which is not persisted as part of the market data object itself. We cannot update ...
        // ... the underlying free object itself, we can make a copy of the object or update the market data local cache which acts as an override to the market data
		lwoHedgeCurveMarketData->setMarketDataLocalCache( GENERATOR_COMPONENTS::KEY_SWAPS, swapTenorsAndRates );
	}


	/* @brief		Builds the hedge curve and stores it in the specified hedgeCurveCollection.
	*  @param[in]	oisCurveObjectName		The name of the LWO handle corresponding to the OIS curve which will be built
	*  @param[in]	swapCurveObjectName		The name of the LWO handle corresponding to the Swap curve which will be built
	*  @param[in]	hedgeCurveCollection	The CurveCollection which will contain the hedge OIS and Swap curve
	*/
	HedgeCurveInfo HedgeCurveEngine::buildHedgeCurve( const std::string& oisCurveObjectName,
												      const std::string& swapCurveObjectName,
												      const std::string& hedgeCurveCollection ) const
	{
		// First build the OIS curve from the provided generator and store in the hedgeCurveCollection
		CurveGeneratorPtr lwoHedgeOISCurveGenerator  = getCurveGenerator(  lwoHedgeOISCurveGeneratorName_ );
		CurveMarketDataPtr lwoHedgeOISCurveMarketData = getCurveMarketData( lwoHedgeOISCurveMarketDataName_ );

		const std::string foreignCurveCollection; // Dummy curve collection name only for xccy swaps
		SingleCurveObject oisCurveObject( oisCurveObjectName, lwoHedgeOISCurveGeneratorName_, lwoHedgeOISCurveMarketDataName_, hedgeCurveCollection, foreignCurveCollection );
		oisCurveObject.calibrateCurve();
        copyToCache<etrading::SingleCurveObject>( oisCurveObject );

		// Build the swap hedge curve using the provided generator and updated market data. Store the curve in the hedgeCurveCollection.
		SingleCurveObject swapCurveObject( swapCurveObjectName, lwoHedgeSwapCurveGeneratorName_, lwoHedgeSwapCurveMarketDataName_, hedgeCurveCollection, foreignCurveCollection );
		swapCurveObject.calibrateCurve();
        copyToCache<etrading::SingleCurveObject>( swapCurveObject );

        // Return a HedgeCurveInfo block so that all the curve names, curve indices are known to the end client
        HedgeCurveInfo hedgeResults;
        hedgeResults.oisCurveName    = oisCurveObjectName;
        hedgeResults.oisCurveIndex   = oisCurveObject.getCurveIndexName();
        hedgeResults.swapCurveName   = swapCurveObjectName; 
        hedgeResults.swapCurveIndex  = swapCurveObject.getCurveIndexName();

		return hedgeResults;
	}


}

