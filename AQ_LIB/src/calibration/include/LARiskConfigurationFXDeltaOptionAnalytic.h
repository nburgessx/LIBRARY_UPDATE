/*! @file
    @brief  FX delta setup class
*/
//  2008, AlgoQuantHub.
#ifndef LARiskConfigurationFXDeltaOptionAnalytic_h
#define LARiskConfigurationFXDeltaOptionAnalytic_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationFXDeltaOptionAnalytic.h
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

#include "LARiskConfigurationFXDeltaEx1.h"


//===================== Class Declare LARiskConfigurationFXDeltaOptionAnalytic==================================
/*! 
    @brief  FX delta setup class

*/
class LARiskConfigurationFXDeltaOptionAnalytic : public LARiskConfigurationFXDeltaEx1
{
public:
	// constructor
	explicit LARiskConfigurationFXDeltaOptionAnalytic(const AQLString& risktype);
	// destructor
	virtual ~LARiskConfigurationFXDeltaOptionAnalytic(void);
	// copy constructor
	LARiskConfigurationFXDeltaOptionAnalytic(const LARiskConfigurationFXDeltaOptionAnalytic &rhs);
	LARiskConfigurationFXDeltaOptionAnalytic &operator=(const LARiskConfigurationFXDeltaOptionAnalytic &rhs);

protected:
	//==============================================
	// get target names 
	virtual  AQLString getTargetNames(const AQLString &fx, AQLDataInstance &dataInstance)  const {fx; return AQ_NO_DATA;};
	//==============================================
	// create risk object
	virtual std::vector<std::pair<AQLString, std::vector<AQLObject *> > > createRiskEntity(AQLObjectPool &objPool) const;
	//==============================================
	// setup target names 
	virtual  void setUpTargetNames(const AQLString &ccy, AQLObject &e, AQLDataInstance &dataInstance) const;	
	//==============================================
	// create scenario1 object 
	virtual  std::vector<AQLObject *> createScenario1Entity(const AQLString &fx, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// create scenario2 object
	virtual  std::vector<AQLObject *> createScenario2Entity(const AQLString &fx, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// get coefficient1
	virtual  AQLString  getCoefficient1(const AQLString &fx) const;
	//==============================================
	// get risk name
	virtual  AQLString getRiskName(void) const;
	//==============================================
	// get ShiftValForOptionAnalytic
	virtual double getShiftValForOptionAnalytic(const AQLString &fx) const ;

	
};


#endif
