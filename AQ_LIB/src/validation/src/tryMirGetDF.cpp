#include "tryMirGetDF.h"

#include "LACurveForwardRateHelpers.h"
#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace
{
    /* @brief			Validation and defaulting routine on a list of given parameters
    *  @param [inout]	interp			Interpolation
    *  @param [inout]	daycount		Day count convention
    *  @param [inout]	curve			Name of curve from which we read DFs
    *  @param [in]		isBasisFlag		Is reading DFs from basis curve? A deprecated flag
    */
    void defaultingAndValidation1( LAString& interp, LAString& daycount, LAString& curve, bool isBasisFlag )
    {
        if( interp == LAString( "" ) )
        {
            interp = LAString( "SPLINE" );
        }

        if( daycount == LAString( "" ) )
        {
            daycount = LAString( "ACT/365" );
        }

        if( curve == LAString( "" ) )
        {
            curve = LAString( "OIS" );
        }

        if ( isBasisFlag )
        {
            throw LACoreInvalidData( "#Error: 'IsBasisFlag' is a deprecated input and it should be set to FALSE", __FILE__, __LINE__ );
        }
    }

    /* @brief			Validation and defaulting routine on a list of given parameters
    *  @param [inout]	interp			Interpolation
    *  @param [inout]	daycount		Day count convention
    *  @param [inout]	curve			Name of curve from which we read DFs
    *  @param [inout]	slidingRule		Sliding rule
    *  @param [inout]	calendar		Calendar
    *  @param [in]		isBasisFlag		Is reading DFs from basis curve? A deprecated flag
    */
    void defaultingAndValidation2( LAString& interp, LAString& daycount, LAString& curve, LAString& slidingRule, LAString& calendar, bool isBasisFlag )
    {
        defaultingAndValidation1( interp, daycount, curve, isBasisFlag );

        if( calendar == LAString( "" ) )
        {
            calendar = LAString( "TKB:LNB" );
        }

        if( slidingRule == LAString( "" ) )
        {
            slidingRule = LAString( "NO_CHANGE" );
        }
    }
}

