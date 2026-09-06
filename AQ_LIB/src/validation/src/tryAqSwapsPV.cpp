#include "tryAqSwapsPV.h"

#include "RecordMacros.h"
#include "CurveInstrumentPricing.h"
#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"
#include "CommonConstants.h"

using etrading::CreateDataFile;
using etrading::decorateFilename;

namespace validation
{

    /* @brief			return a set of expected keys for swap pv label value block
    *  @return			expected keys
    */
    std::vector<std::string> tryAqSwapsPVLVBKeys()
    {
        const std::string arr[] =
        {
            etrading::MARKET_KEY::CURVE_COLLECTION
            , etrading::MARKET_KEY::FORECAST_CURVE
            , etrading::MARKET_KEY::DISCOUNT_CURVE
            , etrading::IRS_KEY::NOTIONAL
            , etrading::IRS_KEY::PAY_RECEIVE
            , etrading::IRS_KEY::PAYER_RECEIVER
            , etrading::IRS_KEY::FIXED_RATE
            , etrading::IRS_KEY::FLOAT_SPREAD
        };

        std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );
        std::vector<std::string> commonKeys = etrading::getSwapCommonLVBKeys();
        expectedKeys.insert( expectedKeys.end(), commonKeys.begin(), commonKeys.end() );

        return expectedKeys;
    }

    /* @brief			validation interface for the aqSwapsPV method
    *  @param [in]		swapLVB		A label value block defining the swap
    *  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
    *  @return			Swap PV
    */
    double tryAqSwapsPV( const LabelValueBlock& swapLVB, bool validateKeys )
    {
        VALID_EXCEPTION_START

        etrading::validateKeysForLVB( tryAqSwapsPVLVBKeys(), swapLVB.getKeys(), validateKeys );

        const std::string inputLVB = "SwapLVB";
        AQLString curveCollection = swapLVB.getCompulsoryValueAsAQLString( etrading::MARKET_KEY::CURVE_COLLECTION, inputLVB );

        // Recording of inputs for playback
        AQ_RECORD_DECORATED_INPUTS( curveCollection, "", swapLVB );


        //----------------------------------------------------------------------------------
        // Validate non-cash flow related parameters
        AQLString forecastCurveIndex	= swapLVB.getCompulsoryValueAsAQLString( etrading::MARKET_KEY::FORECAST_CURVE, inputLVB );
        AQLString discountCurveIndex = swapLVB.getCompulsoryValueAsAQLString( etrading::MARKET_KEY::DISCOUNT_CURVE, inputLVB );

        //Throw exception if the curve has not been built.
        etrading::getCurveStaticDataTableName( curveCollection, discountCurveIndex );
        AQLString forecastCurveStaticDataTable = etrading::getCurveStaticDataTableName( curveCollection, forecastCurveIndex );

        // Synchronize the OIS pricing interpolation with that used within the OIS Curve calibration routine
        AQLString interpolation = etrading::getCurveInterpolation( curveCollection, forecastCurveStaticDataTable );

        //----------------------------------------------------------------------------------
        // Validate swap trade specific parameters

        // Payer or Receiver Swap i.e. Pay or Receive Fixed Coupons
        AQLString payRec	= swapLVB.getCompulsoryValueAsAQLStringFromKeys( etrading::IRS_KEY::PAYER_RECEIVER, etrading::IRS_KEY::PAY_RECEIVE , inputLVB );
        bool isFixedRatePayerSwap = etrading::validateSwapPayRecFlag( payRec );


        //----------------------------------------------------------------------------------
        // Validate cash flow related parameters and generate cash flows

        DateVector fixedAccrualDates;
        DateVector fixedPaymentDates;
        DateVector floatAccrualDates;
        DateVector floatFixingDates;
        DateVector floatPaymentDates;
        etrading::validateAndGenerateSwapCashflows( swapLVB, inputLVB, fixedAccrualDates, fixedPaymentDates, floatFixingDates, floatAccrualDates, floatPaymentDates );


        //----------------------------------------------------------------------------------
        // Get the Swap Par Rates using the full swap definition for both legs of the swap

        // Float Leg Fixing Parameters
        double floatFirstFix = swapLVB.getOptionalValueAsDouble( etrading::IRS_KEY::FLOAT_FIRSTFIXING, std::numeric_limits<double>::quiet_NaN() );
        double floatLastFix	= swapLVB.getOptionalValueAsDouble( etrading::IRS_KEY::FLOAT_LASTFIXING, std::numeric_limits<double>::quiet_NaN() );
        bool useFirstFixing = etrading::useFloatFixing( floatFirstFix );
        bool useLastFixing = etrading::useFloatFixing( floatLastFix );

		const std::string fwdInter = swapLVB.getOptionalValueAsAQLString( etrading::IRS_KEY::IS_FWD_INTER, "").getCString();

        // Get the Swap PV using the full swap definition for both legs of the swap
        etrading::FwdInterInfo info = etrading::getfwdInterInfo( curveCollection, forecastCurveStaticDataTable, etrading::toBooleanEnum(fwdInter) );
        bool isFwdInter = info.isFwdInter;
        bool useFwdData = info.useFwdData;

        double notional                         = swapLVB.getOptionalValueAsDouble( etrading::IRS_KEY::NOTIONAL );
        double fixedRate                        = swapLVB.getOptionalValueAsDouble( etrading::IRS_KEY::FIXED_RATE );
        double floatSpread                      = swapLVB.getOptionalValueAsDouble( etrading::IRS_KEY::FLOAT_SPREAD );
        AQLString fixedDayCount                  = swapLVB.getCompulsoryValueAsAQLString( etrading::IRS_KEY::FIXED_DAYCOUNT );
        AQLString floatDayCount                  = swapLVB.getCompulsoryValueAsAQLString( etrading::IRS_KEY::FLOAT_DAYCOUNT );
		AQLString floatFrequency                 = swapLVB.getCompulsoryValueAsAQLString( etrading::IRS_KEY::FLOAT_FREQUENCY );
        
        AQLString floatCalendar                  = swapLVB.getOptionalValueAsAQLStringFromMultipleKeys(boost::assign::list_of (etrading::IRS_KEY::CALENDAR)
                                                                                                                            (etrading::IRS_KEY::ACCRUALCALENDAR)
                                                                                                                            (etrading::IRS_KEY::FLOAT_CALENDAR)
                                                                                                                            (etrading::IRS_KEY::FLOAT_ACCRUALCALENDAR) );

		AQLString floatBusinessDayAdjustment     = swapLVB.getOptionalValueAsAQLStringFromMultipleKeys( boost::assign::list_of(etrading::IRS_KEY::BUSINESSDAYADJUSTMENT)
                                                                                                                            (etrading::IRS_KEY::ACCRUALBUSINESSDAYADJUSTMENT)
                                                                                                                            (etrading::IRS_KEY::FLOAT_BUSINESSDAYADJUSTMENT)
                                                                                                                            (etrading::IRS_KEY::FLOAT_ACCRUALBUSINESSDAYADJUSTMENT) );

        double ret = CurveInstrumentPricing::getSwapPV( isFixedRatePayerSwap,
                                                   notional,
                                                   fixedAccrualDates,
                                                   fixedPaymentDates,
                                                   floatFixingDates,
                                                   floatAccrualDates,
                                                   floatPaymentDates,
                                                   etrading::getDataInstance(),
                                                   curveCollection,
                                                   fixedRate,
                                                   fixedDayCount,
                                                   floatSpread,
                                                   floatDayCount,
                                                   interpolation,
                                                   forecastCurveIndex,
                                                   discountCurveIndex,
                                                   isFwdInter,
                                                   useFirstFixing,
                                                   floatFirstFix,
                                                   useLastFixing,
                                                   floatLastFix,
                                                   useFwdData,
					                               false,	// isOIS
					                               "",	    // compoundingMethod
					                               floatCalendar, 
					                               floatBusinessDayAdjustment,
					                               "",		// sliding rule
					                               floatFrequency
					                               );

        AQ_RECORD_DECORATED_OUTPUTS( curveCollection, "", ret );

        return ret;

        VALID_EXCEPTION_END
    }


}








