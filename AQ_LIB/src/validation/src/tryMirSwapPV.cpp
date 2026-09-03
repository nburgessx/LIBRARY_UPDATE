#include "tryMirSwapPV.h"

#include "CurveInstrumentPricing.h"
#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"

using etrading::CreateDataFile;
using etrading::decorateFilename;

namespace validation
{

    /* @brief			validation interface for the mirSwapPV method
    *  @param [in]		dataInstance			                Pointer to the object pool
    *  @param [in]		notional		                Swap notional
    *  @param [in]		payRec			                Payer swap or Receiver swap
    *  @param [in]		effectDt		                The effective start date of the swap, ie, base date + spot date
    *  @param [in]		maturity		                Swap maturity date or tenor
    *  @param [in]		curveID			                ID of the curve set
    *  @param [in]		fixedRate				        Swap fixed leg rate
    *  @param [in]		fixedFrequency			        Fixed leg frequency
    *  @param [in]		fixedDaycount		            Fixed leg day count convention
    *  @param [in]		fixedBusinessDayAdjustment		Fixed leg roll convention
    *  @param [in]		fixedCalendar		            Fixed leg calendar
    *  @param [in]		fixedFirstStub		            End date of the front stub period on fixed leg
    *  @param [in]		fixedLastStub		            Start date of the end stub period on fixed leg
    *  @param [in]		fixedRollDay		            Fixed leg rolling date
    *  @param [in]		fixedPayLag			            Fixed leg payment date lag
    *  @param [in]		fixedStubType			        Fixed leg Stub type
    *  @param [in]		floatFrequency			        Floating leg frequency
    *  @param [in]		floatDayCount		            Floating leg day count convention
    *  @param [in]		floatBusinessDayAdjustment		Floating leg rolling convention
    *  @param [in]		floatCalendar		            Floating leg calendar
    *  @param [in]		tFistStub		                End date of the front stub period on floating leg
    *  @param [in]		floatLastStub		            Start date of the end stub period on floating leg
    *  @param [in]		floatRollDay		            Floating leg rolling day
    *  @param [in]		floatFixingLag			        Floating leg fixing day lag
    *  @param [in]		tFistFix		                Floating leg first fixing
    *  @param [in]		floatLastFixing		            Floating leg last fixing
    *  @param [in]		floatPayLag			            Floating leg payment date lag
    *  @param [in]		floatStubType			        Floating leg stub type
    *  @param [in]		interpolation	                Interpolation method
    *  @param [in]		forecastCurve	                Forecasting curve
    *  @param [in]		discountCurve	                Discounting curve
    *  @param [in]		interpFwds		                Boolean that decides if direct interpolation on fwd rates is employed
    *  @param [in]		eomRoll			                Boolean that decides if EOM rolling is enforced (only when effective date is on EOM)
    *  @param [in]		floatSpread			            Floating leg spread
    */
    double tryMirSwapPV( AQLDataInstance* dataInstance,
                         double notional,
                         const AQLString& payRec,
                         const AQLString& effectDt,
                         const AQLString& maturity,
                         const AQLString& curveID,
                         double fixedRate,
                         const AQLString& fixedFrequency,
                         const AQLString& fixedDaycount,
                         const AQLString& fixedBusinessDayAdjustment,
                         const AQLString& fixedCalendar,
                         const AQLString& fixedFirstStub,
                         const AQLString& fixedLastStub,
                         const AQLString& fixedRollDay,
                         const AQLString& fixedPayLag,
                         const AQLString& fixedStubType,
                         const AQLString& floatFrequency,
                         const AQLString& floatDayCount,
                         const AQLString& floatBusinessDayAdjustment,
                         const AQLString& floatCalendar,
                         const AQLString& floatFirstStub,
                         const AQLString& floatLastStub,
                         const AQLString& floatRollDay,
                         const AQLString& floatFixingLag,
                         double floatFirstFixing,
                         double floatLastFixing,
                         const AQLString& floatPayLag,
                         const AQLString& floatStubType,
                         const AQLString& interpolation,
                         const AQLString& forecastCurve,
                         const AQLString& discountCurve,
                         bool interpFwds,
                         bool eomRoll,
                         double floatSpread )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMirSwapPV_inputs", curveID ) );
            file.write( "generatorFunction", "tryMirSwapPV" );
            file.write( "notional", notional );
            file.write( "payRec", payRec );
            file.write( "effectDt", effectDt );
            file.write( "maturity", maturity );
            file.write( "curveID", curveID );
            file.write( "xRt", fixedRate );
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
        }

        //----------------------------------------------------------------------------------
        // Validate non-cash flow related parameters

        if( forecastCurve == AQLString( "" ) )
        {
            throw AQLCoreInvalidData( "#Error: The Swap 'forecast Curve' must be specified.", __FILE__, __LINE__ );
        }

        if( discountCurve == AQLString( "" ) )
        {
            throw AQLCoreInvalidData( "#Error: The Swap 'discount Curve' must be specified.", __FILE__, __LINE__ );
        }

        AQLString interp( interpolation );
        if( interpolation == AQLString( "" ) )
        {
            interp = AQLString( "SPLINE" );
        }


        //----------------------------------------------------------------------------------
        // Validate swap trade specific parameters

        // Payer or Receiver Swap i.e. Pay or Receive Fixed Coupons
        bool isFixedRatePayerSwap = etrading::validateSwapPayRecFlag( payRec );


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
                fixedBusinessDayAdjustment,             // fixedAccrualBusinessDayAdjustment
                fixedCalendar,                          // fixedAccrualCalendar
                fixedBusinessDayAdjustment,             // fixedPaymentBusinessDayAdjustment
                fixedCalendar,                          // fixedPaymentCalendar
                fixedFirstStub,
                fixedLastStub,
                fixedRollDay,                           // fixedRollDayString: An integer for the day of the month or a string: IMM, EOM, Start, End or NULL
                fixedPayLag,
                fixedStubType,
                floatFrequency,
                floatDayCount,
                floatBusinessDayAdjustment,             // floatAccrualBusinessDayAdjustment
                floatCalendar,                          // floatAccrualCalendar
                floatBusinessDayAdjustment,             // floatAccrualBusinessDayAdjustment
                floatCalendar,                          // floatAccrualCalendar
                floatBusinessDayAdjustment,             // floatAccrualBusinessDayAdjustment
                floatCalendar,                          // floatAccrualCalendar
                floatFirstStub,
                floatLastStub,
                floatRollDay,                           // floatRollDayString: An integer for the day of the month or a string: IMM, EOM, Start, End or NULL
                floatFixingLag,
                floatFirstFixing,
                floatLastFixing,
                floatPayLag,
                floatStubType );


        //----------------------------------------------------------------------------------
        // Get the Swap Par Rates using the full swap definition for both legs of the swap

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

        // Get the Swap Par Rates using the full swap definition for both legs of the swap
        double ret = CurveInstrumentPricing::getSwapPV( isFixedRatePayerSwap,
                     notional,
                     fixedAccrualDates,
                     fixedPaymentDates,
                     floatFixingDates,
                     floatAccrualDates,
                     floatPaymentDates,
                     dataInstance,
                     curveID,
                     fixedRate,
                     fixedDaycount,
                     floatSpread,
                     floatDayCount,
                     interpolation,
                     forecastCurve,
                     discountCurve,
                     interpFwds,	// isFwdInter. An obsolete parameter for OIS pricing
                     useFirstFixing,
                     floatFirstFixing,
                     useLastFixing,
                     floatLastFixing,
					 false,		// useFwdData
					 false,		// isOIS
					 "",		// compoundingMethod
					 floatCalendar, 
					 floatBusinessDayAdjustment,
					 "",		// sliding rule
					 floatFrequency
					 );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMirSwapPV_outputs", curveID ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


}








