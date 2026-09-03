#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include <LACoreTemplateType.h>
#include <LAString.h>
#include "LABasic.h"
#include "LADist.h"

#include "LAFunctionUtilities.h"
#include "LAMathInterpolationUtilities.h"
#include "LACoreComponentManager.h"
#include "LAAnalyticFormula.h"
#include "LAMathIRVanillaFuncUtility.h"
#include "LAMathSABR.h"
#include "LAMathLeastSquareSABR.h"
#include <math.h>
#include "LAMathOptionTools.h"


#include <boost/math/special_functions/asinh.hpp>
//#include <boost/math/special_functions/atanh.hpp>

//++++++++ Funahashi ++++++++ // delete
////================ LAMathSABR ===================================
///*!
//	@brief Default constructor
//*/
//LAMathSABR::LAMathSABR()
//:alpha(1.),beta(0.5),nu(1.),rho(0.){}
//
//LAMathSABR::LAMathSABR(double alpha_, double beta_, double nu_, double rho_)
//:alpha(alpha_),beta(beta_),nu(nu_),rho(rho_)
//{ checkSABRParam2(); }
//
///*!
//	@brief Destructor
//*/
//LAMathSABR::~LAMathSABR(){}
//
//void 
//LAMathSABR::setSABRParam(double alpha_, double beta_, double nu_, double rho_)
//{
//    alpha = alpha_; beta = beta_; nu = nu_; rho = rho_;
//}
//    
////bool
////LAMathSABR::checkSABRParam()
////{
////    if( alpha >= alpha_high + 2. * eps_SABR ) return true;
////    if( alpha <= alpha_low - 2 * eps_SABR ) return true;
////    
////    if( beta >= beta_high + 2. * eps_SABR ) return true;
////    if( beta <= beta_low - 2. * eps_SABR ) return true;
////    
////    if( nu >= nu_high + 2 * eps_SABR ) return true;
////    if( nu <= nu_low - 2 * eps_SABR ) return true;
////
////    if( rho >= rho_high + 2. * eps_SABR ) return true;
////    if( rho <= rho_low - 2. * eps_SABR ) return true;
////
////    return false;
////}
//        
//void
//LAMathSABR::checkSABRParam2()
//{
//    if(alpha > alpha_high) throw LACoreInvalidData("alpha is too big!",__FILE__,__LINE__);
//    if(alpha < alpha_low) throw LACoreInvalidData("alpha is too small!",__FILE__,__LINE__);
//
//    if(beta > beta_high) throw LACoreInvalidData("beta is too big!",__FILE__,__LINE__);
//    if(beta < beta_low) throw LACoreInvalidData("beta is too small!",__FILE__,__LINE__);
//    
//    if(nu > nu_high) throw LACoreInvalidData("nu is too big!",__FILE__,__LINE__);
//    if(nu < nu_low) throw LACoreInvalidData("nu is too small!",__FILE__,__LINE__);
//
//    if(rho > rho_high) throw LACoreInvalidData("rho is too big!",__FILE__,__LINE__);
//    if(rho < rho_low) throw LACoreInvalidData("rho is too small!",__FILE__,__LINE__);
//}
//
//double LAMathSABR::getSABRVol(double T, double F, double K)
//{
//    double tmp1 = nu * nu * ( 2. - 3. * rho * rho ) / 24.;
//    tmp1 += rho * beta * nu * alpha / LAMath::pow( F * K, (1. - beta) / 2. ) / 4.;
//    tmp1 += (1. - beta) * (1. - beta) * alpha * alpha / LAMath::pow( F * K, (1. - beta) ) / 24.;
//    tmp1 = alpha * (1. + tmp1 * T);
//    
//    double tmp2 = (1. + LAMath::pow( LAMath::log(F / K) * (1. - beta), 2. )  / 24. 
//                      + LAMath::pow( LAMath::log(F / K) * (1. - beta), 4. ) / 1920.);
//    tmp2 *= LAMath::pow( F * K, (1. - beta) / 2. );
//
//    double z = nu / alpha * LAMath::pow( F * K, (1. - beta) / 2. ) * LAMath::log(F / K);
//    double chi_z = chi(z);
//    
//    if( tmp1 / tmp2 * z / chi_z == 0. )
//    {
//        double tmp = chi(z);
//    }
//
//    if(LAMath::abs(F - K) > eps_SABR) return  tmp1 / tmp2 * z / chi_z;
//    else return tmp1 / LAMath::pow( F, (1. - beta) );
//}
//
//double LAMathSABR::chi(double z)
//{
//    double tmp1 = LAMath::sqrt(1. - 2. * rho * z + z * z);
//    if(LAMath::abs(z)<eps_SABR) return z;
//       
//    if(tmp1 + z - rho > 0.)
//    {
//        double tmp2 = (tmp1 + z - rho) / (1. - rho);
//        return LAMath::log(tmp2);
//    }
//    else
//    {
//        double tmp2 = tmp1 - (z - rho);
//        return LAMath::log( (1. + rho) / tmp2 );
//    }
//}
//
//double LAMathSABR::getSABRPrem(double T, double F, double K, double Nu, unsigned int sgn)
//{
//    double stdDev = getSABRVol( T, F, K ) * LAMath::sqrt( T );
//
//    double d1 = LAMath::log( F/K ) / stdDev + 0.5 * stdDev;
//
//    double d2 = LAMath::log( F/K ) / stdDev - 0.5 * stdDev;
// 
//    return Nu * ( sgn * F * LADist::normsdist( sgn * d1 ) - sgn * K * LADist::normsdist( sgn * d2 ) );
//}
//
//void LAMathSABR::setAlphaForATMVol(double atmVol, double T, double F, bool isAlpha0Use)
//{
//	double alpha0 = alpha;
//    double a0 = (1.-beta)*(1.-beta)/LAMath::pow(F, 3.-3.*beta)/24.*T;
//    double a1 = rho*beta*nu/LAMath::pow(F, 2.-2.*beta)/4.*T;
//    double a2 = (1.+(2.-3.*rho*rho)*nu*nu/24.*T)/LAMath::pow(F, 1.-beta);
//    double a3 = -atmVol;
//
//    DoubleArray sol;
//	if( LAMath::abs(a0) > eps_SABR )
//	{
//		sol = LAMathInterpolationUtilities::solve_cubic_equation(a1/a0, a2/a0, a3/a0);
//	}
//	else if( LAMath::abs(a1) > eps_SABR )
//	{
//		sol = LAMathInterpolationUtilities::solve_quadratic_equation(a2/a1, a3/a1);
//		if(sol.size() == 0) throw LACoreInvalidData("No solution!",	__FILE__,__LINE__);
//	}
//	else if( LAMath::abs(a2) > eps_SABR )
//    {
//		sol = DoubleArray(1, -a3/a2);
//    }
//    else
//    {
//		throw LACoreInvalidData("No solution!",	__FILE__,__LINE__);
//	}
//
//    DoubleArray sol_;
//    for(size_t i=0; i<sol.size(); i++)
//        {
//            if(sol[i]>0.) sol_.push_back(sol[i]);
//        }
//        if(sol_.size()==0) throw LACoreInvalidData("solutions are negative!",	__FILE__,__LINE__);
//        alpha = sol_[0];
//        for(size_t i=1;i<sol_.size();i++)
//        {
//		if(isAlpha0Use)
//		{
//			if(LAMath::abs(sol_[i]-alpha0)<LAMath::abs(alpha-alpha0)) alpha = sol_[i];
//		}
//		else
//		{
//            if(sol_[i]<alpha) alpha = sol_[i];
//        }
//    }
//}
//
//bool LAMathSABR::checkAlphaForATMVol(double atmVol, double T, double F)
//{
//    double a0 = (1.-beta)*(1.-beta)/LAMath::pow(F, 3.-3.*beta)/24.*T;
//    double a1 = rho*beta*nu/LAMath::pow(F, 2.-2.*beta)/4.*T;
//    double a2 = (1.+(2.-3.*rho*rho)*nu*nu/24.*T)/LAMath::pow(F, 1.-beta);
//    double a3 = -atmVol;
//
//	DoubleArray sol;
//    if( LAMath::abs(a0) > eps_SABR )
//	{
//		sol = LAMathInterpolationUtilities::solve_cubic_equation(a1/a0, a2/a0, a3/a0);
//	}
//	else if( LAMath::abs(a1) > eps_SABR )
//	{
//		sol = LAMathInterpolationUtilities::solve_quadratic_equation(a2/a1, a3/a1);
//		if(sol.size() == 0) return true;
//	}
//	else if( LAMath::abs(a2) > eps_SABR )
//    {
//		sol = DoubleArray(1, -a3/a2);
//    }
//    else
//    {
//		return true;
//	}
//
//    DoubleArray sol_;
//    for(size_t i=0; i<sol.size(); i++)
//        {
//            if(sol[i]>0.) sol_.push_back(sol[i]);
//        }
//        if(sol_.size()==0) return true;
//
//    return false;
//    }
//++++++++ Funahashi ++++++++ // delete

