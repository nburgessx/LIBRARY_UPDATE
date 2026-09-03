// tryMeLWOFixingTable.cpp

#include "tryMeLWOFixingTable.h"
#include "FixingTableSet.h"
#include "ObjectUtilities.h"
#include "Environment.h"
#include "ContainerUtilities.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "RecordMacros.h"

using etrading::CreateDataFile;
using etrading::decorateFilename;

namespace validation
{
	/* @brief			function to create a fixing table object. This function specific to interest rate fixings
	*
	*  @param [in]		tableName		    Fixing Table name
	*  @param [in]		currencyString	    The currency corresponding to the interest rate fixings
	*  @param [in]		curveTenorString	The curve tenor corresponding to the interest rate fixings
	*  @param [in]		fixingDates		    The fixing dates to store
	*  @param [in]		fixingValues	    The fixing values to store
	*  @return			returns the name of the fixing table on the cache
	*/
    std::string tryMeLWOFixingTableCreate( const std::string& tableName,
                                           const std::string& currencyString,                                   
                                           const std::string& curveTenorString,
                                           const std::vector< boost::gregorian::date >& fixingDates,
                                           const std::vector< double >& fixingValues )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
		if (etrading::CreateDataFile::recordEnabled()) 
		{
			CreateDataFile file(decorateFilename("tryMeLWOFixingTableCreate_inputs", tableName.c_str()));
			file.write("generatorFunction", "tryMeLWOFixingTableCreate");
			file.write("tableName", tableName);
			file.write("currency", currencyString);
			file.write("curveTenor", curveTenorString);
			file.write("fixingDates", fixingDates);
            file.write("fixingValues", fixingValues);
		}
        
        // Cast Curve and CurveTenor parameters to their respective Enumerators
        auto currency = etrading::toCCYEnum( currencyString );
        auto curveTenor = etrading::toCurveTenorEnum( curveTenorString );

        // Create a fixing table object
        etrading::FixingTable fixingTable( tableName, fixingDates, fixingValues, curveTenor, currency );
        
        // Move the fixing table instance to the cache
        etrading::moveToCache< etrading::FixingTable >( std::move( fixingTable ) );

        // Check Object exists on the cache
        auto ptrCachedObject = etrading::Environment::defaultEnv().accessObject<etrading::FixingTable>( tableName );

        // Return the Object Name if it has been created successfuly on the cache
        if ( ptrCachedObject != nullptr )
        {
            if ( CreateDataFile::recordEnabled() )
            {
				CreateDataFile file(decorateFilename("tryMeLWOFixingTableCreate_outputs", tableName.c_str()));
				file.write("output", tableName);
            }
            return tableName.c_str();
        }
        else
        {
            std::string errString =  ( boost::format( "Unable to create LWO Fixing Table named %s" ) % tableName.c_str() ).str();
            if ( CreateDataFile::recordEnabled() )
            {
				CreateDataFile file( decorateFilename( "tryMeLWOFixingTableCreate_outputs", tableName.c_str() ) );
                file.write( "output", errString.c_str() );
            }
            AQ_THROW( errString );
        }
        
