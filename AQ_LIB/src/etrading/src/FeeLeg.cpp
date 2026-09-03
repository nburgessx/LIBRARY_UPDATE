/*
 * @brief			Class the defines the fixed leg
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */
#include "FeeLeg.h"
#include "FeeCashflow.h"
#include "SwapValidation.h"

namespace etrading
{
   
	FeeLeg::FeeLeg(const LabelValueBlock& legLVB, const std::string& instanceName, const SchedulePtr& schedule) : Leg(legLVB, instanceName, schedule)
	{
        const std::string inputLVB = "legLVB";

        legStaticData_ = LegStaticDataPtr (new FeeStaticData(legLVB));

		//Schedule
		if (schedule==nullptr)
		{
			schedule_ = SchedulePtr(new FeeSchedule(instanceName));
		}

        inputParameters_ = LabelValueBlock( inputParameters_, schedule_->getInputParameters() );
    }

    FeeLeg::FeeLeg(const std::string& instanceName, const LegStaticDataPtr& legStaticData, const SchedulePtr& schedule) : Leg(instanceName, legStaticData, schedule)
    {
    }

	/*	@brief	Initialise the DataProvider object from the supplied curveCollection and fixngtable
	*	@param[out]		dataProvider		A reference to the DataProvider object which will be populated
	*	@param[in]		updateCurveData		Whether to update the curve data inside the dataProvider
	*/
    void FeeLeg::initializeDataProvider( DataProvider& dataProvider, bool updateCurveData )
    {
        Leg::initializeDataProvider( dataProvider, updateCurveData );

        if(!legStaticData_->isCurveDataLoaded() && updateCurveData)
        {
            schedule_->initializeDataProviderWithCurveData( dataProvider, legStaticData_->getDiscountCurve() );

			/*
			 *  NOTE: The next line of code is commented out because we always want to enter this block of code
			 *  and recalculate the schedule discount factors. A better optimisation is to check
			 *  whether the discountCurve has changed (for example via a curve build time).
			 */
            // legStaticData_->setCurveDataLoaded(true);
        }

    }

    LegPtr FeeLeg::clone()
    {
        LegPtr leg = LegPtr(new FeeLeg(*this));
        return leg;
    }

    FeeLeg::FeeLeg( const FeeLeg& rhs) : Leg(rhs)
    {}

    double FeeLeg::annuityWithNotional(DataProvider& dataProvider)
	{
        return 0;
    }

   	double FeeLeg::pv( DataProvider& dataProvider, bool nativeCurrencyPV, bool updateCurveData)
	{
		initializeDataProvider( dataProvider, updateCurveData );
   	    
	    double pv = 0.0;
		if (schedule_ == nullptr)
		{
			throw LACoreInvalidData( "#Error: Schedule has not been built", __FILE__, __LINE__ );
		}

		CashflowPtr cf;
		for( unsigned int i = 0; i < schedule_->getCashflowSize(); i++ )
		{
			cf = schedule_->getCashflow(i);
			pv += cf->getCouponPv( dataProvider.getCashflowDataExcludingUpfront( i ), nativeCurrencyPV);
		}
		return pv;
	}

    ScheduleTypeEnum FeeLeg::getType() const
    {
        return FEE_SCHEDULE_TYPE;
    }

    void FeeLeg::flipPayerReceiver()
    {
		if (schedule_ == nullptr)
		{
			throw LACoreInvalidData( "#Error: LegStaticData or Schedule has not been built", __FILE__, __LINE__ );
		}

        size_t expectedSize = schedule_->getCashflowSize();
 		for( size_t i = 0; i < expectedSize; i++ )
		{			
			CashflowPtr cf = schedule_->getCashflow(i);
 			cf->flipPayerReceiver();
    	}
    }

	std::unordered_set<CashflowHeaderEnum,EnumClassHash> FeeLeg::allowedColumns() const
	{
		std::unordered_set<CashflowHeaderEnum,EnumClassHash> expectedList
		{
			DISCOUNT_FACTOR_HEADER
			, COUPON_PV_HEADER
		};

		return expectedList;

	}


}

