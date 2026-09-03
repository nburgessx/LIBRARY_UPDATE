/*! @file
    @brief Source code for LAMathBlackCmsOption.
*/
#include <LAMathBlackCmsOption.h>
#include "AQLDist.h"

/*!
    @brief constructor

	@param[in] S0 Swaprate
	@param[in] annuity0 Annuity at spot
	@param[in] option_maturity Option maturity of underling swaption
	@param[in] T_fix Fixing time of CMS
	@param[in] T_pay Peyment time of CMS
	@param[in] vol volatility of swaprate
	@param[in] swaption_prem Premium of reference swaption
	@param[in] num_cf Number of swap cash flows
*/
LAMathBalckCmsOption::LAMathBalckCmsOption(double S0_,
								   double annuity0_,
								   double option_maturity_,
								   double T_fix_,
								   double T_pay_,
								   double vol_,
								   double swaption_prem_,
								   size_t num_cf_
								  )
:
S0(S0_),
annuity0(annuity0_),
t_f(T_fix_),
t_p(T_pay_),
vol(vol_),
swaption_prem(swaption_prem_),
T(option_maturity_),
n(static_cast<double>(num_cf_))
{
}

/*!
    @brief get CMS Cap/Floor value

	@param[in] P Discount bond price
	@param[in] K Strike
	@param[in] sgn Flg for cap/floor (sgn = 1 represents cap, sgn = -1 indicates floor)
    @return CMS Cap/Floor value
*/
double LAMathBalckCmsOption::Get_cms_option(double P, double K, int sgn)
{
	double var = vol * vol * t_f;

	double tmp1 = S0 * S0 * exp(vol * vol * t_f) * AQLDist::normsdist( sgn * d(S0, K, var, 1.5) );
    double tmp2 = S0 * (S0 + K) * AQLDist::normsdist( sgn * d(S0, K, var, 0.5) );
	double tmp3 = S0 * K * AQLDist::normsdist( sgn * d(S0, K, var, -0.5) );
    
    return P / annuity0 * swaption_prem + sgn * G_D() * annuity0 * (tmp1 - tmp2 + tmp3);
}

/*!
    @brief get d value

	@param[in] S Swaprate
	@param[in] K Strike
	@param[in] var Variance of swap rate
	@param[in] lambda 
    @return d value
*/
double LAMathBalckCmsOption::d(double S, double K, double var, double lambda)
{
	return (log(S/K) + lambda * var) / sqrt(var);
}

/*!
    @brief constructor

	@param[in] S0 Swaprate
	@param[in] annuity0 Annuity at spot
	@param[in] option_maturity Option maturity of underling swaption
	@param[in] T_fix Fixing time of CMS
	@param[in] T_pay T_pay Peyment time of CMS
	@param[in] vol volatility of swaprate
	@param[in] swaption_prem Premium of reference swaption
	@param[in] num_cf Number of swap cash flows
	@param[in] num_cf_per_year Number of swap cash flows per year
*/
LAMathBalckCmsOptionApproxA::LAMathBalckCmsOptionApproxA(double S0_,
												   double annuity0_,
												   double option_maturity_,
												   double T_fix_,
												   double T_pay_,
												   double vol_,
												   double swaption_prem_,
												   size_t num_cf_,
												   size_t num_cf_per_year_
												  )
:
LAMathBalckCmsOption(S0_,
				 annuity0_,
				 option_maturity_,
				 T_fix_,
				 T_pay_,
				 vol_,
				 swaption_prem_,num_cf_
				),
q(static_cast<double>(num_cf_per_year_))
{}

/*!
    @brief delta
    @return delta value
*/
double LAMathBalckCmsOptionApproxA::delta()
{
	return q * (t_p - T);
}

/*!
    @brief Derivative of G
    @return G_D value
*/
double LAMathBalckCmsOptionApproxA::G_D()
{
	double tmp = (q + S0) / q;
	double denominator = (q + S0) * (-1 + pow(tmp,n)) * (-1 + pow(tmp,n));
	double multiplor = pow(tmp, n - delta()) * (q * (-1 + pow(tmp,n)) - S0 * (n + (-1 + pow(tmp,n)) * (-1 + delta())));
	
	return multiplor / denominator;
}