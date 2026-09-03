/*! @file
    @brief Source code of of path element class that represents curve object for HW


*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif



#include "LAModelDynamicsHW1FCurve.h"
#include "LAAlgorithm.h"
#include <cmath>

using namespace std;
const double INFINITESIMAL = 1E-7;

//
//------------------------------ LARatesPathElementHW1FCurve ------------------------------
//

/*!
	@brief default constructor
	@param[in] tenor tenor
	@param[in] delta_tenor delta tenor
	@param[in] t start time of this curve 

*/
LARatesPathElementHW1FCurve::LARatesPathElementHW1FCurve( double t,double a, double s)
: 
LARatesPathElementAFFCurve(t), 
mMR(a),
mSigma(s),
s_SQa(0.5 * s * s / a),
delete_cache( true ),
mpInitialCurve(0),
is_cloned(false),
mInitialF(0)
{
    if ( a < 0 || s < 0 )
    {
		throw LACoreInvalidData("Wrong a or s : YieldCurve_HW1F::YieldCurve_HW1F", __FILE__, __LINE__);
    }

	E_cache = new map<double, double>;
    A_cache = new map<double, double>;
    B_cache = new map<double, double>;
    varp_cache = new map<double, double>;
}

/*!
	@brief destructor
*/
LARatesPathElementHW1FCurve::~LARatesPathElementHW1FCurve() 
{
	//clear();
    if(is_cloned)
    {
		delete mpInitialCurve;
		mpInitialCurve = 0;
    }
	
	if ( delete_cache )
    {
        delete E_cache;
        delete A_cache;
        delete B_cache;
        delete varp_cache;
    }
	mValue.clear();
}

/*!
	@brief copy constructor
	@param[in] v copy source
*/
LARatesPathElementHW1FCurve::LARatesPathElementHW1FCurve(const LARatesPathElementHW1FCurve& v)
: 
LARatesPathElementAFFCurve(v), 
mMR(v.mMR), 
mSigma(v.mSigma),
s_SQa(0.5 * mSigma * mSigma / mMR),
E_cache( new map<double, double>() ),
A_cache( new map<double, double>() ),
B_cache( new map<double, double>() ),
varp_cache( new map<double, double>() ),
delete_cache( true ),
is_cloned(true),
mInitialF(v.mInitialF)
{
  mpInitialCurve = v.mpInitialCurve != 0 ? dynamic_cast<LARatesPathElementCurve*>(v.mpInitialCurve->clone()) : 0;
}

/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LARatesPathElementHW1FCurve::isTypeOf(pathelement_t id) const
{
	return (id == PE_HWCURVE ? true : LARatesPathElementAFFCurve::isTypeOf(id));
}

