/*! @file
    @brief  FX delta setup class
*/
//  2008, AlgoQuantHub.
#ifndef LARiskConfigurationFXDeltaEx1_h
#define LARiskConfigurationFXDeltaEx1_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationFXDeltaEx1.h
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


//===================== Class Declare LARiskConfigurationFXDeltaEx1==================================
/*! 
    @brief  FX delta setup class

*/
class LARiskConfigurationFXDeltaEx1 : public LARiskConfigurationFX
{
public:
	// constructor
	explicit LARiskConfigurationFXDeltaEx1();
	// destructor
	virtual ~LARiskConfigurationFXDeltaEx1(void);
	// copy constructor
	LARiskConfigurationFXDeltaEx1(const LARiskConfigurationFXDeltaEx1 &rhs);
	LARiskConfigurationFXDeltaEx1 &operator=(const LARiskConfigurationFXDeltaEx1 &rhs);

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
	// get bump direction
	virtual  LAString getBumpDirection(const LAString &fx) const;
	//==============================================
	// get target currencies
	virtual  LAString getTargetCurrencies() const;
	//==============================================
	// get calibration target currencies
	virtual  LAString getCalibTargetCurrencies() const;
	//==============================================
	// get divid one unit
	virtual  double getDivUnit(const LAString &fx) const;
	//==============================================
	// get shift value
	virtual double getShiftVal(const LAString &fx, SCENARIONUM scenarioNum) const;
	//==============================================
	// get coefficient ratio1
	virtual double getCoefficientRatio1(const LAString& ccy, LAObject& e, LADataInstance& dataInstance) const;
	//==============================================
	// get coefficient ratio2
	virtual double getCoefficientRatio2(const LAString& ccy, LAObject& e, LADataInstance& dataInstance) const;
	//==============================================
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const LAString& fx) const;
	//==============================================
	// is zero rate bump
	virtual bool isZeroBump(const LAString& ccy) const { (void)ccy; return false; }


};


#endif
