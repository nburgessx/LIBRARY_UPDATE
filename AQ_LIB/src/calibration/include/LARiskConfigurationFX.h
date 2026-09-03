/*! @file
    @brief   FX setup class for calc risk
*/
//  2007, Mizuho International London.
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
	virtual  LAString getTargetNames(const LAString &fx, LADataInstance &dataInstance)  const;
	//==============================================
	// create scenario1 object 
	virtual  std::vector<LAObject *> createScenario1Entity(const LAString &fx, LADataInstance &dataInstance, int index) const;
	//==============================================
	// create scenario2 object
	virtual  std::vector<LAObject *> createScenario2Entity(const LAString &fx, LADataInstance &dataInstance, int index) const;
	//==============================================
	// get extra target names1
	virtual  LAStringVector getExtraTargetNames1(const LAString &fx, LADataInstance &dataInstance) const;
	//==============================================
	// get extra target names2
	virtual  LAStringVector getExtraTargetNames2(const LAString &fx, LADataInstance &dataInstance) const { return getExtraTargetNames1(fx, dataInstance); };
	//==============================================
	// create extraScenario1 object 
	virtual  std::vector<std::vector<LAObject *> > createExtraScenario1Entity(const LAString &fx, LADataInstance &dataInstance, int index) const;
	//==============================================
	// create extraScenario2 object 
	virtual  std::vector<std::vector<LAObject *> > createExtraScenario2Entity(const LAString &fx, LADataInstance &dataInstance, int index) const;
	//==============================================
	// create scenario fx object
	virtual std::vector<LAObject *> createFXEntity(const LAString &fx, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// create fx volatility scenario object 
	virtual  std::vector<LAObject *> createFXVolEntity(const LAString &fx, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// isgridsensitivity
	virtual  bool  isGridSensitivity(const LAString &fx) const;
	//==============================================
	// isparallelshift
	virtual  bool isParallelShift(const LAString &fx) const;
	//==============================================
	// getRiksCurrencys 
	virtual LAStringVector getRiskCurrencys(LAObjectPool& objPool) const;

	//==============================================
	// LARiskConfiguration
	//==============================================
	// get operator1
	virtual  LAString  getOperator1(void) const {return MLIB_NO_DATA; };
	//==============================================
	// get coefficient1
	virtual  LAString  getCoefficient1(const LAString &fx) const {return MLIB_NO_DATA; };
	//==============================================
	// get grid term
	virtual std::vector<LAString> getGridTerm(const LAString &fx) const;
	//==============================================
	// get bucket grid term
	virtual std::vector<LAString> getBucketGridTerm(const LAString &fx) const;

	//==============================================
	// LARiskConfigurationYield
	//==============================================
	// create scenario yield object
	virtual std::vector<LAObject *> createYieldEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const {return std::vector<LAObject *>(0); };
	//==============================================
	// get grid calc buffer
	virtual  int getGridCalcBuffer()  const {return 0; };
	//==============================================
	// get scenario1 parallel shift
	virtual double getScenario1ParallelShift(const LAString &ccy) const {return 0.0; };
	//==============================================
	// get scenario2 parallel shift
	virtual double getScenario2ParallelShift(const LAString &ccy) const {return 0.0; };
	//==============================================
	// get scenario1 grid shift
	virtual DoubleArray getScenario1GridShift(const LAString &ccy) const {return LARiskConfiguration::getScenario1GridShift(ccy); };
	//==============================================
	// get scenario2 grid shift
	virtual DoubleArray getScenario2GridShift(const LAString &ccy) const {return LARiskConfiguration::getScenario1GridShift(ccy); };
	//==============================================
	// create extraScenario object 
	virtual  std::vector<std::vector<LAObject *> > createExtraScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// create extra yield object
	virtual std::vector<LAObject *> createExtraYieldEntity(const LAString &fx, const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index, const bool isFirst = true) const;
	//==============================================
	// create extra ir vol object for risk scenario
	virtual std::vector<LAObject *> createExtraIRVolEntity(const LAString &fx, const LAString& ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// create extra ir vol object for base scenario
	virtual std::vector<LAObject *> createExtraIRVolEntityBase(const LAString &fx, const LAString& ccy, LADataInstance &dataInstance, int index) const;
	//==============================================
	// create extra collateral yield object
	virtual std::vector<LAObject *> createCollateralYieldEntity(const LAString &fx, const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// create extra fx volatility object 
	virtual  std::vector<LAObject *> createExtraFXVolEntity(const LAString &fx, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;

	virtual std::vector<LAObject *> createBaseScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const;

    virtual LAString getBaseOutPutName(const LAString &ccy , int index) const;
        
	virtual double getBaseSpotVal(const LAString &ccy, int index) const;

	virtual bool omitNotionalExposure(const LAString &ccy) const;

};
#endif
