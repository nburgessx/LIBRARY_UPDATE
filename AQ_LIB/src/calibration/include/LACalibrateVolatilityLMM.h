/*! @file
    @brief Volatility data create class for lmm
*/
//  2008, AlgoQuantHub.
#ifndef LACalibrateVolatilityLMM_h
#define LACalibrateVolatilityLMM_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateVolatilityLMM.h
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


//===================== Class Declare LACalibrateVolatilityLMM==================================
/*! 
    @brief Volatility data create class for lmm
	

*/
class LACalibrateVolatilityLMM : public LACalibrateVolatility
{
public:
	// constructor
	explicit LACalibrateVolatilityLMM(void);
	// destructor
	virtual ~LACalibrateVolatilityLMM(void);
	// copy constructor
	LACalibrateVolatilityLMM(const LACalibrateVolatilityLMM &rhs);
	LACalibrateVolatilityLMM &operator=(const LACalibrateVolatilityLMM &rhs);

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
	//==============================================
	// set interpolation method of adjust parameter
	virtual void setAdjParamInterpolation(const AQLString &adjParamInterpolationStr);
	//==============================================
	// set frequency of adjust parameter input
	virtual void setAdjParamFrequency(const AQLString &adjParamFrequency);

private:
	//==============================================
	// get grid info
	void getGridInfo(DoubleArray &tenor_30_360, DoubleArray &tenor, DoubleArray &deltatenor, BoolVector &exFlag) const;
	//==============================================================================
	// create volatility function
	std::vector<AQLFunctionBase *> createVolFunc(const AQLString &calibFileName, const AQLString &adjFileName, 
								const AQLString &maxFileName, const AQLString &skewFileName, const DoubleArray &tenor_30_360, const DoubleArray &tenor, const BoolVector &exFlag) const;

	AQLString mAdjParamInterpolationStr;
	AQLString mAdjParamFrequency;

};


#endif
