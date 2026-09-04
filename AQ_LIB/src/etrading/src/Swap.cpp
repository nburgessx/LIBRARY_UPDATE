#include "Swap.h"
#include "SwapCalculation.h"
#include "AQLCurvePricingObject.h"
#include "SettingsValidation.h"

namespace etrading
{

    Swap::Swap(const std::string& instanceName, const SwapTypeEnum swapType, const LabelValueBlock& swapPropertiesLVB) : IsAQObject(instanceName, SWAP_OBJECT), swapType_(swapType), inputParameters_(swapPropertiesLVB)
	{}

    void Swap::validateLegs(const LegPtr& leg1, const LegPtr& leg2) const
    {
		if(leg1 == nullptr || leg2 == nullptr)
		{
			throw AQLCoreInvalidData( "#Error: One of the legs is NOT set on the Swap", __FILE__, __LINE__ );
		}

        if(same(leg1->getLegName(), leg2->getLegName()))
		{
			throw AQLCoreInvalidData( "#Error: Swap's two legs cannot have the same leg name", __FILE__, __LINE__ );
		}



    }

    Swap::Swap(const Swap& rhs) : IsAQObject(rhs.getRefToName(), SWAP_OBJECT), 
                                swapType_(rhs.swapType_),
                                inputParameters_(rhs.inputParameters_)
	{
		for(size_t i =0 ; i < rhs.legs_.size(); ++i)
		{
            auto leg = rhs.legs_.get(i);
			if(leg != nullptr)
			{
				addToLegCollection(leg->clone()); 
			}
		}
	}

	void Swap::setDefaultFwdInterFlag(const LegPtr& leg)
	{
		//Set Default Swap level fwdInter flag, but the user specified value will take priority

		//Only one leg exists, set the fwdInter flag for the existing leg and the new leg as follows:
		if (legs_.size() == 1)
		{
			auto firstLeg = legs_.get(0);

			// By Default, if the flag is NOT set by the user, fwdInterp set to TRUE only when it's a Single Currency FixedFloatSwap, for other cases it's false
			
			//1) For FloatfloatSwap: FALSE
			if ( firstLeg->getType() == FLOAT_SCHEDULE_TYPE && leg->getType() == FLOAT_SCHEDULE_TYPE )
			{
				auto defaultFwdInter = FALSE_BOOL;
				firstLeg->getStaticData()->setFwdInter(defaultFwdInter);
				leg->getStaticData()->setFwdInter(defaultFwdInter);
			}
			else
			{
				//2.1) For Xccy FixedFloatSwap: default to FALSE
				if (firstLeg->getStaticData()->getCurrency() != leg->getStaticData()->getCurrency() )
				{
					if( leg->getType() == FLOAT_SCHEDULE_TYPE)
					{
						leg->getStaticData()->setFwdInter(FALSE_BOOL);
					} 
					else if ( firstLeg->getType() == FLOAT_SCHEDULE_TYPE )
					{
						firstLeg->getStaticData()->setFwdInter(FALSE_BOOL);
					}
				}
				else
				{
				// 2.2) For Single Currency FixedFloatSwap, we don't know whether it is a STD curve or not at this stage, so we do not set any default here, 
				// the fwdInter value will be retrieved from the curve (ref: getfwdInterInfo() of CurveValidation) 
				}
			}
		}

		//If a swap have more than two legs, all other legs will have fwdInter flag as FALSE
		else if (legs_.size() >= 2 && leg->getType() == FLOAT_SCHEDULE_TYPE)
		{
			leg->getStaticData()->setFwdInter(FALSE_BOOL);
		}
	}


  	void Swap::addToLegCollection(const LegPtr& leg)
	{
        auto legName = leg->getLegName();

		//Set Default Swap level fwdInter flag:
		setDefaultFwdInterFlag(leg);

		// The leg is valid to add if it doesn't already exist i.e. don't add fee legs in duplicate
        if (!legs_.exists(legName))  
        {
            legs_.add(leg);
        }


    }

    LegPtr Swap::getLeg( const std::string & legName ) const
    {
        // Iterate over each leg and return the leg if the name matches
        for (size_t i = 0; i < legs_.size(); ++i)
        {
            auto leg = legs_.get(i);
            if (same(leg->getLegName(), legName))
            {
                return leg;
            }
        }
        AQ_THROW("Swap leg with name '" + legName + "' not found");
    }

