/*! @file
    @brief Volatility data create class for hw
*/
//  2008, AlgoQuantHub.
#ifndef LACalibrateVolatilityHW_h
#define LACalibrateVolatilityHW_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateVolatilityHW.h
//
//  DESCRIPTION :         Volatility data create class for hw
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LACalibrateVolatility.h"


//===================== Class Declare LACalibrateVolatilityHW==================================
/*! 
    @brief Volatility data create class for hw
	

*/
class LACalibrateVolatilityHW : public LACalibrateVolatility
{
public:
	// constructor
	explicit LACalibrateVolatilityHW(void);
	// destructor
	virtual ~LACalibrateVolatilityHW(void);
	// copy constructor
	LACalibrateVolatilityHW(const LACalibrateVolatilityHW &rhs);
	LACalibrateVolatilityHW &operator=(const LACalibrateVolatilityHW &rhs);

	//==============================================
	// create  Volatility from grid_t and Volatility of DoubleMatrix vector
	virtual void createVolatility(DoubleArray &grid_t, std::vector<DoubleMatrix> &vol, const AQLStringVector &filePath, 
								const MAScenarioParam *param = 0, AQLObjectPool *objPool = 0) const ;
	//==============================================
	// create Volatility as function matrix
	virtual void createVolatility(std::vector<std::vector<AQLFunctionBase *> > &vol, const AQLStringVector &filePath, 
								const MAScenarioParam *param = 0, AQLObjectPool *objPool = 0) const ;
	//==============================================
	// create Volatility from grid_t and Volatility of Function vector
	virtual void createVolatility(DoubleArray &grid_T, std::vector<AQLFunctionBase *> &vol, const AQLStringVector &filePath, 
								const MAScenarioParam *param = 0, AQLObjectPool *objPool = 0) const ;
	//==============================================
	// create Volatility from grid_t and Volatility of DoubleMatrix
	virtual void createVolatility(DoubleArray &grid_t, DoubleMatrix &vol, const AQLStringVector &filePath, 
								const MAScenarioParam *param = 0, AQLObjectPool *objPool = 0) const ;
	//==============================================
	// create Volatility as function vector
	virtual void createVolatility(std::vector<AQLFunctionBase *> &vol, const AQLStringVector &filePath, 
								const MAScenarioParam *param = 0, AQLObjectPool *objPool = 0, int gridPos = -1) const ;
	//==============================================
	// create Volatility from grid_t and Volatility of function
	virtual void createVolatility(DoubleArray &grid_T, AQLFunctionBase *vol, const AQLStringVector &filePath, 
								const MAScenarioParam *param = 0, AQLObjectPool *objPool = 0) const ;
	//==============================================
	// create Volatility from grid_t and Volatility of vector
	virtual void createVolatility (DoubleArray &grid_t, DoubleArray &vol, const AQLStringVector &filePath, 
								const MAScenarioParam *param = 0, AQLObjectPool *objPool = 0) const ;
	//==============================================
	// create Volatility as function for fx
	virtual AQLFunctionBase *createVolatility (const AQLStringVector &filePath, 
								const MAScenarioParam *param = 0, AQLObjectPool *objPool = 0, int gridPos = -1) const ;
	//==============================================
	// create Volatility as double
	virtual void createVolatility (double &vol, const AQLStringVector &filePath, 
								const MAScenarioParam *param = 0, AQLObjectPool *objPool = 0) const;
	//==============================================
	// get grid_T
	virtual void getGrid_T (DoubleArray &grid_T, const AQLStringVector &filePath) const;


};


#endif
