/*! @file
    @brief  FX delta setup class
*/
//  2008, Mizuho International London.
#ifndef LARiskConfigurationThetaOptionAnalytic_h
#define LARiskConfigurationThetaOptionAnalytic_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationThetaOptionAnalytic.h
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

#include "LARiskConfigurationTheta.h"


//===================== Class Declare LARiskConfigurationThetaOptionAnalytic==================================
/*! 
    @brief  FX delta setup class

*/
class LARiskConfigurationThetaOptionAnalytic : public LARiskConfigurationTheta
{
public:
	// constructor
	explicit LARiskConfigurationThetaOptionAnalytic(const LAString& risktype);
	// destructor
	virtual ~LARiskConfigurationThetaOptionAnalytic(void);
	// copy constructor
	LARiskConfigurationThetaOptionAnalytic(const LARiskConfigurationThetaOptionAnalytic &rhs);
	LARiskConfigurationThetaOptionAnalytic &operator=(const LARiskConfigurationThetaOptionAnalytic &rhs);

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
	//==============================================
	virtual  LAString  getShiftType(const LAString &fx) const {(void)fx; return RISK_SHIFTTYPE_DIFF;};
	//==============================================
	// IsSucceedAnalyticalRiskType
	virtual bool IsSucceedAnalyticalRiskType(const LAString& fx, LAObject& e) const {(void)fx;(void)e;return true;};
	
	
};


#endif
