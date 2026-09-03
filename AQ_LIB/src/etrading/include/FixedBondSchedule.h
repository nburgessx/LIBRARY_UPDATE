#pragma once

#include "AQLCoreTemplateType.h"
#include "BondSchedule.h"

namespace etrading
{
	
	class FixedBondSchedule : public BondSchedule
    {
	public:

		FixedBondSchedule(const std::string& instanceName);
		FixedBondSchedule(const LabelValueBlock& scheduleLVB, const std::string& instanceName);
		FixedBondSchedule(const FixedBondSchedule& rhs);
        virtual ~FixedBondSchedule() {};
		
		SchedulePtr clone();

		//Override
		double getFixedRate() const;

	protected:

		//Input parameters
		double fixedRate_;
		//Override
		void createUpfrontCashflow(const AQLDate& paymentDate, double leverage);
		//Override
		void createCashflows();

		//Override
		void initialize(const LabelValueBlock& scheduleLVB);
        
	};

}

