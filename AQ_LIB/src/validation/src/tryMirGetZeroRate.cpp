/*
 * @brief			validation interface for mirGetZeroRate method(s)
 * @Created:		01 April 2016
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

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

namespace validation_api
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
    DoubleVector tryMirGetZeroRate1( LADataInstance* dataInstance,
                                     const DoubleVector& terms,
                                     const LAString& curveid,
                                     const LAString& frequency,
                                     const LAString& dayCount,
                                     const LAString& interpolation,
                                     const LAString& curveName,
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
        if( isFwdInterp && LAString( frequency ).toUpper() != "SIMPLE" )
        {
            throw LACoreInvalidData( "forward interpolation may not be used with Frequency other than 'SIMPLE'", __FILE__, __LINE__ );
        }

        if( terms.size() == 0 )
        {
            throw LACoreInvalidData( "a size of  vector is zero.", __FILE__, __LINE__ );
        }

        LAString freq( etrading::getDefaultValueForEmptyString( frequency, LAString( "SEMI-ANNUAL" ) ) );
        LAString dayC( etrading::getDefaultValueForEmptyString( dayCount, LAString( "ACT/365" ) ) );
        LAString interp( etrading::getDefaultValueForEmptyString( interpolation, LAString( "SPLINE" ) ) );
        LAString curvename( etrading::getDefaultValueForEmptyString( curveName, LAString( "STD" ) ) );

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
    DoubleVector tryMirGetZeroRate2( LADataInstance* dataInstance,
                                     const LAStringVector& terms,
                                     const LAString& curveId,
                                     const LAString& frequency,
                                     const LAString& dayCount,
                                     const LAString& slidingRule,
                                     const LAString& calendar,
                                     const LAString& interpolation,
                                     const LAString& curveName,
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
        if( isFwdInterp && LAString( frequency ).toUpper() != "SIMPLE" )
        {
            throw LACoreInvalidData( "forward interpolation may not be used with Frequency other than 'SIMPLE'", __FILE__, __LINE__ );
        }

        if( terms.size() == 0 )
        {
            throw LACoreInvalidData( "a size of  vector is zero.", __FILE__, __LINE__ );
        }

        LAString freq( etrading::getDefaultValueForEmptyString( frequency, LAString( "SEMI-ANNUAL" ) ) );
        LAString dayC( etrading::getDefaultValueForEmptyString( dayCount, LAString( "ACT/365" ) ) );
        LAString interp( etrading::getDefaultValueForEmptyString( interpolation, LAString( "SPLINE" ) ) );
        LAString cal( etrading::getDefaultValueForEmptyString( calendar, LAString( "TKB:LNB" ) ) );
        LAString rollConv( etrading::getDefaultValueForEmptyString( slidingRule, LAString( "NO_CHANGE" ) ) );
        LAString curvename( etrading::getDefaultValueForEmptyString( curveName, LAString( "STD" ) ) );

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

