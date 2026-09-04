#include "tryMirGetAnnuity.h"
#include "AQLCurveForwardRateHelpers.h"
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
    double tryMirGetAnnuity1( AQLDataInstance* dataInstance,
                              const AQLDate& fromDate,
                              const AQLDate& toDate,
                              const AQLString& curveId,
                              const AQLString& frequency,
                              const AQLString& slidingRule,
                              const AQLString& calendar,
                              const AQLString& firstOddDate,
                              const AQLString& lastOddDate,
                              const AQLString& paymentDay,
                              const AQLString& dayCount,
                              const AQLString& interpolation,
                              const AQLString& curveName )
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

        AQLDate* firstOddDt    = NULL;
        if ( firstOddDate.size() != 0 )
        {
            firstOddDt = &etrading::stringToDate( firstOddDate, "#Error: Invalid 'firstOddDate'." );
        }

        AQLDate* lastOddDt  = NULL;
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

        AQLString interp( etrading::getDefaultValueForEmptyString( interpolation, AQLString( "SPLINE" ) ) );
        AQLString dayC( etrading::getDefaultValueForEmptyString( dayCount, AQLString( "ACT/365" ) ) );
        AQLString cal( etrading::getDefaultValueForEmptyString( calendar, AQLString( "TKB:LNB" ) ) );
        AQLString sRule( etrading::getDefaultValueForEmptyString( slidingRule, AQLString( "NO_CHANGE" ) ) );
        AQLString freq( etrading::getDefaultValueForEmptyString( frequency, AQLString( "SEMI-ANNUAL" ) ) );
        AQLString curName( etrading::getDefaultValueForEmptyString( curveName, AQLString( "STD" ) ) );

        double ret = etrading::AQLCurveForwardRateHelpers::getAnnuity( fromDate, toDate, dataInstance, curveId, freq, sRule, cal, firstOddDt, lastOddDt, pDay, dayC, interp, curName );

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
    double tryMirGetAnnuity2( AQLDataInstance* dataInstance,
                              const AQLString& curveId,
                              const DateVector& dates,
                              const AQLString& dayCount,
                              const AQLString& interpolation,
                              const AQLString& curveName )
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
            throw AQLCoreInvalidData( "#Error: The size of dates is zero.", __FILE__, __LINE__ );
        }

        AQLString curName( etrading::getDefaultValueForEmptyString( curveName, AQLString( "STD" ) ) );
        AQLString dayC( etrading::getDefaultValueForEmptyString( dayCount, AQLString( "ACT/365" ) ) );
        AQLString interp( etrading::getDefaultValueForEmptyString( interpolation, AQLString( "SPLINE" ) ) );

        double ret = etrading::AQLCurveForwardRateHelpers::getAnnuity( dates, dataInstance, curveId, dayC, interp, curName );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMirGetAnnuity2_outputs", curveId ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

}
