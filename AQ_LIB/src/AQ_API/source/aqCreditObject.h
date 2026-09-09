// aqCreditObject.h

/*
 * @brief			Swig Interface file for Credit Model
 *					This is used to calibrate credit curves and price CDS / TRS
 */

#pragma once
#include "SwigTypes.h"
#include "AQLCoreTemplateType.h"     // Data TypeDefs
#include <string>
#include <vector>


/* @brief Creates a credit model
* @param [in] objectName        The name of the credit model object name
* @param [in] key1              Key1: The label for the data block 1
* @param [in] dataBlock1        Value1: Data block 1
* @param [in] key2              Key2: The label for the data block 2
* @param [in] dataBlock2        Value2: Data block 2
* @param [out]                  The credit model object Name
*/
std::string aqCreditModelCreate( const std::string & objectName,
                                    const std::string & key1,
                                    const std::vector<std::vector<std::string> > & dataBlock1,
                                    const std::string & key2,
                                    const std::vector<std::vector<std::string> > & dataBlock2 );


// Extracts the Survival Probability from the Credit Model
double aqCreditModelSurvivalProbability( const std::string& creditModelName, const std::string& toDate, const std::string& fromDate );

// Extracts the Default Probability from the Credit Model
double aqCreditModelDefaultProbability( const std::string& creditModelName, const std::string& toDate, const std::string& fromDate );

// Calculates the HarzardRate
double aqCreditModelHazardRate( const std::string& creditModelName, const std::string & paymentDate );

// Calculates the Risky Discount Factor from the Credit Model
double aqObjCreditModelRiskyDiscountFactor( const std::string& creditModelName, const std::string & paymentDate );

// Calculates a Vector of Risky Discount Factors from the Credit Model
std::vector<double> aqCreditModelRiskyDiscountFactors( const std::string& creditModelName, const std::vector<std::string>& paymentDates );


/* @brief validation interface for the aqCreditObjectDefaultSwapPV method.
*   Calculates the PV of the specified Credit Default Swap.
*
* @param[in]	swapName			Swap object name
* @param[in]	creditModelName		Credit Model object name
* @param[in]	legName			    If specified, calculate the PV of the single swap leg
* @returns	The calculated PV value
*/
double aqCreditObjectDefaultSwapPV( const std::string& swapName, const std::string& creditModelName, const std::string& legName );

/* @brief Calculates the risky annuity of the specified Credit Default Swap Premium leg.
*
* @param [in]	swapName			The name of the credit default swap object
* @param[in]	creditModelName		Credit Model object name
* @param[in]	legName				The Premium leg to use when calculating the risky annuity. A mandatory parameter
* @returns	The risky annuity
*/
double aqCreditObjectDefaultSwapRiskyAnnuity( const std::string& swapName, const std::string& creditModelName, const std::string& legName );

/* @brief Calculates the CS01 of the specified Credit Default Swap Premium leg.
*
* @param [in]	swapName			The name of the credit default swap object
* @param[in]	creditModelName		Credit Model object name
* @param[in]	legName				The Premium leg to use when calculating the risky annuity. A mandatory parameter
* @returns	The risky annuity
*/
double aqCreditObjectDefaultSwapCS01( const std::string& swapName, const std::string& creditModelName, const std::string& legName );


/* @brief Calculates the par spread of the specified Credit Default Swap.
*
* @param[in]	swapName				The name of the credit default swap object
* @param[in]	creditModelName			Credit Model object name
* @param[in]	premiumLegName			The Premium leg name of the CDS
* @param[in]	protectionLegName		The Protection leg name of the CDS
* @returns	The CDS par spread
*/
double aqCreditObjectDefaultSwapParSpread( const std::string& swapName, const std::string& creditModelName, const std::string& premiumLegName, const std::string& protectionLegName );

/* @brief validation interface for the aqTotalReturnSwapObjectPV method.
*   Calculates the PV of the specified Total Return Swap.
*
* @param[in]	swapName			The name of the total return swap object
* @param[in]	creditModelName		Credit Model object name
* @param[in]	legName			    If specified, calculate the PV of the single swap leg
* @param[in]	fixingTableNames	Fixing table object names
* @returns	The calculated PV value
*/
double aqTotalReturnSwapObjectPV( const std::string& swapName, const std::string& creditModelName, const std::string& legName, const SWIG_STRINGMATRIX & fixingTableNames );

// Optional Argument Method
double aqTotalReturnSwapObjectPV( const std::string& swapName, const std::string& creditModelName, const std::string& legName );

/* @brief Calculates the par rate of the specified Total Return Swap's premium leg.
*
* @param[in]	swapName				The name of the total return swap object
* @param[in]	creditModelName			Credit Model object name
* @param[in]	fixingTableNames		Fixing table object names
* @returns	The par rate of the TRS premium leg
*/
double aqTotalReturnSwapObjectParRate( const std::string& swapName, const std::string& creditModelName, const SWIG_STRINGMATRIX & fixingTableNames );

// Optional Argument Method
double aqTotalReturnSwapObjectParRate( const std::string& swapName, const std::string& creditModelName );

/* @brief Calculates the par spread of the specified Total Return Swap's float leg.
*
* @param[in]	swapName				The name of the total return swap object
* @param[in]	creditModelName			Credit Model object name
* @param[in]	fixingTableNames		Fixing table object names
* @returns	The par spread of the TRS float leg
*/
double aqTotalReturnSwapObjectParSpread( const std::string& swapName, const std::string& creditModelName, const SWIG_STRINGMATRIX & fixingTableNames );

// Optional Argument Method
double aqTotalReturnSwapObjectParSpread( const std::string& swapName, const std::string& creditModelName );

/* @brief validation interface for the aqTotalReturnSwapObjectPV method.
*   Calculates the annity of the specified Total Return Swap leg
*
* @param[in]	swapName			The name of the total return swap object
* @param[in]	creditModelName		Credit Model object name
* @param[in]	legName			    Mandatory, calculate the annuity of this leg.
* @returns	The calculated annuity value
*/
double aqTotalReturnSwapObjectAnnuity( const std::string& swapName, const std::string& creditModelName, const std::string& legName );