        VALID_EXCEPTION_END
    }


	/* @brief			function to create a fixing table object. This function can create
	*					a fixing table of any type. e.g. INTERESTRATE, INFLATION, FX
	*
	*  @param [in]		tableName		    Fixing Table name
	*  @param [in]		parameterLVB	    A LabelValueBlock specifying the type of fixing table and various additional parameters
	*  @param [in]		fixingDates		    The fixing dates to store
	*  @param [in]		fixingValues	    The fixing values to store
	*  @return			returns the name of the fixing table on the cache
	*/
	std::string tryMeLWOFixingTableCreate( const std::string& tableName,
										   const etrading::LabelValueBlock& parameterLVB,
										   const std::vector< boost::gregorian::date >& fixingDates,
										   const std::vector< double >& fixingValues )
	{
		VALID_EXCEPTION_START;

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(tableName, std::string(), tableName, parameterLVB, fixingDates, fixingValues );

		// Create a fixing table object
		etrading::FixingTable fixingTable(tableName, fixingDates, fixingValues, parameterLVB );

		// Move the fixing table instance to the cache
		etrading::moveToCache< etrading::FixingTable >(std::move( fixingTable ));

		// Check Object exists on the cache
		auto ptrCachedObject = etrading::Environment::defaultEnv().accessObject<etrading::FixingTable>( tableName );
		AQ_REQUIRE( ptrCachedObject != nullptr, "Unable to store LWO Fixing Table named " + tableName );

		std::string result = tableName;

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(tableName, std::string(), result);

		VALID_EXCEPTION_END
	}


    etrading::VariantMatrix tryMeLWOFixingTableDisplay( const std::string& tableName )
    {
         VALID_EXCEPTION_START

        // Recording of inputs for playback
		if (etrading::CreateDataFile::recordEnabled()) 
		{
			CreateDataFile file(decorateFilename("tryMeLWOFixingTableDisplay_inputs", tableName.c_str()));
			file.write("generatorFunction", "tryMeLWOFixingTableDisplay");
			file.write("tableName", tableName);
		}
        
        // Check Object exists on the cache
        auto ptrCachedObject = etrading::Environment::defaultEnv().accessObject<etrading::FixingTable>( tableName );
        
        // Return the Object Name if it has been created successfuly on the cache
        if ( ptrCachedObject != nullptr )
        {
			
			auto fixingTableMatrix = ptrCachedObject->getVariantMatrix();

			if ( CreateDataFile::recordEnabled() )
            {
		        CreateDataFile file( decorateFilename("tryMeLWOFixingTableDisplay_outputs", tableName.c_str() ) );
				
                // Note Need to know Schema to get display values
                file.write( "currency", fixingTableMatrix[1][0] );
                file.write( "curveTenor", fixingTableMatrix[1][1] );
                
				std::vector<Variant> fixingDates;
				std::vector<Variant> fixingValues;
				ptrCachedObject->populateFixingTableDatesValues(fixingDates, fixingValues);
				
				file.write( "fixingValues" , fixingDates );
                file.write( "fixingDates", fixingValues );
            }
            return fixingTableMatrix;
        }
        else
        {
            std::string errString =  ( boost::format( "Fixing table %s does not exist" ) % tableName.c_str() ).str();
            if ( CreateDataFile::recordEnabled() )
            {
				CreateDataFile file( decorateFilename( "tryMeLWOFixingTableDisplaye_outputs", tableName.c_str() ) );
                file.write( "output", errString.c_str() );
            }
            AQ_THROW( errString );
        }
        
        VALID_EXCEPTION_END
    }


    double tryMeLWOFixingTableValue( const std::string& tableName, const boost::gregorian::date fixingDate )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
		if (etrading::CreateDataFile::recordEnabled()) 
		{
			CreateDataFile file(decorateFilename("tryMeLWOFixingTableValue_inputs", tableName.c_str() ) );
			file.write("generatorFunction", "tryMeLWOFixingTableValue");
			file.write("tableName", tableName);
            file.write("fixingDate", fixingDate);
		}
        
        // Check Object exists on the cache
        auto ptrCachedObject = etrading::Environment::defaultEnv().accessObject<etrading::FixingTable>( tableName );
         
        // Return the Object Name if it has been created successfuly on the cache
        if ( ptrCachedObject != nullptr )
        {

            auto fixingValue = ptrCachedObject->getFixingValue( fixingDate );

            if ( CreateDataFile::recordEnabled() )
            {
				CreateDataFile file( decorateFilename("tryMeLWOFixingTableValue_outputs", tableName.c_str() ) );
                file.write( "output", fixingValue );
            }
            return fixingValue;
        }
        else
        {
            throw etrading::ETradingException( ( boost::format( "#Error: Fixing Table %s does not exist" ) % tableName ).str().c_str() );
        }
        
        VALID_EXCEPTION_END
    }

	/* @brief			function to get fixing values for a vector of dates
	*					NOTE: This function does not perform any validation of inputs
	*
	*  @param [in]		tableName		    Fixing Table name
	*  @param [in]		currency		    Currency
	*  @param [in]		curveTenor		    Curve Tenor
	*  @param [in]		fixingDates     	Fixing Dates
	*  @return			A vector of fixing values
	*/
    std::vector< double > tryMeLWOFixingTableValues( const std::string& tableName, const std::vector< boost::gregorian::date >& fixingDates )
    {
        VALID_EXCEPTION_START
        
        RECORD_INPUTS ( tableName, fixingDates )
        
        // Create Pointer to the Fixing Table Object in the Cache and throw if a null pointer
        auto fixingTable = etrading::Environment::defaultEnv().accessObject<etrading::FixingTable>( tableName );
        AQ_REQUIRE( fixingTable != nullptr, "Fixing Table " + tableName + " does not exist" )

        // Generate a Vector of Fixing Values
        DoubleVector fixingValues( fixingDates.size(), 0.0 );
        for ( size_t i = 0; i < fixingDates.size(); ++i )
        {
            fixingValues[i] = fixingTable->getFixingValue( fixingDates[i] );
        }

        RECORD_OUTPUTS_AND_RETURN_RESULT( fixingValues )

        VALID_EXCEPTION_END
    }

	/* @brief			function to get fixing values for a vector of dates and validate the fixing table currency and curve Tenor
	*					NOTE: This function is specific to interest rate fixings
	*
	*  @param [in]		tableName		    Fixing Table name
	*  @param [in]		currency		    Currency.    Used for validation.
	*  @param [in]		curveTenor		    Curve Tenor. Used for validation.
	*  @param [in]		fixingDates     	Fixing Dates
	*  @return			A vector of fixing values
	*/
    std::vector< double > tryMeLWOFixingTableValues( const std::string& tableName, const std::string& currency, const std::string& curveTenor, const std::vector< boost::gregorian::date >& fixingDates )
    {
        VALID_EXCEPTION_START

        std::string fixingTableCurrency     = etrading::trim_to_upper( currency.c_str() );
        std::string fixingTableCurveTenor   = etrading::trim_to_upper( curveTenor.c_str() );

        // Recording of inputs for playback
		if (etrading::CreateDataFile::recordEnabled()) 
		{
			CreateDataFile file(decorateFilename("tryMeLWOFixingTableValues_inputs", tableName.c_str() ) );
			file.write("generatorFunction", "tryMeLWOFixingTableValues");
			file.write("tableName", tableName);
            file.write("currency", fixingTableCurrency);
            file.write("curveTenor", fixingTableCurveTenor);
            file.write("fixingDates", fixingDates);
		}
        
        // Check Object exists on the cache
        auto ptrCachedObject = etrading::Environment::defaultEnv().accessObject<etrading::FixingTable>( tableName );
         
        // Return the Object Name if it has been created successfuly on the cache
        if ( ptrCachedObject != nullptr )
        {

            auto fixingValues = ptrCachedObject->getFixingValues( fixingTableCurrency, fixingTableCurveTenor, fixingDates );

            if ( CreateDataFile::recordEnabled() )
            {
				CreateDataFile file( decorateFilename("tryMeLWOFixingTableValues_outputs", tableName.c_str() ) );
                file.write( "output", fixingValues );
            }
            return fixingValues;
        }
        else
        {
            throw etrading::ETradingException( ( boost::format( "#Error: Fixing Table %s does not exist" ) % tableName ).str().c_str() );
        }

        VALID_EXCEPTION_END
    }


	/* @brief			function to get fixing values for a vector of dates and validate the fixing table currency and curve Tenor
	*					NOTE: This function works with all fixing table types. e.g. INTERESTRATE, INFLATION, FX
	*
	*  @param [in]		tableName		    Fixing Table name
	*  @param [in]		parameterLVB	    A LabelValueBlock specifying the type of fixing table and other fixing-specific parameters
	*										Used for validation: The parameters in this block must match those held inside the fixing table.
	*  @param [in]		fixingDates     	Fixing Dates
	*  @return			A vector of fixing values
	*/
	std::vector< double > tryMeLWOFixingTableValues(const std::string& tableName, const etrading::LabelValueBlock& parameterLVB, const std::vector< boost::gregorian::date >& fixingDates)
	{
		VALID_EXCEPTION_START;

		RECORD_INPUTS(tableName, fixingDates)

		// Create Pointer to the Fixing Table Object in the Cache and throw if a null pointer
		auto fixingTable = etrading::Environment::defaultEnv().accessObject<etrading::FixingTable>( tableName );
		AQ_REQUIRE(fixingTable != nullptr, "Fixing Table " + tableName + " does not exist");

		// Generate a Vector of Fixing Values
		auto fixingValues = fixingTable->getFixingValues( parameterLVB, fixingDates );

		RECORD_OUTPUTS_AND_RETURN_RESULT( fixingValues );

		VALID_EXCEPTION_END
	}


}