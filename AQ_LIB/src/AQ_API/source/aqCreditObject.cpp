// aqCreditObject.cpp

/*
 * @brief			Swig Interface file for Credit Model
 *					This is used to calibrate credit curves and price CDS / TRS
 */

#include "aqCreditObject.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros
#include "TypeUtilities.h"          // Swig Type Marshalling & TypeDefs
#include "tryAqSwapObjectPricing.h"    // Credit Model
#include "tryAqSwapObjectLeg.h"        // tryAqCreditObjectFeeLegCreate
#include "tryAqSwapObjectSchedule.h"   // tryAqCreditObjectFeeScheduleCreate
#include "JSONInfoBlock.h"          // JSON InfoBlock Container
#include "ParameterValidation.h"
#include "DateUtilities.h"          // etrading::toYYYYMMDDFromDate - date to string converter


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

/* @brief The as-of / valuation date of a cached credit model.
* @param [in] creditModelName	A cached credit-model handle
* @returns	The as-of date, as a YYYYMMDD string
*/
std::string aqCreditModelAsOfDate( const std::string& creditModelName )
{
    AQ_API_START

    AQLDate result = validation::tryAqCreditModelAsOfDate( creditModelName );
    return etrading::toYYYYMMDDFromDate( result );

    AQ_API_END
}

/* @brief The calibrated payment dates and hazard rates of a cached credit model.
* @param [in] creditModelName	A cached credit-model handle
* @returns	A matrix containing payment dates and hazard rates
*/
SWIG_STRINGMATRIX aqCreditModelCalibrationParameters( const std::string& creditModelName )
{
    AQ_API_START

    AnyTypeMatrix result = validation::tryAqCreditModelCalibrationParameters( creditModelName );
    return swig::fromAnyTypeMatrixToMatrixOfString( result );

    AQ_API_END
}

/* @brief The date implied by a target survival probability, from a cached credit model (inverse of aqCreditModelSurvivalProbability).
* @param [in] creditModelName		A cached credit-model handle
* @param [in] survivalProbability	The target survival probability
* @returns	The implied survival date, as a YYYYMMDD string
*/
std::string aqCreditModelImpliedSurvivalDate( const std::string& creditModelName, const double survivalProbability )
{
    AQ_API_START

    AQLDate result = validation::tryAqCreditModelImpliedSurvivalDate( creditModelName, survivalProbability );
    return etrading::toYYYYMMDDFromDate( result );

    AQ_API_END
}

/* @brief Forward credit spread between two dates, implied by a cached credit model (spot spread if startDate is the model's as-of date).
* @param [in] creditModelName	A cached credit-model handle
* @param [in] startDate		The date protection begins
* @param [in] endDate			The date protection ends
* @returns	The calculated forward spread
*/
double aqCreditObjectSpread( const std::string& creditModelName, const std::string& startDate, const std::string& endDate )
{
    AQ_API_START

    AQLDate startDate_( etrading::stringToDate( startDate ) );
    AQLDate endDate_( etrading::stringToDate( endDate ) );

    double result = validation::tryAqCreditObjectSpread( creditModelName, startDate_, endDate_ );
    return result;

    AQ_API_END
}

/* @brief Forward credit-index spread between two dates (Bloomberg credit-index convention: protection starts
*         immediately, the annuity leg starts on the forward start date).
* @param [in] creditModelName	A cached credit-model handle
* @param [in] startDate		The date protection begins
* @param [in] endDate			The date protection ends
* @returns	The calculated forward spread
*/
double aqCreditObjectIndexSpread( const std::string& creditModelName, const std::string& startDate, const std::string& endDate )
{
    AQ_API_START

    AQLDate startDate_( etrading::stringToDate( startDate ) );
    AQLDate endDate_( etrading::stringToDate( endDate ) );

    double result = validation::tryAqCreditObjectIndexSpread( creditModelName, startDate_, endDate_ );
    return result;

    AQ_API_END
}

/* @brief PV of an option on a credit default swap (Hull-White CDS option model).
*/
double aqCreditObjectOptionPV( const std::string& creditModelName, const std::string& payerReceiver, const double strike,
                                const std::string& optionExpiryDate, const std::string& cdsMaturityDate, const double volatility )
{
    AQ_API_START

    AQLDate optionExpiryDate_( etrading::stringToDate( optionExpiryDate ) );
    AQLDate cdsMaturityDate_( etrading::stringToDate( cdsMaturityDate ) );

    double result = validation::tryAqCreditObjectOptionPV( creditModelName, payerReceiver, strike, optionExpiryDate_, cdsMaturityDate_, volatility );
    return result;

    AQ_API_END
}

