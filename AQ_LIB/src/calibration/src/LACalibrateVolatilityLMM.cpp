/*! @file
    @brief Volatility data create class for lmm
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateVolatilityLMM.cpp
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


#include "LACalibrateVolatilityLMM.h"
#include "LACoreDataService.h"
#include "LAMarketData.h"
#include "LAMarketDataLMM.h"
#include "LAScenarioConfiguration.h"
#include "LADate.h"
#include "LABasic.h"
#include "LAPriceDataDayCount.h"
#include "LAMathInterpolationUtilities.h"
#include "LAMathVolFuncStructureBase.h"
#include "LAMathVolFuncWave.h"
#include "LAMathVolFuncLMM.h"
#include "LACalibratePool.h"
#include "LACalibrateLMM.h"
#include "LACalibrationFunc.h"
#include "LACalibrationParametersLMM.h"
#include "LADefinitionsCalibration.h"
#include "LAPriceLMMCalibration.h"


using namespace std;

// constructor
/*!
	@param funFlg

*/
LACalibrateVolatilityLMM::LACalibrateVolatilityLMM(void)
: mAdjParamInterpolationStr(""),
  mAdjParamFrequency("")
{
}

// destructor
/*!

*/
LACalibrateVolatilityLMM::~LACalibrateVolatilityLMM(void)
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
LACalibrateVolatilityLMM::createVolatility(DoubleArray &grid_t, vector<DoubleMatrix> &vol, const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool) const
{
	grid_t, vol, filePath, param, objPool;
	throw LACoreInvalidData("This create method is not support in lmm.", __FILE__, __LINE__);
}

/*!
    @brief create Volatility as function matrix (T*i*f(t))

	@param[out] vol		Volatility as function matrix
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
LACalibrateVolatilityLMM::createVolatility(vector<vector<LAFunctionBase *> > &vol, const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool) const
{
	vol, filePath, param, objPool;
	throw LACoreInvalidData("This create method is not support in lmm.", __FILE__, __LINE__);
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
LACalibrateVolatilityLMM::createVolatility(DoubleArray &grid_T, vector<LAFunctionBase *> &vol, const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool) const
{
	grid_T, vol, filePath, param, objPool;
	throw LACoreInvalidData("This create method is not support in lmm.", __FILE__, __LINE__);
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
LACalibrateVolatilityLMM::createVolatility(DoubleArray &grid_t, DoubleMatrix &vol, const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool) const
{
	param, objPool;
	if (filePath.size() != 4)
	{
		throw LACoreInvalidData("FilePath size must be 4, calibfile, adjfile, maxfile, skewfile.", __FILE__, __LINE__);
	}
	vol.clear();
	grid_t.clear();

	// get tenor info
	DoubleArray tenor_30_360;
	DoubleArray tenor;
	DoubleArray deltatenor;
	BoolVector exFlag;
	getGridInfo(tenor_30_360, tenor, deltatenor, exFlag);

	vector<LAFunctionBase *> volFunc = createVolFunc(filePath[0], filePath[1], filePath[2], filePath[3], tenor_30_360, tenor, exFlag);
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
LACalibrateVolatilityLMM::createVolatility(vector<LAFunctionBase *> &vol, const LAStringVector &filePath, const MAScenarioParam *param, 
										 LAObjectPool *objPool, int gridPos) const
{
	// null check
	if (!param || !objPool)
	{
		throw LACoreInvalidData("Param or entitypool is null.", __FILE__, __LINE__);
	}

	if (param->isCalib)
	{
		// get calibration tenor 
		if (param->refName.empty())
		{
			throw LACoreInvalidData("calibration info name is empty.", __FILE__, __LINE__);
		}
		const LAString &calibInfoName = param->refName[0];
		const LAObject &calibInfo = objPool->getObject(calibInfoName, ENCHKTYPE_ISDEFINED).get();
		const DoubleVector &tenor = dynamic_cast<const LADataDoubles &>(calibInfo.getData(PRICING_DATA_CALIBCANONICAL_T, ISNOTNULL).get()).get();

		LACalibrateLMM *request = new LACalibrateLMM();
		unsigned int size = tenor.size() - 2;
		MACalibrationFunc *method = new MACalibrationFunc[size];

		// set up
		request->setUp(*objPool, *param, method, gridPos);
		if (LACoreDataService::getContext(ARG_KEY_NOCALIBTHREAD) == MLIB_NO_DATA)
		{
			LACalibratePool *reqChannel = LACalibratePool::getInstance();
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
			MACalibrationFunc *f = new MACalibrationFunc(method[i]);
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
			throw LACoreInvalidData("FilePath size must be 4, calibfile, adjfile, maxfile, skewfile.", __FILE__, __LINE__);
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
LACalibrateVolatilityLMM::createVolatility(DoubleArray &grid_T, LAFunctionBase *vol, const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool) const
{
	grid_T, vol, filePath, param, objPool;
	throw LACoreInvalidData("This create method is not support in lmm.", __FILE__, __LINE__);
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
LACalibrateVolatilityLMM::createVolatility(DoubleArray &grid_t, DoubleArray &vol, const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool) const
{
	grid_t, vol, filePath, param, objPool;
	throw LACoreInvalidData("This create method is not support in lmm.", __FILE__, __LINE__);
}

/*!
    @brief create Volatility as function for fx (f(t))

	@param[in] filepath
	@param[in] param
	@param[in] objPool
	@return vol		Volatility as function 

*/
LAFunctionBase * 
LACalibrateVolatilityLMM::createVolatility(const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool, int gridPos) const
{
	filePath, param, objPool;
	throw LACoreInvalidData("This create method is not support in lmm.", __FILE__, __LINE__);
}

