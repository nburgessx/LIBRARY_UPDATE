/*! @file
    @brief Source code of displaced diffusion type of fx sde integral class



*/
//  2008, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLPriceFXDDIntegral.cpp
//
//  SYNOPSIS    :       AQLPriceFXDDIntegral
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


#include "AQLPriceFXDDIntegral.h"
#include "AQLBasic.h"
#include "AQLPriceDriftFX.h"
#include "AQLMathVolFuncBase.h"
#include "AQLMathVolFuncFXDD.h"
#include <float.h>

using namespace std;

static const double MAXIMUM_1804 = AQLMath::log(DBL_MAX) - 1.0;
//================ AQLPriceFXDDIntegral ===================================
/*!
	@brief default constructor

	@param[in] type sde integral type

*/
AQLPriceFXDDIntegral::AQLPriceFXDDIntegral()
: AQLRatesSDEIntegralBase(NORMAL_INTEGRAL)
{
	mVar.resize(3);
}
/*!
	@brief copy constructor
*/
AQLPriceFXDDIntegral::AQLPriceFXDDIntegral(const AQLPriceFXDDIntegral& v) 
: AQLRatesSDEIntegralBase(v), mVar(v.mVar)
{

}

/*!
	@brief destructor
*/
AQLPriceFXDDIntegral::~AQLPriceFXDDIntegral() 
{

}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLPriceFXDDIntegral::clone() const	
{
    try 
	{
		return new AQLPriceFXDDIntegral(*this);
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
AQLPriceFXDDIntegral::isTypeOf(function_t id) const
{
	return (id==FN_FXDDINTEGRAL ? true : AQLRatesSDEIntegralBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLPriceFXDDIntegral::getType() const
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
AQLPriceFXDDIntegral::integral(double ts, double te, 
							vector<AQLFunctionBase*>::const_iterator drift,										
							vector<vector<AQLFunctionBase*> >::const_iterator vol,
							DoubleArray::const_iterator	bm,
							SCALARARRAY::iterator	x_in_out,
							unsigned int varnum
							) const
{
	(void)varnum;
	if (!(*drift)->isTypeOf(FN_DRIFTFX))
	{
		//error
		throw AQLCoreInvalidData("drift class must be AQLPriceDriftFX!", __FILE__, __LINE__);
	}	
	const AQLFunctionBase* volfunc;
	if ((*vol)[0]->isTypeOf(FN_VOLFUNCBASE))
	{
		volfunc = dynamic_cast<AQLMathVolFuncBase*>((*vol)[0])->getVolatility();
	}
	else
	{
		volfunc = (*vol)[0];
	}
	if (!volfunc->isTypeOf(FN_VOLFUNCFXDD))
	{
		//error
		throw AQLCoreInvalidData("volatility class must be AQLMathVolFuncFXDD!", __FILE__, __LINE__);
	}
	const AQLMathVolFuncFXDD* volfuncdd =dynamic_cast<const AQLMathVolFuncFXDD*>(volfunc);

	mVar[0] = ts;
	mVar[2] = te;
#ifdef __SCALAR_FLOAT__
	mVar[1] = static_cast<double>(*x_in_out);
#else
	mVar[1] = (*x_in_out);
#endif
	
	
	double v = volfuncdd->getIntegralofV(ts, te);
	double mt = -(*drift)->operator ()(mVar) * (te - ts) + 0.5 * v * v 
				- v / AQLMath::sqrt(te - ts) * (*bm);
	double ret;
	if (mt > MAXIMUM_1804) ret = - 0.5 * volfuncdd->getIntegralofSVV(ts, te);
	else
	{
		mt = AQLMath::exp(mt); 

		ret = mVar[1] - 0.5 * (1.0 + 1.0/*mt*/) * volfuncdd->getIntegralofSVV(ts, te)
				+ volfuncdd->getIntegralofSV(ts, te) / AQLMath::sqrt(te - ts) * (*bm);
		ret /= mt;
	}
#ifdef __SCALAR_FLOAT__
	(*x_in_out) = static_cast<SCALAR>(ret);
#else
	(*x_in_out) = ret;
#endif


}

