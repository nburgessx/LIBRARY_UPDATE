#pragma once

#include <ql/math/optimization/constraint.hpp>
#include "LAQuantLibConstraint.h"


class LAQuantLibConstraint
{
public:
	LAQuantLibConstraint();

	virtual ~LAQuantLibConstraint();

	std::shared_ptr<QuantLib::Constraint> getConstraint(void) const { return mConstraint; }

protected:
	std::shared_ptr<QuantLib::Constraint> mConstraint;
};

