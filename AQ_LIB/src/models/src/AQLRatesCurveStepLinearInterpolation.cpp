/*! @file
    @brief Source code of class of linear interpolation of curve path element


*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLRatesCurveStepLinearInterpolation.h"


using namespace std;
//================ AQLRatesCurveLinearInterpolation ===================================
/*!
	@brief default constructor
	
	@param[in] type sde integral type
*/
AQLRatesCurveStepLinearInterpolation::AQLRatesCurveStepLinearInterpolation()
{

}
/*!
	@brief copy constructor
*/
AQLRatesCurveStepLinearInterpolation::AQLRatesCurveStepLinearInterpolation(const AQLRatesCurveStepLinearInterpolation& v) 
: AQLRatesPEInterpolationBase(v), mValue(v.mValue)
{

}

/*!
	@brief destructor
*/
AQLRatesCurveStepLinearInterpolation::~AQLRatesCurveStepLinearInterpolation() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLRatesCurveStepLinearInterpolation::clone() const
		//20070410--Nagase--g++ throw
{
    try 
	{
		return new AQLRatesCurveStepLinearInterpolation(*this);
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
AQLRatesCurveStepLinearInterpolation::isTypeOf(function_t id) const
{
	return (id==FN_CURVESTEPLINEARINTERPOLATION ? true : AQLRatesPEInterpolationBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLRatesCurveStepLinearInterpolation::getType() const
{
	return FN_CURVESTEPLINEARINTERPOLATION;
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
AQLRatesCurveStepLinearInterpolation::value(double t, double t1, double t2,
										const AQLRatesPathElementBase& val1,
										const AQLRatesPathElementBase& val2)
{
	if (t1 > t2) return value(t, t2, t1, val2, val1);

	mValue.set_t(t);
	mValue.mpCurve1 = &dynamic_cast<const AQLRatesPathElementCurve&>(val1);
	return mValue;
}


//================ AQLRatesPathElementLMMCurve ===================================
/*!
	@brief default constructor
	@param[in] tenor tenor
	@param[in] t start time of this curve 

*/
AQLRatesCurveStepLinearInterpolation::AQLRatesCurveForStepLinearInterpolation::AQLRatesCurveForStepLinearInterpolation() 
: AQLRatesPathElementCurve()
{

}


/*!
	@brief destructor
*/
AQLRatesCurveStepLinearInterpolation::AQLRatesCurveForStepLinearInterpolation::~AQLRatesCurveForStepLinearInterpolation() 
{

}

/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLRatesCurveStepLinearInterpolation::AQLRatesCurveForStepLinearInterpolation::isTypeOf(pathelement_t id) const
{
	return (id == PE_CURVEFORSTEPLINEARINTER ? true : AQLRatesPathElementCurve::isTypeOf(id));
}

/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
AQLRatesCurveStepLinearInterpolation::AQLRatesCurveForStepLinearInterpolation::getType() const
{
	return PE_CURVEFORSTEPLINEARINTER;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLRatesPathElementBase*	
AQLRatesCurveStepLinearInterpolation::AQLRatesCurveForStepLinearInterpolation::clone() const
		//20070410--Nagase--g++ throw
{
    try 
	{
		return new AQLRatesCurveForStepLinearInterpolation(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief equal operator
    @param[in] a source object

    @return this object
*/
AQLRatesCurveStepLinearInterpolation::AQLRatesCurveForStepLinearInterpolation&
AQLRatesCurveStepLinearInterpolation::AQLRatesCurveForStepLinearInterpolation::operator = (const AQLRatesCurveStepLinearInterpolation::AQLRatesCurveForStepLinearInterpolation& a)
{
	// 
	if (this == &a) return *this;

	if (!a.isTypeOf(PE_CURVEFORSTEPLINEARINTER)) 
	{	// 
		// 
		AQLString err = "Assignment error for AQLRatesCurveForStepLinearInterpolation : from ";
		err += AQLString(a.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}

	AQLRatesPathElementCurve::operator = (a);
	// 	
	mpCurve1 = dynamic_cast<const AQLRatesCurveForStepLinearInterpolation&>(a).mpCurve1;
	
	return *this;
}

/*!
    @brief get discount bond price
	@param[in] T maturity
    @return discount bond price
*/
double
AQLRatesCurveStepLinearInterpolation::AQLRatesCurveForStepLinearInterpolation::getP (double T) const
{
	return mpCurve1->getP(T);
}


