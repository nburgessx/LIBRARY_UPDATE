#include "CMSSchedule.h"
#include "CMSCashflow.h"
#include "SwapValidation.h"
#include "AQObjUtilities.h"
#include "AQLDateSchedule.h"
#include "DataUtilities.h"          // For AQ_TO_STRING Macros

namespace etrading
{

	CMSSchedule::CMSSchedule( const std::string& instanceName ) : FloatSchedule( instanceName )
	{
        scheduleType_ = CMS_SCHEDULE_TYPE;
    }

	CMSSchedule::CMSSchedule( const LabelValueBlock& scheduleLVB, const std::string& instanceName ) : FloatSchedule( instanceName )
	{
		initialise( scheduleLVB );

		//Populate the cashflows based on schedule inputs
        createCashflows();

        // Determine if the Stubs are Irregular Stub
        determineIsIrregularStub();

    	scheduleType_ = CMS_SCHEDULE_TYPE;
    }

    SchedulePtr CMSSchedule::clone()
    {
        SchedulePtr sch = SchedulePtr( new CMSSchedule( *this ));
        return sch;
    }

	CMSSchedule::CMSSchedule( const CMSSchedule& rhs ) : FloatSchedule( rhs )
	{
		scheduleType_ = CMS_SCHEDULE_TYPE;
	}

    void CMSSchedule::createCashflows() 
    {
        //Use the first accrualStartDate as the paymentDate
        auto paymentDate = accrualStartDates_.at(0);

		auto cashflowSize = accrualStartDates_.size();

        std::vector<double> cashflowNotionals = getCashflowNotionals( cashflowSize ); 

        for (size_t i=0; i < cashflowSize; ++i)
		{
			auto cashflowType = (i == cashflowSize - 1) ? NORMAL_LAST_CASHFLOW_TYPE : NORMAL_CASHFLOW_TYPE;

            double cashflowNotional = cashflowNotionals[i];

			CashflowPtr cf = CashflowPtr( new CMSCashflow( payerReceiver_, getSpread(), getFixingDate(i), accrualStartDates_[i], accrualEndDates_[i], accrualDays_[i], accrualYearFractions_[i], paymentDates_[i], cashflowNotional, leverage_, paymentFreqEnum_, cashflowType));
            
            cashflows_.push_back(cf);
		}

        // Update notional exchanges based on notionals of cashflows
        updateNotionalExchange();
    }

	/* @brief		A private helper which constructs a default swap expression LVB used for computing swap par-rates
	*/
	LabelValueBlock CMSSchedule::setupSwapExpressionLVBForCMS() const
	{		
		// Set up the Swap Expression LVB used for constructing Swap Index underlying
		AQLStringVector keys; 
		AQLStringVector values;

        // Reserve vector sizes to speed_up data push_back
        keys.reserve(6);
        values.reserve(6);

		// We standardize these trade keys because we are only interested in par rate calculations 
	    keys.push_back( IRS_KEY::PAY_RECEIVE.c_str() );					    values.push_back("PAY");
		keys.push_back( IRS_KEY::NOTIONAL.c_str() );					    values.push_back("1.0");
		keys.push_back( IRS_KEY::EFFECTIVE_DATE.c_str() );				    values.push_back("");
		keys.push_back( IRS_KEY::MATURITY_DATE.c_str() );				    values.push_back("");
		keys.push_back( SWAP_EXPRESSION_KEY::RATE_OR_SPREAD1.c_str() );	    values.push_back("0.0");
		keys.push_back( SWAP_EXPRESSION_KEY::RATE_OR_SPREAD2.c_str() );	    values.push_back("0.0");

		LabelValueBlock swapExpressionLVB = populateLabelValueBlock( keys, values );

		return swapExpressionLVB;
	}

