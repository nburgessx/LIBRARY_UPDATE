#include "tryMirSwapPV01.h"
#include "LACurveForwardRateHelpers.h"
#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"

using etrading::CreateDataFile;
using etrading::decorateFilename;

namespace validation_api
{

    /* @brief			validation interface for the tryMirSwapPV01 method
    *  @param [in]		dataInstance			                Pointer to the object pool
    *  @param [in]		notional		                Swap notional
    *  @param [in]		payRec			                Payer swap or Receiver swap
    *  @param [in]		effectDt	                    The effective start date of the swap, ie, base date + spot date
    *  @param [in]		maturity		                Swap maturity date or tenor
    *  @param [in]		curveId			                ID of the curve set
    *  @param [in]		fixedFrequency			        Fixed leg frequency
    *  @param [in]		fixedDaycount		            Fixed leg day count convention
    *  @param [in]		fixedBusinessDayAdjustment		Fixed leg roll convention
    *  @param [in]		fixedCalendar		            Fixed leg calendar
    *  @param [in]		fixedFirstStub		            End date of the front stub period on fixed leg
    *  @param [in]		fixedLastStub		            Start date of the end stub period on fixed leg
    *  @param [in]		fixedRollDay		            Fixed leg rolling date
    *  @param [in]		fixedPayLag			            Fixed leg payment date lag
    *  @param [in]		fixedStubType			        Fixed leg Stub type
    *  @param [in]		tSpread			                Floating Leg spread in basis points. This param is not used in the core functions.
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
    *  @param [in]		interpFwds		                Boolean that decides if direct interpolation on fwd rates is employed. This param is not used in the core functions.
    *  @param [in]		eomRoll			                Boolean that decides if EOM rolling is enforced (only when effective date is on EOM)
    *  @return			swap pv01
    */
    double tryMirSwapPV01( LADataInstance* dataInstance,
                           double notional,
                           const LAString& payRec,
                           const LAString& effectDt,
                           const LAString& maturity,
                           const LAString& curveId,
                           const LAString& fixedFrequency,
                           const LAString& fixedDaycount,
                           const LAString& fixedBusinessDayAdjustment,
                           const LAString& fixedCalendar,
                           const LAString& fixedFirstStub,
                           const LAString& fixedLastStub,
                           const LAString& fixedRollDay,
                           const LAString& fixedPayLag,
                           const LAString& fixedStubType,
                           double tSpread, // this param is not used
                           const LAString& floatFrequency,
                           const LAString& floatDayCount,
                           const LAString& floatBusinessDayAdjustment,
                           const LAString& floatCalendar,
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
                           bool interpFwds, // this param is not used
                           bool eomRoll )

    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMirSwapPV01_inputs", curveId ) );
            file.write( "generatorFunction", "tryMirSwapPV01" );
            file.write( "notional", notional );
            file.write( "payRec", payRec );
            file.write( "effectDt", effectDt );
            file.write( "maturity", maturity );
            file.write( "curveId", curveId );
            file.write( "xFreq", fixedFrequency );
            file.write( "xDayCount", fixedDaycount );
            file.write( "xRollCnv", fixedBusinessDayAdjustment );
            file.write( "xCalendar", fixedCalendar );
            file.write( "xFirstStub", fixedFirstStub );
            file.write( "xLastStub", fixedLastStub );
            file.write( "xRollDay", fixedRollDay );
            file.write( "xPayLag", fixedPayLag );
            file.write( "xStub", fixedStubType );
            file.write( "tSpread", tSpread );
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
        }

        // Generate Swap Schedule with validation
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

        bool isFixedRatePayerSwap = etrading::validateSwapPayRecFlag( payRec );

        etrading::validateStringEmptiness( forecastCurve, "#Error: The Swap 'forecast Curve' must be specified." );
        etrading::validateStringEmptiness( discountCurve, "#Error: The Swap 'discount Curve' must be specified." );

        LAString interp( etrading::getDefaultValueForEmptyString( interpolation, LAString( "SPLINE" ) ) );

        // Get the Swap PV01
        double ret = etrading::LACurveForwardRateHelpers::getSwapPV01( isFixedRatePayerSwap,
                                                                  notional,
                                                                  fixedAccrualDates,
                                                                  fixedPaymentDates,
                                                                  dataInstance,
                                                                  curveId,
                                                                  fixedDaycount,
                                                                  interp,
                                                                  forecastCurve,
                                                                  discountCurve );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMirSwapPV01_outputs", curveId ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }
}

