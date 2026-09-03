/*! @file
    @brief  FX shift delta setup class
*/
//  2008, AlgoQuantHub.
#ifndef LARiskConfigurationFXShiftDelta_h
#define LARiskConfigurationFXShiftDelta_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationFXShiftDelta.h
//
//  DESCRIPTION :         FX delta setup class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LARiskConfigurationFXDeltaEx1.h"


//===================== Class Declare LARiskConfigurationFXShiftDelta==================================
/*! 
    @brief  FX delta setup class

*/
class LARiskConfigurationFXShiftDelta : public LARiskConfigurationFXDeltaEx1
{
public:
	// constructor
	explicit LARiskConfigurationFXShiftDelta();
	// destructor
	virtual ~LARiskConfigurationFXShiftDelta(void);
	// copy constructor
	LARiskConfigurationFXShiftDelta(const LARiskConfigurationFXShiftDelta &rhs);
	LARiskConfigurationFXShiftDelta &operator=(const LARiskConfigurationFXShiftDelta &rhs);

protected:
	//==============================================
	// get base outputname
	virtual LAString getBaseOutPutName(const LAString &ccy , int index) const;
	//==============================================
	// get base operator
	virtual  LAString getBaseOperator() const;
	//==============================================
	// create basecoefficient
	virtual  LAString getBaseCoefficient(const LAString &ccy) const;
	//==============================================
	// get get FX shift vals
	virtual  DoubleArray  getBaseShiftVals(const LAString &ccy) const;
	//==============================================
	// get shift vals
	virtual double getBaseSpotVal(const LAString &ccy, int index) const;
	//==============================================
	// create basescenario object
	virtual std::vector<LAObject *> createBaseScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const; 
	//==============================================
	// get base target names 
	virtual  LAString getBaseTargetNames(const LAString &ccy, LADataInstance &dataInstance)  const;	
	////==============================================
	// get outputname1
	virtual  LAString  getOutPutName1(const LAString &fx) const;
	////==============================================
	// get outputname1
	virtual  LAString  getOutPutName1(const LAString &ccy, int index) const;
	////==============================================
	// get risk name
	virtual  LAString getRiskName(void) const;
	//==============================================
	// get bump direction
	virtual  LAString getBumpDirection(const LAString &fx) const;
	//==============================================
	// get shift type
	virtual  LAString getShiftType(const LAString &fx) const;
	//==============================================
	// get target currencies
	virtual  LAString getTargetCurrencies() const;
	////==============================================
	// get calibration target currencies
	virtual  LAString getCalibTargetCurrencies() const;
	//==============================================
	// get divid one unit
	virtual  double getDivUnit(const LAString &fx) const;
	//==============================================
	// get shift value
	virtual double getShiftVal(const LAString &fx, SCENARIONUM scenarioNum) const;
	//==============================================
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const LAString& fx) const;
	//==============================================
	// create extra scenario object for base scenario 
	virtual std::vector<LAObject *> createBaseExtraScenarioEntity(const LAString &fx, LADataInstance &dataInstance, int index) const;
	//==============================================
	// get extra target names for base scenario
	virtual  LAStringVector getBaseExtraTargetNames(const LAString &fx, LADataInstance &dataInstance) const;
	//==============================================
	// create extra yield object for base scenario 
	virtual std::vector<LAObject *> createBaseExtraYieldEntity(const LAString &fx, const LAString &ccy, LADataInstance &dataInstance, int index, const bool isFirst = true)  const;
	//==============================================
	// create extra collateral yield object for base scenario 
	virtual std::vector<LAObject *> createBaseCollateralYieldEntity(const LAString &fx, const LAString &ccy, LADataInstance &dataInstance, int index)  const;
};


#endif