	LegPtr Swap::getLeg(size_t i) const
	{
		return legs_.get(i);
	}

    size_t Swap::getLegSize() const
    {
        return legs_.size();
    }

	SwapTypeEnum Swap::getSwapType() const
	{
		return swapType_;
	}

    void Swap::validateCollectionSize(const LabelValueBlock& valuationSettingsLVB, const AQLString& legName) const
    {

        if (valuationSettingsLVB.size() == 0)
        {
    		throw AQLCoreInvalidData( "#Error: A curveCollection must be provided", __FILE__, __LINE__ );
        }

        //LegName is specified
        if (legName.size() != 0 && !legs_.exists(legName))
        {
            throw AQLCoreInvalidData( "#Error: Leg name does not exist in the swap", __FILE__, __LINE__ );
        }

    }
   
	double Swap::getFxAsOfDateRate(const LabelValueBlock& valuationSettingsLVB)
	{
		AQ_THROW("getFxAsOfDateRate() not supported.");
	}

	double Swap::pv(const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames, const AQLString& legName)
	{
        validateCollectionSize(valuationSettingsLVB, legName);

        double ret = 0;

        if (legName.size() == 0)
        {
            ret = legs_.pv(valuationSettingsLVB, fixingTableNames);
            return ret;
        }

        for (size_t i = 0; i < legs_.size(); ++i)
        {
            auto leg = legs_.get(i);
            if (same(leg->getLegName(), legName))
            {
                DataProvider dataProvider(ValuationSettings(valuationSettingsLVB, fixingTableNames, leg->getLegName()));
                ret = leg->pv( dataProvider, false);
                break;
            }
        }
        return ret;
	}

	double Swap::parRate(const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames)
    {
        validateCollectionSize(valuationSettingsLVB);

		double parRate = calculateParRate(valuationSettingsLVB, legs_, fixingTableNames);

		return parRate;
	}

    double Swap::pv01(const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames)
	{

        validateCollectionSize(valuationSettingsLVB);

        //PV01: dPV/dParRate, is from the first fixed leg's annuity
		LegPtr leg1 = legs_.get(0);
		LegPtr leg2 = legs_.get(1);

        double pv01 = 0.0;

        // Only calculate PV01 for (Fixed / Float) Swap, or (FixedBond / Float) Swap
		// NOTE that we return 0 for any other type of swap, in case users request PV01 for whole portfolios.
		//
		const bool fixedFloatSwap = ( ( (leg1->getType() == FIXED_SCHEDULE_TYPE) || (leg1->getType() == SWAPSCHEDULE_FIXEDBOND) )  && leg2->getType() == FLOAT_SCHEDULE_TYPE );
        if (fixedFloatSwap)
        {

			DataProvider dataProvider(ValuationSettings(valuationSettingsLVB, fixingTableNames, leg1->getLegName()));

			double firstFixedAnnuity = leg1->annuityWithNotional(dataProvider);

            double oneBasisPoint = 0.0001;

            //PV01 is always positive
		    pv01 = oneBasisPoint * firstFixedAnnuity;
        }

        return pv01;
	}


    double Swap::annuity(const LabelValueBlock& valuationSettingsLVB, const AQLString& legName, const bool & includeSign)
	{
	    double result = 0;

        if (legName.size() == 0)
        {
           throw AQLCoreInvalidData( "#Error: Leg name must be provided", __FILE__, __LINE__ );
        }

        if (!legs_.exists(legName)) 
        {
            throw AQLCoreInvalidData( "#Error: Leg name does not exist in the swap", __FILE__, __LINE__ );
        }

        for (size_t i = 0; i < legs_.size(); ++i)
        {
            auto leg = legs_.get(i);
            if (same(leg->getLegName(), legName))
            {
				DataProvider dataProvider(ValuationSettings(valuationSettingsLVB, {}, leg->getLegName()));
				result = leg->annuityWithNotional(dataProvider);
                if ( includeSign )
                {
                    result *= leg->getSchedule()->getPayRecIndicator();
                }
				break;
            }
        }
        return result;
	}