//++++++++ Funahashi ++++++++ // add

//================ LAMathSABR ===================================
/*!
	@brief Default constructor
*/
LAMathSABR::LAMathSABR()
:alpha(1.),beta(0.5),nu(1.),rho(0.){}

LAMathSABR::LAMathSABR(double alpha_, double beta_, double nu_, double rho_)
:alpha(alpha_),beta(beta_),nu(nu_),rho(rho_)
{ checkSABRParam(); }

/*!
	@brief Destructor
*/
LAMathSABR::~LAMathSABR(){}

void 
LAMathSABR::setSABRParam(double alpha_, double beta_, double nu_, double rho_)
{
    alpha = alpha_; beta = beta_; nu = nu_; rho = rho_;
}
    
//bool
//LAMathSABR::checkSABRParam()
//{
//    if( alpha >= alpha_high + 2. * eps_SABR ) return true;
//    if( alpha <= alpha_low - 2 * eps_SABR ) return true;
//    
//    if( beta >= beta_high + 2. * eps_SABR ) return true;
//    if( beta <= beta_low - 2. * eps_SABR ) return true;
//    
//    if( nu >= nu_high + 2 * eps_SABR ) return true;
//    if( nu <= nu_low - 2 * eps_SABR ) return true;
//
//    if( rho >= rho_high + 2. * eps_SABR ) return true;
//    if( rho <= rho_low - 2. * eps_SABR ) return true;
//
//    return false;
//}
        
void
LAMathSABR::checkSABRParam()
{
    //if(alpha > alpha_high) throw LACoreInvalidData("alpha is too big!",__FILE__,__LINE__);
    //if(alpha < alpha_low) throw LACoreInvalidData("alpha is too small!",__FILE__,__LINE__);

    //if(beta > beta_high) throw LACoreInvalidData("beta is too big!",__FILE__,__LINE__);
    //if(beta < beta_low) throw LACoreInvalidData("beta is too small!",__FILE__,__LINE__);
    //
    //if(nu > nu_high) throw LACoreInvalidData("nu is too big!",__FILE__,__LINE__);
    //if(nu < nu_low) throw LACoreInvalidData("nu is too small!",__FILE__,__LINE__);

    //if(rho > rho_high) throw LACoreInvalidData("rho is too big!",__FILE__,__LINE__);
    //if(rho < rho_low) throw LACoreInvalidData("rho is too small!",__FILE__,__LINE__);
}

double LAMathSABR::BlackImplVol( double prem, double F, double T, double K, int sgn, double low, double high)
{

	class BlackImplVolFunc : public LAFunction
	{
	public:
		BlackImplVolFunc( double prem_, double F_, double T_, double K_, int sgn_ )
			: prem(prem_), F(F_), T(T_), K(K_), sgn(sgn_){};
		virtual ~BlackImplVolFunc(){};
		double operator()(double x) const 
		{ 
			double stdDev = x * LAMath::sqrt( T );

			double d1 = LAMath::log( F/K ) / stdDev + 0.5 * stdDev;

			double d2 = LAMath::log( F/K ) / stdDev - 0.5 * stdDev;
 
			double BlackFormula = static_cast<double>(sgn) * F * LADist::normsdist( static_cast<double>(sgn) * d1 ) - static_cast<double>(sgn) * K * LADist::normsdist( static_cast<double>(sgn) * d2 );

			return BlackFormula / prem - 1.; 
		};
	private:
		double prem;
		double F;
		double T;
		double K;
		int sgn;
	} sub_func(prem, F, T, K, sgn);

    return sub_func.SolveBR( low, high, 100000, 0.000000000000001 );
};

double LAMathSABR::BlackPrem( double vol, double F, double T, double K, double Nu, int sgn)
{
	double stdDev = vol * LAMath::sqrt( T );
	double d1 = LAMath::log( F/K ) / stdDev + 0.5 * stdDev;
	double d2 = LAMath::log( F/K ) / stdDev - 0.5 * stdDev;
	return Nu * ( static_cast<double>(sgn) * F * LADist::normsdist( static_cast<double>(sgn) * d1 ) - static_cast<double>(sgn) * K * LADist::normsdist( static_cast<double>(sgn) * d2 ) );
};

void LAMathSABR::set_alpha(double alpha_, bool isMap) 
{ 
    double width = (alpha_high - alpha_low) / 2.;
    double center = (alpha_high + alpha_low) / 2.;
    alpha = isMap ? atan(alpha_) * 2. / LAMath::pi() * width  + center : alpha_;
};

void LAMathSABR::set_beta(double beta_, bool isMap) 
{ 
    double width = (beta_high - beta_low) / 2.;
    double center = (beta_high + beta_low) / 2.;
    beta = isMap ? atan(beta_) * 2. / LAMath::pi() * width  + center : beta_;
};

void LAMathSABR::set_nu(double nu_, bool isMap)
{ 
    double width = (nu_high - nu_low) / 2.;
    double center = (nu_high + nu_low) / 2.;
    nu = isMap ? atan(nu_) * 2. / LAMath::pi() * width  + center : nu_;
};

void LAMathSABR::set_rho(double rho_,   bool isMap )
{ 
    double width = (rho_high - rho_low) / 2.;
    double center = (rho_high + rho_low) / 2.;
    rho = isMap ? atan(rho_) * 2. / LAMath::pi() * width  + center  : rho_;
};

//
//----------------------------------------------------------------
//

// constructor
LAMathSABR_Hagan::LAMathSABR_Hagan(double alpha_, double beta_, double nu_, double rho_, bool isLognormal_)
:
LAMathSABR(alpha_,beta_,nu_,rho_), isLognormal(isLognormal_)
{
}

LAMathSABR_Hagan::LAMathSABR_Hagan(const vector<double>& parameters)
:
LAMathSABR(parameters[0],parameters[1],parameters[2],parameters[3])
{
}

double powWithCheck(double base, double exp)
{
	double result = std::pow(base, exp);
	if (base < 0 && std::isnan(result))
	{
		char_t  msg[128];
		SPRINTF(msg, "Error: The power [%lf] of Negative input [%lf] is not allowed .", exp, base);
		throw LACoreNumericalError(msg, __FILE__, __LINE__);
	}
	return result;
}

// Lognormal, Shifted Lognormal Vol
double LAMathSABR_Hagan::getSABRLognormalVol(double T, double F, double K)
{
	double tmp1 = nu * nu * (2. - 3. * rho * rho) / 24.;
	tmp1 += rho * beta * nu * alpha / LAMath::pow(F * K, (1. - beta) / 2.) / 4.;
	tmp1 += (1. - beta) * (1. - beta) * alpha * alpha / LAMath::pow(F * K, (1. - beta)) / 24.;
	tmp1 = alpha * (1. + tmp1 * T);

	double sabrVol = 0.0;

	//When F is the same as K 
	if (LAMath::abs(F - K) < eps_SABR)
	{
		double tempF = powWithCheck(F, (1. - beta));

		sabrVol = tmp1 / tempF;
	}
	//When F not the same as K 
	else
	{
		double tmp2 = (1. + LAMath::pow(LAMath::log(F / K) * (1. - beta), 2.) / 24.
			+ LAMath::pow(LAMath::log(F / K) * (1. - beta), 4.) / 1920.);

		tmp2 *= LAMath::pow(F * K, (1. - beta) / 2.);

		double z = nu / alpha * LAMath::pow(F * K, (1. - beta) / 2.) * LAMath::log(F / K);
		double chi_z = chi(z);

		//if (tmp1 / tmp2 * z / chi_z == 0.)
		//{
		//	double tmp = chi(z);
		//}

		sabrVol =  tmp1 / tmp2 * z / chi_z;
	}

	return sabrVol;
}