/* @brief PV of an option on a CDS, given its forward spread at expiry directly (Hull-White CDS option model).
*/
double aqCreditObjectOptionPVFromForward( const std::string& creditModelName, const std::string& payerReceiver, const double strike,
                                           const std::string& optionExpiryDate, const std::string& cdsStartDate, const std::string& cdsMaturityDate,
                                           const double volatility, const double forwardSpread )
{
    AQ_API_START

    AQLDate optionExpiryDate_( etrading::stringToDate( optionExpiryDate ) );
    AQLDate cdsStartDate_( etrading::stringToDate( cdsStartDate ) );
    AQLDate cdsMaturityDate_( etrading::stringToDate( cdsMaturityDate ) );

    double result = validation::tryAqCreditObjectOptionPVFromForward( creditModelName, payerReceiver, strike,
        optionExpiryDate_, cdsStartDate_, cdsMaturityDate_, volatility, forwardSpread );
    return result;

    AQ_API_END
}

/* @brief Implied volatility of a CDS option, given a target quoted option value.
*/
double aqCreditObjectOptionImpliedVol( const std::string& creditModelName, const std::string& payerReceiver, const double strike,
                                        const std::string& optionExpiryDate, const std::string& cdsMaturityDate, const double targetOptionValue )
{
    AQ_API_START

    AQLDate optionExpiryDate_( etrading::stringToDate( optionExpiryDate ) );
    AQLDate cdsMaturityDate_( etrading::stringToDate( cdsMaturityDate ) );

    double result = validation::tryAqCreditObjectOptionImpliedVol( creditModelName, payerReceiver, strike,
        optionExpiryDate_, cdsMaturityDate_, targetOptionValue );
    return result;

    AQ_API_END
}

/* @brief Implied volatility of a CDS option, given a target quoted option value and the CDS forward spread.
*/
double aqCreditObjectOptionImpliedVolFromForward( const std::string& creditModelName, const std::string& payerReceiver, const double strike,
                                                   const std::string& optionExpiryDate, const std::string& cdsStartDate, const std::string& cdsMaturityDate,
                                                   const double targetOptionValue, const double forwardSpread )
{
    AQ_API_START

    AQLDate optionExpiryDate_( etrading::stringToDate( optionExpiryDate ) );
    AQLDate cdsStartDate_( etrading::stringToDate( cdsStartDate ) );
    AQLDate cdsMaturityDate_( etrading::stringToDate( cdsMaturityDate ) );

    double result = validation::tryAqCreditObjectOptionImpliedVolFromForward( creditModelName, payerReceiver, strike,
        optionExpiryDate_, cdsStartDate_, cdsMaturityDate_, targetOptionValue, forwardSpread );
    return result;

    AQ_API_END
}

/* @brief PV of a credit index option (Bloomberg credit-index-option model).
*/
double aqCreditObjectIndexOptionPV( const std::string& creditModelName, const SWIG_STRINGMATRIX& optionLVB )
{
    AQ_API_START

    LabelValueBlock optionLVB_ = swig::buildSingleLabelValueBlock( optionLVB );

    double result = validation::tryAqCreditObjectIndexOptionPV( creditModelName, optionLVB_ );
    return result;

    AQ_API_END
}

/* @brief Implied volatility of a credit index option, given a target quote and forward.
*/
double aqCreditObjectIndexOptionImpliedVol( const std::string& creditModelName, const SWIG_STRINGMATRIX& optionLVB )
{
    AQ_API_START

    LabelValueBlock optionLVB_ = swig::buildSingleLabelValueBlock( optionLVB );

    double result = validation::tryAqCreditObjectIndexOptionImpliedVol( creditModelName, optionLVB_ );
    return result;

    AQ_API_END
}

/* @brief Vega of a credit index option.
*/
double aqCreditObjectIndexOptionVega( const std::string& creditModelName, const SWIG_STRINGMATRIX& optionLVB, const double volatilityBump )
{
    AQ_API_START

    LabelValueBlock optionLVB_ = swig::buildSingleLabelValueBlock( optionLVB );

    double result = validation::tryAqCreditObjectIndexOptionVega( creditModelName, optionLVB_, volatilityBump );
    return result;

    AQ_API_END
}

