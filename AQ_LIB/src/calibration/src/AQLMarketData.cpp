#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLMarketData.h"
#include "AQLDataReference.h"
#include "AQLStaticData.h"
#include "AQLLinearInterpolation.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLAlgorithm.h"
#include "AQLDefinitionsCalibration.h"

#include "AQLDealUtils.h"
#include "AQLCoreThread.h"
#include "AQLRiskConfiguration.h"
#include "AQLDefinitionsLMM.h"
#include "AQLDefinitionsHW.h"
#include "AQLDefinitionsPtberg.h"
#include "AQLScenarioConfiguration.h"
#include "AQLMathPlainVanillaEntity.h"
#include "AQLLinearRatesOptionValue.h"
#include "AQLPricePortfolioValue.h"
#include "AQLMathCurveFuncUtility.h"
#include "AQLMathDateUtilities.h"

#include "AQLMathSwaptionVolUtility.h"
#include "AQLCoreDataService.h"

#ifndef STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_VOL_FILE
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_VOL_FILE ".calib.lmm.volatility.swaption.vol.file"
#endif

#ifndef STATIC_DATA_KEY_CALIB_IRSABR_UNDERLYING
#define STATIC_DATA_KEY_CALIB_IRSABR_UNDERLYING					".calib.irsabr.underlying"
#endif

#ifdef __HAS_MIC__

#endif

using namespace std;

AQLString AQLMarketData::mCalFileName;
#ifdef __HAS_MIC__
common_lib::StaticMutex AQLMarketData::mMutex;
#endif
// constructor
/*!

*/
AQLMarketData::AQLMarketData(void)
{
}

// destructor
/*!

*/
AQLMarketData::~AQLMarketData(void)
{
}

// 
/*!
    @brief setup calender data
	
	setup holyday data to dataInstance object
	holyday data is load from file(csv file)
	
	@param[in] fileName
*/
void 
AQLMarketData::registCalendar(const AQLString &fileName)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	// already calendar file regist
	if (mCalFileName == fileName)
	{
		return;
	}
	AQLFileAccessor file(fileName);
	AQLStringMatrix data;
	file.readAllData(MARKET_DATA_DELIMITER, data);
	//AQLStringVector city = data[0];
	//const int cNum = city.size();

	//AQLStringVector city = data[0];
	//const int cNum = city.size();
	//for (int i = 0; i < cNum; i++)
	//	if (city[i].size() != 3) throw AQLCoreInvalidData("Calendar file format is wrong !! ", __FILE__, __LINE__);
	//typedef AQLStringMatrix::const_iterator CItr;
	//typedef vector<DateVector> DateMatrix;

	//DateMatrix holMatrix(cNum);
	//CItr it = ++data.begin();

	//while (it != data.end())
	//{
	//	AQLStringVector dates = *it;
	//	//check
	//	if (static_cast<int>(dates.size()) != cNum)
	//	{
	//		throw AQLCoreInvalidData("Calendar file format is wrong !! ", __FILE__, __LINE__);
	//	}

	//	for (int i = 0; i < cNum; ++i)
	//	{
	//		AQLString strDate = dates[i];
	//		if (strDate.size() > 0)
	//		{
	//			holMatrix[i].push_back(AQLDate(strDate.getCString()));				
	//		}
	//	}
	//	++it;
	//}

	//AQLMathCalendarSet calMaster;
	//for (int i = 0; i < cNum; ++i)
	//{
	//	AQLMathCalendar cal;
	//	cal.setWeekly(SAT);
	//	cal.setWeekly(SUN);
	//	cal.setDate(holMatrix[i]);
	//	calMaster.setCalendarData(city[i], cal);
	//}
	AQLString asofdateStr = AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE);
	AQLStringVector terms = AQLCoreDataService::getContext(CONTEXT_KEY_CALENDAR_TERM).toUpper().toToken(MULTI_STATIC_DATA_DELIMITER);
	DateVector boundDates(2);
	BoolVector isBounds(2, false);
	if (asofdateStr != AQ_NO_DATA && terms.size() == 2) 
	{
		AQLDate asofdate = AQLDate(asofdateStr.getCString());
		if (terms[0].size() != 0) 
		{
			boundDates[0] = AQLMathDateCalculations::getDate(asofdate, terms[0], false);
			isBounds[0] = true;
		}
		if (terms[1].size() != 0) 
		{
			boundDates[1] = AQLMathDateCalculations::getDate(asofdate, terms[1], true);
			isBounds[1] = true;
		}
	}
	AQLString tmpcities = AQLCoreDataService::getContext(CONTEXT_KEY_CALENDAR_CITY);
	const AQLStringVector cities = tmpcities.toUpper().toToken(MULTI_STATIC_DATA_DELIMITER);

	AQLMathCalendarSet calMaster;
	for (int i = 0; i < data.size(); i++)
	{
		const AQLString city = data[i][0].toUpper();
		if (city.size() != 3) throw AQLCoreInvalidData("Calendar file format is wrong !! ", __FILE__, __LINE__);
		if (cities[0] != AQ_NO_DATA && find(cities.begin(), cities.end(), city) == cities.end()) continue;

		AQLMathCalendar cal;
		cal.setWeekly(SAT);
		cal.setWeekly(SUN);
		DateVector dvec;
		const int size = data[i].size();
		if (size > 1)
		{
			for (int j = 1; j < size; j++)
			{
				const AQLString strDate = data[i][j];
				if (strDate.size() == 0) break;
				const AQLDate date = AQLDate(strDate.getCString());
				if ((!isBounds[0] || boundDates[0] <= date) && (!isBounds[1] || date <= boundDates[1]))
				{
					dvec.push_back(date);
				}
			}
		}
		cal.setDate(dvec);
		calMaster.setCalendarData(city, cal);
	}
	
	mCalFileName = fileName;
}


/*!
    @brief reset market data use Libor

	calc libor rate from df
	and set libor rate to market data 

	@param[in] yield curve
	@param[in] ccy
*/
void 
AQLMarketData::resetMarketDataUseL(AQLMathYieldCurvePro &curve, const AQLString &ccy, const AQLString *pCurveType)
{
	AQLString tmpCurrency = ccy;
	tmpCurrency.toLower();
	AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();

	AQLString suffix = "";
	AQLString data_suffix = "";
	if (pCurveType)
	{
		if (*pCurveType != STD)
		{
			suffix = "." + *pCurveType;
			suffix.toLower();
			data_suffix = "_" + *pCurveType;
		}
	}
	//AQLIndexData index = getIndexInfoLibor(ccy);
	//AQLDataMultiReference &refMarkets = curve.getMarketData();
	AQLDataMultiReference &refMarkets = dynamic_cast<AQLDataMultiReference &>(curve.getData(CALIBRATION_DATA_MARKETDATA + data_suffix, ISNOTNULL).get()); 
	const int mSize = refMarkets.getSize();
	// set data for holiday adjustment
	// daycount
	AQLPriceDataDayCount dc;
	AQLString dcStr = staticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_DAYCOUNT + suffix).toUpper();
	dc.convertFromString(dcStr);
	// sliding rule
	AQLPriceDataSlidingRule sliding;
	AQLString slidingStr = staticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_SLIDINGRULE + suffix).toUpper();
	sliding.convertFromString(slidingStr);
	// calendar
	AQLPriceDataCalendar cal;
	AQLString calStr = staticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_CALENDAR  + suffix);
	cal.convertFromString(calStr);
	// asOfDate
	const AQLDate asOfDate = curve.getAsOfDate();
	// spotDate
	AQLDate spotDate;
	// check spotDate use ?
	AQLDataBool tmpAttrB;
	tmpAttrB.convertFromString(staticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISSPOTUSE));
	if (tmpAttrB.get())
	{
		spotDate.setDate(staticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_SPOTDATE + suffix).getCString());
	}
	else
	{
		spotDate = cal.getBusinessDay(asOfDate, staticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_RESETLAG + suffix).getIntValue());
	}

	if (asOfDate > spotDate)
	{
		throw AQLCoreInvalidData("AsofDate > spotDate, cannnot calc. ", __FILE__, __LINE__);
	}

	double termSpot = dc.getTerm(asOfDate, spotDate);

	AQLPriceDataInterpolation inter = dynamic_cast<AQLPriceDataInterpolation &>(curve.getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
	AQLObject &yieldData = curve.getYieldData().get().get();
	const DoubleArray &terms = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(CALIBRATION_DATA_TERMS + data_suffix, ISNOTNULL)).get()).get();
	const DoubleArray &dfs   = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(IR_CALIBRATION_DATA_DFS + data_suffix, ISNOTNULL)).get()).get();
	inter.set(terms, dfs);

	double dfSpot   = inter.value(termSpot);
	//double dfSpot   = curve.getDF(termSpot);

	AQLStringVector liborYTerm;
	// calc rate
	for (int i = 0; i < mSize; ++i)
	{
		AQLObjectHolder &mktData = refMarkets.get(i);
		AQLString dataType = dynamic_cast<const AQLDataString &>
								(mktData.getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL).get()).get();

		dataType.toUpper();

		if (dataType != YIELD_TYPE_O_N && dataType != YIELD_TYPE_T_N 
			&& dataType != YIELD_TYPE_ZERO)
		{
			AQLString termStr = dynamic_cast<const AQLDataString &>
								(mktData.getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL).get()).get();

			if (dataType == YIELD_TYPE_FRA3M || dataType == YIELD_TYPE_FRA6M) 
			{
				termStr = AQLPriceYieldGenerator::changeFRATermFormat(termStr);
				mktData.remove(IR_CALIBRATION_DATA_TERM);
				mktData.add(IR_CALIBRATION_DATA_TERM, new AQLDataString()).convertFromString(termStr);
			}

			AQLString searchTerm = termStr;

			if (termStr == "12M")
			{
				searchTerm = "1Y";
			}
			if (find(liborYTerm.begin(), liborYTerm.end(), searchTerm) == liborYTerm.end())
			{
				// calc date from spotDate
				AQLDate date = AQLMathDateCalculations::getDate(spotDate, termStr, sliding, &cal, true);
					
				double term     = dc.getTerm(asOfDate, date);
				double df       = inter.value(term);
				//double df       = curve.getDF(term);
				double delta    = term - termSpot;

				// calc libor rate
				double rate = (dfSpot - df) / (df * delta);

				// reset attr libor value
				mktData.remove(CALIBRATION_DATA_RATE);
				mktData.add(CALIBRATION_DATA_RATE, new AQLDataDouble(rate));
				// set spot date
				mktData.remove(IR_CALIBRATION_DATA_SPOTDATE);
				mktData.add(IR_CALIBRATION_DATA_SPOTDATE, new AQLDataDate(spotDate));
				// set calendar
				mktData.remove(CALIBRATION_DATA_CALENDAR);
				mktData.add(CALIBRATION_DATA_CALENDAR , new AQLPriceDataCalendar()).convertFromString(calStr);
				// set daycount
				mktData.remove(IR_CALIBRATION_DATA_DAYCOUNT);
				mktData.add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(dcStr);
				// set data type
				mktData.remove(IR_CALIBRATION_DATA_DATATYPE);
				mktData.add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_ZERO);
				// set frequency
				mktData.remove(IR_CALIBRATION_DATA_FREQUENCY);
				mktData.add(IR_CALIBRATION_DATA_FREQUENCY, new AQLDataString()).convertFromString(FREQ_SIMPLE);
				// set slidingrule
				mktData.remove(CALIBRATION_DATA_SLIDINGRULE);
				mktData.add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(slidingStr);
			}
		}
		else if (dataType == YIELD_TYPE_ZERO)
		{
			const AQLString &termStr = dynamic_cast<const AQLDataString &>
								(mktData.getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL).get()).get();

			if (termStr == "12M")
			{
				liborYTerm.push_back("1Y");
			}
			else if (termStr.findString("Y") >= 0)
			{
				liborYTerm.push_back(termStr);
			}
		}
	}
}

