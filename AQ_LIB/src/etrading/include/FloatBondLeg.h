#pragma once

#include "FloatLeg.h"
#include "FloatBondLegSchedule.h"

namespace etrading
{
	//This is the leg that proxy the fixed bond in asset swap
    class FloatBondLeg : public FloatLeg
    {
    public:

		FloatBondLeg(const LabelValueBlock& legLVB, const std::string& instanceName, const SchedulePtr& schedule={} );
		FloatBondLeg(const std::string& instanceName, const LegStaticDataPtr& legStaticData, const SchedulePtr& schedule);
		FloatBondLeg(const FloatBondLeg& rhs);
		virtual ~FloatBondLeg() {}
        
        LegPtr clone();

        ScheduleTypeEnum getType() const;

	};

}

