/*
 * @brief			validation interface for the meProductSwapPV method
 * @Created:		27 April 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "LACoreTemplateType.h"
#include "LabelValueBlock.h"
#include "Swap.h"
#include "JSONInfoBlock.h"   // JSON Info Blocks

using etrading::LabelValueBlock;

namespace validation_api
{
	/* @brief			validation interface for the meLWOSwapPVs method
	*  @param [in]		swapNames		    Swap object names
	*  @param [in]		valuationSettingsLVB    A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		legNames	        Leg names
    *  @param [in]		fixingTableNames	Fixing table object names
    *  @param [in]		optimize            Optimize performance using OMP threading
	*  @return			Swap PV
	*/
	std::vector<double> tryMeLWOSwapPVs(const std::vector<std::string>& swapNames, const std::vector<LabelValueBlock>& valuationSettingsLVB, std::vector<std::string>& legNames, std::vector<LabelValueBlock>& fixingTableNames, const bool optimize);

    /* @brief			validation interface for the meLWOSwapPV method
	*  @param [in]		swapName		    Swap object name
	*  @param [in]		valuationSettingsLVB    A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		legName		        Leg name
    *  @param [in]		fixingTableNames	Fixing table object names
	*  @return			Swap PV
	*/
	double tryMeLWOSwapPV(const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const LAString& legName="", const LabelValueBlock& fixingTableNames=LabelValueBlock());

    /* @brief			validation interface for the meLWOSwapPV01 method
	*  @param [in]		swapName		    Swap object name
	*  @param [in]		valuationSettingsLVB    A LVB containing ModelName, CurveCollection, ValuationDate, etc.
    *  @param [in]		fixingTableNames	Fixing table object names
	*  @return			Swap PV01
	*/
	double tryMeLWOSwapPV01(const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames=LabelValueBlock());

    /* @brief			validation interface for the meLWOSwapParRate method
	*  @param [in]		swapName		        Swap name
	*  @param [in]		valuationSettingsLVB    A LVB containing ModelName, CurveCollection, ValuationDate, etc.
    *  @param [in]		fixingTableNames	    Fixing table object names
    *  @param [in]		legName             	(Optional) Leg Name
	*  @return			Swap par rate
	*/
	double tryMeLWOSwapParRate(const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames=LabelValueBlock(), const LAString& legName="" );

    /* @brief			validation interface for the meLWOSwapAnnuity method
	*  @param [in]		swapName		        Swap object name
	*  @param [in]		valuationSettingsLVB    A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		legName		            Leg name
	*  @return			Swap PV
	*/
	double tryMeLWOSwapAnnuity(const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const LAString& legName);

    /* @brief			validation interface for the meLWOSwapDisplayCashflows method
	*  @param [in]		swapName		    Swap name
	*  @param [in]		valuationSettingsLVB   A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		legName		        Leg name
    *  @param [in]		fixingTableNames	Fixing table object names
	*  @param [in]		showColumnHeaders   showColumnHeaders
	*  @param [in]	    columnList          Column header names to show specified columns. Default to empty list showing all columns.
	*  @return			Swap display
	*/
	std::vector<AnyTypeMatrix> tryMeLWOSwapDisplayCashflows(const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const LAString& legName="",
                                                            const LabelValueBlock& fixingTableNames=LabelValueBlock(), bool showColumnHeaders=true,
                                                            const std::vector<std::string>& columnList=std::vector<std::string>());


    /* @brief			validation interface for the meLWOSwapSpread method
	*  @param [in]		swapName		Swap name
	*  @param [in]		curveCollection A LVB containing ModelName, CurveCollection, ValuationDate, etc.
    *  @param [in]		fixingTableName	Fixing table object names
	*  @param [in]		spreadLegName   Leg name the spread will be applied to
	*  @return			Swap PV
	*/
	double tryMeLWOSwapSpread(const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames=LabelValueBlock(), const LAString& spreadLegName="");

    /* @brief			validation interface for the meLWOSwapParSpread method 
    *  @param [in]		swapName		Swap name
	*  @param [in]		curveCollection A LVB containing ModelName, CurveCollection, ValuationDate, etc.
    *  @param [in]		fixingTableName	Fixing table object names
	*  @param [in]		spreadLegName   Leg name the spread will be applied to
	*  @return			The spread that make the swap PV zero, without taking the legs' existing spreads into account
	*/
	double tryMeLWOSwapParSpread(const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames=LabelValueBlock(), const LAString& spreadLegName="");

	/* @brief			validation interface for the meLWOSwapAccruedInterest method
	*  @param [in]		swapName		Swap object name
	*  @param [in]		curveCollection Curve collection
	*  @param [in]		legName		    Leg name
    *  @param [in]		fixingTableName	Fixing table object name
	*  @return			Swap Leg's AccruedInterest
	*/
	double tryMeLWOSwapAccruedInterest(const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const LAString& legName, const LabelValueBlock& fixingTableName);

	/* @brief validation interface for the meLWOCreditDefaultSwapPVFromHazardRate method.
	*   Calculates the PV of the specified Credit Default Swap.
	*
	* @param[in]	swapName			Swap object name
	* @param[in]	curveCollection		The collection containing the OIS curve for discounting
	* @param[in]	hazardRate			The CDS hazard rate, used to calculate survival probabilities
	* @param[in]	recoveryRate		The estimated amount of capital recovered after default
	* @param[in]	legName			    If specified, calculate the PV of the single swap leg
	* @param[in]	includeAccruedInterest	Specifies whether the accruedInterest should be included in the PV
	* @returns	The calculated PV value
	*/
    double tryMeLWOCreditDefaultSwapPVFromHazardRate( const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const double hazardRate, const double recoveryRate, const LAString& legName, const bool includeAccruedInterest );

	/* @brief validation interface for the meLWOCreditDefaultSwapPV method.
	*   Calculates the PV of the specified Credit Default Swap.
	*
	* @param[in]	swapName			Swap object name
	* @param[in]	creditModelName		Credit Model object name
	* @param[in]	legName			    If specified, calculate the PV of the single swap leg
	* @returns	The calculated PV value
	*/
	double tryMeLWOCreditDefaultSwapPV( const std::string& swapName, const std::string& creditModelName,  const LAString& legName );

	/* @brief	validation interface for the meLWOCreditDefaultSwapPVByIntegration method.
	*			Calculates the total PV of all the Credit Default Swap Legs, by integrating the payoff over survivial probability.
	*			The integration over survival probability is equivalent to an integration over survival time, with an appropriate change of variable.
	*
	* @param[in]	swapName							Swap object name
	* @param[in]	creditModelName						Credit Model object name
	* @param[in]	legName								If specified, calculate the PV of the single swap leg
	* @param[in]	numberOfIntegrationPoints			Specifies the number of (x,y) points to use in the numerical integration
	* @param[in]	evaluateInParallel					When TRUE, evaluate loops in parallel, where possible.
	* @param[in]	payDefaultCashflowsOnNextCouponDate	When default occurs, whether to pay out the protection and accrued interest immediately, or wait to the next coupon date.
	*													TRUE means wait to the next coupon date. This flag is used to match the PV by integration to the analytic PV formula.
	* @returns	The calculated PV value
	*/
	double tryMeLWOCreditDefaultSwapPVByIntegration( const std::string& swapName, const std::string& creditModelName, const LAString& legName, const size_t numberOfIntegrationPoints, const bool evaluateInParallel, const bool payDefaultCashflowsOnNextCouponDate );

	/* @brief	validation interface for the meLWOCreditDefaultSwapPVByMonteCarlo method.
	*			Calculates the total PV of all the Credit Default Swap Legs, by a monte-carlo simulation over survivial probability.
	*			The monte-carlo over survival probability is equivalent to a monte-carlo over survival time, with an appropriate change of variable.
	*
	* @param[in]	swapName							Swap object name
	* @param[in]	creditModelName						Credit Model object name
	* @param[in]	legName								If specified, calculate the PV of the single swap leg
	* @param[in]	mcParametersLVB						A label value block specifying Monte-Carlo / Random number generator parameters. 
	* @param[in]	payDefaultCashflowsOnNextCouponDate	When default occurs, whether to pay out the protection and accrued interest immediately, or wait to the next coupon date.
	*													TRUE means wait to the next coupon date. This flag is used to match the PV by integration to the analytic PV formula.
	* @param[out]	standardError						The Monte-Carlo standard error in the result.
	*
	* @returns	The calculated PV value
	*/
	double tryMeLWOCreditDefaultSwapPVByMonteCarlo( const std::string& swapName, const std::string& creditModelName, const LAString& legName, const LabelValueBlock& mcParametersLVB, const bool payDefaultCashflowsOnNextCouponDate, double& standardError );

	/* @brief Calculates the risky annuity of the specified Credit Default Swap Premium leg.
	*
	* @param [in]	swapName			The name of the credit default swap object
	* @param[in]	curveCollection		The collection containing the OIS curve for discounting
	* @param[in]	hazardRate			The CDS hazard rate, used to calculate survival probabilities
	* @param[in]	recoveryRate		The estimated amount of capital recovered after default
	* @param[in]	legName				The Premium leg to use when calculating the risky annuity. A mandatory parameter
	* @param[in]	includeAccruedInterest	Specifies whether the accruedInterest should be included in the risky annuity
	* @returns	The risky annuity
	*/
	double tryMeLWOCreditDefaultSwapRiskyAnnuityFromHazardRate( const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const double hazardRate, const double recoveryRate, const LAString& legName, const bool includeAccruedInterest );

	/* @brief Calculates the risky annuity of the specified Credit Default Swap Premium leg.
	*
	* @param [in]	swapName			The name of the credit default swap object
	* @param[in]	creditModelName		Credit Model object name
	* @param[in]	legName				The Premium leg to use when calculating the risky annuity. A mandatory parameter
	* @returns	The risky annuity
	*/
	double tryMeLWOCreditDefaultSwapRiskyAnnuity( const std::string& swapName, const std::string& creditModelName, const LAString& legName );

	/* @brief[in]	Computes the accrued year fraction from the previous coupon date to the specified date
	*				Used in accrued interest calculations.
	*
	* @param [in]	swapName			The name of the credit default swap object
	* @param[in]	creditModelName		Credit Model object name
	* @param[in]	toDate				The date to which we wish to calculate the year fraction
	* @param[in]	legName				The Premium leg name
	* @returns: The year fraction
	*/
	double tryMeLWOCreditDefaultSwapAccruedYearFraction( const std::string& swapName, const std::string& creditModelName, const LADate& toDate, const LAString& legName );

    /* @brief Calculates the CS01 of the specified Credit Default Swap Premium leg.
	*
	* @param [in]	swapName			The name of the credit default swap object
	* @param[in]	creditModelName		Credit Model object name
	* @param[in]	legName				The Premium leg to use when calculating the risky annuity. A mandatory parameter
	* @returns	The risky annuity
	*/
	double tryMeLWOCreditDefaultSwapCS01( const std::string& swapName, const std::string& creditModelName, const LAString& legName );

	/* @brief Calculates the par spread of the specified Credit Default Swap.
	*
	* @param[in]	swapName				The name of the credit default swap object
	* @param[in]	valuationSettingsLVB		A label value block containing a single collection name or a curveCollection per leg
	* @param[in]	hazardRate				The CDS hazard rate, used to calculate survival probabilities
	* @param[in]	recoveryRate			The estimated amount of capital recovered after default
	* @param[in]	premiumLegName			The Premium leg name of the CDS
	* @param[in]	protectionLegName		The Protection leg name of the CDS
	* @param[in]	includeAccruedInterest	Specifies whether the accruedInterest should be included in the risky annuity
	* @returns	The CDS par spread
	*/
	double tryMeLWOCreditDefaultSwapParSpreadFromHazardRate( const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const double hazardRate, const double recoveryRate, const LAString& premiumLegName, const LAString& protectionLegName, const bool includeAccruedInterest );

	/* @brief Calculates the par spread of the specified Credit Default Swap.
	*
	* @param[in]	swapName				The name of the credit default swap object
	* @param[in]	creditModelName			Credit Model object name
	* @param[in]	premiumLegName			The Premium leg name of the CDS
	* @param[in]	protectionLegName		The Protection leg name of the CDS
	* @returns	The CDS par spread
	*/
	double tryMeLWOCreditDefaultSwapParSpread( const std::string& swapName, const std::string& creditModelName, const LAString& premiumLegName, const LAString& protectionLegName );

	/* @brief Solves for the hazard rate, given the specified CDS par spread
	*
	* @param[in]	swapName				The name of the credit default swap object
	* @param[in]	valuationSettingsLVB		A label value block containing a single collection name or a curveCollection per leg
	* @param[in]	parSpread				The CDS par spread ( as a decimal )
	* @param[in]	recoveryRate			The estimated amount of capital recovered after default
	* @param[in]	premiumLegName			The Premium leg name of the CDS
	* @param[in]	protectionLegName		The Protection leg name of the CDS
	* @param[in]	includeAccruedInterest	Specifies whether the accruedInterest should be included in the risky annuity
	* @returns	The CDS hazard rate
	*/
	double tryMeLWOCreditDefaultSwapHazardRateFromParSpread( const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const double parSpread, const double recoveryRate, const LAString& premiumLegName, const LAString& protectionLegName, const bool includeAccruedInterest );

	/* @brief Solves for the hazard rate, given the specified CDS par spread
	*
	* @param[in]	swapName				The name of the credit default swap object
	* @param[in]	creditModelName			Credit Model object name
	* @param[in]	premiumLegName			The Premium leg name of the CDS
	* @param[in]	protectionLegName		The Protection leg name of the CDS
	* @returns	The CDS hazard rate
	*/
	double tryMeLWOCreditDefaultSwapHazardRateFromParSpread( const std::string& swapName, const std::string& creditModelName, const double parSpread, const LAString& premiumLegName, const LAString& protectionLegName );

	typedef std::tuple<std::vector<std::string>, std::vector<etrading::ContainedTypeEnum>, etrading::VariantMatrix>  TableInfo;

    /* @brief Creates a CreditModel, calibrated from CDS or bond quotes
	 * @param [in] objectName        The name of the Credit Model object to create
	 * @param [in] propertyNames     A vector of property names corresponding to each label-value block of properties.
	 *                               MODEL_PROPERTIES, ( CDS_MARKETDATA or BOND_MARKETDATA )
	 * @param [in] infoBlocks        A vector of containing the label-value blocks of properties
	 * @param [out]                  The objectName
	 */
    std::string tryMeLWOCreditModelCreate( const std::string& objectName,
                                           const std::vector<std::string>& dataBlockNames,
                                           const etrading::JSONInfoBlockTuples& infoBlocks ); // TODO: Move the JSON Info Block below the validation_api - does not belong here

	/*  @brief Obtains the credit model asOf / valuation Date
	 *  @param[in] creditModelName	Credit Model object name
	 *  @returns The asOfDate.
	 */
	LADate tryMeLWOCreditModelAsOfDate( const std::string& creditModelName );

	 /* @brief Obtains the calibration parameters from the specified credit model
	 * @param[in] creditModelName	Credit Model object name
	 * @param[out]                  A matrix containing payment dates and hazard rates
	 */
	AnyTypeMatrix tryMeLWOCreditModelCalibrationParameters( const std::string& creditModelName );

    // Extracts the Hazard Rate from the Credit Model
	double tryMeLWOCreditModelHazardRate( const std::string& creditModelName, const LADate& paymentDate );

    // Extracts the Survival Probability from the Credit Model
	double tryMeLWOCreditModelSurvivalProbability( const std::string& creditModelName, const LADate& toDate, const LADate& fromDate );

    // Extracts the Default Probability from the Credit Model
	double tryMeLWOCreditModelDefaultProbability( const std::string& creditModelName, const LADate& toDate, const LADate& fromDate );

	/* @brief	Given a survival probability, calculate the implied survival date from the specified credit model.
	*			i.e. this function is the inverse of getSurvivalProbability().
	*
	*  @param[in]	creditModelName			Credit Model object name
	*  @param[in]	survivalProbability		The input survivalProbability
	*
	*  @returns		The survival date corresponding to the input survivalProbability.
	*/ 
	LADate tryMeLWOCreditModelImpliedSurvivalDate( const std::string& creditModelName, const double survivalProbability );

    // Calculates the Risky Discount Factor from the Credit Model
	DoubleVector tryMeLWOCreditModelRiskyDiscountFactors( const std::string& creditModelName, const DateVector&  paymentDates );

	/* @brief validation interface for the meLWOCreditSpread method.
	*  Calculates the Forward spread between start date and end date implied by the credit model.
	*  If the startDate is the model AsOf Date, returns the spot spread.
	*  See article "Credit Derivatives Handbook" by JPM p20-21.
	*
	* @param[in]	creditModelName		Credit Model object name
	* @param[in]	StartDate			The date on which credit protection begins
	* @param[in]	EndDate				The date on which credit protection ends
	* @returns	The calculated forward spread
	*/
	double tryMeLWOCreditSpread( const std::string& creditModelName, const LADate& startDate, const LADate& endDate );

	/* @brief validation interface for the meLWOCreditSpread method.
	*		Given a start and end date, calculates the forward spread of a credit index, implied by the credit model.
	*		This calculation follows the Bloomberg approach for a Credit Index.
	*		See Bloomberg whitepaper "Pricing Credit Index Options", 1 March 2012
	*		In summary: Protection leg starts immediately; annuity leg starts on forward start date.
	*
	* @param[in]	creditModelName		Credit Model object name
	* @param[in]	StartDate			The date on which credit protection begins
	* @param[in]	EndDate				The date on which credit protection ends
	* @returns	The calculated forward spread
	*/
	double tryMeLWOCreditIndexSpread( const std::string& creditModelName, const LADate& startDate, const LADate& endDate );

	/* @brief validation interface for the meLWOCreditOptionPV method.
	*  Calculates the value of a credit option i.e. option on a credit default swap instrument.
	*  See article "The Valuation of Credit Default Swap Options" by Hull and White
	*
	* @param[in]	creditModelName		Credit Model object name
	* @param[in]	payerReceiver		Whether the option is a payer (PUT, the right to sell risk, pay CDS spread for protection)
	* 									or receiver (CALL, buy risk, receive CDS spread)
	* @param[in]	strike				The strike spread
	* @param[in]	optionExpiryDate	The expiry date of the option; the effective date of the underlying CDS
	* @param[in]	cdsMaturityDate		The maturity date of the underlying CDS
	* @param[in]	volatility			The volatility of the underlying CDS spread
	* @returns	The calculated option price
	*/
	double tryMeLWOCreditOptionPV( const std::string& creditModelName, const std::string& payerReceiver, const double strike, const LADate& optionExpiryDate, const LADate& cdsMaturityDate, const double volatility );

	/* @brief validation interface for the meLWOCreditOptionPV method.
	*  Calculates the value of a credit option i.e. option on a CDS instrument
	*  See article "The Valuation of Credit Default Swap Options" by Hull and White
	*
	* @param[in]	creditModelName				Credit Model object name
	* @param[in]	payerReceiver				Whether the option is a payer (PUT, the right to sell risk, pay CDS spread for protection)
	* 											or receiver (CALL, buy risk, receive CDS spread)
	* @param[in]	strike						The strike spread
	* @param[in]	optionExpiryDate			The expiry date of the option; the effective date of the underlying CDS
	* @param[in]	cdsStartDate				If the option is exercised, the date on which the underlying CDS is delivered
	* @param[in]	cdsMaturityDate				The maturity date of the underlying CDS
	* @param[in]	volatility					The volatility of the underlying CDS spread
	* @param[in]	forwardSpread				The forward spread at the option expiry date
	* @returns	The calculated option price
	*/
	double tryMeLWOCreditOptionPVFromForward( const std::string& creditModelName, const std::string& payerReceiver, const double strike, const LADate& optionExpiryDate, const LADate& cdsStartDate, const LADate& cdsMaturityDate, const double volatility, const double forwardSpread );

	/* @brief validation interface for the meLWOCreditIndexOptionPV method.
	*  Calculates the value of a credit index option i.e. option on a CDS index
	*  See Bloomberg Whitepaper "Pricing Credit Index Options", March 1st, 2012
	*
	* @param[in]	creditModelName				Credit Model object name
	* @param[in]	optionLVB					Option parameters such as strike, expiryDate, cdsCoupon, volatility
	* @returns	The calculated option price
	*/
	double tryMeLWOCreditIndexOptionPV( const std::string& creditModelName, const etrading::LabelValueBlock& optionLVB );

	/* @brief  validation interface for the meLWOCreditIndexOptionImpliedVol method.
	*  Calculates the implied vol of a credit index option, given a target CDS index option quote and CDS index forward at option expiry
	*  See Bloomberg Whitepaper "Pricing Credit Index Options", March 1st, 2012
	*
	* @param[in]	creditModelName				Credit Model object name
	* @param[in]	optionLVB					Option parameters such as strike, expiryDate, cdsCoupon, volatility
	* @returns	The implied volatility
	*/
	double tryMeLWOCreditIndexOptionImpliedVol( const std::string& creditModelName, const etrading::LabelValueBlock& optionLVB );

	/* @brief validation interface for the meLWOCreditIndexOptionVega method.
	*  Calculates the value of a credit index option i.e. option on a CDS index
	*  See Bloomberg Whitepaper "Pricing Credit Index Options", March 1st, 2012
	*
	* @param[in]	creditModelName				Credit Model object name
	* @param[in]	optionLVB					Option parameters such as strike, expiryDate, cdsCoupon, volatility
	* @param[in]	volatilityBump				The volatility bump size
	* @returns	The calculated option vega
	*/
	double tryMeLWOCreditIndexOptionVega( const std::string& creditModelName, const etrading::LabelValueBlock& optionLVB, const double volatilityBump );

	/* @brief validation interface for the meLWOCreditIndexOptionCS01 method.
	*  Calculates the value of a credit index option i.e. option on a CDS index
	*  See Bloomberg Whitepaper "Pricing Credit Index Options", March 1st, 2012
	*
	* @param[in]	creditModelName				Credit Model object name
	* @param[in]	optionLVB					Option parameters such as strike, expiryDate, cdsCoupon, volatility
	* @returns	The calculated option CS01
	*/
	double tryMeLWOCreditIndexOptionCS01( const std::string& creditModelName, const etrading::LabelValueBlock& optionLVB );

	/* @brief validation interface for the meLWOCreditIndexOptionTheta method.
	*  Calculates the value of a credit index option i.e. option on a CDS index
	*  See Bloomberg Whitepaper "Pricing Credit Index Options", March 1st, 2012
	*
	* @param[in]	creditModelName				Credit Model object name
	* @param[in]	optionLVB					Option parameters such as strike, expiryDate, cdsCoupon, volatility
	* @returns	The calculated option theta
	*/
	double tryMeLWOCreditIndexOptionTheta( const std::string& creditModelName, const etrading::LabelValueBlock& optionLVB );

	/* @brief Calculates the implied vol of a credit option, given a target CDS option quote
	*  See article "The Valuation of Credit Default Swap Options" by Hull and White
	*
	* @param[in]	creditModelName		Credit Model object name
	* @param[in]	payerReceiver		Whether the option is a payer (PUT, the right to sell risk, pay CDS spread for protection)
	* 									or receiver (CALL, buy risk, receive CDS spread)
	* @param[in]	strike				The strike spread
	* @param[in]	optionExpiryDate	The expiry date of the option; the effective date of the underlying CDS
	* @param[in]	cdsMaturityDate		The maturity date of the underlying CDS
	* @param[in]	targetOptionValue	Calculate the implied vol for this target option value
	* @returns	The implied volatility
	*/
	double tryMeLWOCreditOptionImpliedVol( const std::string& creditModelName, const std::string& payerReceiver, const double strike, const LADate& optionExpiryDate, const LADate& cdsMaturityDate, const double targetOptionValue );


	/* @brief Calculates the implied vol of a credit option, given a target CDS option quote and a CDS forward spread.
	*  See article "The Valuation of Credit Default Swap Options" by Hull and White
	*
	* @param[in]	creditModelName		Credit Model object name
	* @param[in]	payerReceiver		Whether the option is a payer (PUT, the right to sell risk, pay CDS spread for protection)
	* 									or receiver (CALL, buy risk, receive CDS spread)
	* @param[in]	strike				The strike spread
	* @param[in]	optionExpiryDate	The expiry date of the option; the effective date of the underlying CDS
	* @param[in]	cdsStartDate		If the option is exercised, the date on which the underlying CDS is delivered
	* @param[in]	cdsMaturityDate		The maturity date of the underlying CDS
	* @param[in]	targetOptionValue	Calculate the implied vol for this target option value
	* @param[in]	forwardSpread		The CDS forward spread at the option expiry date
	* @returns	The implied volatility
	*/
	double tryMeLWOCreditOptionImpliedVolFromForward( const std::string& creditModelName, const std::string& payerReceiver, const double strike, const LADate& optionExpiryDate, const LADate& cdsStartDate, const LADate& cdsMaturityDate, const double targetOptionValue, const double forwardSpread );

    /* @brief Creates a Credit Basket-Model, constructed from underlying Credit Models
	 * @param [in] objectName        The name of the Credit Basket-Model object to create
	 * @param [in] propertyNames     A vector of property names corresponding to each label-value block of properties.
	 *                               MODEL_PROPERTIES, CREDIT_MODELS.
	 * @param [in] infoBlocks        A vector of containing the label-value blocks of properties.
	 *                               The CREDIT_MODELS block consists of CreditModelName and correlationBeta.
	 * @param [out]                  The objectName
	 */
	std::string tryMeLWOCreditBasketModelCreate( const std::string& objectName,
												 const std::vector<std::string>& dataBlockNames,
												 const etrading::JSONInfoBlockTuples& infoBlocks ); // TODO: Move the JSON Info Block below the validation_api - does not belong here


	// Compute the Survival Probability using the Credit Basket Model
	double tryMeLWOCreditBasketModelSurvivalProbability( const std::string& creditBasketModelName, const LADate& toDate, const LADate& fromDate );

	/* @brief validation interface for the meLWOConstantMaturitySwapPVFromConvexity() method.
	*   Calculates the PV of a Constant Maturity Swap.
	*
	* @param[in]	swapName			Swap object name
	* @param[in]	valuationSettingsLVB	A label value block containing a single collection name or a curveCollection per leg
	* @param[in]	convexityAdjustment	The convexity adjustment which is added to the PV of the CMS leg.
	* @param[in]	legName			    If specified, calculate the PV of the single swap leg
	* @param[in]	fixingTableNames	Fixing table object names
	* @returns	The calculated PV value
	*/
	double tryMeLWOConstantMaturitySwapPVUsingConvexityAdjustment( const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const double convexityAdjustment, const LAString& legName, const LabelValueBlock& fixingTableNames=LabelValueBlock() );

	/* @brief validation interface for the meLWOConstantMaturitySwapParRateFromConvexity() method.
	*   Calculates the ParRate of a Constant Maturity Swap.
	*
	* @param[in]	swapName			Swap object name
	* @param[in]	valuationSettingsLVB	A label value block containing a single collection name or a curveCollection per leg
	* @param[in]	convexityAdjustment	The convexity adjustment which is added to the PV of the CMS leg.
	* @param[in]	fixingTableNames	Fixing table object names
	* @returns	The calculated ParRate value
	*/
	double tryMeLWOConstantMaturitySwapParRateUsingConvexityAdjustment( const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const double convexityAdjustment, const LabelValueBlock& fixingTableNames=LabelValueBlock() );

	/* @brief validation interface for the meLWOTotalReturnSwapPV method.
	*   Calculates the PV of the specified Total Return Swap.
	*
	* @param[in]	swapName			The name of the total return swap object
	* @param[in]	creditModelName		Credit Model object name
	* @param[in]	legName			    If specified, calculate the PV of the single swap leg
	* @param[in]	fixingTableNames	Fixing table object names
	* @returns	The calculated PV value
	*/
	double tryMeLWOTotalReturnSwapPV( const std::string& swapName, const std::string& creditModelName,  const std::string& legName="", const LabelValueBlock& fixingTableNames=LabelValueBlock() );

	/* @brief Calculates the par rate of the specified Total Return Swap's premium leg.
	*
	* @param[in]	swapName				The name of the total return swap object
	* @param[in]	creditModelName			Credit Model object name
	* @param[in]	fixingTableNames		Fixing table object names
	* @returns	The par rate of the TRS premium leg
	*/
	double tryMeLWOTotalReturnSwapParRate( const std::string& swapName, const std::string& creditModelName, const LabelValueBlock& fixingTableNames=LabelValueBlock() );

	/* @brief Calculates the par spread of the specified Total Return Swap's float leg.
	*
	* @param[in]	swapName				The name of the total return swap object
	* @param[in]	creditModelName			Credit Model object name
	* @param[in]	fixingTableNames		Fixing table object names
	* @returns	The par spread of the TRS float leg
	*/
	double tryMeLWOTotalReturnSwapParSpread( const std::string& swapName, const std::string& creditModelName, const LabelValueBlock& fixingTableNames=LabelValueBlock() );

	/* @brief validation interface for the meLWOTotalReturnSwapPV method.
	*   Calculates the annity of the specified Total Return Swap leg
	*
	* @param[in]	swapName			The name of the total return swap object
	* @param[in]	creditModelName		Credit Model object name
	* @param[in]	legName			    Mandatory, calculate the annuity of this leg.
	* @returns	The calculated annuity value
	*/
	double tryMeLWOTotalReturnSwapAnnuity( const std::string& swapName, const std::string& creditModelName,  const std::string& legName );


}
    