#include <string>
#include <limits>
#include <sstream>
#include <boost/date_time.hpp>

#include "Environment.h"
#include "StructuredExceptionHandler.h"
#include "CreateDataFile.h"
#include "RecordMacros.h"
#include "AQLDateSchedule.h"
#include "AQLCurveForwardRateHelpers.h"
#include "ParameterValidation.h"
#include "tryAqDate.h"
#include "tryAqCurveDiscountFactor.h"
#include "CurveValidation.h"
#include "CurveStreaming.h"
#include "CurveResultsContainer.h"
#include "AQObjUtilities.h"			// includes getCurveCollectionFromHandle()

using etrading::CreateDataFile;
using etrading::decorateCurvename;


namespace validation
{
    /* @brief			return the day count used in the core library
    */
    AQLString getDayCount()
    {
        return AQLString( "ACT/365" );
    }

    bool isBasisFlag()
    {
        return false;
    }

    /* @brief			populate and setting default values
    *  @param [in]		curveCollection	curve collection
    *  @param [inout]	curveIndex		curve index
    *  @param [inout]	interpolation	interpolation
    */
    void populateDiscountFactorConventions( const AQLString& curveCollection, AQLString& curveIndex, AQLString& interpolation )
    {
        curveIndex = etrading::getDefaultValueForEmptyString( curveIndex, "OIS" );

        //Throw exception if the curve has not been built.
        AQLString staticDataTable = etrading::getCurveStaticDataTableName( curveCollection, curveIndex );
        interpolation = etrading::getCurveInterpolation( curveCollection, staticDataTable );
    }

    /* @brief			populate and setting default values
    *  @param [in]		curveCollection	curve collection
    *  @param [inout]	curveIndex		curve index
    *  @param [inout]	interpolation	interpolation
    *  @param [inout]	businessDayAdj	business day adjustment
    *  @param [inout]	calendar		calendar
    */
    void populateDiscountFactorConventions( const AQLString& curveCollection, AQLString& curveIndex, AQLString& interpolation, AQLString& businessDayAdj, AQLString& calendar )
    {
        populateDiscountFactorConventions( curveCollection, curveIndex, interpolation );
        businessDayAdj = etrading::getDefaultValueForEmptyString( businessDayAdj, "NO_CHANGE" );
        calendar = etrading::getDefaultCalendarForEmptyString( calendar, curveCollection );
    }

