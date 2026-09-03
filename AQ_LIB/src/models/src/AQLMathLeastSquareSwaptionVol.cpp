// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/

// stl
#include <utility>

#include "AQLMathLeastSquareSwaptionVol.h"
#include "AQLMathLeastSquareSwaptionVolQlib.h"
#include "AQLMathSwaptionVolLMM.h"

//
//--------------------------------------------------------------------------------
//

AQLMathLeastSquareSwaptionVol::AQLMathLeastSquareSwaptionVol(const DoubleMatrix& Swaption_target_value_Mkt_,
													   mode mode_,
													   target target_
//                                                    const vector< vector< pair<double, double> > >& OptMat_and_Expire_
		)
{
	//mCostFunction = new AQLMathLeastSquareSwaptionVolQlib(Swaption_vol_Mkt_, mode_);
}

AQLMathLeastSquareSwaptionVol::AQLMathLeastSquareSwaptionVol(const DoubleMatrix& Swaption_target_value_Mkt_,
													   const DoubleMatrix& weight_,
													   mode mode_,
													   target target_
//                                                    const vector< vector< pair<double, double> > >& OptMat_and_Expire_
		)
{
	//mCostFunction = new AQLMathLeastSquareSwaptionVolQlib(Swaption_vol_Mkt_, weight_, mode_);
}

AQLMathLeastSquareSwaptionVol::AQLMathLeastSquareSwaptionVol(bool isNew_)
{
}


AQLMathLeastSquareSwaptionVol::AQLMathLeastSquareSwaptionVol( const AQLMathLeastSquareSwaptionVol& rhs )
{
	//mCostFunction = new AQLMathLeastSquareSwaptionVolQlib(*(dynamic_cast<AQLMathLeastSquareSwaptionVolQlib*>(rhs.mCostFunction)));
}

AQLMathLeastSquareSwaptionVol::~AQLMathLeastSquareSwaptionVol()
{
}


//
//--------------------------------------------------------------------------------
//

AQLMathLeastSquareSwaptionVolDiscModel::AQLMathLeastSquareSwaptionVolDiscModel( const DoubleMatrix& Swaption_target_value_Mkt_,
																		  vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
																		  mode mode_,
																		  target target_
		)
:
AQLMathLeastSquareSwaptionVol(false)
{
	mCostFunction = std::make_shared<AQLMathLeastSquareSwaptionVolDiscModelQlib>(Swaption_target_value_Mkt_, Swaption_Vol_, (AQLMathLeastSquareSwaptionVolDiscModelQlib::mode)mode_, (AQLMathLeastSquareSwaptionVolDiscModelQlib::target)target_);
}

AQLMathLeastSquareSwaptionVolDiscModel::AQLMathLeastSquareSwaptionVolDiscModel( const DoubleMatrix& Swaption_target_value_Mkt_,
																		  vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
																		  const DoubleMatrix& weight_,
																		  mode mode_,
																		  target target_
		)
:
AQLMathLeastSquareSwaptionVol(false)
{
	mCostFunction = std::make_shared<AQLMathLeastSquareSwaptionVolDiscModelQlib>(Swaption_target_value_Mkt_, Swaption_Vol_, weight_, (AQLMathLeastSquareSwaptionVolDiscModelQlib::mode)mode_, (AQLMathLeastSquareSwaptionVolDiscModelQlib::target)target_);
}

AQLMathLeastSquareSwaptionVolDiscModel::AQLMathLeastSquareSwaptionVolDiscModel( const AQLMathLeastSquareSwaptionVolDiscModel& rhs )
:
AQLMathLeastSquareSwaptionVol(false)
{
	mCostFunction = (rhs.mCostFunction != nullptr ? std::shared_ptr<QuantLib::CostFunction>(dynamic_pointer_cast<AQLMathLeastSquareSwaptionVolDiscModelQlib>(rhs.mCostFunction)->clone()) : std::shared_ptr<QuantLib::CostFunction>());
}

