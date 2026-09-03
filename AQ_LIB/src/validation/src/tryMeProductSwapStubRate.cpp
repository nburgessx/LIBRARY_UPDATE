#include "tryMeProductSwapStubRate.h"

#include "LACurveForwardRateHelpers.h"
#include "CurveInstrumentPricing.h"
#include "LADateScheduleHelpers.h"
#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "SwapUtilities.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"
#include "CommonConstants.h"
#include "CurveBuildDefaults.h"
#include "RecordMacros.h"

#include <boost/algorithm/string.hpp>

using etrading::CreateDataFile;
using etrading::decorateFilename;

namespace
{
	StubRateAndFixingDate getStubRateAndFixingDate( const LabelValueBlock& swapLVB,
													const LAStringVector& curveIndices,
													const LAStringVector& curveTenors,
													const DoubleVector& tenorCurveFixings,
													bool validateKeys )
	{
		etrading::validateKeysForLVB( validation_api::tryMeProductSwapStubRateLVBKeys(), swapLVB.getKeys(), validateKeys );
        
        const std::string inputLVB              = "SwapLVB";
        LAString curveCollection                = swapLVB.getCompulsoryValueAsLAString( etrading::MARKET_KEY::CURVE_COLLECTION, inputLVB );


        // Get and Validate Label Value Block Inputs
        // --------------------------------------------------------------------------------------------------------
        LADate effectiveDate	                = swapLVB.getCompulsoryValueAsDate( etrading::IRS_KEY::EFFECTIVE_DATE, inputLVB );
        LAString maturityDateString             = swapLVB.getCompulsoryValueAsLAString( etrading::IRS_KEY::MATURITY_DATE, inputLVB );

        LAString floatLegFreq			        = swapLVB.getCompulsoryValueAsLAStringFromKeys( etrading::IRS_KEY::FLOAT_FREQUENCY, etrading::IRS_KEY::FREQUENCY, inputLVB );
        LAString floatLegDayCount			    = swapLVB.getCompulsoryValueAsLAStringFromKeys( etrading::IRS_KEY::FLOAT_DAYCOUNT, etrading::IRS_KEY::DAYCOUNT, inputLVB );

        LAString floatCalendar                  = swapLVB.getOptionalValueAsLAStringFromMultipleKeys(boost::assign::list_of(etrading::IRS_KEY::CALENDAR)
                                                                                                                           (etrading::IRS_KEY::ACCRUALCALENDAR)
                                                                                                                           (etrading::IRS_KEY::FLOAT_CALENDAR)
                                                                                                                           (etrading::IRS_KEY::FLOAT_ACCRUALCALENDAR) );

        LAString floatBusinessDayAdjustment     = swapLVB.getOptionalValueAsLAStringFromMultipleKeys(boost::assign::list_of(etrading::IRS_KEY::BUSINESSDAYADJUSTMENT)
                                                                                                                           (etrading::IRS_KEY::ACCRUALBUSINESSDAYADJUSTMENT)
                                                                                                                           (etrading::IRS_KEY::FLOAT_BUSINESSDAYADJUSTMENT)
                                                                                                                           (etrading::IRS_KEY::FLOAT_ACCRUALBUSINESSDAYADJUSTMENT) );

        LAString floatLegFixLag			        = swapLVB.getOptionalValueAsLAStringFromKeys( etrading::IRS_KEY::FLOAT_FIXINGLAG, "0D" );
        LAString firstStubDate		            = swapLVB.getOptionalValueAsLAStringFromKeys( etrading::IRS_KEY::FLOAT_FIRSTSTUBDATE, etrading::IRS_KEY::FIRSTSTUBDATE );
        LAString lastStubDate		            = swapLVB.getOptionalValueAsLAStringFromKeys( etrading::IRS_KEY::FLOAT_LASTSTUBDATE, etrading::IRS_KEY::LASTSTUBDATE );
        LAString stubType			            = swapLVB.getOptionalValueAsLAStringFromKeys( etrading::IRS_KEY::FLOAT_STUBTYPE, etrading::IRS_KEY::STUBTYPE );
        LAString floatLegRollDayString		    = swapLVB.getOptionalValueAsLAStringFromKeys( etrading::IRS_KEY::FLOAT_ROLLDAY, etrading::IRS_KEY::ROLLDAY );

        LAString interpolation                  = swapLVB.getOptionalValueAsLAString( etrading::PRICING_PARAMS::INTERPOLATION, "SPLINE" );
        LAString useCurveIndex		            = swapLVB.getOptionalValueAsLAString( etrading::SWAP_STUB::USE_CURVE_INDEX );
        LAString toleranceTenor		            = swapLVB.getOptionalValueAsLAString( etrading::SWAP_STUB::STUB_TOLERANCE, "0D" );
		// --------------------------------------------------------------------------------------------------------

        LADate maturityDate	                    = etrading::validateMaturityDate( effectiveDate, maturityDateString);

        if ( maturityDate < effectiveDate )
        {
            throw LACoreInvalidData( "#Error: The swap maturity date cannot be before the swap start date", __FILE__, __LINE__ );
        }
        //----------------------------------------
        
        LAString stubTypeTmp( stubType );
        LAString* stubTypePtr = NULL;
        if( stubType.size() != 0  )
        {
            stubTypePtr = const_cast<LAString*>( &stubTypeTmp );
        }

        // Set-Up First and Last Stub Parameters
        LADate* firstOddDate    = NULL;
        LADate* lastOddDate     = NULL;
        LADate tempFirst;
        LADate tempLast;

        if( firstStubDate.size() != 0 && lastStubDate.size() != 0 )
        {
            throw LACoreInvalidData( "#Error: Don't support setting FirstStub and LastStub at the same time. Function won't know which stubType period to calculate stubType rate for.", __FILE__, __LINE__ );
        }

        if( firstStubDate.size() != 0 )
        {
            // Client should not specify both the stubType type and the first- and lastStubDates
            if( stubType.size() != 0 && ( LAString( stubType ).toUpper() ) != "NONE" )
            {
                throw LACoreInvalidData( "#Error: Float Leg cannot have both the StubType and First- or LastStubDate specified.", __FILE__, __LINE__ );
            }

            tempFirst       = etrading::stringToDate( firstStubDate, "#Error: Invalid 'FirstStubDate'." );
            firstOddDate    = & tempFirst;
            stubType		= "SS";		// Artificially set the value of stubType so that later on we know if we are dealing with front or end stubType
        }
        else if( lastStubDate.size() != 0 )
        {
            // Client should not specify both the stubType type and the first- and lastStubDates
            if( stubType.size() != 0 && ( LAString( stubType ).toUpper() ) != "NONE" )
            {
                throw LACoreInvalidData( "#Error: Float Leg cannot have both the StubType and First- or LastStubDate specified.", __FILE__, __LINE__ );
            }

            tempLast        = etrading::stringToDate( lastStubDate, "#Error: Invalid 'LastStubDate'." );
            lastOddDate     = & tempLast;
            stubType		= "SE";		// Artificially set the value of stubType so that later on we know if we are dealing with front or end stubType
        }
				
        //----------------------------------------
        
        bool useNearestCurve = false;
        if( ( boost::iequals( useCurveIndex.getCString(), "NATURAL" ) || useCurveIndex.getCString() == "" || useCurveIndex.size() == 0 ) )
        {
            if ( toleranceTenor.size() != 0 )
            {
                // Choose the nearest curve when 1. no explicit curve has been chosen to use and 2. a tolerance tenor is given
                useNearestCurve = true;
            }
        }

        int* floatLegRollDayPtr             = nullptr;
        LAString* floatLegRollConventionPtr = nullptr;
        int floatLegRollDay                 = 0;
        LAString floatLegRollConvention     = LAString( "" );
        bool floatLegIsEOMRoll              = false;
        bool floatLegIsStartRoll            = false;

        // Note: We check for RollDayString = LAString("0") for backwards compatibility
        if ( floatLegRollDayString != LAString( "0" ) && floatLegRollDayString.size() != 0 )
        {
            //// Generate Fixed Leg Coupon Roll Conventions if the rollDayString populated
            floatLegRollDayPtr              = & floatLegRollDay;
            floatLegRollConventionPtr       = & floatLegRollConvention;
            etrading::validateAndPopulateRollDayConventions( floatLegRollDayString, effectiveDate, maturityDate, &floatLegRollDayPtr, &floatLegRollConventionPtr, floatLegIsEOMRoll, floatLegIsStartRoll );
        }

        // Generate the fixing schedule
        DateVector fixingDates;
        etrading::LACurveForwardRateHelpers::generateFixingSchedule( fixingDates,
                                                                effectiveDate,
                                                                maturityDate,
                                                                floatLegFreq,
                                                                floatBusinessDayAdjustment,
                                                                floatCalendar,
                                                                LAString( "PRECEDING" ),      // fixingbusinessDayAdj
                                                                floatCalendar,
                                                                floatLegFixLag,
                                                                stubTypePtr,
                                                                firstOddDate,
                                                                lastOddDate,
                                                                floatLegRollDayPtr,
                                                                floatLegIsStartRoll,          // 'isStartRoll'
                                                                floatLegIsEOMRoll,
                                                                floatLegRollConventionPtr );	// This RollConv = Start, End, IMM, EOM or NULL. Defaults to NULL

        // Important Note: Curve Validation
        // --------------------------------
        // CurveIndices are not used if tenorCurveFixings are provided, which act as a lookup table. 
        //
        // Therefore we only check if the curve index exists in the case when the fixings are not provided.
        // This allows users to provide fixings for curves without the need to always build the curve index.
        if ( tenorCurveFixings.size() == 0 || tenorCurveFixings.empty() )
        {   
            //Throw exception if any curve has not been built.
            for ( size_t i = 0; i < curveIndices.size(); ++i )
            {
                etrading::getCurveStaticDataTableName( curveCollection, curveIndices[i] );
            }
        }

		//For stub rate, fwdInter always false
        etrading::FwdInterInfo info = etrading::getfwdInterInfo( "", "", etrading::FALSE_BOOL );
        bool isFwdInter = info.isFwdInter;
        bool useFwdData = info.useFwdData;
         
		// Determine if there is actually stub in the swap
		bool isStubDateSpecified = false;
		if (firstOddDate != nullptr || lastOddDate != nullptr)
		{
			isStubDateSpecified = true;
		}

		bool isRegularSwapSchedule = false;
		LAString rollConv("");
		if (isStubDateSpecified)
		{
			isRegularSwapSchedule = false;
		}
		else
		{
			bool isMaturityDateTenor = etrading::isMaturityDateTenor(maturityDateString);
			if (floatLegRollConventionPtr != nullptr)
			{
				rollConv = *floatLegRollConventionPtr;
			}

			int rollDay(0);
			if (floatLegRollDayPtr != NULL)
			{
				rollDay = * floatLegRollDayPtr;
			}

			isRegularSwapSchedule = etrading::isRegularSwapSchedule(effectiveDate, maturityDate, isMaturityDateTenor, floatLegFreq, floatBusinessDayAdjustment, floatCalendar, rollDay, rollConv);
		}

		// Calculate stub rate
        StubRateAndFixingDate stubRateAndFixingDate = CurveInstrumentPricing::getStubRate(fixingDates,
																						   curveIndices,
																						   curveTenors,
																						   tenorCurveFixings,
																						   curveCollection,
																						   stubType,
																						   interpolation,
																						   floatLegDayCount,
																						   floatCalendar,
																						   floatBusinessDayAdjustment,
																						   rollConv,
																						   useNearestCurve,
																						   isFwdInter,
																						   useFwdData,
																						   toleranceTenor,
																						   useCurveIndex,
																						   floatLegFreq,
																						   isRegularSwapSchedule);

		return stubRateAndFixingDate;
	}
}

