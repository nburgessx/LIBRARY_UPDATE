// BondTypes.h

/* 
 * @brief			Collection of utility methods for Bond Struct Management
 */

#pragma once

#include "AQLCoreTemplateType.h"
#include "BondCurves.h"                 // For Bond Curve Fitting - Struct Definition: NelsonSiegelSvenssonParameters
#include "NelsonSiegelFitting.h"        // For Bond Curve Fitting - Struct Definition: NelsonSiegelSvenssonCalibrationResults
#include "PolynomialFitting.h"          // For Bond Curve Fitting - Struct Definition: PolynomialCalibrationResults

class AQLString;

namespace swig
{
	

    /* @brief	Struct to consolidate & contain Nelson-Siegel Bond Curve Fitting Parameters
    *
	*  @param [in]		beta0	    Long term yield
    *  @param [in]		beta1	    Slope
    *  @param [in]		beta2	    Curvature
    *  @param [in]		lambda1	    Time decay
    *  @param [in]		beta3	    Secondary Curvature for Svensson Method only
    *  @param [in]		lambda2	    Time decay for Svensson Method only
    *
    *  @return		A Parameter Struct Container
    */ 
	struct NelsonSiegelSvenssonParameters
	{
		double beta0_;	    // Long term yield
		double beta1_;	    // Slope
		double beta2_;	    // Curvature
		double lambda1_;	// Time decay
        double beta3_;	    // Secondary Curvature for Svensson Method only
		double lambda2_;    // Secondary Time decay for Svensson Method only

        // Constructors - Needed for Server Users to Create the Bond Curve Fitting Function Inputs
        NelsonSiegelSvenssonParameters() {};
        NelsonSiegelSvenssonParameters( const double& beta0,
                                        const double& beta1,
                                        const double& beta2,
                                        const double& beta3,
                                        const double& lambda1,
                                        const double& lambda2 )
        {
            beta0_      = beta0;
            beta1_      = beta1;
            beta2_      = beta2;
            beta3_      = beta3;
            lambda1_    = lambda1;
            lambda2_    = lambda2;
        };

	};



    /* @brief	Struct to consolidate & contain Nelson-Siegel Bond Curve Calibration Results
    *
	*  @param [in]		modelParameters	        Nelson-Siegel Model Parameters
    *  @param [in]		leastSquaresError	    The Residual Least Squares Error
    *  @param [in]		iterations	            Number of Interations
    *
    *  @return		A Calibration Results Struct Container
    */
	struct NelsonSiegelSvenssonCalibrationResults
	{
		NelsonSiegelSvenssonParameters parameters_;	    // The calibrated model parameters
		double leastSquaresError_;		                // Minimum error achieved by the minimizer corresponding to the final calibration parameters
		double iterations_;				                // Number of iterations the minimizer used
	};



    /* @brief	Struct to consolidate & contain Polynomial Bond Curve Calibration Results
    *
	*  @param [in]		coefficients	        Polynomial Coefficients
    *  @param [in]		leastSquaresError	    The Residual Least Squares Error
    *  @param [in]		iterations	            Number of Interations
    *
    *  @return		A Calibration Results Struct Container
    */
	struct PolynomialCalibrationResults
	{
		DoubleVector coefficients_;	            // The calibrated model parameters
		double leastSquaresError_;	            // Minimum error achieved by the minimizer corresponding to the final calibration parameters
		double iterations_;			            // Number of iterations the minimizer used
	};

}