/* @brief CS01 of a credit index option.
*/
double aqCreditObjectIndexOptionCS01( const std::string& creditModelName, const SWIG_STRINGMATRIX& optionLVB )
{
    AQ_API_START

    LabelValueBlock optionLVB_ = swig::buildSingleLabelValueBlock( optionLVB );

    double result = validation::tryAqCreditObjectIndexOptionCS01( creditModelName, optionLVB_ );
    return result;

    AQ_API_END
}

/* @brief Theta of a credit index option.
*/
double aqCreditObjectIndexOptionTheta( const std::string& creditModelName, const SWIG_STRINGMATRIX& optionLVB )
{
    AQ_API_START

    LabelValueBlock optionLVB_ = swig::buildSingleLabelValueBlock( optionLVB );

    double result = validation::tryAqCreditObjectIndexOptionTheta( creditModelName, optionLVB_ );
    return result;

    AQ_API_END
}

// dataBlock1/dataBlock2 are arbitrary-shape; see the matching note in aqCreditObject.h.
#if (!defined(SWIG_R)) && (!defined(SWIGR))
/* @brief Creates a Credit Basket-Model, constructed from underlying Credit Models.
*/
std::string aqCreditBasketModelCreate( const std::string& objectName,
                                        const std::string& key1, const SWIG_STRINGMATRIX& dataBlock1,
                                        const std::string& key2, const SWIG_STRINGMATRIX& dataBlock2 )
{
    AQ_API_START

    // Marshall Inputs - mirrors aqCreditModelCreate's pattern above
    etrading::VariantMatrix variantDataBlock1;
    swig::buildVariantMatrix( variantDataBlock1, dataBlock1 );

    etrading::VariantMatrix variantDataBlock2;
    swig::buildVariantMatrix( variantDataBlock2, dataBlock2 );

    const etrading::JSONInfoBlockTuple infoBlock1  = etrading::JSONInfoBlock::createInfoBlock( variantDataBlock1 );
    const etrading::JSONInfoBlockTuple infoBlock2  = etrading::JSONInfoBlock::createInfoBlock( variantDataBlock2 );
    const etrading::JSONInfoBlockTuples infoBlocks = { infoBlock1, infoBlock2 };

    const std::vector<std::string> dataBlockNames = { key1, key2 };

    // Call the Function
    std::string result = validation::tryAqCreditBasketModelCreate( objectName, dataBlockNames, infoBlocks );
    return result;

    AQ_API_END
}
#endif

/* @brief Survival probability between two dates, computed by a cached credit basket model.
*/
double aqCreditBasketModelSurvivalProbability( const std::string& creditBasketModelName, const std::string& toDate, const std::string& fromDate )
{
    AQ_API_START

    AQLDate toDate_( etrading::stringToDate( toDate ) );
    AQLDate fromDate_( etrading::stringToDate( fromDate ) );

    double result = validation::tryAqCreditBasketModelSurvivalProbability( creditBasketModelName, toDate_, fromDate_ );
    return result;

    AQ_API_END
}

/* @brief Create and store a CDS fee (premium) leg; returns its handle.
*/
std::string aqCreditObjectFeeLegCreate( const std::string& legObjectName, const SWIG_STRINGMATRIX& feeProperties,
                                         const SWIG_STRINGMATRIX& feeScheduleLVB, const bool validateKeys )
{
    AQ_API_START

    LabelValueBlock feeProperties_ = swig::buildSingleLabelValueBlock( feeProperties );

    AQLStringMatrix feeScheduleLVB_;
    swig::buildStringMatrix( feeScheduleLVB_, feeScheduleLVB );

    std::string result = validation::tryAqCreditObjectFeeLegCreate( legObjectName, feeProperties_, feeScheduleLVB_, validateKeys );
    return result;

    AQ_API_END
}

/* @brief Create and store a CDS fee schedule from a cashflow matrix; returns its handle.
*/
std::string aqCreditObjectFeeScheduleCreate( const std::string& scheduleName, const SWIG_STRINGMATRIX& feeScheduleLVB, const bool validateKeys )
{
    AQ_API_START

    AQLStringMatrix feeScheduleLVB_;
    swig::buildStringMatrix( feeScheduleLVB_, feeScheduleLVB );

    std::string result = validation::tryAqCreditObjectFeeScheduleCreate( scheduleName, feeScheduleLVB_, validateKeys );
    return result;

    AQ_API_END
}


