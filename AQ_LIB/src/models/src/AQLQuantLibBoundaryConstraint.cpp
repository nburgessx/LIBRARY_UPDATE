/*!
	@brief a class of boundary constraint for optimization using external library

 */

#include "AQLQuantLibBoundaryConstraint.h"

AQLQuantLibBoundaryConstraint::AQLQuantLibBoundaryConstraint(double low, double high)
: AQLQuantLibConstraint()
{
	mConstraint = std::shared_ptr<QuantLib::Constraint>( new QuantLib::BoundaryConstraint(low, high) );
}

AQLQuantLibBoundaryConstraint::~AQLQuantLibBoundaryConstraint()
{
}
