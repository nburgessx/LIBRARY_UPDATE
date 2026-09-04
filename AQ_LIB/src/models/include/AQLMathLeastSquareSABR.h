/*! @file
    @brief Class declaration to AQLMathSABR.
*/
#ifndef __AQLMATHLEASTSQUARESABR_H__
#define __AQLMATHLEASTSQUARESABR_H__

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif


#include "AQLQuantLibCostFunction.h"
#include "AQLQuantLibConstraint.h"
#include "AQLMathLeastSquareSABRQlib.h"
#include "AQLBasic.h"
#include "AQLCoreTemplateType.h"
#include "AQLInterpolationBase.h"
#include <vector>

class AQLMathSABR;
class AQLMathSABR_Hagan;


//  Cost function
class AQLMathLeastSquareSABRCostFunc : public AQLQuantLibCostFunction
{

public:

	AQLMathLeastSquareSABRCostFunc(AQLMathSABR& sabr_,
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

	AQLMathLeastSquareSABRCostFunc(const AQLMathLeastSquareSABRCostFunc& rhs);

	virtual ~AQLMathLeastSquareSABRCostFunc();

	virtual AQLMathLeastSquareSABRCostFunc* clone() const { return new AQLMathLeastSquareSABRCostFunc(*this); }

};

//  Constraint
class AQLMathLeastSquareSABRConstraint : public AQLQuantLibConstraint
{
public:
 	AQLMathLeastSquareSABRConstraint( std::vector<double>& param0,
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
	AQLMathLeastSquareSABRConstraint(const AQLMathLeastSquareSABRConstraint& rhs);

	virtual ~AQLMathLeastSquareSABRConstraint();

	virtual AQLMathLeastSquareSABRConstraint* clone() const { return new AQLMathLeastSquareSABRConstraint(*this); }

};

//
class AQLMathLeastSquareSABRConstraint_Antonov : public AQLQuantLibConstraint
{
public:
	AQLMathLeastSquareSABRConstraint_Antonov( double fwd,
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

	AQLMathLeastSquareSABRConstraint_Antonov(const AQLMathLeastSquareSABRConstraint_Antonov& rhs);

	virtual ~AQLMathLeastSquareSABRConstraint_Antonov();

	virtual AQLMathLeastSquareSABRConstraint_Antonov* clone() const { return new AQLMathLeastSquareSABRConstraint_Antonov(*this); }

};

class AQLMathLeastSquareSABRCapFloorCostFunc : public AQLQuantLibCostFunction
{

public:

	AQLMathLeastSquareSABRCapFloorCostFunc(
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

	AQLMathLeastSquareSABRCapFloorCostFunc(const AQLMathLeastSquareSABRCapFloorCostFunc& rhs);

	virtual ~AQLMathLeastSquareSABRCapFloorCostFunc();

	virtual AQLMathLeastSquareSABRCapFloorCostFunc* clone() const { return new AQLMathLeastSquareSABRCapFloorCostFunc(*this); }

};

#endif
