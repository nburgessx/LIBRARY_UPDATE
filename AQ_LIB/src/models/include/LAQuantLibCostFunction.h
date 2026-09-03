#pragma once

#include <ql/math/optimization/costfunction.hpp>


class LAQuantLibCostFunction
{
public:
	LAQuantLibCostFunction();

	virtual ~LAQuantLibCostFunction();

	std::shared_ptr<QuantLib::CostFunction> getCostFunction(void) const { return mCostFunction; }

protected:
	std::shared_ptr<QuantLib::CostFunction> mCostFunction;
};
