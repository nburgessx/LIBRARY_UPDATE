/*! @file
    @brief IR vega setup class
*/
//  2008, AlgoQuantHub.
#ifndef LARiskConfigurationVolIRVega_h
#define LARiskConfigurationVolIRVega_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationVolIRVega.h
//
//  DESCRIPTION :        IR Vega setup class 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LARiskConfigurationVolatility.h"



class LAString;
class LADataInstance;
class LARiskConfigurationVolCreatorLMM;
//===================== Class Declare LARiskConfigurationVolIRVega==================================
/*! 
    @brief IR Vega setup class
	

*/
class LARiskConfigurationVolIRVega : public LARiskConfigurationVolatility
{
public:
	// constructor
	explicit LARiskConfigurationVolIRVega(void);
	// destructor
	virtual ~LARiskConfigurationVolIRVega(void);
	// copy constructor
	LARiskConfigurationVolIRVega(const LARiskConfigurationVolIRVega &rhs);
	LARiskConfigurationVolIRVega &operator=(const LARiskConfigurationVolIRVega &rhs);

protected:
friend class LARiskConfigurationVolCreatorLMM;
	//==============================================
	// @Description: Creates the risk entities;
	//                overridden to provide them with additional dataValues required to perform the vega conversion
	//  @param objPool [in] The object pool
	//  @return A vector of pairs each entry of which is a pair of the currency and the risk object
	virtual std::vector<std::pair<LAString, std::vector<LAObject *> > > createRiskEntity(LAObjectPool &objPool) const;
	//==============================================
	// get  target names 
	virtual  LAString getTargetNames(const LAString &ccy, LADataInstance &dataInstance)  const;	
	//==============================================
	// create scenario volatility object
	virtual std::vector<LAObject *> createVolatilityEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1, int index = 0) const;
	//==============================================
	// create scenario volatility object
	virtual std::vector<LAObject *> createVolatilityEntityOld(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1) const;
	//==============================================
	// get outputname1
	virtual  LAString  getOutPutName1(const LAString &ccy) const;
	//==============================================
	// get shift type
	virtual  LAString  getShiftType(const LAString &ccy) const;
	//==============================================
	// get bump direction
	virtual  LAString  getBumpDirection(const LAString &ccy) const;
	//==============================================
	// get risk name
	virtual  LAString getRiskName(void) const;
	//==============================================
	// check target currencies
	virtual  LAString getTargetCurrencies() const;
	//==============================================
	// get calibration target currencies
	virtual  LAString getCalibTargetCurrencies() const;
	//==============================================
	// isgridsensitivity
	virtual  bool  isGridSensitivity(const LAString &ccy) const;
	//==============================================
	// isparallelshift
	virtual  bool isParallelShift(const LAString &ccy) const;
	//==============================================
	// get grid term
	virtual std::vector<LAString> getGridTerm(const LAString &ccy) const;
	//==============================================
	// get grid term
	virtual std::vector<LAString> getMarketTerm(const LAString &ccy) const;
	//==============================================
	// get bucket grid term
	virtual std::vector<LAString> getBucketGridTerm(const LAString &ccy) const;
	//==============================================
	// isfileuse
	virtual  bool isShiftValFileUse(const LAString &ccy) const;
	//==============================================
	// get scenario1 shift
	virtual double getScenario1ShiftValue(const LAString &ccy) const;
	//==============================================
	// get scenario2 shift
	virtual double getScenario2ShiftValue(const LAString &ccy) const;
	//==============================================
	// get divunit
	virtual double getDivUnit(const LAString &ccy) const;
	//==============================================
	// get double matrix
	virtual DoubleMatrix getCoordinatesMatrix(const LAString &ccy) const;
	//==============================================
	// get fileval matrix
	virtual DoubleMatrix getFileValMatrix(const LAString &ccy) const;
	//==============================================
	// get extra target names1
	virtual  LAStringVector getExtraTargetNames1(const LAString &ccy, LADataInstance &dataInstance) const;
	//==============================================
	// create extraScenario1 object 
	virtual  std::vector<std::vector<LAObject *> > createExtraScenario1Entity(const LAString &ccy, LADataInstance &dataInstance, int index) const;
	//==============================================
	// get extra target names2
	virtual  LAStringVector getExtraTargetNames2(const LAString &ccy, LADataInstance &dataInstance) const;
	//==============================================
	// create extraScenario2 object 
	virtual  std::vector<std::vector<LAObject *> > createExtraScenario2Entity(const LAString &ccy, LADataInstance &dataInstance, int index) const;
	//==============================================
	// create extraScenario object 
	virtual  std::vector<std::vector<LAObject *> > createExtraFXScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// create extraScenario object 
	virtual  std::vector<std::vector<LAObject *> > createExtraFXScenarioEntityOld(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// get bump type
	virtual  LAString getBumpType(const LAString &ccy) const;
	//==============================================
	// get grid type
	virtual  LAString getGridType(const LAString &ccy) const;
	//==============================================
	// get grid term
	virtual std::vector<LAString> getCoordinates(const LAString &ccy) const;
	//==============================================
	// get is wave
	virtual  bool  isWave(const LAString &ccy) const;
	//==============================================
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const LAString& ccy) const;
	//==============================================
	// check calibration target
	virtual  LAStringVector getCalibTargetFX(const LAString &ccy, LADataInstance &dataInstance) const;
	//==============================================
	// reduce market grids in SABR model
	virtual  LAStringVector reduceTargetGrids(const LAString &ccy, const LAStringVector &mktGrids) const;
	//==============================================
	// get vega type
	virtual LAString getVegaType(const LAString &ccy) const;
	//==============================================
	// store Swap Rate and Vol for risk print value
	virtual void storeIRAdditionalInfo(LADataInstance &dataInstance, const MAScenarioParam& param) const;
};


#endif
