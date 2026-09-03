#ifndef __LAMATHLEASTSQUARESZ_H__
#define __LAMATHLEASTSQUARESZ_H__

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#include <vector>

#include "AQLBasic.h"
#include "AQLCoreTemplateType.h"
#include "AQLInterpolationBase.h"
#include "LAMathCostFuncSZ.h"
#include "LAMathConstraintSZ.h"
#include "LAQuantLibCostFunction.h"
#include "LAQuantLibConstraint.h"

//class LAPriceSZCalibration;

//  Cost function for optimizing SZ++ models
class LAMathLeastSquareCostFuncSZ : public LAQuantLibCostFunction
{

public:

	LAMathLeastSquareCostFuncSZ(double S0_,
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

	//LAMathLeastSquareCostFuncSZ(const LAMathLeastSquareCostFuncSZ& rhs);

	virtual ~LAMathLeastSquareCostFuncSZ();

	virtual LAMathLeastSquareCostFuncSZ* clone() const { return new LAMathLeastSquareCostFuncSZ(*this); }
	
	// method to overload to compute the cost function value in x
	virtual Real value(const Array& x) const;

	// method to overload to compute the cost function values in x
	virtual Array values(const Array& x) const;

	virtual Real finiteDifferenceEpsilon() const { return 0.001; }

	void Set_t2f_LS(double T_,
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

	void set_params(const Array& x) const;

	LAMathSwaptionFormulaSZ* ddsv_approx;

	size_t idx;

	//mutable Func<CostFunc_SZ> Int_phi_SQ;

};

//  Constraint for optimizing SZ++ models
class LAMathLeastSquareConstraintSZ : public LAQuantLibConstraint
{
public:
	LAMathLeastSquareConstraintSZ(double max_beta,
		double min_beta,
		double max_theta,
		double min_theta,
		double max_kappa,
		double min_kappa,
		double max_epsilon,
		double min_epsilon,
		const vector<bool>& calibFlag
		);

	//LAMathLeastSquareConstraintSZ(const LAMathLeastSquareConstraintSZ& rhs);

	virtual ~LAMathLeastSquareConstraintSZ();

	virtual LAMathLeastSquareConstraintSZ* clone() const { return new LAMathLeastSquareConstraintSZ(*this); }
};

#endif
