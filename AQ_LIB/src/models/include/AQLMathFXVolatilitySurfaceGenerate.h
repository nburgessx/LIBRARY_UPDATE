/*!
    \file AQLMathFXVolatilitySurfaceGenerate.h
    \brief
    \authorb kikuchei

*/

#ifndef __AQLMathFXVOLATILITYSURFACEGENERATE_H__
#define __AQLMathFXVOLATILITYSURFACEGENERATE_H__

//include
#include <vector>
#include <AQLDataInstance.h>
#include <AQLString.h>
#include <AQLPriceDataCalendar.h>

const bool isErrorMessage = false;
const double EPS_Vol = 0.0000001;
const double EPS_Vol1 = 0.001;

enum FXDeltaType 
{
	FWD_NONPRE,
	FWD_PRE,
	SPOT_NONPRE,
    SPOT_PRE,
};

enum FXATMStrikeType 
{
	FORWARDATM,	
	SPOTATM,		
	DELTANEUTRAL_NONPRE,
    DELTANEUTRAL_PRE
};

enum InterpolationTarget 
{
    TargetLogStrike,
    TargetDeltaPut,
    TargetStrike,
};

enum ATMInterpolationMethod
{
    TermWeighted,
    DailyWeighted,
    SquareDailyWeighted,
    TermNoWeighted
};

enum InterpolationMethod 
{
    SPLINE_FXVOL,
    CONSTRAINEDSPLINE_FXVOL,
    LINEAR_FXVOL
};

enum InterpolationVariable 
{
	VariableDeltaPut,	
	VariableDeltaCall,		
    VariableLogStrike
};

/*!
	    @param[in] T    Optin Maturity
        @param[in] S    Spot
	    @param[in] k    log( Strike / Forward )
        @param[in] F    Forward
        @param[in] K    Strike
	    @param[in] Pd   Domestic Discounted Bond
        @param[in] Pf   Foreign Discounted Bond  
        @param[in] V    Volatility
        @param[in] sgn  {-1,1}

        @return 
*/
struct FXOptionData 
{
    double T;
    double Pd;
    double Pf;
    double F;
    double spotFX;
    FXDeltaType deltaType;
    FXATMStrikeType atmType;
    double Days;
};

struct SmileData
{
    DoubleVector deltaPuts;
    DoubleVector logStrikes;
    DoubleVector strikes;
    DoubleVector vols;
};

struct SmileParam
{
    double atmVol;
    double highRR;
    double highBF;
    double lowRR;
    double lowBF;
};

struct Strangle
{
    double vol;
    double strikePut;
    double strikeCall;
    double prem;
};

class AQLMathFXVolatilitySurfaceGenerate 
{
public:
    static void SetInterpolationTarget( const AQLString& target, InterpolationTarget& target_ );

    static void SetATMInterpolationMethod( const AQLString& target, ATMInterpolationMethod& target_ );

    static void SetInterpolationMethod( const AQLString& target, InterpolationMethod& target_ );    

    static void SetInterpolationVariable( const AQLString& variable, InterpolationVariable& variable_ );

    static void SetFXOptionParam( AQLStringVector& str, FXOptionData& x );

    static FXOptionData SetFXOptionParam(AQLDataInstance& dataInstance,
                                         const AQLString& dCurveID,
                                         const AQLString& fCurveID,
                                         const AQLDate& maturityDate,
                                          const AQLDate& deliveryDate,
                                         double spotFX,
                                         const AQLString& deltaType,
                                         const AQLString& atmType,
                                          const AQLPriceDataCalendar& calendar );

    /*static FXOptionData SetFXOptionParam( AQLDataInstance& dataInstance,
                                          const AQLString& dCurveID,
                                          const AQLString& fCurveID,
                                          double termPoint,
                                          double spotFX,
                                          ATMInterpolationMethod method,
                                          const AQLString& spotLag,
                                          const AQLString& calendar );*/

