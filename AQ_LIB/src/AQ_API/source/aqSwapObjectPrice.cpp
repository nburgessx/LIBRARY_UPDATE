// aqSwapObjectPrice.cpp

/* 
 * @brief			Swig interface for aqObjSwapsPrice... functions
 */

#include "aqSwapObjectPrice.h"
#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"
#include "tryAqSwapObjectPricing.h"
#include "APISetUp.h"					// AQ_API_START and AQ_API_END Macros

/* @brief			Function to calculate the Swap PV
*  @param [in]		swapName			Swap object name
*  @param [in]		curveCollection		Valuation settings
*  @param [in]		legName				Leg name
*  @param [in]		fixingTableNames	Fixing table object names
*  @return			Swap PV
*/
double aqSwapObjectPV( const std::string& swapName,
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
    double result = validation::tryAqSwapObjectPV( swapName,
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
double aqSwapObjectPV01( const std::string& swapName,
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
    double result = validation::tryAqSwapObjectPV01( swapName,
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
double aqSwapObjectParRate( const std::string& swapName,
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
    double result = validation::tryAqSwapObjectParRate( swapName,
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
double aqSwapObjectParRate( const std::string& swapName,
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
    double result = validation::tryAqSwapObjectParRate( swapName, valuationSettingsAsStringMatrix, fixingTableAsStringMatrix, legName );

    return result;
    
    AQ_API_END
}

/* @brief			Function to calculate the Swap Annuity
*  @param [in]		swapName		Swap object name
*  @param [in]		curveCollection Valuation settings
*  @param [in]		legName		    Leg name
*  @return			Swap PV
*/
double aqSwapObjectAnnuity( const std::string& swapName,
                         const SWIG_STRINGMATRIX & valuationSettings,
                         const std::string& legName )
{
    AQ_API_START
    
    // Marshall Inputs
    AQLString legNameAsAQLString( legName.c_str() );
    
    AQLStringMatrix valuationSettingsAsStringMatrix;
    swig::buildStringMatrix( valuationSettingsAsStringMatrix, valuationSettings );

    // Call Function and Return Result
    double result = validation::tryAqSwapObjectAnnuity( swapName,
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
double aqSwapObjectParSpread( const std::string& swapName,
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
    double result = validation::tryAqSwapObjectParSpread( swapName,
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
double aqSwapObjectSpread( const std::string& swapName,
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
    double result = validation::tryAqSwapObjectSpread( swapName,
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
SWIG_STRINGMATRIX aqSwapObjectDisplayCashflows( const std::string& swapName,
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
		= validation::tryAqSwapObjectDisplayCashflows( swapName,
														valuationSettingsAsStringMatrix,
														legNameAsAQLString,
														fixingTableAsStringMatrix,
														showColumnHeaders );

	// Marshall Output to Standard String Matrix
	SWIG_STRINGMATRIX resultsStringMatrix = swig::fromVectorOfAnyTypeMatrixToMatrixOfString( result );

	return resultsStringMatrix;

	AQ_API_END
}
