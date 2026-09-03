/*! @file
    @brief Source code of class to represent exponential function

	This class derives from LAFunctionBase

*/


////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAExpMethod.h
//
//  SYNOPSIS    :       LAExpMethod
//  DESCRIPTION :       Source code of class to represent exponential function
//						This class derives from LAFunctionBase
//                      
//  VERSION		:
////X///////////////////X///////////////////////////////X///////////////////

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAExpFunc.h"
#include "LABasic.h"
#include "LADist.h"

using namespace std;
//================ LAExpMethod ===================================
/*!
	@brief constructor to make exp(a*method)
	@param[in] a coefficient
	@param[in] method power function
*/
LAExpMethod::LAExpMethod(double a, const LAFunctionBase& method) 
: LAFunctionBase(method), mpFunc(NULL)
{
	DoubleArray param(1);
	param[0] = a;
	setParam(param);
	mpFunc = dynamic_cast<LAFunctionBase*>(method.clone());
//	mDiscontPoints = method.getDiscontPoints();
}

/*!
	@brief destructor
*/
LAExpMethod::~LAExpMethod() 
{
	delete mpFunc;
}

/*!
	@brief copy constructor
*/
LAExpMethod::LAExpMethod(const LAExpMethod& v) 
: LAFunctionBase(v), mpFunc(NULL)
{
	if (v.mpFunc) mpFunc = dynamic_cast<LAFunctionBase*>(v.mpFunc->clone());
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAExpMethod::clone() const
{
    try 
	{
		return new LAExpMethod(*this);
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
LAExpMethod::isTypeOf(function_t id) const
{
	return (id == FN_EXPFUNC ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAExpMethod::getType() const
{
	return FN_EXPFUNC;
}

/*!
    @brief Return function value
	@param[in] x: point
    @return function value
*/
double
LAExpMethod::operator()(const DoubleArray& x) const
{
//	if (mCheckFlag && !isInDomain(x))
//	{
//		throw LACoreNumericalError("Not in Domain" , __FILE__, __LINE__);
//	}
	return LAMath::exp(mParam[0]*mpFunc->operator()(x));
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
LAExpMethod::partialDerivative(const DoubleArray& x,unsigned int pos,
								CALC_TYPE calctype, DIFF_TYPE difftype,double delta) const
{
	if (calctype == NUMERICAL)
		return LAFunctionBase::partialDerivative(x,pos,calctype,difftype,delta);
	else
	{
	//	if (mCheckFlag && !isDifferentiable(x,pos))
	//	{
	//		throw LACoreNumericalError("Not Derivable" , __FILE__, __LINE__);
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
LAExpMethod::partialDerivative2(const DoubleArray& x, unsigned int posi,unsigned int posj,
												CALC_TYPE calctype, double delta) const
{
	if (calctype == NUMERICAL)
		return LAFunctionBase::partialDerivative2(x, posi, posj, calctype, delta);
	else
	{
	//	if (mCheckFlag && !isDifferentiable(x,posi,posj))
	//	{
	//		throw LACoreNumericalError("Not Derivable" , __FILE__, __LINE__);
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
LAExpMethod::setCheckFlag(bool checkflag)
{
	mCheckFlag = checkflag;
	mpFunc->setCheckFlag(checkflag);
}
