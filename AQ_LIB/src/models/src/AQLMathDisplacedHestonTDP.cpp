/*! @file
    @brief Class declaration to AQLMathDisplacedHeston.

*/

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
#include "AQLMathJumpDiffusion.h"
#include "AQLMathDisplacedHestonTDP.h"
#include "AQLMathDisplacedHeston.h"
#include "AQLFunctionVector.h"
#include "AQLMathFXVolatilitySurfaceGenerate.h"
#include "AQLNl2sol.h"
#include "AQLFunction.h"
#include "AQLMathAnalyticalFormula.h"

void AQLMathDisplacedHestonTDP::SetHestonParams_TDP(HestonParams_TDP& params, DoubleMatrix inputdatas)
{
    //error check
    if( inputdatas.empty() ) {	throw AQLCoreInvalidData("data size is not supported",__FILE__, __LINE__);}
    for(size_t i=0;i<6;i++)
    {
        if( inputdatas[i].size() < 1 ) {	throw AQLCoreInvalidData("data size is not supported",__FILE__, __LINE__);}
    }

	params.T = inputdatas[0];
	params.V0 = inputdatas[1][0];
	params.Kappa = inputdatas[2];
	params.Theta = inputdatas[3];
	params.Epsilon = inputdatas[4];
	params.Rho = inputdatas[5];
};

void AQLMathDisplacedHestonTDP::AdjustHestonParam_TDP(HestonParams_TDP& params, double T)
	{
		size_t n = params.T.size();
		size_t i = 1;

		while(params.T[i-1]<T && i<n ) i++;

		params.T.resize(i);
		params.Theta.resize(i);
		params.T[i-1]=T;

		for(int j = i-1; j>0; j--)
		{
			params.T[j] = params.T[j] - params.T[j-1];
		}
	}

DoubleComplex AQLMathDisplacedHestonTDP::B_DD_TDP(DoubleComplex B,
					                           DoubleComplex psi_plus, 
					                           DoubleComplex psi_minus,
					                           DoubleComplex zeta, 
					                           DoubleComplex phi, 
					                           double tau_,
					                           double beta_,
                                               double epsilon_											
					                           )
{
    DoubleComplex tmp;
	tmp = (phi * phi - phi) * beta_ * beta_*(1. - exp(-zeta * tau_));
	tmp += (psi_plus + psi_minus * exp(-zeta * tau_)) * B;
	DoubleComplex tmp1;
	tmp1 = psi_plus * exp(-zeta * tau_) + psi_minus;
	tmp1 -= (1. - exp(-zeta * tau_)) * epsilon_ * epsilon_ * B;

    return tmp/tmp1;
}

DoubleComplex AQLMathDisplacedHestonTDP::A_TDP(DoubleComplex B,
					                        DoubleComplex psi_plus, 
					                        DoubleComplex psi_minus,
					                        DoubleComplex zeta, 
					                        double tau_, 
					                        double kappa_, 
					                        double theta_,
					                        double epsilon_								 
					                        )
{
  DoubleComplex ret;
	ret = psi_minus + psi_plus * exp( -zeta * tau_);
	ret -= (1. - exp(-zeta * tau_)) * epsilon_ * epsilon_ * B;
	ret /= 2. * zeta;
	ret = log(ret) * 2.;
	ret += psi_plus*tau_;
	ret *= DoubleComplex(-kappa_ * theta_ / epsilon_ / epsilon_, 0.0);

  return ret;
};

DoubleComplex AQLMathDisplacedHestonTDP::G_DD_TDP(DoubleComplex phi, 
                                               double T,
					                           double FX0, 
					                           double beta_, 
					                           const HestonParams_TDP& heston_params
					                           )
{
	//Setup HestonParams
    HestonParams_TDP heston_params_ = heston_params;
    AQLMathDisplacedHestonTDP::AdjustHestonParam_TDP(heston_params_,T);
	double v0_ = heston_params_.V0;
    DoubleComplex v0 = DoubleComplex(v0_, 0);
    DoubleVector T_ = heston_params_.T;
    DoubleVector kappa_ = heston_params_.Kappa;
    DoubleVector theta_ = heston_params_.Theta;
    DoubleVector epsilon_ = heston_params_.Epsilon;
    DoubleVector rho_ = heston_params_.Rho;
	size_t n = heston_params_.T.size();
    
    //Get A,B
	DoubleComplex  A_ = 0.;
	DoubleComplex  B_ = 0.;
	for(int i = n-1; i>=0 ; i--)
	{
        DoubleComplex zeta_ = AQLMathDisplacedHeston::zeta_DD(phi, beta_, kappa_[i], epsilon_[i], rho_[i]);
		DoubleComplex psi_plus = AQLMathDisplacedHeston::psi_DD(phi, zeta_, beta_, kappa_[i], epsilon_[i], rho_[i], 1);
		DoubleComplex psi_minus = AQLMathDisplacedHeston::psi_DD(phi, zeta_, beta_, kappa_[i], epsilon_[i], rho_[i], -1);
		A_ += A_TDP(B_, psi_plus, psi_minus, zeta_, T_[i], kappa_[i], theta_[i], epsilon_[i] );
		B_ = B_DD_TDP(B_, psi_plus, psi_minus, zeta_, phi, T_[i], beta_ , epsilon_[i] );
    }
    
    //
    DoubleComplex LN_FX0_ = DoubleComplex(AQLMath::log(FX0), 0.0);
    DoubleComplex tmp =  A_ + B_ * v0 + LN_FX0_ * phi;

	DoubleComplex ret = exp(tmp);
  
	return ret;
};

double AQLMathDisplacedHestonTDP::F_Integral_DD_TDP(double z,
                                                 double T, 
						                         double FX0, 
						                         double K, 
						                         double beta_,
						                         const HestonParams_TDP& heston_params
						                         )
{
	DoubleComplex phi = DoubleComplex(0.5, -z);
	DoubleComplex G_ = G_DD_TDP(phi, T, FX0, beta_, heston_params);
	DoubleComplex Ln_K = DoubleComplex(AQLMath::log(K),0.0);

	double ret = real( exp(-Ln_K * phi) * G_ ) / (z * z + 0.25);

	return ret;
}

double AQLMathDisplacedHestonTDP::Get_F_DD_TDP(double T,
                                            double FX0, 
					                        double K,
					                        double beta_, 
					                        const HestonParams_TDP& heston_params,
					                        unsigned short GL_Number,
					                        double width
					                        )
{
	DoubleVector x(GL_Number);
	DoubleVector weight(GL_Number);
	//AQLGaussLegendre gauss_legendre(GL_Number);
    AQLGaussLaguerre gauss_laguerre(GL_Number);
	double integral;

	double tmp = 0.;

	/*for(size_t j=0;j<50000;j++)
	{
		gauss_legendre.get(x, weight, width*(j+1), width*j);
		for(size_t i=0;i<GL_Number;i++)
		{
			
			integral = F_Integral_DD_TDP(x[i], FX0, K, beta_, heston_params);
			tmp += integral * weight[i];
		}

		if(AQLMath::abs( integral ) < 1.0e-12 ) break;
	}	*/

    gauss_laguerre.get(x, weight);
	for(size_t i=0;i<GL_Number;i++)
	{
		integral = F_Integral_DD_TDP(x[i],T, FX0, K, beta_, heston_params);
		tmp += integral * weight[i];
	}

	return tmp * K / AQLMath::pi() ;
};

double AQLMathDisplacedHestonTDP::BS_DDHeston_TDP(double T,
                                               double FX0,
					                           double K, 
					                           double P0,
					                           int sgn,
					                           double beta_,
					                           const HestonParams_TDP& heston_params										  
					                           )
{
	//param check
	if(sgn != 1 &&  sgn != -1)
	{
		throw AQLCoreInvalidData("sgn is 1 or -1",__FILE__, __LINE__);
	}

	double K_ = beta_ * K + (1 - beta_) * FX0;
	double F = Get_F_DD_TDP(T,FX0, K_, beta_, heston_params ,192, 1.);

	return P0 / beta_ * ( (1 + sgn) / 2 * FX0 + (1 - sgn) / 2 * K_ - F );
};

double AQLMathDisplacedHestonTDP::BS_DDHestonImpVol_TDP(double T,
                                                     double FX0,
					                                 double K, 
					                                 double P0,
					                                 int sgn,
					                                 double beta_,
					                                 const HestonParams_TDP& heston_params										  
					                                 )
{
    double prem = BS_DDHeston_TDP(T,FX0,K,P0,sgn,beta_,heston_params);
    double highPrem = AQLMathAnalyticalFormula::BlackFormula( FX0, 10. * AQLMath::sqrt(T), K, sgn) * P0;
	double lowPrem = AQLMathAnalyticalFormula::BlackFormula( FX0, 0.000001 * AQLMath::sqrt(T), K, sgn) * P0;

    if( prem <= lowPrem ) return 0.000001;
    if( prem >= highPrem ) return 10.;

	return AQLMathAnalyticalFormula::BlackImplVol(prem/P0, FX0, K, sgn, 0.000001 * AQLMath::sqrt(T), 10. * AQLMath::sqrt(T)) 
                                / AQLMath::sqrt(T);
};

