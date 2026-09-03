/*! @file
    @brief  FX delta setup class
*/
//  2008, AlgoQuantHub.
#ifndef AQLRiskConfigurationThetaOptionAnalytic_h
#define AQLRiskConfigurationThetaOptionAnalytic_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLRiskConfigurationThetaOptionAnalytic.h
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

#include "AQLRiskConfigurationTheta.h"


//===================== Class Declare AQLRiskConfigurationThetaOptionAnalytic==================================
/*! 
    @brief  FX delta setup class

*/
class AQLRiskConfigurationThetaOptionAnalytic : public AQLRiskConfigurationTheta
{
public:
	// constructor
	explicit AQLRiskConfigurationThetaOptionAnalytic(const AQLString& risktype);
	// destructor
	virtual ~AQLRiskConfigurationThetaOptionAnalytic(void);
	// copy constructor
	AQLRiskConfigurationThetaOptionAnalytic(const AQLRiskConfigurationThetaOptionAnalytic &rhs);
	AQLRiskConfigurationThetaOptionAnalytic &operator=(const AQLRiskConfigurationThetaOptionAnalytic &rhs);

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
	//==============================================
	virtual  AQLString  getShiftType(const AQLString &fx) const {(void)fx; return RISK_SHIFTTYPE_DIFF;};
	//==============================================
	// IsSucceedAnalyticalRiskType
	virtual bool IsSucceedAnalyticalRiskType(const AQLString& fx, AQLObject& e) const {(void)fx;(void)e;return true;};
	
	
};


#endif