/*!
    @brief create Volatility as double

	@param[out] vol		Volatility as double 
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
LACalibrateVolatilityLMM::createVolatility(double &vol, const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool) const
{
	vol, filePath, param, objPool;
	throw LACoreInvalidData("This create method is not support in lmm.", __FILE__, __LINE__);
}

/*!
    @brief get grid_T

	@param[out] grid_T as double 
	@param[in] filepath
*/
void 
LACalibrateVolatilityLMM::getGrid_T(DoubleArray &grid_T, const LAStringVector &filePath) const
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
LACalibrateVolatilityLMM::getGridInfo(DoubleArray &tenor_30_360, DoubleArray &tenor, DoubleArray &deltatenor, BoolVector &exFlag) const
{
	tenor_30_360.clear();
	tenor.clear();
	deltatenor.clear();
	exFlag.clear();

	unsigned int term = LACoreDataService::getContext(CONTEXT_KEY_MAXTERM).getIntValue();
	LAString dayCountStr = LACoreDataService::getContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT);
	LAPriceDataDayCount dayCount;
	dayCount.convertFromString(dayCountStr);
	LADate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	LAString freq = FREQ_SEMI_ANNUAL;
	if (LACoreDataService::getContext(ARG_KEY_CANONICALFREQ) == "QA")
	{
		freq = FREQ_QUARTERLY;
	}
	LAStringVector exTenor = LAMarketDataLMM::getCanonicalGridExTenor();
	const bool isDataOut = (LACoreDataService::getContext(ARG_KEY_DATAOUT) != MLIB_NO_DATA);
	LAMarketDataLMM::getCanonicalGrid(tenor_30_360, tenor, deltatenor, exFlag, asOfDate, dayCount, freq, term, exTenor, isDataOut);
}

/*!
    @brief set adjust param interpolation method

	@param[in] adjParamInterpolationStr

*/
void 
LACalibrateVolatilityLMM::setAdjParamInterpolation(const LAString &adjParamInterpolationStr)
{
	mAdjParamInterpolationStr = adjParamInterpolationStr;
}

/*!
    @brief set adjust param frequency

	@param[in] adjParamInterpolationStr

*/
void 
LACalibrateVolatilityLMM::setAdjParamFrequency(const LAString &adjParamFrequency)
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
	@return vector<LAFunctionBase *> 

