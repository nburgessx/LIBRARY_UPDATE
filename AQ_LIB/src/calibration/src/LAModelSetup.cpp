#define _HAS_STD_BYTE 0

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLMathYieldCurve.h"
#include "AQLMathYieldCurvePro.h"
#include "AQLMathJamshidianSwaption.h"
#include "LAModelSetup.h"
#include "LACoreDataService.h"
#include "LAMarketData.h"
#include "LADefinitions.h"
#include "LADefinitionsRisk.h"
#include "LADefinitionsCalibration.h"
#include "LADefinitionsPtberg.h"
#include "LAStaticData.h"

#include <memory>
#include <sstream>

using namespace std;

const AQLString LF = "\n";

void
LAModelSetup::
setUpCalibMktSkewVol(AQLDataInstance &dataInstance,
					const AQLDate& asofDate,
					const AQLString& fx,
					const double spotFX,
					const AQLString& curveType_d,
					const AQLString& curveType_f,
					const AQLStringVector& terms,
					const DoubleMatrix& weightMtx,
					const DoubleMatrix& volMtx,
					const DoubleArray& initialValues,
					const double boundaryMaxSkew,
					const double boundaryMinSkew,
					const double boundaryMaxVol,
					const double boundaryMinVol,
					unsigned int maxIteration,
					unsigned int maxStationaryStateIteration,
					const double rootEpsilon,
					const double functionEpsilon,
					const double gradientNormEpsilon,
					const AQLString& type,
					const double maxTerm,
					const AQLStringVector& spotOrFwd,
					const AQLStringVector& isFitATM,
					const AQLStringVector& termBeta,
					const AQLStringVector& isDeltaNeutral)
{
	LACoreDataService::setContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT, DAYCOUNT_ACT_365_ISDA);
	//max term
	LACoreDataService::setContext(CONTEXT_KEY_MAXTERM, AQLString(maxTerm));

	asofDate;
	// data check
	unsigned int size = terms.size();
	if (weightMtx.size() != size)
	{
		throw AQLCoreInvalidData("weight matrix size is invalid", __FILE__, __LINE__);
	}
	if (volMtx.size() != size)
	{
		throw AQLCoreInvalidData("volatility matrix size is invalid", __FILE__, __LINE__);
	}
	if (initialValues.size() != 2)
	{
		throw AQLCoreInvalidData("initialvalue array size must be 2", __FILE__, __LINE__);
	}
	if (spotOrFwd.size() != size)
	{
		throw AQLCoreInvalidData("spotOrFwd array size is invalid", __FILE__, __LINE__);
	}
	if (isFitATM.size() != size)
	{
		throw AQLCoreInvalidData("isFitATM array size is invalid", __FILE__, __LINE__);
	}
	if (termBeta.size() != size)
	{
		throw AQLCoreInvalidData("termBeta array size is invalid", __FILE__, __LINE__);
	}
	if (isDeltaNeutral.size() != size)
	{
		throw AQLCoreInvalidData("isDeltaNeutral array size is invalid", __FILE__, __LINE__);
	}

	// do preparation
	setUpCalibPreparation(dataInstance, fx);
	AQLString tmpFX = fx;
	tmpFX.toLower();

	AQLStringVector ccys;
	LAMarketData::convertToCurrency(fx, ccys);
	ccys[0].toLower(), ccys[1].toLower();

	//set volatility file
	LAStaticData &calibProp = LACoreDataService::getStaticDataManager().getCalibStaticData();
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_VOLATILITY_FILE, "data/in/" + ccys[0] + "_" + ccys[1] + "_fxvol.csv");
	AQLString volFileName = calibProp.getStaticData(tmpFX + FX_KEY_CALIB_VOLATILITY_FILE);
	
	AQLString fileCont = ",10DLow_Vol,25DLow_Vol,ATM_Vol,25DHigh_Vol,10DHigh_Vol\n";
	AQLString termStr = "";
	for (unsigned int i = 0; i < size; ++i)
	{
		// create vol file contents
		if (volMtx[i].size() != 5)
		{
			throw AQLCoreInvalidData("volatility matrix size is invalid", __FILE__, __LINE__);
		}
		termStr += terms[i] + ":";
		fileCont += terms[i];
		fileCont += "," + AQLString(volMtx[i][0]) + "," + AQLString(volMtx[i][1]) + "," 
			+ AQLString(volMtx[i][2]) + "," + AQLString(volMtx[i][3]) + "," + AQLString(volMtx[i][4]) + LF;

		if (weightMtx[i].size() != 5)
		{
			throw AQLCoreInvalidData("weightMtx matrix size is invalid", __FILE__, __LINE__);
		}
		AQLString tmpTerm = terms[i];
		tmpTerm.toLower();
		// set weight
		calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_10DL_WEIGHT + "." + tmpTerm, AQLString(weightMtx[i][0]));
		calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_25DL_WEIGHT + "." + tmpTerm, AQLString(weightMtx[i][1]));
		calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_ATM_WEIGHT + "." + tmpTerm, AQLString(weightMtx[i][2]));
		calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_25DH_WEIGHT + "." + tmpTerm, AQLString(weightMtx[i][3]));
		calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_10DH_WEIGHT + "." + tmpTerm, AQLString(weightMtx[i][4]));
		// set calib config
		calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_SPOTORFWD + "." + tmpTerm, spotOrFwd[i]);
		calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_ISFITATM + "." + tmpTerm, isFitATM[i]);
		calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_TERMBETA + "." + tmpTerm, termBeta[i]);
		calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_ISDELTANEUTRAL + "." + tmpTerm, isDeltaNeutral[i]);
	}
	istringstream *pVolStream = new istringstream(fileCont.getCString());
	LACoreDataService::setIStringStream(LAMarketData::getNumFileName(volFileName), pVolStream);
	fileCont = "";
	// spot fx
	LAStaticData &irProp = LACoreDataService::getStaticDataManager().getStaticData();
	setStaticDataValue(irProp, KEY_FXSPOTRATES_FILE, "data/in/spotrate.csv");
	AQLString fxFileName = irProp.getStaticData(KEY_FXSPOTRATES_FILE);
	fileCont = ccys[0].toUpper() + "," + ccys[1].toUpper()  + "," + AQLString(spotFX) + LF;
	if (ccys[0] != CURRENCY_USD && ccys[1] != CURRENCY_USD)
	{
		fileCont += ccys[0] + "," + CURRENCY_USD + "," + AQLString(100.0) + LF;
	}
	istringstream *pFXStream = new istringstream(fileCont.getCString());
	LACoreDataService::setIStringStream(LAMarketData::getNumFileName(fxFileName), pFXStream);
	ccys[0].toLower(), ccys[1].toLower();
	fileCont = "";

	// set term
	if (termStr.size())
	{
		termStr.remove(termStr.size() - 1, 1);
	}
	calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_TERM, termStr);
	// set curvetype
	if (curveType_d != STD)
	{
		irProp.setStaticData(ccys[0] + STATIC_DATA_KEY_YIELD_DF2, curveType_d);
	}
	if (curveType_f != STD)
	{
		irProp.setStaticData(ccys[1] + STATIC_DATA_KEY_YIELD_DF2, curveType_f);
	}	
	
	// set initialvalue
	calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_INITIAL_VOL, AQLString(initialValues[0]));
	calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_INITIAL_SKEW, AQLString(initialValues[1]));
	// set boundary
	calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_BOUNDARY_MAX_SKEW, AQLString(boundaryMaxSkew));
	calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_BOUNDARY_MIN_SKEW, AQLString(boundaryMinSkew));
	calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_BOUNDARY_MAX_VOL, AQLString(boundaryMaxVol));
	calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_BOUNDARY_MIN_VOL, AQLString(boundaryMinVol));
	// set other param
	calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_MAXITERATION, AQLString(static_cast<int>(maxIteration)));
	calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_MAX_STATIONARY_ITERATION, AQLString(static_cast<int>(maxStationaryStateIteration)));
	calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_ROOT_EPSILON, AQLString(rootEpsilon));
	calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_FUNCTION_EPSILON, AQLString(functionEpsilon));
	calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_GRADIENT_NORM_EPSILON, AQLString(gradientNormEpsilon));
	calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_METHOD_TYPE, type);

}

