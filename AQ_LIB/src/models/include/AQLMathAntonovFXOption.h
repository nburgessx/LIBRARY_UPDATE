/*! @file
    @brief Class declaration to AQLMathAntonovFXOption model.

*/

#ifndef PRICING_DATA_ENDPOSITION
#define PRICING_DATA_ENDPOSITION					"EndPosition"			//  data name of EndPosition
#endif
#ifndef PRICING_DATA_STRIKES
#define PRICING_DATA_STRIKES						"Strikes"				//  data name of Strikes
#endif
#ifndef PRICING_DATA_OPTIONPREMIUMS
#define PRICING_DATA_OPTIONPREMIUMS					"OptionPremiums"		//  data name of OptionPremiums
#endif
#ifndef PRICING_DATA_WEIGHTS
#define PRICING_DATA_WEIGHTS						"Weights"				//  data name of Weights
#endif
#ifndef PRICING_DATA_ISCALLS
#define PRICING_DATA_ISCALLS						"IsCalls"				//  data name of IsCalls
#endif
#ifndef PRICING_DATA_VOLATILITYS
#define PRICING_DATA_VOLATILITYS					"Volatilitys"			//  data name of Volatilitys
#endif
#ifndef PRICING_DATA_ISFITATM
#define PRICING_DATA_ISFITATM						"IsFitATM"				//  data name of IsFitATM
#endif
#ifndef PRICING_DATA_ISMAKECALIBDATAFROMVOLATILITY
#define PRICING_DATA_ISMAKECALIBDATAFROMVOLATILITY	"IsMakeCalibDataFromVolatility"	//  data name of IsMakeCalibDataFromVolatility
#endif
#ifndef PRICING_DATA_ISDELTANEUTRAL
#define PRICING_DATA_ISDELTANEUTRAL					"IsDeltaNeutral"		//  data name of IsDeltaNeutral
#endif
#ifndef PRICING_DATA_TERMBETA
#define PRICING_DATA_TERMBETA						"TermBeta"				//  data name of TermBeta
#endif
#ifndef PRICING_DATA_STRIKESTRINGS
#define PRICING_DATA_STRIKESTRINGS					"StrikeStrings"			//  data name of StrikeStrings
#endif
#ifndef PRICING_DATA_SDECORRELATIONS
#define PRICING_DATA_SDECORRELATIONS				"SDECorrelations"		//  data name of ForeignIRModel
#endif
#ifndef PRICING_DATA_SDEVOLCORRELATIONS
#define PRICING_DATA_SDEVOLCORRELATIONS				"SDEVolCorrelations"		//  data name of SDECorrelations for Vol vs FX
#endif
#ifndef PRICING_DATA_FXTIMEGRIDS
#define PRICING_DATA_FXTIMEGRIDS					"FxTimeGrids"			//  data name of StrikeStrings
#endif
#ifndef PRICING_DATA_FXFORWARDGRIDS
#define PRICING_DATA_FXFORWARDGRIDS					"FxForwardGrids"		//  data name of FxForwardGrids
#endif
#ifndef PRICING_DATA_FXBETAGRIDS
#define PRICING_DATA_FXBETAGRIDS					"FxBetaGrids"			//  data name of FxBetaGrids
#endif
#ifndef PRICING_DATA_FXVOLGRIDS
#define PRICING_DATA_FXVOLGRIDS						"FxVolGrids"			//  data name of FxVolGrids
#endif
#ifndef PRICING_DATA_SPOTORFWD
#define PRICING_DATA_SPOTORFWD						"SpotOrFwd"				//  data name of SpotOrFwd
#endif
#ifndef PRICING_DATA_DOMESTICDFS
#define PRICING_DATA_DOMESTICDFS					"DomesticDFs"			//  data name of DomesticDFs
#endif
#ifndef PRICING_DATA_FOREIGNDFS
#define PRICING_DATA_FOREIGNDFS						"ForeignDFs"			//  data name of ForeignDFs
#endif
#ifndef PRICING_DATA_IRDOMESTICBONDVOLATILITY
#define PRICING_DATA_IRDOMESTICBONDVOLATILITY		"IRDomesticBondVolatility"		//  data name of IRDomesticBondVolatility
#endif
#ifndef PRICING_DATA_IRFOREIGNBONDVOLATILITY
#define PRICING_DATA_IRFOREIGNBONDVOLATILITY		"IRForeignBondVolatility"		//  data name of IRForeignBondVolatility
#endif
#ifndef PRICING_DATA_BMATRIX
#define PRICING_DATA_BMATRIX						"bMatrix"				//  data name of bMatrix
#endif
#ifndef PRICING_DATA_AMATRIX
#define PRICING_DATA_AMATRIX						"aMatrix"				//  data name of aMatrix
#endif
#ifndef PRICING_DATA_FXINITIALVOL
#define PRICING_DATA_FXINITIALVOL					"InitialVol"			//  data name of InitialVol
#endif
#ifndef PRICING_DATA_FXINITIALSKEW
#define PRICING_DATA_FXINITIALSKEW					"InitialSkew"			//  data name of InitialVol
#endif
#ifndef PRICING_DATA_DOMESTICDF
#define PRICING_DATA_DOMESTICDF					    "DomesticDF"			//  data name of DomesticDF
#endif
#ifndef PRICING_DATA_FOREIGNDF
#define PRICING_DATA_FOREIGNDF						"ForeignDF"			    //  data name of ForeignDF
#endif
#ifndef PRICING_DATA_FXTERM
#define PRICING_DATA_FXTERM					        "FXTerm"			    //  data name of FXTerm
#endif
#ifndef PRICING_DATA_FORWARDFX
#define PRICING_DATA_FORWARDFX						"ForwardFX"			    //  data name of ForwardFX
#endif

