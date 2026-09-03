/*! @file
    @brief IR Delta setup class
*/
//  2008, AlgoQuantHub.
#ifndef LARiskConfigurationFXShiftYieldIRDelta_h
#define LARiskConfigurationFXShiftYieldIRDelta_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationFXShiftYieldIRDelta.h
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


#include "LARiskConfigurationYieldIRDelta.h"

//===================== Class Declare LARiskConfigurationFXShiftYieldIRDelta==================================
/*! 
    @brief IR Delta setup class
	

*/
class LARiskConfigurationFXShiftYieldIRDelta : public LARiskConfigurationYieldIRDelta
{
public:
	// constructor
	explicit LARiskConfigurationFXShiftYieldIRDelta(void);
	// destructor
	virtual ~LARiskConfigurationFXShiftYieldIRDelta(void);
	// copy constructor
	LARiskConfigurationFXShiftYieldIRDelta(const LARiskConfigurationFXShiftYieldIRDelta &rhs);
	LARiskConfigurationFXShiftYieldIRDelta &operator=(const LARiskConfigurationFXShiftYieldIRDelta &rhs);

protected:
	//==============================================
	// get outputname1
	virtual  AQLString  getOutPutName1(const AQLString &ccy) const;
	//==============================================
	// get outputname1
	virtual  AQLString  getOutPutName1(const AQLString &ccy, int index) const;
	//==============================================
	// isgridsensitivity
	virtual  bool isGridSensitivity(const AQLString &ccy) const;
	//==============================================
	// isparallelshift
	virtual  bool isParallelShift(const AQLString &ccy) const;
	//==============================================
	// return risk name
	virtual  AQLString  getRiskName(void) const;
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
	//==============================================
	virtual int getGridCalcBuffer() const;
	// get extra target names1
	virtual  AQLStringVector getExtraTargetNames1(const AQLString &ccy, AQLDataInstance &dataInstance) const;
	//==============================================
	// create extraScenario1 object 
	virtual  std::vector<std::vector<AQLObject *> > createExtraScenario1Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// create extraScenario2 object 
	virtual  std::vector<std::vector<AQLObject *> > createExtraScenario2Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// get get shift vals
	virtual  DoubleArray  getBaseShiftVals(const AQLString &ccy) const;
	//==============================================
	// get shift vals
	virtual double getBaseSpotVal(const AQLString &ccy, int index) const;
	//==============================================
	// get baseoperator
	virtual  AQLString  getBaseOperator(void) const;	
	//==============================================
	// get basecoefficient
	virtual  AQLString  getBaseCoefficient(const AQLString &ccy) const;
	//==============================================
	// get baseoutputname
	virtual  AQLString  getBaseOutPutName(const AQLString &ccy, int index) const;
	//==============================================
	//  get cross base currency
	virtual  AQLString  getCrossBaseCurrency(const AQLString &ccy) const;
	//==============================================
	// get base target names 
	virtual  AQLString getBaseTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance)  const;	
	//==============================================
	// create basescenario object
	virtual std::vector<AQLObject *> createBaseScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// get base extra target names 
	virtual  AQLStringVector getBaseExtraTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance) const;
	//==============================================
	// create base extraScenario object 
	virtual  std::vector<AQLObject *> createBaseExtraScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const AQLString& ccy) const;
	//==============================================
	// is zero rate bump
	virtual bool isZeroBump(const AQLString& ccy) const;

};
#endif
