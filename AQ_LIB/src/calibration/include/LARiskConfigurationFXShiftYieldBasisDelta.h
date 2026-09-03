/*! @file
    @brief FX shift Basis Delta setup class
*/
//  2008, Mizuho International London.
#ifndef LARiskConfigurationFXShiftYieldBasisDelta_h
#define LARiskConfigurationFXShiftYieldBasisDelta_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationFXShiftYieldBasisDelta.h
//
//  DESCRIPTION :       FX Shift Basis Delta setup class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif


#include "LARiskConfigurationYieldBasisDelta.h"

//===================== Class Declare LARiskConfigurationFXShiftYieldBasisDelta==================================
/*! 
    @brief IR Delta setup class
	

*/
class LARiskConfigurationFXShiftYieldBasisDelta : public LARiskConfigurationYieldBasisDelta
{
public:
	// constructor
	explicit LARiskConfigurationFXShiftYieldBasisDelta(void);
	// destructor
	virtual ~LARiskConfigurationFXShiftYieldBasisDelta(void);
	// copy constructor
	LARiskConfigurationFXShiftYieldBasisDelta(const LARiskConfigurationFXShiftYieldBasisDelta &rhs);
	LARiskConfigurationFXShiftYieldBasisDelta &operator=(const LARiskConfigurationFXShiftYieldBasisDelta &rhs);

protected:
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
	// return risk name
	virtual  LAString  getRiskName(void) const;
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
	// get scenario1 parallel shift str
	virtual LAString getScenario1ParallelShiftStr(const LAString &ccy) const;
	//==============================================
	// get scenario1 grid shift
	virtual LAString getScenario1GridShiftStr(const LAString &ccy) const;
	//==============================================
	// get shift type
	virtual  LAString getShiftType(const LAString &ccy) const;
	//==============================================
	// get shift type
	virtual  LAString getBasisType(const LAString &ccy) const;
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
	//==============================================
	virtual int getGridCalcBuffer() const;
	// get extra target names1
	virtual  LAStringVector getExtraTargetNames1(const LAString &ccy, LADataInstance &dataInstance) const;
	//==============================================
	// create extraScenario1 object 
	virtual  std::vector<std::vector<LAObject *> > createExtraScenario1Entity(const LAString &ccy, LADataInstance &dataInstance, int index) const;
	//==============================================
	// create extraScenario2 object 
	virtual  std::vector<std::vector<LAObject *> > createExtraScenario2Entity(const LAString &ccy, LADataInstance &dataInstance, int index) const;
	//==============================================
	// get get shift vals
	virtual  DoubleArray  getBaseShiftVals(const LAString &fx) const;
	//==============================================
	// get shift vals
	virtual double getBaseSpotVal(const LAString &fx, int index) const;
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
	//  get cross base currency
	virtual  LAString  getCrossBaseCurrency(const LAString &ccy) const;
	//==============================================
	// get base extra target names 
	virtual  LAStringVector getBaseExtraTargetNames(const LAString &ccy, LADataInstance &dataInstance) const;
	//==============================================
	// create base extraScenario object 
	virtual  std::vector<LAObject *> createBaseExtraScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const;
	//==============================================
	// create basescenario object
	virtual std::vector<LAObject *> createBaseScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const;
	//==============================================
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const LAString& ccy) const;
	//==============================================
	// get base target names 
	virtual  LAString getBaseTargetNames(const LAString &ccy, LADataInstance &dataInstance)  const;	


};
#endif