//furuya
#ifndef PRICING_DATA_FXTHETAGRIDS
#define PRICING_DATA_FXTHETAGRIDS					"FxThetaGrids"			//  data name of FxThetaGrids
#endif
#ifndef PRICING_DATA_FXKAPPAGRIDS
#define PRICING_DATA_FXKAPPAGRIDS					"FxKappaGrids"			//  data name of FxKappaGrids
#endif
#ifndef PRICING_DATA_FXEPSILONGRIDS
#define PRICING_DATA_FXEPSILONGRIDS					"FxEpsilonGrids"			//  data name of FxEpsilonGrids
#endif
#ifndef PRICING_DATA_BOUNDARY_RANGE
#define PRICING_DATA_BOUNDARY_RANGE					"Boundary_Range"			//  data name of Boundary_Range
#endif
#ifndef PRICING_DATA_INITIALRHO
#define PRICING_DATA_INITIALRHO						"Initial_Rho"			//  data name of InitialRho
#endif
#ifndef PRICING_DATA_CALIBRATIONFLAG_SZ
#define PRICING_DATA_CALIBRATIONFLAG_SZ				"CalibrationFlag_SZ"	//  data name of CalibrationFlag_SZ
#endif

#ifndef PRICING_DATA_INITIALBETA
#define PRICING_DATA_INITIALBETA				"Initialg_Beta_SZ"	//  data name of Initial Beta for SZ
#endif
#ifndef PRICING_DATA_INITIALTHETA
#define PRICING_DATA_INITIALTHETA				"Initialg_Theta_SZ"	//  data name of Initial Theta for SZ
#endif
#ifndef PRICING_DATA_INITIALKAPPA
#define PRICING_DATA_INITIALKAPPA				"Initialg_Kappa_SZ"	//  data name of Initial Kappa for SZ
#endif
#ifndef PRICING_DATA_INITIALEPSILON
#define PRICING_DATA_INITIALEPSILON				"Initialg_Epsilon_SZ"	//  data name of Initial Epsilon for SZ
#endif

