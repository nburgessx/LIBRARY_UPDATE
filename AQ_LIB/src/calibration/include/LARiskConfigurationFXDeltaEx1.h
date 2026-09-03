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
	// get bump direction
	virtual  AQLString getBumpDirection(const AQLString &fx) const;
	//==============================================
	// get target currencies
	virtual  AQLString getTargetCurrencies() const;
	//==============================================
	// get calibration target currencies
	virtual  AQLString getCalibTargetCurrencies() const;
	//==============================================
	// get divid one unit
	virtual  double getDivUnit(const AQLString &fx) const;
	//==============================================
	// get shift value
	virtual double getShiftVal(const AQLString &fx, SCENARIONUM scenarioNum) const;
	//==============================================
	// get coefficient ratio1
	virtual double getCoefficientRatio1(const AQLString& ccy, AQLObject& e, AQLDataInstance& dataInstance) const;
	//==============================================
	// get coefficient ratio2
	virtual double getCoefficientRatio2(const AQLString& ccy, AQLObject& e, AQLDataInstance& dataInstance) const;
	//==============================================
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const AQLString& fx) const;
	//==============================================
	// is zero rate bump
	virtual bool isZeroBump(const AQLString& ccy) const { (void)ccy; return false; }


};


#endif
