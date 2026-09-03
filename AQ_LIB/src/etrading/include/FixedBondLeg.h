/*
 * @brief			Class the defines the fixed leg
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */
#pragma once

#include "FixedLeg.h"
#include "FixedBondLegSchedule.h"

namespace etrading
{
	//This is the leg that proxy the fixed bond in asset swap
    class FixedBondLeg : public FixedLeg
    {
    public:

		FixedBondLeg(const LabelValueBlock& legLVB, const std::string& instanceName, const SchedulePtr& schedule={} );
        FixedBondLeg(const std::string& instanceName, const LegStaticDataPtr& legStaticData, const SchedulePtr& schedule);
		FixedBondLeg(const FixedBondLeg& rhs);
		virtual ~FixedBondLeg() {}
        
        LegPtr clone();

        ScheduleTypeEnum getType() const;

	};

}

