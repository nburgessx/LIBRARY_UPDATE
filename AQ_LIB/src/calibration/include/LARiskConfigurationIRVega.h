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



class AQLString;
class AQLDataInstance;
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
	virtual  AQLString getTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance)  const;	
	//==============================================
	// create scenario volatility object
	virtual std::vector<AQLObject *> createVolatilityEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1, int index = 0) const;
	//==============================================
	// get operator1
	virtual  AQLString  getOperator1(void) const;	
	//==============================================
	// get coefficient1
	virtual  AQLString  getCoefficient1(const AQLString &ccy) const;
	//==============================================
	// get outputname1
	virtual  AQLString  getOutPutName1(const AQLString &ccy) const;
	//==============================================
	// get operator2
	virtual  AQLString  getOperator2(void) const;	
	//==============================================
	// get coefficient2
	virtual  AQLString  getCoefficient2(const AQLString &ccy) const;	
	//==============================================
	// get outputname2
	virtual  AQLString  getOutPutName2(const AQLString &ccy) const;
	//==============================================
	// get shift type
	virtual  AQLString  getShiftType(const AQLString &ccy) const;
	//==============================================
	// get bump direction
	virtual  AQLString  getBumpDirection(const AQLString &ccy) const;
	//==============================================
	// get risk name
	virtual  AQLString getRiskName(void) const;
	//==============================================
	// check target currencies
	virtual  AQLString getTargetCurrencies() const;
	//==============================================
	// isgridsensitivity
	virtual  bool  isGridSensitivity(const AQLString &ccy) const;
	//==============================================
	// isparallelshift
	virtual  bool isParallelShift(const AQLString &ccy) const;
	//==============================================
	// get grid term
	virtual std::vector<AQLString> getGridTerm(const AQLString &ccy) const;
	//==============================================
	// isfileuse
	virtual  bool isShiftValFileUse(const AQLString &ccy) const;
	//==============================================
	// get scenario1 shift
	virtual double getScenario1ShiftValue(const AQLString &ccy) const;
	//==============================================
	// get scenario2 shift
	virtual double getScenario2ShiftValue(const AQLString &ccy) const;
	//==============================================
	// get divunit
	virtual double getDivUnit(const AQLString &ccy) const;
	//==============================================
	// get double matrix
	virtual DoubleMatrix getCoordinatesMatrix(const AQLString &ccy) const;

};


#endif
