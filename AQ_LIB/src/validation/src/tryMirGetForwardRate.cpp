#include "tryMirGetForwardRate.h"
#include "AQLCurveForwardRateHelpers.h"
#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation
{


    /* @brief			validation interface for mrGetForwardRate1
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		fromDateVec	    A vector of from dates
    *  @param [in]		term			Term
    *  @param [in]		curveID			ID of the yield curve
    *  @param [in]		frequency		Frequency of the yield curve
    *  @param [in]		dayCount		Day count convention of the yield curve
    *  @param [in]		slidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		calendar		Calendar of the yield curve
    *  @param [in]		interpolation	Interpolation method of the yield curve
    *  @param [in]		curveName		Type of the yield curve, default to STD
    *  @param [in]		isFwdInterp		Boolean that decides if direct interpolation on fwd rates is employed
    *  @return			The forward rates based on fromDates and term
    */
    DoubleVector tryMirGetForwardRate1( AQLDataInstance* dataInstance,
                                        const DateVector& fromDateVec,
                                        double term,
                                        const AQLString& curveId,
                                        const AQLString& frequency,
                                        const AQLString& dayCount,
                                        const AQLString& slidingRule,
                                        const AQLString& calendar,
                                        const AQLString& interpolation,
                                        const AQLString& curveName,
                                        bool isFwdInterp )
    {

        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMirGetForwardRate1_inputs", curveId, curveName ) );
            file.write( "generatorFunction", "tryMirGetForwardRate1" );
            file.write( "fromDateVec", fromDateVec );
            file.write( "term", term );
            file.write( "curveId", curveId );
            file.write( "frequency", frequency );
            file.write( "dayCount", dayCount );
            file.write( "slidingRule", slidingRule );
            file.write( "calendar", calendar );
            file.write( "interpolation", interpolation );
            file.write( "curveName", curveName );
            file.write( "isFwdInterp", isFwdInterp );
        }

        if( isFwdInterp && AQLString( frequency ).toUpper() != "SIMPLE" )
        {
            throw AQLCoreInvalidData( "forward interpolation may not be used with Frequency other than 'SIMPLE'", __FILE__, __LINE__ );
        }


        if( fromDateVec.size() == 0 )
        {
            throw AQLCoreInvalidData( "a size of  vector is zero.", __FILE__, __LINE__ );
        }

        AQLString freq( etrading::getDefaultValueForEmptyString( frequency, AQLString( "SEMI-ANNUAL" ) ) );
        AQLString dayC( etrading::getDefaultValueForEmptyString( dayCount, AQLString( "ACT/365" ) ) );
        AQLString sRule( etrading::getDefaultValueForEmptyString( slidingRule, AQLString( "NO_CHANGE" ) ) );
        AQLString cal( etrading::getDefaultValueForEmptyString( calendar, AQLString( "TKB:LNB" ) ) );
        AQLString interp( etrading::getDefaultValueForEmptyString( interpolation, AQLString( "SPLINE" ) ) );
        AQLString curveNm( etrading::getDefaultValueForEmptyString( curveName, AQLString( "STD" ) ) );

        DoubleArray ret = etrading::AQLCurveForwardRateHelpers::getMultiForwardRate( fromDateVec, term, dataInstance, curveId, freq, dayC, sRule, calendar, interp, curveNm, isFwdInterp );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMirGetForwardRate1_outputs", curveId, curveNm ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

    /* @brief			validation interface for mrGetForwardRate2
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		fromDates	    A vector of from dates
    *  @param [in]		toDates			A vector of to dates
    *  @param [in]		curveID			ID of the yield curve
    *  @param [in]		frequency		Frequency of the yield curve
    *  @param [in]		dayCount		Day count convention of the yield curve
    *  @param [in]		slidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		calendar		Calendar of the yield curve
    *  @param [in]		interpolation	Interpolation method of the yield curve
    *  @param [in]		curveName		Type of the yield curve, default to STD
    *  @param [in]		isFwdInterp		Boolean that decides if direct interpolation on fwd rates is employed
    *  @param [in]		useFwdData		Use False to imply forwards from Discount Factors and True to use Forward Data directly. It's default to False.
    *  @return			The forward rates based on fromDates and toDates
    */
    DoubleVector tryMirGetForwardRate2( AQLDataInstance* dataInstance,
                                        const DateVector& fromDates,
                                        const DateVector& toDates,
                                        const AQLString& curveId,
                                        const AQLString& frequency,
                                        const AQLString& dayCount,
                                        const AQLString& slidingRule,
                                        const AQLString& calendar,
                                        const AQLString& interpolation,
                                        const AQLString& curveName,
                                        bool isFwdInterp,
                                        bool useFwdData )

    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMirGetForwardRate2_inputs", curveId, curveName ) );
            file.write( "generatorFunction", "tryMirGetForwardRate2" );
            file.write( "fromDates", fromDates );
            file.write( "toDates", toDates );
            file.write( "curveId", curveId );
            file.write( "frequency", frequency );
            file.write( "dayCount", dayCount );
            file.write( "slidingRule", slidingRule );
            file.write( "calendar", calendar );
            file.write( "interpolation", interpolation );
            file.write( "curveName", curveName );
            file.write( "isFwdInterp", isFwdInterp );
            file.write( "useFwdData", useFwdData );
        }

        // Validate parameters
        if( isFwdInterp && AQLString( frequency ).toUpper() != "SIMPLE" )
        {
            throw AQLCoreInvalidData( "forward interpolation may not be used with Frequency other than 'SIMPLE'", __FILE__, __LINE__ );
        }
        if( fromDates.size() == 0 || toDates.size() == 0 )
        {
            throw AQLCoreInvalidData( "a size of  vector is zero.", __FILE__, __LINE__ );
        }
        if ( fromDates.size() != toDates.size() )
        {
            throw AQLCoreInvalidData( "FromDate Arr. does not match ToDate Arr.", __FILE__, __LINE__ );
        }
        AQLString freq( etrading::getDefaultValueForEmptyString( frequency, AQLString( "SEMI-ANNUAL" ) ) );
        AQLString dayC( etrading::getDefaultValueForEmptyString( dayCount, AQLString( "ACT/365" ) ) );
        AQLString sRule( etrading::getDefaultValueForEmptyString( slidingRule, AQLString( "NO_CHANGE" ) ) );
        AQLString cal( etrading::getDefaultValueForEmptyString( calendar, AQLString( "TKB:LNB" ) ) );
        AQLString interp( etrading::getDefaultValueForEmptyString( interpolation, AQLString( "SPLINE" ) ) );
        AQLString curveNm( etrading::getDefaultValueForEmptyString( curveName, AQLString( "STD" ) ) );

        DoubleArray ret = etrading::AQLCurveForwardRateHelpers::getMultiForwardRate( fromDates, toDates, dataInstance, curveId, freq, dayC, sRule, cal, interp, curveName, isFwdInterp, useFwdData );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMirGetForwardRate2_outputs", curveId, curveNm ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

}

