#include <AQLMathSwaptionFormulaSZ.h>
#include "AQLFunction.h"

const size_t int_step = 10;

//
DDSZ_util::DDSZ_util(AQLMathPathYieldCurve& curve_d_,
    			     AQLMathPathYieldCurve& curve_f_,
                     vector<double> T_param_,
                     vector<double> beta_,
                     double rho_,
                     vector<double> theta_,
                     vector<double> kappa_,
                     vector<double> epsilon_,
                     double S0_,
                     double V0_
					 )

:
curve_d(&curve_d_),
curve_f(&curve_f_),
T_param(T_param_),
beta(beta_),
rho(rho_),
theta(theta_),
kappa(kappa_),
epsilon(epsilon_),
S0(S0_),
V0(V0_)
{
    Int_kappa.SetFunc(*this, &DDSZ_util::kappa_);
    //Int_E_theta.SetFunc(*this, &DDSZ_util::E_theta_);

	Set_theta_over_kappa(theta_, kappa_);
	Set_exp_kappa_t(kappa_, T_param_);

	calc_cathe_E_theta();
}

//
double DDSZ_util::operator()(double x)
{
	return static_cast<const DDSZ_util*>(this)->operator()(x);
}

//
void DDSZ_util::Get_param(vector<double>& T_param_,
						  vector<double>& beta_,
						  vector<double>& theta_,
						  vector<double>& kappa_,
						  vector<double>& epsilon_
						 ) const
{
	T_param_ = T_param; beta_ = beta;
	theta_ = theta; kappa_ = kappa; epsilon_ = epsilon;
}

//
void DDSZ_util::Set_param(const vector<double>& T_param_,
						  const vector<double>& beta_,
						  const vector<double>& theta_,
						  const vector<double>& kappa_,
						  const vector<double>& epsilon_
						 )
{
	Set_T_param(T_param_);
	Set_beta(beta_); Set_theta(theta_);
	Set_kappa(kappa_); Set_epsilon(epsilon_);
	Set_theta_over_kappa(theta_, kappa_);
	Set_exp_kappa_t(kappa_, T_param_);

	calc_cathe_E_theta();
}

//
void DDSZ_util::Set_param(const vector<double>& T_param_,
				          const vector<double>& beta_,
						  const vector<double>& theta_,
						  const vector<double>& kappa_,
						  const vector<double>& epsilon_,
						  const map<double, double>& cathe_Int_E_theta_ 
						 )
{
	Set_T_param(T_param_);
	Set_beta(beta_); Set_theta(theta_);
	Set_kappa(kappa_); Set_epsilon(epsilon_);
	Set_theta_over_kappa(theta_, kappa_);
	Set_exp_kappa_t(kappa_, T_param_);

	cathe_Int_E_theta = cathe_Int_E_theta_;
}

//
////double DDSZ_util::fwd1( double t )
double DDSZ_util::fwd1( double t ) const
{
	double P_d = curve_d->getP(t);
	double P_f = curve_f->getP(t);


	return S0 * P_f / P_d; 
}

//
////double DDSZ_util::E( double t )
double DDSZ_util::E( double t ) const
{
  //  if( cathe_Int_kappa[t] == 0 )
  //  {
		////cathe_Int_kappa[t] = Int_kappa.Integrate_GLeg(0., t, 8);
		//cathe_Int_kappa[t] = Int_kappa.Integrate_GLeg_piecewise(0., t, T_param, 4);
  //  }
  //  
  //  return exp( cathe_Int_kappa[t] );
  return exp( kappa[0] * t );
    //return 1.;
}

double DDSZ_util::E_fwd2(double t) const
{
	double tmp = 0.;
	if (t < T_param.front())
	{
		tmp = theta_over_kappa[0] * (exp(kappa[0] * t) - 1.);
	}
	else if (T_param.back() < t)
	{
		double t_ = T_param.back();
		tmp = cathe_Int_E_theta[t_] + Int_E_theta(t_, t);
	}
	else
	{
		size_t i = lower_bound(T_param.begin(), T_param.end(), t) - T_param.begin();
		if (0 < i) i--;

		double t_ = T_param[i];
		tmp = cathe_Int_E_theta[t_] + Int_E_theta(t_, t);
	}
	return V0 + tmp;
}

//
double DDSZ_util::fwd2( double t ) const
{
    //return 1. / E(t) * ( V0 + theta[0] / kappa[0] * (1. - E(t)) );
    //double a = theta[0];
    //double b = theta[1];
    //double c = theta[2];
    //double d = theta[3];

    //double tmp = a / c * ( 1. - exp(- c * t) );
    //tmp -= b / c * exp(- c * t) * t;
    //tmp -= b / c / c * ( exp(- c * t) - 1. );
    //tmp += d * t;
    //double tmp = a * t + 0.5 * b * t * t;
  //  if( cathe_Int_E_theta[t] == 0 )
  //  {
  //      //cathe_Int_E_theta[t] = Int_E_theta.Integrate_GLeg(0., t, 20);
		////cathe_Int_E_theta[t] = Int_E_theta.Integrate_GLeg(0., t, 30);

		//if ( t < T_param.front() )
		//{
		//	cathe_Int_E_theta[t] = Int_E_theta.Integrate_GLeg(0., t, 3.);
		//}
		//else if ( T_param.back() < t )
		//{
		//	double t_ = T_param.back();
		//	cathe_Int_E_theta[t] = cathe_Int_E_theta[t_] + Int_E_theta.Integrate_GLeg(t_, t, 3.);
		//}
		//else
		//{
		//	size_t i = lower_bound(T_param.begin(), T_param.end(), t) - T_param.begin();
		//	if ( 0 < i ) i--;
		//	
		//	double t_ = T_param[i];
		//	cathe_Int_E_theta[t] = cathe_Int_E_theta[t_] + Int_E_theta.Integrate_GLeg(t_, t, 3.);
		//}
  //  }

	double tmp = 0.;
	//if ( t < T_param.front() )
	//{
	//	tmp = Int_E_theta.Integrate_GLeg(0., t, 3.);
	//}
	//else if ( T_param.back() < t )
	//{
	//	double t_ = T_param.back();
	//	tmp = cathe_Int_E_theta[t_] + Int_E_theta.Integrate_GLeg(t_, t, 5.);
	//}
	//else
	//{
	//	size_t i = lower_bound(T_param.begin(), T_param.end(), t) - T_param.begin();
	//	if ( 0 < i ) i--;
	//	
	//	double t_ = T_param[i];
	//	tmp = cathe_Int_E_theta[t_] + Int_E_theta.Integrate_GLeg(t_, t, 5.);
	//}
	
	if ( t < T_param.front() )
	{
		tmp = theta_over_kappa[0] * ( exp(kappa[0] * t) - 1. );
	}
	else if ( T_param.back() < t )
	{
		double t_ = T_param.back();
		tmp = cathe_Int_E_theta[t_] + Int_E_theta(t_, t);
	}
	else
	{
		size_t i = lower_bound(T_param.begin(), T_param.end(), t) - T_param.begin();
		if ( 0 < i ) i--;
		
		double t_ = T_param[i];
		tmp = cathe_Int_E_theta[t_] + Int_E_theta(t_, t);
	}

	return E(-t) * ( V0 + tmp );
}

//
////double DDSZ_util::vol1( double t )
double DDSZ_util::vol1( double t ) const
{
/* v2 * ( beta + (1 - beta) F(0,t) / v1 ) */
	return fwd2(t);

/* v2 * v2 * ( beta + (1 - beta) F(0,t) / v1 ) */
	//double tmp = fwd2(t);
	//return tmp;

/* exp(v2) * ( beta + (1 - beta) F(0,t) / v1 ) */
	//return exp(fwd2(t));

}

