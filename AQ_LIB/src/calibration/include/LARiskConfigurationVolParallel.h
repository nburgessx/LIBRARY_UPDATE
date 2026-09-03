/*! @file
    @brief Volatility setup class for calc parallel shift risk 
*/
//  2008, Mizuho International London.
#ifndef LARiskConfigurationVolParallel_h
#define LARiskConfigurationVolParallel_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationVolParallel.h
//
//  DESCRIPTION :       Volatility setup class for calc parallel shift risk 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LARiskConfigurationVolatility.h"

//===================== Class Declare LARiskConfigurationVolParallel==================================
/*! 
    @brief Volatility setup class for calc parallel shift risk
	

*/
class LARiskConfigurationVolParallel : public LARiskConfigurationVolatility
{
public:
	// constructor
	explicit LARiskConfigurationVolParallel(bool fxFlg = false);
	// destructor
	virtual ~LARiskConfigurationVolParallel(void);
	// copy constructor
	LARiskConfigurationVolParallel(const LARiskConfigurationVolParallel &rhs);
	LARiskConfigurationVolParallel &operator=(const LARiskConfigurationVolParallel &rhs);

protected:
	//==============================================
	// get  target names 
	virtual  LAString getTargetNames(const LAString &ccy, LADataInstance &dataInstance)  const;	
	//==============================================
	// create scenario volatility object
	virtual std::vector<LAObject *> createVolatilityEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1, int index = 0) const;
	//==============================================
	// create scenario volatility object
	virtual std::vector<LAObject *> createVolatilityEntityOld(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1) const;
	////==============================================
	//// get operator2
	//virtual  LAString  getOperator2(void) const;	
	////==============================================
	//// get coefficient2
	//virtual  LAString  getCoefficient2(const LAString &ccy) const;	
	////==============================================
	//// get outputname2
	//virtual  LAString  getOutPutName2(const LAString &ccy) const;
	//==============================================
	// isgridsensitivity
	virtual  bool  isGridSensitivity(const LAString &ccy) const { (void)ccy; return false; }
	//==============================================
	// isparallelshift
	virtual  bool isParallelShift(const LAString &ccy) const { (void)ccy; return true; }
	//==============================================
	// get grid term
	virtual std::vector<LAString> getGridTerm(const LAString &ccy) const { (void)ccy; return std::vector<LAString>(0); }
	//==============================================
	// get bucket grid term
	virtual std::vector<LAString> getBucketGridTerm(const LAString &ccy) const { (void)ccy;std::vector<LAString> ret; ret.push_back(MLIB_NO_DATA); return ret; }
	//==============================================
	// get scenario1 parallel shift
	virtual double getParallelShiftVal(const LAString &ccy) const = 0;
	//==============================================
	// get grid term
	virtual LAString getParallelShiftTerm(const LAString &ccy) const { (void)ccy; return MLIB_NO_DATA; }

};


#endif

