/*
 * @brief			Class the defines the schedule 
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "LACoreTemplateType.h"
#include "Schedule.h"
#include "BondCurves.h"

namespace etrading
{
	
	class BondSchedule : public Schedule
    {
	public:

		BondSchedule(const std::string& instanceName);
		BondSchedule(const LabelValueBlock& scheduleLVB, const std::string& instanceName);
		BondSchedule(const BondSchedule& rhs);
        virtual ~BondSchedule() {};
		
		SchedulePtr clone() = 0;

        // Return struct BondActiveCouponDates
        // It it is not the first cashflow, the priorDate is the payment date before the first active coupon (payment) date
        // If it is the first cashflow, 1) when isPriorDtWithFullCouponPeriod is true (used in bond yield calculation), then priorDate is the prior date with full coupon period from the first active payment date. 
        //                              2) when isPriorDtWithFullCouponPeriod is false (used in bondAccruedInterest calculation), then priorDate is first accrualStartDate (effective date).
		BondActiveCouponDates getBondFirstActiveCouponDates( const LADate& settlementDate, bool isPriorDtWithFullCouponPeriod) const;

        //Get the front stub type enum based on the first coupon date, or get the end stub type enum based on the last coupon date
        StubTypeEnum getBondStubTypeEnum(bool isFrontStub) const;

		double calculateBondAccruedInterest(const LADate& settlementDate, const BondYieldParameters & bondYieldParameters) const;
        int calculateBondAccruedInterestDays( const LADate& settlementDate, const BondYieldParameters & bondYieldParameters) const;
		
        // For Bonds: Update the Cashflow Yields and Forward Rates                                                                                           
        void initializeDataProviderWithYieldData( DataProvider& dataProvider, const BondYieldParameters & bondYieldParameters, const double& yield, const YieldCalculationTypeEnum& yieldCalcType, const std::shared_ptr<BondActiveCouponDates>& activeCouponDatesPtr, const std::vector< FloatRateData >& floatRates = std::vector< FloatRateData >()) const;
		
		/* @brief: Initializes the dataProvider using discount factors calculated using yield points obtained from a BondCurve.
		*
		* @param[out]	dataProvider			The dataProvider to initialize
		* @param[in]	bondYieldParameters		Specifies static bond data such as calculationType, coupon payment frequency etc
		* @param[in]	bondCurve				The bond curve
		* @param[in]	activeCouponDatesPtr	Specifies the coupon dates which bracket the settlement date.
		* @param[in]	floatRates				Optional float rates
		*/
		void initializeDataProviderWithBondCurve( DataProvider& dataProvider, const BondYieldParameters & bondYieldParameters, const BondCurve& bondCurve, const std::shared_ptr<BondActiveCouponDates>& activeCouponDatesPtr, const std::vector< FloatRateData >& floatRates = std::vector< FloatRateData >() ) const;

		// Override
		void initializeDataProviderWithCurveData(DataProvider& dataProvider, const LAString& discountCurve, const std::vector< FloatRateData >& floatRates = std::vector< FloatRateData >()) const;

		const FrequencyEnum getYieldFrequency() const { return yieldFrequency_; };
		const BondCalculationTypeEnum getBondCalculationType() const { return bondCalculationType_; };
		const std::string getExDividendTenor() const { return exDividendTenor_; };
		const BusinessDayAdjustmentEnum getExDividendBusinessDayAdj() const { return exDividendBusinessDayAdj_; };

		//Override
		virtual void calculateScheduleDates();

		//Override
		LADate getMaturityDate() const;

		//Include cashflow's couponRate when settlementDate >= exDividendDate
		bool getFirstActiveCashflowIncludeCouponRate(const CashflowPtr& firstActiveCashflow, const LADate& settleDate) const;

		//When settlementDate >= exDividendDate, this method will set the firstActiveCashflow's includeCouponRate to false; otherwise it will be true
		void updateIncludeCouponRates(DataProvider& dataProvider, const size_t& firstActiveCashflowIndex) const;

		void populateHeaderAndBody(AnyTypeVector& headers, std::vector<AnyTypeVector>& bodys, bool showColumnHeaders, const std::unordered_set<CashflowHeaderEnum,EnumClassHash>& columnList) const;

		std::unordered_set<CashflowHeaderEnum,EnumClassHash> allowedColumns() const;

	protected:

		//Input parameters
		BondCalculationTypeEnum bondCalculationType_;
		LAString issueDate_;
		double issuePrice_;			
		double taxRate_;			
		FrequencyEnum yieldFrequency_;			
		std::string exDividendTenor_;
		BusinessDayAdjustmentEnum exDividendBusinessDayAdj_;

		//Output parameters
		
		//Bond actual year fractions for TRUE yield calculation, with consideration of accrual business adjustment 
      	DoubleVector	bondTrueYieldYearFractions_;
		DateVector		exDividendDates_;

		//Override
		virtual void populateAccrualStartDates(const LabelValueBlock& scheduleLVB);
		//Override
		void populateNotionalAndPaymentFreqEnum(const LabelValueBlock& scheduleLVB = LabelValueBlock());
		//Override
		double getFinalCashflowNotionalExchange(const double& notional) const;

	
        // Virtual payment date with full coupon period prior the given paymentDate
        LADate getBondPriorVirtualPaymentDate(const LADate& paymentDate) const;

        // If the first/last cashflow has stub, calculate the yearFraction differently from the normalYearFraction.
        double calculateBondCashflowYearFraction(bool firstCashflow, bool lastCashflow) const; 

		//Bond actual year fractions for TRUE yield calculation, with consideration of accrual business adjustment 
        double calculateBondTrueYieldYearFraction(size_t cashflowIndex, const LADate& accrualStart, const LADate& accrualEnd, const LADate& paymentDate) const;

		//Helper function to initialize the member variables
		void initialize(const LabelValueBlock& scheduleLVB);

		void populateExDividendDates();

		/* @brief Private helper method which stores the input discount factors and float rates in the dataProvider
		*  @param[out] dataProvider				The DataProvider to update
		*  @param[in] calculatedDiscountFactors	The input discount factors to set in the data provider
		*  @param[in] floatRates				The input float rates
		*  @param[in] activeCouponDates			Specifies the coupon dates which bracket the settlement date.	
		*/
		void setDiscountFactorsAndFloatRatesInDataProvider( DataProvider& dataProvider, const DoubleVector& calculatedDiscountFactors, const std::vector< FloatRateData >& floatRates, const BondActiveCouponDates& activeCouponDates ) const;

	};

	typedef std::shared_ptr<BondSchedule> BondSchedulePtr;

}

