/*! @file
    @brief Source code of class of linear interpolation of curve path element
*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

//+++++ include +++++
#include "AQLRatesCurveHWInterpolation.h"
#include "AQLModelDynamicsHW1FCurve.h"

using namespace std;
//================ AQLRatesCurveHWInterpolation ===================================
/*!
	@brief default constructor
	
	@param[in] type sde integral type
*/

AQLRatesCurveHWInterpolation::AQLRatesCurveHWInterpolation()
{
}

/*!
	@brief copy constructor
*/
AQLRatesCurveHWInterpolation::AQLRatesCurveHWInterpolation(const AQLRatesCurveHWInterpolation& v) 
:
AQLRatesPEInterpolationBase(v),
mValue(v.mValue)
{
}

/*!
	@brief destructor
*/
AQLRatesCurveHWInterpolation::~AQLRatesCurveHWInterpolation() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLRatesCurveHWInterpolation::clone() const
		// g++ throw
{
    try 
	{
		return new AQLRatesCurveHWInterpolation(*this);
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
AQLRatesCurveHWInterpolation::isTypeOf(function_t id) const
{
	return (id==FN_CURVEHWINTERPOLATION ? true : AQLRatesPEInterpolationBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLRatesCurveHWInterpolation::getType() const
{
	return FN_CURVEHWINTERPOLATION;
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
AQLRatesCurveHWInterpolation::value( double t, double t1, double t2, 
                                 const AQLRatesPathElementBase& val1,
								 const AQLRatesPathElementBase& val2
                               )
{
	if (t1 > t2) return value(t, t2, t1, val2, val1);

	mValue.set_t(t);
	mValue.mpCurve1 = &dynamic_cast<const AQLRatesPathElementCurve&>(val1);
	mValue.mpCurve2 = &dynamic_cast<const AQLRatesPathElementCurve&>(val2);
	return mValue;
}


/*!
    @brief clear the cache
    @return 
*/
void
AQLRatesCurveHWInterpolation::init(void)
{
	map<double, AQLRatesPathElementBase*>::iterator itr;
	for(itr =  mValue.curve_cache.begin(); itr!= mValue.curve_cache.end();++itr)
	{
		delete itr->second ;
		itr->second = 0;
	}
	
	mValue.curve_cache.clear();
	mValue.is_curve_cache.clear();
	mValue.initialF_cache.clear();
	return;
}

//================ AQLRatesPathElementHWCurve ===================================

AQLRatesCurveHWInterpolation::AQLRatesCurveForHWInterpolation::AQLRatesCurveForHWInterpolation() 
: AQLRatesPathElementCurve(), is_cloned(false)
{
}

/*!
	@brief destructor
*/
AQLRatesCurveHWInterpolation::AQLRatesCurveForHWInterpolation::~AQLRatesCurveForHWInterpolation() 
{
    map<double, AQLRatesPathElementBase*>::iterator itr;
	for(itr = curve_cache.begin(); itr!= curve_cache.end();++itr)
	{
		delete itr->second ;
		itr->second = 0;
	}
}

/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLRatesCurveHWInterpolation::AQLRatesCurveForHWInterpolation::isTypeOf(pathelement_t id) const
{
	return (id == PE_CURVEFORHW ? true : AQLRatesPathElementCurve::isTypeOf(id));
}

/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
AQLRatesCurveHWInterpolation::AQLRatesCurveForHWInterpolation::getType() const
{
	return PE_CURVEFORHW;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLRatesPathElementBase*	
AQLRatesCurveHWInterpolation::AQLRatesCurveForHWInterpolation::clone() const
		// g++ throw
{
    try 
	{
		return new AQLRatesCurveForHWInterpolation(*this);
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
AQLRatesCurveHWInterpolation::AQLRatesCurveForHWInterpolation&
AQLRatesCurveHWInterpolation::AQLRatesCurveForHWInterpolation::operator = (const AQLRatesCurveHWInterpolation::AQLRatesCurveForHWInterpolation& a)
{
	// 
	if (this == &a) return *this;

	if (!a.isTypeOf(PE_CURVEFORHW)) 
	{	// 
		// 
		AQLString err = "Assignment error for AQLRatesCurveForHWInterpolation : from ";
		err += AQLString(a.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}

	// 	
	mpCurve1 = dynamic_cast<const AQLRatesCurveForHWInterpolation&>(a).mpCurve1;
	mpCurve2 = dynamic_cast<const AQLRatesCurveForHWInterpolation&>(a).mpCurve2;

	AQLRatesPathElementCurve::operator = (a);
	return *this;
}

/*!
    @brief get discount bond price
	@param[in] T maturity
    @return discount bond price
*/
double
AQLRatesCurveHWInterpolation::AQLRatesCurveForHWInterpolation::getP (double T) const
{
	double t1 = mpCurve1->get_t();
	double t2 = mpCurve2->get_t();

	if (t1 > m_t || t2 < m_t)
	{
		//error
		throw AQLCoreInvalidData("Condition of t1 <= t <= t2 is not maintain", __FILE__, __LINE__);
	}
	    
    if (T <= m_t) return 1.0;

	double ft1 = dynamic_cast<const AQLRatesPathElementHW1FCurve*>(mpCurve1)->initialF();
	double ft2 = dynamic_cast<const AQLRatesPathElementHW1FCurve*>(mpCurve2)->initialF();
   	double rt1= mpCurve1->get()[0] - ft1;
	double rt2 = mpCurve2->get()[0] - ft2;
	
	double r = rt1 + ( rt2 - rt1 ) * ( m_t - t1 ) / ( t2 - t1 );
 
	if(!is_curve_cache[m_t])	
    {
        curve_cache[m_t] = mpCurve1->clone();
        dynamic_cast<AQLRatesPathElementCurve*>(curve_cache[m_t])->set_t(m_t);        
		initialF_cache[m_t] = dynamic_cast<const AQLRatesPathElementHW1FCurve*>(mpCurve1)->initialF(m_t);
		is_curve_cache[m_t] = true;
    }
	curve_cache[m_t]->set(r + initialF_cache[m_t]);
	return dynamic_cast<AQLRatesPathElementCurve*>(curve_cache[m_t])->getP(T);

}



