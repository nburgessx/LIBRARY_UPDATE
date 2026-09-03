// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/

// stl
#include <utility>

#include "LAMathLeastSquareSwaptionVol.h"
#include "LAMathLeastSquareSwaptionVolQlib.h"
#include "LAMathSwaptionVolLMM.h"

//
//--------------------------------------------------------------------------------
//

LAMathLeastSquareSwaptionVol::LAMathLeastSquareSwaptionVol(const DoubleMatrix& Swaption_target_value_Mkt_,
													   mode mode_,
													   target target_
//                                                    const vector< vector< pair<double, double> > >& OptMat_and_Expire_
		)
{
	//mCostFunction = new LAMathLeastSquareSwaptionVolQlib(Swaption_vol_Mkt_, mode_);
}

LAMathLeastSquareSwaptionVol::LAMathLeastSquareSwaptionVol(const DoubleMatrix& Swaption_target_value_Mkt_,
													   const DoubleMatrix& weight_,
													   mode mode_,
													   target target_
//                                                    const vector< vector< pair<double, double> > >& OptMat_and_Expire_
		)
{
	//mCostFunction = new LAMathLeastSquareSwaptionVolQlib(Swaption_vol_Mkt_, weight_, mode_);
}

LAMathLeastSquareSwaptionVol::LAMathLeastSquareSwaptionVol(bool isNew_)
{
}


LAMathLeastSquareSwaptionVol::LAMathLeastSquareSwaptionVol( const LAMathLeastSquareSwaptionVol& rhs )
{
	//mCostFunction = new LAMathLeastSquareSwaptionVolQlib(*(dynamic_cast<LAMathLeastSquareSwaptionVolQlib*>(rhs.mCostFunction)));
}

LAMathLeastSquareSwaptionVol::~LAMathLeastSquareSwaptionVol()
{
}


//
//--------------------------------------------------------------------------------
//

LAMathLeastSquareSwaptionVolDiscModel::LAMathLeastSquareSwaptionVolDiscModel( const DoubleMatrix& Swaption_target_value_Mkt_,
																		  vector< LAMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
																		  mode mode_,
																		  target target_
		)
:
LAMathLeastSquareSwaptionVol(false)
{
	mCostFunction = std::make_shared<LAMathLeastSquareSwaptionVolDiscModelQlib>(Swaption_target_value_Mkt_, Swaption_Vol_, (LAMathLeastSquareSwaptionVolDiscModelQlib::mode)mode_, (LAMathLeastSquareSwaptionVolDiscModelQlib::target)target_);
}

LAMathLeastSquareSwaptionVolDiscModel::LAMathLeastSquareSwaptionVolDiscModel( const DoubleMatrix& Swaption_target_value_Mkt_,
																		  vector< LAMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
																		  const DoubleMatrix& weight_,
																		  mode mode_,
																		  target target_
		)
:
LAMathLeastSquareSwaptionVol(false)
{
	mCostFunction = std::make_shared<LAMathLeastSquareSwaptionVolDiscModelQlib>(Swaption_target_value_Mkt_, Swaption_Vol_, weight_, (LAMathLeastSquareSwaptionVolDiscModelQlib::mode)mode_, (LAMathLeastSquareSwaptionVolDiscModelQlib::target)target_);
}

LAMathLeastSquareSwaptionVolDiscModel::LAMathLeastSquareSwaptionVolDiscModel( const LAMathLeastSquareSwaptionVolDiscModel& rhs )
:
LAMathLeastSquareSwaptionVol(false)
{
	mCostFunction = (rhs.mCostFunction != nullptr ? std::shared_ptr<QuantLib::CostFunction>(dynamic_pointer_cast<LAMathLeastSquareSwaptionVolDiscModelQlib>(rhs.mCostFunction)->clone()) : std::shared_ptr<QuantLib::CostFunction>());
}