/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
LARatesPathElementHW1FCurve::getType() const
{
	return PE_HWCURVE;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LARatesPathElementBase*	
LARatesPathElementHW1FCurve::clone() const 
{
    try 
	{
		return new LARatesPathElementHW1FCurve(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief equal operator
    @param[in] a source object

    @return this object
*/
LARatesPathElementHW1FCurve&
LARatesPathElementHW1FCurve::operator = (const LARatesPathElementHW1FCurve& rhs)
{
	// 
	if (this == &rhs) return *this;

	// 
	//clear();
    LARatesPathElementAFFCurve::operator = (rhs);
	mpInitialCurve= rhs.mpInitialCurve != 0 ? dynamic_cast<LARatesPathElementCurve*>(rhs.mpInitialCurve->clone()) : 0;
	mMR = rhs.mMR;
    mSigma = rhs.mSigma;
    s_SQa = rhs.s_SQa;
    is_cloned = true;

	E_cache = new map<double, double>( rhs.E_cache->begin(), rhs.E_cache->end() );
    A_cache = new map<double, double>( rhs.A_cache->begin(), rhs.A_cache->end() );
    B_cache = new map<double, double>( rhs.B_cache->begin(), rhs.B_cache->end() );
    varp_cache = new map<double, double>( rhs.varp_cache->begin(), rhs.varp_cache->end() );
	delete_cache = true;

	return *this;
}

/*!
    @brief set value
	@param[in] curve curve object
*/	
void
LARatesPathElementHW1FCurve::set(const LARatesPathElementBase& a)
{
	const LAMathPathYieldCurve& curve = dynamic_cast<const LAMathPathYieldCurve &>(a);
	mpInitialCurve = dynamic_cast<LAMathPathYieldCurve*>(curve.clone());
	mValue[0]  = dynamic_cast<const LAMathPathYieldCurve*>(mpInitialCurve)->getForward(0.0); 
	mInitialF = mValue[0];
}
/*!
    @brief set start time of this curve
	@param[in] t start time
*/	
void
LARatesPathElementHW1FCurve::set_t (double t) 
{
	LARatesPathElementCurve::set_t(t);
	if (mpInitialCurve != 0)
		mInitialF = dynamic_cast<const LAMathPathYieldCurve*>(mpInitialCurve)->getForward(t); 
}
/*!
    @brief clear data
	@param[in] curve curve object
*/	
void
LARatesPathElementHW1FCurve::clear()
{
	//if ((*mpRefCount)-- == 1)
    if (is_cloned)
	{
		delete mpInitialCurve;
		mpInitialCurve = 0;
	}
	mValue.clear();
}

/*!
    @brief E function
	@param[in] T
*/
double LARatesPathElementHW1FCurve::E( double T ) const
{
    if((*E_cache)[T] == 0.0)
	{
		(*E_cache)[T] = exp(mMR*T);
	}
	return (*E_cache)[T];
}

/*!
    @brief A function
	@param[in] T
*/
double 
LARatesPathElementHW1FCurve::A( double T ) const
{
    if((*A_cache)[T]==0.0)
	{
		double x = B(T);
		double x2= dynamic_cast<const LAMathPathYieldCurve*>(mpInitialCurve)->getForward(m_t);
		double x3 = varp(T);
		double y1 = mpInitialCurve->getP(T);
		double y2 = mpInitialCurve->getP(m_t);
		(*A_cache)[T] = log(y1 /y2) + x * x2 - 0.5 * x3 ;
	}
	return (*A_cache)[T];
}

/*!
    @brief B function
	@param[in] T
*/
double 
LARatesPathElementHW1FCurve::B(double T) const
{
	if ( (*B_cache)[T] == 0.0 )
	{
		(*B_cache)[T] = (1.0 - exp(-mMR * (T - m_t))) / mMR;
	}
	return (*B_cache)[T];

}

/*!
    @brief varp function
	@param[in] T
*/

double 
LARatesPathElementHW1FCurve::varp(double T) const
{
    if ( (*varp_cache)[T] == 0.0 )
    {
		double B_ = B(T);
		double B_SQ = B_ * B_;
		(*varp_cache)[T] =  s_SQa * (1.0 - exp(- 2.0 * mMR * (T-m_t) )) * B_SQ;
    }
    return (*varp_cache)[T];
}
/*!
    @brief get initial forward rate
	@param[in] T forward time
	@return initial forward time
*/
double LARatesPathElementHW1FCurve::initialF(double T) const
{
	return dynamic_cast<const LAMathPathYieldCurve*>(mpInitialCurve)->getForward(T);
}


//
//------------------------------ LARatesPathElementHW1FCurveTMDPT ------------------------------
//
/*!
	@brief default constructor
	@param[in] t start time of this curve 

*/
LARatesPathElementHW1FCurveTMDPT::LARatesPathElementHW1FCurveTMDPT( double t)
:
//LARatesPathElementHW1FCurve(t, (&a_)->get_a(t), (&s_)->operator ()(t)),
LARatesPathElementHW1FCurve(t, 0.000001,0.0001),
is_cloned( false ),
mGL(HWGAUSSLEGENDRENUM),
mpHWtoolMR(0),
mpHWtoolVar(0)
{
	/*mpHWtoolMR = new LAMathHWFuncToolForMR(a_,s_);
	mpHWtoolVar = new LAMathHWFuncToolForVar(a_,s_);*/
}

/*!
	@brief constructor
	@param[in] t start time of this curve
	@param[in] r0 initialrate
	@param[in] funcHWMR LAMathHWFuncMR
	@param[in] funcHWSigma LAMathHWFuncSigma
	@param[in] initCurve LARatesPathElementCurve

*/
LARatesPathElementHW1FCurveTMDPT::LARatesPathElementHW1FCurveTMDPT(double t, double r0, LAMathHWFuncMR& funcHWMR, LAMathHWFuncSigma& funcHWSigma, 
															 LARatesPathElementCurve& initCurve)
:
//LARatesPathElementHW1FCurve(t, (&a_)->get_a(t), (&s_)->operator ()(t)),
LARatesPathElementHW1FCurve(t, 0.000001,0.0001),
is_cloned( true ),
mGL(HWGAUSSLEGENDRENUM)
{
	mpHWtoolMR = new LAMathHWFuncToolForMR(funcHWMR,funcHWSigma);
	mpHWtoolVar = new LAMathHWFuncToolForVar(*dynamic_cast<LAMathHWFuncMR*>(funcHWMR.clone()),*dynamic_cast<LAMathHWFuncSigma*>(funcHWSigma.clone()));
	mValue[0] = r0;
	mpInitialCurve = &initCurve;
}

/*!
	@brief destructor
*/
LARatesPathElementHW1FCurveTMDPT::~LARatesPathElementHW1FCurveTMDPT() 
{
    if ( is_cloned )
    {
		delete mpHWtoolMR;
		delete mpHWtoolVar;
    }
}
/*!
	@brief copy constructor
	@param[in] v copy source
*/
LARatesPathElementHW1FCurveTMDPT::LARatesPathElementHW1FCurveTMDPT(const LARatesPathElementHW1FCurveTMDPT& rhs)
: 
LARatesPathElementHW1FCurve(rhs),
is_cloned( true ),
mGL(HWGAUSSLEGENDRENUM),
mpHWtoolMR(rhs.mpHWtoolMR !=0 ? dynamic_cast<LAMathHWFuncToolForMR*>(rhs.mpHWtoolMR->clone()) : 0),
mpHWtoolVar(rhs.mpHWtoolVar !=0 ? dynamic_cast<LAMathHWFuncToolForVar*>(rhs.mpHWtoolVar->clone()) : 0)
{
 
}

/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LARatesPathElementHW1FCurveTMDPT::isTypeOf(pathelement_t id) const
{
	return (id == PE_HWCURVETMDPT ? true : LARatesPathElementHW1FCurve::isTypeOf(id));
}

/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
LARatesPathElementHW1FCurveTMDPT::getType() const
{
	return PE_HWCURVETMDPT;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LARatesPathElementBase*	
LARatesPathElementHW1FCurveTMDPT::clone() const
{
    try 
	{
		return new LARatesPathElementHW1FCurveTMDPT(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

//
LARatesPathElementHW1FCurveTMDPT&
LARatesPathElementHW1FCurveTMDPT::operator = (const LARatesPathElementHW1FCurveTMDPT& rhs)
{
    if (this == &rhs) return *this;

	LARatesPathElementHW1FCurveTMDPT::operator = (rhs);
    
	mpHWtoolMR = rhs.mpHWtoolMR !=0 ? dynamic_cast<LAMathHWFuncToolForMR*>(rhs.mpHWtoolMR->clone()) : 0;
	mpHWtoolVar = rhs.mpHWtoolVar !=0 ? dynamic_cast<LAMathHWFuncToolForVar*>(rhs.mpHWtoolVar->clone()) : 0;
	is_cloned = true;
	return *this;
}

//
double 
LARatesPathElementHW1FCurveTMDPT::E( double T ) const
{
    if ( (*E_cache)[T] == 0.0 )
    {
        (*E_cache)[T] = mpHWtoolMR->ExpIntegralMR(T);
    }
    return (*E_cache)[T];
}

// A function
double 
LARatesPathElementHW1FCurveTMDPT::A(double T) const
{
    if ( (*A_cache)[T] == 0.0 )
    {
        (*A_cache)[T] = LARatesPathElementHW1FCurve::A(T);
    }
    return (*A_cache)[T];
}

//
double 
LARatesPathElementHW1FCurveTMDPT::B( double T ) const
{
    if ( (*B_cache)[T] == 0.0 )
    {	
		(*B_cache)[T] = mpHWtoolMR->ExpIntegralMR(m_t) * mGL.integrate((*mpHWtoolMR),m_t,T);
    }
    return (*B_cache)[T];
}

//
double 
LARatesPathElementHW1FCurveTMDPT::varp( double T ) const
{
    if ( (*varp_cache)[T] == 0.0 )
    {
		double tmp = mGL.integrate((*mpHWtoolMR),m_t,T);
		//double tmp2 = mpHWtoolMR->ExpIntegralInvMR(m_t);
		//(*varp_cache)[T] = tmp * tmp * tmp2 * tmp2 * mGL.integrate((*mpHWtoolVar),0.0,m_t);
		(*varp_cache)[T] = tmp * tmp * mGL.integrate((*mpHWtoolVar),0.0,m_t);

    }
    return (*varp_cache)[T];
}