namespace validation_api
{
    /* @brief			return a set of expected keys for swap stub rate label value block
    *  @return			expected keys
    */
    std::vector<std::string> tryMeProductSwapStubRateLVBKeys()
    {
        const std::string arr[] =
        {
            etrading::MARKET_KEY::CURVE_COLLECTION
            , etrading::SWAP_STUB::USE_CURVE_INDEX
            , etrading::PRICING_PARAMS::INTERPOLATION
            , etrading::SWAP_STUB::STUB_TOLERANCE
            , etrading::IRS_KEY::EFFECTIVE_DATE
            , etrading::IRS_KEY::MATURITY_DATE
            , etrading::IRS_KEY::FLOAT_FREQUENCY
            , etrading::IRS_KEY::FLOAT_DAYCOUNT
            , etrading::IRS_KEY::FLOAT_BUSINESSDAYADJUSTMENT
            , etrading::IRS_KEY::FLOAT_CALENDAR
            , etrading::IRS_KEY::FLOAT_FIRSTSTUBDATE
            , etrading::IRS_KEY::FLOAT_LASTSTUBDATE
            , etrading::IRS_KEY::FLOAT_ROLLDAY
            , etrading::IRS_KEY::FLOAT_FIXINGLAG
            , etrading::IRS_KEY::FLOAT_STUBTYPE
        };

        std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );

