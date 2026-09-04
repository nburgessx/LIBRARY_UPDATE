/*! @file
    @brief Yield Risk Scenario create class
*/
#ifndef AQLScenarioConfigurationYieldCurveBasis_h
#define AQLScenarioConfigurationYieldCurveBasis_h
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLScenarioConfiguration.h"

//===================== Class Declare AQLScenarioConfigurationYieldCurveBasis==================================
/*! 
    @brief Yield Risk Scenario create class
	

*/
class AQLScenarioConfigurationYieldCurveBasis : public AQLScenarioConfiguration
{
public:
	// constructor
	explicit AQLScenarioConfigurationYieldCurveBasis(void);
	// destructor
	virtual ~AQLScenarioConfigurationYieldCurveBasis(void);
	// copy constructor
	AQLScenarioConfigurationYieldCurveBasis(const AQLScenarioConfigurationYieldCurveBasis &rhs);
	AQLScenarioConfigurationYieldCurveBasis &operator=(const AQLScenarioConfigurationYieldCurveBasis &rhs);

	//==============================================
	// create risk scenario
	virtual std::vector<AQLObject *> createScenario(AQLDataInstance &dataInstance, const AQLScenarioParam &param) const;
protected:
private:
	void createBumpedRate(AQLObjectHolder& basis, const AQLString& shiftType, const double baseRate, const double shiftVal) const;
	void restoreBumpedRate(std::vector<AQLObjectHolder>& basisVec, const DoubleArray& baseRates) const;
	void copyBumpedDataForForeignYield(const std::vector<AQLObjectHolder>& basisVec, const AQLString& suffix, AQLObjectPool& objPool, AQLObject& yieldData, const AQLString& attrSuffix) const;

};


#endif
