/*! @file
    @brief Source code of base class of polynomial class



*/
//  2007, AlgoQuantHub..

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLPolynomialBase.h"
#include "AQLCoreTemplateType.h"
#include "AQLBasic.h"
#include <numeric>


//================ AQLPolynomialBase ===================================
/*!
	@brief default constructor
	@param[in] order order of polynomial
	@param[in] varnum number of variables
	@param[in] checkflag check flag
*/
AQLPolynomialBase::AQLPolynomialBase(unsigned int order, unsigned int varnum , bool checkflag)
: AQLFunctionBase(checkflag)
, mOrderBF(order), mNumVar(varnum), mIsConvert(false), mShift_Y(0.0), mScale_Y(0.0), mShift_X(DoubleArray(1, 0.0)), mScale_X(DoubleArray(1, 0.0))
{

}
/*!
	@brief copy constructor
*/
AQLPolynomialBase::AQLPolynomialBase(const AQLPolynomialBase& v) 
	: AQLFunctionBase(v), mOrderBF(v.mOrderBF), mNumVar(v.mNumVar), mIsConvert(v.mIsConvert), mShift_Y(v.mShift_Y), mScale_Y(v.mScale_Y), mShift_X(v.mShift_X), mScale_X(v.mScale_X)
{

}

/*!
	@brief destructor
*/
AQLPolynomialBase::~AQLPolynomialBase() 
{

}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLPolynomialBase::isTypeOf(function_t id) const
{
	return (id == FN_POLYNOMIALBASE ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLPolynomialBase::getType() const
{
	return FN_POLYNOMIALBASE;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
AQLPolynomialBase::operator()(const DoubleArray& x) const
{
	if (!mIsConvert)
	{
		DoubleArray basis_func;
		calcBasisFunc(x, basis_func);
		return inner_product(basis_func.begin(), basis_func.end(), mParam.begin(), 0.0);
	}

	DoubleArray tmpX = x;
	DoubleArray basis_func;
	for (unsigned int i = 0; i < x.size(); ++i)
	{
		tmpX[i] -= mShift_X[i];
		tmpX[i] *= mScale_X[i];
	}
	calcBasisFunc(tmpX, basis_func);
	return inner_product(basis_func.begin(), basis_func.end(), mParam.begin(), 0.0) / mScale_Y +  mShift_Y;

}
/*!
    @brief combination function
	@param[in] m	total set size
	@param[in] n	size of choice
	@return the number of ways to uniquely choose n items from a set of size m
*/
unsigned int
AQLPolynomialBase::combin(unsigned int m, unsigned int n) const
{
	if (m < n)
	{
		throw AQLCoreInvalidData("first parameter must be equal or more than second parameter ", __FILE__, __LINE__);
	}
	if (n == 0 || m == n) return 1;
	unsigned int ret = 1;
	for (unsigned int i = m; i > m-n; i--)
		ret *= i;
	for (unsigned int i = 2; i < n+1; i++)
		ret /= i;
	return ret;
}

/*!
    @brief calculate transformed value
	@param[in] y           explained variables
	@param[in] x           explanatory variables
	@param[in] isShift_Y   flag of shift y
	@param[in] isScalse_Y  flag of scale y
	@param[in] isShift_X   flag of shift x
	@param[in] isScalse_X  flag of scale x
	@param[out] y_t        transformed y variables
	@param[out] x_t        transformed x variables
*/
void
AQLPolynomialBase::calcTransformedValue(const DoubleArray& y, const DoubleMatrix& x , bool isScale_Y, bool isShift_Y, bool isScale_X, bool isShift_X, DoubleArray& y_t,  DoubleMatrix& x_t)
{
	y_t = y;
	x_t = x;
	if (isShift_Y || isShift_X)
	{
		mIsConvert = true;
	}
	if (mIsConvert)
	{
		mScale_Y = 1.0;
		mScale_X = DoubleArray(x[0].size(), 1.0);
		mShift_Y = 0.0;
		mShift_X = DoubleArray(x[0].size(), 0.0);
		if (isShift_Y)
		{
			double sum1 = 0.0, sum2 = 0.0;
			unsigned int size_y = y.size();
			for (unsigned int i = 0; i < size_y; i++)
			{
				sum1 += y[i];
				sum2 += y[i] * y[i];
			}
			sum1 /= static_cast<double>(size_y);
			sum2 /= static_cast<double>(size_y);
			sum2 -= sum1 * sum1;
			if (sum2 > 0.0)
			{
				sum2 = AQLMath::sqrt(sum2);
			}
			if (isScale_Y && sum2 != 0.0)
			{
				mScale_Y /= sum2;
			}
			mShift_Y = sum1;
			for (unsigned int i = 0; i < size_y; i++)
			{
				y_t[i] -= mShift_Y;
				y_t[i] *= mScale_Y;
			}
		}
		if (isShift_X)
		{
			double sum1, sum2;
			unsigned int size_x = x.size();
			for (unsigned int j = 0; j < x[0].size(); j++)
			{
				sum1 = 0.0;
				sum2 = 0.0;
				for (unsigned int i = 0; i < size_x; i++)
				{
					sum1 += x[i][j];
					sum2 += x[i][j] * x[i][j];
				}
				sum1 /= static_cast<double>(size_x);
				sum2 /= static_cast<double>(size_x);
				sum2 -= sum1 * sum1;
				if (sum2 > 0.0)
				{
					sum2 = AQLMath::sqrt(sum2);
				}
				mShift_X[j] = sum1;
				if (isScale_X && sum2 != 0.0)
				{
					mScale_X[j] /= sum2;
				}
				for (unsigned int i = 0; i < size_x; i++)
				{
					x_t[i][j] -= mShift_X[j];
					x_t[i][j] *= mScale_X[j];
				}
			}
		}
	}
}

/*!
    @brief set standardization param
	@param[in] shift_y  
	@param[in] scale_y 
	@param[in] shift_x
	@param[in] scale_y
*/
void
AQLPolynomialBase::setStandardizationParam(const double shift_y, const double scale_y, const DoubleArray &shift_x, const DoubleArray &scale_x)
{
	mShift_Y = shift_y;
	mScale_Y = scale_y;
	mShift_X = shift_x;
	mScale_X = scale_x;
}

/*!
    @brief get standardization param
	@param[out] shift_y  
	@param[out] scale_y 
	@param[out] shift_x
	@param[out] scale_y
*/
void
AQLPolynomialBase::getStandardizationParam(double &shift_y, double &scale_y, DoubleArray &shift_x, DoubleArray &scale_x) const
{
	shift_y = mShift_Y;
	scale_y = mScale_Y;
	shift_x = mShift_X;
	scale_x = mScale_X;
}