    double Swap::spread(const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames, bool isParSpread, const AQLString& spreadLegName)
	{
        validateCollectionSize(valuationSettingsLVB);

        double spread = calculateBasisSwapSpread(legs_, spreadLegName, valuationSettingsLVB, fixingTableNames, isParSpread);

        return spread;	
    }

	double Swap::accruedInterest(const LabelValueBlock& valuationSettingsLVB, const AQLString& legName, const LabelValueBlock&  fixingTableNames)
	{
	    double ret = 0;

        if (legName.size() == 0)
        {
           throw AQLCoreInvalidData( "#Error: Leg name must be provided", __FILE__, __LINE__ );
        }

        if (!legs_.exists(legName)) 
        {
            throw AQLCoreInvalidData( "#Error: Leg name does not exist in the swap", __FILE__, __LINE__ );
        }

        for (size_t i = 0; i < legs_.size(); ++i)
        {
            auto leg = legs_.get(i);
            if (same(leg->getLegName(), legName))
            {
				DataProvider dataProvider (ValuationSettings(valuationSettingsLVB, fixingTableNames, leg->getLegName()));

				leg->initializeDataProvider(dataProvider);

				ret = leg->getSchedule()->calculateAccruedInterest(dataProvider, false);

				break;
            }
        }
        return ret;
	}

    // Method to check if the swap is a single currency swap
    bool Swap::isSingleCurrency()
    {
        return legs_.isSingleCurrency();
    }

	/* @brief	Generate a display-friendly vector of the swap cashflows.
	*
	*  @param[in]	valuationSettingsLVB	A LabelValue block containing model names or curve collection names per leg. Used to initialize dynamic data in the cashflows (such as discount factor)
	*  @param[in]	fixingTableNames		Optional: A map of historical rate fixings
	*  @param[in]	legName				Optional: If specified, limits the output to just that leg, otherwise output for all swap legs is generated
	*  @param[in]	showColumnHeaders	Optional: Whether to label each column with a header string
	*  @param[in]	columnList			Optional: The columns to generate. If missing, all cashflow data is displayed
	*
	*  Returns a vector of Header-Body matrix pairs
	*/ 
	std::vector<AnyTypeMatrix> Swap::view(const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames, const AQLString& legName, bool showColumnHeaders, const std::unordered_set<CashflowHeaderEnum,EnumClassHash>& columnList)
	{
        validateCollectionSize(valuationSettingsLVB, legName);

        //Find the matching legs
        std::vector<LegPtr> matchingLegs;
        for (size_t i = 0; i < legs_.size(); ++i)
		{
            auto leg = legs_.get(i);
            if (legName.size()==0) 
            {
                matchingLegs.push_back(leg);
            }
            else if (same(leg->getLegName(), legName))
            {
                matchingLegs.push_back(leg);
            }
        }

        if (matchingLegs.size() == 0)
        {
            throw AQLCoreInvalidData( "#Error: Cannot find matching leg name from the Swap", __FILE__, __LINE__ );
        }

        //Display the matching legs
        std::vector<AnyTypeMatrix> swapView;
        for (size_t i = 0; i < matchingLegs.size(); ++i)
		{
 			auto leg = matchingLegs[i];

			DataProvider dataProvider(ValuationSettings(valuationSettingsLVB, fixingTableNames, leg->getLegName()));

			AnyTypeMatrix legView = leg->view(dataProvider, showColumnHeaders, columnList);
			
	        AnyTypeMatrix newLegView;
			if( showColumnHeaders )
			{
				//Add extra line of header to distinguish which leg it is
				AnyTypeVector row;
				row.push_back(leg->getLegName() + " Leg Schedule & Cashflows");
				newLegView.push_back(row);
			}
			newLegView.insert(newLegView.end(), legView.begin(), legView.end());

			swapView.push_back(newLegView);
        }
        return swapView;

    }

