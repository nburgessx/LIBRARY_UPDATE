/*! @file
    @brief FX atm parallel vega setup class 
*/
//  2007, AlgoQuantHub.
#ifndef AQLRiskConfigurationVolFXATMParallelVega_h
#define AQLRiskConfigurationVolFXATMParallelVega_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLRiskConfigurationVolFXATMParallelVega.h
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

#include "AQLRiskConfigurationVolParallel.h"


//===================== Class Declare AQLRiskConfigurationVolFXATMParallelVega==================================
/*! 
    @brief FX atm parallel vega setup class
	

*/
class AQLRiskConfigurationVolFXATMParallelVega : public AQLRiskConfigurationVolParallel
{
public:
	// constructor
	explicit AQLRiskConfigurationVolFXATMParallelVega(void);
	// destructor
	virtual ~AQLRiskConfigurationVolFXATMParallelVega(void);
	// copy constructor
	AQLRiskConfigurationVolFXATMParallelVega(const AQLRiskConfigurationVolFXATMParallelVega &rhs);
	AQLRiskConfigurationVolFXATMParallelVega &operator=(const AQLRiskConfigurationVolFXATMParallelVega &rhs);

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
