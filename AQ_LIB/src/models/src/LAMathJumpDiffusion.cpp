#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include <LACoreTemplateType.h>
#include <complex>
#include "LABasic.h"
#include "LAMathJumpDiffusion.h"
#include "LAGaussLegendre.h"
#include "LAGaussLaguerre.h"

void LAMathJumpDiffusion::SetHestonParams(HestonParams& params, const DoubleMatrix& inputdatas)
{
    //error check
    if( inputdatas.empty() ) {	throw LACoreInvalidData("data size is not supported",__FILE__, __LINE__);}
    for(size_t i=0;i<6;i++)
    {
        if( inputdatas[i].size() < 1 ) {	throw LACoreInvalidData("data size is not supported",__FILE__, __LINE__);}
    }

	params.T_ = inputdatas[0];
	params.V0_ = inputdatas[1][0];
	params.Kappa_ = inputdatas[2][0];
	params.Theta_ = inputdatas[3];
	params.Epsilon_ = inputdatas[4][0];
	params.Rho_ = inputdatas[5][0];
};

void LAMathJumpDiffusion::SetHestonParams(HestonParams& params, const DoubleVector& inputdatas, double term )
{
    //error check
    if( inputdatas.empty() ) {	throw LACoreInvalidData("data size is not supported",__FILE__, __LINE__);}
    for(size_t i=0;i<6;i++)
    {
        if( inputdatas.size() < 1 ) {	throw LACoreInvalidData("data size is not supported",__FILE__, __LINE__);}
    }

	params.T_.resize(1);
    params.T_[0] = term;
	params.V0_ = inputdatas[0];
	params.Kappa_ = inputdatas[1];
    params.Theta_.resize(1);
	params.Theta_[0] = inputdatas[2];
	params.Epsilon_ = inputdatas[3];
	params.Rho_ = inputdatas[4];
};

void LAMathJumpDiffusion::AdjustHestonParam(HestonParams& params, double T)
{
	size_t n = params.T_.size();
	size_t i = 1;
    
	while(params.T_[i-1] + 1.0e-14 < T && i<n ) i++;

	params.T_.resize(i);
	params.Theta_.resize(i);
	params.T_[i-1]=T;
};

void LAMathJumpDiffusion::SetIntensityParams(IntensityParams& params, DoubleVector inputdatas)
{
	params.Lambda0_ = inputdatas[0];
	params.Kappa_lambda_ = inputdatas[1];
	params.Theta_lambda_ = inputdatas[2];
};

void LAMathJumpDiffusion::SetLNParams(LNParams& params, DoubleVector inputdatas)
{
	params.Nu_ = inputdatas[0];
	params.Delta_ = inputdatas[1];
};

DoubleComplex LAMathJumpDiffusion::MeanJump_LN(DoubleComplex phi, LNParams ln_params)
{
	DoubleComplex one = DoubleComplex(1, 0.0);
    DoubleComplex two = DoubleComplex(2, 0.0);
    DoubleComplex nu = DoubleComplex(ln_params.Nu_, 0.0);
    DoubleComplex delta_SQ = DoubleComplex(ln_params.Delta_ * ln_params.Delta_, 0.0);
      
    DoubleComplex ret;
    ret = (delta_SQ * phi / two + ln_params.Nu_) * phi;
    ret = exp(ret) - one;
    
	return ret;
};

void LAMathJumpDiffusion::SetLEParams(LEParams& params, DoubleVector inputdatas)
{
	params.Nu_up_ = inputdatas[0];
	params.Nu_down_ = inputdatas[1];
	params.P_ = inputdatas[2];
};

DoubleComplex LAMathJumpDiffusion::MeanJump_LE(DoubleComplex phi, LEParams le_params)
{
	DoubleComplex one = DoubleComplex(1, 0);    
	DoubleComplex tmp = le_params.P_ / (one - phi * le_params.Nu_up_);
    DoubleComplex tmp1 = (1 - le_params.P_) / (one + phi * le_params.Nu_down_);
  
    return tmp + tmp1 - one;
};