LAMathLeastSquareSwaptionVolDiscModel::LAMathLeastSquareSwaptionVolDiscModel(bool isNew_)
:
LAMathLeastSquareSwaptionVol(false)
{
}


LAMathLeastSquareSwaptionVolDiscModel::~LAMathLeastSquareSwaptionVolDiscModel()
{
}


LAMathLeastSquareSwaptionVolDiscModel& LAMathLeastSquareSwaptionVolDiscModel::operator =(const LAMathLeastSquareSwaptionVolDiscModel& rhs)
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

LAMathLeastSquareSwaptionVolDiscModelV::LAMathLeastSquareSwaptionVolDiscModelV(const DoubleMatrix& Swaption_target_value_Mkt_,
																		   vector< LAMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
																		   mode mode_,
																		   target target_
		)
:
LAMathLeastSquareSwaptionVolDiscModel(false)
{
	mCostFunction = std::make_shared<LAMathLeastSquareSwaptionVolDiscModelVQlib>(Swaption_target_value_Mkt_, Swaption_Vol_, (LAMathLeastSquareSwaptionVolDiscModelQlib::mode)mode_, (LAMathLeastSquareSwaptionVolDiscModelQlib::target)target_);
}

LAMathLeastSquareSwaptionVolDiscModelV::LAMathLeastSquareSwaptionVolDiscModelV(const DoubleMatrix& Swaption_target_value_Mkt_,
                                                                           vector< LAMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
                                                                           const DoubleMatrix& weight_,
                                                                           mode mode_,
																		   target target_
                                                                          )
:
LAMathLeastSquareSwaptionVolDiscModel(false)
{
	mCostFunction = std::make_shared<LAMathLeastSquareSwaptionVolDiscModelVQlib>(Swaption_target_value_Mkt_, Swaption_Vol_, weight_, (LAMathLeastSquareSwaptionVolDiscModelQlib::mode)mode_, (LAMathLeastSquareSwaptionVolDiscModelQlib::target)target_);
}

LAMathLeastSquareSwaptionVolDiscModelV::LAMathLeastSquareSwaptionVolDiscModelV( const LAMathLeastSquareSwaptionVolDiscModelV& rhs )
:
LAMathLeastSquareSwaptionVolDiscModel(false)
{
	mCostFunction = (rhs.mCostFunction != nullptr ? std::shared_ptr<QuantLib::CostFunction>(dynamic_pointer_cast<LAMathLeastSquareSwaptionVolDiscModelVQlib>(rhs.mCostFunction)->clone()) : std::shared_ptr<QuantLib::CostFunction>() );
}

LAMathLeastSquareSwaptionVolDiscModelV::~LAMathLeastSquareSwaptionVolDiscModelV()
{
}


LAMathLeastSquareSwaptionVolDiscModelV& LAMathLeastSquareSwaptionVolDiscModelV::operator =(const LAMathLeastSquareSwaptionVolDiscModelV& rhs)
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

LAMathLeastSquareSwaptionVolDiscModelF::LAMathLeastSquareSwaptionVolDiscModelF(const DoubleMatrix& Swaption_target_value_Mkt_,
																		   vector< LAMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
																		   double T_max_,
																		   mode mode_,
																		   target target_
		)
:
LAMathLeastSquareSwaptionVolDiscModel(false)
{
	mCostFunction = std::make_shared<LAMathLeastSquareSwaptionVolDiscModelFQlib>(Swaption_target_value_Mkt_, Swaption_Vol_, T_max_, (LAMathLeastSquareSwaptionVolDiscModelQlib::mode)mode_, (LAMathLeastSquareSwaptionVolDiscModelQlib::target)target_);
}

LAMathLeastSquareSwaptionVolDiscModelF::LAMathLeastSquareSwaptionVolDiscModelF(const DoubleMatrix& Swaption_target_value_Mkt_,
                                                                           vector< LAMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
                                                                           double T_max_,
                                                                           const DoubleMatrix& weight_,
                                                                           mode mode_,
																		   target target_
                                                                          )