// NORMAL Vol
double LAMathSABR_Hagan::getSABRNormalVol(double T, double F, double K)
{
	// Normal analytical  formula from Hagan's paper 'Managing Smile Risk 

	double F_mid = (fabs(F - K) < eps_SABR) ? F : ((F + K) * 0.5);

	double term1 = beta * (beta - 2.) * alpha * alpha / (24 * powWithCheck(F_mid, (2. - 2. * beta)));

	double term2 = rho * beta * nu * alpha / (4 * powWithCheck(F_mid, (1.0 - beta)));

	double term3 = (2. - 3. * rho * rho) * nu * nu / 24.;

	double sabrVol = 0.0;

	//When F is the same as K 
	if (fabs(F - K) < eps_SABR)
	{
		sabrVol = alpha * powWithCheck(F, beta) * (1. + (term1 + term2 + term3) * T);
	}
	//When F is NOT the same as K 
	else
	{

		double z = 0.0;

		// If beta equals to 1
		if ((1. - beta)< eps_SABR)
		{
			z = nu / alpha * LAMath::log(F / K);
		}
		// If beta not equals to 1
		else
		{
			z = nu / alpha * (powWithCheck(F, (1. - beta)) - powWithCheck(K, (1. - beta))) / (1. - beta);
		}


		double x_z = chi(z);

		sabrVol = nu * (F - K) / x_z * (1. + (term1 + term2 + term3) * T);
	}

	return sabrVol;
}

double LAMathSABR_Hagan::getSABRVol(double T, double F, double K)
{
	if (isLognormal)
	{
		return getSABRLognormalVol(T, F, K);
	}
	else
	{
		return getSABRNormalVol(T, F, K);
	}
}

double LAMathSABR_Hagan::getSABRPremLognormalVol(double T, double F, double K, double numeraire, int sgn)
{
	double stdDev = getSABRVol(T, F, K) * LAMath::sqrt(T);

	double d1 = LAMath::log(F / K) / stdDev + 0.5 * stdDev;

	double d2 = LAMath::log(F / K) / stdDev - 0.5 * stdDev;

	double optionPV = numeraire * (static_cast<double>(sgn) * F * LADist::normsdist(static_cast<double>(sgn) * d1) - static_cast<double>(sgn) * K * LADist::normsdist(static_cast<double>(sgn) * d2));

	return optionPV;
}


double LAMathSABR_Hagan::getSABRPremNormalVol(double T, double F, double K, double numeraire, int sgn)
{
	double vol = getSABRVol(T, F, K);

	double optionPV = numeraire * BachelierPrice(T, K, sgn > 0, F, vol);

	return optionPV;
}


double LAMathSABR_Hagan::getSABRPrem(double T, double F, double K, double Nu, int sgn)
{
	if (isLognormal)
	{
		return getSABRPremLognormalVol(T, F, K, Nu, sgn);
	}
	else
	{
		return getSABRPremNormalVol(T, F, K, Nu, sgn);
	}

}


void LAMathSABR_Hagan::setAlphaForATMVol(double atmVol, double T, double F, bool isAlpha0Use)
{
	double alpha0 = alpha;

	AQ_REQUIRE(!AQ_IS_EQUAL_ZERO(F), "SABR: swapRate/forwardRate cannot be zero.");

	double a0 = isLognormal ? ((1. - beta) * (1. - beta) / LAMath::pow(F, 2. - 2. * beta) / 24. * T)
							: (beta * (beta - 2.) / LAMath::pow(F, 2. - 2. * beta) / 24. * T);
	double a1 = rho * beta * nu / LAMath::pow(F, 1. - beta) / 4. * T;
	double a2 = (1. + (2. - 3. * rho * rho) * nu * nu / 24. * T);
	double a3 = isLognormal ? (-atmVol * LAMath::pow(F, 1. - beta)) : (-atmVol * LAMath::pow(F, -beta));

    DoubleArray sol;
	if( LAMath::abs(a0) > eps_SABR )
	{
		sol = LAMathInterpolationUtilities::solve_cubic_equation(a1/a0, a2/a0, a3/a0);
	}
	else if( LAMath::abs(a1) > eps_SABR )
	{
		sol = LAMathInterpolationUtilities::solve_quadratic_equation(a2/a1, a3/a1);
		if(sol.size() == 0) throw LACoreInvalidData("No solution!",	__FILE__,__LINE__);
	}
	else if( LAMath::abs(a2) > eps_SABR )
    {
		sol = DoubleArray(1, -a3/a2);
    }
    else
    {
		throw LACoreInvalidData("No solution!",	__FILE__,__LINE__);
	}

    DoubleArray sol_;
	for(size_t i=0; i<sol.size(); i++)
    {
		if(sol[i]>0.) sol_.push_back(sol[i]);
    }
    if(sol_.size()==0) throw LACoreInvalidData("solutions are negative!",	__FILE__,__LINE__);
    alpha = sol_[0];
    for(size_t i=1;i<sol_.size();i++)
    {
		if(isAlpha0Use)
		{
			if(LAMath::abs(sol_[i]-alpha0)<LAMath::abs(alpha-alpha0)) alpha = sol_[i];
		}
		else
		{
		     if(sol_[i]<alpha) alpha = sol_[i];
	    }
	}
}

bool LAMathSABR_Hagan::checkAlphaForATMVol(double atmVol, double T, double F)
{
	double a0 = isLognormal ? ((1. - beta) * (1. - beta) / LAMath::pow(F, 2. - 2. * beta) / 24. * T)
							: (beta * (beta - 2.) / LAMath::pow(F, 2. - 2. * beta) / 24. * T);
	double a1 = rho * beta * nu / LAMath::pow(F, 1. - beta) / 4. * T;
	double a2 = (1. + (2. - 3. * rho * rho) * nu * nu / 24. * T);
	double a3 = isLognormal ? (-atmVol * LAMath::pow(F, 1. - beta)) : (-atmVol * LAMath::pow(F, -beta));

	DoubleArray sol;
    if( LAMath::abs(a0) > eps_SABR )
	{
		sol = LAMathInterpolationUtilities::solve_cubic_equation(a1/a0, a2/a0, a3/a0);
	}
	else if( LAMath::abs(a1) > eps_SABR )
	{
		sol = LAMathInterpolationUtilities::solve_quadratic_equation(a2/a1, a3/a1);
		if(sol.size() == 0) return true;
	}
	else if( LAMath::abs(a2) > eps_SABR )
    {
		sol = DoubleArray(1, -a3/a2);
    }
    else
    {
		return true;
	}

    DoubleArray sol_;
    for(size_t i=0; i<sol.size(); i++)
        {
            if(sol[i]>0.) sol_.push_back(sol[i]);
        }
        if(sol_.size()==0) return true;

    return false;
}

double LAMathSABR_Hagan::chi(double z)
{
    double tmp1 = LAMath::sqrt(1. - 2. * rho * z + z * z);
    if(LAMath::abs(z)<eps_SABR) return z;
       
    if(tmp1 + z - rho > 0.)
    {
        double tmp2 = (tmp1 + z - rho) / (1. - rho);
        return LAMath::log(tmp2);
    }
    else
    {
        double tmp2 = tmp1 - (z - rho);
        return LAMath::log( (1. + rho) / tmp2 );
    }
}

//
//----------------------------------------------------------------
//

//
 // constructor
LAMathSABR_Antonov::LAMathSABR_Antonov(double alpha_, double beta_, double nu_, double rho_, int int_step_)
:
LAMathSABR(alpha_,beta_,nu_,rho_)
{
	sub_func = new swaption_prem_antonov(int_step_);
}

LAMathSABR_Antonov::LAMathSABR_Antonov(const LAMathSABR_Antonov& rhs )
:
sub_func(new swaption_prem_antonov())
{
}


double LAMathSABR_Antonov::getSABRVol(double T, double F, double K)
{
	vector<double> sabr_params(4);
	sabr_params[0] = alpha;
	sabr_params[1] = beta;
	sabr_params[2] = nu;
	sabr_params[3] = rho;
	double int_regime = 0.;

	set(F,T,sabr_params);
	double premium = sub_func->operator()(K);
	if( premium == 0. ) premium = 0.000001;
	return LAMathSABR::BlackImplVol(premium,F,T,K,1);
}

double LAMathSABR_Antonov::getSABRPrem(double T, double F, double K, double Nu, int sgn)
{
	vector<double> sabr_params(4);
	sabr_params[0] = alpha;
	sabr_params[1] = beta;
	sabr_params[2] = nu;
	sabr_params[3] = rho;
	double int_regime = 0.;

	set(F,T,sabr_params);
	double premium = sub_func->operator()(K);
	
	if( sgn == 1 )
	{
		return Nu * premium;	//call
	}	
	else
	{
		return Nu * (premium - F + K);	//put
	}	
}

