/*! @file
    @brief Volatility data create class for lmm
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLCalibrateVolatilityLMM.cpp
//
//  DESCRIPTION :      Volatility data create class for lmm
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLCalibrateVolatilityLMM.h"
#include "AQLCoreDataService.h"
#include "AQLMarketData.h"
#include "AQLMarketDataLMM.h"
#include "AQLScenarioConfiguration.h"
#include "AQLDate.h"
#include "AQLBasic.h"
#include "AQLPriceDataDayCount.h"
#include "AQLMathInterpolationUtilities.h"
#include "AQLMathVolFuncStructureBase.h"
#include "AQLMathVolFuncWave.h"
#include "AQLMathVolFuncLMM.h"
#include "AQLCalibratePool.h"
#include "AQLCalibrateLMM.h"
#include "AQLCalibrationFunc.h"
#include "AQLCalibrationParametersLMM.h"
#include "AQLDefinitionsCalibration.h"
#include "AQLPriceLMMCalibration.h"


using namespace std;

// constructor
/*!
	@param funFlg

*/
AQLCalibrateVolatilityLMM::AQLCalibrateVolatilityLMM(void)
: mAdjParamInterpolationStr(""),
  mAdjParamFrequency("")
{
}

// destructor
/*!

*/
AQLCalibrateVolatilityLMM::~AQLCalibrateVolatilityLMM(void)
{
}


 
/*!
    @brief create Volatility from grid_t and Volatility of DoubleMatrix vector (T*i*t)

	@param[out] grid_t	grid of t
	@param[out] vol		Volatility of DoubleMatrix vector
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
AQLCalibrateVolatilityLMM::createVolatility(DoubleArray &grid_t, vector<DoubleMatrix> &vol, const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool) const
{
	grid_t, vol, filePath, param, objPool;
	throw AQLCoreInvalidData("This create method is not support in lmm.", __FILE__, __LINE__);
}

/*!
    @brief create Volatility as function matrix (T*i*f(t))

	@param[out] vol		Volatility as function matrix
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
AQLCalibrateVolatilityLMM::createVolatility(vector<vector<AQLFunctionBase *> > &vol, const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool) const
{
	vol, filePath, param, objPool;
	throw AQLCoreInvalidData("This create method is not support in lmm.", __FILE__, __LINE__);
}


/*!
    @brief create Volatility from grid_t and Volatility of Function vector (i*f(t,T))

	@param[out] grid_T	grid of T
	@param[out] vol		Volatility of Function vector
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
AQLCalibrateVolatilityLMM::createVolatility(DoubleArray &grid_T, vector<AQLFunctionBase *> &vol, const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool) const
{
	grid_T, vol, filePath, param, objPool;
	throw AQLCoreInvalidData("This create method is not support in lmm.", __FILE__, __LINE__);
}


/*!
    @brief create Volatility from grid_t and Volatility of DoubleMatrix (T*t)

	@param[out] grid of t
	@param[out] vol		Volatility of DoubleMatrix
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
AQLCalibrateVolatilityLMM::createVolatility(DoubleArray &grid_t, DoubleMatrix &vol, const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool) const
{
	param, objPool;
	if (filePath.size() != 4)
	{
		throw AQLCoreInvalidData("FilePath size must be 4, calibfile, adjfile, maxfile, skewfile.", __FILE__, __LINE__);
	}
	vol.clear();
	grid_t.clear();

	// get tenor info
	DoubleArray tenor_30_360;
	DoubleArray tenor;
	DoubleArray deltatenor;
	BoolVector exFlag;
	getGridInfo(tenor_30_360, tenor, deltatenor, exFlag);

	vector<AQLFunctionBase *> volFunc = createVolFunc(filePath[0], filePath[1], filePath[2], filePath[3], tenor_30_360, tenor, exFlag);
	unsigned int marketSize = 0;
	if (tenor[0] == 0.0)
	{
		marketSize = tenor.size() - 2;
	}
	else
	{
		marketSize = tenor.size() - 1;
	}
	tenor.erase(--tenor.end());
	const unsigned int tSize = tenor.size();
		
	vol.resize(marketSize);
	// set volatility val
	for (unsigned int i = 0; i < marketSize; ++i)
	{
		vol[i].resize(tSize, 0.0);
		for (unsigned int j = 0; j < tSize; ++j)
		{
			vol[i][j] = volFunc[i]->operator ()(tenor[j]);
		}
	}
	// delete
	for (unsigned int i = 0; i < marketSize; ++i)
	{
		delete volFunc[i];
	}
	// set grid_t
	grid_t = tenor;
}


/*!
    @brief create Volatility as function vector (T*f(t))

	@param[out] vol		Volatility as function vector
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
AQLCalibrateVolatilityLMM::createVolatility(vector<AQLFunctionBase *> &vol, const AQLStringVector &filePath, const AQLScenarioParam *param, 
										 AQLObjectPool *objPool, int gridPos) const
{
	// null check
	if (!param || !objPool)
	{
		throw AQLCoreInvalidData("Param or entitypool is null.", __FILE__, __LINE__);
	}

	if (param->isCalib)
	{
		// get calibration tenor 
		if (param->refName.empty())
		{
			throw AQLCoreInvalidData("calibration info name is empty.", __FILE__, __LINE__);
		}
		const AQLString &calibInfoName = param->refName[0];
		const AQLObject &calibInfo = objPool->getObject(calibInfoName, ENCHKTYPE_ISDEFINED).get();
		const DoubleVector &tenor = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(PRICING_DATA_CALIBCANONICAL_T, ISNOTNULL).get()).get();

		AQLCalibrateLMM *request = new AQLCalibrateLMM();
		unsigned int size = tenor.size() - 2;
		AQLCalibrationFunc *method = new AQLCalibrationFunc[size];

		// set up
		request->setUp(*objPool, *param, method, gridPos);
		if (AQLCoreDataService::getContext(ARG_KEY_NOCALIBTHREAD) == AQ_NO_DATA)
		{
			AQLCalibratePool *reqChannel = AQLCalibratePool::getInstance();
			// put request ,calibration will be done in differenct thread.
			reqChannel->putRequest(request);
		}
		else
		{
			// calibrate in single thread
			request->doCalibrate();
			delete request;
		}

		vol.resize(size);
		for(unsigned int i = 0; i < size; i++)
		{
			AQLCalibrationFunc *f = new AQLCalibrationFunc(method[i]);
			vol[i] = f;
		}

		delete[] method;

	}
	else
	{
		// get tenor info
		DoubleArray tenor_30_360;
		DoubleArray tenor;
		DoubleArray deltatenor;
		BoolVector exFlag;

		getGridInfo(tenor_30_360, tenor, deltatenor, exFlag);

		if (filePath.size() != 4)
		{
			throw AQLCoreInvalidData("FilePath size must be 4, calibfile, adjfile, maxfile, skewfile.", __FILE__, __LINE__);
		}
		vol.clear();

		vol = createVolFunc(filePath[0], filePath[1], filePath[2], filePath[3], tenor_30_360, tenor, exFlag);
	}

}


/*!
    @brief create Volatility from grid_t and Volatility of function (f(t,T))

	@param[out] grid_T	grid of T
	@param[out] vol		Volatility as function vector
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
AQLCalibrateVolatilityLMM::createVolatility(DoubleArray &grid_T, AQLFunctionBase *vol, const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool) const
{
	grid_T, vol, filePath, param, objPool;
	throw AQLCoreInvalidData("This create method is not support in lmm.", __FILE__, __LINE__);
}


/*!
    @brief create Volatility from grid_t and Volatility of vector (t)

	@param[out] grid_t	grid of t
	@param[out] vol		Volatility as vector
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
AQLCalibrateVolatilityLMM::createVolatility(DoubleArray &grid_t, DoubleArray &vol, const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool) const
{
	grid_t, vol, filePath, param, objPool;
	throw AQLCoreInvalidData("This create method is not support in lmm.", __FILE__, __LINE__);
}

/*!
    @brief create Volatility as function for fx (f(t))

	@param[in] filepath
	@param[in] param
	@param[in] objPool
	@return vol		Volatility as function 

*/
AQLFunctionBase * 
AQLCalibrateVolatilityLMM::createVolatility(const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool, int gridPos) const
{
	filePath, param, objPool;
	throw AQLCoreInvalidData("This create method is not support in lmm.", __FILE__, __LINE__);
}