:
LAMathLeastSquareSwaptionVolDiscModel(false)
{
	mCostFunction = std::make_shared<LAMathLeastSquareSwaptionVolDiscModelFQlib>(Swaption_target_value_Mkt_, Swaption_Vol_, T_max_, weight_, (LAMathLeastSquareSwaptionVolDiscModelQlib::mode)mode_, (LAMathLeastSquareSwaptionVolDiscModelQlib::target)target_);
}

LAMathLeastSquareSwaptionVolDiscModelF::LAMathLeastSquareSwaptionVolDiscModelF( const LAMathLeastSquareSwaptionVolDiscModelF& rhs )
:
LAMathLeastSquareSwaptionVolDiscModel(false)
{
	mCostFunction = (rhs.mCostFunction != nullptr ? std::shared_ptr<QuantLib::CostFunction>(dynamic_pointer_cast<LAMathLeastSquareSwaptionVolDiscModelFQlib>(rhs.mCostFunction)->clone()) : std::shared_ptr<QuantLib::CostFunction>() );
}

LAMathLeastSquareSwaptionVolDiscModelF::~LAMathLeastSquareSwaptionVolDiscModelF()
{
}


LAMathLeastSquareSwaptionVolDiscModelF& LAMathLeastSquareSwaptionVolDiscModelF::operator =(const LAMathLeastSquareSwaptionVolDiscModelF& rhs)
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

LAMathLeastSquareSwaptionVolDiscModelG::LAMathLeastSquareSwaptionVolDiscModelG(const DoubleMatrix& Swaption_target_value_Mkt_,
																		   vector< LAMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
																		   mode mode_,
																		   target target_
		)
:
LAMathLeastSquareSwaptionVolDiscModel(false)
{
	mCostFunction = std::make_shared<LAMathLeastSquareSwaptionVolDiscModelGQlib>(Swaption_target_value_Mkt_, Swaption_Vol_, (LAMathLeastSquareSwaptionVolDiscModelQlib::mode)mode_, (LAMathLeastSquareSwaptionVolDiscModelQlib::target)target_);
}

LAMathLeastSquareSwaptionVolDiscModelG::LAMathLeastSquareSwaptionVolDiscModelG(const DoubleMatrix& Swaption_target_value_Mkt_,
                                                                           vector< LAMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
                                                                           const DoubleMatrix& weight_,
                                                                           mode mode_,
																		   target target_
                                                                          )
:
LAMathLeastSquareSwaptionVolDiscModel(false)
{
	mCostFunction = std::make_shared<LAMathLeastSquareSwaptionVolDiscModelGQlib>(Swaption_target_value_Mkt_, Swaption_Vol_, weight_, (LAMathLeastSquareSwaptionVolDiscModelQlib::mode)mode_, (LAMathLeastSquareSwaptionVolDiscModelQlib::target)target_);
}

LAMathLeastSquareSwaptionVolDiscModelG::LAMathLeastSquareSwaptionVolDiscModelG( const LAMathLeastSquareSwaptionVolDiscModelG& rhs )
:
LAMathLeastSquareSwaptionVolDiscModel(false)
{
	mCostFunction = (rhs.mCostFunction != nullptr ? std::shared_ptr<QuantLib::CostFunction>(dynamic_pointer_cast<LAMathLeastSquareSwaptionVolDiscModelGQlib>(rhs.mCostFunction)->clone()) : std::shared_ptr<QuantLib::CostFunction>() );
}

LAMathLeastSquareSwaptionVolDiscModelG::~LAMathLeastSquareSwaptionVolDiscModelG()
{
}


LAMathLeastSquareSwaptionVolDiscModelG& LAMathLeastSquareSwaptionVolDiscModelG::operator =(const LAMathLeastSquareSwaptionVolDiscModelG& rhs)
{
    if ( this != &rhs )
    {
        *mCostFunction = *rhs.mCostFunction;
    }
    return *this;
}
