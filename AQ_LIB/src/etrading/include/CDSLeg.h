#pragma once

#include "Leg.h"
#include "CreditModel.h"

namespace etrading
{
    class CDSLeg : public Leg
    {
    public:

		CDSLeg(const LabelValueBlock& legLVB, const std::string& instanceName, const SchedulePtr& schedule={});
        CDSLeg(const std::string& instanceName, const LegStaticDataPtr& legStaticData, const SchedulePtr& schedule);
		CDSLeg(const CDSLeg& rhs);
		virtual ~CDSLeg() {}

		/* @brief Updates the cashflow discount factors using the provided curveCollection
		*
		* @param[out]	dataProvider		A reference to the DataProvider object which will be populated
		* @param[in]	updateCurveData		Whether to update the Curve data in the dataProvider
		*/
		virtual void initializeDataProvider( DataProvider& dataProvider, bool updateCurveData=true);

		/* @brief Updates the cashflow survival / default probabilities using the hazard rate and recovery rate parameters
		*         Derived classes must implement this.
		*
		* @param[in]	asOfDate				The valuation date of the leg
		* @param[in]	hazardRate				The CDS hazard rate parameter, used to calculate survival probabilities
		* @param[in]	recoveryRate			The estimated amount of capital recovered after default
		* @param[in]	includeAccruedInterest	Specifies whether cashflows should include the accruedInterest
		*/
		virtual void setSurvivalProbabilitiesUsingHazardRate( const AQLDate& asOfDate, const double hazardRate, const double recoveryRate, const bool includeAccruedInterest ) = 0;

		/* @brief Updates the cashflow survival / default probabilities using the provided credit model.
		*         Derived classes must implement this.
		*
		* @param[in]	asOfDate				The valuation date of the leg
		* @param[in]	creditModel				The calibrated credit model
		*/
		virtual void setSurvivalProbabilitiesUsingCreditModel( const AQLDate& asOfDate, const CreditModel& creditModel ) = 0;

		/* @brief Calculates the PV of the CDS leg.
		*
		* @param[in]	dataProvider			The data provider which holds the valuation settings
		* @param[in]	hazardRate				The CDS hazard rate, used to calculate survival probabilities
		* @param[in]	recoveryRate			The estimated amount of capital recovered after default
		* @param[in]	includeAccruedInterest	Specifies whether the accruedInterest should be included in the PV
		* @param[in]	updateCurveData			Whether to update the Curve details in the leg static data
		* @returns	The calculated PV value
		*/
		virtual double pvFromHazardRate( DataProvider& dataProvider, const double hazardRate, const double recoveryRate, bool includeAccruedInterest, bool updateCurveData=true );

		/* @brief Calculates the PV of the CDS leg using the provided credit model. 
		*
		* @param[in]	creditModel				The calibrated credit model
		* @param[in]	updateCurveData			Whether to update the Curve details in the leg static data
		* @returns	The calculated PV value
		*/
		virtual double pv( const CreditModel& creditModel, bool updateCurveData=true );

		/* @brief Calculates the leg PV assuming all coupons are paid with certainty right up to the stoppingDate. No coupons are paid after the stopping date.
		*  @param[in]	DataProvider						The data provider which holds the valuation settings, discount factors
		*  @param[in]	creditModel							The calibrated credit model
		*  @param[in]	stoppingDate						The date at which the underlying bond defaults
		*  @param[in]	discountFactorAtStoppingDate		The discount factor on the stopping date, if required.
		*  @param[in]	payDefaultCashflowsOnNextCouponDate	Whether to pay default-related cashflows on the stoppingDate, or on the next coupon date.
		*				When set to TRUE (pay on next coupon date ), the PV should match the analytic formula.
		*  @returns	The calculated PV value
		*/
		virtual double riskFreePVtoStoppingDate( DataProvider& dataProvider, const CreditModel& creditModel, const AQLDate& stoppingDate, const double discountFactorAtStoppingDate = std::numeric_limits<double>::quiet_NaN(), const bool payDefaultCashflowsOnNextCouponDate = false ) = 0;

	private:
		/* @brief Updates the dataProvider survival probabilities and discount factors using the provided modelName.
		*
		* @param[out]	dataProvider		A reference to the DataProvider object which will be populated
		* @param[in]	isFloatRateRequired	Whether to calculate float rates
		* @param[in]	updateCurveData		Whether to update the curve data inside the dataProvider
		*/
		virtual void initializeDataProviderUsingModel( DataProvider& dataProvider, bool isFloatRateRequired=true, bool updateCurveData=true );

	};

}