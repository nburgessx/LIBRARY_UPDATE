#include "Option.h"
#include "SwapValidation.h"
#include "SwapUtilities.h"
#include <iomanip>
#include "StaticStructureStore.h"


namespace etrading
{

	Option::Option(const std::string& objectName) : IsAQObject(objectName, OPTION),
		inputParameters_(LabelValueBlock()),
		callPut_(NONE_OPTION), 
		scheduleParams_(),
		enrichedSchedule_()
	{}


    Option::Option( const Option& rhs) : IsAQObject(rhs.getRefToName(), OPTION), 
		inputParameters_(rhs.inputParameters_),
		callPut_(rhs.callPut_)
	{
		if (rhs.scheduleParams_ != nullptr)
		{
			scheduleParams_ = rhs.scheduleParams_->clone();
		}
		if (rhs.enrichedSchedule_ != nullptr)
		{
			enrichedSchedule_ = rhs.enrichedSchedule_->clone();
		}
	}

	//--- Start of caching related methods -- //

	const SchemaObject Option::toSchemaObject() const
	{
        SchemaObject schemaObject(OPTION, getRefToName());

		auto schemaName = toString(OPTION);

		//dynamic generate schema
		schemaObject.addDataSchema(generateDataSchema(schemaName));

		//add data to schema
		schemaObject.setDataForSchemaWithMap(schemaName, getDataMap());

		return schemaObject;
	}

	const DataSchema Option::generateDataSchema(const std::string& schemaName) const
    {
        std::string schName = (schemaName.size()!=0) ? schemaName : toString(OPTION);
	    const DataSchema  dynamicSchema(    schName,2,
                                            boost::assign::list_of(STRING_VALUE)(VARIANT_VALUE),
		                                    boost::assign::list_of("VARIABLE_NAME")("VARIABLE_VALUE"));
        return dynamicSchema;
    }

	std::map<std::string, Variant> Option::getDataMap() const
	{
        std::map<std::string, Variant> dataMap;
        auto keys = inputParameters_.getKeys();
        auto values = inputParameters_.getValues();
        for(size_t i=0; i < keys.size(); ++i) 
        {
			dataMap[keys[i]] = values[i];
        }
        return dataMap;
	}
	//--- End of caching related methods -- //

}

