/*! @file
    @brief Source code of class of linear interpolation of curve path element


*/
//  2007, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLRatesCurveLinearInterpolation.cpp
//
//  SYNOPSIS    :       AQLRatesCurveLinearInterpolation
//  DESCRIPTION :       Source code of class of linear interpolation of curve path element
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


#include "AQLRatesCurveLinearInterpolation.h"


using namespace std;
//================ AQLRatesCurveLinearInterpolation ===================================
/*!
	@brief default constructor
	
	@param[in] type sde integral type
*/
AQLRatesCurveLinearInterpolation::AQLRatesCurveLinearInterpolation()
{

}
/*!
	@brief copy constructor
*/
AQLRatesCurveLinearInterpolation::AQLRatesCurveLinearInterpolation(const AQLRatesCurveLinearInterpolation& v) 
: AQLRatesPEInterpolationBase(v), mValue(v.mValue)
{

}

/*!
	@brief destructor
*/
AQLRatesCurveLinearInterpolation::~AQLRatesCurveLinearInterpolation() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLRatesCurveLinearInterpolation::clone() const
		//20070410--Nagase--g++ throw
{
    try 
	{
		return new AQLRatesCurveLinearInterpolation(*this);
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
AQLRatesCurveLinearInterpolation::isTypeOf(function_t id) const
{
	return (id==FN_CURVELINEARINTERPOLATION ? true : AQLRatesPEInterpolationBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLRatesCurveLinearInterpolation::getType() const
{
	return FN_CURVELINEARINTERPOLATION;
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
AQLRatesCurveLinearInterpolation::value(double t, double t1, double t2,
										const AQLRatesPathElementBase& val1,
										const AQLRatesPathElementBase& val2)
{
	if (t1 > t2) return value(t, t2, t1, val2, val1);

	mValue.set_t(t);
	mValue.mpCurve1 = &dynamic_cast<const AQLRatesPathElementCurve&>(val1);
	mValue.mpCurve2 = &dynamic_cast<const AQLRatesPathElementCurve&>(val2);
	return mValue;
}


//================ AQLRatesPathElementLMMCurve ===================================
/*!
	@brief default constructor
	@param[in] tenor tenor
	@param[in] t start time of this curve 

*/
AQLRatesCurveLinearInterpolation::AQLRatesCurveForLinearInterpolation::AQLRatesCurveForLinearInterpolation() 
: AQLRatesPathElementCurve()
{

}


/*!
	@brief destructor
*/
AQLRatesCurveLinearInterpolation::AQLRatesCurveForLinearInterpolation::~AQLRatesCurveForLinearInterpolation() 
{

}

/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLRatesCurveLinearInterpolation::AQLRatesCurveForLinearInterpolation::isTypeOf(pathelement_t id) const
{
	return (id == PE_CURVEFORLINEARINTER ? true : AQLRatesPathElementCurve::isTypeOf(id));
}

/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
AQLRatesCurveLinearInterpolation::AQLRatesCurveForLinearInterpolation::getType() const
{
	return PE_CURVEFORLINEARINTER;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLRatesPathElementBase*	
AQLRatesCurveLinearInterpolation::AQLRatesCurveForLinearInterpolation::clone() const
		//20070410--Nagase--g++ throw
{
    try 
	{
		return new AQLRatesCurveForLinearInterpolation(*this);
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
AQLRatesCurveLinearInterpolation::AQLRatesCurveForLinearInterpolation&
AQLRatesCurveLinearInterpolation::AQLRatesCurveForLinearInterpolation::operator = (const AQLRatesCurveLinearInterpolation::AQLRatesCurveForLinearInterpolation& a)
{
	// 
	if (this == &a) return *this;

	if (!a.isTypeOf(PE_CURVEFORLINEARINTER)) 
	{	// 
		// 
		AQLString err = "Assignment error for AQLRatesCurveForLinearInterpolation : from ";
		err += AQLString(a.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}

	// 	
	mpCurve1 = dynamic_cast<const AQLRatesCurveForLinearInterpolation&>(a).mpCurve1;
	mpCurve2 = dynamic_cast<const AQLRatesCurveForLinearInterpolation&>(a).mpCurve2;

	AQLRatesPathElementCurve::operator = (a);
	return *this;
}

/*!
    @brief get discount bond price
	@param[in] T maturity
    @return discount bond price
*/
double
AQLRatesCurveLinearInterpolation::AQLRatesCurveForLinearInterpolation::getP (double T) const
{
	double t1 = mpCurve1->get_t();
	double t2 = mpCurve2->get_t();
	if (t1 > m_t || t2 < m_t)
	{
		//error
		throw AQLCoreInvalidData("Condition of t1 <= t <= t2 is not maintain", __FILE__, __LINE__);
	}
	if (T <= m_t) return 1.0;
	else if (T <= t2)
		return ((m_t - t1) + mpCurve1->getP(T) * (T - m_t)) / (T - t1);
	else
		return 	(mpCurve2->getP(T) * (m_t - t1) + mpCurve1->getP(T) * (t2 - m_t)) / (t2 - t1);
}


