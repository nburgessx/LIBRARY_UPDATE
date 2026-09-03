/*! @file
    @brief IR Delta setup class
*/
//  2008, Mizuho International London.
#ifndef LARiskConfigurationYieldIRDelta_h
#define LARiskConfigurationYieldIRDelta_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationYieldIRDelta.h
//
//  DESCRIPTION :       IR Delta setup class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif


#include "LARiskConfigurationYield.h"

//===================== Class Declare LARiskConfigurationYieldIRDelta==================================
/*! 
    @brief IR Delta setup class
	

*/
class LARiskConfigurationYieldIRDelta : public LARiskConfigurationYield
{
public:
	// constructor
	explicit LARiskConfigurationYieldIRDelta(void);
	// destructor
	virtual ~LARiskConfigurationYieldIRDelta(void);
	// copy constructor
	LARiskConfigurationYieldIRDelta(const LARiskConfigurationYieldIRDelta &rhs);
	LARiskConfigurationYieldIRDelta &operator=(const LARiskConfigurationYieldIRDelta &rhs);

protected:
	//==============================================
	// create scenario yield object
	virtual std::vector<LAObject *> createYieldEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// create market bump yield object
	virtual std::vector<LAObject *> createMarketBumpYieldEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// create zero bump yield object
	virtual std::vector<LAObject *> createZeroBumpYieldEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// get outputname1
	virtual  LAString  getOutPutName1(const LAString &ccy) const;
	//==============================================
	// isgridsensitivity
	virtual  bool isGridSensitivity(const LAString &ccy) const;
	//==============================================
	// isparallelshift
	virtual  bool isParallelShift(const LAString &ccy) const;

	//==============================================
	// @Description: Tells IMM forward risk mode
	//  @param ccy [in] The currency
	//  @return 0 if no IMM-forward-risk-like care is applied,
	//          1 if market rate bump risk -> IMM forward risk conversion is applied,
	//          2 if market zero rate bump risk -> IMM forward risk conversion is applied
	virtual int getIMMFwdRiskMode(const LAString &ccy) const;

	//==============================================
	// get imm term
	virtual std::vector<int> getIMMTerm(const LAString &ccy) const;

	//==============================================
	// @Description: Returns (possibly term-wise) risk yield curve name(s) for IMM forward risks
	//  @param ccy [in] The currency
	//  @return An empty vector if all IMM forward risks shall be zero,
	//          a vector with one risk curve name if all risk curves are the same,
	//          a vector with risk curve names whose number is the same as the risk grids,
	//          where a risk curve name is the curve type which is suitable to be passed to
	//          LAMathYieldCurvePro::setCurveType (for example, "JPYOIS") to deduce the forward rate changes
	//          against the market rate bumps
	virtual LAStringVector getIMMRiskYieldCurveName(const LAString &ccy) const;

	//==============================================
	// return risk name
	virtual  LAString  getRiskName(void) const;
	//==============================================
	// get grid term
	virtual std::vector<LAString> getGridTerm(const LAString &ccy) const;
	//==============================================
	// get shift grid term
	virtual std::vector<LAString> getShiftGridTerm(const LAString &ccy) const;
	//==============================================
	// get bucket grid term
	virtual std::vector<LAString> getBucketGridTerm(const LAString &ccy) const;
	//==============================================
	// get property bucket grid term
	virtual LAString getPropertyBucketGridTerm(const LAString &ccy) const;
	//==============================================
	// get scenario1 parallel shift
	virtual double getScenario1ParallelShift(const LAString &ccy) const;
	//==============================================
	// get scenario2 parallel shift
	virtual double getScenario2ParallelShift(const LAString &ccy) const;
	//==============================================
	// get scenario1 parallel shift
	virtual void getScenario1ParallelShift(const LAString &ccy, DoubleArray &vals) const;
	//==============================================
	// get scenario2 parallel shift
	virtual void getScenario2ParallelShift(const LAString &ccy, DoubleArray &vals)  const;
	//==============================================
	// get scenario1 grid shift
	virtual DoubleArray getScenario1GridShift(const LAString &ccy) const;
	//==============================================
	// get scenario2 grid shift
	virtual DoubleArray getScenario2GridShift(const LAString &ccy) const;
	//==============================================
	// get scenario1 parallel shift str
	virtual LAString getScenario1ParallelShiftStr(const LAString &ccy) const;
	//==============================================
	// get scenario1 grid shift
	virtual LAString getScenario1GridShiftStr(const LAString &ccy) const;
	//==============================================
	// get shift type
	virtual  LAString getShiftType(const LAString &ccy) const;
	//==============================================
	// get bump direction
	virtual  LAString getBumpDirection(const LAString &ccy) const;
	//==============================================
	// get is wave
	virtual  bool  isWave(const LAString &ccy) const;
	//==============================================
	// get target currencies
	virtual  LAString getTargetCurrencies() const;
	//==============================================
	// get calibration target currencies
	virtual  LAString getCalibTargetCurrencies() const;
	//==============================================
	// get divid one unit
	virtual  double getDivUnit(const LAString &ccy) const;
	//==============================================
	// get grid calc buffer
	virtual int getGridCalcBuffer() const;
	//==============================================
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const LAString& ccy) const;
	//==============================================
	// is zero rate bump
	virtual bool isZeroBump(const LAString& ccy) const;
	//==============================================
	// 
	virtual void getMarketTerms(const LAString& ccy, LAStringVector& terms) const;

	virtual std::vector<LAObject *> createBaseScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const;

    virtual LAString getBaseOutPutName(const LAString &ccy , int index) const;

	virtual bool omitNotionalExposure(const LAString &ccy) const;
    
};
#endif
