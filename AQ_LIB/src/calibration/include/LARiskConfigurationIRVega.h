/*! @file
    @brief IR vega setup class
*/
//  2007, AlgoQuantHub.
#ifndef LARiskConfigurationIRVega_h
#define LARiskConfigurationIRVega_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationIRVega.h
//
//  DESCRIPTION :        IR Vega setup class 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LARiskConfigurationVolatility.h"



class LAString;
class LADataInstance;
class LARiskConfigurationVolCreatorLMM;
//===================== Class Declare LARiskConfigurationIRVega==================================
/*! 
    @brief IR Vega setup class
	

*/
class LARiskConfigurationIRVega : public LARiskConfigurationVolatility
{
public:
	// constructor
	explicit LARiskConfigurationIRVega(void);
	// destructor
	virtual ~LARiskConfigurationIRVega(void);
	// copy constructor
	LARiskConfigurationIRVega(const LARiskConfigurationIRVega &rhs);
	LARiskConfigurationIRVega &operator=(const LARiskConfigurationIRVega &rhs);

protected:
friend class LARiskConfigurationVolCreatorLMM;
	//==============================================
	// get  target names 
	virtual  LAString getTargetNames(const LAString &ccy, LADataInstance &dataInstance)  const;	
	//==============================================
	// create scenario volatility object
	virtual std::vector<LAObject *> createVolatilityEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1, int index = 0) const;
	//==============================================
	// get operator1
	virtual  LAString  getOperator1(void) const;	
	//==============================================
	// get coefficient1
	virtual  LAString  getCoefficient1(const LAString &ccy) const;
	//==============================================
	// get outputname1
	virtual  LAString  getOutPutName1(const LAString &ccy) const;
	//==============================================
	// get operator2
	virtual  LAString  getOperator2(void) const;	
	//==============================================
	// get coefficient2
	virtual  LAString  getCoefficient2(const LAString &ccy) const;	
	//==============================================
	// get outputname2
	virtual  LAString  getOutPutName2(const LAString &ccy) const;
	//==============================================
	// get shift type
	virtual  LAString  getShiftType(const LAString &ccy) const;
	//==============================================
	// get bump direction
	virtual  LAString  getBumpDirection(const LAString &ccy) const;
	//==============================================
	// get risk name
	virtual  LAString getRiskName(void) const;
	//==============================================
	// check target currencies
	virtual  LAString getTargetCurrencies() const;
	//==============================================
	// isgridsensitivity
	virtual  bool  isGridSensitivity(const LAString &ccy) const;
	//==============================================
	// isparallelshift
	virtual  bool isParallelShift(const LAString &ccy) const;
	//==============================================
	// get grid term
	virtual std::vector<LAString> getGridTerm(const LAString &ccy) const;
	//==============================================
	// isfileuse
	virtual  bool isShiftValFileUse(const LAString &ccy) const;
	//==============================================
	// get scenario1 shift
	virtual double getScenario1ShiftValue(const LAString &ccy) const;
	//==============================================
	// get scenario2 shift
	virtual double getScenario2ShiftValue(const LAString &ccy) const;
	//==============================================
	// get divunit
	virtual double getDivUnit(const LAString &ccy) const;
	//==============================================
	// get double matrix
	virtual DoubleMatrix getCoordinatesMatrix(const LAString &ccy) const;

};


#endif
