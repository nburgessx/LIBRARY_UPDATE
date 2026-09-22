// aqCreditObject.cpp

/*
 * @brief			Swig Interface file for Credit Model
 *					This is used to calibrate credit curves and price CDS / TRS
 */

#include "aqCreditObject.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros
#include "TypeUtilities.h"          // Swig Type Marshalling & TypeDefs
#include "tryAqSwapObjectPricing.h"    // Credit Model
#include "JSONInfoBlock.h"          // JSON InfoBlock Container
#include "ParameterValidation.h"


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
                                    const std::vector<std::vector<std::string> > & dataBlock2 )
{
    AQ_API_START

    // Marshall Inputs
    etrading::VariantMatrix variantDataBlock1;
    swig::buildVariantMatrix( variantDataBlock1, dataBlock1 );
        
    etrading::VariantMatrix variantDataBlock2;
    swig::buildVariantMatrix( variantDataBlock2, dataBlock2 );

    // TODO: JSONInfoBlocks should not be here move below the valiation_api
    // Create InfoBlock Tuples - A data container; a tuple of columnNames, columnTypes and a variant data matrix
    const etrading::JSONInfoBlockTuple infoBlock1       = etrading::JSONInfoBlock::createInfoBlock( variantDataBlock1 );
    const etrading::JSONInfoBlockTuple infoBlock2       = etrading::JSONInfoBlock::createInfoBlock( variantDataBlock2 );
    const etrading::JSONInfoBlockTuples infoBlocks      = { infoBlock1, infoBlock2 };

    // PropertyNames: A vector of keys
    const std::vector<std::string> infoBlockNames = { key1, key2 };

    // Call the Function
    std::string result = validation::tryAqCreditModelCreate( objectName, infoBlockNames, infoBlocks );
        
    // Marshall Output(s)
    return result;
    
    AQ_API_END
}


// Extracts the Survival Probability from the Credit Model
double aqCreditModelSurvivalProbability( const std::string& creditModelName, const std::string& toDate, const std::string& fromDate )
{
    AQ_API_START

    // Marshall Input(s)
    AQLDate toDate_( etrading::stringToDate( toDate ) );
    AQLDate fromDate_( etrading::stringToDate( fromDate ) );

    // Call the Function
    double result = validation::tryAqCreditModelSurvivalProbability( creditModelName, toDate_, fromDate_ );
        
    // Marshall Output(s)
    return result;
    
    AQ_API_END
}

// Extracts the Default Probability from the Credit Model
double aqCreditModelDefaultProbability( const std::string& creditModelName, const std::string& toDate, const std::string& fromDate )
{
    AQ_API_START

    // Marshall Input(s)
    AQLDate toDate_( etrading::stringToDate( toDate ) );
    AQLDate fromDate_( etrading::stringToDate( fromDate ) );

    // Call the Function
    double result = validation::tryAqCreditModelDefaultProbability( creditModelName, toDate_, fromDate_ );
        
    // Marshall Output(s)
    return result;
    
    AQ_API_END
}

// Calculates the HarzardRate
double aqCreditModelHazardRate( const std::string& creditModelName, const std::string& paymentDate )
{
    AQ_API_START

    // Marshall Input(s)
    AQLDate paymentDate_( etrading::stringToDate( paymentDate ) );
        
    // Call the Function
    double result = validation::tryAqCreditModelHazardRate( creditModelName, paymentDate_ );

    // Marshall Output(s)
    return result;
    
    AQ_API_END
}

// Calculates the Risky Discount Factor from the Credit Model
double aqCreditModelRiskyDiscountFactor( const std::string& creditModelName, const std::string & paymentDate )
{
    AQ_API_START

    // Marshall Input(s)
    AQLDate paymentDate_( etrading::stringToDate( paymentDate ) );
        
    // Call the Function
    DateVector paymentDateAsVector( 1, paymentDate_ );
    std::vector<double> resultVector = validation::tryAqCreditModelRiskyDiscountFactors( creditModelName, paymentDateAsVector );
    double result = resultVector[0]; 

    // Marshall Output(s)
    return result;
    
    AQ_API_END
}

