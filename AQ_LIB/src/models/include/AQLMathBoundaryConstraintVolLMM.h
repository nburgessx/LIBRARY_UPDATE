#ifndef AQLMathBoundaryConstraintVolLMM_h
#define AQLMathBoundaryConstraintVolLMM_h

#include "AQLMathBoundaryConstraintVolLMMQlib.h"

// %Constraint of vol_LMM_model1_V
class AQLMathBoundaryConstraintLMMVolV : public AQLQuantLibConstraint
{
public:
    AQLMathBoundaryConstraintLMMVolV(double low, double high)
    {
		mConstraint = std::shared_ptr<QuantLib::Constraint>(new AQLMathBoundaryConstraintLMMVolVQlib(low, high));
    }

	virtual ~AQLMathBoundaryConstraintLMMVolV()
	{
	}

};

//
//----------
//
class AQLMathBoundaryConstraintLMMVolF : public AQLQuantLibConstraint
{
public:
    AQLMathBoundaryConstraintLMMVolF(double low, double high)
    {
		mConstraint = std::shared_ptr<QuantLib::Constraint>(new AQLMathBoundaryConstraintLMMVolFQlib(low, high));
    }

	virtual ~AQLMathBoundaryConstraintLMMVolF()
	{
	}
};

//
//----------
//
class AQLMathBoundaryConstraintLMMVolG : public AQLQuantLibConstraint
{
public:
    AQLMathBoundaryConstraintLMMVolG(double low, double high, size_t G_size_, double smooth_bound_ = numeric_limits<double>::max())
    {
		mConstraint = std::shared_ptr<QuantLib::Constraint>(new AQLMathBoundaryConstraintLMMVolGQlib(low, high, G_size_, smooth_bound_));
    }

	virtual ~AQLMathBoundaryConstraintLMMVolG()
	{
	}
};
#endif
