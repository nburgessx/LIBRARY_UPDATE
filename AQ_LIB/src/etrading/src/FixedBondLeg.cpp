#include "FixedBondLeg.h"
#include "FixedBondCashflow.h"
#include "BondUtilities.h"

namespace etrading
{
	//This is the leg that proxy the fixed bond in asset swap

	FixedBondLeg::FixedBondLeg(const LabelValueBlock& legLVB, const std::string& instanceName, const SchedulePtr& schedule) : FixedLeg(legLVB, instanceName, schedule)
	{
        legStaticData_ = LegStaticDataPtr (new FixedStaticData(legLVB));

		//Schedule
		if (schedule==nullptr)
		{
			schedule_ = SchedulePtr(new FixedBondLegSchedule(legLVB, instanceName));
		}
	}

    FixedBondLeg::FixedBondLeg(const std::string& instanceName, const LegStaticDataPtr& legStaticData, const SchedulePtr& schedule) : FixedLeg(instanceName, legStaticData, schedule)
    {}

    LegPtr FixedBondLeg::clone()
    {
        LegPtr leg = LegPtr(new FixedBondLeg(*this));
        return leg;
    }

	FixedBondLeg::FixedBondLeg( const FixedBondLeg& rhs) : FixedLeg(rhs)
	{}

    ScheduleTypeEnum FixedBondLeg::getType() const
    {
        return SWAPSCHEDULE_FIXEDBOND;
    }

}

