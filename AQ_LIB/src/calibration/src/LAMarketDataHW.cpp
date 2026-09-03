#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif
//#define __DATAOUT__

#include <math.h>
#include "LAMarketDataHW.h"
#include "LADealUtils.h"
#include "LAMarketData.h"
#include "LACoreDataService.h"
#include "LADefinitions.h"
#include "LADefinitionsHW.h"
#include "LAStaticData.h"

using namespace std;

// constructor
/*!

*/
LAMarketDataHW::LAMarketDataHW(void)
{
}

// destructor
/*!

*/
LAMarketDataHW::~LAMarketDataHW(void)
{
}



/*!
    @brief get canonical libor grid

	@param[out] tenor
	@param[in] asofdate
	@param[in] daycout for tenor
	@param[in] max term
*/
void
LAMarketDataHW::getCanonicalGrid(DoubleArray &tenor, const AQLDate &asOfDate, const AQLPriceDataDayCount &dayCount, int max, bool isReflesh, const AQLString *p_freq_str)
{
	isReflesh;
	tenor.clear();

	AQLString str_tenor = LACoreDataService::getContext(CONTEXT_KEY_HW_TENOR);
	if (str_tenor != AQ_NO_DATA)
	{
		AQLDataDoubles data_tenor;
		data_tenor.convertFromString(str_tenor);
		tenor = data_tenor.get();
		return;
	}
	AQLString freq_str = "SA";
	AQLString freq = FREQ_SEMI_ANNUAL;
	if (p_freq_str)
	{
		freq_str = *p_freq_str;
	}

	if (freq_str == "QA")
	{
		freq = FREQ_QUARTERLY;
	}
	else if (freq_str == "BD")
	{
		freq = FREQ_BUSINESS_DAYS;
	}
	else if (freq_str == "WK")
	{
		freq = FREQ_WEEKLY;
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
	AQLMathDateCalculations::generateSchedule(start, end, freq, true, 0, 0, 0, dates, &sliding, &cal);
	
	DateVector exDates;
	//if (freq == FREQ_SEMI_ANNUAL)
	//{
	LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	AQLDataBool tmp;
	tmp.convertFromString(staticData.getStaticData(KEY_HW_GRID_ISEXTRATENORUSE));
	bool isExTenor = tmp.get();
	if (isExTenor)
	{
		AQLString exTenorStr = staticData.getStaticData(KEY_HW_GRID_EXTRATENOR);
		AQLStringVector exTenor = exTenorStr.toToken(MULTI_STATIC_DATA_DELIMITER);
		unsigned int exTSize = exTenor.size();
		exDates.resize(exTSize);
		for (unsigned int i = 0; i < exTSize; ++i)
		{
			exDates[i] = AQLMathDateCalculations::getDate(asOfDate, exTenor[i], sliding, &cal, true);
		}
		if (exDates[0] == start)
		{
			exDates.erase(exDates.begin());
		}
	}
	//}

	// merget extra dates
	DateVector dates_;
	const unsigned int dateSize = dates.size();
	const unsigned int exDateSize = exDates.size();
	unsigned int counter = 0;
	for (unsigned int i = 0; i < dateSize; ++i)
	{
		for (unsigned int j = counter; j < exDateSize; ++j)
		{
			if(exDates[counter] < dates[i])
			{
				if(find(dates.begin(), dates.end(), exDates[counter]) == dates.end())
				{
					dates_.push_back(exDates[counter++]);
				}
				else
				{
					counter++;
				}
			}
		}
		dates_.push_back(dates[i]);	
	}

	const unsigned int size = dates_.size();
	tenor.resize(size + 1, 0.0);

	for (unsigned int i = 1; i <= size; ++i)
	{
		tenor[i] = dayCount.getTerm(start, dates_[i - 1]);
	}

	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
    {
 		const AQLString fileSuffix = LACoreDataService::getContext(ARG_KEY_FILENUM);
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
	}
}



/*!
    @brief get Hull-White parameter (alpha, sigma)

	@param[in] fileName
	@param[out] time grid
	@param[out] alpha
	@param[out] sigma
*/
void
LAMarketDataHW::getHullWhiteParam(const AQLString &fileName, DoubleArray &t_grid, DoubleArray &alpha, DoubleArray &sigma)
{
	MAFileAccessor paramFile(fileName);
	AQLStringMatrix strMtx;
	paramFile.readAllData(MARKET_DATA_DELIMITER, strMtx);
	paramFile.close();

	if (strMtx.size() == 0 || strMtx[0].size() != 3) 
	{
		AQLString msg = "Invalid Hull-White parameter size";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	const unsigned int size = strMtx.size();
	
	t_grid.resize(size);
	alpha.resize(size);
	sigma.resize(size);

	for (unsigned int i = 0; i < size; i++)
	{
		t_grid[i] = strMtx[i][0].trimLeft().trimRight().getDoubleValue();
		alpha[i] = strMtx[i][1].trimLeft().trimRight().getDoubleValue();
		sigma[i] = strMtx[i][2].trimLeft().trimRight().getDoubleValue();
	}

	// check if alpha, sigma is constant
	if (size > 0) 
	{
		// alpha
		bool const_alpha = true;
		const double _alpha = alpha[0];
		for (unsigned int i = 1; i < size; i++)
		{
			if (alpha[i] != _alpha) 
			{
				const_alpha = false;
				break;
			}
		}

		// resize if all elements are same
		if (const_alpha) 
		{
			alpha.clear();
			alpha.resize(1,_alpha);
		}

		// sigma
		bool const_sigma = true;
		const double _sigma = sigma[0];
		for (unsigned int i = 1; i < size; i++)
		{
			if (sigma[i] != _sigma) 
			{
				const_sigma = false;
				break;
			}
		}

		// resize if all elements are same
		if (const_sigma) 
		{
			sigma.clear();
			sigma.resize(1,_sigma);
		}
	}
}
