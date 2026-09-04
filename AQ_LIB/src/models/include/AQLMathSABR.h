/*! @file
    @brief Class declaration to AQLMathSABR.

*/
#ifndef __AQLMATHSABR_H__
#define __AQLMATHSABR_H__

#ifdef __GNUG__
#pragma interface
#endif

//#include <ql/qldefines.hpp>
//#include <ql/math/optimization/costfunction.hpp>
//#include <ql/math/optimization/constraint.hpp>


#include "AQLFunctionVector.h"
#include "AQLBasic.h"
#include <math.h>
#include <AQLCoreTemplateType.h>
#include "AQLInterpolationBase.h"
#include "AQLCoreUtility.h"
#include "AQLMathBaseFuncUtility.h"
#include "AQLFunction.h"
#include <vector>

const double eps_SABR = 0.0000001; //0.001bp
//const double alpha_high = 2.;
const double alpha_high = 3.;
const double alpha_low = eps_SABR;
const double beta_high = 1.;
const double beta_low = 0.;
const double nu_high = 3.;
const double nu_low = eps_SABR;
const double rho_high = 1. - eps_SABR;
const double rho_low = -1. + eps_SABR;


#ifndef	CALIB_TARGET_VOLATILITY
#define CALIB_TARGET_VOLATILITY	"VOLATILITY"
#endif
#ifndef	CALIB_TARGET_PREMIUM
#define CALIB_TARGET_PREMIUM	"PREMIUM"
#endif

using namespace std;

//++++++++ Funahashi ++++++++ //delete
//class AQLMathSABR
//{
//public:
//    // constructor
//    AQLMathSABR(double alpha_, double beta_, double nu_, double rho_);
//
//    AQLMathSABR();
//
//    // destructor
//    virtual ~AQLMathSABR();
//
//    //bool checkSABRParam();
//
//    void checkSABRParam2();
//
//    void setSABRParam(double alpha_, double beta_, double nu_, double rho_);
//
//    double getSABRVol(double T, double F, double K);
//
//    double getSABRPrem(double T, double F, double K, double Nu, unsigned int sgn);
//
//    void setAlphaForATMVol(double atmVol, double T, double F, bool isAlpha0Use = false);
//
//    bool checkAlphaForATMVol(double atmVol, double T, double F);
//
//    void set_alpha(double alpha_, bool isMap) 
//    { 
//        double width = (alpha_high - alpha_low) / 2.;
//        double center = (alpha_high + alpha_low) / 2.;
//        alpha = isMap ? atan(alpha_) * 2. / AQLMath::pi() * width  + center : alpha_;
//    };
//
//    void set_beta(double beta_, bool isMap) 
//    { 
//        double width = (beta_high - beta_low) / 2.;
//        double center = (beta_high + beta_low) / 2.;
//        beta = isMap ? atan(beta_) * 2. / AQLMath::pi() * width  + center : beta_;
//    };
//
//    void set_nu(double nu_, bool isMap)
//    { 
//        double width = (nu_high - nu_low) / 2.;
//        double center = (nu_high + nu_low) / 2.;
//        nu = isMap ? atan(nu_) * 2. / AQLMath::pi() * width  + center : nu_;
//    };
//
//    void set_rho(double rho_,   bool isMap )
//    { 
//        double width = (rho_high - rho_low) / 2.;
//        double center = (rho_high + rho_low) / 2.;
//        rho = isMap ? atan(rho_) * 2. / AQLMath::pi() * width  + center  : rho_;
//    };
//
//    double getAlpha() { return alpha; }
//
//    double getBeta() { return beta; }
//
//    double getNu() { return nu; }
//
//    double getRho() { return rho; }
//
//private:
//    double alpha;
//    double beta;
//    double nu;
//    double rho;
//
//    double chi(double z);
//};
//++++++++ Funahashi ++++++++ // delete

//++++++++ Funahashi ++++++++ // add
class AQLMathSABR
{
public:
    // constructor
    AQLMathSABR(double alpha_, double beta_, double nu_, double rho_);

    AQLMathSABR();

    // destructor
    virtual ~AQLMathSABR();

	virtual AQLMathSABR* clone() const = 0;

    //bool checkSABRParam();

    void checkSABRParam();

