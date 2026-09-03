#pragma once

#include <string>
#include <vector>
#include "Swap.h"

#include "CreditModel.h"
#include "CDSLeg.h"
#include "MonteCarloStatistics.h"

namespace etrading
{

    class CreditDefaultSwap : public Swap 
    {
	public:
		CreditDefaultSwap(const std::string& instanceName, const LegPtr& leg1, const LegPtr& leg2, const LabelValueBlock& swapPropertiesLVB);
   		CreditDefaultSwap(const CreditDefaultSwap& rhs);

		virtual ~CreditDefaultSwap() {}
        
        std::shared_ptr<Swap> clone();

		/* @brief Calculates the total PV of all of the Credit Default Swap legs.
		*
		* @param[in]	valuationSettingsLVB	The valuation settings containing curveCollections, and valuation date
		* @param[in]	hazardRate				The CDS hazard rate, used to calculate survival probabilities
		* @param[in]	recoveryRate			The estimated amount of capital recovered after default
		* @param[in]	legName					Optionally calculate the PV of the specified leg only.
		* @param[in]	includeAccruedInterest	Specifies whether the accruedInterest should be included in the PV
		* @returns	The calculated PV value
		*/
		double pvFromHazardRate( const LabelValueBlock& valuationSettingsLVB, const double hazardRate, const double recoveryRate, const AQLString& legName, const bool includeAccruedInterest ) const;

		/* @brief Calculates the total PV of all of the Credit Default Swap legs.
		*
		* @param[in]	creditModel				The calibrated credit model
		* @param[in]	legName					Optionally calculate the PV of the specified leg only.
		* @returns	The calculated PV value
		*/
		double pv( const CreditModel& creditModel, const AQLString& legName ) const;

		/* @brief	Calculates the total PV of all the Credit Default Swap Legs, by integrating the payoff over survivial probability.
		*			The integration over survival probability is equivalent to an integration over survival time, with an appropriate change of variable.
		*			The PV of coupon payments in each leg is calculated by assuming they are paid out with certainty, up to the default time.
		*			When default occurs after the maturity of the CDS there is no more protection payout.
		*
		* @param[in]	creditModel							The calibrated credit model
		* @param[in]	legName								Optionally calculate the PV of the specified leg only.
		* @param[in]	numberOfIntegrationPoints			Specifies the number of (x,y) points to use in the numerical integration
		* @param[in]	evaluateInParallel					When TRUE, evaluate loops in parallel, where possible.
		* @param[in]	payDefaultCashflowsOnNextCouponDate	When default occurs, whether to pay out the protection and accrued interest immediately, or wait to the next coupon date.
		*													TRUE means wait to the next coupon date. This flag is used to match the PV by integration to the analytic PV formula.
		* @returns	The calculated PV value
		*/
		double pvByIntegration( const CreditModel& creditModel, const AQLString& legName, const size_t numberOfIntegrationPoints, const bool evaluateInParallel, const bool payDefaultCashflowsOnNextCouponDate ) const;


		/* @brief	Calculates the total PV of all the Credit Default Swap Legs, by a monte-carlo simulation over survivial probability.
		*			The monte-carlo over survival probability is equivalent to a monte-carlo over survival time, with an appropriate change of variable.
		*			The PV of coupon payments in each leg is calculated by assuming they are paid out with certainty, up to the default time.
		*			When default occurs after the maturity of the CDS there is no more protection payout.
		*
		* @param[in]	creditModel							The calibrated credit model
		* @param[in]	legName								Optionally calculate the PV of the specified leg only.
		* @param[in]	mcParametersLVB						A label value block specifying Monte-Carlo / Random number generator parameters. 
		* @param[in]	payDefaultCashflowsOnNextCouponDate	When default occurs, whether to pay out the protection and accrued interest immediately, or wait to the next coupon date.
		*													TRUE means wait to the next coupon date. This flag is used to match the PV by integration to the analytic PV formula.
		* @param[out]	standardError						The Monte-Carlo standard error in the result.
		*
		* @returns	The calculated PV value
		*/
		double pvByMonteCarlo( const CreditModel& creditModel, const AQLString& legName, const LabelValueBlock& mcParametersLVB, const bool payDefaultCashflowsOnNextCouponDate, double& standardError ) const;


		/* @brief Calculate the PV of a CDS instrument which pays the specified fixedCoupon
		*
		* @param[in]	creditModel		The calibrated credit model
		* @param[in]	parSpread		The fair-value par-spread of the CDS instrument			
		* @param[in]	fixedCoupon		The coupon rate which standardises the CDS instrument, e.g. 100bps
		* @param[in]	assumeFlatCurve	If true, the ISDA flat curve approximation is used to calculate the risky annuity factor.
		*								If false, the calibrated term structure of hazard rates is used.
		*/
		double pvFromSpread( const CreditModel& creditModel, const double parSpread, const double fixedCoupon, const bool assumeFlatCurve ) const;

