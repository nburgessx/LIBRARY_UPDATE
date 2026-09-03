/*! @file
    @brief Source code of ScobelZhu & DD type of fx sde integral class



*/
//  2008, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAPriceSZDDIntegralMelstein.cpp
//
//  SYNOPSIS    :       LAPriceSZDDIntegralMelstein
//  DESCRIPTION :       Source code of ScobelZhu & DD type of fx sde integral class
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


#include <algorithm>
#include "LAPriceSZDDIntegralMelstein.h"
#include "AQLBasic.h"
#include "LAPriceDriftFX.h"
#include "LAPriceDriftFXLogNumeraire.h"
#include "LAMathVolFuncBase.h"
#include "LAMathVolFuncSZDD.h"
#include "LAPriceQuantAdjustmentHWFXDD.h"
#include "LAMathIndexEntity.h"
#include "AQLDataBasics.h"
#include "AQLDataMatrix.h"

using namespace std;

//================ LAPriceSZDDIntegralMelstein ===================================
/*!
	@brief default constructor

	@param[in] type sde integral type

*/
//LAPriceSZDDIntegralMelstein::LAPriceSZDDIntegralMelstein()
//: LARatesSDEIntegralBase(NORMAL_INTEGRAL), mPos_old(0)
//{
//	mVar.resize(3);
//}

//furuya20140423
/*!
	@brief default constructor

	@param[in] type sde integral type

*/
LAPriceSZDDIntegralMelstein::LAPriceSZDDIntegralMelstein(const AQLString &sdeAttrName, const AQLString &sdeAttrNameVol)
: LARatesSDEIntegralBase(NORMAL_INTEGRAL, sdeAttrName), mpVolSDE(0), mSDEAttrNameVol(sdeAttrNameVol), mPos_old(0), mCorr(0)
{
	mVar.resize(3);
}

/*!
	@brief copy constructor
*/
LAPriceSZDDIntegralMelstein::LAPriceSZDDIntegralMelstein(const LAPriceSZDDIntegralMelstein& v) 
: LARatesSDEIntegralBase(v), mVar(v.mVar), mpVolSDE(v.mpVolSDE), mSDEAttrNameVol(v.mSDEAttrNameVol), mPos_old(0), mCorr(0)
{

}

