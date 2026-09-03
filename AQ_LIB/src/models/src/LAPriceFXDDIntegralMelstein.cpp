/*! @file
    @brief Source code of displaced diffusion type of fx sde integral class



*/
//  2008, Mizuho International London..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAPriceFXDDIntegralMelstein.cpp
//
//  SYNOPSIS    :       LAPriceFXDDIntegralMelstein
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


#include "LAPriceFXDDIntegralMelstein.h"
#include "LABasic.h"
#include "LAPriceDriftFX.h"
#include "LAPriceDriftFXLogNumeraire.h"
#include "LAMathVolFuncBase.h"
#include "LAMathVolFuncFXDD.h"
#include "LAPriceQuantAdjustmentHWFXDD.h"

using namespace std;

//================ LAPriceFXDDIntegralMelstein ===================================
/*!
	@brief default constructor

	@param[in] type sde integral type

*/
LAPriceFXDDIntegralMelstein::LAPriceFXDDIntegralMelstein()
: LARatesSDEIntegralBase(NORMAL_INTEGRAL)
{
	mVar.resize(3);
}
/*!
	@brief copy constructor
*/
LAPriceFXDDIntegralMelstein::LAPriceFXDDIntegralMelstein(const LAPriceFXDDIntegralMelstein& v) 
: LARatesSDEIntegralBase(v), mVar(v.mVar)
{

}

/*!
	@brief destructor
*/
LAPriceFXDDIntegralMelstein::~LAPriceFXDDIntegralMelstein() 
{

}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAPriceFXDDIntegralMelstein::clone() const	
{
    try 
	{
		return new LAPriceFXDDIntegralMelstein(*this);
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
LAPriceFXDDIntegralMelstein::isTypeOf(function_t id) const
{
	return (id==FN_FXDDINTEGRAL2 ? true : LARatesSDEIntegralBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAPriceFXDDIntegralMelstein::getType() const
{
	return FN_FXDDINTEGRAL2;
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
LAPriceFXDDIntegralMelstein::integral(double ts, double te, 
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

	double ret = 0.0;
	if (mSdeType == DIVIDEdXbyX)
	{
		const double rr = (*drift)->operator ()(mVar) * (te - ts);// integral of rd-rf from ts to te
		const double sigma = volfuncdd->getV()(ts);
		const double s = volfuncdd->getS()(ts);

		ret = mVar[1] * rr + (mVar[1] + s) * sigma * (*bm)
				+ 0.5 * (mVar[1] + s) * sigma * sigma * ((*bm) * (*bm) - (te - ts))
				+ 0.5 * mVar[1] * rr * rr
				+ 0.5 * (mVar[1] + s) * sigma * sigma * sigma * ((*bm) * (*bm) / 3.0 - (te - ts)) * (*bm)
				+ mVar[1] * rr * sigma * (*bm)
				+ 0.5 * rr * sigma * s * (*bm);

	}
	else if (mSdeType == dX)
	{
		double int_drift = (*drift)->operator ()(mVar) * (te - ts);
		const double sigma_vol = volfuncdd->getIntegralofSigma(ts, te) / LAMath::sqrt(te - ts);
		const double alpha = volfuncdd->getAlpha()(ts);
		const double beta = volfuncdd->getBeta(ts);
		const double fx_ = *x_in_out;
		const double x = beta * fx_ + alpha;
		const double tau = te - ts;
		const double brown = *bm;
		const double brownSQ = brown * brown;
		const double diffusion = sigma_vol * brown;
		
		double quantadjust1 = 0.0;
		double quantadjust2 = 0.0;
		if ((*drift)->isTypeOf(FN_DRIFTFXLOGNUMERARIE))
		{
			const LAPriceQuantAdjustmentHWFXDD &quantadjuster = dynamic_cast<const LAPriceDriftFXLogNumeraire *>(*drift)->getQuantAdjuster();
			quantadjust1 = quantadjuster.getFXQuantAdjustNoFX(ts, te);
			quantadjust2 = quantadjuster.getFXQuantAdjustFXMulti(ts, te);
		}
		int_drift += quantadjust1;
		const double muD = int_drift / (te - ts);
		const double mu = muD * fx_ + quantadjust2;
		const double sigma = sigma_vol * x;
		const double sigmaD = sigma_vol * beta;
		
		const double remainder = 0.5 * ( sigma * sigmaD * (brownSQ - tau)
								+ (mu * muD) * (tau * tau)
								+ (sigma * muD + mu * sigmaD) * brown * tau
								+ sigma * (sigmaD * sigmaD) * (brownSQ / 3.0 - tau) * brown);

		ret = fx_ * int_drift + quantadjust2 + x * diffusion + remainder;
	}
#ifdef __SCALAR_FLOAT__
	(*x_in_out) += static_cast<SCALAR>(ret);
#else
	(*x_in_out) += ret;
#endif
}

