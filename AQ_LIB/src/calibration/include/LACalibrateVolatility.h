/*! @file
    @brief  Volatility data create class
*/
//  2008, Mizuho International London.
#ifndef LACalibrateVolatility_h
#define LACalibrateVolatility_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateVolatility.h
//
//  DESCRIPTION :         Volatility data create class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////


#include "LAString.h"
#include "LACoreTemplateType.h"


class LAFunctionBase;
class LAObjectPool;
struct MAScenarioParam;


//===================== Class Declare LACalibrateVolatility==================================
/*! 
    @brief Volatility data create class
	
	this class is abstract

*/
class LACalibrateVolatility
{
public:
	// constructor
	explicit LACalibrateVolatility(void) {}
	// destructor
	virtual ~LACalibrateVolatility(void) {}
	//// copy constructor
	//LACalibrateVolatility(const LACalibrateVolatility &rhs) {}
	//LACalibrateVolatility &operator=(const LACalibrateVolatility &rhs) {}

	//==============================================
	// create  Volatility from grid_t and Volatility of DoubleMatrix vector
	virtual void createVolatility(DoubleArray &grid_t, std::vector<DoubleMatrix> &vol, const LAStringVector &filePath, 
								const MAScenarioParam *param = 0, LAObjectPool *objPool = 0) const = 0;
	//==============================================
	// create Volatility as function matrix
	virtual void createVolatility(std::vector< std::vector<LAFunctionBase *> > &vol, const LAStringVector &filePath, 
								const MAScenarioParam *param = 0, LAObjectPool *objPool = 0) const = 0;
	//==============================================
	// create Volatility from grid_t and Volatility of Function vector
	virtual void createVolatility(DoubleArray &grid_T, std::vector<LAFunctionBase *> &vol, const LAStringVector &filePath, 
								const MAScenarioParam *param = 0, LAObjectPool *objPool = 0) const = 0;
	//==============================================
	// create Volatility from grid_t and Volatility of DoubleMatrix
	virtual void createVolatility(DoubleArray &grid_t, DoubleMatrix &vol, const LAStringVector &filePath, 
								const MAScenarioParam *param = 0, LAObjectPool *objPool = 0) const = 0;
	//==============================================
	// create Volatility as function vector
	virtual void createVolatility(std::vector<LAFunctionBase *> &vol, const LAStringVector &filePath, 
								const MAScenarioParam *param = 0, LAObjectPool *objPool = 0, int gridPos = -1) const = 0;
	//==============================================
	// create Volatility from grid_t and Volatility of function
	virtual void createVolatility(DoubleArray &grid_T, LAFunctionBase *vol, const LAStringVector &filePath, 
								const MAScenarioParam *param = 0, LAObjectPool *objPool = 0) const = 0;
	//==============================================
	// create Volatility from grid_t and Volatility of vector
	virtual void createVolatility (DoubleArray &grid_t, DoubleArray &vol, const LAStringVector &filePath, 
								const MAScenarioParam *param = 0, LAObjectPool *objPool = 0) const = 0;
	//==============================================
	// create Volatility as function for fx
	virtual LAFunctionBase *createVolatility (const LAStringVector &filePath, 
								const MAScenarioParam *param = 0, LAObjectPool *objPool = 0, int gridPos = -1) const = 0;
	//==============================================
	// create Volatility as double
	virtual void createVolatility (double &vol, const LAStringVector &filePath, 
								const MAScenarioParam *param = 0, LAObjectPool *objPool = 0) const = 0;
	//==============================================
	// get grid_T
	virtual void getGrid_T (DoubleArray &grid_T, const LAStringVector &filePath) const = 0;

};


#endif
