//
//  AQLCurveMarketDataHelpers.cpp
//  This file was previous called YieldCurveMarketUti1s.cpp and before that AQLMarketData.cpp
//

#include "AQLCurveMarketDataHelpers.h"
#include "AQLDataReference.h"
#include "AQLStaticData.h"
#include "AQLLinearInterpolation.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLAlgorithm.h"
#include "AQLDefinitionsCalibration.h"
#include "CurveInstruments.h"

using namespace std;

// constructor
/*!

*/
AQLCurveMarketDataHelpers::AQLCurveMarketDataHelpers(void)
{
}

// destructor
/*!

*/
AQLCurveMarketDataHelpers::~AQLCurveMarketDataHelpers(void)
{
}


/*!
    @brief reset market data use Libor

	calc libor rate from df
	and set libor rate to market data 

	@param[in] yield curve
	@param[in] ccy
*/
void 
AQLCurveMarketDataHelpers::resetMarketDataUsingLibor(CurveCalibrationData &curve, const AQLString &ccy, const AQLString *pCurveType)
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

	AQ_THROW_IF( asOfDate > spotDate, "AsofDate > spotDate, cannnot calc. " );

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
				termStr = etrading::changeFRATermFormat(termStr);
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
				AQLDate date = etrading::AQLDateHelpers::getDate(spotDate, termStr, sliding, &cal, true);
					
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


/*!
    @brief sort market data

	@param[out] ypro
*/
void AQLCurveMarketDataHelpers::sortMarketData(CurveCalibrationData &ypro)
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
	sort(data_libor.begin(), data_libor.end(), InstrumentComp());
	sort(data_swap.begin(), data_swap.end(), InstrumentComp());
	sort(data_future.begin(), data_future.end(), InstrumentComp());
	sort(data_basis.begin(), data_basis.end(), InstrumentComp());
	sort(data_fra3m.begin(), data_fra3m.end(), InstrumentComp());
	sort(data_fra6m.begin(), data_fra6m.end(), InstrumentComp());

	AQ_THROW_IF( data_on.empty() || data_tn.empty(), "ON or TN is not set." );
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

void AQLCurveMarketDataHelpers::restoreSwapRateFromLibor(CurveCalibrationData &curve, const map<AQLString, double> &sRateMap, const AQLString &ccy, const AQLString *pCurveType)
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

	AQ_THROW_IF( asOfDate > spotDate, "AsofDate > spotDate, cannnot calc. " );

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


