#include "CrossCurrencySwap.h"
#include "SwapCalculation.h"
#include "SettingsValidation.h"

namespace etrading
{

    CrossCurrencySwap::CrossCurrencySwap(const std::string& instanceName, const SwapTypeEnum swapType, const LabelValueBlock& swapPropertiesLVB) : Swap(instanceName, swapType, swapPropertiesLVB)
    {}

    CrossCurrencySwap::CrossCurrencySwap(const std::string& instanceName, const LegPtr& leg1, const LegPtr& leg2, const LabelValueBlock& swapPropertiesLVB) : Swap(instanceName, CROSS_CURRENCY_SWAP, swapPropertiesLVB)
	{
		validateLegs(leg1, leg2);

        AQ_THROW_IF( leg1->getStaticData()->getCurrency() == leg2->getStaticData()->getCurrency(), "Invalid Xccy Swap: Leg Notionals are in the same currency" );

        AQ_THROW_IF( boost::math::isnan(leg1->getSchedule()->getNotional()) || boost::math::isnan(leg2->getSchedule()->getNotional()), "Invalid Xccy Swap Notional(s)" );

        //Handle XccySwap properties
        populateAndValidateXccySwapStaticDataObject(leg1, leg2, swapPropertiesLVB);

        addToLegCollection(leg1);
		addToLegCollection(leg2);

	}

    void CrossCurrencySwap::addToLegCollection(const LegPtr& leg)
	{
        auto legName = leg->getLegName();

		//Set Default Swap level fwdInter flag:addToLegCollectionaddToLegCollection
		setDefaultFwdInterFlag(leg);

        // The leg is valid to add if: 1) if it doesn't already exist i.e. don't add fee legs in duplicate, and 2) the leg's currency is one of the swap first two legs' currencies 
		auto legCcy = leg->getStaticData()->getCurrency();

        bool validLegCurrency = (legs_.size() < 2) || legs_.currencyExists(legCcy);

        if (!legs_.exists(legName) && validLegCurrency)  
        {
            if (valuationCurrency_ != NO_CCY)
            {
                leg->getStaticData()->setValuationCurrency(valuationCurrency_);
            }

            legs_.add(leg);
        }

    }

    CrossCurrencySwap::CrossCurrencySwap(const CrossCurrencySwap& rhs) : Swap(rhs),
                                isMTM_(rhs.isMTM_),
                                notionalResetLegName_(rhs.notionalResetLegName_), 
                                valuationCurrency_(rhs.valuationCurrency_)
    {}

    std::shared_ptr<Swap> CrossCurrencySwap::clone()
    {
        SwapPtr swap = SwapPtr(new CrossCurrencySwap(*this));
        return swap;
    }

    void CrossCurrencySwap::populateAndValidateXccySwapStaticDataObject(const LegPtr& leg1, const LegPtr& leg2, const LabelValueBlock& swapPropertiesLVB)
    {

        //Swap level properties take priority than the leg level ones
        if (swapPropertiesLVB.size() != 0)
        {
            valuationCurrency_ = toCCYEnum(swapPropertiesLVB.getOptionalValueAsAQLString(IRS_KEY::VALUATION_CURRENCY).getCString());
            isMTM_ = swapPropertiesLVB.getOptionalValueAsBool(IRS_KEY::MTM, false);
            notionalResetLegName_ = swapPropertiesLVB.getOptionalValueAsAQLString(IRS_KEY::NOTIONAL_FX_RESET_LEG);
        }
        else
        {
            // Swap level properties' default values
            valuationCurrency_ = NO_CCY;
            isMTM_ = false;
            notionalResetLegName_ = "";
        }

        AQ_REQUIRE( valuationCurrency_ != NO_CCY, "Missing Valuation Currency - For Xccy Swap valuation currency is required" )

        AQ_THROW_IF( (valuationCurrency_ != leg1->getStaticData()->getCurrency()) 
                    && (valuationCurrency_ != leg2->getStaticData()->getCurrency()), "Invalid Valuation Currency, For Xccy Swaps valuationCurrency must be one of the swap legs' currencies" );

		// 1) Clear the Notional Reset Leg Name if specified on a non-MtM Xccy Swap (only required for MtM Xccy Swaps)
		if( !isMTM_ )
		{
			notionalResetLegName_ = AQLString();
		}

        if (notionalResetLegName_.size() != 0)
        {
            // 2) notionalResetLeg provided and MTM true, check if the legName is valid 
            AQ_THROW_IF( !same(leg1->getLegName(), notionalResetLegName_) && !same(leg2->getLegName(), notionalResetLegName_), "Invalid Xccy Swap Notional Reset Leg: The NotionalFxResetLeg must match one of the swap legs" );
        }
        else
        {
            // 3) notionalResetLeg not provided and MTM true, use USD leg name as default, if no USD leg, user need to provided one 
            if (isMTM_)
            {
                AQLString defaultNotionalResetLeg ="";
                if (leg1->getStaticData()->getCurrency() == USD)
                {
                    defaultNotionalResetLeg = leg1->getLegName();
                }
                else if (leg2->getStaticData()->getCurrency() == USD)
                {
                    defaultNotionalResetLeg = leg2->getLegName();
                }

                notionalResetLegName_ = defaultNotionalResetLeg;

                AQ_THROW_IF( notionalResetLegName_.size() == 0, "Invalid Xccy Swap Notional Reset Leg: For MTM XCCY Swaps we must specify a NotionalResetLeg" );
            }
            else
            {
                // 4) notionalResetLeg not provided and MTM false, do nothing
            }
        }
    }

