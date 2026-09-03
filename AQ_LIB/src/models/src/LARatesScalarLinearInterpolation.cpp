/*! @file
    @brief Source code of class of linear interpolation of scalar path element


*/
//  2007, Mizuho International London..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARatesScalarLinearInterpolation.cpp
//
//  SYNOPSIS    :       LARatesScalarLinearInterpolation
//  DESCRIPTION :       Source code of class of linear interpolation of scalar path element
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


#include "LARatesScalarLinearInterpolation.h"


using namespace std;
//================ LARatesScalarLinearInterpolation ===================================
/*!
	@brief default constructor
*/
LARatesScalarLinearInterpolation::LARatesScalarLinearInterpolation()
{

}
/*!
	@brief copy constructor
*/
LARatesScalarLinearInterpolation::LARatesScalarLinearInterpolation(const LARatesScalarLinearInterpolation& v) 
: LARatesPEInterpolationBase(v), mValue(v.mValue)
{

}

/*!
	@brief destructor
*/
LARatesScalarLinearInterpolation::~LARatesScalarLinearInterpolation() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LARatesScalarLinearInterpolation::clone() const
{
    try 
	{
		return new LARatesScalarLinearInterpolation(*this);
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
LARatesScalarLinearInterpolation::isTypeOf(function_t id) const
{
	return (id==FN_SCALARLINEARINTERPOLATION ? true : LARatesPEInterpolationBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LARatesScalarLinearInterpolation::getType() const
{
	return FN_SCALARLINEARINTERPOLATION;
}
/*!
    @brief Return interpolated value
	@param[in] t point to get value
	@param[in] t1 point of left end
	@param[in] t2 point of right end
	@param[in] val1 value at t1
	@param[in] val2 value at t2

	@return interpolated value
*/
const LARatesPathElementBase&
LARatesScalarLinearInterpolation::value(double t,	double t1, double t2,
										const LARatesPathElementBase& val1,
										const LARatesPathElementBase& val2)
{
//	SCALAR v1 = *reinterpret_cast<const SCALAR*>(dynamic_cast<const LARatesPathElementScalar&>(val1).get());
//	SCALAR v2 = *reinterpret_cast<const SCALAR*>(dynamic_cast<const LARatesPathElementScalar&>(val2).get());
	SCALAR v1 = dynamic_cast<const LARatesPathElementScalar&>(val1).get()[0];
	SCALAR v2 = dynamic_cast<const LARatesPathElementScalar&>(val2).get()[0];
	mValue = (v2 * (t - t1) + v1 * (t2 - t)) / (t2 - t1);
	return mValue;
}
