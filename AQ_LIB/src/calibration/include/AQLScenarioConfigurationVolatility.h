/*! @file
    @brief Volatility Risk Scenario create class
*/
#ifndef AQLScenarioConfigurationVolatility_h
#define AQLScenarioConfigurationVolatility_h
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLScenarioConfiguration.h"


class AQLObject;
class AQLMathVolatility;
class AQLObjectPool;

//===================== Class Declare AQLScenarioConfigurationVolatility==================================
/*! 
    @brief Volatility Risk Scenario create class
	

*/
class AQLScenarioConfigurationVolatility : public AQLScenarioConfiguration
{
public:
	// constructor
	explicit AQLScenarioConfigurationVolatility(void);
	// destructor
	virtual ~AQLScenarioConfigurationVolatility(void);
	// copy constructor
	AQLScenarioConfigurationVolatility(const AQLScenarioConfigurationVolatility &rhs);
	AQLScenarioConfigurationVolatility &operator=(const AQLScenarioConfigurationVolatility &rhs);

	//==============================================
	// create risk scenario
	virtual std::vector<AQLObject *> createScenario(AQLDataInstance &dataInstance, const AQLScenarioParam &param) const;

private:
	//==============================================
	// create parallel shift vol
	std::vector<AQLObject *> createParallelShiftVol(AQLDataInstance &dataInstance, AQLMathVolatility &baseVol, const AQLScenarioParam &param) const;
	//==============================================
	// create grid shift vol
	std::vector<AQLObject *> createGridShiftVol(AQLDataInstance &dataInstance, AQLMathVolatility &baseVol, const AQLScenarioParam &param) const;
	//==============================================
	// seartch coordinates matrix
	void searchCoordinatesMatrix(const DoubleArray &grid_t, const DoubleArray &grid_T, const DoubleArray &area,
													 unsigned int &pos_t_s, unsigned int &pos_t_e,
													 unsigned int &pos_T_s, unsigned int &pos_T_e) const;
	//==============================================
	// get double matrix
	void getCoordinatesMatrix(const AQLStringVector &term, DoubleMatrix &matrix) const;

};


#endif
