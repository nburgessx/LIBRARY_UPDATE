#pragma once

#include <string>
#include "LabelValueBlock.h"

#include "IsLWOObject.h"
#include "SchemaObject.h"
#include "ScheduleValidation.h"
#include "CommonConstants.h"

namespace etrading
{
	class Volatility : public IsLWOObject
    {
	public:

		Volatility(const LabelValueBlock& volLVB, const std::string& instanceName);
		Volatility(const Volatility& rhs);
		virtual ~Volatility() {}

		// Override the abstract class:
		const SchemaObject toSchemaObject() const;
		const DataSchema generateDataSchema(const std::string& schemaName) const; //helper method for caching
		std::map<std::string, Variant> getDataMap() const; //helper method for caching
														   
		// Get Accessors:
		const LADate& asOfDate() const              { return asOfDate_; };
		const CCY currency() const                  { return currency_; };
        const VolatilityTypeEnum volType() const    { return volType_; };
		const double vol() const                    { return vol_; };
		const std::string volDataSource() const     { return volDataSource_; };
        const double shiftSize() const              { return shiftSize_; };

	private:
		
		LADate asOfDate_;
		CCY currency_;

		std::string volDataSource_;
		VolatilityTypeEnum volType_;
		double vol_;
        double shiftSize_;

		LabelValueBlock inputParameters_;
	};

	typedef std::shared_ptr< Volatility > VolatilityPtr;


}
