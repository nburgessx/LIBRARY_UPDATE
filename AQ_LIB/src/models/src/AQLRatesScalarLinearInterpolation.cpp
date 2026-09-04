/*! @file
    @brief Source code of class of linear interpolation of scalar path element


*/
//  2007, AlgoQuantHub..

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLRatesScalarLinearInterpolation.h"


using namespace std;
//================ AQLRatesScalarLinearInterpolation ===================================
/*!
	@brief default constructor
*/
AQLRatesScalarLinearInterpolation::AQLRatesScalarLinearInterpolation()
{

}
/*!
	@brief copy constructor
*/
AQLRatesScalarLinearInterpolation::AQLRatesScalarLinearInterpolation(const AQLRatesScalarLinearInterpolation& v) 
: AQLRatesPEInterpolationBase(v), mValue(v.mValue)
{

}

/*!
	@brief destructor
*/
AQLRatesScalarLinearInterpolation::~AQLRatesScalarLinearInterpolation() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLRatesScalarLinearInterpolation::clone() const
{
    try 
	{
		return new AQLRatesScalarLinearInterpolation(*this);
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
AQLRatesScalarLinearInterpolation::isTypeOf(function_t id) const
{
	return (id==FN_SCALARLINEARINTERPOLATION ? true : AQLRatesPEInterpolationBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLRatesScalarLinearInterpolation::getType() const
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
const AQLRatesPathElementBase&
AQLRatesScalarLinearInterpolation::value(double t,	double t1, double t2,
										const AQLRatesPathElementBase& val1,
										const AQLRatesPathElementBase& val2)
{
//	SCALAR v1 = *reinterpret_cast<const SCALAR*>(dynamic_cast<const AQLRatesPathElementScalar&>(val1).get());
//	SCALAR v2 = *reinterpret_cast<const SCALAR*>(dynamic_cast<const AQLRatesPathElementScalar&>(val2).get());
	SCALAR v1 = dynamic_cast<const AQLRatesPathElementScalar&>(val1).get()[0];
	SCALAR v2 = dynamic_cast<const AQLRatesPathElementScalar&>(val2).get()[0];
	mValue = (v2 * (t - t1) + v1 * (t2 - t)) / (t2 - t1);
	return mValue;
}
