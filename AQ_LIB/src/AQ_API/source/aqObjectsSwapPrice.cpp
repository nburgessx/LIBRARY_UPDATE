// aqObjectsSwapPrice.cpp

/* 
 * @brief			Swig interface for aqObjectsSwapPrice... functions
 * @Created:		25th June 2018
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "aqObjectsSwapPrice.h"
#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"
#include "tryAqObjectsSwapPricing.h"
#include "APISetUp.h"					// AQ_API_START and AQ_API_END Macros

/* @brief			Function to calculate the Swap PV
*  @param [in]		swapName			Swap object name
*  @param [in]		curveCollection		Valuation settings
*  @param [in]		legName				Leg name
*  @param [in]		fixingTableNames	Fixing table object names
*  @return			Swap PV
*/
double aqObjectsSwapPV( const std::string& swapName,
                    const SWIG_STRINGMATRIX & valuationSettings,
                    const std::string& legName,
                    const SWIG_STRINGMATRIX & fixingTableNames )
{
    AQ_API_START
    
    // Marshall Inputs
    AQLString legNameAsAQLString( legName.c_str() );
    
    AQLStringMatrix valuationSettingsAsStringMatrix;
    swig::buildStringMatrix( valuationSettingsAsStringMatrix, valuationSettings );

    AQLStringMatrix fixingTableAsStringMatrix;
    swig::buildStringMatrix( fixingTableAsStringMatrix, fixingTableNames );

    // Call Function and Return Result
    double result = validation::tryAqObjectsSwapPV( swapName,
                                                    valuationSettingsAsStringMatrix,
                                                    legNameAsAQLString,
                                                    fixingTableAsStringMatrix );
    return result;
    
    AQ_API_END
	
}

/* @brief			Function to calculate the Swap PV01
*  @param [in]		swapName			Swap object name
*  @param [in]		curveCollection		Valuation settings
*  @param [in]		fixingTableNames	Fixing table object names
*  @return			Swap PV01
*/
double aqObjectsSwapPV01( const std::string& swapName,
                      const SWIG_STRINGMATRIX & valuationSettings,
                      const SWIG_STRINGMATRIX & fixingTableNames )
{
    AQ_API_START
    
    // Marshall Inputs
    AQLStringMatrix valuationSettingsAsStringMatrix;
    swig::buildStringMatrix( valuationSettingsAsStringMatrix, valuationSettings );

    AQLStringMatrix fixingTableAsStringMatrix;
    swig::buildStringMatrix( fixingTableAsStringMatrix, fixingTableNames );

    // Call Function and Return Result
    double result = validation::tryAqObjectsSwapPV01( swapName,
                                                        valuationSettingsAsStringMatrix,
                                                        fixingTableAsStringMatrix );
    return result;
    
    AQ_API_END
}

/* @brief			Function to calculate the Swap Par Rate
*  @param [in]		swapName		 Swap object name
*  @param [in]		valuationSettings  Valuation settings map
*  @param [in]		fixingTableNames Fixing table object names
*  @return			Swap par rate
*/
double aqObjectsSwapParRate( const std::string& swapName,
                         const SWIG_STRINGMATRIX & valuationSettings,
                         const SWIG_STRINGMATRIX & fixingTableNames )
{
    AQ_API_START    	
    
    // Marshall Inputs
    AQLStringMatrix valuationSettingsAsStringMatrix;
    swig::buildStringMatrix( valuationSettingsAsStringMatrix, valuationSettings );

    AQLStringMatrix fixingTableAsStringMatrix;
    swig::buildStringMatrix( fixingTableAsStringMatrix, fixingTableNames );

    // Call Function and Return Result
    double result = validation::tryAqObjectsSwapParRate( swapName,
                                                            valuationSettingsAsStringMatrix,
                                                            fixingTableAsStringMatrix );
    return result;
    
    AQ_API_END
}

/* @brief			Function to calculate the Swap Par Rate
*  @param [in]		swapName		 Swap object name
*  @param [in]		valuationSettings  Valuation settings map
*  @param [in]		fixingTableNames Fixing table object names
*  @return			Swap par rate
*/
double aqObjectsSwapParRate( const std::string& swapName,
                         const SWIG_STRINGMATRIX & valuationSettings,
                         const SWIG_STRINGMATRIX & fixingTableNames,
                         const std::string& legName )
{
    AQ_API_START    	
    
    // Marshall Inputs
    AQLStringMatrix valuationSettingsAsStringMatrix;
    swig::buildStringMatrix( valuationSettingsAsStringMatrix, valuationSettings );

    AQLStringMatrix fixingTableAsStringMatrix;
    swig::buildStringMatrix( fixingTableAsStringMatrix, fixingTableNames );

    // Call Function and Return Result
    double result = validation::tryAqObjectsSwapParRate( swapName, valuationSettingsAsStringMatrix, fixingTableAsStringMatrix, legName );

    return result;
    
    AQ_API_END
}