    static void SmileDataCheck( SmileData& x, bool isWing );

	static bool SmileDataCheck_Bool( const SmileParam& y,        
									 const FXOptionData& x,
                                     double wf );

    static void SetSmileParam( AQLStringVector& str, SmileParam& x );

    static double GetATMStrike( double V, const FXOptionData& x );                      

    static double GetFXOptionPremium( double K,
                                      double V,
                                      int sgn,
                                      const FXOptionData& x );

    static double GetFXOptionPremium( double T,
                                      double F,
                                      double Pd,
                                      double K,
                                      double V,
                                      int sgn );

    static void GetWingSmile( DoubleVector& impliedVol, 
                              double atmDelta,
                              double wf );

    static double GetMinimumCallDeltaLogStrike( double V, const FXOptionData& x );   

    static double GetFXOptionDelta( double k,
                                    double V,
                                    int sgn,
                                    const FXOptionData& x );

    static double FindLogStrikeFromDelta( double delta, 
                                          double V,
                                          int sgn,
                                          const FXOptionData& x );

    static double Interpolate( DoubleArray& array1,
                               DoubleArray& array2,
                               double point,
                               const InterpolationMethod& method,
                               bool isReverse = false );

    static SmileData BuildSmile( const SmileParam& y,
                                 const FXOptionData& x,
								 bool isWing,
                                 double wf,
                                 AQLString& warningMSG );

    static double GetSmileInterpolation( double point,
                                         const InterpolationMethod& method,
                                         const InterpolationTarget& target,
                                         const InterpolationVariable& variable,
                                         const FXOptionData& x,
                                         SmileData& y,
                                         bool isWing );

    static double GetModelPrem( const FXOptionData& x,
                                SmileData& y,
                                const Strangle& z,
                                const InterpolationMethod& method,
                                const InterpolationTarget& target,
                                bool isWing );

    static Strangle GetHighStrangle( double atmVol,
                                     double BF,
                                     const FXOptionData& x );

    static Strangle GetLowStrangle( double atmVol,
                                    double BF,
                                    const FXOptionData& x );

    static SmileData FindStrangleVol( const FXOptionData& x,
                                      const SmileParam& y,
                                      const InterpolationMethod& method,
                                      const InterpolationTarget& target,
                                      bool isWing,
                                      double wf );

    static double GetSurfaceInterpolation( double point,
                                           double termPoint,
                                           const InterpolationMethod& method,
                                           const InterpolationTarget& target,
                                           const InterpolationVariable& variable,
                                           const ATMInterpolationMethod& atmMethod,
                                           const std::vector<FXOptionData >& x,
                                           std::vector<SmileData >& y,
                                           bool isWing );

    static double GetATMVolatility( double termPoint,
                                    const ATMInterpolationMethod& atmMethod,
                                    const std::vector<FXOptionData >& x,
                                    std::vector<SmileData >& y );

	//hishida vanna volga
	static double GetVolatilityFromMaturityInterp( double termPoint,
                                                    const ATMInterpolationMethod& atmMethod,
													const std::vector<FXOptionData >& x, 
													const std::vector<SmileData >& y,
													int volpos);

    static double FindLowButterfly( const InterpolationMethod& method,
									const InterpolationTarget& target, 
									const FXOptionData& x,
									SmileData& y,
                                    bool isWing );

	static double FindHighButterfly( const InterpolationMethod& method,
						 			 const InterpolationTarget& target, 
						 			 const FXOptionData& x,
									 SmileData& y,
                                     bool isWing );
private:
	AQLMathFXVolatilitySurfaceGenerate(void);
	~AQLMathFXVolatilitySurfaceGenerate(void);
	AQLMathFXVolatilitySurfaceGenerate(const AQLMathFXVolatilitySurfaceGenerate &rhs);
	AQLMathFXVolatilitySurfaceGenerate &operator=(const AQLMathFXVolatilitySurfaceGenerate &rhs);
};
#endif