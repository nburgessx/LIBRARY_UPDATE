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
	explicit LARiskConfigurationFXShiftDeltaOptionAnalytic(const LAString& risktype);
	// destructor
	virtual ~LARiskConfigurationFXShiftDeltaOptionAnalytic(void);
	// copy constructor
	LARiskConfigurationFXShiftDeltaOptionAnalytic(const LARiskConfigurationFXShiftDeltaOptionAnalytic &rhs);
	LARiskConfigurationFXShiftDeltaOptionAnalytic &operator=(const LARiskConfigurationFXShiftDeltaOptionAnalytic &rhs);

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
	// get get FX shift vals
	virtual  DoubleArray  getBaseShiftVals(const LAString &ccy) const {(void)ccy; return DoubleArray();};
	//==============================================
	// create basescenario object
	virtual std::vector<LAObject *> createBaseScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const { (void)ccy, (void)dataInstance, (void)index; return std::vector<LAObject *>(0); }
	//==============================================
	// get baseoutputname
	virtual  LAString  getBaseOutPutName(const LAString &ccy, int index) const  { (void)ccy, (void)index; return MLIB_NO_DATA; }
	//==============================================
	// get baseoperator
	virtual  LAString  getBaseOperator(void) const { return MLIB_NO_DATA; }	
	//==============================================
	// get basecoefficient
	virtual  LAString  getBaseCoefficient(const LAString &ccy) const  { (void)ccy; return MLIB_NO_DATA; }
	////==============================================
	// get outputname1
	virtual  LAString  getOutPutName1(const LAString &ccy, int index) const {(void)index; return LARiskConfigurationFXShiftDelta::getOutPutName1(ccy);}
	
};


#endif