#ifndef AQLMathAntonovFXOption_h
#define AQLMathAntonovFXOption_h

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreValuation.h"
#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLBasic.h"
#include "AQLDist.h"
#include "AQLDataValuation.h"
#include "AQLFindRootBrent.h"
#include "AQL1DDataSet.h"
#include "AQLGaussLegendre.h"
#include "AQLPriceHWCalibration.h"

#include "AQLAnalyticFormula.h"
#include "AQLBlackScholesCalc.h"


// Funciton ID of AQLShiftMethod
#define FN_ANTONOVFXOPTIOIN	10043
// Function Name of AQLShiftMethod
#define FN_ANTONOVFXOPTIOIN_STR	"fn_antonovfxoption"


class AQLObject;
class AQLRatesPathElementCurve;
class AQLPriceDataManager;


class AQLMathAntonovFXOption : public AQLCoreValuation
{
public:
	// Default constructor
	AQLMathAntonovFXOption();
	// Destructor
	~AQLMathAntonovFXOption();
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%

	// register dataValues that this class uses
	virtual void				registerData(AQLPriceDataManager& dm) const;

	virtual double              value(const AQLDate& basedate, AQLObject& inst, const AQLDataValuation& att) const;

	/*!
		@brief cache class for performance up
	*/
	class AQLMathAntonovFXOptionDataProvider : public AQLDataProvider
	{
	public:
		AQLMathAntonovFXOptionDataProvider();
		
		virtual ~AQLMathAntonovFXOptionDataProvider(); 
		
		int mSmileNum;
		unsigned int mSpos;					// swaptionstatpos
		unsigned int mEpos;					// swaptionendpos
		double mTm;							// OptionMaturity
		double mFxTm;						// FxForward
		double mPTm;						// DiscountBond Value
		double mBetaTm;						// Beta Value
		double mVolTm;						// Volatility Value
		DoubleVector mFxGrids;				// FxForward Vector
		mutable DoubleVector mBetaGrids;	// Beta Vector
		mutable DoubleVector mVolGrids;		// Volatility Vector
		DoubleVector mTimeGrids;			// TimeGrid Vector
		DoubleMatrix mPdvol;				// Domestic Bond Volatility Matrix
		DoubleMatrix mPfvol;				// Foreign Bond Volatility Matrix
		DoubleMatrix mamat;					// a Matrix
		DoubleMatrix mbmat;					// b Matrix
		double mCovdf;						// rho(r_d,r_f)
		double mCovdfx;						// rho(r_d,fx)
		double mCovffx;						// rho(r_f,fx)
		BoolVector mIsCallVec;				// Call Flag Vector 
		DoubleVector mStrikeVec;			// Strike Vector
		DoubleVector mPremVec;				// Premium Vector
		DoubleVector mWeightVec;			// Weight Vector
		bool mIsfitATM;						// Fit ATM or not
		mutable DoubleVector mVGrids;		// Beta*Vol Vector
		mutable DoubleVector mSGrids;		// (1-Beta)/Beta*Vol Vector
		mutable DoubleVector mAGrids;		// A Vector
		mutable DoubleVector mBGrids;		// B Vector
		mutable DoubleVector mCGrids;		// C Vector
		mutable DoubleVector maGrids;		// a Vector
		mutable DoubleVector mbGrids;		// b Vector
		mutable DoubleVector mSigmaGrids;	// SigmaStar Vector
		mutable DoubleVector mSigmaFXGrids;	// SigmaFx Vector
		mutable DoubleVector mSigmaSt2Grids;// SigmaStar^2 Vector
		mutable AQL1DDataSet mSigmaSt2Data;			// SigmaStar^2 DataSet
		mutable DoubleVector mSigmaStSigmaFXGrids;	// SigmaStSigmaFX Vector
		mutable AQL1DDataSet mSigmaStSigmaFXData;	// SigmaStSigmaFX DataSet
		mutable DoubleVector mIntSigmaSt2Grids;		// Integral of SigmaStar^2 Vector
		mutable DoubleVector mIntSigmaStSigmaFXGrids;	// Integral of SigmaStar* SigmaFx Vector
		mutable DoubleVector mIntSimgaSt2BetaStIntSigmaSt2; // Integral of SimgaSt2BetaStIntSigmaSt2
		mutable DoubleVector mIntSimgaSt2IntSigmaSt2;		 // Integral of SimgaSt2IntSigmaSt2
		mutable DoubleVector mSmallFactorGrids; // SmallFactor Vector
		mutable DoubleVector mbSigmaFXGrids;	// bSigmaFX Vector
		//modify antonov
		mutable DoubleVector mR;
		mutable DoubleVector mrFF;
		mutable DoubleVector mrZZ;
		mutable DoubleVector mr;
		//modify antonov
		mutable DoubleVector mBetaSt;	// BetaSt Vector
		mutable bool mUpdateABCflag;	// UpdateABCflag 
		mutable bool mUpdateIntegral1flag;	// UpdateIntegral1flag
		double getA(unsigned int pos) const;
		double getB(unsigned int pos) const;
		double getC(unsigned int pos) const;
		void calcABC(void) const;
		void calcIntegral1(void) const;
		void reset(double varV, double VarB,unsigned int spos,unsigned int epos) const;
	};

