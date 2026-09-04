/*! @file
    @brief Class declaration to AQLMathSABR.
*/
#ifndef __AQLMATHLEASTSQUARESABRQLIB_H__
#define __AQLMATHLEASTSQUARESABRQLIB_H__

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#include <ql/qldefines.hpp>
#include <ql/math/optimization/costfunction.hpp>
#include <ql/math/optimization/constraint.hpp>


#include "AQLFunctionVector.h"
#include "AQLBasic.h"
#include "AQLCoreTemplateType.h"
#include "AQLInterpolationBase.h"
#include "AQLCoreUtility.h"
#include "AQLMathBaseFuncUtility.h"
#include <vector>
#include <memory>

class AQLMathSABR;
class AQLMathSABR_Hagan;


//  Cost function
class AQLMathLeastSquareSABRCostFuncQlib : public QuantLib::CostFunction
{

public:

	AQLMathLeastSquareSABRCostFuncQlib(AQLMathSABR& sabr_,
								double expiry_,
								double forward_,
								double numeraire_,
								double atmVol_,
								const std::vector<double>& strike_,
				                const std::vector<double>& weight_,
								const std::vector<int>& cpsign_,
				                const std::vector<double>& param0_,
								const std::vector<bool>& flg_,
				                const std::vector<double>& target2fit_,
								const AQLString& target_);

	AQLMathLeastSquareSABRCostFuncQlib() {}

	AQLMathLeastSquareSABRCostFuncQlib(const AQLMathLeastSquareSABRCostFuncQlib& rhs);

	virtual ~AQLMathLeastSquareSABRCostFuncQlib();

	virtual AQLMathLeastSquareSABRCostFuncQlib* Clone() const { return new AQLMathLeastSquareSABRCostFuncQlib(*this); }

	// method to overload to compute the cost function value in x
	virtual QuantLib::Real value(const QuantLib::Array& x) const;
	//virtual double value(double x) const;

	// method to overload to compute the cost function values in x
	virtual QuantLib::Array values(const QuantLib::Array& x) const;
	//virtual double values(double x) const;

	virtual QuantLib::Real finiteDifferenceEpsilon() const { return 1e-8; }

	double getAlpha() { return alpha; }
	double getBeta() { return beta; }
	double getNu() { return nu; }
	double getRho() { return rho; }

protected:

private:
	size_t n;
	double expiry;
	double forward;
	double numeraire;
	double atmVol;
	std::vector<double> strike;
	std::vector<double> weight;
	std::vector<int> sgn;
	std::vector<double> param0;
	std::vector<bool> flg;
	std::vector<double> target2fit;

	void set_params(const QuantLib::Array& x) const;

	AQLMathSABR* sabr;

	mutable double alpha;
	mutable double beta;
	mutable double nu;
	mutable double rho;
	
	AQLString target;

	bool cloned;

};

//  Constraint
class AQLMathLeastSquareSABRConstraintQlib : public QuantLib::Constraint
{
public:
 	AQLMathLeastSquareSABRConstraintQlib( std::vector<double>& param0,
								   const std::vector<bool>& flg,
								   double max_alpha,
					               double min_alpha,
					               double max_beta,
					               double min_beta,
					               double max_nu,
					               double min_nu,
					               double max_rho,
					               double min_rho
								   )
	: QuantLib::Constraint(boost::shared_ptr<Constraint::Impl>( new AQLMathLeastSquareSABRConstraintQlib::Impl(param0,
																										flg,
																										max_alpha,
																										min_alpha,
																										max_beta,
																										min_beta,
																										max_nu,
																										min_nu,
																										max_rho,
																										min_rho
																										)) ) {}

private:
    class Impl : public QuantLib::Constraint::Impl
    {
    public:
		Impl(std::vector<double>& param0,
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
 //       
		virtual bool test(const QuantLib::Array& p) const;

	protected:
		void set_params(const QuantLib::Array& x) const;

		std::vector<double> param0;
		std::vector<bool> flg;
		double max_alpha;
		double min_alpha;
		double max_beta;
		double min_beta;
		double max_nu;
		double min_nu;
		double max_rho;
		double min_rho;

		mutable double alpha;
		mutable double beta;
		mutable double nu;
		mutable double rho;

    };
};

//
class AQLMathLeastSquareSABRConstraint_AntonovQlib : public QuantLib::Constraint
{
public:
	AQLMathLeastSquareSABRConstraint_AntonovQlib( double fwd,
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
										  )
	:
	Constraint(boost::shared_ptr<Constraint::Impl>( new AQLMathLeastSquareSABRConstraint_AntonovQlib::Impl(fwd,
																									param0,
																									flg,
																									max_alpha,
																									min_alpha,
																									max_beta,
																									min_beta,
																									max_nu,
																									min_nu,
																									max_rho,
																									min_rho
																									)) ) {}

protected:
    class Impl : public QuantLib::Constraint::Impl
    {
    public:
        Impl(double fwd,
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
        
        bool test(const QuantLib::Array& p) const;

    private:
		void set_params(const QuantLib::Array& x) const;

		double fwd;
		std::vector<double> param0;
		std::vector<bool> flg;
		double max_alpha;
		double min_alpha;
		double max_beta;
		double min_beta;
		double max_nu;
		double min_nu;
		double max_rho;
		double min_rho;

		mutable double alpha;
		mutable double beta;
		mutable double nu;
		mutable double rho;
    };
};

//  Cost function for CapFloor
class AQLMathLeastSquareSABRCapFloorCostFuncQlib : public QuantLib::CostFunction
{

public:

