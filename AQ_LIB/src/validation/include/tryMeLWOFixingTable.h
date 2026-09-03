// tryMeLWOFixingTable.h

#pragma once

#include <boost/date_time.hpp>
#include "CoreEnumerations.h"
#include "Variant.h"
#include "LabelValueBlock.h"

namespace validation
{
    /* @brief			function to create a fixing table object. This function specific to interest rate fixings
	*
	*  @param [in]		tableName		    Fixing Table name
	*  @param [in]		currencyString	    The currency corresponding to the interest rate fixings
	*  @param [in]		curveTenorString	The curve tenor corresponding to the interest rate fixings
	*  @param [in]		fixingDates		    The fixing dates to store
	*  @param [in]		fixingValues	    The fixing values to store
	*  @return			returns the name of the fixing table in the cache
	*/
    std::string tryMeLWOFixingTableCreate( const std::string& tableName,
                                           const std::string& currencyString,                                   
                                           const std::string& curveTenorString,
                                           const std::vector< boost::gregorian::date >& fixingDates,
                                           const std::vector< double >& fixingValues );

	/* @brief			function to create a fixing table object. This function can create
	*					a fixing table of any type. e.g. INTERESTRATE, INFLATION, FX
	*
	*  @param [in]		tableName		    Fixing Table name
	*  @param [in]		parameterLVB	    A LabelValueBlock specifying the type of fixing table and various additional parameters
	*  @param [in]		fixingDates		    The fixing dates to store
	*  @param [in]		fixingValues	    The fixing values to store
	*  @return			returns the name of the fixing table in the cache
	*/
	std::string tryMeLWOFixingTableCreate( const std::string& tableName,
										   const etrading::LabelValueBlock& parameterLVB,
										   const std::vector< boost::gregorian::date >& fixingDates,
										   const std::vector< double >& fixingValues );

    /* @brief			function to display a fixing table object
	*  @param [in]		tableName		    Fixing Table name
	*  @return			returns a VariantMatrix representing the fixing currency, curveTenor, fixingDates and fixingValues
	*/
    etrading::VariantMatrix tryMeLWOFixingTableDisplay( const std::string& tableName );

    /* @brief			function to get the fixing value for a particular date
	*  @param [in]		tableName		    Fixing Table name
	*  @param [in]		fixingDate	        Fixing Date
	*  @return			swapName
	*/
    double tryMeLWOFixingTableValue( const std::string& tableName, const boost::gregorian::date fixingDate );

    /* @brief			function to get fixing values for a vector of dates
	*					NOTE: This function does not perform any validation of inputs
	*
	*  @param [in]		tableName		    Fixing Table name
    *  @param [in]		currency		    Currency
    *  @param [in]		curveTenor		    Curve Tenor
	*  @param [in]		fixingDates     	Fixing Dates
	*  @return			A vector of fixing values
	*/
    std::vector< double > tryMeLWOFixingTableValues( const std::string& tableName, const std::vector< boost::gregorian::date >& fixingDates );


	/* @brief			function to get fixing values for a vector of dates and validate the fixing table currency and curve Tenor
	*					NOTE: This function is specific to interest rate fixings
	*
	*  @param [in]		tableName		    Fixing Table name
	*  @param [in]		currency		    Currency.    Used for validation.
	*  @param [in]		curveTenor		    Curve Tenor. Used for validation.
	*  @param [in]		fixingDates     	Fixing Dates
	*  @return			A vector of fixing values
	*/
    std::vector< double > tryMeLWOFixingTableValues( const std::string& tableName, const std::string& currency, const std::string& curveTenor, const std::vector< boost::gregorian::date >& fixingDates );

	/* @brief			function to get fixing values for a vector of dates and validate the fixing table currency and curve Tenor
	*					NOTE: This function works with all fixing table types. e.g. INTERESTRATE, INFLATION, FX
	*
	*  @param [in]		tableName		    Fixing Table name
	*  @param [in]		parameterLVB	    A LabelValueBlock specifying the type of fixing table and other fixing-specific parameters
	*										Used for validation: The parameters in this block must match those held inside the fixing table.
	*  @param [in]		fixingDates     	Fixing Dates
	*  @return			A vector of fixing values
	*/
	std::vector< double > tryMeLWOFixingTableValues( const std::string& tableName, const etrading::LabelValueBlock& parameterLVB, const std::vector< boost::gregorian::date >& fixingDates );

}