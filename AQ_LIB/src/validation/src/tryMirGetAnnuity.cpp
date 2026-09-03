#include "tryMirGetAnnuity.h"
#include "LACurveForwardRateHelpers.h"
#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"

namespace validation
{

    using etrading::CreateDataFile;
    using etrading::decorateFilename;

    /* @brief			validation interface for the mirGetAnnuity1 method
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		fromDate	    From Date
    *  @param [in]		toDate			To Date
    *  @param [in]		curveId			ID of the curve set
    *  @param [in]		frequency		Frequency
    *  @param [in]		slidingRule		Sliding rule
    *  @param [in]		calendar		Calendar
    *  @param [in]		firstOddDate	End date of the front stub period
    *  @param [in]		lastOddDate		Start date of the end stub period
    *  @param [in]		paymentDay		PaymentDay
    *  @param [in]		dayCount		Day count convention
    *  @param [in]		interpolation	Interpolation method
    *  @param [in]		curveName		Name of the curve set
    *  @return			Annuity
    */
    double tryMirGetAnnuity1( LADataInstance* dataInstance,
                              const LADate& fromDate,
                              const LADate& toDate,
                              const LAString& curveId,
                              const LAString& frequency,
                              const LAString& slidingRule,
                              const LAString& calendar,
                              const LAString& firstOddDate,
                              const LAString& lastOddDate,
                              const LAString& paymentDay,
                              const LAString& dayCount,
                              const LAString& interpolation,
                              const LAString& curveName )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMirGetAnnuity1_inputs", curveId ) );
            file.write( "generatorFunction", "tryMirGetAnnuity1" );
            file.write( "fromDate", fromDate );
            file.write( "toDate", toDate );
            file.write( "curveId", curveId );
            file.write( "frequency", frequency );
            file.write( "slidingRule", slidingRule );
            file.write( "calendar", calendar );
            file.write( "firstOddDate", firstOddDate );
            file.write( "lastOddDate", lastOddDate );
            file.write( "paymentDay", paymentDay );
            file.write( "dayCount", dayCount );
            file.write( "interpolation", interpolation );
            file.write( "curveName", curveName );
        }

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

        LAString interp( etrading::getDefaultValueForEmptyString( interpolation, LAString( "SPLINE" ) ) );
        LAString dayC( etrading::getDefaultValueForEmptyString( dayCount, LAString( "ACT/365" ) ) );
        LAString cal( etrading::getDefaultValueForEmptyString( calendar, LAString( "TKB:LNB" ) ) );
        LAString sRule( etrading::getDefaultValueForEmptyString( slidingRule, LAString( "NO_CHANGE" ) ) );
        LAString freq( etrading::getDefaultValueForEmptyString( frequency, LAString( "SEMI-ANNUAL" ) ) );
        LAString curName( etrading::getDefaultValueForEmptyString( curveName, LAString( "STD" ) ) );

        double ret = etrading::LACurveForwardRateHelpers::getAnnuity( fromDate, toDate, dataInstance, curveId, freq, sRule, cal, firstOddDt, lastOddDt, pDay, dayC, interp, curName );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMirGetAnnuity1_outputs", curveId ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

    /* @brief			validation interface for the mirGetAnnuity2 method
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		curveId			ID of the curve set
    *  @param [in]		dates			A vector of the payment dates
    *  @param [in]		dayCount		Day count convention
    *  @param [in]		interpolation	Interpolation method
    *  @param [in]		curveName		Name of the curve set
    *  @return			Aannuity
    */
    double tryMirGetAnnuity2( LADataInstance* dataInstance,
                              const LAString& curveId,
                              const DateVector& dates,
                              const LAString& dayCount,
                              const LAString& interpolation,
                              const LAString& curveName )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMirGetAnnuity2_inputs", curveId ) );
            file.write( "generatorFunction", "tryMirGetAnnuity2" );
            file.write( "curveId", curveId );
            file.write( "dates", dates );
            file.write( "dayCount", dayCount );
            file.write( "interpolation", interpolation );
            file.write( "curveName", curveName );
        }

        if( dates.size() == 0 )
        {
            throw LACoreInvalidData( "#Error: The size of dates is zero.", __FILE__, __LINE__ );
        }

        LAString curName( etrading::getDefaultValueForEmptyString( curveName, LAString( "STD" ) ) );
        LAString dayC( etrading::getDefaultValueForEmptyString( dayCount, LAString( "ACT/365" ) ) );
        LAString interp( etrading::getDefaultValueForEmptyString( interpolation, LAString( "SPLINE" ) ) );

        double ret = etrading::LACurveForwardRateHelpers::getAnnuity( dates, dataInstance, curveId, dayC, interp, curName );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMirGetAnnuity2_outputs", curveId ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

}