DoubleMatrix AQLMathDisplacedHestonTDP::CalibrationHelper( const std::vector<FXOptionData >& datas,
														const DoubleMatrix& strikes,
														const IntMatrix& sgns,
														HestonParams_TDP& hestonParam
														)
{
	size_t termSize = strikes.size();
    if( strikes.size() != termSize || sgns.size() != termSize || hestonParam.T.size() != termSize
		|| datas.size() != termSize)
    {
            AQLString msg("size is not supported!");
	        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

    size_t strikeSize = strikes[0].size();
    if( strikes[0].size() != strikeSize || sgns[0].size() != strikeSize )
    {
            AQLString msg("size is not supported!");
	        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

    size_t i,j,k;
	for(i=0;i<termSize;i++)
    {
        if( AQLMath::abs(hestonParam.T[i] -  datas[i].T) > 0.000001)
        {
            throw AQLCoreInvalidData("input term is incorrect",__FILE__,__LINE__);
        }
    }

	//Setup HestonParams
    HestonParams_TDP hestonParam_ = hestonParam;
    AQLMathDisplacedHestonTDP::AdjustHestonParam_TDP(hestonParam_,datas[termSize-1].T);
	double v0_ = hestonParam_.V0;
    DoubleComplex v0 = DoubleComplex(v0_, 0);
    DoubleVector T = hestonParam_.T;
    DoubleVector kappa = hestonParam_.Kappa;
    DoubleVector theta = hestonParam_.Theta;
    DoubleVector epsilon = hestonParam_.Epsilon;
    DoubleVector rho = hestonParam_.Rho;
    size_t GL_Size = 192;
    DoubleMatrix ret(termSize,DoubleVector(strikeSize));

    //Set GLParams
    DoubleVector x(GL_Size);
	DoubleVector weight(GL_Size);
    AQLGaussLaguerre gauss_laguerre(GL_Size);
    gauss_laguerre.get(x, weight);

    //Set A,B,B_tau
    ComplexMatrix A(termSize,ComplexVector(GL_Size));
    ComplexMatrix B(termSize,ComplexVector(GL_Size));
    ComplexVector phi(GL_Size);
    ComplexMatrix zeta(termSize,ComplexVector(GL_Size));
    ComplexMatrix psi_plus(termSize,ComplexVector(GL_Size));
    ComplexMatrix psi_minus(termSize,ComplexVector(GL_Size));
    for(k=0;k<GL_Size;k++)
	{
		phi[k] = DoubleComplex(0.5,-x[k]);
        for(i=0;i<termSize;i++)
		{            
			zeta[i][k] = AQLMathDisplacedHeston::zeta_DD(phi[k], 1., kappa[i], epsilon[i], rho[i]);
			psi_plus[i][k] = AQLMathDisplacedHeston::psi_DD(phi[k], zeta[i][k], 1., kappa[i], epsilon[i], rho[i], 1);
			psi_minus[i][k] = AQLMathDisplacedHeston::psi_DD(phi[k], zeta[i][k], 1., kappa[i], epsilon[i], rho[i], -1);
        }
    }

    for(k=0;k<GL_Size;k++)
	{
        for(i=0;i<termSize;i++)
        {
            A[i][k] = 0.;
            B[i][k] = 0.;
            for(int i_=i;i_>=0;i_--)
		    {            
			    A[i][k] += A_TDP(B[i][k], psi_plus[i_][k], psi_minus[i_][k], zeta[i_][k], T[i_], kappa[i_], theta[i_], epsilon[i_]);
                B[i][k] = B_DD_TDP(B[i][k], psi_plus[i_][k], psi_minus[i_][k], zeta[i_][k], phi[k], T[i_], 1., epsilon[i_]);
            }
        }
    }	

    //Set ret
    DoubleComplex G;
	double F;
	DoubleComplex Ln_K;
    double highPrem,lowPrem,prem;
    for(i=0;i<termSize;i++)
	{  
        for(j=0;j<strikeSize;j++)
		{
			F = 0.;
            for(k=0;k<GL_Size;k++)
			{
                G = exp( A[i][k] + B[i][k] * v0 + AQLMath::log(datas[i].F / strikes[i][j]) * phi[k] );
                F += real( G ) / (x[k] * x[k] + 0.25) * weight[k];
            }
			F = F * strikes[i][j] / AQLMath::pi();
			prem = datas[i].Pd * ( (1 + sgns[i][j]) / 2 * datas[i].F + (1 - sgns[i][j]) / 2 * strikes[i][j] - F);
			highPrem = AQLMathAnalyticalFormula::BlackFormula( datas[i].F, 10. * AQLMath::sqrt(datas[i].T), 
												strikes[i][j], sgns[i][j]) * datas[i].Pd;
			lowPrem = AQLMathAnalyticalFormula::BlackFormula( datas[i].F, 0.000001 * AQLMath::sqrt(datas[i].T), 
											strikes[i][j], sgns[i][j]) * datas[i].Pd;

			if( prem <= lowPrem )  ret[i][j] = 0.000001;
			else if( prem >= highPrem ) ret[i][j] = 10.;
            else
            {
				ret[i][j] = AQLMathAnalyticalFormula::BlackImplVol(prem/datas[i].Pd, datas[i].F, strikes[i][j], sgns[i][j],
                                0.000001 * AQLMath::sqrt(datas[i].T), 10. * AQLMath::sqrt(datas[i].T) ) 
                                / AQLMath::sqrt(datas[i].T);
            }
        }
    }

	return ret;
};

DoubleMatrix AQLMathDisplacedHestonTDP::CalibrationHelperGL( const std::vector<FXOptionData >& datas,
														  const DoubleMatrix& strikes,
														  const IntMatrix& sgns,
														  HestonParams_TDP& hestonParam
														  )
{
    if(hestonParam.T.size() != 1) 
        throw AQLCoreInvalidData("heston param is not glabal!",__FILE__,__LINE__);

	size_t termSize = strikes.size();
    if( strikes.size() != termSize || sgns.size() != termSize || datas.size() != termSize)
    {
            AQLString msg("size is not supported!");
	        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

    size_t strikeSize = strikes[0].size();
    if( strikes[0].size() != strikeSize || sgns[0].size() != strikeSize )
    {
            AQLString msg("size is not supported!");
	        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

    size_t i,j,k;

	//Setup HestonParams
	double v0_ = hestonParam.V0;
    DoubleComplex v0 = DoubleComplex(v0_, 0);
    double kappa = hestonParam.Kappa[0];
    double theta = hestonParam.Theta[0];
    double epsilon = hestonParam.Epsilon[0];
    double rho = hestonParam.Rho[0];
    size_t GL_Size = 192;
    DoubleMatrix ret(termSize,DoubleVector(strikeSize));

    //Set GLParams
    DoubleVector x(GL_Size);
	DoubleVector weight(GL_Size);
    AQLGaussLaguerre gauss_laguerre(GL_Size);
    gauss_laguerre.get(x, weight);

    //Set A,B,B_tau
    ComplexMatrix A(termSize,ComplexVector(GL_Size));
    ComplexMatrix B(termSize,ComplexVector(GL_Size));
    ComplexVector phi(GL_Size);
    ComplexVector zeta(GL_Size);
    ComplexVector psi_plus(GL_Size);
    ComplexVector psi_minus(GL_Size);
    for(k=0;k<GL_Size;k++)
	{
		phi[k] = DoubleComplex(0.5,-x[k]);
        for(i=0;i<termSize;i++)
		{            
			zeta[k] = AQLMathDisplacedHeston::zeta_DD(phi[k], 1., kappa, epsilon, rho);
			psi_plus[k] = AQLMathDisplacedHeston::psi_DD(phi[k], zeta[k], 1., kappa, epsilon, rho, 1);
			psi_minus[k] = AQLMathDisplacedHeston::psi_DD(phi[k], zeta[k], 1., kappa, epsilon, rho, -1);
        }
    }

    for(k=0;k<GL_Size;k++)
	{
        for(i=0;i<termSize;i++)
        {
            A[i][k] = A_TDP(0.0, psi_plus[k], psi_minus[k], zeta[k], datas[i].T, kappa, theta, epsilon);
            B[i][k] = B_DD_TDP(0.0, psi_plus[k], psi_minus[k], zeta[k], phi[k], datas[i].T, 1., epsilon);
        }
    }	

    //Set ret
    DoubleComplex G;
	double F;
	DoubleComplex Ln_K;
    double highPrem,lowPrem,prem;
    for(i=0;i<termSize;i++)
	{  
        for(j=0;j<strikeSize;j++)
		{
			F = 0.;
            for(k=0;k<GL_Size;k++)
			{
                G = exp( A[i][k] + B[i][k] * v0 + AQLMath::log(datas[i].F / strikes[i][j]) * phi[k] );
                F += real( G ) / (x[k] * x[k] + 0.25) * weight[k];
            }
			F = F * strikes[i][j] / AQLMath::pi();
			prem = datas[i].Pd * ( (1 + sgns[i][j]) / 2 * datas[i].F + (1 - sgns[i][j]) / 2 * strikes[i][j] - F);
			highPrem = AQLMathAnalyticalFormula::BlackFormula( datas[i].F, 10. * AQLMath::sqrt(datas[i].T), 
												strikes[i][j], sgns[i][j]) * datas[i].Pd;
			lowPrem = AQLMathAnalyticalFormula::BlackFormula( datas[i].F, 0.000001 * AQLMath::sqrt(datas[i].T), 
											strikes[i][j], sgns[i][j]) * datas[i].Pd;

			if( prem <= lowPrem )  ret[i][j] = 0.000001 ;
			else if( prem >= highPrem ) ret[i][j] = 10. ;
            else
            {
				ret[i][j] = AQLMathAnalyticalFormula::BlackImplVol(prem/datas[i].Pd, datas[i].F, strikes[i][j], sgns[i][j],
                                    0.000001 * AQLMath::sqrt(datas[i].T), 10. * AQLMath::sqrt(datas[i].T) ) 
                                    / AQLMath::sqrt(datas[i].T);
            }
        }
    }

	return ret;
};
#ifdef isQuantLib
#include <ql/math/array.hpp>
#include <ql/math/optimization/costfunction.hpp>
#include <ql/math/optimization/constraint.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>

const double HESTON_UPPER = 1000.;

class HestonCalibratorGlobal : public QuantLib::CostFunction
{
public:
    /// constructor
    HestonCalibratorGlobal( const std::vector<FXOptionData >& datas_,
                            const DoubleMatrix& vols_,
                            const DoubleMatrix& strikes_,
                            const IntMatrix& sgns_,
                            HestonParams_TDP& hestonParam_)
            : datas(datas_), vols(vols_), strikes(strikes_), sgns(sgns_), hestonParam(hestonParam_)
    {
        termSize = vols_.size();
        if( strikes_.size() != termSize || sgns_.size() != termSize )
        {
             AQLString msg("size is not supported!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }

        strikeSize = vols_[0].size();
        if( strikes_[0].size() != strikeSize || sgns_[0].size() != strikeSize )
        {
             AQLString msg("size is not supported!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }
    };
    virtual ~HestonCalibratorGlobal(){};

    QuantLib::Real value(const QuantLib::Array& x) const 
    {
        QuantLib::Real ret=0;
        size_t i,j;

        hestonParam.V0 = x[0]; 
        hestonParam.Kappa[0] = x[1];
        hestonParam.Theta[0] = x[2];
        hestonParam.Epsilon[0] = x[3];
        hestonParam.Rho[0] = x[4];

        DoubleMatrix modelVol = AQLMathDisplacedHestonTDP::CalibrationHelperGL( datas, strikes, sgns, hestonParam );
        for(i=0;i<termSize;i++)
        {
            for(j=0;j<strikeSize;j++)
            {
                ret += AQLMath::pow(modelVol[i][j] - vols[i][j],2);
            }
        }
        return ret;
    };

    QuantLib::Array values(const QuantLib::Array& x) const
    {
        QuantLib::Array y(termSize*strikeSize);
        size_t i,j;
        
        hestonParam.V0 = x[0];
        hestonParam.Kappa[0] = x[1];
        hestonParam.Theta[0] = x[2];
        hestonParam.Epsilon[0] = x[3];
        hestonParam.Rho[0] = x[4];
    
        DoubleMatrix modelVol = AQLMathDisplacedHestonTDP::CalibrationHelperGL( datas, strikes, sgns, hestonParam );
        for(i=0;i<termSize;i++)
        {
            for(j=0;j<strikeSize;j++)
            {
                 y[i*strikeSize+j] = AQLMath::pow(modelVol[i][j] - vols[i][j],2);
            }
        }
        return y;
    };

private:
    size_t termSize;
    size_t strikeSize;
    std::vector<FXOptionData > datas;
    DoubleMatrix vols;
    DoubleMatrix strikes;
    IntMatrix sgns;
    HestonParams_TDP& hestonParam;
};

class HestonGlobalConstraint : public QuantLib::Constraint 
{
      private:
        class Impl : public QuantLib::Constraint::Impl 
        {
          public:
            bool test(const QuantLib::Array& params) const 
            {
                const QuantLib::Real V0 = params[0];
                const QuantLib::Real Kappa = params[1];
                const QuantLib::Real Theta = params[2];
                const QuantLib::Real Epsilon = params[3];
                const QuantLib::Real Rho = params[4];

                return (V0 > 0.0 && V0 <1. && Kappa > 0. && Kappa < HESTON_UPPER &&
                        Theta > 0. && Theta < HESTON_UPPER && Epsilon > 0. && Epsilon < HESTON_UPPER && 
                        Rho <= 1. && Rho >= -1.);
            }
        };
      public:
        HestonGlobalConstraint()
        : QuantLib::Constraint(boost::shared_ptr<QuantLib::Constraint::Impl>(
                                           new HestonGlobalConstraint::Impl)) {}
};

void AQLMathDisplacedHestonTDP::FXCalibrationHestonGlobal( const std::vector<FXOptionData >& datas,
                                                        const DoubleMatrix& vols,
                                                        const DoubleMatrix& strikes,
                                                        const IntMatrix& sgns,
                                                        HestonParams_TDP& hestonParam
                                                        )
 {
     //error check
     size_t termSize = datas.size();
     if( vols.size() != termSize || strikes.size() != termSize || sgns.size() != termSize )
     {
         AQLString msg("data sizes are not same!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
     }
      
     size_t strikeSize = vols[0].size();
     if( strikes[0].size() != strikeSize || sgns[0].size() != strikeSize )
     {
         AQLString msg("data sizes are not same!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
     }

     QuantLib::Array initialValues(5);
     initialValues[0] = hestonParam.V0;
     initialValues[1] = hestonParam.Kappa[0];
     initialValues[2] = hestonParam.Theta[0];
     initialValues[3] = hestonParam.Epsilon[0];
     initialValues[4] = hestonParam.Rho[0];

     HestonParams_TDP hestonParam_ = hestonParam;
     hestonParam_.V0 = hestonParam.V0;
     hestonParam_.T.resize(1); hestonParam_.T[0] = hestonParam.T[0];
     hestonParam_.Kappa.resize(1); hestonParam_.Kappa[0] = hestonParam.Kappa[0];
     hestonParam_.Theta.resize(1); hestonParam_.Theta[0] = hestonParam.Theta[0];
     hestonParam_.Epsilon.resize(1); hestonParam_.Epsilon[0] = hestonParam.Epsilon[0];
     hestonParam_.Rho.resize(1); hestonParam_.Rho[0] = hestonParam.Rho[0];

     HestonCalibratorGlobal suv_func( datas, vols, strikes, sgns, hestonParam_ );
     HestonGlobalConstraint constraint;
     QuantLib::LevenbergMarquardt optimizationMethod;
     QuantLib::EndCriteria endCriteria(400, 40, 1e-8, 1e-8, 1e-8);
     QuantLib::Problem problem(suv_func, constraint,initialValues);
     optimizationMethod.minimize(problem, endCriteria);
     QuantLib::Array result(problem.currentValue());

     hestonParam.V0 = result[0];
     for(size_t i=0;i<termSize;i++)
     {
		hestonParam.Kappa[i] = result[1];
		hestonParam.Theta[i] = result[2];
		hestonParam.Epsilon[i] = result[3];
		hestonParam.Rho[i] = result[4];
	 }
 };

class HestonCalibratorGlobalV0Fix : public QuantLib::CostFunction
{
public:
    /// constructor
    HestonCalibratorGlobalV0Fix( const std::vector<FXOptionData >& datas_,
                            const DoubleMatrix& vols_,
                            const DoubleMatrix& strikes_,
                            const IntMatrix& sgns_,
                            HestonParams_TDP& hestonParam_)
            : datas(datas_), vols(vols_), strikes(strikes_), sgns(sgns_), hestonParam(hestonParam_)
    {
        termSize = vols_.size();
        if( strikes_.size() != termSize || sgns_.size() != termSize )
        {
             AQLString msg("size is not supported!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }

        strikeSize = vols_[0].size();
        if( strikes_[0].size() != strikeSize || sgns_[0].size() != strikeSize )
        {
             AQLString msg("size is not supported!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }
    };
    virtual ~HestonCalibratorGlobalV0Fix(){};

    QuantLib::Real value(const QuantLib::Array& x) const 
    {
        QuantLib::Real ret=0;
        size_t i,j;

        hestonParam.Kappa[0] = x[1];
        hestonParam.Theta[0] = x[2];
        hestonParam.Epsilon[0] = x[3];
        hestonParam.Rho[0] = x[4];

        DoubleMatrix modelVol = AQLMathDisplacedHestonTDP::CalibrationHelperGL( datas, strikes, sgns, hestonParam );
        for(i=0;i<termSize;i++)
        {
            for(j=0;j<strikeSize;j++)
            {
                ret += AQLMath::pow(modelVol[i][j] - vols[i][j],2);
            }
        }
        return ret;
    };

    QuantLib::Array values(const QuantLib::Array& x) const
    {
        QuantLib::Array y(termSize*strikeSize);
        size_t i,j;
        
        hestonParam.Kappa[0] = x[0];
        hestonParam.Theta[0] = x[1];
        hestonParam.Epsilon[0] = x[2];
        hestonParam.Rho[0] = x[3];
    
        DoubleMatrix modelVol = AQLMathDisplacedHestonTDP::CalibrationHelperGL( datas, strikes, sgns, hestonParam );
        for(i=0;i<termSize;i++)
        {
            for(j=0;j<strikeSize;j++)
            {
                 y[i*strikeSize+j] = AQLMath::pow(modelVol[i][j] - vols[i][j],2);
            }
        }
        return y;
    };

private:
    size_t termSize;
    size_t strikeSize;
    std::vector<FXOptionData > datas;
    DoubleMatrix vols;
    DoubleMatrix strikes;
    IntMatrix sgns;
    HestonParams_TDP& hestonParam;
};

class HestonGlobalV0FixConstraint : public QuantLib::Constraint 
{
      private:
        class Impl : public QuantLib::Constraint::Impl 
        {
          public:
            bool test(const QuantLib::Array& params) const 
            {
                const QuantLib::Real Kappa = params[0];
                const QuantLib::Real Theta = params[1];
                const QuantLib::Real Epsilon = params[2];
                const QuantLib::Real Rho = params[3];

                return (Kappa > 0. && Kappa < HESTON_UPPER &&
                        Theta > 0. && Theta < HESTON_UPPER && Epsilon > 0. && Epsilon < HESTON_UPPER && 
                        Rho <= 1. && Rho >= -1.);
            }
        };
      public:
        HestonGlobalV0FixConstraint()
        : QuantLib::Constraint(boost::shared_ptr<QuantLib::Constraint::Impl>(
                                           new HestonGlobalV0FixConstraint::Impl)) {}
};

void AQLMathDisplacedHestonTDP::FXCalibrationHestonGlobalV0Fix( const std::vector<FXOptionData >& datas,
                                                             const DoubleMatrix& vols,
                                                             const DoubleMatrix& strikes,
                                                             const IntMatrix& sgns,
                                                             HestonParams_TDP& hestonParam
                                                             )
 {
     //error check
     size_t termSize = datas.size();
     if( vols.size() != termSize || strikes.size() != termSize || sgns.size() != termSize )
     {
         AQLString msg("data sizes are not same!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
     }
      
     size_t strikeSize = vols[0].size();
     if( strikes[0].size() != strikeSize || sgns[0].size() != strikeSize )
     {
         AQLString msg("data sizes are not same!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
     }

     QuantLib::Array initialValues(4);
     initialValues[0] = hestonParam.Kappa[0];
     initialValues[1] = hestonParam.Theta[0];
     initialValues[2] = hestonParam.Epsilon[0];
     initialValues[3] = hestonParam.Rho[0];

     HestonParams_TDP hestonParam_ = hestonParam;
     hestonParam_.V0 = hestonParam.V0;
     hestonParam_.T.resize(1); hestonParam_.T[0] = hestonParam.T[0];
     hestonParam_.Kappa.resize(1); hestonParam_.Kappa[0] = hestonParam.Kappa[0];
     hestonParam_.Theta.resize(1); hestonParam_.Theta[0] = hestonParam.Theta[0];
     hestonParam_.Epsilon.resize(1); hestonParam_.Epsilon[0] = hestonParam.Epsilon[0];
     hestonParam_.Rho.resize(1); hestonParam_.Rho[0] = hestonParam.Rho[0];

     HestonCalibratorGlobalV0Fix suv_func( datas, vols, strikes, sgns, hestonParam_ );
     HestonGlobalV0FixConstraint constraint;
     QuantLib::LevenbergMarquardt optimizationMethod;
     QuantLib::EndCriteria endCriteria(400, 40, 1e-8, 1e-8, 1e-8);
     QuantLib::Problem problem(suv_func, constraint,initialValues);
     optimizationMethod.minimize(problem, endCriteria);
     QuantLib::Array result(problem.currentValue());

     for(size_t i=0;i<termSize;i++)
     {
		hestonParam.Kappa[i] = result[0];
		hestonParam.Theta[i] = result[1];
		hestonParam.Epsilon[i] = result[2];
		hestonParam.Rho[i] = result[3];
	 }
 };

class HestonCalibratorGlobalKappaFix : public QuantLib::CostFunction
{
public:
    /// constructor
    HestonCalibratorGlobalKappaFix( const std::vector<FXOptionData >& datas_,
                            const DoubleMatrix& vols_,
                            const DoubleMatrix& strikes_,
                            const IntMatrix& sgns_,
                            HestonParams_TDP& hestonParam_)
            : datas(datas_), vols(vols_), strikes(strikes_), sgns(sgns_), hestonParam(hestonParam_)
    {
        termSize = vols_.size();
        if( strikes_.size() != termSize || sgns_.size() != termSize )
        {
             AQLString msg("size is not supported!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }

        strikeSize = vols_[0].size();
        if( strikes_[0].size() != strikeSize || sgns_[0].size() != strikeSize )
        {
             AQLString msg("size is not supported!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }
    };
    virtual ~HestonCalibratorGlobalKappaFix(){};

    QuantLib::Real value(const QuantLib::Array& x) const 
    {
        QuantLib::Real ret=0;
        size_t i,j;

        hestonParam.V0 = x[0]; 
        hestonParam.Theta[0] = x[1];
        hestonParam.Epsilon[0] = x[2];
        hestonParam.Rho[0] = x[3];

        DoubleMatrix modelVol = AQLMathDisplacedHestonTDP::CalibrationHelperGL( datas, strikes, sgns, hestonParam );
        for(i=0;i<termSize;i++)
        {
            for(j=0;j<strikeSize;j++)
            {
                ret += AQLMath::pow(modelVol[i][j] - vols[i][j],2);
            }
        }
        return ret;
    };

    QuantLib::Array values(const QuantLib::Array& x) const
    {
        QuantLib::Array y(termSize*strikeSize);
        size_t i,j;
        
        hestonParam.V0 = x[0];
        hestonParam.Theta[0] = x[1];
        hestonParam.Epsilon[0] = x[2];
        hestonParam.Rho[0] = x[3];
    
        DoubleMatrix modelVol = AQLMathDisplacedHestonTDP::CalibrationHelperGL( datas, strikes, sgns, hestonParam );
        for(i=0;i<termSize;i++)
        {
            for(j=0;j<strikeSize;j++)
            {
                 y[i*strikeSize+j] = AQLMath::pow(modelVol[i][j] - vols[i][j],2);
            }
        }
        return y;
    };

private:
    size_t termSize;
    size_t strikeSize;
    std::vector<FXOptionData > datas;
    DoubleMatrix vols;
    DoubleMatrix strikes;
    IntMatrix sgns;
    HestonParams_TDP& hestonParam;
};

class HestonGlobalKappaFixConstraint : public QuantLib::Constraint 
{
      private:
        class Impl : public QuantLib::Constraint::Impl 
        {
          public:
            bool test(const QuantLib::Array& params) const 
            {
                const QuantLib::Real V0 = params[0];
                const QuantLib::Real Theta = params[1];
                const QuantLib::Real Epsilon = params[2];
                const QuantLib::Real Rho = params[3];

                return (V0 > 0.0 && V0 <1. &&
                        Theta > 0. && Theta < HESTON_UPPER && Epsilon > 0. && Epsilon < HESTON_UPPER && 
                        Rho <= 1. && Rho >= -1.);
            }
        };
      public:
        HestonGlobalKappaFixConstraint()
        : QuantLib::Constraint(boost::shared_ptr<QuantLib::Constraint::Impl>(
                                           new HestonGlobalKappaFixConstraint::Impl)) {}
};

void AQLMathDisplacedHestonTDP::FXCalibrationHestonGlobalKappaFix( const std::vector<FXOptionData >& datas,
                                                                const DoubleMatrix& vols,
                                                                const DoubleMatrix& strikes,
                                                                const IntMatrix& sgns,
                                                                HestonParams_TDP& hestonParam
                                                                )
 {
     //error check
     size_t termSize = datas.size();
     if( vols.size() != termSize || strikes.size() != termSize || sgns.size() != termSize )
     {
         AQLString msg("data sizes are not same!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
     }
      
     size_t strikeSize = vols[0].size();
     if( strikes[0].size() != strikeSize || sgns[0].size() != strikeSize )
     {
         AQLString msg("data sizes are not same!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
     }

     QuantLib::Array initialValues(4);
     initialValues[0] = hestonParam.V0;
     initialValues[1] = hestonParam.Theta[0];
     initialValues[2] = hestonParam.Epsilon[0];
     initialValues[3] = hestonParam.Rho[0];

     HestonParams_TDP hestonParam_ = hestonParam;
     hestonParam_.V0 = hestonParam.V0;
     hestonParam_.T.resize(1); hestonParam_.T[0] = hestonParam.T[0];
     hestonParam_.Kappa.resize(1); hestonParam_.Kappa[0] = hestonParam.Kappa[0];
     hestonParam_.Theta.resize(1); hestonParam_.Theta[0] = hestonParam.Theta[0];
     hestonParam_.Epsilon.resize(1); hestonParam_.Epsilon[0] = hestonParam.Epsilon[0];
     hestonParam_.Rho.resize(1); hestonParam_.Rho[0] = hestonParam.Rho[0];

     HestonCalibratorGlobalKappaFix suv_func( datas, vols, strikes, sgns, hestonParam_ );
     HestonGlobalKappaFixConstraint constraint;
     QuantLib::LevenbergMarquardt optimizationMethod;
     QuantLib::EndCriteria endCriteria(400, 40, 1e-8, 1e-8, 1e-8);
     QuantLib::Problem problem(suv_func, constraint,initialValues);
     optimizationMethod.minimize(problem, endCriteria);
     QuantLib::Array result(problem.currentValue());

     hestonParam.V0 = result[0];
     for(size_t i=0;i<termSize;i++)
     {
        hestonParam.Kappa[i] = hestonParam.Kappa[0];
		hestonParam.Theta[i] = result[1];
		hestonParam.Epsilon[i] = result[2];
		hestonParam.Rho[i] = result[3];
	 }
 };

class HestonCalibratorGlobalThetaFix : public QuantLib::CostFunction
{
public:
    /// constructor
    HestonCalibratorGlobalThetaFix( const std::vector<FXOptionData >& datas_,
                            const DoubleMatrix& vols_,
                            const DoubleMatrix& strikes_,
                            const IntMatrix& sgns_,
                            HestonParams_TDP& hestonParam_)
            : datas(datas_), vols(vols_), strikes(strikes_), sgns(sgns_), hestonParam(hestonParam_)
    {
        termSize = vols_.size();
        if( strikes_.size() != termSize || sgns_.size() != termSize )
        {
             AQLString msg("size is not supported!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }

        strikeSize = vols_[0].size();
        if( strikes_[0].size() != strikeSize || sgns_[0].size() != strikeSize )
        {
             AQLString msg("size is not supported!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }
    };
    virtual ~HestonCalibratorGlobalThetaFix(){};

    QuantLib::Real value(const QuantLib::Array& x) const 
    {
        QuantLib::Real ret=0;
        size_t i,j;

        hestonParam.V0 = x[0]; 
        hestonParam.Kappa[0] = x[1];
        hestonParam.Epsilon[0] = x[2];
        hestonParam.Rho[0] = x[3];

        DoubleMatrix modelVol = AQLMathDisplacedHestonTDP::CalibrationHelperGL( datas, strikes, sgns, hestonParam );
        for(i=0;i<termSize;i++)
        {
            for(j=0;j<strikeSize;j++)
            {
                ret += AQLMath::pow(modelVol[i][j] - vols[i][j],2);
            }
        }
        return ret;
    };

    QuantLib::Array values(const QuantLib::Array& x) const
    {
        QuantLib::Array y(termSize*strikeSize);
        size_t i,j;
        
        hestonParam.V0 = x[0];
        hestonParam.Kappa[0] = x[1];
        hestonParam.Epsilon[0] = x[2];
        hestonParam.Rho[0] = x[3];
    
        DoubleMatrix modelVol = AQLMathDisplacedHestonTDP::CalibrationHelperGL( datas, strikes, sgns, hestonParam );
        for(i=0;i<termSize;i++)
        {
            for(j=0;j<strikeSize;j++)
            {
                 y[i*strikeSize+j] = AQLMath::pow(modelVol[i][j] - vols[i][j],2);
            }
        }
        return y;
    };

private:
    size_t termSize;
    size_t strikeSize;
    std::vector<FXOptionData > datas;
    DoubleMatrix vols;
    DoubleMatrix strikes;
    IntMatrix sgns;
    HestonParams_TDP& hestonParam;
};

class HestonGlobalThetaFixConstraint : public QuantLib::Constraint 
{
      private:
        class Impl : public QuantLib::Constraint::Impl 
        {
          public:
            bool test(const QuantLib::Array& params) const 
            {
                const QuantLib::Real V0 = params[0];
                const QuantLib::Real Kappa = params[1];
                const QuantLib::Real Epsilon = params[2];
                const QuantLib::Real Rho = params[3];

                return (V0 > 0.0 && V0 <1. && Kappa > 0. && Kappa < HESTON_UPPER &&
                        Epsilon > 0. && Epsilon < HESTON_UPPER && 
                        Rho <= 1. && Rho >= -1.);
            }
        };
      public:
        HestonGlobalThetaFixConstraint()
        : QuantLib::Constraint(boost::shared_ptr<QuantLib::Constraint::Impl>(
                                           new HestonGlobalThetaFixConstraint::Impl)) {}
};

void AQLMathDisplacedHestonTDP::FXCalibrationHestonGlobalThetaFix( const std::vector<FXOptionData >& datas,
                                                                const DoubleMatrix& vols,
                                                                const DoubleMatrix& strikes,
                                                                const IntMatrix& sgns,
                                                                HestonParams_TDP& hestonParam
                                                                )
 {
     //error check
     size_t termSize = datas.size();
     if( vols.size() != termSize || strikes.size() != termSize || sgns.size() != termSize )
     {
         AQLString msg("data sizes are not same!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
     }
      
     size_t strikeSize = vols[0].size();
     if( strikes[0].size() != strikeSize || sgns[0].size() != strikeSize )
     {
         AQLString msg("data sizes are not same!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
     }

     QuantLib::Array initialValues(4);
     initialValues[0] = hestonParam.V0;
     initialValues[1] = hestonParam.Kappa[0];
     initialValues[2] = hestonParam.Epsilon[0];
     initialValues[3] = hestonParam.Rho[0];

     HestonParams_TDP hestonParam_ = hestonParam;
     hestonParam_.V0 = hestonParam.V0;
     hestonParam_.T.resize(1); hestonParam_.T[0] = hestonParam.T[0];
     hestonParam_.Kappa.resize(1); hestonParam_.Kappa[0] = hestonParam.Kappa[0];
     hestonParam_.Theta.resize(1); hestonParam_.Theta[0] = hestonParam.Theta[0];
     hestonParam_.Epsilon.resize(1); hestonParam_.Epsilon[0] = hestonParam.Epsilon[0];
     hestonParam_.Rho.resize(1); hestonParam_.Rho[0] = hestonParam.Rho[0];

     HestonCalibratorGlobalThetaFix suv_func( datas, vols, strikes, sgns, hestonParam_ );
     HestonGlobalThetaFixConstraint constraint;
     QuantLib::LevenbergMarquardt optimizationMethod;
     QuantLib::EndCriteria endCriteria(400, 40, 1e-8, 1e-8, 1e-8);
     QuantLib::Problem problem(suv_func, constraint,initialValues);
     optimizationMethod.minimize(problem, endCriteria);
     QuantLib::Array result(problem.currentValue());

     hestonParam.V0 = result[0];
     for(size_t i=0;i<termSize;i++)
     {
		hestonParam.Kappa[i] = result[1];
        hestonParam.Theta[i] = hestonParam.Theta[0];
		hestonParam.Epsilon[i] = result[2];
		hestonParam.Rho[i] = result[3];
	 }
 };

class HestonCalibratorTDP : public QuantLib::CostFunction
{
public:
    /// constructor
    HestonCalibratorTDP( const std::vector<FXOptionData >& datas_,
						 const DoubleMatrix& vols_,
						 const DoubleMatrix& strikes_,
						 const IntMatrix& sgns_,
						 HestonParams_TDP& hestonParam_ )
            : datas(datas_), vols(vols_), strikes(strikes_), sgns(sgns_), hestonParam(hestonParam_)
    {
        termSize = vols_.size();
        if( strikes_.size() != termSize || sgns_.size() != termSize || hestonParam.T.size() != termSize )
        {
             AQLString msg("size is not supported!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }

        strikeSize = vols_[0].size();
        if( strikes_[0].size() != strikeSize || sgns_[0].size() != strikeSize )
        {
             AQLString msg("size is not supported!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }
    };
    virtual ~HestonCalibratorTDP(){};

    QuantLib::Real value(const QuantLib::Array& x) const 
    {
        QuantLib::Real y=0;
        size_t i,j;

        hestonParam.V0 = x[termSize*4];
        for(i=0;i<termSize;i++)
        {
            hestonParam.Kappa[i] = x[i*4];
			hestonParam.Theta[i] = x[i*4+1]; 
			hestonParam.Epsilon[i] = x[i*4+2];
			hestonParam.Rho[i] = x[i*4+3];
        }

        DoubleMatrix modelVol = AQLMathDisplacedHestonTDP::CalibrationHelper( datas, strikes, sgns, hestonParam );
        for(i=0;i<termSize;i++)
        {
            for(j=0;j<strikeSize;j++)
            {
                y += AQLMath::pow(modelVol[i][j] - vols[i][j],2);
            }
        }
        return y;
    };

    QuantLib::Array values(const QuantLib::Array& x) const
    {
        QuantLib::Array y(termSize * strikeSize);
        //double modelVol;
        size_t i,j;

        hestonParam.V0 = x[termSize*4];
        for(i=0;i<termSize;i++)
        {
            hestonParam.Kappa[i] = x[i*4];
			hestonParam.Theta[i] = x[i*4+1]; 
			hestonParam.Epsilon[i] = x[i*4+2];
			hestonParam.Rho[i] = x[i*4+3];
        }

        DoubleMatrix modelVol = AQLMathDisplacedHestonTDP::CalibrationHelper( datas, strikes, sgns, hestonParam);
        for(i=0;i<termSize;i++)
        {
			for(j=0;j<strikeSize;j++)
			{
				y[i * strikeSize + j] = AQLMath::pow(modelVol[i][j] - vols[i][j],2);
			}
		}
        return y;
    };
    
private:
    size_t termSize;
    size_t strikeSize;
    std::vector<FXOptionData > datas;
    DoubleMatrix vols;
    DoubleMatrix strikes;
    IntMatrix sgns;
    HestonParams_TDP& hestonParam;
};

class HestonTDPConstraint : public QuantLib::Constraint 
{
      private:
        class Impl : public QuantLib::Constraint::Impl 
        {
          public:
            bool test(const QuantLib::Array& params) const 
            {
				size_t termSize = (params.size() - 1) / 4;
                for(size_t i=0;i<termSize;i++)
				{
					if( params[i*4]<=0. || params[i*4]>=HESTON_UPPER || 
                        params[1+i*4]<=0. || params[1+i*4]>=HESTON_UPPER || 
                        params[2+i*4]<=0. || params[2+i*4]>=HESTON_UPPER || 
					    params[3+i*4]<-1. || params[3+i*4]>1. ) 
                        return false;
				}
				if( params[termSize*4]<0.0 || params[termSize*4]>1.0 ) return false;

				return true;
            }
        };
      public:
        HestonTDPConstraint()
        : QuantLib::Constraint(boost::shared_ptr<QuantLib::Constraint::Impl>(
                                           new HestonTDPConstraint::Impl)) {}
};

void AQLMathDisplacedHestonTDP::FXCalibrationHestonTDP( const std::vector<FXOptionData >& datas,
													 const DoubleMatrix& vols,
													 const DoubleMatrix& strikes,
													 const IntMatrix& sgns,
													 HestonParams_TDP& hestonParam
													 )
 {
     //error check
     size_t termSize = datas.size();
     if( vols.size() != termSize || strikes.size() != termSize || sgns.size() != termSize )
     {
         AQLString msg("data sizes are not same!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
     }
      
     size_t strikeSize = vols[0].size();
     if( strikes[0].size() != strikeSize || sgns[0].size() != strikeSize )
     {
         AQLString msg("data sizes are not same!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
     }

     size_t i;
     QuantLib::Array initialValues(termSize * 4 + 1);
	 initialValues[termSize * 4] = hestonParam.V0;
     for(i=0;i<termSize;i++)
     {
         initialValues[i*4] = hestonParam.Kappa[i];
         initialValues[i*4+1] = hestonParam.Theta[i];
         initialValues[i*4+2] = hestonParam.Epsilon[i];
         initialValues[i*4+3] = hestonParam.Rho[i];
     }

     HestonCalibratorTDP suv_func(datas, vols, strikes, sgns, hestonParam);
     HestonTDPConstraint constraint;
     QuantLib::LevenbergMarquardt optimizationMethod(1e-8, 1e-8, 1e-8);
     QuantLib::EndCriteria endCriteria(400, 40, 1e-8, 1e-8, 1e-8);
	 QuantLib::Problem problem(suv_func, constraint,initialValues);
	 optimizationMethod.minimize(problem, endCriteria);
	 //Get Result
	 QuantLib::Array result(problem.currentValue());
	 hestonParam.V0 = result[termSize * 4];
	 for(i=0;i<termSize;i++)
     {
		hestonParam.Kappa[i] = result[4*i];
		hestonParam.Theta[i] = result[4*i+1];
		hestonParam.Epsilon[i] = result[4*i+2];
		hestonParam.Rho[i] = result[4*i+3];
	 }
 };

class HestonCalibratorTDPV0Fix : public QuantLib::CostFunction
{
public:
    /// constructor
    HestonCalibratorTDPV0Fix( const std::vector<FXOptionData >& datas_,
						      const DoubleMatrix& vols_,
						      const DoubleMatrix& strikes_,
						      const IntMatrix& sgns_,
						      HestonParams_TDP& hestonParam_ )
            : datas(datas_), vols(vols_), strikes(strikes_), sgns(sgns_), hestonParam(hestonParam_)
    {
        termSize = vols_.size();
        if( strikes_.size() != termSize || sgns_.size() != termSize || hestonParam.T.size() != termSize )
        {
             AQLString msg("size is not supported!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }

        strikeSize = vols_[0].size();
        if( strikes_[0].size() != strikeSize || sgns_[0].size() != strikeSize )
        {
             AQLString msg("size is not supported!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }
    };
    virtual ~HestonCalibratorTDPV0Fix(){};

    QuantLib::Real value(const QuantLib::Array& x) const 
    {
        QuantLib::Real y=0;
        size_t i,j;

        for(i=0;i<termSize;i++)
        {
            hestonParam.Kappa[i] = x[i*4];
			hestonParam.Theta[i] = x[i*4+1]; 
			hestonParam.Epsilon[i] = x[i*4+2];
			hestonParam.Rho[i] = x[i*4+3];
        }

        DoubleMatrix modelVol = AQLMathDisplacedHestonTDP::CalibrationHelper( datas, strikes, sgns, hestonParam );
        for(i=0;i<termSize;i++)
        {
            for(j=0;j<strikeSize;j++)
            {
                y += AQLMath::pow(modelVol[i][j] - vols[i][j],2);
            }
        }
        return y;
    };

    QuantLib::Array values(const QuantLib::Array& x) const
    {
        QuantLib::Array y(termSize * strikeSize);
        //double modelVol;
        size_t i,j;

        for(i=0;i<termSize;i++)
        {
            hestonParam.Kappa[i] = x[i*4];
			hestonParam.Theta[i] = x[i*4+1]; 
			hestonParam.Epsilon[i] = x[i*4+2];
			hestonParam.Rho[i] = x[i*4+3];
        }

        DoubleMatrix modelVol = AQLMathDisplacedHestonTDP::CalibrationHelper( datas, strikes, sgns, hestonParam);
        for(i=0;i<termSize;i++)
        {
			for(j=0;j<strikeSize;j++)
			{
				y[i * strikeSize + j] = AQLMath::pow(modelVol[i][j] - vols[i][j],2);
			}
		}
        return y;
    };
    
private:
    size_t termSize;
    size_t strikeSize;
    std::vector<FXOptionData > datas;
    DoubleMatrix vols;
    DoubleMatrix strikes;
    IntMatrix sgns;
    HestonParams_TDP& hestonParam;
};

class HestonTDPV0FixConstraint : public QuantLib::Constraint 
{
      private:
        class Impl : public QuantLib::Constraint::Impl 
        {
          public:
            bool test(const QuantLib::Array& params) const 
            {
				size_t termSize = params.size() / 4;
                for(size_t i=0;i<termSize;i++)
				{
					if( params[i*4]<=0. || params[i*4]>=HESTON_UPPER || 
                        params[1+i*4]<=0. || params[1+i*4]>=HESTON_UPPER || 
                        params[2+i*4]<=0. || params[2+i*4]>=HESTON_UPPER || 
					    params[3+i*4]<-1. || params[3+i*4]>1. ) 
                        return false;
				}

				return true;
            }
        };
      public:
        HestonTDPV0FixConstraint()
        : QuantLib::Constraint(boost::shared_ptr<QuantLib::Constraint::Impl>(
                                           new HestonTDPV0FixConstraint::Impl)) {}
};

void AQLMathDisplacedHestonTDP::FXCalibrationHestonTDPV0Fix( const std::vector<FXOptionData >& datas,
													      const DoubleMatrix& vols,
													      const DoubleMatrix& strikes,
													      const IntMatrix& sgns,
													      HestonParams_TDP& hestonParam
													      )
 {
     //error check
     size_t termSize = datas.size();
     if( vols.size() != termSize || strikes.size() != termSize || sgns.size() != termSize )
     {
         AQLString msg("data sizes are not same!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
     }
      
     size_t strikeSize = vols[0].size();
     if( strikes[0].size() != strikeSize || sgns[0].size() != strikeSize )
     {
         AQLString msg("data sizes are not same!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
     }

     size_t i;
     QuantLib::Array initialValues(termSize * 4);
     for(i=0;i<termSize;i++)
     {
         initialValues[i*4] = hestonParam.Kappa[i];
         initialValues[i*4+1] = hestonParam.Theta[i];
         initialValues[i*4+2] = hestonParam.Epsilon[i];
         initialValues[i*4+3] = hestonParam.Rho[i];
     }

     HestonCalibratorTDPV0Fix suv_func(datas, vols, strikes, sgns, hestonParam);
     HestonTDPV0FixConstraint constraint;
     QuantLib::LevenbergMarquardt optimizationMethod(1e-8, 1e-8, 1e-8);
     QuantLib::EndCriteria endCriteria(400, 40, 1e-8, 1e-8, 1e-8);
	 QuantLib::Problem problem(suv_func, constraint,initialValues);
	 optimizationMethod.minimize(problem, endCriteria);
	 //Get Result
	 QuantLib::Array result(problem.currentValue());
	 for(i=0;i<termSize;i++)
     {
		hestonParam.Kappa[i] = result[4*i];
		hestonParam.Theta[i] = result[4*i+1];
		hestonParam.Epsilon[i] = result[4*i+2];
		hestonParam.Rho[i] = result[4*i+3];
	 }
 };

class HestonCalibratorTDPRhoFix : public QuantLib::CostFunction
{
public:
    /// constructor
    HestonCalibratorTDPRhoFix( const std::vector<FXOptionData >& datas_,
							   const DoubleMatrix& vols_,
							   const DoubleMatrix& strikes_,
							   const IntMatrix& sgns_,
							   HestonParams_TDP& hestonParam_ )
            : datas(datas_), vols(vols_), strikes(strikes_), sgns(sgns_), hestonParam(hestonParam_)
    {
        termSize = vols_.size();
        if( strikes_.size() != termSize || sgns_.size() != termSize || hestonParam.T.size() != termSize )
        {
             AQLString msg("size is not supported!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }

        strikeSize = vols_[0].size();
        if( strikes_[0].size() != strikeSize || sgns_[0].size() != strikeSize )
        {
             AQLString msg("size is not supported!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }
    };
    virtual ~HestonCalibratorTDPRhoFix(){};

    QuantLib::Real value(const QuantLib::Array& x) const 
    {
        QuantLib::Real y=0;
        size_t i,j;

        hestonParam.V0 = x[termSize*3];
        for(i=0;i<termSize;i++)
        {
            hestonParam.Kappa[i] = x[i*3];
			hestonParam.Theta[i] = x[i*3+1]; 
			hestonParam.Epsilon[i] = x[i*3+2];
        }

        DoubleMatrix modelVol = AQLMathDisplacedHestonTDP::CalibrationHelper( datas, strikes, sgns, hestonParam );
        for(i=0;i<termSize;i++)
        {
            for(j=0;j<strikeSize;j++)
            {
                y += AQLMath::pow(modelVol[i][j] - vols[i][j],2);
            }
        }
        return y;
    };

    QuantLib::Array values(const QuantLib::Array& x) const
    {
        QuantLib::Array y(termSize * strikeSize);
        size_t i,j;

        hestonParam.V0 = x[termSize*3];
        for(i=0;i<termSize;i++)
        {
            hestonParam.Kappa[i] = x[i*3];
			hestonParam.Theta[i] = x[i*3+1]; 
			hestonParam.Epsilon[i] = x[i*3+2];
        }

        DoubleMatrix modelVol = AQLMathDisplacedHestonTDP::CalibrationHelper( datas, strikes, sgns, hestonParam);
		for(i=0;i<termSize;i++)
        {
			for(j=0;j<strikeSize;j++)
			{
				y[i * strikeSize + j] = AQLMath::pow(modelVol[i][j] - vols[i][j],2);
			}
		}
        return y;
    };
    
private:
    size_t termSize;
    size_t strikeSize;
    std::vector<FXOptionData > datas;
    DoubleMatrix vols;
    DoubleMatrix strikes;
    IntMatrix sgns;
    HestonParams_TDP& hestonParam;
};

class HestonTDPRhoFixConstraint : public QuantLib::Constraint 
{
      private:
        class Impl : public QuantLib::Constraint::Impl 
        {
          public:
            bool test(const QuantLib::Array& params) const 
            {
				size_t termSize = (params.size() - 1) / 3;
                for(size_t i=0;i<termSize;i++)
				{
                    if(params[i*3]<=0. || params[i*3]>=HESTON_UPPER || 
                       params[1+i*3]<=0. || params[1+i*3]>=HESTON_UPPER || 
                       params[2+i*3]<=0. || params[2+i*3]>=HESTON_UPPER ) 
                       return false;
				}
				if(params[termSize*3]<0.0 || params[termSize*3]>1.0 ) return false;

				return true;
            }
        };
      public:
        HestonTDPRhoFixConstraint()
        : QuantLib::Constraint(boost::shared_ptr<QuantLib::Constraint::Impl>(
                                           new HestonTDPRhoFixConstraint::Impl)) {}
};

void AQLMathDisplacedHestonTDP::FXCalibrationHestonTDPRhoFix( const std::vector<FXOptionData >& datas,
														   const DoubleMatrix& vols,
														   const DoubleMatrix& strikes,
														   const IntMatrix& sgns,
														   HestonParams_TDP& hestonParam
														   )
 {
     //error check
     size_t termSize = datas.size();
     if( vols.size() != termSize || strikes.size() != termSize || sgns.size() != termSize )
     {
         AQLString msg("data sizes are not same!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
     }
      
     size_t strikeSize = vols[0].size();
     if( strikes[0].size() != strikeSize || sgns[0].size() != strikeSize )
     {
         AQLString msg("data sizes are not same!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
     }

     QuantLib::Array initialValues(termSize * 3 + 1);
     size_t i;
	 initialValues[termSize * 3] = hestonParam.V0;
     for(i=0;i<termSize;i++)
     {
         initialValues[i*3] = hestonParam.Kappa[i];
         initialValues[i*3+1] = hestonParam.Theta[i];
         initialValues[i*3+2] = hestonParam.Epsilon[i];
     }

     HestonCalibratorTDPRhoFix suv_func(datas, vols, strikes, sgns, hestonParam);
     HestonTDPRhoFixConstraint constraint;
     QuantLib::LevenbergMarquardt optimizationMethod;
     QuantLib::EndCriteria endCriteria(400, 40, 1e-8, 1e-8, 1e-8);
	 QuantLib::Problem problem(suv_func, constraint,initialValues);
	 optimizationMethod.minimize(problem, endCriteria);
	 //Get Result
	 QuantLib::Array result(problem.currentValue());
	 hestonParam.V0 = result[termSize * 3];
	 for(i=0;i<termSize;i++)
     {
		hestonParam.Kappa[i] = result[3*i];
		hestonParam.Theta[i] = result[3*i+1];
		hestonParam.Epsilon[i] = result[3*i+2];
	 }
 };

class HestonCalibratorTDPKappaFix : public QuantLib::CostFunction
{
public:
    /// constructor
    HestonCalibratorTDPKappaFix( const std::vector<FXOptionData >& datas_,
							     const DoubleMatrix& vols_,
							     const DoubleMatrix& strikes_,
							     const IntMatrix& sgns_,
							     HestonParams_TDP& hestonParam_ )
            : datas(datas_), vols(vols_), strikes(strikes_), sgns(sgns_), hestonParam(hestonParam_)
    {
        termSize = vols_.size();
        if( strikes_.size() != termSize || sgns_.size() != termSize || hestonParam.T.size() != termSize )
        {
             AQLString msg("size is not supported!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }

        strikeSize = vols_[0].size();
        if( strikes_[0].size() != strikeSize || sgns_[0].size() != strikeSize )
        {
             AQLString msg("size is not supported!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }
    };
    virtual ~HestonCalibratorTDPKappaFix(){};

    QuantLib::Real value(const QuantLib::Array& x) const 
    {
        QuantLib::Real y=0;
        size_t i,j;

        hestonParam.V0 = x[termSize*3];
        for(i=0;i<termSize;i++)
        {
            hestonParam.Theta[i] = x[i*3];
			hestonParam.Epsilon[i] = x[i*3+1]; 
			hestonParam.Rho[i] = x[i*3+2];
        }

        DoubleMatrix modelVol = AQLMathDisplacedHestonTDP::CalibrationHelper( datas, strikes, sgns, hestonParam);
        for(i=0;i<termSize;i++)
        {
            for(j=0;j<strikeSize;j++)
            {
                y += AQLMath::pow(modelVol[i][j] - vols[i][j],2);
            }
        }
        return y;
    };

    QuantLib::Array values(const QuantLib::Array& x) const
    {
        QuantLib::Array y(termSize * strikeSize);
        size_t i,j;

        hestonParam.V0 = x[termSize*3];
        for(i=0;i<termSize;i++)
        {
            hestonParam.Theta[i] = x[i*3];
			hestonParam.Epsilon[i] = x[i*3+1]; 
			hestonParam.Rho[i] = x[i*3+2];
        }

        DoubleMatrix modelVol = AQLMathDisplacedHestonTDP::CalibrationHelper( datas, strikes, sgns, hestonParam);
		for(i=0;i<termSize;i++)
        {
			for(j=0;j<strikeSize;j++)
			{
				y[i * strikeSize + j] = AQLMath::pow(modelVol[i][j] - vols[i][j],2);
			}
		}
        return y;
    };
    
private:
    size_t termSize;
    size_t strikeSize;
    std::vector<FXOptionData > datas;
    DoubleMatrix vols;
    DoubleMatrix strikes;
    IntMatrix sgns;
    HestonParams_TDP& hestonParam;
};

class HestonTDPKappaFixConstraint : public QuantLib::Constraint 
{
      private:
        class Impl : public QuantLib::Constraint::Impl 
        {
          public:
            bool test(const QuantLib::Array& params) const 
            {
				size_t termSize = (params.size() - 1) / 3;
                for(size_t i=0;i<termSize;i++)
				{
					if(params[i*3]<=0. || params[i*3]>=HESTON_UPPER || params[1+i*3]<=0. || 
					params[1+i*3]>=HESTON_UPPER || params[2+i*3]<-1. || params[2+i*3]>1.) 
					return false;
				}
				if(params[termSize*3]<0.0 || params[termSize*3]>1.0 ) return false;

				return true;
            }
        };
      public:
        HestonTDPKappaFixConstraint()
        : QuantLib::Constraint(boost::shared_ptr<QuantLib::Constraint::Impl>(
                                           new HestonTDPKappaFixConstraint::Impl)) {}
};

void AQLMathDisplacedHestonTDP::FXCalibrationHestonTDPKappaFix( const std::vector<FXOptionData >& datas,
														     const DoubleMatrix& vols,
														     const DoubleMatrix& strikes,
														     const IntMatrix& sgns,
														     HestonParams_TDP& hestonParam
														     )
 {
     //error check
     size_t termSize = datas.size();
     if( vols.size() != termSize || strikes.size() != termSize || sgns.size() != termSize )
     {
         AQLString msg("data sizes are not same!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
     }
      
     size_t strikeSize = vols[0].size();
     if( strikes[0].size() != strikeSize || sgns[0].size() != strikeSize )
     {
         AQLString msg("data sizes are not same!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
     }


     QuantLib::Array initialValues(termSize * 3 + 1);
     size_t i;
	 initialValues[termSize * 3] = hestonParam.V0;
     for(i=0;i<termSize;i++)
     {
         initialValues[i*3] = hestonParam.Theta[i];
         initialValues[i*3+1] = hestonParam.Epsilon[i];
         initialValues[i*3+2] = hestonParam.Rho[i];
     }

     HestonCalibratorTDPKappaFix suv_func(datas, vols, strikes, sgns, hestonParam);
     HestonTDPKappaFixConstraint constraint;
     QuantLib::LevenbergMarquardt optimizationMethod;
     QuantLib::EndCriteria endCriteria(400, 40, 1e-8, 1e-8, 1e-8);
	 QuantLib::Problem problem(suv_func, constraint,initialValues);
	 optimizationMethod.minimize(problem, endCriteria);
	 //Get Result
	 QuantLib::Array result(problem.currentValue());
	 hestonParam.V0 = result[termSize * 3];
	 for(i=0;i<termSize;i++)
     {
		hestonParam.Theta[i] = result[3*i];
		hestonParam.Epsilon[i] = result[3*i+1];
		hestonParam.Rho[i] = result[3*i+2];
	 }
 };

class HestonCalibratorTDPThetaFix : public QuantLib::CostFunction
{
public:
    /// constructor
    HestonCalibratorTDPThetaFix( const std::vector<FXOptionData >& datas_,
							     const DoubleMatrix& vols_,
							     const DoubleMatrix& strikes_,
							     const IntMatrix& sgns_,
							     HestonParams_TDP& hestonParam_ )
            : datas(datas_), vols(vols_), strikes(strikes_), sgns(sgns_), hestonParam(hestonParam_)
    {
        termSize = vols_.size();
        if( strikes_.size() != termSize || sgns_.size() != termSize || hestonParam.T.size() != termSize )
        {
             AQLString msg("size is not supported!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }

        strikeSize = vols_[0].size();
        if( strikes_[0].size() != strikeSize || sgns_[0].size() != strikeSize )
        {
             AQLString msg("size is not supported!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }
    };
    virtual ~HestonCalibratorTDPThetaFix(){};

    QuantLib::Real value(const QuantLib::Array& x) const 
    {
        QuantLib::Real y=0;
        size_t i,j;

        hestonParam.V0 = x[termSize*3];
        for(i=0;i<termSize;i++)
        {
            hestonParam.Kappa[i] = x[i*3];
			hestonParam.Epsilon[i] = x[i*3+1]; 
			hestonParam.Rho[i] = x[i*3+2];
        }

        DoubleMatrix modelVol = AQLMathDisplacedHestonTDP::CalibrationHelper( datas, strikes, sgns, hestonParam);
        for(i=0;i<termSize;i++)
        {
            for(j=0;j<strikeSize;j++)
            {
                y += AQLMath::pow(modelVol[i][j] - vols[i][j],2);
            }
        }
        return y;
    };

    QuantLib::Array values(const QuantLib::Array& x) const
    {
        QuantLib::Array y(termSize * strikeSize);
        size_t i,j;

        hestonParam.V0 = x[termSize*3];
        for(i=0;i<termSize;i++)
        {
            hestonParam.Kappa[i] = x[i*3];
			hestonParam.Epsilon[i] = x[i*3+1]; 
			hestonParam.Rho[i] = x[i*3+2];
        }

        DoubleMatrix modelVol = AQLMathDisplacedHestonTDP::CalibrationHelper( datas, strikes, sgns, hestonParam);
		for(i=0;i<termSize;i++)
        {
			for(j=0;j<strikeSize;j++)
			{
				y[i * strikeSize + j] = AQLMath::pow(modelVol[i][j] - vols[i][j],2);
			}
		}
        return y;
    };
    
private:
    size_t termSize;
    size_t strikeSize;
    std::vector<FXOptionData > datas;
    DoubleMatrix vols;
    DoubleMatrix strikes;
    IntMatrix sgns;
    HestonParams_TDP& hestonParam;
};

class HestonTDPThetaFixConstraint : public QuantLib::Constraint 
{
      private:
        class Impl : public QuantLib::Constraint::Impl 
        {
          public:
            bool test(const QuantLib::Array& params) const 
            {
				size_t termSize = (params.size() - 1) / 3;
                for(size_t i=0;i<termSize;i++)
				{
					if(params[i*3]<=0. || params[i*3]>=HESTON_UPPER || params[1+i*3]<=0. || 
					params[1+i*3]>=HESTON_UPPER || params[2+i*3]<-1. || params[2+i*3]>1.) 
					return false;
				}
				if(params[termSize*3]<0.0 || params[termSize*3]>1.0 ) return false;

				return true;
            }
        };
      public:
        HestonTDPThetaFixConstraint()
        : QuantLib::Constraint(boost::shared_ptr<QuantLib::Constraint::Impl>(
                                           new HestonTDPThetaFixConstraint::Impl)) {}
};

void AQLMathDisplacedHestonTDP::FXCalibrationHestonTDPThetaFix( const std::vector<FXOptionData >& datas,
														     const DoubleMatrix& vols,
														     const DoubleMatrix& strikes,
														     const IntMatrix& sgns,
														     HestonParams_TDP& hestonParam
														     )
 {
     //error check
     size_t termSize = datas.size();
     if( vols.size() != termSize || strikes.size() != termSize || sgns.size() != termSize )
     {
         AQLString msg("data sizes are not same!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
     }
      
     size_t strikeSize = vols[0].size();
     if( strikes[0].size() != strikeSize || sgns[0].size() != strikeSize )
     {
         AQLString msg("data sizes are not same!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
     }


     QuantLib::Array initialValues(termSize * 3 + 1);
     size_t i;
	 initialValues[termSize * 3] = hestonParam.V0;
     for(i=0;i<termSize;i++)
     {
         initialValues[i*3] = hestonParam.Kappa[i];
         initialValues[i*3+1] = hestonParam.Epsilon[i];
         initialValues[i*3+2] = hestonParam.Rho[i];
     }

     HestonCalibratorTDPThetaFix suv_func(datas, vols, strikes, sgns, hestonParam);
     HestonTDPThetaFixConstraint constraint;
     QuantLib::LevenbergMarquardt optimizationMethod;
     QuantLib::EndCriteria endCriteria(400, 40, 1e-8, 1e-8, 1e-8);
	 QuantLib::Problem problem(suv_func, constraint,initialValues);
	 optimizationMethod.minimize(problem, endCriteria);
	 //Get Result
	 QuantLib::Array result(problem.currentValue());
	 hestonParam.V0 = result[termSize * 3];
	 for(i=0;i<termSize;i++)
     {
		hestonParam.Kappa[i] = result[3*i];
		hestonParam.Epsilon[i] = result[3*i+1];
		hestonParam.Rho[i] = result[3*i+2];
	 }
 };

class HestonCalibratorTDP_Boot : public QuantLib::CostFunction
{
public:
    /// constructor
    HestonCalibratorTDP_Boot( const std::vector<FXOptionData >& datas_,
							  const DoubleMatrix& vols_,
							  const DoubleMatrix& strikes_,
							  const IntMatrix& sgns_,
							  HestonParams_TDP& hestonParam_,
							  int caliIndex_ )
            : datas(datas_), vols(vols_), strikes(strikes_), sgns(sgns_), hestonParam(hestonParam_), caliIndex(caliIndex_)
    {
        termSize = vols_.size();
        if( strikes_.size() != termSize || sgns_.size() != termSize || hestonParam.T.size() != termSize )
        {
             AQLString msg("size is not supported!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }

        strikeSize = vols_[0].size();
        if( strikes_[0].size() != strikeSize || sgns_[0].size() != strikeSize )
        {
             AQLString msg("size is not supported!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }

        if( caliIndex_ >= termSize)
        {
            AQLString msg("calibNum is over termSize!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);    
        }
    };
    virtual ~HestonCalibratorTDP_Boot(){};

    QuantLib::Real value(const QuantLib::Array& x) const 
    {
        double modelVol;
        QuantLib::Real y=0;
        size_t j;

        hestonParam.Kappa[caliIndex] = x[0];
        hestonParam.Theta[caliIndex] = x[1];
        hestonParam.Epsilon[caliIndex] = x[2];
        hestonParam.Rho[caliIndex] = x[3];

        for(j=0;j<strikeSize;j++)
        {
            modelVol = AQLMathDisplacedHestonTDP::BS_DDHestonImpVol_TDP(datas[caliIndex].T, datas[caliIndex].F, strikes[caliIndex][j], 
                            datas[caliIndex].Pd, sgns[caliIndex][j], 1., hestonParam );
            y += AQLMath::pow(modelVol - vols[caliIndex][j],2);
        }
        return y;
    };

    QuantLib::Array values(const QuantLib::Array& x) const
    {
        QuantLib::Array y(strikeSize);
        double modelVol;
        size_t j;

        hestonParam.Kappa[caliIndex] = x[0];
        hestonParam.Theta[caliIndex] = x[1];
        hestonParam.Epsilon[caliIndex] = x[2];
        hestonParam.Rho[caliIndex] = x[3];

        for(j=0;j<strikeSize;j++)
        {
            modelVol = AQLMathDisplacedHestonTDP::BS_DDHestonImpVol_TDP(datas[caliIndex].T, datas[caliIndex].F, strikes[caliIndex][j], 
                            datas[caliIndex].Pd, sgns[caliIndex][j], 1., hestonParam );
           y[j] = AQLMath::pow(modelVol - vols[caliIndex][j],2);
        }
        return y;
    };
    
    int caliIndex;
private:
    size_t termSize;
    size_t strikeSize;
    std::vector<FXOptionData > datas;
    DoubleMatrix vols;
    DoubleMatrix strikes;
    IntMatrix sgns;
    HestonParams_TDP& hestonParam;
};

class HestonTDP_BootConstraint : public QuantLib::Constraint 
{
      private:
        class Impl : public QuantLib::Constraint::Impl 
        {
          public:
            bool test(const QuantLib::Array& params) const 
            {
                const QuantLib::Real Kappa = params[0];
                const QuantLib::Real Theta = params[1];
                const QuantLib::Real Epsilon = params[2];
                const QuantLib::Real Rho = params[3];

                return (Kappa > 0. && Kappa < HESTON_UPPER && Theta > 0. && Theta < HESTON_UPPER && 
                        Epsilon > 0. && Epsilon < HESTON_UPPER && Rho <= 1. && Rho >= -1.);
            }
        };
      public:
        HestonTDP_BootConstraint()
        : QuantLib::Constraint(boost::shared_ptr<QuantLib::Constraint::Impl>(
                                           new HestonTDP_BootConstraint::Impl)) {}
};

void AQLMathDisplacedHestonTDP::FXCalibrationHestonTDP_Boot( const std::vector<FXOptionData >& datas,
														  const DoubleMatrix& vols,
													      const DoubleMatrix& strikes,
												          const IntMatrix& sgns,
													      HestonParams_TDP& hestonParam
														  )
 {
     //error check
     size_t termSize = datas.size();
     if( vols.size() != termSize || strikes.size() != termSize || sgns.size() != termSize )
     {
         AQLString msg("data sizes are not same!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
     }
      
     size_t strikeSize = vols[0].size();
     if( strikes[0].size() != strikeSize || sgns[0].size() != strikeSize )
     {
         AQLString msg("data sizes are not same!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
     }

     size_t i;
     std::vector<QuantLib::Array > initialValues(termSize, QuantLib::Array(4));
     for(i=0;i<termSize;i++)
     {
         initialValues[i][0] = hestonParam.Kappa[i];
         initialValues[i][1] = hestonParam.Theta[i];
         initialValues[i][2] = hestonParam.Epsilon[i];
         initialValues[i][3] = hestonParam.Rho[i];
     }

     HestonCalibratorTDP_Boot suv_func( datas, vols, strikes, sgns, hestonParam ,0);
     HestonTDP_BootConstraint constraint;
     QuantLib::LevenbergMarquardt optimizationMethod;
     QuantLib::EndCriteria endCriteria(400, 40, 1e-8, 1e-8, 1e-8);
     for(i=0;i<termSize;i++)
     {
        suv_func.caliIndex = i;
        QuantLib::Problem problem(suv_func, constraint,initialValues[i]);
        optimizationMethod.minimize(problem, endCriteria);
        QuantLib::Array result(problem.currentValue());

        hestonParam.Kappa[i] = result[0];
        hestonParam.Theta[i] = result[1];
        hestonParam.Epsilon[i] = result[2];
        hestonParam.Rho[i] = result[3];
     }
 };

class HestonCalibratorTDTheta : public QuantLib::CostFunction
{
public:
    /// constructor
    HestonCalibratorTDTheta( const std::vector<FXOptionData >& datas_,
                             const DoubleMatrix& vols_,
                             const DoubleMatrix& strikes_,
                             const IntMatrix& sgns_,
                             HestonParams_TDP& hestonParam_ )
            : datas(datas_), vols(vols_), strikes(strikes_), sgns(sgns_), hestonParam(hestonParam_)
    {
        termSize = vols_.size();
        if( strikes_.size() != termSize || sgns_.size() != termSize || hestonParam.T.size() != termSize )
        {
             AQLString msg("size is not supported!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }

        strikeSize = vols_[0].size();
        if( strikes_[0].size() != strikeSize || sgns_[0].size() != strikeSize )
        {
             AQLString msg("size is not supported!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }
    };
    virtual ~HestonCalibratorTDTheta(){};

    QuantLib::Real value(const QuantLib::Array& x) const 
    {
        QuantLib::Real y=0;
        size_t i,j;

        hestonParam.V0 = x[termSize];
        for(i=0;i<termSize;i++)
        {
            hestonParam.Kappa[i] = x[termSize+1];
            hestonParam.Epsilon[i] = x[termSize+2];
            hestonParam.Rho[i] = x[termSize+3];
            hestonParam.Theta[i] = x[i];
        }

        DoubleMatrix modelVol = AQLMathDisplacedHestonTDP::CalibrationHelper( datas, strikes, sgns, hestonParam);
        for(i=0;i<termSize;i++)
        {
            for(j=0;j<strikeSize;j++)
            {
                y += AQLMath::pow(modelVol[i][j] - vols[i][j],2);
            }
        }
        return y;
    };

    QuantLib::Array values(const QuantLib::Array& x) const
    {
        QuantLib::Array y(strikeSize*termSize);
        size_t i,j;

        hestonParam.V0 = x[termSize];
        for(i=0;i<termSize;i++)
        {
            hestonParam.Kappa[i] = x[termSize+1];
            hestonParam.Epsilon[i] = x[termSize+2];
            hestonParam.Rho[i] = x[termSize+3];
            hestonParam.Theta[i] = x[i];
        }

        DoubleMatrix modelVol = AQLMathDisplacedHestonTDP::CalibrationHelper( datas, strikes, sgns, hestonParam);
        for(i=0;i<termSize;i++)
        {
            for(j=0;j<strikeSize;j++)
            {
                y[i*strikeSize+j] = AQLMath::pow(modelVol[i][j] - vols[i][j],2);
            }
        }
        return y;
    };

private:
    size_t termSize;
    size_t strikeSize;
    std::vector<FXOptionData > datas;
    DoubleMatrix vols;
    DoubleMatrix strikes;
    IntMatrix sgns;
    HestonParams_TDP& hestonParam;
};

class HestonTDTetaConstraint : public QuantLib::Constraint 
{
      private:
        class Impl : public QuantLib::Constraint::Impl 
        {
          public:
            bool test(const QuantLib::Array& params) const 
            {
                size_t termSize = params.size() - 4;
                //theta check
                for(size_t i=0;i<termSize;i++)
                {
                    if(params[i]<=0. || params[i]>=HESTON_UPPER) return false;
                }

                if(params[termSize]<=0. || params[termSize]>=HESTON_UPPER || params[1+termSize]<=0. || 
                   params[1+termSize]>=HESTON_UPPER || params[2+termSize]<=0. || params[2+termSize]>=HESTON_UPPER || 
                   params[3+termSize]<-1. || params[3+termSize]>1. ) return false;

                return true;       
            }
        };
      public:
        HestonTDTetaConstraint()
        : QuantLib::Constraint(boost::shared_ptr<QuantLib::Constraint::Impl>(
                                           new HestonTDTetaConstraint::Impl)) {}
};

void AQLMathDisplacedHestonTDP::FXCalibrationHestonTDTheta( const std::vector<FXOptionData >& datas,
                                                         const DoubleMatrix& vols,
                                                         const DoubleMatrix& strikes,
                                                         const IntMatrix& sgns,
                                                         HestonParams_TDP& hestonParam
                                                         )
 {
     //error check
     size_t termSize = datas.size();
     if( vols.size() != termSize || strikes.size() != termSize || sgns.size() != termSize )
     {
         AQLString msg("data sizes are not same!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
     }
      
     size_t strikeSize = vols[0].size();
     if( strikes[0].size() != strikeSize || sgns[0].size() != strikeSize )
     {
         AQLString msg("data sizes are not same!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
     }

     size_t i;
     QuantLib::Array initialValues(termSize+4);
     initialValues[termSize] = hestonParam.V0;
     initialValues[termSize+1] = hestonParam.Kappa[0];
     initialValues[termSize+2] = hestonParam.Epsilon[0];
     initialValues[termSize+3] = hestonParam.Rho[0];
     for(i=0;i<termSize;i++)
     {
        initialValues[i] = hestonParam.Theta[i];
        hestonParam.Kappa[i] = hestonParam.Kappa[0];
        hestonParam.Epsilon[i] = hestonParam.Epsilon[0];
        hestonParam.Rho[i] = hestonParam.Rho[0];
     }

     HestonCalibratorTDTheta suv_func( datas, vols, strikes, sgns, hestonParam );
     HestonTDTetaConstraint constraint;
     QuantLib::LevenbergMarquardt optimizationMethod;
     QuantLib::EndCriteria endCriteria(400, 40, 1e-8, 1e-8, 1e-8);
     QuantLib::Problem problem(suv_func, constraint,initialValues);
     optimizationMethod.minimize(problem, endCriteria);
     QuantLib::Array result(problem.currentValue());

     hestonParam.V0 = result[termSize];
     for(i=0;i<termSize;i++)
     {
        hestonParam.Theta[i] = result[i];
        hestonParam.Kappa[i] = result[termSize+1];
        hestonParam.Epsilon[i] = result[termSize+2];
        hestonParam.Rho[i] = result[termSize+3];
     }
 };

//other param is fixed
class HestonCalibratorTDThetaFix : public QuantLib::CostFunction
{
public:
    /// constructor
    HestonCalibratorTDThetaFix( const std::vector<FXOptionData >& datas_,
                                const DoubleMatrix& vols_,
                                const DoubleMatrix& strikes_,
                                const IntMatrix& sgns_,
                                HestonParams_TDP& hestonParam_ )
            : datas(datas_), vols(vols_), strikes(strikes_), sgns(sgns_), hestonParam(hestonParam_)
    {
        termSize = vols_.size();
        if( strikes_.size() != termSize || sgns_.size() != termSize || hestonParam.T.size() != termSize )
        {
             AQLString msg("size is not supported!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }

        strikeSize = vols_[0].size();
        if( strikes_[0].size() != strikeSize || sgns_[0].size() != strikeSize )
        {
             AQLString msg("size is not supported!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }
    };
    virtual ~HestonCalibratorTDThetaFix(){};

    QuantLib::Real value(const QuantLib::Array& x) const 
    {
        QuantLib::Real y=0;
        size_t i,j;

        for(i=0;i<termSize;i++)
        {
            hestonParam.Theta[i] = x[i];
        }

        DoubleMatrix modelVol = AQLMathDisplacedHestonTDP::CalibrationHelper( datas, strikes, sgns, hestonParam);
        for(i=0;i<termSize;i++)
        {
            for(j=0;j<strikeSize;j++)
            {
                y += AQLMath::pow(modelVol[i][j] - vols[i][j],2);
            }
        }
        return y;
    };

    QuantLib::Array values(const QuantLib::Array& x) const
    {
        QuantLib::Array y(termSize*strikeSize);
        size_t i,j;

        for(i=0;i<termSize;i++)
        {
            hestonParam.Theta[i] = x[i];
        }

        DoubleMatrix modelVol = AQLMathDisplacedHestonTDP::CalibrationHelper( datas, strikes, sgns, hestonParam);
        for(i=0;i<termSize;i++)
        {
            for(j=0;j<strikeSize;j++)
            {
                y[i*strikeSize+j] = AQLMath::pow(modelVol[i][j] - vols[i][j],2);
            }
        }
        return y;
    };


private:
    size_t termSize;
    size_t strikeSize;
    std::vector<FXOptionData > datas;
    DoubleMatrix vols;
    DoubleMatrix strikes;
    IntMatrix sgns;
    HestonParams_TDP& hestonParam;
};

class HestonTDThetaFixConstraint : public QuantLib::Constraint 
{
      private:
        class Impl : public QuantLib::Constraint::Impl 
        {
          public:
            bool test(const QuantLib::Array& params) const 
            {
                size_t termSize = params.size();
                //theta check
                for(size_t i=0;i<termSize;i++)
                {
                    if(params[i]<=0. || params[i]>=HESTON_UPPER) return false;
                }
                return true;     
            }
        };
      public:
        HestonTDThetaFixConstraint()
        : QuantLib::Constraint(boost::shared_ptr<QuantLib::Constraint::Impl>(
                                           new HestonTDThetaFixConstraint::Impl)) {}
};

void AQLMathDisplacedHestonTDP::FXCalibrationHestonTDThetaFix( const std::vector<FXOptionData >& datas,
                                                            const DoubleMatrix& vols,
                                                            const DoubleMatrix& strikes,
                                                            const IntMatrix& sgns,
                                                            HestonParams_TDP& hestonParam )
 {
     //error check
     size_t termSize = datas.size();
     if( vols.size() != termSize || strikes.size() != termSize || sgns.size() != termSize )
     {
         AQLString msg("data sizes are not same!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
     }
      
     size_t strikeSize = vols[0].size();
     if( strikes[0].size() != strikeSize || sgns[0].size() != strikeSize )
     {
         AQLString msg("data sizes are not same!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
     }

     size_t i;
     QuantLib::Array initialValues(termSize);
     for(i=0;i<termSize;i++)
     {
        initialValues[i] = hestonParam.Theta[i];
     }

     HestonCalibratorTDThetaFix suv_func( datas, vols, strikes, sgns, hestonParam );
     HestonTDThetaFixConstraint constraint;
     QuantLib::LevenbergMarquardt optimizationMethod;
     QuantLib::EndCriteria endCriteria(400, 40, 1e-8, 1e-8, 1e-8);
     QuantLib::Problem problem(suv_func, constraint,initialValues);
     optimizationMethod.minimize(problem, endCriteria);
     QuantLib::Array result(problem.currentValue());

     for(i=0;i<termSize;i++)
     {
        hestonParam.Theta[i] = result[i];
     }
 };

#endif
//find logStrike of -0.01 deltaPut 
class FindLogStrikeFromHeston : public AQLFunction
{
public:
    FindLogStrikeFromHeston( const FXOptionData& x_,
                             const HestonParams_TDP& y_)
        : x(x_), y(y_){};
    virtual ~FindLogStrikeFromHeston(){};
    double operator()(double z) const //z[0] is log strike
    { 
        double modelPrem = AQLMathDisplacedHestonTDP::BS_DDHeston_TDP( x.T, x.F, AQLMath::exp(z)*x.F, x.Pd, -1, 1., y );
		double impVol = AQLMathAnalyticalFormula::BlackImplVol( modelPrem/x.Pd, x.F, AQLMath::exp(z)*x.F, -1 ) / AQLMath::sqrt(x.T);
        return AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( z, impVol, -1, x ) / (-0.01) - 1.0;
    };
private:
    FXOptionData x;
    HestonParams_TDP y;
};

double AQLMathDisplacedHestonTDP::GetWingFactorFromHeston( const FXOptionData& x,
                                                        const SmileParam& smParams,
                                                        double atmDelta,
                                                        const HestonParams_TDP hestonParams,
                                                        double lower,
                                                        double high )
{
    FindLogStrikeFromHeston sub_func( x, hestonParams );
    double solve = sub_func.SolveBR( lower, high, 10000, 1.0e-8 );
    double modelPrem = AQLMathDisplacedHestonTDP::BS_DDHeston_TDP( x.T, x.F, AQLMath::exp(solve)*x.F, x.Pd, -1, 1., hestonParams );
	double impVol = AQLMathAnalyticalFormula::BlackImplVol( modelPrem/x.Pd, x.F, AQLMath::exp(solve)*x.F, -1 ) / AQLMath::sqrt(x.T);

    atmDelta *= -100.;
    double alpha, beta, RR, BF, tmp;

    tmp = AQLMath::abs( atmDelta - 25 ) / AQLMath::abs( atmDelta - 10 );
    alpha = log( smParams.highRR / smParams.lowRR ) / log( tmp );
    RR = ( smParams.highRR ) / AQLMath::pow( AQLMath::abs( atmDelta - 25 ), alpha );
    beta = log( smParams.highBF / smParams.lowBF ) / log( tmp );
    BF = smParams.highBF / AQLMath::pow( AQLMath::abs( atmDelta - 25 ), beta );

    return  ( impVol- smParams.atmVol + 0.5 * RR * AQLMath::pow( AQLMath::abs( atmDelta - 1. ), alpha ) ) / 
            ( BF * AQLMath::pow( AQLMath::abs( atmDelta - 1. ), beta ) );
};
