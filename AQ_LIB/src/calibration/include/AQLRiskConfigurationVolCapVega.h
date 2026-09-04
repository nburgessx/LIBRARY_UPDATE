/*! @file
    @brief Cap vega  setup class
*/
//  2008, AlgoQuantHub.
#ifndef AQLRiskConfigurationVolCapVega_h
#define AQLRiskConfigurationVolCapVega_h
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLRiskConfigurationVolParallel.h"
#include "AQLDefinitionsCalibration.h"


//===================== Class Declare AQLRiskConfigurationVolCapVega==================================
/*! 
    @brief Cap vega setup class
	

*/
class AQLRiskConfigurationVolCapVega : public AQLRiskConfigurationVolParallel
{
public:
	// constructor
	explicit AQLRiskConfigurationVolCapVega(void);
	// destructor
	virtual ~AQLRiskConfigurationVolCapVega(void);
	// copy constructor
	AQLRiskConfigurationVolCapVega(const AQLRiskConfigurationVolCapVega &rhs);
	AQLRiskConfigurationVolCapVega &operator=(const AQLRiskConfigurationVolCapVega &rhs);

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
