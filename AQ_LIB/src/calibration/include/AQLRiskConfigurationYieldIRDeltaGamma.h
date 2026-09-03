#pragma once

#ifdef __GNUG__
#pragma interface
#endif


#include "AQLRiskConfigurationYieldIRDelta.h"



class AQLRiskConfigurationYieldIRDeltaGamma : public AQLRiskConfigurationYieldIRDelta
{
public:
	// constructor
	explicit AQLRiskConfigurationYieldIRDeltaGamma(void);
	// destructor
	virtual ~AQLRiskConfigurationYieldIRDeltaGamma(void);
	// copy constructor
	AQLRiskConfigurationYieldIRDeltaGamma(const AQLRiskConfigurationYieldIRDeltaGamma &rhs);
	AQLRiskConfigurationYieldIRDeltaGamma &operator=(const AQLRiskConfigurationYieldIRDeltaGamma &rhs);

protected:
	//==============================================
	// get operator2
	virtual  AQLString  getOperator2(void) const;	
	//==============================================
	// get coefficient1
	virtual  AQLString  getCoefficient1(const AQLString &ccy) const;
	//==============================================
	// get coefficient2
	virtual  AQLString  getCoefficient2(const AQLString &ccy) const;
	//==============================================
	// get outputname2
	virtual  AQLString  getOutPutName2(const AQLString &ccy) const;
	//==============================================
	// get bump direction
	virtual  AQLString getBumpDirection(const AQLString &ccy) const;
	//==============================================
	// get property bump direction
	AQLString getPropBumpDirection(const AQLString &ccy) const;

};