    void CrossCurrencySwap::validateCollectionSize(const LabelValueBlock& valuationSettingsLVB, const AQLString& legName) const
    {

        Swap::validateCollectionSize(valuationSettingsLVB, legName);

		size_t legSize = legs_.size();

		for (size_t i = 0; i < legs_.size(); ++i)
		{
			AQ_THROW_IF( getAQObjCurveCollectionFromValuationSettings(valuationSettingsLVB, legs_.get(i)->getLegName()).size() == 0, "Invalid Valuation Settings: For Xccy Swaps the number of Valuation Settings blocks must match the number of trade legs" );

		}

    }

	double CrossCurrencySwap::getFxAsOfDateRate(const LabelValueBlock& valuationSettingsLVB)
	{
		auto asOfDateFxRate = calculateAsOfDateFxRate(valuationSettingsLVB, valuationCurrency_, legs_.get(0), legs_.get(1));
		return asOfDateFxRate;
	}


	LabelValueBlock CrossCurrencySwap::preCalculate (const LabelValueBlock& valuationSettingsLVB)
	{
		validateCollectionSize(valuationSettingsLVB);

		auto asOfDateFxRate = calculateAsOfDateFxRate(valuationSettingsLVB, valuationCurrency_, legs_.get(0), legs_.get(1));
		LabelValueBlock valSettingLVB(valuationSettingsLVB, VALUATION_SETTING_KEYS::FX_AS_OF_DATE_RATE, boost::lexical_cast<std::string>(asOfDateFxRate));

		updateMTMXccyNotionalResetByFxLeg(valSettingLVB, legs_.get(0), legs_.get(1), notionalResetLegName_, isMTM_, "");

		return valSettingLVB;
	}

    double CrossCurrencySwap::spread(const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames, bool isParSpread, const AQLString& spreadLegName)
	{
		LabelValueBlock valSettingLVB = preCalculate(valuationSettingsLVB);

        double ret = calculateXccySwapSpread(legs_, spreadLegName, notionalResetLegName_, valSettingLVB, fixingTableNames, isParSpread);
        return ret;	
    }

    double CrossCurrencySwap::pv(const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames, const AQLString& legName)
	{
		LabelValueBlock valSettingLVB = preCalculate(valuationSettingsLVB);

        double ret = Swap::pv(valSettingLVB, fixingTableNames, legName);
        return ret;
	}

    double CrossCurrencySwap::pv01(const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames)
	{
		LabelValueBlock valSettingLVB = preCalculate(valuationSettingsLVB);

        double ret = Swap::pv01(valSettingLVB, fixingTableNames);
        return ret;
    }

    double CrossCurrencySwap::annuity(const LabelValueBlock& valuationSettingsLVB, const AQLString& legName)
	{
		LabelValueBlock valSettingLVB = preCalculate(valuationSettingsLVB);
		
		double ret = Swap::annuity(valSettingLVB, legName);
        return ret;
	}

    double CrossCurrencySwap::parRate(const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames)
    {
		LabelValueBlock valSettingLVB = preCalculate(valuationSettingsLVB);

        double ret = Swap::parRate(valSettingLVB, fixingTableNames);
		return ret;
	}

  	std::vector<AnyTypeMatrix> CrossCurrencySwap::view(const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames, const AQLString& legName, bool showColumnHeaders, const std::unordered_set<CashflowHeaderEnum,EnumClassHash>& columnList)
	{
		LabelValueBlock valSettingLVB = preCalculate(valuationSettingsLVB);

        auto ret = Swap::view(valSettingLVB, fixingTableNames, legName, showColumnHeaders, columnList);
        return ret;
    }



}