//
////double DDSZ_util::D1_vol1( double t )
double DDSZ_util::D1_vol1( double t ) const
{
    double fwd1_ = fwd1(t);

    //double a = beta[0];
    //double b = beta[1];
    //double c = beta[2];
    //double d = beta[3];

    //double beta_ = ( a + b * t ) * exp( -c * t ) + d;
    //double beta_ = a + b * t;

/* v2 * ( beta + (1 - beta) F(0,t) / v1 ) */
	return fwd2(t) * (beta[idx(t)] - 1.) / fwd1_; 

/* v2 * v2 * ( beta + (1 - beta) F(0,t) / v1 ) */
	//double tmp = fwd2(t);
	//return tmp * tmp * (beta[idx(t)] - 1.) / fwd1_; 
/* exp(v2) * ( beta + (1 - beta) F(0,t) / v1 ) */
	//return exp(fwd2(t)) * (beta[idx(t)] - 1.) / fwd1_; 
}

//
////double DDSZ_util::D2_vol1( double t )
double DDSZ_util::D2_vol1( double t ) const
{
/* v2 * ( beta + (1 - beta) F(0,t) / v1 ) */
	return 1.; 

/* v2 * v2 * ( beta + (1 - beta) F(0,t) / v1 ) */
	//double tmp = fwd2(t);
	//return 2. * tmp; 
/* exp(v2) * ( beta + (1 - beta) F(0,t) / v1 ) */
	//return exp(fwd2(t)); 

}

//
////double DDSZ_util::D11_vol1( double t )
double DDSZ_util::D11_vol1( double t ) const
{
    //double a = beta[0];
    //double b = beta[1];
    //double c = beta[2];
    //double d = beta[3];

    //double beta_ = ( a + b * t ) * exp( -c * t ) + d;
    //double beta_ = a + b * t;

    double fwd1_ = fwd1(t);
    double tmp = fwd1_ * fwd1_;
/* v2 * ( beta + (1 - beta) F(0,t) / v1 ) */
	return 2. * fwd2(t) * (1. - beta[idx(t)]) / tmp;

/* v2 * v2 * ( beta + (1 - beta) F(0,t) / v1 ) */
	//double fwd2_ = fwd2(t);
	//double tmp2 = fwd2_ * fwd2_;

	//return 2. * tmp2 * (1. - beta[idx(t)]) / tmp;
/* exp(v2) * ( beta + (1 - beta) F(0,t) / v1 ) */
	//return 2. * exp(fwd2(t)) * (1. - beta[idx(t)]) / tmp;

}

//
////double DDSZ_util::D12_vol1( double t )
double DDSZ_util::D12_vol1( double t ) const
{	
    //double a = beta[0];
    //double b = beta[1];
    //double c = beta[2];
    //double d = beta[3];

    //double beta_ = ( a + b * t ) * exp( -c * t ) + d;
    //double beta_ = a + b * t;

    double fwd1_ = fwd1(t);
/* v2 * ( beta + (1 - beta) F(0,t) / v1 ) */
	return (beta[idx(t)] - 1.) / fwd1_;

/* v2 * v2 * ( beta + (1 - beta) F(0,t) / v1 ) */
	//return 2. * fwd2(t) * (beta[idx(t)] - 1.) / fwd1_; 

/* exp(v2) * ( beta + (1 - beta) F(0,t) / v1 ) */
	//return exp(fwd2(t)) * (beta[idx(t)] - 1.) / fwd1_;
}

//
////double DDSZ_util::D22_vol1( double t )
double DDSZ_util::D22_vol1( double t ) const
{
/* v2 * ( beta + (1 - beta) F(0,t) / v1 ) */
	return 0.;

/* v2 * v2 * ( beta + (1 - beta) F(0,t) / v1 ) */
	//return 2.;

/* exp(v2) * ( beta + (1 - beta) F(0,t) / v1 ) */
	//return vol1(t);
}

//
////double DDSZ_util::vol2( double t )
double DDSZ_util::vol2( double t ) const
{
    //if(t < 1.)
    //{
    //    return epsilon[idx(t)] * sqrt( fwd2(t) );
    //}
    //else
    //{
        //return epsilon[idx(t)] * ( 1. + exp( - kappa[idx(t)] * fwd2(t) ) );
/* epsilon */
	return epsilon[idx(t)];

/* epsilon * sqrt( v2 ) */
	//return epsilon[idx(t)] * sqrt( fwd2(t) );
	//}
}

//
////double DDSZ_util::D_vol2( double t )
double DDSZ_util::D_vol2( double t ) const
{
    //if(t < 1.)
    //{
    //    return 0.5 * epsilon[idx(t)] / sqrt( fwd2(t) );
    //}
    //else
    //{
      //return - epsilon[idx(t)] * kappa[idx(t)] * exp( -kappa[idx(t)] * fwd2(t) );
    //}
/* epsilon */
    return 0.;

/* epsilon * sqrt( v2 ) */
	//return 0.5 * epsilon[idx(t)] / sqrt( fwd2(t) );
}

//
void DDSZ_util::calc_cathe_E_theta()
{
	cathe_Int_E_theta[0] = 0.;
	cathe_Int_E_theta[T_param[0]] = theta_over_kappa[0] * (exp_kappa_t[0][0] - 1.);

	for(size_t i = 1; i < T_param.size(); ++i)
	{
		cathe_Int_E_theta[T_param[i]] = cathe_Int_E_theta[T_param[i-1]] + Int_E_theta(T_param[i-1], T_param[i]);
	}

}

//
////double DDSZ_util::kappa_(double t)
double DDSZ_util::kappa_(double t) const
{
    size_t idx_ = idx(t);
    return kappa[idx_];
}

//
////double DDSZ_util::E_theta_(double t)
double DDSZ_util::E_theta_(double t) const
{
    size_t idx_ = idx(t);
    return E(t) * theta[idx_];
}

//
////double DDSZ_util::Int_E_theta(double ts, double te)
double DDSZ_util::Int_E_theta(double ts, double te) const
{
    size_t idx1 = idx(ts);
    size_t idx2 = idx(te);

	double int_E_theta_ = 0.;

	if(te <= T_param[idx1])
	{
		int_E_theta_ = theta_over_kappa[idx1] * ( exp(kappa[idx1] * te) - exp(kappa[idx2] * ts) );
	}
	else
	{
		if (AQLMath::abs(ts - T_param[idx1]) > 1.0e-7)
		{
			int_E_theta_ += theta_over_kappa[idx1] * (exp(kappa[idx1] * ts) - exp_kappa_t[idx1][idx1]);
		}
		for(size_t i = idx1; i < idx2; ++i)
		{
			int_E_theta_ += theta_over_kappa[i] * ( exp_kappa_t[i][i + 1] - exp_kappa_t[i][i] );
		}
		if (AQLMath::abs(te - T_param[idx2]) > 1.0e-7)
		{
			int_E_theta_ += theta_over_kappa[idx2] * (exp(kappa[idx2] * te) - exp_kappa_t[idx2][idx2]);
		}
	}

	return int_E_theta_;
}

//
////size_t DDSZ_util::idx(double t)
size_t DDSZ_util::idx(double t) const
{
    return --upper_bound(T_param.begin(), T_param.end(), t) - T_param.begin();
}

