#include "tryMirGetZeroRate.h"
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

namespace validation
{
    /* @brief			swig interface for mrGetZeroRate1
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		terms			A  list of given terms in the form of year fraction
    *  @param [in]		curveID			ID of the yield curve
    *  @param [in]		frequency		Frequency of the yield curve
    *  @param [in]		dayCount		Day count convention of the yield curve
    *  @param [in]		interpolation	Interpolation method of the yield curve
    *  @param [in]		curveName		Type of the yield curve, default to STD
    *  @param [in]		isFwdInterp		Boolean that decides if direct interpolation on fwd rates is employed
    *  @return			Zero rate over a given term from the curve's asof date
    */
    DoubleVector tryMirGetZeroRate1( AQLDataInstance* dataInstance,
                                     const DoubleVector& terms,
                                     const AQLString& curveid,
                                     const AQLString& frequency,
                                     const AQLString& dayCount,
                                     const AQLString& interpolation,
                                     const AQLString& curveName,
                                     bool isFwdInterp )

    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMirGetZeroRate1_inputs", curveid, curveName ) );
            file.write( "generatorFunction", "tryMirGetZeroRate1" );
            file.write( "terms", terms );
            file.write( "curveid", curveid );
            file.write( "frequency", frequency );
            file.write( "dayCount", dayCount );
            file.write( "interpolation", interpolation );
            file.write( "curveName", curveName );
            file.write( "isFwdInterp", isFwdInterp );
        }

        // Validate parameters
        if( isFwdInterp && AQLString( frequency ).toUpper() != "SIMPLE" )
        {
            throw AQLCoreInvalidData( "forward interpolation may not be used with Frequency other than 'SIMPLE'", __FILE__, __LINE__ );
        }

        if( terms.size() == 0 )
        {
            throw AQLCoreInvalidData( "a size of  vector is zero.", __FILE__, __LINE__ );
        }

        AQLString freq( etrading::getDefaultValueForEmptyString( frequency, AQLString( "SEMI-ANNUAL" ) ) );
        AQLString dayC( etrading::getDefaultValueForEmptyString( dayCount, AQLString( "ACT/365" ) ) );
        AQLString interp( etrading::getDefaultValueForEmptyString( interpolation, AQLString( "SPLINE" ) ) );
        AQLString curvename( etrading::getDefaultValueForEmptyString( curveName, AQLString( "STD" ) ) );

        DoubleArray arr = etrading::LACurveForwardRateHelpers::getMultiZeroRate( terms, dataInstance, curveid, freq, dayC, interp, curvename, isFwdInterp );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMirGetZeroRate1_outputs", curveid, curvename ) );
            file.write( "output", arr );
        }

        return arr;

        VALID_EXCEPTION_END
    }


    /* @brief			swig interface for mrGetZeroRate2
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		terms			A  list of given terms in the form of tenor string
    *  @param [in]		curveID			ID of the yield curve
    *  @param [in]		frequency		Frequency of the yield curve
    *  @param [in]		dayCount		Day count convention of the yield curve
    *  @param [in]		slidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		calendar		Calendar used to calculate zero dates
    *  @param [in]		interpolation	Interpolation method of the yield curve
    *  @param [in]		curveName		Type of the yield curve, default to STD
    *  @param [in]		isFwdInterp		Boolean that decides if direct interpolation on fwd rates is employed
    *  @return			Zero rate over a given term from the curve's asof date
    */
    DoubleVector tryMirGetZeroRate2( AQLDataInstance* dataInstance,
                                     const AQLStringVector& terms,
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
            CreateDataFile file( decorateCurvename( "tryMirGetZeroRate2_inputs", curveId, curveName ) );
            file.write( "generatorFunction", "tryMirGetZeroRate2" );
            file.write( "terms", terms );
            file.write( "curveId", curveId );
            file.write( "frequency", frequency );
            file.write( "dayCount", dayCount );
            file.write( "slidingRule", slidingRule );
            file.write( "calendar", calendar );
            file.write( "interpolation", interpolation );
            file.write( "curveName", curveName );
            file.write( "isFwdInterp", isFwdInterp );
        }

        // Validate parameters
        if( isFwdInterp && AQLString( frequency ).toUpper() != "SIMPLE" )
        {
            throw AQLCoreInvalidData( "forward interpolation may not be used with Frequency other than 'SIMPLE'", __FILE__, __LINE__ );
        }

        if( terms.size() == 0 )
        {
            throw AQLCoreInvalidData( "a size of  vector is zero.", __FILE__, __LINE__ );
        }

        AQLString freq( etrading::getDefaultValueForEmptyString( frequency, AQLString( "SEMI-ANNUAL" ) ) );
        AQLString dayC( etrading::getDefaultValueForEmptyString( dayCount, AQLString( "ACT/365" ) ) );
        AQLString interp( etrading::getDefaultValueForEmptyString( interpolation, AQLString( "SPLINE" ) ) );
        AQLString cal( etrading::getDefaultValueForEmptyString( calendar, AQLString( "TKB:LNB" ) ) );
        AQLString rollConv( etrading::getDefaultValueForEmptyString( slidingRule, AQLString( "NO_CHANGE" ) ) );
        AQLString curvename( etrading::getDefaultValueForEmptyString( curveName, AQLString( "STD" ) ) );

        DoubleArray arr = etrading::LACurveForwardRateHelpers::getMultiZeroRate( terms, dataInstance, curveId, freq, dayC, rollConv, cal, interp, curvename, isFwdInterp );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMirGetZeroRate2_outputs", curveId, curvename ) );
            file.write( "output", arr );
        }

        return arr;

        VALID_EXCEPTION_END
    }

}

