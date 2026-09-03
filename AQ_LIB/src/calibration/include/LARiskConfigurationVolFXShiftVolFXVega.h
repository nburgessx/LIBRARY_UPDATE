/*! @file
    @brief FX vega setup class
*/
//  2008, Mizuho International London.
#ifndef LARiskConfigurationVolFXShiftVolFXVega_h
#define LARiskConfigurationVolFXShiftVolFXVega_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationVolFXShiftVolFXVega.h
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
class LARiskConfigurationVolCreatorLMM;
//===================== Class Declare LARiskConfigurationVolFXShiftVolFXVega==================================
/*! 
    @brief FX Vega setup class
	

*/
class LARiskConfigurationVolFXShiftVolFXVega : public LARiskConfigurationVolFXVega
{
public:
	// constructor
	explicit LARiskConfigurationVolFXShiftVolFXVega(void);
	// destructor
	virtual ~LARiskConfigurationVolFXShiftVolFXVega(void);
	// copy constructor
	LARiskConfigurationVolFXShiftVolFXVega(const LARiskConfigurationVolFXShiftVolFXVega &rhs);
	LARiskConfigurationVolFXShiftVolFXVega &operator=(const LARiskConfigurationVolFXShiftVolFXVega &rhs);

protected:
friend class LARiskConfigurationVolCreatorLMM;
	//==============================================
	// get outputname1
	virtual  LAString  getOutPutName1(const LAString &fx) const;
	//==============================================
	// get outputname1
	virtual  LAString  getOutPutName1(const LAString &ccy, int index) const;
	//==============================================
	// get shift type
	virtual  LAString  getShiftType(const LAString &fx) const;
	//==============================================
	// get bump direction
	virtual  LAString  getBumpDirection(const LAString &fx) const;
	//==============================================
	// get risk name
	virtual  LAString getRiskName(void) const;
	//==============================================
	// check target currencies
	virtual  LAString getTargetCurrencies() const;
	//==============================================
	// isgridsensitivity
	virtual  bool  isGridSensitivity(const LAString &fx) const;
	//==============================================
	// isparallelshift
	virtual  bool isParallelShift(const LAString &fx) const;
	//==============================================
	// get deltatype
	virtual LAString getDeltaType(const LAString &ccy) const;
	//==============================================
	// get grid term
	virtual std::vector<LAString> getGridTerm(const LAString &fx) const;
	//==============================================
	// get bucket grid term
	virtual std::vector<LAString> getBucketGridTerm(const LAString &fx) const;
	//==============================================
	// get property bucket grid term
	virtual LAString getPropertyBucketGridTerm(const LAString &fx) const;
	//==============================================
	// get divunit
	virtual double getDivUnit(const LAString &fx) const;
	//==============================================
	// get is wave
	virtual  bool  isWave(const LAString &fx) const;
	//==============================================
	// get scenario1 shift
	virtual double getScenario1ShiftValue(const LAString &fx) const;
	//==============================================
	// get get shift vals
	virtual  DoubleArray  getBaseShiftVals(const LAString &ccy) const;
	//==============================================
	// get base shift val
	virtual  double getBaseVolVal(const LAString &ccy, int index) const;
	//==============================================
	// get baseoperator
	virtual  LAString  getBaseOperator(void) const;	
	//==============================================
	// get basecoefficient
	virtual  LAString  getBaseCoefficient(const LAString &ccy) const;
	//==============================================
	// get baseoutputname
	virtual  LAString  getBaseOutPutName(const LAString &ccy, int index) const;
	//==============================================
	// create basescenario object
	virtual std::vector<LAObject *> createBaseScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const;
	//==============================================
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const LAString& fx) const;
	


};


#endif
