/*! @file
    @brief  FX shift delta setup class
*/
//  2008, AlgoQuantHub.
#ifndef AQLRiskConfigurationFXShiftDelta_h
#define AQLRiskConfigurationFXShiftDelta_h
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLRiskConfigurationFXDeltaEx1.h"


//===================== Class Declare AQLRiskConfigurationFXShiftDelta==================================
/*! 
    @brief  FX delta setup class

*/
class AQLRiskConfigurationFXShiftDelta : public AQLRiskConfigurationFXDeltaEx1
{
public:
	// constructor
	explicit AQLRiskConfigurationFXShiftDelta();
	// destructor
	virtual ~AQLRiskConfigurationFXShiftDelta(void);
	// copy constructor
	AQLRiskConfigurationFXShiftDelta(const AQLRiskConfigurationFXShiftDelta &rhs);
	AQLRiskConfigurationFXShiftDelta &operator=(const AQLRiskConfigurationFXShiftDelta &rhs);

protected:
	//==============================================
	// get base outputname
	virtual AQLString getBaseOutPutName(const AQLString &ccy , int index) const;
	//==============================================
	// get base operator
	virtual  AQLString getBaseOperator() const;
	//==============================================
	// create basecoefficient
	virtual  AQLString getBaseCoefficient(const AQLString &ccy) const;
	//==============================================
	// get get FX shift vals
	virtual  DoubleArray  getBaseShiftVals(const AQLString &ccy) const;
	//==============================================
	// get shift vals
	virtual double getBaseSpotVal(const AQLString &ccy, int index) const;
	//==============================================
	// create basescenario object
	virtual std::vector<AQLObject *> createBaseScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const; 
	//==============================================
	// get base target names 
	virtual  AQLString getBaseTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance)  const;	
	////==============================================
	// get outputname1
	virtual  AQLString  getOutPutName1(const AQLString &fx) const;
	////==============================================
	// get outputname1
	virtual  AQLString  getOutPutName1(const AQLString &ccy, int index) const;
	////==============================================
	// get risk name
	virtual  AQLString getRiskName(void) const;
	//==============================================
	// get bump direction
	virtual  AQLString getBumpDirection(const AQLString &fx) const;
	//==============================================
	// get shift type
	virtual  AQLString getShiftType(const AQLString &fx) const;
	//==============================================
	// get target currencies
	virtual  AQLString getTargetCurrencies() const;
	////==============================================
	// get calibration target currencies
	virtual  AQLString getCalibTargetCurrencies() const;
	//==============================================
	// get divid one unit
	virtual  double getDivUnit(const AQLString &fx) const;
	//==============================================
	// get shift value
	virtual double getShiftVal(const AQLString &fx, SCENARIONUM scenarioNum) const;
	//==============================================
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const AQLString& fx) const;
	//==============================================
	// create extra scenario object for base scenario 
	virtual std::vector<AQLObject *> createBaseExtraScenarioEntity(const AQLString &fx, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// get extra target names for base scenario
	virtual  AQLStringVector getBaseExtraTargetNames(const AQLString &fx, AQLDataInstance &dataInstance) const;
	//==============================================
	// create extra yield object for base scenario 
	virtual std::vector<AQLObject *> createBaseExtraYieldEntity(const AQLString &fx, const AQLString &ccy, AQLDataInstance &dataInstance, int index, const bool isFirst = true)  const;
	//==============================================
	// create extra collateral yield object for base scenario 
	virtual std::vector<AQLObject *> createBaseCollateralYieldEntity(const AQLString &fx, const AQLString &ccy, AQLDataInstance &dataInstance, int index)  const;
};


#endif