AQLMathLeastSquareSwaptionVolDiscModel::AQLMathLeastSquareSwaptionVolDiscModel(bool isNew_)
:
AQLMathLeastSquareSwaptionVol(false)
{
}


AQLMathLeastSquareSwaptionVolDiscModel::~AQLMathLeastSquareSwaptionVolDiscModel()
{
}


AQLMathLeastSquareSwaptionVolDiscModel& AQLMathLeastSquareSwaptionVolDiscModel::operator =(const AQLMathLeastSquareSwaptionVolDiscModel& rhs)
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

AQLMathLeastSquareSwaptionVolDiscModelV::AQLMathLeastSquareSwaptionVolDiscModelV(const DoubleMatrix& Swaption_target_value_Mkt_,
																		   vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
																		   mode mode_,
																		   target target_
		)
:
AQLMathLeastSquareSwaptionVolDiscModel(false)
{
	mCostFunction = std::make_shared<AQLMathLeastSquareSwaptionVolDiscModelVQlib>(Swaption_target_value_Mkt_, Swaption_Vol_, (AQLMathLeastSquareSwaptionVolDiscModelQlib::mode)mode_, (AQLMathLeastSquareSwaptionVolDiscModelQlib::target)target_);
}

AQLMathLeastSquareSwaptionVolDiscModelV::AQLMathLeastSquareSwaptionVolDiscModelV(const DoubleMatrix& Swaption_target_value_Mkt_,
                                                                           vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
                                                                           const DoubleMatrix& weight_,
                                                                           mode mode_,
																		   target target_
                                                                          )
:
AQLMathLeastSquareSwaptionVolDiscModel(false)
{
	mCostFunction = std::make_shared<AQLMathLeastSquareSwaptionVolDiscModelVQlib>(Swaption_target_value_Mkt_, Swaption_Vol_, weight_, (AQLMathLeastSquareSwaptionVolDiscModelQlib::mode)mode_, (AQLMathLeastSquareSwaptionVolDiscModelQlib::target)target_);
}

AQLMathLeastSquareSwaptionVolDiscModelV::AQLMathLeastSquareSwaptionVolDiscModelV( const AQLMathLeastSquareSwaptionVolDiscModelV& rhs )
:
AQLMathLeastSquareSwaptionVolDiscModel(false)
{
	mCostFunction = (rhs.mCostFunction != nullptr ? std::shared_ptr<QuantLib::CostFunction>(dynamic_pointer_cast<AQLMathLeastSquareSwaptionVolDiscModelVQlib>(rhs.mCostFunction)->clone()) : std::shared_ptr<QuantLib::CostFunction>() );
}

AQLMathLeastSquareSwaptionVolDiscModelV::~AQLMathLeastSquareSwaptionVolDiscModelV()
{
}


AQLMathLeastSquareSwaptionVolDiscModelV& AQLMathLeastSquareSwaptionVolDiscModelV::operator =(const AQLMathLeastSquareSwaptionVolDiscModelV& rhs)
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

AQLMathLeastSquareSwaptionVolDiscModelF::AQLMathLeastSquareSwaptionVolDiscModelF(const DoubleMatrix& Swaption_target_value_Mkt_,
																		   vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
																		   double T_max_,
																		   mode mode_,
																		   target target_
		)
:
AQLMathLeastSquareSwaptionVolDiscModel(false)
{
	mCostFunction = std::make_shared<AQLMathLeastSquareSwaptionVolDiscModelFQlib>(Swaption_target_value_Mkt_, Swaption_Vol_, T_max_, (AQLMathLeastSquareSwaptionVolDiscModelQlib::mode)mode_, (AQLMathLeastSquareSwaptionVolDiscModelQlib::target)target_);
}

AQLMathLeastSquareSwaptionVolDiscModelF::AQLMathLeastSquareSwaptionVolDiscModelF(const DoubleMatrix& Swaption_target_value_Mkt_,
                                                                           vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
                                                                           double T_max_,
                                                                           const DoubleMatrix& weight_,
                                                                           mode mode_,
																		   target target_
                                                                          )