//
//--------------------------------------------------------------------------------
//
////double P1_SQ_DDSV::operator() (double t)
double P1_SQ_DDSV::operator() (double t) const
{
	//double tmp1 = vol1_SQ->Integrate_GLeg(0.,t,20);

	// no longer used
	//double tmp1 = vol1_SQ->IntegrateGLegendre(0.,t,20);
	
	//double tmp2 = vol2_E_SQ->Integrate_GLeg(0.,t,20);
	
	// no longer used
	//double tmp2 = vol2_E_SQ->IntegrateGLegendre(0.,t,20);
	
	//double tmp3 = rho_vol1_vol2_E->Integrate_GLeg(0.,t,20);
	double tmp3 = rho_vol1_vol2_E->IntegrateGLegendre(0.,t,20);

	// no longer used
    //double F = fwd1(t); 

    double E_inv = E(-t);

    double p1 = vol1(t);
    // D22_vol1 always returns zero
	//p1 += 0.5 * D22_vol1(t) * E_inv * E_inv * tmp2;

	// D2_vol1 always returns unity
    //p1 += (D2_vol1(t) + D12_vol1(t) * F ) * E_inv * tmp3;

	// D12_vol1 = (beta - 1) / F
	//p1 += (1.0 + D12_vol1(t) * F) * E_inv * tmp3;
	p1 += beta[idx(t)] * E_inv * tmp3;

	// D1_vol1 = fwd2 * (beta - 1) / F
	// D11_vol1 = 2 * fwd2 * (1- beta) / (F * F)
	//p1 += (D1_vol1(t) * F + 0.5 * D11_vol1(t) * F * F ) * tmp1;

    return p1 * p1;
}

////double P1_Vol1_DDSV::operator() (double t)
double P1_Vol1_DDSV::operator() (double t) const
{
	//double tmp1 = vol1_SQ->Integrate_GLeg(0.,t,20);
	
	// no longer used
	//double tmp1 = vol1_SQ->IntegrateGLegendre(0.,t,20);
	
	//double tmp2 = vol2_E_SQ->Integrate_GLeg(0.,t,20);

	// no longer used
	//double tmp2 = vol2_E_SQ->IntegrateGLegendre(0.,t,20);
	
	//double tmp3 = rho_vol1_vol2_E->Integrate_GLeg(0.,t,20);
	double tmp3 = rho_vol1_vol2_E->IntegrateGLegendre(0.,t,20);

	// no longer used
    //double F = fwd1(t); 
    
	double E_inv = E(-t);

	double F2 = fwd2(t);

	// vol1 = fwd2
    double p1 = F2;
	// D22_vol1 always returns zero
    //p1 += 0.5 * D22_vol1(t) * E_inv * E_inv * tmp2;

	// D2_vol1 always returns unity
    //p1 += (D2_vol1(t) + D12_vol1(t) * F ) * E_inv * tmp3;

	// D12_vol1 = (beta - 1) / F
	//p1 += (1.0 + D12_vol1(t) * F) * E_inv * tmp3;
	p1 += beta[idx(t)] * E_inv * tmp3;

	// D1_vol1 = fwd2 * (beta - 1) / F
	// D11_vol1 = 2 * fwd2 * (1- beta) / (F * F)	
	//p1 += (D1_vol1(t) * F + 0.5 * D11_vol1(t) * F * F ) * tmp1;

	// vol1 = fwd2
    //return p1 * vol1(t);
	return p1 * F2;
}

////double P1_P2_DDSV::operator() (double t)
double P1_P2_DDSV::operator() (double t) const
{
	//double tmp1 = vol1_SQ->Integrate_GLeg(0.,t,20);
	
	// no longer used
	//double tmp1 = vol1_SQ->IntegrateGLegendre(0.,t,20);
	
	//double tmp2 = vol2_E_SQ->Integrate_GLeg(0.,t,20);

	// no longer used
	//double tmp2 = vol2_E_SQ->IntegrateGLegendre(0.,t,20);
	
	//double tmp3 = rho_vol1_vol2_E->Integrate_GLeg(0.,t,20);
	double tmp3 = rho_vol1_vol2_E->IntegrateGLegendre(0.,t,2);

	// no longer used
    //double F = fwd1(t); 
    
	double E_inv = E(-t);

	double F2 = fwd2(t);

	// vol1 = fwd2
    //double p1 = vol1(t);
	double p1 = F2;

	// D22_vol1 always returns zero
    //p1 += 0.5 * D22_vol1(t) * E_inv * E_inv * tmp2;

	// D2_vol1 always returns unity
	//p1 += (D2_vol1(t) + D12_vol1(t) * F) * E_inv * tmp3;

	// D12_vol1 = (beta - 1) / F
	//p1 += (1.0 + D12_vol1(t) * F ) * E_inv * tmp3;
    p1 += beta[idx(t)] * E_inv * tmp3;

	// D1_vol1 = fwd2 * (beta - 1) / F
	// D11_vol1 = 2 * fwd2 * (1- beta) / (F * F)
    //p1 += (D1_vol1(t) * F + 0.5 * D11_vol1(t) * F * F ) * tmp1;
    
	// vol1 = fwd2, D1_vol1 = fwd2 * (beta - 1) / F
	double p2 = F2 * beta[idx(t)];
    return p1 * p2;
}


////double P1_P3_DDSV::operator() (double t)
double P1_P3_DDSV::operator() (double t) const
{
	//double tmp1 = vol1_SQ->Integrate_GLeg(0.,t,20);

	// no longer used
	//double tmp1 = vol1_SQ->IntegrateGLegendre(0.,t,20);
	
	//double tmp2 = vol2_E_SQ->Integrate_GLeg(0.,t,20);
	
	// no longer used
	//double tmp2 = vol2_E_SQ->IntegrateGLegendre(0.,t,20);
	
	//double tmp3 = rho_vol1_vol2_E->Integrate_GLeg(0.,t,20);
	double tmp3 = rho_vol1_vol2_E->IntegrateGLegendre(0.,t,2);

	// no longer used
    //double F = fwd1(t); 
    
	double E_inv = E(-t);
	
	double F2 = fwd2(t);

	// vol1 = fwd2
    //double p1 = vol1(t);
	double p1 = F2;

	// D22_vol1 always returns zero
    //p1 += 0.5 * D22_vol1(t) * E_inv * E_inv * tmp2;

	// D2_vol1 always returns unity
    //p1 += (D2_vol1(t) + D12_vol1(t) * F ) * E_inv * tmp3;

	// D12_vol1 = (beta - 1) / F
	// p1 += (1.0 + D12_vol1(t) * F) * E_inv * tmp3;
	p1 += beta[idx(t)] * E_inv * tmp3;

	// D1_vol1 = fwd2 * (beta - 1) / F
	// D11_vol1 = 2 * fwd2 * (1 - beta) / (F * F)
	// p1 += (D1_vol1(t) * F + 0.5 * D11_vol1(t) * F * F ) * tmp1;
    
	// D1_vol1 = fwd2 * (beta - 1) / F
	// D11_vol1 = 2 * fwd2 * (1 - beta) / (F * F)
	// vol1 = fwd2
	//double p3 = vol1(t) + 3. * D1_vol1(t) * F + D11_vol1(t) * F * F;
    double p3 = F2 * beta[idx(t)];
    return p1 * p3;
}

