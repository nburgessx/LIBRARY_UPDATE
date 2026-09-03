#pragma once

#include "AQLRiskConfigurationYieldIRDelta.h"


class AQLRiskConfigurationYieldIRDeltaMultiParallel : public AQLRiskConfigurationYieldIRDelta
{
protected:
    virtual std::vector<AQLObject *> createMarketBumpYieldEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;

	virtual int getMaxGridIndex (const AQLString &ccy) const { return 0;}
};

