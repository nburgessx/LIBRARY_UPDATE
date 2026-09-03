/*! @file
    @brief Source code of displaced diffusion type of fx sde integral class



*/
//  2008, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAPriceFXDDIntegral.cpp
//
//  SYNOPSIS    :       LAPriceFXDDIntegral
//  DESCRIPTION :       Source code of displaced diffusion type of fx sde integral class
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


#include "LAPriceFXDDIntegral.h"
#include "LABasic.h"
#include "LAPriceDriftFX.h"
#include "LAMathVolFuncBase.h"
#include "LAMathVolFuncFXDD.h"
#include <float.h>

using namespace std;

static const double MAXIMUM_1804 = LAMath::log(DBL_MAX) - 1.0;
//================ LAPriceFXDDIntegral ===================================
/*!
	@brief default constructor

	@param[in] type sde integral type

*/
LAPriceFXDDIntegral::LAPriceFXDDIntegral()
: LARatesSDEIntegralBase(NORMAL_INTEGRAL)
{
	mVar.resize(3);
}
/*!
	@brief copy constructor
*/
LAPriceFXDDIntegral::LAPriceFXDDIntegral(const LAPriceFXDDIntegral& v) 
: LARatesSDEIntegralBase(v), mVar(v.mVar)
{

}

/*!
	@brief destructor
*/
LAPriceFXDDIntegral::~LAPriceFXDDIntegral() 
{

}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAPriceFXDDIntegral::clone() const	
{
    try 
	{
		return new LAPriceFXDDIntegral(*this);
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
LAPriceFXDDIntegral::isTypeOf(function_t id) const
{
	return (id==FN_FXDDINTEGRAL ? true : LARatesSDEIntegralBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAPriceFXDDIntegral::getType() const
{
	return FN_FXDDINTEGRAL;
}

/*!
    @brief excecute integral
    @param[in] ts starttime
    @param[in] te endtime
    @param[in] drift drift
    @param[in] vol volatility
    @param[in] bm brownian motion
    @param[in, out] x_in_out input and output 
    @param[in] varnum number of input(output)
*/
void
LAPriceFXDDIntegral::integral(double ts, double te, 
							vector<LAFunctionBase*>::const_iterator drift,										
							vector<vector<LAFunctionBase*> >::const_iterator vol,
							DoubleArray::const_iterator	bm,
							SCALARARRAY::iterator	x_in_out,
							unsigned int varnum
							) const
{
	(void)varnum;
	if (!(*drift)->isTypeOf(FN_DRIFTFX))
	{
		//error
		throw LACoreInvalidData("drift class must be LAPriceDriftFX!", __FILE__, __LINE__);
	}	
	const LAFunctionBase* volfunc;
	if ((*vol)[0]->isTypeOf(FN_VOLFUNCBASE))
	{
		volfunc = dynamic_cast<LAMathVolFuncBase*>((*vol)[0])->getVolatility();
	}
	else
	{
		volfunc = (*vol)[0];
	}
	if (!volfunc->isTypeOf(FN_VOLFUNCFXDD))
	{
		//error
		throw LACoreInvalidData("volatility class must be LAMathVolFuncFXDD!", __FILE__, __LINE__);
	}
	const LAMathVolFuncFXDD* volfuncdd =dynamic_cast<const LAMathVolFuncFXDD*>(volfunc);

	mVar[0] = ts;
	mVar[2] = te;
#ifdef __SCALAR_FLOAT__
	mVar[1] = static_cast<double>(*x_in_out);
#else
	mVar[1] = (*x_in_out);
#endif
	
	
	double v = volfuncdd->getIntegralofV(ts, te);
	double mt = -(*drift)->operator ()(mVar) * (te - ts) + 0.5 * v * v 
				- v / LAMath::sqrt(te - ts) * (*bm);
	double ret;
	if (mt > MAXIMUM_1804) ret = - 0.5 * volfuncdd->getIntegralofSVV(ts, te);
	else
	{
		mt = LAMath::exp(mt); 

		ret = mVar[1] - 0.5 * (1.0 + 1.0/*mt*/) * volfuncdd->getIntegralofSVV(ts, te)
				+ volfuncdd->getIntegralofSV(ts, te) / LAMath::sqrt(te - ts) * (*bm);
		ret /= mt;
	}
#ifdef __SCALAR_FLOAT__
	(*x_in_out) = static_cast<SCALAR>(ret);
#else
	(*x_in_out) = ret;
#endif


}

