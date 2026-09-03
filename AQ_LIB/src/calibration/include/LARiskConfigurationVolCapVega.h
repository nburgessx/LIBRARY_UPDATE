/*! @file
    @brief Cap vega  setup class
*/
//  2008, AlgoQuantHub.
#ifndef LARiskConfigurationVolCapVega_h
#define LARiskConfigurationVolCapVega_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationVolCapVega.h
//
//  DESCRIPTION :        Cap vega  setup class 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LARiskConfigurationVolParallel.h"
#include "LADefinitionsCalibration.h"


//===================== Class Declare LARiskConfigurationVolCapVega==================================
/*! 
    @brief Cap vega setup class
	

*/
class LARiskConfigurationVolCapVega : public LARiskConfigurationVolParallel
{
public:
	// constructor
	explicit LARiskConfigurationVolCapVega(void);
	// destructor
	virtual ~LARiskConfigurationVolCapVega(void);
	// copy constructor
	LARiskConfigurationVolCapVega(const LARiskConfigurationVolCapVega &rhs);
	LARiskConfigurationVolCapVega &operator=(const LARiskConfigurationVolCapVega &rhs);

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
	// get outputName1
	virtual  AQLString  getOutPutName1(const AQLString &ccy) const;
	//==============================================
	// setup isgridsensitivity
	virtual  AQLString getRiskName(void) const;
	//==============================================
	// get target currencies
	virtual  AQLString getTargetCurrencies() const;
	//==============================================
	// get calibration target currencies
	virtual  AQLString getCalibTargetCurrencies() const;
	//==============================================
	// get parallel shift
	virtual double getParallelShiftVal(const AQLString &ccy) const { ccy; return 0.0;}
	//==============================================
	// get grid term
	virtual AQLString getParallelShiftTerm(const AQLString &ccy) const { (void)ccy; return CALIB_MARKET_CAP; }

};


#endif
