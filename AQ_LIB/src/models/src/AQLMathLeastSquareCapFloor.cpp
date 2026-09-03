// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/

// stl
#include <utility>

//#include <ql/math/optimization/costfunction.hpp>
#include "AQLMathLeastSquareCapFloor.h"
#include "AQLMathCapFloor.h"

//
//--------------------------------------------------------------------------------
//

AQLMathLeastSquareCapFloor::AQLMathLeastSquareCapFloor(const vector< AQLMathCapFloor* >& CapFloor_Mkt_, bool vega_weighted, mode mode__, target target_)
{
}

AQLMathLeastSquareCapFloor::AQLMathLeastSquareCapFloor(bool isNew_)
{
}

AQLMathLeastSquareCapFloor::AQLMathLeastSquareCapFloor( const AQLMathLeastSquareCapFloor& rhs )
{
}

AQLMathLeastSquareCapFloor::~AQLMathLeastSquareCapFloor()
{
}


//
//--------------------------------------------------------------------------------
//

AQLMathLeastSquareCapFloorVolDiscModel::AQLMathLeastSquareCapFloorVolDiscModel(const vector< AQLMathCapFloor* >& CapFloor_Mkt_,
																		 vector< AQLMathCapFloorVolLMMDiscModel* >& CapFloor_LMM_,
																		 bool vega_weighted,
																		 mode mode__,
																		 target target_
		)
:
AQLMathLeastSquareCapFloor(false)
{
	mCostFunction = std::make_shared<AQLMathLeastSquareCapFloorVolDiscModelQlib>(CapFloor_Mkt_, CapFloor_LMM_, vega_weighted, (AQLMathLeastSquareCapFloorVolDiscModelQlib::mode)mode__, (AQLMathLeastSquareCapFloorVolDiscModelQlib::target)target_);
}

AQLMathLeastSquareCapFloorVolDiscModel::AQLMathLeastSquareCapFloorVolDiscModel(bool isNew_)
:
AQLMathLeastSquareCapFloor(false)
{
}

AQLMathLeastSquareCapFloorVolDiscModel::AQLMathLeastSquareCapFloorVolDiscModel( const AQLMathLeastSquareCapFloorVolDiscModel& rhs )
:
AQLMathLeastSquareCapFloor(false)
{
    mCostFunction = (rhs.mCostFunction != nullptr ? std::shared_ptr<QuantLib::CostFunction>(dynamic_pointer_cast<AQLMathLeastSquareCapFloorVolDiscModelQlib>(rhs.mCostFunction)->clone()) : std::shared_ptr<QuantLib::CostFunction>() );
}

AQLMathLeastSquareCapFloorVolDiscModel::~AQLMathLeastSquareCapFloorVolDiscModel()
{
}


AQLMathLeastSquareCapFloorVolDiscModel& AQLMathLeastSquareCapFloorVolDiscModel::operator =(const AQLMathLeastSquareCapFloorVolDiscModel& rhs)
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

AQLMathLeastSquareCapFloorVolDiscModelV::AQLMathLeastSquareCapFloorVolDiscModelV(const vector< AQLMathCapFloor* >& CapFloor_Mkt_,
																		   vector< AQLMathCapFloorVolLMMDiscModel* >& CapFloor_LMM_,
																		   bool vega_weighted,
																		   mode mode__,
																		   target target_
		)
:
AQLMathLeastSquareCapFloorVolDiscModel(false)
{
	mCostFunction = std::make_shared<AQLMathLeastSquareCapFloorVolDiscModelVQlib>(CapFloor_Mkt_, CapFloor_LMM_, vega_weighted, (AQLMathLeastSquareCapFloorVolDiscModelQlib::mode)mode__, (AQLMathLeastSquareCapFloorVolDiscModelQlib::target)target_);
}

AQLMathLeastSquareCapFloorVolDiscModelV::AQLMathLeastSquareCapFloorVolDiscModelV( const AQLMathLeastSquareCapFloorVolDiscModelV& rhs )
:
AQLMathLeastSquareCapFloorVolDiscModel(false)
{
	mCostFunction = (rhs.mCostFunction != nullptr ? std::shared_ptr<QuantLib::CostFunction>(dynamic_pointer_cast<AQLMathLeastSquareCapFloorVolDiscModelVQlib>(rhs.mCostFunction)->clone()) : std::shared_ptr<QuantLib::CostFunction>() );
}

