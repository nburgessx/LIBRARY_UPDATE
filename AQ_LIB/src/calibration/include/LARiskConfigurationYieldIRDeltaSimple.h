#pragma once

#include "LARiskConfigurationYieldIRDelta.h"

class LARiskConfigurationYieldIRDeltaSimple : public LARiskConfigurationYieldIRDelta
{
protected:
    virtual std::vector<AQLObject *> createMarketBumpYieldEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
};

