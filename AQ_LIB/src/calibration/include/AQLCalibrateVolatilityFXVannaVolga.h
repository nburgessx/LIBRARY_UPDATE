/*! @file
    @brief Volatility data create class for ptberg
*/
//  2011, AlgoQuantHub.
#ifndef AQLCalibrateVolatilityFXVannaVolga_h
#define AQLCalibrateVolatilityFXVannaVolga_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLCalibrateVolatilityFXVannaVolga.h
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

#include "AQLCalibrateVolatility.h"
//// FROTOTYPE ////

//===================== Class Declare AQLCalibrateVolatilityFXVannaVolga==================================
/*! 
    @brief Volatility data create class for ptberg
	

*/
class AQLCalibrateVolatilityFXVannaVolga : public AQLCalibrateVolatility
{
public:
	// constructor
	explicit AQLCalibrateVolatilityFXVannaVolga(void);
	// destructor
	virtual ~AQLCalibrateVolatilityFXVannaVolga(void);
	// copy constructor
	AQLCalibrateVolatilityFXVannaVolga(const AQLCalibrateVolatilityFXVannaVolga &rhs);
	AQLCalibrateVolatilityFXVannaVolga &operator=(const AQLCalibrateVolatilityFXVannaVolga &rhs);

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
