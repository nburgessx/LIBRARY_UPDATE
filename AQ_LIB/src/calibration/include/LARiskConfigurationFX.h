/*! @file
    @brief   FX setup class for calc risk
*/
//  2007, AlgoQuantHub.
#ifndef LARiskConfigurationFX_h
#define LARiskConfigurationFX_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationFX.h
//
//  DESCRIPTION :         FX setup class for calc risk
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LARiskConfigurationYield.h"

//===================== Class Declare LARiskConfigurationFX==================================
/*! 
    @brief FX setup class for calc risk
	
	this class is abstract

*/
class LARiskConfigurationFX : public LARiskConfigurationYield
{
public:
	// constructor
	explicit LARiskConfigurationFX(void);
	// destructor
	virtual ~LARiskConfigurationFX(void);
	// copy constructor
	LARiskConfigurationFX(const LARiskConfigurationFX &rhs);
	LARiskConfigurationFX &operator=(const LARiskConfigurationFX &rhs);

protected:
	//==============================================
	// get target names 
	virtual  AQLString getTargetNames(const AQLString &fx, AQLDataInstance &dataInstance)  const;
	//==============================================
	// create scenario1 object 
	virtual  std::vector<AQLObject *> createScenario1Entity(const AQLString &fx, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// create scenario2 object
	virtual  std::vector<AQLObject *> createScenario2Entity(const AQLString &fx, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// get extra target names1
	virtual  AQLStringVector getExtraTargetNames1(const AQLString &fx, AQLDataInstance &dataInstance) const;
	//==============================================
	// get extra target names2
	virtual  AQLStringVector getExtraTargetNames2(const AQLString &fx, AQLDataInstance &dataInstance) const { return getExtraTargetNames1(fx, dataInstance); };
	//==============================================
	// create extraScenario1 object 
	virtual  std::vector<std::vector<AQLObject *> > createExtraScenario1Entity(const AQLString &fx, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// create extraScenario2 object 
	virtual  std::vector<std::vector<AQLObject *> > createExtraScenario2Entity(const AQLString &fx, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// create scenario fx object
	virtual std::vector<AQLObject *> createFXEntity(const AQLString &fx, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// create fx volatility scenario object 
	virtual  std::vector<AQLObject *> createFXVolEntity(const AQLString &fx, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// isgridsensitivity
	virtual  bool  isGridSensitivity(const AQLString &fx) const;
	//==============================================
	// isparallelshift
	virtual  bool isParallelShift(const AQLString &fx) const;
	//==============================================
	// getRiksCurrencys 
	virtual AQLStringVector getRiskCurrencys(AQLObjectPool& objPool) const;

	//==============================================
	// LARiskConfiguration
	//==============================================
	// get operator1
	virtual  AQLString  getOperator1(void) const {return AQ_NO_DATA; };
	//==============================================
	// get coefficient1
	virtual  AQLString  getCoefficient1(const AQLString &fx) const {return AQ_NO_DATA; };
	//==============================================
	// get grid term
	virtual std::vector<AQLString> getGridTerm(const AQLString &fx) const;
	//==============================================
	// get bucket grid term
	virtual std::vector<AQLString> getBucketGridTerm(const AQLString &fx) const;

	//==============================================
	// LARiskConfigurationYield
	//==============================================
	// create scenario yield object
	virtual std::vector<AQLObject *> createYieldEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const {return std::vector<AQLObject *>(0); };
	//==============================================
	// get grid calc buffer
	virtual  int getGridCalcBuffer()  const {return 0; };
	//==============================================
	// get scenario1 parallel shift
	virtual double getScenario1ParallelShift(const AQLString &ccy) const {return 0.0; };
	//==============================================
	// get scenario2 parallel shift
	virtual double getScenario2ParallelShift(const AQLString &ccy) const {return 0.0; };
	//==============================================
	// get scenario1 grid shift
	virtual DoubleArray getScenario1GridShift(const AQLString &ccy) const {return LARiskConfiguration::getScenario1GridShift(ccy); };
	//==============================================
	// get scenario2 grid shift
	virtual DoubleArray getScenario2GridShift(const AQLString &ccy) const {return LARiskConfiguration::getScenario1GridShift(ccy); };
	//==============================================
	// create extraScenario object 
	virtual  std::vector<std::vector<AQLObject *> > createExtraScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// create extra yield object
	virtual std::vector<AQLObject *> createExtraYieldEntity(const AQLString &fx, const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index, const bool isFirst = true) const;
	//==============================================
	// create extra ir vol object for risk scenario
	virtual std::vector<AQLObject *> createExtraIRVolEntity(const AQLString &fx, const AQLString& ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// create extra ir vol object for base scenario
	virtual std::vector<AQLObject *> createExtraIRVolEntityBase(const AQLString &fx, const AQLString& ccy, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// create extra collateral yield object
	virtual std::vector<AQLObject *> createCollateralYieldEntity(const AQLString &fx, const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// create extra fx volatility object 
	virtual  std::vector<AQLObject *> createExtraFXVolEntity(const AQLString &fx, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;

	virtual std::vector<AQLObject *> createBaseScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const;

    virtual AQLString getBaseOutPutName(const AQLString &ccy , int index) const;
        
	virtual double getBaseSpotVal(const AQLString &ccy, int index) const;

	virtual bool omitNotionalExposure(const AQLString &ccy) const;

};
#endif
