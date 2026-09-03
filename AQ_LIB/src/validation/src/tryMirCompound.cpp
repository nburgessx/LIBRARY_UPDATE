/*
 * @brief			validation interface for mirCompound method(s)
 * @Created:		04 April 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */
#include "tryMirCompound.h"
#include "LACurveForwardRateHelpers.h"
#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "LAPriceDataCalendar.h"
#include "StructuredExceptionHandler.h"


namespace validation_api
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
    DoubleVector tryMirCompound3( LADataInstance* dataInstance,
                                  const DateVector& startDates,
                                  const DateVector& endDates,
                                  const LAString& curveId,
                                  const LAString& forecastCurve,
                                  const LAString& frequency,
                                  double spread,
                                  bool isStartRoll,
                                  const LAString& rollConvention,
                                  const LAString& calendar,
                                  const LAString& slidingRule,
                                  const LAString& dayCount,
                                  const LAString& interpolation,
                                  const LAString& compoundType,
                                  const LAString& firstOddDate,
                                  const LAString& lastOddDate )

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
            throw LACoreInvalidData( "#Error: The input startDates and endDates must be of the same size.", __FILE__, __LINE__ );
        }

        LADate* firstOddDt  = NULL;
        if ( firstOddDate.size() != 0 )
        {
            firstOddDt = &etrading::stringToDate( firstOddDate, "#Error: Invalid 'firstOddDate'." );
        }
        LADate* lastOddDt  = NULL;
        if ( lastOddDate.size() != 0 )
        {
            lastOddDt = &etrading::stringToDate( lastOddDate, "#Error: Invalid 'lastOddDate'." );
        }

        LAString cal( calendar );
        if( calendar == LAString( "" ) )
        {
            cal = etrading::LACurveForwardRateHelpers::getYieldCurveForCurveID( dataInstance, curveId ).getCalendar().convertToString();
        }

        LAString rollCon( etrading::getDefaultValueForEmptyString( rollConvention, LAString( "ENDDATE" ) ) );
        LAString sRule( etrading::getDefaultValueForEmptyString( slidingRule, LAString( "MOD_FOLLOWING" ) ) );
        LAString dayC( etrading::getDefaultValueForEmptyString( dayCount, LAString( "ACT/360" ) ) );
        LAString interp( etrading::getDefaultValueForEmptyString( interpolation, LAString( "SPLINE" ) ) );
        LAString cType( etrading::getDefaultValueForEmptyString( compoundType, LAString( "FLAT" ) ) );

        DoubleVector ret;
        for( size_t i = 0; i < startDates.size(); i++ )
        {
            ret[i] = etrading::LACurveForwardRateHelpers::compound( dataInstance,
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
    double tryMirCompound3( LADataInstance* dataInstance,
                            const LADate& startDate,
                            const LADate& endDate,
                            const LAString& curveId,
                            const LAString& forecastCurve,
                            const LAString& frequency,
                            double spread,
                            bool isStartRoll,
                            const LAString& rollConvention,
                            const LAString& calendar,
                            const LAString& slidingRule,
                            const LAString& dayCount,
                            const LAString& interpolation,
                            const LAString& compoundType,
                            const LAString& firstOddDate,
                            const LAString& lastOddDate )
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


