/*
* @brief			Base Class the defines the Option
* @Created:			05 Feb 2018
* @Author:			Yongyan Zheng
* @Department:		ISD Front Office Development
*
* The copyright to the computer program(s) herein is the property of AlgoQuantHub.
*/
#pragma once

#include <string>

#include "IsLWOObject.h"
#include "SchemaObject.h"
#include "ScheduleValidation.h"
#include "CommonConstants.h"

namespace etrading
{
	// This is the Abstract class of an Optional deal/structure/trade, to store the fields
	class Option : public IsLWOObject 
    {
	public:

		Option(const std::string& objectName);
        Option(const Option& rhs);
		virtual ~Option() {};

        virtual std::shared_ptr<Option> clone()=0;

		//Override the abstract class:
        const SchemaObject toSchemaObject() const;
		const DataSchema generateDataSchema(const std::string& schemaName) const; //helper method for caching
		std::map<std::string, Variant> getDataMap() const; //helper method for caching

		//Getter:
		const LabelValueBlock inputParameters() const { return inputParameters_; };

		const std::shared_ptr<ScheduleParameters>  scheduleParameters() const { return scheduleParams_; };
		const std::shared_ptr<EnrichedSchedule> enrichedSchedule() const { return enrichedSchedule_; };

		const CallOrPutEnum callPut() const { return callPut_; };

	protected:
		
		LabelValueBlock inputParameters_;

		std::shared_ptr<ScheduleParameters> scheduleParams_;
		std::shared_ptr<EnrichedSchedule> enrichedSchedule_;

		CallOrPutEnum callPut_;

	};

	typedef std::shared_ptr< Option > OptionPtr;


}
