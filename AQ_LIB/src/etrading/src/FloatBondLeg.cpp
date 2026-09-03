#include "FloatBondLeg.h"

namespace etrading
{
	//This is the leg that proxy the fixed bond in asset swap
	FloatBondLeg::FloatBondLeg(const LabelValueBlock& legLVB, const std::string& instanceName, const SchedulePtr& schedule) : FloatLeg(legLVB, instanceName, schedule)
	{

		legStaticData_ = LegStaticDataPtr(new FloatStaticData(legLVB));

		//Schedule
		if (schedule == nullptr)
		{
			schedule_ = SchedulePtr(new FloatBondLegSchedule(legLVB, instanceName));
		}
	}

	FloatBondLeg::FloatBondLeg(const std::string& instanceName, const LegStaticDataPtr& legStaticData, const SchedulePtr& schedule) : FloatLeg(instanceName, legStaticData, schedule)
	{}

	FloatBondLeg::FloatBondLeg(const FloatBondLeg& rhs) : FloatLeg(rhs)
	{}

	LegPtr FloatBondLeg::clone()
	{
		LegPtr leg = LegPtr(new FloatBondLeg(*this));
		return leg;
	}


	ScheduleTypeEnum FloatBondLeg::getType() const
	{
		return SWAPSCHEDULE_FLOATBOND;
	}

}

