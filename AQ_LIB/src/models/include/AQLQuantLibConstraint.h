#pragma once

#include <ql/math/optimization/constraint.hpp>
#include "AQLQuantLibConstraint.h"


class AQLQuantLibConstraint
{
public:
	AQLQuantLibConstraint();

	virtual ~AQLQuantLibConstraint();

	std::shared_ptr<QuantLib::Constraint> getConstraint(void) const { return mConstraint; }

protected:
	std::shared_ptr<QuantLib::Constraint> mConstraint;
};

