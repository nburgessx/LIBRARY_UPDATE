#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include <AQLCoreTemplateType.h>
#include <AQLString.h>
#include "AQLBasic.h"
#include "AQLDist.h"

#include "AQLMathIRVanillaFuncUtility.h"
#include "AQLMathLeastSquareSABR.h"
#include "AQLMathLeastSquareSABRQlib.h"

using namespace std;
//
//----------------------------------------------------------------
//

//  Cost Func for optimizing SABR Parameters
AQLMathLeastSquareSABRCostFunc::AQLMathLeastSquareSABRCostFunc( AQLMathSABR& sabr_,
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
														  const AQLString& target_)
{
	mCostFunction = std::make_shared<AQLMathLeastSquareSABRCostFuncQlib>(sabr_, expiry_, forward_, numeraire_, atmVol_, strike_, weight_, sgn_, param0_, flg_, target2fit_, target_);
}


//
AQLMathLeastSquareSABRCostFunc::AQLMathLeastSquareSABRCostFunc(const AQLMathLeastSquareSABRCostFunc& rhs)
{
}


AQLMathLeastSquareSABRCostFunc::~AQLMathLeastSquareSABRCostFunc()
{
}



AQLMathLeastSquareSABRConstraint::AQLMathLeastSquareSABRConstraint(vector<double>& param0,
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
	mConstraint = std::shared_ptr<QuantLib::Constraint>(new AQLMathLeastSquareSABRConstraintQlib(param0, flg, max_alpha, min_alpha, max_beta, min_beta, max_nu, min_nu, max_rho, min_rho));

}

AQLMathLeastSquareSABRConstraint::AQLMathLeastSquareSABRConstraint(const AQLMathLeastSquareSABRConstraint& rhs)
{
}


AQLMathLeastSquareSABRConstraint::~AQLMathLeastSquareSABRConstraint()
{
}


//

//
//----------------------------------------------------------------
//

AQLMathLeastSquareSABRConstraint_Antonov::AQLMathLeastSquareSABRConstraint_Antonov( double fwd,
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
	mConstraint = std::shared_ptr<QuantLib::Constraint>(new AQLMathLeastSquareSABRConstraint_AntonovQlib(fwd, param0, flg, max_alpha, min_alpha, max_beta, min_beta, max_nu, min_nu, max_rho, min_rho));
}

AQLMathLeastSquareSABRConstraint_Antonov::AQLMathLeastSquareSABRConstraint_Antonov(const AQLMathLeastSquareSABRConstraint_Antonov& rhs)
{
}


AQLMathLeastSquareSABRConstraint_Antonov::~AQLMathLeastSquareSABRConstraint_Antonov()
{
}

//  Cost Func for optimizing SABR Parameters with CapFloor
AQLMathLeastSquareSABRCapFloorCostFunc::AQLMathLeastSquareSABRCapFloorCostFunc(
	DoubleMatrix alphamat_,
	DoubleMatrix betamat_,
	DoubleMatrix numat_,
	DoubleMatrix rhomat_,
	const std::vector<int>& param_pos_,
	const AQLString& approxmethod_,
	const AQLString& tenorpoint_,
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
	const AQLString& target_,
	const size_t& num_capfloorlet_,
	const size_t& num_diffterm_)
{

	mCostFunction = std::make_shared<AQLMathLeastSquareSABRCapFloorCostFuncQlib>(alphamat_, betamat_, numat_, rhomat_, param_pos_, approxmethod_, tenorpoint_, expiry_capfloor_, expiry_capfloorlet_, tenor_, forward_, numeraire_, strike_, weight_, sgn_, param0_, flg_, target2fit_, target_, num_capfloorlet_, num_diffterm_);
}


//
AQLMathLeastSquareSABRCapFloorCostFunc::AQLMathLeastSquareSABRCapFloorCostFunc(const AQLMathLeastSquareSABRCapFloorCostFunc& rhs)
{
}


AQLMathLeastSquareSABRCapFloorCostFunc::~AQLMathLeastSquareSABRCapFloorCostFunc()
{
}