// Calculates a Vector of Risky Discount Factors from the Credit Model
std::vector<double> aqCreditModelRiskyDiscountFactors( const std::string& creditModelName, const std::vector<std::string>& paymentDates )
{
    AQ_API_START

    // Marshall Input(s)
    DateVector paymentDates_;
    swig::buildDateVector( paymentDates_, paymentDates );

    // Call the Function
    std::vector<double> result = validation::tryAqCreditModelRiskyDiscountFactors( creditModelName, paymentDates_ );
        
    // Marshall Output(s)
    return result;
    
    AQ_API_END
}

/* @brief validation interface for the aqCDSObjectPV method.
*   Calculates the PV of the specified Credit Default Swap.
*
* @param[in]	swapName			Swap object name
* @param[in]	creditModelName		Credit Model object name
* @param[in]	legName			    If specified, calculate the PV of the single swap leg
* @returns	The calculated PV value
*/
double aqCDSObjectPV( const std::string& swapName, const std::string& creditModelName,  const std::string& legName )
{
    AQ_API_START

    // Marshall Input(s)
                
    // Call the Function
    double result = validation::tryAqCDSObjectPV( swapName, creditModelName, legName.c_str() );
        
    // Marshall Output(s)
    return result;
    
    AQ_API_END
}


/* @brief Calculates the risky annuity of the specified Credit Default Swap Premium leg.
*
* @param [in]	swapName			The name of the credit default swap object
* @param[in]	creditModelName		Credit Model object name
* @param[in]	legName				The Premium leg to use when calculating the risky annuity. A mandatory parameter
* @returns	The risky annuity
*/
double aqCDSObjectRiskyAnnuity( const std::string& swapName, const std::string& creditModelName, const std::string& legName )
{
    AQ_API_START

    // Marshall Input(s)
        
    // Call the Function
    double result = validation::tryAqCDSObjectRiskyAnnuity( swapName, creditModelName, legName.c_str() );
        
    // Marshall Output(s)
    return result;
    
    AQ_API_END 
}


/* @brief Calculates the CS01 of the specified Credit Default Swap Premium leg.
*
* @param [in]	swapName			The name of the credit default swap object
* @param[in]	creditModelName		Credit Model object name
* @param[in]	legName				The Premium leg to use when calculating the risky annuity. A mandatory parameter
* @returns	The risky annuity
*/
double aqCDSObjectCS01( const std::string& swapName, const std::string& creditModelName, const std::string& legName )
{
    AQ_API_START

    // Marshall Input(s)
        
    // Call the Function
    double result = validation::tryAqCDSObjectCS01( swapName, creditModelName, legName.c_str() );
        
    // Marshall Output(s)
    return result;
    
    AQ_API_END 
}



/* @brief Calculates the par spread of the specified Credit Default Swap.
*
* @param[in]	swapName				The name of the credit default swap object
* @param[in]	creditModelName			Credit Model object name
* @param[in]	premiumLegName			The Premium leg name of the CDS
* @param[in]	protectionLegName		The Protection leg name of the CDS
* @returns	The CDS par spread
*/
double aqCDSObjectParSpread( const std::string& swapName, const std::string& creditModelName, const std::string& premiumLegName, const std::string& protectionLegName )
{
    AQ_API_START

    // Marshall Input(s)
        
    // Call the Function
    double result = validation::tryAqCDSObjectParSpread( swapName, creditModelName, premiumLegName.c_str(), protectionLegName.c_str() );
        
    // Marshall Output(s)
    return result;
    
    AQ_API_END
}

