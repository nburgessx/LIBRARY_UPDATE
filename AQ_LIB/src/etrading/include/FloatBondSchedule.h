#pragma once

#include "LACoreTemplateType.h"
#include "BondSchedule.h"

namespace etrading
{
	
	class FloatBondSchedule : public BondSchedule
    {
	public:

		FloatBondSchedule(const std::string& instanceName);
		FloatBondSchedule(const LabelValueBlock& scheduleLVB, const std::string& instanceName);
		FloatBondSchedule(const FloatBondSchedule& rhs);
        virtual ~FloatBondSchedule() {};
		
		SchedulePtr clone();

	protected:

		//Override
		void createUpfrontCashflow(const LADate& paymentDate, double leverage);
		//Override
		void createCashflows();

		//Override
		void initialize(const LabelValueBlock& scheduleLVB);
        

	};

}

