#pragma once

#include "AQLRiskConfigurationYieldIRDelta.h"

class AQLRiskConfigurationYieldIRDeltaSimple : public AQLRiskConfigurationYieldIRDelta
{
protected:
    virtual std::vector<AQLObject *> createMarketBumpYieldEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
};

