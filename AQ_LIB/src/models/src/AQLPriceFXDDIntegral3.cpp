/*! @file
    @brief Source code of displaced diffusion type of fx sde integral class
*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLPriceFXDDIntegral3.h"
#include "AQLBasic.h"
#include "AQLPriceDriftFX.h"
#include "AQLMathVolFuncBase.h"
#include "AQLMathVolFuncFXDD.h"
#include <float.h>

using namespace std;

static const double MAXIMUM_1806 = AQLMath::log(DBL_MAX) - 1.0;

//================ AQLPriceFXDDIntegral3 ===================================
/*!
	@brief default constructor

	@param[in] type sde integral type

*/
AQLPriceFXDDIntegral3::AQLPriceFXDDIntegral3()
: AQLRatesSDEIntegralBase(NORMAL_INTEGRAL)
{
	mVar.resize(3);
}
/*!
	@brief copy constructor
*/
AQLPriceFXDDIntegral3::AQLPriceFXDDIntegral3(const AQLPriceFXDDIntegral3& v) 
: AQLRatesSDEIntegralBase(v), mVar(v.mVar)
{

}

/*!
	@brief destructor
*/
AQLPriceFXDDIntegral3::~AQLPriceFXDDIntegral3() 
{

}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLPriceFXDDIntegral3::clone() const	
{
    try 
	{
		return new AQLPriceFXDDIntegral3(*this);
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
AQLPriceFXDDIntegral3::isTypeOf(function_t id) const
{
	return (id==FN_FXDDINTEGRAL3 ? true : AQLRatesSDEIntegralBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLPriceFXDDIntegral3::getType() const
{
	return FN_FXDDINTEGRAL3;
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
AQLPriceFXDDIntegral3::integral(double ts, double te, 
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
	if (mt > MAXIMUM_1806) ret = - 0.5 * volfuncdd->getIntegralofSVV(ts, te);
	else
	{
		mt = AQLMath::exp(mt); 

		ret = mVar[1] - 0.5 * (1.0 + 1.0/*mt*/) * volfuncdd->getIntegralofSVV(ts, te)
				+ volfuncdd->getIntegralofSV(ts, te) / AQLMath::sqrt(te - ts) * (*bm);
		ret /= mt;
	}



	double rr = (*drift)->operator ()(mVar) * (te - ts);// integral of rd-rf from ts to te
	double sigma = volfuncdd->getV()(ts);
	double s = volfuncdd->getS()(ts);

	ret += 0.5 * (mVar[1] + s) * sigma * sigma * ((*bm) * (*bm) - (te - ts))
			+ 0.5 * mVar[1] * rr * rr
			+ 0.5 * (mVar[1] + s) * sigma * sigma * sigma * ((*bm) * (*bm) / 3.0 - (te - ts)) * (*bm)
			+ mVar[1] * rr * sigma * (*bm)
			+ 0.5 * rr * sigma * s * (*bm);
	

#ifdef __SCALAR_FLOAT__
	(*x_in_out) = static_cast<SCALAR>(ret);
#else
	(*x_in_out) = ret;
#endif


}

