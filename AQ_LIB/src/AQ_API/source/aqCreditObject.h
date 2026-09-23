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
double aqCreditModelRiskyDiscountFactor( const std::string& creditModelName, const std::string & paymentDate );

// Calculates a Vector of Risky Discount Factors from the Credit Model
std::vector<double> aqCreditModelRiskyDiscountFactors( const std::string& creditModelName, const std::vector<std::string>& paymentDates );


/* @brief validation interface for the aqCDSObjectPV method.
*   Calculates the PV of the specified Credit Default Swap.
*
* @param[in]	swapName			Swap object name
* @param[in]	creditModelName		Credit Model object name
* @param[in]	legName			    If specified, calculate the PV of the single swap leg
* @returns	The calculated PV value
*/
double aqCDSObjectPV( const std::string& swapName, const std::string& creditModelName, const std::string& legName );

/* @brief Calculates the risky annuity of the specified Credit Default Swap Premium leg.
*
* @param [in]	swapName			The name of the credit default swap object
* @param[in]	creditModelName		Credit Model object name
* @param[in]	legName				The Premium leg to use when calculating the risky annuity. A mandatory parameter
* @returns	The risky annuity
*/
double aqCDSObjectRiskyAnnuity( const std::string& swapName, const std::string& creditModelName, const std::string& legName );

/* @brief Calculates the CS01 of the specified Credit Default Swap Premium leg.
*
* @param [in]	swapName			The name of the credit default swap object
* @param[in]	creditModelName		Credit Model object name
* @param[in]	legName				The Premium leg to use when calculating the risky annuity. A mandatory parameter
* @returns	The risky annuity
*/
double aqCDSObjectCS01( const std::string& swapName, const std::string& creditModelName, const std::string& legName );


/* @brief Calculates the par spread of the specified Credit Default Swap.
*
* @param[in]	swapName				The name of the credit default swap object
* @param[in]	creditModelName			Credit Model object name
* @param[in]	premiumLegName			The Premium leg name of the CDS
* @param[in]	protectionLegName		The Protection leg name of the CDS
* @returns	The CDS par spread
*/
double aqCDSObjectParSpread( const std::string& swapName, const std::string& creditModelName, const std::string& premiumLegName, const std::string& protectionLegName );

/* @brief validation interface for the aqTRSObjectPV method.
*   Calculates the PV of the specified Total Return Swap.
*
* @param[in]	swapName			The name of the total return swap object
* @param[in]	creditModelName		Credit Model object name
* @param[in]	legName			    If specified, calculate the PV of the single swap leg
* @param[in]	fixingTableNames	Fixing table object names
* @returns	The calculated PV value
*/
double aqTRSObjectPV( const std::string& swapName, const std::string& creditModelName, const std::string& legName, const SWIG_STRINGMATRIX & fixingTableNames );

// Optional Argument Method
double aqTRSObjectPV( const std::string& swapName, const std::string& creditModelName, const std::string& legName );

/* @brief Calculates the par rate of the specified Total Return Swap's premium leg.
*
* @param[in]	swapName				The name of the total return swap object
* @param[in]	creditModelName			Credit Model object name
* @param[in]	fixingTableNames		Fixing table object names
* @returns	The par rate of the TRS premium leg
*/
double aqTRSObjectParRate( const std::string& swapName, const std::string& creditModelName, const SWIG_STRINGMATRIX & fixingTableNames );

// Optional Argument Method
double aqTRSObjectParRate( const std::string& swapName, const std::string& creditModelName );

/* @brief Calculates the par spread of the specified Total Return Swap's float leg.
*
* @param[in]	swapName				The name of the total return swap object
* @param[in]	creditModelName			Credit Model object name
* @param[in]	fixingTableNames		Fixing table object names
* @returns	The par spread of the TRS float leg
*/
double aqTRSObjectParSpread( const std::string& swapName, const std::string& creditModelName, const SWIG_STRINGMATRIX & fixingTableNames );

// Optional Argument Method
double aqTRSObjectParSpread( const std::string& swapName, const std::string& creditModelName );

/* @brief validation interface for the aqTRSObjectPV method.
*   Calculates the annity of the specified Total Return Swap leg
*
* @param[in]	swapName			The name of the total return swap object
* @param[in]	creditModelName		Credit Model object name
* @param[in]	legName			    Mandatory, calculate the annuity of this leg.
* @returns	The calculated annuity value
*/
double aqTRSObjectAnnuity( const std::string& swapName, const std::string& creditModelName, const std::string& legName );

/* @brief The as-of / valuation date of a cached credit model.
* @param [in] creditModelName	A cached credit-model handle
* @returns	The as-of date, as a YYYYMMDD string
*/
std::string aqCreditModelAsOfDate( const std::string& creditModelName );

