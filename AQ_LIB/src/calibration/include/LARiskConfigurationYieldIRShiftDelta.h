/*! @file
    @brief IR Shift Delta setup class
*/
//  2008, Mizuho International London.
#ifndef LARiskConfigurationYieldIRShiftDelta_h
#define LARiskConfigurationYieldIRShiftDelta_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationYieldIRShiftDelta.h
//
//  DESCRIPTION :       IR Shift Delta setup class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif


#include "LARiskConfigurationYieldIRDelta.h"

//===================== Class Declare LARiskConfigurationYieldIRShiftDelta==================================
/*! 
    @brief IR Shift PV setup class
	

*/
class LARiskConfigurationYieldIRShiftDelta : public LARiskConfigurationYieldIRDelta
{
public:
	// constructor
	explicit LARiskConfigurationYieldIRShiftDelta(void);
	// destructor
	virtual ~LARiskConfigurationYieldIRShiftDelta(void);
	// copy constructor
	LARiskConfigurationYieldIRShiftDelta(const LARiskConfigurationYieldIRShiftDelta &rhs);
	LARiskConfigurationYieldIRShiftDelta &operator=(const LARiskConfigurationYieldIRShiftDelta &rhs);

protected:
	//==============================================
	// create basescenario object
	virtual std::vector<LAObject *> createBaseScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const; 
	//==============================================
	// get baseoperator
	virtual  LAString  getBaseOperator(void) const;	
	//==============================================
	// get basecoefficient
	virtual  LAString  getBaseCoefficient(const LAString &ccy) const;
	//==============================================
	// get baseoutputname
	virtual  LAString  getBaseOutPutName(const LAString &ccy, int index) const;
	//==============================================
	// get baseshift string
	virtual  LAString  getBaseShiftStr(const LAString &ccy, int index) const;
	//==============================================
	// get outputname1
	virtual  LAString  getOutPutName1(const LAString &ccy) const;
	//==============================================
	// get outputname1
	virtual  LAString  getOutPutName1(const LAString &ccy, int index) const;
	//==============================================
	// isgridsensitivity
	virtual  bool isGridSensitivity(const LAString &ccy) const;
	//==============================================
	// isparallelshift
	virtual  bool isParallelShift(const LAString &ccy) const;
	//==============================================
	// create risk object
	virtual  std::vector<std::pair<LAString, std::vector<LAObject *> > > createRiskEntity(LAObjectPool &objPool) const;
	//==============================================
	// return risk name
	virtual  LAString  getRiskName(void) const;
	//==============================================
	// get shift vals(bp)
	virtual DoubleArray getBaseShifts(const LAString &ccy, int index) const;
	//==============================================
	// get shift grid term
	virtual std::vector<LAString> getShiftGridTerm(const LAString &ccy) const;
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
	// get target currency
	virtual  LAString getTargetCurrencies() const;
	//==============================================
	// get calibration target currencies
	virtual  LAString getCalibTargetCurrencies() const;
	//==============================================
	// get divid one unit
	virtual  double getDivUnit(const LAString &ccy) const;
	//==============================================
	// get isirshiftscenario
	virtual  bool  isIRShiftScenario(const LAString &ccy) const;
	//==============================================
	// get get IR shift vals
	virtual  DoubleArray  getIRShiftVals(const LAString &ccy) const;
	//==============================================
	// get get IR shift scenario vals
	virtual  DoubleArray  getIRShiftScenarioVals(const LAString &ccy) const;
	//==============================================
	// get grid calc buffer
	virtual int getGridCalcBuffer() const;
	//==============================================
	// get base extra target names 
	virtual  LAStringVector getBaseExtraTargetNames(const LAString &ccy, LADataInstance &dataInstance) const;
	//==============================================
	// create base extraScenario object 
	virtual  std::vector<LAObject *> createBaseExtraScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const;
	//==============================================
	// get bucket grid term
	virtual std::vector<LAString> getBucketGridTerm(const LAString &ccy) const;
	//==============================================
	// get property bucket grid term
	virtual LAString getPropertyBucketGridTerm(const LAString &ccy) const;
	//==============================================
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const LAString& ccy) const;
	//==============================================
	// is zero rate bump
	virtual bool isZeroBump(const LAString& ccy) const;

};
#endif
