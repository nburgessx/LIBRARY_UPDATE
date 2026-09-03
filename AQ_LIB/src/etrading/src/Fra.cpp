/*
 * @brief			Class the defines the float leg
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */
#include "Fra.h"
#include "FraCashflow.h"
#include "SettingsValidation.h"

namespace etrading
{

	Fra::Fra(const std::string& instanceName) : FloatLeg(instanceName)
    {}

    Fra::Fra(const std::string& instanceName, const LabelValueBlock& fraLVB, const SchedulePtr& schedule) : FloatLeg(instanceName)
	{
		const std::string inputLVB = "fraLVB";

    	//Float Static Data
        legStaticData_ = LegStaticDataPtr (new FloatStaticData(fraLVB));

		inputParameters_ = fraLVB;

    	//Fra Schedule
		if (schedule==nullptr)
		{
			schedule_ = SchedulePtr(new FraSchedule(fraLVB, instanceName));
		}
		else
		{
			//if schedule is not given, the fraLVB will only contains the FRA description part ...
            // ... therefore call append constructor
            inputParameters_ = LabelValueBlock( inputParameters_, schedule_->getInputParameters() );
		}
	}

    LegPtr Fra::clone()
    {
        LegPtr myFra = LegPtr(new Fra(*this));
        return myFra;
    }

    Fra::Fra( const Fra& rhs) : FloatLeg(rhs)	
	{}

    ScheduleTypeEnum Fra::getType() const
    {
        return FRA_SCHEDULE_TYPE;
    }

	//Return the fixed strikeRate that make the FRA PV zero
	double Fra::fraRate(const LabelValueBlock& valuationSettingsLVB)
	{
		//load discount factors & floatRates from the curve
		DataProvider dataProvider(ValuationSettings(valuationSettingsLVB, {}, getLegName()));
		initializeDataProvider( dataProvider);
		
		//*** Strike Rate K formula: FraPV = 0 => sumOf(Notional*tao_i*(L_i-k)/(1+tao_i*L_i)*DF_i) = 0 => k = sumOf(tao_i*L_i*DF_i/(1+tao_i*L_i))/sumOf(tao_i*DF_i/(1+tao_i*L_i)) ---

		// sumOf(tao_i*L_i*DF_i/(1+tao_i*L_i))
		double sumOfLiborRateAmount = 0.0;

		// sumOf(tao_i*DF_i/(1+tao_i*L_i)) 
		double sumOfAnnuityAmount = 0.0;

		CashflowPtr cf;

		// getCashflowSize() excludes the notional exchange upfront cashflow
		for (unsigned int i = 0; i < schedule_->getCashflowSize(); i++)
		{
			cf = schedule_->getCashflow(i);
			CashflowData cashflowData = dataProvider.getCashflowDataExcludingUpfront( i );
			double discountFactor = cashflowData.discountFactor;
			double floatRate = cashflowData.floatRateData.resetRate;
			double accrualYearFaction = cf->getAccrualYearFraction();

			// Ignore annuity terms if they are fully in the past
			if (MLIB_IS_GREATER_THAN_ZERO(discountFactor))
			{
				//double factor = 1.0 / (1.0 + accrualYearFaction * floatRate);

				double factor = discountFactor * accrualYearFaction / (1.0 + accrualYearFaction * floatRate);

				//tao_i*L_i*DF_i/(1+tao_i*L_i)
				sumOfLiborRateAmount += factor * floatRate;

				//tao_i*DF_i/(1+tao_i*L_i)
				sumOfAnnuityAmount += factor;
			}
		}

		MLIB_REQUIRE( !MLIB_IS_EQUAL_ZERO(sumOfAnnuityAmount), "Cannot calculate FraRate since Fra's annuity is zero.");

		//3) k = sumOf(tao_i*L_i*DF_i/(1+tao_i*L_i))/sumOf(tao_i*DF_i/(1+tao_i*L_i)) 
		const double parRate = sumOfLiborRateAmount / sumOfAnnuityAmount;

		return parRate;

	}

	//Return Future Price from FRA rate based on the meanReversion and volatility (convexity adjustment is calculated internally)
	double Fra::fraRateToFuturePrice(const LabelValueBlock& valuationSettingsLVB, const double& meanReversion, const double& volatility)
	{

		const LADate futuresStartDate = schedule_->getEffectiveDate();
		const LADate futuresEndDate = schedule_->getMaturityDate();

		ValuationSettings valSettings(valuationSettingsLVB, {}, getLegName());

		const LADate valuationDate = valSettings.getValuationDate();

		double convexityAdjustment = getCurveEuroDollarConvexityAdjustment(valuationDate, futuresStartDate, futuresEndDate, meanReversion, volatility);

		double fraRt = fraRate(valuationSettingsLVB);

		double futurePrice = fromFraRateToFuturePrice(fraRt, convexityAdjustment);

		return futurePrice;


	}

	//Return Future Price from FRA rate based on the given convexity adjustment
	double Fra::fraRateToFuturePrice(const LabelValueBlock& valuationSettingsLVB, const double& convexityAdjustment)
	{

		double fraRt = fraRate(valuationSettingsLVB);

		double futurePrice = fromFraRateToFuturePrice(fraRt, convexityAdjustment);

		return futurePrice;

	}

	std::unordered_set<CashflowHeaderEnum,EnumClassHash> Fra::allowedColumns() const
	{
		std::unordered_set<CashflowHeaderEnum,EnumClassHash> expectedList
		{
			FLOAT_RATE_HEADER
			, COUPON_HEADER
			, DISCOUNT_FACTOR_HEADER
			, COUPON_PV_HEADER
		};

		return expectedList;
	}



}