#ifndef VISUAL_STUDIO_2010_ANALYTICS 
/*!
    @brief return correlation among sde

	@param[in] ccys
	@param[out] corData
*/
void
AQLMarketData::getSDECorrelation(const AQLStringVector &ccys, DoubleMatrix &corData)
{
	corData.clear();
	const unsigned int ccySize = ccys.size();
	IntArray indexs(ccySize, -1);

	AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	AQLString filePath = staticData.getStaticData(KEY_SDE_CORRELATION_FILE);

	AQLFileAccessor file(getNumFileName(filePath));
	AQLStringMatrix corDataStr;
	file.readAllData(MARKET_DATA_DELIMITER, corDataStr);
	file.close();

	const unsigned int corSize = corDataStr.size();
	for (unsigned int i = 0; i < ccySize; ++i)
	{
		if (ccys[i].findString(FX_DELIMITER) < 0)
		{
			// for currency
			AQLString tmpCurrency = ccys[i];
			tmpCurrency.toUpper();
			for (unsigned int j = 0; j < corSize; ++j)
			{
				if (corDataStr[j].size() < 2)
				{
					throw AQLCoreInvalidData("SDE correlation file, format is wrong.", __FILE__, __LINE__);
				}
				if (tmpCurrency == corDataStr[j][0].toUpper())
				{
					indexs[i] = j;
					break;
				}
			}
		}
		else if(ccys[i].findString("_VOL") < 0)
		{
			// for fx
			AQLStringVector ccyVec;
			convertToCurrency(ccys[i], ccyVec);
			ccyVec[0].toUpper();
			ccyVec[1].toUpper();
			for (unsigned int j = 0; j < corSize; ++j)
			{
				if (corDataStr[j].size() < 2)
				{
					throw AQLCoreInvalidData("SDE correlation file, format is wrong.", __FILE__, __LINE__);
				}
				corDataStr[j][0].toUpper();
				if (corDataStr[j][0].findString(ccyVec[0]) >= 0 && 
					corDataStr[j][0].findString(ccyVec[1]) >= 0 &&
					corDataStr[j][0].findString("_VOL") < 0)
				{
					indexs[i] = j;
					break;
				}
			}
		}
		else if (ccys[i].findString("_VOL") >= 0)
		{
			// for vol
			AQLStringVector ccyVec;
			convertToCurrency(ccys[i], ccyVec);
			ccyVec[0].toUpper();
			ccyVec[1].toUpper();
			for (unsigned int j = 0; j < corSize; ++j)
			{
				if (corDataStr[j].size() < 2)
				{
					throw AQLCoreInvalidData("SDE correlation file, format is wrong.", __FILE__, __LINE__);
				}
				corDataStr[j][0].toUpper();
				if (corDataStr[j][0].findString(ccyVec[0]) >= 0 &&
					corDataStr[j][0].findString(ccyVec[1]) >= 0 &&
					corDataStr[j][0].findString("_VOL") >= 0)
				{
					indexs[i] = j;
					break;
				}
			}
		}
		if (indexs[i] < 0)
		{
			AQLString msg = "This currency does not exist in sde correlation file. currency = " + ccys[i];
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
	}

	// create correlation data
	DoubleArray line(ccySize);
	corData.resize(ccySize, line);

	for(unsigned int i = 0; i < ccySize; ++i)
	{
		for(unsigned int j = i; j < ccySize; ++j)
		{
			if (i != j)
			{
				int index_i =  indexs[i];
				int index_j =  indexs[j];
				corData[i][j] = corDataStr[index_i][index_j + 1].trimLeft().trimRight().getDoubleValue();
			}
			else
			{
				corData[i][j] = 1.0;
			}
			corData[j][i] = corData[i][j];
		}
	}
}
#endif

/*!
    @brief number attached fileName

	@param fileName
	@return AQLString
*/
AQLString
AQLMarketData::getNumFileName(const AQLString &fileName, FileID fileid)
{
	AQLString num = AQ_NO_DATA;
	if (fileid == MARKETID)
	{
		num = AQLCoreDataService::getContext(ARG_KEY_MARKETID);
	}
	else if (fileid == AQLID)
	{
		num = AQLCoreDataService::getContext(ARG_KEY_AQLID);
	}
	else if (fileid == PROPERTIESID)
	{
		num = AQLCoreDataService::getContext(ARG_KEY_PROPERTIESID);
	}

	if (num == AQ_NO_DATA)
	{
		 num = AQLCoreDataService::getContext(ARG_KEY_FILENUM);
	}

	if (num != AQ_NO_DATA)
	{
		std::string sFileName(fileName.getCString());
		unsigned int ex_pos = sFileName.find_last_of(".");
		if (ex_pos != std::string::npos)
		{
			AQLString extension = fileName.subString(ex_pos, fileName.size() - 1);
			AQLString fileName_no_ex = fileName.subString(0, ex_pos - 1);
			return fileName_no_ex + num + extension;
		}
		else
		{
			// no extension case
			return fileName + num;
		}
	}
	else
	{
		return fileName;
	}
}


/*!
    @brief get volatiltiy object name

	@param[in] type

*/
AQLString 
AQLMarketData::getBaseVolatilityName(const AQLString &key)
{

	AQLString tmpKey = key;
	AQLString name = AQLCoreDataService::getContext(tmpKey.toLower() + CONTEXT_KEY_VOLENTITY_NAME);
	if (name != AQ_NO_DATA)
	{
		return name;
	}

	AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	if ( key.findString(FX_DELIMITER) < 0)
	{
		AQLString sdeName = staticData.getStaticData(tmpKey.toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
		return PREFIX_VOL + sdeName;
	}
	else
	{
		AQLStringVector ccys = tmpKey.toToken(FX_DELIMITER);
		if (ccys.size() != 2)
		{
			throw AQLCoreInvalidData("FX format maust be ccy1/ccy2 !", __FILE__, __LINE__);
		}
		AQLString fxKey = getFXKey(ccys[0], ccys[1]);
		 AQLString sdeName = staticData.getStaticData(fxKey.toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
		return PREFIX_VOL + sdeName;
	}
}

/*!
    @brief get correlation object name

	@param[in] ccy
	@return AQLString
*/
AQLString 
AQLMarketData::getBaseCorrelationName(const AQLString &ccy)
{

	AQLString tmpCurrency = ccy;
	AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	AQLString sdeName = staticData.getStaticData(tmpCurrency.toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
	return PREFIX_COR + sdeName;

}

/*!
    @brief get yeild object name

	@param[in] ccy
	@return AQLString
*/
AQLString 
AQLMarketData::getBaseYieldName(const AQLString &ccy)
{
	AQLString ret;

	AQLString tmpCurrency = ccy;
	AQLString name = AQLCoreDataService::getContext(tmpCurrency.toLower() + CONTEXT_KEY_YIELDENTITY_NAME);
	if (name != AQ_NO_DATA)
	{
		return name;
	}

	AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	AQLString isSetCurveID = AQLCoreDataService::getContext(CONTEXT_KEY_ISSETCURVEID);

	if (isSetCurveID == "TRUE")
	{
		AQLString curveID = staticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID);
		ret = PREFIX_YIELD + curveID;
	}
	else
	{
		AQLString sdeName = staticData.getStaticData(tmpCurrency.toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
		ret = PREFIX_YIELD + sdeName;
	}

	return ret;
}

/*!
    @brief get yeildcurvepro object name

	@param[in] ccy
	@return AQLString
*/
AQLString 
AQLMarketData::getBaseYieldProName(const AQLString &ccy)
{
	AQLString tmpCurrency = ccy;
	AQLString name = AQLCoreDataService::getContext(tmpCurrency.toLower() + CONTEXT_KEY_YIELDPROENTITY_NAME);
	if (name != AQ_NO_DATA)
	{
		return name;
	}
	return "PRO_" + getBaseYieldName(ccy);
}
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
// 
/*!
    @brief get asofdate
	note: if path object is not set throw AQLCoreInvalidData

	@param[in] objPool
	@return AQLDate
*/
AQLDate 
AQLMarketData::getAsofDate(const AQLObjectPool &objPool)
{
	const AQLString asofStr = AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE);
	if (asofStr != AQ_NO_DATA)
	{
		return AQLDate(asofStr.getCString());
	}
	else
	{
		EntityConstIter it = objPool.begin();
		while (it != objPool.end())
		{
			if (it->second.isTypeOf(ENTITY_PLAINVANILLA))
			{
				return dynamic_cast<const AQLMathPlainVanillaEntity&>
								(it->second.get()).getAsOfDate().get();
			}
			++it;
		}
		it = objPool.begin();
		

		while (it != objPool.end())
		{
			if (it->second.isTypeOf(ENTITY_PATH))
			{
				return dynamic_cast<const AQLMathPathEntity&>
								(it->second.get()).getAsOfDate().get();
			}
			++it;
		}


		throw AQLCoreInvalidData("Path object is not set in object pool", __FILE__, __LINE__);
	}
}


// 
/*!
    @brief get time grid day count
	note: if path object is not set throw AQLCoreInvalidData

	@param[in] objPool
	@return AQLString
*/
AQLString 
AQLMarketData::getTimeGridDayCount(const AQLObjectPool &objPool)
{
	EntityConstIter it = objPool.begin();
	while (it != objPool.end())
	{
		if (it->second.isTypeOf(ENTITY_PATH))
		{
			return dynamic_cast<const AQLMathPathEntity&>
							(it->second.get()).getDayCount().convertToString();
		}
		else if (it->second.isTypeOf(ENTITY_PLAINVANILLA))
		{
			return dynamic_cast<const AQLMathPlainVanillaEntity&>
							(it->second.get()).getDayCount().convertToString();
		}
		++it;
	}

	throw AQLCoreInvalidData("Path object is not set in object pool", __FILE__, __LINE__);
}

// 
/*!
    @brief get AQLMathPathEntity from object pool

	@param[in] objPool
	@return AQLMathPathEntity &
*/
AQLMathPathEntity * 
AQLMarketData::getPathEnitty(AQLObjectPool &objPool)
{
	AQLMathPathEntity *pPath = 0;
	EntityIter it = objPool.begin();
	while (it != objPool.end())
	{
		if (it->second.isTypeOf(ENTITY_PATH))
		{
			AQLMathPathEntity &tmpPath = dynamic_cast<AQLMathPathEntity &>(it->second.get());
			const AQLDataHolder &attrIsRisk = tmpPath.getData(AP_CALIBRATION_DATA_ISRISKENTITY, NOCHECK);
			// get original object (not for risk)
			if (!attrIsRisk.isDefined() || attrIsRisk.isNull())
			{
				pPath = &tmpPath;
				break;
			}
			else
			{
				bool isRisk = dynamic_cast<const AQLDataBool &>(attrIsRisk.get()).get();
				if (!isRisk)
				{
					pPath = &tmpPath;
					break;
				}
			}
		}
		++it;
	}

	return pPath;
}

// 
/*!
    @brief get AQLMathFXEntity from object pool

	get original object (not for risk)

	@param[in] objPool
	@param[in] type
	@return AQLMathFXEntity &
*/
AQLMathFXEntity * 
AQLMarketData::getFXEntity(AQLObjectPool &objPool, const AQLString &type)
{
	AQLString ltype = type;
	AQLMathFXEntity *pFX = 0;
	EntityIter it = objPool.begin();
	while (it != objPool.end())
	{
		if (it->second.isTypeOf(ENTITY_FX))
		{
			AQLMathFXEntity &tmpFx = dynamic_cast<AQLMathFXEntity &>(it->second.get());
			AQLString tmpType = tmpFx.getFXType().get();
			if (ltype.toUpper() == tmpType.toUpper())
			{
				const AQLDataHolder &attrIsRisk = tmpFx.getData(AP_CALIBRATION_DATA_ISRISKENTITY, NOCHECK);
				// get original object (not for risk)
				if (!attrIsRisk.isDefined() || attrIsRisk.isNull())
				{
					pFX = &tmpFx;
					break;
				}
				else
				{
					bool isRisk = dynamic_cast<const AQLDataBool &>(attrIsRisk.get()).get();
					if (!isRisk)
					{
						pFX = &tmpFx;
						break;
					}
				}
			}
		}
		++it;
	}

	return pFX;
}

// 
/*!
    @brief setup mareket data to AQLMathFXEntity

	@param[in, out] fx

*/
void
AQLMarketData::setUpMarket2FXEntity(AQLMathFXEntity &fx)
{
	// set asof date
	AQLString asofstr = AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE);
	fx.getAsOfDate().convertFromString(asofstr);
	AQLStringVector ccys = AQLDealUtils::getAllSingleCurrencys();

	// read spot rate file
	AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	AQLString fxfile = staticData.getStaticData(KEY_FXSPOTRATES_FILE);
	AQLStringMatrix fxratesMatrix;
	AQLFileAccessor fxratesFile(AQLMarketData::getNumFileName(fxfile));
	fxratesFile.readAllData(',',fxratesMatrix);
	fxratesFile.close();

	DoubleVector spotrates(ccys.size());
	AQLStringVector calendars(ccys.size());
	AQLString unitccy = staticData.getStaticData(KEY_FXSPOTRATES_UNITCCY);
	unitccy.toUpper();
    map<AQLString, double> spot_rate_map;
    for(unsigned int i = 0; i < fxratesMatrix.size(); ++i)
	{
        if (fxratesMatrix[i].size() < 3)
		{
			throw AQLCoreInvalidData("spotrate file size error", __FILE__, __LINE__);
		}
        if (fxratesMatrix[i][0] != unitccy)
		{
			throw AQLCoreInvalidData("The first column of a spot rate file must be unit currency", __FILE__, __LINE__);
		}
        spot_rate_map[fxratesMatrix[i][1]] = fxratesMatrix[i][2].getDoubleValue();
    }

	for (unsigned int i = 0; i < ccys.size(); ++i)
	{
		AQLString sccy = ccys[i];
		sccy.toLower();

		//set calendar;
		AQLString calstr = staticData.getStaticData(sccy + STATIC_DATA_KEY_FXSPOTRATES_CALENDAR);
		if (calstr == AQ_NO_DATA)
		{
			throw AQLCoreInvalidData("fxspotrates.calendar is not set.", __FILE__, __LINE__);
		}
		calendars[i] = calstr;


		if (ccys[i] == unitccy)
		{
			spotrates[i] = 1.0;		
		}
		else
		{
            map<AQLString, double>::const_iterator it = spot_rate_map.find(ccys[i]);
            if(it==spot_rate_map.end()){
                AQLString msg;
                msg += "A spot rate for ";
                msg += ccys[i] + " is missed.";
                throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
            }
            spotrates[i] = it->second;
		}
	}
    AQLStringVector temp_ccy = ccys;
    if (find(ccys.begin(), ccys.end(), "USD") == ccys.end())
	{
        temp_ccy.push_back("USD");
        calendars.push_back(staticData.getStaticData(AQLString("usd") + STATIC_DATA_KEY_FXSPOTRATES_CALENDAR));
        map<AQLString, double>::const_iterator it = spot_rate_map.find("USD");
        if (it==spot_rate_map.end())
		{
            throw AQLCoreInvalidData("A spot rate for usd is missed.", __FILE__, __LINE__);
        }
        spotrates.push_back(it->second);
    }

	//calendar and spotrates
    fx.getCurrencys().set(temp_ccy);
	fx.getCalendarNames().set(calendars);
	fx.getSpotRates().set(spotrates);

	AQLStringVector sdeccys = AQLDealUtils::getSDECurrencys();
	for (unsigned int i = 0; i < sdeccys .size();i++)
	{
		if (sdeccys [i].findString('/') < 0)
		{
			AQLString tmp_currency = sdeccys [i];
			AQLString spotlag_txt = staticData.getStaticData(tmp_currency.toLower() + STATIC_DATA_KEY_SPOTLAG);
			if (spotlag_txt != AQ_NO_DATA)
			{
				AQLDataDouble spot_tmp;
				spot_tmp.convertFromString(spotlag_txt);
				double check_spotlag = spot_tmp;
				if (check_spotlag < 0)
				{
					throw AQLCoreInvalidData("spotlag must be positive value",__FILE__,__LINE__);
				}
				unsigned int spotlag = static_cast<unsigned int>(check_spotlag);
				fx.setSpotLag(tmp_currency.toUpper(), spotlag);
			}
			else
			{
				fx.setSpotLag(tmp_currency.toUpper(), 0);
			}
		}
	}
}

// 
/*!
    @brief get volatility function file path(include file num)

	@param[in] model
	@param[in] ccy
	@param[in] shift target ccy
	@param[in] calcType
	@param[in] scenarioNum
	@param[out] filePath
	@param[in] isgrid
	@param[in] gridTerm
	@param[in] baseshift
	@param[in] isbase
	
*/
void 
AQLMarketData::getVolFuncFilePath(const AQLString &model, const AQLString &ccy, const AQLString &shift_ccy, const AQLString &calcType, SCENARIONUM scenarioNum, AQLStringVector &filePath, bool isGrid, const AQLString *pGridTerm, const AQLString *pBaseShift, bool isBase)
{
	AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	AQLStaticData &riskStaticData = AQLCoreDataService::getStaticDataManager().getRiskStaticData();
	AQLString tmpModel = model;
	AQLString tmpType = calcType;
	AQLString tmpCurrency = ccy;
	AQLString tmpGridTerm;
	AQLString tmpBaseShift;
	tmpModel.toUpper();
	tmpType.toUpper();
	tmpCurrency.toLower();

	if (pGridTerm)
	{
		tmpGridTerm = *pGridTerm;
		tmpGridTerm.toLower();
	}

	if (pBaseShift)
	{
		tmpBaseShift = *pBaseShift;
		tmpBaseShift.toLower();
	}


	filePath.clear();
	// LMM
	if (tmpModel == MODEL_LMM)
	{
		if (tmpType == "PV")
		{
			filePath.push_back(getNumFileName(staticData.getStaticData(tmpCurrency + 
								STATIC_DATA_KEY_LMM_VOLATILITY_FUNC_CALIB_FILE)));
			filePath.push_back(getNumFileName(staticData.getStaticData(tmpCurrency + 
								STATIC_DATA_KEY_LMM_VOLATILITY_FUNC_ADJ_FILE)));
		}
		else if (tmpType == RISK_OFFICIAL_VOL_CAPVEGA)
		{
			filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
								STATIC_DATA_KEY_RISK_OFFICIAL_VOL_CAPVEGA_LMM_VOLATILITY_FUNC_CALIB_FILE)));
			filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
								STATIC_DATA_KEY_RISK_OFFICIAL_VOL_CAPVEGA_LMM_VOLATILITY_FUNC_ADJ_FILE)));
		}
		else if (tmpType == RISK_OFFICIAL_VOL_SWAPTIONVEGA)
		{
			filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
								STATIC_DATA_KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_LMM_VOLATILITY_FUNC_CALIB_FILE)));
			filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
								STATIC_DATA_KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_LMM_VOLATILITY_FUNC_ADJ_FILE)));
		}
		else if (tmpType == RISK_FRONT_VOL_IRVEGA)
		{
			// ir vega pv file
			filePath.push_back(getNumFileName(staticData.getStaticData(tmpCurrency + 
								STATIC_DATA_KEY_LMM_VOLATILITY_FUNC_CALIB_FILE)));
			filePath.push_back(getNumFileName(staticData.getStaticData(tmpCurrency + 
								STATIC_DATA_KEY_LMM_VOLATILITY_FUNC_ADJ_FILE)));

		}
		else if (tmpType == RISK_FRONT_YIELD_IRDELTA || tmpType == RISK_FRONT_YIELD_IRSHIFTDELTA )
		{
			// ir delta pv file
			filePath.push_back(getNumFileName(staticData.getStaticData(tmpCurrency + 
								STATIC_DATA_KEY_LMM_VOLATILITY_FUNC_CALIB_FILE)));
			filePath.push_back(getNumFileName(staticData.getStaticData(tmpCurrency + 
								STATIC_DATA_KEY_LMM_VOLATILITY_FUNC_ADJ_FILE)));
		}
		else
		{
			AQLString msg = "CalcType  " + tmpType + " is not support in lmm";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__); 
		}
		// max file push
		//if (tmpType == "PV")
		//{
		//	filePath.push_back(getNumFileName(staticData.getStaticData(tmpCurrency + 
		//								STATIC_DATA_KEY_LMM_MAXTERM_FILE)));
		//}
		//else
		if (tmpType == RISK_OFFICIAL_VOL_CAPVEGA)
		{
			filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
										STATIC_DATA_KEY_RISK_OFFICIAL_VOL_CAPVEGA_LMM_MAXTERM_FILE)));
		}
		else if (tmpType == RISK_OFFICIAL_VOL_SWAPTIONVEGA)
		{
			filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
										STATIC_DATA_KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_LMM_MAXTERM_FILE)));
			
		}
		else
		{
			filePath.push_back(getNumFileName(staticData.getStaticData(tmpCurrency + 
										STATIC_DATA_KEY_LMM_MAXTERM_FILE)));
		}

