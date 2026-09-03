#pragma once

#include <string>
#include "Swap.h"

namespace etrading
{

    class CrossCurrencySwap : public Swap
    {
    public:
        CrossCurrencySwap(const std::string& instanceName, const SwapTypeEnum swapType, const LabelValueBlock& swapPropertiesLVB);
        CrossCurrencySwap(const std::string& instanceName, const LegPtr& leg1, const LegPtr& leg2, const LabelValueBlock& swapPropertiesLVB);
   		CrossCurrencySwap(const CrossCurrencySwap& rhs);
        virtual ~CrossCurrencySwap() {}
        std::shared_ptr<Swap> clone();

        void addToLegCollection(const LegPtr& leg);	

		//Override
        double spread(const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames, bool isParSpread, const LAString& spreadLegName="");

        double pv(const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames, const LAString& legName="");
        double pv01(const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames);         // Swap pv01 = dPV/dParRate

        double annuity(const LabelValueBlock& valuationSettingsLVB, const LAString& legName);
        virtual double parRate(const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames);

		std::vector<AnyTypeMatrix> view(const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames, const LAString& legName = "", bool showColumnHeaders = true, const std::unordered_set<CashflowHeaderEnum,EnumClassHash>& columnList = std::unordered_set<CashflowHeaderEnum,EnumClassHash>());
    
		double getFxAsOfDateRate(const LabelValueBlock& valuationSettingsLVB);

		static std::vector<std::string> swapPropertiesKeys()
		{
            const std::string arr[] =
			{
				IRS_KEY::MTM
				,IRS_KEY::NOTIONAL_FX_RESET_LEG
				, IRS_KEY::VALUATION_CURRENCY
				, IRS_KEY::FX_RATE // Needed for Xccy Zero Coupon Swaps
			};

			std::vector<std::string> myKeys(arr, arr + sizeof(arr) / sizeof(arr[0]));

            std::vector<std::string> expectedKeys = Swap::swapPropertiesKeys();

            expectedKeys.insert(expectedKeys.end(), myKeys.begin(), myKeys.end());

            return expectedKeys;
		}

    protected:

        //For XCCY Swap, user need to specify if it is a MTM, and if it is MTM, user can specify which leg to have notional adjusted (USD leg by default)
        bool isMTM_;
        LAString notionalResetLegName_;
        CCY valuationCurrency_;

        void populateAndValidateXccySwapStaticDataObject(const LegPtr& leg1, const LegPtr& leg2, const LabelValueBlock& swapPropertiesLVB);

        void validateCollectionSize(const LabelValueBlock& valuationSettingsLVB, const LAString& legName="") const;

		//update valuationSettingLVB with asOfDate fxRate, and do the common precalculation for every pricing function.
		virtual LabelValueBlock preCalculate(const LabelValueBlock& valuationSettingsLVB);
	};

}
