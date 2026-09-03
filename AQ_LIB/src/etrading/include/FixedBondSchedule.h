/*
 * @brief			Class the defines the schedule 
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "LACoreTemplateType.h"
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
		void createUpfrontCashflow(const LADate& paymentDate, double leverage);
		//Override
		void createCashflows();

		//Override
		void initialize(const LabelValueBlock& scheduleLVB);
        
	};

}

