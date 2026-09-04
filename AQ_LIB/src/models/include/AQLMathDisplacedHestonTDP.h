/*! @file
    @brief Class declaration to AQLMathDisplacedHestonTDP.
*/
#ifndef __AQLMATHDISPLACEDHESTONTDP_H__
#define __AQLMATHDISPLACEDHESTONTDP_H__

#ifdef __GNUG__
#pragma interface
#endif
//#define isQuantLib

#include <AQLCoreTemplateType.h>
#include <AQLDate.h>
#include <complex>
#include "AQLMathFXVolatilitySurfaceGenerate.h"

//Heston Param
struct HestonParams_TDP
{
    AQLDate asOfDate;
	DoubleVector T;
	double V0;
	DoubleVector Kappa;
	DoubleVector Theta;
	DoubleVector Epsilon;
	DoubleVector Rho;
};

class AQLMathDisplacedHestonTDP
{
public:
    static void SetHestonParams_TDP(HestonParams_TDP& params, DoubleMatrix inputdatas);

    static void AdjustHestonParam_TDP(HestonParams_TDP& params, double T);

    static DoubleComplex B_DD_TDP(DoubleComplex B,
					              DoubleComplex psi_plus, 
					              DoubleComplex psi_minus,
					              DoubleComplex zeta, 
					              DoubleComplex phi, 
					              double tau_,
					              double beta_,
                                  double epsilon_											
					              );

    static DoubleComplex A_TDP(DoubleComplex B,
					           DoubleComplex psi_plus, 
					           DoubleComplex psi_minus,
					           DoubleComplex zeta, 
					           double tau_, 
					           double kappa_, 
					           double theta_,
					           double epsilon_								 
                               );

    static DoubleComplex G_DD_TDP(DoubleComplex phi, 
                                  double T,
					              double FX0, 
					              double beta_, 
					              const HestonParams_TDP& heston_params
					              );

    static double F_Integral_DD_TDP(double z, 
                                    double T,
						            double FX0, 
						            double K, 
						            double beta_,
						            const HestonParams_TDP& heston_params
						            );

    static double Get_F_DD_TDP(double T,
                               double FX0, 
					           double K,
					           double beta_, 
					           const HestonParams_TDP& heston_params,
					           unsigned short GL_Number,
					           double width
					           );

    static double BS_DDHeston_TDP(double T,
                                  double FX0,
					              double K, 
					              double P0,
					              int sgn,
					              double beta_,
					              const HestonParams_TDP& heston_params										  
					              );

    static double BS_DDHestonImpVol_TDP(double T,
                                        double FX0,
					                    double K, 
					                    double P0,
					                    int sgn,
					                    double beta_,
					                    const HestonParams_TDP& heston_params										  
					                    );

	static DoubleMatrix CalibrationHelper( const std::vector<FXOptionData >& datas,
										   const DoubleMatrix& strikes,
										   const IntMatrix& sgns,
										   HestonParams_TDP& hestonParam
										   );

    static DoubleMatrix CalibrationHelperGL( const std::vector<FXOptionData >& datas,
											 const DoubleMatrix& strikes,
											 const IntMatrix& sgns,
											 HestonParams_TDP& hestonParam
                                             );
#ifdef isQuantLib
	static void FXCalibrationHestonTDP( const std::vector<FXOptionData >& datas,
										const DoubleMatrix& vols,
										const DoubleMatrix& strikes,
										const IntMatrix& sgns,
										HestonParams_TDP& hestonParam
										);

    static void FXCalibrationHestonTDPV0Fix( const std::vector<FXOptionData >& datas,
										     const DoubleMatrix& vols,
										     const DoubleMatrix& strikes,
										     const IntMatrix& sgns,
										     HestonParams_TDP& hestonParam
										     );

	static void FXCalibrationHestonTDPRhoFix( const std::vector<FXOptionData >& datas,
											  const DoubleMatrix& vols,
											  const DoubleMatrix& strikes,
											  const IntMatrix& sgns,
											  HestonParams_TDP& hestonParam
											  );

    static void FXCalibrationHestonTDPKappaFix( const std::vector<FXOptionData >& datas,
										        const DoubleMatrix& vols,
											    const DoubleMatrix& strikes,
											    const IntMatrix& sgns,
											    HestonParams_TDP& hestonParam
											    );

    static void FXCalibrationHestonTDPThetaFix( const std::vector<FXOptionData >& datas,
												const DoubleMatrix& vols,
												const DoubleMatrix& strikes,
												const IntMatrix& sgns,
												HestonParams_TDP& hestonParam
												);

    static void FXCalibrationHestonTDP_Boot( const std::vector<FXOptionData >& datas,
											 const DoubleMatrix& vols,
											 const DoubleMatrix& strikes,
											 const IntMatrix& sgns,
											 HestonParams_TDP& hestonParam
											 );

    static void FXCalibrationHestonGlobal( const std::vector<FXOptionData >& datas,
                                           const DoubleMatrix& vols,
                                           const DoubleMatrix& strikes,
                                           const IntMatrix& sgns,
                                           HestonParams_TDP& hestonParam
                                           );

    static void FXCalibrationHestonGlobalV0Fix( const std::vector<FXOptionData >& datas,
                                                const DoubleMatrix& vols,
                                                const DoubleMatrix& strikes,
                                                const IntMatrix& sgns,
                                                HestonParams_TDP& hestonParam
                                                );

    static void FXCalibrationHestonGlobalKappaFix( const std::vector<FXOptionData >& datas,
                                                   const DoubleMatrix& vols,
                                                   const DoubleMatrix& strikes,
                                                   const IntMatrix& sgns,
                                                   HestonParams_TDP& hestonParam
                                                   );

    static void FXCalibrationHestonGlobalThetaFix( const std::vector<FXOptionData >& datas,
                                                   const DoubleMatrix& vols,
                                                   const DoubleMatrix& strikes,
                                                   const IntMatrix& sgns,
                                                   HestonParams_TDP& hestonParam
                                                   );

    static void FXCalibrationHestonTDTheta( const std::vector<FXOptionData >& datas,
                                            const DoubleMatrix& vols,
                                            const DoubleMatrix& strikes,
                                            const IntMatrix& sgns,
                                            HestonParams_TDP& hestonParam
                                            );

    static void FXCalibrationHestonTDThetaFix( const std::vector<FXOptionData >& datas,
                                               const DoubleMatrix& vols,
                                               const DoubleMatrix& strikes,
                                               const IntMatrix& sgns,
                                               HestonParams_TDP& hestonParam
                                               );
#endif

    static double GetWingFactorFromHeston( const FXOptionData& x,
                                           const SmileParam& smParams,
                                           double atmDelta,
                                           const HestonParams_TDP hestonParams,
                                           double lower,
                                           double high );

};
#endif