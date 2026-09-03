#pragma once

#include "LAQuantLibConstraint.h"

class LAQuantLibBoundaryConstraint : public LAQuantLibConstraint
{
public:
	LAQuantLibBoundaryConstraint(double low, double high);

	~LAQuantLibBoundaryConstraint();

};
