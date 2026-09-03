// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/

// stl
#include <utility>

//#include <ql/math/optimization/costfunction.hpp>
#include "LAMathLeastSquareCapFloor.h"
#include "LAMathCapFloor.h"

//
//--------------------------------------------------------------------------------
//

LAMathLeastSquareCapFloor::LAMathLeastSquareCapFloor(const vector< LAMathCapFloor* >& CapFloor_Mkt_, bool vega_weighted, mode mode__, target target_)
{
}

LAMathLeastSquareCapFloor::LAMathLeastSquareCapFloor(bool isNew_)
{
}

LAMathLeastSquareCapFloor::LAMathLeastSquareCapFloor( const LAMathLeastSquareCapFloor& rhs )
{
}

LAMathLeastSquareCapFloor::~LAMathLeastSquareCapFloor()
{
}


//
//--------------------------------------------------------------------------------
//

LAMathLeastSquareCapFloorVolDiscModel::LAMathLeastSquareCapFloorVolDiscModel(const vector< LAMathCapFloor* >& CapFloor_Mkt_,
																		 vector< LAMathCapFloorVolLMMDiscModel* >& CapFloor_LMM_,
																		 bool vega_weighted,
																		 mode mode__,
																		 target target_
		)
:
LAMathLeastSquareCapFloor(false)
{
	mCostFunction = std::make_shared<LAMathLeastSquareCapFloorVolDiscModelQlib>(CapFloor_Mkt_, CapFloor_LMM_, vega_weighted, (LAMathLeastSquareCapFloorVolDiscModelQlib::mode)mode__, (LAMathLeastSquareCapFloorVolDiscModelQlib::target)target_);
}

LAMathLeastSquareCapFloorVolDiscModel::LAMathLeastSquareCapFloorVolDiscModel(bool isNew_)
:
LAMathLeastSquareCapFloor(false)
{
}

LAMathLeastSquareCapFloorVolDiscModel::LAMathLeastSquareCapFloorVolDiscModel( const LAMathLeastSquareCapFloorVolDiscModel& rhs )
:
LAMathLeastSquareCapFloor(false)
{
    mCostFunction = (rhs.mCostFunction != nullptr ? std::shared_ptr<QuantLib::CostFunction>(dynamic_pointer_cast<LAMathLeastSquareCapFloorVolDiscModelQlib>(rhs.mCostFunction)->clone()) : std::shared_ptr<QuantLib::CostFunction>() );
}

LAMathLeastSquareCapFloorVolDiscModel::~LAMathLeastSquareCapFloorVolDiscModel()
{
}


LAMathLeastSquareCapFloorVolDiscModel& LAMathLeastSquareCapFloorVolDiscModel::operator =(const LAMathLeastSquareCapFloorVolDiscModel& rhs)
{
    if ( this != &rhs )
    {
		*mCostFunction = *rhs.mCostFunction;
    }
    return *this;
}

//
//--------------------------------------------------------------------------------
//

LAMathLeastSquareCapFloorVolDiscModelV::LAMathLeastSquareCapFloorVolDiscModelV(const vector< LAMathCapFloor* >& CapFloor_Mkt_,
																		   vector< LAMathCapFloorVolLMMDiscModel* >& CapFloor_LMM_,
																		   bool vega_weighted,
																		   mode mode__,
																		   target target_
		)
:
LAMathLeastSquareCapFloorVolDiscModel(false)
{
	mCostFunction = std::make_shared<LAMathLeastSquareCapFloorVolDiscModelVQlib>(CapFloor_Mkt_, CapFloor_LMM_, vega_weighted, (LAMathLeastSquareCapFloorVolDiscModelQlib::mode)mode__, (LAMathLeastSquareCapFloorVolDiscModelQlib::target)target_);
}

LAMathLeastSquareCapFloorVolDiscModelV::LAMathLeastSquareCapFloorVolDiscModelV( const LAMathLeastSquareCapFloorVolDiscModelV& rhs )
:
LAMathLeastSquareCapFloorVolDiscModel(false)
{
	mCostFunction = (rhs.mCostFunction != nullptr ? std::shared_ptr<QuantLib::CostFunction>(dynamic_pointer_cast<LAMathLeastSquareCapFloorVolDiscModelVQlib>(rhs.mCostFunction)->clone()) : std::shared_ptr<QuantLib::CostFunction>() );
}

