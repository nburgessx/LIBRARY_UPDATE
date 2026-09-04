/*! @file
    @brief  FX delta setup class
*/
#ifndef AQLRiskConfigurationFXDeltaGamma_h
#define AQLRiskConfigurationFXDeltaGamma_h
#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#include "AQLRiskConfigurationFXDeltaEx1.h"


class AQLRiskConfigurationFXDeltaGamma : public AQLRiskConfigurationFXDeltaEx1
{
public:
	// constructor
	explicit AQLRiskConfigurationFXDeltaGamma(void);
	// destructor
	virtual ~AQLRiskConfigurationFXDeltaGamma(void);
	// copy constructor
	AQLRiskConfigurationFXDeltaGamma(const AQLRiskConfigurationFXDeltaGamma &rhs);
	AQLRiskConfigurationFXDeltaGamma &operator=(const AQLRiskConfigurationFXDeltaGamma &rhs);

protected:
	//==============================================
	// get coefficient1
	virtual  AQLString  getCoefficient1(const AQLString &ccy) const;
	//==============================================
	// get operator2
	virtual  AQLString  getOperator2(void) const;	
	//==============================================
	// get coefficient2
	virtual  AQLString  getCoefficient2(const AQLString &ccy) const;
	//==============================================
	// get outputname2
	virtual  AQLString  getOutPutName2(const AQLString &ccy) const;
	//==============================================
	// create scenario2 object
	virtual  std::vector<AQLObject *> createScenario2Entity(const AQLString &fx, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// get bump direction
	virtual  AQLString getBumpDirection(const AQLString &ccy) const;
	//==============================================
	// get property bump direction
	AQLString getPropBumpDirection(const AQLString &ccy) const;
};


#endif
