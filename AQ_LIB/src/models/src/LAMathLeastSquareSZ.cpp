#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include <LACoreTemplateType.h>
#include <LAString.h>
#include "LABasic.h"
#include "LADist.h"
#include <LAMathLeastSquareSZ.h>
#include <LAMathSwaptionFormulaSZ.h>
#include <LACoreAppError.h>
#include <LAMathCostFuncSZ.h>
#include <LAMathConstraintSZ.h>

using namespace std;

//  Cost Func for optimizing SZ Parameters
LAMathLeastSquareCostFuncSZ::LAMathLeastSquareCostFuncSZ(double S0_,
	double V0_,
	double rho_,
	LAMathPathYieldCurve& curve_d_,
	LAMathPathYieldCurve& curve_f_,
	vector<double>& T_param_,
	vector<double>& beta_,
	vector<double>& theta_,
	vector<double>& kappa_,
	vector<double>&	epsilon_,
	const vector<bool>& calibFlag_
	)
{

	mCostFunction = std::make_shared<LAMathCostFuncSZ>(S0_, V0_, rho_, curve_d_, curve_f_, T_param_, beta_, theta_, kappa_, epsilon_, calibFlag_);
}


//
//LAMathLeastSquareCostFuncSZ::LAMathLeastSquareCostFuncSZ(const LAMathLeastSquareCostFuncSZ& rhs)
//{
//}


LAMathLeastSquareCostFuncSZ::~LAMathLeastSquareCostFuncSZ()
{
}



//  Constraint for optimizing SZ Parameters
LAMathLeastSquareConstraintSZ::LAMathLeastSquareConstraintSZ(double max_beta,
														double min_beta,
														double max_theta,
														double min_theta,
														double max_kappa,
														double min_kappa,
														double max_epsilon,
														double min_epsilon,
														const vector<bool>& calibFlag
														)
{
	mConstraint = std::shared_ptr<QuantLib::Constraint>(new LAMathConstraintSZ(max_beta, min_beta, max_theta, min_theta, max_kappa, min_kappa, max_epsilon, min_epsilon, calibFlag));

}

//LAMathLeastSquareConstraintSZ::LAMathLeastSquareConstraintSZ(const LAMathLeastSquareConstraintSZ& rhs)
//{
//}


LAMathLeastSquareConstraintSZ::~LAMathLeastSquareConstraintSZ()
{
}

// method to overload to compute the cost function value in x
Real LAMathLeastSquareCostFuncSZ::value(const Array& x) const
{
	if (n == 0) throw LACoreInvalidData("target to fit is not setted! :: CostFunc_SZ::value", __FILE__, __LINE__);
	double m = x.size();

	if (m == 0 || m > 4)
		//if(m == 0 || m > 3)
		throw LACoreInvalidData("m == 0 || m > 4 : CostFunc_SZ::value", __FILE__, __LINE__);

	set_params(x);
	//beta[idx] = x[0]; theta[idx] = x[1]; kappa[idx] = x[2]; epsilon[idx] = x[3];
	//beta[idx] = x[0]; theta[idx] = x[1]; epsilon[idx] = x[2];
	ddsv_approx->Set_param(T_param, beta, theta, kappa, epsilon);

	double diff_SQ = 0.;
	for (size_t i = 0; i < n; ++i)
	{
		//double imp_vol = ddsv_approx->Get_imp_vol(T, strike[i]);
		//diff_SQ += weight[i] * ( imp_vol - t2f[i] ) * ( imp_vol - t2f[i] );
		double vol = ddsv_approx->Get_imp_vol(T, strike[i], true);
		diff_SQ += weight[i] * (vol - t2f[i]) * (vol - t2f[i]);
	}

	return diff_SQ;
}

// method to overload to compute the cost function values in x
Array LAMathLeastSquareCostFuncSZ::values(const Array& x) const
{
	double m = x.size();

	//if(m == 0 || m > 4)
	if (m == 0 || m > 3)
		throw LACoreInvalidData("m == 0 || m > 3 : CostFunc_SZ::value", __FILE__, __LINE__);

	set_params(x);
	//beta[idx] = x[0]; theta[idx] = x[1]; kappa[idx] = x[2]; epsilon[idx] = x[3];
	//beta[idx] = x[0]; theta[idx] = x[1]; epsilon[idx] = x[2];
	ddsv_approx->Set_param(T_param, beta, theta, kappa, epsilon);

	size_t k = 0;
	Array diff_SQs(n);
	for (size_t i = 0; i < n; ++i)
	{
		//diff_SQs[i-1] = sqrt( weight[i] ) * ( ddsv_approx->Get_Call_Opt(T, strike[i]) - t2f[i] );
		double vol = ddsv_approx->Get_imp_vol(T, strike[i], true);
		diff_SQs[i] = sqrt(weight[i]) * (vol - t2f[i]);
		//diff_SQs[i-1] = sqrt( weight[i] ) * ( ddsv_approx->Get_imp_vol(T, strike[i]) - t2f[i] );
	}

	//yc_cir.activate_cache(true);
	return diff_SQs;
}

void LAMathLeastSquareCostFuncSZ::Set_t2f_LS(double T_,
						  const vector<double>& strike_,
						  const vector<double>& weight_,
						  const vector<double>& t2f_
						 )
{
	dynamic_pointer_cast<LAMathCostFuncSZ>(mCostFunction)->Set_t2f(T_, strike_, weight_, t2f_);
}

double LAMathLeastSquareCostFuncSZ::Get_Call_Opt(double t, double K)
{
	return ddsv_approx->Get_Call_Opt(t, K, true);
}

double LAMathLeastSquareCostFuncSZ::Get_ImpVol(double t, double K)
{
	return ddsv_approx->Get_imp_vol(t, K, true);
}


void LAMathLeastSquareCostFuncSZ::set_params(const Array& x) const
{
	size_t counter = 0;
	for (size_t i = 0; i < 4; ++i)
	{
		if (calibFlag[i])
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