        return expectedKeys;
    }

    /* @brief			validation interface for the meProductSwapStubRate method
    *  @param [in]		swapLVB				A label value block defining the swap
	*  @param [in]		curveIndices		A list of curves to interpolate from
    *  @param [in]		curveTenors			A list of tenors corresponding to curve indexes
    *  @param [in]		tenorCurveFixings	A list of fixings corresponding to curve indexes
    *  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to True
    *  @return			Swap stub rate
    */
    double tryMeProductSwapStubRate( const LabelValueBlock& swapLVB,
									 const LAStringVector& curveIndices,
									 const LAStringVector& curveTenors,
									 const DoubleVector& tenorCurveFixings,
                                     bool validateKeys )
    {
        VALID_EXCEPTION_START
        
        RECORD_INPUTS( swapLVB, curveIndices, curveTenors, tenorCurveFixings, validateKeys );

        const double stubRate = getStubRateAndFixingDate( swapLVB, curveIndices, curveTenors, tenorCurveFixings, validateKeys ).stubRate_;

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( stubRate );

        VALID_EXCEPTION_END
    }

	/* @brief			validation interface for the meProductSwapStubFixingDate method
    *  @param [in]		swapLVB				A label value block defining the swap
	*  @param [in]		curveIndices		A list of curves to interpolate from
    *  @param [in]		curveTenors			A list of tenors corresponding to curve indexes
    *  @param [in]		tenorCurveFixings	A list of fixings corresponding to curve indexes
    *  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to True
    *  @return			Swap stub rate
    */
    LADate tryMeProductSwapStubFixingDate( const LabelValueBlock& swapLVB,
									       const LAStringVector& curveIndices,
									       const LAStringVector& curveTenors,
									       const DoubleVector& tenorCurveFixings,
                                           bool validateKeys )
    {
        VALID_EXCEPTION_START
        
        RECORD_INPUTS( swapLVB, validateKeys );

        const LADate stubFixingDate = getStubRateAndFixingDate( swapLVB, curveIndices, curveTenors, tenorCurveFixings, validateKeys ).fixingDate_;

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( stubFixingDate );

        VALID_EXCEPTION_END
    }
}