//		filePath.push_back(getNumFileName(staticData.getStaticData(tmpCurrency + 
//										STATIC_DATA_KEY_LMM_MAXTERM_FILE)));
		// skew file push
		AQLStringVector ccys = AQLDealUtils::getSDECurrencys();
		if (ccys.size() > 1)
		{
			filePath.push_back(getNumFileName(staticData.getStaticData(tmpCurrency + 
										STATIC_DATA_KEY_LMM_SKEW_CROSS_FILE)));	
		}
		else
		{
			filePath.push_back(getNumFileName(staticData.getStaticData(tmpCurrency + 
										STATIC_DATA_KEY_LMM_SKEW_SINGLE_FILE)));	
		}
		return;
	}
	// HW
	else if (tmpModel == MODEL_HW)
	{
		if (tmpType == RISK_OFFICIAL_VOL_CAPVEGA)
		{
			filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
								STATIC_DATA_KEY_RISK_OFFICIAL_VOL_CAPVEGA_HW_PARAMETER_FILE)));
		}
		else if (tmpType == RISK_OFFICIAL_VOL_SWAPTIONVEGA)
		{
			filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
								STATIC_DATA_KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_HW_PARAMETER_FILE)));
		}
		else if (tmpType == RISK_FRONT_YIELD_IRDELTA)
		{
			// front risk irdelta
			if (isGrid && ! pGridTerm)
			{
				throw AQLCoreInvalidData("If grid risk calc, grid term must be need.", __FILE__, __LINE__);
			}
			if (scenarioNum == SCENARIO_1)
			{
				if (isGrid)
				{
					filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_SCE1_GRID_HW_PARAMETER_FILE + tmpGridTerm)));

				}
				else
				{
					filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_SCE1_PARALLEL_HW_PARAMETER_FILE)));
				}
			}
			else
			{
				if (isGrid)
				{
					filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_SCE2_GRID_HW_PARAMETER_FILE + tmpGridTerm)));

				}
				else
				{
					filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_SCE2_PARALLEL_HW_PARAMETER_FILE)));
				}
			}
		}
		else if (tmpType == RISK_FRONT_YIELD_IRSHIFTDELTA)
		{
			// front risk irshift delta
			if (!pBaseShift)
			{
				throw AQLCoreInvalidData("If shift delta. shift valu must be needed.", __FILE__, __LINE__);
			}
			if (isGrid && ! pGridTerm)
			{
				throw AQLCoreInvalidData("If grid risk calc, grid term must be needed.", __FILE__, __LINE__);
			}
			if (isBase)
			{
				filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_BASE_HW_PARAMETER_FILE + tmpBaseShift)));

			}
			else
			{
				if (scenarioNum == SCENARIO_1)
				{
					if (isGrid)
					{
						filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
										STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_SCE1_GRID_HW_PARAMETER_FILE + tmpBaseShift + "." + tmpGridTerm)));

					}
					else
					{
						filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
										STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_SCE1_PARALLEL_HW_PARAMETER_FILE + tmpBaseShift)));
					}
				}
				else
				{
					if (isGrid)
					{
						filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
										STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_SCE2_GRID_HW_PARAMETER_FILE + tmpBaseShift + "." + tmpGridTerm)));

					}
					else
					{
						filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
										STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_SCE2_PARALLEL_HW_PARAMETER_FILE + tmpBaseShift)));
					}
				}
			}
		}
		if (tmpType == RISK_FRONT_YIELD_BASISDELTA)
		{
			// front risk basisdelta
			if (isGrid && ! pGridTerm)
			{
				throw AQLCoreInvalidData("If grid risk calc, grid term must be need.", __FILE__, __LINE__);
			}
			if (scenarioNum == SCENARIO_1)
			{
				if (isGrid)
				{
					filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_SCE1_GRID_HW_PARAMETER_FILE + tmpGridTerm)));

				}
				else
				{
					filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_SCE1_PARALLEL_HW_PARAMETER_FILE)));
				}
			}
			else
			{
				if (isGrid)
				{
					filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_SCE2_GRID_HW_PARAMETER_FILE + tmpGridTerm)));

				}
				else
				{
					filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_SCE2_PARALLEL_HW_PARAMETER_FILE)));
				}
			}
		}
		else if (tmpType == RISK_FRONT_VOL_IRVEGA)
		{
			// front risk irvega 
			if (isGrid && ! pGridTerm)
			{
				throw AQLCoreInvalidData("If grid risk calc, grid term must be needed.", __FILE__, __LINE__);
			}
			if (scenarioNum == SCENARIO_1)
			{
				if (isGrid)
				{
					filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
									STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_SCE1_GRID_HW_PARAMETER_FILE  + tmpGridTerm)));

				}
				else
				{
					filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
									STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_SCE1_PARALLEL_HW_PARAMETER_FILE)));
				}
			}
			else
			{
				if (isGrid)
				{
					filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
									STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_SCE2_GRID_HW_PARAMETER_FILE  + tmpGridTerm)));

				}
				else
				{
					filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
									STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_SCE2_PARALLEL_HW_PARAMETER_FILE)));
				}
			}
		}
	}
	// PTBERG
	else if (tmpModel == MODEL_PTBERG)
	{
		if (tmpType == "PV")
		{
			filePath.push_back(getNumFileName(staticData.getStaticData(tmpCurrency + 
								FX_KEY_PTBERG_VOLATILITY_CALIB_FILE)));
		}
		else if (tmpType == RISK_OFFICIAL_FX_DELTA)
		{
			if (scenarioNum == SCENARIO_1)
			{
				filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
									FX_KEY_RISK_OFFICIAL_FX_DELTA_PTBERG_UPSHIFT_CALIB_FILE)));

			}
			else
			{
				filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
									FX_KEY_RISK_OFFICIAL_FX_DELTA_PTBERG_DOWNSHIFT_CALIB_FILE)));
			}
		}
		else if (tmpType == RISK_OFFICIAL_VOL_FXATMPARALLELVEGA)
		{
			filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
								FX_KEY_RISK_OFFICIAL_VOL_ATMPARALLELVEGA_PTBERG_CALIB_FILE)));
		}
		else if (tmpType == RISK_FRONT_YIELD_IRDELTA)
		{
			// front risk irdelta
			if (isGrid && ! pGridTerm)
			{
				throw AQLCoreInvalidData("If grid risk calc, grid term must be need.", __FILE__, __LINE__);
			}
			AQLStringVector ccys = shift_ccy.toToken(FX_DELIMITER);
			if (ccys.size() != 2)
			{
				throw AQLCoreInvalidData("Ptberg model shift_ccy must be fx.", __FILE__, __LINE__);
			}
			AQLString key_fx = getFXKey(ccys[0], ccys[1]);
			if (scenarioNum == SCENARIO_1)
			{
				if (isGrid)
				{
					filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_SCE1_GRID_PTBERG_PARAMETER_FILE + key_fx + "." + tmpGridTerm)));

				}
				else
				{
					filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_SCE1_PARALLEL_PTBERG_PARAMETER_FILE + key_fx)));
				}
			}
			else
			{
				if (isGrid)
				{
					filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_SCE2_GRID_PTBERG_PARAMETER_FILE + key_fx + "." + tmpGridTerm)));

				}
				else
				{
					filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_SCE2_PARALLEL_PTBERG_PARAMETER_FILE + key_fx)));
				}
			}
		}
		else if (tmpType == RISK_FRONT_YIELD_IRSHIFTDELTA)
		{
			// front risk irshiftdelta
			if (!pBaseShift)
			{
				throw AQLCoreInvalidData("If shift delta. shift valu must be needed.", __FILE__, __LINE__);
			}
			if (isGrid && ! pGridTerm)
			{
				throw AQLCoreInvalidData("If grid risk calc, grid term must be need.", __FILE__, __LINE__);
			}
			AQLStringVector ccys = shift_ccy.toToken(FX_DELIMITER);
			if (ccys.size() != 2)
			{
				throw AQLCoreInvalidData("Ptberg model shift_ccy must be fx.", __FILE__, __LINE__);
			}
			AQLString key_fx = getFXKey(ccys[0], ccys[1]);
			if (isBase)
			{
				filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_BASE_PTBERG_PARAMETER_FILE + key_fx + "." + tmpBaseShift)));
			}
			else
			{
				if (scenarioNum == SCENARIO_1)
				{
					if (isGrid)
					{
						filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
										STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_SCE1_GRID_PTBERG_PARAMETER_FILE + key_fx + "." + tmpBaseShift + "." + tmpGridTerm)));

					}
					else
					{
						filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
										STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_SCE1_PARALLEL_PTBERG_PARAMETER_FILE + key_fx + "." + tmpBaseShift)));
					}
				}
				else
				{
					if (isGrid)
					{
						filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
										STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_SCE2_GRID_PTBERG_PARAMETER_FILE + key_fx + "." + tmpBaseShift + "." + tmpGridTerm)));

					}
					else
					{
						filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
										STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_SCE2_PARALLEL_PTBERG_PARAMETER_FILE + key_fx + "." + tmpBaseShift)));
					}
				}
			}
		}
		else if (tmpType == RISK_FRONT_YIELD_BASISDELTA)
		{
			// front risk basisdelta
			if (isGrid && ! pGridTerm)
			{
				throw AQLCoreInvalidData("If grid risk calc, grid term must be need.", __FILE__, __LINE__);
			}
			AQLStringVector ccys = shift_ccy.toToken(FX_DELIMITER);
			if (ccys.size() != 2)
			{
				throw AQLCoreInvalidData("Ptberg model shift_ccy must be fx.", __FILE__, __LINE__);
			}
			AQLString key_fx = getFXKey(ccys[0], ccys[1]);
			if (scenarioNum == SCENARIO_1)
			{
				if (isGrid)
				{
					filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_SCE1_GRID_PTBERG_PARAMETER_FILE + key_fx + "." + tmpGridTerm)));

				}
				else
				{
					filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_SCE1_PARALLEL_PTBERG_PARAMETER_FILE + key_fx)));
				}
			}
			else
			{
				if (isGrid)
				{
					filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_SCE2_GRID_PTBERG_PARAMETER_FILE + key_fx + "." + tmpGridTerm)));

				}
				else
				{
					filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_SCE2_PARALLEL_PTBERG_PARAMETER_FILE + key_fx)));
				}
			}
		}
		else if (tmpType == RISK_FRONT_VOL_IRVEGA)
		{
			// front risk irvega
			if (isGrid && ! pGridTerm)
			{
				throw AQLCoreInvalidData("If grid risk calc, grid term must be need.", __FILE__, __LINE__);
			}
			AQLStringVector ccys = shift_ccy.toToken(FX_DELIMITER);
			if (ccys.size() != 2)
			{
				throw AQLCoreInvalidData("Ptberg model shift_ccy must be fx.", __FILE__, __LINE__);
			}
			AQLString key_fx = getFXKey(ccys[0], ccys[1]);
			if (scenarioNum == SCENARIO_1)
			{
				if (isGrid)
				{
					filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
									STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_SCE1_GRID_PTBERG_PARAMETER_FILE + key_fx + "." + tmpGridTerm)));

				}
				else
				{
					filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
									STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_SCE1_PARALLEL_PTBERG_PARAMETER_FILE + key_fx)));
				}
			}
			else
			{
				if (isGrid)
				{
					filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
									STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_SCE2_GRID_PTBERG_PARAMETER_FILE + key_fx + "." + tmpGridTerm)));

				}
				else
				{
					filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
									STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_SCE2_PARALLEL_PTBERG_PARAMETER_FILE + key_fx)));
				}
			}
		}
		else if (tmpType == RISK_FRONT_FX_DELTA)
		{
			// front risk fxdelta
			if (scenarioNum == SCENARIO_1)
			{
				filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
												FX_KEY_RISK_FRONT_FX_DELTA_SCE1_PTBERG_PARAMETER_FILE)));

			}
			else
			{
				filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
												FX_KEY_RISK_FRONT_FX_DELTA_SCE2_PTBERG_PARAMETER_FILE)));
			}
		}
		else if (tmpType == RISK_FRONT_VOL_FXVEGA)
		{
			// front risk fxvega
			if (isGrid && ! pGridTerm)
			{
				throw AQLCoreInvalidData("If grid risk calc, grid term must be need.", __FILE__, __LINE__);
			}
			AQLStringVector ccys = shift_ccy.toToken(FX_DELIMITER);
			if (ccys.size() != 2)
			{
				throw AQLCoreInvalidData("Ptberg model shift_ccy must be fx.", __FILE__, __LINE__);
			}
			AQLString key_fx = getFXKey(ccys[0], ccys[1]);
			if (scenarioNum == SCENARIO_1)
			{
				if (isGrid)
				{
					filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
									FX_KEY_RISK_FRONT_VOL_FXVEGA_SCE1_GRID_PTBERG_PARAMETER_FILE  + tmpGridTerm)));

				}
				else
				{
					filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
									FX_KEY_RISK_FRONT_VOL_FXVEGA_SCE1_PARALLEL_PTBERG_PARAMETER_FILE)));
				}
			}
			else
			{
				if (isGrid)
				{
					filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
									FX_KEY_RISK_FRONT_VOL_FXVEGA_SCE2_GRID_PTBERG_PARAMETER_FILE  + tmpGridTerm)));

				}
				else
				{
					filePath.push_back(getNumFileName(riskStaticData.getStaticData(tmpCurrency + 
									FX_KEY_RISK_FRONT_VOL_FXVEGA_SCE2_PARALLEL_PTBERG_PARAMETER_FILE)));
				}
			}
		}
		else
		{
			AQLString msg = "CalcType  " + tmpType + " is not support in ptberg";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__); 
		}
		return;
	}
}

