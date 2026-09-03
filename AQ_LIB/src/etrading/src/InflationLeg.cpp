/*
 * @brief			Class which defines the Inflation Leg of a Zero Coupon Inflation Swap
 * @Created:		19 May 2030
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "InflationLeg.h"
#include "InflationSchedule.h"

#include "LWOUtilities.h"

namespace etrading
{
   
	InflationLeg::InflationLeg( const LabelValueBlock& legLVB, const std::string& instanceName, const SchedulePtr& schedule )
		: Leg(legLVB, instanceName, schedule )
	{
		legStaticData_ = LegStaticDataPtr(new LegStaticData(legLVB));

		if (schedule == nullptr)
		{
			schedule_ = SchedulePtr(new InflationSchedule(legLVB, instanceName));
		}
	}

    InflationLeg::InflationLeg( const std::string& instanceName, const LegStaticDataPtr& legStaticData, const SchedulePtr& schedule ) 
		: Leg( instanceName, legStaticData, schedule )
    {}

	InflationLeg::InflationLeg( const InflationLeg& rhs ) : Leg( rhs )
	{}

	LegPtr InflationLeg::clone()
	{
		LegPtr leg = LegPtr( new InflationLeg( *this ));
		return leg;
	}

	ScheduleTypeEnum InflationLeg::getType() const
	{
		return INFLATION_SCHEDULE_TYPE;
	}

	std::unordered_set<CashflowHeaderEnum, EnumClassHash> InflationLeg::allowedColumns() const
	{
		std::unordered_set<CashflowHeaderEnum, EnumClassHash> expectedList
		{
			COUPON_HEADER
			, DISCOUNT_FACTOR_HEADER
			, COUPON_PV_HEADER
		};

		return expectedList;
	}

	/* @brief Updates the cashflow discount factors using the provided curveCollection
	*
	* @param[out]	dataProvider		A reference to the DataProvider object which will be populated
	* @param[in]	updateCurveData		Whether to update the Curve data in the dataProvider
	*/
    void InflationLeg::initializeDataProvider( DataProvider& dataProvider, bool updateCurveData )
    {
        Leg::initializeDataProvider( dataProvider,  updateCurveData );

        if ( ! legStaticData_->isCurveDataLoaded() && updateCurveData )
        {
			//Populate DFs for cashflows
			schedule_->initializeDataProviderWithCurveData( dataProvider, legStaticData_->getDiscountCurve() );
		}
    }

	/* @brief Calculates the PV of the Inflation leg, given inflation Index fixing values at start and end of the leg
	*
	* @param[in]	dataProvider			The data provider which holds the valuation settings
	* @param[in]	baseIndex				The inflation level at the effective date of the swap
	* @param[in]	resetIndex				The inflation level at the maturity of the swap
	* @param[in]	updateCurveData			Whether to update the Curve details in the leg static data
	* @returns	The calculated PV value
	*/
	double InflationLeg::pvFromInflationIndex( DataProvider& dataProvider, const double baseIndex, const double resetIndex, bool updateCurveData )
	{
		// Calculate Discount Factors
		initializeDataProvider( dataProvider, updateCurveData );

		std::shared_ptr<InflationSchedule> inflationSchedule = std::static_pointer_cast<InflationSchedule> (schedule_);
		inflationSchedule->setInflationIndex( baseIndex, resetIndex );

        //get all the cashflows including the upfrontCashflow
        auto cashflows = schedule_->getAllCashflows();
        if (cashflows.size() == 0)
		{
			throw LACoreInvalidData("#Error: No cashflow has been built yet", __FILE__, __LINE__ );
		}

        double pv = 0;
 		for( size_t i = 0; i < cashflows.size(); i++ )
		{			
			auto cf = cashflows[i];
			pv += cf->getCouponPv( dataProvider.getCashflowDataIncludingUpfront( i ) );
		}

		return pv;
	}

	/* @brief Calculates the PV of the Inflation leg, given an inflation curve
	*
	* @param[in]	dataProvider			The data provider which holds the valuation settings
	* @param[in]	inflationCurve			An inflation curve from which index levels can be found
	* @param[in]	updateCurveData			Whether to update the Curve details in the leg static data
	* @returns	The calculated PV value
	*/
	double InflationLeg::pv( DataProvider& dataProvider, const InflationCurve& inflationCurve, bool updateCurveData )
	{
		// Calculate Discount Factors
		initializeDataProvider(dataProvider, updateCurveData);

		const LADate curveAsOfDate = inflationCurve.getAsOfDate();
		const LADate baseFixingDate = getSchedule()->getFixingDates().front();
		
		// The inflation curve knows about fixings. So we can simply ask the curve for the baseIndex
		const double baseIndex = inflationCurve.getMonthlyInflationIndexForLaggedDate( baseFixingDate );

		LADate finalFixingDate = getSchedule()->getFixingDates().back();
		const double resetIndex = inflationCurve.getMonthlyInflationIndexForLaggedDate( finalFixingDate );

		std::shared_ptr<InflationSchedule> inflationSchedule = std::static_pointer_cast<InflationSchedule> (schedule_);
		inflationSchedule->setInflationIndex( baseIndex, resetIndex );

		//get all the cashflows including the upfrontCashflow
		auto cashflows = schedule_->getAllCashflows();
		if (cashflows.size() == 0)
		{
			throw LACoreInvalidData("#Error: No cashflow has been built yet", __FILE__, __LINE__);
		}

		double pv = 0;
		for (size_t i = 0; i < cashflows.size(); i++)
		{
			auto cf = cashflows[i];
			pv += cf->getCouponPv(dataProvider.getCashflowDataIncludingUpfront(i));
		}

		return pv;
	}

}

