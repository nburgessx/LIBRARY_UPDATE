#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include <math.h>
#include "LAMarketDataLMM.h"
#include "AQLDataInstance.h"
#include "AQLFunctionManager.h"
#include "AQLCoreTemplateType.h"
#include "AQLAlgorithm.h"
#include "AQLDataBasics.h"
#include "AQLBasic.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataCalendar.h"
#include "LAMathDateCalculations.h"
#include "LAMathPathEntity.h"
#include "LAMathVolatility.h"
#include "AQLPriceDataDayCount.h"
#include "LADefinitions.h"
#include "LADefinitionsLMM.h"
#include "LACoreDataService.h"
#include "LAStaticDataManager.h"
#include "LAFileAccessor.h"
#include "LAMathVolFuncLMM.h"
#include "LAMathVolFuncWave.h"
#include "LAMathVolFuncStructureBase.h"
#include "LADealUtils.h"
#include "LAMarketData.h"
#include "LAStaticData.h"

using namespace std;

// constructor
/*!

*/
LAMarketDataLMM::LAMarketDataLMM(void)
{
}

// destructor
/*!

*/
LAMarketDataLMM::~LAMarketDataLMM(void)
{
}

/*!
    @brief get canonical libor grid

	@param[out] tenor
	@param[out] tenor_30_360
	@param[out] deltatenor
	@param[out] extratenor flag
	@param[in] asofdate
	@param[in] daycout for tenor
	@param[in] max term
	@param[in] flag for wheather we read extra tenor from property
	
*/
void
LAMarketDataLMM::getCanonicalGrid(DoubleArray &tenor_30_360, DoubleArray &tenor, 
					    DoubleArray &deltatenor, BoolVector &extraflag, 
						const AQLDate &asOfDate, const AQLPriceDataDayCount &dayCount,
						const AQLString &freq, int max, AQLStringVector &exTenor, const bool isDataOut) 
{
	tenor.clear();
	tenor_30_360.clear();
	deltatenor.clear();
	extraflag.clear();

	AQLString str_tenor = LACoreDataService::getContext(CONTEXT_KEY_LMM_TENOR);
	AQLString str_tenor_30_360 = LACoreDataService::getContext(CONTEXT_KEY_LMM_30_360_TENOR);
	AQLString str_deltatenor = LACoreDataService::getContext(CONTEXT_KEY_LMM_DELTATENOR);
	if (str_tenor != AQ_NO_DATA && str_tenor_30_360 != AQ_NO_DATA && str_deltatenor != AQ_NO_DATA)
	{
		AQLDataDoubles data_tenor;
		data_tenor.convertFromString(str_tenor);
		AQLDataDoubles data_tenor_30_360;
		data_tenor_30_360.convertFromString(str_tenor_30_360);
		AQLDataDoubles data_deltatenor;
		data_deltatenor.convertFromString(str_deltatenor);

		tenor = data_tenor.get();
		tenor_30_360 = data_tenor_30_360.get();
		deltatenor = data_deltatenor.get();
		extraflag.resize(tenor.size(), false);

		return;
	}

	AQLString tmp_freq = freq;
	tmp_freq.toUpper();
	if (tmp_freq != FREQ_SEMI_ANNUAL && tmp_freq != FREQ_QUARTERLY)
	{
		AQLString msg = "Frequency = " + tmp_freq + " is not supported !";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	AQLDate start = asOfDate;
	AQLDate end = asOfDate;
	end.addYears(max);

	// sliding
	AQLPriceDataSlidingRule sliding;
	sliding.convertFromString(SLIDING_MOD_FOLLOWING);

	// calendar
	AQLPriceDataCalendar cal;
	cal.convertFromString(CITY_LnB);

	DateVector dates;
	LAMathDateCalculations::generateSchedule(start, end, freq, true, 0, 0, 0, dates, &sliding, &cal);
	
	DateVector exDates;
	unsigned int exTSize = exTenor.size();
	exDates.resize(exTSize);
	if (exTSize > 0)
	{
		for (unsigned int i = 0; i < exTSize; ++i)
		{
			// sliding
			AQLPriceDataSlidingRule sld_nochange;
			sld_nochange.convertFromString(SLIDING_NO_CHANGE);
			exDates[i] = LAMathDateCalculations::getDate(asOfDate, exTenor[i], sld_nochange, NULL, true);
		}
		if (exDates[0] == start)
		{
			exDates.erase(exDates.begin());
		}
	}

	// merge extra dates
	DateVector dates_;
	const unsigned int dateSize = dates.size();
	const unsigned int exDateSize = exDates.size();
	extraflag.resize(1, false);

	// check
	for (unsigned int i = 0; i < exDateSize; ++i)
	{
		if (exDates[i] > dates[0])
		{
			AQLString msg = "Extra dates must be below normal first date, exDate = " + exDates[i].stringWithFormat();
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
	}
	unsigned int counter = 0;
	for (unsigned int i = 0; i < dateSize; ++i)
	{
		for (unsigned int j = counter; j < exDateSize; ++j)
		{
			if(exDates[counter] < dates[i])
			{
				dates_.push_back(exDates[counter++]);
				extraflag.push_back(true);
			}
		}
		dates_.push_back(dates[i]);
		extraflag.push_back(false);
	}

	AQLPriceDataDayCount dayCount_ACT_360(ACT_360);
	AQLPriceDataDayCount dayCount_30_360(N30_360);

	unsigned int size = dates_.size();
	tenor.resize(size + 1, 0.0);
	tenor_30_360.resize(size + 1, 0.0);
	deltatenor.resize(size , 0.0);
	deltatenor[0] = dayCount_ACT_360.getTerm(start, dates_[0]);

	for (unsigned int i = 1; i <= size; ++i)
	{
		tenor[i] = dayCount.getTerm(start, dates_[i - 1]);
		tenor_30_360[i] = dayCount_30_360.getTerm(start, dates_[i - 1]);
		if (i != size)
		{
			deltatenor[i] = dayCount_ACT_360.getTerm(dates_[i - 1], dates_[i]);
		}
	}

	if (isDataOut)
    {
 		const AQLString fileSuffix = LACoreDataService::getContext(ARG_KEY_FILENUM) + "_" + freq;
		const AQLString dirName = LACoreDataService::getOutputDirectory(); 
		const AQLString tenorFileName = dirName + "tenor" + fileSuffix + ".csv";
		const AQLString tenor30_360FileName = dirName + "tenor30_360" + fileSuffix + ".csv";
		const AQLString deltaFileName = dirName + "deltatenor" + fileSuffix + ".csv";
		ifstream fin;
		ofstream fout;
		// tenor file
		fin.open(tenorFileName.getCString());
		if (!fin)
		{
			fout.open(tenorFileName.getCString());
			const int size = tenor.size();
			for (int i = 0; i < size; ++i)
			{
				AQLString outStr = AQLString(tenor[i]);
				fout << outStr.getCString() << "," << std::endl;
			}
			fout.close();
			fin.close();
			fin.clear();
		}
		else
		{
			tenor.clear();
			string line;
			while (getline(fin, line))
			{
				const char *c_line = line.c_str();
				tenor.push_back(AQLString(c_line).toToken(MARKET_DATA_DELIMITER)[0].trimLeft().trimRight().getDoubleValue());
			}
			fin.close();
			fin.clear();
		}
		// tenor 30/360 file
		fin.open(tenor30_360FileName.getCString());
		if (!fin)
		{
			fout.open(tenor30_360FileName.getCString());
			const int size = tenor_30_360.size();
			for (int i = 0; i < size; ++i)
			{
				AQLString outStr = AQLString(tenor_30_360[i]);
				fout << outStr.getCString() << "," << std::endl;
			}
			fout.close();
			fin.close();
			fin.clear();
		}
		else
		{
			tenor_30_360.clear();
			string line;
			while (getline(fin, line))
			{
				const char *c_line = line.c_str();
				tenor_30_360.push_back(AQLString(c_line).toToken(MARKET_DATA_DELIMITER)[0].trimLeft().trimRight().getDoubleValue());
			}
			fin.close();
			fin.clear();
		}

		// deltatenor
		fin.open(deltaFileName.getCString());
		if (!fin)
		{
			fout.open(deltaFileName.getCString());
			const int size = deltatenor.size();
			for (int i = 0; i < size; ++i)
			{
				AQLString outStr = AQLString(deltatenor[i]);
				fout << outStr.getCString() << "," << std::endl;
			}
			fout.close();
			fin.close();
			fin.clear();
		}
		else
		{
			deltatenor.clear();
			string line;
			while (getline(fin, line))
			{
				const char *c_line = line.c_str();
				deltatenor.push_back(AQLString(c_line).toToken(MARKET_DATA_DELIMITER)[0].trimLeft().trimRight().getDoubleValue());
			}
			fin.close();
			fin.clear();
		}
    }
}


// 
/*!
    @brief get skew val

	@param[in] currency
	@return skew
*/
double
LAMarketDataLMM::getSkew(const AQLString &currency)
{
	AQLString key_ccy = currency;
	key_ccy.toLower();

	LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	// read skew
	AQLString skew_fPath = staticData.getStaticData(key_ccy + STATIC_DATA_KEY_LMM_SKEW_CROSS_FILE);

	if (MADealUtils::getSDECurrencys().size() == 1)
	{
		skew_fPath = staticData.getStaticData(key_ccy + STATIC_DATA_KEY_LMM_SKEW_SINGLE_FILE);
	}

	MAFileAccessor file(LAMarketData::getNumFileName(skew_fPath));
	AQLStringMatrix skewData;
	file.readAllData(MARKET_DATA_DELIMITER, skewData);
	file.close();

	// data format check
	if (skewData.size() != 1 || skewData[0].size() != 1)
	{
		AQLString msg = "LMM skew file, format is wrong. file = " + LAMarketData::getNumFileName(skew_fPath);
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	return  skewData[0][0].getDoubleValue();
}


// 
/*!
    @brief get const shift val

	@param[in] currency
	@return skew
*/
double
LAMarketDataLMM::getConstShift(const AQLString &currency)
{
	AQLString key_ccy = currency;
	key_ccy.toLower();

	LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	// read constShift
	AQLString constShift_fPath = staticData.getStaticData(key_ccy + STATIC_DATA_KEY_LMM_CONSTSHIFT_CROSS_FILE);

	if (MADealUtils::getSDECurrencys().size() == 1)
	{
		constShift_fPath = staticData.getStaticData(key_ccy + STATIC_DATA_KEY_LMM_CONSTSHIFT_SINGLE_FILE);
	}

	MAFileAccessor file(LAMarketData::getNumFileName(constShift_fPath));
	AQLStringMatrix constShiftData;
	file.readAllData(MARKET_DATA_DELIMITER, constShiftData);
	file.close();

	// data format check
	if (constShiftData.size() != 1 || constShiftData[0].size() != 1)
	{
		AQLString msg = "LMM con file, format is wrong. file = " + LAMarketData::getNumFileName(constShift_fPath);
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	return  constShiftData[0][0].getDoubleValue();
}


// 
/*!
    @brief get maxterm val

	@param[in] currency
	@return maxterm
*/
double
LAMarketDataLMM::getMaxTerm(const AQLString &currency)
{
	AQLString key_ccy = currency;
	key_ccy.toLower();

	LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	// read maxTerm
	AQLString maxterm_fPath = staticData.getStaticData(key_ccy + STATIC_DATA_KEY_LMM_MAXTERM_FILE);

	MAFileAccessor file(LAMarketData::getNumFileName(maxterm_fPath));
	AQLStringMatrix maxtermData;
	file.readAllData(MARKET_DATA_DELIMITER, maxtermData);
	file.close();

	// data format check
	if (maxtermData.size() != 1 || maxtermData[0].size() != 1)
	{
		AQLString msg = "LMM max term file, format is wrong. file = " + LAMarketData::getNumFileName(maxterm_fPath);
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	return  maxtermData[0][0].getDoubleValue();
}

//
/*!
    @brief get extra tenor for simulation grid

	@return ExTenor
*/
AQLStringVector
LAMarketDataLMM::getGridExTenor()
{
	LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	AQLString isExTenorStr = staticData.getStaticData(KEY_LMM_GRID_ISEXTRATENORUSE);
	if (isExTenorStr == AQ_NO_DATA)
	{
		isExTenorStr = "FALSE";
	}
	AQLDataBool tmp;
	tmp.convertFromString(isExTenorStr);

	AQLStringVector ret(0);
	if(tmp.get())
	{
		AQLString exTenorStr = staticData.getStaticData(KEY_LMM_GRID_EXTRATENOR);
		if (exTenorStr == AQ_NO_DATA)
		{
			AQLString msg = AQLString(KEY_LMM_GRID_EXTRATENOR) + " must be set ";
			msg += AQLString("when ") + KEY_LMM_GRID_ISEXTRATENORUSE + " = TRUE in property file!";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		exTenorStr.toUpper();
		ret = exTenorStr.toToken(MULTI_STATIC_DATA_DELIMITER);
	}

	//check if all of extra tenor for canonical grid are contained in return variable
	AQLStringVector exTenorForCanonicalGrid = LAMarketDataLMM::getCanonicalGridExTenor();
	for (unsigned int i = 0; i < exTenorForCanonicalGrid.size(); ++i)
	{
		if(std::find(ret.begin(), ret.end(), exTenorForCanonicalGrid[i]) == ret.end())
		{
			if((exTenorForCanonicalGrid[i] == "3M" && LACoreDataService::getContext(ARG_KEY_SIMFREQ) == "QA") ||
			   (exTenorForCanonicalGrid[i] == "6M" && LACoreDataService::getContext(ARG_KEY_SIMFREQ) == "SA"))
			{
				continue;
			}
			AQLString msg = AQLString(KEY_LMM_GRID_EXTRATENOR) + " must contain " + exTenorForCanonicalGrid[i] + "!";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
	}

	return ret;
}

//
/*!
    @brief get extra tenor for canonical grid

	@return ExTenor
*/
AQLStringVector
LAMarketDataLMM::getCanonicalGridExTenor()
{
	LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	AQLString isExTenorStr = staticData.getStaticData(KEY_LMM_CANONICALGRID_ISEXTRATENORUSE);
	if (isExTenorStr == AQ_NO_DATA)
	{
		isExTenorStr = "FALSE";
	}
	AQLDataBool tmp;
	tmp.convertFromString(isExTenorStr);
	
	AQLStringVector ret(0);
	if(tmp.get())
	{
		AQLString exTenorStr = staticData.getStaticData(KEY_LMM_CANONICALGRID_EXTRATENOR);
		if (exTenorStr == AQ_NO_DATA)
		{
			AQLString msg = AQLString(KEY_LMM_CANONICALGRID_EXTRATENOR) + " must be set ";
			msg += AQLString("when ") + KEY_LMM_CANONICALGRID_ISEXTRATENORUSE + " = TRUE in property file!";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		exTenorStr.toUpper();
		ret = exTenorStr.toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	return ret;
}
