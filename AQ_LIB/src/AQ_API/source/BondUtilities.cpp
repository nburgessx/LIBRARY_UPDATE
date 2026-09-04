// BondTypes.cpp

/* 
 * @brief			Collection of utility methods to do with data types
 */

#include "BondTypes.h"


namespace swig
{
	//
    // Swig utility methods for the Bond Type Methods

	swig::NelsonSiegelSvenssonParameters toSwigNelsonSiegelSvenssonParameters( const etrading::NelsonSiegelSvenssonParameters& p )
    {
        swig::NelsonSiegelSvenssonParameters nssp;
        nssp.beta0_     = p.beta0_;
        nssp.beta1_     = p.beta1_;
        nssp.beta2_     = p.beta2_;
        nssp.beta3_     = p.beta3_;
        nssp.lambda1_   = p.lambda1_;
        nssp.lambda2_   = p.lambda2_;
        return nssp;
    }

    etrading::NelsonSiegelSvenssonParameters fromSwigNelsonSiegelSvenssonParameters( const swig::NelsonSiegelSvenssonParameters& p )
    {
        etrading::NelsonSiegelSvenssonParameters nssp;
        nssp.beta0_     = p.beta0_;
        nssp.beta1_     = p.beta1_;
        nssp.beta2_     = p.beta2_;
        nssp.beta3_     = p.beta3_;
        nssp.lambda1_   = p.lambda1_;
        nssp.lambda2_   = p.lambda2_;
        return nssp;
    }

    swig::NelsonSiegelSvenssonCalibrationResults toSwigNelsonSiegelSvenssonCalibrationResults( const etrading::NelsonSiegelSvenssonCalibrationResults& r )
    {
        swig::NelsonSiegelSvenssonCalibrationResults nssr;
        nssr.parameters_            = toSwigNelsonSiegelSvenssonParameters( r.parameters_ );
        nssr.leastSquaresError_     = r.leastSquaresError_;
        nssr.iterations_            = r.iterations_;
        return nssr;
    }
    
    etrading::NelsonSiegelSvenssonCalibrationResults fromSwigNelsonSiegelSvenssonCalibrationResults( const swig::NelsonSiegelSvenssonCalibrationResults& r )
    {
        etrading::NelsonSiegelSvenssonCalibrationResults nssr;
        nssr.parameters_            = fromSwigNelsonSiegelSvenssonParameters( r.parameters_ );
        nssr.leastSquaresError_     = r.leastSquaresError_;
        nssr.iterations_            = r.iterations_;
        return nssr;
    }
   
    swig::PolynomialCalibrationResults toSwigPolynomialCalibrationResults( const etrading::PolynomialCalibrationResults& r )
    {
        swig::PolynomialCalibrationResults pr;
        pr.coefficients_            = r.coefficients_;
        pr.leastSquaresError_       = r.iterations_;
        pr.iterations_              = r.iterations_;
        return pr;
    }
    
    etrading::PolynomialCalibrationResults fromSwigPolynomialCalibrationResults( const swig::PolynomialCalibrationResults& r )
    {
        etrading::PolynomialCalibrationResults pr;
        pr.coefficients_            = r.coefficients_;
        pr.leastSquaresError_       = r.iterations_;
        pr.iterations_              = r.iterations_;
        return pr;
    }

}