void
LAModelSetup::
setUpCalibHybridHWDDProcess(AQLDataInstance& dataInstance,
							const AQLDate& asofDate,
							const AQLString& fx,
							const double spotFX,
							const AQLString& curveType_d,
							const AQLString& curveType_f,
							const DoubleMatrix& hwParams_d,
							const DoubleMatrix& hwParams_f,
							const DoubleMatrix& ddParams,
							const DoubleVector& corVec,
							const AQLStringVector& terms,
							unsigned int numSteps,
							const double boundaryMaxSigma,
							const double boundaryMinSigma,
							const double boundaryMaxBeta,
							const double boundaryMinBeta,
							unsigned int maxIteration,
							unsigned int maxStationaryStateIteration,
							const double rootEpsilon,
							const double functionEpsilon,
							const double gradientNormEpsilon,
							const bool isCalib,
							const AQLString& type)
{
	asofDate;
	// data check
	if (corVec.size() != 3)
	{
		throw AQLCoreInvalidData("correlation vec size is must be 3", __FILE__, __LINE__);
	}
	if (hwParams_d.size() != 3)
	{
		throw AQLCoreInvalidData("Domestic HW parameter matrix size is must be 3", __FILE__, __LINE__);
	}
	if (hwParams_f.size() != 3)
	{
		throw AQLCoreInvalidData("Foreign HW parameter matrix size is must be 3", __FILE__, __LINE__);
	}
	if (ddParams.size() != 3)
	{
		throw AQLCoreInvalidData("DD parameter matrix size is must be 3", __FILE__, __LINE__);
	}

	// do preparation
	setUpCalibPreparation(dataInstance, fx);
	AQLString tmpFX = fx;
	tmpFX.toLower();

	AQLStringVector ccys;
	LAMarketData::convertToCurrency(fx, ccys);
	ccys[0].toLower(), ccys[1].toLower();

	//set volatility file
	LAStaticData &calibProp = LACoreDataService::getStaticDataManager().getCalibStaticData();
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_VOLATILITY_FILE, "data/in/" + ccys[0] + "_" + ccys[1] + "_fxvol.csv");
	AQLString volFileName = calibProp.getStaticData(tmpFX + FX_KEY_CALIB_VOLATILITY_FILE);

	/*AQLString fileCont = ",10DLow_Vol,25DLow_Vol,ATM_Vol,25DHigh_Vol,10DHigh_Vol\n";
	const AQLString volVal = ",0.000001,0.000001,0.000001,0.000001,0.000001\n";
	AQLString termStr = "";
	// create vol file contents	
	for (unsigned int i = 0; i < terms.size(); ++i)
	{
		fileCont += terms[i] + volVal;
		termStr += terms[i] + ":";
	}
	istringstream *pVolStream = new istringstream(fileCont.getCString());
	LACoreDataService::setIStringStream(LAMarketData::getNumFileName(volFileName), pVolStream);*/
	// spot fx
	LAStaticData &irProp = LACoreDataService::getStaticDataManager().getStaticData();
	setStaticDataValue(irProp, KEY_FXSPOTRATES_FILE, "data/in/spotrate.csv");
	AQLString fxFileName = irProp.getStaticData(KEY_FXSPOTRATES_FILE);
	AQLString fileCont = ccys[0].toUpper() + "," + ccys[1].toUpper()  + "," + AQLString(spotFX) + LF;
	if (ccys[0] != CURRENCY_USD && ccys[1] != CURRENCY_USD)
	{
		fileCont += ccys[0] + "," + CURRENCY_USD + "," + AQLString(100.0) + LF;
	}
	istringstream *pFXStream = new istringstream(fileCont.getCString());
	LACoreDataService::setIStringStream(LAMarketData::getNumFileName(fxFileName), pFXStream);
	ccys[0].toLower(), ccys[1].toLower();
	fileCont = "";
	// correlation file
	setStaticDataValue(irProp, KEY_SDE_CORRELATION_FILE, "data/in/sde_cor.csv");
	AQLString corFileName = irProp.getStaticData(KEY_SDE_CORRELATION_FILE);
	ccys[0].toUpper(), ccys[1].toUpper();
	fileCont += ccys[0] + ",1.0," + AQLString(corVec[0]) + "," + AQLString(corVec[1]) + LF;
	fileCont += ccys[1] + "," + AQLString(corVec[0]) + ",1.0," + AQLString(corVec[2]) + LF;
	fileCont += ccys[0] + "_" + ccys[1] + "," + AQLString(corVec[2]) + "," + AQLString(corVec[1]) + ",1.0" + LF;
	istringstream *pCorStream = new istringstream(fileCont.getCString());
	LACoreDataService::setIStringStream(LAMarketData::getNumFileName(corFileName), pCorStream);
	ccys[0].toLower(), ccys[1].toLower();
	fileCont = "";

	AQLObjectPool &objPool = dataInstance.getObjectPool();
	// set domestic hw params 
	AQLObject *pHWParams_d = 0;
	AQLString hwParamName_d = LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, LAMarketData::getBaseYieldName(ccys[0])));
	AQLObjectHolder objHolder = objPool.getObject(hwParamName_d, ENCHKTYPE_NOCHECK);
	if (!objHolder.isDefined())
	{
		pHWParams_d = new AQLObject();
		objPool.set(hwParamName_d, pHWParams_d);
	}
	else
	{
		pHWParams_d = &objHolder.get();
		pHWParams_d->reset();
	}
	// name
	pHWParams_d->add(CALIBRATION_DATA_NAME, new AQLDataString(hwParamName_d));
	unsigned int gridSize = hwParams_d[0].size();
	if (hwParams_d[1].size() != gridSize || hwParams_d[2].size() != gridSize)
	{
		throw AQLCoreInvalidData("Domestic HW parameter matrix format is wrong. Each vector must be same size.", __FILE__, __LINE__); 
	}
	pHWParams_d->add(PRICING_DATA_CALIBCANONICAL_T, new AQLDataDoubles(hwParams_d[0]));
	pHWParams_d->add(PRICING_DATA_CALIBMEANREV_T, new AQLDataDoubles(hwParams_d[1]));
	pHWParams_d->add(PRICING_DATA_CALIBVOL_T, new AQLDataDoubles(hwParams_d[2]));

	// set foreign hw params 
	AQLObject *pHWParams_f = 0;
	AQLString hwParamName_f = LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, LAMarketData::getBaseYieldName(ccys[1])));
	objHolder = objPool.getObject(hwParamName_f, ENCHKTYPE_NOCHECK);
	if (!objHolder.isDefined())
	{
		pHWParams_f = new AQLObject();
		objPool.set(hwParamName_f, pHWParams_f);
	}
	else
	{
		pHWParams_f = &objHolder.get();
		pHWParams_f->reset();
	}
	// name
	pHWParams_f->add(CALIBRATION_DATA_NAME, new AQLDataString(hwParamName_f));
	gridSize = hwParams_f[0].size();
	if (hwParams_f[1].size() != gridSize || hwParams_f[2].size() != gridSize)
	{
		throw AQLCoreInvalidData("Foreign HW parameter matrix format is wrong. Each vector must be same size.", __FILE__, __LINE__); 
	}
	pHWParams_f->add(PRICING_DATA_CALIBCANONICAL_T, new AQLDataDoubles(hwParams_f[0]));
	pHWParams_f->add(PRICING_DATA_CALIBMEANREV_T, new AQLDataDoubles(hwParams_f[1]));
	pHWParams_f->add(PRICING_DATA_CALIBVOL_T, new AQLDataDoubles(hwParams_f[2]));


	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_INITIALPARAM_FILE, "data/in/" + ccys[0] + "_" + ccys[1] + "_fxdd.csv");
	AQLString initalFileName = calibProp.getStaticData(tmpFX + FX_KEY_CALIB_PTBERG_INITIALPARAM_FILE);
	// create initial file contents
	gridSize = ddParams[0].size();
	if (ddParams[1].size() != gridSize || ddParams[2].size() != gridSize)
	{
		throw AQLCoreInvalidData("DD parameter matrix format is wrong. Each vector must be same size.", __FILE__, __LINE__); 
	}
	for (unsigned int i = 0; i < gridSize; ++i)
	{
		fileCont += AQLString(ddParams[0][i]) + "," + AQLString(ddParams[1][i]) + "," + AQLString(ddParams[2][i]) + LF;
		//fileCont += AQLString(ddParams[1][i]) + "," + AQLString(ddParams[2][i]) + LF;
	}
	istringstream *pInitialStream = new istringstream(fileCont.getCString());
	LACoreDataService::setIStringStream(LAMarketData::getNumFileName(initalFileName), pInitialStream);
	calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_INITIALPARAM_ISFILEREAD, "TRUE");

	AQLString termStr = "";
	for (unsigned int i = 0; i < terms.size(); ++i)
	{
		termStr += terms[i] + ":";
	}
	// set term
	if (termStr.size())
	{
		termStr.remove(termStr.size() - 1, 1);
	}
	calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_TERM, termStr);
	// set curvetype
	if (curveType_d != STD)
	{
		irProp.setStaticData(ccys[0] + STATIC_DATA_KEY_YIELD_DF2, curveType_d);
	}
	if (curveType_f != STD)
	{
		irProp.setStaticData(ccys[1] + STATIC_DATA_KEY_YIELD_DF2, curveType_f);
	}	
	// set boundary
	calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_BOUNDARY_MAX_SIGMA, AQLString(boundaryMaxSigma));
	calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_BOUNDARY_MIN_SIGMA, AQLString(boundaryMinSigma));
	calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_BOUNDARY_MAX_BETA, AQLString(boundaryMaxBeta));
	calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_BOUNDARY_MIN_BETA, AQLString(boundaryMinBeta));
	// set other param
	calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_MAX_ITERATION, AQLString(static_cast<int>(maxIteration)));
	calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_MAX_STATIONARY_ITERATION, AQLString(static_cast<int>(maxStationaryStateIteration)));
	calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_NUM_SMALL_STEP, AQLString(static_cast<int>(numSteps)));
	calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_ROOT_EPSILON, AQLString(rootEpsilon));
	calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_FUNCTION_EPSILON, AQLString(functionEpsilon));
	calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_GRADIENT_NORM_EPSILON, AQLString(gradientNormEpsilon));
	if (isCalib)
	{
		calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_TARGET, "TRUE");
	}
	else
	{
		calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_TARGET, "FALSE");
	}
	
	calibProp.setStaticData(tmpFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_METHOD_TYPE, type);

}

