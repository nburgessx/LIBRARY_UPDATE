/*! @file
    @brief Risk Scenario create class
*/
//  2008, Mizuho International London.
#ifndef LAScenarioConfiguration_h
#define LAScenarioConfiguration_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAScenarioConfiguration.h
//
//  DESCRIPTION :        Risk Scenario create class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////


#include "LAString.h"
#include "LACoreTemplateType.h"
#include "LADefinitions.h"
#include "LAMarketData.h"

class LADataInstance;
class LAObject;

//===================== Struct Declare LAScenarioConfiguration==================================
/*! 
    @brief Risk Scenario Parameter struct
	

*/
struct MAScenarioParam
{
	// constructor
	MAScenarioParam(void)
		:bumpType(""), isParallel(false), isGrid(false), isLJ(false), isDDL(false), isWave(false), isCalib(false), isOutPut(false),
		isAdjustDF(false), isAudExtra(true), isBusinessDayRoll(false), maxIndex(0), extraBaseParam(0.0), extraBaseVolParam(0.0), calendar(MLIB_NO_DATA),
		isForeignCcy(false), isCollateralCcy(false), isFirst(true), fwdfxDenominator(10000.0), isExtraXccyCurveForFXDelta(false), isExtraFwdFXConstCurveForFXDelta(false), isFwdFXZeroRateBump(false), isFwdPointBump(true)
	{
		LAMarketData::setUpScenario(*this);
	}
	// destructor
	~MAScenarioParam(void) {}

	LAString ccy;                  // currency
	LAString calcType;		       // calcType
	LAString model;		           // model
	LAString inputType;		       // market input type (for volatility or correlation)
	LAString shiftType;            // shiftType
	LAString basisType;            // basisType
	LAString bumpDirection;        // bump direction
	LAString bumpType;             // bump type
	LAString serializeStatus;      // scerialize status
	LAString serializeFile;        // scerialize file
	LAString calendar;             // calendar
	LAString vegaType;             // vega type
	bool isParallel;               // parallel shift flag
	bool isGrid;                   // grid shift flag
	bool isLJ;                     // long jump flag
	bool isDDL;                     // displaced diffusion flag
	bool isWave;                   // wave flag
	bool isCalib;                   // calibrate flag
	bool isOutPut;                   // output flag
	bool isAdjustDF;               // adjust discount factor flag
	bool isAudExtra;               // aud extra flag
	bool isBusinessDayRoll;          // business day roll flag
	LAStringVector paraTerm;          // parallel shift term
	DoubleVector paraShiftVec;     // parallel shift vector
	DoubleMatrix paraShiftMtx;     // parallel shift matrix
	LAStringVector paraFile;        // file path for parameter(parallel)
	LAStringVector gridTerm;         // grid shift term
	IntArray     gridGroupID;      // grid shift group id
	DoubleVector gridShiftVec;     // grid shift vector
	DoubleMatrix gridShiftMtx;     // grid shift matrix
	LAStringMatrix gridFile;        // file path for parameter(grid)
	DoubleVector baseShiftVal;     // base shift val
	DoubleVector baseShiftMtx;     // base shift matrix
	LAString targetName;           // target market
	LAString targetCurveType;           // target curve Type
	LAStringVector refName;          // reference market data val
	unsigned int maxIndex;         // max index for grid risk
	DoubleVector extraParam;              // extra param
	double extraBaseParam;			// extra base param
	DoubleVector extraBaseParamVec;	// extra base paramvec
	double extraBaseVolParam;		// extra base param for volatility
	bool isForeignCcy;					// foreign currency object flag
	bool isCollateralCcy;				// collateral currency object flag
	bool isFirst;				// first execute flag
	double fwdfxDenominator;	// denominator for forward fx market
	bool isExtraXccyCurveForFXDelta;		// extra xccy basis curve flag (for fxdelta)
	bool isExtraFwdFXConstCurveForFXDelta;	// extra forward fx constant curve flag (for fxdelta)
	bool isZeroVol;				// is zero volatility
	DoubleMatrix corMtx;		// alternative correlation matrix
	bool recalcBasisDFsOnZeroRateBump;	// whether remove basis DFs in prior to processing
	bool isFwdFXZeroRateBump;   // whether forward FX risk is bumped via zero rate.
	bool isFwdPointBump;   // flag for 1 point forward FX bumped. Default value is TRUE.
};

//===================== Class Declare LAScenarioConfiguration==================================
/*! 
    @brief Risk Scenario create class
	
	this class is abstract

*/
class LAScenarioConfiguration
{
public:
	// constructor
	explicit LAScenarioConfiguration(void){}
	// destructor
	virtual ~LAScenarioConfiguration(void){}
	//// copy constructor
	//LAScenarioConfiguration(const LAScenarioConfiguration &rhs);
	//LAScenarioConfiguration &operator=(const LAScenarioConfiguration &rhs);

	//==============================================
	// create risk scenario
	virtual std::vector<LAObject *> createScenario(LADataInstance &dataInstance, const MAScenarioParam &param) const = 0;
protected:
	// create risk scenario for foregin currency
	virtual std::vector<LAObject *> createScenarioForeignYield(LADataInstance &dataInstance, const MAScenarioParam &param) const;
	// create risk scenario for foregin currency
	virtual std::vector<LAObject *> createScenarioCollateralYield(LADataInstance &dataInstance, const MAScenarioParam &param) const;
	// dataout curves
	virtual void dataoutCurve(LADataInstance &dataInstance, const LAMathYieldCurvePro& yieldCurvePro, const std::vector<LAObject *>& ret) const;
	// create scenario of extra Xccy basis curve for fxdelta
	virtual std::vector<LAObject *> createScenarioExtraXccyCurveForFXDelta(LADataInstance &dataInstance, const MAScenarioParam &param) const;
	// create scenario of extra FwdFX constant curve for fxdelta
	virtual std::vector<LAObject *> createScenarioExtraFwdFXConstCurveForFXDelta(LADataInstance &dataInstance, const MAScenarioParam &param) const;
};


#endif
