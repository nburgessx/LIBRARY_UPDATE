/*! @file
    @brief Declaration of base class of Function class

	Declare abstract base class to represent function(y=f(x1,x2,...,xn)).
	This class has a pure virtual method "operator()".
*/


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLCombinationFunc.h"
#include "AQLFunctionBase.h"
#include "AQLIntegralBase.h"
#include "AQL1DIntegral.h"
#include "AQLDataVector.h"

using namespace std;
//================ AQLFunctionBase ===================================
/*!
	@brief default constructor
	@param[in] checkflag check flag
*/
AQLFunctionBase::AQLFunctionBase(bool checkflag) 
: AQLCoreFunctionBase(), mPos(0), mUpdateFlag(true), mCheckFlag(checkflag)
{
	mX.resize(1);
}
/*!
	@brief copy constructor
*/
/*AQLFunctionBase::AQLFunctionBase(const AQLFunctionBase& v) 
: AQLCoreFunctionBase(v)
{
	mCheckFlag = v.mCheckFlag;
	mParam = v.mParam;
	mPos = v.mPos;
	mX = v.mX;
	mUpdateFlag = v.mUpdateFlag;
	mDiscontPoints = v.mDiscontPoints;
}*/

/*!
	@brief destructor
*/
AQLFunctionBase::~AQLFunctionBase() 
{

}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLFunctionBase::isTypeOf(function_t id) const
{
	return (id==FN_FUNCTION ? true : AQLCoreFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLFunctionBase::getType() const
{
	return FN_FUNCTION;
}

/*!
    @brief Return function value
	@param[in] x point, use mX except x
    @return function value
*/
double
AQLFunctionBase::operator()(double x) const
{
	DoubleArray xx = mX;
	xx[mPos] = x;
	return operator()(xx);
}

/*!
    @brief Set default position and point
	@param[in] pos default position in arguments
	@param[in] x default point to be evaluated
*/
AQLFunctionBase&
AQLFunctionBase::operator()(unsigned int pos, const DoubleArray& x)
{
	mPos = pos;
	mX = x;
/*!	mX.resize(x.size()+1);
	unsigned int i;
	for (i=0;i<pos;i++)
		mX=x[i];
	for (i=pos+1;i<mX.size();i++)
		mX=x[i-1];*/
	return *this;
}

/*!
    @brief Return integral result
	@param[in] x integral region
	@param[in] pIntegral pointer to integral method
    @return integral result
*/
double
AQLFunctionBase::integral(const std::vector<std::pair<double,double> >& x,const AQLIntegralBase* pIntegral) const
{
	if (pIntegral == NULL)
	{
		return integral(x);	
	}

	return pIntegral->integrate(*this, x);
}
/*!
    @brief Return integral result
	@param[in] x integral region
    @return integral result
*/
double
AQLFunctionBase::integral(const std::vector<std::pair<double,double> >& x) const
{	
	(void)x;
	throw AQLCoreNumericalError("This function does not support analyticaly integrated method", __FILE__, __LINE__);
}
/*!
    @brief Return integral result
	@param[in] xl lower side of integral region
	@param[in] xu upper side of integral region
	@param[in] pIntegral pointer to integral method
    @return integral result
*/
double
AQLFunctionBase::integral(double xl, double xu, const AQL1DIntegral* pIntegral) const
{
	if (pIntegral == NULL)
	{
		return integral(xl, xu);
	}
	return pIntegral->integrate(*this, xl, xu, mDiscontPoints);
}
/*!
    @brief Return integral result
	@param[in] x1 lower side of integral region
	@param[in] x2 upper side of integral region
	@param[in] pIntegral pointer to integral method
 	@param[in] grids boundary of partitions  
	@return integral result

	@note each partition integrate separately, and sum results
	@note grids must be sorted in advance
*/
/*double
AQLFunctionBase::integral(double xl, double xu, const AQL1DIntegral* pIntegral, const DoubleArray& grids) const
{
	if (pIntegral == NULL)
	{
		return integral(xl, xu);
	}
	return pIntegral->integrate(*this, xl, xu, grids);
}*/
/*!
    @brief Return integral result
	@param[in] xl lower side of integral region
	@param[in] xu upper side of integral region
    @return integral result
*/
double
AQLFunctionBase::integral(double xl, double xu) const
{
	(void)xl; (void)xu;
	throw AQLCoreNumericalError("This function does not support analyticaly integrated method", __FILE__, __LINE__);
}
/*!
    @brief Operator definition "+"
	@param[in] method added function
	@return result of addition
*/
AQLCombinationMethod
AQLFunctionBase::operator+(const AQLFunctionBase& method) const
{
	if (method.getType() == FN_COMBINATIONFUNC)return method+(*this);
	
	AQLCombinationMethod combifunc(mCheckFlag && method.mCheckFlag);
	vector<AQLFunctionBase*> vfunc,vfunc2;
	vfunc.push_back(dynamic_cast<AQLFunctionBase*>(clone()));
	vfunc2.push_back(dynamic_cast<AQLFunctionBase*>(method.clone()));	
	combifunc.mFuncs.push_back(pair<double,vector<AQLFunctionBase*> >(1,vfunc));
	combifunc.mFuncs.push_back(pair<double,vector<AQLFunctionBase*> >(1,vfunc2));
	combifunc.mDiscontPoints = mDiscontPoints;
	combifunc.mDiscontPoints.insert(method.mDiscontPoints.begin(), method.mDiscontPoints.end()); 

	return combifunc;	
}

/*!
    @brief Operator definition "+"
	@param[in] a added constant
	@return result of addition
*/
AQLCombinationMethod
AQLFunctionBase::operator+(double a) const
{
	AQLCombinationMethod combifunc(mCheckFlag);
	combifunc.mConst = a;
	
	vector<AQLFunctionBase*> vfunc;
	vfunc.push_back(dynamic_cast<AQLFunctionBase*>(clone()));
	combifunc.mFuncs.push_back(pair<double,vector<AQLFunctionBase*> >(1,vfunc));
	return combifunc;
}

/*!
    @brief Operator definition "*"
	@param[in] method multiplied function
	@return result of multiplicatioin
*/
AQLCombinationMethod
AQLFunctionBase::operator*(const AQLFunctionBase& method) const
{
	if (method.getType() == FN_COMBINATIONFUNC)return method*(*this);
	
	AQLCombinationMethod combifunc(mCheckFlag && method.mCheckFlag);
	vector<AQLFunctionBase*> vfunc;
	vfunc.push_back(dynamic_cast<AQLFunctionBase*>(clone()));
	vfunc.push_back(dynamic_cast<AQLFunctionBase*>(method.clone()));	
	combifunc.mFuncs.push_back(pair<double,vector<AQLFunctionBase*> >(1,vfunc));
	combifunc.mDiscontPoints = mDiscontPoints;
	combifunc.mDiscontPoints.insert(method.mDiscontPoints.begin(), method.mDiscontPoints.end()); 

	return combifunc;
}

/*!
    @brief Operator definition "*"
	@param[in] a multiplied constant
	@return result of multiplication
*/
AQLCombinationMethod
AQLFunctionBase::operator*(double a) const
{
	AQLCombinationMethod combifunc(mCheckFlag);
	vector<AQLFunctionBase*> vfunc;
	vfunc.push_back(dynamic_cast<AQLFunctionBase*>(clone()));
	combifunc.mFuncs.push_back(pair<double,vector<AQLFunctionBase*> >(a,vfunc));
	return combifunc;
}

/*!
    @brief Return partial derivative value
	@param[in] x integral point
	@param[in] pos variable location to implement derivative
	@param[in] calctype analytical or numerical integral
	@param[in] difftype both side or one side
	@param[in] delta grid width for numerical method
    @return partial deribative value
*/
double
AQLFunctionBase::partialDerivative(const DoubleArray& x,unsigned int pos,
								CALC_TYPE calctype, DIFF_TYPE difftype,double delta) const 
{
	if (calctype == NUMERICAL || calctype == DEFAULT)
	{
		if (mCheckFlag && !isDifferentiable(x,pos))
		{
			AQLString msg = "Not derivable";
			throw AQLCoreNumericalError(msg.getCString() , __FILE__, __LINE__);
		}
		if (difftype==ONESIDE)
		{
			DoubleArray x1 = x;
			x1[pos] += delta;
			return (operator()(x1)-operator()(x))/delta;
		}
		else
		{
			DoubleArray x1 = x;
			DoubleArray x2 = x;
			x1[pos] -= delta;
			x2[pos] += delta;
			return (operator()(x2)-operator()(x1))*0.5/delta;		
		}			
	}
	else
		throw AQLCoreNumericalError("This function does not support analytical derivative", __FILE__, __LINE__);

}
/*!
    @brief Return second partial derivative value
	@param[in] x point
	@param[in] posi one variable location to implement derivative
	@param[in] posj another variable location to implement derivative
	@param[in] calctype analytical or numerical integral
	@param[in] delta grid width for numerical method
    @return partial deribative value
*/
double
AQLFunctionBase::partialDerivative2(const DoubleArray& x, unsigned int posi,unsigned int posj,
									CALC_TYPE calctype, double delta) const
{
	if (calctype == NUMERICAL || calctype == DEFAULT)
	{
		if (mCheckFlag && !isDifferentiable(x,posi,posj))
		{
			AQLString msg = "Not derivable";
			throw AQLCoreNumericalError(msg.getCString() , __FILE__, __LINE__);
		}		
		if (posi==posj)
		{
			DoubleArray x1 = x;
			DoubleArray x2 = x;
			x1[posi] -= delta;
			x2[posi] += delta;
			return (operator()(x2)+operator()(x1) - 2.0 * operator()(x))/delta/delta;		
		}
		else
		{
			DoubleArray x1 = x;
			DoubleArray x2 = x;
			DoubleArray x3= x;
			DoubleArray x4 = x;
			x1[posi] += delta;
			x1[posj] += delta;
			x2[posi] -= delta;
			x2[posj] -= delta;
			x3[posi] -= delta;
			x3[posj] += delta;
			x4[posi] += delta;
			x4[posj] -= delta;
			return (operator()(x1)+operator()(x2) - operator()(x3) - operator()(x4))*0.25/delta/delta;	
		}			
	}
	else
		throw AQLCoreNumericalError("This function does not support analytical derivative", __FILE__, __LINE__);
}
/*!
    @brief Calculate function gradient
	@param[in] x point
	@param[out] out gradient
	@param[in] calctype analytical or numerical integral
	@param[in] difftype both side or one side
	@param[in] delta grid width for numerical method
*/
void
AQLFunctionBase::gradient(const DoubleArray& x, DoubleArray& out,
						 CALC_TYPE calctype, DIFF_TYPE difftype,double delta) const
{
	for (unsigned int i=0;i<x.size();i++)
		out[i]=partialDerivative(x,i,calctype,difftype,delta);
}
/*!
    @brief Calculate function hessian
	@param[in] x point
	@param[out] out hessian matrix
	@param[in] calctype analytical or numerical integral
	@param[in] delta grid width for numerical method
*/
void
AQLFunctionBase::hessian(const DoubleArray& x, DoubleMatrix& out,
						CALC_TYPE calctype, double delta) const
{
	for (unsigned int i=0;i<x.size();i++)
	{
		for (unsigned int j=0;j<i;j++)
		{		
			out[i][j] = partialDerivative2(x, i, j, calctype, delta);
			out[j][i] = out[i][j];
		}
		out[i][i] = partialDerivative2(x, i, i, calctype, delta);
	}
}
/*!
    @brief Check x is in valid domain
	@param[in] x point
	@return true x is in valid domain
*/
bool
AQLFunctionBase::isInDomain(const DoubleArray& x)const
{
	(void)x;
	throw AQLCoreNumericalError("This function does not support this method", __FILE__, __LINE__);
}

/*!
    @brief Check derivable or not at x
	@param[in] x point to be evaluated
	@param[in] pos armument location to implement derivative
	@return true x is in valid domain
*/
bool
AQLFunctionBase::isDifferentiable(const DoubleArray& x, unsigned int pos)const
{
	(void)pos; (void)x;
	throw AQLCoreNumericalError("This function does not support this method", __FILE__, __LINE__);
}

/*!
    @brief Check derivable or not at x
	@param[in] x point to be evaluated
	@param[in] posi armument location to implement first derivative
	@param[in] posj armument location to implement second derivative
	@return derivable or not
*/
bool
AQLFunctionBase::isDifferentiable(const DoubleArray& x, unsigned int posi,unsigned int posj)const
{
	(void)posi; (void)posj; (void)x;
	throw AQLCoreNumericalError("This function does not support this method", __FILE__, __LINE__);
}

/*!
    @brief Retern paramter
	@return function parameter
*/
/*DoubleArray&
AQLFunctionBase::getParam()
{
	mUpdateFlag = true;
	return mParam;
};*/ 

/*!
    @brief Set check flag
	@param[in] checkflag check flag to be set
*/
void
AQLFunctionBase::setCheckFlag(bool checkflag)
{
	mCheckFlag = checkflag;
}

/*!
    @brief Set function parameter
	@param[in] param parameter to be set
*/
void
AQLFunctionBase::setParam(const DoubleArray& param)
{
	mUpdateFlag = true;
	mParam.resize(param.size());
	mParam = param;
};

/*!
    @brief set discontinuaous points for 1DIntegral
	@param[in] points discontinuaous points for 1DIntegral
*/
void
AQLFunctionBase::setDiscontPoints(const std::set<double>& points)
{
	mDiscontPoints = points;
}

/*!
    @brief return string representaion of this function parameter
	@return string representaion of this function parameter
*/
AQLString
AQLFunctionBase::convertToString(void) const
{
	AQLDataDoubles ret(mParam);
	return ret.convertToString();
}

								

/*!
    @brief transform from string representaion
	@param[in str string representaion of this function parameter
*/
void
AQLFunctionBase::convertFromString(const AQLString& str)
{
	AQLDataDoubles ret;
	ret.convertFromString(str);
	if (!ret.isNull()) mParam = ret.get();
	else mParam.clear();
}

/*!
    @brief equal operator
    @param[in] a source object

    @return this object
*/
AQLFunctionBase&
AQLFunctionBase::operator = (const AQLFunctionBase& v)
{
	if (&v == this) return *this;

	mPos = v.mPos;
	mX = v.mX;
	mUpdateFlag = v.mUpdateFlag;
	mParam = v.mParam;
	mCheckFlag = v.mCheckFlag;
	mDiscontPoints = v.mDiscontPoints;
	
	return *this;
}
