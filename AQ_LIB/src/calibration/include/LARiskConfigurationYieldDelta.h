/*! @file
    @brief Yield delta setup class
*/
//  2007, AlgoQuantHub.
#ifndef LARiskConfigurationYieldDelta_h
#define LARiskConfigurationYieldDelta_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationYieldDelta.h
//
//  DESCRIPTION :       Yield delta setup class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LARiskConfigurationYield.h"

//===================== Class Declare LARiskConfigurationYieldDelta==================================
/*! 
    @brief Yield delta setup class
	

*/
class LARiskConfigurationYieldDelta : public LARiskConfigurationYield
{
public:
	// constructor
	explicit LARiskConfigurationYieldDelta(void);
	// destructor
	virtual ~LARiskConfigurationYieldDelta(void);
	// copy constructor
	LARiskConfigurationYieldDelta(const LARiskConfigurationYieldDelta &rhs);
	LARiskConfigurationYieldDelta &operator=(const LARiskConfigurationYieldDelta &rhs);

protected:
	//==============================================
	// create scenario yield object
	virtual std::vector<AQLObject *> createYieldEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// get operator1
	virtual  AQLString  getOperator1(void) const;	
	//==============================================
	// get operator2
	virtual  AQLString  getOperator2(void) const;	
	//==============================================
	// get coefficient1
	virtual  AQLString  getCoefficient1(const AQLString &ccy) const ;
	//==============================================
	// get coefficient2
	virtual  AQLString  getCoefficient2(const AQLString &ccy) const;	
	//==============================================
	// get outputname1
	virtual  AQLString  getOutPutName1(const AQLString &ccy) const;
	//==============================================
	// get outputname2
	virtual  AQLString  getOutPutName2(const AQLString &ccy) const;
	//==============================================
	// isgridsensitivity
	virtual  bool isGridSensitivity(const AQLString &ccy) const;
	//==============================================
	// isparallelshift
	virtual  bool isParallelShift(const AQLString &ccy) const;
	//==============================================
	// return risk name
	virtual  AQLString  getRiskName(void) const;
	//==============================================
	// get grid term
	virtual std::vector<AQLString> getGridTerm(const AQLString &ccy) const;
	//==============================================
	// get bucket grid term
	virtual std::vector<AQLString> getBucketGridTerm(const AQLString &ccy) const;
	//==============================================
	// get scenario1 parallel shift
	virtual double getScenario1ParallelShift(const AQLString &ccy) const;
	//==============================================
	// get scenario2 parallel shift
	virtual double getScenario2ParallelShift(const AQLString &ccy) const;
	//==============================================
	// get scenario1 grid shift
	virtual DoubleArray getScenario1GridShift(const AQLString &ccy) const;
	//==============================================
	// get scenario2 grid shift
	virtual DoubleArray getScenario2GridShift(const AQLString &ccy) const;
	//==============================================
	// get grid calc buffer
	virtual int getGridCalcBuffer() const;
	//==============================================
	// get  target currencies
	virtual  AQLString getTargetCurrencies() const;
	//==============================================
	// get calibration target currencies
	virtual  AQLString getCalibTargetCurrencies() const;
	//==============================================
	// get bump direction
	virtual  AQLString getBumpDirection(const AQLString &ccy) const {(void)ccy; return RISK_BUMPDIRECTION_UPDOWNSHIFT; }
	// get coefficient ratio1
	virtual double getCoefficientRatio1(const AQLString& ccy, AQLObject& e, AQLDataInstance& dataInstance) const;
	//==============================================
	// get coefficient ratio2
	virtual double getCoefficientRatio2(const AQLString& ccy, AQLObject& e, AQLDataInstance& dataInstance) const;
	//==============================================
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const AQLString& ccy) const;


};
#endif
