/*! @file
    @brief  FX delta setup class
*/
//  2008, AlgoQuantHub.
#ifndef LARiskConfigurationFXDeltaGamma_h
#define LARiskConfigurationFXDeltaGamma_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationFXDeltaGamma.h
//
//  DESCRIPTION :         FX delta gamma setup class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#include "LARiskConfigurationFXDeltaEx1.h"


class LARiskConfigurationFXDeltaGamma : public LARiskConfigurationFXDeltaEx1
{
public:
	// constructor
	explicit LARiskConfigurationFXDeltaGamma(void);
	// destructor
	virtual ~LARiskConfigurationFXDeltaGamma(void);
	// copy constructor
	LARiskConfigurationFXDeltaGamma(const LARiskConfigurationFXDeltaGamma &rhs);
	LARiskConfigurationFXDeltaGamma &operator=(const LARiskConfigurationFXDeltaGamma &rhs);

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