// 
/*!
    @brief get volatility input type

	@param[in] model
	@param[in] ccy
	@param[in] calcType
	@return AQLString
	
*/
AQLString  
AQLMarketData::getVolInputType(const AQLString &model, const AQLString &ccy, const AQLString &calcType)
{
	AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	//AQLStaticData &riskStaticData = AQLCoreDataService::getStaticDataManager().getRiskStaticData();
	AQLString tmpModel = model;
	AQLString tmpType = calcType;
	AQLString tmpCurrency = ccy;
	tmpModel.toUpper();
	tmpType.toUpper();
	tmpCurrency.toLower();

	// LMM
	if (tmpModel == MODEL_LMM)
	{
		if (tmpType == "PV")
		{
			AQLString inputType = staticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_LMM_VOLATILITY_TYPE);
			inputType.toUpper();
			if (inputType == INPUT_FUNC)
			{
				return INPUT_T_FUNC_VECTOR;
			}
			else if (inputType == INPUT_DATA)
			{
				return INPUT_T_DATA_MATRIX;
			}
			else
			{
				throw AQLCoreInvalidData("CalcType PV. input type only support func or data in lmm.", __FILE__, __LINE__);
			}
		}
		else if (tmpType == RISK_FRONT_VOL_IRVEGA)
		{
//			return INPUT_T_DATA_MATRIX;
			bool isCalibrate = isCalibrateModel(tmpModel);
			AQLString inputType = staticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_LMM_VOLATILITY_TYPE);
			inputType.toUpper();
			if (inputType == INPUT_FUNC)
			{
				if (isCalibrate)
					return INPUT_T_FUNC_VECTOR;
				else
					return INPUT_T_DATA_MATRIX;
			}
			else if (inputType == INPUT_DATA)
			{
				return INPUT_T_DATA_MATRIX;
			}
			else
			{
				throw AQLCoreInvalidData("CalcType FrontRisk:VolIRVega. input type only support func or data in lmm.", __FILE__, __LINE__);
			}
		}
		else
		{
			//other case T_FUNC_VECTOR
			return INPUT_T_FUNC_VECTOR;
		} 
	}
	else if (tmpModel == MODEL_HW)
	{
		return INPUT_T_FUNC_VECTOR;
	}
	// PTBERG
	else if (tmpModel == MODEL_PTBERG)
	{
		return INPUT_FUNC_SCALAR;
	}
	else if (tmpModel == MODEL_FXSTRGLSLV)
	{
		return INPUT_FUNC_SCALAR;
	}
	//hishida vannavolga
	else if (tmpModel == MODEL_FXVANNAVOLGA)
	{
		return INPUT_FUNC_SCALAR;
	}
	else if (tmpModel == MODEL_IRSABR)
	{
		return INPUT_T_FUNC_VECTOR;
	}
	else if (tmpModel == MODEL_SZ)
	{
		return INPUT_FUNC_SCALAR;
	}
	else
	{
		AQLString msg = "This model is no supported now." + model;
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
}
// 
/*!
    @brief get model name
	note: key is currency or fx

	@param[in] key
	@return AQLString
*/
AQLString 
AQLMarketData::getModelName(const AQLString &key)
{
	AQLString tmpKey = key;
	tmpKey.toUpper();
	bool isVolIndex = false;

	int volPos = tmpKey.findString(POSTFIX_VOL);
	if (tmpKey.findString(POSTFIX_VOL) >= 0)
	{
		tmpKey.remove(volPos, AQLString(POSTFIX_VOL).size());
		isVolIndex = true;
	}

	AQLStringVector keyVec = tmpKey.toToken(FX_DELIMITER);
	if (keyVec.size() != 1 && !isVolIndex)
	{
		// for fx
		tmpKey = getFXKey(keyVec[0], keyVec[1]);
	}
	AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();

	AQLString ret;
	if (AQLDealUtils::getSDECurrencys().size() > 1)
	{
		ret = staticData.getStaticData(tmpKey.toLower() + STATIC_DATA_FX_KEY_SDE_CROSS_MODEL);
	}
	else
	{
		ret = staticData.getStaticData(tmpKey.toLower() + STATIC_DATA_FX_KEY_SDE_SINGLE_MODEL);
	}
	if (isVolIndex)
	{
		ret = ret + POSTFIX_VOL;
	}
	return ret;
}
// 
/*!
    @brief get model name
	note: key is currency or fx

	@param[in] key
	@return AQLString
*/
AQLString 
AQLMarketData::getSDEName(const AQLString &key)
{
	AQLString tmpKey = key;
	tmpKey.toUpper();
	bool isVolIndex = false;

	int volPos = tmpKey.findString(POSTFIX_VOL);
	if (tmpKey.findString(POSTFIX_VOL) >= 0)
	{
		tmpKey.remove(volPos, AQLString(POSTFIX_VOL).size());
		isVolIndex = true;
	}

	AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	return staticData.getStaticData(tmpKey.toLower() + (isVolIndex ? ".volatility" : "") + STATIC_DATA_FX_KEY_SDE_NAME);
}
#endif

// 
/*!
    @brief get key fx
	
	get fx key string for static data

	@param[in] ccy1
	@param[in] ccy2
	@return AQLString
*/
AQLString 
AQLMarketData::getFXKey(const AQLString &ccy1, const AQLString &ccy2)
{
	AQLStringVector ccys;
	convertToCurrency(ccy1 + AQLString("/") + ccy2, ccys);
	ccys[0].toLower();
	ccys[1].toLower();
	return ccys[0] + AQLString("/") + ccys[1];
}