:
AQLMathLeastSquareSwaptionVolDiscModel(false)
{
	mCostFunction = std::make_shared<AQLMathLeastSquareSwaptionVolDiscModelFQlib>(Swaption_target_value_Mkt_, Swaption_Vol_, T_max_, weight_, (AQLMathLeastSquareSwaptionVolDiscModelQlib::mode)mode_, (AQLMathLeastSquareSwaptionVolDiscModelQlib::target)target_);
}

AQLMathLeastSquareSwaptionVolDiscModelF::AQLMathLeastSquareSwaptionVolDiscModelF( const AQLMathLeastSquareSwaptionVolDiscModelF& rhs )
:
AQLMathLeastSquareSwaptionVolDiscModel(false)
{
	mCostFunction = (rhs.mCostFunction != nullptr ? std::shared_ptr<QuantLib::CostFunction>(dynamic_pointer_cast<AQLMathLeastSquareSwaptionVolDiscModelFQlib>(rhs.mCostFunction)->clone()) : std::shared_ptr<QuantLib::CostFunction>() );
}

AQLMathLeastSquareSwaptionVolDiscModelF::~AQLMathLeastSquareSwaptionVolDiscModelF()
{
}


AQLMathLeastSquareSwaptionVolDiscModelF& AQLMathLeastSquareSwaptionVolDiscModelF::operator =(const AQLMathLeastSquareSwaptionVolDiscModelF& rhs)
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

AQLMathLeastSquareSwaptionVolDiscModelG::AQLMathLeastSquareSwaptionVolDiscModelG(const DoubleMatrix& Swaption_target_value_Mkt_,
																		   vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
																		   mode mode_,
																		   target target_
		)
:
AQLMathLeastSquareSwaptionVolDiscModel(false)
{
	mCostFunction = std::make_shared<AQLMathLeastSquareSwaptionVolDiscModelGQlib>(Swaption_target_value_Mkt_, Swaption_Vol_, (AQLMathLeastSquareSwaptionVolDiscModelQlib::mode)mode_, (AQLMathLeastSquareSwaptionVolDiscModelQlib::target)target_);
}

AQLMathLeastSquareSwaptionVolDiscModelG::AQLMathLeastSquareSwaptionVolDiscModelG(const DoubleMatrix& Swaption_target_value_Mkt_,
                                                                           vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
                                                                           const DoubleMatrix& weight_,
                                                                           mode mode_,
																		   target target_
                                                                          )
:
AQLMathLeastSquareSwaptionVolDiscModel(false)
{
	mCostFunction = std::make_shared<AQLMathLeastSquareSwaptionVolDiscModelGQlib>(Swaption_target_value_Mkt_, Swaption_Vol_, weight_, (AQLMathLeastSquareSwaptionVolDiscModelQlib::mode)mode_, (AQLMathLeastSquareSwaptionVolDiscModelQlib::target)target_);
}

AQLMathLeastSquareSwaptionVolDiscModelG::AQLMathLeastSquareSwaptionVolDiscModelG( const AQLMathLeastSquareSwaptionVolDiscModelG& rhs )
:
AQLMathLeastSquareSwaptionVolDiscModel(false)
{
	mCostFunction = (rhs.mCostFunction != nullptr ? std::shared_ptr<QuantLib::CostFunction>(dynamic_pointer_cast<AQLMathLeastSquareSwaptionVolDiscModelGQlib>(rhs.mCostFunction)->clone()) : std::shared_ptr<QuantLib::CostFunction>() );
}

AQLMathLeastSquareSwaptionVolDiscModelG::~AQLMathLeastSquareSwaptionVolDiscModelG()
{
}


AQLMathLeastSquareSwaptionVolDiscModelG& AQLMathLeastSquareSwaptionVolDiscModelG::operator =(const AQLMathLeastSquareSwaptionVolDiscModelG& rhs)
{
    if ( this != &rhs )
    {
        *mCostFunction = *rhs.mCostFunction;
    }
    return *this;
}
