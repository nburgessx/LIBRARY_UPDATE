#pragma once

#include "AQLQuantLibConstraint.h"

class AQLQuantLibBoundaryConstraint : public AQLQuantLibConstraint
{
public:
	AQLQuantLibBoundaryConstraint(double low, double high);

	~AQLQuantLibBoundaryConstraint();

};