/* @brief			Function to calculate the Swap Annuity
*  @param [in]		swapName		Swap object name
*  @param [in]		curveCollection Valuation settings
*  @param [in]		legName		    Leg name
*  @return			Swap PV
*/
double aqObjectsSwapAnnuity( const std::string& swapName,
                         const SWIG_STRINGMATRIX & valuationSettings,
                         const std::string& legName )
{
    AQ_API_START
    
    // Marshall Inputs
    AQLString legNameAsAQLString( legName.c_str() );
    
    AQLStringMatrix valuationSettingsAsStringMatrix;
    swig::buildStringMatrix( valuationSettingsAsStringMatrix, valuationSettings );

    // Call Function and Return Result
    double result = validation::tryAqObjectsSwapAnnuity( swapName,
                                                            valuationSettingsAsStringMatrix,
                                                            legNameAsAQLString );
    return result;
    AQ_API_END
}

/* @brief			Function to calculate the Par Spread - Excluding existing spread
*  @param [in]		swapName		Swap name
*  @param [in]		curveCollection Curve collections
*  @param [in]		fixingTableName	Fixing table object names
*  @param [in]		spreadLegName   Leg name the spread will be applied to
*  @return			The spread that make the swap PV zero
*/
double aqObjectsSwapParSpread( const std::string& swapName,
                           const SWIG_STRINGMATRIX & valuationSettings,
                           const SWIG_STRINGMATRIX & fixingTableNames,
                           const std::string & spreadLegName )
{
    AQ_API_START
    
    // Marshall Inputs
    AQLString spreadLegNameAsAQLString( spreadLegName.c_str() );
    
    AQLStringMatrix valuationSettingsAsStringMatrix;
    swig::buildStringMatrix( valuationSettingsAsStringMatrix, valuationSettings );

    AQLStringMatrix fixingTableAsStringMatrix;
    swig::buildStringMatrix( fixingTableAsStringMatrix, fixingTableNames );

    // Call Function and Return Result
    double result = validation::tryAqObjectsSwapParSpread( swapName,
                                                           valuationSettingsAsStringMatrix,
                                                           fixingTableAsStringMatrix,
                                                           spreadLegNameAsAQLString );
    return result;
    AQ_API_END
}


/* @brief			Function to calculate the Swap Spread - Including existing spread
*  @param [in]		swapName		Swap name
*  @param [in]		curveCollection Curve collections
*  @param [in]		fixingTableName	Fixing table object names
*  @param [in]		spreadLegName   Leg name the spread will be applied to
*  @return			The spread that make the swap PV zero
*/
double aqObjectsSwapSpread( const std::string& swapName,
                        const SWIG_STRINGMATRIX & valuationSettings,
                        const SWIG_STRINGMATRIX & fixingTableNames,
                        const std::string & spreadLegName )
{
    AQ_API_START
    
    // Marshall Inputs
    AQLString spreadLegNameAsAQLString( spreadLegName.c_str() );
    
    AQLStringMatrix valuationSettingsAsStringMatrix;
    swig::buildStringMatrix( valuationSettingsAsStringMatrix, valuationSettings );

    AQLStringMatrix fixingTableAsStringMatrix;
    swig::buildStringMatrix( fixingTableAsStringMatrix, fixingTableNames );

    // Call Function and Return Result
    double result = validation::tryAqObjectsSwapSpread( swapName,
                                                       valuationSettingsAsStringMatrix,
                                                       fixingTableAsStringMatrix,
                                                       spreadLegNameAsAQLString );
    return result;
    AQ_API_END
}


/* @brief			Function to calculate the display swap cashflows
*  @param [in]		swapName			Swap object name
*  @param [in]		curveCollection		Valuation settings
*  @param [in]		legName				Leg name
*  @param [in]		fixingTableNames	Fixing table object names
*  @param [in]		showColumnHeaders	Show Column Headers, true/false
*  @return			Swap Cashflow Table
*/
SWIG_STRINGMATRIX aqObjectsSwapDisplayCashflows( const std::string& swapName,
											 const SWIG_STRINGMATRIX & valuationSettings,
											 const std::string& legName,
											 const SWIG_STRINGMATRIX & fixingTableNames,
											 const bool showColumnHeaders )
{
	AQ_API_START

	// Marshall Inputs
	AQLString legNameAsAQLString( legName.c_str() );

	AQLStringMatrix valuationSettingsAsStringMatrix;
	swig::buildStringMatrix( valuationSettingsAsStringMatrix, valuationSettings );

	AQLStringMatrix fixingTableAsStringMatrix;
	swig::buildStringMatrix( fixingTableAsStringMatrix, fixingTableNames );

	// Call Function and Return Result
	std::vector<AnyTypeMatrix> result
		= validation::tryAqObjectsSwapDisplayCashflows( swapName,
														valuationSettingsAsStringMatrix,
														legNameAsAQLString,
														fixingTableAsStringMatrix,
														showColumnHeaders );

	// Marshall Output to Standard String Matrix
	SWIG_STRINGMATRIX resultsStringMatrix = swig::fromVectorOfAnyTypeMatrixToMatrixOfString( result );

	return resultsStringMatrix;

	AQ_API_END
}
