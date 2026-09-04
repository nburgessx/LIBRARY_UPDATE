#include "tryMirCompound.h"
#include "AQLCurveForwardRateHelpers.h"
#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "AQLPriceDataCalendar.h"
#include "StructuredExceptionHandler.h"


namespace validation
{

    using etrading::CreateDataFile;
    using etrading::decorateFilename;

    /* @brief			validation interface for the tryMirCompound3 method
    /* @brief			validation interface for the mirCompound3 method
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		startDates		A vector of start dates
    *  @param [in]		endDates		A vector of end dates
    *  @param [in]		curveId			ID of the curve set
    *  @param [in]		forecastCurve	Forecasting curve
    *  @param [in]		frequency		Frequency
    *  @param [in]		spread			Spread in basis points.
    *  @param [in]		isStartRoll		Roll dates: True if roll dates are calculated from start date, false from end date
    *  @param [in]		rollConvention	Roll conventions
    *  @param [in]		calendar		Calendar
    *  @param [in]		slidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		dayCount		Day count convention
    *  @param [in]		interpolation	Interpolation method
    *  @param [in]		compoundType	Compound Type: NORMAL, FLAT, SIMPLE, SIMPLE, AVERAGE
    *  @param [in]		firstOddDate	End date of the front stub period
    *  @param [in]		lastOddDate		Start date of the end stub period
    *  @return			interest rates calculated based on the startDates, endDates, and compound type
    */
    DoubleVector tryMirCompound3( AQLDataInstance* dataInstance,
                                  const DateVector& startDates,
                                  const DateVector& endDates,
                                  const AQLString& curveId,
                                  const AQLString& forecastCurve,
                                  const AQLString& frequency,
                                  double spread,
                                  bool isStartRoll,
                                  const AQLString& rollConvention,
                                  const AQLString& calendar,
                                  const AQLString& slidingRule,
                                  const AQLString& dayCount,
                                  const AQLString& interpolation,
                                  const AQLString& compoundType,
                                  const AQLString& firstOddDate,
                                  const AQLString& lastOddDate )

    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMirCompound3_inputs", curveId ) );
            file.write( "generatorFunction", "tryMirCompound3" );
            file.write( "startDates", startDates );
            file.write( "endDates", endDates );
            file.write( "curveId", curveId );
            file.write( "forecastCurve", forecastCurve );
            file.write( "frequency", frequency );
            file.write( "spread", spread );
            file.write( "isStartRoll", isStartRoll );
            file.write( "rollConvention", rollConvention );
            file.write( "calendar", calendar );
            file.write( "slidingRule", slidingRule );
            file.write( "dayCount", dayCount );
            file.write( "interpolation", interpolation );
            file.write( "compoundType", compoundType );
            file.write( "firstOddDate", firstOddDate );
            file.write( "lastOddDate", lastOddDate );
        }

        if( startDates.size() != endDates.size() )
        {
            throw AQLCoreInvalidData( "#Error: The input startDates and endDates must be of the same size.", __FILE__, __LINE__ );
        }

        AQLDate* firstOddDt  = NULL;
        if ( firstOddDate.size() != 0 )
        {
            firstOddDt = &etrading::stringToDate( firstOddDate, "#Error: Invalid 'firstOddDate'." );
        }
        AQLDate* lastOddDt  = NULL;
        if ( lastOddDate.size() != 0 )
        {
            lastOddDt = &etrading::stringToDate( lastOddDate, "#Error: Invalid 'lastOddDate'." );
        }

        AQLString cal( calendar );
        if( calendar == AQLString( "" ) )
        {
            cal = etrading::AQLCurveForwardRateHelpers::getYieldCurveForCurveID( dataInstance, curveId ).getCalendar().convertToString();
        }

        AQLString rollCon( etrading::getDefaultValueForEmptyString( rollConvention, AQLString( "ENDDATE" ) ) );
        AQLString sRule( etrading::getDefaultValueForEmptyString( slidingRule, AQLString( "MOD_FOLLOWING" ) ) );
        AQLString dayC( etrading::getDefaultValueForEmptyString( dayCount, AQLString( "ACT/360" ) ) );
        AQLString interp( etrading::getDefaultValueForEmptyString( interpolation, AQLString( "SPLINE" ) ) );
        AQLString cType( etrading::getDefaultValueForEmptyString( compoundType, AQLString( "FLAT" ) ) );

        DoubleVector ret;
        for( size_t i = 0; i < startDates.size(); i++ )
        {
            ret[i] = etrading::AQLCurveForwardRateHelpers::compound( dataInstance,
                                                               curveId,
                                                               forecastCurve,
                                                               startDates[i],
                                                               endDates[i],
                                                               spread,
                                                               frequency,
                                                               isStartRoll,
                                                               rollCon,
                                                               cal,
                                                               sRule,
                                                               dayC,
                                                               interp,
                                                               cType,
                                                               firstOddDt,
                                                               lastOddDt );
        }


        if ( CreateDataFile::recordEnabled() )
        {

            CreateDataFile file( decorateFilename( "tryMirCompound3_outputs", curveId ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation interface for the mirCompound3 method
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		startDate		Start date
    *  @param [in]		endDate			End date
    *  @param [in]		curveId			ID of the curve set
    *  @param [in]		forecastCurve	Forecasting curve
    *  @param [in]		frequency		Frequency
    *  @param [in]		spread			Spread in basis points.
    *  @param [in]		isStartRoll		Roll dates: True if roll dates are calculated from start date, false from end date
    *  @param [in]		rollConvention	Roll conventions
    *  @param [in]		calendar		Calendar
    *  @param [in]		slidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		dayCount		Day count convention
    *  @param [in]		interpolation	Interpolation method
    *  @param [in]		compoundType	Compound Type: NORMAL, FLAT, SIMPLE, SIMPLE, AVERAGE
    *  @param [in]		firstOddDate	End date of the front stub period
    *  @param [in]		lastOddDate		Start date of the end stub period
    *  @return			compounding rate calculated based on the startDate, endDate, and compound type
    */
    double tryMirCompound3( AQLDataInstance* dataInstance,
                            const AQLDate& startDate,
                            const AQLDate& endDate,
                            const AQLString& curveId,
                            const AQLString& forecastCurve,
                            const AQLString& frequency,
                            double spread,
                            bool isStartRoll,
                            const AQLString& rollConvention,
                            const AQLString& calendar,
                            const AQLString& slidingRule,
                            const AQLString& dayCount,
                            const AQLString& interpolation,
                            const AQLString& compoundType,
                            const AQLString& firstOddDate,
                            const AQLString& lastOddDate )
    {
        VALID_EXCEPTION_START

        DateVector startDateVector;
        startDateVector.push_back( startDate );
        DateVector endDateVector;
        endDateVector.push_back( endDate );

        DoubleVector ret = tryMirCompound3( dataInstance,
                                            startDateVector,
                                            endDateVector,
                                            curveId,
                                            forecastCurve,
                                            frequency,
                                            spread,
                                            isStartRoll,
                                            rollConvention,
                                            calendar,
                                            slidingRule,
                                            dayCount,
                                            interpolation,
                                            compoundType,
                                            firstOddDate,
                                            lastOddDate );

        return ret[0];

        VALID_EXCEPTION_END
    }


}


