/*!
	@file LAQuantLibBoundaryConstraint.cpp
	@brief a class of boundary constraint for optimization using external library

 */

#include "LAQuantLibBoundaryConstraint.h"

LAQuantLibBoundaryConstraint::LAQuantLibBoundaryConstraint(double low, double high)
: LAQuantLibConstraint()
{
	mConstraint = std::shared_ptr<QuantLib::Constraint>( new QuantLib::BoundaryConstraint(low, high) );
}

LAQuantLibBoundaryConstraint::~LAQuantLibBoundaryConstraint()
{
}