    /* @brief			validation method for aqCurveDiscountFactorsFromYearFractions
    *  @param [in]		yearFractions	An array of yearFractions by year count
    *  @param [in]		dayCount		Day count
    *  @param [in]		curveCollection	Curve set ID
    *  @param [in]		curveIndex		Index of the curve set. Default to OIS
    *  @return			An array of discount factor
    */
    DoubleVector tryAqCurveDiscountFactorsFromYearFractions( const DoubleVector& yearFractions,
                                                             const AQLString& dayCount,
                                                             const AQLString& curveCollection,
                                                             const AQLString& curveIndex )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurveDiscountFactorsFromYearFractions_inputs", curveCollection, curveIndex ) );
            file.write( "generatorFunction", "tryAqCurveDiscountFactorsFromYearFractions" );
            file.write( "yearFractions",		yearFractions );
            file.write( "dayCount",          dayCount );
            file.write( "curveCollection",	curveCollection );
            file.write( "curveIndex",		curveIndex );
        }

        // Get DF
        DoubleVector ret = etrading::getCurveDiscountFactorsFromYearFractions(yearFractions, dayCount, curveCollection, curveIndex);

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurveDiscountFactorsFromYearFractions_outputs", curveCollection, curveIndex ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

    DoubleVector tryAqCurveObjectDiscountFactorsFromYearFractions( const DoubleVector& yearFractions,
                                                                const AQLString& dayCount,
                                                                const std::string& aqObjCurveName )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( std::string( std::string( "tryAqCurveObjectDiscountFactorsFromYearFractions_inputs_" ) + aqObjCurveName ).c_str()  );
            file.write( "generatorFunction", "tryAqCurveObjectDiscountFactorsFromYearFractions" );
            file.write( "yearFractions",		yearFractions );
            file.write( "dayCount",          dayCount );
            file.write( "aqObjCurveName",	    aqObjCurveName );
        }

        // Input validations
        AQ_THROW_IF( yearFractions.size() == 0, "Size of input 'yearFractions' is zero." );

        DoubleVector ret( 0, std::numeric_limits<double>::quiet_NaN() );

        auto& env = etrading::Environment::defaultEnv();
        auto aqObjCurve = env.accessObject<etrading::AQObjCurve>( aqObjCurveName ); // 			etrading::getAQObjCurve(aqObjCurveName);

        // Convert Year Fractions to ACT/365 ones
        AQLDate curveAsOfDate = etrading::toAQLDateFromGregorianDate( aqObjCurve->getCurveBuildStaticDataObject()->asOfDate_ );
        etrading::convertToYearFractionsACT365( curveAsOfDate, const_cast< DoubleVector& >( yearFractions ), dayCount );

        if( aqObjCurve )
        {
            ret = aqObjCurve->calculateDiscountFactor( yearFractions );
        }
        else
        {
            std::ostringstream curveMsg;
            curveMsg << "Curve " << aqObjCurveName << " does not exist.";
            AQ_THROW( curveMsg.str() );
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( std::string( std::string( "tryAqCurveObjectDiscountFactorsFromYearFractions_outputs_" ) + aqObjCurveName ).c_str() );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }



    /* @brief			validation method for aqCurveDiscountFactorsFromTenors
    *  @param [in]		tenors			An array of tenor strings
    *  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
    *  @param [in]		calendar		Calendar
    *  @param [in]		curveCollection	Curve set ID
    *  @param [in]		curveIndex		Index of the curve set. Default to OIS
    *  @return			An array of discount factor
    */
    DoubleVector tryAqCurveDiscountFactorsFromTenors( const AQLStringVector& tenors,
            const AQLString& businessDayAdj,
            const AQLString& calendar,
            const AQLString& curveCollection,
            const AQLString& curveIndex )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurveDiscountFactorsFromTenors_inputs", curveCollection, curveIndex ) );
            file.write( "generatorFunction", "tryAqCurveDiscountFactorsFromTenors" );
            file.write( "tenors",			tenors );
            file.write( "businessDayAdj",	businessDayAdj );
            file.write( "calendar",			calendar );
            file.write( "curveCollection",	curveCollection );
            file.write( "curveIndex",		curveIndex );
        }

        // Input validations
        AQ_THROW_IF( tenors.size() == 0, "Size of input 'tenors' is zero." );

        AQLString curIndex( curveIndex );
        AQLString interp;
        AQLString bdAdj( businessDayAdj );
        AQLString cal( calendar );
        populateDiscountFactorConventions( curveCollection, curIndex, interp, bdAdj, cal );

        DoubleVector ret = etrading::AQLCurveForwardRateHelpers::getMultiDF( tenors, etrading::getDataInstance(), curveCollection, getDayCount(), bdAdj, cal, interp, isBasisFlag(), curIndex );
        AQ_THROW_IF( ret.size() == 0, "No DFs have been returned." );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurveDiscountFactorsFromTenors_outputs", curveCollection, curveIndex ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

    /* @brief			validation method for aqCurveDiscountFactorsFromTenors
    *  @param [in]		tenors			An array of tenor strings
    *  @param [in]		curveCollection	Curve set ID
    *  @param [in]		curveIndex		Index of the curve set. Default to OIS
    *  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
    *  @param [in]		calendar		Calendar
    *  @return			An array of discount factor
    */
    DoubleVector tryAqCurveObjectDiscountFactorsFromTenors( const std::vector<std::string>& tenors,
            const std::string& businessDayAdj,
            const std::string& calendar,
            const std::string& aqObjCurveName )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( std::string( std::string( "tryAqCurveObjectDiscountFactorsFromTenors_inputs_" ) + aqObjCurveName ).c_str()  );
            file.write( "generatorFunction", "tryAqCurveObjectDiscountFactorsFromTenors" );
            file.write( "tenors",			tenors );
            file.write( "businessDayAdj",	businessDayAdj );
            file.write( "calendar",			calendar );
            file.write( "aqObjCurveName",	    aqObjCurveName );
        }

        // Input validations
        AQ_THROW_IF( tenors.size() == 0, "Size of input 'tenors' is zero." );

        etrading::BusinessDayAdjustmentEnum busDayAdjust = etrading::toBusinessDayAdjustmentEnum( businessDayAdj );

        DoubleVector ret( 0, std::numeric_limits<double>::quiet_NaN() );
        auto& env = etrading::Environment::defaultEnv();
        auto aqObjCurve = env.accessObject<etrading::AQObjCurve>( aqObjCurveName ); // 			etrading::getAQObjCurve(aqObjCurveName);
        if( aqObjCurve )
        {
            ret = aqObjCurve->calculateDiscountFactor( tenors, busDayAdjust, calendar );
        }
        else
        {
            std::ostringstream curveMsg;
            curveMsg << "Curve " << aqObjCurveName << " does not exist.";
            AQ_THROW( curveMsg.str() );
        }

        AQ_THROW_IF( ret.size() == 0, "No DFs have been returned." );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( std::string( std::string( "tryAqCurveObjectDiscountFactorsFromTenors_outputs_" ) + aqObjCurveName ).c_str()  );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation method for aqCurveDiscountFactorsForwardStarting
    *  @param [in]		fromDates		A single from-date in YYYYMMDD formate
    *  @param [in]		toDates			An array of single to-date in YYYYMMDD formate
    *  @param [in]		curveCollection	Curve set ID
    *  @param [in]		curveIndex		Index of the curve set. Default to OIS
    *  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
    *  @param [in]		calendar		Calendar
    *  @return			A array of discount factors
    */
    DoubleVector tryAqCurveDiscountFactorsForwardStarting( const DateVector& fromDates,
            const DateVector& toDates,
            const AQLString& curveCollection,
            const AQLString& curveIndex )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurveDiscountFactorsForwardStarting_inputs", curveCollection, curveIndex ) );
            file.write( "generatorFunction", "tryAqCurveDiscountFactorsForwardStarting" );
            file.write( "fromDates",			fromDates );
            file.write( "toDates",			toDates );
            file.write( "curveCollection",	curveCollection );
            file.write( "curveIndex",		curveIndex );
        }

        size_t M = fromDates.size();
        size_t N = toDates.size();
        AQ_THROW_IF( M != N && M != 1, "'fromDates' either takes 1 date, or an array of dates in equal size of 'toDates'" );

        AQ_THROW_IF( N == 0 || M == 0, "'fromDates' and 'toDates' must not be empty" );

        // Create pairs of fromDate and toDate even when there is only one fromDate
        DateVector fromDateVec;
        fromDateVec.push_back( fromDates[0] );
        if( M == 1 )
        {
            for( size_t i = 1; i < N; ++i )
            {
                fromDateVec.push_back( fromDates[0] );
            }
        }
        else
        {
            for( size_t i = 1; i < M; ++i )
            {
                fromDateVec.push_back( fromDates[i] );
            }
        }

        AQLString curIndex( curveIndex );
        AQLString interp;
        AQLString bdAdj( "NO_CHANGE" );
        AQLString cal;
        populateDiscountFactorConventions( curveCollection, curIndex, interp, bdAdj, cal );

        DoubleVector ret = etrading::AQLCurveForwardRateHelpers::getMultiDF( fromDateVec, toDates, etrading::getDataInstance(), curveCollection, getDayCount(), bdAdj, cal, interp, isBasisFlag(), curIndex );

        AQ_THROW_IF( ret.size() == 0, "No DFs have been returned." );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurveDiscountFactorsForwardStarting_outputs", curveCollection, curveIndex ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

    DoubleVector tryAqCurveObjectDiscountFactorsForwardStarting( const std::vector<boost::gregorian::date>& fromDates,
            const std::vector<boost::gregorian::date>& toDates,
            const std::string& businessDayAdj,
            const std::string& calendar,
            const std::string& aqObjCurveName )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryAqCurveObjectDiscountFactorsForwardStarting_inputs_" ) + aqObjCurveName ).c_str() );
            file.write( "generatorFunction", "tryAqCurveObjectDiscountFactorsForwardStarting" );
            file.write( "fromDates",			fromDates );
            file.write( "toDates",			toDates );
            file.write( "businessDayAdj",	businessDayAdj );
            file.write( "calendar",			calendar );
            file.write( "aqObjCurveName",	    aqObjCurveName );
        }

        size_t M = fromDates.size();
        size_t N = toDates.size();
        AQ_THROW_IF( M != N && M != 1, "'fromDates' either takes 1 date, or an array of dates in equal size of 'toDates'" );
        AQ_THROW_IF( N == 0 || M == 0, "'fromDates' and 'toDates' must not be empty" );

        // Create pairs of fromDate and toDate even when there is only one fromDate
        std::vector<boost::gregorian::date> fromDateVec;
        fromDateVec.push_back( fromDates[0] );
        if( M == 1 )
        {
            for( size_t i = 1; i < N; ++i )
            {
                fromDateVec.push_back( fromDates[0] );
            }
        }
        else
        {
            for( size_t i = 1; i < M; ++i )
            {
                fromDateVec.push_back( fromDates[i] );
            }
        }

        etrading::BusinessDayAdjustmentEnum busDayAdjust = etrading::toBusinessDayAdjustmentEnum( businessDayAdj );
        DoubleVector ret( 0, std::numeric_limits<double>::quiet_NaN() );

        auto& env = etrading::Environment::defaultEnv();
        auto aqObjCurve = env.accessObject<etrading::AQObjCurve>( aqObjCurveName ); // 			etrading::getAQObjCurve(aqObjCurveName);

        if( aqObjCurve )
        {
            ret = aqObjCurve->calculateDiscountFactor( fromDateVec, toDates, busDayAdjust, calendar );
        }
        else
        {
            std::ostringstream curveMsg;
            curveMsg << "Curve " << aqObjCurveName << " does not exist.";
            AQ_THROW( curveMsg.str() );
        }

        AQ_THROW_IF( ret.size() == 0, "No DFs have been returned." );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryAqCurveObjectDiscountFactorsForwardStarting_outputs_" ) + aqObjCurveName ).c_str() );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END

    };


    /* @brief			validation method for aqCurveDiscountFactorsForwardStartingFromYearFractions
    *  @param [in]		fromDates			An array of from-dates in YYYYMMDD formate
    *  @param [in]		yearFractions		An array of year fraction yearFractions
    *  @param [in]		dayCount	        The daycount fraction used to generate the yearFraction
    *  @param [in]		curveCollection		Curve set ID
    *  @param [in]		curveIndex			Index of the curve set. Default to OIS
    *  @return			A array of discount factors
    */
    DoubleVector tryAqCurveDiscountFactorsForwardStartingFromYearFractions( const DateVector& fromDates,
                                                                            const DoubleVector& yearFractions,
                                                                            const AQLString& dayCount,
                                                                            const AQLString& curveCollection,
                                                                            const AQLString& curveIndex )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurveDiscountFactorsForwardStartingFromYearFractions_inputs", curveCollection, curveIndex ) );
            file.write( "generatorFunction", "tryAqCurveDiscountFactorsForwardStartingFromYearFractions" );
            file.write( "fromDates",			fromDates );
            file.write( "yearFractions",		yearFractions );
            file.write( "dayCount",          dayCount );
            file.write( "curveCollection",	curveCollection );
            file.write( "curveIndex",		curveIndex );
        }

        DoubleVector ret = etrading::getCurveDiscountFactorsForwardStartingFromYearFractions( fromDates, yearFractions, dayCount, curveCollection,  curveIndex );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurveDiscountFactorsForwardStartingFromYearFractions_outputs", curveCollection, curveIndex ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

    /* @brief			validation method for aqCurveDiscountFactorsForwardStartingFromYearFractions
    *  @param [in]		fromDates			An array of from-dates in YYYYMMDD formate
    *  @param [in]		yearFractions		An array of year fraction yearFractions
    *  @param [in]		dayCount	        The daycount fraction used to generate the yearFraction
    *  @param [in]		aqObjCurveName		Curve Name
    *  @return			A array of discount factors
    */
    DoubleVector tryAqCurveObjectDiscountFactorsForwardStartingFromYearFractions( const std::vector<boost::gregorian::date>& fromDates,
            const DoubleVector& yearFractions,
            const AQLString& dayCount,
            const std::string& aqObjCurveName )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryAqCurveObjectDiscountFactorsForwardStartingFromYearFractions_inputs_" ) + aqObjCurveName ).c_str() );
            file.write( "generatorFunction", "tryAqCurveObjectDiscountFactorsForwardStartingFromYearFractions" );
            file.write( "fromDates",			fromDates );
            file.write( "yearFractions",		yearFractions );
            file.write( "dayCount",          dayCount );
            file.write( "aqObjCurveName",		aqObjCurveName );
        }

        size_t M = fromDates.size();
        size_t N = yearFractions.size();
        AQ_THROW_IF( M != N, "'fromDates' and 'yearFractions' must be of equal size'" );

        AQ_THROW_IF( N == 0 || M == 0, "'fromDates' and 'yearFractions' must not be empty" );

        etrading::BusinessDayAdjustmentEnum busDayAdjust = etrading::toBusinessDayAdjustmentEnum( "NO_CHANGE" );
        DoubleVector ret( 0, std::numeric_limits<double>::quiet_NaN() );

        auto& env = etrading::Environment::defaultEnv();
        auto aqObjCurve = env.accessObject<etrading::AQObjCurve>( aqObjCurveName ); // 			etrading::getAQObjCurve(aqObjCurveName);


        // Convert Year Fractions to ACT/365 ones
        AQLDate curveAsOfDate = etrading::toAQLDateFromGregorianDate( aqObjCurve->getCurveBuildStaticDataObject()->asOfDate_ );
        etrading::convertToYearFractionsACT365( curveAsOfDate, const_cast< DoubleVector& >( yearFractions ), dayCount );

        if( aqObjCurve )
        {
            std::string calendar( "" );
            ret = aqObjCurve->calculateDiscountFactor( fromDates, yearFractions, busDayAdjust, calendar );
        }
        else
        {
            std::ostringstream curveMsg;
            curveMsg << "Curve " << aqObjCurveName << " does not exist.";
            AQ_THROW( curveMsg.str() );
        }

        AQ_THROW_IF( ret.size() == 0, "No DFs have been returned." );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryAqCurveDiscountFactorsForwardStartingFromYearFractions_outputs_" ) + aqObjCurveName ).c_str() );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation method for aqCurveDiscountFactorsForwardStartingFromTenor
    *  @param [in]		fromDates		An array of from-dates in YYYYMMDD formate
    *  @param [in]		tenor			A tenor string
    *  @param [in]		curveCollection	Curve set ID
    *  @param [in]		curveIndex		Index of the curve set. Default to OIS
    *  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
    *  @param [in]		calendar		Calendar
    *  @return			A array of discount factors
    */
    DoubleVector tryAqCurveDiscountFactorsForwardStartingFromTenor( const DateVector& fromDates,
            const AQLString& tenor,
            const AQLString& curveCollection,
            const AQLString& curveIndex,
            const AQLString& businessDayAdj,
            const AQLString& calendar )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurveDiscountFactorsForwardStartingFromTenor_inputs", curveCollection, curveIndex ) );
            file.write( "generatorFunction", "tryAqCurveDiscountFactorsForwardStartingFromTenor" );
            file.write( "fromDates",			fromDates );
            file.write( "tenor",				tenor );
            file.write( "curveCollection",	curveCollection );
            file.write( "curveIndex",		curveIndex );
            file.write( "businessDayAdj",	businessDayAdj );
            file.write( "calendar",			calendar );
        }

        AQ_THROW_IF( fromDates.size() == 0, "'fromDates' must not be empty" );

        AQLString curIndex( curveIndex );
        AQLString interp;
        AQLString bdAdj( businessDayAdj );
        AQLString cal( calendar );
        populateDiscountFactorConventions( curveCollection, curIndex, interp, bdAdj, cal );

        DoubleVector ret = etrading::AQLCurveForwardRateHelpers::getMultiDF( fromDates, tenor, etrading::getDataInstance(), curveCollection, getDayCount(), bdAdj, cal, interp, isBasisFlag(), curIndex );
        AQ_THROW_IF( ret.size() == 0, "No DFs have been returned." );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurveDiscountFactorsForwardStartingFromTenor_outputs", curveCollection, curveIndex ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

    /* @brief			validation method for aqCurveDiscountFactorsForwardStartingFromTenor
    *  @param [in]		fromDates		An array of from-dates in YYYYMMDD formate
    *  @param [in]		tenor			A tenor string
    *  @param [in]		curveCollection	Curve set ID
    *  @param [in]		curveIndex		Index of the curve set. Default to OIS
    *  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
    *  @param [in]		calendar		Calendar
    *  @return			A array of discount factors
    */
    DoubleVector tryAqCurveObjectDiscountFactorsForwardStartingFromTenor( const std::vector<boost::gregorian::date>& fromDates,
            const std::vector<std::string>& tenors,
            const std::string& businessDayAdj,
            const std::string& calendar,
            const std::string& aqObjCurveName )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryAqCurveObjectDiscountFactorsForwardStartingFromTenor_inputs_" ) + aqObjCurveName ).c_str() );
            file.write( "generatorFunction", "tryAqCurveObjectDiscountFactorsForwardStartingFromTenor" );
            file.write( "fromDates",			fromDates );
            file.write( "tenor",				tenors );
            file.write( "businessDayAdj",	businessDayAdj );
            file.write( "calendar",			calendar );
            file.write( "curveIndex",		aqObjCurveName );
        }

        AQ_THROW_IF( fromDates.size() == 0, "'fromDates' must not be empty" );

        if( fromDates.size() != tenors.size() )
        {
            std::ostringstream errStream;
            errStream << "Number of fromDates (" << fromDates.size() << ") is not equal to the number of tenors (" << tenors.size() << ")";
            AQ_THROW( errStream.str() );
        }

        etrading::BusinessDayAdjustmentEnum busDayAdjust = etrading::toBusinessDayAdjustmentEnum( businessDayAdj );
        DoubleVector ret( 0, std::numeric_limits<double>::quiet_NaN() );

        auto& env = etrading::Environment::defaultEnv();
        auto aqObjCurve = env.accessObject<etrading::AQObjCurve>( aqObjCurveName ); // 			etrading::getAQObjCurve(aqObjCurveName);
        if( aqObjCurve )
        {
            ret = aqObjCurve->calculateDiscountFactor( fromDates, tenors, busDayAdjust, calendar );
        }
        else
        {
            std::ostringstream curveMsg;
            curveMsg << "Curve " << aqObjCurveName << " does not exist.";
            AQ_THROW( curveMsg.str() );
        }

        AQ_THROW_IF( ret.size() == 0, "No DFs have been returned." );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryAqCurveObjectDiscountFactorsForwardStartingFromTenor_outputs_" ) + aqObjCurveName ).c_str() );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation method for aqCurveDiscountFactors
    *  @param [in]		toDates			A single or an array of to-date in YYYYMMDD formate
    *  @param [in]		curveCollection	Curve set ID
    *  @param [in]		curveIndex		Index of the curve set. Default to OIS
    *  @return			A array of discount factors
    */
    DoubleVector tryAqCurveDiscountFactors( const DateVector& toDates,
                                            const AQLString& curveCollectionOrHandle,
                                            const AQLString& curveIndex )
    {
        VALID_EXCEPTION_START

		// To Allow Support for Curve Objects or Curve Collections
		AQLString curveCollectionFromHandle = etrading::getCurveCollectionFromHandle( curveCollectionOrHandle.c_str() );

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurveDiscountFactors_inputs", curveCollectionFromHandle, curveIndex ) );
            file.write( "generatorFunction", "tryAqCurveDiscountFactors" );
            file.write( "toDates",			toDates );
            file.write( "curveCollection",	curveCollectionFromHandle );
            file.write( "curveIndex",		curveIndex );
        }

        size_t N = toDates.size();
        AQ_THROW_IF( N == 0, "'toDates' must not be empty" );

        AQLString curIndex( curveIndex );
        AQLString interp;
        AQLString bdAdj( "NO_CHANGE" );
        AQLString cal;
        populateDiscountFactorConventions( curveCollectionFromHandle, curIndex, interp, bdAdj, cal );

        DoubleVector ret = etrading::AQLCurveForwardRateHelpers::getMultiSpotDiscountFactors( toDates, etrading::getDataInstance(), curveCollectionFromHandle, getDayCount(), bdAdj, cal, interp, isBasisFlag(), curIndex );

        AQ_THROW_IF( ret.size() == 0, "No DFs have been returned." );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurveDiscountFactors_outputs", curveCollectionFromHandle, curveIndex ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }
	
    /* @brief			Validation interface for tryAqCurveForwardRatesOverride function, which overrides curve forward rates by setting equivalent discount factors
	*  @param [in]		curveCollection		        The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndex			        The curveindex within the curve
	*  @param [in]		fixingDates                 The fixing dates corresponding to our forward rates
	*  @param [in]		forwardRates		        The new discount factors
    *  @param [in]		setCorrespondingForwards	Set the corresponding discount factors on STD curves, will set DFs = 1.0 on STD curves if false
	*/
    AQLString tryAqCurveForwardRatesOverride( const AQLString& curveCollection,
                                             const AQLString& curveIndex,
                                             const DateVector& fixingDates,
                                             const DoubleVector& forwardRates,
                                             const bool setCorrespondingDiscountFactors )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS( curveCollection, curveIndex, fixingDates, forwardRates, setCorrespondingDiscountFactors );

        // Validate Inputs
		etrading::getCurveStaticDataTableName( curveCollection, curveIndex );
        AQ_REQUIRE( fixingDates.size() == forwardRates.size(), "Unable to Override Forward Rates - Inconsistent number of fixing dates & forward rates" );

        // Override the Curve Forward Rates
        AQLString result = etrading::setCurveForwardRates( curveCollection, curveIndex, fixingDates, forwardRates, setCorrespondingDiscountFactors ); // setCorrespondingDiscountFactors for STD curves only

        // Record Outputs and Return the result
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }

    /* @brief			Validation interface for tryAqCurveDiscountFactorsOverride function, which overrides curve discount factors
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndex			The curveindex within the curve
	*  @param [in]		paymentDates        The payment dates corresponding to our discount factors
	*  @param [in]		discountFactors		The new discount factors
    *  @param [in]		setCorrespondingForwards	Set the corresponding forwards on STD curves, will set Forwards on STD curves to zero if false. *** Note *** When true this involves reverse calibration solving and can fail to solve / converge for extreme data sets.
	*/
    AQLString tryAqCurveDiscountFactorsOverride( const AQLString& curveCollection,
                                                const AQLString& curveIndex,
                                                const DateVector& paymentDates,
                                                const DoubleVector& discountFactors,
                                                const bool setCorrespondingForwards )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS( curveCollection, curveIndex, paymentDates, discountFactors, setCorrespondingForwards );

        // Validate Inputs
        etrading::getCurveStaticDataTableName( curveCollection, curveIndex );
        AQ_REQUIRE( paymentDates.size() == discountFactors.size(), "Unable to Override Discount Factors - Inconsistent number of payment dates & discount factors" );

		// Override the Curve Discount Factors
        AQLString result = etrading::setCurveDiscountFactors( curveCollection, curveIndex, paymentDates, discountFactors, setCorrespondingForwards ); // setCorrespondingForwards for STD curves only

        // Record Outputs and Return the result
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }

    /* @brief			Validation interface for tryAqCurveDiscountFactorsSetToOne function, which sets all discount factors to one
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndex			The curveindex within the curve
	*/
    AQLString tryAqCurveDiscountFactorsSetToOne( const AQLString& curveCollection, const AQLString& curveIndex )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS( curveCollection, curveIndex );

        // Validate Inputs
        etrading::getCurveStaticDataTableName( curveCollection, curveIndex );

        // Override the Curve Discount Factors
        AQLString result = etrading::setCurveDiscountFactorsToOne( curveCollection, curveIndex );

        // Record Outputs and Return the result
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }

    /* @brief			Validation interface for tryAqCurveDiscountFactorsDisplay function, which retrieves all discount factors
	*  @param [in]		curveCollection		    The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndex			    The curveindex within the curve
	*  @param [out]		DiscountFactorTable     A discount factor table structure that contains paymentDates_ and discountFactors_
    */
    DiscountFactorTable tryAqCurveDiscountFactorsDisplay( const AQLString& curveCollection, const AQLString& curveIndex )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        const std::string filePrefix = curveCollection.getCString() + std::string("_") + curveIndex.getCString();
        const std::string fileSuffix = "";

        AQ_RECORD_DECORATED_INPUTS( filePrefix, fileSuffix, curveCollection, curveIndex )

        // Validate Inputs
        etrading::getCurveStaticDataTableName( curveCollection, curveIndex );

        // Get the Curve Discount Factors
        DiscountFactorTable results = etrading::getCurveDiscountFactors( curveCollection, curveIndex );

        // Record Outputs and return the results
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurveDiscountFactorsDisplay_outputs", curveCollection, curveIndex ) );
            file.write( "output",               results.discountFactors_    );
            file.write( "curveCollection",      curveCollection             );
            file.write( "curveIndex",           curveIndex                  );
            file.write( "paymentDates",         results.paymentDates_       );
            file.write( "discountFactors",      results.discountFactors_    );
        }
        return results;

        VALID_EXCEPTION_END
    }

    /* @brief			Validation interface for tryAqCurveTermsToDates function, which retrieves curve payment dates given terms year fractions
	*  @param [in]		curveCollection		    The curveCollection to use to get the curve asOfDate
	*  @param [in]		terms			        A vector of terms year fractions
	*  @param [out]		paymentDates            A vector of corresponding payment dates
    */
    DateVector tryAqCurveTermsToDates( const AQLString& curveCollection, const DoubleVector terms )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS( curveCollection, terms );

        etrading::checkIfCurveExists( etrading::getDataInstance(), curveCollection );
        DateVector result = etrading::convertCurveTermsToDates( curveCollection, terms );

        // Record Outputs and Return the result
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }

    /* @brief			Validation interface for tryAqCurveDatesToTerms function, which retrieves curve payment dates given terms year fractions
	*  @param [in]		curveCollection		    The curveCollection to use to get the curve asOfDate
	*  @param [in]		paymentDates			A vector of corresponding payment dates
	*  @param [out]		terms                   A vector of corresponding terms year fractions
    */
    DoubleVector tryAqCurveDatesToTerms( const AQLString& curveCollection, const DateVector paymentDates )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS( curveCollection, paymentDates );

        etrading::checkIfCurveExists( etrading::getDataInstance(), curveCollection );
        DoubleVector result = etrading::convertCurveDatesToTerms( curveCollection, paymentDates );

        // Record Outputs and Return the result
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }

}

