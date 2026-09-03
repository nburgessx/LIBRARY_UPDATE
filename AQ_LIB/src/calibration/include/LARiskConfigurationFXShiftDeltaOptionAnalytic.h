/*! @file
    @brief  FX delta setup class
*/
//  2008, AlgoQuantHub.
#ifndef LARiskConfigurationFXShiftDeltaOptionAnalytic_h
#define LARiskConfigurationFXShiftDeltaOptionAnalytic_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationFXShiftDeltaOptionAnalytic.h
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

#include "LARiskConfigurationFXShiftDelta.h"


//===================== Class Declare LARiskConfigurationFXShiftDeltaOptionAnalytic==================================
/*! 
    @brief  FX delta setup class

*/
class LARiskConfigurationFXShiftDeltaOptionAnalytic : public LARiskConfigurationFXShiftDelta
{
public:
	// constructor
	explicit LARiskConfigurationFXShiftDeltaOptionAnalytic(const AQLString& risktype);
	// destructor
	virtual ~LARiskConfigurationFXShiftDeltaOptionAnalytic(void);
	// copy constructor
	LARiskConfigurationFXShiftDeltaOptionAnalytic(const LARiskConfigurationFXShiftDeltaOptionAnalytic &rhs);
	LARiskConfigurationFXShiftDeltaOptionAnalytic &operator=(const LARiskConfigurationFXShiftDeltaOptionAnalytic &rhs);

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
	virtual  AQLString  getOutPutName1(const AQLString &ccy, int index) const {(void)index; return LARiskConfigurationFXShiftDelta::getOutPutName1(ccy);}
	
};


#endif
