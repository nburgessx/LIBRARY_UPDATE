/*! @file
    @brief Yield risk setup class
*/
//  2007, AlgoQuantHub.
#ifndef LARiskConfigurationYield_h
#define LARiskConfigurationYield_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationYield.h
//
//  DESCRIPTION :       Yield risk setup class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LARiskConfiguration.h"

//===================== Class Declare LARiskConfigurationYield==================================
/*! 
    @brief Yield risk setup class
	
	this class is abstract

*/
class LARiskConfigurationYield : public LARiskConfiguration
{
public:
	// constructor
	explicit LARiskConfigurationYield(void);
	// destructor
	virtual ~LARiskConfigurationYield(void);
	// copy constructor
	LARiskConfigurationYield(const LARiskConfigurationYield &rhs);
	LARiskConfigurationYield &operator=(const LARiskConfigurationYield &rhs);

protected:
	//==============================================
	// create scenario1 object 
	virtual  std::vector<LAObject *> createScenario1Entity(const LAString &ccy, LADataInstance &dataInstance, int index) const;
	//==============================================
	// create scenario2 object
	virtual  std::vector<LAObject *> createScenario2Entity(const LAString &ccy, LADataInstance &dataInstance, int index) const;
	//==============================================
	// create scenario yield object
	virtual std::vector<LAObject *> createYieldEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const = 0;
	//==============================================
	// get target names 
	virtual  LAString getTargetNames(const LAString &ccy, LADataInstance &dataInstance)  const;
	//==============================================
	// get max  grid index 
	virtual  int getMaxGridIndex(const LAString &ccy)  const;
	//==============================================
	// get grid calc buffer
	virtual  int getGridCalcBuffer()  const = 0;
	//==============================================
	// get scenario1 parallel shift
	virtual double getScenario1ParallelShift(const LAString &ccy) const = 0;
	//==============================================
	// get scenario2 parallel shift
	virtual double getScenario2ParallelShift(const LAString &ccy) const = 0;
	//==============================================
	// get scenario1 grid shift
	virtual DoubleArray getScenario1GridShift(const LAString &ccy) const = 0;
	//==============================================
	// get scenario2 grid shift
	virtual DoubleArray getScenario2GridShift(const LAString &ccy) const = 0;
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
	virtual  std::vector<std::vector<LAObject *> > createExtraScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// create extraScenario object old(not calibration)
	virtual  std::vector<std::vector<LAObject *> > createExtraScenarioEntityOld(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// get shift grid term
	virtual std::vector<LAString> getShiftGridTerm(const LAString &ccy) const { return getGridTerm(ccy); }
	//==============================================
	// get ir vol scenario
	virtual std::vector<LAObject *> createIRVolEntity(const LAString& ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// get foreign ir vol scenario
	virtual std::vector<LAObject *> createForeignIRVolEntity(const LAString& ccy, const LAString& fCcy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// create scenario foreign yield object
	virtual std::vector<LAObject *> createForeignYieldEntity(const LAString &ccy, const LAString &fCcy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index, const bool isFirst = true) const;
	//==============================================
	// create scenario collateral yield object
	virtual std::vector<LAObject *> createCollateralYieldEntity(const LAString &ccy, const LAString &colCcy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// get fx vol scenario
	virtual std::vector<LAObject *> createFXVolEntity(const LAString& ccy, const LAString& fx, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// is zero rate bump
	virtual bool isZeroBump(const LAString& ccy) const { (void)ccy; return true; }

private:

};
#endif
