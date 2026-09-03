/*! @file
    @brief Volatility setup class for calc parallel shift risk 
*/
//  2008, AlgoQuantHub.
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
	virtual  AQLString getTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance)  const;	
	//==============================================
	// create scenario volatility object
	virtual std::vector<AQLObject *> createVolatilityEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1, int index = 0) const;
	//==============================================
	// create scenario volatility object
	virtual std::vector<AQLObject *> createVolatilityEntityOld(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1) const;
	////==============================================
	//// get operator2
	//virtual  AQLString  getOperator2(void) const;	
	////==============================================
	//// get coefficient2
	//virtual  AQLString  getCoefficient2(const AQLString &ccy) const;	
	////==============================================
	//// get outputname2
	//virtual  AQLString  getOutPutName2(const AQLString &ccy) const;
	//==============================================
	// isgridsensitivity
	virtual  bool  isGridSensitivity(const AQLString &ccy) const { (void)ccy; return false; }
	//==============================================
	// isparallelshift
	virtual  bool isParallelShift(const AQLString &ccy) const { (void)ccy; return true; }
	//==============================================
	// get grid term
	virtual std::vector<AQLString> getGridTerm(const AQLString &ccy) const { (void)ccy; return std::vector<AQLString>(0); }
	//==============================================
	// get bucket grid term
	virtual std::vector<AQLString> getBucketGridTerm(const AQLString &ccy) const { (void)ccy;std::vector<AQLString> ret; ret.push_back(AQ_NO_DATA); return ret; }
	//==============================================
	// get scenario1 parallel shift
	virtual double getParallelShiftVal(const AQLString &ccy) const = 0;
	//==============================================
	// get grid term
	virtual AQLString getParallelShiftTerm(const AQLString &ccy) const { (void)ccy; return AQ_NO_DATA; }

};


#endif

