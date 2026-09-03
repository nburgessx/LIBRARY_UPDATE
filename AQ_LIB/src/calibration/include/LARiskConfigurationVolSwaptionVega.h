/*! @file
    @brief Swaption vega setup class
*/
//  2008, AlgoQuantHub.
#ifndef LARiskConfigurationVolSwaptionVega_h
#define LARiskConfigurationVolSwaptionVega_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationVolSwaptionVega.h
//
//  DESCRIPTION :        Swaption vega setup class 
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

//===================== Class Declare LARiskConfigurationVolSwaptionVega==================================
/*! 
    @brief Swaption vega setup class
	

*/
class LARiskConfigurationVolSwaptionVega : public LARiskConfigurationVolParallel
{
public:
	// constructor
	explicit LARiskConfigurationVolSwaptionVega(void);
	// destructor
	virtual ~LARiskConfigurationVolSwaptionVega(void);
	// copy constructor
	LARiskConfigurationVolSwaptionVega(const LARiskConfigurationVolSwaptionVega &rhs);
	LARiskConfigurationVolSwaptionVega &operator=(const LARiskConfigurationVolSwaptionVega &rhs);

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
	// get outputname1
	virtual  LAString  getOutPutName1(const LAString &ccy) const;
	//==============================================
	// get risk name
	virtual  LAString getRiskName(void) const;
	//==============================================
	// get target currencies
	virtual  LAString getTargetCurrencies() const;
	//==============================================
	// get calibration target currencies
	virtual  LAString getCalibTargetCurrencies() const;
	//==============================================
	// get parallel shift
	virtual double getParallelShiftVal(const LAString &ccy) const;
	//==============================================
	// create extraScenario1 object 
	virtual  std::vector<std::vector<LAObject *> > createExtraScenario1Entity(const LAString &ccy, LADataInstance &dataInstance, int index) const;
	//==============================================
	// get extra target names1
	virtual  LAStringVector getExtraTargetNames1(const LAString &ccy, LADataInstance &dataInstance) const;
	//==============================================
	// get grid term
	virtual LAString getParallelShiftTerm(const LAString &ccy) const { (void)ccy; return CALIB_MARKET_SWAPTION; }

    virtual bool isRiskCurrencyMode(const LAString &ccy) const;
};


#endif