/*!
    @brief setup preparation for calibration
	set default value to properties and create file contents to avoid error

	@param[out] dataInstance
	@param[in] fx

*/
void
LAModelSetup::
setUpCalibPreparation(AQLDataInstance &dataInstance, const AQLString& fx)
{
	AQLString tmpFX = fx;
	tmpFX.toLower();
	// set asof
	AQLString asofStr = LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE);
	if (asofStr == AQ_NO_DATA)
	{
		LACoreDataService::setContext(CONTEXT_KEY_ASOFDATE, "19000101");
	}
	AQLStringVector ccys;
	LAMarketData::convertToCurrency(fx, ccys);
	// set currency
	LACoreDataService::setContext(ARG_KEY_CURRENCY, ccys[0].toUpper() + ":" + ccys[1].toUpper() + ":" + tmpFX.toUpper());
	tmpFX.toLower(), ccys[0].toLower(), ccys[1].toLower();

	LAStaticData &calibProp = LACoreDataService::getStaticDataManager().getCalibStaticData();
	setStaticDataValue(calibProp, KEY_CALIB_INFO_ISRESET, "TRUE");
	////////////////////////////
	// setup ptberg default value
	////////////////////////////
	// beta cap and beta floor
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_BETACAP, "1.0");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_BETAFLOOR, "-3.0");
	// initial file
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_INITIALPARAM_ISFILEREAD, "TRUE");
	ccys[0].toLower(), ccys[1].toLower();
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_INITIALPARAM_FILE, "data/in/" + ccys[0] + "_" + ccys[1] + "_fxdd.csv");
	//AQLString initalFileName = calibProp.getStaticData(tmpFX + FX_KEY_CALIB_PTBERG_INITIALPARAM_FILE);
	AQLString fileCont = "";
	//istringstream *pInitailStream = LACoreDataService::getIStringStream(LAMarketData::getNumFileName(initalFileName));
	//if (!pInitailStream)
	//{
	//	const unsigned int size_max = 80;
	//	const double step = 0.5;
	//	const double initialvol = 0.0000000001;
	//	const double initialbeta = 0.1;
	//	for (unsigned int i = 0; i < size_max; ++i)
	//	{
	//		fileCont += AQLString(step * i) + "," + AQLString(initialvol) + "," + AQLString(initialbeta) + LF;
	//	}
	//	pInitailStream = new istringstream(fileCont.getCString());
	//	LACoreDataService::setIStringStream(LAMarketData::getNumFileName(initalFileName), pInitailStream);
	//	fileCont = "";
	//}
	// correlation file
	LAStaticData &irProp = LACoreDataService::getStaticDataManager().getStaticData();
	setStaticDataValue(irProp, KEY_SDE_CORRELATION_FILE, "data/in/sde_cor.csv");
	AQLString corFileName = irProp.getStaticData(KEY_SDE_CORRELATION_FILE);
	ccys[0].toUpper(), ccys[1].toUpper();
	fileCont += ccys[0] + "," + "1.0,0.374,0.2" + LF;
	fileCont += ccys[1] + "," + "0.374,1.0,0.34" + LF;
	fileCont += ccys[0] + "_" + ccys[1] + "," + "0.2,0.34,1.0" + LF;
	istringstream *pCorStream = new istringstream(fileCont.getCString());
	LACoreDataService::setIStringStream(LAMarketData::getNumFileName(corFileName), pCorStream);
	ccys[0].toLower(), ccys[1].toLower();
	fileCont = "";

	// spot fx
	setStaticDataValue(irProp, KEY_FXSPOTRATES_UNITCCY, ccys[0], true);
	setStaticDataValue(irProp, AQLString("usd") + STATIC_DATA_KEY_FXSPOTRATES_CALENDAR, CITY_NYB);
	for (unsigned int i = 0; i < ccys.size(); ++i)
	{
		setStaticDataValue(irProp, ccys[i] + STATIC_DATA_KEY_FXSPOTRATES_CALENDAR, CITY_TkB);
		setStaticDataValue(irProp, ccys[i] + STATIC_DATA_KEY_SPOTLAG, "2");
	}

	// set strike fwd term
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_STRIKEFWDTERM, "12Y:15Y:20Y:25Y:30Y");
	// set calib variable
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_CALIBVARIABLE, "0.001");
	// set calib param
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_CALIBPARAM, "Both");
	// set calib input type
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_INPUTPARAMTYPE, "ALL", true);
	// terms calendar
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_TERM_CALENDAR, AQLString(CITY_TkB) + ":" + AQLString(CITY_NYB));
	// terms slidingrule
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_TERM_SLIDINGRULE, SLIDING_MOD_FOLLOWING);
	// terms spotlag
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_TERM_SPOTLAG, "2");
	// terms daycount
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_TERM_DAYCOUNT, DAYCOUNT_ACT_365_ISDA);
	// terms isincludelast
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_TERM_ISINCLUDELAST, "FALSE");
	// volatility file
	// terms
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_TERM, "3M:6M:1Y:2Y:3Y:4Y:5Y:7Y:10Y:12Y:15Y:20Y:25Y:30Y");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_VOLATILITY_FILE, "data/in/" + ccys[0] + "_" + ccys[1] + "_fxvol.csv");
	const AQLString volFileName = calibProp.getStaticData(tmpFX + FX_KEY_CALIB_VOLATILITY_FILE);
	istringstream *pVolStream = LACoreDataService::getIStringStream(LAMarketData::getNumFileName(volFileName));
	if (!pVolStream)
	{
		AQLStringVector terms = calibProp.getStaticData(tmpFX + FX_KEY_CALIB_PTBERG_TERM).toUpper().toToken(MULTI_STATIC_DATA_DELIMITER);
		fileCont = ",10DLow_Vol,25DLow_Vol,ATM_Vol,25DHigh_Vol,10DHigh_Vol\n";
		const AQLString volVal = ",0.000001,0.000001,0.000001,0.000001,0.000001\n";
		for (unsigned int i = 0; i < terms.size(); ++i)
		{
			fileCont += terms[i] + volVal;
		}
		istringstream *pVolStream = new istringstream(fileCont.getCString());
		LACoreDataService::setIStringStream(LAMarketData::getNumFileName(volFileName), pVolStream);
		fileCont = "";
	}

	AQLObjectPool &objPool = dataInstance.getObjectPool();
	// set domestic hw params 
	AQLObject *pHWParams_d = 0;
	AQLString hwParamName_d = LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, LAMarketData::getBaseYieldName(ccys[0])));
	AQLObjectHolder objHolder = objPool.getObject(hwParamName_d, ENCHKTYPE_NOCHECK);
	if (!objHolder.isDefined())
	{
		pHWParams_d = new AQLObject();
		objPool.set(hwParamName_d, pHWParams_d);
		pHWParams_d->add(CALIBRATION_DATA_NAME, new AQLDataString(hwParamName_d));
	}
	// set foreign hw params 
	AQLObject *pHWParams_f = 0;
	AQLString hwParamName_f = LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, LAMarketData::getBaseYieldName(ccys[1])));
	objHolder = objPool.getObject(hwParamName_f, ENCHKTYPE_NOCHECK);
	if (!objHolder.isDefined())
	{
		pHWParams_f = new AQLObject();
		objPool.set(hwParamName_f, pHWParams_f);
		pHWParams_f->add(CALIBRATION_DATA_NAME, new AQLDataString(hwParamName_f));
	}

	// term convention
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_CALENDAR, AQLString(CITY_TkB) + ":" + AQLString(CITY_LnB));
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_SLIDINGRULE, SLIDING_MOD_FOLLOWING);
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_SPOTORFWD, "Fwd");
	// optiontype
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_OPTIONTYPE_ATM, "CALL");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_OPTIONTYPE_25DH, "CALL");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_OPTIONTYPE_10DH, "CALL");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_OPTIONTYPE_25DL, "PUT");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_OPTIONTYPE_10DL, "PUT");
	// weight
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_WEIGHT_ATM, "1.0");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_WEIGHT_25DH, "0.5");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_WEIGHT_10DH, "0.5");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_WEIGHT_25DL, "0.9");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_WEIGHT_10DL, "0.7");
	// beta
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_TERMBETA, "0.1");
	// is fit atm
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_ISFITATM, "TRUE");
	// is delta neutral
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_ISDELTANEUTRAL, "TRUE");

	////////////////////////////
	// setup 3F default value
	////////////////////////////
	///////////////////////////
	// set skew & vol info
	///////////////////////////
	// boundary vol skew
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_INITIAL_VOL, "0.1");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_INITIAL_SKEW, "0.1");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_BOUNDARY_MAX_VOL, "1.0");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_BOUNDARY_MIN_VOL, "0.00001");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_BOUNDARY_MAX_SKEW, "1.0");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_BOUNDARY_MIN_SKEW, "-3.0");
	// other calib param
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_MAXITERATION, "10000");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_MAX_STATIONARY_ITERATION, "5000");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_ROOT_EPSILON, "0.000000001");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_FUNCTION_EPSILON, "0.000000001");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_GRADIENT_NORM_EPSILON, "0.000000001");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_METHOD_TYPE, "Simplex_method");
	// weight
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_ATM_WEIGHT, "100.0");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_25DH_WEIGHT, "2.0");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_10DH_WEIGHT, "1.0");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_25DL_WEIGHT, "2.0");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_10DL_WEIGHT, "1.0");
	///////////////////////////
	// set sigma & beta info
	///////////////////////////
	// initial val
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_INITIAL_SIGMA, "0.1");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_INITIAL_BETA, "0.2");
	// boundary sigma beta
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_BOUNDARY_MAX_SIGMA, "1.0");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_BOUNDARY_MIN_SIGMA, "0.0000001");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_BOUNDARY_MAX_BETA, "1.0");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_BOUNDARY_MIN_BETA, "-3.0");
	// other calib param
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_MAX_ITERATION, "10000");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_MAX_STATIONARY_ITERATION, "5000");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_NUM_SMALL_STEP, "2");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_ROOT_EPSILON, "0.000000001");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_FUNCTION_EPSILON, "0.000000001");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_GRADIENT_NORM_EPSILON, "0.000000001");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_TARGET, "TRUE");
	setStaticDataValue(calibProp, tmpFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_METHOD_TYPE, "Simplex_method");

}