#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include <LACoreTemplateType.h>
#include <LAString.h>
#include "LABasic.h"
#include "LADist.h"

#include "LAMathIRVanillaFuncUtility.h"
#include "LAMathLeastSquareSABR.h"
#include "LAMathLeastSquareSABRQlib.h"

using namespace std;
//
//----------------------------------------------------------------
//

//  Cost Func for optimizing SABR Parameters
LAMathLeastSquareSABRCostFunc::LAMathLeastSquareSABRCostFunc( LAMathSABR& sabr_,
														  double expiry_,
														  double forward_,
														  double numeraire_,
														  double atmVol_,
														  const vector<double>& strike_,
														  const vector<double>& weight_,
														  const vector<int>& sgn_,
														  const vector<double>& param0_,
														  const vector<bool>& flg_,
														  const vector<double>& target2fit_,
														  const LAString& target_)
{
	mCostFunction = std::make_shared<LAMathLeastSquareSABRCostFuncQlib>(sabr_, expiry_, forward_, numeraire_, atmVol_, strike_, weight_, sgn_, param0_, flg_, target2fit_, target_);
}


//
LAMathLeastSquareSABRCostFunc::LAMathLeastSquareSABRCostFunc(const LAMathLeastSquareSABRCostFunc& rhs)
{
}


LAMathLeastSquareSABRCostFunc::~LAMathLeastSquareSABRCostFunc()
{
}



LAMathLeastSquareSABRConstraint::LAMathLeastSquareSABRConstraint(vector<double>& param0,
	                                                         const vector<bool>& flg,
															 double max_alpha,
								                             double min_alpha,
								                             double max_beta,
								                             double min_beta,
								                             double max_nu,
								                             double min_nu,
								                             double max_rho,
								                             double min_rho)
{
	mConstraint = std::shared_ptr<QuantLib::Constraint>(new LAMathLeastSquareSABRConstraintQlib(param0, flg, max_alpha, min_alpha, max_beta, min_beta, max_nu, min_nu, max_rho, min_rho));

}

LAMathLeastSquareSABRConstraint::LAMathLeastSquareSABRConstraint(const LAMathLeastSquareSABRConstraint& rhs)
{
}


LAMathLeastSquareSABRConstraint::~LAMathLeastSquareSABRConstraint()
{
}


//

//
//----------------------------------------------------------------
//

LAMathLeastSquareSABRConstraint_Antonov::LAMathLeastSquareSABRConstraint_Antonov( double fwd,
																			  vector<double>& param0,
																			  const vector<bool>& flg,
																			  double max_alpha,
																			  double min_alpha,
																			  double max_beta,
																			  double min_beta,
																			  double max_nu,
																			  double min_nu,
																			  double max_rho,
																			  double min_rho)
{
	mConstraint = std::shared_ptr<QuantLib::Constraint>(new LAMathLeastSquareSABRConstraint_AntonovQlib(fwd, param0, flg, max_alpha, min_alpha, max_beta, min_beta, max_nu, min_nu, max_rho, min_rho));
}

LAMathLeastSquareSABRConstraint_Antonov::LAMathLeastSquareSABRConstraint_Antonov(const LAMathLeastSquareSABRConstraint_Antonov& rhs)
{
}


LAMathLeastSquareSABRConstraint_Antonov::~LAMathLeastSquareSABRConstraint_Antonov()
{
}

//  Cost Func for optimizing SABR Parameters with CapFloor
LAMathLeastSquareSABRCapFloorCostFunc::LAMathLeastSquareSABRCapFloorCostFunc(
	DoubleMatrix alphamat_,
	DoubleMatrix betamat_,
	DoubleMatrix numat_,
	DoubleMatrix rhomat_,
	const std::vector<int>& param_pos_,
	const LAString& approxmethod_,
	const LAString& tenorpoint_,
	const vector<double>& expiry_capfloor_,
	const vector<double>& expiry_capfloorlet_,
	const vector<double>& tenor_,
	const vector<double>& forward_,
	const vector<double>& numeraire_,
	const vector<double>& strike_,
	const vector<double>& weight_,
	const vector<int>& sgn_,
	const vector<double>& param0_,
	const vector<bool>& flg_,
	const vector<double>& target2fit_,
	const LAString& target_,
	const size_t& num_capfloorlet_,
	const size_t& num_diffterm_)
{

	mCostFunction = std::make_shared<LAMathLeastSquareSABRCapFloorCostFuncQlib>(alphamat_, betamat_, numat_, rhomat_, param_pos_, approxmethod_, tenorpoint_, expiry_capfloor_, expiry_capfloorlet_, tenor_, forward_, numeraire_, strike_, weight_, sgn_, param0_, flg_, target2fit_, target_, num_capfloorlet_, num_diffterm_);
}


//
LAMathLeastSquareSABRCapFloorCostFunc::LAMathLeastSquareSABRCapFloorCostFunc(const LAMathLeastSquareSABRCapFloorCostFunc& rhs)
{
}


LAMathLeastSquareSABRCapFloorCostFunc::~LAMathLeastSquareSABRCapFloorCostFunc()
{
}
