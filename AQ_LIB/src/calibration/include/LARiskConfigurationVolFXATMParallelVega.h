/*! @file
    @brief FX atm parallel vega setup class 
*/
//  2007, AlgoQuantHub.
#ifndef LARiskConfigurationVolFXATMParallelVega_h
#define LARiskConfigurationVolFXATMParallelVega_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationVolFXATMParallelVega.h
//
//  DESCRIPTION :        FX atm parallel vega setup class 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LARiskConfigurationVolParallel.h"


//===================== Class Declare LARiskConfigurationVolFXATMParallelVega==================================
/*! 
    @brief FX atm parallel vega setup class
	

*/
class LARiskConfigurationVolFXATMParallelVega : public LARiskConfigurationVolParallel
{
public:
	// constructor
	explicit LARiskConfigurationVolFXATMParallelVega(void);
	// destructor
	virtual ~LARiskConfigurationVolFXATMParallelVega(void);
	// copy constructor
	LARiskConfigurationVolFXATMParallelVega(const LARiskConfigurationVolFXATMParallelVega &rhs);
	LARiskConfigurationVolFXATMParallelVega &operator=(const LARiskConfigurationVolFXATMParallelVega &rhs);

protected:
	//==============================================
	// create scenario volatility object
	virtual std::vector<AQLObject *> createVolatilityEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1, int index = 0) const;
		//==============================================
	// create scenario volatility object
	virtual std::vector<AQLObject *> createVolatilityEntityOld(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1) const;
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
	// get risk name
	virtual  AQLString getRiskName(void) const;
	//==============================================
	// get parallel shift
	virtual double getParallelShiftVal(const AQLString &ccy) const;
	//==============================================
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const AQLString& fx) const;

};


#endif
