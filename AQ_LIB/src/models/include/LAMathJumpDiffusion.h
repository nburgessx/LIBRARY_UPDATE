/*! @file
    @brief Class declaration to LAMathJumpDiffusion.

*/
//  2009, AlgoQuantHub.
#ifndef __LAMATHJUMPDIFFUSION_H__
#define __LAMATHJUMPDIFFUSION_H__


#include <AQLCoreTemplateType.h>

//Heston Param
struct HestonParams
{
	DoubleVector T_;
	double V0_;
	double Kappa_;
	DoubleVector Theta_;
	double Epsilon_;
	double Rho_;
};

//Intensity Param
struct IntensityParams
{
	double Lambda0_;
	double Kappa_lambda_;
	double Theta_lambda_;
};

//Jump Param
enum TypeOfJump{LN, LE};

/*!
	Log-Normal Jump-Diffusions Parameter
*/
struct LNParams
{
	double Nu_;
	double Delta_;
};

/*!
	Double-Exponential Jump-Diffusions Parameter
*/
struct LEParams
{
	double Nu_up_;
	double Nu_down_;
	double P_;
};

class LAMathJumpDiffusion
{
public:
    static void SetHestonParams(HestonParams& params, const DoubleMatrix& inputdatas);

    static void SetHestonParams(HestonParams& params, const DoubleVector& inputdatas, double term );

    static void AdjustHestonParam(HestonParams& params, double T);


    static void SetIntensityParams(IntensityParams& params, DoubleVector inputdatas);


    static void SetLNParams(LNParams& params, DoubleVector inputdatas);

    static DoubleComplex MeanJump_LN(DoubleComplex phi, LNParams ln_params);



    static void SetLEParams(LEParams& params, DoubleVector inputdatas);

    static DoubleComplex MeanJump_LE(DoubleComplex phi, LEParams le_params);

    static DoubleComplex Lambda_LN(DoubleComplex phi, LNParams ln_params);

    static DoubleComplex Lambda_LE(DoubleComplex phi, LEParams le_params);

    static DoubleComplex zeta(DoubleComplex phi, 
				    double kappa_, 
				    double epsilon_, 
                    double rho_
				    );

    //if phi_plus,sgn_=1. if phi_minus,sgn_=-1
    static DoubleComplex psi(DoubleComplex phi, 
				             DoubleComplex zeta, 
				             double kappa_, 
                             double epsilon_, 
				             double rho_, 
				             int sgn
				             );

    static DoubleComplex D(DoubleComplex Lambda, 
				           double tau_, 
				           double kappa_lambda_
                           );

    static DoubleComplex C(DoubleComplex Lambda, 
				           double tau_, 
				           double kappa_lambda_,
				           double theta_lambda_
				           );

    static DoubleComplex B(DoubleComplex psi_plus, 
				           DoubleComplex psi_minus,
                           DoubleComplex zeta, 
				           DoubleComplex phi, 
				           double tau_
				           );

    static DoubleComplex A__(DoubleComplex psi_plus, 
				             DoubleComplex psi_minus,
                             DoubleComplex zeta, 
				             double tau_, 
				             double kappa_, 
				             double theta_, 
				             double epsilon_
				             );

    static DoubleComplex A_(DoubleComplex psi_plus, 
				            DoubleComplex psi_minus,
				            DoubleComplex zeta, 
				            double tau_, 
				            double tau1_, 
				            double kappa_, 
				            double theta_,
				            double epsilon_
				            );

    static DoubleComplex A(DoubleComplex psi_plus, 
				           DoubleComplex psi_minus, 
                           DoubleComplex zeta, 
				           DoubleVector tau_, 
				           double kappa_, 
                           DoubleVector theta_, 
				           double epsilon_
				           );

    static DoubleComplex G(DoubleComplex phi, 
				           double T, 
				           double FX0, 
				           const TypeOfJump type_of_jump, 
				           const HestonParams& heston_params,
				           const IntensityParams& intensity_params,
				           DoubleVector jump_params
				           );

    static double F_Integral(double z, 
				             double T, 
				             double FX0, 
				             double K, 
                             const TypeOfJump& type_of_jump, 
				             const HestonParams& heston_params,	
				             const IntensityParams& intensity_arams,
				             DoubleVector jump_params
				             );

    static double Get_F(double T, 
			            double FX0, 
			            double K,
			            int sgn,
			            const TypeOfJump& type_of_jump, 
			            const HestonParams& heston_params, 							 
			            const IntensityParams& intensity_arams,
			            DoubleVector jump_params,
			            unsigned short GL_Number,
			            double width
			            );

    static double BS_Heston_Jump(double T,
					             double FX0,
					             double K, 
					             double P0,
					             int sgn,
					             const TypeOfJump& type_of_jump,
					             const HestonParams& heston_params,
					             const IntensityParams& intensity_arams,
					             DoubleVector jump_params
					             );
};
#endif