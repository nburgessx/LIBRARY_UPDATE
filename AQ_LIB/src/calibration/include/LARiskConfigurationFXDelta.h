/*! @file
    @brief  FX delta setup class
*/
//  2007, AlgoQuantHub.
#ifndef LARiskConfigurationFXDelta_h
#define LARiskConfigurationFXDelta_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationFXDelta.h
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

#include "LARiskConfigurationFX.h"


//===================== Class Declare LARiskConfigurationFXDelta==================================
/*! 
    @brief  FX delta setup class

*/
class LARiskConfigurationFXDelta : public LARiskConfigurationFX
{
public:
	// constructor
	explicit LARiskConfigurationFXDelta();
	// destructor
	virtual ~LARiskConfigurationFXDelta(void);
	// copy constructor
	LARiskConfigurationFXDelta(const LARiskConfigurationFXDelta &rhs);
	LARiskConfigurationFXDelta &operator=(const LARiskConfigurationFXDelta &rhs);

protected:
	//==============================================
	// get operator1
	virtual  LAString  getOperator1(void) const;	
	//==============================================
	// get operator2
	virtual  LAString  getOperator2(void) const;
	//==============================================
	// get coefficient1
	virtual  LAString  getCoefficient1(const LAString &fx) const;
	//==============================================
	// get coefficient2
	virtual  LAString  getCoefficient2(const LAString &fx) const;
	//==============================================
	// get outputname1
	virtual  LAString  getOutPutName1(const LAString &fx) const;
	//==============================================
	// get outputname2
	virtual  LAString  getOutPutName2(const LAString &fx) const;
	//==============================================
	// get risk name
	virtual  LAString getRiskName(void) const;
	//==============================================
	// get shift type
	virtual  LAString getShiftType(const LAString &fx) const;
	//==============================================
	// get shift value
	virtual double getShiftVal(const LAString &fx, SCENARIONUM scenarioNum) const;
	//==============================================
	// get target currencies
	virtual  LAString getTargetCurrencies() const;
	// get calibration target currencies
	virtual  LAString getCalibTargetCurrencies() const;
	// get coefficient ratio1
	virtual double getCoefficientRatio1(const LAString& ccy, LAObject& e, LADataInstance& dataInstance) const;
	// get coefficient ratio2
	virtual double getCoefficientRatio2(const LAString& ccy, LAObject& e, LADataInstance& dataInstance) const;
	//==============================================
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const LAString& fx) const;
	//==============================================
	// create scenario2 object
	virtual  std::vector<LAObject *> createScenario2Entity(const LAString &fx, LADataInstance &dataInstance, int index) const;

};


#endif