////double Rho_P1_P4_DDSV::operator() (double t)
double Rho_P1_P4_DDSV::operator() (double t) const
{
	//double tmp1 = vol1_SQ->Integrate_GLeg(0.,t,20);
	
	// no longer used
	//double tmp1 = vol1_SQ->IntegrateGLegendre(0.,t,20);
	
	//double tmp2 = vol2_E_SQ->Integrate_GLeg(0.,t,20);
	
	// no longer used
	//double tmp2 = vol2_E_SQ->IntegrateGLegendre(0.,t,20);
	
	//double tmp3 = rho_vol1_vol2_E->Integrate_GLeg(0.,t,20);
	double tmp3 = rho_vol1_vol2_E->IntegrateGLegendre(0.,t,2);

	// no longer used
    //double F = fwd1(t); 
    
	double E_inv = E(-t);

	// vol1 = fwd2
    //double p1 = vol1(t);
	double p1 = fwd2(t);

	// D22_vol1 always returns zero
    //p1 += 0.5 * D22_vol1(t) * E_inv * E_inv * tmp2;

	// D2_vol1 always returns unity
	//p1 += (D2_vol1(t) + D12_vol1(t) * F) * E_inv * tmp3;

	// D12_vol1 = (beta - 1) / F
	//p1 += (1.0 + D12_vol1(t) * F) * E_inv * tmp3;
	p1 += beta[idx(t)] * E_inv * tmp3;

	// D1_vol1 = fwd2 * (beta - 1) / F
	// D11_vol1 = 2 * fwd2 * (1 - beta) / (F * F)
	//p1 += (D1_vol1(t) * F + 0.5 * D11_vol1(t) * F * F) * tmp1;
    
	// vol2 = epsilon
    //double p4 = 1.0 / E_inv * vol2(t);
	double p4 = epsilon[idx(t)] / E_inv;

    return rho * p1 * p4;
}

////double P1_P5_DDSV::operator() (double t)
double P1_P5_DDSV::operator() (double t) const
{
	//double tmp1 = vol1_SQ->Integrate_GLeg(0.,t,20);

	// no longer used
	//double tmp1 = vol1_SQ->IntegrateGLegendre(0.,t,20);
	//double tmp2 = vol2_E_SQ->Integrate_GLeg(0.,t,20);
	
	// no longer used
	//double tmp2 = vol2_E_SQ->IntegrateGLegendre(0.,t,20);
	
	//double tmp3 = rho_vol1_vol2_E->Integrate_GLeg(0.,t,20);
	double tmp3 = rho_vol1_vol2_E->IntegrateGLegendre(0.,t,2);

	// no longer used
    //double F = fwd1(t); 
    
	double E_inv = E(-t);

	double F2 = fwd2(t);

	// vol1 = fwd2
    //double p1 = vol1(t);
	double p1 = F2;

	// D22_vol1 always returns zero
    //p1 += 0.5 * D22_vol1(t) * E_inv * E_inv * tmp2;

	// D2_vol1 always returns unity
	//p1 += (D2_vol1(t) + D12_vol1(t) * F ) * E_inv * tmp3;

	// D12_vol1 = (beta - 1) / F
	//p1 += (1.0 + D12_vol1(t) * F ) * E_inv * tmp3;
	p1 += beta[idx(t)] * E_inv * tmp3;

	// D1_vol1 = fwd2 * (beta - 1) / F
	// D11_vol1 = 2 * fwd2 * (1 - beta) / (F * F)
    //p1 += (D1_vol1(t) * F + 0.5 * D11_vol1(t) * F * F ) * tmp1;
    
	// D1_vol1 = fwd2 * (beta - 1) / F
    //double p5 =D1_vol1(t) * F;
	double p5 = F2 * (beta[idx(t)] - 1.);

    return p1 * p5;
}

////double P1_P6_DDSV::operator() (double t)
double P1_P6_DDSV::operator() (double t) const
{
	//double tmp1 = vol1_SQ->Integrate_GLeg(0.,t,20);
	
	// no longer used
	//double tmp1 = vol1_SQ->IntegrateGLegendre(0.,t,20);
	
	//double tmp2 = vol2_E_SQ->Integrate_GLeg(0.,t,20);
	
	// no longer used
	//double tmp2 = vol2_E_SQ->IntegrateGLegendre(0.,t,20);
	
	//double tmp3 = rho_vol1_vol2_E->Integrate_GLeg(0.,t,20);
	double tmp3 = rho_vol1_vol2_E->IntegrateGLegendre(0.,t,2);

	// no longer used
    //double F = fwd1(t); 
    
	double E_inv = E(-t);

    double p1 = vol1(t);
	// D22_vol1 always returns zero
    //p1 += 0.5 * D22_vol1(t) * E_inv * E_inv * tmp2;

	// D2_vol1 always returns unity
	//p1 += (D2_vol1(t) + D12_vol1(t) * F) * E_inv * tmp3;

	// D12_vol1 = (beta - 1) / F
    //p1 += (1.0 + D12_vol1(t) * F ) * E_inv * tmp3;
	p1 += beta[idx(t)] * E_inv * tmp3;

	// D1_vol1 = fwd2 * (beta - 1) / F
	// D11_vol1 = 2 * fwd2 * (1 - beta) / (F * F)
	// p1 += (D1_vol1(t) * F + 0.5 * D11_vol1(t) * F * F ) * tmp1;
        
	// D2_vol1 always returns unity
    //double p6 = D2_vol1(t) * E_inv;
    return p1 * E_inv;
}

////double Rho_P1_P7_DDSV::operator() (double t)
double Rho_P1_P7_DDSV::operator() (double t) const
{
	//double tmp1 = vol1_SQ->Integrate_GLeg(0.,t,20);
	//double tmp1 = vol1_SQ->IntegrateGLegendre(0.,t,20);
	//double tmp2 = vol2_E_SQ->Integrate_GLeg(0.,t,20);
	//double tmp2 = vol2_E_SQ->IntegrateGLegendre(0.,t,20);
	//double tmp3 = rho_vol1_vol2_E->Integrate_GLeg(0.,t,20);
	//double tmp3 = rho_vol1_vol2_E->IntegrateGLegendre(0.,t,20);

    //double F = fwd1(t); 
    //double E_inv = E(-t);

    //double p1 = vol1(t);
	// D22_vol1 always returns zero
    //p1 += 0.5 * D22_vol1(t) * E_inv * E_inv * tmp2;

	// D2_vol1 always returns unity
    //p1 += (D2_vol1(t) + D12_vol1(t) * F ) * E_inv * tmp3;

	// D12_vol1 = (beta - 1) / F
	//p1 += (1.0 + D12_vol1(t) * F) * E_inv * tmp3;
	//p1 += beta[idx(t)] * E_inv * tmp3;
	
	// D1_vol1 = fwd2 * (beta - 1) / F 
	// D11_vol1 = 2 * fwd2 * (1 - beta) / (F * F)
	//p1 += (D1_vol1(t) * F + 0.5 * D11_vol1(t) * F * F) * tmp1;
    
	// D_vol2 = 0
    //double p7 = D_vol2(t);
    //return rho * p1 * p7;

	return 0.;
}

////double P1_P8_DDSV::operator() (double t)
double P1_P8_DDSV::operator() (double t) const
{
	//double tmp1 = vol1_SQ->Integrate_GLeg(0.,t,20);

	// no longer used
	//double tmp1 = vol1_SQ->IntegrateGLegendre(0.,t,20);
	
	//double tmp2 = vol2_E_SQ->Integrate_GLeg(0.,t,20);
	
	// no longer used
	//double tmp2 = vol2_E_SQ->IntegrateGLegendre(0.,t,20);
	
	//double tmp3 = rho_vol1_vol2_E->Integrate_GLeg(0.,t,20);
	double tmp3 = rho_vol1_vol2_E->IntegrateGLegendre(0.,t,2);

	// no longer used
    //double F = fwd1(t); 
    
	double E_inv = E(-t);

    double p1 = vol1(t);
	// D22_vol1 always returns zero
    //p1 += 0.5 * D22_vol1(t) * E_inv * E_inv * tmp2;

	// D2_vol1 always returns unity
	//p1 += (D2_vol1(t) + D12_vol1(t) * F ) * E_inv * tmp3;

	// D12_vol1 = (beta - 1) / F
    //p1 += (1.0 + D12_vol1(t) * F ) * E_inv * tmp3;
	p1 += beta[idx(t)] * E_inv * tmp3;

	// D1_vol1 = fwd2 * (beta - 1) / F
	// D11_vol1 = 2 * fwd2 * (1 - beta) / (F * F)
	//p1 += (D1_vol1(t) * F + 0.5 * D11_vol1(t) * F * F) * tmp1;
	
	// D2_vol1 always returns unity
    //double p8 = (D2_vol1(t) + D12_vol1(t) * F) * E_inv;

	// D12_vol1 = (beta - 1) / F
	//double p8 = (1.0 + D12_vol1(t) * F) * E_inv;
	double p8 = beta[idx(t)] * E_inv;
    return p1 * p8;
}