		/* @brief	Models the CDS spread as log-normal process. S_t = S0 exp( -0.5 vol^2 t + vol sqrt(T) )
		*			Given a normal variate taken from a standard normal distribution, returns the corresponding spread value
		*
		* @param[in]	meanSpreadEstimate	The mean of the log-normal process
		* @param[in]	normalVariateSample	A sample taken from a standard normal distribution
		* @param[in]	drift				The log-normal process drift term
		* @param[in]	volatilityTime		The volatility scaling factor multiplied by the normal variate
		*/
		double logNormalSpreadProcess( const double meanSpreadEstimate, const double normalVariateSample, const double drift, const double volatilityTime ) const;

		/* @brief	Calculates the expected value of a forward CDS, assuming the spread follows a log-normal process
		*
		* @param[in]	creditModel			The calibrated credit model
		* @param[in]	meanSpreadEstimate	A sample taken from a standard normal distribution
		* @param[in]	drift				The log-normal process drift term
		* @param[in]	volatilityTime		The volatility scaling factor which multiplies normal variate
		* @param[in]	fixedCoupon			The coupon rate which standardises the CDS instrument, e.g. 100bps
		*/
		double calculateCDSIndexExpectedValue( const CreditModel& creditModel, const double meanSpreadEstimate, const double drift, const double volatilityTime, const double fixedCoupon ) const;

		/* @brief	Calibrates the spread log-normal process so that it reproduces the price of a forward CDS contract
		*			The calibration parameter is the mean of the log-normal distribution
		* @param[in]	creditModel			The calibrated credit model
		* @param[in]	targetForwardPrice	The forward spread to match
		* @param[in]	drift				The log-normal process drift term
		* @param[in]	volatilityTime		The volatility scaling factor which multiplies normal variate
		* @param[in]	forwardCoupon		The coupon rate which standardises the CDS instrument, e.g. 100bps
		*/
		double calibrateLogNormalMeanSpread( const CreditModel& creditModel, const double targetForwardPrice, const double drift, const double volatilityTime, const double forwardCoupon ) const;

		/* @brief Calculates the risky annuity of the specified Credit Default Swap Premium leg.
		*
		* @param[in]	valuationSettingsLVB		The valuation settings with curveCollection and valuationDate etc.
		* @param[in]	hazardRate				The CDS hazard rate, used to calculate survival probabilities
		* @param[in]	recoveryRate			The estimated amount of capital recovered after default
		* @param[in]	legName					The Premium leg to use when calculating the risky annuity. A mandatory parameter.
		* @param[in]	includeAccruedInterest	Specifies whether the accruedInterest should be included in the risky annuity
		* @returns	The risky annuity
		*/
		double riskyAnnuityFromHazardRate(const LabelValueBlock& valuationSettingsLVB, const double hazardRate, const double recoveryRate, const AQLString& legName, const bool includeAccruedInterest ) const;

		/* @brief Calculates the risky annuity of the specified Credit Default Swap Premium leg.
		*
		* @param[in]	creditModel				The calibrated credit model
		* @param[in]	legName					The Premium leg to use when calculating the risky annuity. A mandatory parameter.
		* @returns	The risky annuity
		*/
		double riskyAnnuity( const CreditModel& creditModel, const AQLString& legName ) const;

		/* @brief[in]	Computes the accrued year fraction from the previous coupon date to the specified date
		*				Used in accrued interest calculations.
		*
		* @param[in]	creditModel		The calibrated credit model
		* @param[in]	toDate			The date to which we wish to calculate the year fraction
		* @param[in]	legName			The Premium leg name
		* @returns: The year fraction
		*/
		double accruedYearFraction( const CreditModel& creditModel, const AQLDate& toDate, const AQLString& legName ) const;

		/* @brief[in]	Computes the accrued interest from the previous coupon date to the specified date
		*				i.e. this corresponds to the amount of premium coupon that is accrued
		*
		* @param[in]	creditModel		The calibrated credit model
		* @param[in]	toDate			The date to which we wish to calculate the accrued interest for
		* @param[in]	legName			The Premium leg name
		* @returns: The accrued interest
		*/
		double accruedInterest( const CreditModel& creditModel, const AQLDate& toDate, const AQLString& legName ) const;

