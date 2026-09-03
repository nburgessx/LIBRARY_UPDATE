////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LADigitalMethod.cpp
//
//  SYNOPSIS    :       LADigitalMethod
//  DESCRIPTION :       Source code of class to represent digital function
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


#include "LADigitalFunc.h"
#include <algorithm>

using namespace std;
//================ LADigitalMethod ===================================
/*!
	@brief default constructor
*/
LADigitalMethod::LADigitalMethod() 
: LAFunctionBase()
{

}

/*!
	@brief destructor
*/
LADigitalMethod::~LADigitalMethod() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LADigitalMethod::clone() const
{
    try 
	{
		return new LADigitalMethod(*this);
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
LADigitalMethod::isTypeOf(function_t id) const
{
	return (id == FN_DIGITAL ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LADigitalMethod::getType() const
{
	return FN_DIGITAL;
}

/*!
    @brief Return function value
	@param[in] x index
    @return x[i] * mParam[i] + mParam[i + x.size() - 1] if mParam[i + 2 * x.size() - 3] < mParam[0] * x[0] <= mParam[i + 2 * x.size() - 2]
*/
double
LADigitalMethod::operator()(const DoubleArray& x) const
{
	if(mParam.size() != 3 * x.size() - 3)
		throw LACoreInvalidData("parameter size should be equal to 3 * index size - 3.",__FILE__,__LINE__);

	// check input if mParam[x.size()] < mParam[x.size() + 1] < ... or not
	for(size_t i = 2 * x.size() + 1; i < mParam.size(); ++i)
	{
		if(mParam[i - 1] > mParam[i])
			throw LACoreInvalidData("trigger strike should be in order",__FILE__,__LINE__);
	}

	double referenceValue = mParam[0] * x[0];
	for(size_t i = 1; i < x.size() - 1; ++i)
	{
		if( referenceValue < mParam[i + 2 * x.size() - 2] )
		{
			return x[i] * mParam[i] + mParam[i + x.size() - 1];
		}
	}

	return x[x.size() - 1] * mParam[x.size() - 1] + mParam[2 * x.size() - 2];
}