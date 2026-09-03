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



class AQLString;
class AQLDataInstance;
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
	virtual std::vector<std::pair<AQLString, std::vector<AQLObject *> > > createRiskEntity(AQLObjectPool &objPool) const;
	//==============================================
	// get  target names 
	virtual  AQLString getTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance)  const;	
	//==============================================
	// create scenario volatility object
	virtual std::vector<AQLObject *> createVolatilityEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1, int index = 0) const;
	//==============================================
	// create scenario volatility object
	virtual std::vector<AQLObject *> createVolatilityEntityOld(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1) const;
	//==============================================
	// get outputname1
	virtual  AQLString  getOutPutName1(const AQLString &ccy) const;
	//==============================================
	// get shift type
	virtual  AQLString  getShiftType(const AQLString &ccy) const;
	//==============================================
	// get bump direction
	virtual  AQLString  getBumpDirection(const AQLString &ccy) const;
	//==============================================
	// get risk name
	virtual  AQLString getRiskName(void) const;
	//==============================================
	// check target currencies
	virtual  AQLString getTargetCurrencies() const;
	//==============================================
	// get calibration target currencies
	virtual  AQLString getCalibTargetCurrencies() const;
	//==============================================
	// isgridsensitivity
	virtual  bool  isGridSensitivity(const AQLString &ccy) const;
	//==============================================
	// isparallelshift
	virtual  bool isParallelShift(const AQLString &ccy) const;
	//==============================================
	// get grid term
	virtual std::vector<AQLString> getGridTerm(const AQLString &ccy) const;
	//==============================================
	// get grid term
	virtual std::vector<AQLString> getMarketTerm(const AQLString &ccy) const;
	//==============================================
	// get bucket grid term
	virtual std::vector<AQLString> getBucketGridTerm(const AQLString &ccy) const;
	//==============================================
	// isfileuse
	virtual  bool isShiftValFileUse(const AQLString &ccy) const;
	//==============================================
	// get scenario1 shift
	virtual double getScenario1ShiftValue(const AQLString &ccy) const;
	//==============================================
	// get scenario2 shift
	virtual double getScenario2ShiftValue(const AQLString &ccy) const;
	//==============================================
	// get divunit
	virtual double getDivUnit(const AQLString &ccy) const;
	//==============================================
	// get double matrix
	virtual DoubleMatrix getCoordinatesMatrix(const AQLString &ccy) const;
	//==============================================
	// get fileval matrix
	virtual DoubleMatrix getFileValMatrix(const AQLString &ccy) const;
	//==============================================
	// get extra target names1
	virtual  AQLStringVector getExtraTargetNames1(const AQLString &ccy, AQLDataInstance &dataInstance) const;
	//==============================================
	// create extraScenario1 object 
	virtual  std::vector<std::vector<AQLObject *> > createExtraScenario1Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// get extra target names2
	virtual  AQLStringVector getExtraTargetNames2(const AQLString &ccy, AQLDataInstance &dataInstance) const;
	//==============================================
	// create extraScenario2 object 
	virtual  std::vector<std::vector<AQLObject *> > createExtraScenario2Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// create extraScenario object 
	virtual  std::vector<std::vector<AQLObject *> > createExtraFXScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// create extraScenario object 
	virtual  std::vector<std::vector<AQLObject *> > createExtraFXScenarioEntityOld(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// get bump type
	virtual  AQLString getBumpType(const AQLString &ccy) const;
	//==============================================
	// get grid type
	virtual  AQLString getGridType(const AQLString &ccy) const;
	//==============================================
	// get grid term
	virtual std::vector<AQLString> getCoordinates(const AQLString &ccy) const;
	//==============================================
	// get is wave
	virtual  bool  isWave(const AQLString &ccy) const;
	//==============================================
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const AQLString& ccy) const;
	//==============================================
	// check calibration target
	virtual  AQLStringVector getCalibTargetFX(const AQLString &ccy, AQLDataInstance &dataInstance) const;
	//==============================================
	// reduce market grids in SABR model
	virtual  AQLStringVector reduceTargetGrids(const AQLString &ccy, const AQLStringVector &mktGrids) const;
	//==============================================
	// get vega type
	virtual AQLString getVegaType(const AQLString &ccy) const;
	//==============================================
	// store Swap Rate and Vol for risk print value
	virtual void storeIRAdditionalInfo(AQLDataInstance &dataInstance, const MAScenarioParam& param) const;
};


#endif