    double BlackImplVol( double prem, double F, double T, double K, int sgn, double low = 0.000001, double high = 10.);

	double BlackPrem( double vol, double F, double T, double K, double NU, int sgn);

    void setSABRParam(double alpha_, double beta_, double nu_, double rho_);

	virtual double getSABRVol(double T, double F, double K) = 0;

	virtual double getSABRPrem(double T, double F, double K, double Nu, int sgn) = 0;

	virtual void setAlphaForATMVol(double atmVol, double T, double F, bool isAlpha0Use = false) = 0;

    virtual bool checkAlphaForATMVol(double atmVol, double T, double F) = 0;

	//
    void set_alpha(double alpha_, bool isMap);
    void set_beta(double beta_, bool isMap);
    void set_nu(double nu_, bool isMap);
    void set_rho(double rho_,   bool isMap );

	//
    double getAlpha() { return alpha; }
    double getBeta() { return beta; }
    double getNu() { return nu; }
    double getRho() { return rho; }

protected:

	double alpha;
    double beta;
    double nu;
    double rho;

private:
};

//
class AQLMathSABR_Hagan : public AQLMathSABR
{
public:
    // constructor
    AQLMathSABR_Hagan(double alpha_, double beta_, double nu_, double rho_, bool isLognormal = true);

    // constructor
    AQLMathSABR_Hagan(const vector<double>& parameters_);

	AQLMathSABR_Hagan() : AQLMathSABR() {}

    // destructor
	virtual ~AQLMathSABR_Hagan() {}

	virtual AQLMathSABR_Hagan* clone() const { return new AQLMathSABR_Hagan(*this); }

    double getSABRVol(double T, double F, double K);

	double getSABRPrem(double T, double F, double K, double Nu, int sgn);

	void setAlphaForATMVol(double atmVol, double T, double F, bool isAlpha0Use = false);
    
	bool checkAlphaForATMVol(double atmVol, double T, double F);



private:

    double chi(double z);

	double getSABRLognormalVol(double T, double F, double K);

	double getSABRNormalVol(double T, double F, double K);

	double getSABRPremLognormalVol(double T, double F, double K, double numeraire, int sgn);

	double getSABRPremNormalVol(double T, double F, double K, double numeraire, int sgn);

	bool isLognormal = true;

};

//
class AQLMathSABR_Antonov : public AQLMathSABR
{
public:
    // constructor
    AQLMathSABR_Antonov(double alpha_, double beta_, double nu_, double rho_, int int_step_ = 20);
	AQLMathSABR_Antonov() : AQLMathSABR() {}

    // destructor
	virtual ~AQLMathSABR_Antonov() { delete sub_func; sub_func = 0; }

	AQLMathSABR_Antonov(const AQLMathSABR_Antonov& rhs );

	virtual AQLMathSABR_Antonov* clone() const { return new AQLMathSABR_Antonov(*this); }

	double getSABRVol(double T, double F, double K);

    double getSABRPrem(double T, double F, double K, double Nu, int sgn);

    void setAlphaForATMVol(double atmVol, double T, double F, bool isAlpha0Use = false);

	bool checkAlphaForATMVol(double atmVol, double T, double F);

	void set(double F,double T, const std::vector<double>& sabr_params) { sub_func->set(F,T,sabr_params); }

	bool getFlg() { return sub_func->getFlg(); }

private:

	class swaption_prem_antonov : public AQLFunction
	{
		public:
			swaption_prem_antonov(int int_step_ = 20);
			void setIntegralParams(const double start, const double step, size_t num) { flg_negative_nu = false; }
			
			void set(double S0_,double T_,const std::vector<double>& sabr_params);

			virtual swaption_prem_antonov* clone() const { return new swaption_prem_antonov(*this); }

			double operator()(double K) const;

			//double operator()(double x);
			bool getFlg() const { return flg_negative_nu; }
		
		private:

			//
			double S0;
			double T;

			// SABR params
			double alpha;
			double beta;
			double nu;
			double rho;
			size_t num_int_step;
			//
			mutable double nu_;
			mutable double s_plus;
			mutable double s_minus;
	
			mutable bool flg_negative_nu;

			AQLMathFunction<swaption_prem_antonov> int_1_;
			double int_1(double x) const;
			double phi(double s) const;