AQLMathLeastSquareCapFloorVolDiscModelV::~AQLMathLeastSquareCapFloorVolDiscModelV()
{
}


AQLMathLeastSquareCapFloorVolDiscModelV& AQLMathLeastSquareCapFloorVolDiscModelV::operator =(const AQLMathLeastSquareCapFloorVolDiscModelV& rhs)
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

AQLMathLeastSquareCapFloorVolDiscModelF::AQLMathLeastSquareCapFloorVolDiscModelF(const vector< AQLMathCapFloor* >& CapFloor_Mkt_,
																		   vector< AQLMathCapFloorVolLMMDiscModel* >& CapFloor_LMM_,
																		   double T_max_,
																		   bool vega_weighted,
																		   mode mode__,
																		   target target_
		)
:
AQLMathLeastSquareCapFloorVolDiscModel(false)
{
	mCostFunction = std::make_shared<AQLMathLeastSquareCapFloorVolDiscModelFQlib>(CapFloor_Mkt_, CapFloor_LMM_, T_max_, vega_weighted, (AQLMathLeastSquareCapFloorVolDiscModelQlib::mode)mode__, (AQLMathLeastSquareCapFloorVolDiscModelQlib::target)target_);
}

AQLMathLeastSquareCapFloorVolDiscModelF::AQLMathLeastSquareCapFloorVolDiscModelF( const AQLMathLeastSquareCapFloorVolDiscModelF& rhs )
:
AQLMathLeastSquareCapFloorVolDiscModel(false)
{
	mCostFunction = (rhs.mCostFunction != nullptr ? std::shared_ptr<QuantLib::CostFunction>(dynamic_pointer_cast<AQLMathLeastSquareCapFloorVolDiscModelFQlib>(rhs.mCostFunction)->clone()) : std::shared_ptr<QuantLib::CostFunction>() );
}

AQLMathLeastSquareCapFloorVolDiscModelF::~AQLMathLeastSquareCapFloorVolDiscModelF()
{
}


AQLMathLeastSquareCapFloorVolDiscModelF& AQLMathLeastSquareCapFloorVolDiscModelF::operator =(const AQLMathLeastSquareCapFloorVolDiscModelF& rhs)
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

AQLMathLeastSquareCapFloorVolDiscModelG::AQLMathLeastSquareCapFloorVolDiscModelG(const vector< AQLMathCapFloor* >& CapFloor_Mkt_,
																		   vector< AQLMathCapFloorVolLMMDiscModel* >& CapFloor_LMM_,
																		   bool vega_weighted,
																		   mode mode__,
																		   target target_
		)
:
AQLMathLeastSquareCapFloorVolDiscModel(false)
{
	mCostFunction = std::make_shared<AQLMathLeastSquareCapFloorVolDiscModelGQlib>(CapFloor_Mkt_, CapFloor_LMM_, vega_weighted, (AQLMathLeastSquareCapFloorVolDiscModelQlib::mode)mode__, (AQLMathLeastSquareCapFloorVolDiscModelQlib::target)target_);
}

AQLMathLeastSquareCapFloorVolDiscModelG::AQLMathLeastSquareCapFloorVolDiscModelG( const AQLMathLeastSquareCapFloorVolDiscModelG& rhs )
:
AQLMathLeastSquareCapFloorVolDiscModel(false)
{
	mCostFunction = (rhs.mCostFunction != nullptr ? std::shared_ptr<QuantLib::CostFunction>(dynamic_pointer_cast<AQLMathLeastSquareCapFloorVolDiscModelGQlib>(rhs.mCostFunction)->clone()) : std::shared_ptr<QuantLib::CostFunction>() );
}

AQLMathLeastSquareCapFloorVolDiscModelG::~AQLMathLeastSquareCapFloorVolDiscModelG()
{
}


AQLMathLeastSquareCapFloorVolDiscModelG& AQLMathLeastSquareCapFloorVolDiscModelG::operator =(const AQLMathLeastSquareCapFloorVolDiscModelG& rhs)
{
    if ( this != &rhs )
    {
		*mCostFunction = *rhs.mCostFunction;
    }
    return *this;
}