namespace validation
{
    /* @brief			validation method for mirGetDF1
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		term			An array of terms by year count
    *  @param [in]		curveID			Curve set ID
    *  @param [in]		dayCount		Day count convention. Default to ACT/365
    *  @param [in]		interpolation	Interpolation method. Default to 'SPLINE'
    *  @param [in]		isBasisFlag		Is using basis curve or not. Deprecated input
    *  @param [in]		curveName		Name of the curve where DFs are read off
    *  @return			An array of discount factor
    */
    DoubleVector tryMirGetDF1( LADataInstance* dataInstance,
                               const DoubleVector& terms,
                               const LAString& curveID,
                               const LAString& dayCount,
                               const LAString& interpolation,
                               bool isBasisFlag,
                               const LAString& curveName )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMirGetDF1_inputs", curveID, curveName ) );
            file.write( "generatorFunction", "tryMirGetDF1" );
            file.write( "terms",			terms );
            file.write( "curveID",		curveID );
            file.write( "dayCount",		dayCount );
            file.write( "interpolation",	interpolation );
            file.write( "isBasisFlag",	isBasisFlag );
            file.write( "curveName",		curveName );
        }

        // Input validations
        if( terms.size() == 0 )
        {
            throw LACoreInvalidData( "#Error: Size of input 'terms' is zero.", __FILE__, __LINE__ );
        }

        LAString interp( interpolation );
        LAString daycount( dayCount );
        LAString curveNm( curveName );
        defaultingAndValidation1( interp, daycount, curveNm, isBasisFlag );

        // Get DF
        DoubleVector ret = etrading::LACurveForwardRateHelpers::getMultiDF( terms, dataInstance, curveID, daycount, interp, isBasisFlag, curveNm );

        if ( ret.size() == 0 )
        {
            throw LACoreInvalidData( "#Error: No DFs have been returned.", __FILE__, __LINE__ );
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMirGetDF1_outputs", curveID, curveNm ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation method for mirGetDF2
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		terms			An array of tenor strings
    *  @param [in]		curveID			Curve set ID
    *  @param [in]		dayCount		Day count convention. Default to ACT/365
    *  @param [in]		slidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
    *  @param [in]		calendar		Calendar. Default to 'TKB:LNB'
    *  @param [in]		interpolation	Interpolation method. Default to 'SPLINE'
    *  @param [in]		isBasisFlag		Is using basis curve or not. Deprecated input
    *  @param [in]		curveName		Name of the curve where DFs are read off
    *  @return			An array of discount factor
    */
    DoubleVector tryMirGetDF2( LADataInstance* dataInstance,
                               const LAStringVector& terms,
                               const LAString& curveID,
                               const LAString& dayCount,
                               const LAString& slidingRule,
                               const LAString& calendar,
                               const LAString& interpolation,
                               bool isBasisFlag,
                               const LAString& curveName )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMirGetDF2_inputs", curveID, curveName ) );
            file.write( "generatorFunction", "tryMirGetDF2" );
            file.write( "terms",			terms );
            file.write( "curveID",		curveID );
            file.write( "dayCount",		dayCount );
            file.write( "slidingRule",	slidingRule );
            file.write( "calendar",		calendar );
            file.write( "interpolation",	interpolation );
            file.write( "isBasisFlag",	isBasisFlag );
            file.write( "curveName",		curveName );
        }

        // Input validations
        if( terms.size() == 0 )
        {
            throw LACoreInvalidData( "#Error: Size of input 'terms' is zero.", __FILE__, __LINE__ );
        }

        LAString interp( interpolation );
        LAString daycount( dayCount );
        LAString curveNm( curveName );
        LAString slideRule( slidingRule );
        LAString cal( calendar );
        defaultingAndValidation2( interp, daycount, curveNm, slideRule, cal, isBasisFlag );

        DoubleVector ret = etrading::LACurveForwardRateHelpers::getMultiDF( terms, dataInstance, curveID, daycount, slideRule, cal, interp, isBasisFlag, curveNm );

        if ( ret.size() == 0 )
        {
            throw LACoreInvalidData( "#Error: No DFs have been returned.", __FILE__, __LINE__ );
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMirGetDF2_outputs", curveID, curveNm ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation method for mirGetDF3
    *  @param [in]		fromDate		A single from-date in YYYYMMDD formate
    *  @param [in]		toDate			An array of single to-date in YYYYMMDD formate
    *  @param [in]		curveID			Curve set ID
    *  @param [in]		dayCount		Day count convention. Default to ACT/365
    *  @param [in]		slidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
    *  @param [in]		calendar		Calendar. Default to 'TKB:LNB'
    *  @param [in]		interpolation	Interpolation method. Default to 'SPLINE'
    *  @param [in]		isBasisFlag		Is using basis curve or not. Deprecated input
    *  @param [in]		curveName		Name of the curve where DFs are read off. Default to 'STD'
    *  @return			A array of discount factors
    */
    DoubleVector tryMirGetDF3( LADataInstance* dataInstance,
                               const DateVector& fromDates,
                               const DateVector& toDates,
                               const LAString& curveID,
                               const LAString& dayCount,
                               const LAString& slidingRule,
                               const LAString& calendar,
                               const LAString& interpolation,
                               bool isBasisFlag,
                               const LAString& curveName )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMirGetDF3_inputs", curveID, curveName ) );
            file.write( "generatorFunction", "tryMirGetDF3" );
            file.write( "fromDates",		fromDates );
            file.write( "toDates",		toDates );
            file.write( "curveID",		curveID );
            file.write( "dayCount",		dayCount );
            file.write( "slidingRule",	slidingRule );
            file.write( "calendar",		calendar );
            file.write( "interpolation",	interpolation );
            file.write( "isBasisFlag",	isBasisFlag );
            file.write( "curveName",		curveName );
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

        // Other validations
        LAString interp( interpolation );
        LAString daycount( dayCount );
        LAString curveNm( curveName );
        LAString slideRule( slidingRule );
        LAString cal( calendar );
        defaultingAndValidation2( interp, daycount, curveNm, slideRule, cal, isBasisFlag );

        DoubleVector ret = etrading::LACurveForwardRateHelpers::getMultiDF( fromDateVec, toDates, dataInstance, curveID, daycount, slideRule, cal, interp, isBasisFlag, curveNm );

        if ( ret.size() == 0 )
        {
            throw LACoreInvalidData( "#Error: No DFs have been returned.", __FILE__, __LINE__ );
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMirGetDF3_outputs", curveID, curveNm ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation method for mirGetDF1
    *  @param [in]		fromDates		An array of from-dates in YYYYMMDD formate
    *  @param [in]		terms			An array of year fraction terms
    *  @param [in]		curveID			Curve set ID
    *  @param [in]		dayCount		Day count convention. Default to ACT/365
    *  @param [in]		slidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
    *  @param [in]		calendar		Calendar. Default to 'TKB:LNB'
    *  @param [in]		interpolation	Interpolation method. Default to 'SPLINE'
    *  @param [in]		isBasisFlag		Is using basis curve or not. Deprecated input
    *  @param [in]		curveName		Name of the curve where DFs are read off. Default to 'STD'
    *  @return			A array of discount factors
    */
    DoubleVector tryMirGetDF4( LADataInstance* dataInstance,
                               const DateVector& fromDates,
                               const DoubleVector& terms,
                               const LAString& curveID,
                               const LAString& dayCount,
                               const LAString& slidingRule,
                               const LAString& calendar,
                               const LAString& interpolation,
                               bool isBasisFlag,
                               const LAString& curveName )

    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMirGetDF4_inputs", curveID, curveName ) );
            file.write( "generatorFunction", "tryMirGetDF4" );
            file.write( "fromDates",		fromDates );
            file.write( "terms",			terms );
            file.write( "curveID",		curveID );
            file.write( "dayCount",		dayCount );
            file.write( "slidingRule",	slidingRule );
            file.write( "calendar",		calendar );
            file.write( "interpolation",	interpolation );
            file.write( "isBasisFlag",	isBasisFlag );
            file.write( "curveName",		curveName );
        }

        size_t M = fromDates.size();
        size_t N = terms.size();
        if( M != N )
        {
            throw LACoreInvalidData( "#Error: 'fromDates' and 'terms' must be of equal size'", __FILE__, __LINE__ );
        }

        if( N == 0 || M == 0 )
        {
            throw LACoreInvalidData( "#Error: 'fromDates' and 'terms' must not be empty", __FILE__, __LINE__ );
        }

        // Other validations
        LAString interp( interpolation );
        LAString daycount( dayCount );
        LAString curveNm( curveName );
        LAString slideRule( slidingRule );
        LAString cal( calendar );
        defaultingAndValidation2( interp, daycount, curveNm, slideRule, cal, isBasisFlag );

        DoubleVector ret = etrading::LACurveForwardRateHelpers::getMultiDF( fromDates, terms, dataInstance, curveID, daycount, slideRule, cal, interp, isBasisFlag, curveNm );

        if ( ret.size() == 0 )
        {
            throw LACoreInvalidData( "#Error: No DFs have been returned.", __FILE__, __LINE__ );
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMirGetDF4_outputs", curveID, curveNm ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

    /* @brief			validation method for mirGetDF5
    *  @param [in]		fromDates		An array of from-dates in YYYYMMDD formate
    *  @param [in]		term			A tenor string
    *  @param [in]		curveID			Curve set ID
    *  @param [in]		dayCount		Day count convention. Default to ACT/365
    *  @param [in]		slidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
    *  @param [in]		calendar		Calendar. Default to 'TKB:LNB'
    *  @param [in]		interpolation	Interpolation method. Default to 'SPLINE'
    *  @param [in]		isBasisFlag		Is using basis curve or not. Deprecated input
    *  @param [in]		curveName		Name of the curve where DFs are read off. Default to 'STD'
    *  @return			A array of discount factors
    */
    DoubleVector tryMirGetDF5( LADataInstance* dataInstance,
                               const DateVector& fromDates,
                               const LAString& term,
                               const LAString& curveID,
                               const LAString& dayCount,
                               const LAString& slidingRule,
                               const LAString& calendar,
                               const LAString& interpolation,
                               bool isBasisFlag,
                               const LAString& curveName )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMirGetDF5_inputs", curveID, curveName ) );
            file.write( "generatorFunction", "tryMirGetDF5" );
            file.write( "fromDates",		fromDates );
            file.write( "term",			term );
            file.write( "curveID",		curveID );
            file.write( "dayCount",		dayCount );
            file.write( "slidingRule",	slidingRule );
            file.write( "calendar",		calendar );
            file.write( "interpolation",	interpolation );
            file.write( "isBasisFlag",	isBasisFlag );
            file.write( "curveName",		curveName );
        }

        size_t M = fromDates.size();
        if( M == 0 )
        {
            throw LACoreInvalidData( "#Error: 'fromDates' must not be empty", __FILE__, __LINE__ );
        }

        // Other validations
        LAString interp( interpolation );
        LAString daycount( dayCount );
        LAString curveNm( curveName );
        LAString slideRule( slidingRule );
        LAString cal( calendar );
        defaultingAndValidation2( interp, daycount, curveNm, slideRule, cal, isBasisFlag );

        DoubleVector ret = etrading::LACurveForwardRateHelpers::getMultiDF( fromDates, term, dataInstance, curveID, daycount, slideRule, cal, interp, isBasisFlag, curveNm );

        if ( ret.size() == 0 )
        {
            throw LACoreInvalidData( "#Error: No DFs have been returned.", __FILE__, __LINE__ );
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMirGetDF5_outputs", curveID, curveNm ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }
}


