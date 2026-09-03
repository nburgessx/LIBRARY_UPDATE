#include "tryMeLWOCurveForwardRate.h"
#include "tryMeCurveForwardRate.h" // needed for the utility functions
#include "tryMeDate.h"
#include "LACurveForwardRateHelpers.h"
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

    DoubleVector tryMeLWOCurveForwardRatesFromYearFraction( const std::string &                                 lwoCurveName,
                                                            const std::vector< boost::gregorian::date > &       fromDates,
                                                            double                                              yearFraction,
                                                            const std::string &                                 dayCount )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryMeLWOCurveForwardRatesFromYearFraction_inputs_" ) + lwoCurveName ).c_str() ) ;
            file.write( "generatorFunction",    "tryMeLWOCurveForwardRatesFromYearFraction" );
            file.write( "lwoCurveName",	        lwoCurveName );
            file.write( "fromDates",			fromDates );
            file.write( "yearFraction",		    yearFraction );
            file.write( "dayCount",			    dayCount );
        }

        DoubleVector forwardRateResults;

        // Load the LWO Curve
        auto& curveStore = etrading::getObjectStore<etrading::LWOCurve>( etrading::Environment::DEFAULT_ENV_NAME );
        if( curveStore.has( lwoCurveName ) )
        {
            auto lwoCurve = curveStore.get( lwoCurveName );
            if( lwoCurve )
            {
                // Get the Curve Collection and Curve Index names
                AQLString curveCollection    = AQLString( lwoCurve->getCurveBuildStaticDataObject()->curveCollectionName_.c_str() );
                AQLString curveIndex         = AQLString( lwoCurve->getCurveBuildStaticDataObject()->curveIndexName_.c_str() );

                // Cast the boost gregorian dates into LADates
                std::vector< AQLDate > fromDatesFormatted = etrading::toLADatesFromGregorianDates( fromDates );
        
                // Calculate the forward rate
                forwardRateResults = etrading::getCurveForwardRatesFromYearFraction( fromDatesFormatted,
                                                                                     yearFraction,
                                                                                     AQLString( dayCount.c_str() ),
                                                                                     curveCollection,
                                                                                     curveIndex );
            }
            else
            {
                std::string errString = ( boost::format( "#Error: Curve  %s does not exist" ) % lwoCurveName.c_str() ).str();
                throw AQLCoreInvalidData( errString.c_str(), __FILE__, __LINE__ );
            }
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryMeLWOCurveForwardRatesFromYearFraction_outputs_" ) + lwoCurveName ).c_str() );
            file.write( "output", forwardRateResults );
        }

        return forwardRateResults;

        VALID_EXCEPTION_END

    };


    DoubleVector tryMeLWOCurveForwardRatesFromForwardDates( const std::string& lwoCurveName,
                                                            const std::vector<boost::gregorian::date>& fromDates,
                                                            const std::vector<boost::gregorian::date>& toDates )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryMeLWOCurveForwardRatesFromForwardDates_inputs_" ) + lwoCurveName ).c_str() );
            file.write( "generatorFunction",    "tryMeLWOCurveForwardRatesFromForwardDates" );
            file.write( "lwoCurveName",	        lwoCurveName );
            file.write( "fromDates",			fromDates );
            file.write( "toDates",			    toDates );
        }

        DoubleVector forwardRateResults;

        // Load the LWO Curve
        auto& curveStore = etrading::getObjectStore<etrading::LWOCurve>( etrading::Environment::DEFAULT_ENV_NAME );
        if( curveStore.has( lwoCurveName ) )
        {
            auto lwoCurve = curveStore.get( lwoCurveName );
            if( lwoCurve )
            {
                // Get the Curve Collection and Curve Index names
                AQLString curveCollection    = AQLString( lwoCurve->getCurveBuildStaticDataObject()->curveCollectionName_.c_str() );
                AQLString curveIndex         = AQLString( lwoCurve->getCurveBuildStaticDataObject()->curveIndexName_.c_str() );

                // Cast the boost gregorian dates into LADates
                std::vector< AQLDate > fromDatesFormatted    = etrading::toLADatesFromGregorianDates( fromDates );
                std::vector< AQLDate > toDatesFormatted      = etrading::toLADatesFromGregorianDates( toDates );
        
                // Calculate the forward rate
                forwardRateResults = etrading::getCurveForwardRatesFromForwardDates( fromDatesFormatted,
                                                                                     toDatesFormatted,
                                                                                     curveCollection,
                                                                                     curveIndex );
            }
            else
            {
                std::string errString = ( boost::format( "#Error: Curve  %s does not exist" ) % lwoCurveName.c_str() ).str();
                throw AQLCoreInvalidData( errString.c_str(), __FILE__, __LINE__ );
            }
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryMeLWOCurveForwardRatesFromForwardDates_outputs_" ) + lwoCurveName ).c_str() );
            file.write( "output", forwardRateResults );
        }

        return forwardRateResults;

        VALID_EXCEPTION_END
    };


    /* @brief			validation interface for meLWOCurveForwardRates, which uses the curve frequency
    *  @param [in]		lwoCurveName	LWO Curve Name
    *  @param [in]		fixingDates	    A vector of fixing dates
    *  @return			The forward rates based on fromDates and toDates
    */
    DoubleVector tryMeLWOCurveForwardRates( const std::string& lwoCurveName,
                                            const std::vector<boost::gregorian::date>& fixingDates )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryMeLWOCurveForwardRates_inputs_" ) + lwoCurveName ).c_str() );
            file.write( "generatorFunction",     "tryMeLWOCurveForwardRates" );
            file.write( "lwoCurveName",	        lwoCurveName );
            file.write( "fixingDates",			fixingDates );
        }

         DoubleVector forwardRateResults;

        // Load the LWO Curve
        auto& curveStore = etrading::getObjectStore<etrading::LWOCurve>( etrading::Environment::DEFAULT_ENV_NAME );
        if( curveStore.has( lwoCurveName ) )
        {
            auto lwoCurve = curveStore.get( lwoCurveName );
            if( lwoCurve )
            {
                // Get the Curve Collection and Curve Index names
                AQLString curveCollection    = AQLString( lwoCurve->getCurveBuildStaticDataObject()->curveCollectionName_.c_str() );
                AQLString curveIndex         = AQLString( lwoCurve->getCurveBuildStaticDataObject()->curveIndexName_.c_str() );

                // Cast the boost gregorian dates into LADates
                std::vector< AQLDate > fixingDatesFormatted    = etrading::toLADatesFromGregorianDates( fixingDates );
                
                // Calculate the forward rates
                forwardRateResults = etrading::getCurveForwardRates( fixingDatesFormatted, curveCollection, curveIndex );
            }
            else
            {
                std::string errString = ( boost::format( "#Error: Curve  %s does not exist" ) % lwoCurveName.c_str() ).str();
                throw AQLCoreInvalidData( errString.c_str(), __FILE__, __LINE__ );
            }
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryMeLWOCurveForwardRatesFromForwardDates_outputs_" ) + lwoCurveName ).c_str() );
            file.write( "output", forwardRateResults );
        }

        return forwardRateResults;

        VALID_EXCEPTION_END
    };

}

