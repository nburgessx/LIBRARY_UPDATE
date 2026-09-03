#pragma once

#include <vector>

//// Forward Declaration ////
class LADist;
using namespace std;

/*! 
    @brief Class to compute approximated CMS options prices.
*/
class LAMathBalckCmsOption
{
public:
	// constructor
	LAMathBalckCmsOption(double S0_,
					   double annuity0_,
					   double option_maturity_,
					   double T_fix_,
					   double T_pay_,
					   double vol_,
					   double swaption_prem_,
					   size_t num_cf_
					  );

	// return cms option prices (sgn=1 represents cap, sgn = -1 indicates floor)
	double Get_cms_option(double P, double K, int sgn);

protected:

	double S0; // swaprate
	double T; // option maturity
	
	double t_f; // fixing timing
	double t_p; // payment timing

	double n; // number of cash flows


private:

	// a function 
	double d(double S, double K, double var, double lambda);

	// common function for subclasses
	virtual double G_D() = 0;

	double annuity0; // annuity

	double swaption_prem; // swaption premium

	double vol; // volatiltiy of swap rate
};

//
class LAMathBalckCmsOptionApproxA : public LAMathBalckCmsOption
{
public:

	// constructor
	LAMathBalckCmsOptionApproxA(double S0_,
							 double annuity0_,
							 double option_maturity_,
							 double T_fix_,
							 double T_pay_,
							 double vol_,
							 double swaption_prem_,
							 size_t num_cf_,
							 size_t num_cf_per_year_
							);

private:

	double delta();	
	double G_D();

	//
	double q;
};

