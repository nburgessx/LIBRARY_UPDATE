/*! @file
    @brief FX vega setup class
*/
#ifndef AQLRiskConfigurationVolFXVegaOptionAnalytic_h
#define AQLRiskConfigurationVolFXVegaOptionAnalytic_h
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLRiskConfigurationVolFXVega.h"



class AQLString;
class AQLDataInstance;
//===================== Class Declare AQLRiskConfigurationVolFXVegaOptionAnalytic==================================
/*! 
    @brief FX Vega setup class
	

*/
class AQLRiskConfigurationVolFXVegaOptionAnalytic : public AQLRiskConfigurationVolFXVega
{
public:
	// constructor
	explicit AQLRiskConfigurationVolFXVegaOptionAnalytic(const AQLString& risktype);
	// destructor
	virtual ~AQLRiskConfigurationVolFXVegaOptionAnalytic(void);
	// copy constructor
	AQLRiskConfigurationVolFXVegaOptionAnalytic(const AQLRiskConfigurationVolFXVegaOptionAnalytic &rhs);
	AQLRiskConfigurationVolFXVegaOptionAnalytic &operator=(const AQLRiskConfigurationVolFXVegaOptionAnalytic &rhs);

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
	// get is wave
	virtual  bool  isWave(const AQLString &fx) const;
	//==============================================
	// get is gridsensitivity
	virtual  bool isGridSensitivity(const AQLString &fx) const;
	//==============================================
	// get ShiftValForOptionAnalytic
	virtual double getShiftValForOptionAnalytic(const AQLString &fx) const;
	
};


#endif
