/*! @file
    @brief Source code of class to represent min function

    This class derives from AQLFunctionBase
*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLMinFunc.h"
#include <algorithm>

using namespace std;
//================ AQLMinMethod ===================================
/*!
	@brief default constructor
*/
AQLMinMethod::AQLMinMethod() 
: AQLFunctionBase()
{

}

/*!
	@brief destructor
*/
AQLMinMethod::~AQLMinMethod() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLMinMethod::clone() const
{
    try 
	{
		return new AQLMinMethod(*this);
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
AQLMinMethod::isTypeOf(function_t id) const
{
	return (id == FN_MIN ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLMinMethod::getType() const
{
	return FN_MIN;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
AQLMinMethod::operator()(const DoubleArray& x) const
{
	DoubleArray::const_iterator it = min_element(x.begin(), x.end());
	return *it;
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
AQLMinMethod::partialDerivative(const DoubleArray& x, unsigned int pos,
								CALC_TYPE calctype, DIFF_TYPE difftype, double delta) const
{
	if (calctype == NUMERICAL)
		return AQLFunctionBase::partialDerivative(x, pos, calctype, difftype, delta);
	else
	{
		DoubleArray::const_iterator it = min_element(x.begin(), x.end());
		unsigned int i = static_cast<unsigned int>(it - x.begin());
		if (i != pos)
		{
			if (x.at(pos) > *it) return 0;
			//error
			throw AQLCoreInvalidData("not derivable", __FILE__, __LINE__);
		}
		if (x.size() == 1) return 1;

		double v = *it;
		DoubleArray tmp = x;
		tmp.erase(min_element(tmp.begin(), tmp.end()));
		it = min_element(tmp.begin(), tmp.end());
		if (v == *it)
		{
			//error
			throw AQLCoreInvalidData("not derivable", __FILE__, __LINE__);		
		}
		return 1;
	}

}

/*!
    @brief Return second partial derivative value
	@param[in] x integral point
	@param[in] posi one variable location to implement derivative
	@param[in] posj another variable location to implement derivative
	@param[in] calctype analytical or numerical integral
	@param[in] delta grid width for numerical method
    @return second partial deribative value
*/
double
AQLMinMethod::partialDerivative2(const DoubleArray& x, unsigned int posi, unsigned int posj,
												CALC_TYPE calctype, double delta) const
{
	if (calctype == NUMERICAL)
		return AQLFunctionBase::partialDerivative2(x, 0, 0, calctype, delta);
	else
	{
		//check exception occur or not
		partialDerivative(x, posi, calctype);
		partialDerivative(x, posj, calctype);
		
		return 0;
	}
}

/*!
    @brief Return derivable or not
	@param[in] x point
	@param[in] pos variable location to implement derivative
    @return derivable or not
*/
bool
AQLMinMethod::isDifferentiable(const DoubleArray& x, unsigned int pos) 
const
{
	DoubleArray::const_iterator it = min_element(x.begin(), x.end());
	unsigned int i = static_cast<unsigned int>(it - x.begin());
	if (i != pos)
	{
		if (x.at(pos) > *it) return true;
		//error
		return false;
	}

	if (x.size() == 1) return true;
		
	double v = *it;
	DoubleArray tmp = x;
	tmp.erase(min_element(tmp.begin(), tmp.end()));
	it = min_element(tmp.begin(), tmp.end());
	if (v == *it) return false;	
	return true;

}

/*!
    @brief Return derivable or not
	@param[in] x point
	@param[in] posi one variable location to implement derivative
	@param[in] posj another variable location to implement derivative
    @return derivable or not
*/	
bool
AQLMinMethod::isDifferentiable(const DoubleArray& x, unsigned int posi, unsigned int posj) 
const
{
	return isDifferentiable(x, posi) && isDifferentiable(x, posj); 
}
