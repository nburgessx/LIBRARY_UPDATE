// meLWOSwapPrice.cpp

/* 
 * @brief			Swig interface for meLWOSwapPrice... functions
 * @Created:		25th June 2018
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "meLWOSwapPrice.h"
#include "LACoreTemplateType.h"
#include "TypeUtilities.h"
#include "tryMeLWOSwapPricing.h"
#include "APISetUp.h"					// MLIB_API_START and MLIB_API_END Macros

/* @brief			Function to calculate the Swap PV
*  @param [in]		swapName			Swap object name
*  @param [in]		curveCollection		Valuation settings
*  @param [in]		legName				Leg name
*  @param [in]		fixingTableNames	Fixing table object names
*  @return			Swap PV
*/
double meLWOSwapPV( const std::string& swapName,
                    const SWIG_STRINGMATRIX & valuationSettings,
                    const std::string& legName,
                    const SWIG_STRINGMATRIX & fixingTableNames )
{
    MLIB_API_START
    
    // Marshall Inputs
    LAString legNameAsLAString( legName.c_str() );
    
    LAStringMatrix valuationSettingsAsStringMatrix;
    swig::buildStringMatrix( valuationSettingsAsStringMatrix, valuationSettings );

    LAStringMatrix fixingTableAsStringMatrix;
    swig::buildStringMatrix( fixingTableAsStringMatrix, fixingTableNames );

    // Call Function and Return Result
    double result = validation_api::tryMeLWOSwapPV( swapName,
                                                    valuationSettingsAsStringMatrix,
                                                    legNameAsLAString,
                                                    fixingTableAsStringMatrix );
    return result;
    
    MLIB_API_END
	
}

/* @brief			Function to calculate the Swap PV01
*  @param [in]		swapName			Swap object name
*  @param [in]		curveCollection		Valuation settings
*  @param [in]		fixingTableNames	Fixing table object names
*  @return			Swap PV01
*/
double meLWOSwapPV01( const std::string& swapName,
                      const SWIG_STRINGMATRIX & valuationSettings,
                      const SWIG_STRINGMATRIX & fixingTableNames )
{
    MLIB_API_START
    
    // Marshall Inputs
    LAStringMatrix valuationSettingsAsStringMatrix;
    swig::buildStringMatrix( valuationSettingsAsStringMatrix, valuationSettings );

    LAStringMatrix fixingTableAsStringMatrix;
    swig::buildStringMatrix( fixingTableAsStringMatrix, fixingTableNames );

    // Call Function and Return Result
    double result = validation_api::tryMeLWOSwapPV01( swapName,
                                                        valuationSettingsAsStringMatrix,
                                                        fixingTableAsStringMatrix );
    return result;
    
    MLIB_API_END
}

/* @brief			Function to calculate the Swap Par Rate
*  @param [in]		swapName		 Swap object name
*  @param [in]		valuationSettings  Valuation settings map
*  @param [in]		fixingTableNames Fixing table object names
*  @return			Swap par rate
*/
double meLWOSwapParRate( const std::string& swapName,
                         const SWIG_STRINGMATRIX & valuationSettings,
                         const SWIG_STRINGMATRIX & fixingTableNames )
{
    MLIB_API_START    	
    
    // Marshall Inputs
    LAStringMatrix valuationSettingsAsStringMatrix;
    swig::buildStringMatrix( valuationSettingsAsStringMatrix, valuationSettings );

    LAStringMatrix fixingTableAsStringMatrix;
    swig::buildStringMatrix( fixingTableAsStringMatrix, fixingTableNames );

    // Call Function and Return Result
    double result = validation_api::tryMeLWOSwapParRate( swapName,
                                                            valuationSettingsAsStringMatrix,
                                                            fixingTableAsStringMatrix );
    return result;
    
    MLIB_API_END
}

/* @brief			Function to calculate the Swap Par Rate
*  @param [in]		swapName		 Swap object name
*  @param [in]		valuationSettings  Valuation settings map
*  @param [in]		fixingTableNames Fixing table object names
*  @return			Swap par rate
*/
double meLWOSwapParRate( const std::string& swapName,
                         const SWIG_STRINGMATRIX & valuationSettings,
                         const SWIG_STRINGMATRIX & fixingTableNames,
                         const std::string& legName )
{
    MLIB_API_START    	
    
    // Marshall Inputs
    LAStringMatrix valuationSettingsAsStringMatrix;
    swig::buildStringMatrix( valuationSettingsAsStringMatrix, valuationSettings );

    LAStringMatrix fixingTableAsStringMatrix;
    swig::buildStringMatrix( fixingTableAsStringMatrix, fixingTableNames );

    // Call Function and Return Result
    double result = validation_api::tryMeLWOSwapParRate( swapName, valuationSettingsAsStringMatrix, fixingTableAsStringMatrix, legName );

    return result;
    
    MLIB_API_END
}