void LAMathSABR_Antonov::setAlphaForATMVol(double atmVol, double T, double F, bool isAlpha0Use)
{
	class ATMVolCostFunc : public LAFunction
	{
	public:
		ATMVolCostFunc( double F_, double T_, double atmVol_, double beta0_, double nu0_, double rho0_)
			: F(F_), T(T_), atmVol(atmVol_), beta(beta0_), nu(nu0_), rho(rho0_){};
		virtual ~ATMVolCostFunc(){};
		double operator()(double x) const 
		{ 
			LAMathSABR_Antonov sabr;
			sabr.set_alpha (x,false);
			sabr.set_beta(beta,false);
			sabr.set_nu(nu,false);
			sabr.set_rho(rho,false);
			return sabr.getSABRVol(T, F, F) / atmVol - 1.; 
		};
	private:
		double F;
		double T;
		double atmVol;
		double beta;
		double nu;
		double rho;
	} sub_func(F, T, atmVol, beta, nu, rho);

	double tmp = sub_func.SolveBR( alpha * 0.5, alpha * 1.2, 100000, 0.000000000000001 );
	alpha = tmp;
}

bool LAMathSABR_Antonov::checkAlphaForATMVol(double atmVol, double T, double F)
{
	return true;
}

//
LAMathSABR_Antonov::swaption_prem_antonov::swaption_prem_antonov(int int_step_)
:
num_int_step(int_step_),
flg_negative_nu(false)
{
	int_1_.SetFunc(*this, &LAMathSABR_Antonov::swaption_prem_antonov::int_1);
	int_2_.SetFunc(*this, &LAMathSABR_Antonov::swaption_prem_antonov::int_2);
}

void LAMathSABR_Antonov::swaption_prem_antonov::set(double S0_,double T_,const vector<double>& sabr_params)
{
	if(S0_ < 0.0) throw LACoreInvalidData("negative foward Rate!",__FILE__,__LINE__);
	S0 = S0_ > eps_SABR ? S0_ : eps_SABR;
	T = T_;
	alpha = sabr_params[0];
	beta = sabr_params[1] < 0.99 ? sabr_params[1] : 0.99;
	nu = sabr_params[2];
	rho = sabr_params[3];
}

double LAMathSABR_Antonov::swaption_prem_antonov::operator()(double K) const
{
	// transformed strike and forward
	double q  = pow(K, 1. - beta ) / (1. - beta );
	double q0 = pow(S0, 1. - beta ) / (1. - beta );
	double dq = q - q0;

	// zero map parameters
	nu_ = sqrt(max( nu * nu - 1.5 * (nu * nu * rho * rho + alpha * nu * rho * (1. - beta) * pow( S0, beta - 1. ) ), 0.00000001));
	double alpha_min = sqrt( (nu * dq) * (nu * dq) + 2. * rho * (nu * dq) * alpha + alpha * alpha );
	double Phi = pow( (alpha_min + rho * alpha + nu * dq)/(1.+rho)/alpha, nu_/ nu );	
	double alpha_ = fabs(dq) < 0.0000000001 ? alpha : 2. * Phi * dq * nu_ / (Phi * Phi - 1. );
	double nu1_ = alpha_ * (1. + T * ( 1. / 12. * (1. -  nu_ * nu_ / (nu * nu) - 1.5 * rho * rho) * nu * nu + 0.25 * beta * rho * alpha * nu * pow(S0, beta - 1.) ) );
	// avoid singular case of non-positive nu_
	if( nu1_ < std::numeric_limits<double>::epsilon() )
	{
		flg_negative_nu = true;
		return 0.;
	}

	// integral range
	s_minus = boost::math::asinh( nu_ * fabs(q - q0) / nu1_ );
	s_plus  = boost::math::asinh( nu_ * fabs(q + q0) / nu1_ );

	double ret = 0.;
	double dt = (s_plus - s_minus) / static_cast<double>(num_int_step);
	for (unsigned int i = 0; i < num_int_step; ++i)
	{
		double t_s = s_minus + static_cast<double>(i) * dt;
		double t_e = s_minus + static_cast<double>(i+1) * dt;
		ret +=  int_1_.IntegrateGLegendre(t_s, t_e, 20);
	}

	//
	//double s_max = 100.;
	double s_max = 20.;
	dt = fabs(s_max - s_plus) / static_cast<double>(num_int_step);
	double InitialValue2 = int_2(s_plus); 
	for (unsigned int i = 0; i < num_int_step ; ++i)
	{
		double t_s = s_plus + static_cast<double>(i) * dt;
		double t_e = s_plus + static_cast<double>(i+1) * dt;

		ret += int_2_.IntegrateGLegendre(t_s, t_e, 20);

		//if(fabs(int_2(t_s) / InitialValue2) < 10e-15)
		if(fabs(int_2(t_s) / InitialValue2) < 10e-10)
		{
			break;
		}
	}

	ret *= 2. / M_PI * sqrt(K * S0) ;
	ret += max(S0 - K, 0.) ;

	return ret;
}

double LAMathSABR_Antonov::swaption_prem_antonov::int_1(double x) const
{
	if( x < std::numeric_limits<double>::epsilon() ) return 1.;
	double eta = fabs( 0.5 / ( beta - 1.) );
	return sin(eta * phi(x)) / sinh(x) * G(nu_ * nu_ * T,  x);
}

double LAMathSABR_Antonov::swaption_prem_antonov::phi(double s) const
{	
	double arg = (sinh(s) - sinh(s_minus) * sinh(s_minus) / sinh(s)) / (sinh(s_plus) * sinh(s_plus) / sinh(s) - sinh(s));
	arg = sqrt(abs(arg));
	return 2. * atan(arg);
}

double LAMathSABR_Antonov::swaption_prem_antonov::int_2(double x) const
{
	double eta = fabs( 0.5 / ( beta - 1.) );
	return sin(eta * M_PI) * pow(psi(x), eta) / sinh(x) * G(nu_ * nu_ * T,  x);
}

double LAMathSABR_Antonov::swaption_prem_antonov::psi(double s) const
{	
		double arg = abs( (sinh(s) - sinh(s_plus)) / (sinh(s) - sinh(s_minus)) 
							* (sinh(s) + sinh(s_plus)) / (sinh(s) + sinh(s_minus)) ) ;
		arg = sqrt(arg);

		return (1. - arg)/(1. + arg);
}

double LAMathSABR_Antonov::swaption_prem_antonov::G(double t, double s) const
{
	if( s < std::numeric_limits<double>::epsilon() ) return 1.;
	return sqrt(sinh(s)/s) * exp(-0.5 * s * s / t - t / 8.) * (R(t,s) + dR(t,s));

}
double LAMathSABR_Antonov::swaption_prem_antonov::R(double t, double s) const
{
	if( s < 0.1 ) s = 0.1;
	double g = s / tanh(s) - 1.;
	return 1. + 3. * t *  g / 8. / s / s  - 5. * t * t * (-8. * s * s + 3. * g * g + 24. * g) / 128. / pow(s, 4.)
		  + 35. * t * t * t * (-40. * s * s + 3.0 * g * g * g + 24. * g * g + 120. * g) / 1024. /pow(s, 6.) ;
}

double LAMathSABR_Antonov::swaption_prem_antonov::dR(double t, double s) const
{
	return exp(t/8.) - (3072. + 384. * t + 24. * t * t + t * t * t) / 3072.;
}

//
//----------------------------------------------------------------
//

//
 // constructor
LAMathSABR_Chaos::LAMathSABR_Chaos(double alpha_, double beta_, double nu_, double rho_)
:
LAMathSABR(alpha_,beta_,nu_,rho_)
{
}

double LAMathSABR_Chaos::getSABRVol(double T, double F, double K)
{
	vector<double> sabr_params(4);
	sabr_params[0] = alpha;
	sabr_params[1] = beta;
	sabr_params[2] = nu;
	sabr_params[3] = rho;
	double int_regime = 0.;

	double premium =getSABRPrem(T, F, K, 1., 1);
	if( premium == 0. ) premium = 0.000001;
	double domesticDF	= 1.;
	int Sgn				= 1; //Call:1, Put: -1
	return LAMathSABR::BlackImplVol(premium/domesticDF,F,T,K,(int)Sgn);
}

