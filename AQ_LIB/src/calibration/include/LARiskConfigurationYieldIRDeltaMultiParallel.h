#pragma once

#include "LARiskConfigurationYieldIRDelta.h"


class LARiskConfigurationYieldIRDeltaMultiParallel : public LARiskConfigurationYieldIRDelta
{
protected:
    virtual std::vector<LAObject *> createMarketBumpYieldEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;

	virtual int getMaxGridIndex (const LAString &ccy) const { return 0;}
};

