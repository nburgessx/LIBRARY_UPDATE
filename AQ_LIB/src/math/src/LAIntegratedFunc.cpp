/*! @file
    @brief Source code of class to represent function integrated with respect to one parameter

	This class derives from LAFunctionBase

*/


////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAIntegrandMethod.h
//
//  SYNOPSIS    :       LAIntegrandMethod
//  DESCRIPTION :       Source code of class to represent function integrated
//						with respect to one parameter. This class derives from LAFunctionBase
//                      
//  VERSION		:
////X///////////////////X///////////////////////////////X///////////////////

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAIntegratedFunc.h"
#include "LA1DIntegral.h"

using namespace std;
//================ LAIntegrandMethod ===================================
/*!
	@brief constructor
	@param[in] type integrate region type (const,variable)(variable,const)(const,cosnt)
	@param[in] pos armument location to be integrated
	@param[in] param function paramter
	@param[in] method original function
	@param[in] pIntegral pointer to integral method
*/
LAIntegrandMethod::LAIntegrandMethod(IntegratedFuncType type,
									unsigned int pos,
									const DoubleArray& param,
									const LAFunctionBase& method,
									const LA1DIntegral* pIntegral)
: LAFunctionBase(method.isCheck()),mpFunc(NULL),mpIntegral(NULL),mType(type),mPos(pos)
{
	mpFunc = dynamic_cast<LAFunctionBase*>(method.clone());
	if (pIntegral) mpIntegral = dynamic_cast<LA1DIntegral*>(pIntegral->clone());
	mParam = param;
}

/*!
	@brief destructor
*/
LAIntegrandMethod::~LAIntegrandMethod() 
{
	delete mpFunc;
	if (mpIntegral) delete mpIntegral;
}

/*!
	@brief copy constructor
*/
LAIntegrandMethod::LAIntegrandMethod(const LAIntegrandMethod& v) 
: LAFunctionBase(v), mpFunc(NULL), mpIntegral(NULL)
{
	if (v.mpFunc) mpFunc = dynamic_cast<LAFunctionBase*>(v.mpFunc->clone());
	if (v.mpIntegral)mpIntegral = dynamic_cast<LA1DIntegral*>(v.mpIntegral->clone());
	mType = v.mType;
	mPos = v.mPos;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAIntegrandMethod::clone() const
{
    try 
	{
		return new LAIntegrandMethod(*this);
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
LAIntegrandMethod::isTypeOf(function_t id) const
{
	return (id == FN_INTEGRATEDFUNC ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAIntegrandMethod::getType() const
{
	return FN_INTEGRATEDFUNC;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LAIntegrandMethod::operator()(const DoubleArray& x) const
{
	if (mCheckFlag && !isInDomain(x))
	{
		throw LACoreNumericalError("Not in Domain" , __FILE__, __LINE__);
	}

	if (mType == TYPE_C)
	{
		DoubleArray xx(x.size()+1);
		unsigned int i;
		for (i = 0; i < mPos; i++)
			xx[i] = x[i];
		for (i = mPos + 1; i < xx.size(); i++)
			xx[i] = x[i-1];	
		mpFunc->operator ()(mPos, xx);
		//
		return mpFunc->integral(mParam[0], mParam[1], mpIntegral);
	}
	
	mpFunc->operator ()(mPos, x);
	
	if (mType == TYPE_A)
		return mpFunc->integral(mParam[0], x[mPos], mpIntegral);
	else 
		return mpFunc->integral(x[mPos], mParam[0], mpIntegral);
	
		
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
LAIntegrandMethod::partialDerivative(const DoubleArray& x,unsigned int pos,
								CALC_TYPE calctype, DIFF_TYPE difftype,double delta) const
{
	if (calctype == NUMERICAL)
		return LAFunctionBase::partialDerivative(x, pos, calctype, difftype, delta);
	else
	{
		if (mCheckFlag && !isDifferentiable(x, pos))
		{
			throw LACoreNumericalError("Not Derivable" , __FILE__, __LINE__);
		}			
		
		if (pos == mPos && (mType == TYPE_A || mType == TYPE_B))
		{
			if (mType == TYPE_A) return mpFunc->operator ()(x);
			else 
				return -mpFunc->operator ()(x);
		}
		else if (calctype == DEFAULT)
			return LAFunctionBase::partialDerivative(x, pos, NUMERICAL, difftype, delta);
		else
			throw LACoreNumericalError("This function does not support this method", __FILE__, __LINE__);
	
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
LAIntegrandMethod::partialDerivative2(const DoubleArray& x, unsigned int posi,unsigned int posj,
												CALC_TYPE calctype, double delta) const
{
	if (calctype == NUMERICAL)
		return LAFunctionBase::partialDerivative2(x, posi, posj, calctype, delta);
	else
	{
		if (mCheckFlag && !isDifferentiable(x, posi, posj))
		{
			throw LACoreNumericalError("Not Derivable" , __FILE__, __LINE__);
		}			
		
		if (posi == mPos && (mType == TYPE_A || mType == TYPE_B))
		{
			if (mType == TYPE_A) return mpFunc->partialDerivative(x, posj);
			else 
				return -mpFunc->partialDerivative(x, posj);
		}
		else if (posj == mPos && (mType == TYPE_A || mType == TYPE_B))
		{
			if (mType == TYPE_A) return mpFunc->partialDerivative(x, posi);
			else 
				return -mpFunc->partialDerivative(x, posi);

		}
		else if (calctype == DEFAULT)
			return LAFunctionBase::partialDerivative2(x, posi, posj, NUMERICAL, delta);
		else
			throw LACoreNumericalError("This function does not support this method", __FILE__, __LINE__);
	}
}

/*!
    @brief Return derivable or not
	@param[in] x point
	@param[in] pos variable location to implement derivative
    @return derivable or not
*/
bool
LAIntegrandMethod::isDifferentiable(const DoubleArray& x, unsigned int pos)const
{
	if (pos == mPos && (mType == TYPE_A || mType == TYPE_B))
		return mpFunc->isInDomain(x);
	else
		return LAFunctionBase::isDifferentiable(x, pos);
}

/*!
    @brief Return derivable or not
	@param[in] x point
	@param[in] posi one variable location to implement derivative
	@param[in] posj another variable location to implement derivative
    @return derivable or not
*/	
bool
LAIntegrandMethod::isDifferentiable(const DoubleArray& x, unsigned int posi,unsigned int posj)const
{
	if (posi == mPos && (mType == TYPE_A || mType == TYPE_B))
		return mpFunc->isDifferentiable(x, posj);
	else if (posj == mPos && (mType == TYPE_A || mType == TYPE_B))
		return mpFunc->isDifferentiable(x, posi);
	else
		return LAFunctionBase::isDifferentiable(x, posi, posj);
}

/*!
    @brief Set check flag
	@param[in] checkflag check flag to be set
*/	
void
LAIntegrandMethod::setCheckFlag(bool checkflag)
{
	mCheckFlag=checkflag;
	mpFunc->setCheckFlag(checkflag);
}
