/*! @file
    @brief Source code of base class of polynomial fitting class



*/
//  2007, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAPolyFitLS.cpp
//
//  SYNOPSIS    :       LAPolyFitLS
//  DESCRIPTION :       Declaration of base class of polynomial fitting class
//
//  SEE ALSO    :       
//  TYME        :       CLASS
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAPolyFitLS.h"
#include "LAPolynomialBase.h"
#include "LALinearRegression.h"
using namespace std;

//================ LAPolyFitLS ===================================
/*!
	@brief default constructor
    @param[in] isExcludeOutlier_Y	flag of exclude outlier by judging Y(explained variables) value 
    @param[in] criteria_Y			criteria of outlier of judging Y value case
    @param[in] isExcludeOutlier_X	flag of exclude outlier by judging X(explanatory variables) value   
    @param[in] criteria_X			criteria of outlier of judging X value case
    @param[in] isExcludeOutlier_e   flag of exclude outlier by judging residual value
    @param[in] criteria_e			criteria of outlier of judging residual value case
	@param[in] isShift_Y            flag of shift Y value
	@param[in] isScalse_Y           flag of scale Y value
	@param[in] isShift_X            flag of shift X value
	@param[in] isScalse_X           flag of scale X value

*/
LAPolyFitLS::LAPolyFitLS(bool isExcludeOutlier_Y, double criteria_Y, bool isExcludeOutlier_X, double criteria_X, bool isExcludeOutlier_e, double criteria_e, bool isShift_Y, bool isScale_Y, bool isShift_X, bool isScale_X)
: mReg(isExcludeOutlier_Y, criteria_Y, isExcludeOutlier_X, criteria_X, isExcludeOutlier_e, criteria_e), mIsShift_Y(isShift_Y), mIsScale_Y(isScale_Y), mIsShift_X(isShift_X), mIsScale_X(isScale_X)
{

}
/*!
	@brief copy constructor
*/
/*LAPolyFitLS::LAPolyFitLS(const LAPolyFitLS& v) 
: LACoreFunctionBase(v)
{

}*/

/*!
	@brief destructor
*/
LAPolyFitLS::~LAPolyFitLS() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAPolyFitLS::clone() const
{
    try 
	{
		return new LAPolyFitLS(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAPolyFitLS::isTypeOf(function_t id) const
{
	return (id == FN_POLYFITLS ? true : LAPolyFitBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAPolyFitLS::getType() const
{
	return FN_POLYFITLS;
}


/*!
	@brief execute regression
	
	@param[in] y explained variables
	@param[in] x explanatory variables
	@param[in, out] poly polynomial
	@param[out] y transformed variables
	@param[out] x transformed variables
*/
void
LAPolyFitLS::regress(const DoubleArray& y, const DoubleMatrix& x, LAPolynomialBase& poly, DoubleArray *p_y_t, DoubleMatrix *p_x_t) const
{
	if (x.size() != y.size())
	{
		//error
        throw LACoreInvalidData("size of y and size of x rows are not same", __FILE__, __LINE__);
	}
	DoubleMatrix basis_func(y.size());

	DoubleArray y_t;
	DoubleMatrix x_t;
	poly.calcTransformedValue(y, x, mIsScale_Y, mIsShift_Y, mIsScale_X, mIsShift_X, y_t, x_t);

	if (p_y_t)
	{
		*p_y_t = y_t;
	}
	if (p_x_t)
	{
		*p_x_t = x_t;
	}

	DoubleArray _y_t;
	unsigned int count = 0;
	bool isFirst = true;
	for (unsigned int i = 0; i < y_t.size(); i++)
	{
		if (x_t[i].size() == 0)
		{
			if (_y_t.size() == 0) _y_t.resize(y_t.size());
			continue;
		}
		if (_y_t.size() != 0) _y_t[count] = y_t[i];
		if (isFirst)
		{
			poly.setNumVar(x_t[i].size());
			isFirst = false;
		}
		poly.calcBasisFunc(x_t[i], basis_func[count++]);
	}
    if (count == 0 || count < basis_func[0].size())
	{
		//error
        throw LACoreInvalidData("Data size is less than polynomial basis_func num.", __FILE__, __LINE__);
		
	}
	if (count != y_t.size())
	{
		_y_t.resize(count);
		basis_func.resize(count);
	}
	LALinearRegression reg(mReg);
	reg.setZeroInterception(true);
	if (count != y.size()) reg.setData(_y_t, basis_func);
	else reg.setData(y_t, basis_func);
	reg.executeRegression();
	poly.setParam(reg.getCoefficient());
}

void
LAPolyFitLS::setRegParam(bool isExcludeOutlier_Y, double criteria_Y, bool isExcludeOutlier_X, double criteria_X, bool isExcludeOutlier_e, double criteria_e, bool isShift_Y, bool isScale_Y, bool isShift_X, bool isScale_X, double SVDTolerance)
{
    mReg.setExcludeOutlier_Y(isExcludeOutlier_Y);
    mReg.setCriteria_Y(criteria_Y);
    mReg.setExcludeOutlier_X(isExcludeOutlier_X);
    mReg.setCriteria_X(criteria_X);
    mReg.setExcludeOutlier_e(isExcludeOutlier_e);
    mReg.setCriteria_e(criteria_e);
	mIsShift_Y = isShift_Y;
	mIsScale_Y = isScale_Y;
	mIsShift_X = isShift_X;
	mIsScale_X = isScale_X;
	mReg.setSVDTolerance(SVDTolerance);
	
}
