#pragma once

#include "LARiskConfigurationYieldIRDelta.h"

class LARiskConfigurationYieldIRDeltaSimple : public LARiskConfigurationYieldIRDelta
{
protected:
    virtual std::vector<LAObject *> createMarketBumpYieldEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
};

