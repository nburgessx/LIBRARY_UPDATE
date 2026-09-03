#pragma once

#ifdef __GNUG__
#pragma interface
#endif


#include "LARiskConfigurationYieldIRDelta.h"



class LARiskConfigurationYieldIRDeltaGamma : public LARiskConfigurationYieldIRDelta
{
public:
	// constructor
	explicit LARiskConfigurationYieldIRDeltaGamma(void);
	// destructor
	virtual ~LARiskConfigurationYieldIRDeltaGamma(void);
	// copy constructor
	LARiskConfigurationYieldIRDeltaGamma(const LARiskConfigurationYieldIRDeltaGamma &rhs);
	LARiskConfigurationYieldIRDeltaGamma &operator=(const LARiskConfigurationYieldIRDeltaGamma &rhs);

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

