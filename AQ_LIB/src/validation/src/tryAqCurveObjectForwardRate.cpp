#include "tryAqCurveObjectForwardRate.h"
#include "tryAqCurveForwardRate.h" // needed for the utility functions
#include "tryAqDate.h"
#include "AQLCurveForwardRateHelpers.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "ParameterValidation.h"
#include "CurveValidation.h"
#include "EnvironmentUtilities.h"
#include "ContainerUtilities.h"
#include "CurveValidation.h"
#include "CurveUtilities.h"

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation
{

    DoubleVector tryAqCurveObjectForwardRatesFromYearFraction( const std::string &                                 aqObjCurveName,
                                                            const std::vector< boost::gregorian::date > &       fromDates,
                                                            double                                              yearFraction,
                                                            const std::string &                                 dayCount )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryAqCurveObjectForwardRatesFromYearFraction_inputs_" ) + aqObjCurveName ).c_str() ) ;
            file.write( "generatorFunction",    "tryAqCurveObjectForwardRatesFromYearFraction" );
            file.write( "aqObjCurveName",	        aqObjCurveName );
            file.write( "fromDates",			fromDates );
            file.write( "yearFraction",		    yearFraction );
            file.write( "dayCount",			    dayCount );
        }

        DoubleVector forwardRateResults;

        // Load the AQObj Curve
        auto& curveStore = etrading::getObjectStore<etrading::AQObjCurve>( etrading::Environment::DEFAULT_ENV_NAME );
        if( curveStore.has( aqObjCurveName ) )
        {
            auto aqObjCurve = curveStore.get( aqObjCurveName );
            if( aqObjCurve )
            {
                // Get the Curve Collection and Curve Index names
                AQLString curveCollection    = AQLString( aqObjCurve->getCurveBuildStaticDataObject()->curveCollectionName_.c_str() );
                AQLString curveIndex         = AQLString( aqObjCurve->getCurveBuildStaticDataObject()->curveIndexName_.c_str() );

                // Cast the boost gregorian dates into AQLDates
                std::vector< AQLDate > fromDatesFormatted = etrading::toAQLDatesFromGregorianDates( fromDates );
        
                // Calculate the forward rate
                forwardRateResults = etrading::getCurveForwardRatesFromYearFraction( fromDatesFormatted,
                                                                                     yearFraction,
                                                                                     AQLString( dayCount.c_str() ),
                                                                                     curveCollection,
                                                                                     curveIndex );
            }
            else
            {
                std::string errString = ( boost::format( "#Error: Curve  %s does not exist" ) % aqObjCurveName.c_str() ).str();
                throw AQLCoreInvalidData( errString.c_str(), __FILE__, __LINE__ );
            }
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryAqCurveObjectForwardRatesFromYearFraction_outputs_" ) + aqObjCurveName ).c_str() );
            file.write( "output", forwardRateResults );
        }

        return forwardRateResults;

        VALID_EXCEPTION_END

    };


    DoubleVector tryAqCurveObjectForwardRatesFromForwardDates( const std::string& aqObjCurveName,
                                                            const std::vector<boost::gregorian::date>& fromDates,
                                                            const std::vector<boost::gregorian::date>& toDates )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryAqCurveObjectForwardRatesFromForwardDates_inputs_" ) + aqObjCurveName ).c_str() );
            file.write( "generatorFunction",    "tryAqCurveObjectForwardRatesFromForwardDates" );
            file.write( "aqObjCurveName",	        aqObjCurveName );
            file.write( "fromDates",			fromDates );
            file.write( "toDates",			    toDates );
        }

        DoubleVector forwardRateResults;

        // Load the AQObj Curve
        auto& curveStore = etrading::getObjectStore<etrading::AQObjCurve>( etrading::Environment::DEFAULT_ENV_NAME );
        if( curveStore.has( aqObjCurveName ) )
        {
            auto aqObjCurve = curveStore.get( aqObjCurveName );
            if( aqObjCurve )
            {
                // Get the Curve Collection and Curve Index names
                AQLString curveCollection    = AQLString( aqObjCurve->getCurveBuildStaticDataObject()->curveCollectionName_.c_str() );
                AQLString curveIndex         = AQLString( aqObjCurve->getCurveBuildStaticDataObject()->curveIndexName_.c_str() );

                // Cast the boost gregorian dates into AQLDates
                std::vector< AQLDate > fromDatesFormatted    = etrading::toAQLDatesFromGregorianDates( fromDates );
                std::vector< AQLDate > toDatesFormatted      = etrading::toAQLDatesFromGregorianDates( toDates );
        
                // Calculate the forward rate
                forwardRateResults = etrading::getCurveForwardRatesFromForwardDates( fromDatesFormatted,
                                                                                     toDatesFormatted,
                                                                                     curveCollection,
                                                                                     curveIndex );
            }
            else
            {
                std::string errString = ( boost::format( "#Error: Curve  %s does not exist" ) % aqObjCurveName.c_str() ).str();
                throw AQLCoreInvalidData( errString.c_str(), __FILE__, __LINE__ );
            }
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryAqCurveObjectForwardRatesFromForwardDates_outputs_" ) + aqObjCurveName ).c_str() );
            file.write( "output", forwardRateResults );
        }

        return forwardRateResults;

        VALID_EXCEPTION_END
    };


    /* @brief			validation interface for aqCurveObjectForwardRates, which uses the curve frequency
    *  @param [in]		aqObjCurveName	AQObj Curve Name
    *  @param [in]		fixingDates	    A vector of fixing dates
    *  @return			The forward rates based on fromDates and toDates
    */
    DoubleVector tryAqCurveObjectForwardRates( const std::string& aqObjCurveName,
                                            const std::vector<boost::gregorian::date>& fixingDates )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryAqCurveObjectForwardRates_inputs_" ) + aqObjCurveName ).c_str() );
            file.write( "generatorFunction",     "tryAqCurveObjectForwardRates" );
            file.write( "aqObjCurveName",	        aqObjCurveName );
            file.write( "fixingDates",			fixingDates );
        }

         DoubleVector forwardRateResults;

        // Load the AQObj Curve
        auto& curveStore = etrading::getObjectStore<etrading::AQObjCurve>( etrading::Environment::DEFAULT_ENV_NAME );
        if( curveStore.has( aqObjCurveName ) )
        {
            auto aqObjCurve = curveStore.get( aqObjCurveName );
            if( aqObjCurve )
            {
                // Get the Curve Collection and Curve Index names
                AQLString curveCollection    = AQLString( aqObjCurve->getCurveBuildStaticDataObject()->curveCollectionName_.c_str() );
                AQLString curveIndex         = AQLString( aqObjCurve->getCurveBuildStaticDataObject()->curveIndexName_.c_str() );

                // Cast the boost gregorian dates into AQLDates
                std::vector< AQLDate > fixingDatesFormatted    = etrading::toAQLDatesFromGregorianDates( fixingDates );
                
                // Calculate the forward rates
                forwardRateResults = etrading::getCurveForwardRates( fixingDatesFormatted, curveCollection, curveIndex );
            }
            else
            {
                std::string errString = ( boost::format( "#Error: Curve  %s does not exist" ) % aqObjCurveName.c_str() ).str();
                throw AQLCoreInvalidData( errString.c_str(), __FILE__, __LINE__ );
            }
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryAqCurveObjectForwardRatesFromForwardDates_outputs_" ) + aqObjCurveName ).c_str() );
            file.write( "output", forwardRateResults );
        }

        return forwardRateResults;

        VALID_EXCEPTION_END
    };

}

