/*
* @brief			Class the defines the Volatility surface object
* @Created:			05 Feb 2018
* @Author:			Yongyan Zheng
* @Department:		ISD Front Office Development
*
* The copyright to the computer program(s) herein is the property of Mizuho International.
*/

#include "Volatility.h"
#include "ParameterValidation.h"

namespace etrading
{

	Volatility::Volatility(const LabelValueBlock& volLVB, const std::string& objectName) : IsLWOObject(objectName, VOLATILITY), 
																						inputParameters_(volLVB)
	{
        // Validate Object Type
        const std::string inputLVB = "VolatilityLVB";
        CachedObjectEnum objectType = toCachedObjectEnum( volLVB.getCompulsoryValueAsString( OBJECT_KEY::OBJECT_TYPE, inputLVB ) );
        MLIB_REQUIRE( objectType == VOLATILITY, "Invalid Object Type - 'VOLATILITY' type required" )

		asOfDate_       = stringToDate(volLVB.getCompulsoryValueAsString(OPTION_KEYS::AS_OF_DATE, inputLVB).c_str(), "#Error: Invalid Volatility 'AsOfDate'");
		currency_       = toCCYEnum(volLVB.getCompulsoryValueAsString(IRS_KEY::CURRENCY, inputLVB));
        
        volDataSource_  = toVolatilityDataSourceEnum( volLVB.getCompulsoryValueAsString(VOLATILITY_KEYS::VOL_DATA_SOURCE, inputLVB) );
		volType_        = toVolatilityTypeEnum(volLVB.getCompulsoryValueAsString(VOLATILITY_KEYS::VOL_TYPE, inputLVB));
		vol_            = volLVB.getCompulsoryValueAsDouble( VOLATILITY_KEYS::VOLATILITY );

        // Shift-Size is required when the volatility type is SHIFTED-LOGNORMAL
        if ( volType_ == SHIFTED_LOGNORMAL_VOLATILITY )
        {
            shiftSize_ = volLVB.getCompulsoryValueAsDouble( VOLATILITY_KEYS::SHIFT_SIZE, inputLVB );
        }
        else
        {
            // Default ShiftSize is Zero when VolType is not Shifted-LogNormal Vol
            shiftSize_ = volLVB.getOptionalValueAsDouble( VOLATILITY_KEYS::SHIFT_SIZE, 0.0 );
        }

	}

    Volatility::Volatility(const Volatility& rhs) : IsLWOObject(rhs.getRefToName(), VOLATILITY), 
													inputParameters_(rhs.inputParameters_), 
													asOfDate_(rhs.asOfDate_),
													currency_(rhs.currency_),
													volDataSource_(rhs.volDataSource_),
													volType_(rhs.volType_),
													vol_(rhs.vol_),
                                                    shiftSize_(rhs.shiftSize_)
	{}

	//--- Start of caching related methods -- //

	const SchemaObject Volatility::toSchemaObject() const
	{
		SchemaObject schemaObject(VOLATILITY, getRefToName());

		auto schemaName = toString(VOLATILITY);

		//dynamic generate schema
		schemaObject.addDataSchema(generateDataSchema(schemaName));

		//add data to schema
		schemaObject.setDataForSchemaWithMap(schemaName, getDataMap());

		return schemaObject;
	}

	const DataSchema Volatility::generateDataSchema(const std::string& schemaName) const
	{
		std::string schName = (schemaName.size() != 0) ? schemaName : toString(VOLATILITY);
		const DataSchema  dynamicSchema(schName, 2,
			boost::assign::list_of(STRING_VALUE)(VARIANT_VALUE),
			boost::assign::list_of("VARIABLE_NAME")("VARIABLE_VALUE"));
		return dynamicSchema;
	}

	std::map<std::string, Variant> Volatility::getDataMap() const
	{
		std::map<std::string, Variant> dataMap;
		auto keys = inputParameters_.getKeys();
		auto values = inputParameters_.getValues();
		for (size_t i = 0; i < keys.size(); ++i)
		{
			dataMap[keys[i]] = values[i];
		}
		return dataMap;
	}
	//--- End of caching related methods -- //
	

}