////double P1_P9_DDSV::operator() (double t)
double P1_P9_DDSV::operator() (double t) const
{
	//double tmp1 = vol1_SQ->Integrate_GLeg(0.,t,20);
	//double tmp1 = vol1_SQ->IntegrateGLegendre(0.,t,20);
	//double tmp2 = vol2_E_SQ->Integrate_GLeg(0.,t,20);
	//double tmp2 = vol2_E_SQ->IntegrateGLegendre(0.,t,20);
	//double tmp3 = rho_vol1_vol2_E->Integrate_GLeg(0.,t,20);
	//double tmp3 = rho_vol1_vol2_E->IntegrateGLegendre(0.,t,20);

    //double F = fwd1(t); 
    //double E_inv = E(-t);

    //double p1 = vol1(t);
	// D22_vol1 = 0
	//p1 += 0.5 * D22_vol1(t) * E_inv * E_inv * tmp2;

	// D2_vol1 always returns unity
    //p1 += (D2_vol1(t) + D12_vol1(t) * F ) * E_inv * tmp3;

	// D12_vol1 = (beta - 1) / F
	//p1 += (1.0 + D12_vol1(t) * F) * E_inv * tmp3;
	//p1 += beta[idx(t)] * E_inv * tmp3;

	// D1_vol1 = fwd2 * (beta - 1) / F
	// D11_vol1 = 2 * fwd2 * (1 - beta) / (F * F)
	//p1 += (D1_vol1(t) * F + 0.5 * D11_vol1(t) * F * F ) * tmp1;

	// D22_vol1 = 0
    //double p9 = D22_vol1(t) * E_inv * E_inv;
	//return p1 * p9;
	return 0.;
}

////double P2_SQ_DDSV::operator() (double t)
double P2_SQ_DDSV::operator() (double t) const
{
	// no longer used
    //double F = fwd1(t); 
	
	// vol1 = fwd2
	// D1_vol1 = fwd2 * (beta - 1) / F
    double p2 = fwd2(t) * beta[idx(t)];
    return p2 * p2;
}

////double P4_SQ_DDSV::operator() (double t)
double P4_SQ_DDSV::operator() (double t) const
{
	// vol2 = epsilon
    //double p4 = E(t) * vol2(t);
	double p4 = E(t) * epsilon[idx(t)];
    return p4 * p4;
}

////double P6_SQ_DDSV::operator() (double t)
double P6_SQ_DDSV::operator() (double t) const
{
    double E_inv = E(-t);

	// D2_vol1 always returns unity
    //double p6 = D2_vol1(t) * E_inv;
    return E_inv * E_inv;
}

////double Rho_P4_P6_DDSV::operator() (double t)
double Rho_P4_P6_DDSV::operator() (double t) const
{
	// D2_vol1 always returns unity
    //double E_ = E(t);
    //double F = fwd1(t); 

    //double p4 = E_ * vol2(t);
    //double p6 = D2_vol1(t) / E_;
    //return rho * p4 * p6;
	
	// vol2 = epsilon
	//return rho * vol2(t);
	return rho * epsilon[idx(t)];
}

////double P2_Vol1_DDSV::operator() (double t)
double P2_Vol1_DDSV::operator() (double t) const
{        
	// no longer used
    //double F = fwd1(t); 

	double F2 = fwd2(t);
	// vol1 = fwd2
	// D1_vol1 = fwd2 * (beta - 1) / F
    //double p2 = vol1(t) + D1_vol1(t) * F;
	double p2 = F2 * beta[idx(t)];

	// vol1 = F2
	//return vol1(t) * p2;
	return F2 * p2;
}

////double Rho_Vol1_P4_DDSV::operator() (double t)
double Rho_Vol1_P4_DDSV::operator() (double t) const
{
    double p4 = E(t) * vol2(t);
    return rho * vol1(t) * p4;
}

////double P6_Vol1_DDSV::operator() (double t)
double P6_Vol1_DDSV::operator() (double t) const
{    
	// D2_vol1 always returns unity
    //double p6 = D2_vol1(t) * E(-t);
    return vol1(t) * E(-t);
}

////double Rho_P2_P4_DDSV::operator() (double t)
double Rho_P2_P4_DDSV::operator() (double t) const
{
	// vol1 = fwd2
	// D1_vol1 = fwd2 * (beta[idx(t)] - 1) / fwd1
    //double p2 = vol1(t) + D1_vol1(t) * fwd1(t);

	//double p2 = fwd2(t) * beta[idx(t)];
	//double p4 = E(t) * vol2(t);
	return rho * E_fwd2(t) * beta[idx(t)] * vol2(t);
}

////double P2_P6_DDSV::operator() (double t)
double P2_P6_DDSV::operator() (double t) const
{   
	// vol1 = fwd2
	// D1_vol1 = fwd2 * (beta - 1) / fwd1
    double p2 = fwd2(t) * beta[idx(t)];

	// D2_vol1 always returns unity
	//double p6 = D2_vol1(t) * E(-t);
    return p2 * E(-t);
}


//
//--------------------------------------------------------------------------------
//

AQLMathSwaptionFormulaSZ::AQLMathSwaptionFormulaSZ(AQLMathPathYieldCurve& curve_d_,
     									 AQLMathPathYieldCurve& curve_f_,
										 vector<double> T_param_,
										 vector<double> beta_,
										 double rho_,
										 vector<double> theta_,
										 vector<double> kappa_,
										 vector<double> epsilon_,
										 double S0_,
										 double V0_
										)
:
S0(S0_),
curve_d(&curve_d_),
curve_f(&curve_f_),
vol1_SQ( new Vol1_SQ_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_)),
vol2_E_SQ( new Vol2_E_SQ_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_)),
rho_vol1_vol2_E( new Rho_vol1_vol2_E_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,  theta_, kappa_, epsilon_, S0_, V0_)),
p1_SQ( new P1_SQ_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,  theta_, kappa_, epsilon_, S0_, V0_)),
p1_vol1( new P1_Vol1_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,  theta_, kappa_, epsilon_, S0_, V0_)),
p1_p2( new P1_P2_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,  theta_, kappa_, epsilon_, S0_, V0_)),
p1_p3( new P1_P3_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,  theta_, kappa_, epsilon_, S0_, V0_)),
rho_p1_p4( new Rho_P1_P4_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,  theta_, kappa_, epsilon_, S0_, V0_)),
p1_p5( new P1_P5_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,  theta_, kappa_, epsilon_, S0_, V0_)),
p1_p6( new P1_P6_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,  theta_, kappa_, epsilon_, S0_, V0_)),
rho_p1_p7( new Rho_P1_P7_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,  theta_, kappa_, epsilon_, S0_, V0_)),
p1_p8( new P1_P8_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,  theta_, kappa_, epsilon_, S0_, V0_)),
p1_p9( new P1_P9_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,  theta_, kappa_, epsilon_, S0_, V0_)),
p2_SQ( new P2_SQ_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,  theta_, kappa_, epsilon_, S0_, V0_)),
p4_SQ( new P4_SQ_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,  theta_, kappa_, epsilon_, S0_, V0_)),
p6_SQ( new P6_SQ_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,  theta_, kappa_, epsilon_, S0_, V0_)),
rho_p4_p6( new Rho_P4_P6_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,  theta_, kappa_, epsilon_, S0_, V0_)),
p2_vol1( new P2_Vol1_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,  theta_, kappa_, epsilon_, S0_, V0_)),
rho_vol1_p4( new Rho_Vol1_P4_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,  theta_, kappa_, epsilon_, S0_, V0_)),
p6_vol1( new P6_Vol1_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,  theta_, kappa_, epsilon_, S0_, V0_)),
rho_p2_p4( new Rho_P2_P4_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,  theta_, kappa_, epsilon_, S0_, V0_)),
p2_p6( new P2_P6_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,  theta_, kappa_, epsilon_, S0_, V0_)),
int_multi(new AQLMathIntegralMulti())
{
	cathe_q1[0.] = 0.;
	cathe_q2[0.] = 0.;
	cathe_q3[0.] = 0.;
	cathe_q4[0.] = 0.;
	cathe_q5[0.] = 0.;
	cathe_q6[0.] = 0.;
	cathe_q7[0.] = 0.;
	cathe_q8[0.] = 0.;
	cathe_q9[0.] = 0.;
	cathe_q10[0.] = 0.;
	cathe_q11[0.] = 0.;
	cathe_q12[0.] = 0.;
	cathe_Sigma[0.] = 0.;
}

