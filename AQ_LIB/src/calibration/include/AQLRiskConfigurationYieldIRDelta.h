/*! @file
    @brief IR Delta setup class
*/
//  2008, AlgoQuantHub.
#ifndef AQLRiskConfigurationYieldIRDelta_h
#define AQLRiskConfigurationYieldIRDelta_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLRiskConfigurationYieldIRDelta.h
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


#include "AQLRiskConfigurationYield.h"

//===================== Class Declare AQLRiskConfigurationYieldIRDelta==================================
/*! 
    @brief IR Delta setup class
	

*/
class AQLRiskConfigurationYieldIRDelta : public AQLRiskConfigurationYield
{
public:
	// constructor
	explicit AQLRiskConfigurationYieldIRDelta(void);
	// destructor
	virtual ~AQLRiskConfigurationYieldIRDelta(void);
	// copy constructor
	AQLRiskConfigurationYieldIRDelta(const AQLRiskConfigurationYieldIRDelta &rhs);
	AQLRiskConfigurationYieldIRDelta &operator=(const AQLRiskConfigurationYieldIRDelta &rhs);

protected:
	//==============================================
	// create scenario yield object
	virtual std::vector<AQLObject *> createYieldEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// create market bump yield object
	virtual std::vector<AQLObject *> createMarketBumpYieldEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// create zero bump yield object
	virtual std::vector<AQLObject *> createZeroBumpYieldEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// get outputname1
	virtual  AQLString  getOutPutName1(const AQLString &ccy) const;
	//==============================================
	// isgridsensitivity
	virtual  bool isGridSensitivity(const AQLString &ccy) const;
	//==============================================
	// isparallelshift
	virtual  bool isParallelShift(const AQLString &ccy) const;

	//==============================================
	// @Description: Tells IMM forward risk mode
	//  @param ccy [in] The currency
	//  @return 0 if no IMM-forward-risk-like care is applied,
	//          1 if market rate bump risk -> IMM forward risk conversion is applied,
	//          2 if market zero rate bump risk -> IMM forward risk conversion is applied
	virtual int getIMMFwdRiskMode(const AQLString &ccy) const;

	//==============================================
	// get imm term
	virtual std::vector<int> getIMMTerm(const AQLString &ccy) const;

	//==============================================
	// @Description: Returns (possibly term-wise) risk yield curve name(s) for IMM forward risks
	//  @param ccy [in] The currency
	//  @return An empty vector if all IMM forward risks shall be zero,
	//          a vector with one risk curve name if all risk curves are the same,
	//          a vector with risk curve names whose number is the same as the risk grids,
	//          where a risk curve name is the curve type which is suitable to be passed to
	//          AQLMathYieldCurvePro::setCurveType (for example, "JPYOIS") to deduce the forward rate changes
	//          against the market rate bumps
	virtual AQLStringVector getIMMRiskYieldCurveName(const AQLString &ccy) const;

	//==============================================
	// return risk name
	virtual  AQLString  getRiskName(void) const;
	//==============================================
	// get grid term
	virtual std::vector<AQLString> getGridTerm(const AQLString &ccy) const;
	//==============================================
	// get shift grid term
	virtual std::vector<AQLString> getShiftGridTerm(const AQLString &ccy) const;
	//==============================================
	// get bucket grid term
	virtual std::vector<AQLString> getBucketGridTerm(const AQLString &ccy) const;
	//==============================================
	// get property bucket grid term
	virtual AQLString getPropertyBucketGridTerm(const AQLString &ccy) const;
	//==============================================
	// get scenario1 parallel shift
	virtual double getScenario1ParallelShift(const AQLString &ccy) const;
	//==============================================
	// get scenario2 parallel shift
	virtual double getScenario2ParallelShift(const AQLString &ccy) const;
	//==============================================
	// get scenario1 parallel shift
	virtual void getScenario1ParallelShift(const AQLString &ccy, DoubleArray &vals) const;
	//==============================================
	// get scenario2 parallel shift
	virtual void getScenario2ParallelShift(const AQLString &ccy, DoubleArray &vals)  const;
	//==============================================
	// get scenario1 grid shift
	virtual DoubleArray getScenario1GridShift(const AQLString &ccy) const;
	//==============================================
	// get scenario2 grid shift
	virtual DoubleArray getScenario2GridShift(const AQLString &ccy) const;
	//==============================================
	// get scenario1 parallel shift str
	virtual AQLString getScenario1ParallelShiftStr(const AQLString &ccy) const;
	//==============================================
	// get scenario1 grid shift
	virtual AQLString getScenario1GridShiftStr(const AQLString &ccy) const;
	//==============================================
	// get shift type
	virtual  AQLString getShiftType(const AQLString &ccy) const;
	//==============================================
	// get bump direction
	virtual  AQLString getBumpDirection(const AQLString &ccy) const;
	//==============================================
	// get is wave
	virtual  bool  isWave(const AQLString &ccy) const;
	//==============================================
	// get target currencies
	virtual  AQLString getTargetCurrencies() const;
	//==============================================
	// get calibration target currencies
	virtual  AQLString getCalibTargetCurrencies() const;
	//==============================================
	// get divid one unit
	virtual  double getDivUnit(const AQLString &ccy) const;
	//==============================================
	// get grid calc buffer
	virtual int getGridCalcBuffer() const;
	//==============================================
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const AQLString& ccy) const;
	//==============================================
	// is zero rate bump
	virtual bool isZeroBump(const AQLString& ccy) const;
	//==============================================
	// 
	virtual void getMarketTerms(const AQLString& ccy, AQLStringVector& terms) const;

	virtual std::vector<AQLObject *> createBaseScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const;

    virtual AQLString getBaseOutPutName(const AQLString &ccy , int index) const;

	virtual bool omitNotionalExposure(const AQLString &ccy) const;
    
};
#endif
