#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLObject.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLCoreTemplateType.h"
#include "AQLMathDefine.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLBasic.h"
#include "AQLAlgorithm.h"
#include "AQLSplineInterpolation.h"
#include "AQLStepInterpolation.h"
#include "AQLLinearInterpolation.h"
#include "AQLInterpolationBase.h"
#include "AQLPriceDataDayCount.h"
#include "LAPriceCFGenUtility.h"

#include "LAAnalyticFormula.h"
#include "LABlackScholesCalc.h"
#include "LAMathYieldCurve.h"
#include "AQLDataProcedure.h"
#include "AQLPriceDataInterpolation.h"

#include <map>
using namespace std;



/*!
	Garman-Kohlgagen Formula
	S		spot price
	K		strike
	Vol	volatility
	Td	delivery terms
	Te	expiry terms
	rf	foreign rate
	rd	local rate
*/
	//differencial of Normdist	
	double AnalyticFormulae::diffNormdist(double z)
	{
		return 1.0/(AQLMath::sqrt(2.0*AQLMath::pi()) ) * AQLMath::exp(-0.5*z*z);
	}
	//2nd differencial of Normdist
	double AnalyticFormulae::diff2ndNormdist(double z)
	{
		return - z * diffNormdist(z);
	}
	double AnalyticFormulae::GKd1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticGKParam& x = dynamic_cast<AnalyticGKParam& >(param);
		double ret = ( AQLMath::log(x.S/x.K)+(x.rd-x.rf) * x.Td + 0.5*x.Vol*x.Vol*x.Te) / (x.Vol* AQLMath::sqrt( x.Te) ) ; 	
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::GKd2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticGKParam& x = dynamic_cast<AnalyticGKParam& >(param);
		double ret = GKd1(x) - x.Vol*AQLMath::sqrt(x.Te);	
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::GKpremCall(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticGKParam& x = dynamic_cast<AnalyticGKParam& >(param);
		double ret= x.S*AQLMath::exp(-x.rf * x.Td )*AQLDist::normsdist(GKd1(x)) - x.K*AQLMath::exp(-x.rd * x.Td )*AQLDist::normsdist(GKd2(x));
		retchk;
		//return ret;
		return ret < 0.0 ? 0.0 : ret;
		FORMULAE_END
	}
	double AnalyticFormulae::GKpremPut(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticGKParam& x = dynamic_cast<AnalyticGKParam& >(param);
		double ret = GKpremCall(x) - x.S*AQLMath::exp(-x.rf * x.Td) + x.K * AQLMath::exp(-x.rd * x.Td);
		retchk;
		//return ret;
		return ret < 0.0 ? 0.0 : ret;
		FORMULAE_END
	}
	double AnalyticFormulae::GKdeltaCall(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticGKParam& x = dynamic_cast<AnalyticGKParam& >(param);
		double ret = AQLMath::exp(-x.rf * x.Td ) * AQLDist::normsdist(GKd1(x));
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::GKdeltaPut(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticGKParam& x = dynamic_cast<AnalyticGKParam& >(param);
		double ret =  GKdeltaCall(x) - AQLMath::exp(-x.rf * x.Td);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::GKgammaCall(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticGKParam& x = dynamic_cast<AnalyticGKParam& >(param);
		double ret = ( AQLMath::exp(-x.rf * x.Td ) * diffNormdist( GKd1(x) ) ) / ( x.S * x.Vol * AQLMath::sqrt(x.Te) );
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::GKgammaPut(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticGKParam& x = dynamic_cast<AnalyticGKParam& >(param);
		double ret = GKgammaCall(x);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::GKthetaCall(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticGKParam& x = dynamic_cast<AnalyticGKParam& >(param);
		double ret = x.rd * GKpremCall(x) - ( x.rd - x.rf ) * x.S * GKdeltaCall(x) - 0.5*x.Vol*x.Vol*x.S*x.S* GKgammaCall(x); 
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::GKthetaPut(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticGKParam& x = dynamic_cast<AnalyticGKParam& >(param);
		double ret = x.rd * GKpremPut(x) - ( x.rd - x.rf ) * x.S * GKdeltaPut(x) - 0.5*x.Vol*x.Vol*x.S*x.S* GKgammaCall(x); 
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::GKvegaCall(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticGKParam& x = dynamic_cast<AnalyticGKParam& >(param);
		double ret = GKgammaCall(x) * x.S * x.S * x.Vol * x.Te;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::GKvegaPut(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticGKParam& x = dynamic_cast<AnalyticGKParam& >(param);
		double ret = GKvegaCall(x);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::GKphiCall(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticGKParam& x = dynamic_cast<AnalyticGKParam& >(param);
		double ret = -x.S * GKdeltaCall(x) * x.Te ;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::GKphiPut(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticGKParam& x = dynamic_cast<AnalyticGKParam& >(param);
		double ret = -x.S * GKdeltaPut(x) * x.Te ;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::GKrhoCall(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticGKParam& x = dynamic_cast<AnalyticGKParam& >(param);
		double ret = - GKphiCall(x) - GKpremCall(x) * x.Te;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::GKrhoPut(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticGKParam& x = dynamic_cast<AnalyticGKParam& >(param);
		double ret = - GKphiPut(x) - GKpremPut(x) * x.Te;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::GKvannaCall(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticGKParam& x = dynamic_cast<AnalyticGKParam& >(param);
		double ret = -GKvegaCall(x)/x.Vol/x.S/AQLMath::sqrt(x.Te)*GKd2(x);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::GKvannaPut(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticGKParam& x = dynamic_cast<AnalyticGKParam& >(param);
		double ret = GKvannaCall(x);
		retchk;
		return ret;
		FORMULAE_END
	}

	double AnalyticFormulae::GKvolgaCall(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticGKParam& x = dynamic_cast<AnalyticGKParam& >(param);
		double ret = GKvegaCall(x)/x.Vol*GKd1(x)*GKd2(x);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::GKvolgaPut(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticGKParam& x = dynamic_cast<AnalyticGKParam& >(param);
		double ret = GKvolgaCall(x);
		retchk;
		return ret;
		FORMULAE_END
	}
/*!
	Black option Formula
	F		spot price
	K		strike 
	Vol	volatility
	Td	delivery terms
	Te	expiry terms
	Nu	Numeraire
	rd	Local Rate
*/
	double AnalyticFormulae::BKd1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticBKParam& x = dynamic_cast<AnalyticBKParam& >(param);
		double ret = ( AQLMath::log(x.F/x.K) + 0.5*x.Vol*x.Vol*x.Te) / (x.Vol* AQLMath::sqrt(x.Te) ) ; 	
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::BKd2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticBKParam& x = dynamic_cast<AnalyticBKParam& >(param);
		double ret = ( AQLMath::log(x.F/x.K) - 0.5*x.Vol*x.Vol*x.Te) / (x.Vol * AQLMath::sqrt(x.Te) );	
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::BKpremCall	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticBKParam& x = dynamic_cast<AnalyticBKParam& >(param);
		double ret = x.Nu * (x.F * AQLDist::normsdist( BKd1(x) ) - x.K * AQLDist::normsdist(BKd2(x)) ); 
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::BKpremPut	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticBKParam& x = dynamic_cast<AnalyticBKParam& >(param);
		double ret = x.Nu * (- x.F * AQLDist::normsdist( -BKd1(x) ) + x.K * AQLDist::normsdist( -BKd2(x)) ); 
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::BKvegaCall	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticBKParam& x = dynamic_cast<AnalyticBKParam& >(param);
		double ret = x.Nu * (x.F * diffNormdist( BKd1(x) ) )*AQLMath::sqrt(x.Te); 
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::BKvegaPut	(AnalyticParam& param)
	{
		return BKvegaCall(param);
	}
	double AnalyticFormulae::BKdeltaCall(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticBKParam& x = dynamic_cast<AnalyticBKParam& >(param);
		double ret = x.Nu * AQLDist::normsdist( BKd1(x) ); 
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::BKdeltaPut	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticBKParam& x = dynamic_cast<AnalyticBKParam& >(param);
		double ret = -x.Nu * AQLDist::normsdist( -BKd1(x) ); 
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::BKgammaCall	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticBKParam& x = dynamic_cast<AnalyticBKParam& >(param);
		double ret = x.Nu / (x.F * x.Vol * AQLMath::sqrt(x.Te)) * diffNormdist( BKd1(x) ); 
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::BKgammaPut	(AnalyticParam& param)
	{
		return BKgammaCall(param);
	}
	
	//if Te=Td=T, ThetaCall=\frac{\partial Call}{\partial T}
	double AnalyticFormulae::BKthetaCall	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticBKParam& x = dynamic_cast<AnalyticBKParam& >(param);
		double ret = - x.rd * BKpremCall(param) + x.Nu * x.F * 0.5* x.Vol / AQLMath::sqrt(x.Te) * diffNormdist( BKd1(x) ); 
		ret *= -1;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::BKthetaPut	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticBKParam& x = dynamic_cast<AnalyticBKParam& >(param);
		double ret = - x.rd * BKpremPut(param) + x.Nu * x.F * 0.5* x.Vol / AQLMath::sqrt(x.Te) * diffNormdist( -BKd1(x) ); 
		ret *= -1;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::BKPayOffpremCall	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticBKParam& x = dynamic_cast<AnalyticBKParam& >(param);
		double ret = x.Nu * AQLMath::max(x.F - x.K, 0.0);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::BKPayOffpremPut	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticBKParam& x = dynamic_cast<AnalyticBKParam& >(param);
		double ret = x.Nu * AQLMath::max(x.K - x.F, 0.0);
		retchk;
		return ret;
		FORMULAE_END
	}

	/*!
	Cap Floor Formula
	
*/

	double AnalyticFormulae::MMCFpremCall(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticCFParam& x = dynamic_cast<AnalyticCFParam& >(param);
		AnalyticBKParam y;
		size_t N = x.F.size(); 

		double ret = AQLMath::max((x.F[0]-x.K)*x.Nu[0],0);
		for(size_t i=1; i<N;i++)
			{
				y.K		= x.K;
				y.Vol	=x.Vol;
				y.F		= x.F[i];
				y.Nu	= x.Nu[i];
				y.Te	= x.Te[i];
				ret+= BKpremCall(y); 
			}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::MMCFvegaCall(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticCFParam& x = dynamic_cast<AnalyticCFParam& >(param);
		AnalyticBKParam y;
		size_t N = x.F.size(); 

		double ret=0;
		for(size_t i=1; i<N;i++)
			{
				y.K		= x.K;
				y.Vol	=x.Vol;
				y.F		= x.F[i];
				y.Nu	= x.Nu[i];
				y.Te	= x.Te[i];
				ret+= BKvegaCall(y); 
			}

		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::MMCFpremPut(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticCFParam& x = dynamic_cast<AnalyticCFParam& >(param);
		AnalyticBKParam y;
		size_t N = x.F.size(); 

		double	ret	=	AQLMath::max((x.K-x.F[0])*x.Nu[0],0);
		for(size_t i=1; i<N;i++)
			{
				y.K		= x.K;
				y.Vol	=x.Vol;
				y.F		= x.F[i];
				y.Nu	= x.Nu[i];
				y.Te	= x.Te[i];
				ret+= BKpremPut(y); 
			}

		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::MMCFvegaPut(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticCFParam& x = dynamic_cast<AnalyticCFParam& >(param);
		AnalyticBKParam y;
		size_t N = x.F.size(); 

		double ret=0;
		for(size_t i=1; i<N;i++)
			{
				y.K		= x.K;
				y.Vol	=x.Vol;
				y.F		= x.F[i];
				y.Nu	= x.Nu[i];
				y.Te	= x.Te[i];
				ret+= BKvegaPut(y); 
			}

		retchk;
		return ret;
		FORMULAE_END
	}
/*!
	/////Digital option Formula
	S		spot price
	K		strike
	Dig	payoff of digital option 
	Vol	volatility
	Td	delivery terms
	Te	expiry terms
	rf	foreign rate
	rd	local rate
*/
	double AnalyticFormulae::DGd1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticDGParam& x = dynamic_cast<AnalyticDGParam& >(param);
		double ret = ( AQLMath::log(x.S/x.K)+(x.rd-x.rf) * x.Td + 0.5*x.Vol*x.Vol*x.Te) / (x.Vol* AQLMath::sqrt(x.Te) );
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::DGd2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticDGParam& x = dynamic_cast<AnalyticDGParam& >(param);
		double ret = DGd1(x) - x.Vol*AQLMath::sqrt(x.Te);
		retchk;
		return ret;
		FORMULAE_END
	}

	double AnalyticFormulae::DGpremCall	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticDGParam& x = dynamic_cast<AnalyticDGParam& >(param);
		double ret = x.Dig * AQLMath::exp(-x.rd * x.Td ) * AQLDist::normsdist(DGd2(x)); 
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::DGpremPut	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticDGParam& x = dynamic_cast<AnalyticDGParam& >(param);
		double ret = x.Dig * AQLMath::exp(-x.rd * x.Td ) * AQLDist::normsdist(-DGd2(x)); 
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::DGdeltaCall	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticDGParam& x = dynamic_cast<AnalyticDGParam& >(param);
		double ret =  ( x.Dig * AQLMath::exp(-x.rd * x.Td )* diffNormdist(DGd2(x)) ) / ( x.S * x.Vol * AQLMath::sqrt(x.Te) );  
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::DGdeltaPut	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticDGParam& x = dynamic_cast<AnalyticDGParam& >(param);
		double ret =  -DGdeltaCall(x);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::DGgammaCall	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticDGParam& x = dynamic_cast<AnalyticDGParam& >(param);
		double ret = x.Dig * AQLMath::exp(-x.rd * x.Td ) / ( x.Vol * x.Vol * x.S * x.S * sqrt(x.Te) )  *  (diff2ndNormdist(DGd2(x)) - x.Vol * diffNormdist(DGd2(x)) );
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::DGgammaPut	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticDGParam& x = dynamic_cast<AnalyticDGParam& >(param);
		double ret = - DGgammaCall(x);	
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::DGthetaCall	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticDGParam& x = dynamic_cast<AnalyticDGParam& >(param);
		double ret = - x.Dig * ( x.rd * AQLMath::exp(-x.rd * x.Td ) * AQLDist::normsdist(DGd2(x)) + AQLMath::exp(-x.rd * x.Td ) * diffNormdist(DGd2(x)) * 
			(  (x.rd-x.rf- 0.5*x.Vol*x.Vol)/(2*x.Vol*AQLMath::sqrt(x.Te)) - AQLMath::log(x.S/x.K)/(2*x.Vol*AQLMath::sqrt(x.Te)*x.Te) )
						);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::DGthetaPut	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticDGParam& x = dynamic_cast<AnalyticDGParam& >(param);
		double ret = - x.Dig * x.rd * AQLMath::exp(- x.rd * x.Td ) - DGthetaCall(x);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::DGphiCall	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticDGParam& x = dynamic_cast<AnalyticDGParam& >(param);
		double ret = -  x.Dig * AQLMath::exp(- x.rd * x.Td ) * diffNormdist( DGd2(x) ) * DGd1(x)  / x.Vol ; 	
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::DGphiPut	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticDGParam& x = dynamic_cast<AnalyticDGParam& >(param);
		double ret = 	- DGphiCall(x);
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::DGrhoCall	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticDGParam& x = dynamic_cast<AnalyticDGParam& >(param);
		double ret = x.Dig * ( -x.Td * AQLMath::exp(- x.rd * x.Td ) * AQLDist::normsdist(DGd2(x)) 
			+ ( sqrt(x.Te)*AQLMath::exp(-x.rd*x.Td) * diffNormdist(DGd2(x)) ) / x.Vol ); 
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::DGrhoPut	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticDGParam& x = dynamic_cast<AnalyticDGParam& >(param);
		double ret = - x.Dig * x.Td * AQLMath::exp(- x.rd * x.Td ) - DGrhoCall(x);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::DGvegaCall	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticDGParam& x = dynamic_cast<AnalyticDGParam& >(param);
		double ret = - x.Dig * AQLMath::exp(- x.rd * x.Td ) * DGd1(x) / x.Vol * diffNormdist(DGd2(x));
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::DGvegaPut	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticDGParam& x = dynamic_cast<AnalyticDGParam& >(param);
		double ret = -DGvegaCall(x);
		retchk;
		return ret;
		FORMULAE_END
	}

	/*!
	//////Singlebarrier option Formula
	S		spot price
	K		strike 
	Vol	volatility
	Td	delivery terms
	Te	expiry terms
	rd	local rate
	rf	foreign rate
	L		Limit value
	R		Rebate value
	cp	if call cp =1, if put cp=-1
	du	if down du =1, if up  du=-1
	ReTime time at Barrier Reach ReTime=1, time at DeliveryDate ReTime=0 (Defalut ReTime ==1)
*/

//hishida vannavolga
	double AnalyticFormulae::MMSBprobUNT(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double keyval = x.L/x.S;

		double ret = 0.0;
		if (x.L > x.S)
		{
			ret =  AQLDist::normsdist((AQLMath::log(keyval) -(x.rd-x.rf)*x.Td)/x.Vol/AQLMath::sqrt(x.Te))
			- AQLMath::pow(keyval,2*(x.rd-x.rf)/x.Vol/x.Vol) 
				* AQLDist::normsdist(- (AQLMath::log(keyval) +(x.rd-x.rf)*x.Td)/x.Vol/AQLMath::sqrt(x.Te));
		}
		else
		{
			ret = 0.0;
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::MMSBprobDNT(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double keyval = x.S/x.L;

		double ret = 0.0;
		if (x.L < x.S)
		{
			ret =  AQLDist::normsdist((AQLMath::log(keyval) -(x.rd-x.rf)*x.Td)/x.Vol/AQLMath::sqrt(x.Te))
			- AQLMath::pow(keyval,2*(x.rd-x.rf)/x.Vol/x.Vol) 
				* AQLDist::normsdist(- (AQLMath::log(keyval) +(x.rd-x.rf)*x.Td)/x.Vol/AQLMath::sqrt(x.Te));
		}
		else
		{
			ret = 0.0;
		}
		retchk;
		return ret;
		FORMULAE_END
	}


//hishida vannavolga



	double AnalyticFormulae::SBmu1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =  x.rd-x.rf;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBmu2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = SBmu1(x)*x.Td/x.Te - 0.5*x.Vol*x.Vol;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBmu3(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = AQLMath::sqrt( SBmu2(x)*SBmu2(x) + 2.0 * x.rd * x.Vol * x.Vol);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBnu1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = SBmu2(x)/x.Vol/x.Vol;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBnu2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = SBnu1(x)+1;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBx1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = AQLMath::log(x.S/x.K)/(x.Vol*AQLMath::sqrt(x.Te)) + SBnu2(x) * x.Vol * AQLMath::sqrt(x.Te); 
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBx2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = AQLMath::log(x.L*x.L/x.S/x.K)/(x.Vol*AQLMath::sqrt(x.Te)) + SBnu2(x) * x.Vol * AQLMath::sqrt(x.Te);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBy1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = AQLMath::log(x.S/x.L)/(x.Vol*AQLMath::sqrt(x.Te)) + SBnu2(x) * x.Vol * AQLMath::sqrt(x.Te); 
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBy2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = AQLMath::log(x.L/x.S)/(x.Vol*AQLMath::sqrt(x.Te)) + SBnu2(x) * x.Vol * AQLMath::sqrt(x.Te); 
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBz(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret=0;
		if(x.ReTime)
		{
			ret = AQLMath::log(x.L/x.S)/(x.Vol*AQLMath::sqrt(x.Te)) + SBmu3(x) / x.Vol * AQLMath::sqrt(x.Te); 
		}
		else if(!x.ReTime)
		{
			ret = AQLMath::log(x.L/x.S)/(x.Vol*AQLMath::sqrt(x.Te)) + SBmu2(x) / x.Vol * AQLMath::sqrt(x.Te);
		}
		retchk;
		return ret;
		FORMULAE_END
	}

	double AnalyticFormulae::SBdx1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = 1/x.S/x.Vol/AQLMath::sqrt(x.Te); 
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBdx2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =  - 1/x.S/x.Vol/AQLMath::sqrt(x.Te);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBdy1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =  1/x.S/x.Vol/AQLMath::sqrt(x.Te);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBdy2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = - 1.0/x.S/x.Vol/AQLMath::sqrt(x.Te);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBdz(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret=0;
		if(x.ReTime)
		{
			ret =  - 1/x.S/x.Vol/AQLMath::sqrt(x.Te);
		}
		else if(!x.ReTime)
		{
			ret =  - 1/x.S/x.Vol/AQLMath::sqrt(x.Te);
		}
		retchk;
		return ret;
		FORMULAE_END
	}


	double AnalyticFormulae::SBddx1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = - 1/x.S/x.S/x.Vol/AQLMath::sqrt(x.Te); 
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBddx2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =   1/x.S/x.S/x.Vol/AQLMath::sqrt(x.Te);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBddy1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =  - 1/x.S/x.S/x.Vol/AQLMath::sqrt(x.Te);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBddy2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =  1/x.S/x.S/x.Vol/AQLMath::sqrt(x.Te);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBddz(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret=0;
		if(x.ReTime)
		{
			ret =  1/x.S/x.S/x.Vol/AQLMath::sqrt(x.Te);
		}
		else if(!x.ReTime)
		{
			ret =   1/x.S/x.S/x.Vol/AQLMath::sqrt(x.Te);
		}
		retchk;
		return ret;
		FORMULAE_END
	}

	double AnalyticFormulae::SBvmu2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = -x.Vol;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBvmu3(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = x.Vol*(-SBmu2(x) + 2.0*x.rd)/AQLMath::sqrt( SBmu2(x)*SBmu2(x) + 2.0 * x.rd * x.Vol * x.Vol);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBvnu1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = -1.0/x.Vol - 2.0 * SBmu2(x)/x.Vol/x.Vol/x.Vol;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBvnu2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =  -1.0/x.Vol - 2.0*SBmu2(x)/x.Vol/x.Vol/x.Vol;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBvx1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = -AQLMath::log(x.S/x.K)/(x.Vol*x.Vol*AQLMath::sqrt(x.Te)) + SBnu2(x) * AQLMath::sqrt(x.Te) + x.Vol * AQLMath::sqrt(x.Te) * SBvnu2(x) ; 
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBvx2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = -AQLMath::log(x.L*x.L/x.S/x.K)/(x.Vol*AQLMath::sqrt(x.Te)) + SBnu2(x) * AQLMath::sqrt(x.Te) + x.Vol * AQLMath::sqrt(x.Te) * SBvnu2(x) ; 
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBvy1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = -AQLMath::log(x.S/x.L)/(x.Vol*x.Vol*AQLMath::sqrt(x.Te)) + SBnu2(x) * AQLMath::sqrt(x.Te) + x.Vol * AQLMath::sqrt(x.Te) * SBvnu2(x) ; 
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBvy2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = -AQLMath::log(x.L/x.S)/(x.Vol*x.Vol*AQLMath::sqrt(x.Te)) + SBnu2(x) * AQLMath::sqrt(x.Te) + x.Vol * AQLMath::sqrt(x.Te) * SBvnu2(x) ; 
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBvz(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret=0;
		if(x.ReTime)
		{
			ret = -AQLMath::log(x.L/x.S)/(x.Vol*x.Vol*AQLMath::sqrt(x.Te)) 
					- SBmu3(x) / x.Vol/x.Vol * AQLMath::sqrt(x.Te) + SBvmu3(x) * AQLMath::sqrt(x.Te) /x.Vol; 
		}
		else if(!x.ReTime)
		{
			ret = -AQLMath::log(x.L/x.S)/(x.Vol*x.Vol*AQLMath::sqrt(x.Te)) 
					- SBmu2(x) / x.Vol/x.Vol * AQLMath::sqrt(x.Te) + SBvmu2(x) * AQLMath::sqrt(x.Te) /x.Vol;
		}
		retchk;
		return ret;
		FORMULAE_END
	}

	double AnalyticFormulae::SBtmu2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = SBmu1(x)/x.Te - SBmu1(x)*x.Td/x.Te/x.Te;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBtmu3(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = SBmu2(x) / AQLMath::sqrt( SBmu2(x)*SBmu2(x) + 2.0 * x.rd * x.Vol * x.Vol) * SBtmu2(x);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBtnu1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = SBtmu2(x) /x.Vol/x.Vol;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBtnu2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = SBtmu2(x) /x.Vol/x.Vol;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBtx1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = -AQLMath::log(x.S/x.K)/(2.0*x.Vol*x.Te*AQLMath::sqrt(x.Te)) 
						+ SBtnu2(x) * x.Vol * AQLMath::sqrt(x.Te) + SBnu2(x) * x.Vol /2.0/AQLMath::sqrt(x.Te); 
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBtx2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = -AQLMath::log(x.L*x.L/x.S/x.K)/(2.0*x.Vol*x.Te*AQLMath::sqrt(x.Te))
					+ SBtnu2(x) * x.Vol * AQLMath::sqrt(x.Te) + SBnu2(x) * x.Vol /2.0/AQLMath::sqrt(x.Te);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBty1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = -AQLMath::log(x.S/x.L)/2.0/x.Vol/x.Te/AQLMath::sqrt(x.Te) 
					+SBtnu2(x)*x.Vol*AQLMath::sqrt(x.Te) + SBnu2(x) * x.Vol /2.0/ AQLMath::sqrt(x.Te); 
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBty2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = -AQLMath::log(x.L/x.S)/2.0/x.Vol/x.Te/AQLMath::sqrt(x.Te) 
					+ SBtnu2(x) * x.Vol * AQLMath::sqrt(x.Te) + SBnu2(x) * x.Vol /2.0/AQLMath::sqrt(x.Te); 
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBtz(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret=0;
		if(x.ReTime)
		{
			ret = -AQLMath::log(x.L/x.S)/(2.0*x.Vol*x.Te*AQLMath::sqrt(x.Te)) 
					+ SBtmu3(x) / x.Vol * AQLMath::sqrt(x.Te)+ SBmu3(x) /2.0 / x.Vol / AQLMath::sqrt(x.Te); 
		}
		else if(!x.ReTime)
		{
			ret = -AQLMath::log(x.L/x.S)/(2.0*x.Vol*x.Te*AQLMath::sqrt(x.Te)) 
					+ SBtmu2(x) / x.Vol * AQLMath::sqrt(x.Te)+ SBmu2(x) /2.0 / x.Vol / AQLMath::sqrt(x.Te);
		}
		retchk;
		return ret;
		FORMULAE_END
	}


	double AnalyticFormulae::SBrmu1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =  1;
			retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBrmu2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = x.Td/x.Te;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBrmu3(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = 1/2.0/AQLMath::sqrt( SBmu2(x)*SBmu2(x) + 2.0 * x.rd * x.Vol * x.Vol)*( 2.0*SBmu2(x)*x.Td/x.Te + 2.0*x.Vol*x.Vol );
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBrnu1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = x.Td/x.Te/x.Vol/x.Vol;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBrnu2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = x.Td/x.Te/x.Vol/x.Vol;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBrx1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = SBrnu2(x) * x.Vol * AQLMath::sqrt(x.Te); 
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBrx2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =  SBrnu2(x) * x.Vol * AQLMath::sqrt(x.Te);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBry1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = SBrnu2(x) * x.Vol * AQLMath::sqrt(x.Te); 
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBry2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = SBrnu2(x) * x.Vol * AQLMath::sqrt(x.Te); 
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBrz(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret=0;
		if(x.ReTime)
		{
			ret = SBrmu3(x) / x.Vol * AQLMath::sqrt(x.Te); 
		}
		else if(!x.ReTime)
		{
			ret = SBrmu2(x) / x.Vol * AQLMath::sqrt(x.Te);
		}
		retchk;
		return ret;
		FORMULAE_END
	}

	double AnalyticFormulae::SBpmu1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =  -1;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBpmu2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = -x.Td/x.Te;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBpmu3(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = -1/AQLMath::sqrt( SBmu2(x)*SBmu2(x) + 2.0 * x.rd * x.Vol * x.Vol)*SBmu2(x)*x.Td/x.Te ;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae:: SBpnu1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = -x.Td/x.Vol/x.Vol/x.Te;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBpnu2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = -x.Td/x.Vol/x.Vol/x.Te;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBpx1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = SBpnu2(x) * x.Vol * AQLMath::sqrt(x.Te); 
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBpx2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = SBpnu2(x) * x.Vol * AQLMath::sqrt(x.Te);
		retchk;
		return ret;
		FORMULAE_END
	}

	double AnalyticFormulae::SBpy1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =  SBpnu2(x) * x.Vol * AQLMath::sqrt(x.Te); 
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBpy2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = SBpnu2(x) * x.Vol * AQLMath::sqrt(x.Te); 
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBpz(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret=0;
		if(x.ReTime)
		{
			ret = SBpmu3(x) / x.Vol * AQLMath::sqrt(x.Te); 
		}
		else if(!x.ReTime)
		{
			ret = SBpmu2(x) / x.Vol * AQLMath::sqrt(x.Te);
		}
		retchk;
		return ret;
		FORMULAE_END
	}


	double AnalyticFormulae::SBdI1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = x.cp*AQLMath::exp(-x.rf*x.Td)*AQLDist::normsdist(x.cp*SBx1(x)) ;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBdI2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = x.cp*AQLMath::exp(-x.rf*x.Td)*AQLDist::normsdist(x.cp*SBy1(x))
			+ x.S * AQLMath::exp(- x.rf * x.Td ) * diffNormdist(x.cp * SBy1(x) ) * SBdy1(x)
			- x.K * AQLMath::exp(- x.rd * x.Td ) * diffNormdist(x.cp * SBy1(x) - x.cp * x.Vol * AQLMath::sqrt(x.Te) ) * SBdy1(x) ;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBdI3(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = (1.0-2.0*SBnu2(x))*x.cp*AQLMath::exp(-x.rf*x.Td)* AQLMath::pow(x.L/x.S, 2*SBnu2(x)) * AQLDist::normsdist(x.du*SBx2(x))
			+2.0 * SBnu1(x) * x.cp*x.K/x.S * AQLMath::exp(-x.rd*x.Td)* AQLMath::pow(x.L/x.S, 2*SBnu1(x)) * AQLDist::normsdist( x.du*SBx2(x)- x.du*x.Vol*AQLMath::sqrt(x.Te) ) ;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBdI4(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = (1.0-2.0*SBnu2(x) ) * x.cp * AQLMath::exp(-x.rf*x.Td) * AQLMath::pow(x.L/x.S, 2.0*SBnu2(x)) * AQLDist::normsdist(x.du*SBy2(x))
			+2.0 * SBnu1(x) * x.cp*x.K/x.S * AQLMath::exp(-x.rd*x.Td) * AQLMath::pow(x.L/x.S, 2.0*SBnu1(x)) * AQLDist::normsdist( x.du*SBy2(x)- x.du*x.Vol*AQLMath::sqrt(x.Te) )
			+ x.cp * x.du * x.S * AQLMath::exp(-x.rf * x.Td) * AQLMath::pow(x.L/x.S, 2.0 * SBnu2(x) ) * diffNormdist( x.du * SBy2(x) ) * SBdy2(x) 
			- x.cp * x.du * x.K * AQLMath::exp(-x.rd * x.Td) * AQLMath::pow(x.L/x.S, 2.0 * SBnu1(x) ) * diffNormdist( x.du * SBy2(x) - x.du * x.Vol * AQLMath::sqrt(x.Te) ) * SBdy2(x) ;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBdI5(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = x.R*AQLMath::exp(-x.rd*x.Td)* ( x.du*diffNormdist(x.du*SBy1(x)- x.du*x.Vol*AQLMath::sqrt(x.Te))*SBdy1(x) 
					 - AQLMath::pow(x.L/x.S, 2*SBnu1(x)) *x.du* diffNormdist( x.du*SBy2(x)- x.du*x.Vol*AQLMath::sqrt(x.Te) )*SBdy2(x) )
					 +x.R*AQLMath::exp(-x.rd*x.Td)*2.0*SBnu1(x)/x.S*AQLMath::pow(x.L/x.S, 2*SBnu1(x)) * AQLDist::normsdist(x.du*SBy2(x)- x.du*x.Vol*AQLMath::sqrt(x.Te)) ;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBdI6(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret=0;
		if(x.ReTime)
		{
			ret = x.du*x.R * ( AQLMath::pow( x.L/x.S, (SBmu2(x)+SBmu3(x))/x.Vol/x.Vol) * diffNormdist(x.du * SBz(x)) * SBdz(x)
								+AQLMath::pow(x.L/x.S, (SBmu2(x)-SBmu3(x))/x.Vol/x.Vol) * diffNormdist(x.du * SBz(x) -2* x.du * SBmu3(x)/x.Vol * AQLMath::sqrt(x.Te) )*SBdz(x))	
				-x.R*((SBmu2(x)+SBmu3(x))/x.Vol/x.Vol/x.S*AQLMath::pow(x.L/x.S, (SBmu2(x)+SBmu3(x))/x.Vol/x.Vol) * AQLDist::normsdist(x.du * SBz(x))
						+(SBmu2(x)-SBmu3(x))/x.Vol/x.Vol/x.S*AQLMath::pow(x.L/x.S, (SBmu2(x)-SBmu3(x))/x.Vol/x.Vol) * AQLDist::normsdist(x.du * SBz(x)-2* x.du * SBmu3(x)/x.Vol * AQLMath::sqrt(x.Te) ) ) ;
		}
		else if(!x.ReTime)
		{
			ret = x.du*x.R * ( AQLMath::pow( x.L/x.S, 2.0*SBmu2(x)/x.Vol/x.Vol) * diffNormdist(x.du * SBz(x)) * SBdz(x)
								+diffNormdist(x.du * SBz(x) -2* x.du * SBmu2(x)/x.Vol * AQLMath::sqrt(x.Te) )*SBdz(x))	
				-x.R*2.0*SBmu2(x)/x.Vol/x.Vol/x.S*AQLMath::pow(x.L/x.S, 2.0*SBmu2(x)/x.Vol/x.Vol) * AQLDist::normsdist(x.du * SBz(x)) ;
		}
		retchk;
		return ret;
		FORMULAE_END
	}

	double AnalyticFormulae::SBgI1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =  AQLMath::exp(-x.rf*x.Td)*diffNormdist(x.cp*SBx1(x))*SBdx1(x) ;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBgI2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = 2.0 * AQLMath::exp(-x.rf*x.Td)*diffNormdist(x.cp*SBy1(x))*SBdy1(x)
			- x.S * AQLMath::exp(-x.rf * x.Td ) * diffNormdist(x.cp * SBy1(x)) * SBy1(x) * SBdy1(x) * SBdy1(x) 
			+ x.S * AQLMath::exp(-x.rf*x.Td) * diffNormdist(x.cp * SBy1(x) )*SBddy1(x)
			+ x.K * AQLMath::exp(-x.rd * x.Td) * diffNormdist(x.cp * SBy1(x) - x.cp * x.Vol * AQLMath::sqrt(x.Te) ) * (SBy1(x) - x.Vol * AQLMath::sqrt(x.Te) ) *SBdy1(x) * SBdy1(x)
			- x.K * AQLMath::exp(-x.rd * x.Td) * diffNormdist(x.cp * SBy1(x) - x.cp * x.Vol * AQLMath::sqrt(x.Te) ) * SBddy1(x);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBgI3(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = 2.0* SBnu2(x) * ( 2.0 * SBnu2(x) - 1 ) * x.cp*AQLMath::exp(-x.rf*x.Td)* AQLMath::pow(x.L, 2*SBnu2(x)) * AQLMath::pow(1/x.S, 2*SBnu2(x)+1) * AQLDist::normsdist(x.du*SBx2(x))
					- ( 2.0* SBnu2(x) - 1 ) * x.cp * x.du * AQLMath::exp(-x.rf*x.Td)*AQLMath::pow(x.L/x.S, 2*SBnu2(x)) * diffNormdist(x.du*SBx2(x)) * SBdx2(x)
					- 2.0 * SBnu1(x) * (2.0*SBnu1(x) + 1) * x.cp * x.K * AQLMath::exp(-x.rd*x.Td) * AQLMath::pow(x.L, 2*SBnu1(x))* AQLMath::pow(1/x.S, 2*SBnu1(x)+2) * AQLDist::normsdist(x.du*SBx2(x) - x.du * x.Vol * AQLMath::sqrt(x.Te))
					+ 2.0 * SBnu1(x) * x.cp * x.du * x.K * AQLMath::exp(-x.rd*x.Td) * AQLMath::pow(x.L, 2*SBnu1(x)) * AQLMath::pow(1/x.S, 2*SBnu1(x)+1)* diffNormdist(x.du * SBx2(x) - x.du * x.Vol  *AQLMath::sqrt(x.Te)) * SBdx2(x);			
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBgI4(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =  2.0* SBnu2(x) * ( 2.0 * SBnu2(x) - 1 ) * x.cp *AQLMath::exp(-x.rf*x.Td)* AQLMath::pow(x.L, 2*SBnu2(x)) * AQLMath::pow(1/x.S, 2*SBnu2(x)+1) * AQLDist::normsdist(x.du*SBy2(x))
					-2.0*( 2.0* SBnu2(x) - 1 ) * x.cp * x.du * AQLMath::exp(-x.rf*x.Td)*AQLMath::pow(x.L/x.S, 2*SBnu2(x)) * diffNormdist(x.du*SBy2(x)) * SBdy2(x)
					-x.cp * x.du * AQLMath::exp(-x.rf*x.Td) * AQLMath::pow(x.L, 2*SBnu2(x)) * AQLMath::pow(1/x.S, 2*SBnu2(x) - 1) * diffNormdist(x.du*SBy2(x))* ( SBy2(x) * SBdy2(x) * SBdy2(x) - SBddy2(x) )
					- 2.0 * SBnu1(x) * (2.0*SBnu1(x) + 1) * x.cp * x.K * AQLMath::exp(-x.rd*x.Td) * AQLMath::pow(x.L, 2*SBnu1(x))* AQLMath::pow(1/x.S, 2*SBnu1(x)+2) * AQLDist::normsdist(x.du*SBy2(x) - x.du * x.Vol * AQLMath::sqrt(x.Te))
					+ 4.0 * SBnu1(x) * x.cp * x.du * x.K * AQLMath::exp(-x.rd*x.Td) * AQLMath::pow(x.L, 2*SBnu1(x))* AQLMath::pow(1/x.S, 2*SBnu1(x)+1) * diffNormdist(x.du*SBy2(x) - x.du * x.Vol * AQLMath::sqrt(x.Te)) *SBdy2(x)
					+ x.cp * x.du * x.K * AQLMath::exp(-x.rd*x.Td) * AQLMath::pow(x.L/x.S, 2*SBnu1(x)) * diffNormdist(x.du*SBy2(x) - x.du * x.Vol * AQLMath::sqrt(x.Te)) * ( ( SBy2(x) - x.Vol * AQLMath::sqrt(x.Te) ) * SBdy2(x) * SBdy2(x) - SBddy2(x) );	
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBgI5(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = x.R * AQLMath::exp(-x.rd*x.Td) * x.du * diffNormdist(x.du*SBy1(x) - x.du*x.Vol*AQLMath::sqrt(x.Te)) * ( -( SBy1(x) - x.Vol * AQLMath::sqrt(x.Te)) * SBdy1(x) * SBdy1(x) + SBddy1(x))
					+ x.R * AQLMath::exp(-x.rd*x.Td) * ( -2.0 *SBnu1(x) * ( 2.0 * SBnu1(x) + 1) * AQLMath::pow(x.L, 2*SBnu1(x))* AQLMath::pow(1/x.S, 2*SBnu1(x)+2) * AQLDist::normsdist(x.du*SBy2(x) - x.du * x.Vol * AQLMath::sqrt(x.Te))
					+ 2.0*SBnu1(x) * AQLMath::pow(x.L, 2*SBnu1(x))* AQLMath::pow(1/x.S, 2*SBnu1(x)+1) * diffNormdist(x.du*SBy2(x) - x.du * x.Vol * AQLMath::sqrt(x.Te)) * x.du * SBdy2(x))
                    - x.R * AQLMath::exp(-x.rd*x.Td) * x.du * ( -2.0 * SBnu1(x) * AQLMath::pow(x.L, 2*SBnu1(x)) * AQLMath::pow(1/x.S, 2*SBnu1(x)+1) * diffNormdist( x.du*SBy2(x) - x.du*x.Vol*AQLMath::sqrt(x.Te)) * SBdy2(x)
					+ AQLMath::pow(x.L/x.S, 2*SBnu1(x)) * diffNormdist( x.du*SBy2(x) - x.du*x.Vol*AQLMath::sqrt(x.Te))  * ( -(SBy2(x) - x.Vol*AQLMath::sqrt(x.Te)) * (SBdy2(x) * SBdy2(x)) + SBddy2(x) )  );
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBgI6(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret=0;
		if(x.ReTime)
		{
			ret = x.R * AQLMath::pow( x.L/x.S, (SBmu2(x)+SBmu3(x))/x.Vol/x.Vol) * ( ((SBmu2(x)+SBmu3(x))/x.Vol/x.Vol) * ((SBmu2(x)+SBmu3(x))/x.Vol/x.Vol + 1)  / x.S/x.S * AQLDist::normsdist(x.du * SBz(x))
				- 2.0 *  (SBmu2(x)+SBmu3(x))/x.Vol/x.Vol /x.S *  diffNormdist(x.du * SBz(x)) * x.du * SBdz(x)
				- diffNormdist(x.du * SBz(x)) * x.du * (SBz(x) * SBdz(x) * SBdz(x) - SBddz(x)) ) 
				+ x.R * AQLMath::pow(x.L/x.S, (SBmu2(x)-SBmu3(x))/x.Vol/x.Vol) * (  (SBmu2(x)-SBmu3(x))/x.Vol/x.Vol * ((SBmu2(x)-SBmu3(x))/x.Vol/x.Vol+1) /x.S/x.S * AQLDist::normsdist(x.du * SBz(x) -2* x.du * SBmu3(x)/x.Vol * AQLMath::sqrt(x.Te))
				- 2.0 * (SBmu2(x)-SBmu3(x))/x.Vol/x.Vol /x.S * diffNormdist(x.du * SBz(x) -2* x.du * SBmu3(x)/x.Vol * AQLMath::sqrt(x.Te))*x.du*SBdz(x)
				- diffNormdist(x.du * SBz(x) -2* x.du * SBmu3(x)/x.Vol * AQLMath::sqrt(x.Te)) * x.du * ( ( SBz(x) - 2 * SBmu3(x)/x.Vol * AQLMath::sqrt(x.Te)) * SBdz(x) * SBdz(x) - SBddz(x))  );
		}
		else if(!x.ReTime)
		{
			ret = x.R * AQLMath::pow( x.L/x.S, 2.0*SBmu2(x)/x.Vol/x.Vol) * ( (2.0*SBmu2(x)/x.Vol/x.Vol) * (2.0*SBmu2(x)/x.Vol/x.Vol + 1)  / x.S/x.S * AQLDist::normsdist(x.du * SBz(x))
				- 4.0 * SBmu2(x)/x.Vol/x.Vol /x.S * diffNormdist(x.du * SBz(x)) * x.du * SBdz(x)
				- diffNormdist(x.du * SBz(x)) * x.du * (SBz(x) * SBdz(x) * SBdz(x) - SBddz(x)) ) 
				- x.R * diffNormdist(x.du * SBz(x) -2* x.du * SBmu2(x)/x.Vol * AQLMath::sqrt(x.Te)) * x.du * (( SBz(x) - 2 * SBmu2(x)/x.Vol * AQLMath::sqrt(x.Te)) * SBdz(x) * SBdz(x) - SBddz(x));
		}
		retchk;
		return ret;
		FORMULAE_END
	}


	double AnalyticFormulae::SBvI1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =x.S*x.S*x.Vol*x.Te*SBgI1(x);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBvI2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = x.S*AQLMath::exp(-x.rf *x.Td)*diffNormdist(x.cp * SBy1(x))*SBvy1(x) 
					- x.K * AQLMath::exp(-x.rd *x.Td) * diffNormdist(x.cp * SBy1(x) - x.cp * x.Vol * AQLMath::sqrt(x.Te)) * (SBvy1(x) - AQLMath::sqrt(x.Te));
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBvI3(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = x.cp*x.K*AQLMath::exp(-x.rd *x.Td)* AQLMath::pow(x.L/x.S, 2*SBnu1(x)) * diffNormdist(x.du*SBx2(x) - x.du*x.Vol*AQLMath::sqrt(x.Te) ) * x.du * AQLMath::sqrt(x.Te) 
					+ 2.0 * SBI3(x) *SBvnu1(x) * AQLMath::log(x.L/x.S);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBvI4(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =x.cp * x.S * AQLMath::exp(-x.rf *x.Td) * AQLMath::pow(x.L/x.S, 2*SBnu2(x)) * diffNormdist(x.du*SBy2(x)) * x.du * SBvy2(x)  
					- x.cp*x.K*AQLMath::exp(-x.rd *x.Td)* AQLMath::pow(x.L/x.S, 2*SBnu1(x)) * diffNormdist(x.du*SBy2(x) - x.du*x.Vol*AQLMath::sqrt(x.Te) ) * (x.du * SBvy2(x)  - x.du * AQLMath::sqrt(x.Te) ) 
					+ 2.0 * SBI4(x) *SBvnu1(x) * AQLMath::log(x.L/x.S);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBvI5(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = x.R*AQLMath::exp(-x.rd*x.Td)* (  diffNormdist(x.du*SBy1(x)- x.du*x.Vol*AQLMath::sqrt(x.Te)) * x.du * (SBvy1(x) - AQLMath::sqrt(x.Te) ) 
														- AQLMath::pow(x.L/x.S, 2*SBnu1(x)) * diffNormdist( x.du*SBy2(x)- x.du*x.Vol*AQLMath::sqrt(x.Te) ) * x.du * (SBvy2(x) - AQLMath::sqrt(x.Te)) 
														- AQLMath::log(x.L/x.S) *AQLMath::pow(x.L/x.S, 2.0*SBnu1(x))*2.0*SBvnu1(x)*AQLDist::normsdist( x.du*SBy2(x)- x.du*x.Vol*AQLMath::sqrt(x.Te)) ); 
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBvI6(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret=0;
		if(x.ReTime)
		{
			ret = x.R * (  AQLMath::pow( x.L/x.S, (SBmu2(x)+SBmu3(x))/x.Vol/x.Vol) * diffNormdist(x.du * SBz(x)) * x.du * SBvz(x)
							+ AQLMath::pow(x.L/x.S, (SBmu2(x)-SBmu3(x))/x.Vol/x.Vol) * diffNormdist(x.du * SBz(x) -2.0* x.du * SBmu3(x)/x.Vol * AQLMath::sqrt(x.Te) ) * x.du * (SBvz(x) - 2.0* AQLMath::sqrt(x.Te) / x.Vol *SBvmu3(x) + 2.0 * SBmu3(x) * AQLMath::sqrt(x.Te) /x.Vol/x.Vol  )  )
				+ x.R * AQLMath::log(x.L/x.S) * AQLMath::pow( x.L/x.S, (SBmu2(x)+SBmu3(x))/x.Vol/x.Vol) *( (SBvmu2(x) + SBvmu3(x))/x.Vol/x.Vol - 2.0*(SBmu2(x)+SBmu3(x))/x.Vol/x.Vol/x.Vol ) * AQLDist::normsdist(x.du * SBz(x)) 
				+ x.R * AQLMath::log(x.L/x.S) * AQLMath::pow( x.L/x.S, (SBmu2(x)-SBmu3(x))/x.Vol/x.Vol) *( (SBvmu2(x) - SBvmu3(x))/x.Vol/x.Vol - 2.0*(SBmu2(x)-SBmu3(x))/x.Vol/x.Vol/x.Vol ) * AQLDist::normsdist(x.du * SBz(x) - 2.0 * x.du * SBmu3(x) *AQLMath::sqrt(x.Te) / x.Vol);
				
		}
		else if(!x.ReTime)
		{
			ret = x.R * ( AQLMath::pow( x.L/x.S, (2.0 * SBmu2(x))/x.Vol/x.Vol) * diffNormdist(x.du * SBz(x)) * x.du * SBvz(x)
				+ diffNormdist(x.du * SBz(x) -2.0* x.du * SBmu2(x)/x.Vol * AQLMath::sqrt(x.Te) ) * x.du * (SBvz(x) - 2.0* AQLMath::sqrt(x.Te) / x.Vol *SBvmu2(x) + 2.0 * SBmu2(x) * AQLMath::sqrt(x.Te) /x.Vol/x.Vol  ))
				+ x.R * AQLMath::log(x.L/x.S) * AQLMath::pow( x.L/x.S, 2.0*SBmu2(x)/x.Vol/x.Vol) *( 2.0*SBvmu2(x)/x.Vol/x.Vol - 4.0*SBmu2(x)/x.Vol/x.Vol/x.Vol ) * AQLDist::normsdist(x.du * SBz(x));
		}
		retchk;
		return ret;
		FORMULAE_END
	}

	double AnalyticFormulae::SBtI1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = -x.rd * SBI1(x) + SBmu1(x) * x.S * SBdI1(x) + x.Vol * x.Vol*x.S * x.S /2 * SBgI1(x);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBtI2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =- x.cp * x.rf * x.S * AQLMath::exp(-x.rf*x.Td) * AQLDist::normsdist(x.cp*SBy1(x))
					+ x.rd * x.cp * x.K * AQLMath::exp(-x.rd*x.Td) * AQLDist::normsdist(x.cp*SBy1(x) - x.cp * x.Vol * AQLMath::sqrt(x.Te))
					+ x.S * AQLMath::exp(-x.rf*x.Td) * diffNormdist(x.cp*SBy1(x)) * SBty1(x) 
					- x.K * AQLMath::exp(-x.rd*x.Td) * diffNormdist(x.cp*SBy1(x) - x.cp * x.Vol * AQLMath::sqrt(x.Te) ) * ( SBty1(x) - x.Vol/2/AQLMath::sqrt(x.Te));
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBtI3(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = -x.cp* x.rf *x.S*AQLMath::exp(-x.rf*x.Td)* AQLMath::pow(x.L/x.S, 2*SBnu2(x)) * AQLDist::normsdist(x.du*SBx2(x))
					+x.cp* x.rd *x.K*AQLMath::exp(-x.rd*x.Td)* AQLMath::pow(x.L/x.S, 2*SBnu1(x)) * AQLDist::normsdist( x.du*SBx2(x)- x.du*x.Vol*AQLMath::sqrt(x.Te) )
					+2.0*SBtnu2(x)* AQLMath::log(x.L/x.S) *SBI3(x)
					+x.cp* x.du *x.K*AQLMath::exp(-x.rd*x.Td)* AQLMath::pow(x.L/x.S, 2*SBnu1(x)) * diffNormdist( x.du*SBx2(x)- x.du*x.Vol*AQLMath::sqrt(x.Te) ) * x.Vol/2/AQLMath::sqrt(x.Te);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBtI4(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = -x.cp* x.rf *x.S*AQLMath::exp(-x.rf*x.Td)* AQLMath::pow(x.L/x.S, 2*SBnu2(x)) * AQLDist::normsdist(x.du*SBy2(x))
					+x.cp* x.rd *x.K*AQLMath::exp(-x.rd*x.Td)* AQLMath::pow(x.L/x.S, 2*SBnu1(x)) * AQLDist::normsdist( x.du*SBy2(x)- x.du*x.Vol*AQLMath::sqrt(x.Te) )
					+2.0*SBtnu2(x)* AQLMath::log(x.L/x.S) *SBI4(x)
					+x.cp * x.du * x.S * AQLMath::exp(-x.rf*x.Td) * AQLMath::pow(x.L/x.S, 2*SBnu2(x)) * diffNormdist( x.du*SBy2(x)) * SBty2(x) 
					-x.cp * x.du * x.K * AQLMath::exp(-x.rd*x.Td) * AQLMath::pow(x.L/x.S, 2*SBnu1(x)) * diffNormdist( x.du*SBy2(x)- x.du*x.Vol*AQLMath::sqrt(x.Te) ) * ( SBty2(x) - x.Vol/2/AQLMath::sqrt(x.Te));
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBtI5(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = - x.rd * SBI5(x)
					 + x.du * x.R * AQLMath::exp(-x.rd*x.Td) * ( diffNormdist(x.du*SBy1(x) - x.du * x.Vol*AQLMath::sqrt(x.Te)) * (SBty1(x) - x.Vol/2.0/AQLMath::sqrt(x.Te))
																	-AQLMath::pow(x.L/x.S,2.0*SBnu1(x)) * diffNormdist(x.du*SBy2(x)-x.du*x.Vol*AQLMath::sqrt(x.Te)) * ( SBty2(x) - x.Vol/2.0/AQLMath::sqrt(x.Te)) )
					 - x.R * AQLMath::exp(-x.rd*x.Td) * AQLMath::log(x.L/x.S)*AQLMath::pow(x.L/x.S, 2.0*SBnu1(x))*2.0*SBtnu1(x) * AQLDist::normsdist( x.du*SBy2(x)- x.du*x.Vol*AQLMath::sqrt(x.Te));
		retchk;
		return ret;
		FORMULAE_END
	}

	
	double AnalyticFormulae::SBtI6(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret=0;
		if(x.ReTime)
		{
			ret = x.R * ( AQLMath::log(x.L/x.S) * AQLMath::pow( x.L/x.S, (SBmu2(x)+SBmu3(x))/x.Vol/x.Vol) /x.Vol/x.Vol * (SBtmu2(x) + SBtmu3(x)) * AQLDist::normsdist(x.du * SBz(x))
							+AQLMath::log(x.L/x.S) * AQLMath::pow( x.L/x.S, (SBmu2(x)-SBmu3(x))/x.Vol/x.Vol) /x.Vol/x.Vol * (SBtmu2(x) - SBtmu3(x)) * AQLDist::normsdist(x.du * SBz(x) - 2.0 * x.du * SBmu3(x) /x.Vol * AQLMath::sqrt(x.Te)) )
				+ x.du * x.R * ( AQLMath::pow( x.L/x.S, (SBmu2(x)+SBmu3(x))/x.Vol/x.Vol) * diffNormdist(x.du * SBz(x)) * SBtz(x)
									+AQLMath::pow( x.L/x.S, (SBmu2(x)-SBmu3(x))/x.Vol/x.Vol)* diffNormdist(x.du * SBz(x) -2.0 * x.du * SBmu3(x) /x.Vol * AQLMath::sqrt(x.Te)) * ( SBtz(x) - 2.0 * AQLMath::sqrt(x.Te)/x.Vol*SBtmu3(x) - SBmu3(x)/x.Vol/AQLMath::sqrt(x.Te)));
		}
		else if(!x.ReTime)
		{
			ret = x.R * ( AQLMath::log(x.L/x.S) * AQLMath::pow( x.L/x.S, 2.0*SBmu2(x)/x.Vol/x.Vol) /x.Vol/x.Vol * 2.0*SBtmu2(x) * AQLDist::normsdist(x.du * SBz(x)) )
				+ x.du * x.R * ( AQLMath::pow( x.L/x.S, 2.0*SBmu2(x)/x.Vol/x.Vol) * diffNormdist(x.du * SBz(x)) * SBtz(x)
									+diffNormdist(x.du * SBz(x) -2.0 * x.du * SBmu2(x) /x.Vol * AQLMath::sqrt(x.Te)) * ( SBtz(x) - 2.0 * AQLMath::sqrt(x.Te)/x.Vol*SBtmu2(x) - SBmu2(x)/x.Vol/AQLMath::sqrt(x.Te)));
		}
		retchk;
		return ret;
		FORMULAE_END
	}


	double AnalyticFormulae::SBrI1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = -SBpI1(x) - SBI1(x) * x.Td;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBrI2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = x.Td * x.cp * x.K * AQLMath::exp( - x.rd * x.Td ) * AQLDist::normsdist(x.cp * SBy1(x) - x.cp * x.Vol * AQLMath::sqrt(x.Te))
					+ x.S * AQLMath::exp( - x.rf * x.Td ) * diffNormdist(x.cp * SBy1(x)) * SBry1(x)
					- x.K * AQLMath::exp( - x.rd * x.Td ) * diffNormdist(x.cp * SBy1(x) - x.cp * x.Vol * AQLMath::sqrt(x.Te)) * SBry1(x);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBrI3(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = x.cp * x.K * x.Td * AQLMath::exp(-x.rd * x.Td) * AQLMath::pow(x.L/x.S, 2*SBnu1(x))  * AQLDist::normsdist(x.du*SBx2(x) - x.du * x.Vol * AQLMath::sqrt(x.Te))
					+2.0*AQLMath::log(x.L/x.S)*SBrnu2(x) *SBI3(x);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBrI4(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = x.cp * x.K * x.Td * AQLMath::exp(-x.rd * x.Td) * AQLMath::pow(x.L/x.S, 2*SBnu1(x))  * AQLDist::normsdist(x.du*SBy2(x) - x.du * x.Vol * AQLMath::sqrt(x.Te))
					+ 2.0 * AQLMath::log(x.L/x.S) * SBrnu2(x) * SBI4(x)
					+ x.cp * x.du * x.S * AQLMath::exp(-x.rf * x.Td) * AQLMath::pow(x.L/x.S, 2*SBnu2(x))  * diffNormdist(x.du*SBy2(x) ) * SBry2(x) 
					- x.cp * x.du * x.K * AQLMath::exp(-x.rd * x.Td) * AQLMath::pow(x.L/x.S, 2*SBnu1(x))  * diffNormdist(x.du*SBy2(x) - x.du * x.Vol * AQLMath::sqrt(x.Te)) * SBry2(x);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBrI5(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = -x.Td *SBI5(x) 
					- x.R * AQLMath::exp(-x.rd*x.Td) * AQLMath::log(x.L/x.S) * AQLMath::pow(x.L/x.S, 2*SBnu1(x)) * 2.0 * SBrnu1(x) * AQLDist::normsdist( x.du*SBy2(x)- x.du*x.Vol*AQLMath::sqrt(x.Te))
					+ x.R * AQLMath::exp(-x.rd*x.Td) * ( diffNormdist(x.du*SBy1(x)- x.du*x.Vol*AQLMath::sqrt(x.Te)) * x.du * SBry1(x) - AQLMath::pow(x.L/x.S, 2*SBnu1(x)) * diffNormdist(x.du*SBy2(x) - x.du * x.Vol * AQLMath::sqrt(x.Te)) * x.du * SBry2(x) );
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBrI6(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret=0;
		if(x.ReTime)
		{
			ret = x.R * ( AQLMath::log(x.L/x.S) * AQLMath::pow( x.L/x.S, (SBmu2(x)+SBmu3(x))/x.Vol/x.Vol) /x.Vol/x.Vol * ( SBrmu2(x) + SBrmu3(x) ) * AQLDist::normsdist(x.du * SBz(x))
							+ AQLMath::log(x.L/x.S) * AQLMath::pow( x.L/x.S, (SBmu2(x)-SBmu3(x))/x.Vol/x.Vol) /x.Vol/x.Vol * ( SBrmu2(x) - SBrmu3(x) ) * AQLDist::normsdist(x.du * SBz(x) - 2.0 * x.du * SBmu3(x) / x.Vol * AQLMath::sqrt(x.Te)) ) 
				+ x.R * ( AQLMath::pow(x.L/x.S, (SBmu2(x)+SBmu3(x))/x.Vol/x.Vol) * diffNormdist(x.du * SBz(x) ) * x.du * SBrz(x) + AQLMath::pow(x.L/x.S, (SBmu2(x)-SBmu3(x))/x.Vol/x.Vol) * diffNormdist(x.du * SBz(x) - 2.0 * x.du * SBmu3(x) / x.Vol * AQLMath::sqrt(x.Te) ) * x.du * ( SBrz(x) - 2.0 / x.Vol * AQLMath::sqrt(x.Te) * SBrmu3(x)) );
		}
		else if(!x.ReTime)
		{
			ret = x.R * AQLMath::log(x.L/x.S) * AQLMath::pow( x.L/x.S, 2.0*SBmu2(x)/x.Vol/x.Vol) /x.Vol/x.Vol * 2.0 * SBrmu2(x) * AQLDist::normsdist(x.du * SBz(x))		
				+ x.R * ( AQLMath::pow(x.L/x.S, 2.0*SBmu2(x)/x.Vol/x.Vol) * diffNormdist(x.du * SBz(x) ) * x.du * SBrz(x) + diffNormdist(x.du * SBz(x) - 2.0 * x.du * SBmu2(x) / x.Vol * AQLMath::sqrt(x.Te) ) * x.du * ( SBrz(x) - 2.0 / x.Vol * AQLMath::sqrt(x.Te) * SBrmu2(x)) );
		}
		retchk;
		return ret;
		FORMULAE_END
	}

	double AnalyticFormulae::SBpI1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = - x.S * x.Td * SBdI1(x)
			;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBpI2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =- x.Td * x.cp * x.S * AQLMath::exp( - x.rf * x.Td ) * AQLDist::normsdist(x.cp * SBy1(x))
					+ x.S * AQLMath::exp( - x.rf * x.Td ) * diffNormdist(x.cp * SBy1(x)) * SBpy1(x)
					- x.K * AQLMath::exp( - x.rd * x.Td ) * diffNormdist(x.cp * SBy1(x) - x.cp * x.Vol * AQLMath::sqrt(x.Te)) * SBpy1(x);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBpI3(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = - x.cp * x.S * x.Td * AQLMath::exp(-x.rf * x.Td) * AQLMath::pow(x.L/x.S, 2*SBnu2(x)) * AQLDist::normsdist(x.du*SBx2(x) )
					+ 2.0*AQLMath::log(x.L/x.S) *SBpnu2(x) * SBI3(x);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBpI4(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =- x.cp * x.S * x.Td * AQLMath::exp(-x.rf * x.Td) * AQLMath::pow(x.L/x.S, 2*SBnu2(x))  * AQLDist::normsdist(x.du*SBy2(x))
					+ 2.0 * AQLMath::log(x.L/x.S) * SBpnu2(x) * SBI4(x)
					+ x.cp * x.du * x.S * AQLMath::exp(-x.rf * x.Td) * AQLMath::pow(x.L/x.S, 2*SBnu2(x))  * diffNormdist(x.du*SBy2(x) ) * SBpy2(x) 
					- x.cp * x.du * x.K * AQLMath::exp(-x.rd * x.Td) * AQLMath::pow(x.L/x.S, 2*SBnu1(x))  * diffNormdist(x.du*SBy2(x) - x.du * x.Vol * AQLMath::sqrt(x.Te)) * SBpy2(x);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBpI5(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = - x.R * AQLMath::exp(-x.rd*x.Td) *  AQLMath::log(x.L/x.S) * AQLMath::pow(x.L/x.S, 2*SBnu1(x)) * 2.0 * SBpnu1(x) * AQLDist::normsdist( x.du*SBy2(x)- x.du*x.Vol*AQLMath::sqrt(x.Te))
					+ x.R * AQLMath::exp(-x.rd*x.Td) * ( diffNormdist(x.du*SBy1(x)- x.du*x.Vol*AQLMath::sqrt(x.Te)) * x.du * SBpy1(x) - AQLMath::pow(x.L/x.S, 2*SBnu1(x)) * diffNormdist(x.du*SBy2(x) - x.du * x.Vol * AQLMath::sqrt(x.Te)) * x.du * SBpy2(x) );
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBpI6(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret=0;
		if(x.ReTime)
		{
			ret = x.R * ( AQLMath::log(x.L/x.S) * AQLMath::pow( x.L/x.S, (SBmu2(x)+SBmu3(x))/x.Vol/x.Vol) /x.Vol/x.Vol * ( SBpmu2(x) + SBpmu3(x) ) * AQLDist::normsdist(x.du * SBz(x))
							+ AQLMath::log(x.L/x.S) * AQLMath::pow( x.L/x.S, (SBmu2(x)-SBmu3(x))/x.Vol/x.Vol) /x.Vol/x.Vol * ( SBpmu2(x) - SBpmu3(x) ) * AQLDist::normsdist(x.du * SBz(x) - 2.0 * x.du * SBmu3(x) / x.Vol * AQLMath::sqrt(x.Te)) ) 
				+ x.R * ( AQLMath::pow(x.L/x.S, (SBmu2(x)+SBmu3(x))/x.Vol/x.Vol) * diffNormdist(x.du * SBz(x) ) * x.du * SBpz(x) 
							+ AQLMath::pow(x.L/x.S, (SBmu2(x)-SBmu3(x))/x.Vol/x.Vol) * diffNormdist(x.du * SBz(x) - 2.0 * x.du * SBmu3(x) / x.Vol * AQLMath::sqrt(x.Te) ) * x.du * ( SBpz(x) - 2.0 / x.Vol * AQLMath::sqrt(x.Te) * SBpmu3(x)) );
		}
		else if(!x.ReTime)
		{
			ret = x.R * AQLMath::log(x.L/x.S) * AQLMath::pow( x.L/x.S, 2.0*SBmu2(x)/x.Vol/x.Vol) /x.Vol/x.Vol * 2.0 * SBpmu2(x) * AQLDist::normsdist(x.du * SBz(x))		
				+ x.R * ( AQLMath::pow(x.L/x.S, 2.0*SBmu2(x)/x.Vol/x.Vol) * diffNormdist(x.du * SBz(x) ) * x.du * SBpz(x) 
							+ diffNormdist(x.du * SBz(x) - 2.0 * x.du * SBmu2(x) / x.Vol * AQLMath::sqrt(x.Te) ) * x.du * ( SBpz(x) - 2.0 / x.Vol * AQLMath::sqrt(x.Te) * SBpmu2(x)) );
		}
		retchk;
		return ret;
		FORMULAE_END
	}


	double AnalyticFormulae::SBI1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = x.cp*x.S*AQLMath::exp(-x.rf*x.Td)*AQLDist::normsdist(x.cp*SBx1(x))
			- x.cp*x.K*AQLMath::exp(-x.rd*x.Td)*AQLDist::normsdist(x.cp*SBx1(x)-x.cp*x.Vol*AQLMath::sqrt(x.Te));
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBI2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = x.cp*x.S*AQLMath::exp(-x.rf*x.Td)*AQLDist::normsdist(x.cp*SBy1(x))
			- x.cp*x.K*AQLMath::exp(-x.rd*x.Td)*AQLDist::normsdist(x.cp*SBy1(x)-x.cp*x.Vol*AQLMath::sqrt(x.Te));
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBI3(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = x.cp*x.S*AQLMath::exp(-x.rf*x.Td)* AQLMath::pow(x.L/x.S, 2*SBnu2(x)) * AQLDist::normsdist(x.du*SBx2(x))
			-x.cp*x.K*AQLMath::exp(-x.rd*x.Td)* AQLMath::pow(x.L/x.S, 2*SBnu1(x)) * AQLDist::normsdist( x.du*SBx2(x)- x.du*x.Vol*AQLMath::sqrt(x.Te) );
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBI4(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = x.cp*x.S*AQLMath::exp(-x.rf*x.Td)* AQLMath::pow(x.L/x.S, 2*SBnu2(x)) * AQLDist::normsdist(x.du*SBy2(x))
			-x.cp*x.K*AQLMath::exp(-x.rd*x.Td)* AQLMath::pow(x.L/x.S, 2*SBnu1(x)) * AQLDist::normsdist( x.du*SBy2(x)- x.du*x.Vol*AQLMath::sqrt(x.Te) );
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBI5(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret = x.R*AQLMath::exp(-x.rd*x.Td)* ( AQLDist::normsdist(x.du*SBy1(x)- x.du*x.Vol*AQLMath::sqrt(x.Te)) 
					 - AQLMath::pow(x.L/x.S, 2*SBnu1(x)) * AQLDist::normsdist( x.du*SBy2(x)- x.du*x.Vol*AQLMath::sqrt(x.Te) ));
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBI6(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret=0;
		if(x.ReTime)
		{
			if (fabs(x.R) > 1.0e-8)
			{
				ret = x.R * (AQLMath::pow(x.L / x.S, (SBmu2(x) + SBmu3(x)) / x.Vol / x.Vol) * AQLDist::normsdist(x.du * SBz(x))
					+ AQLMath::pow(x.L / x.S, (SBmu2(x) - SBmu3(x)) / x.Vol / x.Vol) * AQLDist::normsdist(x.du * SBz(x) - 2 * x.du * SBmu3(x) / x.Vol * AQLMath::sqrt(x.Te)));
			}
		}
		else if(!x.ReTime)
		{
			if (fabs(x.R) > 1.0e-8)
			{
				ret = x.R * AQLMath::exp(-x.rd * x.Td) *  (AQLMath::pow(x.L / x.S, 2.0 *SBmu2(x) / x.Vol / x.Vol) * AQLDist::normsdist(x.du * SBz(x))
					+ AQLDist::normsdist(x.du * SBz(x) - 2.0 * x.du * SBmu2(x) / x.Vol * AQLMath::sqrt(x.Te)));
			}
		}
		retchk;
		return ret;
		FORMULAE_END
	}

	double AnalyticFormulae::SBpremDIC	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S>=x.L && x.K<=x.L)
		{
			ret = SBI1(x) - SBI2(x) + SBI4(x) + SBI5(x);
		}
		else if(x.S>=x.L && x.K>=x.L)
		{
			ret = SBI3(x)+SBI5(x);
		}
		else if(x.S <= x.L )
		{
			ret = GKpremCall(x);
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBpremDIP	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S>=x.L && x.K<=x.L)
		{
			ret = SBI1(x) + SBI5(x);
		}
		else if(x.S>=x.L && x.K>=x.L)
		{
			ret = SBI2(x) - SBI3(x) + SBI4(x) + SBI5(x);
		}
		else if(x.S <= x.L )
		{
			ret = GKpremPut(x);
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBpremDOC	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S>=x.L && x.K<=x.L)
		{
			ret = SBI2(x) - SBI4(x) + SBI6(x);
		}
		else if(x.S>=x.L && x.K>=x.L)
		{
			ret = SBI1(x) - SBI3(x) + SBI6(x);
		}
		else if(x.S <= x.L )
		{
			ret = x.R;
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBpremDOP	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S>=x.L && x.K<=x.L)
		{
			ret = SBI6(x);
		}
		else if(x.S>=x.L && x.K>=x.L)
		{
			ret = SBI1(x) - SBI2(x) + SBI3(x) - SBI4(x) + SBI6(x);
		}
		else if(x.S <= x.L )
		{
			ret = x.R;
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBpremUIC	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S<=x.L && x.K<=x.L)
		{
			ret = SBI2(x) - SBI3(x) + SBI4(x) + SBI5(x);
		}
		else if(x.S<=x.L && x.K>=x.L)
		{
			ret = SBI1(x) + SBI5(x);
		}
		else if(x.S >= x.L )
		{
			ret = GKpremCall(x);
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBpremUIP	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S<=x.L && x.K<=x.L)
		{
			ret = SBI3(x) + SBI5(x);
		}
		else if(x.S<=x.L && x.K>=x.L)
		{
			ret = SBI1(x) - SBI2(x) + SBI4(x) + SBI5(x);
		}
		else if(x.S >= x.L )
		{
			ret = GKpremPut(x);
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBpremUOC	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S<=x.L && x.K<=x.L)
		{
			ret = SBI1(x) - SBI2(x) + SBI3(x) - SBI4(x) + SBI6(x);
		}
		else if(x.S<=x.L && x.K>=x.L)
		{
			ret = SBI6(x);
		}
		else if(x.S >= x.L )
		{
			ret = x.R;
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBpremUOP	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S<=x.L && x.K<=x.L)
		{
			ret = SBI1(x) - SBI3(x) + SBI6(x);
		}
		else if(x.S<=x.L && x.K>=x.L)
		{
			ret = SBI2(x) - SBI4(x) + SBI6(x);
		}
		else if(x.S >= x.L )
		{
			ret = x.R;
		}
		retchk;
		return ret;
		FORMULAE_END
	}


	double AnalyticFormulae::SBdeltaDIC	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S>=x.L && x.K<=x.L)
		{
			ret = SBdI1(x) - SBdI2(x) + SBdI4(x) + SBdI5(x);
		}
		else if(x.S>=x.L && x.K>=x.L)
		{
			ret = SBdI3(x)+SBdI5(x);
		}
		else if(x.S <= x.L )
		{
			ret = GKdeltaCall(x);
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBdeltaDIP	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S>=x.L && x.K<=x.L)
		{
			ret = SBdI1(x) + SBdI5(x);
		}
		else if(x.S>=x.L && x.K>=x.L)
		{
			ret = SBdI2(x) - SBdI3(x) + SBdI4(x) + SBdI5(x);
		}
		else if(x.S <= x.L )
		{
			ret = GKdeltaPut(x);
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBdeltaDOC	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S>=x.L && x.K<=x.L)
		{
			ret = SBdI2(x) - SBdI4(x) + SBdI6(x);
		}
		else if(x.S>=x.L && x.K>=x.L)
		{
			ret = SBdI1(x) - SBdI3(x) + SBdI6(x);
		}
		else if(x.S <= x.L )
		{
			ret = 0;
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBdeltaDOP	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S>=x.L && x.K<=x.L)
		{
			ret = SBdI6(x);
		}
		else if(x.S>=x.L && x.K>=x.L)
		{
			ret = SBdI1(x) - SBdI2(x) + SBdI3(x) - SBdI4(x) + SBdI6(x);
		}
		else if(x.S <= x.L )
		{
			ret = 0;
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBdeltaUIC	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S<=x.L && x.K<=x.L)
		{
			ret = SBdI2(x) - SBdI3(x) + SBdI4(x) + SBdI5(x);
		}
		else if(x.S<=x.L && x.K>=x.L)
		{
			ret = SBdI1(x) + SBdI5(x);
		}
		else if(x.S >= x.L )
		{
			ret = GKdeltaCall(x);
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBdeltaUIP	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S<=x.L && x.K<=x.L)
		{
			ret = SBdI3(x) + SBdI5(x);
		}
		else if(x.S<=x.L && x.K>=x.L)
		{
			ret = SBdI1(x) - SBdI2(x) + SBdI4(x) + SBdI5(x);
		}
		else if(x.S >= x.L )
		{
			ret = GKdeltaPut(x);
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBdeltaUOC	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S<=x.L && x.K<=x.L)
		{
			ret = SBdI1(x) - SBdI2(x) + SBdI3(x) - SBdI4(x) + SBdI6(x);
		}
		else if(x.S<=x.L && x.K>=x.L)
		{
			ret = SBdI6(x);
		}
		else if(x.S >= x.L )
		{
			ret = 0;
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBdeltaUOP	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S<=x.L && x.K<=x.L)
		{
			ret = SBdI1(x) - SBdI3(x) + SBdI6(x);
		}
		else if(x.S<=x.L && x.K>=x.L)
		{
			ret = SBdI2(x) - SBdI4(x) + SBdI6(x);
		}
		else if(x.S >= x.L )
		{
			ret = 0;
		}
		retchk;
		return ret;
		FORMULAE_END
	}


	double AnalyticFormulae::SBgammaDIC	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S>=x.L && x.K<=x.L)
		{
			ret = SBgI1(x) - SBgI2(x) + SBgI4(x) + SBgI5(x);
		}
		else if(x.S>=x.L && x.K>=x.L)
		{
			ret = SBgI3(x)+SBgI5(x);
		}
		else if(x.S <= x.L )
		{
			ret = GKgammaCall(x);
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBgammaDIP	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S>=x.L && x.K<=x.L)
		{
			ret = SBgI1(x) + SBgI5(x);
		}
		else if(x.S>=x.L && x.K>=x.L)
		{
			ret = SBgI2(x) - SBgI3(x) + SBgI4(x) + SBgI5(x);
		}
		else if(x.S <= x.L )
		{
			ret = GKgammaPut(x);
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBgammaDOC	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S>=x.L && x.K<=x.L)
		{
			ret = SBgI2(x) - SBgI4(x) + SBgI6(x);
		}
		else if(x.S>=x.L && x.K>=x.L)
		{
			ret = SBgI1(x) - SBgI3(x) + SBgI6(x);
		}
		else if(x.S <= x.L )
		{
			ret = 0;
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBgammaDOP	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S>=x.L && x.K<=x.L)
		{
			ret = SBgI6(x);
		}
		else if(x.S>=x.L && x.K>=x.L)
		{
			ret = SBgI1(x) - SBgI2(x) + SBgI3(x) - SBgI4(x) + SBgI6(x);
		}
		else if(x.S <= x.L )
		{
			ret = 0;
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBgammaUIC	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S<=x.L && x.K<=x.L)
		{
			ret = SBgI2(x) - SBgI3(x) + SBgI4(x) + SBgI5(x);
		}
		else if(x.S<=x.L && x.K>=x.L)
		{
			ret = SBgI1(x) + SBgI5(x);
		}
		else if(x.S >= x.L )
		{
			ret = GKgammaCall(x);
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBgammaUIP	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S<=x.L && x.K<=x.L)
		{
			ret = SBgI3(x) + SBgI5(x);
		}
		else if(x.S<=x.L && x.K>=x.L)
		{
			ret = SBgI1(x) - SBgI2(x) + SBgI4(x) + SBgI5(x);
		}
		else if(x.S >= x.L )
		{
			ret = GKpremPut(x);
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBgammaUOC	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S<=x.L && x.K<=x.L)
		{
			ret = SBgI1(x) - SBgI2(x) + SBgI3(x) - SBgI4(x) + SBgI6(x);
		}
		else if(x.S<=x.L && x.K>=x.L)
		{
			ret = SBgI6(x);
		}
		else if(x.S >= x.L )
		{
			ret = 0;
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBgammaUOP	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S<=x.L && x.K<=x.L)
		{
			ret = SBgI1(x) - SBgI3(x) + SBgI6(x);
		}
		else if(x.S<=x.L && x.K>=x.L)
		{
			ret = SBgI2(x) - SBgI4(x) + SBgI6(x);
		}
		else if(x.S >= x.L )
		{
			ret = 0;
		}
		retchk;
		return ret;
		FORMULAE_END
	}

	double AnalyticFormulae::SBvegaDIC	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S>=x.L && x.K<=x.L)
		{
			ret = SBvI1(x) - SBvI2(x) + SBvI4(x) + SBvI5(x);
		}
		else if(x.S>=x.L && x.K>=x.L)
		{
			ret = SBvI3(x)+SBvI5(x);
		}
		else if(x.S <= x.L )
		{
			ret = GKvegaCall(x);
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBvegaDIP	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S>=x.L && x.K<=x.L)
		{
			ret = SBvI1(x) + SBvI5(x);
		}
		else if(x.S>=x.L && x.K>=x.L)
		{
			ret = SBvI2(x) - SBvI3(x) + SBvI4(x) + SBvI5(x);
		}
		else if(x.S <= x.L )
		{
			ret = GKvegaPut(x);
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBvegaDOC	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S>=x.L && x.K<=x.L)
		{
			ret = SBvI2(x) - SBvI4(x) + SBvI6(x);
		}
		else if(x.S>=x.L && x.K>=x.L)
		{
			ret = SBvI1(x) - SBvI3(x) + SBvI6(x);
		}
		else if(x.S <= x.L )
		{
			ret = 0;
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBvegaDOP	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S>=x.L && x.K<=x.L)
		{
			ret = SBvI6(x);
		}
		else if(x.S>=x.L && x.K>=x.L)
		{
			ret = SBvI1(x) - SBvI2(x) + SBvI3(x) - SBvI4(x) + SBvI6(x);
		}
		else if(x.S <= x.L )
		{
			ret = 0;
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBvegaUIC	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S<=x.L && x.K<=x.L)
		{
			ret = SBvI2(x) - SBvI3(x) + SBvI4(x) + SBvI5(x);
		}
		else if(x.S<=x.L && x.K>=x.L)
		{
			ret = SBvI1(x) + SBvI5(x);
		}
		else if(x.S >= x.L )
		{
			ret = GKvegaCall(x);
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBvegaUIP	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S<=x.L && x.K<=x.L)
		{
			ret = SBvI3(x) + SBvI5(x);
		}
		else if(x.S<=x.L && x.K>=x.L)
		{
			ret = SBvI1(x) - SBvI2(x) + SBvI4(x) + SBvI5(x);
		}
		else if(x.S >= x.L )
		{
			ret = GKvegaPut(x);
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBvegaUOC	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S<=x.L && x.K<=x.L)
		{
			ret = SBvI1(x) - SBvI2(x) + SBvI3(x) - SBvI4(x) + SBvI6(x);
		}
		else if(x.S<=x.L && x.K>=x.L)
		{
			ret = SBvI6(x);
		}
		else if(x.S >= x.L )
		{
			ret = 0;
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBvegaUOP	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S<=x.L && x.K<=x.L)
		{
			ret = SBvI1(x) - SBvI3(x) + SBvI6(x);
		}
		else if(x.S<=x.L && x.K>=x.L)
		{
			ret = SBvI2(x) - SBvI4(x) + SBvI6(x);
		}
		else if(x.S >= x.L )
		{
			ret = 0;
		}
		retchk;
		return ret;
		FORMULAE_END
	}

	double AnalyticFormulae::SBthetaDIC	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S>=x.L && x.K<=x.L)
		{
			ret = -( SBtI1(x) - SBtI2(x) + SBtI4(x) + SBtI5(x) );
		}
		else if(x.S>=x.L && x.K>=x.L)
		{
			ret = -( SBtI3(x) + SBtI5(x) );
		}
		else if(x.S <= x.L )
		{
			ret = GKthetaCall(x);
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBthetaDIP	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S>=x.L && x.K<=x.L)
		{
			ret = -(SBtI1(x) + SBtI5(x));
		}
		else if(x.S>=x.L && x.K>=x.L)
		{
			ret = -( SBtI2(x) - SBtI3(x) + SBtI4(x) + SBtI5(x) );
		}
		else if(x.S <= x.L )
		{
			ret = GKthetaPut(x);
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBthetaDOC	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S>=x.L && x.K<=x.L)
		{
			ret = -( SBtI2(x) - SBtI4(x) + SBtI6(x) );
		}
		else if(x.S>=x.L && x.K>=x.L)
		{
			ret = -( SBtI1(x) - SBtI3(x) + SBtI6(x) );
		}
		else if(x.S <= x.L )
		{
			ret = 0;
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBthetaDOP	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S>=x.L && x.K<=x.L)
		{
			ret = - SBtI6(x);
		}
		else if(x.S>=x.L && x.K>=x.L)
		{
			ret = -( SBtI1(x) - SBtI2(x) + SBtI3(x) - SBtI4(x) + SBtI6(x) );
		}
		else if(x.S <= x.L )
		{
			ret = 0;
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBthetaUIC	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S<=x.L && x.K<=x.L)
		{
			ret = -( SBtI2(x) - SBtI3(x) + SBtI4(x) + SBtI5(x) );
		}
		else if(x.S<=x.L && x.K>=x.L)
		{
			ret = -( SBtI1(x) + SBtI5(x) );
		}
		else if(x.S >= x.L )
		{
			ret = GKthetaCall(x);
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBthetaUIP	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S<=x.L && x.K<=x.L)
		{
			ret = -( SBtI3(x) + SBtI5(x) );
		}
		else if(x.S<=x.L && x.K>=x.L)
		{
			ret = -( SBtI1(x) - SBtI2(x) + SBtI4(x) + SBtI5(x) );
		}
		else if(x.S >= x.L )
		{
			ret = GKthetaPut(x);
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBthetaUOC	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S<=x.L && x.K<=x.L)
		{
			ret = -( SBtI1(x) - SBtI2(x) + SBtI3(x) - SBtI4(x) + SBtI6(x) );
		}
		else if(x.S<=x.L && x.K>=x.L)
		{
			ret = - SBtI6(x);
		}
		else if(x.S >= x.L )
		{
			ret = 0;
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBthetaUOP	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S<=x.L && x.K<=x.L)
		{
			ret = -( SBtI1(x) - SBtI3(x) + SBtI6(x) );
		}
		else if(x.S<=x.L && x.K>=x.L)
		{
			ret = -( SBtI2(x) - SBtI4(x) + SBtI6(x) );
		}
		else if(x.S >= x.L )
		{
			ret = 0;
		}
		retchk;
		return ret;
		FORMULAE_END
	}

	double AnalyticFormulae::SBrhoDIC	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S>=x.L && x.K<=x.L)
		{
			ret = SBrI1(x) - SBrI2(x) + SBrI4(x) + SBrI5(x);
		}
		else if(x.S>=x.L && x.K>=x.L)
		{
			ret = SBrI3(x)+SBrI5(x);
		}
		else if(x.S <= x.L )
		{
			ret = GKrhoCall(x);
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBrhoDIP	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S>=x.L && x.K<=x.L)
		{
			ret = SBrI1(x) + SBrI5(x);
		}
		else if(x.S>=x.L && x.K>=x.L)
		{
			ret = SBrI2(x) - SBrI3(x) + SBrI4(x) + SBrI5(x);
		}
		else if(x.S <= x.L )
		{
			ret = GKrhoPut(x);
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBrhoDOC	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S>=x.L && x.K<=x.L)
		{
			ret = SBrI2(x) - SBrI4(x) + SBrI6(x);
		}
		else if(x.S>=x.L && x.K>=x.L)
		{
			ret = SBrI1(x) - SBrI3(x) + SBrI6(x);
		}
		else if(x.S <= x.L )
		{
			ret = 0;
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBrhoDOP	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S>=x.L && x.K<=x.L)
		{
			ret = SBrI6(x);
		}
		else if(x.S>=x.L && x.K>=x.L)
		{
			ret = SBrI1(x) - SBrI2(x) + SBrI3(x) - SBrI4(x) + SBrI6(x);
		}
		else if(x.S <= x.L )
		{
			ret = 0;
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBrhoUIC	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S<=x.L && x.K<=x.L)
		{
			ret = SBrI2(x) - SBrI3(x) + SBrI4(x) + SBrI5(x);
		}
		else if(x.S<=x.L && x.K>=x.L)
		{
			ret = SBrI1(x) + SBrI5(x);
		}
		else if(x.S >= x.L )
		{
			ret = GKrhoCall(x);
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBrhoUIP	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S<=x.L && x.K<=x.L)
		{
			ret = SBrI3(x) + SBrI5(x);
		}
		else if(x.S<=x.L && x.K>=x.L)
		{
			ret = SBrI1(x) - SBrI2(x) + SBrI4(x) + SBrI5(x);
		}
		else if(x.S >= x.L )
		{
			ret = GKrhoPut(x);
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBrhoUOC	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S<=x.L && x.K<=x.L)
		{
			ret = SBrI1(x) - SBrI2(x) + SBrI3(x) - SBrI4(x) + SBrI6(x);
		}
		else if(x.S<=x.L && x.K>=x.L)
		{
			ret = SBrI6(x);
		}
		else if(x.S >= x.L )
		{
			ret = 0;
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBrhoUOP	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S<=x.L && x.K<=x.L)
		{
			ret = SBrI1(x) - SBrI3(x) + SBrI6(x);
		}
		else if(x.S<=x.L && x.K>=x.L)
		{
			ret = SBrI2(x) - SBrI4(x) + SBrI6(x);
		}
		else if(x.S >= x.L )
		{
			ret = 0;
		}
		retchk;
		return ret;
		FORMULAE_END
	}

	double AnalyticFormulae::SBphiDIC	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S>=x.L && x.K<=x.L)
		{
			ret = SBpI1(x) - SBpI2(x) + SBpI4(x) + SBpI5(x);
		}
		else if(x.S>=x.L && x.K>=x.L)
		{
			ret = SBpI3(x)+SBpI5(x);
		}
		else if(x.S <= x.L )
		{
			ret = GKphiCall(x);
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBphiDIP	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S>=x.L && x.K<=x.L)
		{
			ret = SBpI1(x) + SBpI5(x);
		}
		else if(x.S>=x.L && x.K>=x.L)
		{
			ret = SBpI2(x) - SBpI3(x) + SBpI4(x) + SBpI5(x);
		}
		else if(x.S <= x.L )
		{
			ret = GKphiPut(x);
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBphiDOC	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S>=x.L && x.K<=x.L)
		{
			ret = SBpI2(x) - SBpI4(x) + SBpI6(x);
		}
		else if(x.S>=x.L && x.K>=x.L)
		{
			ret = SBpI1(x) - SBpI3(x) + SBpI6(x);
		}
		else if(x.S <= x.L )
		{
			ret = 0;
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBphiDOP	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S>=x.L && x.K<=x.L)
		{
			ret = SBpI6(x);
		}
		else if(x.S>=x.L && x.K>=x.L)
		{
			ret = SBpI1(x) - SBpI2(x) + SBpI3(x) - SBpI4(x) + SBpI6(x);
		}
		else if(x.S <= x.L )
		{
			ret = 0;
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBphiUIC	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S<=x.L && x.K<=x.L)
		{
			ret = SBpI2(x) - SBpI3(x) + SBpI4(x) + SBpI5(x);
		}
		else if(x.S<=x.L && x.K>=x.L)
		{
			ret = SBpI1(x) + SBpI5(x);
		}
		else if(x.S >= x.L )
		{
			ret = GKphiCall(x);
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBphiUIP	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S<=x.L && x.K<=x.L)
		{
			ret = SBpI3(x) + SBpI5(x);
		}
		else if(x.S<=x.L && x.K>=x.L)
		{
			ret = SBpI1(x) - SBpI2(x) + SBpI4(x) + SBpI5(x);
		}
		else if(x.S >= x.L )
		{
			ret = GKphiPut(x);
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBphiUOC	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S<=x.L && x.K<=x.L)
		{
			ret = SBpI1(x) - SBpI2(x) + SBpI3(x) - SBpI4(x) + SBpI6(x);
		}
		else if(x.S<=x.L && x.K>=x.L)
		{
			ret = SBpI6(x);
		}
		else if(x.S >= x.L )
		{
			ret = 0;
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::SBphiUOP	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticSBParam& x = dynamic_cast<AnalyticSBParam& >(param);
		double ret =0;
		if(x.S<=x.L && x.K<=x.L)
		{
			ret = SBpI1(x) - SBpI3(x) + SBpI6(x);
		}
		else if(x.S<=x.L && x.K>=x.L)
		{
			ret = SBpI2(x) - SBpI4(x) + SBpI6(x);
		}
		else if(x.S >= x.L )
		{
			ret = 0;
		}
		retchk;
		return ret;
		FORMULAE_END
	}


/*!
	//////DoubleBarrier option Formula
	S		spot price
	K		strike 
	Vol	volatility
	Td	delivery terms
	Te	expiry terms
	rd	local rate
	rf	foreign rate
	Ll	Limit low value
	Lh	Limit high value
	Rl	Rebate low value(when receive Knockout)
	Rh	Rebate high value(when receive Knockout)
	Ri	Rebate value(when receive "KnockIn")
	RelTime   time at Barrier Reach RelTime=1, time at DeliveryDate RelTime=0 (Defalut RelTime ==1)
	RehTime	time at Barrier Reach RehTime=1, time at DeliveryDate RehTime=0 (Defalut RehTime ==1)
	Num is number of sum used for Double Barrier
*/

	double AnalyticFormulae::DBmu1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticDBParam& x = dynamic_cast<AnalyticDBParam& >(param);
		double ret =  x.rd-x.rf;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::DBmu2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticDBParam& x = dynamic_cast<AnalyticDBParam& >(param);
		double ret = 	DBmu1(x)*x.Td/x.Te - 0.5*x.Vol*x.Vol;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::DBmu3(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticDBParam& x = dynamic_cast<AnalyticDBParam& >(param);
		double ret =	AQLMath::sqrt( DBmu2(x)*DBmu2(x) + 2.0*x.rd*x.Vol*x.Vol);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::x0	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticDBParam& x = dynamic_cast<AnalyticDBParam& >(param);
		double ret = AQLMath::log(x.K/x.S);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::xl	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticDBParam& x = dynamic_cast<AnalyticDBParam& >(param);
		double ret = AQLMath::log(x.Ll/x.S);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::xh	(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticDBParam& x = dynamic_cast<AnalyticDBParam& >(param);
		double ret = AQLMath::log(x.Lh/x.S);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::Phi(double z1, double z2, double b, double c, double d)
	{
		double ret = AQLMath::exp(0.5*d*(d*c*c-2.0*b))*( AQLDist::normsdist(c*d-(b+z1)/c)-AQLDist::normsdist(c*d-(b+z2)/c) );
		retchk;
		return ret;
	}
	double AnalyticFormulae::u(int k, double xh, double xl)
	{
		double ret = 2.0*k*(xh - xl) ;
		retchk;
		return ret;
	}

	double AnalyticFormulae::F(double t, double mu, double vol, double xl, double xh, double nu, double z1, double z2, int sumrange)
	{
		double ret    = 0.0;
		double bfret  = 0.0;
		double alpha1 = max(z1, xl);
		double alpha2 = min(z2, xh);
		for(int steps = 0; steps < sumrange+1;steps++)
		{
			bfret = ret;
			ret   = 0.0;
			for(int k=-steps; k< steps+1; k++)
			{
				ret += AQLMath::exp(-mu* u(k,xh,xl) /vol/vol) * Phi(alpha1, alpha2, -mu *t +u(k, xh, xl) , vol*AQLMath::sqrt(t), nu) 
					 - AQLMath::exp(mu/vol/vol* (2*xh-u(k,xh,xl) ) ) * Phi(alpha1, alpha2, -mu*t-2*xh+u(k,xh,xl), vol*AQLMath::sqrt(t), nu) ;
			}
			if(AQLMath::abs(ret-bfret) <= 0.0000000001 && steps !=0) break;
		}
		if(AQLMath::abs(ret-bfret)>0.0000000001 ) throw AQLCoreNumericalError("Not Convergence",__FILE__,__LINE__);
		retchk;
		return ret;
	}
	double AnalyticFormulae::Gh(double t, double mu, double vol, double xl, double xh, int sumrange)
	{
		double ret    = 0.0;
		double bfret  = 0.0;
		for(int steps = 0; steps < sumrange+1;steps++)
		{
			bfret = ret;	
			ret   = 0.0;
			for(int k=0; k<steps+1; k++)
			{
				double uh = xh + u(k, xh, xl);
				ret += AQLMath::exp(mu*uh/vol/vol)
					  * AQLDist::normsdist( (-uh-mu*t)/vol/AQLMath::sqrt(t) ) 
					  + AQLMath::exp(-mu*uh/vol/vol) * AQLDist::normsdist( (-uh+mu*t)/vol/AQLMath::sqrt(t) ) ;
			}

			for(int k=-steps ;k<0; k++)
			{
				double uh = xh + u(k, xh, xl);
				ret -= AQLMath::exp(mu*uh/vol/vol)*AQLDist::normsdist( (uh+mu*t)/vol/AQLMath::sqrt(t)) 
					+ AQLMath::exp(-mu*uh/vol/vol)*AQLDist::normsdist( (uh-mu*t)/vol/AQLMath::sqrt(t)) ;
			}
			ret *= AQLMath::exp(mu*xh/vol/vol);
			if(AQLMath::abs(ret-bfret) <= 0.0000000001 && steps !=0) break;
		}
		if(AQLMath::abs(ret-bfret)>0.0000000001 ) throw AQLCoreNumericalError("Not Convergence",__FILE__,__LINE__);
		retchk;
		return ret;
	}
	double AnalyticFormulae::Gl(double t, double mu, double vol, double xl, double xh, int sumrange)
	{
		double ret    = 0.0;
		double bfret  = 0.0;
		for(int steps = 0; steps < sumrange+1;steps++)
		{
			bfret = ret;
			for(int k=0; k<steps+1; k++)
			{
				double ul = -xl + u(k, xh, xl);
				ret += AQLMath::exp(mu*ul/vol/vol)*AQLDist::normsdist( (-ul-mu*t)/vol/AQLMath::sqrt(t)) 
					+ AQLMath::exp(-mu*ul/vol/vol)*AQLDist::normsdist( (-ul+mu*t)/vol/AQLMath::sqrt(t)) ;
			}
			for(int k=-steps ;k<0; k++)
			{
				double ul = -xl + u(k, xh, xl);
				ret -= AQLMath::exp(mu*ul/vol/vol)*AQLDist::normsdist( (ul+mu*t)/vol/AQLMath::sqrt(t)) 
					+ AQLMath::exp(-mu*ul/vol/vol)*AQLDist::normsdist( (ul-mu*t)/vol/AQLMath::sqrt(t)) ;
			}
			ret *= AQLMath::exp(mu*xl/vol/vol);
			if(AQLMath::abs(ret-bfret) <= 0.0000000001 && steps !=0) break;
		}
		if(AQLMath::abs(ret-bfret)>0.0000000001 ) throw AQLCoreNumericalError("Not Convergence",__FILE__,__LINE__);
		retchk;
		return ret;
	}

	double AnalyticFormulae::DBpremKOC(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticDBParam& x = dynamic_cast<AnalyticDBParam& >(param);
		double ret = AQLMath::exp(-x.rd*x.Td) *
			( x.S * F(x.Te, DBmu2(x), x.Vol, xl(x), xh(x), 1.0, x0(x), xh(x), x.Num)
			- x.K * F(x.Te, DBmu2(x), x.Vol, xl(x), xh(x), 0.0, x0(x), xh(x), x.Num)
			);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::DBpremKIC(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticDBParam& x = dynamic_cast<AnalyticDBParam& >(param);
		double ret = GKpremCall(x) - DBpremKOC(x);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::DBpremKOP(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticDBParam& x = dynamic_cast<AnalyticDBParam& >(param);
		double ret = AQLMath::exp(-x.rd * x.Td ) *
			( x.S * F(x.Te, DBmu2(x), x.Vol, xl(x), xh(x), 1.0, xl(x), x0(x), x.Num)
			- x.K * F(x.Te, DBmu2(x), x.Vol, xl(x), xh(x), 0.0, xl(x), x0(x), x.Num)
			);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::DBpremKIP(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticDBParam& x = dynamic_cast<AnalyticDBParam& >(param);
		double ret = GKpremPut(x) - DBpremKOP(x);
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::DBpremRL(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticDBParam& x = dynamic_cast<AnalyticDBParam& >(param);
		double ret=0.0;
		if(x.RelTime)
		{
			ret = x.Rl * AQLMath::exp( xl(x) * (DBmu2(x)-DBmu3(x) ) / x.Vol /x.Vol)  * Gl(x.Te, DBmu3(x), x.Vol, xl(x), xh(x), x.Num);
		}
		else 
		{
			ret = x.Rl * AQLMath::exp(-x.rd * x.Td )  * Gl(x.Te, DBmu2(x), x.Vol, xl(x), xh(x), x.Num);
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::DBpremRH(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticDBParam& x = dynamic_cast<AnalyticDBParam& >(param);
		double ret=0.0;
		if(x.RehTime)
		{
			ret = x.Rh * AQLMath::exp( xl(x) * (DBmu2(x)-DBmu3(x) ) / x.Vol /x.Vol)  * Gh(x.Te, DBmu3(x), x.Vol, xl(x), xh(x), x.Num);
		}
		else 
		{
			ret = x.Rh * AQLMath::exp(- x.rd * x.Td ) * Gh(x.Te, DBmu2(x), x.Vol, xl(x), xh(x), x.Num);
		}
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::DBpremRI(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticDBParam& x = dynamic_cast<AnalyticDBParam& >(param);
		double ret = x.Ri * AQLMath::exp(-x.rd * x.Td) * F(x.Te,DBmu2(x), x.Vol, xl(x), xh(x), 0.0, xl(x), xh(x), x.Num);
		retchk;
		return ret;
		FORMULAE_END
	}

	//option calculation for fwddelta
	double AnalyticFormulae::FDd1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticGKParam& x = dynamic_cast<AnalyticGKParam& >(param);
		double ret = ( AQLMath::log(x.F/x.K) + 0.5*x.Vol*x.Vol*x.Te) / (x.Vol* AQLMath::sqrt( x.Te) ) ; 	
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::FDd2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticGKParam& x = dynamic_cast<AnalyticGKParam& >(param);
		double ret = FDd1(x) - x.Vol*AQLMath::sqrt(x.Te);	
		retchk;
		return ret;
		FORMULAE_END
	}

	double AnalyticFormulae::FDdeltaCallSpot(AnalyticParam& param)
	{
		FORMULAE_BEGIN    
		AnalyticGKParam& x = dynamic_cast<AnalyticGKParam& >(param);
		double ret = x.DFf * x.K/x.F*AQLDist::normsdist(FDd2(x));
		retchk;      
		return ret;  
		FORMULAE_END 
	}

	double AnalyticFormulae::FDdeltaPutSpot(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticGKParam& x = dynamic_cast<AnalyticGKParam& >(param);
		double ret = - x.DFf * x.K/x.F*AQLDist::normsdist(-FDd2(x));
		retchk;
		return ret;
		FORMULAE_END
	}

	//first order differential of FWDDelta used for Newton Raphson 
	double AnalyticFormulae::FD1stDiffCallSpot(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticGKParam& x = dynamic_cast<AnalyticGKParam& >(param);
		double ret = x.DFd/x.S * (  AQLDist::normsdist(FDd2(x)) - diffNormdist(FDd2(x))/x.Vol/AQLMath::sqrt(x.Te) );
		retchk;
		return ret;
		FORMULAE_END
	}
	//first order differential of FWDDelta used for Newton Raphson 
	double AnalyticFormulae::FD1stDiffPutSpot(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticGKParam& x = dynamic_cast<AnalyticGKParam& >(param);
		double ret = x.DFd /x.S * (  - AQLDist::normsdist(-FDd2(x)) - diffNormdist(-FDd2(x))/x.Vol/AQLMath::sqrt(x.Te) );
		retchk;
		return ret;
		FORMULAE_END
	}

		double AnalyticFormulae::FDdeltaCallFwd(AnalyticParam& param)
	{
		FORMULAE_BEGIN    
		AnalyticGKParam& x = dynamic_cast<AnalyticGKParam& >(param);
		double ret = x.K/x.F*AQLDist::normsdist(FDd2(x));
		retchk;      
		return ret;  
		FORMULAE_END 
	}

	double AnalyticFormulae::FDdeltaPutFwd(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticGKParam& x = dynamic_cast<AnalyticGKParam& >(param);
		double ret = -x.K/x.F*AQLDist::normsdist(-FDd2(x));
		retchk;
		return ret;
		FORMULAE_END
	}
	//first order differential of FWDDelta used for Newton Raphson 
	double AnalyticFormulae::FD1stDiffCallFwd(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticGKParam& x = dynamic_cast<AnalyticGKParam& >(param);
		double ret = 1.0/x.F * ( AQLDist::normsdist(FDd2(x)) - diffNormdist(FDd2(x))/x.Vol/AQLMath::sqrt(x.Te) );
		retchk;
		return ret;
		FORMULAE_END
	}
	//first order differential of FwdDelta used for Newton Raphson 
	double AnalyticFormulae::FD1stDiffPutFwd(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticGKParam& x = dynamic_cast<AnalyticGKParam& >(param);
		double ret = 1.0/x.F * (  - AQLDist::normsdist(-FDd2(x)) - diffNormdist(-FDd2(x))/x.Vol/AQLMath::sqrt(x.Te) );
		retchk;
		return ret;
		FORMULAE_END
	}
   
    //second order differential of SpotDelta used for Newton Raphson
    double AnalyticFormulae::FD2ndDiffCallSpot(AnalyticParam& param)
    {
        FORMULAE_BEGIN
        AnalyticGKParam& x = dynamic_cast<AnalyticGKParam&> (param);
        double ret =  FD2ndDiffCallFwd(x) * x.DFf ; 
		retchk;
		return ret;
		FORMULAE_END
    }
    //second order differential of FWDDelta used for Newton Raphson
    double AnalyticFormulae::FD2ndDiffCallFwd(AnalyticParam& param)
    {
        FORMULAE_BEGIN
        AnalyticGKParam& x = dynamic_cast<AnalyticGKParam&> (param);
        double ret = - AQLDist::normsdist(FDd2(x)) / (x.F * x.K ) * (1.0 + FDd2(x)/x.Vol/AQLMath::sqrt(x.Te) ); 
		retchk;
		return ret;
		FORMULAE_END
    }

/*!
	AFFineModel Formula
	K		strike
	Vol		volatility
	Pbondm	Zero Bond(underlying)
	Poptm	Zero Bond Price whose maturity is option maturity
*/
	
	double AnalyticFormulae::AFFd1(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticAFFParam& x = dynamic_cast<AnalyticAFFParam& >(param);
		double ret = AQLMath::log(x.Pbondm/x.Poptm/x.K)/x.Vol + 0.5* x.Vol;
		retchk;
		return ret;
		FORMULAE_END
	}
	double AnalyticFormulae::AFFd2(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticAFFParam& x = dynamic_cast<AnalyticAFFParam& >(param);
		double ret = AFFd1(x) - x.Vol;
		retchk;
		return ret;
		FORMULAE_END
	}

	double AnalyticFormulae::AFFpremCall(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticAFFParam& x = dynamic_cast<AnalyticAFFParam& >(param);
		double ret = x.Pbondm * AQLDist::normsdist(AFFd1(x)) - x.K * x.Poptm * AQLDist::normsdist(AFFd2(x));
		retchk;
		return ret;
		FORMULAE_END

	}
	double AnalyticFormulae::AFFpremPut(AnalyticParam& param)
	{
		FORMULAE_BEGIN
		AnalyticAFFParam& x = dynamic_cast<AnalyticAFFParam& >(param);
		double ret = - x.Pbondm * AQLDist::normsdist(-AFFd1(x)) + x.K * x.Poptm * AQLDist::normsdist(-AFFd2(x));
		retchk;
		return ret;
		FORMULAE_END
	
	}