	/* @brief		A pivate helper which constructs a temporary interest rate swap (IRS) from the supplied SwapGeneratorName, and uses the swap to calculate a par rate.
	*  @param[in]	valuationSettingsLVB	The valuationSettingsLVB used to price the IRS
	*  @param[in]	swapGeneratorName	The name of the swapGenerator to use when constructing the IRS
	*  @param[in]	irsEffectiveDate	The effective (start) date of the IRS
	*  @param[in]	swapIndexMaturity	A string containing the IRS maturity tenor, e.g '5Y'
	*  @returns		The IRS par-rate
	*/
	double CMSSchedule::calculateUnderlyingParRate( const LabelValueBlock& valuationSettingsLVB, const AQLString& swapGeneratorName, const AQLDate& underlyingEffectiveDate, const AQLString& underlyingMaturity ) const
	{
        // Calculate the parRate for and IRS starting on each accrualStartDate
        LabelValueBlock swapPropertiesLVB;
		std::string swapName = "CMS_IRS";

		bool isXccySwap = false;
		LabelValueBlock fixingTable;

        StandardStringVector addKeys(2);
        addKeys[0] = IRS_KEY::EFFECTIVE_DATE;
        addKeys[1] = IRS_KEY::MATURITY_DATE;

        StandardStringVector addValues(2);
        addValues[0] = AQ_TO_STRING_FROM_INT( AQLDateSchedule::getExcelDate( underlyingEffectiveDate ) );
        const AQLDate irsMaturityDate = validateMaturityDate( underlyingEffectiveDate, underlyingMaturity );
        addValues[1] = AQ_TO_STRING_FROM_INT(AQLDateSchedule::getExcelDate( irsMaturityDate ) );

        LabelValueBlock swapExpressionLVB( setupSwapExpressionLVBForCMS(), addKeys, addValues );

		auto swapInstrument = createSwapFromGenerator( swapName, swapGeneratorName.getCString(), swapExpressionLVB, swapPropertiesLVB, isXccySwap );
		const double parRate = swapInstrument->parRate( valuationSettingsLVB, fixingTable );

		return parRate;
	}

	/* @brief		updates the cashflows in this CMS leg with the float rate calculated from swap index par rates
	*  @param[in]	cmsStaticData		Contains the CMS static data parameters ( swap index, maturity, multiplier )
	*/
	void CMSSchedule::initializeDataProviderWithCMSParameters( DataProvider& dataProvider, const std::shared_ptr<CMSStaticData>& cmsLegStaticData ) const
	{
		LabelValueBlock valuationSettingsLVB( VALUATION_SETTING_KEYS::CURVE_COLLECTION, dataProvider.getValuationSettings().getCurveCollection() );

		const AQLString& swapGeneratorName1 = cmsLegStaticData->getSwapGeneratorName1();
		const AQLString& swapIndexMaturity1 = cmsLegStaticData->getSwapIndexMaturity1();
		const double swapIndexMultiplier1  = cmsLegStaticData->getSwapIndexMultiplier1();

		const AQLString& swapGeneratorName2 = cmsLegStaticData->getSwapGeneratorName2();
		const AQLString& swapIndexMaturity2 = cmsLegStaticData->getSwapIndexMaturity2();
		const double swapIndexMultiplier2  = cmsLegStaticData->getSwapIndexMultiplier2();

		const size_t cashflowSize = cashflows_.size();
		std::vector<FloatRateData> floatRates( cashflowSize );

		for ( size_t i=0; i<cashflowSize; i++ )
		{
			CashflowPtr curCashflow = cashflows_[i];
			std::shared_ptr<CMSCashflow> cmsCashflow = std::dynamic_pointer_cast<CMSCashflow>( curCashflow );
			AQ_REQUIRE( cmsCashflow != nullptr,  "Found a non CMS cashflow in the CMS Schedule! ");

			const AQLDate& irsEffectiveDate = cmsCashflow->getAccrualStartDate();	

			double parRate1 = calculateUnderlyingParRate( valuationSettingsLVB, swapGeneratorName1, irsEffectiveDate, swapIndexMaturity1 );
			cmsCashflow->setParRate1( parRate1 );
			cmsCashflow->setMultiplier1( swapIndexMultiplier1 );

			double floatRate = swapIndexMultiplier1 * parRate1;
			
			if ( swapIndexMultiplier2 != 0 )
			{
				double parRate2 = calculateUnderlyingParRate( valuationSettingsLVB, swapGeneratorName2, irsEffectiveDate, swapIndexMaturity2 );
				cmsCashflow->setParRate2( parRate2 );
				cmsCashflow->setMultiplier2( swapIndexMultiplier2 );

				floatRate = floatRate + swapIndexMultiplier2 * parRate2;
			}

			floatRates[ i ] = floatRate;
		}
		dataProvider.setFloatRates( floatRates );
	
	}

}
