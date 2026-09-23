// aqCDSObject.h

/*
 * @brief			Swig interface for aqCDSObject... functions that price a cached
 *					credit default swap directly off a hazard rate, plus the
 *					credit-model-driven forms that were not yet exposed alongside
 *					aqCDSObjectPV / RiskyAnnuity / CS01 / ParSpread in aqCreditObject.h.
 *					See src/AQ_XLL/src/xllCredit.cpp for the marshalling reference.
 */

#pragma once
#include "SwigTypes.h"
#include "AQLCoreTemplateType.h"     // Data TypeDefs
#include <string>
#include <vector>


/* @brief validation interface for the aqCDSObjectPVFromHazardRate method.
*   Calculates the PV of the specified Credit Default Swap, given a hazard rate directly (no credit model needed).
*
* @param[in]	swapName				Swap object name
* @param[in]	valuationSettingsLVB	ModelName, CurveCollection, ValuationDate etc, as a label/value block
* @param[in]	hazardRate				The CDS hazard rate, used to calculate survival probabilities
* @param[in]	recoveryRate			The estimated amount of capital recovered after default
* @param[in]	legName					If specified, calculate the PV of the single swap leg
* @param[in]	includeAccruedInterest	Specifies whether the accruedInterest should be included in the PV
* @returns	The calculated PV value
*/
double aqCDSObjectPVFromHazardRate( const std::string& swapName,
                                     const std::vector<std::vector<std::string> >& valuationSettingsLVB,
                                     const double hazardRate,
                                     const double recoveryRate,
                                     const std::string& legName,
                                     const bool includeAccruedInterest = true );

/* @brief	validation interface for the aqCDSObjectPVByIntegration method.
*			Calculates the total PV of all the Credit Default Swap Legs, by integrating the payoff over survival probability.
*
* @param[in]	swapName							Swap object name
* @param[in]	creditModelName						Credit Model object name
* @param[in]	legName								If specified, calculate the PV of the single swap leg
* @param[in]	numberOfIntegrationPoints			Specifies the number of (x,y) points to use in the numerical integration
* @param[in]	evaluateInParallel					When TRUE, evaluate loops in parallel, where possible.
* @param[in]	payDefaultCashflowsOnNextCouponDate	When default occurs, whether to pay out the protection and accrued interest immediately, or wait to the next coupon date.
* @returns	The calculated PV value
*/
double aqCDSObjectPVByIntegration( const std::string& swapName,
                                    const std::string& creditModelName,
                                    const std::string& legName,
                                    const size_t numberOfIntegrationPoints,
                                    const bool evaluateInParallel = true,
                                    const bool payDefaultCashflowsOnNextCouponDate = true );

/* @brief	validation interface for the aqCDSObjectPVByMonteCarlo method.
*			Calculates the total PV of all the Credit Default Swap Legs, by a Monte-Carlo simulation over survival probability.
*
* @param[in]	swapName							Swap object name
* @param[in]	creditModelName						Credit Model object name
* @param[in]	legName								If specified, calculate the PV of the single swap leg
* @param[in]	mcParametersLVB						Monte-Carlo / random-number-generator parameters, as a label/value block
* @param[in]	payDefaultCashflowsOnNextCouponDate	When default occurs, whether to pay out the protection and accrued interest immediately, or wait to the next coupon date.
* @returns	A 2-element vector: [PV, Monte-Carlo standard error]
*/
std::vector<double> aqCDSObjectPVByMonteCarlo( const std::string& swapName,
                                                const std::string& creditModelName,
                                                const std::string& legName,
                                                const std::vector<std::vector<std::string> >& mcParametersLVB,
                                                const bool payDefaultCashflowsOnNextCouponDate = true );

/* @brief Calculates the risky annuity of the specified Credit Default Swap Premium leg, given a hazard rate directly.
*
* @param [in]	swapName				The name of the credit default swap object
* @param[in]	valuationSettingsLVB	ModelName, CurveCollection, ValuationDate etc, as a label/value block
* @param[in]	hazardRate				The CDS hazard rate, used to calculate survival probabilities
* @param[in]	recoveryRate			The estimated amount of capital recovered after default
* @param[in]	legName					The Premium leg to use when calculating the risky annuity. A mandatory parameter
* @param[in]	includeAccruedInterest	Specifies whether the accruedInterest should be included in the risky annuity
* @returns	The risky annuity
*/
double aqCDSObjectRiskyAnnuityFromHazardRate( const std::string& swapName,
                                               const std::vector<std::vector<std::string> >& valuationSettingsLVB,
                                               const double hazardRate,
                                               const double recoveryRate,
                                               const std::string& legName,
                                               const bool includeAccruedInterest = true );

/* @brief[in]	Computes the accrued year fraction from the previous coupon date to the specified date, priced off a cached credit model.
*				Used in accrued interest calculations.
*
* @param [in]	swapName			The name of the credit default swap object
* @param[in]	creditModelName		Credit Model object name
* @param[in]	toDate				The date to which we wish to calculate the year fraction, as a YYYYMMDD string
* @param[in]	legName				The Premium leg name
* @returns: The year fraction
*/
double aqCDSObjectAccruedYearFraction( const std::string& swapName,
                                        const std::string& creditModelName,
                                        const std::string& toDate,
                                        const std::string& legName );

/* @brief Calculates the par spread of the specified Credit Default Swap, given a hazard rate directly.
*
* @param[in]	swapName				The name of the credit default swap object
* @param[in]	valuationSettingsLVB	A single collection name, or a curveCollection per leg, as a label/value block
* @param[in]	hazardRate				The CDS hazard rate, used to calculate survival probabilities
* @param[in]	recoveryRate			The estimated amount of capital recovered after default
* @param[in]	premiumLegName			The Premium leg name of the CDS
* @param[in]	protectionLegName		The Protection leg name of the CDS
* @param[in]	includeAccruedInterest	Specifies whether the accruedInterest should be included in the risky annuity
* @returns	The CDS par spread
*/
double aqCDSObjectParSpreadFromHazardRate( const std::string& swapName,
                                            const std::vector<std::vector<std::string> >& valuationSettingsLVB,
                                            const double hazardRate,
                                            const double recoveryRate,
                                            const std::string& premiumLegName,
                                            const std::string& protectionLegName,
                                            const bool includeAccruedInterest = true );

/* @brief Solves for the hazard rate, given the specified CDS par spread, using a valuation-settings label/value block directly
*		 (rather than a cached credit model - see the file header in xllCredit.cpp for why the credit-model-driven
*		 overload of this same validation name is not exposed here).
*
* @param[in]	swapName				The name of the credit default swap object
* @param[in]	valuationSettingsLVB	A single collection name, or a curveCollection per leg, as a label/value block
* @param[in]	parSpread				The CDS par spread ( as a decimal )
* @param[in]	recoveryRate			The estimated amount of capital recovered after default
* @param[in]	premiumLegName			The Premium leg name of the CDS
* @param[in]	protectionLegName		The Protection leg name of the CDS
* @param[in]	includeAccruedInterest	Specifies whether the accruedInterest should be included in the risky annuity
* @returns	The CDS hazard rate
*/
double aqCDSObjectHazardRateFromParSpread( const std::string& swapName,
                                            const std::vector<std::vector<std::string> >& valuationSettingsLVB,
                                            const double parSpread,
                                            const double recoveryRate,
                                            const std::string& premiumLegName,
                                            const std::string& protectionLegName,
                                            const bool includeAccruedInterest = true );
