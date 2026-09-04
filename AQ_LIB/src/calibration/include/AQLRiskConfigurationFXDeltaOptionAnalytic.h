/*! @file
    @brief  FX delta setup class
*/
#ifndef AQLRiskConfigurationFXDeltaOptionAnalytic_h
#define AQLRiskConfigurationFXDeltaOptionAnalytic_h
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLRiskConfigurationFXDeltaEx1.h"


//===================== Class Declare AQLRiskConfigurationFXDeltaOptionAnalytic==================================
/*! 
    @brief  FX delta setup class

*/
class AQLRiskConfigurationFXDeltaOptionAnalytic : public AQLRiskConfigurationFXDeltaEx1
{
public:
	// constructor
	explicit AQLRiskConfigurationFXDeltaOptionAnalytic(const AQLString& risktype);
	// destructor
	virtual ~AQLRiskConfigurationFXDeltaOptionAnalytic(void);
	// copy constructor
	AQLRiskConfigurationFXDeltaOptionAnalytic(const AQLRiskConfigurationFXDeltaOptionAnalytic &rhs);
	AQLRiskConfigurationFXDeltaOptionAnalytic &operator=(const AQLRiskConfigurationFXDeltaOptionAnalytic &rhs);

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