DoubleComplex LAMathJumpDiffusion::Lambda_LN(DoubleComplex phi, LNParams ln_params)
{
	DoubleComplex one = DoubleComplex(1, 0.0);
    
    DoubleComplex tmp = MeanJump_LN(phi, ln_params);
    DoubleComplex tmp1 = MeanJump_LN(one, ln_params);

	return  tmp - tmp1* phi;
};

DoubleComplex LAMathJumpDiffusion::Lambda_LE(DoubleComplex phi, LEParams le_params)
{
	if (le_params.P_ < -1 && le_params.P_ > 1)
	{
		throw LACoreInvalidData("-1<= p_ <= 1",__FILE__, __LINE__);
	}
    
    DoubleComplex one = DoubleComplex(1, 0);  
    DoubleComplex tmp = MeanJump_LE(phi, le_params);
	DoubleComplex tmp1 = MeanJump_LE(one, le_params);
  
    return tmp - tmp1 * phi ;
};

DoubleComplex LAMathJumpDiffusion::zeta(DoubleComplex phi, 
				                      double kappa_, 
				                      double epsilon_, 
                                      double rho_
				                      )
{
	DoubleComplex kappa = DoubleComplex(kappa_, 0.0);
    DoubleComplex rho = DoubleComplex(rho_, 0.0);
    DoubleComplex epsilon = DoubleComplex(epsilon_, 0.0);
    DoubleComplex epsilon_SQ = epsilon * epsilon;
      
    DoubleComplex tmp = (phi - phi * phi) * epsilon_SQ;
  
    DoubleComplex tmp1;
	tmp1 = kappa - (rho * epsilon) * phi;
    tmp1 *= tmp1;

	return sqrt( tmp1 + tmp);
};

//if phi_plus,sgn_=1. if phi_minus,sgn_=-1
DoubleComplex LAMathJumpDiffusion::psi(DoubleComplex phi, 
			                         DoubleComplex zeta, 
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

	return (rho * epsilon * phi - kappa) * sgn_ + zeta;
};

DoubleComplex LAMathJumpDiffusion::D(DoubleComplex Lambda, 
				                   double tau_, 
				                   double kappa_lambda_
                                   )
{
	double tmp = (1 - LAMath::exp(-kappa_lambda_ * tau_)) / kappa_lambda_;
  
    return DoubleComplex(tmp, 0.0) * Lambda;
};

DoubleComplex LAMathJumpDiffusion::C(DoubleComplex Lambda, 
	                               double tau_, 
	                               double kappa_lambda_,
	                               double theta_lambda_
	                               )
{
	double tmp;
	tmp = kappa_lambda_ * tau_;
	tmp = tmp + LAMath::exp(-tmp) - 1;
	tmp *= theta_lambda_ / kappa_lambda_ ;

    return DoubleComplex(tmp, 0.0) * Lambda;
};

DoubleComplex LAMathJumpDiffusion::B(DoubleComplex psi_plus, 
				                   DoubleComplex psi_minus,
                                   DoubleComplex zeta, 
				                   DoubleComplex phi, 
				                   double tau_
				                   )
{
	DoubleComplex tau = DoubleComplex(tau_,0.0);
	DoubleComplex one = DoubleComplex(1.0,0.0);
    DoubleComplex ret;
	ret = phi * phi - phi;
	ret /= psi_minus + psi_plus * exp(-zeta * tau);
    ret *= one - exp(-zeta * tau);

    return ret;
};


DoubleComplex LAMathJumpDiffusion::A__(DoubleComplex psi_plus, 
				                     DoubleComplex psi_minus,
                                     DoubleComplex zeta, 
				                     double tau_, 
				                     double kappa_, 
				                     double theta_, 
				                     double epsilon_
				                     )
{
	DoubleComplex tau = DoubleComplex(tau_, 0.0);
    DoubleComplex two = DoubleComplex(2, 0.0);
  
    DoubleComplex ret;
	ret = psi_minus + psi_plus * exp(-zeta * tau);
	ret = log(ret) * two;
	ret += psi_plus*tau;
	ret *= DoubleComplex(-kappa_ * theta_ / epsilon_ / epsilon_, 0.0);

    return ret;
};