	std::vector<std::pair<AnyTypeMatrix, AnyTypeMatrix>> Swap::viewInputParameters() const
	{
		if(legs_.size() < 2)
		{
			throw AQLCoreInvalidData( "#Error: Swap should have at least two legs", __FILE__, __LINE__ );
		}
		
		// Find the user-input key set from all legs
		std::set<std::string> legKeys;
		for (size_t i = 0; i < legs_.size(); ++i)
		{
			std::set<std::string> curKeys = legs_.get(i)->getInputParameters().getKeysAsSet();
			legKeys.insert(curKeys.begin(), curKeys.end());
		}

		// Sorted the user-input keys by the expected order 
		std::vector<std::string> orderedKeys;
		auto expectedKey = Swap::swapLegCommonLVBKeys();
		std::string key;
		for (size_t i = 0; i < expectedKey.size(); ++i)
		{
			key = expectedKey[i];
			if (legKeys.find(key) != legKeys.end())
			{
				orderedKeys.push_back(key);
			}
		}

        std::vector<std::pair<AnyTypeMatrix, AnyTypeMatrix>> swapView;

        AnyTypeMatrix result;
		AnyTypeVector row;

        AnyTypeMatrix header;
        AnyTypeVector headerRow;
 
        // If the Swap Properties are populated, we need to include the headers to distinquish swap properties input and swap leg LVB input
        if (inputParameters_.size() != 0)
        {
            //Header
		    headerRow.push_back( (std::string)("SWAP_PROPERTIES") );
		    header.push_back(headerRow);

            // 1) Swap properties input LVB
            auto spKeys = inputParameters_.getKeys();
            auto spValues = inputParameters_.getValues();
		    for (size_t i = 0; i < spKeys.size(); ++i)
		    {
			    row.clear();
			    row.push_back(spKeys[i]);
			    row.push_back(spValues[i]);
			    result.push_back(row);
		    }
            
            // Add Blank Row After Xccy Properties
            row.clear();
            result.push_back(row);
		    
            swapView.push_back(std::make_pair(header, result));

            // Add Swap legs' input LVB Header needed for Step 2 Below
            header.clear();
            headerRow.clear();
            headerRow.push_back( (std::string)("SWAP_LEG_PROPERTIES") );
		    header.push_back(headerRow);
        }

        // 2) Swap legs' input LVB
        // show common key and each leg value 
        result.clear();
		for (size_t i = 0; i < orderedKeys.size(); ++i)
		{
			key = orderedKeys[i]; 
			row.clear();
			row.push_back(key);
			for (size_t j = 0; j < legs_.size(); ++j)
			{
				auto lvb = legs_.get(j)->getInputParameters();
				row.push_back(lvb.getOptionalValueAsLAString(key));
			}
			result.push_back(row);
		}
		swapView.push_back(std::make_pair(header, result));

		return swapView;
	}

	//For swap level properties
    const DataSchema Swap::generateDataSchema(const std::string& schemaName) const
    {
        std::string schName = (schemaName.size()!=0) ? schemaName : toString(SWAP_OBJECT);
	    const DataSchema  dynamicSchema(    schName,2,
                                            boost::assign::list_of(STRING_VALUE)(VARIANT_VALUE),
		                                    boost::assign::list_of("VARIABLE_NAME")("VARIABLE_VALUE"));
        return dynamicSchema;
    }

	//For swap level properties
    std::map<std::string, Variant> Swap::getDataMap() const
    {
        std::map<std::string, Variant> dataMap;

        // For string variables, can be populated using inputParameters (LVB) directly
        auto keys = inputParameters_.getKeys();
        auto values = inputParameters_.getValues();
        for(size_t i=0; i < keys.size(); ++i) 
        {
   			dataMap[keys[i]] = values[i];
        }

        return dataMap;
    }

	const SchemaObject Swap::toSchemaObject() const
	{
		if(legs_.size() < 2)
		{
			throw AQLCoreInvalidData( "#Error: One of the legs is NOT set on the Swap", __FILE__, __LINE__ );
		}

		SchemaObject schemaObject( SWAP_OBJECT, getRefToName());

		//For swap level properties
		auto schemaName = toString(SWAP_OBJECT);
        schemaObject.addDataSchema(generateDataSchema(schemaName));
        schemaObject.setDataForSchemaWithMap(schemaName, getDataMap());

		//For legs
        for (size_t i = 0; i < legs_.size(); ++i)
        {
            auto leg = legs_.get(i);
            
            leg->toSchemaObject(schemaObject, i);
        }

		return schemaObject;
	}



}

