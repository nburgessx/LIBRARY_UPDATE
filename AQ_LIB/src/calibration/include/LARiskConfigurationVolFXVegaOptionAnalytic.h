/*! @file
    @brief FX vega setup class
*/
//  2008, AlgoQuantHub.
#ifndef LARiskConfigurationVolFXVegaOptionAnalytic_h
#define LARiskConfigurationVolFXVegaOptionAnalytic_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationVolFXVegaOptionAnalytic.h
//
//  DESCRIPTION :        FX Vega setup class 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LARiskConfigurationVolFXVega.h"



class AQLString;
class AQLDataInstance;
//===================== Class Declare LARiskConfigurationVolFXVegaOptionAnalytic==================================
/*! 
    @brief FX Vega setup class
	

*/
class LARiskConfigurationVolFXVegaOptionAnalytic : public LARiskConfigurationVolFXVega
{
public:
	// constructor
	explicit LARiskConfigurationVolFXVegaOptionAnalytic(const AQLString& risktype);
	// destructor
	virtual ~LARiskConfigurationVolFXVegaOptionAnalytic(void);
	// copy constructor
	LARiskConfigurationVolFXVegaOptionAnalytic(const LARiskConfigurationVolFXVegaOptionAnalytic &rhs);
	LARiskConfigurationVolFXVegaOptionAnalytic &operator=(const LARiskConfigurationVolFXVegaOptionAnalytic &rhs);

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
