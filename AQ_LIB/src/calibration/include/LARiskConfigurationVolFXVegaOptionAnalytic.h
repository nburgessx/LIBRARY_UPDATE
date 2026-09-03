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



class LAString;
class LADataInstance;
//===================== Class Declare LARiskConfigurationVolFXVegaOptionAnalytic==================================
/*! 
    @brief FX Vega setup class
	

*/
class LARiskConfigurationVolFXVegaOptionAnalytic : public LARiskConfigurationVolFXVega
{
public:
	// constructor
	explicit LARiskConfigurationVolFXVegaOptionAnalytic(const LAString& risktype);
	// destructor
	virtual ~LARiskConfigurationVolFXVegaOptionAnalytic(void);
	// copy constructor
	LARiskConfigurationVolFXVegaOptionAnalytic(const LARiskConfigurationVolFXVegaOptionAnalytic &rhs);
	LARiskConfigurationVolFXVegaOptionAnalytic &operator=(const LARiskConfigurationVolFXVegaOptionAnalytic &rhs);

protected:

	//==============================================
	// get target names 
	virtual  LAString getTargetNames(const LAString &fx, LADataInstance &dataInstance)  const {fx; return MLIB_NO_DATA;};
	//==============================================
	// create risk object
	virtual std::vector<std::pair<LAString, std::vector<LAObject *> > > createRiskEntity(LAObjectPool &objPool) const;
	//==============================================
	// setup target names 
	virtual  void setUpTargetNames(const LAString &ccy, LAObject &e, LADataInstance &dataInstance) const;	
	//==============================================
	// create scenario1 object 
	virtual  std::vector<LAObject *> createScenario1Entity(const LAString &fx, LADataInstance &dataInstance, int index) const;
	//==============================================
	// create scenario2 object
	virtual  std::vector<LAObject *> createScenario2Entity(const LAString &fx, LADataInstance &dataInstance, int index) const;
	//==============================================
	// get coefficient1
	virtual  LAString  getCoefficient1(const LAString &fx) const;
	//==============================================
	// get risk name
	virtual  LAString getRiskName(void) const;
	//==============================================
	// get is wave
	virtual  bool  isWave(const LAString &fx) const;
	//==============================================
	// get is gridsensitivity
	virtual  bool isGridSensitivity(const LAString &fx) const;
	//==============================================
	// get ShiftValForOptionAnalytic
	virtual double getShiftValForOptionAnalytic(const LAString &fx) const;
	
};


#endif