/* @brief The calibrated payment dates and hazard rates of a cached credit model.
* @param [in] creditModelName	A cached credit-model handle
* @returns	A matrix containing payment dates and hazard rates
*/
SWIG_STRINGMATRIX aqCreditModelCalibrationParameters( const std::string& creditModelName );

/* @brief The date implied by a target survival probability, from a cached credit model (inverse of aqCreditModelSurvivalProbability).
* @param [in] creditModelName		A cached credit-model handle
* @param [in] survivalProbability	The target survival probability
* @returns	The implied survival date, as a YYYYMMDD string
*/
std::string aqCreditModelImpliedSurvivalDate( const std::string& creditModelName, const double survivalProbability );

/* @brief Forward credit spread between two dates, implied by a cached credit model (spot spread if startDate is the model's as-of date).
* @param [in] creditModelName	A cached credit-model handle
* @param [in] startDate		The date protection begins
* @param [in] endDate			The date protection ends
* @returns	The calculated forward spread
*/
double aqCreditObjectSpread( const std::string& creditModelName, const std::string& startDate, const std::string& endDate );

/* @brief Forward credit-index spread between two dates (Bloomberg credit-index convention: protection starts
*         immediately, the annuity leg starts on the forward start date).
* @param [in] creditModelName	A cached credit-model handle
* @param [in] startDate		The date protection begins
* @param [in] endDate			The date protection ends
* @returns	The calculated forward spread
*/
double aqCreditObjectIndexSpread( const std::string& creditModelName, const std::string& startDate, const std::string& endDate );

/* @brief PV of an option on a credit default swap (Hull-White CDS option model).
* @param [in] creditModelName		A cached credit-model handle
* @param [in] payerReceiver		PAYER (pay CDS spread for protection) or RECEIVER (receive CDS spread)
* @param [in] strike				The strike spread
* @param [in] optionExpiryDate		The option expiry / underlying CDS effective date
* @param [in] cdsMaturityDate		The underlying CDS maturity date
* @param [in] volatility			The underlying CDS spread volatility
* @returns	The calculated option price
*/
double aqCreditObjectOptionPV( const std::string& creditModelName, const std::string& payerReceiver, const double strike,
                                const std::string& optionExpiryDate, const std::string& cdsMaturityDate, const double volatility );

/* @brief PV of an option on a CDS, given its forward spread at expiry directly (Hull-White CDS option model).
* @param [in] creditModelName		A cached credit-model handle
* @param [in] payerReceiver		PAYER (pay CDS spread for protection) or RECEIVER (receive CDS spread)
* @param [in] strike				The strike spread
* @param [in] optionExpiryDate		The option expiry date
* @param [in] cdsStartDate			The underlying CDS's delivery/start date if exercised
* @param [in] cdsMaturityDate		The underlying CDS maturity date
* @param [in] volatility			The underlying CDS spread volatility
* @param [in] forwardSpread		The CDS forward spread at the option expiry date
* @returns	The calculated option price
*/
double aqCreditObjectOptionPVFromForward( const std::string& creditModelName, const std::string& payerReceiver, const double strike,
                                           const std::string& optionExpiryDate, const std::string& cdsStartDate, const std::string& cdsMaturityDate,
                                           const double volatility, const double forwardSpread );

/* @brief Implied volatility of a CDS option, given a target quoted option value.
* @param [in] creditModelName		A cached credit-model handle
* @param [in] payerReceiver		PAYER (pay CDS spread for protection) or RECEIVER (receive CDS spread)
* @param [in] strike				The strike spread
* @param [in] optionExpiryDate		The option expiry / underlying CDS effective date
* @param [in] cdsMaturityDate		The underlying CDS maturity date
* @param [in] targetOptionValue	The target option value to solve the volatility for
* @returns	The implied volatility
*/
double aqCreditObjectOptionImpliedVol( const std::string& creditModelName, const std::string& payerReceiver, const double strike,
                                        const std::string& optionExpiryDate, const std::string& cdsMaturityDate, const double targetOptionValue );

/* @brief Implied volatility of a CDS option, given a target quoted option value and the CDS forward spread.
* @param [in] creditModelName		A cached credit-model handle
* @param [in] payerReceiver		PAYER (pay CDS spread for protection) or RECEIVER (receive CDS spread)
* @param [in] strike				The strike spread
* @param [in] optionExpiryDate		The option expiry date
* @param [in] cdsStartDate			The underlying CDS's delivery/start date if exercised
* @param [in] cdsMaturityDate		The underlying CDS maturity date
* @param [in] targetOptionValue	The target option value to solve the volatility for
* @param [in] forwardSpread		The CDS forward spread at the option expiry date
* @returns	The implied volatility
*/
double aqCreditObjectOptionImpliedVolFromForward( const std::string& creditModelName, const std::string& payerReceiver, const double strike,
                                                   const std::string& optionExpiryDate, const std::string& cdsStartDate, const std::string& cdsMaturityDate,
                                                   const double targetOptionValue, const double forwardSpread );