			AQLMathFunction<swaption_prem_antonov> int_2_;
			double int_2(double x) const;
			double psi(double s) const;

			double G(double t, double s) const;
			double R(double t, double s) const;

			double dR(double t, double s) const;

	}* sub_func;
};

//
class AQLMathSABR_Chaos : public AQLMathSABR
{
public:
    // constructor
    AQLMathSABR_Chaos(double alpha_, double beta_, double nu_, double rho_);

	AQLMathSABR_Chaos() : AQLMathSABR() {}

    // destructor
	virtual ~AQLMathSABR_Chaos() {}

	virtual AQLMathSABR_Chaos* clone() const = 0;

	double getSABRVol(double T, double F, double K);

    double getSABRPrem(double T, double F, double K, double Nu, int sgn);

    void setAlphaForATMVol(double atmVol, double T, double F, bool isAlpha0Use = false);

	bool checkAlphaForATMVol(double atmVol, double T, double F);

	virtual double Sigma(double T) const = 0;
	virtual double q1(double T) const = 0;
	virtual double q2(double T) const = 0;
	virtual double q3(double T) const = 0;
	virtual double q4(double T) const = 0;
	virtual double q5(double T) const = 0;

private:
};

//
class AQLMathSABR_Chaos_DD : public AQLMathSABR_Chaos
{
public:
    // constructor
    AQLMathSABR_Chaos_DD(double alpha_, double beta_, double nu_, double rho_);

	AQLMathSABR_Chaos_DD() : AQLMathSABR_Chaos() {}

    // destructor
	virtual ~AQLMathSABR_Chaos_DD() {}

	virtual AQLMathSABR_Chaos_DD* clone() const { return new AQLMathSABR_Chaos_DD(*this); }

	double Sigma(double T) const;
	double q1(double T) const;
	double q2(double T) const;
	double q3(double T) const;
	double q4(double T) const;
	double q5(double T) const;

private:
};

//
class AQLMathConvexityAdust_Antonov : public AQLFunction
{
public:
	AQLMathConvexityAdust_Antonov( double T_,
							    double S0_,
								double alpha,
								double beta,
								double nu,
								double rho,
								size_t num_int_step = 20);

	AQLMathConvexityAdust_Antonov(const AQLMathConvexityAdust_Antonov& rhs);

	virtual ~AQLMathConvexityAdust_Antonov()
	{
		delete swaption_prem; swaption_prem = 0;
	}

	bool getFlg() { return swaption_prem->getFlg();}

	virtual AQLMathConvexityAdust_Antonov* clone() const { return new AQLMathConvexityAdust_Antonov(*this); }

	double operator()(double K) const { return swaption_prem->getSABRPrem(T,S0,K,1.,1); }
private:

	AQLMathSABR_Antonov* swaption_prem;
	
	double T;
	double S0;
};

