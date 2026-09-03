/*! @file
    @brief Volatility data create class for ptberg
*/
//  2008, AlgoQuantHub.
#ifndef LACalibrateVolatilityIRSABR_h
#define LACalibrateVolatilityIRSABR_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateVolatilityIRSABR.h
//
//  DESCRIPTION :         Volatility data create class for lmm
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LACalibrateVolatility.h"
//// FROTOTYPE ////

//===================== Class Declare LACalibrateVolatilityIRSABR==================================
/*! 
    @brief Volatility data create class for ptberg
	

*/
class LACalibrateVolatilityIRSABR : public LACalibrateVolatility
{
public:
	// constructor
	explicit LACalibrateVolatilityIRSABR(void);
	// destructor
	virtual ~LACalibrateVolatilityIRSABR(void);
	// copy constructor
	LACalibrateVolatilityIRSABR(const LACalibrateVolatilityIRSABR &rhs);
	LACalibrateVolatilityIRSABR &operator=(const LACalibrateVolatilityIRSABR &rhs);

	//==============================================
	// create  Volatility from grid_t and Volatility of DoubleMatrix vector
	virtual void createVolatility(DoubleArray &grid_t, std::vector<DoubleMatrix> &vol, const LAStringVector &filePath, 
								const MAScenarioParam *param = 0, LAObjectPool *objPool = 0) const ;
	//==============================================
	// create Volatility as function matrix
	virtual void createVolatility(std::vector<std::vector<LAFunctionBase *> > &vol, const LAStringVector &filePath, 
								const MAScenarioParam *param = 0, LAObjectPool *objPool = 0) const ;
	//==============================================
	// create Volatility from grid_t and Volatility of Function vector
	virtual void createVolatility(DoubleArray &grid_T, std::vector<LAFunctionBase *> &vol, const LAStringVector &filePath, 
								const MAScenarioParam *param = 0, LAObjectPool *objPool = 0) const ;
	//==============================================
	// create Volatility from grid_t and Volatility of DoubleMatrix
	virtual void createVolatility(DoubleArray &grid_t, DoubleMatrix &vol, const LAStringVector &filePath, 
								const MAScenarioParam *param = 0, LAObjectPool *objPool = 0) const ;
	//==============================================
	// create Volatility as function vector
	virtual void createVolatility(std::vector<LAFunctionBase *> &vol, const LAStringVector &filePath, 
								const MAScenarioParam *param = 0, LAObjectPool *objPool = 0, int gridPos = -1) const ;
	//==============================================
	// create Volatility from grid_t and Volatility of function
	virtual void createVolatility(DoubleArray &grid_T, LAFunctionBase *vol, const LAStringVector &filePath, 
								const MAScenarioParam *param = 0, LAObjectPool *objPool = 0) const ;
	//==============================================
	// create Volatility from grid_t and Volatility of vector
	virtual void createVolatility (DoubleArray &grid_t, DoubleArray &vol, const LAStringVector &filePath, 
								const MAScenarioParam *param = 0, LAObjectPool *objPool = 0) const ;
	//==============================================
	// create Volatility as function for fx
	virtual LAFunctionBase *createVolatility (const LAStringVector &filePath, 
								const MAScenarioParam *param = 0, LAObjectPool *objPool = 0, int gridPos = -1) const ;
	//==============================================
	// create Volatility as double
	virtual void createVolatility (double &vol, const LAStringVector &filePath, 
								const MAScenarioParam *param = 0, LAObjectPool *objPool = 0) const;

	//==============================================
	// get grid_T
	virtual void getGrid_T (DoubleArray &grid_T, const LAStringVector &filePath) const;

};


#endif
