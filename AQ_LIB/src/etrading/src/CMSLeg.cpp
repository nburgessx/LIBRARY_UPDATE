#include "CMSLeg.h"
#include "CMSSchedule.h"
#include "LACurveForwardRateHelpers.h"
#include "CurveInstrumentPricing.h"
#include "LACurvePricingObject.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "CurveBuildDefaults.h"

namespace etrading
{
	CMSLeg::CMSLeg( const std::string& instanceName ) : Leg( instanceName )
    {}

    CMSLeg::CMSLeg( const LabelValueBlock& legLVB, const std::string& instanceName, const SchedulePtr& schedule ) : Leg( legLVB, instanceName, schedule )
	{
        const std::string inputLVB = "legLVB";

        legStaticData_ = LegStaticDataPtr ( new CMSStaticData( legLVB ));

    	//Schedule
		if ( schedule==nullptr )
		{
			schedule_ = SchedulePtr( new CMSSchedule( legLVB, instanceName ));
		}
	}

    CMSLeg::CMSLeg( const std::string& instanceName, const LegStaticDataPtr& legStaticData, const SchedulePtr& schedule) : Leg( instanceName, legStaticData, schedule )
    {}

    void CMSLeg::initializeDataProvider( DataProvider& dataProvider, bool updateCurveData )
    {
        initializeDataProviderInternal( dataProvider, true, updateCurveData );
    }

    void CMSLeg::initializeDataProviderInternal( DataProvider& dataProvider, bool isFloatRateRequired, bool updateCurveData )
    {
        Leg::initializeDataProvider( dataProvider, updateCurveData );

        if ( ! legStaticData_->isCurveDataLoaded() && updateCurveData )
        {
            
			// Populate DFs for cashflows
			schedule_->initializeDataProviderWithCurveData( dataProvider, legStaticData_->getDiscountCurve() );

            //Populate DFs and FloatRates for cashflows. The float rates are obtained from the par-rates of an underlying swap index
			if (isFloatRateRequired)
            {
				const std::shared_ptr<CMSStaticData>& cmsLegStaticData = std::static_pointer_cast<CMSStaticData> ( legStaticData_ );

				const std::shared_ptr<CMSSchedule>& cmsSchedule = std::static_pointer_cast<CMSSchedule> ( schedule_ );
				cmsSchedule->initializeDataProviderWithCMSParameters( dataProvider, cmsLegStaticData );
            }

            //Populate DFs for cashflows
            else
            {
                schedule_->initializeDataProviderWithCurveData( dataProvider, legStaticData_->getDiscountCurve() );
            }

			/*
			 *  NOTE: The next line of code is commented out because we always want to enter this block of code
			 *  and recalculate the schedule discount factors. A better optimisation is to check
			 *  whether the discountCurve has changed (for example via a curve build time).
			 */
            // legStaticData_->setCurveDataLoaded(true);
        }
    }

    LegPtr CMSLeg::clone()
    {
        LegPtr leg = LegPtr( new CMSLeg( *this ));
        return leg;
    }

    CMSLeg::CMSLeg( const CMSLeg& rhs ) : Leg( rhs )
	{}


	double CMSLeg::annuityWithNotional(DataProvider& dataProvider)
	{
        initializeDataProviderInternal( dataProvider, false );

        return calculateAnnuityWithNotional( dataProvider );
	}

	/* @brief Calculates the PV of the CMSLeg, using the ConvexityAdjustment input
	*
	* @param[in]	swapName			Swap object name
	* @param[in]	dataProvider		The data provider
	* @param[in]	convexityAdjustment	The convexity adjustment which is added to the PV of the CMS leg.
	* @param[in]	updateCurveData		Optional flag, whether to update cashflows with curve data
	* @returns	The calculated PV value
	*/
    double CMSLeg::pvUsingConvexityAdjustment(DataProvider& dataProvider, const double convexityAdjustment, bool updateCurveData )
	{
		initializeDataProvider( dataProvider, updateCurveData );

        double pv = 0.;

        //get all the cashflows including the upfrontCashflow
        auto cashflows = schedule_->getAllCashflows();

		for( size_t i = 0; i < cashflows.size(); i++ )
		{			
			CashflowPtr cf = cashflows[i];
			pv += cf->getCouponPv( dataProvider.getCashflowDataIncludingUpfront( i ) );
		}

		pv += convexityAdjustment;

		return pv;
	}

    ScheduleTypeEnum CMSLeg::getType() const
    {
        return CMS_SCHEDULE_TYPE;
    }

    LabelValueBlock CMSLeg::getInputParameters() const
	{
        std::vector<std::string> unchangedKeys;
		unchangedKeys.push_back( IRS_KEY::FLOAT_SPREAD );
		return removeKeyPrefix( inputParameters_, "CMS", unchangedKeys );
	}

	std::unordered_set<CashflowHeaderEnum,EnumClassHash> CMSLeg::allowedColumns() const
	{
		std::unordered_set<CashflowHeaderEnum,EnumClassHash> expectedList
		{
			DISCOUNT_FACTOR_HEADER
			, COUPON_PV_HEADER
		};

		return expectedList;

	}
}