//  Cost function
//class AQLMathLeastSquareSABRCostFunc : public QuantLib::CostFunction
//{
//
//public:
//
//	AQLMathLeastSquareSABRCostFunc(AQLMathSABR& sabr_,
//								double expiry_,
//								double forward_,
//								double numeraire_,
//								const std::vector<double>& strike_,
//				                const std::vector<double>& weight_,
//								const std::vector<int>& cpsign_,
//				                const std::vector<double>& param0_,
//								const std::vector<bool>& flg_,
//				                const std::vector<double>& vol0_,
//								const AQLString& target2fit_
//				                );
//
//	AQLMathLeastSquareSABRCostFunc() {}
//
//	AQLMathLeastSquareSABRCostFunc(const AQLMathLeastSquareSABRCostFunc& rhs);
//
//	virtual ~AQLMathLeastSquareSABRCostFunc();
//
//	virtual AQLMathLeastSquareSABRCostFunc* Clone() const { return new AQLMathLeastSquareSABRCostFunc(*this); }
//
//	// method to overload to compute the cost function value in x
//	virtual QuantLib::Real value(const QuantLib::Array& x) const;
//	//virtual double value(double x) const;
//
//	// method to overload to compute the cost function values in x
//	virtual QuantLib::Array values(const QuantLib::Array& x) const;
//	//virtual double values(double x) const;
//
//	virtual QuantLib::Real finiteDifferenceEpsilon() const { return 1e-8; }
//
//	double Get_alpha() { return alpha; }
//	double Get_beta() { return beta; }
//	double Get_nu() { return nu; }
//	double Get_rho() { return rho; }
//
//protected:
//
//private:
//	size_t n;
//	double expiry;
//	double forward;
//	double numeraire;
//	std::vector<double> strike;
//	std::vector<double> weight;
//	std::vector<int> sgn;
//	std::vector<double> param0;
//	std::vector<bool> flg;
//	std::vector<double> target2fit;
//
//	void set_params(const QuantLib::Array& x) const;
//
//	AQLMathSABR* sabr;
//
//	mutable double alpha;
//	mutable double beta;
//	mutable double nu;
//	mutable double rho;
//	
//	AQLString target;
//
//	bool cloned;
//
//};
//
////  Constraint
//class AQLMathLeastSquareSABRConstraint : public QuantLib::Constraint
//{
//public:
// 	AQLMathLeastSquareSABRConstraint( std::vector<double>& param0,
//								   const std::vector<bool>& flg,
//								   double max_alpha,
//					               double min_alpha,
//					               double max_beta,
//					               double min_beta,
//					               double max_nu,
//					               double min_nu,
//					               double max_rho,
//					               double min_rho
//								   )
//	: QuantLib::Constraint(boost::shared_ptr<Constraint::Impl>( new AQLMathLeastSquareSABRConstraint::Impl(param0,
//																										flg,
//																										max_alpha,
//																										min_alpha,
//																										max_beta,
//																										min_beta,
//																										max_nu,
//																										min_nu,
//																										max_rho,
//																										min_rho
//																										)) ) {}
//
//private:
//    class Impl : public QuantLib::Constraint::Impl
//    {
//    public:
//		Impl(std::vector<double>& param0,
//			 const std::vector<bool>& flg,
//			 double max_alpha,
//			 double min_alpha,
//			 double max_beta,
//			 double min_beta,
//			 double max_nu,
//			 double min_nu,
//			 double max_rho,
//			 double min_rho
//			);
// //       
//		virtual bool test(const QuantLib::Array& p) const;
//
//	protected:
//		void set_params(const QuantLib::Array& x) const;
//
//		std::vector<double> param0;
//		std::vector<bool> flg;
//		double max_alpha;
//		double min_alpha;
//		double max_beta;
//		double min_beta;
//		double max_nu;
//		double min_nu;
//		double max_rho;
//		double min_rho;
//
//		mutable double alpha;
//		mutable double beta;
//		mutable double nu;
//		mutable double rho;
//
//    };
//};
//
////
//class AQLMathLeastSquareSABRConstraint_Antonov : public QuantLib::Constraint
//{
//public:
//	AQLMathLeastSquareSABRConstraint_Antonov( double fwd,
//										   std::vector<double>& param0,
//										   const std::vector<bool>& flg,
//										   double max_alpha,
//										   double min_alpha,
//										   double max_beta,
//										   double min_beta,
//										   double max_nu,
//										   double min_nu,
//										   double max_rho,
//										   double min_rho
//										  )
//	:
//	Constraint(boost::shared_ptr<Constraint::Impl>( new AQLMathLeastSquareSABRConstraint_Antonov::Impl(fwd,
//																									param0,
//																									flg,
//																									max_alpha,
//																									min_alpha,
//																									max_beta,
//																									min_beta,
//																									max_nu,
//																									min_nu,
//																									max_rho,
//																									min_rho
//																									)) ) {}
//
//protected:
//    class Impl : public QuantLib::Constraint::Impl
//    {
//    public:
//        Impl(double fwd,
// 			 std::vector<double>& param0,
//			 const std::vector<bool>& flg,
//			 double max_alpha,
//			 double min_alpha,
//			 double max_beta,
//			 double min_beta,
//			 double max_nu,
//			 double min_nu,
//			 double max_rho,
//			 double min_rho
//			);
//        
//        bool test(const QuantLib::Array& p) const;
//
//    private:
//		void set_params(const QuantLib::Array& x) const;
//
//		double fwd;
//		std::vector<double> param0;
//		std::vector<bool> flg;
//		double max_alpha;
//		double min_alpha;
//		double max_beta;
//		double min_beta;
//		double max_nu;
//		double min_nu;
//		double max_rho;
//		double min_rho;
//
//		mutable double alpha;
//		mutable double beta;
//		mutable double nu;
//		mutable double rho;
//    };
//};

