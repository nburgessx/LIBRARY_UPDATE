/*! @file
    @brief Yield basis setup class
*/
//  2007, Mizuho International London.
#ifndef LARiskConfigurationYieldBasis_h
#define LARiskConfigurationYieldBasis_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationYieldBasis.h
//
//  DESCRIPTION :       Yield basis setup class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif


#include "LARiskConfigurationYield.h"

//===================== Class Declare LARiskConfigurationYieldBasis==================================
/*! 
    @brief Yield basis setup class
	

*/
class LARiskConfigurationYieldBasis : public LARiskConfigurationYield
{
public:
	// constructor
	explicit LARiskConfigurationYieldBasis(void);
	// destructor
	virtual ~LARiskConfigurationYieldBasis(void);
	// copy constructor
	LARiskConfigurationYieldBasis(const LARiskConfigurationYieldBasis &rhs);
	LARiskConfigurationYieldBasis &operator=(const LARiskConfigurationYieldBasis &rhs);

protected:
	//==============================================
	// create scenario yield object
	virtual std::vector<LAObject *> createYieldEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// get operator1
	virtual  LAString  getOperator1(void) const;	
	//==============================================
	// get operator2
	virtual  LAString  getOperator2(void) const;	
	//==============================================
	// get coefficient1
	virtual  LAString  getCoefficient1(const LAString &ccy) const ;
	//==============================================
	// get coefficient2
	virtual  LAString  getCoefficient2(const LAString &ccy) const;	
	//==============================================
	// get outputname1
	virtual  LAString  getOutPutName1(const LAString &ccy) const;
	//==============================================
	// get outputname2
	virtual  LAString  getOutPutName2(const LAString &ccy) const;
	//==============================================
	// isgridsensitivity
	virtual  bool isGridSensitivity(const LAString &ccy) const;
	//==============================================
	// isparallelshift
	virtual  bool isParallelShift(const LAString &ccy) const;
	//==============================================
	// return risk name
	virtual  LAString  getRiskName(void) const;
	//==============================================
	// get grid term
	virtual std::vector<LAString> getGridTerm(const LAString &ccy) const;
	//==============================================
	// get bucket grid term
	virtual std::vector<LAString> getBucketGridTerm(const LAString &ccy) const;
	//==============================================
	// get scenario1 parallel shift
	virtual double getScenario1ParallelShift(const LAString &ccy) const;
	//==============================================
	// get scenario2 parallel shift
	virtual double getScenario2ParallelShift(const LAString &ccy) const;
	//==============================================
	// get scenario1 grid shift
	virtual DoubleArray getScenario1GridShift(const LAString &ccy) const;
	//==============================================
	// get scenario2 grid shift
	virtual DoubleArray getScenario2GridShift(const LAString &ccy) const;
	//==============================================
	// get grid calc buffer
	virtual int getGridCalcBuffer() const;

	//==============================================
	// check target currency or fx
	virtual  bool isTarget(const LAString &ccy) const;
	//==============================================
	// get target currencies
	virtual  LAString getTargetCurrencies() const;
	//==============================================
	// get calibration target currencies
	virtual  LAString getCalibTargetCurrencies() const;
	// get bump direction
	virtual  LAString getBumpDirection(const LAString &ccy) const {(void)ccy; return RISK_BUMPDIRECTION_UPDOWNSHIFT; }
	// get coefficient ratio1
	virtual double getCoefficientRatio1(const LAString& ccy, LAObject& e, LADataInstance& dataInstance) const;
	//==============================================
	// get coefficient ratio2
	virtual double getCoefficientRatio2(const LAString& ccy, LAObject& e, LADataInstance& dataInstance) const;
	//==============================================
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const LAString& ccy) const;

private:
	LAString mBasisCurrency; // basis currency

};
#endif