		// set up dataProvider
	AQLDataProvider*					setUpDataProvider(const AQLDate& basedate, AQLObject& object, 
											const AQLDataValuation& att) const;
	// get volatility result
	DoubleVector				getVolatilityResult(const AQLDataValuation& att) const;
	// get beta result
	DoubleVector				getBetaResult(const AQLDataValuation& att) const;
	// get next position
	unsigned int				getNextPos(const AQLDataValuation& att) const;
	// get time maturity
	double						getTM(const AQLDataValuation& att) const;
	// get fx forward
	double						getFxTM(const AQLDataValuation& att) const;
	// get bond value
	double						getPTM(const AQLDataValuation& att) const;
	// get volatility for black
	double						getVTM(double T, const AQLDataValuation& att) const;
	// get volatility for black
	double						getVTM(const AQLDataValuation& att) const;
	// get beta for black
	double						getBetaTM(double T, const AQLDataValuation& att) const;
	// get beta for black
	double						getBetaTM(const AQLDataValuation& att) const;
	// get call value
	double DisplayDiffCall(double beta,double V,double strike,double Fx,double P)  const 
	 {
		 double f = strike* beta+ Fx*(1.0-beta);
		 if(f<=0.0)
			 return 0.0;

		 double sqrtV = (beta > 0.0) ? AQLMath::sqrt(V) : -AQLMath::sqrt(V);
		 double d1 = ( AQLMath::log(Fx/f) + 0.5*V)/sqrtV;
		 double d2 = d1 - sqrtV;

		 return P*(Fx/beta*AQLDist::normsdist(d1)-AQLDist::normsdist(d2)*(strike+Fx*(1.0-beta)/beta));
	 };

	// get put  value
	double DisplayDiffPut(double beta, double V, double strike, double Fx, double P) const
	 {
		  double f = strike* beta+ Fx*(1.0-beta);
		 if(f<=0.0)
			 return 0.0;

		 double sqrtV = (beta > 0.0) ? AQLMath::sqrt(V) : -AQLMath::sqrt(V);
		 double d1 = ( AQLMath::log(Fx/f) + 0.5*V)/sqrtV;
		 double d2 = d1 - sqrtV;

		 return P*( -Fx/beta*AQLDist::normsdist(-d1)+AQLDist::normsdist(-d2)*(strike+Fx*(1.0-beta)/beta));
		
	 }

 protected:
	 // Copy constructor
	AQLMathAntonovFXOption(const AQLMathAntonovFXOption& v);
	// virtual method
	 virtual void			setCalibParamFirst(AQLMathAntonovFXOptionDataProvider* dataProvider, double vol, double beta) const;
	// virtual method
	 virtual void			setCalibParam(AQLMathAntonovFXOptionDataProvider* dataProvider, double vol, double beta) const;

private:
	// create new cache class
	virtual	AQLDataProvider*			createNewDataProvider() const;

};
#endif

