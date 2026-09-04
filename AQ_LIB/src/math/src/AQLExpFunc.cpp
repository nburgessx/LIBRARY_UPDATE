/*! @file
    @brief Source code of class to represent exponential function

	This class derives from AQLFunctionBase

*/



#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLExpFunc.h"
#include "AQLBasic.h"
#include "AQLDist.h"

using namespace std;
//================ AQLExpMethod ===================================
/*!
	@brief constructor to make exp(a*method)
	@param[in] a coefficient
	@param[in] method power function
*/
AQLExpMethod::AQLExpMethod(double a, const AQLFunctionBase& method) 
: AQLFunctionBase(method), mpFunc(NULL)
{
	DoubleArray param(1);
	param[0] = a;
	setParam(param);
	mpFunc = dynamic_cast<AQLFunctionBase*>(method.clone());
//	mDiscontPoints = method.getDiscontPoints();
}

/*!
	@brief destructor
*/
AQLExpMethod::~AQLExpMethod() 
{
	delete mpFunc;
}

/*!
	@brief copy constructor
*/
AQLExpMethod::AQLExpMethod(const AQLExpMethod& v) 
: AQLFunctionBase(v), mpFunc(NULL)
{
	if (v.mpFunc) mpFunc = dynamic_cast<AQLFunctionBase*>(v.mpFunc->clone());
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLExpMethod::clone() const
{
    try 
	{
		return new AQLExpMethod(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLExpMethod::isTypeOf(function_t id) const
{
	return (id == FN_EXPFUNC ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLExpMethod::getType() const
{
	return FN_EXPFUNC;
}

/*!
    @brief Return function value
	@param[in] x: point
    @return function value
*/
double
AQLExpMethod::operator()(const DoubleArray& x) const
{
//	if (mCheckFlag && !isInDomain(x))
//	{
//		throw AQLCoreNumericalError("Not in Domain" , __FILE__, __LINE__);
//	}
	return AQLMath::exp(mParam[0]*mpFunc->operator()(x));
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
AQLExpMethod::partialDerivative(const DoubleArray& x,unsigned int pos,
								CALC_TYPE calctype, DIFF_TYPE difftype,double delta) const
{
	if (calctype == NUMERICAL)
		return AQLFunctionBase::partialDerivative(x,pos,calctype,difftype,delta);
	else
	{
	//	if (mCheckFlag && !isDifferentiable(x,pos))
	//	{
	//		throw AQLCoreNumericalError("Not Derivable" , __FILE__, __LINE__);
	//	}			
		return operator()(x) * mParam[0] * mpFunc->partialDerivative(x, pos, calctype, difftype, delta);
	}

}

/*!
    @brief Return second partial derivative value
	@param[in] x integral point
	@param[in] posi one variable location to implement derivative
	@param[in] posj another variable location to implement derivative
	@param[in] calctype analytical or numerical integral
	@param[in] delta grid width for numerical method
    @return partial deribative value
*/
double
AQLExpMethod::partialDerivative2(const DoubleArray& x, unsigned int posi,unsigned int posj,
												CALC_TYPE calctype, double delta) const
{
	if (calctype == NUMERICAL)
		return AQLFunctionBase::partialDerivative2(x, posi, posj, calctype, delta);
	else
	{
	//	if (mCheckFlag && !isDifferentiable(x,posi,posj))
	//	{
	//		throw AQLCoreNumericalError("Not Derivable" , __FILE__, __LINE__);
	//	}			
		return operator()(x) * mParam[0] * mParam[0] * mpFunc->partialDerivative(x, posi) * mpFunc->partialDerivative(x, posj, calctype, BOTHSIDE, delta)
			+ operator()(x) * mParam[0] * mpFunc->partialDerivative2(x, posi, posj, calctype, delta);
	}
}

/*!
    @brief Set check flag
	@param[in] checkflag check flag
*/
void
AQLExpMethod::setCheckFlag(bool checkflag)
{
	mCheckFlag = checkflag;
	mpFunc->setCheckFlag(checkflag);
}
