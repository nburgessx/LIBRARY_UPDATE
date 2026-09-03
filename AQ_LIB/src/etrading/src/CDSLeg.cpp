#include "CDSLeg.h"
#include "LWOUtilities.h"

namespace etrading
{
   
	CDSLeg::CDSLeg(const LabelValueBlock& legLVB, const std::string& instanceName, const SchedulePtr& schedule) : Leg(legLVB, instanceName, schedule)
	{
	}

    CDSLeg::CDSLeg(const std::string& instanceName, const LegStaticDataPtr& legStaticData, const SchedulePtr& schedule) : Leg(instanceName, legStaticData, schedule)
    {}

	CDSLeg::CDSLeg( const CDSLeg& rhs) : Leg(rhs)
	{}

	/* @brief Updates the dataProvider survival probabilities and discount factors using the provided modelName.
	*
	* @param[out]	dataProvider		A reference to the DataProvider object which will be populated
	* @param[in]		isFloatRateRequired	Whether to calculate float rates
	* @param[in]	updateCurveData		Whether to update the Curve details in the leg static data
	*/
	void CDSLeg::initializeDataProviderUsingModel( DataProvider& dataProvider, bool isFloatRateRequired, bool updateCurveData )
	{
		// Locate the credit model
		auto creditModelPtr = getCreditModel(dataProvider.getValuationSettings().getCreditModelName() );

		// Calculate Discount Factors
		initializeDataProvider( dataProvider, updateCurveData );

		// Calculate Survival Probabilities
		const LADate& asOfDate = dataProvider.getValuationSettings().getValuationDate();
		setSurvivalProbabilitiesUsingCreditModel( asOfDate, *creditModelPtr );	
	}

	/* @brief Updates the cashflow discount factors using the provided curveCollection
	*
	* @param[out]	dataProvider		A reference to the DataProvider object which will be populated
	* @param[in]	updateCurveData		Whether to update the Curve data in the dataProvider
	*/
    void CDSLeg::initializeDataProvider( DataProvider& dataProvider, bool updateCurveData )
    {
        Leg::initializeDataProvider( dataProvider,  updateCurveData );

        if ( ! legStaticData_->isCurveDataLoaded() && updateCurveData )
        {
			//Populate DFs for cashflows
			schedule_->initializeDataProviderWithCurveData( dataProvider, legStaticData_->getDiscountCurve() );
		}
    }

	/* @brief Calculates the PV of the CDS leg. Derived Leg classes must implement this method.
	*
	* @param[in]	dataProvider			The data provider to hold the valuation settings
	* @param[in]	hazardRate				The CDS hazard rate, used to calculate survival probabilities
	* @param[in]	recoveryRate			The estimated amount of capital recovered after default

	* @param[in]	includeAccruedInterest	Specifies whether the accruedInterest should be included in the PV
	* @param[in]	updateCurveData			Whether to update the Curve details in the leg static data
	* @param[out]	The calculated PV value
	*/
	double CDSLeg::pvFromHazardRate(DataProvider& dataProvider, const double hazardRate, const double recoveryRate, bool includeAccruedInterest, bool updateCurveData )
	{
		// Calculate Discount Factors
		initializeDataProvider( dataProvider, updateCurveData );

		// Calculate Survival Probabilities
		const LADate& asOfDate = dataProvider.getValuationSettings().getValuationDate();
		setSurvivalProbabilitiesUsingHazardRate( asOfDate, hazardRate, recoveryRate, includeAccruedInterest );

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

	/* @brief Calculates the PV of the CDS leg using the provided credit model. 
	*
	* @param[in]	creditModel				The calibrated credit model
	* @param[in]	updateCurveData			Whether to update the Curve details in the leg static data
	* @returns	The calculated PV value
	*/
	double CDSLeg::pv( const CreditModel& creditModel, bool updateCurveData )
	{
		// Calculate Discount Factors
		DataProvider dataProvider(ValuationSettings(creditModel, {}));
		initializeDataProvider( dataProvider, updateCurveData );

		// Calculate Survival Probabilities
		const LADate& asOfDate = dataProvider.getValuationSettings().getValuationDate();
		setSurvivalProbabilitiesUsingCreditModel( asOfDate, creditModel );

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
}

