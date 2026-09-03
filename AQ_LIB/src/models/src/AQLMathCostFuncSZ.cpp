
#include <AQLMathCostFuncSZ.h>
#include <AQLMathYieldCurve.h>
#include <AQLMathSwaptionFormulaSZ.h>
#include "AQLModelUtilities.h"
#include <AQLCoreAppError.h>

AQLMathCostFuncSZ::AQLMathCostFuncSZ(double S0_,
						 double V0_,
						 double rho_,
						 AQLMathPathYieldCurve& curve_d_,
						 AQLMathPathYieldCurve& curve_f_,
						 vector<double>& T_param_,
						 vector<double>& beta_,
						 vector<double>& theta_,
						 vector<double>& kappa_,
								vector<double>& epsilon_,
								const vector<bool>& calibFlag_
					    )
:
CostFunction(),
T(0),
n(0),
T_param(0),
beta(0),
theta(0),
kappa(0),
epsilon(0),
calibFlag(calibFlag_),
ddsv_approx(new AQLMathSwaptionFormulaSZ(curve_d_,
                 		            curve_f_,
									T_param_,
			                        beta_,      
						            rho_,                          
									theta_,
			                        kappa_,
						            epsilon_,
			                        S0_,
						            V0_
								   ))
{
	if(weight.size() != n)
		throw AQLCoreInvalidData("T_grid.size() != weight.size() : CostFunc_SZ::CostFunc_SZ", __FILE__, __LINE__);

	if(t2f.size() != n)
		throw AQLCoreInvalidData("t2f.size() != n : CostFunc_SZ::CostFunc_SZ", __FILE__, __LINE__);

	if(calibFlag.size() != 4)
	{
		throw AQLCoreInvalidData("flg.size() != 4 : CostFunc_SZ::CostFunc_SZ", __FILE__, __LINE__);
	}
}

AQLMathCostFuncSZ::~AQLMathCostFuncSZ()
{
	delete ddsv_approx;
	ddsv_approx = 0;
}

// method to overload to compute the cost function value in x
Real AQLMathCostFuncSZ::value(const Array& x) const
{   
	if(n == 0) throw AQLCoreInvalidData("target to fit is not setted! :: CostFunc_SZ::value", __FILE__, __LINE__);
	double m = x.size();
	
	if(m == 0 || m > 4)
	//if(m == 0 || m > 3)
		throw AQLCoreInvalidData("m == 0 || m > 4 : CostFunc_SZ::value", __FILE__, __LINE__);

	set_params(x);
	//beta[idx] = x[0]; theta[idx] = x[1]; kappa[idx] = x[2]; epsilon[idx] = x[3];
	//beta[idx] = x[0]; theta[idx] = x[1]; epsilon[idx] = x[2];
	ddsv_approx->Set_param(T_param, beta, theta, kappa, epsilon);

	double diff_SQ = 0.;
	for(size_t i = 0; i < n; ++i)
	{
		//double imp_vol = ddsv_approx->Get_imp_vol(T, strike[i]);
		//diff_SQ += weight[i] * ( imp_vol - t2f[i] ) * ( imp_vol - t2f[i] );
		double vol = ddsv_approx->Get_imp_vol(T, strike[i], true);
		diff_SQ += weight[i] * ( vol - t2f[i] ) * ( vol - t2f[i] );
	}

	return diff_SQ;
}

// method to overload to compute the cost function values in x
Array AQLMathCostFuncSZ::values(const Array& x) const
{
	double m = x.size();
	
	//if(m == 0 || m > 4)
	if(m == 0 || m > 3)
		throw AQLCoreInvalidData("m == 0 || m > 3 : CostFunc_SZ::value", __FILE__, __LINE__);

	set_params(x);
	//beta[idx] = x[0]; theta[idx] = x[1]; kappa[idx] = x[2]; epsilon[idx] = x[3];
	//beta[idx] = x[0]; theta[idx] = x[1]; epsilon[idx] = x[2];
	ddsv_approx->Set_param(T_param, beta, theta, kappa, epsilon);
	
    size_t k = 0;
	Array diff_SQs( n );
	for(size_t i = 0; i < n; ++i)
	{
		//diff_SQs[i-1] = sqrt( weight[i] ) * ( ddsv_approx->Get_Call_Opt(T, strike[i]) - t2f[i] );
		double vol = ddsv_approx->Get_imp_vol(T, strike[i], true);
		diff_SQs[i] = sqrt( weight[i] ) * ( vol - t2f[i] );
		//diff_SQs[i-1] = sqrt( weight[i] ) * ( ddsv_approx->Get_imp_vol(T, strike[i]) - t2f[i] );
	}
	
	//yc_cir.activate_cache(true);
	return diff_SQs;
}

//
void AQLMathCostFuncSZ::Set_t2f(double T_,
						  const vector<double>& strike_,
						  const vector<double>& weight_,
						  const vector<double>& t2f_
						 )
{
	T = T_;
	strike = strike_;
	weight = weight_;
	t2f = t2f_;

	ddsv_approx->Get_param(T_param, beta, theta, kappa, epsilon);
	idx = AQLModelUtilities::getIndex(T_, T_param);
	idx > 0 && AQLModelUtilities::eq(T_, T_param[idx], 1. /365) ? --idx : idx;

	n = strike.size();
	if(n != weight.size()) throw AQLCoreInvalidData("strike.size() != weight.size() : CostFunc_SZ::Set_t2f", __FILE__, __LINE__);
	if(n != t2f.size()) throw AQLCoreInvalidData("strike.size() != t2f.size() : CostFunc_SZ::Set_t2f", __FILE__, __LINE__);
}

double AQLMathCostFuncSZ::Get_Call_Opt(double t, double K)
{
	return ddsv_approx->Get_Call_Opt(t, K, true);
}

double AQLMathCostFuncSZ::Get_ImpVol(double t, double K)
{
	return ddsv_approx->Get_imp_vol(t, K, true);
}


void AQLMathCostFuncSZ::set_params(const Array& x) const
{
	size_t counter = 0;
	for(size_t i = 0; i < 4; ++i)
	{
		if(calibFlag[i])
		{
			if (i == 0)
			{
				beta[idx] = x[counter++];
			}
			else if (i == 1)
			{
				theta[idx] = x[counter++];
			}
			else if (i == 2)
			{
				kappa[idx] = x[counter++];
			}
			else if (i == 3)
			{
				epsilon[idx] = x[counter++];
			}
		}
	}
}