/*!
    @brief create Volatility as double

	@param[out] vol		Volatility as double 
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
AQLCalibrateVolatilityLMM::createVolatility(double &vol, const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool) const
{
	vol, filePath, param, objPool;
	throw AQLCoreInvalidData("This create method is not support in lmm.", __FILE__, __LINE__);
}

/*!
    @brief get grid_T

	@param[out] grid_T as double 
	@param[in] filepath
*/
void 
AQLCalibrateVolatilityLMM::getGrid_T(DoubleArray &grid_T, const AQLStringVector &filePath) const
{
	filePath;
	grid_T.clear();
	// get tenor info
	DoubleArray tenor_30_360;
	DoubleArray deltatenor;
	BoolVector exFlag;
	getGridInfo(tenor_30_360, grid_T, deltatenor, exFlag);

	grid_T.erase(--grid_T.end());
	if (grid_T[0] == 0.0)
	{
		grid_T.erase(grid_T.begin());
	}
}

/*!
    @brief get grid info

	@param[out] tenor_30_360
	@param[out] tenor
	@param[out] deltatenor
	@param[out] exflag

*/
void 
AQLCalibrateVolatilityLMM::getGridInfo(DoubleArray &tenor_30_360, DoubleArray &tenor, DoubleArray &deltatenor, BoolVector &exFlag) const
{
	tenor_30_360.clear();
	tenor.clear();
	deltatenor.clear();
	exFlag.clear();

	unsigned int term = AQLCoreDataService::getContext(CONTEXT_KEY_MAXTERM).getIntValue();
	AQLString dayCountStr = AQLCoreDataService::getContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT);
	AQLPriceDataDayCount dayCount;
	dayCount.convertFromString(dayCountStr);
	AQLDate asOfDate(AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	AQLString freq = FREQ_SEMI_ANNUAL;
	if (AQLCoreDataService::getContext(ARG_KEY_CANONICALFREQ) == "QA")
	{
		freq = FREQ_QUARTERLY;
	}
	AQLStringVector exTenor = AQLMarketDataLMM::getCanonicalGridExTenor();
	const bool isDataOut = (AQLCoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA);
	AQLMarketDataLMM::getCanonicalGrid(tenor_30_360, tenor, deltatenor, exFlag, asOfDate, dayCount, freq, term, exTenor, isDataOut);
}

