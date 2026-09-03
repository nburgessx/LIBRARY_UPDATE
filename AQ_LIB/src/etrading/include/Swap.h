/*
 * @brief			Class the defines the interest rate swap instrument
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include <string>
#include <vector>

#include "LegCollection.h"
#include "IsLWOObject.h"
#include "SchemaObject.h"
#include "CoreEnumerations.h"
#include "ParameterValidation.h"
#include "FloatLeg.h"
#include "FixedLeg.h"

namespace etrading
{

    class Swap : public IsLWOObject 
    {
	public:
        Swap(const std::string& instanceName, const SwapTypeEnum swapType, const LabelValueBlock& swapPropertiesLVB);
   		Swap(const Swap& rhs);
		virtual ~Swap() {}
        virtual std::shared_ptr<Swap> clone()=0;

        const DataSchema generateDataSchema(const std::string& schemaName="") const;
        std::map<std::string, Variant> getDataMap() const;
        const SchemaObject toSchemaObject() const;

		virtual void addToLegCollection(const LegPtr& leg);	
		virtual void setDefaultFwdInterFlag(const LegPtr& leg);

        LegPtr getLeg( const std::string & legName ) const;
		LegPtr getLeg( size_t i ) const;	
		size_t getLegSize() const;	

		SwapTypeEnum getSwapType() const;

		virtual double getFxAsOfDateRate(const LabelValueBlock& valuationSettingsLVB);

        virtual double pv(const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames, const LAString& legName = "");
        virtual double pv01(const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames);         // Swap pv01 = dPV/dParRate
        virtual double annuity(const LabelValueBlock& valuationSettingsLVB, const LAString& legName, const bool & includeSign = false);
        virtual double parRate(const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames);
        virtual double spread(const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames, bool isParSpread, const LAString& spreadLegName="");
		double accruedInterest(const LabelValueBlock& valuationSettingsLVB, const LAString& legName, const LabelValueBlock& fixingTableNames);
        
        // Method to check if the swap is a single currency swap
        bool isSingleCurrency();
        
        /* @brief	Generate a display-friendly vector of the swap cashflows.
		*
		*  @param[in]	modelNames			A LabelValue block containing model names or curve collection names per leg. Used to initialize dynamic data in the cashflows (such as discount factor)
		*  @param[in]	legName				Optional: If specified, limits the output to just that leg, otherwise output for all swap legs is generated
		*  @param[in]	fixingTableNames		Optional: A map of historical rate fixings
		*  @param[in]	showColumnHeaders	Optional: Whether to label each column with a header string
		*  @param[in]	columnList			Optional: The columns to generate. If missing, all cashflow data is displayed
		*
		*  Returns a vector of Header-Body matrix pairs
		*/ 
        virtual std::vector<AnyTypeMatrix> view(const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames, const LAString& legName="", bool showColumnHeaders=true, const std::unordered_set<CashflowHeaderEnum, EnumClassHash>& columnList=std::unordered_set<CashflowHeaderEnum, EnumClassHash>());

        // return a pair of Header matrix and Body matrix
        std::vector<std::pair<AnyTypeMatrix, AnyTypeMatrix>> viewInputParameters() const;

        static std::vector<std::string> swapPropertiesKeys()
		{
			std::vector<std::string> expectedKeys; //empty for now
        	return expectedKeys;
		}

 		// These keys are used for: 1) the swap created from a single label value block, 2) display order of the input parameters
		static std::vector<std::string> swapLegCommonLVBKeys()
		{
			std::vector<std::string> expectedKeys = LegStaticData::lvbKeys();

			std::vector<std::string> scheduleLVB = Schedule::lvbKeys();
			expectedKeys.insert(expectedKeys.end(), scheduleLVB.begin(), scheduleLVB.end());

            return expectedKeys;
		}

        static std::vector<std::string> fixedfixedSwapLvbKeys()
        {
            std::vector<std::string> expectedKeys = FixedLeg::lvbKeys();

            std::vector<std::string> leg2Keys = FixedLeg::lvbKeys();
            expectedKeys.insert( expectedKeys.end(), leg2Keys.begin(), leg2Keys.end() );
            return expectedKeys;
        }

        static std::vector<std::string> fixedfloatSwapLvbKeys()
        {
            std::vector<std::string> expectedKeys = FixedLeg::lvbKeys();

            std::vector<std::string> leg2Keys = FloatLeg::lvbKeys();
            expectedKeys.insert( expectedKeys.end(), leg2Keys.begin(), leg2Keys.end() );
            return expectedKeys;
        }

        static std::vector<std::string> floatfloatSwapLvbKeys()
        {
            std::vector<std::string> expectedKeys = FloatLeg::lvbKeys();

            std::vector<std::string> leg2Keys = FloatLeg::lvbKeys();
            expectedKeys.insert( expectedKeys.end(), leg2Keys.begin(), leg2Keys.end() );
            return expectedKeys;
        }


	protected:
        LegCollection legs_;

        //input parameters of Swap properties LVB
		LabelValueBlock inputParameters_;

		SwapTypeEnum swapType_;

        virtual void validateCollectionSize(const LabelValueBlock& valuationSettingsLVB, const LAString& legName="") const;

        void validateLegs(const LegPtr& leg1, const LegPtr& leg2) const;

    };

	typedef std::shared_ptr<Swap> SwapPtr;

}
