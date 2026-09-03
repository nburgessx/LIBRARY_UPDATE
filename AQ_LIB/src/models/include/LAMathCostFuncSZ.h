#ifndef __LAMATHCOSTFUNCSZ_H__
#define __LAMATHCOSTFUNCSZ_H__

#include <ql/math/optimization/costfunction.hpp>
//furuya
//#include <function.h>
#include "LAFunction.h"

#include <vector>

using namespace std;
using namespace QuantLib;

//class YieldCurve;
class LAMathPathYieldCurve;
class LAMathSwaptionFormulaSZ;

//  Cost function for optimizing SZ++ models
class LAMathCostFuncSZ : public QuantLib::CostFunction
{

public:
	
	LAMathCostFuncSZ(double S0_,
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
               );		

	virtual ~LAMathCostFuncSZ();

	// method to overload to compute the cost function value in x
    virtual Real value(const Array& x) const;

	// method to overload to compute the cost function values in x
    virtual Array values(const Array& x) const;

	virtual Real finiteDifferenceEpsilon() const { return 0.001; }

	void Set_t2f(double T_,
				 const vector<double>& strike_,
				 const vector<double>& weight_,
				 const vector<double>& t2f_
				);

	double Get_Call_Opt(double t, double K);
	double Get_ImpVol(double t, double K);

protected:	

	double T;
	size_t n;
	vector<double> strike;
	vector<double> weight;
	vector<double> t2f;
	vector<bool> calibFlag;

	vector<double> T_param;
	mutable vector<double> beta;
	mutable vector<double> theta;
	mutable vector<double> epsilon;
	mutable vector<double> kappa;

private:

	LAMathCostFuncSZ(const LAMathCostFuncSZ& rhs) {rhs;}

	void set_params(const Array& x) const;

	LAMathSwaptionFormulaSZ* ddsv_approx;

	size_t idx;

	//mutable Func<CostFunc_SZ> Int_phi_SQ;

};

#endif