/* @brief validation interface for the aqTRSObjectPV method.
*   Calculates the PV of the specified Total Return Swap.
*
* @param[in]	swapName			The name of the total return swap object
* @param[in]	creditModelName		Credit Model object name
* @param[in]	legName			    If specified, calculate the PV of the single swap leg
* @param[in]	fixingTableNames	Fixing table object names
* @returns	The calculated PV value
*/
double aqTRSObjectPV( const std::string& swapName, const std::string& creditModelName,  const std::string& legName, const SWIG_STRINGMATRIX & fixingTableNames )
{
    AQ_API_START

    // Marshall Input(s)
    AQLStringMatrix fixingTableLVB;
	swig::buildStringMatrix( fixingTableLVB, fixingTableNames );

    // Call the Function
    double result = validation::tryAqTRSObjectPV( swapName, creditModelName, legName, fixingTableLVB );
        
    // Marshall Output(s)
    return result;
    
    AQ_API_END
}

// Optional Argument Method
double aqTRSObjectPV( const std::string& swapName, const std::string& creditModelName, const std::string& legName )
{
    return aqTRSObjectPV( swapName, creditModelName, legName, SWIG_STRINGMATRIX() ); // Fixing Tables = Null
}

/* @brief Calculates the par rate of the specified Total Return Swap's premium leg.
*
* @param[in]	swapName				The name of the total return swap object
* @param[in]	creditModelName			Credit Model object name
* @param[in]	fixingTableNames		Fixing table object names
* @returns	The par rate of the TRS premium leg
*/
double aqTRSObjectParRate( const std::string& swapName, const std::string& creditModelName, const SWIG_STRINGMATRIX & fixingTableNames )
{
    AQ_API_START

    // Marshall Input(s)
    AQLStringMatrix fixingTableLVB;
	swig::buildStringMatrix( fixingTableLVB, fixingTableNames );

    // Call the Function
    double result = validation::tryAqTRSObjectParRate( swapName, creditModelName, fixingTableLVB );
        
    // Marshall Output(s)
    return result;
    
    AQ_API_END
}

// Optional Argument Method
double aqTRSObjectParRate( const std::string& swapName, const std::string& creditModelName )
{
    return aqTRSObjectParRate( swapName, creditModelName, SWIG_STRINGMATRIX() ); // Fixing Tables = Null
}

/* @brief Calculates the par spread of the specified Total Return Swap's float leg.
*
* @param[in]	swapName				The name of the total return swap object
* @param[in]	creditModelName			Credit Model object name
* @param[in]	fixingTableNames		Fixing table object names
* @returns	The par spread of the TRS float leg
*/
double aqTRSObjectParSpread( const std::string& swapName, const std::string& creditModelName, const SWIG_STRINGMATRIX & fixingTableNames )
{
    AQ_API_START

    // Marshall Input(s)
    AQLStringMatrix fixingTableLVB;
	swig::buildStringMatrix( fixingTableLVB, fixingTableNames );

    // Call the Function
    double result = validation::tryAqTRSObjectParSpread( swapName, creditModelName, fixingTableLVB );
        
    // Marshall Output(s)
    return result;
    
    AQ_API_END
}

// Optional Argument Method
double aqTRSObjectParSpread( const std::string& swapName, const std::string& creditModelName )
{
    return aqTRSObjectParSpread( swapName, creditModelName, SWIG_STRINGMATRIX() ); // Fixing Tables = Null
}

/* @brief validation interface for the aqTRSObjectPV method.
*   Calculates the annity of the specified Total Return Swap leg
*
* @param[in]	swapName			The name of the total return swap object
* @param[in]	creditModelName		Credit Model object name
* @param[in]	legName			    Mandatory, calculate the annuity of this leg.
* @returns	The calculated annuity value
*/
double aqTRSObjectAnnuity( const std::string& swapName, const std::string& creditModelName, const std::string& legName )
{
    AQ_API_START

    // Marshall Input(s)
        
    // Call the Function
    double result = validation::tryAqTRSObjectAnnuity( swapName, creditModelName, legName );
        
    // Marshall Output(s)
    return result;
        
    AQ_API_END 
}