		/* @brief Calculates the par spread of the specified Credit Default Swap.
		*
		* @param[in]	curveCollections		A label value block containing a single collection name or a curveCollection per leg
		* @param[in]	hazardRate				The CDS hazard rate, used to calculate survival probabilities
		* @param[in]	recoveryRate			The estimated amount of capital recovered after default
		* @param[in]	premiumLegName			The Premium leg name of the CDS
		* @param[in]	protectionLegName		The Protection leg name of the CDS
		* @param[in]	includeAccruedInterest	Specifies whether the accruedInterest should be included in the risky annuity
		* @returns	The CDS par spread
		*/
		double parSpreadFromHazardRate( const LabelValueBlock& curveCollections, const double hazardRate, const double recoveryRate, const AQLString& premiumLegName, const AQLString& protectionLegName, const bool includeAccruedInterest ) const;

		/* @brief Calculates the par spread of the specified Credit Default Swap.
		*
		* @param[in]	creditModel				The calibrated credit model
		* @param[in]	premiumLegName			The Premium leg name of the CDS
		* @param[in]	protectionLegName		The Protection leg name of the CDS
		* @returns	The CDS par spread
		*/
		double parSpread( const CreditModel& creditModel, const AQLString& premiumLegName, const AQLString& protectionLegName ) const;

		/* @brief Solves for the hazard rate, given the specified CDS par spread
		*
		* @param[in]	curveCollections		A label value block containing a single collection name or a curveCollection per leg
		* @param[in]	parSpread				The CDS par spread ( as a decimal )
		* @param[in]	recoveryRate			The estimated amount of capital recovered after default
		* @param[in]	premiumLegName			The Premium leg name of the CDS
		* @param[in]	protectionLegName		The Protection leg name of the CDS
		* @param[in]	includeAccruedInterest	Specifies whether the accruedInterest should be included in the risky annuity
		* @returns	The CDS hazard rate
		*/
		double hazardRateFromParSpread( const LabelValueBlock& curveCollections, const double parSpread, const double recoveryRate, const AQLString& premiumLegName, const AQLString& protectionLegName, const bool includeAccruedInterest ) const;

		/* @brief Solves for the hazard rate, given the specified CDS par spread.
		*         NOTE: This modified the hazard rate in the credit model. Used in calibration
		*
		* @param[in]	parSpread				The CDS par spread ( as a decimal )
		* @param[inout]	creditModel				The calibrated credit model
		* @param[in]	premiumLegName			The Premium leg name of the CDS
		* @param[in]	protectionLegName		The Protection leg name of the CDS
		* @returns	The CDS hazard rate
		*/
		double hazardRateFromParSpread( const double parSpread, CreditModel& creditModel, const AQLString& premiumLegName, const AQLString& protectionLegName ) const;

	private:

		/* @brief		Private helper method which assigns premiumLeg / protectionLeg pointers for the credit default swap.
		*				If legName is provided, then search for that leg and assign one of premiumLeg / protectionLeg.
		*
		*  @param[in]	legName			Optional: Specify the legName to search for within the CDS. If legName is an empty string, assign both premiunmLeg and protectionLeg parameters
		*  @param[out]	premiumLeg		On output, initialized to the premiumLeg, if found
		*  @param[out]  protectionLeg	On output, initialized to the protectionLeg, if found
		*/
		void identifyCdsLegsUsingLegNameifProvided( const AQLString& legName, std::shared_ptr<CDSLeg>& premiumLeg, std::shared_ptr<CDSLeg>& protectionLeg ) const;

		/* @brief Calculates the par spread of the specified Credit Default Swap. The internal implementation method
		*
		* @param[in]	curveCollections		A label value block containing a single collection name or a curveCollection per leg
		* @param[in]	hazardRate				The CDS hazard rate, used to calculate survival probabilities
		* @param[in]	recoveryRate			The estimated amount of capital recovered after default
		* @param[in]	premiumLegName			The Premium leg name of the CDS
		* @param[in]	protectionLegName		The Protection leg name of the CDS
		* @param[in]	includeAccruedInterest	Specifies whether the accruedInterest should be included in the risky annuity
		* @returns	The CDS par spread
		*/
		double parSpreadFromHazardRate_impl( const LabelValueBlock& curveCollections, const double hazardRate, const double recoveryRate, const AQLString& premiumLegName, const AQLString& protectionLegName, const bool includeAccruedInterest ) const;

		/* @brief Calculates the par spread of the specified Credit Default Swap using the Credit Model. The internal implementation method
		*
		* @param[in]	curveCollection			The curve collection name
		* @param[in]	creditModel				The calibrated credit modelt
		* @param[in]	premiumLegName			The Premium leg name of the CDS
		* @param[in]	protectionLegName		The Protection leg name of the CDS
		* @returns	The CDS par spread
		*/
		double parSpread_impl( const AQLString& curveCollection, const CreditModel& creditModel, const AQLString& premiumLegName, const AQLString& protectionLegName ) const;

		void validateCreditModel( const CreditModel& creditModel ) const;
	};

}