	AQLMathLeastSquareSABRCapFloorCostFuncQlib(
		DoubleMatrix alphamat_,
		DoubleMatrix betamat_,
		DoubleMatrix numat_,
		DoubleMatrix rhomat_,
		const std::vector<int>& param_pos_,
		const AQLString& approxmethod_,
		const AQLString& tenor_point_str_,
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

	AQLMathLeastSquareSABRCapFloorCostFuncQlib() {}

	AQLMathLeastSquareSABRCapFloorCostFuncQlib(const AQLMathLeastSquareSABRCapFloorCostFuncQlib& rhs);

	virtual ~AQLMathLeastSquareSABRCapFloorCostFuncQlib();

	virtual AQLMathLeastSquareSABRCapFloorCostFuncQlib* Clone() const { return new AQLMathLeastSquareSABRCapFloorCostFuncQlib(*this); }

	// method to overload to compute the cost function value in x
	virtual QuantLib::Real value(const QuantLib::Array& x) const;
			
	// method to overload to compute the cost function values in x
	virtual QuantLib::Array values(const QuantLib::Array& x) const;

	virtual QuantLib::Real finiteDifferenceEpsilon() const { return 1e-8; }

	double getAlpha() { return alpha; }
	double getBeta() { return beta; }
	double getNu() { return nu; }
	double getRho() { return rho; }

	double getAlpha_capfloor(size_t i, size_t j) { return alphamat_temp[i][j]; }
	double getBeta_capfloor(size_t i, size_t j) { return betamat_temp[i][j]; }
	double getNu_capfloor(size_t i, size_t j) { return numat_temp[i][j]; }
	double getRho_capfloor(size_t i, size_t j) { return rhomat_temp[i][j]; }

protected:

private:
	size_t n;
	DoubleMatrix  alphamat;
	DoubleMatrix  betamat;
	DoubleMatrix  numat;
	DoubleMatrix  rhomat;
	std::vector<int> param_pos;
	AQLString approxmethod;
	double tenor_point;
	std::vector<double> expiry_capfloor;
	std::vector<double> expiry_capfloorlet;
	std::vector<double> tenor;
	std::vector<double> forward;
	std::vector<double> numeraire;
	std::vector<double> strike;
	std::vector<double> weight;
	std::vector<int> sgn;
	std::vector<double> param0;
	std::vector<bool> flg;
	std::vector<double> target2fit;
	size_t num_capfloorlet;
	size_t num_diffterm;

	mutable DoubleMatrix alphamat_temp;
	mutable DoubleMatrix betamat_temp;
	mutable DoubleMatrix numat_temp;
	mutable DoubleMatrix rhomat_temp;

	unique_ptr<AQLMathSABR> mpSabr;
	void set_params(const QuantLib::Array& x) const;

	mutable double alpha;
	mutable double beta;
	mutable double nu;
	mutable double rho;

	AQLString target;

	bool cloned;

};


#endif
