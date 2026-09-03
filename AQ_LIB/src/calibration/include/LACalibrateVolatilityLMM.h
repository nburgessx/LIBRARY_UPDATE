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
	//==============================================
	// set interpolation method of adjust parameter
	virtual void setAdjParamInterpolation(const LAString &adjParamInterpolationStr);
	//==============================================
	// set frequency of adjust parameter input
	virtual void setAdjParamFrequency(const LAString &adjParamFrequency);

private:
	//==============================================
	// get grid info
	void getGridInfo(DoubleArray &tenor_30_360, DoubleArray &tenor, DoubleArray &deltatenor, BoolVector &exFlag) const;
	//==============================================================================
	// create volatility function
	std::vector<LAFunctionBase *> createVolFunc(const LAString &calibFileName, const LAString &adjFileName, 
								const LAString &maxFileName, const LAString &skewFileName, const DoubleArray &tenor_30_360, const DoubleArray &tenor, const BoolVector &exFlag) const;

	LAString mAdjParamInterpolationStr;
	LAString mAdjParamFrequency;

};


#endif