DoubleComplex LAMathJumpDiffusion::A_(DoubleComplex psi_plus, 
				                    DoubleComplex psi_minus,
				                    DoubleComplex zeta, 
				                    double tau_, 
				                    double tau1_, 
				                    double kappa_, 
				                    double theta_,
				                    double epsilon_
				                    )
{
	 return LAMathJumpDiffusion::A__(psi_plus, psi_minus, zeta, tau_, kappa_, theta_, epsilon_) -
					LAMathJumpDiffusion::A__(psi_plus, psi_minus, zeta, tau1_, kappa_, theta_, epsilon_); 
};

DoubleComplex LAMathJumpDiffusion::A(DoubleComplex psi_plus, 
				                   DoubleComplex psi_minus, 
                                   DoubleComplex zeta, 
				                   DoubleVector tau_, 
				                   double kappa_, 
                                   DoubleVector theta_, 
				                   double epsilon_
				                   )
{
	size_t n = tau_.size();

    DoubleComplex ret;
	ret = LAMathJumpDiffusion::A_(psi_plus, psi_minus, zeta, tau_[n-1], tau_[n-1] - tau_[0], kappa_, theta_[0], epsilon_);
    for(size_t i=1;i<n;i++)
	{
        ret += LAMathJumpDiffusion::A_(psi_plus, psi_minus, zeta, tau_[n-1] - tau_[i-1], tau_[n-1] - tau_[i], kappa_, 
                    theta_[i], epsilon_);
	}
    
    return ret;
};

DoubleComplex LAMathJumpDiffusion::G(DoubleComplex phi, 
				                   double T, 
				                   double FX0, 
				                   const TypeOfJump type_of_jump, 
				                   const HestonParams& heston_params,
				                   const IntensityParams& intensity_params,
				                   DoubleVector jump_params
				                   )
{
	//Setup JumpParams
	DoubleComplex Lambda_;
	if(type_of_jump == LN)
	{
		LNParams ln_params;
		SetLNParams(ln_params, jump_params);
		Lambda_ = Lambda_LN(phi,ln_params);
	}
	else if(type_of_jump == LE)
	{
		LEParams le_params;
		SetLEParams(le_params, jump_params);
		Lambda_ = Lambda_LE(phi, le_params);
	}
	else
	{
		throw LACoreInvalidData("TypeOfJump is not supported",__FILE__, __LINE__);
	}

    //Setup HestonParams
	double v0_ = heston_params.V0_;
    DoubleComplex v0 = DoubleComplex(v0_, 0);
    DoubleVector T_ = heston_params.T_;
    double kappa_ = heston_params.Kappa_;
    DoubleVector theta_ = heston_params.Theta_;
    double epsilon_ = heston_params.Epsilon_;
    double rho_ = heston_params.Rho_;
        
    //Get A,B
    DoubleComplex zeta_ = LAMathJumpDiffusion::zeta(phi, kappa_, epsilon_, rho_);
    DoubleComplex psi_plus = LAMathJumpDiffusion::psi(phi, zeta_, kappa_, epsilon_, rho_, 1);
    DoubleComplex psi_minus = LAMathJumpDiffusion::psi(phi, zeta_, kappa_, epsilon_, rho_, -1);
    DoubleComplex  A_ = LAMathJumpDiffusion::A(psi_plus, psi_minus, zeta_, T_, kappa_, theta_, epsilon_);
    DoubleComplex  B_ = LAMathJumpDiffusion::B(psi_plus, psi_minus, zeta_, phi, T);
        
    //Setup IntensityParams
    double lambda0_ = intensity_params.Lambda0_;
    DoubleComplex lambda0 = DoubleComplex(lambda0_, 0.0);
    double kappa_lambda_ = intensity_params.Kappa_lambda_;
    double theta_lambda_ = intensity_params.Theta_lambda_;
            
    //Get C,D
    DoubleComplex C_ = LAMathJumpDiffusion::C(Lambda_, T, kappa_lambda_, theta_lambda_);
    DoubleComplex D_ = LAMathJumpDiffusion::D(Lambda_, T, kappa_lambda_);
        
    //
    DoubleComplex LN_FX0_ = DoubleComplex(LAMath::log(FX0), 0.0);
    DoubleComplex tmp =  A_ + B_ * v0 + C_+ D_ * lambda0 + LN_FX0_ * phi;

	DoubleComplex ret = exp(tmp);
  
	return ret;
};

