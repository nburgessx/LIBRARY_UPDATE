#ifndef __LAMATHLEASTSQUARESZ_H__
#define __LAMATHLEASTSQUARESZ_H__

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#include <vector>

#include "AQLBasic.h"
#include "AQLCoreTemplateType.h"
#include "AQLInterpolationBase.h"
#include "AQLMathCostFuncSZ.h"
#include "AQLMathConstraintSZ.h"
#include "AQLQuantLibCostFunction.h"
#include "AQLQuantLibConstraint.h"

//class AQLPriceSZCalibration;

//  Cost function for optimizing SZ++ models
class AQLMathLeastSquareCostFuncSZ : public AQLQuantLibCostFunction
{

public:

	AQLMathLeastSquareCostFuncSZ(double S0_,
								double V0_,
								double rho_,
								AQLMathPathYieldCurve& curve_d_,
								AQLMathPathYieldCurve& curve_f_,
								vector<double>& T_param_,
								vector<double>& beta_,
								vector<double>& theta_,
								vector<double>& kappa_,
								vector<double>&	epsilon_,
								const vector<bool>& calibFlag_
								);

	//AQLMathLeastSquareCostFuncSZ(const AQLMathLeastSquareCostFuncSZ& rhs);

	virtual ~AQLMathLeastSquareCostFuncSZ();

	virtual AQLMathLeastSquareCostFuncSZ* clone() const { return new AQLMathLeastSquareCostFuncSZ(*this); }
	
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

	AQLMathSwaptionFormulaSZ* ddsv_approx;

	size_t idx;

	//mutable Func<CostFunc_SZ> Int_phi_SQ;

};

//  Constraint for optimizing SZ++ models
class AQLMathLeastSquareConstraintSZ : public AQLQuantLibConstraint
{
public:
	AQLMathLeastSquareConstraintSZ(double max_beta,
		double min_beta,
		double max_theta,
		double min_theta,
		double max_kappa,
		double min_kappa,
		double max_epsilon,
		double min_epsilon,
		const vector<bool>& calibFlag
		);

	//AQLMathLeastSquareConstraintSZ(const AQLMathLeastSquareConstraintSZ& rhs);

	virtual ~AQLMathLeastSquareConstraintSZ();

	virtual AQLMathLeastSquareConstraintSZ* clone() const { return new AQLMathLeastSquareConstraintSZ(*this); }
};

#endif
