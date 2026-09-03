/*! @file
    @brief  FX delta setup class
*/
//  2008, AlgoQuantHub.
#ifndef AQLRiskConfigurationFXShiftDeltaOptionAnalytic_h
#define AQLRiskConfigurationFXShiftDeltaOptionAnalytic_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLRiskConfigurationFXShiftDeltaOptionAnalytic.h
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

#include "AQLRiskConfigurationFXShiftDelta.h"


//===================== Class Declare AQLRiskConfigurationFXShiftDeltaOptionAnalytic==================================
/*! 
    @brief  FX delta setup class

*/
class AQLRiskConfigurationFXShiftDeltaOptionAnalytic : public AQLRiskConfigurationFXShiftDelta
{
public:
	// constructor
	explicit AQLRiskConfigurationFXShiftDeltaOptionAnalytic(const AQLString& risktype);
	// destructor
	virtual ~AQLRiskConfigurationFXShiftDeltaOptionAnalytic(void);
	// copy constructor
	AQLRiskConfigurationFXShiftDeltaOptionAnalytic(const AQLRiskConfigurationFXShiftDeltaOptionAnalytic &rhs);
	AQLRiskConfigurationFXShiftDeltaOptionAnalytic &operator=(const AQLRiskConfigurationFXShiftDeltaOptionAnalytic &rhs);

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
	// get get FX shift vals
	virtual  DoubleArray  getBaseShiftVals(const AQLString &ccy) const {(void)ccy; return DoubleArray();};
	//==============================================
	// create basescenario object
	virtual std::vector<AQLObject *> createBaseScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const { (void)ccy, (void)dataInstance, (void)index; return std::vector<AQLObject *>(0); }
	//==============================================
	// get baseoutputname
	virtual  AQLString  getBaseOutPutName(const AQLString &ccy, int index) const  { (void)ccy, (void)index; return AQ_NO_DATA; }
	//==============================================
	// get baseoperator
	virtual  AQLString  getBaseOperator(void) const { return AQ_NO_DATA; }	
	//==============================================
	// get basecoefficient
	virtual  AQLString  getBaseCoefficient(const AQLString &ccy) const  { (void)ccy; return AQ_NO_DATA; }
	////==============================================
	// get outputname1
	virtual  AQLString  getOutPutName1(const AQLString &ccy, int index) const {(void)index; return AQLRiskConfigurationFXShiftDelta::getOutPutName1(ccy);}
	
};


#endif
