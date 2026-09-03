/*! @file
    @brief Swaption vega setup class
*/
//  2008, AlgoQuantHub.
#ifndef AQLRiskConfigurationVolSwaptionVega_h
#define AQLRiskConfigurationVolSwaptionVega_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLRiskConfigurationVolSwaptionVega.h
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

#include "AQLRiskConfigurationVolParallel.h"
#include "AQLDefinitionsCalibration.h"

//===================== Class Declare AQLRiskConfigurationVolSwaptionVega==================================
/*! 
    @brief Swaption vega setup class
	

*/
class AQLRiskConfigurationVolSwaptionVega : public AQLRiskConfigurationVolParallel
{
public:
	// constructor
	explicit AQLRiskConfigurationVolSwaptionVega(void);
	// destructor
	virtual ~AQLRiskConfigurationVolSwaptionVega(void);
	// copy constructor
	AQLRiskConfigurationVolSwaptionVega(const AQLRiskConfigurationVolSwaptionVega &rhs);
	AQLRiskConfigurationVolSwaptionVega &operator=(const AQLRiskConfigurationVolSwaptionVega &rhs);

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
	// get target currencies
	virtual  AQLString getTargetCurrencies() const;
	//==============================================
	// get calibration target currencies
	virtual  AQLString getCalibTargetCurrencies() const;
	//==============================================
	// get parallel shift
	virtual double getParallelShiftVal(const AQLString &ccy) const;
	//==============================================
	// create extraScenario1 object 
	virtual  std::vector<std::vector<AQLObject *> > createExtraScenario1Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// get extra target names1
	virtual  AQLStringVector getExtraTargetNames1(const AQLString &ccy, AQLDataInstance &dataInstance) const;
	//==============================================
	// get grid term
	virtual AQLString getParallelShiftTerm(const AQLString &ccy) const { (void)ccy; return CALIB_MARKET_SWAPTION; }

    virtual bool isRiskCurrencyMode(const AQLString &ccy) const;
};


#endif
