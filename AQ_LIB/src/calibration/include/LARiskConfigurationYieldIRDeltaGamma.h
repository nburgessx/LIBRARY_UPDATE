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
	virtual  LAString  getOperator2(void) const;	
	//==============================================
	// get coefficient1
	virtual  LAString  getCoefficient1(const LAString &ccy) const;
	//==============================================
	// get coefficient2
	virtual  LAString  getCoefficient2(const LAString &ccy) const;
	//==============================================
	// get outputname2
	virtual  LAString  getOutPutName2(const LAString &ccy) const;
	//==============================================
	// get bump direction
	virtual  LAString getBumpDirection(const LAString &ccy) const;
	//==============================================
	// get property bump direction
	LAString getPropBumpDirection(const LAString &ccy) const;

};

