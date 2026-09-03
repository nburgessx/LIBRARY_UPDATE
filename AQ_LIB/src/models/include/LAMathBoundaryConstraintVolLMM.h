#ifndef LAMathBoundaryConstraintVolLMM_h
#define LAMathBoundaryConstraintVolLMM_h

#include "LAMathBoundaryConstraintVolLMMQlib.h"

// %Constraint of vol_LMM_model1_V
class LAMathBoundaryConstraintLMMVolV : public LAQuantLibConstraint
{
public:
    LAMathBoundaryConstraintLMMVolV(double low, double high)
    {
		mConstraint = std::shared_ptr<QuantLib::Constraint>(new LAMathBoundaryConstraintLMMVolVQlib(low, high));
    }

	virtual ~LAMathBoundaryConstraintLMMVolV()
	{
	}

};

//
//----------
//
class LAMathBoundaryConstraintLMMVolF : public LAQuantLibConstraint
{
public:
    LAMathBoundaryConstraintLMMVolF(double low, double high)
    {
		mConstraint = std::shared_ptr<QuantLib::Constraint>(new LAMathBoundaryConstraintLMMVolFQlib(low, high));
    }

	virtual ~LAMathBoundaryConstraintLMMVolF()
	{
	}
};

//
//----------
//
class LAMathBoundaryConstraintLMMVolG : public LAQuantLibConstraint
{
public:
    LAMathBoundaryConstraintLMMVolG(double low, double high, size_t G_size_, double smooth_bound_ = numeric_limits<double>::max())
    {
		mConstraint = std::shared_ptr<QuantLib::Constraint>(new LAMathBoundaryConstraintLMMVolGQlib(low, high, G_size_, smooth_bound_));
    }

	virtual ~LAMathBoundaryConstraintLMMVolG()
	{
	}
};
#endif