/*!
	@brief destructor
*/
LAPriceSZDDIntegralMelstein::~LAPriceSZDDIntegralMelstein() 
{

}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
LAPriceSZDDIntegralMelstein::clone() const	
{
    try 
	{
		return new LAPriceSZDDIntegralMelstein(*this);
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
LAPriceSZDDIntegralMelstein::isTypeOf(function_t id) const
{
	return (id==FN_SZDDINTEGRAL2 ? true : LARatesSDEIntegralBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAPriceSZDDIntegralMelstein::getType() const
{
	return FN_SZDDINTEGRAL2;
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
LAPriceSZDDIntegralMelstein::integral(double ts, double te, 
							vector<AQLFunctionBase*>::const_iterator drift,										
							vector<vector<AQLFunctionBase*> >::const_iterator vol,
							DoubleArray::const_iterator	bm,
							SCALARARRAY::iterator	x_in_out,
							unsigned int varnum
							) const
{
	(void)varnum;
	////if (!(*drift)->isTypeOf(FN_DRIFTFX))
	////{	
	////	//error
	////	throw AQLCoreInvalidData("drift class must be LAPriceDriftFX!", __FILE__, __LINE__);
	////}	
	
	const AQLFunctionBase* volfunc;
	if ((*vol)[0]->isTypeOf(FN_VOLFUNCBASE))
	{
		volfunc = dynamic_cast<LAMathVolFuncBase*>((*vol)[0])->getVolatility();
	}
	else
	{
		volfunc = (*vol)[0];
	}
	if (!volfunc->isTypeOf(FN_VOLFUNCSZDD))
	{
		//error
		throw AQLCoreInvalidData("volatility class must be LAMathVolFuncSZDD!", __FILE__, __LINE__);
	}
	const LAMathVolFuncSZDD* volfuncsz =dynamic_cast<const LAMathVolFuncSZDD*>(volfunc);

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
		//const double rr = (*drift)->operator ()(mVar) * (te - ts);// integral of rd-rf from ts to te
		//const double sigma = volfuncsz->getV()(ts);
		//const double s = volfuncsz->getS()(ts);

		//ret = mVar[1] * rr + (mVar[1] + s) * sigma * (*bm)
		//		+ 0.5 * (mVar[1] + s) * sigma * sigma * ((*bm) * (*bm) - (te - ts))
		//		+ 0.5 * mVar[1] * rr * rr
		//		+ 0.5 * (mVar[1] + s) * sigma * sigma * sigma * ((*bm) * (*bm) / 3.0 - (te - ts)) * (*bm)
		//		+ mVar[1] * rr * sigma * (*bm)
		//		+ 0.5 * rr * sigma * s * (*bm);

	}
	else if (mSdeType == dX)
	{
				
	//--------
		// SZDD Integral
		//////////////////////////////////////////////////////////////////////////////////////////

		////YieldCurve* curve_d = curve.get(fx.get(ID_SDE,0)->curve_d, 0);
		////YieldCurve* curve_f = curve.get(fx.get(ID_SDE,0)->curve_f, 0) ;

		const double S_prev = *x_in_out;	//Sts(start)
		//temp hardcorted!!
		//const double v_prev = 0.2;	//Vts(start)	temp!! We need to get this param from outside! 

		if (!mpVolSDE)
		{
			throw AQLCoreInvalidData("Vol SDE is not set in LAPriceSZDDIntegralMelstein", __FILE__, __LINE__);
		}
		const DoubleArray& timegrid = mpVolSDE->getBM()->getTimeGrid();
		unsigned int pos;
		if (ts == 0.0) pos = 0;
		else if (ts == timegrid[mPos_old]) pos = mPos_old;
		else if (mPos_old + 2 < timegrid.size() && ts == timegrid[mPos_old + 1]) pos = mPos_old + 1;
		else if (!AQLAlgorithm::find<DoubleArray, double>(timegrid, ts, 0, timegrid.size() - 1, pos))
		{
			//error
			AQLString msg = "Time =" + AQLDataDouble(ts).convertToString();
			msg += " is not in sde integral time grid";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}

		mPos_old = pos;

		double v_prev =	mpVolSDE->getPathElement(pos)->get()[0];
		
	   //
		const double alpha		= volfuncsz->getAlpha()(ts);	//alpha (=(1 - beta) * fx0)
		const double beta		= volfuncsz->getBeta(ts);
		const double epsilon	= volfuncsz->getEpsilon()(ts);

		//temp erased!!//
		////double rho = -0.2;

		double rho = mCorr;

		//colleration & bm for VolSDE I want to reborn !!
		LARatesBM* bm1 = mpVolSDE->getBM();	//vol

		const double dBM1 = bm1->getBM()[pos].front();	//for Vol
		const double dBM2 = *bm ;	//for FX

		////double max_S_ = max_S.AsDouble();	//pricing function's argument	//Sum are outside!
		////double max_v_ = max_v.AsDouble(); 	//pricing function's argument	//Sum are outside!

		//BM.get(ID_BM_.AsString(),0)->nextStep(brown);
		//vector<double> brown(2);
		////brown[0] = BM_generator->next().value;
		////brown[1] = BM_generator->next().value;
		//brown[0] = *bm;
		//brown[1] = *bm;

		//brown[0] = BM_generator.next().value;
		//brown[1] = BM_generator.next().value;        
        
		//temp erased//
		////////dBM1 = inner_product( factor_loading_[0].begin(),
		////////					  factor_loading_[0].end(),
		////////					  brown.begin(),
		////////					  0.
		////////					) * sqrt(dt);

		////////dBM2 = inner_product( factor_loading_[1].begin(),
		////////					  factor_loading_[1].end(),
		////////					  brown.begin(),
		////////					  0.
		////////					) * sqrt(dt);


		////r_d = curve_d->Getf(t_prev);
		////r_f = curve_f->Getf(t_prev);

		//double fwd = S0 * curve_f->GetP(t) / curve_d->GetP(t);
		////double fwd = 0.5 * S0 * ( curve_f->GetP(t_prev) / curve_d->GetP(t_prev) + curve_f->GetP(t) / curve_d->GetP(t) );
		//double fwd_s = S0 * curve_f->GetP(t_prev) / curve_d->GetP(t_prev);
		//double fwd_e = S0 * curve_f->GetP(t) / curve_d->GetP(t);
		//double fwd = S0 * exp( 0.5 * log( (fwd_s * fwd_e) / (S0 * S0) )) ;
		//S_ += ( r_d - r_f ) * S_prev * dt + v_prev * ( beta * S_prev + (1. - beta) * fwd ) * dBM2;

		////double rho = factor_loading_[1][0];
		
		double s1;
		double s2;
		double s3;

		// 2D-Milstein
		//s1 = ( r_d - r_f ) * S_prev * dt + v_prev * ( beta * S_prev + (1. - beta) * fwd ) * dBM2;
		//s2 = 0.5 * ( v_prev * v_prev * beta + epsilon * rho ) * ( beta * S_prev + (1. - beta) * fwd ) * (dBM2 * dBM2 - dt);
		//s3 = epsilon * ( beta * S_prev + (1. - beta) * fwd ) * 0.5 * dBM2 * ( dBM1 - rho * dBM2);
		//S_ += s1 + s2 + s3;

		// 1.5order_2D-Milstein //
		double tau = te - ts;
		//double mu = ( r_d - r_f ) * S_prev ;
		////double Pd_s = curve_d->GetP(t_prev);
		////double Pf_s = curve_f->GetP(t_prev);
		////double Pd_e = curve_d->GetP(t);
		////double Pf_e = curve_f->GetP(t);

		////double mu = S_prev * log( (Pd_s * Pf_e) / (Pd_e * Pf_s) ) / tau;	
		////double muD = mu / S_prev;
		//furuya
		double int_drift = (*drift)->operator ()(mVar) * tau;		//drift(IR) is ok!	"(*drift)->operator ()(mVar)" is same as rd-rf !
		
		//const LAPriceDriftFX *pDrift = 0;
		//pDrift = dynamic_cast<const LAPriceDriftFX *>(*drift);
		//double int_drift =  pDrift->getDriftValue(mVar) * (te - ts);		//drift(IR) is ok!	"(*drift)->operator ()(mVar)" is same as rd-rf !

		//const LAPriceDriftFXForBarrier *pDrift = 0;
		//pDrift = dynamic_cast<const LAPriceDriftFXForBarrier *>(*drift);
		//double int_drift = pDrift->getDriftValue(mVar) * (te - ts);		//drift(IR) is ok!	"(*drift)->operator ()(mVar)" is same as rd-rf !

		double mu = S_prev * int_drift / tau;	//ok!
		double muD = mu / S_prev;	//ok!
		double sigma = v_prev * (beta * S_prev + alpha);	//ok!
		double sigmaD = v_prev * beta;	//ok!
		double brown_SQ = dBM2 * dBM2;

		//s1 = ( r_d - r_f ) * S_prev * dt + v_prev * ( beta * S_prev + (1. - beta) * fwd ) * dBM2;
		// Eular
		s1 = mu * tau + sigma * dBM2;	//ok!

		// Milstein 1D for S
		s2 = 0.5 * (sigma * sigmaD * (brown_SQ - tau)
					+ (mu * muD) * (tau * tau)
					+ (sigma * muD + mu * sigmaD) * dBM2 * tau
					+ sigma * (sigmaD * sigmaD) * (brown_SQ / 3. - tau) * dBM2);	//ok!
		
		// Adjust for Milstein 2D
		s3 = 0.5 * (epsilon * rho) * (beta * S_prev + alpha) * (dBM2 * dBM2 - tau)
			+ epsilon * (beta * S_prev + alpha)* 0.5 * dBM2 * (dBM1 - rho * dBM2);	//ok!

		////S_ += s1 + s2 + s3 ;
		ret = s1 + s2 + s3 ;	//Sum are outside!
		

		//log-Eular
		//S_ *= exp( ( r_d - r_f - 0.5 * v_prev * v_prev ) * dt + v_prev * dBM2 );
		
		//1D-Milstein
		//S_+=( r_d - r_f ) * S_prev * dt + v_prev * S_prev * dBM2
		//	+0.5*v_prev *v_prev *S_prev *(dBM2*dBM2-dt);

		//S_ += ( r_d - r_f ) * S_prev * dt + v_prev * ( beta * S_prev + (1. - beta) * fwd ) * dBM2
		//	+0.5*(v_prev *v_prev*beta +epsilon*rho)*( beta * S_prev + (1. - beta) * fwd )*(dBM2* dBM2-dt)
		//	+epsilon*( beta * S_prev + (1. - beta) * fwd )*0.5*dBM2*(dBM1-rho*dBM2);

		//S_ += ( r_d - r_f ) * S_prev * dt + v_ * ( beta * S_prev + (1. - beta) * fwd ) * dBM2;

		////sgn = sign(S_);
		////S_ = fabs(S_) > max_S_ ? sgn * max_S_ : S_;	//Sum are outside!

		//S_ += ( r_d - r_f ) * S_prev * dt + v_prev * S_prev * dBM2;
		////S.Set( t, S_ );
		
		//////////////////////////////////////////////////////////////////////////////////////////

	}
#ifdef __SCALAR_FLOAT__
	(*x_in_out) += static_cast<SCALAR>(ret);
#else
	(*x_in_out) += ret;
#endif
}

void LAPriceSZDDIntegralMelstein::setUp(LAMathPathEntity& path)
{
	//fx sde 
	AQLDataHolder *dh = &path.getData(mSDEAttrNameVol, ISNOTNULL);
	mpVolSDE = &dynamic_cast<LAMathAttrSDE&>(dh->get()).getSDE();

	//correlation
	AQLStringVector SDEAttrNames = path.getSimulationSDEAttrNames().get();
	if (SDEAttrNames.size() == 0)
		SDEAttrNames = path.getSDEAttrNames().get();

	AQLStringVector::iterator it;
	it = std::find(SDEAttrNames.begin(), SDEAttrNames.end(), mSDEAttrName);
	if (it == SDEAttrNames.end())
	{
		AQLString msg = mSDEAttrName + "is not found path.mSDEAttrName.";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	unsigned int spotpos = std::distance(SDEAttrNames.begin(), it);
	it = std::find(SDEAttrNames.begin(), SDEAttrNames.end(), mSDEAttrNameVol);
	if (it == SDEAttrNames.end())
	{
		AQLString msg = mSDEAttrNameVol + "is not found path.mSDEAttrName.";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	unsigned int volapos = std::distance(SDEAttrNames.begin(), it);
	unsigned int CorrMat1Dsize = path.getCorrelationMatrix().get1DSize();
	if (CorrMat1Dsize < SDEAttrNames.size())
		throw AQLCoreInvalidData("CorrelationMatrix size must be larger than SDEAttrNames size", __FILE__, __LINE__);
	else
		mCorr = path.getCorrelationMatrix().get()[spotpos][volapos];

}