*/
vector<LAFunctionBase *> 
LACalibrateVolatilityLMM::createVolFunc(const LAString &calibFileName, const LAString &adjFileName, 
									  const LAString &maxFileName, const LAString &skewFileName, const DoubleArray &tenor_30_360, const DoubleArray &tenor, const BoolVector &exFlag) const
{
	MAFileAccessor calibFile(calibFileName);
	LAStringMatrix paramCalibMtx;
	calibFile.readAllData(MARKET_DATA_DELIMITER, paramCalibMtx);
	calibFile.close();

	MAFileAccessor adjFile(adjFileName);
	LAStringMatrix paramAdjMtx;
	adjFile.readAllData(MARKET_DATA_DELIMITER, paramAdjMtx);
	adjFile.close();

	MAFileAccessor maxFile(maxFileName);
	LAStringMatrix maxMtx;
	maxFile.readAllData(MARKET_DATA_DELIMITER, maxMtx);
	maxFile.close();

	MAFileAccessor skewFile(skewFileName);
	LAStringMatrix skewMtx;
	skewFile.readAllData(MARKET_DATA_DELIMITER, skewMtx);
	skewFile.close();

	// data format check
	// calibration
	const unsigned int CALIBPARAMSIZE = 12;
	if (paramCalibMtx.size() != CALIBPARAMSIZE)
	{
		LAString msg = "LMM calibration file, format is wrong. file = " + calibFileName;
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	for (unsigned int i = 0; i < CALIBPARAMSIZE; ++i)
	{
		if (paramCalibMtx[i].size() != 1)
		{
			LAString msg = "LMM calibration file, format is wrong. file = " + calibFileName;
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
	}
	// adjust
	const unsigned int size = tenor.size() - 2;
	if (paramAdjMtx.size() != 2 || paramAdjMtx[0].size() < size)
	{
		LAString msg = "LMM volatility adjust file, format is wrong. file = " + adjFileName;
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	// tmax 
	if (maxMtx.size() != 1 || maxMtx[0].size() != 1)
	{
		LAString msg = "LMM tmax file, format is wrong. file = " + maxFileName;
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	// skew
	if (skewMtx.size() != 1 || skewMtx[0].size() != 1)
	{
		LAString msg = "LMM skew file, format is wrong. file = " + skewFileName;
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
		LAString msg = "Q = " + LAString(skew, 2) + " is out of range. Q must be in (0, 2).";
		throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}

	vector<LAFunctionBase *> volVec(size);
	// set vol
	LAFunctionBase *funcTerm = new LAMathVolFuncWave(tmax, decay, amp1, phase1, 
														amp2, phase2, amp3, phase3, shift);
	LAFunctionBase *funcTenor = new LAMathVolFuncStructureBase(a, b, c, d);

	const double qval = 1.0 / (1.0 - LAMath::log(skew) / LAMath::log(2.0));

	//const double f_adjParam = paramAdjMtx[0][0].trimLeft().trimRight().getDoubleValue() * qval;
	if (paramAdjMtx[0].size() != paramAdjMtx[1].size())
	{
		LAString msg = "Size of adjust tenor and adjust parameter are different.";
		throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
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
	if (mAdjParamFrequency == LACoreDataService::getContext(ARG_KEY_CANONICALFREQ))
	{
		extG = adjParamMat[1];
	}
	else
	{
		std::shared_ptr<LAInterpolationBase> inter_adjParam = LAMathInterpolationUtilities::createInterpolation(mAdjParamInterpolationStr);
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
			volVec[i] = new LAMathVolFuncLMM(funcTenor, funcTerm, tenor, tenor_30_360, i + 1, adjParam);
		}
		else
		{
			volVec[i] = new LAMathVolFuncLMM(funcTenor, funcTerm, tenor, tenor_30_360, i + 1, adjParam, true);
		}
	}

	return volVec;

}

