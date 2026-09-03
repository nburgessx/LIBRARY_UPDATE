/*! @file
    @brief Class declaration to LAMathSABR.

*/
//  2009, AlgoQuantHub.
#ifndef __LAMATHLEASTSQUARESABR_H__
#define __LAMATHLEASTSQUARESABR_H__

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif


#include "LAQuantLibCostFunction.h"
#include "LAQuantLibConstraint.h"
#include "LAMathLeastSquareSABRQlib.h"
#include "AQLBasic.h"
#include "AQLCoreTemplateType.h"
#include "AQLInterpolationBase.h"
#include <vector>

class LAMathSABR;
class LAMathSABR_Hagan;


//  Cost function
class LAMathLeastSquareSABRCostFunc : public LAQuantLibCostFunction
{

public:

	LAMathLeastSquareSABRCostFunc(LAMathSABR& sabr_,
								double expiry_,
								double forward_,
								double numeraire_,
								double atmVol_,
								const std::vector<double>& strike_,
				                const std::vector<double>& weight_,
								const std::vector<int>& cpsign_,
				                const std::vector<double>& param0_,
								const std::vector<bool>& flg_,
				                const std::vector<double>& vol0_,
								const AQLString& target2fit_
				                );

	LAMathLeastSquareSABRCostFunc(const LAMathLeastSquareSABRCostFunc& rhs);

	virtual ~LAMathLeastSquareSABRCostFunc();

	virtual LAMathLeastSquareSABRCostFunc* clone() const { return new LAMathLeastSquareSABRCostFunc(*this); }

};

//  Constraint
class LAMathLeastSquareSABRConstraint : public LAQuantLibConstraint
{
public:
 	LAMathLeastSquareSABRConstraint( std::vector<double>& param0,
								   const std::vector<bool>& flg,
								   double max_alpha,
					               double min_alpha,
					               double max_beta,
					               double min_beta,
					               double max_nu,
					               double min_nu,
					               double max_rho,
					               double min_rho
								   );
	LAMathLeastSquareSABRConstraint(const LAMathLeastSquareSABRConstraint& rhs);

	virtual ~LAMathLeastSquareSABRConstraint();

	virtual LAMathLeastSquareSABRConstraint* clone() const { return new LAMathLeastSquareSABRConstraint(*this); }

};

//
class LAMathLeastSquareSABRConstraint_Antonov : public LAQuantLibConstraint
{
public:
	LAMathLeastSquareSABRConstraint_Antonov( double fwd,
										   std::vector<double>& param0,
										   const std::vector<bool>& flg,
										   double max_alpha,
										   double min_alpha,
										   double max_beta,
										   double min_beta,
										   double max_nu,
										   double min_nu,
										   double max_rho,
										   double min_rho
										  );

	LAMathLeastSquareSABRConstraint_Antonov(const LAMathLeastSquareSABRConstraint_Antonov& rhs);

	virtual ~LAMathLeastSquareSABRConstraint_Antonov();

	virtual LAMathLeastSquareSABRConstraint_Antonov* clone() const { return new LAMathLeastSquareSABRConstraint_Antonov(*this); }

};

class LAMathLeastSquareSABRCapFloorCostFunc : public LAQuantLibCostFunction
{

public:

	LAMathLeastSquareSABRCapFloorCostFunc(
		DoubleMatrix alphamat_,
		DoubleMatrix betamat_,
		DoubleMatrix numat_,
		DoubleMatrix rhomat_,
		const std::vector<int>& param_pos_,
		const AQLString& approxmethod_,
		const AQLString& tenorpoint_,
		const std::vector<double>& expiry_capfloor_,
		const std::vector<double>& expiry_capfloorlet_,
		const std::vector<double>& tenor_,
		const std::vector<double>& forward_,
		const std::vector<double>& numeraire_,
		const std::vector<double>& strike_,
		const std::vector<double>& weight_,
		const std::vector<int>& cpsign_,
		const std::vector<double>& param0_,
		const std::vector<bool>& flg_,
		const std::vector<double>& vol0_,
		const AQLString& target2fit_,
		const size_t& num_capfloorlet_,
		const size_t& num_diffterm_
	);

	LAMathLeastSquareSABRCapFloorCostFunc(const LAMathLeastSquareSABRCapFloorCostFunc& rhs);

	virtual ~LAMathLeastSquareSABRCapFloorCostFunc();

	virtual LAMathLeastSquareSABRCapFloorCostFunc* clone() const { return new LAMathLeastSquareSABRCapFloorCostFunc(*this); }

};

#endif