double LAMathSABR_Chaos::getSABRPrem(double T, double F, double K, double Nu, int sgn)
{
	if(F > 0.)
	{
		double K_ = 1. - K / F;

		double Sigma_ = Sigma(T);
		double Sigma_sq = Sigma_ * Sigma_;
    
		double normal_ = ONE_OVER_SQRT_TWO_PI * exp( -0.5 * (K_ * K_ / sqrt(Sigma_)/ sqrt(Sigma_)) ) / sqrt(Sigma_);
		double coef = F * normal_ / (2. * sqrt(2.) * Sigma_sq * Sigma_sq);
		double temp1 = sqrt(2.) * q3(T) * (pow(K_, 4.) - 6. * K_ * K_ * Sigma_ + 3. * Sigma_sq);
		double temp2 = Sigma_sq * sqrt(2.) * (q4(T) + 2. * q2(T)) * (K_ * K_ - Sigma_);
		double temp3 = Sigma_sq * Sigma_ * (-2. * sqrt(2.) * q1(T) * K_ + sqrt(2.) * q5(T) * Sigma_ + 2. * sqrt(2.) * Sigma_sq);
		double temp4 = F * K_ * (1. - LADist::normsdist(-K_ / sqrt(Sigma_)));

		double CallValue = coef * (temp1 + temp2 + temp3) + temp4;
		return sgn < 0 ? Nu * (CallValue - F + K) : Nu * CallValue;
	}
	else
	{
		double K_ = -(1. - K / F);

		double Sigma_ = Sigma(T);
		double Sigma_sq = Sigma_ * Sigma_;
    
		double normal_ = ONE_OVER_SQRT_TWO_PI * exp( -0.5 * (K_ * K_ / sqrt(Sigma_)/ sqrt(Sigma_)) ) / sqrt(Sigma_);
		double coef = F * normal_ / (2. * sqrt(2.) * Sigma_sq * Sigma_sq);
		double temp1 = sqrt(2.) * q3(T) * (pow(K_, 4.) - 6. * K_ * K_ * Sigma_ + 3. * Sigma_sq);
		double temp2 = Sigma_sq * sqrt(2.) * (q4(T) + 2. * q2(T)) * (K_ * K_ - Sigma_);
		double temp3 = Sigma_sq * Sigma_ * (+2. * sqrt(2.) * q1(T) * K_ + sqrt(2.) * q5(T) * Sigma_ + 2. * sqrt(2.) * Sigma_sq);	//Watch a difference fropm above!
		double temp4 = F * K_ * (1. - LADist::normsdist(-K_ / sqrt(Sigma_)));

		double CallValue = coef * (temp1 + temp2 + temp3) + temp4;
		return sgn < 0 ? Nu * (-CallValue - F + K) : -Nu * CallValue;
	}
}

void LAMathSABR_Chaos::setAlphaForATMVol(double atmVol, double T, double F, bool isAlpha0Use)
{
	class ATMVolCostFunc : public LAFunction
	{
	public:
		ATMVolCostFunc( double F_, double T_, double atmVol_, double beta0_, double nu0_, double rho0_)
			: F(F_), T(T_), atmVol(atmVol_), beta(beta0_), nu(nu0_), rho(rho0_){};
		virtual ~ATMVolCostFunc(){};
		double operator()(double x) const 
		{ 
			LAMathSABR_Antonov sabr;
			sabr.set_alpha (x,false);
			sabr.set_beta(beta,false);
			sabr.set_nu(nu,false);
			sabr.set_rho(rho,false);
			return sabr.getSABRVol(T, F, F) / atmVol - 1.; 
		};
	private:
		double F;
		double T;
		double atmVol;
		double beta;
		double nu;
		double rho;
	} sub_func(F, T, atmVol, beta, nu, rho);

	double tmp = sub_func.SolveBR( alpha * 0.5, alpha * 1.2, 100000, 0.000000000000001 );
	alpha = tmp;
}

bool LAMathSABR_Chaos::checkAlphaForATMVol(double atmVol, double T, double F)
{
	return true;
}

//
//----------------------------------------------------------------
//

LAMathSABR_Chaos_DD::LAMathSABR_Chaos_DD(double alpha_, double beta_, double nu_, double rho_)
:
LAMathSABR_Chaos(alpha_,beta_,nu_,rho_)
{
}

//SABR & DD-SABR WIC Approximation
double 
LAMathSABR_Chaos_DD::Sigma(double T_) const
{
	double A = alpha;
	double B = (beta * rho * nu * alpha) * alpha;
	double Sigma = A * A * T_+ A * B * T_ * T_  + B * B * T_ * T_ * T_ / 3. ;
	return Sigma;
}

double 
LAMathSABR_Chaos_DD::q1(double T_) const
{
	double delta = alpha * beta * nu * rho * T_;
	double q1 = T_ * T_ * pow(alpha, 3.) * (alpha * beta + rho * nu) * pow(2. + delta, 2.) / 8.;
	return q1;
}

double 
LAMathSABR_Chaos_DD::q2(double T_) const
{
	double delta = alpha * beta * nu * rho * T_;
	double q2 = pow(alpha, 3.) * pow(beta, 2.) + pow(rho * nu, 2.) * alpha + 3. * beta * rho * pow(alpha, 2.) * nu;
	q2 = q2 * pow(T_, 3.0) * pow(alpha, 3.0) *  pow(2. + delta, 3.) / 48.;
	return q2;
}

double 
LAMathSABR_Chaos_DD::q3(double T_) const
{
    double temp = q1(T_);
    double q3 = temp * temp;
	return q3;
}

double 
LAMathSABR_Chaos_DD::q4(double T_) const
{
		double delta = alpha * beta * nu * rho * T_;
		vector<double> temp(3);
        
		temp[0] = pow(alpha, 5.) * beta * (alpha * beta + 2. * rho * nu) * T_ * T_ * T_;
		temp[0] = temp[0] * (1. + delta + delta * delta / 4.);
    
		temp[1] = T_ * T_ * T_ * nu * nu * pow(alpha, 4.);
		temp[1] = temp[1] * (20. + 25. * delta + 8. * delta * delta) / 60.;
    
		temp[2] = T_ * T_ * T_ * rho * rho * nu * nu * pow(alpha, 4.);
		temp[2] = temp[2] * (40. + 35. * delta + 7. * delta * delta) / 60.;
    
		double q4 = temp[0] + temp[1] + temp[2];
		return q4;
	//}
}

double 
LAMathSABR_Chaos_DD::q5(double T_) const
{
		double q5 = pow(alpha, 4.) * pow(beta, 2.) 
					+ pow(nu * alpha, 2.) 
					+ 2. * rho * pow(alpha, 3.) * nu * beta;
		q5 = q5 * T_ * T_ / 2.;
			
		return q5;
}

//
//----------------------------------------------------------------
//

//
LAMathConvexityAdust_Antonov::LAMathConvexityAdust_Antonov(double T_,
													   double S0_,
													   double alpha,
													   double beta,
													   double nu,
													   double rho,
													   size_t num_int_step)
: T(T_),S0(S0_),swaption_prem(new LAMathSABR_Antonov(alpha,beta,nu,rho,num_int_step)) {}

LAMathConvexityAdust_Antonov::LAMathConvexityAdust_Antonov(const LAMathConvexityAdust_Antonov& rhs)
:
T(rhs.T),
S0(rhs.S0)
{
	swaption_prem = new LAMathSABR_Antonov(rhs.swaption_prem->getAlpha(),
										 rhs.swaption_prem->getBeta(),
										 rhs.swaption_prem->getNu(),
										 rhs.swaption_prem->getRho());
}

//
//----------------------------------------------------------------
//