//
double AQLMathSwaptionFormulaSZ::q1( double t )
{
	//
	vector<AQLFunction*> funcs;
	vector<int> steps;

	funcs.resize(2); funcs[0] = p1_p2; funcs[1] = p1_vol1;
	steps.resize(2); steps[0] = 2; steps[1] = 5;

	int_multi->Set_func_integral_2D_0(funcs, steps);
	return int_multi->integral_2D_0(t);
}

double AQLMathSwaptionFormulaSZ::q1( double ts, double te )
{
	//
	vector<AQLFunction*> funcs;
	vector<int> steps;

	funcs.resize(2); funcs[0] = p1_p2; funcs[1] = p1_vol1;
	steps.resize(2); steps[0] = int_step; steps[1] = 5;

	int_multi->Set_func_integral_2D_0(funcs, steps);
	return int_multi->integral_2D_0(ts, te);
}

//
double AQLMathSwaptionFormulaSZ::q2( double t )
{ 
	//
	vector<AQLFunction*> funcs;
	vector<int> steps;

	funcs.resize(2); funcs[0] = p1_p6; funcs[1] = rho_p1_p4;
	steps.resize(2); steps[0] = 2; steps[1] = 5;

	int_multi->Set_func_integral_2D_0(funcs, steps);
	return int_multi->integral_2D_0(t);
}

//
double AQLMathSwaptionFormulaSZ::q2( double ts, double te )
{
	//
	vector<AQLFunction*> funcs;
	vector<int> steps;

	funcs.resize(2); funcs[0] = p1_p6; funcs[1] = rho_p1_p4;
	steps.resize(2); steps[0] = int_step; steps[1] = 5;

	int_multi->Set_func_integral_2D_0(funcs, steps);
	return int_multi->integral_2D_0(ts, te);
}

//
double AQLMathSwaptionFormulaSZ::q3( double t )
{
	double q3_;
	//
	vector<AQLFunction*> funcs;
	vector<int> steps;

	funcs.resize(3); funcs[0] = p1_p3; funcs[1] = p1_vol1; funcs[2] = p1_vol1;
	steps.resize(3); steps[0] = 2; steps[1] = 5; steps[2] = 5;

	int_multi->Set_func_integral_3D_0(funcs, steps);
	q3_ = int_multi->integral_3D_0(t);

	funcs.resize(3); funcs[0] = p1_p2; funcs[1] = p1_p5; funcs[2] = p1_vol1;
	steps.resize(3); steps[0] = 2; steps[1] = 5; steps[2] = 5;

	int_multi->Set_func_integral_3D_0(funcs, steps);
	q3_ += int_multi->integral_3D_0(t);

    funcs.resize(3); funcs[0] = p1_vol1; funcs[1] = p1_p6; funcs[2] = rho_p1_p4;
	steps.resize(3); steps[0] = 2; steps[1] = 5; steps[2] = 5;

	int_multi->Set_func_integral_3D_0(funcs, steps);
	q3_ += int_multi->integral_3D_0(t);

    funcs.resize(3); funcs[0] = p1_p6; funcs[1] = rho_p1_p7; funcs[2] = rho_p1_p4;
	steps.resize(3); steps[0] = 2; steps[1] = 5; steps[2] = 5;

	int_multi->Set_func_integral_3D_0(funcs, steps);
	q3_ += int_multi->integral_3D_0(t);

    funcs.resize(3); funcs[0] = p1_p8; funcs[1] = p1_vol1; funcs[2] = rho_p1_p4;
	steps.resize(3); steps[0] = 2; steps[1] = 5; steps[2] = 5;

	int_multi->Set_func_integral_3D_0(funcs, steps);
	q3_ += int_multi->integral_3D_0(t);

    funcs.resize(3); funcs[0] = p1_p8; funcs[1] = rho_p1_p4; funcs[2] = p1_vol1;
	steps.resize(3); steps[0] = 2; steps[1] = 5; steps[2] = 5;

	int_multi->Set_func_integral_3D_0(funcs, steps);
	q3_ += int_multi->integral_3D_0(t);

    funcs.resize(3); funcs[0] = p1_p9; funcs[1] = rho_p1_p4; funcs[2] = rho_p1_p4;
	steps.resize(3); steps[0] = 2; steps[1] = 5; steps[2] = 5;

	int_multi->Set_func_integral_3D_0(funcs, steps);
	q3_ += int_multi->integral_3D_0(t);

    funcs.resize(3); funcs[0] = p1_p5; funcs[1] = p1_p6; funcs[2] = rho_p1_p4;
	steps.resize(3); steps[0] = 2; steps[1] = 5; steps[2] = 5;

	int_multi->Set_func_integral_3D_0(funcs, steps);
	q3_ += int_multi->integral_3D_0(t);

    return q3_;

}

