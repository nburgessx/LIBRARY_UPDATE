/*! @file
    @brief Yield risk setup class
*/
//  2007, AlgoQuantHub.
#ifndef AQLRiskConfigurationYield_h
#define AQLRiskConfigurationYield_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLRiskConfigurationYield.h
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

#include "AQLRiskConfiguration.h"

//===================== Class Declare AQLRiskConfigurationYield==================================
/*! 
    @brief Yield risk setup class
	
	this class is abstract

*/
class AQLRiskConfigurationYield : public AQLRiskConfiguration
{
public:
	// constructor
	explicit AQLRiskConfigurationYield(void);
	// destructor
	virtual ~AQLRiskConfigurationYield(void);
	// copy constructor
	AQLRiskConfigurationYield(const AQLRiskConfigurationYield &rhs);
	AQLRiskConfigurationYield &operator=(const AQLRiskConfigurationYield &rhs);

protected:
	//==============================================
	// create scenario1 object 
	virtual  std::vector<AQLObject *> createScenario1Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// create scenario2 object
	virtual  std::vector<AQLObject *> createScenario2Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// create scenario yield object
	virtual std::vector<AQLObject *> createYieldEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const = 0;
	//==============================================
	// get target names 
	virtual  AQLString getTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance)  const;
	//==============================================
	// get max  grid index 
	virtual  int getMaxGridIndex(const AQLString &ccy)  const;
	//==============================================
	// get grid calc buffer
	virtual  int getGridCalcBuffer()  const = 0;
	//==============================================
	// get scenario1 parallel shift
	virtual double getScenario1ParallelShift(const AQLString &ccy) const = 0;
	//==============================================
	// get scenario2 parallel shift
	virtual double getScenario2ParallelShift(const AQLString &ccy) const = 0;
	//==============================================
	// get scenario1 grid shift
	virtual DoubleArray getScenario1GridShift(const AQLString &ccy) const = 0;
	//==============================================
	// get scenario2 grid shift
	virtual DoubleArray getScenario2GridShift(const AQLString &ccy) const = 0;
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
	virtual  std::vector<std::vector<AQLObject *> > createExtraScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// create extraScenario object old(not calibration)
	virtual  std::vector<std::vector<AQLObject *> > createExtraScenarioEntityOld(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// get shift grid term
	virtual std::vector<AQLString> getShiftGridTerm(const AQLString &ccy) const { return getGridTerm(ccy); }
	//==============================================
	// get ir vol scenario
	virtual std::vector<AQLObject *> createIRVolEntity(const AQLString& ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// get foreign ir vol scenario
	virtual std::vector<AQLObject *> createForeignIRVolEntity(const AQLString& ccy, const AQLString& fCcy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// create scenario foreign yield object
	virtual std::vector<AQLObject *> createForeignYieldEntity(const AQLString &ccy, const AQLString &fCcy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index, const bool isFirst = true) const;
	//==============================================
	// create scenario collateral yield object
	virtual std::vector<AQLObject *> createCollateralYieldEntity(const AQLString &ccy, const AQLString &colCcy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// get fx vol scenario
	virtual std::vector<AQLObject *> createFXVolEntity(const AQLString& ccy, const AQLString& fx, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// is zero rate bump
	virtual bool isZeroBump(const AQLString& ccy) const { (void)ccy; return true; }

private:

};
#endif