////  Cost Func for optimizing SABR Parameters
//LAMathLeastSquareSABRCostFunc::LAMathLeastSquareSABRCostFunc( LAMathSABR& sabr_,
//														  double expiry_,
//														  double forward_,
//														  double numeraire_,
//														  const vector<double>& strike_,
//														  const vector<double>& weight_,
//														  const vector<int>& sgn_,
//														  const vector<double>& param0_,
//														  const vector<bool>& flg_,
//														  const vector<double>& target2fit_,
//														  const LAString& target_
//														)
//:
//CostFunction(),
//n(strike_.size()),
//expiry(expiry_),
//forward(forward_),
//numeraire(numeraire_),
//strike(strike_),	
//weight(weight_),	
//sgn(sgn_),//Call-Put Sign (-1: put, 1: call)
//param0(param0_),//Initial alpha, beta, nu, rho
//flg(flg_),
//target2fit(target2fit_),
//sabr(&sabr_),
//target(target_),
//cloned(false)
//{
//	if(weight.size() != n)
//	{
//		throw invalid_argument(" strike.size() != weight.size() : LAMathLeastSquareSABRCostFunc::LAMathLeastSquareSABRCostFunc");
//	}
//	
//	if(param0.size() != 4)
//	{
//		throw invalid_argument("param0.size() != 4 : LAMathLeastSquareSABRCostFunc::LAMathLeastSquareSABRCostFunc");
//	}
//
//	if(flg.size() != 4)
//	{
//		throw invalid_argument("flg.size() != 4 : LAMathLeastSquareSABRCostFunc::LAMathLeastSquareSABRCostFunc");
//	}
//
//	for(size_t i = 0; i < n; ++i)
//	{
//		if(sgn[i] != 1 && sgn[i] != -1)
//		{
//			throw invalid_argument("sgn must -1 or 1 : LAMathLeastSquareSABRCostFunc::LAMathLeastSquareSABRCostFunc");
//		}
//	}
//
//	alpha = param0[0];
//	beta = param0[1];
//	nu = param0[2];
//	rho = param0[3];
//}
//
//
////
//LAMathLeastSquareSABRCostFunc::LAMathLeastSquareSABRCostFunc(const LAMathLeastSquareSABRCostFunc& rhs)
//:
//CostFunction(rhs),
//n(rhs.n),
//expiry(rhs.expiry),
//forward(rhs.forward),
//numeraire(rhs.numeraire),
//strike(rhs.strike),
//weight(rhs.weight),
//sgn(rhs.sgn),
//param0(rhs.param0),
//flg(rhs.flg),
//target2fit(rhs.target2fit),
//sabr(rhs.sabr != 0 ? rhs.sabr->clone() : 0),
//alpha(rhs.alpha),
//beta(rhs.beta),
//nu(rhs.nu),
//rho(rhs.rho),
//cloned(true)
//{}
//
//
//LAMathLeastSquareSABRCostFunc::~LAMathLeastSquareSABRCostFunc()
//{
//	if(cloned)
//	{
//		delete sabr;
//		sabr = 0;
//	}
//}
//
//QuantLib::Real LAMathLeastSquareSABRCostFunc::value(const QuantLib::Array& x) const
//{   
//	size_t m = x.size();
//	
//	if(m == 0 || m > 4)
//		throw invalid_argument("m == 0 || m > 4 : LAMathLeastSquareSABRCostFunc::value");
//
//	set_params(x);
//	sabr->setSABRParam(alpha,beta,nu,rho); 
//
//	QuantLib::Real tmp = 0.;
//	double tmp2 = 0.;
//	for(size_t i = 0; i < n; ++i)
//	{
//		if( weight[i] > 0.000000001 )
//		{
//			if( target == "PREMIUM" )
//			{
//				int sgn_ = sgn[i];
//				tmp2 = sqrt( weight[i] ) * ( target2fit[i] - sabr->getSABRPrem(expiry,forward,strike[i],numeraire,sgn_) );
//			}
//			else if( target == "VOLATILITY" )
//			{
//				tmp2 = sqrt( weight[i] ) * ( target2fit[i] - sabr->getSABRVol(expiry,forward,strike[i]) );
//			}
//		
//			tmp += tmp2 * tmp2;
//		}
//	}
//
//	return tmp;
//}
//
//QuantLib::Array LAMathLeastSquareSABRCostFunc::values(const QuantLib::Array& x) const
//{
//	size_t m = x.size();
//	
//	if(m == 0 || m > 4)
//		throw invalid_argument("m == 0 || m > 4 : LAMathLeastSquareSABRCostFunc::value");
//
//	set_params(x);    
//	sabr->setSABRParam(alpha,beta,nu,rho); 
//	
//    size_t k = 0;
//	QuantLib::Array diff_SQs( n );
//	
//	for(size_t i = 0; i < n; ++i)
//	{
//		if( weight[i] > 0.000000001 )
//		{
//			if( target == "PREMIUM" )
//			{
//				int sgn_ = sgn[i];
//				diff_SQs[i] = sqrt( weight[i] ) * (  sabr->getSABRPrem(expiry,forward,strike[i],numeraire,sgn_)- target2fit[i] ); 
//			}
//			else if( target == "VOLATILITY" )
//			{
//				diff_SQs[i] = sqrt( weight[i] ) * ( sabr->getSABRVol(expiry,forward,strike[i]) - target2fit[i] ); 
//			}
//		}
//	}
//
//	return diff_SQs;
//}
//
//void LAMathLeastSquareSABRCostFunc::set_params(const QuantLib::Array& x) const
//{
//	// 1st
//	if(!flg[0])
//	{
//		if(!flg[1])
//		{
//			if(!flg[2])
//			{
//				if(!flg[3])
//				{
//					throw invalid_argument("All element of flg is false : LAMathLeastSquareSABRCostFunc::set_params");
//				}
//				else rho = x[0];
//			}
//			else
//			{
//				nu = x[0];
//				if(flg[3]) rho = x[1];
//			}
//		}
//		else
//		{
//			beta = x[0];
//			if(flg[2])
//			{
//				nu = x[1];
//				if(flg[3]) rho = x[2];
//			}
//			else
//			{
//				if(flg[3]) rho = x[1];
//			}
//		}
//	}
//	else
//	{
//		alpha = x[0];
//		if(!flg[1])
//		{
//			if(flg[2])
//			{
//				nu = x[1];
//				if(flg[3]) rho = x[2];
//			}
//			else
//			{
//				if(flg[3]) rho = x[1];
//			}
//		}
//		else
//		{
//			beta = x[1];
//			if(flg[2])
//			{
//				nu = x[2];
//				if(flg[3]) rho = x[3];
//			}
//			else
//			{
//				if(flg[3]) rho = x[2];
//			}
//		}
//	}
//}
//
////
////----------------------------------------------------------------
////
//
////LAMathLeastSquareSABRConstraint::LAMathLeastSquareSABRConstraint(vector<double>& param0,
////															 double max_alpha,
////								                             double min_alpha,
////								                             double max_beta,
////								                             double min_beta,
////								                             double max_nu,
////								                             double min_nu,
////								                             double max_rho,
////								                             double min_rho
////								                            )
////: QuantLib::Constraint(boost::shared_ptr<QuantLib::Constraint::Impl>( new LAMathLeastSquareSABRConstraint::Impl(param0,
////																											  max_alpha,
////																											  min_alpha,
////																											  max_beta,
////																											  min_beta,
////																											  max_nu,
////																											  min_nu,
////																											  max_rho,
////																											  min_rho
////																											 )) )
////{
////}
//
////
//LAMathLeastSquareSABRConstraint::Impl::Impl(vector<double>& param0_,
//										  const vector<bool>& flg_,
//										  double max_alpha_,
//							              double min_alpha_,
//							              double max_beta_,
//							              double min_beta_,
//							              double max_nu_,
//							              double min_nu_,
//							              double max_rho_,
//							              double min_rho_
//							             )
//:
//param0(param0_),
//flg(flg_),
//max_alpha(max_alpha_),
//min_alpha(min_alpha_),
//max_beta(max_beta_),
//min_beta(min_beta_),
//max_nu(max_nu_),
//min_nu(min_nu_),
//max_rho(max_rho_),
//min_rho(min_rho_),
//alpha(param0_[0]),
//beta(param0_[1]),
//nu(param0_[2]),
//rho(param0_[3])
//{	
//}
////
////
//bool LAMathLeastSquareSABRConstraint::Impl::test(const QuantLib::Array& p) const
//{
// 	size_t m = p.size();
//	
//	set_params(p);
//
//	if(alpha < min_alpha || max_alpha < alpha ) return false;
//	if(beta < min_beta || max_beta < beta ) return false;
//	if(nu < min_nu || max_nu < nu ) return false;
//	if(rho < min_rho || max_rho < rho ) return false;
//
//	return true;
//}
//
//void LAMathLeastSquareSABRConstraint::Impl::set_params(const QuantLib::Array& x) const
//{
//	// 1st
//	if(!flg[0])
//	{
//		if(!flg[1])
//		{
//			if(!flg[2])
//			{
//				if(!flg[3])
//				{
//					throw invalid_argument("All element of flg is false : LAMathLeastSquareSABRCostFunc::set_params");
//				}
//				else rho = x[0];
//			}
//			else
//			{
//				nu = x[0];
//				if(flg[3]) rho = x[1];
//			}
//		}
//		else
//		{
//			beta = x[0];
//			if(flg[2])
//			{
//				nu = x[1];
//				if(flg[3]) rho = x[2];
//			}
//			else
//			{
//				if(flg[3]) rho = x[1];
//			}
//		}
//	}
//	else
//	{
//		alpha = x[0];
//		if(!flg[1])
//		{
//			if(flg[2])
//			{
//				nu = x[1];
//				if(flg[3]) rho = x[2];
//			}
//			else
//			{
//				if(flg[3]) rho = x[1];
//			}
//		}
//		else
//		{
//			beta = x[1];
//			if(flg[2])
//			{
//				nu = x[2];
//				if(flg[3]) rho = x[3];
//			}
//			else
//			{
//				if(flg[3]) rho = x[2];
//			}
//		}
//	}
//}
//
////
////----------------------------------------------------------------
////
//
///*LAMathLeastSquareSABRConstraint_Antonov::LAMathLeastSquareSABRConstraint_Antonov( double fwd,
//																			  vector<double>& param0,
//																			  double max_alpha,
//																			  double min_alpha,
//																			  double max_beta,
//																			  double min_beta,
//																			  double max_nu,
//																			  double min_nu,
//																			  double max_rho,
//																			  double min_rho
//																			)
//:
//Constraint(boost::shared_ptr<Constraint::Impl>( new LAMathLeastSquareSABRConstraint_Antonov::Impl(fwd,
//																								param0,
//																								max_alpha,
//																								min_alpha,
//																								max_beta,
//																								min_beta,
//																								max_nu,
//																								min_nu,
//																								max_rho,
//																								min_rho
//																								)) )
//{
//}*/
//
////
//LAMathLeastSquareSABRConstraint_Antonov::Impl::Impl( double fwd_,
//												   vector<double>& param0_,
//												   const vector<bool>& flg_,
//												   double max_alpha_,
//												   double min_alpha_,
//												   double max_beta_,
//												   double min_beta_,
//												   double max_nu_,
//												   double min_nu_,
//												   double max_rho_,
//												   double min_rho_
//												  )
//:
//fwd(fwd_),
//param0(param0_),
//flg(flg_),
//max_alpha(max_alpha_),
//min_alpha(min_alpha_),
//max_beta(max_beta_),
//min_beta(min_beta_),
//max_nu(max_nu_),
//min_nu(min_nu_),
//max_rho(max_rho_),
//min_rho(min_rho_),
//alpha(param0_[0]),
//beta(param0_[1]),
//nu(param0_[2]),
//rho(param0_[3])
//{	
//}
//
////
//bool LAMathLeastSquareSABRConstraint_Antonov::Impl::test(const QuantLib::Array& p) const
//{
// 	size_t m = p.size();
//	
//	set_params(p);
//
//	if(alpha < min_alpha || max_alpha < alpha ) return false;
//	if(beta < min_beta || max_beta < beta ) return false;
//	if(nu < min_nu || max_nu < nu ) return false;
//	if(rho < min_rho || max_rho < rho ) return false;
//
//	if(nu * nu - 1.5 * (nu * nu * rho * rho + alpha * nu * rho * (1. - beta) * pow(fwd, beta - 1.) ) < 0. )  return false;
//	
//	return true;
//}
//
//void LAMathLeastSquareSABRConstraint_Antonov::Impl::set_params(const QuantLib::Array& x) const
//{
//	if(!flg[0])
//	{
//		if(!flg[1])
//		{
//			if(!flg[2])
//			{
//				if(!flg[3])
//				{
//					throw invalid_argument("All element of flg is false : LAMathLeastSquareSABRCostFunc::set_params");
//				}
//				else rho = x[0];
//			}
//			else
//			{
//				nu = x[0];
//				if(flg[3]) rho = x[1];
//			}
//		}
//		else
//		{
//			beta = x[0];
//			if(flg[2])
//			{
//				nu = x[1];
//				if(flg[3]) rho = x[2];
//			}
//			else
//			{
//				if(flg[3]) rho = x[1];
//			}
//		}
//	}
//	else
//	{
//		alpha = x[0];
//		if(!flg[1])
//		{
//			if(flg[2])
//			{
//				nu = x[1];
//				if(flg[3]) rho = x[2];
//			}
//			else
//			{
//				if(flg[3]) rho = x[1];
//			}
//		}
//		else
//		{
//			beta = x[1];
//			if(flg[2])
//			{
//				nu = x[2];
//				if(flg[3]) rho = x[3];
//			}
//			else
//			{
//				if(flg[3]) rho = x[2];
//			}
//		}
//	}
//
//}

