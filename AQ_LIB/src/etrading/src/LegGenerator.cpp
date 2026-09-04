#include "LegGenerator.h"

namespace etrading
{

    LegGenerator::LegGenerator() : inputParameters_(LabelValueBlock())  
    {}

	LegGenerator::LegGenerator(const LabelValueBlock& legGeneratorLVB) 
	{
        inputParameters_ = legGeneratorLVB;
	}

    LegGenerator::LegGenerator( const LegGenerator& rhs) : inputParameters_(rhs.inputParameters_)
	{}

	LabelValueBlock LegGenerator::getInputParameters() const
	{
		return inputParameters_;
	}

    const DataSchema LegGenerator::generateDataSchema(const std::string& schemaName) const
    {
        std::string schName = (schemaName.size()!=0) ? schemaName : toString(LEG_GENERATOR);
	    const DataSchema  dynamicSchema(    schName,2,
                                            boost::assign::list_of(STRING_VALUE)(VARIANT_VALUE),
		                                    boost::assign::list_of("VARIABLE_NAME")("VARIABLE_VALUE"));
        return dynamicSchema;
    }

    std::map<std::string, Variant> LegGenerator::getDataMap() const
    {
        std::map<std::string, Variant> dataMap;
        dataMap[IRS_KEY::LEG_TYPE]              = inputParameters_.getOptionalValueAsAQLString(IRS_KEY::LEG_TYPE).getCString();
        dataMap[IRS_KEY::LEVERAGE]              = inputParameters_.getOptionalValueAsAQLString(IRS_KEY::LEVERAGE).getCString();
        dataMap[MARKET_KEY::FORECAST_CURVE]     = inputParameters_.getOptionalValueAsAQLString(MARKET_KEY::FORECAST_CURVE).getCString();
        dataMap[MARKET_KEY::DISCOUNT_CURVE]     = inputParameters_.getOptionalValueAsAQLString(MARKET_KEY::DISCOUNT_CURVE).getCString();

        dataMap[IRS_KEY::COMPOUND_METHOD]       = inputParameters_.getOptionalValueAsAQLString(IRS_KEY::COMPOUND_METHOD).getCString();
        dataMap[IRS_KEY::NOTIONAL_EXCHANGE]     = inputParameters_.getOptionalValueAsAQLString(IRS_KEY::NOTIONAL_EXCHANGE).getCString();

        dataMap[IRS_KEY::CURRENCY]              = inputParameters_.getOptionalValueAsAQLString(IRS_KEY::CURRENCY).getCString();
        dataMap[IRS_KEY::VALUATION_CURRENCY]    = inputParameters_.getOptionalValueAsAQLStringFromKeys(IRS_KEY::VALUATION_CURRENCY, IRS_KEY::CURRENCY).getCString();

        dataMap[IRS_KEY::ACCRUALBUSINESSDAYADJUSTMENT]  = inputParameters_.getOptionalValueAsAQLStringFromKeys(IRS_KEY::BUSINESSDAYADJUSTMENT, IRS_KEY::ACCRUALBUSINESSDAYADJUSTMENT).getCString();
        dataMap[IRS_KEY::ACCRUALCALENDAR]              = inputParameters_.getOptionalValueAsAQLStringFromKeys(IRS_KEY::CALENDAR, IRS_KEY::ACCRUALCALENDAR).getCString();
        dataMap[IRS_KEY::ACCRUALFREQUENCY] = inputParameters_.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FREQUENCY, IRS_KEY::ACCRUALFREQUENCY).getCString();
        dataMap[IRS_KEY::ACCRUALDAYCOUNT] = inputParameters_.getOptionalValueAsAQLStringFromKeys(IRS_KEY::DAYCOUNT, IRS_KEY::ACCRUALDAYCOUNT).getCString();
           
        dataMap[IRS_KEY::PAYMENTBUSINESSDAYADJUSTMENT]= inputParameters_.getOptionalValueAsAQLStringFromKeys(IRS_KEY::PAYMENTBUSINESSDAYADJUSTMENT, dataMap[IRS_KEY::ACCRUALBUSINESSDAYADJUSTMENT] ).getCString();
        dataMap[IRS_KEY::PAYMENTCALENDAR] = inputParameters_.getOptionalValueAsAQLStringFromKeys(IRS_KEY::PAYMENTCALENDAR, dataMap[IRS_KEY::ACCRUALCALENDAR] ).getCString();
        dataMap[IRS_KEY::PAYMENTFREQUENCY] = inputParameters_.getOptionalValueAsAQLStringFromKeys(IRS_KEY::PAYMENTFREQUENCY, dataMap[IRS_KEY::ACCRUALFREQUENCY]).getCString();
        dataMap[IRS_KEY::PAYMENTLAG]= inputParameters_.getOptionalValueAsAQLString(IRS_KEY::PAYMENTLAG).getCString();

