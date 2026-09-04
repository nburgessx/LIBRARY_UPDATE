/*! @file
    @brief Volatility data create class for ptberg
*/
#ifndef AQLCalibrateVolatilityPtberg_h
#define AQLCalibrateVolatilityPtberg_h
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCalibrateVolatility.h"
//// FROTOTYPE ////

//===================== Class Declare AQLCalibrateVolatilityPtberg==================================
/*! 
    @brief Volatility data create class for ptberg
	

*/
class AQLCalibrateVolatilityPtberg : public AQLCalibrateVolatility
{
public:
	// constructor
	explicit AQLCalibrateVolatilityPtberg(void);
	// destructor
	virtual ~AQLCalibrateVolatilityPtberg(void);
	// copy constructor
	AQLCalibrateVolatilityPtberg(const AQLCalibrateVolatilityPtberg &rhs);
	AQLCalibrateVolatilityPtberg &operator=(const AQLCalibrateVolatilityPtberg &rhs);

	//==============================================
	// create  Volatility from grid_t and Volatility of DoubleMatrix vector
	virtual void createVolatility(DoubleArray &grid_t, std::vector<DoubleMatrix> &vol, const AQLStringVector &filePath, 
								const AQLScenarioParam *param = 0, AQLObjectPool *objPool = 0) const ;
	//==============================================
	// create Volatility as function matrix
	virtual void createVolatility(std::vector<std::vector<AQLFunctionBase *> > &vol, const AQLStringVector &filePath, 
								const AQLScenarioParam *param = 0, AQLObjectPool *objPool = 0) const ;
	//==============================================
	// create Volatility from grid_t and Volatility of Function vector
	virtual void createVolatility(DoubleArray &grid_T, std::vector<AQLFunctionBase *> &vol, const AQLStringVector &filePath, 
								const AQLScenarioParam *param = 0, AQLObjectPool *objPool = 0) const ;
	//==============================================
	// create Volatility from grid_t and Volatility of DoubleMatrix
	virtual void createVolatility(DoubleArray &grid_t, DoubleMatrix &vol, const AQLStringVector &filePath, 
								const AQLScenarioParam *param = 0, AQLObjectPool *objPool = 0) const ;
	//==============================================
	// create Volatility as function vector
	virtual void createVolatility(std::vector<AQLFunctionBase *> &vol, const AQLStringVector &filePath, 
								const AQLScenarioParam *param = 0, AQLObjectPool *objPool = 0, int gridPos = -1) const ;
	//==============================================
	// create Volatility from grid_t and Volatility of function
	virtual void createVolatility(DoubleArray &grid_T, AQLFunctionBase *vol, const AQLStringVector &filePath, 
								const AQLScenarioParam *param = 0, AQLObjectPool *objPool = 0) const ;
	//==============================================
	// create Volatility from grid_t and Volatility of vector
	virtual void createVolatility (DoubleArray &grid_t, DoubleArray &vol, const AQLStringVector &filePath, 
								const AQLScenarioParam *param = 0, AQLObjectPool *objPool = 0) const ;
	//==============================================
	// create Volatility as function for fx
	virtual AQLFunctionBase *createVolatility (const AQLStringVector &filePath, 
								const AQLScenarioParam *param = 0, AQLObjectPool *objPool = 0, int gridPos = -1) const ;
	//==============================================
	// create Volatility as double
	virtual void createVolatility (double &vol, const AQLStringVector &filePath, 
								const AQLScenarioParam *param = 0, AQLObjectPool *objPool = 0) const;

	//==============================================
	// get grid_T
	virtual void getGrid_T (DoubleArray &grid_T, const AQLStringVector &filePath) const;

};


#endif