//++++++++ Funahashi ++++++++ // add

LAMathSABRLimiter::LAMathSABRLimiter(LAStringMatrix paramFlag)
{
    upper(paramFlag);
    paramNum = 0;
    //alpha
    if( searchbyrow(paramFlag,"ALPHA",1,true) == "YES" )
    {
        alphaFlag = true; 
        paramNum++;
    }
    else if( searchbyrow(paramFlag,"ALPHA",1,true) == "NO" ) 
    {
        alphaFlag = false;
    }
    else 
    {
        throw LACoreInvalidData("No alpha!",__FILE__,__LINE__);
    }

    if( searchbyrow(paramFlag,"BETA",1,true) == "YES" ) 
    {
        betaFlag = true; 
        paramNum++;
    }
    else if( searchbyrow(paramFlag,"BETA",1,true) == "NO" ) 
    {
        betaFlag = false;
    }
    else 
    {
        throw LACoreInvalidData("No beta!",__FILE__,__LINE__);
    }

    if( searchbyrow(paramFlag,"NU",1,true) == "YES" ) 
    {
        nuFlag = true; 
        paramNum++;
    }
    else if( searchbyrow(paramFlag,"NU",1,true) == "NO" ) 
    {
        nuFlag = false;
    }
    else 
    {
        throw LACoreInvalidData("No nu!",__FILE__,__LINE__);
    }

    if( searchbyrow(paramFlag,"RHO",1,true) == "YES" ) 
    {
        rhoFlag = true; 
        paramNum++;
    }
    else if( searchbyrow(paramFlag,"RHO",1,true) == "NO" ) 
    {
        rhoFlag = false;
    }
    else 
    {
        throw LACoreInvalidData("No rho!",__FILE__,__LINE__);
    }
}

void
LAMathSABRLimiter::getArgument( LAMathSABR_Hagan sabr, DoubleArray& x, bool isMap )
{
    size_t index=0;
    if( getAlphaFlag() ) 
    {
        double width = (alpha_high - alpha_low) / 2.;
        double center = (alpha_high + alpha_low) / 2.;
        x[index] = isMap ? tan(LAMath::pi() / 2. * (sabr.getAlpha()-center) / width) : sabr.getAlpha();
        index++;
    }
    if( getBetaFlag() ) 
    {
        double width = (beta_high - beta_low) / 2.;
        double center = (beta_high + beta_low) / 2.;
        x[index] = isMap ? tan(LAMath::pi() / 2. * (sabr.getBeta()-center) / width) : sabr.getBeta();            index++;
    }
    if( getNuFlag() ) 
    {
        double width = (nu_high - nu_low) / 2.;
        double center = (nu_high + nu_low) / 2.;
        x[index] = isMap ? tan(LAMath::pi() / 2. * (sabr.getNu()-center) / width) : sabr.getNu();
        index++;
    }
    if( getRhoFlag() ) 
    {
        double width = (rho_high - rho_low) / 2.;
        double center = (rho_high + rho_low) / 2.;
        x[index] = isMap ? tan(LAMath::pi() / 2. * (sabr.getRho()-center) / width): sabr.getRho();
        index++;
    }
}

LAMathSABR_Hagan
LAMathSABRLimiter::getSABR(LAMathSABR_Hagan& sabrOri, const DoubleArray& x, bool isMap)
{
    if( x.size() != paramNum ) 
        throw LACoreInvalidData("sizes are inconsistent",__FILE__,__LINE__);

    size_t index=0;
    LAMathSABR_Hagan sabr;
    if( getAlphaFlag() ) 
    {
        sabr.set_alpha(x[index],isMap);
        index++;
    }
    else
    {
        sabr.set_alpha(sabrOri.getAlpha(),false);
    }

    if( getBetaFlag() ) 
    {
        sabr.set_beta(x[index],isMap);
        index++;
    }
    else
    {
        sabr.set_beta(sabrOri.getBeta(),false);
    }
    if( getNuFlag() ) 
    {
        sabr.set_nu(x[index],isMap);
        index++;
    }
    else
    {
        sabr.set_nu(sabrOri.getNu(),false);
    }
    if( getRhoFlag() ) 
    {
        sabr.set_rho(x[index],isMap);
        index++;
    }
    else
    {
        sabr.set_rho(sabrOri.getRho(),false);
    }
    
	if( sabr.getAlpha() >= alpha_high ) sabr.set_alpha(alpha_high, false);
	if( sabr.getAlpha() <= alpha_low ) sabr.set_alpha(alpha_low, false);            
	if( sabr.getBeta() >= beta_high ) sabr.set_beta(beta_high, false);
	if( sabr.getBeta() <= beta_low ) sabr.set_beta(beta_low, false);
	if( sabr.getNu() >= nu_high ) sabr.set_nu(nu_high, false);
	if( sabr.getNu() <= nu_low ) sabr.set_nu(nu_low, false);
	if( sabr.getRho() >= rho_high ) sabr.set_rho(rho_high, false);
	if( sabr.getRho() <= rho_low ) sabr.set_rho(rho_low, false);

    return sabr;
}