        dataMap[IRS_KEY::FIRSTSTUBDATE] = inputParameters_.getOptionalValueAsAQLString(IRS_KEY::FIRSTSTUBDATE).getCString();
        dataMap[IRS_KEY::LASTSTUBDATE] = inputParameters_.getOptionalValueAsAQLString(IRS_KEY::LASTSTUBDATE).getCString();
        dataMap[IRS_KEY::ROLLDAY] = inputParameters_.getOptionalValueAsAQLString(IRS_KEY::ROLLDAY).getCString();
        dataMap[IRS_KEY::STUBTYPE]= inputParameters_.getOptionalValueAsAQLString(IRS_KEY::STUBTYPE).getCString();

        dataMap[IRS_KEY::FIXINGADVANCEORARREAR] = inputParameters_.getOptionalValueAsAQLString(IRS_KEY::FIXINGADVANCEORARREAR).getCString();
        dataMap[IRS_KEY::FIXINGBUSINESSDAYADJUSTMENT] = inputParameters_.getOptionalValueAsAQLString(IRS_KEY::FIXINGBUSINESSDAYADJUSTMENT).getCString();
        dataMap[IRS_KEY::FIXINGCALENDAR] = inputParameters_.getOptionalValueAsAQLString(IRS_KEY::FIXINGCALENDAR).getCString();
        dataMap[IRS_KEY::FIXINGLAG] = inputParameters_.getOptionalValueAsAQLString(IRS_KEY::FIXINGLAG).getCString();

        dataMap[IRS_KEY::FIRSTSTUBCURVEINDEX] = inputParameters_.getOptionalValueAsAQLString(IRS_KEY::FIRSTSTUBCURVEINDEX).getCString();
        dataMap[IRS_KEY::LASTSTUBCURVEINDEX] = inputParameters_.getOptionalValueAsAQLString(IRS_KEY::LASTSTUBCURVEINDEX).getCString();

        dataMap[IRS_KEY::IS_FWD_INTER] = inputParameters_.getOptionalValueAsAQLString(IRS_KEY::IS_FWD_INTER).getCString();

		dataMap[IRS_KEY::PAYMENT_TRIGGER] = inputParameters_.getOptionalValueAsAQLString(IRS_KEY::PAYMENT_TRIGGER).getCString();
		dataMap[IRS_KEY::INFLATION_RESET_TYPE] = inputParameters_.getOptionalValueAsAQLString(IRS_KEY::INFLATION_RESET_TYPE).getCString();

		dataMap[CMS_KEY::CMS_GENERATOR_NAME1] = inputParameters_.getOptionalValueAsAQLString(CMS_KEY::CMS_GENERATOR_NAME1).getCString();
		dataMap[CMS_KEY::CMS_INDEX_MATURITY1] = inputParameters_.getOptionalValueAsAQLString(CMS_KEY::CMS_INDEX_MATURITY1).getCString();

		dataMap[CMS_KEY::CMS_GENERATOR_NAME2] = inputParameters_.getOptionalValueAsAQLString(CMS_KEY::CMS_GENERATOR_NAME2).getCString();
		dataMap[CMS_KEY::CMS_INDEX_MATURITY2] = inputParameters_.getOptionalValueAsAQLString(CMS_KEY::CMS_INDEX_MATURITY2).getCString();

		dataMap[BOND_KEY::CALCULATION_TYPE] = inputParameters_.getOptionalValueAsAQLString(BOND_KEY::CALCULATION_TYPE).getCString();
		dataMap[BOND_KEY::EX_DIVIDEND_TENOR] = inputParameters_.getOptionalValueAsAQLString(BOND_KEY::EX_DIVIDEND_TENOR).getCString();
		dataMap[BOND_KEY::EX_DIVIDEND_BUSINESSDAYADJUSTMENT] = inputParameters_.getOptionalValueAsAQLString(BOND_KEY::EX_DIVIDEND_BUSINESSDAYADJUSTMENT).getCString();


		return dataMap;
    }


    const SchemaObject LegGenerator::toSchemaObject() const
    {
        SchemaObject schemaObject(LEG_GENERATOR, ""); // this does not create any data schema
        toSchemaObject(schemaObject, toString(LEG_GENERATOR));
        return schemaObject;
    }

    void LegGenerator::toSchemaObject(SchemaObject& schemaObject, const std::string& schemaName) const
    {
        schemaObject.addDataSchema(generateDataSchema(schemaName));
        schemaObject.setDataForSchemaWithMap(schemaName, getDataMap());
    }



}

