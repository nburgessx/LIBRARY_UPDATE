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
	virtual  AQLString  getOperator1(void) const;	
	//==============================================
	// get operator2
	virtual  AQLString  getOperator2(void) const;
	//==============================================
	// get coefficient1
	virtual  AQLString  getCoefficient1(const AQLString &fx) const;
	//==============================================
	// get coefficient2
	virtual  AQLString  getCoefficient2(const AQLString &fx) const;
	//==============================================
	// get outputname1
	virtual  AQLString  getOutPutName1(const AQLString &fx) const;
	//==============================================
	// get outputname2
	virtual  AQLString  getOutPutName2(const AQLString &fx) const;
	//==============================================
	// get risk name
	virtual  AQLString getRiskName(void) const;
	//==============================================
	// get shift type
	virtual  AQLString getShiftType(const AQLString &fx) const;
	//==============================================
	// get shift value
	virtual double getShiftVal(const AQLString &fx, SCENARIONUM scenarioNum) const;
	//==============================================
	// get target currencies
	virtual  AQLString getTargetCurrencies() const;
	// get calibration target currencies
	virtual  AQLString getCalibTargetCurrencies() const;
	// get coefficient ratio1
	virtual double getCoefficientRatio1(const AQLString& ccy, AQLObject& e, AQLDataInstance& dataInstance) const;
	// get coefficient ratio2
	virtual double getCoefficientRatio2(const AQLString& ccy, AQLObject& e, AQLDataInstance& dataInstance) const;
	//==============================================
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const AQLString& fx) const;
	//==============================================
	// create scenario2 object
	virtual  std::vector<AQLObject *> createScenario2Entity(const AQLString &fx, AQLDataInstance &dataInstance, int index) const;

};


#endif