bool
LAMathSABRLimiter::checkArgument(LAMathSABR_Hagan sabrOri, const DoubleArray& x)
{
	if( x.size() != paramNum ) 
        throw LACoreInvalidData("sizes are inconsistent",__FILE__,__LINE__);

    size_t index=0;
    LAMathSABR_Hagan sabr;

	if( getAlphaFlag() ) 
    {
        sabr.set_alpha(x[index],false);
        index++;
    }
    else
    {
        sabr.set_alpha(sabrOri.getAlpha(),false);
    }

    if( getBetaFlag() ) 
    {
        sabr.set_beta(x[index],false);
        index++;
    }
    else
    {
        sabr.set_beta(sabrOri.getBeta(),false);
    }
    if( getNuFlag() ) 
    {
        sabr.set_nu(x[index],false);
        index++;
    }
    else
    {
        sabr.set_nu(sabrOri.getNu(),false);
    }
    if( getRhoFlag() ) 
    {
        sabr.set_rho(x[index],false);
        index++;
    }
    else
    {
        sabr.set_rho(sabrOri.getRho(),false);
    }

	double coeffi = 2.;

	if( sabr.getAlpha() > alpha_high + coeffi * eps_SABR ) return true;
    if( sabr.getAlpha() < alpha_low  - coeffi * eps_SABR ) return true;
    
    if( sabr.getBeta() > beta_high + coeffi * eps_SABR ) return true;
    if( sabr.getBeta() < beta_low  - coeffi * eps_SABR ) return true;
    
    if( sabr.getNu() > nu_high + coeffi * eps_SABR ) return true;
    if( sabr.getNu() < nu_low  - coeffi * eps_SABR ) return true;

    if( sabr.getRho() > rho_high + coeffi * eps_SABR ) return true;
    if( sabr.getRho() < rho_low  - coeffi * eps_SABR ) return true;

	return false;
}

LAMathSABRCalibrator::LAMathSABRCalibrator
    (const LAMathSABR_Hagan& sabr_, const DoubleArray& strike_, const DoubleArray& vol_, 
     const DoubleArray& forward_, const DoubleArray& expiry_, const DoubleArray& numeraire_, 
     const DoubleArray& weight_, const LAString& target_, bool isMap_)
    : sabr(sabr_), strike(strike_), vol(vol_), forward(forward_), expiry(expiry_), 
      numeraire(numeraire_), weight(weight_), target(target_), isMap(isMap_)
{
    targetNum = strike_.size();
    if( targetNum != vol_.size() ||  targetNum != forward_.size() || 
        targetNum != expiry_.size() || targetNum != weight_.size() )
    {
        throw LACoreInvalidData("sizes are inconsistent",__FILE__,__LINE__);
    }

    for(size_t i=0; i<vol.size(); i++)
    {
        if(vol[i]<0.) throw LACoreInvalidData("volatility is negative!",__FILE__,__LINE__);
    }

    if( target != CALIB_TARGET_VOLATILITY && target != CALIB_TARGET_PREMIUM )
        throw LACoreInvalidData("target is VOLATILITY or PREMIUM",__FILE__,__LINE__);

    if( target == CALIB_TARGET_PREMIUM )
    {
        std::map<LAString, LABlackScholesBase*> &var = LACoreComponentManager::getBlackComponentMap();
        LAString bscomponent = LAString(BK)+LAString(PREM)+LAString(CALL);
        std::map<LAString, LABlackScholesBase*>::iterator it = var.find(bscomponent);
	    if(it==var.end()) throw LACoreInvalidData("Product Type is not supported",__FILE__,__LINE__);
	    LABlackScholesBase* p1 = it->second;
        AnalyticBKParam param;
        for(size_t i=0; i<vol_.size(); i++)
        {
             param.F = forward[i]; param.Nu = numeraire[i]; param.Te =expiry[i]; 
             param.Vol = vol[i]; param.K = strike[i];
             prem.push_back( p1->calc(param) );
        }

        for(size_t i=0; i<prem.size(); i++)
        {
            if(prem[i]<0.) throw LACoreInvalidData("premium is negative!",__FILE__,__LINE__);
        }
    }
}

void
LAMathSABRCalibrator::operator()(DoubleArray& f, const DoubleArray& x)
{
    f.resize(targetNum);
    LAMathSABR_Hagan sabrOpt = sabrLimiter.getSABR( sabr, x, isMap );
    //sabrOpt.checkSABRParam();
    for(size_t j=0; j<targetNum; j++)
    {
        if( target == CALIB_TARGET_PREMIUM )
        {
            f[j] = ( sabrOpt.getSABRPrem(expiry[j], forward[j], strike[j], numeraire[j], 1) / prem[j] - 1. ) * weight[j];
        }
        else if( target == CALIB_TARGET_VOLATILITY )
        {
            f[j] = ( sabrOpt.getSABRVol(expiry[j], forward[j], strike[j]) / vol[j] - 1. ) * weight[j];
        }
    }
}

bool
LAMathSABRCalibrator::constraintsAreViolated( const DoubleArray& x )
{
    if(!isMap && sabrLimiter.checkArgument(sabr, x) ) return true;
    if( anyNotANumber(x) ) return true;

    return false;
}

LAMathSABRCalibratorATMFix::LAMathSABRCalibratorATMFix
    (const LAMathSABR_Hagan& sabr_, const DoubleArray& strike_, const DoubleArray& vol_, 
     double atmVol_, double forward_, double expiry_, 
     double numeraire_, const DoubleArray& weight_, const LAString& target_, bool isMap_)
    : sabr(sabr_), strike(strike_), vol(vol_), atmVol(atmVol_), 
      forward(forward_), expiry(expiry_), 
      numeraire(numeraire_), weight(weight_), target(target_), isMap(isMap_)
{
    targetNum = strike_.size();
    if( targetNum != vol_.size() || targetNum != weight_.size() )
    {
        throw LACoreInvalidData("sizes are inconsistent",__FILE__,__LINE__);
    }

    if( target != CALIB_TARGET_VOLATILITY && target != CALIB_TARGET_PREMIUM )
        throw LACoreInvalidData("target is VOLATILITY or PREMIUM",__FILE__,__LINE__);

    if( target == CALIB_TARGET_PREMIUM )
    {
        std::map<LAString, LABlackScholesBase*> &var = LACoreComponentManager::getBlackComponentMap();
        LAString bscomponent = LAString(BK)+LAString(PREM)+LAString(CALL);
        std::map<LAString, LABlackScholesBase*>::iterator it = var.find(bscomponent);
	    if(it==var.end()) throw LACoreInvalidData("Product Type is not supported",__FILE__,__LINE__);
	    LABlackScholesBase* p1 = it->second;
        AnalyticBKParam param;
        param.F = forward; param.Nu = numeraire; param.Te =expiry; 
        for(size_t i=0; i<vol_.size(); i++)
        {
             param.Vol = vol[i]; param.K = strike[i];
             prem.push_back( p1->calc(param) );
        }
    }
}

void
LAMathSABRCalibratorATMFix::operator()(DoubleArray& f, const DoubleArray& x)
{
    f.resize(targetNum);
    LAMathSABR_Hagan sabrOpt = sabrLimiter.getSABR( sabr, x, isMap );
    sabrOpt.setAlphaForATMVol(atmVol, expiry, forward);
    
    for(size_t j=0; j<targetNum; j++)
    {
        if( target == CALIB_TARGET_PREMIUM )
        {
            f[j] = ( sabrOpt.getSABRPrem(expiry, forward, strike[j], numeraire, 1) - prem[j] ) * weight[j];
        }
        else if( target == CALIB_TARGET_VOLATILITY )
        {
            f[j] = ( sabrOpt.getSABRVol(expiry, forward, strike[j] ) - vol[j] ) * weight[j];
        }
    }
}

bool
LAMathSABRCalibratorATMFix::constraintsAreViolated( const DoubleArray& x )
{
	LAMathSABR_Hagan sabrOpt = sabrLimiter.getSABR( sabr, x , isMap );
    if( sabrOpt.checkAlphaForATMVol(atmVol, expiry, forward) ) return true;
	if(!isMap && sabrLimiter.checkArgument(sabr, x) ) return true;
    if( anyNotANumber(x) ) return true;

    return false;
}