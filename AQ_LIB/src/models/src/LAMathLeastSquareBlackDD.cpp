// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/

// stl

#include "LAMathLeastSquareBlackDD.h"

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
LAMathLeastSquareBlackDD::LAMathLeastSquareBlackDD(double s0,
												LARatesPathElementCurve& curve0_d,
												LARatesPathElementCurve& curve0_f,
												const DoubleArray& t,
												const DoubleMatrix& strike,
												const DoubleMatrix& prem,
												const DoubleMatrix& weight,
												const vector<IntArray>& callPutFlg,
												bool isCalibVol,
												bool isCalibSkew)
{
	mCostFunction = std::make_shared<LAMathLeastSquareBlackDDQlib>(s0, curve0_d, curve0_f, t, strike, prem, weight, callPutFlg, isCalibVol, isCalibSkew);
}

/*!
    @brief copy constructor

	@param[in] LAMathLeastSquareBlackDD object

*/
LAMathLeastSquareBlackDD::LAMathLeastSquareBlackDD(const LAMathLeastSquareBlackDD& rhs)
{
	mCostFunction = (rhs.mCostFunction != nullptr ? std::shared_ptr<QuantLib::CostFunction>(dynamic_pointer_cast<LAMathLeastSquareBlackDDQlib>(rhs.mCostFunction)->clone()) : std::shared_ptr<QuantLib::CostFunction>() );
}

/*!
    @brief destructor

*/
LAMathLeastSquareBlackDD::~LAMathLeastSquareBlackDD()
{
}

/*!
    @brief operator

	@param[in] LAMathLeastSquareBlackDD object

*/
LAMathLeastSquareBlackDD& LAMathLeastSquareBlackDD::operator =(const LAMathLeastSquareBlackDD& rhs)
{
    if ( this != &rhs )
    {
		mCostFunction = (rhs.mCostFunction != nullptr ? std::shared_ptr<QuantLib::CostFunction>(dynamic_pointer_cast<LAMathLeastSquareBlackDDQlib>(rhs.mCostFunction)->clone()) : std::shared_ptr<QuantLib::CostFunction>() );
    }
    return *this;
}

/*!
    @brief operator

	@param[in] LAMathLeastSquareBlackDD object

*/
DoubleArray LAMathLeastSquareBlackDD::getBlackVol(const DoubleArray& x) const
{
    return dynamic_pointer_cast<LAMathLeastSquareBlackDDQlib>(mCostFunction)->getBlackVol(x);
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
LAMathLeastSquareBlackDDVol::LAMathLeastSquareBlackDDVol(double s0,
													LARatesPathElementCurve& curve0_d,
													LARatesPathElementCurve& curve0_f,
													const DoubleArray& t,
													const DoubleMatrix& strike,
													const DoubleMatrix& prem,
													const DoubleMatrix& weight,
													const vector<IntArray>& callPutFlg)
{
	mCostFunction = std::make_shared<LAMathLeastSquareBlackDDVolQlib>(s0, curve0_d, curve0_f, t, strike, prem, weight, callPutFlg);
}

/*!
    @brief copy constructor

	@param[in] LAMathLeastSquareBlackDDVol object

*/
LAMathLeastSquareBlackDDVol::LAMathLeastSquareBlackDDVol(const LAMathLeastSquareBlackDDVol& rhs)
{
	mCostFunction = (rhs.mCostFunction != nullptr ? std::shared_ptr<QuantLib::CostFunction>(dynamic_pointer_cast<LAMathLeastSquareBlackDDVolQlib>(rhs.mCostFunction)->clone()) : std::shared_ptr<QuantLib::CostFunction>() );
}

/*!
    @brief destructor

*/
LAMathLeastSquareBlackDDVol::~LAMathLeastSquareBlackDDVol()
{
}

/*!
    @brief operator

	@param[in] LAMathLeastSquareBlackDDVolQlib object

*/
LAMathLeastSquareBlackDDVol& LAMathLeastSquareBlackDDVol::operator =(const LAMathLeastSquareBlackDDVol& rhs)
{
    if ( this != &rhs )
    {
		mCostFunction = (rhs.mCostFunction != nullptr ? std::shared_ptr<QuantLib::CostFunction>(dynamic_pointer_cast<LAMathLeastSquareBlackDDVolQlib>(rhs.mCostFunction)->clone()) : std::shared_ptr<QuantLib::CostFunction>() );
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
LAMathLeastSquareBlackDDSkew::LAMathLeastSquareBlackDDSkew(double s0,
													 LARatesPathElementCurve& curve0_d,
													 LARatesPathElementCurve& curve0_f,
													 const DoubleArray& t,
													 const DoubleMatrix& strike,
													 const DoubleMatrix& prem,
													 const DoubleMatrix& weight,
													 const DoubleArray& impVol,
													 const vector<IntVector>& callPutFlg)
{
	mCostFunction = std::make_shared<LAMathLeastSquareBlackDDSkewQlib>(s0, curve0_d, curve0_f, t, strike, prem, weight, impVol, callPutFlg);
}

/*!
    @brief copy constructor

	@param[in] LAMathLeastSquareBlackDDSkew object

*/
LAMathLeastSquareBlackDDSkew::LAMathLeastSquareBlackDDSkew(const LAMathLeastSquareBlackDDSkew& rhs)
{
	mCostFunction = (rhs.mCostFunction != nullptr ? std::shared_ptr<QuantLib::CostFunction>(dynamic_pointer_cast<LAMathLeastSquareBlackDDSkewQlib>(rhs.mCostFunction)->clone()) : std::shared_ptr<QuantLib::CostFunction>() );
}

/*!
    @brief destructor

*/
LAMathLeastSquareBlackDDSkew::~LAMathLeastSquareBlackDDSkew()
{
}

/*!
    @brief operator

	@param[in] LAMathLeastSquareBlackDDSkew object

*/
LAMathLeastSquareBlackDDSkew& LAMathLeastSquareBlackDDSkew::operator =(const LAMathLeastSquareBlackDDSkew& rhs)
{
    if ( this != &rhs )
    {
		mCostFunction = (rhs.mCostFunction != nullptr ? std::shared_ptr<QuantLib::CostFunction>(dynamic_pointer_cast<LAMathLeastSquareBlackDDSkewQlib>(rhs.mCostFunction)->clone()) : std::shared_ptr<QuantLib::CostFunction>() );
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
LAMathBoundaryConstraintMktSkewVol::LAMathBoundaryConstraintMktSkewVol(double skewMax,
																   double skewMin, 
																   double volMax, 
																   double volMin, 
																   size_t n)
{
	mConstraint = std::shared_ptr<QuantLib::Constraint>(new LAMathBoundaryConstraintMktSkewVolQLib(skewMax, skewMin, volMax, volMin, n));
}


/*!
    @brief destructor

*/
LAMathBoundaryConstraintMktSkewVol::~LAMathBoundaryConstraintMktSkewVol()
{
}
