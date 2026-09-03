/*! @file
    @brief Cap vega  setup class
*/
//  2008, Mizuho International London.
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
	virtual std::vector<LAObject *> createVolatilityEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1, int index = 0) const;
	//==============================================
	// create scenario volatility object
	virtual std::vector<LAObject *> createVolatilityEntityOld(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1) const;
	//==============================================
	// get operator1
	virtual  LAString  getOperator1(void) const;	
	//==============================================
	// get coefficient1
	virtual  LAString  getCoefficient1(const LAString &ccy) const;
	//==============================================
	// get outputName1
	virtual  LAString  getOutPutName1(const LAString &ccy) const;
	//==============================================
	// setup isgridsensitivity
	virtual  LAString getRiskName(void) const;
	//==============================================
	// get target currencies
	virtual  LAString getTargetCurrencies() const;
	//==============================================
	// get calibration target currencies
	virtual  LAString getCalibTargetCurrencies() const;
	//==============================================
	// get parallel shift
	virtual double getParallelShiftVal(const LAString &ccy) const { ccy; return 0.0;}
	//==============================================
	// get grid term
	virtual LAString getParallelShiftTerm(const LAString &ccy) const { (void)ccy; return CALIB_MARKET_CAP; }

};


#endif
