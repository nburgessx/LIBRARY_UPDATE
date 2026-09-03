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
	explicit LARiskConfigurationFXDeltaOptionAnalytic(const LAString& risktype);
	// destructor
	virtual ~LARiskConfigurationFXDeltaOptionAnalytic(void);
	// copy constructor
	LARiskConfigurationFXDeltaOptionAnalytic(const LARiskConfigurationFXDeltaOptionAnalytic &rhs);
	LARiskConfigurationFXDeltaOptionAnalytic &operator=(const LARiskConfigurationFXDeltaOptionAnalytic &rhs);

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
	// get ShiftValForOptionAnalytic
	virtual double getShiftValForOptionAnalytic(const LAString &fx) const ;

	
};


#endif
