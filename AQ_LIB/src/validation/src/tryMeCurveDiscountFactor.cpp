#include <string>
#include <limits>
#include <boost/date_time.hpp>

#include "Environment.h"
#include "StructuredExceptionHandler.h"
#include "CreateDataFile.h"
#include "RecordMacros.h"
#include "LADateScheduleHelpers.h"
#include "LACurveForwardRateHelpers.h"
#include "ParameterValidation.h"
#include "tryMeDate.h"
#include "tryMeCurveDiscountFactor.h"
#include "CurveValidation.h"
#include "CurveStreaming.h"
#include "CurveResultsContainer.h"
#include "LWOUtilities.h"			// includes getCurveCollectionFromHandle()

using etrading::CreateDataFile;
using etrading::decorateCurvename;


namespace validation
{
    /* @brief			return the day count used in the core library
    */
    LAString getDayCount()
    {
        return LAString( "ACT/365" );
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
    void populateDiscountFactorConventions( const LAString& curveCollection, LAString& curveIndex, LAString& interpolation )
    {
        curveIndex = etrading::getDefaultValueForEmptyString( curveIndex, "OIS" );

        //Throw exception if the curve has not been built.
        LAString staticDataTable = etrading::getCurveStaticDataTableName( curveCollection, curveIndex );
        interpolation = etrading::getCurveInterpolation( curveCollection, staticDataTable );
    }

    /* @brief			populate and setting default values
    *  @param [in]		curveCollection	curve collection
    *  @param [inout]	curveIndex		curve index
    *  @param [inout]	interpolation	interpolation
    *  @param [inout]	businessDayAdj	business day adjustment
    *  @param [inout]	calendar		calendar
    */
    void populateDiscountFactorConventions( const LAString& curveCollection, LAString& curveIndex, LAString& interpolation, LAString& businessDayAdj, LAString& calendar )
    {
        populateDiscountFactorConventions( curveCollection, curveIndex, interpolation );
        businessDayAdj = etrading::getDefaultValueForEmptyString( businessDayAdj, "NO_CHANGE" );
        calendar = etrading::getDefaultCalendarForEmptyString( calendar, curveCollection );
    }

    /* @brief			validation method for meCurveDiscountFactorsFromYearFractions
    *  @param [in]		yearFractions	An array of yearFractions by year count
    *  @param [in]		dayCount		Day count
    *  @param [in]		curveCollection	Curve set ID
    *  @param [in]		curveIndex		Index of the curve set. Default to OIS
    *  @return			An array of discount factor
    */
    DoubleVector tryMeCurveDiscountFactorsFromYearFractions( const DoubleVector& yearFractions,
                                                             const LAString& dayCount,
                                                             const LAString& curveCollection,
                                                             const LAString& curveIndex )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeCurveDiscountFactorsFromYearFractions_inputs", curveCollection, curveIndex ) );
            file.write( "generatorFunction", "tryMeCurveDiscountFactorsFromYearFractions" );
            file.write( "yearFractions",		yearFractions );
            file.write( "dayCount",          dayCount );
            file.write( "curveCollection",	curveCollection );
            file.write( "curveIndex",		curveIndex );
        }

        // Get DF
        DoubleVector ret = etrading::getCurveDiscountFactorsFromYearFractions(yearFractions, dayCount, curveCollection, curveIndex);

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeCurveDiscountFactorsFromYearFractions_outputs", curveCollection, curveIndex ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

