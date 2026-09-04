/*! @file
    @brief  FX delta setup class
*/
//  2008, AlgoQuantHub.
#ifndef AQLRiskConfigurationFXDeltaEx1_h
#define AQLRiskConfigurationFXDeltaEx1_h
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLRiskConfigurationFX.h"


//===================== Class Declare AQLRiskConfigurationFXDeltaEx1==================================
/*! 
    @brief  FX delta setup class

*/
class AQLRiskConfigurationFXDeltaEx1 : public AQLRiskConfigurationFX
{
public:
	// constructor
	explicit AQLRiskConfigurationFXDeltaEx1();
	// destructor
	virtual ~AQLRiskConfigurationFXDeltaEx1(void);
	// copy constructor
	AQLRiskConfigurationFXDeltaEx1(const AQLRiskConfigurationFXDeltaEx1 &rhs);
	AQLRiskConfigurationFXDeltaEx1 &operator=(const AQLRiskConfigurationFXDeltaEx1 &rhs);

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
