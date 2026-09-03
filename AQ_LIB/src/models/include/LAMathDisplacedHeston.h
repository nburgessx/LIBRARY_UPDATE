/*! @file
    @brief Class declaration to LAMathDisplacedHeston.

*/
//  2009, AlgoQuantHub.
#ifndef __LAMATHDISPLACEDHESTON_H__
#define __LAMATHDISPLACEDHESTON_H__

#ifdef __GNUG__
#pragma interface
#endif


#include <AQLCoreTemplateType.h>
#include <complex>
#include "LAMathJumpDiffusion.h"
#include "LAMathFXVolatilitySurfaceGenerate.h"

class LAMathDisplacedHeston
{
public:
    static DoubleComplex zeta_DD(DoubleComplex phi, 
                                 double beta_,
				                 double kappa_, 
				                 double epsilon_, 
                                 double rho_
				                 );

    static DoubleComplex psi_DD(DoubleComplex phi, 
				                DoubleComplex zeta, 
                                double beta_,
				                double kappa_, 
                                double epsilon_, 
				                double rho_, 
				                int sgn
                                );

    static DoubleComplex B_DD(DoubleComplex psi_plus, 
				              DoubleComplex psi_minus,
				              DoubleComplex zeta, 
				              DoubleComplex phi, 
				              double tau_,
				              double beta_										
                              );

    static DoubleComplex G_DD(DoubleComplex phi, 
				              double T, 
				              double FX0,
                              double beta_,
				              const HestonParams& heston_params
				              );

    static double F_Integral_DD(double z, 
					            double T, 
					            double FX0, 
					            double K, 
					            double beta_,
					            const HestonParams& heston_params
					            );

    static double Get_F_DD(double T, 
				           double FX0, 
				           double K, 
				           double beta_, 
				           const HestonParams& heston_params,
				           unsigned short GL_Number
				           );

    static double BS_DDHeston(double T,
				              double FX0,
				              double K, 
				              double P0,
				              int sgn,
				              double beta_,
				              const HestonParams& heston_params										  
				              ); 

};
#endif