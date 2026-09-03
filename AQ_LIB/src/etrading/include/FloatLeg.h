/*
 * @brief			Class the defines the float leg
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "Leg.h"
#include "FloatSchedule.h"
#include "FloatStaticData.h"
#include "CreditModel.h"

namespace etrading
{
    class FloatLeg : public Leg
    {
    public:

        FloatLeg(const std::string& instanceName);
		FloatLeg(const LabelValueBlock& legLVB, const std::string& instanceName, const SchedulePtr& schedule={} );
        FloatLeg(const std::string& instanceName, const LegStaticDataPtr& legStaticData, const SchedulePtr& schedule);
        FloatLeg(const FloatLeg& rhs);
		virtual ~FloatLeg() {}

        LegPtr clone();

        LabelValueBlock getInputParameters() const;

		double annuityWithNotional(DataProvider& dataProvider);

		double pv( DataProvider& dataProvider, bool nativeCurrencyPV=false, bool updateCurveData=true);

		//double dv01(const LAString& curveCollection);

        ScheduleTypeEnum getType() const;

		/*	@brief	Initialise the DataProvider object from the supplied curveCollection and fixngtable
		*	@param[out]		dataProvider		A reference to the DataProvider object which will be populated
		*	@param[in]		updateCurveData		Whether to update the curve data inside the dataProvider
		*/
		virtual void initializeDataProvider( DataProvider& dataProvider, bool updateCurveData=true);

		std::unordered_set<CashflowHeaderEnum,EnumClassHash> allowedColumns() const;

		//Override
		PaymentTriggerEnum getPaymentTrigger() const;

		static std::vector<std::string> legLVBWithoutScheduleKeys()
		{
			std::vector<std::string> expectedKeys = FloatStaticData::lvbKeys();
			return expectedKeys;
		}

		static std::vector<std::string> lvbKeys()
		{
			std::vector<std::string> expectedKeys = FloatLeg::legLVBWithoutScheduleKeys();
			std::vector<std::string> scheduleKeys = FloatSchedule::lvbKeys();
			expectedKeys.insert(expectedKeys.end(), scheduleKeys.begin(), scheduleKeys.end());
			return expectedKeys;
		}

	private:
		DoubleVector calculateOisFloatRates( size_t firstNonpastFixingDateIndex, const LAString& curveCollection, const std::shared_ptr<FixingTable>& fixingTable) const;
		std::vector<FloatRateData> calculateFloatRates( const LAString& curveCollection, const LADate& valuationDate, const std::shared_ptr<FixingTable>& fixingTable, const std::string& volatilityModelName, const ConvexityMethodEnum& convexityMethod) const;

		//Stub rate from float leg's fixing date list
        double calculateStubRate( const LAString& curveCollection, const LAString& stubCurveIndex, const LAStringVector& curveIndices, const LAStringVector& curveTenors, const LAString& indexFrequency) const;

		//Stub rate from fixing start date and fixing end date
		double calculateStubRateFromFixingStartEnd(const LADate& fixingDate, const LADate& fixingEndDate, const LAString& curveCollection, const LAString& stubCurveIndex, const LAStringVector& curveIndices, const LAStringVector& curveTenors, const DayCountEnum& accrualDayCount, const LAString& fixingCalendar, const BusinessDayAdjustmentEnum& fixingBusinessDayAdj) const;

		//floatRate from individual cashflows (bespoke cashflows)
		std::vector<FloatRateData> calculateFloatRatesFromCashflows(const LAString& curveCollection, const LADate& valuationDate, const std::shared_ptr<FixingTable>& fixingTable, const std::string& volatilityModelName, const ConvexityMethodEnum& convexityMethod) const;

		/* @brief Updates the dataProvider survival probabilities and discount factors using the provided modelName.
		*
		* @param[out]	dataProvider		A reference to the DataProvider object which will be populated
		* @param[in]		isFloatRateRequired	Whether to calculate float rates
		* @param[in]	updateCurveData		Whether to update the curve data inside the dataProvider
		*/
		virtual void initializeDataProviderUsingModel( DataProvider& dataProvider, bool isFloatRateRequired=true, bool updateCurveData=true );

        //Helper function to the public initializeDataProvider() functions
		void initializeDataProviderInternal( DataProvider& dataProvider, bool isFloatRateRequired, bool updateCurveData=true );

		const double pvCalc( DataProvider& dataProvider, const bool& nativeCurrencyPV ) const;
				
		/* @brief Updates the cashflow survival / default probabilities using the provided credit model.
		*
		* @param[in]	asOfDate				The valuation date of the leg
		* @param[in]	creditModel				The calibrated credit model
		*/
		void setSurvivalProbabilitiesUsingCreditModel( const LADate& asOfDate, const CreditModel& creditModel );

		/*
		* @brief	Helper method which ensures that all cashflow survivalProbabilities are initialised to 1.0
		*/
		void resetRiskFreeCashflows();

		PaymentTriggerEnum paymentTrigger_; // Whether there is credit risk on the float coupons

	};
}


