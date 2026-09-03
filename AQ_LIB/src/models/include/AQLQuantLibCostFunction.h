#pragma once

#include <ql/math/optimization/costfunction.hpp>


class AQLQuantLibCostFunction
{
public:
	AQLQuantLibCostFunction();

	virtual ~AQLQuantLibCostFunction();

	std::shared_ptr<QuantLib::CostFunction> getCostFunction(void) const { return mCostFunction; }

protected:
	std::shared_ptr<QuantLib::CostFunction> mCostFunction;
};