double LAMathJumpDiffusion::F_Integral(double z, 
				                     double T, 
				                     double FX0, 
				                     double K, 
                                     const TypeOfJump& type_of_jump, 
				                     const HestonParams& heston_params,	
				                     const IntensityParams& intensity_arams,
				                     DoubleVector jump_params
				                     )
{
	DoubleComplex phi = DoubleComplex(0.5, -z);
	DoubleComplex G_ = LAMathJumpDiffusion::G(phi, T, FX0, type_of_jump, heston_params, intensity_arams, jump_params);
	DoubleComplex Ln_K = DoubleComplex(LAMath::log(K),0.0);

	double ret = real( exp(-Ln_K * phi) * G_ ) / (z * z + 0.25);

	return ret;
};

double LAMathJumpDiffusion::Get_F(double T, 
			                    double FX0, 
			                    double K,
			                    int sgn,
			                    const TypeOfJump& type_of_jump, 
			                    const HestonParams& heston_params, 							 
			                    const IntensityParams& intensity_arams,
			                    DoubleVector jump_params,
			                    unsigned short GL_Number,
			                    double width
			                    )
{
	if(sgn != 1 &&  sgn != -1)
	{
		throw LACoreInvalidData("sgn is 1 or -1",__FILE__, __LINE__);
	}

	DoubleVector x(GL_Number);
	DoubleVector weight(GL_Number);
	//LAGaussLegendre gauss_legendre(GL_Number);
    LAGaussLaguerre gauss_laguerre(GL_Number);
	double tmp = 0.0;
    gauss_laguerre.get(x, weight);
	for(size_t i=0;i<GL_Number;i++)
	{
        tmp += LAMathJumpDiffusion::F_Integral(x[i],T, FX0, K, type_of_jump, heston_params, intensity_arams, jump_params) 
                * weight[i];
    }

    /*double integral;

	for(size_t j=0;j<50000;j++)
	{
		gauss_legendre.get(x, weight, width * (j+1), width * j);
		for(size_t i=0;i<GL_Number;i++)
		{
			integral = F_Integral(x[i],T, FX0, K, type_of_jump, heston_params, intensity_arams, jump_params);
			tmp += integral * weight[i];
		}

		if(LAMath::abs( integral ) < 1.0e-12 ) break;
	}	*/

	return tmp * K / LAMath::pi() ;
};

double LAMathJumpDiffusion::BS_Heston_Jump(double T,
					                     double FX0,
					                     double K, 
					                     double P0,
					                     int sgn,
					                     const TypeOfJump& type_of_jump,
					                     const HestonParams& heston_params,
					                     const IntensityParams& intensity_arams,
					                     DoubleVector jump_params
					                     )
{
	if(sgn != 1 &&  sgn != -1)
	{
		throw LACoreInvalidData("sgn is 1 or -1",__FILE__, __LINE__);
	}

	double F = LAMathJumpDiffusion::Get_F(T, FX0, K, sgn, type_of_jump, heston_params, intensity_arams, jump_params, 144, 1.);

	return P0 * ( (1 + sgn) / 2 * FX0 + (1 - sgn) / 2 * K - F );
};