/* @brief PV of a credit index option (Bloomberg credit-index-option model).
* @param [in] creditModelName	A cached credit-model handle
* @param [in] optionLVB		Strike, expiry date, CDS coupon, volatility etc, as a label/value block
* @returns	The calculated option price
*/
double aqCreditObjectIndexOptionPV( const std::string& creditModelName, const SWIG_STRINGMATRIX& optionLVB );

/* @brief Implied volatility of a credit index option, given a target quote and forward.
* @param [in] creditModelName	A cached credit-model handle
* @param [in] optionLVB		Strike, expiry date, CDS coupon, target quote etc, as a label/value block
* @returns	The implied volatility
*/
double aqCreditObjectIndexOptionImpliedVol( const std::string& creditModelName, const SWIG_STRINGMATRIX& optionLVB );

/* @brief Vega of a credit index option.
* @param [in] creditModelName	A cached credit-model handle
* @param [in] optionLVB		Strike, expiry date, CDS coupon, volatility etc, as a label/value block
* @param [in] volatilityBump	The volatility bump size
* @returns	The calculated option vega
*/
double aqCreditObjectIndexOptionVega( const std::string& creditModelName, const SWIG_STRINGMATRIX& optionLVB, const double volatilityBump );

/* @brief CS01 of a credit index option.
* @param [in] creditModelName	A cached credit-model handle
* @param [in] optionLVB		Strike, expiry date, CDS coupon, volatility etc, as a label/value block
* @returns	The calculated option CS01
*/
double aqCreditObjectIndexOptionCS01( const std::string& creditModelName, const SWIG_STRINGMATRIX& optionLVB );

/* @brief Theta of a credit index option.
* @param [in] creditModelName	A cached credit-model handle
* @param [in] optionLVB		Strike, expiry date, CDS coupon, volatility etc, as a label/value block
* @returns	The calculated option theta
*/
double aqCreditObjectIndexOptionTheta( const std::string& creditModelName, const SWIG_STRINGMATRIX& optionLVB );

/* @brief Creates a Credit Basket-Model, constructed from underlying Credit Models.
* @param [in] objectName	The name of the Credit Basket-Model object to create
* @param [in] key1			Name of the first data block, e.g. MODEL_PROPERTIES
* @param [in] dataBlock1	First data block
* @param [in] key2			Name of the second data block, e.g. CREDIT_MODELS (CreditModelName + correlationBeta per row)
* @param [in] dataBlock2	Second data block
* @returns	The objectName
*/
// dataBlock1/dataBlock2 are arbitrary-shape data blocks (no fixed column count); R has no way to
// unflatten a generic-shape matrix (see the matching note in aqToolData.h), so this is excluded there.
#if (!defined(SWIG_R)) && (!defined(SWIGR))
std::string aqCreditBasketModelCreate( const std::string& objectName,
                                        const std::string& key1, const SWIG_STRINGMATRIX& dataBlock1,
                                        const std::string& key2, const SWIG_STRINGMATRIX& dataBlock2 );
#endif

/* @brief Survival probability between two dates, computed by a cached credit basket model.
* @param [in] creditBasketModelName	A cached credit-basket-model handle
* @param [in] toDate					The date survival is measured to
* @param [in] fromDate				The date survival is measured from
* @returns	The survival probability
*/
double aqCreditBasketModelSurvivalProbability( const std::string& creditBasketModelName, const std::string& toDate, const std::string& fromDate );

/* @brief Create and store a CDS fee (premium) leg; returns its handle.
* @param [in] legObjectName	Name for the fee-leg object
* @param [in] feeProperties	Fee leg properties as a label/value block
* @param [in] feeScheduleLVB	The fee schedule cashflows
* @param [in] validateKeys	Optional. Default TRUE. Check feeProperties' keys
* @returns	The fee-leg object handle
*/
std::string aqCreditObjectFeeLegCreate( const std::string& legObjectName, const SWIG_STRINGMATRIX& feeProperties,
                                         const SWIG_STRINGMATRIX& feeScheduleLVB, const bool validateKeys = true );

/* @brief Create and store a CDS fee schedule from a cashflow matrix; returns its handle.
* @param [in] scheduleName		Name for the fee-schedule object
* @param [in] feeScheduleLVB	The fee schedule cashflows
* @param [in] validateKeys		Optional. Default TRUE. Check the schedule keys
* @returns	The fee-schedule object handle
*/
std::string aqCreditObjectFeeScheduleCreate( const std::string& scheduleName, const SWIG_STRINGMATRIX& feeScheduleLVB, const bool validateKeys = true );
