/*! @file
    @brief Yield Risk Scenario create class
*/
//  2008, AlgoQuantHub.
#ifndef LAScenarioConfigurationYieldCurveBasis_h
#define LAScenarioConfigurationYieldCurveBasis_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAScenarioConfigurationYieldCurveBasis.h
//
//  DESCRIPTION :        Yield Risk Scenario create class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LAScenarioConfiguration.h"

//===================== Class Declare LAScenarioConfigurationYieldCurveBasis==================================
/*! 
    @brief Yield Risk Scenario create class
	

*/
class LAScenarioConfigurationYieldCurveBasis : public LAScenarioConfiguration
{
public:
	// constructor
	explicit LAScenarioConfigurationYieldCurveBasis(void);
	// destructor
	virtual ~LAScenarioConfigurationYieldCurveBasis(void);
	// copy constructor
	LAScenarioConfigurationYieldCurveBasis(const LAScenarioConfigurationYieldCurveBasis &rhs);
	LAScenarioConfigurationYieldCurveBasis &operator=(const LAScenarioConfigurationYieldCurveBasis &rhs);

	//==============================================
	// create risk scenario
	virtual std::vector<AQLObject *> createScenario(AQLDataInstance &dataInstance, const MAScenarioParam &param) const;
protected:
private:
	void createBumpedRate(AQLObjectHolder& basis, const AQLString& shiftType, const double baseRate, const double shiftVal) const;
	void restoreBumpedRate(std::vector<AQLObjectHolder>& basisVec, const DoubleArray& baseRates) const;
	void copyBumpedDataForForeignYield(const std::vector<AQLObjectHolder>& basisVec, const AQLString& suffix, AQLObjectPool& objPool, AQLObject& yieldData, const AQLString& attrSuffix) const;

};


#endif