//
double AQLMathSwaptionFormulaSZ::q3( double ts, double te )
{
	double q3_;
	//
	vector<AQLFunction*> funcs;
	vector<int> steps;

	funcs.resize(3); funcs[0] = p1_p3; funcs[1] = p1_vol1; funcs[2] = p1_vol1;
	steps.resize(3); steps[0] = int_step; steps[1] = 3; steps[2] = 2;

	int_multi->Set_func_integral_3D_0(funcs, steps);
	q3_ = int_multi->integral_3D_0(ts,te);

	funcs.resize(3); funcs[0] = p1_p2; funcs[1] = p1_p5; funcs[2] = p1_vol1;
	steps.resize(3); steps[0] = int_step; steps[1] = 3; steps[2] = 2;

	int_multi->Set_func_integral_3D_0(funcs, steps);
	q3_ += int_multi->integral_3D_0(ts,te);

    funcs.resize(3); funcs[0] = p1_vol1; funcs[1] = p1_p6; funcs[2] = rho_p1_p4;
	steps.resize(3); steps[0] = int_step; steps[1] = 3; steps[2] = 2;

	int_multi->Set_func_integral_3D_0(funcs, steps);
	q3_ += int_multi->integral_3D_0(ts,te);

	// integral of rho_p1_p7 is always zero because of DDSZ_util::D_vol2
    //funcs.resize(3); funcs[0] = p1_p6; funcs[1] = rho_p1_p7; funcs[2] = rho_p1_p4;
	//steps.resize(3); steps[0] = int_step; steps[1] = 3; steps[2] = 2;

	//int_multi->Set_func_integral_3D_0(funcs, steps);
	//q3_ += int_multi->integral_3D_0(ts,te);

    funcs.resize(3); funcs[0] = p1_p8; funcs[1] = p1_vol1; funcs[2] = rho_p1_p4;
	steps.resize(3); steps[0] = int_step; steps[1] = 3; steps[2] = 2;

	int_multi->Set_func_integral_3D_0(funcs, steps);
	q3_ += int_multi->integral_3D_0(ts,te);

    funcs.resize(3); funcs[0] = p1_p8; funcs[1] = rho_p1_p4; funcs[2] = p1_vol1;
	steps.resize(3); steps[0] = int_step; steps[1] = 3; steps[2] = 2;

	int_multi->Set_func_integral_3D_0(funcs, steps);
	q3_ += int_multi->integral_3D_0(ts,te);

	// integral of p1_p9 is always zero because of DDSZ_util::D22_vol1
    //funcs.resize(3); funcs[0] = p1_p9; funcs[1] = rho_p1_p4; funcs[2] = rho_p1_p4;
	//steps.resize(3); steps[0] = int_step; steps[1] = 3; steps[2] = 2;

	//int_multi->Set_func_integral_3D_0(funcs, steps);
	//q3_ += int_multi->integral_3D_0(ts,te);

    funcs.resize(3); funcs[0] = p1_p5; funcs[1] = p1_p6; funcs[2] = rho_p1_p4;
	steps.resize(3); steps[0] = int_step; steps[1] = 3; steps[2] = 2;

	int_multi->Set_func_integral_3D_0(funcs, steps);
	q3_ += int_multi->integral_3D_0(ts,te);

    return q3_;

}

double AQLMathSwaptionFormulaSZ::q4( double t )
{
	double tmp = q1(t);
	return tmp * tmp;
}

double AQLMathSwaptionFormulaSZ::q4( double ts, double te )
{
	double tmp = q1(ts, te);
	return tmp * tmp;
}

double AQLMathSwaptionFormulaSZ::q5( double t )
{
	double q5_;

	//
	vector<AQLFunction*> funcs;
	vector<int> steps;

	funcs.resize(3); funcs[0] = p1_p2; funcs[1] = p1_p2; funcs[2] = vol1_SQ;
	steps.resize(3); steps[0] = 2; steps[1] = 5; steps[2] = 5;

	int_multi->Set_func_integral_3D_0(funcs, steps);
	q5_ = 2. * int_multi->integral_3D_0(t);

	funcs.resize(3); funcs[0] = p1_p2; funcs[1] = p2_vol1; funcs[2] = p1_vol1;
	steps.resize(3); steps[0] = 2; steps[1] = 5; steps[2] = 5;

	int_multi->Set_func_integral_3D_0(funcs, steps);
	q5_ += 2. * int_multi->integral_3D_0(t);

	funcs.resize(3); funcs[0] = p2_SQ; funcs[1] = p1_vol1; funcs[2] = p1_vol1;
	steps.resize(3); steps[0] = 2; steps[1] = 5; steps[2] = 5;

	int_multi->Set_func_integral_2D_1(funcs, steps);
	q5_ += int_multi->integral_2D_1(t);

	return q5_;
}

double AQLMathSwaptionFormulaSZ::q5( double ts, double te )
{
	double q5_;

	//
	vector<AQLFunction*> funcs;
	vector<int> steps;

	funcs.resize(3); funcs[0] = p1_p2; funcs[1] = p1_p2; funcs[2] = vol1_SQ;
	steps.resize(3); steps[0] = int_step; steps[1] = 3; steps[2] = 2;

	int_multi->Set_func_integral_3D_0(funcs, steps);
	q5_ = 2. * int_multi->integral_3D_0(ts, te);

	funcs.resize(3); funcs[0] = p1_p2; funcs[1] = p2_vol1; funcs[2] = p1_vol1;
	steps.resize(3); steps[0] = int_step; steps[1] = 3; steps[2] = 2;

	int_multi->Set_func_integral_3D_0(funcs, steps);
	q5_ += 2. * int_multi->integral_3D_0(ts, te);

	funcs.resize(3); funcs[0] = p2_SQ; funcs[1] = p1_vol1; funcs[2] = p1_vol1;
	steps.resize(3); steps[0] = int_step; steps[1] = 3; steps[2] = 2;

	int_multi->Set_func_integral_2D_1(funcs, steps);
	q5_ += int_multi->integral_2D_1(ts, te);

	return q5_;
}

double AQLMathSwaptionFormulaSZ::q6( double t )
{
	double q6_;

	//
	vector<AQLFunction*> funcs;
	vector<int> steps;

	funcs.resize(2); funcs[0] = p2_SQ; funcs[1] = vol1_SQ;
	steps.resize(2); steps[0] = 2; steps[1] = 5;

	int_multi->Set_func_integral_2D_0(funcs, steps);
	q6_ = int_multi->integral_2D_0(t);

	return q6_;
}

double AQLMathSwaptionFormulaSZ::q6( double ts, double te )
{
	double q6_;

	//
	vector<AQLFunction*> funcs;
	vector<int> steps;

	funcs.resize(2); funcs[0] = p2_SQ; funcs[1] = vol1_SQ;
	steps.resize(2); steps[0] = int_step; steps[1] = 5;

	int_multi->Set_func_integral_2D_0(funcs, steps);
	q6_ = int_multi->integral_2D_0(ts,te);

	return q6_;
}

double AQLMathSwaptionFormulaSZ::q7( double t )
{
    double tmp = q2(t); 
    return tmp * tmp;
}

double AQLMathSwaptionFormulaSZ::q7( double ts, double te )
{
    double tmp = q2(ts,te); 
    return tmp * tmp;
}

double AQLMathSwaptionFormulaSZ::q8( double t )
{
	double q8_;

	//
	vector<AQLFunction*> funcs;
	vector<int> steps;

	funcs.resize(3); funcs[0] = p1_p6; funcs[1] = p1_p6; funcs[2] = p4_SQ;
	steps.resize(3); steps[0] = 2; steps[1] = 5; steps[2] = 5;

	int_multi->Set_func_integral_3D_0(funcs, steps);
	q8_ = 2. * int_multi->integral_3D_0(t);

	funcs.resize(3); funcs[0] = p1_p6; funcs[1] = rho_p4_p6; funcs[2] = rho_p1_p4;
	steps.resize(3); steps[0] = 2; steps[1] = 5; steps[2] = 5;

	int_multi->Set_func_integral_3D_0(funcs, steps);
	q8_ += 2. * int_multi->integral_3D_0(t);

	funcs.resize(3); funcs[0] = p6_SQ; funcs[1] = rho_p1_p4; funcs[2] = rho_p1_p4;
	steps.resize(3); steps[0] = 2; steps[1] = 5; steps[2] = 5;

	int_multi->Set_func_integral_2D_1(funcs, steps);
	q8_ += int_multi->integral_2D_1(t);

	return q8_;
}

double AQLMathSwaptionFormulaSZ::q8( double ts, double te )
{
	double q8_;

	//
	vector<AQLFunction*> funcs;
	vector<int> steps;

	funcs.resize(3); funcs[0] = p1_p6; funcs[1] = p1_p6; funcs[2] = p4_SQ;
	steps.resize(3); steps[0] = int_step; steps[1] = 3; steps[2] = 2;

	int_multi->Set_func_integral_3D_0(funcs, steps);
	q8_ = 2. * int_multi->integral_3D_0(ts,te);

	funcs.resize(3); funcs[0] = p1_p6; funcs[1] = rho_p4_p6; funcs[2] = rho_p1_p4;
	steps.resize(3); steps[0] = int_step; steps[1] = 3; steps[2] = 2;

	int_multi->Set_func_integral_3D_0(funcs, steps);
	q8_ += 2. * int_multi->integral_3D_0(ts,te);

	funcs.resize(3); funcs[0] = p6_SQ; funcs[1] = rho_p1_p4; funcs[2] = rho_p1_p4;
	steps.resize(3); steps[0] = int_step; steps[1] = 3; steps[2] = 2;

	int_multi->Set_func_integral_2D_1(funcs, steps);
	q8_ += int_multi->integral_2D_1(ts,te);

	return q8_;
}

