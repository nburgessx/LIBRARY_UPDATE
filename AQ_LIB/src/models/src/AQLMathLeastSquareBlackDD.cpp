// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/

// stl

#include "AQLMathLeastSquareBlackDD.h"

/*!
    @brief constructor

	@param[in] s0
	@param[in] curve0_d
	@param[in] curve0_f
	@param[in] t
	@param[in] strike
	@param[in] prem
	@param[in] weight
	@param[in] callPutFlg
	@param[in] isCalibVol
	@param[in] isCalibSkew

*/
AQLMathLeastSquareBlackDD::AQLMathLeastSquareBlackDD(double s0,
												AQLRatesPathElementCurve& curve0_d,
												AQLRatesPathElementCurve& curve0_f,
												const DoubleArray& t,
												const DoubleMatrix& strike,
												const DoubleMatrix& prem,
												const DoubleMatrix& weight,
												const vector<IntArray>& callPutFlg,
												bool isCalibVol,
												bool isCalibSkew)
{
	mCostFunction = std::make_shared<AQLMathLeastSquareBlackDDQlib>(s0, curve0_d, curve0_f, t, strike, prem, weight, callPutFlg, isCalibVol, isCalibSkew);
}

/*!
    @brief copy constructor

	@param[in] AQLMathLeastSquareBlackDD object

*/
AQLMathLeastSquareBlackDD::AQLMathLeastSquareBlackDD(const AQLMathLeastSquareBlackDD& rhs)
{
	mCostFunction = (rhs.mCostFunction != nullptr ? std::shared_ptr<QuantLib::CostFunction>(dynamic_pointer_cast<AQLMathLeastSquareBlackDDQlib>(rhs.mCostFunction)->clone()) : std::shared_ptr<QuantLib::CostFunction>() );
}

/*!
    @brief destructor

*/
AQLMathLeastSquareBlackDD::~AQLMathLeastSquareBlackDD()
{
}

/*!
    @brief operator

	@param[in] AQLMathLeastSquareBlackDD object

*/
AQLMathLeastSquareBlackDD& AQLMathLeastSquareBlackDD::operator =(const AQLMathLeastSquareBlackDD& rhs)
{
    if ( this != &rhs )
    {
		mCostFunction = (rhs.mCostFunction != nullptr ? std::shared_ptr<QuantLib::CostFunction>(dynamic_pointer_cast<AQLMathLeastSquareBlackDDQlib>(rhs.mCostFunction)->clone()) : std::shared_ptr<QuantLib::CostFunction>() );
    }
    return *this;
}

/*!
    @brief operator

	@param[in] AQLMathLeastSquareBlackDD object

*/
DoubleArray AQLMathLeastSquareBlackDD::getBlackVol(const DoubleArray& x) const
{
    return dynamic_pointer_cast<AQLMathLeastSquareBlackDDQlib>(mCostFunction)->getBlackVol(x);
}

/*!
    @brief constructor

	@param[in] s0
	@param[in] curve0_d
	@param[in] curve0_f
	@param[in] t
	@param[in] strike
	@param[in] prem
	@param[in] weight
	@param[in] callPutFlg

*/
AQLMathLeastSquareBlackDDVol::AQLMathLeastSquareBlackDDVol(double s0,
													AQLRatesPathElementCurve& curve0_d,
													AQLRatesPathElementCurve& curve0_f,
													const DoubleArray& t,
													const DoubleMatrix& strike,
													const DoubleMatrix& prem,
													const DoubleMatrix& weight,
													const vector<IntArray>& callPutFlg)
{
	mCostFunction = std::make_shared<AQLMathLeastSquareBlackDDVolQlib>(s0, curve0_d, curve0_f, t, strike, prem, weight, callPutFlg);
}

/*!
    @brief copy constructor

	@param[in] AQLMathLeastSquareBlackDDVol object

*/
AQLMathLeastSquareBlackDDVol::AQLMathLeastSquareBlackDDVol(const AQLMathLeastSquareBlackDDVol& rhs)
{
	mCostFunction = (rhs.mCostFunction != nullptr ? std::shared_ptr<QuantLib::CostFunction>(dynamic_pointer_cast<AQLMathLeastSquareBlackDDVolQlib>(rhs.mCostFunction)->clone()) : std::shared_ptr<QuantLib::CostFunction>() );
}