/*!
    @brief set adjust param interpolation method

	@param[in] adjParamInterpolationStr

*/
void 
AQLCalibrateVolatilityLMM::setAdjParamInterpolation(const AQLString &adjParamInterpolationStr)
{
	mAdjParamInterpolationStr = adjParamInterpolationStr;
}

/*!
    @brief set adjust param frequency

	@param[in] adjParamInterpolationStr

*/
void 
AQLCalibrateVolatilityLMM::setAdjParamFrequency(const AQLString &adjParamFrequency)
{
	mAdjParamFrequency = adjParamFrequency;
}


/*!
    @brief create vol method

	@param[in] calibfilename
	@param[in] adjfilename
	@param[in] maxfilename
	@param[in] skewfilename
	@param[in] tenor
	@param[in] tenor_30_360
	@return vector<AQLFunctionBase *> 

*/
vector<AQLFunctionBase *> 
AQLCalibrateVolatilityLMM::createVolFunc(const AQLString &calibFileName, const AQLString &adjFileName, 
									  const AQLString &maxFileName, const AQLString &skewFileName, const DoubleArray &tenor_30_360, const DoubleArray &tenor, const BoolVector &exFlag) const
{
	AQLFileAccessor calibFile(calibFileName);
	AQLStringMatrix paramCalibMtx;
	calibFile.readAllData(MARKET_DATA_DELIMITER, paramCalibMtx);
	calibFile.close();

	AQLFileAccessor adjFile(adjFileName);
	AQLStringMatrix paramAdjMtx;
	adjFile.readAllData(MARKET_DATA_DELIMITER, paramAdjMtx);
	adjFile.close();

	AQLFileAccessor maxFile(maxFileName);
	AQLStringMatrix maxMtx;
	maxFile.readAllData(MARKET_DATA_DELIMITER, maxMtx);
	maxFile.close();

	AQLFileAccessor skewFile(skewFileName);
	AQLStringMatrix skewMtx;
	skewFile.readAllData(MARKET_DATA_DELIMITER, skewMtx);
	skewFile.close();

	// data format check
	// calibration
	const unsigned int CALIBPARAMSIZE = 12;
	if (paramCalibMtx.size() != CALIBPARAMSIZE)
	{
		AQLString msg = "LMM calibration file, format is wrong. file = " + calibFileName;
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	for (unsigned int i = 0; i < CALIBPARAMSIZE; ++i)
	{
		if (paramCalibMtx[i].size() != 1)
		{
			AQLString msg = "LMM calibration file, format is wrong. file = " + calibFileName;
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
	}
	// adjust
	const unsigned int size = tenor.size() - 2;
	if (paramAdjMtx.size() != 2 || paramAdjMtx[0].size() < size)
	{
		AQLString msg = "LMM volatility adjust file, format is wrong. file = " + adjFileName;
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	// tmax 
	if (maxMtx.size() != 1 || maxMtx[0].size() != 1)
	{
		AQLString msg = "LMM tmax file, format is wrong. file = " + maxFileName;
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	// skew
	if (skewMtx.size() != 1 || skewMtx[0].size() != 1)
	{
		AQLString msg = "LMM skew file, format is wrong. file = " + skewFileName;
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	// calib param
	const double a = paramCalibMtx[0][0].trimLeft().trimRight().getDoubleValue();
	const double b = paramCalibMtx[1][0].trimLeft().trimRight().getDoubleValue();
	const double c = paramCalibMtx[2][0].trimLeft().trimRight().getDoubleValue();
	const double d = paramCalibMtx[3][0].trimLeft().trimRight().getDoubleValue();
	const double decay = paramCalibMtx[4][0].trimLeft().trimRight().getDoubleValue();
	const double amp1 = paramCalibMtx[5][0].trimLeft().trimRight().getDoubleValue();
	const double phase1 = paramCalibMtx[6][0].trimLeft().trimRight().getDoubleValue();
	const double amp2 = paramCalibMtx[7][0].trimLeft().trimRight().getDoubleValue();
	const double phase2 = paramCalibMtx[8][0].trimLeft().trimRight().getDoubleValue();
	const double amp3 = paramCalibMtx[9][0].trimLeft().trimRight().getDoubleValue();
	const double phase3 = paramCalibMtx[10][0].trimLeft().trimRight().getDoubleValue();
	const double shift = paramCalibMtx[11][0].trimLeft().trimRight().getDoubleValue();

	// tmax
	const double tmax = maxMtx[0][0].getDoubleValue();
	// skew
	const double skew = skewMtx[0][0].getDoubleValue();
	if (skew <= 0. || skew >= 2.)
	{
		AQLString msg = "Q = " + AQLString(skew, 2) + " is out of range. Q must be in (0, 2).";
		throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}

	vector<AQLFunctionBase *> volVec(size);
	// set vol
	AQLFunctionBase *funcTerm = new AQLMathVolFuncWave(tmax, decay, amp1, phase1, 
														amp2, phase2, amp3, phase3, shift);
	AQLFunctionBase *funcTenor = new AQLMathVolFuncStructureBase(a, b, c, d);

	const double qval = 1.0 / (1.0 - AQLMath::log(skew) / AQLMath::log(2.0));

	//const double f_adjParam = paramAdjMtx[0][0].trimLeft().trimRight().getDoubleValue() * qval;
	if (paramAdjMtx[0].size() != paramAdjMtx[1].size())
	{
		AQLString msg = "Size of adjust tenor and adjust parameter are different.";
		throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}
	vector<vector<double> > adjParamMat(2, vector<double>(paramAdjMtx[0].size()));
	for (unsigned int i = 0; i < adjParamMat[0].size(); i++)
	{
		for (unsigned int j = 0; j < 2; ++j)
		{
			adjParamMat[j][i] = paramAdjMtx[j][i].trimLeft().trimRight().getDoubleValue();
		}
	}

	DoubleVector extG;
	if (mAdjParamFrequency == AQLCoreDataService::getContext(ARG_KEY_CANONICALFREQ))
	{
		extG = adjParamMat[1];
	}
	else
	{
		std::shared_ptr<AQLInterpolationBase> inter_adjParam = AQLMathInterpolationUtilities::createInterpolation(mAdjParamInterpolationStr);
		inter_adjParam->set(adjParamMat[0], adjParamMat[1]);

		const size_t extG_size = tenor.size() - 1;
		extG.resize(extG_size);

		for ( size_t i = 0; i < extG_size; ++i )
		{
			extG[i] = inter_adjParam->value(tenor[i + 1]);
		}
	}

	for (unsigned int i = 0; i < size; ++i)
	{
		double adjParam = extG[i] * qval;

		if (i != 0)
		{	
			volVec[i] = new AQLMathVolFuncLMM(funcTenor, funcTerm, tenor, tenor_30_360, i + 1, adjParam);
		}
		else
		{
			volVec[i] = new AQLMathVolFuncLMM(funcTenor, funcTerm, tenor, tenor_30_360, i + 1, adjParam, true);
		}
	}

	return volVec;

}