LAMathLeastSquareCapFloorVolDiscModelV::~LAMathLeastSquareCapFloorVolDiscModelV()
{
}


LAMathLeastSquareCapFloorVolDiscModelV& LAMathLeastSquareCapFloorVolDiscModelV::operator =(const LAMathLeastSquareCapFloorVolDiscModelV& rhs)
{
    if ( this != &rhs )
    {
		*mCostFunction = *rhs.mCostFunction;
    }
    return *this;
}

//
//--------------------------------------------------------------------------------
//

LAMathLeastSquareCapFloorVolDiscModelF::LAMathLeastSquareCapFloorVolDiscModelF(const vector< LAMathCapFloor* >& CapFloor_Mkt_,
																		   vector< LAMathCapFloorVolLMMDiscModel* >& CapFloor_LMM_,
																		   double T_max_,
																		   bool vega_weighted,
																		   mode mode__,
																		   target target_
		)
:
LAMathLeastSquareCapFloorVolDiscModel(false)
{
	mCostFunction = std::make_shared<LAMathLeastSquareCapFloorVolDiscModelFQlib>(CapFloor_Mkt_, CapFloor_LMM_, T_max_, vega_weighted, (LAMathLeastSquareCapFloorVolDiscModelQlib::mode)mode__, (LAMathLeastSquareCapFloorVolDiscModelQlib::target)target_);
}

LAMathLeastSquareCapFloorVolDiscModelF::LAMathLeastSquareCapFloorVolDiscModelF( const LAMathLeastSquareCapFloorVolDiscModelF& rhs )
:
LAMathLeastSquareCapFloorVolDiscModel(false)
{
	mCostFunction = (rhs.mCostFunction != nullptr ? std::shared_ptr<QuantLib::CostFunction>(dynamic_pointer_cast<LAMathLeastSquareCapFloorVolDiscModelFQlib>(rhs.mCostFunction)->clone()) : std::shared_ptr<QuantLib::CostFunction>() );
}

LAMathLeastSquareCapFloorVolDiscModelF::~LAMathLeastSquareCapFloorVolDiscModelF()
{
}


LAMathLeastSquareCapFloorVolDiscModelF& LAMathLeastSquareCapFloorVolDiscModelF::operator =(const LAMathLeastSquareCapFloorVolDiscModelF& rhs)
{
    if ( this != &rhs )
    {
		*mCostFunction = *rhs.mCostFunction;
    }
    return *this;
}

//
//--------------------------------------------------------------------------------
//

LAMathLeastSquareCapFloorVolDiscModelG::LAMathLeastSquareCapFloorVolDiscModelG(const vector< LAMathCapFloor* >& CapFloor_Mkt_,
																		   vector< LAMathCapFloorVolLMMDiscModel* >& CapFloor_LMM_,
																		   bool vega_weighted,
																		   mode mode__,
																		   target target_
		)
:
LAMathLeastSquareCapFloorVolDiscModel(false)
{
	mCostFunction = std::make_shared<LAMathLeastSquareCapFloorVolDiscModelGQlib>(CapFloor_Mkt_, CapFloor_LMM_, vega_weighted, (LAMathLeastSquareCapFloorVolDiscModelQlib::mode)mode__, (LAMathLeastSquareCapFloorVolDiscModelQlib::target)target_);
}

LAMathLeastSquareCapFloorVolDiscModelG::LAMathLeastSquareCapFloorVolDiscModelG( const LAMathLeastSquareCapFloorVolDiscModelG& rhs )
:
LAMathLeastSquareCapFloorVolDiscModel(false)
{
	mCostFunction = (rhs.mCostFunction != nullptr ? std::shared_ptr<QuantLib::CostFunction>(dynamic_pointer_cast<LAMathLeastSquareCapFloorVolDiscModelGQlib>(rhs.mCostFunction)->clone()) : std::shared_ptr<QuantLib::CostFunction>() );
}

LAMathLeastSquareCapFloorVolDiscModelG::~LAMathLeastSquareCapFloorVolDiscModelG()
{
}


LAMathLeastSquareCapFloorVolDiscModelG& LAMathLeastSquareCapFloorVolDiscModelG::operator =(const LAMathLeastSquareCapFloorVolDiscModelG& rhs)
{
    if ( this != &rhs )
    {
		*mCostFunction = *rhs.mCostFunction;
    }
    return *this;
}