//++++++++ Funahashi ++++++++ // add


class AQLMathSABRLimiter
{
public:
    // constructor
    AQLMathSABRLimiter():paramNum(0) {};

    AQLMathSABRLimiter(AQLStringMatrix paramFlag);
    
    // destructor
    virtual ~AQLMathSABRLimiter(){};

    bool getAlphaFlag(){ return alphaFlag; };

    bool getBetaFlag(){ return betaFlag; };

    bool getNuFlag(){ return nuFlag; };

    bool getRhoFlag(){ return rhoFlag; };

    size_t getParamNum(){ return paramNum; };

    AQLMathSABR_Hagan getSABR(AQLMathSABR_Hagan& sabrOri, const DoubleArray& x, bool isMap);

    void getArgument(AQLMathSABR_Hagan sabr, DoubleArray& x, bool isMap);
   
	bool checkArgument(AQLMathSABR_Hagan sabrOri, const DoubleArray& x);

private:
    bool alphaFlag;
    bool betaFlag;
    bool nuFlag;
    bool rhoFlag;

    size_t paramNum;
};

class AQLMathSABRCalibrator : public AQLFunctionVector
{
public:
    // constructor
    AQLMathSABRCalibrator(const AQLMathSABR_Hagan& sabr_, const DoubleArray& strike_, const DoubleArray& vol_, 
                       const DoubleArray& forward_, const DoubleArray& expiry_, const DoubleArray& numeraire_, 
                       const DoubleArray& weight_, const AQLString& target_, bool isMap_);
    // destructor
    virtual ~AQLMathSABRCalibrator(){}

    virtual unsigned long lengthOfArgumentVector( ) { return sabrLimiter.getParamNum(); }

    virtual unsigned long lengthOfFunctionVector( ) { return targetNum; }

    virtual unsigned long maximumNumberOfIterations() { return 150; }

    virtual void operator()(DoubleArray& f, const DoubleArray& x);

    virtual bool constraintsAreViolated(const DoubleArray& x);

    void setLimiter( const AQLMathSABRLimiter& sabrLimiter_ ) { sabrLimiter = sabrLimiter_; }

private:
    AQLMathSABR_Hagan sabr;
    AQLMathSABRLimiter sabrLimiter;
    size_t targetNum;

    DoubleArray strike;
    DoubleArray vol;
    DoubleArray prem;
    DoubleArray forward;
    DoubleArray expiry;
    DoubleArray numeraire;
    DoubleArray weight;
    
    AQLString target;
    bool isMap;
};

class AQLMathSABRCalibratorATMFix : public AQLFunctionVector
{
public:
    // constructor
    AQLMathSABRCalibratorATMFix(
        const AQLMathSABR_Hagan& sabr_, const DoubleArray& strike_, const DoubleArray& vol_, 
        double atmVol_, double forward_, double expiry_, 
        double numeraire_, const DoubleArray& weight_, const AQLString& target_, bool isMap_);

    // destructor
    virtual ~AQLMathSABRCalibratorATMFix(){}

    virtual unsigned long lengthOfArgumentVector( ) { return sabrLimiter.getParamNum(); }

    virtual unsigned long lengthOfFunctionVector( ) { return targetNum; }

    virtual bool constraintsAreViolated( const DoubleArray& x );

    virtual unsigned long maximumNumberOfIterations() { return 150; }

    virtual void operator()(DoubleArray& f, const DoubleArray& x);

    void setLimiter( const AQLMathSABRLimiter& sabrLimiter_ ) { sabrLimiter = sabrLimiter_; }
private:
    AQLMathSABR_Hagan sabr;
    AQLMathSABRLimiter sabrLimiter;
    size_t targetNum;

    DoubleArray strike;
    DoubleArray vol;
    double atmVol;
    DoubleArray prem;
    double forward;
    double expiry;
    double numeraire;
    DoubleArray weight;
    
    AQLString target;
    bool isMap;
};

#endif