    DoubleVector tryMeLWOCurveDiscountFactorsFromYearFractions( const DoubleVector& yearFractions,
                                                                const LAString& dayCount,
                                                                const std::string& lwoCurveName )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( std::string( std::string( "tryMeLWOCurveDiscountFactorsFromYearFractions_inputs_" ) + lwoCurveName ).c_str()  );
            file.write( "generatorFunction", "tryMeLWOCurveDiscountFactorsFromYearFractions" );
            file.write( "yearFractions",		yearFractions );
            file.write( "dayCount",          dayCount );
            file.write( "lwoCurveName",	    lwoCurveName );
        }

        // Input validations
        if( yearFractions.size() == 0 )
        {
            throw LACoreInvalidData( "#Error: Size of input 'yearFractions' is zero.", __FILE__, __LINE__ );
        }

        DoubleVector ret( 0, std::numeric_limits<double>::quiet_NaN() );

        auto& env = etrading::Environment::defaultEnv();
        auto lwoCurve = env.accessObject<etrading::LWOCurve>( lwoCurveName ); // 			etrading::getLWOCurve(lwoCurveName);

        // Convert Year Fractions to ACT/365 ones
        LADate curveAsOfDate = etrading::toLADateFromGregorianDate( lwoCurve->getCurveBuildStaticDataObject()->asOfDate_ );
        etrading::convertToYearFractionsACT365( curveAsOfDate, const_cast< DoubleVector& >( yearFractions ), dayCount );

        if( lwoCurve )
        {
            ret = lwoCurve->calculateDiscountFactor( yearFractions );
        }
        else
        {
            AQ_THROW( ( boost::format( "Curve %s does not exist." ) % lwoCurveName.c_str() ).str().c_str() );
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( std::string( std::string( "tryMeLWOCurveDiscountFactorsFromYearFractions_outputs_" ) + lwoCurveName ).c_str() );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }



    /* @brief			validation method for meCurveDiscountFactorsFromTenors
    *  @param [in]		tenors			An array of tenor strings
    *  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
    *  @param [in]		calendar		Calendar
    *  @param [in]		curveCollection	Curve set ID
    *  @param [in]		curveIndex		Index of the curve set. Default to OIS
    *  @return			An array of discount factor
    */
    DoubleVector tryMeCurveDiscountFactorsFromTenors( const LAStringVector& tenors,
            const LAString& businessDayAdj,
            const LAString& calendar,
            const LAString& curveCollection,
            const LAString& curveIndex )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeCurveDiscountFactorsFromTenors_inputs", curveCollection, curveIndex ) );
            file.write( "generatorFunction", "tryMeCurveDiscountFactorsFromTenors" );
            file.write( "tenors",			tenors );
            file.write( "businessDayAdj",	businessDayAdj );
            file.write( "calendar",			calendar );
            file.write( "curveCollection",	curveCollection );
            file.write( "curveIndex",		curveIndex );
        }

        // Input validations
        if( tenors.size() == 0 )
        {
            throw LACoreInvalidData( "#Error: Size of input 'tenors' is zero.", __FILE__, __LINE__ );
        }

        LAString curIndex( curveIndex );
        LAString interp;
        LAString bdAdj( businessDayAdj );
        LAString cal( calendar );
        populateDiscountFactorConventions( curveCollection, curIndex, interp, bdAdj, cal );

        DoubleVector ret = etrading::LACurveForwardRateHelpers::getMultiDF( tenors, etrading::getDataInstance(), curveCollection, getDayCount(), bdAdj, cal, interp, isBasisFlag(), curIndex );
        if ( ret.size() == 0 )
        {
            throw LACoreInvalidData( "#Error: No DFs have been returned.", __FILE__, __LINE__ );
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeCurveDiscountFactorsFromTenors_outputs", curveCollection, curveIndex ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

    /* @brief			validation method for meCurveDiscountFactorsFromTenors
    *  @param [in]		tenors			An array of tenor strings
    *  @param [in]		curveCollection	Curve set ID
    *  @param [in]		curveIndex		Index of the curve set. Default to OIS
    *  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
    *  @param [in]		calendar		Calendar
    *  @return			An array of discount factor
    */
    DoubleVector tryMeLWOCurveDiscountFactorsFromTenors( const std::vector<std::string>& tenors,
            const std::string& businessDayAdj,
            const std::string& calendar,
            const std::string& lwoCurveName )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( std::string( std::string( "tryMeLWOCurveDiscountFactorsFromTenors_inputs_" ) + lwoCurveName ).c_str()  );
            file.write( "generatorFunction", "tryMeLWOCurveDiscountFactorsFromTenors" );
            file.write( "tenors",			tenors );
            file.write( "businessDayAdj",	businessDayAdj );
            file.write( "calendar",			calendar );
            file.write( "lwoCurveName",	    lwoCurveName );
        }

        // Input validations
        if( tenors.size() == 0 )
        {
            throw LACoreInvalidData( "#Error: Size of input 'tenors' is zero.", __FILE__, __LINE__ );
        }

        etrading::BusinessDayAdjustmentEnum busDayAdjust = etrading::toBusinessDayAdjustmentEnum( businessDayAdj );

        DoubleVector ret( 0, std::numeric_limits<double>::quiet_NaN() );
        auto& env = etrading::Environment::defaultEnv();
        auto lwoCurve = env.accessObject<etrading::LWOCurve>( lwoCurveName ); // 			etrading::getLWOCurve(lwoCurveName);
        if( lwoCurve )
        {
            ret = lwoCurve->calculateDiscountFactor( tenors, busDayAdjust, calendar );
        }
        else
        {
            AQ_THROW( ( boost::format( "Curve %s does not exist." ) % lwoCurveName.c_str() ).str().c_str() );
        }

        if ( ret.size() == 0 )
        {
            throw LACoreInvalidData( "#Error: No DFs have been returned.", __FILE__, __LINE__ );
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( std::string( std::string( "tryMeLWOCurveDiscountFactorsFromTenors_outputs_" ) + lwoCurveName ).c_str()  );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation method for meCurveDiscountFactorsForwardStarting
    *  @param [in]		fromDates		A single from-date in YYYYMMDD formate
    *  @param [in]		toDates			An array of single to-date in YYYYMMDD formate
    *  @param [in]		curveCollection	Curve set ID
    *  @param [in]		curveIndex		Index of the curve set. Default to OIS
    *  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
    *  @param [in]		calendar		Calendar
    *  @return			A array of discount factors
    */
    DoubleVector tryMeCurveDiscountFactorsForwardStarting( const DateVector& fromDates,
            const DateVector& toDates,
            const LAString& curveCollection,
            const LAString& curveIndex )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeCurveDiscountFactorsForwardStarting_inputs", curveCollection, curveIndex ) );
            file.write( "generatorFunction", "tryMeCurveDiscountFactorsForwardStarting" );
            file.write( "fromDates",			fromDates );
            file.write( "toDates",			toDates );
            file.write( "curveCollection",	curveCollection );
            file.write( "curveIndex",		curveIndex );
        }

        size_t M = fromDates.size();
        size_t N = toDates.size();
        if( M != N && M != 1 )
        {
            throw LACoreInvalidData( "#Error: 'fromDates' either takes 1 date, or an array of dates in equal size of 'toDates'", __FILE__, __LINE__ );
        }

        if( N == 0 || M == 0 )
        {
            throw LACoreInvalidData( "#Error: 'fromDates' and 'toDates' must not be empty", __FILE__, __LINE__ );
        }

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

        LAString curIndex( curveIndex );
        LAString interp;
        LAString bdAdj( "NO_CHANGE" );
        LAString cal;
        populateDiscountFactorConventions( curveCollection, curIndex, interp, bdAdj, cal );

        DoubleVector ret = etrading::LACurveForwardRateHelpers::getMultiDF( fromDateVec, toDates, etrading::getDataInstance(), curveCollection, getDayCount(), bdAdj, cal, interp, isBasisFlag(), curIndex );

        if ( ret.size() == 0 )
        {
            throw LACoreInvalidData( "#Error: No DFs have been returned.", __FILE__, __LINE__ );
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeCurveDiscountFactorsForwardStarting_outputs", curveCollection, curveIndex ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

    DoubleVector tryMeLWOCurveDiscountFactorsForwardStarting( const std::vector<boost::gregorian::date>& fromDates,
            const std::vector<boost::gregorian::date>& toDates,
            const std::string& businessDayAdj,
            const std::string& calendar,
            const std::string& lwoCurveName )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryMeLWOCurveDiscountFactorsForwardStarting_inputs_" ) + lwoCurveName ).c_str() );
            file.write( "generatorFunction", "tryMeLWOCurveDiscountFactorsForwardStarting" );
            file.write( "fromDates",			fromDates );
            file.write( "toDates",			toDates );
            file.write( "businessDayAdj",	businessDayAdj );
            file.write( "calendar",			calendar );
            file.write( "lwoCurveName",	    lwoCurveName );
        }

        size_t M = fromDates.size();
        size_t N = toDates.size();
        if( M != N && M != 1 )
        {
            throw LACoreInvalidData( "#Error: 'fromDates' either takes 1 date, or an array of dates in equal size of 'toDates'", __FILE__, __LINE__ );
        }
        if( N == 0 || M == 0 )
        {
            throw LACoreInvalidData( "#Error: 'fromDates' and 'toDates' must not be empty", __FILE__, __LINE__ );
        }

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
        auto lwoCurve = env.accessObject<etrading::LWOCurve>( lwoCurveName ); // 			etrading::getLWOCurve(lwoCurveName);

        if( lwoCurve )
        {
            ret = lwoCurve->calculateDiscountFactor( fromDateVec, toDates, busDayAdjust, calendar );
        }
        else
        {
            AQ_THROW( ( boost::format( "Curve %s does not exist." ) % lwoCurveName.c_str() ).str().c_str() );
        }

        if ( ret.size() == 0 )
        {
            throw LACoreInvalidData( "#Error: No DFs have been returned.", __FILE__, __LINE__ );
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryMeLWOCurveDiscountFactorsForwardStarting_outputs_" ) + lwoCurveName ).c_str() );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END

    };


    /* @brief			validation method for meCurveDiscountFactorsForwardStartingFromYearFractions
    *  @param [in]		fromDates			An array of from-dates in YYYYMMDD formate
    *  @param [in]		yearFractions		An array of year fraction yearFractions
    *  @param [in]		dayCount	        The daycount fraction used to generate the yearFraction
    *  @param [in]		curveCollection		Curve set ID
    *  @param [in]		curveIndex			Index of the curve set. Default to OIS
    *  @return			A array of discount factors
    */
    DoubleVector tryMeCurveDiscountFactorsForwardStartingFromYearFractions( const DateVector& fromDates,
                                                                            const DoubleVector& yearFractions,
                                                                            const LAString& dayCount,
                                                                            const LAString& curveCollection,
                                                                            const LAString& curveIndex )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeCurveDiscountFactorsForwardStartingFromYearFractions_inputs", curveCollection, curveIndex ) );
            file.write( "generatorFunction", "tryMeCurveDiscountFactorsForwardStartingFromYearFractions" );
            file.write( "fromDates",			fromDates );
            file.write( "yearFractions",		yearFractions );
            file.write( "dayCount",          dayCount );
            file.write( "curveCollection",	curveCollection );
            file.write( "curveIndex",		curveIndex );
        }

        DoubleVector ret = etrading::getCurveDiscountFactorsForwardStartingFromYearFractions( fromDates, yearFractions, dayCount, curveCollection,  curveIndex );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeCurveDiscountFactorsForwardStartingFromYearFractions_outputs", curveCollection, curveIndex ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

    /* @brief			validation method for meCurveDiscountFactorsForwardStartingFromYearFractions
    *  @param [in]		fromDates			An array of from-dates in YYYYMMDD formate
    *  @param [in]		yearFractions		An array of year fraction yearFractions
    *  @param [in]		dayCount	        The daycount fraction used to generate the yearFraction
    *  @param [in]		lwoCurveName		Curve Name
    *  @return			A array of discount factors
    */
    DoubleVector tryMeLWOCurveDiscountFactorsForwardStartingFromYearFractions( const std::vector<boost::gregorian::date>& fromDates,
            const DoubleVector& yearFractions,
            const LAString& dayCount,
            const std::string& lwoCurveName )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryMeLWOCurveDiscountFactorsForwardStartingFromYearFractions_inputs_" ) + lwoCurveName ).c_str() );
            file.write( "generatorFunction", "tryMeLWOCurveDiscountFactorsForwardStartingFromYearFractions" );
            file.write( "fromDates",			fromDates );
            file.write( "yearFractions",		yearFractions );
            file.write( "dayCount",          dayCount );
            file.write( "lwoCurveName",		lwoCurveName );
        }

        size_t M = fromDates.size();
        size_t N = yearFractions.size();
        if( M != N )
        {
            throw LACoreInvalidData( "#Error: 'fromDates' and 'yearFractions' must be of equal size'", __FILE__, __LINE__ );
        }

        if( N == 0 || M == 0 )
        {
            throw LACoreInvalidData( "#Error: 'fromDates' and 'yearFractions' must not be empty", __FILE__, __LINE__ );
        }

        etrading::BusinessDayAdjustmentEnum busDayAdjust = etrading::toBusinessDayAdjustmentEnum( "NO_CHANGE" );
        DoubleVector ret( 0, std::numeric_limits<double>::quiet_NaN() );

        auto& env = etrading::Environment::defaultEnv();
        auto lwoCurve = env.accessObject<etrading::LWOCurve>( lwoCurveName ); // 			etrading::getLWOCurve(lwoCurveName);


        // Convert Year Fractions to ACT/365 ones
        LADate curveAsOfDate = etrading::toLADateFromGregorianDate( lwoCurve->getCurveBuildStaticDataObject()->asOfDate_ );
        etrading::convertToYearFractionsACT365( curveAsOfDate, const_cast< DoubleVector& >( yearFractions ), dayCount );

        if( lwoCurve )
        {
            std::string calendar( "" );
            ret = lwoCurve->calculateDiscountFactor( fromDates, yearFractions, busDayAdjust, calendar );
        }
        else
        {
            AQ_THROW( ( boost::format( "Curve %s does not exist." ) % lwoCurveName.c_str() ).str().c_str() );
        }

        if ( ret.size() == 0 )
        {
            throw LACoreInvalidData( "#Error: No DFs have been returned.", __FILE__, __LINE__ );
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryMeCurveDiscountFactorsForwardStartingFromYearFractions_outputs_" ) + lwoCurveName ).c_str() );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation method for meCurveDiscountFactorsForwardStartingFromTenor
    *  @param [in]		fromDates		An array of from-dates in YYYYMMDD formate
    *  @param [in]		tenor			A tenor string
    *  @param [in]		curveCollection	Curve set ID
    *  @param [in]		curveIndex		Index of the curve set. Default to OIS
    *  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
    *  @param [in]		calendar		Calendar
    *  @return			A array of discount factors
    */
    DoubleVector tryMeCurveDiscountFactorsForwardStartingFromTenor( const DateVector& fromDates,
            const LAString& tenor,
            const LAString& curveCollection,
            const LAString& curveIndex,
            const LAString& businessDayAdj,
            const LAString& calendar )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeCurveDiscountFactorsForwardStartingFromTenor_inputs", curveCollection, curveIndex ) );
            file.write( "generatorFunction", "tryMeCurveDiscountFactorsForwardStartingFromTenor" );
            file.write( "fromDates",			fromDates );
            file.write( "tenor",				tenor );
            file.write( "curveCollection",	curveCollection );
            file.write( "curveIndex",		curveIndex );
            file.write( "businessDayAdj",	businessDayAdj );
            file.write( "calendar",			calendar );
        }

        if( fromDates.size() == 0 )
        {
            throw LACoreInvalidData( "#Error: 'fromDates' must not be empty", __FILE__, __LINE__ );
        }

        LAString curIndex( curveIndex );
        LAString interp;
        LAString bdAdj( businessDayAdj );
        LAString cal( calendar );
        populateDiscountFactorConventions( curveCollection, curIndex, interp, bdAdj, cal );

        DoubleVector ret = etrading::LACurveForwardRateHelpers::getMultiDF( fromDates, tenor, etrading::getDataInstance(), curveCollection, getDayCount(), bdAdj, cal, interp, isBasisFlag(), curIndex );
        if ( ret.size() == 0 )
        {
            throw LACoreInvalidData( "#Error: No DFs have been returned.", __FILE__, __LINE__ );
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeCurveDiscountFactorsForwardStartingFromTenor_outputs", curveCollection, curveIndex ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

    /* @brief			validation method for meCurveDiscountFactorsForwardStartingFromTenor
    *  @param [in]		fromDates		An array of from-dates in YYYYMMDD formate
    *  @param [in]		tenor			A tenor string
    *  @param [in]		curveCollection	Curve set ID
    *  @param [in]		curveIndex		Index of the curve set. Default to OIS
    *  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
    *  @param [in]		calendar		Calendar
    *  @return			A array of discount factors
    */
    DoubleVector tryMeLWOCurveDiscountFactorsForwardStartingFromTenor( const std::vector<boost::gregorian::date>& fromDates,
            const std::vector<std::string>& tenors,
            const std::string& businessDayAdj,
            const std::string& calendar,
            const std::string& lwoCurveName )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryMeLWOCurveDiscountFactorsForwardStartingFromTenor_inputs_" ) + lwoCurveName ).c_str() );
            file.write( "generatorFunction", "tryMeLWOCurveDiscountFactorsForwardStartingFromTenor" );
            file.write( "fromDates",			fromDates );
            file.write( "tenor",				tenors );
            file.write( "businessDayAdj",	businessDayAdj );
            file.write( "calendar",			calendar );
            file.write( "curveIndex",		lwoCurveName );
        }

        if( fromDates.size() == 0 )
        {
            throw LACoreInvalidData( "#Error: 'fromDates' must not be empty", __FILE__, __LINE__ );
        }

        if( fromDates.size() != tenors.size() )
        {
            std::string errString = ( boost::format( "Number of fromDates (%i) is not equal to the number of tenors (%i)" ) % fromDates.size() % tenors.size() ).str();
            throw LACoreInvalidData( errString.c_str(), __FILE__, __LINE__ );
        }

        etrading::BusinessDayAdjustmentEnum busDayAdjust = etrading::toBusinessDayAdjustmentEnum( businessDayAdj );
        DoubleVector ret( 0, std::numeric_limits<double>::quiet_NaN() );

        auto& env = etrading::Environment::defaultEnv();
        auto lwoCurve = env.accessObject<etrading::LWOCurve>( lwoCurveName ); // 			etrading::getLWOCurve(lwoCurveName);
        if( lwoCurve )
        {
            ret = lwoCurve->calculateDiscountFactor( fromDates, tenors, busDayAdjust, calendar );
        }
        else
        {
            AQ_THROW( ( boost::format( "Curve %s does not exist." ) % lwoCurveName.c_str() ).str().c_str() );
        }

        if ( ret.size() == 0 )
        {
            throw LACoreInvalidData( "#Error: No DFs have been returned.", __FILE__, __LINE__ );
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryMeLWOCurveDiscountFactorsForwardStartingFromTenor_outputs_" ) + lwoCurveName ).c_str() );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation method for meCurveDiscountFactors
    *  @param [in]		toDates			A single or an array of to-date in YYYYMMDD formate
    *  @param [in]		curveCollection	Curve set ID
    *  @param [in]		curveIndex		Index of the curve set. Default to OIS
    *  @return			A array of discount factors
    */
    DoubleVector tryMeCurveDiscountFactors( const DateVector& toDates,
                                            const LAString& curveCollectionOrHandle,
                                            const LAString& curveIndex )
    {
        VALID_EXCEPTION_START

		// To Allow Support for Curve Objects or Curve Collections
		LAString curveCollectionFromHandle = etrading::getCurveCollectionFromHandle( curveCollectionOrHandle.c_str() );

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeCurveDiscountFactors_inputs", curveCollectionFromHandle, curveIndex ) );
            file.write( "generatorFunction", "tryMeCurveDiscountFactors" );
            file.write( "toDates",			toDates );
            file.write( "curveCollection",	curveCollectionFromHandle );
            file.write( "curveIndex",		curveIndex );
        }

        size_t N = toDates.size();
        if( N == 0 )
        {
            throw LACoreInvalidData( "#Error: 'toDates' must not be empty", __FILE__, __LINE__ );
        }

        LAString curIndex( curveIndex );
        LAString interp;
        LAString bdAdj( "NO_CHANGE" );
        LAString cal;
        populateDiscountFactorConventions( curveCollectionFromHandle, curIndex, interp, bdAdj, cal );

        DoubleVector ret = etrading::LACurveForwardRateHelpers::getMultiSpotDiscountFactors( toDates, etrading::getDataInstance(), curveCollectionFromHandle, getDayCount(), bdAdj, cal, interp, isBasisFlag(), curIndex );

        if ( ret.size() == 0 )
        {
            throw LACoreInvalidData( "#Error: No DFs have been returned.", __FILE__, __LINE__ );
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeCurveDiscountFactors_outputs", curveCollectionFromHandle, curveIndex ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }
	
    /* @brief			Validation interface for tryMeCurveForwardRatesOverride function, which overrides curve forward rates by setting equivalent discount factors
	*  @param [in]		curveCollection		        The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndex			        The curveindex within the curve
	*  @param [in]		fixingDates                 The fixing dates corresponding to our forward rates
	*  @param [in]		forwardRates		        The new discount factors
    *  @param [in]		setCorrespondingForwards	Set the corresponding discount factors on STD curves, will set DFs = 1.0 on STD curves if false
	*/
    LAString tryMeCurveForwardRatesOverride( const LAString& curveCollection,
                                             const LAString& curveIndex,
                                             const DateVector& fixingDates,
                                             const DoubleVector& forwardRates,
                                             const bool setCorrespondingDiscountFactors )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
		RECORD_INPUTS( curveCollection, curveIndex, fixingDates, forwardRates, setCorrespondingDiscountFactors );

        // Validate Inputs
		etrading::getCurveStaticDataTableName( curveCollection, curveIndex );
        AQ_REQUIRE( fixingDates.size() == forwardRates.size(), "Unable to Override Forward Rates - Inconsistent number of fixing dates & forward rates" );

        // Override the Curve Forward Rates
        LAString result = etrading::setCurveForwardRates( curveCollection, curveIndex, fixingDates, forwardRates, setCorrespondingDiscountFactors ); // setCorrespondingDiscountFactors for STD curves only

        // Record Outputs and Return the result
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }

    /* @brief			Validation interface for tryMeCurveDiscountFactorsOverride function, which overrides curve discount factors
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndex			The curveindex within the curve
	*  @param [in]		paymentDates        The payment dates corresponding to our discount factors
	*  @param [in]		discountFactors		The new discount factors
    *  @param [in]		setCorrespondingForwards	Set the corresponding forwards on STD curves, will set Forwards on STD curves to zero if false. *** Note *** When true this involves reverse calibration solving and can fail to solve / converge for extreme data sets.
	*/
    LAString tryMeCurveDiscountFactorsOverride( const LAString& curveCollection,
                                                const LAString& curveIndex,
                                                const DateVector& paymentDates,
                                                const DoubleVector& discountFactors,
                                                const bool setCorrespondingForwards )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
		RECORD_INPUTS( curveCollection, curveIndex, paymentDates, discountFactors, setCorrespondingForwards );

        // Validate Inputs
        etrading::getCurveStaticDataTableName( curveCollection, curveIndex );
        AQ_REQUIRE( paymentDates.size() == discountFactors.size(), "Unable to Override Discount Factors - Inconsistent number of payment dates & discount factors" );

		// Override the Curve Discount Factors
        LAString result = etrading::setCurveDiscountFactors( curveCollection, curveIndex, paymentDates, discountFactors, setCorrespondingForwards ); // setCorrespondingForwards for STD curves only

        // Record Outputs and Return the result
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }

    /* @brief			Validation interface for tryMeCurveDiscountFactorsSetToOne function, which sets all discount factors to one
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndex			The curveindex within the curve
	*/
    LAString tryMeCurveDiscountFactorsSetToOne( const LAString& curveCollection, const LAString& curveIndex )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
		RECORD_INPUTS( curveCollection, curveIndex );

        // Validate Inputs
        etrading::getCurveStaticDataTableName( curveCollection, curveIndex );

        // Override the Curve Discount Factors
        LAString result = etrading::setCurveDiscountFactorsToOne( curveCollection, curveIndex );

        // Record Outputs and Return the result
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }

    /* @brief			Validation interface for tryMeCurveDiscountFactorsDisplay function, which retrieves all discount factors
	*  @param [in]		curveCollection		    The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndex			    The curveindex within the curve
	*  @param [out]		DiscountFactorTable     A discount factor table structure that contains paymentDates_ and discountFactors_
    */
    DiscountFactorTable tryMeCurveDiscountFactorsDisplay( const LAString& curveCollection, const LAString& curveIndex )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        const std::string filePrefix = curveCollection.getCString() + std::string("_") + curveIndex.getCString();
        const std::string fileSuffix = "";

        RECORD_DECORATED_INPUTS( filePrefix, fileSuffix, curveCollection, curveIndex )

        // Validate Inputs
        etrading::getCurveStaticDataTableName( curveCollection, curveIndex );

        // Get the Curve Discount Factors
        DiscountFactorTable results = etrading::getCurveDiscountFactors( curveCollection, curveIndex );

        // Record Outputs and return the results
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeCurveDiscountFactorsDisplay_outputs", curveCollection, curveIndex ) );
            file.write( "output",               results.discountFactors_    );
            file.write( "curveCollection",      curveCollection             );
            file.write( "curveIndex",           curveIndex                  );
            file.write( "paymentDates",         results.paymentDates_       );
            file.write( "discountFactors",      results.discountFactors_    );
        }
        return results;

        VALID_EXCEPTION_END
    }

    /* @brief			Validation interface for tryMeCurveTermsToDates function, which retrieves curve payment dates given terms year fractions
	*  @param [in]		curveCollection		    The curveCollection to use to get the curve asOfDate
	*  @param [in]		terms			        A vector of terms year fractions
	*  @param [out]		paymentDates            A vector of corresponding payment dates
    */
    DateVector tryMeCurveTermsToDates( const LAString& curveCollection, const DoubleVector terms )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
		RECORD_INPUTS( curveCollection, terms );

        etrading::checkIfCurveExists( etrading::getDataInstance(), curveCollection );
        DateVector result = etrading::convertCurveTermsToDates( curveCollection, terms );

        // Record Outputs and Return the result
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }

    /* @brief			Validation interface for tryMeCurveDatesToTerms function, which retrieves curve payment dates given terms year fractions
	*  @param [in]		curveCollection		    The curveCollection to use to get the curve asOfDate
	*  @param [in]		paymentDates			A vector of corresponding payment dates
	*  @param [out]		terms                   A vector of corresponding terms year fractions
    */
    DoubleVector tryMeCurveDatesToTerms( const LAString& curveCollection, const DateVector paymentDates )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
		RECORD_INPUTS( curveCollection, paymentDates );

        etrading::checkIfCurveExists( etrading::getDataInstance(), curveCollection );
        DoubleVector result = etrading::convertCurveDatesToTerms( curveCollection, paymentDates );

        // Record Outputs and Return the result
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }

}