/* @brief			Function to calculate the Swap Annuity
*  @param [in]		swapName		Swap object name
*  @param [in]		curveCollection Valuation settings
*  @param [in]		legName		    Leg name
*  @return			Swap PV
*/
double meLWOSwapAnnuity( const std::string& swapName,
                         const SWIG_STRINGMATRIX & valuationSettings,
                         const std::string& legName )
{
    MLIB_API_START
    
    // Marshall Inputs
    LAString legNameAsLAString( legName.c_str() );
    
    LAStringMatrix valuationSettingsAsStringMatrix;
    swig::buildStringMatrix( valuationSettingsAsStringMatrix, valuationSettings );

    // Call Function and Return Result
    double result = validation_api::tryMeLWOSwapAnnuity( swapName,
                                                            valuationSettingsAsStringMatrix,
                                                            legNameAsLAString );
    return result;
    MLIB_API_END
}

/* @brief			Function to calculate the Par Spread - Excluding existing spread
*  @param [in]		swapName		Swap name
*  @param [in]		curveCollection Curve collections
*  @param [in]		fixingTableName	Fixing table object names
*  @param [in]		spreadLegName   Leg name the spread will be applied to
*  @return			The spread that make the swap PV zero
*/
double meLWOSwapParSpread( const std::string& swapName,
                           const SWIG_STRINGMATRIX & valuationSettings,
                           const SWIG_STRINGMATRIX & fixingTableNames,
                           const std::string & spreadLegName )
{
    MLIB_API_START
    
    // Marshall Inputs
    LAString spreadLegNameAsLAString( spreadLegName.c_str() );
    
    LAStringMatrix valuationSettingsAsStringMatrix;
    swig::buildStringMatrix( valuationSettingsAsStringMatrix, valuationSettings );

    LAStringMatrix fixingTableAsStringMatrix;
    swig::buildStringMatrix( fixingTableAsStringMatrix, fixingTableNames );

    // Call Function and Return Result
    double result = validation_api::tryMeLWOSwapParSpread( swapName,
                                                           valuationSettingsAsStringMatrix,
                                                           fixingTableAsStringMatrix,
                                                           spreadLegNameAsLAString );
    return result;
    MLIB_API_END
}


/* @brief			Function to calculate the Swap Spread - Including existing spread
*  @param [in]		swapName		Swap name
*  @param [in]		curveCollection Curve collections
*  @param [in]		fixingTableName	Fixing table object names
*  @param [in]		spreadLegName   Leg name the spread will be applied to
*  @return			The spread that make the swap PV zero
*/
double meLWOSwapSpread( const std::string& swapName,
                        const SWIG_STRINGMATRIX & valuationSettings,
                        const SWIG_STRINGMATRIX & fixingTableNames,
                        const std::string & spreadLegName )
{
    MLIB_API_START
    
    // Marshall Inputs
    LAString spreadLegNameAsLAString( spreadLegName.c_str() );
    
    LAStringMatrix valuationSettingsAsStringMatrix;
    swig::buildStringMatrix( valuationSettingsAsStringMatrix, valuationSettings );

    LAStringMatrix fixingTableAsStringMatrix;
    swig::buildStringMatrix( fixingTableAsStringMatrix, fixingTableNames );

    // Call Function and Return Result
    double result = validation_api::tryMeLWOSwapSpread( swapName,
                                                       valuationSettingsAsStringMatrix,
                                                       fixingTableAsStringMatrix,
                                                       spreadLegNameAsLAString );
    return result;
    MLIB_API_END
}


/* @brief			Function to calculate the display swap cashflows
*  @param [in]		swapName			Swap object name
*  @param [in]		curveCollection		Valuation settings
*  @param [in]		legName				Leg name
*  @param [in]		fixingTableNames	Fixing table object names
*  @param [in]		showColumnHeaders	Show Column Headers, true/false
*  @return			Swap Cashflow Table
*/
SWIG_STRINGMATRIX meLWOSwapDisplayCashflows( const std::string& swapName,
											 const SWIG_STRINGMATRIX & valuationSettings,
											 const std::string& legName,
											 const SWIG_STRINGMATRIX & fixingTableNames,
											 const bool showColumnHeaders )
{
	MLIB_API_START

	// Marshall Inputs
	LAString legNameAsLAString( legName.c_str() );

	LAStringMatrix valuationSettingsAsStringMatrix;
	swig::buildStringMatrix( valuationSettingsAsStringMatrix, valuationSettings );

	LAStringMatrix fixingTableAsStringMatrix;
	swig::buildStringMatrix( fixingTableAsStringMatrix, fixingTableNames );

	// Call Function and Return Result
	std::vector<AnyTypeMatrix> result
		= validation_api::tryMeLWOSwapDisplayCashflows( swapName,
														valuationSettingsAsStringMatrix,
														legNameAsLAString,
														fixingTableAsStringMatrix,
														showColumnHeaders );

	// Marshall Output to Standard String Matrix
	SWIG_STRINGMATRIX resultsStringMatrix = swig::fromVectorOfAnyTypeMatrixToMatrixOfString( result );

	return resultsStringMatrix;

	MLIB_API_END
}
