/*! @file
    @brief Source code of of path element class that represents curve object


*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAModelDynamicsCurve.h"
#include "LAString.h"
#include "LABasic.h"

using namespace std;
//================ LARatesPathElementCurve ===================================
/*!
	@brief default constructor
	@param[in] t start time of this curve
*/
LARatesPathElementCurve::LARatesPathElementCurve(double t) 
: LARatesPathElementBase(), m_t(t)
{
	if (t < 0.0)
	{
		//error
		throw LACoreInvalidData("Can't set negative t" , __FILE__, __LINE__);
	}
}

/*!
	@brief copy constructor
	@param[in] v copy source
*/
LARatesPathElementCurve::LARatesPathElementCurve(const LARatesPathElementCurve& v)
: LARatesPathElementBase(v), m_t(v.m_t)
{	
}

/*!
	@brief destructor
*/
LARatesPathElementCurve::~LARatesPathElementCurve() 
{
	;
}

/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LARatesPathElementCurve::isTypeOf(pathelement_t id) const
{
	return (id == PE_CURVE ? true : LARatesPathElementBase::isTypeOf(id));
}

/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
LARatesPathElementCurve::getType() const
{
	return PE_CURVE;
}



/*!
    @brief equal operator
    @param[in] a source object

    @return this object
*/
LARatesPathElementCurve&
LARatesPathElementCurve::operator = (const LARatesPathElementCurve& a)
{
	// not copy when a == this
	if (this == &a) return *this;

/*	if (!a.isTypeOf(PE_CURVE)) 
	{	// error when type-check
		// exception
		LAString err = "Assignment error for LARatesPathElementCurve : from ";
		err += LAString(a.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}*/

	m_t = dynamic_cast<const LARatesPathElementCurve&>(a).m_t;
	return *this;
}
/*!
    @brief set value
    @param[in] a source object
*/
void
LARatesPathElementCurve::set(const LARatesPathElementBase& a)
{
	if (!a.isTypeOf(PE_CURVE)) 
	{	
		// error when type-check
		// exception
		LAString err = "set error for LARatesPathElementCurve : from ";
		err += LAString(a.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	
//	m_t = dynamic_cast<const LARatesPathElementCurve&>(a).m_t;
	if (m_t < dynamic_cast<const LARatesPathElementCurve&>(a).m_t)
	{
		throw LACoreInvalidData("input curve start time must be same or before this curve start time", __FILE__, __LINE__);
	}

}

/*!
    @brief get derivative of discount bond price
    @param[in] T maturity
    @param[in] dt difference for numerical method
    @return derivative of discount bond price
*/
double
LARatesPathElementCurve::getP_D (double T, double dt) const
{
    if ( T < m_t ) return 0.0;
    if ( T == m_t ) return getP_D(T + 0.0001, dt);


    double dt0 = 0.5 * dt < T - m_t ? 0.5 * dt : T - m_t;
    double dt1 = 0.5 * dt;

    const double eps = 0.0001;
    double eps0 = dt0 * eps;
    double eps1 = dt1 * eps;

    return ( (getP(T + dt1) - getP(T + eps1)) / (dt1 - eps1)
           + (getP(T - eps0) - getP(T - dt0)) / (dt0 - eps0) ) * 0.5;
}

/*!
    @brief get 2nd derivative of discount bond price
    @param[in] T maturity
    @param[in] dt difference for numerical method
    @return 2nd derivative of discount bond price
*/
double
LARatesPathElementCurve::getP_D2 (double T, double dt) const
{
    if ( T < m_t ) return 0.0;
    if ( T == m_t ) return getP_D2(T + 0.0001, dt);

    double dt0 = dt < T - m_t ? dt : T - m_t;
    double dt1 = dt;

    const double eps = 0.0001;
    double eps0 = dt0 * eps;
    double eps1 = dt1 * eps;

    double DT1 = 0.5 * (dt1 - eps1);
    double DT0 = 0.5 * (dt0 - eps0);
    return ( (getP(T + dt1) - 2.0 * getP(T + eps1 + DT1) + getP(T + eps1)) / (DT1 * DT1)
           + (getP(T - eps0) - 2.0 * getP(T - eps0 - DT0) + getP(T - dt0)) / (DT0 * DT0) ) * 0.5;
}


/*!
    @brief get zero rate
    @param[in] T maturity
    @return zero rate
*/
double
LARatesPathElementCurve::getZero (double T) const
{
	if (m_t > T)
	{
		return 0.0;//throw LACoreInvalidData("maturity is before start", __FILE__, __LINE__);
	}
	if (m_t == T) return getZero(T + 0.0001);
	return -LAMath::log(getP(T)) / (T - m_t);
}

/*!
    @brief get forward spot rate
    @param[in] T forward time
    @param[in] dt difference for numerical method

	@return forward spot rate
*/
double
LARatesPathElementCurve::getF (double T, double dt) const
{
    if ( T < m_t || dt <= 0.0 ) return 0.0;
    if ( T == m_t ) return getF(T + 0.0001, dt);
    return -getP_D(T, dt) / getP(T);}


/*!
    @brief get derivative of forward spot rate
    @param[in] T forward time
    @param[in] dt difference for numerical method
    @return derivative of forward spot rate
*/
double
LARatesPathElementCurve::getF_D (double T, double dt) const
{
    if ( T < m_t || dt <= 0.0 ) return 0.0;
    if ( T == m_t ) return getF_D(T + 0.0001, dt);
    double temp = getF(T, dt);
    return temp * temp - getP_D2(T, dt) / getP(T);
}

/*!
    @brief set start time of this curve
	@param[in] t start time
*/	
void
LARatesPathElementCurve::set_t (double t) 
{
	if (t < 0.0)
	{
		//error
		throw LACoreInvalidData("Can't set negative t" , __FILE__, __LINE__);
	}
	m_t = t;
}
