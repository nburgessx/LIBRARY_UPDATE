#pragma once

#include "LARiskConfigurationYieldIRDelta.h"


class LARiskConfigurationYieldIRDeltaMultiParallel : public LARiskConfigurationYieldIRDelta
{
protected:
    virtual std::vector<AQLObject *> createMarketBumpYieldEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;

	virtual int getMaxGridIndex (const AQLString &ccy) const { return 0;}
};

