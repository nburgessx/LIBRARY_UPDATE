/*! @file
    @brief Source code of of path element class that represents curve object for HW
*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif



#include "AQLModelDynamicsHW1FCurve.h"
#include "AQLAlgorithm.h"
#include <cmath>

using namespace std;
const double INFINITESIMAL = 1E-7;

//
//------------------------------ AQLRatesPathElementHW1FCurve ------------------------------
//

/*!
	@brief default constructor
	@param[in] tenor tenor
	@param[in] delta_tenor delta tenor
	@param[in] t start time of this curve 

*/
AQLRatesPathElementHW1FCurve::AQLRatesPathElementHW1FCurve( double t,double a, double s)
: 
AQLRatesPathElementAFFCurve(t), 
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
		throw AQLCoreInvalidData("Wrong a or s : YieldCurve_HW1F::YieldCurve_HW1F", __FILE__, __LINE__);
    }

	E_cache = new map<double, double>;
    A_cache = new map<double, double>;
    B_cache = new map<double, double>;
    varp_cache = new map<double, double>;
}

/*!
	@brief destructor
*/
AQLRatesPathElementHW1FCurve::~AQLRatesPathElementHW1FCurve() 
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
AQLRatesPathElementHW1FCurve::AQLRatesPathElementHW1FCurve(const AQLRatesPathElementHW1FCurve& v)
: 
AQLRatesPathElementAFFCurve(v), 
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
  mpInitialCurve = v.mpInitialCurve != 0 ? dynamic_cast<AQLRatesPathElementCurve*>(v.mpInitialCurve->clone()) : 0;
}

/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLRatesPathElementHW1FCurve::isTypeOf(pathelement_t id) const
{
	return (id == PE_HWCURVE ? true : AQLRatesPathElementAFFCurve::isTypeOf(id));
}

