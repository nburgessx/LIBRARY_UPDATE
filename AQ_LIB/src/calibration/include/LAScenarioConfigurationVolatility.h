/*! @file
    @brief Volatility Risk Scenario create class
*/
//  2008, Mizuho International London.
#ifndef LAScenarioConfigurationVolatility_h
#define LAScenarioConfigurationVolatility_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAScenarioConfigurationVolatility.h
//
//  DESCRIPTION :        Volatility Risk Scenario create class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LAScenarioConfiguration.h"


class LAObject;
class LAMathVolatility;
class LAObjectPool;

//===================== Class Declare LAScenarioConfigurationVolatility==================================
/*! 
    @brief Volatility Risk Scenario create class
	

*/
class LAScenarioConfigurationVolatility : public LAScenarioConfiguration
{
public:
	// constructor
	explicit LAScenarioConfigurationVolatility(void);
	// destructor
	virtual ~LAScenarioConfigurationVolatility(void);
	// copy constructor
	LAScenarioConfigurationVolatility(const LAScenarioConfigurationVolatility &rhs);
	LAScenarioConfigurationVolatility &operator=(const LAScenarioConfigurationVolatility &rhs);

	//==============================================
	// create risk scenario
	virtual std::vector<LAObject *> createScenario(LADataInstance &dataInstance, const MAScenarioParam &param) const;

private:
	//==============================================
	// create parallel shift vol
	std::vector<LAObject *> createParallelShiftVol(LADataInstance &dataInstance, LAMathVolatility &baseVol, const MAScenarioParam &param) const;
	//==============================================
	// create grid shift vol
	std::vector<LAObject *> createGridShiftVol(LADataInstance &dataInstance, LAMathVolatility &baseVol, const MAScenarioParam &param) const;
	//==============================================
	// seartch coordinates matrix
	void searchCoordinatesMatrix(const DoubleArray &grid_t, const DoubleArray &grid_T, const DoubleArray &area,
													 unsigned int &pos_t_s, unsigned int &pos_t_e,
													 unsigned int &pos_T_s, unsigned int &pos_T_e) const;
	//==============================================
	// get double matrix
	void getCoordinatesMatrix(const LAStringVector &term, DoubleMatrix &matrix) const;

};


#endif
