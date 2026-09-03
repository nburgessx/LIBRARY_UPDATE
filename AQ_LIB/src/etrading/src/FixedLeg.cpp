/*
 * @brief			Class the defines the fixed leg
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */
#include "FixedLeg.h"
#include "FixedCashflow.h"

namespace etrading
{
   
	FixedLeg::FixedLeg(const LabelValueBlock& legLVB, const std::string& instanceName, const SchedulePtr& schedule) : Leg(legLVB, instanceName, schedule)
	{
        const std::string inputLVB = "legLVB";

        legStaticData_ = LegStaticDataPtr (new FixedStaticData(legLVB));

		//Schedule
		if (schedule==nullptr)
		{
			schedule_ = SchedulePtr(new FixedSchedule(legLVB, instanceName));
		}
	}

    FixedLeg::FixedLeg(const std::string& instanceName, const LegStaticDataPtr& legStaticData, const SchedulePtr& schedule) : Leg(instanceName, legStaticData, schedule)
    {}

	/*	@brief	Initialise the DataProvider object from the supplied curveCollection and fixngtable
	*	@param[out]		dataProvider		A reference to the DataProvider object which will be populated
	*	@param[in]		updateCurveData		Whether to update the curve data inside the dataProvider
	*/
    void FixedLeg::initializeDataProvider( DataProvider& dataProvider, bool updateCurveData)
    {	
        Leg::initializeDataProvider( dataProvider, updateCurveData );

        if(!legStaticData_->isCurveDataLoaded() && updateCurveData)
        {
			schedule_->initializeDataProviderWithCurveData( dataProvider, legStaticData_->getDiscountCurve());

			////need to up date previous interests every time the fixedRate is updated
            //schedule_->updatePreviousRatesForZeroCouponFinalCashflow();

			/*
			 *  NOTE: The next line of code is commented out because we always want to enter this block of code
			 *  and recalculate the schedule discount factors. A better optimisation is to check
			 *  whether the discountCurve has changed (for example via a curve build time).
			 */
            //legStaticData_->setCurveDataLoaded(true);

		}

		//When a leg with compounding coupons, the previous coupons need to be updated, as the fixed rate may changed
        schedule_->updateCashflowsCompoundingCoupons( dataProvider );
    }

    LegPtr FixedLeg::clone()
    {
        LegPtr leg = LegPtr(new FixedLeg(*this));
        return leg;
    }

	FixedLeg::FixedLeg( const FixedLeg& rhs) : Leg(rhs)
	{}

	const double FixedLeg::pvCalc( const DataProvider& dataProvider, bool nativeCurrencyPV ) const
	{
		
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
			pv += cf->getCouponPv( dataProvider.getCashflowDataIncludingUpfront( i ), nativeCurrencyPV);
		}

		// If need to exclude accruedInterest, when there is accruedInterest, we need to minus the accrued interest.
		// For instance, when calculating parRate/parSpread, assetSwap par, we need to exclude accruedInterest
		if ( !dataProvider.getValuationSettings().getIncludeAccruedInterest() &&  schedule_->hasAccruedInterest(dataProvider.getValuationSettings().getValuationDate()))
		{
			double accruedInterest = schedule_->calculateAccruedInterest( dataProvider, nativeCurrencyPV);
			//PV minus accrualInterest
			pv -= accruedInterest;
		}

		return pv;
	}

	double FixedLeg::pv( DataProvider& dataProvider, bool nativeCurrencyPV, bool updateCurveData)
    {
		initializeDataProvider( dataProvider, updateCurveData );

		return pvCalc( dataProvider, nativeCurrencyPV);
	}

    ScheduleTypeEnum FixedLeg::getType() const
    {
        return FIXED_SCHEDULE_TYPE;
    }

    LabelValueBlock FixedLeg::getInputParameters() const
	{
		std::vector<std::string> unchangedKeys;
		unchangedKeys.push_back(IRS_KEY::FIXED_RATE);
		return removeKeyPrefix(inputParameters_, "FIXED", unchangedKeys);
	}

	std::unordered_set<CashflowHeaderEnum,EnumClassHash> FixedLeg::allowedColumns() const
	{
		std::unordered_set<CashflowHeaderEnum,EnumClassHash> expectedList
		{
			FX_FIXING_DATE_HEADER
			, FX_RATE_HEADER
			, COUPON_HEADER
			, DISCOUNT_FACTOR_HEADER
			, COUPON_PV_HEADER
		};

		return expectedList;

	}

}