/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
AQLRatesPathElementHW1FCurve::getType() const
{
	return PE_HWCURVE;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLRatesPathElementBase*	
AQLRatesPathElementHW1FCurve::clone() const 
{
    try 
	{
		return new AQLRatesPathElementHW1FCurve(*this);
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
AQLRatesPathElementHW1FCurve&
AQLRatesPathElementHW1FCurve::operator = (const AQLRatesPathElementHW1FCurve& rhs)
{
	// 
	if (this == &rhs) return *this;

	// 
	//clear();
    AQLRatesPathElementAFFCurve::operator = (rhs);
	mpInitialCurve= rhs.mpInitialCurve != 0 ? dynamic_cast<AQLRatesPathElementCurve*>(rhs.mpInitialCurve->clone()) : 0;
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
AQLRatesPathElementHW1FCurve::set(const AQLRatesPathElementBase& a)
{
	const AQLMathPathYieldCurve& curve = dynamic_cast<const AQLMathPathYieldCurve &>(a);
	mpInitialCurve = dynamic_cast<AQLMathPathYieldCurve*>(curve.clone());
	mValue[0]  = dynamic_cast<const AQLMathPathYieldCurve*>(mpInitialCurve)->getForward(0.0); 
	mInitialF = mValue[0];
}
/*!
    @brief set start time of this curve
	@param[in] t start time
*/	
void
AQLRatesPathElementHW1FCurve::set_t (double t) 
{
	AQLRatesPathElementCurve::set_t(t);
	if (mpInitialCurve != 0)
		mInitialF = dynamic_cast<const AQLMathPathYieldCurve*>(mpInitialCurve)->getForward(t); 
}
/*!
    @brief clear data
	@param[in] curve curve object
*/	
void
AQLRatesPathElementHW1FCurve::clear()
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
double AQLRatesPathElementHW1FCurve::E( double T ) const
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
AQLRatesPathElementHW1FCurve::A( double T ) const
{
    if((*A_cache)[T]==0.0)
	{
		double x = B(T);
		double x2= dynamic_cast<const AQLMathPathYieldCurve*>(mpInitialCurve)->getForward(m_t);
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
AQLRatesPathElementHW1FCurve::B(double T) const
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
AQLRatesPathElementHW1FCurve::varp(double T) const
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
double AQLRatesPathElementHW1FCurve::initialF(double T) const
{
	return dynamic_cast<const AQLMathPathYieldCurve*>(mpInitialCurve)->getForward(T);
}


//
//------------------------------ AQLRatesPathElementHW1FCurveTMDPT ------------------------------
//
/*!
	@brief default constructor
	@param[in] t start time of this curve 

*/
AQLRatesPathElementHW1FCurveTMDPT::AQLRatesPathElementHW1FCurveTMDPT( double t)
:
//AQLRatesPathElementHW1FCurve(t, (&a_)->get_a(t), (&s_)->operator ()(t)),
AQLRatesPathElementHW1FCurve(t, 0.000001,0.0001),
is_cloned( false ),
mGL(HWGAUSSLEGENDRENUM),
mpHWtoolMR(0),
mpHWtoolVar(0)
{
	/*mpHWtoolMR = new AQLMathHWFuncToolForMR(a_,s_);
	mpHWtoolVar = new AQLMathHWFuncToolForVar(a_,s_);*/
}

/*!
	@brief constructor
	@param[in] t start time of this curve
	@param[in] r0 initialrate
	@param[in] funcHWMR AQLMathHWFuncMR
	@param[in] funcHWSigma AQLMathHWFuncSigma
	@param[in] initCurve AQLRatesPathElementCurve

*/
AQLRatesPathElementHW1FCurveTMDPT::AQLRatesPathElementHW1FCurveTMDPT(double t, double r0, AQLMathHWFuncMR& funcHWMR, AQLMathHWFuncSigma& funcHWSigma, 
															 AQLRatesPathElementCurve& initCurve)
:
//AQLRatesPathElementHW1FCurve(t, (&a_)->get_a(t), (&s_)->operator ()(t)),
AQLRatesPathElementHW1FCurve(t, 0.000001,0.0001),
is_cloned( true ),
mGL(HWGAUSSLEGENDRENUM)
{
	mpHWtoolMR = new AQLMathHWFuncToolForMR(funcHWMR,funcHWSigma);
	mpHWtoolVar = new AQLMathHWFuncToolForVar(*dynamic_cast<AQLMathHWFuncMR*>(funcHWMR.clone()),*dynamic_cast<AQLMathHWFuncSigma*>(funcHWSigma.clone()));
	mValue[0] = r0;
	mpInitialCurve = &initCurve;
}

/*!
	@brief destructor
*/
AQLRatesPathElementHW1FCurveTMDPT::~AQLRatesPathElementHW1FCurveTMDPT() 
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
AQLRatesPathElementHW1FCurveTMDPT::AQLRatesPathElementHW1FCurveTMDPT(const AQLRatesPathElementHW1FCurveTMDPT& rhs)
: 
AQLRatesPathElementHW1FCurve(rhs),
is_cloned( true ),
mGL(HWGAUSSLEGENDRENUM),
mpHWtoolMR(rhs.mpHWtoolMR !=0 ? dynamic_cast<AQLMathHWFuncToolForMR*>(rhs.mpHWtoolMR->clone()) : 0),
mpHWtoolVar(rhs.mpHWtoolVar !=0 ? dynamic_cast<AQLMathHWFuncToolForVar*>(rhs.mpHWtoolVar->clone()) : 0)
{
 
}

/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLRatesPathElementHW1FCurveTMDPT::isTypeOf(pathelement_t id) const
{
	return (id == PE_HWCURVETMDPT ? true : AQLRatesPathElementHW1FCurve::isTypeOf(id));
}

/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
AQLRatesPathElementHW1FCurveTMDPT::getType() const
{
	return PE_HWCURVETMDPT;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLRatesPathElementBase*	
AQLRatesPathElementHW1FCurveTMDPT::clone() const
{
    try 
	{
		return new AQLRatesPathElementHW1FCurveTMDPT(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

//
AQLRatesPathElementHW1FCurveTMDPT&
AQLRatesPathElementHW1FCurveTMDPT::operator = (const AQLRatesPathElementHW1FCurveTMDPT& rhs)
{
    if (this == &rhs) return *this;

	AQLRatesPathElementHW1FCurveTMDPT::operator = (rhs);
    
	mpHWtoolMR = rhs.mpHWtoolMR !=0 ? dynamic_cast<AQLMathHWFuncToolForMR*>(rhs.mpHWtoolMR->clone()) : 0;
	mpHWtoolVar = rhs.mpHWtoolVar !=0 ? dynamic_cast<AQLMathHWFuncToolForVar*>(rhs.mpHWtoolVar->clone()) : 0;
	is_cloned = true;
	return *this;
}

//
double 
AQLRatesPathElementHW1FCurveTMDPT::E( double T ) const
{
    if ( (*E_cache)[T] == 0.0 )
    {
        (*E_cache)[T] = mpHWtoolMR->ExpIntegralMR(T);
    }
    return (*E_cache)[T];
}

// A function
double 
AQLRatesPathElementHW1FCurveTMDPT::A(double T) const
{
    if ( (*A_cache)[T] == 0.0 )
    {
        (*A_cache)[T] = AQLRatesPathElementHW1FCurve::A(T);
    }
    return (*A_cache)[T];
}

//
double 
AQLRatesPathElementHW1FCurveTMDPT::B( double T ) const
{
    if ( (*B_cache)[T] == 0.0 )
    {	
		(*B_cache)[T] = mpHWtoolMR->ExpIntegralMR(m_t) * mGL.integrate((*mpHWtoolMR),m_t,T);
    }
    return (*B_cache)[T];
}

//
double 
AQLRatesPathElementHW1FCurveTMDPT::varp( double T ) const
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