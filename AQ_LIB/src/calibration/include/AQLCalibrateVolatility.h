/*! @file
    @brief  Volatility data create class
*/
//  2008, AlgoQuantHub.
#ifndef AQLCalibrateVolatility_h
#define AQLCalibrateVolatility_h


#include "AQLString.h"
#include "AQLCoreTemplateType.h"


class AQLFunctionBase;
class AQLObjectPool;
struct AQLScenarioParam;


//===================== Class Declare AQLCalibrateVolatility==================================
/*! 
    @brief Volatility data create class
	
	this class is abstract

*/
class AQLCalibrateVolatility
{
public:
	// constructor
	explicit AQLCalibrateVolatility(void) {}
	// destructor
	virtual ~AQLCalibrateVolatility(void) {}
	//// copy constructor
	//AQLCalibrateVolatility(const AQLCalibrateVolatility &rhs) {}
	//AQLCalibrateVolatility &operator=(const AQLCalibrateVolatility &rhs) {}

	//==============================================
	// create  Volatility from grid_t and Volatility of DoubleMatrix vector
	virtual void createVolatility(DoubleArray &grid_t, std::vector<DoubleMatrix> &vol, const AQLStringVector &filePath, 
								const AQLScenarioParam *param = 0, AQLObjectPool *objPool = 0) const = 0;
	//==============================================
	// create Volatility as function matrix
	virtual void createVolatility(std::vector< std::vector<AQLFunctionBase *> > &vol, const AQLStringVector &filePath, 
								const AQLScenarioParam *param = 0, AQLObjectPool *objPool = 0) const = 0;
	//==============================================
	// create Volatility from grid_t and Volatility of Function vector
	virtual void createVolatility(DoubleArray &grid_T, std::vector<AQLFunctionBase *> &vol, const AQLStringVector &filePath, 
								const AQLScenarioParam *param = 0, AQLObjectPool *objPool = 0) const = 0;
	//==============================================
	// create Volatility from grid_t and Volatility of DoubleMatrix
	virtual void createVolatility(DoubleArray &grid_t, DoubleMatrix &vol, const AQLStringVector &filePath, 
								const AQLScenarioParam *param = 0, AQLObjectPool *objPool = 0) const = 0;
	//==============================================
	// create Volatility as function vector
	virtual void createVolatility(std::vector<AQLFunctionBase *> &vol, const AQLStringVector &filePath, 
								const AQLScenarioParam *param = 0, AQLObjectPool *objPool = 0, int gridPos = -1) const = 0;
	//==============================================
	// create Volatility from grid_t and Volatility of function
	virtual void createVolatility(DoubleArray &grid_T, AQLFunctionBase *vol, const AQLStringVector &filePath, 
								const AQLScenarioParam *param = 0, AQLObjectPool *objPool = 0) const = 0;
	//==============================================
	// create Volatility from grid_t and Volatility of vector
	virtual void createVolatility (DoubleArray &grid_t, DoubleArray &vol, const AQLStringVector &filePath, 
								const AQLScenarioParam *param = 0, AQLObjectPool *objPool = 0) const = 0;
	//==============================================
	// create Volatility as function for fx
	virtual AQLFunctionBase *createVolatility (const AQLStringVector &filePath, 
								const AQLScenarioParam *param = 0, AQLObjectPool *objPool = 0, int gridPos = -1) const = 0;
	//==============================================
	// create Volatility as double
	virtual void createVolatility (double &vol, const AQLStringVector &filePath, 
								const AQLScenarioParam *param = 0, AQLObjectPool *objPool = 0) const = 0;
	//==============================================
	// get grid_T
	virtual void getGrid_T (DoubleArray &grid_T, const AQLStringVector &filePath) const = 0;

};


#endif