/*!
    @brief destructor

*/
AQLMathLeastSquareBlackDDVol::~AQLMathLeastSquareBlackDDVol()
{
}

/*!
    @brief operator

	@param[in] AQLMathLeastSquareBlackDDVolQlib object

*/
AQLMathLeastSquareBlackDDVol& AQLMathLeastSquareBlackDDVol::operator =(const AQLMathLeastSquareBlackDDVol& rhs)
{
    if ( this != &rhs )
    {
		mCostFunction = (rhs.mCostFunction != nullptr ? std::shared_ptr<QuantLib::CostFunction>(dynamic_pointer_cast<AQLMathLeastSquareBlackDDVolQlib>(rhs.mCostFunction)->clone()) : std::shared_ptr<QuantLib::CostFunction>() );
    }
    return *this;
}

/*!
    @brief constructor

	@param[in] s0
	@param[in] curve0_d
	@param[in] curve0_f
	@param[in] t
	@param[in] strike
	@param[in] prem
	@param[in] weight
	@param[in] callPutFlg

*/
AQLMathLeastSquareBlackDDSkew::AQLMathLeastSquareBlackDDSkew(double s0,
													 AQLRatesPathElementCurve& curve0_d,
													 AQLRatesPathElementCurve& curve0_f,
													 const DoubleArray& t,
													 const DoubleMatrix& strike,
													 const DoubleMatrix& prem,
													 const DoubleMatrix& weight,
													 const DoubleArray& impVol,
													 const vector<IntVector>& callPutFlg)
{
	mCostFunction = std::make_shared<AQLMathLeastSquareBlackDDSkewQlib>(s0, curve0_d, curve0_f, t, strike, prem, weight, impVol, callPutFlg);
}

/*!
    @brief copy constructor

	@param[in] AQLMathLeastSquareBlackDDSkew object

*/
AQLMathLeastSquareBlackDDSkew::AQLMathLeastSquareBlackDDSkew(const AQLMathLeastSquareBlackDDSkew& rhs)
{
	mCostFunction = (rhs.mCostFunction != nullptr ? std::shared_ptr<QuantLib::CostFunction>(dynamic_pointer_cast<AQLMathLeastSquareBlackDDSkewQlib>(rhs.mCostFunction)->clone()) : std::shared_ptr<QuantLib::CostFunction>() );
}

/*!
    @brief destructor

*/
AQLMathLeastSquareBlackDDSkew::~AQLMathLeastSquareBlackDDSkew()
{
}

/*!
    @brief operator

	@param[in] AQLMathLeastSquareBlackDDSkew object

*/
AQLMathLeastSquareBlackDDSkew& AQLMathLeastSquareBlackDDSkew::operator =(const AQLMathLeastSquareBlackDDSkew& rhs)
{
    if ( this != &rhs )
    {
		mCostFunction = (rhs.mCostFunction != nullptr ? std::shared_ptr<QuantLib::CostFunction>(dynamic_pointer_cast<AQLMathLeastSquareBlackDDSkewQlib>(rhs.mCostFunction)->clone()) : std::shared_ptr<QuantLib::CostFunction>() );
    }
    return *this;
}


/*!
    @brief constructor

	@param[in] skewMax
	@param[in] skewMin
	@param[in] volMax
	@param[in] volMin
	@param[in] n

*/
AQLMathBoundaryConstraintMktSkewVol::AQLMathBoundaryConstraintMktSkewVol(double skewMax,
																   double skewMin, 
																   double volMax, 
																   double volMin, 
																   size_t n)
{
	mConstraint = std::shared_ptr<QuantLib::Constraint>(new AQLMathBoundaryConstraintMktSkewVolQLib(skewMax, skewMin, volMax, volMin, n));
}


/*!
    @brief destructor

*/
AQLMathBoundaryConstraintMktSkewVol::~AQLMathBoundaryConstraintMktSkewVol()
{
}
