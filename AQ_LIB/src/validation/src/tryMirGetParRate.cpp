/*
 * @brief			validation interface for mirGetParRate4
 * @Created:		17 March 2016
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "tryMirGetParRate.h"

#include "CurveInstrumentPricing.h"
#include "LACurveForwardRateHelpers.h"
#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"
#include "LADateScheduleHelpers.h"

namespace validation_api
{
    using etrading::CreateDataFile;
    using etrading::decorateFilename;

    /* @brief			validation interface for the tryMirGetParRate4 method
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		effectDt	The effective start date of the swap, ie, base date + spot date
    *  @param [in]		maturity		Swap maturity date or tenor
    *  @param [in]		curveID			ID of the curve set
    *  @param [in]		fixedFrequency			Fixed leg frequency
    *  @param [in]		fixedDaycount		Fixed leg day count convention
    *  @param [in]		fixedBusinessDayAdjustment		Fixed leg roll convention
    *  @param [in]		fixedCalendar		Fixed leg calendar
    *  @param [in]		fixedFirstStub		End date of the front stub period on fixed leg
    *  @param [in]		fixedLastStub		Start date of the end stub period on fixed leg
    *  @param [in]		fixedRollDay		Fixed leg rolling date
    *  @param [in]		fixedPayLag			Fixed leg payment date lag
    *  @param [in]		fixedStubType			Fixed leg Stub type
    *  @param [in]		floatFrequency			Floating leg frequency
    *  @param [in]		floatDayCount		Floating leg day count convention
    *  @param [in]		floatBusinessDayAdjustment		Floating leg rolling convention
    *  @param [in]		floatCalendar		Floating leg calendar ( Accrual Calendar )
    *  @param [in]		tFistStub		End date of the front stub period on floating leg
    *  @param [in]		floatLastStub		Start date of the end stub period on floating leg
    *  @param [in]		floatRollDay		Floating leg rolling day
    *  @param [in]		floatFixingLag			Floating leg fixing day lag
    *  @param [in]		tFistFix		Floating leg first fixing
    *  @param [in]		floatLastFixing		Floating leg last fixing
    *  @param [in]		floatPayLag			Floating leg payment date lag
    *  @param [in]		floatStubType			Floating leg stub type
    *  @param [in]		interpolation	Interpolation method
    *  @param [in]		forecastCurve	Forecasting curve
    *  @param [in]		discountCurve	Discounting curve
    *  @param [in]		interpFwds		Boolean that decides if direct interpolation on fwd rates is employed
    *  @param [in]		eomRoll			Boolean that decides if EOM rolling is enforced (only when effective date is on EOM)
    *  @param [in]		floatSpread			Floating leg spread
    *  @param [in]		useFwdData		Use fwd rates directly or derive them from spot rates
    *  @param [in]		floatFixingCalendar		Optional float fixing calendar - needed by the server team to price USD swaps
    */
    double tryMirGetParRate4( LADataInstance* dataInstance,
                              const LAString& effectDt,
                              const LAString& maturity,
                              const LAString& curveID,
                              const LAString& fixedFrequency,
                              const LAString& fixedDaycount,
                              const LAString& fixedBusinessDayAdjustment,
                              const LAString& fixedCalendar,
                              const LAString& fixedFirstStub,
                              const LAString& fixedLastStub,
                              const LAString& fixedRollDay,
                              const LAString& fixedPayLag,
                              const LAString& fixedStubType,
                              const LAString& floatFrequency,
                              const LAString& floatDayCount,
                              const LAString& floatBusinessDayAdjustment,
                              const LAString& floatCalendar,    // Float Accrual Calendar
                              const LAString& floatFirstStub,
                              const LAString& floatLastStub,
                              const LAString& floatRollDay,
                              const LAString& floatFixingLag,
                              double floatFirstFixing,
                              double floatLastFixing,
                              const LAString& floatPayLag,
                              const LAString& floatStubType,
                              const LAString& interpolation,
                              const LAString& forecastCurve,
                              const LAString& discountCurve,
                              bool interpFwds,
                              bool eomRoll,
                              double floatSpread,
                              bool useFwdData,
                              const LAString& floatFixingCalendar ) // Optional: Float Fixing Calendar needed by server team to price USD swaps
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMirGetParRate4_inputs", curveID ) );
            file.write( "generatorFunction", "tryMirGetParRate4" );
            file.write( "effectDt", effectDt );
            file.write( "maturity", maturity );
            file.write( "curveID", curveID );
            file.write( "xFreq", fixedFrequency );
            file.write( "xDayCount", fixedDaycount );
            file.write( "xRollCnv", fixedBusinessDayAdjustment );
            file.write( "xCalendar", fixedCalendar );
            file.write( "xFirstStub", fixedFirstStub );
            file.write( "xLastStub", fixedLastStub );
            file.write( "xRollDay", fixedRollDay );
            file.write( "xPayLag", fixedPayLag );
            file.write( "xStub", fixedStubType );
            file.write( "tFreq", floatFrequency );
            file.write( "tDayCount", floatDayCount );
            file.write( "tRollCnv", floatBusinessDayAdjustment );
            file.write( "tCalendar", floatCalendar );
            file.write( "tFirstStub", floatFirstStub );
            file.write( "tLastStub", floatLastStub );
            file.write( "tRollDay", floatRollDay );
            file.write( "tFixLag", floatFixingLag );
            file.write( "tFirstFix", floatFirstFixing );
            file.write( "tLastFix", floatLastFixing );
            file.write( "tPayLag", floatPayLag );
            file.write( "tStub", floatStubType );
            file.write( "interpolation", interpolation );
            file.write( "forecastCurve", forecastCurve );
            file.write( "discountCurve", discountCurve );
            file.write( "interpFwds", interpFwds );
            file.write( "eomRoll", eomRoll );
            file.write( "tSpd", floatSpread );
            file.write( "tCalF", floatFixingCalendar );
        }

        //----------------------------------------------------------------------------------
        // Validate non-cash flow related parameters

        if( forecastCurve == LAString( "" ) )
        {
            throw LACoreInvalidData( "#Error: The Swap 'forecast Curve' must be specified.", __FILE__, __LINE__ );
        }

        if( discountCurve == LAString( "" ) )
        {
            throw LACoreInvalidData( "#Error: The Swap 'discount Curve' must be specified.", __FILE__, __LINE__ );
        }

        LAString interp( interpolation );
        if( interpolation == LAString( "" ) )
        {
            interp = LAString( "SPLINE" );
        }


        //----------------------------------------------------------------------------------
        // Validate cash flow related parameters and generate cash flows

        DateVector fixedAccrualDates;
        DateVector fixedPaymentDates;
        DateVector floatAccrualDates;
        DateVector floatFixingDates;
        DateVector floatPaymentDates;

        //
        // Generate the Swap Cashflows
        //
        // Assumption:  We have assumed that the Fixing, Accrual and Payment BusinessDayAdjustments and Calendars are the all the same for each leg
        //              This is incorrect and this assumption needs to be relaxed.
        //
        etrading::validateAndGenerateSwapCashflows( fixedAccrualDates,
                                                    fixedPaymentDates,
                                                    floatFixingDates,
                                                    floatAccrualDates,
                                                    floatPaymentDates,
                                                    effectDt,
                                                    maturity,
                                                    fixedFrequency,
                                                    fixedDaycount,
                                                    fixedBusinessDayAdjustment,     // fixedAccrualBusinessDayAdjustment
                                                    fixedCalendar,                  // fixedAccrualCalendar
                                                    fixedBusinessDayAdjustment,     // fixedPaymentBusinessDayAdjustment
                                                    fixedCalendar,                  // fixedPaymentCalendar
                                                    fixedFirstStub,
                                                    fixedLastStub,
                                                    fixedRollDay,                   // fixedRollDayString: An integer for the day of the month or a string: IMM, EOM, Start, End or NULL
                                                    fixedPayLag,
                                                    fixedStubType,
                                                    floatFrequency,
                                                    floatDayCount,
                                                    floatBusinessDayAdjustment,     // floaFixingBusinessDayAdjustment
                                                    floatFixingCalendar,            // floaFixingCalendar
                                                    floatBusinessDayAdjustment,     // floatAccrualBusinessDayAdjustment
                                                    floatCalendar,                  // floatAccrualCalendar
                                                    floatBusinessDayAdjustment,     // floatPaymentBusinessDayAdjustment
                                                    floatCalendar,                  // floatPaymentCalendar
                                                    floatFirstStub,
                                                    floatLastStub,
                                                    floatRollDay,                   // floatRollDayString: An integer for the day of the month or a string: IMM, EOM, Start, End or NULL
                                                    floatFixingLag,
                                                    floatFirstFixing,
                                                    floatLastFixing,
                                                    floatPayLag,
                                                    floatStubType );

        // Float Leg Fixing Parameters
        bool useFirstFixing = false;
        if ( floatFirstFixing != 0.0 )
        {
            useFirstFixing = true;
        }

        bool useLastFixing = false;
        if ( floatLastFixing != 0.0 )
        {
            useLastFixing = true;
        }

        //----------------------------------------------------------------------------------
        // Get the Swap Par Rates using the full swap definition for both legs of the swap
		
		double ret = CurveInstrumentPricing::getParRate( fixedAccrualDates,
														   fixedPaymentDates,
														   floatFixingDates,
														   floatAccrualDates,
														   floatPaymentDates,
														   dataInstance,
														   curveID,
														   fixedDaycount,
														   floatDayCount,
														   interp,
														   forecastCurve,
														   discountCurve,
														   interpFwds,
														   useFirstFixing,
														   floatFirstFixing,
														   useLastFixing,
														   floatLastFixing,
														   floatSpread,
														   useFwdData ); // Optional: useFwdData, False (Default) = imply forwards from discount factors, True = use forward data directly
		
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMirGetParRate4_outputs", curveID ) );
            file.write( "output", ret );
        }

        return ret;


		
		VALID_EXCEPTION_END
    }


    /* @brief			validation interface for the tryMirGetParRate3 method
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		fromDate	    From Date
    *  @param [in]		toDate			To Date
    *  @param [in]		curveId			ID of the curve set
    *  @param [in]		frequency		Frequency
    *  @param [in]		dayCount		Day count convention
    *  @param [in]		slidingRule		Sliding rule
    *  @param [in]		calendar		Calendar
    *  @param [in]		interpolation	Interpolation method
    *  @param [in]		forecastCurve	Forecasting curve
    *  @param [in]		discountCurve	Discounting curve
    *  @param [in]		firstOddDate	End date of the front stub period
    *  @param [in]		lastOddDate		Start date of the end stub period
    *  @param [in]		paymentDay		PaymentDay
    *  @param [in]		forecastCurve	Forecasting curve
    *  @param [in]		discountCurve	Discounting curve
    *  @param [in]		interpFwds		Boolean that decides if direct interpolation on fwd rates is employed
    *  @param [in]		eomRoll			Boolean that decides if EOM rolling is enforced (only when effective date is on EOM)
    *  @return			par rate
    */
    double tryMirGetParRate3( LADataInstance* dataInstance,
                              const LAString& fromDate,
                              const LAString& toDate,
                              const LAString& curveId,
                              const LAString& frequency,
                              const LAString& dayCount,
                              const LAString& slidingRule,
                              const LAString& calendar,
                              const LAString& interpolation,
                              const LAString& firstOddDate,
                              const LAString& lastOddDate,
                              const LAString& paymentDay,
                              const LAString& foreCurveName,
                              const LAString& dfCurveName,
                              bool interpFwds,
                              bool eomRoll )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback

        if ( CreateDataFile::recordEnabled() )
        {

            CreateDataFile file( decorateFilename( "tryMirParRate3_inputs", curveId ) );
            file.write( "generatorFunction", "tryMirGetParRate3" );
            file.write( "fromDate", fromDate );
            file.write( "toDate", toDate );
            file.write( "curveId", curveId );
            file.write( "frequency", frequency );
            file.write( "dayCount", dayCount );
            file.write( "slidingRule", slidingRule );
            file.write( "calendar", calendar );
            file.write( "interpolation", interpolation );
            file.write( "firstOddDate", firstOddDate );
            file.write( "lastOddDate", lastOddDate );

            file.write( "paymentDay", paymentDay );
            file.write( "foreCurveName", foreCurveName );
            file.write( "dfCurveName", dfCurveName );
            file.write( "interpFwds", interpFwds );
            file.write( "eomRoll", eomRoll );
        }


        LADate fromDt =  etrading::stringToDate( fromDate, "#Error: Invalid 'fromDate'." );
        LADate toDt =  etrading::stringToDate( toDate, "#Error: Invalid 'toDate'." );

        LADate* firstOddDt    = NULL;
        if ( firstOddDate.size() != 0 )
        {

            firstOddDt = &etrading::stringToDate( firstOddDate, "#Error: Invalid 'firstOddDate'." );
        }

        LADate* lastOddDt  = NULL;
        if ( lastOddDate.size() != 0 )
        {
            lastOddDt = &etrading::stringToDate( lastOddDate, "#Error: Invalid 'lastOddDate'." );
        }

        int* pDay = NULL;
        int tempDay;
        if ( paymentDay.size() != 0 && paymentDay.getIntValue() != 0 )
        {
            tempDay =  paymentDay.getIntValue();
            pDay = &tempDay;
        }

        LAString freq( etrading::getDefaultValueForEmptyString( frequency, LAString( "SEMI-ANNUAL" ) ) );
        LAString slidingR( etrading::getDefaultValueForEmptyString( slidingRule, LAString( "NO_CHANGE" ) ) );
        LAString cal( etrading::getDefaultValueForEmptyString( calendar, LAString( "TKB:LNB" ) ) );
        LAString rollConvention( "EOM" );

        // get dates
        LAString* rollCon = NULL;
        if ( eomRoll && etrading::isLastDayOfMonth( fromDt, calendar ) )
        {
            rollCon = &rollConvention;
        }

        DateVector datevec = etrading::LADateScheduleHelpers::generateSchedule( fromDt,
                                                                       toDt,
                                                                       freq,
                                                                       slidingR,
                                                                       cal,
                                                                       firstOddDt,
                                                                       lastOddDt,
                                                                       pDay,
                                                                       true,
                                                                       rollCon );

        LAString dayC( etrading::getDefaultValueForEmptyString( dayCount, LAString( "ACT/365" ) ) );
        LAString interp( etrading::getDefaultValueForEmptyString( interpolation, LAString( "SPLINE" ) ) );
        LAString foreCurName( etrading::getDefaultValueForEmptyString( foreCurveName, LAString( "STD" ) ) );
        LAString dfCurName( etrading::getDefaultValueForEmptyString( dfCurveName, LAString( "STD" ) ) );

        // Get par rate based on the given dates
        // Note that the sliding rule from user input is not passed to the core function, in the core getParRate function the slidingRule is default as NO_CHANGE
        double ret = etrading::LACurveForwardRateHelpers::getParRate( datevec,
                                                                 dataInstance,
                                                                 curveId,
                                                                 dayC,
                                                                 interp,
                                                                 foreCurName,
                                                                 dfCurName,
                                                                 interpFwds );

        if ( CreateDataFile::recordEnabled() )
        {

            CreateDataFile file( decorateFilename( "tryMirParRate3_outputs", curveId ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

}