/*!
    @brief sort market data

	@param[out] ypro
*/
void
AQLMarketData::sortMarketData(AQLMathYieldCurvePro &ypro)
{
	AQLDataMultiReference &refMarketDatas = ypro.getMarketData();
	const unsigned int dataSize =refMarketDatas.getSize();
	if (dataSize == 0)
	{
		return;
	}
	vector<const AQLObject*> data_on, data_tn, data_libor, data_swap, data_future, data_basis, data_fra6m, data_fra3m;
	for (unsigned int i = 0; i < dataSize; ++i)
	{
		AQLObject *data = &refMarketDatas.get(i).get();
		AQLString type = dynamic_cast<const AQLDataString&> ((data->getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
		type.toUpper();
		if (type == YIELD_TYPE_O_N)
		{
			data_on.push_back(data);
		}
		else if (type == YIELD_TYPE_T_N)
		{
			data_tn.push_back(data);
		}
		else if (type == YIELD_TYPE_ZERO)
		{
			data_libor.push_back(data);
		}
		else if (type == YIELD_TYPE_PAR)
		{
			data_swap.push_back(data);
		}
		else if (type == YIELD_TYPE_FUTURE)
		{
			data_future.push_back(data);
		}
		else if (type == YIELD_TYPE_BASIS)
		{
			data_basis.push_back(data);
		}
		else if (type == YIELD_TYPE_FRA3M)
		{
			data_fra3m.push_back(data);
		}
		else if (type == YIELD_TYPE_FRA6M)
		{
			data_fra6m.push_back(data);
		}
	}

	// sort term
	sort(data_libor.begin(), data_libor.end(), Comp_term());
	sort(data_swap.begin(), data_swap.end(), Comp_term());
	sort(data_future.begin(), data_future.end(), Comp_term());
	sort(data_basis.begin(), data_basis.end(), Comp_term());
	sort(data_fra3m.begin(), data_fra3m.end(), Comp_term());
	sort(data_fra6m.begin(), data_fra6m.end(), Comp_term());

	if (data_on.empty() || data_tn.empty())
	{
		throw AQLCoreInvalidData("ON or TN is not set.", __FILE__, __LINE__);
	}
	AQLString refStr = dynamic_cast<const AQLDataString &>(data_on[0]->getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
	refStr += ":" + dynamic_cast<const AQLDataString &>(data_tn[0]->getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
	// libor
	const int liborSize = data_libor.size();
	for (int i = 0; i < liborSize; ++i)
	{
		refStr += ":" + dynamic_cast<const AQLDataString &>(data_libor[i]->getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
	}
	// fra 3M
	const int fra3MSize = data_fra3m.size();
	for (int i = 0; i < fra3MSize; ++i)
	{
		refStr += ":" + dynamic_cast<const AQLDataString &>(data_fra3m[i]->getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
	}
	// fra 6M
	const int fra6MSize = data_fra6m.size();
	for (int i = 0; i < fra6MSize; ++i)
	{
		refStr += ":" + dynamic_cast<const AQLDataString &>(data_fra6m[i]->getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
	}
	// future
	const int futureSize = data_future.size();
	for (int i = 0; i < futureSize; ++i)
	{
		refStr += ":" + dynamic_cast<const AQLDataString &>(data_future[i]->getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
	}
	// swap
	const int swapSize = data_swap.size();
	for (int i = 0; i < swapSize; ++i)
	{
		refStr += ":" + dynamic_cast<const AQLDataString &>(data_swap[i]->getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
	}
	// basis
	AQLString b_refStr = "";
	const int basisSize = data_basis.size();
	for (int i = 0; i < basisSize; ++i)
	{
		AQLString name = dynamic_cast<const AQLDataString &>(data_basis[i]->getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
		refStr += ":" + name;
		b_refStr += name + ":";
	}

	refMarketDatas.convertFromString(refStr);
}

/*!
    @brief convert to currency(domain ccy and foregin ccy)

	@param[in] fx
	@param[out] ccys   first element domain ccy, second element foregin ccy
*/

void 
AQLMarketData::convertToCurrency(const AQLString &fx, AQLStringVector &ccys)
{
	ccys.clear();
	AQLStringVector t_ccys = fx.toToken(FX_DELIMITER);
	if (t_ccys.size() != 2)
	{
		throw AQLCoreInvalidData("FX format is wrong fx must be ccy1/ccy2 ", __FILE__, __LINE__);
	}
	// base currency
	ccys.resize(2);
	t_ccys[0].toUpper();
	t_ccys[1].toUpper();
	ccys[0] = t_ccys[0];
	ccys[1] = t_ccys[1];
	return;

	

}

#ifndef VISUAL_STUDIO_2010_ANALYTICS 
/*!
    @brief check correlation use model

	@param[in] model
	@return bool
*/
bool 
AQLMarketData::isCorUse(const AQLString &model)
{
	AQLString tmpModel = model;
	tmpModel.toUpper();
	if (tmpModel == MODEL_LMM)
	{
		// lmm use correlaion
		return true;
	}
	else if (tmpModel == MODEL_HW)
	{
		// hw no use
		return false;
	}
	else if (tmpModel == MODEL_PTBERG)
	{
		// ptberg(fx) no use
		return false;
	}

	return false;
}
#endif

// 
/*!
    @brief convert to M-Lib use term

	SN->1D, SW->7D, WeekType->DayType

	@param[in] term
	@return AQLString
	
*/
AQLString  
AQLMarketData::aqlConvertToTerm(const AQLString &term)
{
	// convert term
	if (term == "SN")
	{
		return "1D";
	}
	else if (term == "SW")
	{
		return "7D";
	}
	//else if (term.findString("W") == static_cast<int>(term.size() - 1))
	//{
	//	int weekNum = term.subString(0, term.size() - 2).getIntValue();
	//	int dayNum = 7 * weekNum;
	//	return AQLString(dayNum) + "D";
	//}
	else
	{
		return term;
	}
}

// 
/*!
    @brief search market grid pos correspond to AlgoQuantLib term

	@param[in] marketGrid
	@param[in] term
	@param[out] pos
	
*/
bool  
AQLMarketData::searchMarketGridPos(const AQLStringVector &marketGrid, const AQLString &term, unsigned int &pos)
{
	AQLStringVector::const_iterator it = find(marketGrid.begin(), marketGrid.end(), term);
	if (it != marketGrid.end())
	{
		pos = static_cast<unsigned int>(it - marketGrid.begin());
		return true;
	}
	else
	{
		// ON or TN first return element
		if (term == "ON" || term == "TN")
		{
			pos = 0;
			return true;
		}
		// 1D case
		else if (term == "1D")
		{
			AQLStringVector::const_iterator it_ = find(marketGrid.begin(), marketGrid.end(), "SN");
			if (it_ != marketGrid.end())
			{
				pos = static_cast<unsigned int>(it_ - marketGrid.begin());
				return true;
			}
			else
			{
				pos = 0;
				return false;
			}
		}
		// 7D case
		else if (term == "7D")
		{
			AQLStringVector::const_iterator it_ = find(marketGrid.begin(), marketGrid.end(), "SW");
			if (it_ != marketGrid.end())
			{
				pos = static_cast<unsigned int>(it_ - marketGrid.begin());
				return true;
			}
			else
			{
				pos = 0;
				return false;
			}
		}
		// Day type
		else if (term.findString("D") == static_cast<int>(term.size() - 1))
		{
			int daynum = term.subString(0, term.size() - 2).getIntValue();

			if (daynum % 7 != 0)
			{
				pos = 0;
				return false;
			}
			else
			{
				int weeknum = daynum / 7;
				AQLStringVector::const_iterator it_ = find(marketGrid.begin(), marketGrid.end(), AQLString(weeknum) + AQLString("D"));
				if (it_ != marketGrid.end())
				{
					pos = static_cast<unsigned int>(it_ - marketGrid.begin());
					return true;
				}
				else
				{
					pos = 0;
					return false;
				}
			}
		}
		else
		{
			pos = 0;
			return false;
		}
	}
}
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
// 
/*!
    @brief return volatility val

	@param[in] fileName
	@param[in] row term
	@param[in] 
	@return volatility
	
*/
double  
AQLMarketData::getVolatilityVal(const AQLString &fileName, const AQLString &termRow, const AQLString &termCol, bool isInter)
{
	AQLFileAccessor file(fileName);
	AQLStringMatrix volDataMtx;
	file.readAllData(MARKET_DATA_DELIMITER, volDataMtx);
	file.close();

	if (volDataMtx.empty())
	{
		AQLString msg("Volatility file format is wrong file = " + fileName);
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	AQLString uTermCol = termCol;
	uTermCol.toUpper();

	AQLString uTermRow = termRow;
	uTermRow.toUpper();

	unsigned int colSize = volDataMtx[0].size();
	unsigned int rowSize = volDataMtx.size();
	// data empty check
	for (unsigned int i = 0; i < rowSize; ++i)
	{
		if (volDataMtx[i].size() <= 1)
		{
			rowSize = i;
			break;
		}
	}

	if (colSize <= 1 || rowSize <= 1)
	{
		AQLString msg("Volatility file format is wrong file = " + fileName);
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	if (isInter)
	{
		AQLDate asof(AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
		AQLPriceDataDayCount act_365(ACT_365_ISDA);
		DoubleArray rowTerm(rowSize);
		DoubleArray colTerm(colSize);		
		// create rowTerm
		for (unsigned int i = 1; i < rowSize; ++i)
		{
			AQLDate date = AQLMathDateCalculations::getDate(asof, volDataMtx[i][0], true);
			rowTerm[i] = act_365.getTerm(asof, date);
		}
		// sorted rowTerm
		DoubleArray s_rowTerm = rowTerm;
		sort(s_rowTerm.begin(), s_rowTerm.end());

		// create colTerm
		for (unsigned int i = 1; i < colSize; ++i)
		{
			AQLDate date = AQLMathDateCalculations::getDate(asof, volDataMtx[0][i], true);
			colTerm[i] = act_365.getTerm(asof, date);
		}
		// sorted rowTerm
		DoubleArray s_colTerm = colTerm;
		sort(s_colTerm.begin(), s_colTerm.end());

		unsigned int rowPos = 0;
		unsigned int colPos = 0;
		DoubleMatrix valMtx(rowSize, DoubleArray(colSize));
		for (unsigned int i = 1; i < rowSize; ++i)
		{
			AQLAlgorithm::find<DoubleArray, double>(s_rowTerm, rowTerm[i], 1, s_rowTerm.size() - 1, rowPos); 
			for (unsigned int j = 1; j < colSize; ++j)
			{
				AQLAlgorithm::find<DoubleArray, double>(s_colTerm, colTerm[j], 1, s_colTerm.size() - 1, colPos);
				valMtx[rowPos][colPos] = volDataMtx[i][j].getDoubleValue();
			}
		}
		AQLDate t_rowDate = AQLMathDateCalculations::getDate(asof, termRow, true);
		double t_rowTerm = act_365.getTerm(asof, t_rowDate);

		AQLDate t_colDate = AQLMathDateCalculations::getDate(asof, termCol, true);
		double t_colTerm = act_365.getTerm(asof, t_colDate);

		AQLAlgorithm::locate<DoubleArray, double>(s_rowTerm, t_rowTerm, s_rowTerm.size(), rowPos); 
		AQLAlgorithm::locate<DoubleArray, double>(s_colTerm, t_colTerm, s_colTerm.size(), colPos);
		
		if (rowPos == 0 || colPos == 0)
		{
			// if rowTerm or colTerm <= zero, return 0.0; 
			return 0.0;
		}
		AQLLinearInterpolation inter;
		if (rowPos < s_rowTerm.size() && colPos < s_colTerm.size())
		{
			// rowTerm and colTerm is in the matrix
			// interpolation for row side
			DoubleArray idx_rowTerm(2);
			idx_rowTerm[0] = s_rowTerm[rowPos - 1];
			idx_rowTerm[1] = s_rowTerm[rowPos];
			DoubleArray val_1(2);
			val_1[0] = valMtx[rowPos - 1][colPos - 1];
			val_1[1] = valMtx[rowPos][colPos - 1];
			DoubleArray val_2(2);
			val_2[0] = valMtx[rowPos - 1][colPos];
			val_2[1] = valMtx[rowPos][colPos];

			DoubleArray inter_val(2);
			inter.set(idx_rowTerm, val_1);
			inter_val[0] = inter.value(t_rowTerm);
			inter.set(idx_rowTerm, val_2);
			inter_val[1] = inter.value(t_rowTerm);

			// interpolation for column side
			DoubleArray idx_colTerm(2);
			idx_colTerm[0] = s_colTerm[colPos - 1];
			idx_colTerm[1] = s_colTerm[colPos];
			inter.set(idx_colTerm, inter_val);

			return inter.value(t_colTerm);

		}
		else if (rowPos >= s_rowTerm.size() && colPos < s_colTerm.size())
		{
			// rowTerm is out of the matrix
			DoubleArray idx_colTerm(2);
			idx_colTerm[0] = s_colTerm[colPos - 1];
			idx_colTerm[1] = s_colTerm[colPos];
			DoubleArray val(2);
			val[0] = valMtx[rowSize - 1][colPos - 1];
			val[1] = valMtx[rowSize - 1][colPos];
			inter.set(idx_colTerm, val);
			return inter.value(t_colTerm);
		}
		else if (rowPos < s_rowTerm.size() && colPos >= s_colTerm.size())
		{
			// colTerm is out of the matrix
			DoubleArray idx_rowTerm(2);
			idx_rowTerm[0] = s_rowTerm[rowPos - 1];
			idx_rowTerm[1] = s_rowTerm[rowPos];
			DoubleArray val(2);
			val[0] = valMtx[rowPos - 1][colSize - 1];
			val[1] = valMtx[rowPos][colSize - 1];
			inter.set(idx_rowTerm, val);
			return inter.value(t_rowTerm);
		}
		else
		{
			// rowTerm and colTerm is out of the matrix
			return valMtx[rowSize - 1][colSize - 1];
		}

	}
	else
	{
		// search col pos
		unsigned int colPos = 0;
		for (unsigned int i = 1; i < colSize; ++i)
		{
			if (volDataMtx[0][i].toUpper() == uTermCol)
			{
				colPos = i;
				break;
			}
		}
		if (colPos == 0)
		{
			AQLString msg("Col term is not in volatility file, term = " + termCol);
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		
		// search row pos
		unsigned int rowPos = 0;
		for (unsigned int i = 1; i < rowSize; ++i)
		{
			if (volDataMtx[i].size() != colSize)
			{
				AQLString msg("Volatility file format is wrong file = " + fileName);
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}

			if (volDataMtx[i][0].toUpper() == uTermRow)
			{
				rowPos = i;
				break;
			}
		}
		if (rowPos == 0)
		{
			AQLString msg("Row term is not in volatility term = " + termRow);
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}

		return volDataMtx[rowPos][colPos].getDoubleValue();
	}

}

// 
/*!
    @brief check calibrate ?

	@param[in] model
	@param[in] ccy
	@return bool
	
*/
bool
AQLMarketData::isCalibrateModel(const AQLString &model)
{
	AQLString tmpModel = model;
	tmpModel.toUpper();
	// check calibrate model ?
	AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	AQLString calibModels = staticData.getStaticData(KEY_SDE_CALIB_MODEL).toUpper();
	
	if (calibModels == "ALL" || calibModels.findString(tmpModel) >= 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// 
/*!
    @brief return calibData Name

	@param[in] calcType
	@param[in] yieldName
	@param[in] gridPos
	@return AQLString
	
*/
AQLString
AQLMarketData::getCalibDataName(const AQLString &calcType, const AQLString &yieldName, int gridPos)
{
	return calcType + "_" + yieldName + "_" + AQLString(gridPos) + "_" + CALIBDATA;
}


// 
/*!
    @brief return calibData Name

	@param[in] objPool
	@param[in] yield
	@return AQLString
	
*/
AQLString
AQLMarketData::getYieldDataName(AQLObjectPool &objPool, const AQLString &ycName)
{
	const AQLMathYieldCurve &yc = dynamic_cast<const AQLMathYieldCurve &>(objPool.getObject(ycName, ENCHKTYPE_ISDEFINED).get());
	return yc.getYieldData().get().getName();
}

// 
/*!
    @brief check iscalibrate ends

	@param[in] calibIDName
	@param[in] objPool
	@return bool
	
*/
bool
AQLMarketData::isCalibrateEnd(const AQLString &calibIDName, AQLObjectPool &objPool)
{
	AQLStaticData &calibStaticData = AQLCoreDataService::getStaticDataManager().getCalibStaticData();
	unsigned long millisec = static_cast<unsigned long>(calibStaticData.getStaticData(KEY_CALIB_WAIT_MILLISECOND).getIntValue());
	unsigned int time = calibStaticData.getStaticData(KEY_CALIB_WAIT_TIME).getIntValue();
	for (unsigned int i = 0; i < time; ++i)
	{
		if (objPool.getObject(calibIDName, ENCHKTYPE_NOCHECK).isDefined())
		{
			return true;
		}

		AQLCoreThread::sleep(millisec);
	}
	return false;
}
#endif
// 
/*!
    @brief convert fx volatility calib format to market format

	@param[in] atm
	@param[in] dh25  25DeltaHight
	@param[in] dh10  10DeltaHight
	@param[in] dl25  25DeltaLow
	@param[in] dl10  10DeltaLow
	@param[out] bf25  25ButterFly
	@param[out] bf10  10ButterFly
	@param[out] rr25  25RiskReversal
	@param[out] rr10  10RiskReversal
	
*/
void 
AQLMarketData::convFXVolCalib2Market(const DoubleVector &atm, const DoubleVector &dh25, const DoubleVector &dh10, const DoubleVector &dl25, const DoubleVector &dl10,
										DoubleVector &bf25, DoubleVector &bf10, DoubleVector &rr25, DoubleVector &rr10)
{
	unsigned int gridSize = atm.size();
	if (gridSize != dh25.size() || gridSize != dh10.size() ||
			gridSize != dl25.size() || gridSize != dl10.size())
	{
		throw AQLCoreInvalidData("Calibration vol grid size is not consistent", __FILE__, __LINE__);
	}

	bf25.resize(gridSize);
	bf10.resize(gridSize);
	rr25.resize(gridSize);
	rr10.resize(gridSize);
	//convert to market vol data
	for (unsigned int i = 0; i < gridSize; ++i)
	{
		// BetterFly
		bf25[i] = 0.5 * (dh25[i] + dl25[i]) - atm[i];
		bf10[i] = 0.5 * (dh10[i] + dl10[i]) - atm[i];
		// RiskReversal
		rr25[i] = dh25[i] - dl25[i];
		rr10[i] = dh10[i] - dl10[i];
	}
}

// 
/*!
    @brief convert fx volatility  market format to calib format

	@param[in] atm
	@param[in] bf25  25ButterFly
	@param[in] bf10  10ButterFly
	@param[in] rr25  25RiskReversal
	@param[in] rr10  10RiskReversal
	@param[out] dh25  25DeltaHight
	@param[out] dh10  10DeltaHight
	@param[out] dl25  25DeltaLow
	@param[out] dl10  10DeltaLow
	
*/
void
AQLMarketData::convFXVolMarket2Calib(const DoubleVector &atm, const DoubleVector &bf25, const DoubleVector &bf10, const DoubleVector &rr25, const DoubleVector &rr10,
										DoubleVector &dh25, DoubleVector &dh10, DoubleVector &dl25, DoubleVector &dl10)
{
	unsigned int gridSize = atm.size();
	if (gridSize != rr25.size() || gridSize != rr10.size() ||
			gridSize != bf25.size() || gridSize != bf10.size())
	{
		throw AQLCoreInvalidData("Market vol grid size is not consistent", __FILE__, __LINE__);
	}

	dh25.resize(gridSize);
	dh10.resize(gridSize);
	dl25.resize(gridSize);
	dl10.resize(gridSize);
	//convert to market vol data
	for (unsigned int i = 0; i < gridSize; ++i)
	{
		//// Delta High
		dh25[i] = atm[i] + 0.5 * rr25[i] + bf25[i];
		dh10[i] = atm[i] + 0.5 * rr10[i] + bf10[i] ;
		// Delta Low
		dl25[i] = atm[i] - 0.5 * rr25[i] + bf25[i] ;
		dl10[i] = atm[i] - 0.5 * rr10[i] + bf10[i];
		
	}

}

// 
/*!
    @brief adjust df if df increase for time t

	@param[in] yielddata
	
*/
void 
AQLMarketData::adjustDiscountFactor(AQLObject& yielddata, const AQLString &curveType)
{
	AQLString suffix = "";
	if (curveType != STD)
	{
		suffix = "_" + curveType;
	}
	DoubleArray terms = dynamic_cast<AQLDataDoubles &>(yielddata.getData(CALIBRATION_DATA_TERMS + suffix, ISNOTNULL).get()).get();
	DoubleArray dfs = dynamic_cast<AQLDataDoubles &>(yielddata.getData(IR_CALIBRATION_DATA_DFS + suffix, ISNOTNULL).get()).get();

	DoubleArray vals = dfs;
	double b_df = 1.0;
	double b_term = 0.0;
	const unsigned int size = terms.size();
	for (unsigned int i = 0; i < size; ++i)
	{
		if  (b_df < dfs[i] && i > 0)
		{
			DoubleArray tmpTerms(1, b_term);
			DoubleArray tmpDfs(1, b_df);

			unsigned int j = i;
			for (; j < size - 1 && b_df < dfs[j]; ++j);

			if (j != i && dfs[j] <= b_df)
			{
				tmpTerms.push_back(terms[j]);
				tmpDfs.push_back(dfs[j]);
			}

			AQLLinearInterpolation interp = AQLLinearInterpolation();
			interp.set(tmpTerms, tmpDfs);
			vals[i] = interp.value(terms[i]);
		}
		else
		{
			b_df = dfs[i];
			b_term = terms[i];
		}
	}
	dynamic_cast<AQLDataDoubles &>(yielddata.getData(IR_CALIBRATION_DATA_DFS + suffix, ISNOTNULL).get()).set(vals);
	return;
}
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
// 
/*!
    @brief setup scenarioparam

	@param[in, out] scenarioparam
	
*/
void 
AQLMarketData::setUpScenario(AQLScenarioParam& param)
{
	// is detail output
	if (AQLCoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
	{
		param.isOutPut = true;
	}
	else 
	{
		param.isOutPut = false;
	}
	// is realicalib
 	AQLStaticData &riskStaticData = AQLCoreDataService::getStaticDataManager().getRiskStaticData();
	AQLDataBool tmpBool;
	tmpBool.convertFromString(riskStaticData.getStaticData(RISK_KEY_CALIB_ISREALCALIB));
	param.isCalib = tmpBool.get();

	// serialize status
	AQLStaticData &calibStaticData = AQLCoreDataService::getStaticDataManager().getCalibStaticData();
	param.serializeStatus = calibStaticData.getStaticData(KEY_CALIB_SERIALIZE_STATUS).toUpper();
	// serialize file
	param.serializeFile = getNumFileName(calibStaticData.getStaticData(KEY_CALIB_SERIALIZE_FILE));
	param.targetCurveType = STD;

	AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	AQLString strIsAudExtra = staticData.getStaticData(KEY_SDE_YIELD_ISAUDEXTRA);
	if (strIsAudExtra != AQ_NO_DATA)
	{
		tmpBool.convertFromString(strIsAudExtra);
		param.isAudExtra = tmpBool.get();
	}
}

#endif
/*!
    @brief restore swap rate

	@param[in, out] yield curve
	@param[in] swap rate map
	@param[in] ccy
*/
void 
AQLMarketData::restoreSwapRateFromL(AQLMathYieldCurvePro &curve, const map<AQLString, double> &sRateMap, const AQLString &ccy, const AQLString *pCurveType)
{
	AQLString suffix = "";
	AQLString data_suffix = "";
	if (pCurveType)
	{
		if (*pCurveType != STD)
		{
			suffix = "." + *pCurveType;
			suffix.toLower();
			data_suffix = "_" + *pCurveType;
		}
	}
	// get swap information
	AQLString tmpCurrency = ccy;
	tmpCurrency.toLower();
	AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	// daycount
	AQLPriceDataDayCount dc;
	AQLString dcStr = staticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNT + suffix).toUpper();
	dc.convertFromString(dcStr);
	// sliding rule
	AQLPriceDataSlidingRule sliding;
	AQLString slidingStr = staticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_SLIDINGRULE + suffix).toUpper();
	sliding.convertFromString(slidingStr);
	// frequency
	AQLString freq = staticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCY + suffix).toUpper();
	// calendar
	AQLPriceDataCalendar cal;
	AQLString calStr = staticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_CALENDAR + suffix);
	cal.convertFromString(calStr);
	// asOfDate
	const AQLDate asOfDate = curve.getAsOfDate();
	// spotDate
	AQLDate spotDate;
	// check spotDate use ?
	AQLDataBool tmpAttrB;
	tmpAttrB.convertFromString(staticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISSPOTUSE));
	if (tmpAttrB.get())
	{
		spotDate.setDate(staticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_SPOTDATE + suffix).getCString());
	}
	else
	{
		spotDate = cal.getBusinessDay(asOfDate, staticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_RESETLAG + suffix).getIntValue());
	}

	if (asOfDate > spotDate)
	{
		throw AQLCoreInvalidData("AsofDate > spotDate, cannnot calc. ", __FILE__, __LINE__);
	}

	// restore swap rate
	AQLDataMultiReference &refMarkets = dynamic_cast<AQLDataMultiReference &>(curve.getData(CALIBRATION_DATA_MARKETDATA + data_suffix, ISNOTNULL).get()); 
	//AQLDataMultiReference &refMarkets = curve.getMarketData();
	const unsigned int mSize = refMarkets.getSize();
	for (unsigned int i = 0; i < mSize; ++i)
	{
		AQLObjectHolder &mktData = refMarkets.get(i);

		AQLString dataType = dynamic_cast<const AQLDataString &>
						(mktData.getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL).get()).get();

		dataType.toUpper();
		if (dataType == YIELD_TYPE_ZERO || dataType == YIELD_TYPE_PAR)
		{
			const AQLString &termStr = dynamic_cast<const AQLDataString &>(mktData.getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL).get());
			map<AQLString, double>::const_iterator it = sRateMap.find(termStr);
			if (it != sRateMap.end())
			{
				// set swap rate and information
				// set original swap value
				mktData.remove(CALIBRATION_DATA_RATE);
				mktData.add(CALIBRATION_DATA_RATE, new AQLDataDouble(it->second));
				// set spot date
				mktData.remove(IR_CALIBRATION_DATA_SPOTDATE);
				mktData.add(IR_CALIBRATION_DATA_SPOTDATE, new AQLDataDate(spotDate));
				// set calendar
				mktData.remove(CALIBRATION_DATA_CALENDAR);
				mktData.add(CALIBRATION_DATA_CALENDAR , new AQLPriceDataCalendar()).convertFromString(calStr);
				// set daycount
				mktData.remove(IR_CALIBRATION_DATA_DAYCOUNT);
				mktData.add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(dcStr);
				// set data type
				mktData.remove(IR_CALIBRATION_DATA_DATATYPE);
				mktData.add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_PAR);
				// set frequency
				mktData.remove(IR_CALIBRATION_DATA_FREQUENCY);
				mktData.add(IR_CALIBRATION_DATA_FREQUENCY, new AQLDataString()).convertFromString(freq);
				// set slidingrule
				mktData.remove(CALIBRATION_DATA_SLIDINGRULE);
				mktData.add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(slidingStr);
			}
		}
	}
}

#ifndef VISUAL_STUDIO_2010_ANALYTICS 
// 
/*!
    @brief get AQLMathPathEntity from object pool

	@param[in] objPool
	@return AQLMathPathEntity &
*/
AQLMathPlainVanillaEntity * 
AQLMarketData::getPlainVanillaEntity(AQLObjectPool &objPool)
{
	AQLMathPlainVanillaEntity *pVanilla = 0;
	EntityIter it = objPool.begin();
	while (it != objPool.end())
	{
		if (it->second.isTypeOf(ENTITY_PLAINVANILLA))
		{
			//pVanilla = &dynamic_cast<AQLMathPlainVanillaEntity&>(it->second.get());
		
			AQLMathPlainVanillaEntity &tmpVanilla = dynamic_cast<AQLMathPlainVanillaEntity &>(it->second.get());
			const AQLDataHolder &attrIsRisk = tmpVanilla.getData(AP_CALIBRATION_DATA_ISRISKENTITY, NOCHECK);
			// get original object (not for risk)
			if (!attrIsRisk.isDefined() || attrIsRisk.isNull())
			{
				pVanilla = &tmpVanilla;
				break;
			}
			else
			{
				bool isRisk = dynamic_cast<const AQLDataBool &>(attrIsRisk.get()).get();
				if (!isRisk)
				{
					pVanilla = &tmpVanilla;
					break;
				}
			}
		}
		++it;
	}
	return pVanilla;
}


double 
AQLMarketData::getMaturityTermFromPlainVanillaEntity(AQLObject& trade, const AQLDate& asOfDate)
{
	double ret;

	AQLDataHolder* dh = &(trade.getData(PRICING_DATA_EXPIRYDATE,ISNOTNULL));
	AQLDate date = dynamic_cast<AQLDataDate &>(dh->get()).get();

	dh = &(trade.getData(PRICING_DATA_DELIVERYDATE,ISNOTNULL));
	AQLDate deldate = dynamic_cast<AQLDataDate &>(dh->get()).get();

	if (deldate > date)
		date = deldate;

	AQLPriceDataDayCount dc(ACT_365_ISDA);
	ret = dc.getTerm(asOfDate,date,true);

	return ret;

}


//get getBaseCurrencyOfFXPair
AQLString 
AQLMarketData::getBaseCurrencyOfFXPair(const AQLString& ccy)
{
	// get swap information
	AQLString tmpCurrency = ccy;
	tmpCurrency.toLower();
	if (-1 == tmpCurrency.findString(FX_DELIMITER))
		throw AQLCoreInvalidData("Input Error",__FILE__,__LINE__);
	
	AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();

	AQLString ret = staticData.getStaticData(tmpCurrency + FX_KEY_VALUATION_BASECURRENCYOFFXPAIR).toUpper();
	if (ret == AQ_NO_DATA)
		throw AQLCoreInvalidData("Input Error", __FILE__,__LINE__);

	return ret;
}

// 
/*!
	In case of SABR, a format of strTerm is Swaption_(OptionMaturity)_(Ternor). So we get the second element of strTerm.
	In case of OIS, a format of strTerm is OIS_(Term). So we get the second element of strTerm for OptionMaturity.
	In case of FRA, a format of strTerm is FRA*M_*x*. So we get the second element of strTerm and change the format for OptionMaturity.
	In case of LMM, a format of strTerm is *_*_*_*. In the case, this function does not make sense.
*/
double 
AQLMarketData::getCalendarTime(const AQLDate& asOfDate, AQLString strTerm)
{
	double ret = 0.0;
	AQLPriceDataCalendar cal;
	AQLPriceDataDayCount dayCount(ACT_365_ISDA);
	AQLPriceDataSlidingRule sl;
	strTerm.toUpper();

	//first check
	if (strTerm.findString("Y") == -1 &&
		strTerm.findString("M") == -1 &&
		strTerm.findString("D") == -1 &&
		strTerm.findString("W") == -1 &&
		strTerm.findString("ON") == -1 &&
		strTerm.findString("TN") == -1 &&
		strTerm.findString("SN") == -1 &&
		strTerm.findString("BOJ") == -1 &&
		strTerm.findString("EUSF") == -1 &&
		strTerm.findString("FF") == -1 &&
		strTerm.findString("FRA") == -1)
		throw AQLCoreInvalidData("strTerm Error",__FILE__,__LINE__);

	if (strTerm.findString("/") != -1)
	{
		ret = 0.;
	}
	else if (strTerm.findString("ON") != -1)
	{
		int d = 1;
		AQLDate date = cal.getBusinessDay(asOfDate, d);
		ret = dayCount.getTerm(asOfDate, date);
	}
	else if (strTerm.findString("TN") != -1)
	{
		int d = 2;
		AQLDate date = cal.getBusinessDay(asOfDate, d);
		ret = dayCount.getTerm(asOfDate, date);
	}
	else if (strTerm.findString("SN") != -1)
	{
		int d = 3;
		AQLDate date = cal.getBusinessDay(asOfDate, d);
		ret = dayCount.getTerm(asOfDate, date);
	}
	else if (strTerm.findString("BOJ") != -1 || strTerm.findString("EUSF") != -1 || strTerm.findString("FF") != -1)
	{
		ret = 0.;
	}
	else if (strTerm.findString("FRA") != -1)
	{
		AQLStringVector tmpStrTerm = strTerm.toToken('_');
		if (tmpStrTerm.size() != 2) throw AQLCoreInvalidData("Format of grid term is not supported!",__FILE__,__LINE__);
		strTerm = AQLPriceYieldGenerator::changeFRATermFormat(tmpStrTerm[1]);
		
		AQLDate date = AQLMathDateCalculations::getDate(asOfDate,strTerm,sl,&cal,true);
		ret = dayCount.getTerm(asOfDate, date);
	}
	else if (strTerm.findString("FUTURE") != -1)
	{
		AQLStringVector tmpStrTerm = strTerm.toToken('_');
		if (tmpStrTerm.size() != 2) throw AQLCoreInvalidData("Format of grid term is not supported!",__FILE__,__LINE__);
		
		AQLDate date = AQLMathDateCalculations::getIMMDateFromTerm(asOfDate, tmpStrTerm[1]);
		date.addMonths(3);
		ret = dayCount.getTerm(asOfDate, date);
	}
	else
	{
		if (strTerm.findString("_") != -1) 
		{
			AQLStringVector tmpStrTerm = strTerm.toToken('_');
			if (tmpStrTerm.size() != 2 && tmpStrTerm.size() != 3 && tmpStrTerm.size() != 4) throw AQLCoreInvalidData("Format of grid term is not supported!",__FILE__,__LINE__);
			strTerm = tmpStrTerm[1];
		}
		AQLDate date = AQLMathDateCalculations::getDate(asOfDate,strTerm,sl,&cal,true);
		ret = dayCount.getTerm(asOfDate, date);
	}
	return ret;
}
#endif
/*!
    @brief return future volatility from swaption market

	@param[in] ccy
	@param[in] expiry date 
	
*/
double
AQLMarketData::getFutureVolFromSwaption(const AQLString &ccy, const AQLDate &expityDate)
{
	AQLStaticData &calibStaticData = AQLCoreDataService::getStaticDataManager().getCalibStaticData();

	AQLString fileName = getNumFileName(calibStaticData.getStaticData(ccy + STATIC_DATA_KEY_CALIB_SWAPTION_FILE));
	AQLFileAccessor file(fileName);
	AQLStringMatrix volDataMtx;
	file.readAllData(MARKET_DATA_DELIMITER, volDataMtx);
	file.close();
	if (volDataMtx.empty())
	{
		AQLString msg("Volatility file format is wrong file = " + fileName);
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	unsigned int colSize = volDataMtx[0].size();
	unsigned int rowSize = volDataMtx.size();
	// data empty check
	for (unsigned int i = 0; i < rowSize; ++i)
	{
		if (volDataMtx[i].size() <= 1)
		{
			rowSize = i;
			break;
		}
	}

	if (colSize <= 1 || rowSize <= 1)
	{
		AQLString msg("Volatility file format is wrong file = " + fileName);
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	unsigned int cosPos_1Y;
	for (cosPos_1Y = 0; cosPos_1Y < colSize; ++cosPos_1Y)
	{
		if (volDataMtx[0][cosPos_1Y] == "1Y") break;
	}
	if (cosPos_1Y == colSize)
	{
		AQLString msg("1Y tenor data does not exist!");
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	// set as of date
	AQLString asOfDateStr = AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE);
	AQLDate asOfDate;
	asOfDate.setDate(asOfDateStr.getCString());
	AQLPriceDataDayCount act_365;
	act_365.setDayCount(ACT_365_ISDA);
	AQLString optionMat;
	DoubleArray optionMatVec(rowSize - 1);
	for (unsigned int i = 1; i < rowSize; i++)
	{
		optionMat = volDataMtx[i][0]; optionMat.toUpper();
		AQLDate toDate = AQLMathDateCalculations::getDate(asOfDate,optionMat,true);
		optionMatVec[i - 1] = act_365.getTerm(asOfDate,toDate,true);
	}
	
	//volatility
	DoubleArray volvec(rowSize - 1);
	for (unsigned int i = 1; i < rowSize; i++)
	{
		volvec[i - 1] = volDataMtx[i][cosPos_1Y].getDoubleValue();
	}

	AQLLinearInterpolation linear(LINEAR_EXTRAPOLATION_TYPE);
	linear.set(optionMatVec, volvec);

	double point = act_365.getTerm(asOfDate,expityDate,true);

	return linear.value(point);
}


/*!
    @brief return property val

	@param[in] staticData
	@param[in] ccy
	@param[in] key
	@param[in] is_include_key

	@return AQLString
	
*/
AQLString
AQLMarketData::getStaticDataValue(const AQLStaticData &staticData, const AQLString &ccy, const AQLString &key, const bool is_include_key)
{
	AQLString tmpCurrency = ccy;
	tmpCurrency.toLower();
	AQLString val = staticData.getStaticData(tmpCurrency + "." + key);
	if (is_include_key)
	{
		if (val == AQ_NO_DATA)
		{
			return staticData.getStaticData(key);
		}
		else
		{
			return val;
		}
	}
	else
	{
		return val;
	}
}

/*!
    @brief get frequency from accessory

	@param[in] accessory
	@return AQLString
	
*/
AQLString
AQLMarketData::getFrequency(const AQLString &accessory)
{
	AQLString c_accessory = accessory;
	c_accessory.toUpper();
	if (c_accessory == ACCESSORY_12M)
	{
		return FREQ_ANNUAL;
	}
	else if (c_accessory == ACCESSORY_6M)
	{
		return FREQ_SEMI_ANNUAL;
	}
	else if (c_accessory == ACCESSORY_3M)
	{
		return FREQ_QUARTERLY;
	}
	else if (c_accessory == ACCESSORY_1M)
	{
		return FREQ_MONTHLY;
	}
	else
	{
		AQLString msg = "Accessory " + c_accessory + " cannot be converted to frequency";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
}

#ifndef VISUAL_STUDIO_2010_ANALYTICS
void
AQLMarketData::shiftGridMarketVol(const AQLScenarioParam &param, int gridPos, const AQLStringVector *capTermVec, const AQLStringVector *capTenorVec, DoubleVector *capVolVec, const AQLStringVector *swapOptionMatVec, const AQLStringVector *swapTenorVec, DoubleMatrix *swaptionVolMtx)
{
	if (gridPos < 0)
	{
		throw AQLCoreInvalidData("AQLMarketData::shiftGridMarketVol is used in grid shift only.", __FILE__, __LINE__);
	}
	// grid shift case
	if (param.gridTerm.size() - 1 < static_cast<unsigned int>(gridPos) || 
			param.gridShiftVec.size() - 1 < static_cast<unsigned int>(gridPos))
	{
		AQLString msg = "Grid shift param does not exist, grid num = " + AQLString(gridPos);
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	// check
	AQLString term = param.gridTerm[gridPos];
	term.toUpper();
	int posSwaption = term.findString(CALIB_MARKET_SWAPTION);
	int posCap = term.findString(CALIB_MARKET_CAP);
	if(posSwaption == -1 && posCap == -1)
	{
		throw AQLCoreInvalidData("Market data is swaption or cap only.", __FILE__, __LINE__);
	}

	AQLString shiftType = param.shiftType;
	shiftType.toUpper();
	// cap
	if(posCap != -1)
	{
		if (!capTermVec || !capTenorVec || !capVolVec)
		{
			throw AQLCoreInvalidData("capTermVec or capTenorVec or capVolVec is Null.", __FILE__, __LINE__);
		}

		// shift val
		if(!param.isWave)
		{
			unsigned int gStartPos = strlen(CALIB_MARKET_CAP)+1;
			AQLString grid = term.subString(gStartPos, term.size() - 1);
			AQLStringVector gridIdx = grid.toToken('_');
			AQLString capTerm = gridIdx[0];
			AQLString capTenor = gridIdx[1];

			// check term
			AQLStringVector::const_iterator it = find(capTermVec->begin(), capTermVec->end(), capTerm);
			if (it == capTermVec->end())
			{
				AQLString msg = "Shift grid does not exist in cap market. grid = " + grid;
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			unsigned int index = static_cast<unsigned int>(it - capTermVec->begin());

			if ((*capTenorVec)[index] != capTenor)
			{
				AQLString msg = "Shift grid does not exist in cap market. grid = " + grid;
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}

			// shift val
			double shiftVal = param.gridShiftVec[gridPos];			
			if (shiftType == RISK_SHIFTTYPE_RATIO)
			{
				shiftVal = (*capVolVec)[index] * param.gridShiftVec[gridPos];
			}
			// add shift val
			(*capVolVec)[index] = AQLMath::max((*capVolVec)[index] + shiftVal, 0.0);
		}
		else
		{
			for(int i =0; i <= gridPos; i++)
			{
				AQLString term = param.gridTerm[i];
				term.toUpper();
				// do wave values when each term is the same as the term at the current gridPos
				int posCap = term.findString(CALIB_MARKET_CAP);
				if(posCap == -1)
				{
					throw AQLCoreInvalidData("Market data is swaption only or cap only.", __FILE__, __LINE__);
				}
					
				unsigned int gStartPos = strlen(CALIB_MARKET_CAP)+1;
				AQLString grid = term.subString(gStartPos, term.size() - 1);
				AQLStringVector gridIdx = grid.toToken('_');
				AQLString capTerm = gridIdx[0];
				AQLString capTenor = gridIdx[1];

				// check term
				AQLStringVector::const_iterator it = find(capTermVec->begin(), capTermVec->end(), capTerm);
				if (it == capTermVec->end())
				{
					AQLString msg = "Shift grid does not exist in cap market. grid = " + grid;
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
				unsigned int tmpindex = static_cast<unsigned int>(it - capTermVec->begin());

				double shiftVal = param.gridShiftVec[i];			
				if (shiftType == RISK_SHIFTTYPE_RATIO)
				{
					shiftVal = (*capVolVec)[tmpindex] * param.gridShiftVec[i];
				}
				// add shift val
				(*capVolVec)[tmpindex] = AQLMath::max((*capVolVec)[tmpindex] + shiftVal, 0.0);
			}	
		}
	}
	else if(posSwaption != -1)
	{
		// swaption
		if (!swapOptionMatVec || !swapTenorVec || !swaptionVolMtx)
		{
			throw AQLCoreInvalidData("swapOptionMatVec or swapTenorVec or swaptionVolMtx is Null.", __FILE__, __LINE__);
		}

		if (!param.isWave)
		{
			AQLStringVector tmpGridTerm = term.toToken('_');
			if (tmpGridTerm.size() != 3)
			{
				AQLString msg = "Grid format is wrong. grid = " + term;
				throw AQLCoreInvalidData("", __FILE__, __LINE__);
			}
			AQLString grid = tmpGridTerm[tmpGridTerm.size() - 2] + "_" + tmpGridTerm[tmpGridTerm.size() - 1];
			AQLStringVector gridIdx = grid.toToken('_');

			AQLString optionMat = gridIdx[0];
			AQLString swapTenor = gridIdx[1];

			AQLStringVector optionMatTerms =  gridIdx[0].toToken('-');
			AQLStringVector swapTenorTerms =  gridIdx[1].toToken('-');

			AQLStringVector::const_iterator it = find(swapOptionMatVec->begin(), swapOptionMatVec->end(), optionMatTerms[0]);
			if (it == swapOptionMatVec->end())
			{
				AQLString msg = "Shift grid does not exist in swaption market. grid = " + grid + ", option maturity = " + optionMatTerms[0];
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			unsigned int indexOptionMat_start = static_cast<unsigned int>(it - swapOptionMatVec->begin());
			unsigned int indexOptionMat_end = indexOptionMat_start;
			for (unsigned int i = 1; i < optionMatTerms.size(); ++i)
			{
				it = find(swapOptionMatVec->begin(), swapOptionMatVec->end(), optionMatTerms[i]);
				if (it == swapOptionMatVec->end())
				{
					AQLString msg = "Shift grid does not exist in swaption market. grid = " + grid + ", option maturity = " + optionMatTerms[i];
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
				unsigned int indexOptionMat_end_tmp = static_cast<unsigned int>(it - swapOptionMatVec->begin());
				if (indexOptionMat_end_tmp < indexOptionMat_end)
				{
					AQLString msg = "Option maturity format is wrong. grid = " + grid + ", option maturity = " + optionMat;
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
				indexOptionMat_end = indexOptionMat_end_tmp;
			}

			it = find(swapTenorVec->begin(), swapTenorVec->end(), swapTenorTerms[0]);
			if (it == swapTenorVec->end())
			{
				AQLString msg = "Shift grid does not exist in swaption market. grid = " + grid + ", swap tenor = " + swapTenorTerms[0];
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			unsigned int indexSwapTenor_start = static_cast<unsigned int>(it - swapTenorVec->begin());
			unsigned int indexSwapTenor_end = indexSwapTenor_start;
			for (unsigned int i = 1; i < swapTenorTerms.size(); ++i)
			{
				it = find(swapTenorVec->begin(), swapTenorVec->end(), swapTenorTerms[i]);
				if (it == swapTenorVec->end())
				{
					AQLString msg = "Shift grid does not exist in swaption market. grid = " + grid + ", swap tenor = " + swapTenorTerms[i];
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
				unsigned int indexSwapTenor_end_tmp = static_cast<unsigned int>(it - swapTenorVec->begin());
				if (indexSwapTenor_end_tmp < indexSwapTenor_end)
				{
					AQLString msg = "Swap tenor format is wrong. grid = " + grid + ", swap tenor = " + swapTenor;
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
				indexSwapTenor_end = indexSwapTenor_end_tmp;
			}
			double shiftVal = param.gridShiftVec[gridPos];			
			for (unsigned int i = indexOptionMat_start; i <= indexOptionMat_end; ++i)
			{
				for (unsigned int j = indexSwapTenor_start; j <= indexSwapTenor_end; ++j)
				{
					if (shiftType == RISK_SHIFTTYPE_RATIO)
					{
						shiftVal = (*swaptionVolMtx)[i][j] * param.gridShiftVec[gridPos];
					}
					// add shift val
					(*swaptionVolMtx)[i][j] =
						AQLMath::max((*swaptionVolMtx)[i][j] + shiftVal, 0.0);

				}
			}
		}
		else
		{
			for(int i = 0; i <= gridPos; i++)
			{
				AQLString term = param.gridTerm[i];
				term.toUpper();
				// do wave values when each term is the same as the term at the current gridPos
				int posSwaption = term.findString(CALIB_MARKET_SWAPTION);
				if(posSwaption == -1)
				{
					throw AQLCoreInvalidData("Market data is swaption only or cap only.", __FILE__, __LINE__);
				}
				//unsigned int gStartPos = strlen(CALIB_MARKET_SWAPTION)+1;
				AQLStringVector tmpGridTerm = term.toToken('_');
				if (tmpGridTerm.size() != 3)
				{
					AQLString msg = "Grid format is wrong. grid = " + term;
					throw AQLCoreInvalidData("", __FILE__, __LINE__);
				}
				//AQLString grid = term.subString(gStartPos, term.size() - 1);
				AQLString grid = tmpGridTerm[tmpGridTerm.size() - 2] + "_" + tmpGridTerm[tmpGridTerm.size() - 1];
				AQLStringVector gridIdx = grid.toToken('_');
				AQLString optionMat = gridIdx[0];
				AQLString swapTenor = gridIdx[1];

				AQLStringVector optionMatTerms =  gridIdx[0].toToken('-');
				AQLStringVector swapTenorTerms =  gridIdx[1].toToken('-');

				AQLStringVector::const_iterator it = find(swapOptionMatVec->begin(), swapOptionMatVec->end(), optionMatTerms[0]);
				if (it == swapOptionMatVec->end())
				{
					AQLString msg = "Shift grid does not exist in swaption market. grid = " + grid + ", option maturity = " + optionMatTerms[0];
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
				unsigned int indexOptionMat_start = static_cast<unsigned int>(it - swapOptionMatVec->begin());
				unsigned int indexOptionMat_end = indexOptionMat_start;
				for (unsigned int j = 1; j < optionMatTerms.size(); ++j)
				{
					it = find(swapOptionMatVec->begin(), swapOptionMatVec->end(), optionMatTerms[j]);
					if (it == swapOptionMatVec->end())
					{
						AQLString msg = "Shift grid does not exist in swaption market. grid = " + grid + ", option maturity = " + optionMatTerms[j];
						throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
					}
					unsigned int indexOptionMat_end_tmp = static_cast<unsigned int>(it - swapOptionMatVec->begin());
					if (indexOptionMat_end_tmp < indexOptionMat_end)
					{
						AQLString msg = "Option maturity format is wrong. grid = " + grid + ", option maturity = " + optionMat;
						throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
					}
					indexOptionMat_end = indexOptionMat_end_tmp;
				}

				it = find(swapTenorVec->begin(), swapTenorVec->end(), swapTenorTerms[0]);
				if (it == swapTenorVec->end())
				{
					AQLString msg = "Shift grid does not exist in swaption market. grid = " + grid + ", swap tenor = " + swapTenorTerms[0];
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
				unsigned int indexSwapTenor_start = static_cast<unsigned int>(it - swapTenorVec->begin());
				unsigned int indexSwapTenor_end = indexSwapTenor_start;
				for (unsigned int j = 1; j < swapTenorTerms.size(); ++j)
				{
					it = find(swapTenorVec->begin(), swapTenorVec->end(), swapTenorTerms[j]);
					if (it == swapTenorVec->end())
					{
						AQLString msg = "Shift grid does not exist in swaption market. grid = " + grid + ", swap tenor = " + swapTenorTerms[j];
						throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
					}
					unsigned int indexSwapTenor_end_tmp = static_cast<unsigned int>(it - swapTenorVec->begin());
					if (indexSwapTenor_end_tmp < indexSwapTenor_end)
					{
						AQLString msg = "Swap tenor format is wrong. grid = " + grid + ", swap tenor = " + swapTenor;
						throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
					}
					indexSwapTenor_end = indexSwapTenor_end_tmp;
				}

				// shift val
				double shiftVal = param.gridShiftVec[gridPos];
				for (unsigned int j = indexOptionMat_start; j <= indexOptionMat_end; ++j)
				{
					for (unsigned int k = indexSwapTenor_start; k <= indexSwapTenor_end; ++k)
					{
						if (shiftType == RISK_SHIFTTYPE_RATIO)
						{
							shiftVal = (*swaptionVolMtx)[j][k] * param.gridShiftVec[gridPos];
						}
						// add shift val
						(*swaptionVolMtx)[j][k] =
							AQLMath::max((*swaptionVolMtx)[j][k] + shiftVal, 0.0);

					}
				}
			}
		}
	}
}



/*!
	@brief get calibration property value

	@param[in] calib property accesor
	@param[in] key
	@param[in] grid
	@return AQLString value

*/
AQLString
AQLMarketData::getCalibStaticDataValue(const AQLStaticData* pCalibStaticData, const AQLString &key, const AQLString &grid)
{
	AQLString ret = pCalibStaticData->getStaticData(key + "." + grid);

	if (ret == AQ_NO_DATA)
	{
		return pCalibStaticData->getStaticData(key);
	}

	return ret;
}


/*!
	@brief get swap convention

	@param[in] entitypool
	@param[in] optionMaturity
	@param[in] tenor
	@param[in] Scenario Param
	@return AQLSwapConvention swap convention

*/
AQLSwapConvention
AQLMarketData::getSwapConvention(AQLObjectPool &objPool, const AQLString& optionMaturity, const AQLString& tenor, const AQLString& curveName, const AQLScenarioParam& param)
{

	AQLSwapConvention res;

	AQLStaticData* pCalibStaticData = &AQLCoreDataService::getStaticDataManager().getCalibStaticData();
	AQLString lowerCcy = param.ccy; lowerCcy.toLower();
	AQLString model = param.model;
	//LMM
	if (model.toUpper() == "LMM")
	{
		if (tenor == AQLString("1M") || tenor == AQLString("3M") || tenor == AQLString("6M"))
		{
			res.frequency = pCalibStaticData->getStaticData(lowerCcy + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_FREQUENCY).toUpper();
			res.spotLag = pCalibStaticData->getStaticData(lowerCcy + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_SPOTLAG).toUpper();
			AQLString strDaycount = pCalibStaticData->getStaticData(lowerCcy + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_DAYCOUNT).toUpper();
			res.daycount.convertFromString(strDaycount);
			AQLString capSlidingRule = pCalibStaticData->getStaticData(lowerCcy + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_SLIDINGRULE).toUpper();
			res.slidingRule.convertFromString(capSlidingRule);
			AQLString capCalendar = pCalibStaticData->getStaticData(lowerCcy + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_CALENDAR).toUpper();
			res.paymentCalendar.convertFromString(capSlidingRule);
			res.fixingCalendar = res.paymentCalendar;
		}
		else
		{
			res.frequency = pCalibStaticData->getStaticData(lowerCcy + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_FREQUENCY);
			res.spotLag = pCalibStaticData->getStaticData(lowerCcy + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_SPOTLAG);
			AQLString strDaycount = pCalibStaticData->getStaticData(lowerCcy + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_DAYCOUNT);
			res.daycount.convertFromString(strDaycount);
			AQLString swaptionSlidingRule = pCalibStaticData->getStaticData(lowerCcy + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_SLIDINGRULE);
			res.slidingRule.convertFromString(swaptionSlidingRule);
			AQLString swaptionCalendar = pCalibStaticData->getStaticData(lowerCcy + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_CALENDAR);
			res.paymentCalendar.convertFromString(swaptionCalendar);
			res.fixingCalendar = res.paymentCalendar;
		}
		res.forecastCurveName = pCalibStaticData->getStaticData(lowerCcy + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_FORECASTCURVE);
		res.discountCurveName = pCalibStaticData->getStaticData(lowerCcy + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_DISCOUNTCURVE);


	}
	else if (model.toUpper() == "HW")
	{
		AQLString appMat = AQLDealUtils::getAppMat();

		AQLString grid = optionMaturity + "." + tenor;
		grid.toLower();

		res.frequency = getCalibStaticDataValue(pCalibStaticData, lowerCcy + STATIC_DATA_KEY_CALIB_HW_FREQUENCY_GRID + appMat, grid).toUpper();
		res.spotLag = getCalibStaticDataValue(pCalibStaticData, lowerCcy + STATIC_DATA_KEY_CALIB_HW_NOTICEPERIOD_GRID + appMat, grid);
		AQLString strDaycount = getCalibStaticDataValue(pCalibStaticData, lowerCcy + STATIC_DATA_KEY_CALIB_HW_DAYCOUNT_GRID + appMat, grid).toUpper();
		res.daycount.convertFromString(strDaycount);
		AQLString strFixingCalendar = getCalibStaticDataValue(pCalibStaticData, lowerCcy + STATIC_DATA_KEY_CALIB_HW_FIXINGCALENDAR_GRID + appMat, grid);
		res.fixingCalendar.convertFromString(strFixingCalendar);
		AQLString strPaymentCalendar = getCalibStaticDataValue(pCalibStaticData, lowerCcy + STATIC_DATA_KEY_CALIB_HW_PAYMENTCALENDAR_GRID + appMat, grid);
		res.paymentCalendar.convertFromString(strPaymentCalendar);
		AQLString strSlidingRule = getCalibStaticDataValue(pCalibStaticData, lowerCcy + STATIC_DATA_KEY_CALIB_HW_SLIDINGRULE_GRID + appMat, grid).toUpper();
		res.slidingRule.convertFromString(strSlidingRule);
		res.forecastCurveName = STD;
		res.discountCurveName = STD;

	}
	else if (model.toUpper() == "IRSABR")
	{
		const AQLStringVector refNames = param.refName[0].toToken(':');
		size_t posUnderlying = 0;
		for (size_t i = 0; i < refNames.size(); ++i)
		{
			const AQLStringVector refNameElements = refNames[i].toToken('_');
			if (refNameElements.size() != 3)
			{
				throw AQLCoreInvalidData("Invalid curveSetID format.", __FILE__, __LINE__);
			}
			const AQLString tempUnderlying = AQLString(refNameElements[2]).toLower();
			if (tempUnderlying == param.targetCurveType)
			{
				posUnderlying = i;
				break;
			}
		}

		AQLString curveSetID = refNames[posUnderlying] + "_CurveSetID";
		AQLString curveSetID_Data = curveSetID + "_DATA";
		AQLObject curveEntity = objPool.getObject(curveSetID, ENCHKTYPE_ISDEFINED).get();
		res.curveID = dynamic_cast<const AQLDataString&>(curveEntity.getData(PRICING_DATA_CURVEID, ISDEFINED).get()).get();

		const AQLString &calibInfoName = refNames[posUnderlying];
		const AQLObject &calibInfo = objPool.getObject(calibInfoName, ENCHKTYPE_ISDEFINED).get();


		//get swaption convention id
		const AQLObject& swapconv = dynamic_cast<const AQLDataReference &>(calibInfo.getData(PRICING_DATA_SWAPTIONCONVID, ISNOTNULL).get()).get().get();
		AQLString swapConvID = dynamic_cast<const AQLDataString &>(swapconv.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();

		//set cap convention id
		const AQLObject& capconv = dynamic_cast<const AQLDataReference &>(calibInfo.getData(PRICING_DATA_CAPCONVID, ISNOTNULL).get()).get().get();
		AQLString capConvID = dynamic_cast<const AQLDataString &>(capconv.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();


		AQLString convID;
		if (tenor == AQLString("1M") || tenor == AQLString("3M") || tenor == AQLString("6M"))
		{
			res.forecastCurveName = dynamic_cast<const AQLDataString&>(curveEntity.getData(tenor + AQLString("LiborCurveName"), ISDEFINED).get()).get();
			convID = capConvID;
		}
		else
		{
			res.forecastCurveName = dynamic_cast<const AQLDataString&>(curveEntity.getData(AQLString("SwapRateCurveName"), ISDEFINED).get()).get();
			convID = swapConvID;
		}
		res.discountCurveName = dynamic_cast<const AQLDataString&>(curveEntity.getData(CALIBRATION_DATA_DISCOUNTCURVENAME, ISDEFINED).get()).get();


		const AQLObject& object = objPool.getObject(convID, ENCHKTYPE_ISDEFINED).get();
		const AQLDataHolder* dh = &object.getData(IR_CALIBRATION_DATA_FREQUENCY + AQLString("_") + tenor, NOCHECK);
		if (!dh->isDefined() || dh->isNull())
		{
			dh = &object.getData(IR_CALIBRATION_DATA_FREQUENCY, ISDEFINED);
		}
		res.frequency = dynamic_cast<const AQLDataString&>(dh->get()).get();
		res.spotLag =
			dynamic_cast<const AQLDataString&>(object.getData(CURVEINPUT_SPOTLAG, ISDEFINED).get()).get();
		res.daycount =
			dynamic_cast<const AQLPriceDataDayCount&>(object.getData(IR_CALIBRATION_DATA_DAYCOUNT, ISDEFINED).get());
		res.slidingRule =
			dynamic_cast<const AQLPriceDataSlidingRule&>(object.getData(CALIBRATION_DATA_SLIDINGRULE, ISDEFINED).get());
		res.paymentCalendar =
			dynamic_cast<const AQLPriceDataCalendar&>(object.getData(CALIBRATION_DATA_CALENDAR, ISDEFINED).get());
		res.fixingCalendar =
			dynamic_cast<const AQLPriceDataCalendar&>(object.getData(PRICING_DATA_FIXINGCALENDAR, ISDEFINED).get());

	}
	else
	{
		throw AQLCoreInvalidData("Model Name was not found in LMM, HWDD, IRSABR", __FILE__, __LINE__);
	}
	AQLMathYieldCurve &bYield = dynamic_cast<AQLMathYieldCurve &>(objPool.getObject(curveName, ENCHKTYPE_ISDEFINED).get());

	// get curve ID
	const AQLObject &bYieldData = bYield.getYieldData().get().get();
	res.curveID = dynamic_cast<const AQLDataString &>(bYieldData.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();

	return res;
}




/*!
	@brief get swap convention

	@param[in] model
	@param[in] currency
	@return swapition vol file name 

*/
AQLString
AQLMarketData::getVolFileName(const AQLString& model, const AQLString& currency, const AQLString& underlying)
{
	AQLStaticData* pCalibStaticData = &AQLCoreDataService::getStaticDataManager().getCalibStaticData();
	AQLString volFileName;
	AQLString currencyLower = currency;
	currencyLower.toLower();
	if (model == "HW")
	{
		volFileName = AQLMarketData::getNumFileName(pCalibStaticData->getStaticData(currencyLower.toLower() + STATIC_DATA_KEY_CALIB_SWAPTION_FILE));
	}
	else if (model == "LMM")
	{
		volFileName = AQLMarketData::getNumFileName(pCalibStaticData->getStaticData(currencyLower + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_VOL_FILE));
	}
	else if (model == "IRSABR")
	{
		const AQLStringVector underlyings = pCalibStaticData->getStaticData(currencyLower + STATIC_DATA_KEY_CALIB_IRSABR_UNDERLYING).toLower().toToken(':');

		volFileName = AQ_NO_DATA;
		for (size_t i = 0; i < underlyings.size(); ++i)
		{
			if (AQLString(underlying).toLower() == underlyings[i])
			{
				volFileName = AQLMarketData::getNumFileName(pCalibStaticData->getStaticData(currencyLower + STATIC_DATA_KEY_CALIB_SWAPTION_FILE + "." + underlyings[i]));
				break;
			}
		}
	}
	else
	{
		volFileName = AQ_NO_DATA;
	}

	return volFileName;
}



#endif
