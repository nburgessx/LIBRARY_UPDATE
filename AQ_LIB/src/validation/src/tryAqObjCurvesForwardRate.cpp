#include "tryAqObjCurvesForwardRate.h"
#include "tryAqCurvesForwardRate.h" // needed for the utility functions
#include "tryAqDates.h"
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

    DoubleVector tryAqObjCurvesForwardRatesFromYearFraction( const std::string &                                 aqObjCurveName,
                                                            const std::vector< boost::gregorian::date > &       fromDates,
                                                            double                                              yearFraction,
                                                            const std::string &                                 dayCount )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryAqObjCurvesForwardRatesFromYearFraction_inputs_" ) + aqObjCurveName ).c_str() ) ;
            file.write( "generatorFunction",    "tryAqObjCurvesForwardRatesFromYearFraction" );
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
            CreateDataFile file( ( std::string( "tryAqObjCurvesForwardRatesFromYearFraction_outputs_" ) + aqObjCurveName ).c_str() );
            file.write( "output", forwardRateResults );
        }

        return forwardRateResults;

        VALID_EXCEPTION_END

    };


    DoubleVector tryAqObjCurvesForwardRatesFromForwardDates( const std::string& aqObjCurveName,
                                                            const std::vector<boost::gregorian::date>& fromDates,
                                                            const std::vector<boost::gregorian::date>& toDates )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryAqObjCurvesForwardRatesFromForwardDates_inputs_" ) + aqObjCurveName ).c_str() );
            file.write( "generatorFunction",    "tryAqObjCurvesForwardRatesFromForwardDates" );
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
            CreateDataFile file( ( std::string( "tryAqObjCurvesForwardRatesFromForwardDates_outputs_" ) + aqObjCurveName ).c_str() );
            file.write( "output", forwardRateResults );
        }

        return forwardRateResults;

        VALID_EXCEPTION_END
    };


    /* @brief			validation interface for aqObjCurvesForwardRates, which uses the curve frequency
    *  @param [in]		aqObjCurveName	AQObj Curve Name
    *  @param [in]		fixingDates	    A vector of fixing dates
    *  @return			The forward rates based on fromDates and toDates
    */
    DoubleVector tryAqObjCurvesForwardRates( const std::string& aqObjCurveName,
                                            const std::vector<boost::gregorian::date>& fixingDates )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryAqObjCurvesForwardRates_inputs_" ) + aqObjCurveName ).c_str() );
            file.write( "generatorFunction",     "tryAqObjCurvesForwardRates" );
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
            CreateDataFile file( ( std::string( "tryAqObjCurvesForwardRatesFromForwardDates_outputs_" ) + aqObjCurveName ).c_str() );
            file.write( "output", forwardRateResults );
        }

        return forwardRateResults;

        VALID_EXCEPTION_END
    };

}

