#include "SwapGenerator.h"
#include "ParameterValidation.h"

namespace etrading
{

	SwapGenerator::SwapGenerator(const std::string& instanceName, const LegGenerator& leg1, const LegGenerator& leg2) : IsAQObject(instanceName, SWAP_GENERATOR)
	{
		legGenerators_.push_back(leg1);
    	legGenerators_.push_back(leg2);
    }

    SwapGenerator::SwapGenerator(const SwapGenerator& rhs) : IsAQObject(rhs.getRefToName(), SWAP_GENERATOR), legGenerators_(rhs.legGenerators_)
	{}

    LegGenerator SwapGenerator::getLegGenerator(size_t i) const
    {
        return legGenerators_.at(i);
    }

	AQLStringMatrix SwapGenerator::viewInputParameters() const
	{
		// Find the user-input key set from all legs
		std::set<std::string> legKeys;
		for (size_t i = 0; i < legGenerators_.size(); ++i)
		{
			std::set<std::string> curKeys = legGenerators_[i].getInputParameters().getKeysAsSet();
			legKeys.insert(curKeys.begin(), curKeys.end());
		}

		// Sorted the user-input keys by the expected order 
		std::vector<std::string> orderedKeys;
		auto expectedKey = SwapGenerator::lvbKeys();
		std::string key;
		for (size_t i = 0; i < expectedKey.size(); ++i)
		{
			key = expectedKey[i];
			if (legKeys.find(key) != legKeys.end())
			{
				orderedKeys.push_back(key);
			}
		}

		// Show common key and each leg value 
		AQLStringMatrix result;
		AQLStringVector row;
		for (size_t i = 0; i < orderedKeys.size(); ++i)
		{
			key = orderedKeys[i]; 
			row.clear();
			row.push_back(key.c_str());
			for (size_t j = 0; j < legGenerators_.size(); ++j)
			{
				auto lvb = legGenerators_[j].getInputParameters();
				row.push_back(lvb.getOptionalValueAsLAString(key));
			}
			result.push_back(row);
		}

		return result;
	}

	const SchemaObject SwapGenerator::toSchemaObject() const
	{
		SchemaObject schemaObject( SWAP_GENERATOR, getRefToName());
        for (size_t i = 0; i < legGenerators_.size(); ++i)
        {
            auto leg = legGenerators_[i];
            std::string legGeneratorSchema = addIndexToSchemaName(toString(LEG_GENERATOR), i);
            leg.toSchemaObject(schemaObject, legGeneratorSchema);
        }
		return schemaObject;
	}



}

