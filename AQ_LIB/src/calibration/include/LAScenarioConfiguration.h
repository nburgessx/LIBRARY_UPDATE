/*! @file
    @brief Risk Scenario create class
*/
//  2008, AlgoQuantHub.
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


#include "AQLString.h"
#include "AQLCoreTemplateType.h"
#include "LADefinitions.h"
#include "LAMarketData.h"

class AQLDataInstance;
class AQLObject;

//===================== Struct Declare LAScenarioConfiguration==================================
/*! 
    @brief Risk Scenario Parameter struct
	

*/
struct MAScenarioParam
{
	// constructor
	MAScenarioParam(void)
		:bumpType(""), isParallel(false), isGrid(false), isLJ(false), isDDL(false), isWave(false), isCalib(false), isOutPut(false),
		isAdjustDF(false), isAudExtra(true), isBusinessDayRoll(false), maxIndex(0), extraBaseParam(0.0), extraBaseVolParam(0.0), calendar(AQ_NO_DATA),
		isForeignCcy(false), isCollateralCcy(false), isFirst(true), fwdfxDenominator(10000.0), isExtraXccyCurveForFXDelta(false), isExtraFwdFXConstCurveForFXDelta(false), isFwdFXZeroRateBump(false), isFwdPointBump(true)
	{
		LAMarketData::setUpScenario(*this);
	}
	// destructor
	~MAScenarioParam(void) {}

	AQLString ccy;                  // currency
	AQLString calcType;		       // calcType
	AQLString model;		           // model
	AQLString inputType;		       // market input type (for volatility or correlation)
	AQLString shiftType;            // shiftType
	AQLString basisType;            // basisType
	AQLString bumpDirection;        // bump direction
	AQLString bumpType;             // bump type
	AQLString serializeStatus;      // scerialize status
	AQLString serializeFile;        // scerialize file
	AQLString calendar;             // calendar
	AQLString vegaType;             // vega type
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
	AQLStringVector paraTerm;          // parallel shift term
	DoubleVector paraShiftVec;     // parallel shift vector
	DoubleMatrix paraShiftMtx;     // parallel shift matrix
	AQLStringVector paraFile;        // file path for parameter(parallel)
	AQLStringVector gridTerm;         // grid shift term
	IntArray     gridGroupID;      // grid shift group id
	DoubleVector gridShiftVec;     // grid shift vector
	DoubleMatrix gridShiftMtx;     // grid shift matrix
	AQLStringMatrix gridFile;        // file path for parameter(grid)
	DoubleVector baseShiftVal;     // base shift val
	DoubleVector baseShiftMtx;     // base shift matrix
	AQLString targetName;           // target market
	AQLString targetCurveType;           // target curve Type
	AQLStringVector refName;          // reference market data val
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
	virtual std::vector<AQLObject *> createScenario(AQLDataInstance &dataInstance, const MAScenarioParam &param) const = 0;
protected:
	// create risk scenario for foregin currency
	virtual std::vector<AQLObject *> createScenarioForeignYield(AQLDataInstance &dataInstance, const MAScenarioParam &param) const;
	// create risk scenario for foregin currency
	virtual std::vector<AQLObject *> createScenarioCollateralYield(AQLDataInstance &dataInstance, const MAScenarioParam &param) const;
	// dataout curves
	virtual void dataoutCurve(AQLDataInstance &dataInstance, const AQLMathYieldCurvePro& yieldCurvePro, const std::vector<AQLObject *>& ret) const;
	// create scenario of extra Xccy basis curve for fxdelta
	virtual std::vector<AQLObject *> createScenarioExtraXccyCurveForFXDelta(AQLDataInstance &dataInstance, const MAScenarioParam &param) const;
	// create scenario of extra FwdFX constant curve for fxdelta
	virtual std::vector<AQLObject *> createScenarioExtraFwdFXConstCurveForFXDelta(AQLDataInstance &dataInstance, const MAScenarioParam &param) const;
};


#endif