double AQLMathSwaptionFormulaSZ::q9( double t )
{
	double q9_;

	//
	vector<AQLFunction*> funcs;
	vector<int> steps;

	funcs.resize(2); funcs[0] = p6_SQ; funcs[1] = p4_SQ;
	steps.resize(2); steps[0] = 2; steps[1] = 5;

	int_multi->Set_func_integral_2D_0(funcs, steps);
	q9_ = int_multi->integral_2D_0(t);

	return q9_;
}

double AQLMathSwaptionFormulaSZ::q9( double ts, double te )
{
	double q9_;

	//
	vector<AQLFunction*> funcs;
	vector<int> steps;

	funcs.resize(2); funcs[0] = p6_SQ; funcs[1] = p4_SQ;
	steps.resize(2); steps[0] = int_step; steps[1] = 5;

	int_multi->Set_func_integral_2D_0(funcs, steps);
	q9_ = int_multi->integral_2D_0(ts, te);

	return q9_;
}

//
double AQLMathSwaptionFormulaSZ::q10( double t )
{
	return q1(t) * q2(t);
}

//
double AQLMathSwaptionFormulaSZ::q10( double ts, double te )
{
	return q1(ts,te) * q2(ts,te);
}

//
double AQLMathSwaptionFormulaSZ::q11( double t )
{
	double q11_;

	//
	vector<AQLFunction*> funcs;
	vector<int> steps;

    //
	funcs.resize(3); funcs[0] = p1_p2; funcs[1] = p1_p6; funcs[2] = rho_vol1_p4;
	steps.resize(3); steps[0] = 2; steps[1] = 5; steps[2] = 5;

	int_multi->Set_func_integral_3D_0(funcs, steps);
	q11_ = int_multi->integral_3D_0(t);

    //
	funcs.resize(3); funcs[0] = p1_p6; funcs[1] = p1_p2; funcs[2] = rho_vol1_p4;
	steps.resize(3); steps[0] = 2; steps[1] = 5; steps[2] = 5;

	int_multi->Set_func_integral_3D_0(funcs, steps);
	q11_ += int_multi->integral_3D_0(t);

    //
	funcs.resize(3); funcs[0] = p1_p2; funcs[1] = p6_vol1; funcs[2] = rho_p1_p4;
	steps.resize(3); steps[0] = 2; steps[1] = 5; steps[2] = 5;

	int_multi->Set_func_integral_3D_0(funcs, steps);
	q11_ += int_multi->integral_3D_0(t);

    //
	funcs.resize(3); funcs[0] = p2_p6; funcs[1] = p1_vol1; funcs[2] = rho_p1_p4;
	steps.resize(3); steps[0] = 2; steps[1] = 5; steps[2] = 5;

    int_multi->Set_func_integral_2D_2(funcs, steps);
	q11_ += int_multi->integral_2D_2(t);

    //
	funcs.resize(3); funcs[0] = p1_p6; funcs[1] = rho_p2_p4; funcs[2] = p1_vol1;
	steps.resize(3); steps[0] = 2; steps[1] = 5; steps[2] = 5;

	int_multi->Set_func_integral_3D_0(funcs, steps);
	q11_ += int_multi->integral_3D_0(t);

	return q11_;
}

//
double AQLMathSwaptionFormulaSZ::q11( double ts, double te )
{
	double q11_;

	//
	vector<AQLFunction*> funcs;
	vector<int> steps;

    //
	funcs.resize(3); funcs[0] = p1_p2; funcs[1] = p1_p6; funcs[2] = rho_vol1_p4;
	steps.resize(3); steps[0] = int_step; steps[1] = 3; steps[2] = 2;

	int_multi->Set_func_integral_3D_0(funcs, steps);
	q11_ = int_multi->integral_3D_0(ts,te);

    //
	funcs.resize(3); funcs[0] = p1_p6; funcs[1] = p1_p2; funcs[2] = rho_vol1_p4;
	steps.resize(3); steps[0] = int_step; steps[1] = 3; steps[2] = 2;

	int_multi->Set_func_integral_3D_0(funcs, steps);
	q11_ += int_multi->integral_3D_0(ts,te);

    //
	funcs.resize(3); funcs[0] = p1_p2; funcs[1] = p6_vol1; funcs[2] = rho_p1_p4;
	steps.resize(3); steps[0] = int_step; steps[1] = 3; steps[2] = 2;

	int_multi->Set_func_integral_3D_0(funcs, steps);
	q11_ += int_multi->integral_3D_0(ts,te);

    //
	funcs.resize(3); funcs[0] = p2_p6; funcs[1] = p1_vol1; funcs[2] = rho_p1_p4;
	steps.resize(3); steps[0] = int_step; steps[1] = 3; steps[2] = 2;

    int_multi->Set_func_integral_2D_2(funcs, steps);
	q11_ += int_multi->integral_2D_2(ts,te);

    //
	funcs.resize(3); funcs[0] = p1_p6; funcs[1] = rho_p2_p4; funcs[2] = p1_vol1;
	steps.resize(3); steps[0] = int_step; steps[1] = 3; steps[2] = 2;

	int_multi->Set_func_integral_3D_0(funcs, steps);
	q11_ += int_multi->integral_3D_0(ts,te);

	return q11_;
}

//
double AQLMathSwaptionFormulaSZ::q12( double t )
{
	double q12_;

	//
	vector<AQLFunction*> funcs;
	vector<int> steps;

	funcs.resize(2); funcs[0] = p2_p6; funcs[1] = rho_vol1_p4;
	steps.resize(2); steps[0] = 2; steps[1] = 5;

	int_multi->Set_func_integral_2D_0(funcs, steps);
	q12_ = int_multi->integral_2D_0(t);

	return q12_;
}

//
double AQLMathSwaptionFormulaSZ::q12( double ts, double te )
{
	double q12_;

	//
	vector<AQLFunction*> funcs;
	vector<int> steps;

	funcs.resize(2); funcs[0] = p2_p6; funcs[1] = rho_vol1_p4;
	steps.resize(2); steps[0] = int_step; steps[1] = 5;

	int_multi->Set_func_integral_2D_0(funcs, steps);
	q12_ = int_multi->integral_2D_0(ts,te);

	return q12_;
}

void AQLMathSwaptionFormulaSZ::clear_cache()
{
	cathe_Sigma.clear();

	cathe_q1.clear();
	cathe_q2.clear();
	cathe_q3.clear();
	cathe_q4.clear();
	cathe_q5.clear();
	cathe_q6.clear();
	cathe_q7.clear();
	cathe_q8.clear();
	cathe_q9.clear();
	cathe_q10.clear();
	cathe_q11.clear();
	
	cathe_q1[0.] = 0.;
	cathe_q2[0.] = 0.;
	cathe_q3[0.] = 0.;
	cathe_q4[0.] = 0.;
	cathe_q5[0.] = 0.;
	cathe_q6[0.] = 0.;
	cathe_q7[0.] = 0.;
	cathe_q8[0.] = 0.;
	cathe_q9[0.] = 0.;
	cathe_q10[0.] = 0.;
	cathe_q11[0.] = 0.;
	cathe_q12[0.] = 0.;
	cathe_Sigma[0.] = 0.;
}
