#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include <AQLCoreTemplateType.h>
#include <complex>
#include "AQLBasic.h"
#include "AQLGaussLegendre.h"
#include "AQLGaussLaguerre.h"
#include "AQLFunction.h"
#include "AQLFunctionVector.h"
#include "AQLNl2sol.h"
#include "LAMathJumpDiffusion.h"
#include "LAMathDisplacedHeston.h"

using namespace std;

DoubleComplex LAMathDisplacedHeston::zeta_DD(DoubleComplex phi, 
                                           double beta_,
				                           double kappa_, 
				                           double epsilon_, 
                                           double rho_
				                           )
{
	DoubleComplex kappa = DoubleComplex(kappa_, 0.0);
    DoubleComplex rho = DoubleComplex(rho_, 0.0);
    DoubleComplex epsilon = DoubleComplex(epsilon_*beta_, 0.0);
    DoubleComplex epsilon_SQ = epsilon * epsilon;
      
    DoubleComplex tmp = (phi - phi * phi) * epsilon_SQ;
  
    DoubleComplex tmp1;
	tmp1 = kappa - (rho * epsilon) * phi;
    tmp1 *= tmp1;

	return sqrt( tmp1 + tmp);
};

DoubleComplex LAMathDisplacedHeston::psi_DD(DoubleComplex phi, 
				                          DoubleComplex zeta, 
                                          double beta_,
				                          double kappa_, 
                                          double epsilon_, 
				                          double rho_, 
				                          int sgn
				                          )
{
	DoubleComplex kappa = DoubleComplex(kappa_, 0.0);
    DoubleComplex rho = DoubleComplex(rho_, 0.0);
    DoubleComplex epsilon = DoubleComplex(epsilon_, 0.0);
	DoubleComplex sgn_ = DoubleComplex(sgn, 0.0);

	return (rho * epsilon * phi * beta_ - kappa) * sgn_ + zeta;
};

DoubleComplex LAMathDisplacedHeston::B_DD(DoubleComplex psi_plus, 
				                        DoubleComplex psi_minus,
				                        DoubleComplex zeta, 
				                        DoubleComplex phi, 
				                        double tau_,
				                        double beta_										
				                        )
{
    DoubleComplex tmp = ( phi * phi - phi) * beta_ * beta_ * (1. - exp(-zeta * tau_));
	DoubleComplex tmp1 = psi_plus * exp(-zeta * tau_) + psi_minus;

    return tmp/tmp1;
};

DoubleComplex LAMathDisplacedHeston::G_DD(DoubleComplex phi, 
				                        double T, 
				                        double FX0,
                                        double beta_,
				                        const HestonParams& heston_params
				                        )
{
	//Setup HestonParams
    HestonParams heston_params_=heston_params;
    LAMathJumpDiffusion::AdjustHestonParam(heston_params_,T);
	double v0_ = heston_params_.V0_;
    DoubleComplex v0 = DoubleComplex(v0_, 0);
    DoubleVector T_ = heston_params_.T_;
    double kappa_ = heston_params_.Kappa_;
    DoubleVector theta_ = heston_params_.Theta_;
    double epsilon_ = heston_params_.Epsilon_;
    double rho_ = heston_params_.Rho_;
        
    //Get A,B
    DoubleComplex zeta_ = LAMathDisplacedHeston::zeta_DD(phi, beta_, kappa_, epsilon_, rho_);
    DoubleComplex psi_plus = LAMathDisplacedHeston::psi_DD(phi, zeta_, beta_, kappa_, epsilon_, rho_, 1);
    DoubleComplex psi_minus = LAMathDisplacedHeston::psi_DD(phi, zeta_, beta_, kappa_, epsilon_, rho_, -1);
    DoubleComplex  A_ = LAMathJumpDiffusion::A(psi_plus, psi_minus, zeta_, T_, kappa_, theta_, epsilon_);
    DoubleComplex  B_ = LAMathDisplacedHeston::B_DD(psi_plus, psi_minus, zeta_, phi, T, beta_);
        
    //
    DoubleComplex LN_FX0_ = DoubleComplex(AQLMath::log(FX0), 0.0);
    DoubleComplex tmp =  A_ + B_ * v0 + LN_FX0_ * phi;

	DoubleComplex ret = exp(tmp);
  
	return ret;
};

double LAMathDisplacedHeston::F_Integral_DD(double z, 
					                      double T, 
					                      double FX0, 
					                      double K, 
					                      double beta_,
					                      const HestonParams& heston_params
					                      )
{
	DoubleComplex phi = DoubleComplex(0.5, -z);
	DoubleComplex G_ = LAMathDisplacedHeston::G_DD(phi, T, FX0, beta_, heston_params);
	DoubleComplex Ln_K = DoubleComplex(AQLMath::log(K),0.0);

	double ret = real( exp(-Ln_K * phi) * G_ ) / (z * z + 0.25);

	return ret;
};

double LAMathDisplacedHeston::Get_F_DD(double T, 
				                     double FX0, 
				                     double K, 
				                     double beta_, 
				                     const HestonParams& heston_params,
				                     unsigned short GL_Number
				                     )
{
	DoubleVector x(GL_Number);
	DoubleVector weight(GL_Number);
	AQLGaussLaguerre gauss_laguerre(GL_Number);
	double integral;

	/*for(size_t j=0;j<50000;j++)
	{
		gauss_legendre.get(x, weight, width*(j+1), width*j);
		for(size_t i=0;i<GL_Number;i++)
		{
			
			integral = F_Integral_DD(x[i],T, FX0, K, beta_, heston_params);
			tmp += integral * weight[i];
		}

		if(AQLMath::abs( integral ) < 1.0e-12 ) break;
	}	*/

    double tmp = 0.;
    gauss_laguerre.get(x, weight);
	for(size_t i=0;i<GL_Number;i++)
	{
		
		integral = LAMathDisplacedHeston::F_Integral_DD(x[i],T, FX0, K, beta_, heston_params);
		tmp += integral * weight[i];
	}

	return tmp * K / AQLMath::pi() ;
};

double LAMathDisplacedHeston::BS_DDHeston(double T,
				                        double FX0,
				                        double K, 
				                        double P0,
				                        int sgn,
				                        double beta_,
				                        const HestonParams& heston_params										  
				                        )
{
	//param check
	if(sgn != 1 &&  sgn != -1)
	{
		throw AQLCoreInvalidData("sgn is 1 or -1",__FILE__, __LINE__);
	}

	double K_ = beta_ * K + (1 - beta_) * FX0;
	double F = Get_F_DD(T, FX0, K_, beta_, heston_params, 192);

	return P0 / beta_ * ( (1 + sgn) / 2 * FX0 + (1 - sgn) / 2 * K_ - F );
};
