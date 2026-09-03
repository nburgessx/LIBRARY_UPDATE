#ifndef PRICING_DATA_CURVEID
#define PRICING_DATA_CURVEID					"CurveID"				//  data name of CurveID
#endif
#ifndef PRICING_DATA_SDEINTEGRALGRID
#define PRICING_DATA_SDEINTEGRALGRID			"IntegralGrid"			//  data name of IntegralGrid
#endif
#ifndef PRICING_DATA_CALIBCANONICAL_T
#define PRICING_DATA_CALIBCANONICAL_T			"CalibCanonical_T"		//  data name of CalibCanonical_T
#endif
#ifndef PRICING_DATA_CALIBVOL_T
#define PRICING_DATA_CALIBVOL_T					"CalibVol_T"			//  data name of CalibVol_T
#endif
#ifndef PRICING_DATA_CALIBMEANREV_T
#define PRICING_DATA_CALIBMEANREV_T				"CalibMeanRev_T"		//  data name of CalibMeanRev_T
#endif
#ifndef PRICING_DATA_CALIBVARIABLES
#define PRICING_DATA_CALIBVARIABLES				"CalibVariables"		//  data name of CalibVariable
#endif
#ifndef IR_CALIBRATION_DATA_OPTIONMATURITY
#define IR_CALIBRATION_DATA_OPTIONMATURITY				"OptionMaturity"		//  data name of OptionMaturity
#endif
#ifndef IR_CALIBRATION_DATA_SWAPTENOR
#define IR_CALIBRATION_DATA_SWAPTENOR						"SwapTenor"				//  data name of SwapTenor
#endif
#ifndef PRICING_DATA_OPTIONTYPE
#define PRICING_DATA_OPTIONTYPE					"OptionType"			//  data name of OptionType
#endif
#ifndef PRICING_DATA_MODELPARAM
#define PRICING_DATA_MODELPARAM					"ModelParam"			//  data name of ModelParam
#endif
#ifndef IR_CALIBRATION_DATA_PAYMENTCALENDAR
#define IR_CALIBRATION_DATA_PAYMENTCALENDAR				"PaymentCalendar"		//  data name of PaymentCalendar
#endif
#ifndef IR_CALIBRATION_DATA_BLACKVOLATILITY
#define IR_CALIBRATION_DATA_BLACKVOLATILITY				"BlackVolatility"		//  data name of BlackVolatility
#endif
#ifndef PRICING_DATA_STRIKE
#define PRICING_DATA_STRIKE						"Strike"				//  data name of Strike
#endif
#ifndef PRICING_DATA_OPTIONPREMIUM
#define PRICING_DATA_OPTIONPREMIUM				"OptionPremium"			//  data name of OptionPremium
#endif
#ifndef PRICING_DATA_WEIGHT
#define PRICING_DATA_WEIGHT						"Weight"				//  data name of Weight
#endif
#ifndef IR_MODEL_DATA_FREQUENCY
#define IR_MODEL_DATA_FREQUENCY					"Frequency"				//  data name of Frequency
#endif
#ifndef IR_MODEL_DATA_DAYCOUNT
#define IR_MODEL_DATA_DAYCOUNT					"DayCount"				//  data name of DayCount
#endif
#ifndef PRICING_DATA_FIXINGCALENDAR
#define PRICING_DATA_FIXINGCALENDAR				"FixingCalendar"		//  data name of calendar for fixing date
#endif
#ifndef PRICING_DATA_TERMCALENDAR
#define PRICING_DATA_TERMCALENDAR				"TermCalendar"			//  data name of calendar for settle / expiration date
#endif
#ifndef PRICING_CALIBRATION_DATAOTICEPERIOD
#define PRICING_CALIBRATION_DATAOTICEPERIOD				"NoticePeriod"			//  data name of NoticePeriod
#endif
#ifndef PRICING_DATA_INITIALRSTAR
#define PRICING_DATA_INITIALRSTAR			"InitialRStar"				//  data name of InitialRStar
#endif
#ifndef PRICING_DATA_STARTPOSITION
#define PRICING_DATA_STARTPOSITION			"StartPosition"				//  data name of StartPosition
#endif
#ifndef PRICING_DATA_ISTIMEDEPENDMEANREV
#define PRICING_DATA_ISTIMEDEPENDMEANREV	"IsTimeDependMeanRev"		//  data name of IsTimeDependMeanRev
#endif

#ifndef LAMathJamshidianSwaption_h
#define LAMathJamshidianSwaption_h

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreValuation.h"
#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LABasic.h"
#include "LADist.h"
#include "LADataValuation.h"
#include "LAFindRootBrent.h"
#include "LAMathJamshidianRStarFinder.h"
#include "LAGaussLegendre.h"

#include "LAAnalyticFormula.h"
#include "LABlackScholesCalc.h"


// Funciton ID of LAShiftMethod
#define FN_JAMSHIDIANSWAPTION	10041
// Function Name of LAShiftMethod
#define FN_JAMSHIDIANSWAPTION_STR	"fn_jamshidianswaption"


class LAObject;
class LARatesPathElementCurve;
class LAPriceDataManager;
class LABlackScholesBase;
class LAMathYieldCurve;


class LAMathJamshidianSwaption : public LACoreValuation
{
public:
	// Default constructor
	LAMathJamshidianSwaption();
	// Copy constructor
	//LAMathJamshidianSwaption(LAMathJamshidianSwaption& v);
	// Destructor
	~LAMathJamshidianSwaption();
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// Make copy(clone) of this class
	virtual LACoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%

	// register dataValues that this class uses
	virtual void				registerData(LAPriceDataManager& dm) const;

	virtual double              value(const LADate& basedate, LAObject& inst, const LADataValuation& att) const;

	/*!
		@brief cache class for performance up
	*/
	class LAMathJamshidianSwaptionDataProvider : public LADataProvider
	{
	public:
		LAMathJamshidianSwaptionDataProvider();
		
		virtual ~LAMathJamshidianSwaptionDataProvider(); 
		
		double mStrike;							// strike
		double mPrem;							// premium
		DoubleVector mGrids;					// grids
		DoubleVector mDelta;					// delta
		DoubleVector mModelGrid;				//modelgrid
		DoubleVector mModelVol;					//modelvol
		unsigned int mStartpos;					// swaptionstatpos
		unsigned int mEndpos;					// swaptionendpos
		double mrStar;							// rstarvalue
		LAMathJamshidianRStarFinder* mprStar;
		LARatesPathElementCurve* mpCurve;
		LARatesPathElementCurve* mpCurve0;
		LAMathYieldCurve* mpTmpCurve;              // temp curve
		double mWeight;							// weight;
		LABlackScholesBase* mAnalyticMethod;
		AnalyticAFFParam mAFFParam;
		LAFindRootBrent mDataInstancefind;
		LAGaussLegendre* mpGL;
		std::vector<pair<double, double> > mBound;
		double mSimPrem;
		double mVol;
		double mAnnuity;
		double mAdjTerm;
		double mAdjDF;
   };

		// set up dataProvider
	LADataProvider*					setUpDataProvider(const LADate& basedate, LAObject& object, 
											const LADataValuation& att) const;
	DoubleVector				getVolatilityResult(const LADataValuation& att) const;

	unsigned int				getNextPos(const LADataValuation& att) const;
	
	double						getNextRstar(const LADataValuation& att) const;
	
	double						getSimPrem(const LADataValuation& att) const;

private:
	// create new cache class
	virtual	LADataProvider*			createNewDataProvider() const;
};
#endif

