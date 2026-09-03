//
//  CurveInstruments.cpp
//  This file was previously called YieldCurveBuildingUtilities.cpp
//

// Etrading Headers
#include "CurveInstruments.h"
#include "CurveCalibrationData.h"
#include "CurveCalibration.h"

// External Headers
#include <boost/format.hpp>
#include <map>

// LA Headers - Put these last so that legacy defines don't conflict
#include "LACurveForwardRateHelpers.h"
#include "LADateScheduleHelpers.h"
#include "LAStaticData.h"
#include "AQLFunctionUtilities.h"
#include "LAMarketData.h"
#include "LACoreDataService.h"
#include "AQLInterpolationBase.h"
#include "AQLLinearInterpolation.h"
#include "AQLAlgorithm.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataCalendar.h"
#include "AQLBasic.h"
#include "AQLMatrix.h"
#include "LADefinitions.h"		// Put legacy defines last
#include "AQLMathDefine.h"       // Put legacy defines last
#include "CurveUtilities.h"		// accrualPeriod method
#include "CurveValidation.h"
#include "Solvers.h"			// Newton-Raphson Minimizer

// Required for Logging and Debugging
#include "RecordMacros.h"

// CurveParameters to replace legacy define statements
using namespace etrading::curveParameters;

// Global Parameters
const int RATE_PRIORITY_PROXIMITY_DAY_TOLERANCE = 20;


namespace etrading
{
	// Get and synch the termsDFMatrix with the paymentDatesAsTerms and discountFactor member variables
	DoubleMatrix DiscountFactors::termsDFMatrix()
	{
		AQ_REQUIRE( paymentDatesAsTerms_.size() == discountFactors_.size(), "Invalid Discount Factor Results: Inconsistent number of payment dates and discount factors" )
		DoubleMatrix termsDFMatrix_(2);
		termsDFMatrix_[0] = paymentDatesAsTerms_;
		termsDFMatrix_[1] = discountFactors_;
		return termsDFMatrix_;
	}

	// Get and synch the termsDFMatrix with the paymentDatesAsTerms and discountFactor member variables
	void DiscountFactors::termsDFMatrix( const DoubleMatrix & termsDFMatrix )
	{
		AQ_REQUIRE( termsDFMatrix.size() == 2, "Invalid Discount Factor Data: termsDFMatrix must have 2 columns with terms & discount factors" )
		termsDFMatrix_ = termsDFMatrix;
		paymentDatesAsTerms_ = termsDFMatrix[0];
		discountFactors_ = termsDFMatrix[1];
	}

	/* @brief			Group all input FRA market data into a file to be used later in yield curve calibration
	*  @param [in]		fraFileName		Name of FRA market data file
	*  @param [in]		fraRates		FRA market quotes
	*  @param [in]		areSwapsForwardStarting		Will FRA instruments have user-specified start and end dates?
	*  @param [inout]	useGrid_FRA		A string used for market data validation
	*  @return			File carrying FRA market data
	*/
	AQLString buildFRAMarketDataFile(const AQLString& fraFileName,
		const AQLStringMatrix& fraRates,
		bool areSwapsForwardStarting,
		AQLString& useGrid_FRA)
	{
		AQLString fraMarketStream;
		for (size_t i = 0; i < fraRates.size(); i++)
		{
			fraMarketStream += fraRates[i][0];
			double frarate = fraRates[i][1].getDoubleValue() * 100.0;
			fraMarketStream += "," + AQLString(frarate);

			if (fraRates[i].size() >= 5)
			{
				AQLString isDate_str = fraRates[i][2];
				isDate_str.toUpper();
				fraMarketStream += "," + isDate_str;

				if (isDate_str == "TRUE")
				{
					const AQLDate& startdate = LADateScheduleHelpers::getLADate(fraRates[i][3]);
					AQLString startdate_str = startdate.stringWithFormat("YYYYMMDD");
					fraMarketStream += "," + startdate_str;
					const AQLDate& enddate = LADateScheduleHelpers::getLADate(fraRates[i][4]);
					AQLString enddate_str = enddate.stringWithFormat("YYYYMMDD");
					fraMarketStream += "," + enddate_str;
				}
				else if (isDate_str == "FALSE")
				{
					AQLString startterm_str = fraRates[i][3];
					fraMarketStream += "," + startterm_str.toUpper();
					AQLString tenor_str = fraRates[i][4];
					fraMarketStream += "," + tenor_str.toUpper();
				}
				else
				{
					throw AQLCoreInvalidData("#Error: Third column of the FRA market data block only takes boolean value", __FILE__, __LINE__);
				}

				// set use grid
				if (fraRates[i].size() == 6)
				{
					AQLString useGridFrag = fraRates[i][5];
					upper(useGridFrag);
					if (useGridFrag == "TRUE")
					{
						useGrid_FRA += fraRates[i][0] + ":";
					}
					else
					{
						useGrid_FRA += "NONE:";
					}
				}
			}
			// set use grid
			else if (fraRates[i].size() == 3)
			{
				AQLString useGridFrag = fraRates[i][2]; upper(useGridFrag);
				if (useGridFrag == "TRUE")
				{
					useGrid_FRA += fraRates[i][0] + ":";
				}
				else
				{
					useGrid_FRA += "NONE:";
				}
			}
			else if (fraRates[i].size() == 4)
			{
				throw AQLCoreInvalidData("#Error: FRA market data block does not accept four columns", __FILE__, __LINE__);
			}

			fraMarketStream += "\n";
		}

		return fraMarketStream;
	}


	/* @brief			Populate FRA data into object pool
	*  @param [out]		mpStaticData				Property manager object
	*  @param [out]		curveCalibrationData		CurveCalibrationData object
	*  @param [out]		refData						Reference data
	*  @param [out]		objPool						Reference object to the object pool
	*  @param [in]		currency					Curve currency
	*  @param [in]		marketName					Market name of curve
	*  @param [in]		yieldDataName				Yield data name
	*  @param [in]		staticDataSuffix			Suffix used with property manager
	*  @param [in]		suffix_data					Suffix used with object pool
	*  @param [in]		isAudExtra					Australian curve flag
	*  @param [in]		isSpotUse					Use use given spot date?
	*  @param [in]		areSwapsForwardStarting		Instruments have user specified start dates?
	*  @param [in]		asOfDate					As of date of curve
	*  @param [in]		isBasisCurve				Is the current curve a basis curve?
	*/
	void populateFRADataToEntityPool(LAStaticData * mpStaticData,
									 CurveCalibrationData &curveCalibrationData,
									 AQLString& refData,
									 AQLObjectPool& objPool,
									 const AQLString& currency,
									 const AQLString& marketName,
									 const AQLString& yieldDataName,
									 const AQLString& staticDataSuffix,
									 const AQLString& suffix_data,
									 bool isAudExtra,
									 bool isSpotUse,
									 bool areSwapsForwardStarting,
									 const AQLDate &asOfDate,
									 bool isBasisCurve)
	{
		bool isFRAUse = false;
		AQLString isFRAUse_str = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFRAUSE + staticDataSuffix);
		AQLDataBool tmpAttrB;
		if (isFRAUse_str != AQ_NO_DATA)
		{
			tmpAttrB.convertFromString(isFRAUse_str);
			isFRAUse = tmpAttrB.get();
		}

		/* Handle isFRAUse flag:
		*  Some care is needed here since some currencies (such as AUD) can have TWO swap curves.
		*  If the curveType is SWAP, *and* there is no curve suffix (i.e. we have a STD curve),
		*  then use the simple data flag stored in curveCalibrationData.
		*  Otherwise, for non-STD curves use the full object pool data IR_CALIBRATION_DATA_ISFRAUSE plus suffix.
		*  This prevents the settings of two or more swap curves from overwriting each other.
		*/
		if ((marketName == AQ_SWAP) && (suffix_data == ""))
		{
			curveCalibrationData.getIsFRAUse().set(isFRAUse);
		}
		else
		{
			curveCalibrationData.AQLObject::remove(IR_CALIBRATION_DATA_ISFRAUSE + suffix_data);
			curveCalibrationData.AQLObject::add(IR_CALIBRATION_DATA_ISFRAUSE + suffix_data, new AQLDataBool(isFRAUse));
		}
		if (isAudExtra && isFRAUse) throw AQLCoreInvalidData("We can not set AUD extra and FRA use at a same time!", __FILE__, __LINE__);

		if (isFRAUse)
		{
			// use grid
			AQLStringVector fraUseGrid;
			AQLString tmpFraUseGrid = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_USEGRID + staticDataSuffix).toUpper();
			if (tmpFraUseGrid != AQ_NO_DATA)
			{
				fraUseGrid = tmpFraUseGrid.toToken(':');
			}

			// get market rate
			AQLString fraFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_FILE + staticDataSuffix);
			if (fraFileName == AQ_NO_DATA)
			{
				throw AQLCoreInvalidData("No FRA File", __FILE__, __LINE__);
			}
			MAFileAccessor fraFile(LAMarketData::getNumFileName(fraFileName));
			AQLStringMatrix fraDataMtx;
			fraFile.readAllData(MARKET_DATA_DELIMITER, fraDataMtx);
			fraFile.close();
			const int fraSize = fraDataMtx.size();

			// get cal
			AQLString calFRAStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_CALENDAR + staticDataSuffix);
			AQLPriceDataCalendar calFRA;
			calFRA.convertFromString(calFRAStr);

			// get spot date
			AQLDate spotDateFRA;
			if (isSpotUse)
			{
				spotDateFRA.setDate(mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_SPOTDATE + staticDataSuffix).getCString());
			}
			else
			{
				spotDateFRA = calFRA.getBusinessDay(asOfDate, mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_RESETLAG + staticDataSuffix).getIntValue());
			}

			// get daycount
			AQLString daycFRAStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_DAYCOUNT + staticDataSuffix).toUpper();

			// get sliding
			AQLString slidingFRAStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_SLIDINGRULE + staticDataSuffix).toUpper();

			// get applyTension
			bool applyTensionFRAs = false;
			AQLString applyTensionFRAsStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_APPLYTENSION + staticDataSuffix).toUpper();
			if (applyTensionFRAsStr != AQ_NO_DATA)
			{
				AQLDataBool tmpApplyTensionFRAs;
				tmpApplyTensionFRAs.convertFromString(applyTensionFRAsStr);
				applyTensionFRAs = tmpApplyTensionFRAs.get();
			}

			// get tensionGap
			int tensionGapFRAs = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_TENSIONGAP + staticDataSuffix).getIntValue();

			// Curve controls
			bool includeSwapsBeforeMPCSwaps = true;
			if (isBasisCurve)
			{
				includeSwapsBeforeMPCSwaps = false;
			}
			else
			{
				AQLString strSmoothShortEnd = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_SMOOTHSHORTEND + staticDataSuffix);
				if (strSmoothShortEnd != AQ_NO_DATA)
				{
					AQLDataBool tmpSmoothShortEnd;
					tmpSmoothShortEnd.convertFromString(strSmoothShortEnd);
					includeSwapsBeforeMPCSwaps = tmpSmoothShortEnd.get();
				}
			}

			//Default to Contiguous for fra
			AQLString fraSerialCalcType = etrading::toString(etrading::CONTIGUOUS_FUTURES);
			
			// Alias Method
			auto dh = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_SERIAL_CALC_TYPE + staticDataSuffix,
												  currency + STATIC_DATA_KEY_YIELD_FRA_INSTRUMENT_TYPE + staticDataSuffix);
			if (dh != AQ_NO_DATA)
			{
				fraSerialCalcType = dh.getCString();
			}

			// get eomroll
			bool isEOMRollFRA = false;
			AQLString strEOMRollFRA = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_ISEOMROLL + staticDataSuffix).toUpper();
			if (strEOMRollFRA != AQ_NO_DATA)
			{
				AQLDataBool tmpIsEOMRoll;
				tmpIsEOMRoll.convertFromString(strEOMRollFRA);
				isEOMRollFRA = tmpIsEOMRoll.get();
			}
			if (isEOMRollFRA)
			{
				AQLString strEOMDay = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_EOMDAY + staticDataSuffix).toUpper();
				if (strEOMDay != AQ_NO_DATA)
				{
					if (spotDateFRA.dayOfMonth() != strEOMDay.getIntValue())
					{
						isEOMRollFRA = false;
					}
				}
				else
				{
					AQLDate eomDate = calFRA.getEOMDay(spotDateFRA);
					if (spotDateFRA != eomDate)
					{
						isEOMRollFRA = false;
					}
				}
			}

			for (int i = 0; i < fraSize; ++i)
			{
				AQLString term = fraDataMtx[i][0].toUpper();
				double rate = fraDataMtx[i][1].getDoubleValue() / 100.0;

				//set FRA object
				AQLObject *mktDataFRA = NULL;
				AQLString nameFRA = yieldDataName + AQ_FRA + AQLString("_") + AQLString(i) + suffix_data;
				const AQLObjectHolder ehfra = objPool.getObject(nameFRA);
				if (!ehfra.isDefined())
				{
					mktDataFRA = new AQLObject();
					objPool.set(nameFRA, mktDataFRA);
				}
				else
				{
					objPool.getObject(nameFRA).get().clear();
					mktDataFRA = &objPool.getObject(nameFRA).get();
				}
				refData += nameFRA + ":";
				mktDataFRA->add(CALIBRATION_DATA_NAME, new AQLDataString(nameFRA));

				if (fraDataMtx[i].size() >= 5)
				{
					mktDataFRA->add(PRICING_DATA_ISFWDSWAP, new AQLDataBool(true));

					AQLString isDateStr = fraDataMtx[i][2].toUpper();
					if (isDateStr != "TRUE" && isDateStr != "FALSE")
					{
						throw AQLCoreInvalidData("#Error: Third column of the FRA market data block only takes boolean value", __FILE__, __LINE__);
					}

					bool isDate = isDateStr == "TRUE";
					mktDataFRA->add(PRICING_DATA_ISDATE, new AQLDataBool(isDate));
					if (isDate)
					{
						const AQLDate startDate = AQLDate(fraDataMtx[i][3].getCString());
						mktDataFRA->add(PRICING_DATA_STARTDATE, new AQLDataDate(startDate));
						const AQLDate endDate = AQLDate(fraDataMtx[i][4].getCString());
						mktDataFRA->add(PRICING_DATA_ENDDATE, new AQLDataDate(endDate));
					}
					else
					{
						const AQLString startTerm = fraDataMtx[i][3].toUpper();
						mktDataFRA->add(PRICING_DATA_STARTTERM, new AQLDataString(startTerm));
						const AQLString tenor = fraDataMtx[i][4].toUpper();
						mktDataFRA->add(PRICING_DATA_TENOR, new AQLDataString(tenor));
					}

					mktDataFRA->add(IR_CALIBRATION_DATA_INSTRUMENTSUBTYPE, new AQLDataString()).convertFromString(IMMFRA);
				}
				mktDataFRA->add(IR_CALIBRATION_DATA_TERM, new AQLDataString(term));
				mktDataFRA->add(CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar(calFRA));
				mktDataFRA->add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(slidingFRAStr);
				mktDataFRA->add(IR_CALIBRATION_DATA_SPOTDATE, new AQLDataDate(spotDateFRA));
				mktDataFRA->add(CALIBRATION_DATA_RATE, new AQLDataDouble(rate));
				mktDataFRA->add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(daycFRAStr);
				mktDataFRA->add(IR_CALIBRATION_DATA_APPLYTENSION, new AQLDataBool(applyTensionFRAs));
				mktDataFRA->add(IR_CALIBRATION_DATA_TENSIONGAP, new AQLDataInt(tensionGapFRAs));
				mktDataFRA->add(IR_CALIBRATION_DATA_ISEOMROLL, new AQLDataBool(isEOMRollFRA));
				//grid use
				if (fraUseGrid.size() != 0 && find(fraUseGrid.begin(), fraUseGrid.end(), term) == fraUseGrid.end())
				{
					mktDataFRA->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool(false));
				}
				else
				{
					mktDataFRA->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool(true));
				}
				//set yield type
				mktDataFRA->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(AQ_FRA);
				// smooth short end of curve
				mktDataFRA->add(IR_CALIBRATION_DATA_SMOOTHSHORTEND, new AQLDataBool(includeSwapsBeforeMPCSwaps));
				// serial calc type
				mktDataFRA->add(IR_CALIBRATION_DATA_FRAFUTURE_SERIAL_CALC_TYPE, new AQLDataString(fraSerialCalcType));
			}
		}
	}


	/* @brief			Insert artificial tension between Future/FRA points
	*  @param [out]		dfResults							dfResults
	*  @param [out]		forwardRatesVector					Forward rates resulted from tension
	*  @param [out]		forwardTermsMatrix					Dates on which forward rates have been computed
	*  @param [out]		discountFactorInterpolationTable	Discount factor interpolation object
	*  @param [in]		stateVariable				        stateVariable
	*  @param [in]		asOfDate							Yield curve asOfDate
	*  @param [in]		spotDate							Yield curve spot date
	*  @param [in]		startDate							Start date of the current Future/FRA
	*  @param [in]		endDate								End date of the current Future/FRA
	*  @param [in]		forwardRate							Forward rate of the current Future/FRA
	*  @param [in]		isFirstDataPoint					Is this the first Future/FRA?
	*  @param [in]		termsToDateDayCount					Date count convention from spot date to current date, usually ACT/365
	*  @param [in]		instrumentDayCount					Date count convention between instrument dates
	*  @param [in]		tensionGap							Gap in nummber of days between artificial forward instruments
	*  @param [in]		instrumentData						Object incapsulating Future/FRA instrument data
	*  @param [in]		cutoffDate							When tension should cease to be applied
	*  @param [in]		implyForwards						Imply forward rates from instruments?
	*/
	void insertSyntheticTensionPoints( DiscountFactors & dfResults,
									   DoubleArray& forwardRatesVector,
									   DoubleMatrix& forwardTermsMatrix,
									   AQLInterpolationBase* discountFactorInterpolationTable,
									   const StateVariableEnum& stateVariable,
									   const AQLDate & asOfDate,
									   const AQLDate& spotDate,
									   const AQLDate& startDate,
									   const AQLDate& endDate,
									   const double& forwardRate,
									   const bool& isFirstDataPoint,
									   const AQLPriceDataDayCount& termsToDateDaycount,
									   const AQLPriceDataDayCount& instrumentDaycount,
									   const unsigned int& tensionGap,
									   tensionMarketData& instrumentData,
									   const AQLDate& cutoffDate,
									   bool implyForwards )

	{
		// Collect Data for front and end Forwards in first iteration of the Forwards loop
		if (!isFirstDataPoint)
		{
			// Update Back Forward rate and Corresponding Start and End-Dates
			instrumentData.backForwardRate = forwardRate;

			instrumentData.backStartDate = startDate;
			instrumentData.backEndDate = endDate;

			//instrumentData.backStartDate.addDays( -1 * tensionGap );
			//instrumentData.backEndDate.addDays( -1 * tensionGap );

			instrumentData.backStartTerm = termsToDateDaycount.getTerm(spotDate, startDate);
			instrumentData.backEndTerm = termsToDateDaycount.getTerm(spotDate, endDate);

			// Generate Synthetic Start- and End-Date
			AQLDate syntheticStartDate = instrumentData.frontStartDate;
			syntheticStartDate.addDays(tensionGap);

			AQLDate syntheticEndDate = instrumentData.frontEndDate;
			syntheticEndDate.addDays(tensionGap);

			RateConvention rateConvention = LACurvePricingObject::setRC(AQ_SIMPLE);
			AQLPriceDataConvention discountConvention(instrumentDaycount.getDayCount(), rateConvention);
			const etrading::DayCountEnum accrualDaycount = instrumentDaycount.dayCountEnum();

			// Linear Interpolate on Forward Rates
			const double linearInterpIntercept	= instrumentData.frontForwardRate;
			const double dRate					= instrumentData.backForwardRate - instrumentData.frontForwardRate;        // dRate = Change in Rate
			const double dTerm					= instrumentData.backStartTerm - instrumentData.frontStartTerm;            // dTerm = Change in Term
			const double linearInterpSlope		= (dTerm == 0) ? 0 : dRate / dTerm;
			
			//
			// Synthetic points are inserted both as forwards and as pairs of discount factors corresponding start and end discount factors for the forward period.
			// Pairs of discount factors must not overlap or clash. If consecutive futures have a gap, daily synthetic discount factor pairs will overlap and this
			// is not allowed. Hence the below while loop restriction "syntheticStartDate < instrumentData.frontEndDate"
			//

			// Insert Synthetic Tension Points
			while (syntheticStartDate < instrumentData.backStartDate && syntheticEndDate < instrumentData.backEndDate && syntheticStartDate < instrumentData.frontEndDate)
			{
				// Must ensure new synthetic points don't overlap into the next existing Forward or Adjacent Instrument Group e.g. Swaps
				// Note: cutoffDate is optional and set to AQLDate() by default
				if (cutoffDate != AQLDate())
				{
					if (syntheticEndDate >= cutoffDate) break;
				}

				double syntheticStartTerm = termsToDateDaycount.getTerm(spotDate, syntheticStartDate);
				double syntheticEndTerm = termsToDateDaycount.getTerm(spotDate, syntheticEndDate);

				// Interest Rate Convention - Controls instrument daycount and interest rate compounding conventions e.g. Simple Interest Act/Act.
				RateConvention rateCompoundingMethod = LACurvePricingObject::setRC(AQ_SIMPLE);
				AQLPriceDataConvention discFactConvention(instrumentDaycount.getDayCount(), rateCompoundingMethod);

				// Linear interpolate existing FRAs to imply an artificial / synthetic FRA rate
				double linearInterpSlopeCoefft = (syntheticStartTerm - instrumentData.frontStartTerm);
				double syntheticForwardRate = linearInterpIntercept + (linearInterpSlopeCoefft * linearInterpSlope);

				// *** IMPORTANT NOTE ***
				// Interpolation State Variable must be carefully managed here
				double syntheticStartDF = etrading::getInterpolatedDiscountfactor( *discountFactorInterpolationTable,
																				   syntheticStartTerm,
																				   stateVariable,
																				   asOfDate,
																				   accrualDaycount,
																				   SIMPLE_COMPOUNDING );

				double syntheticEndDF = syntheticStartDF * discFactConvention.getDF(syntheticForwardRate, syntheticStartDate, syntheticEndDate);

				// Update Discount Factors for Start and End of FRA Period
				insertDFData(dfResults, syntheticStartDF, syntheticStartTerm, syntheticStartDate);
				insertDFData(dfResults, syntheticEndDF, syntheticEndTerm, syntheticEndDate);

				// At the time of writing, basis curve doesn't imply forward rates and only works in DFs.
				if (implyForwards)
				{
					insertForwardRateData(forwardTermsMatrix, forwardRatesVector, syntheticStartTerm, syntheticEndTerm, syntheticForwardRate);
				}

				// Update the next synthetic FRA start- and end date
				syntheticStartDate.addDays(tensionGap);
				syntheticEndDate.addDays(tensionGap);
			}
		}

		// Update the front Forward rate, start and end date for the next group of synthetic Forward points
		instrumentData.frontForwardRate = forwardRate;
		instrumentData.frontStartDate = startDate;
		instrumentData.frontEndDate = endDate;
		instrumentData.frontStartTerm = termsToDateDaycount.getTerm(spotDate, startDate);
		instrumentData.frontEndTerm = termsToDateDaycount.getTerm(spotDate, endDate);
	}

	/*
		@brief insert df, term and date into df data

		@param[out]		dfTerms			Discount factor terms (year fractions)
		@param[out]		dfValues		Discount factor values
		@param[out]		dfDates			Discount factor dates
		@param[in]		df_insert		The discount factor being inserted
		@param[in]		term_insert		Date fraction from spot/as-of date to the discount factor date
		@param[in]		date_insert		Discount factor date
		@param[out]		yields			Yields
		@param[in]		yield_insert	Yield to be insert
		@param[in]		overrideIfDatesClash	Override DF(DiscountFactor)/Yields if dates clashed
	*/
	void insertDFData(DoubleVector& dfTerms, DoubleVector& dfValues, DateVector& dfDates, double df_insert, double term_insert, const AQLDate& date_insert, DoubleVector& yields, const double yield_insert, const bool overrideIfDatesClash)
	{

		if (dfTerms.size() != dfDates.size())
		{
			throw AQLCoreInvalidData("Size of DF data and Size of dates must be same!", __FILE__, __LINE__);
		}

		if (dfDates.back() < date_insert)
		{
			dfDates.push_back(date_insert);
			dfTerms.push_back(term_insert);
			dfValues.push_back(df_insert);
			if (!std::isnan(yield_insert))
			{
				yields.push_back(yield_insert);
			}
		}
		else
		{
			for (size_t pos = 0; pos < dfDates.size(); pos++)
			{

				if (dfDates[pos] == date_insert)
				{
					if (overrideIfDatesClash)
					{
						dfValues[pos] = df_insert;
						if (!std::isnan(yield_insert))
						{
							yields[pos] = yield_insert;
						}
					}
					pos++;
					break;
				}
				else if (dfDates[pos] > date_insert)
				{
					dfDates.insert(dfDates.begin() + pos, date_insert);
					dfTerms.insert(dfTerms.begin() + pos, term_insert);
					dfValues.insert(dfValues.begin() + pos, df_insert);
					if (!std::isnan(yield_insert))
					{
						yields.insert(yields.begin() + pos, yield_insert);
					}
					pos++;
					break;
				}
			}
		}
	}

	// Update dfResults, insert df, term and date info data into the dfResults
	void insertDFData( DiscountFactors & dfResults, double df_insert, double term_insert, const AQLDate& date_insert, const bool overrideIfDatesClash )
	{
		DoubleVector dummyYields;
		insertDFData(dfResults.paymentDatesAsTerms_, dfResults.discountFactors_, dfResults.paymentDates_, df_insert, term_insert, date_insert, dummyYields, std::numeric_limits<double>::quiet_NaN(), overrideIfDatesClash);
	}

	/*
		@brief insert df, term and date into df data
		@param[out]		dfs						Object into which discount factor is to be inserted
		@param[out]		dates					Date vector that correspond to the discount factor vector
		@param[in]		df_in					The discount factor being inserted
		@param[in]		term_in					Date fraction from spot/as-of date to the discount factor date
		@param[in]		date_in					Discount factor date
		@param[in]		overrideIfDatesClash	Override DF(DiscountFactor) if dates clashed
	*/
	void insertDFData(DoubleMatrix& dfs, DateVector& dates, double df_insert, double term_insert, const AQLDate& date_insert, bool overrideIfDatesClash)
	{
		AQ_REQUIRE( dfs.size() == 2, "Invalid Discount Factor Data: TermsDF matrix must have two columns with payment dates and discount factors" )
		DoubleVector dummyYields;
		insertDFData(dfs[0], dfs[1], dates, df_insert, term_insert, date_insert, dummyYields, std::numeric_limits<double>::quiet_NaN(), overrideIfDatesClash);
	}

	/*
		@brief Function to insert a ***SINGLE*** additional forward rate into an existing forward rate array in a sorted manner. Used for insertion of tension data points

		@param[out] fwd_termsmtx		Forward rate matrix where new forward rate is being inserted
		@param[out] fwds				Dates that correspond to all forward rates
		@param[in] insertStartTerm		Date fraction from spot/as-of date to start date of the forward rate term
		@param[in] insertEndTerm		Date fraction from spot/as-of date to end date of the forward rate term
		@param[in] insertFwdRate		Forward rate being inserted
	*/
	void insertForwardRateData(DoubleMatrix &fwd_termsmtx, DoubleArray &fwds, const double &insertStartTerm, const double &insertEndTerm, const double &insertFwdRate)
	{
		// Data Validation
		if (fwd_termsmtx.size() != 2)
			throw AQLCoreInvalidData("#Error: Unable to update forward rates, fwd_termsmtx size must be 2", __FILE__, __LINE__);

		// Sort and Insert Forward Data into "fwd_termsmtx" and "fwds" containers
		unsigned int pos = 0;
		AQLAlgorithm::locate<DoubleArray, double>(fwd_termsmtx[0], insertStartTerm, fwd_termsmtx[0].size(), pos);

		if (pos == fwd_termsmtx[0].size())
		{
			fwd_termsmtx[0].push_back(insertStartTerm);
			fwd_termsmtx[1].push_back(insertEndTerm);
			fwds.push_back(insertFwdRate);
		}
		else
		{
			if (fwd_termsmtx[0][pos] != insertStartTerm)
			{
				fwd_termsmtx[0].insert(fwd_termsmtx[0].begin() + pos, insertStartTerm);
				fwd_termsmtx[1].insert(fwd_termsmtx[1].begin() + pos, insertEndTerm);
				fwds.insert(fwds.begin() + pos, insertFwdRate);
			}
			else
			{
				fwd_termsmtx[0][pos] = insertStartTerm;
				fwd_termsmtx[1][pos] = insertEndTerm;
				fwds[pos] = insertFwdRate;
			}
		}
	}


	/*
		@brief change FRA term from "X" format to "M" format
		@param[in] term in "X" format
		@return term in "M" format
	*/
	AQLString changeFRATermFormat(const AQLString& inputTerm)
	{
		int pos = -1;
		pos = AQLString(inputTerm).toUpper().findString("X");
		if (pos == -1)
		{
			throw AQLCoreInvalidData("x does not exist in FRA Term.", __FILE__, __LINE__);
		}
		return inputTerm.subString(0, pos - 1) + AQLString("M");
	}

	/*
		@brief Function to insert ***MULTIPLE*** additional forward rates into an existing forward rate array in a sorted manner. Used for insertion of tension data points

		@param[out] fwd_termsmtx		Forward rate matrix where new forward rate is being inserted
		@param[out] fwds				Dates that correspond to all forward rates
		@param[in] insertStartTerm		Date fractions from spot/as-of date to start date of the forward rate terms
		@param[in] insertEndTerm		Date fractions from spot/as-of date to end date of the forward rate terms
		@param[in] insertFwdRate		Forward rates being inserted
	*/
	void insertForwardRatesData(DoubleMatrix &fwd_termsmtx, DoubleArray &fwds, const DoubleArray &insesrtStartTerms, const DoubleArray &insertEndTerms, const DoubleArray &insertFwdRates)
	{
		// Data Validation
		if (fwd_termsmtx.size() != 2)
		{
			throw AQLCoreInvalidData("#Error: Unable to update forward rates, fwd_termsmtx size must be 2", __FILE__, __LINE__);
		}

		if (insesrtStartTerms.empty() || insertEndTerms.empty() || insertFwdRates.empty())
		{
			throw AQLCoreInvalidData("#Error: Unable to update forward rates, startTerms, endTerms and fwdRates data cannot be empty", __FILE__, __LINE__);
		}

		if (insesrtStartTerms.size() != insertEndTerms.size() || insesrtStartTerms.size() != insertFwdRates.size())
		{
			throw AQLCoreInvalidData("#Error: Unable to update forward rates, inconsistent startTerms, endTerms and fwdRates data", __FILE__, __LINE__);
		}

		// Sort and Insert Forward Data into "fwd_termsmtx" and "fwds" containers
		unsigned int pos = 0;
		for (unsigned int i = 0; i < insertFwdRates.size() - 1; ++i)
		{

			AQLAlgorithm::locate<DoubleArray, double>(fwd_termsmtx[0], insesrtStartTerms[i], fwd_termsmtx[0].size(), pos);

			if (pos == fwd_termsmtx[0].size())
			{
				fwd_termsmtx[0].push_back(insesrtStartTerms[i]);
				fwd_termsmtx[1].push_back(insertEndTerms[i]);
				fwds.push_back(insertFwdRates[i]);
			}
			else
			{
				if (fwd_termsmtx[0][pos] != insesrtStartTerms[i])
				{
					fwd_termsmtx[0].insert(fwd_termsmtx[0].begin() + pos, insesrtStartTerms[i]);
					fwd_termsmtx[1].insert(fwd_termsmtx[1].begin() + pos, insertEndTerms[i]);
					fwds.insert(fwds.begin() + pos, insertFwdRates[i]);
				}
				else
				{
					fwd_termsmtx[0][pos] = insesrtStartTerms[i];
					fwd_termsmtx[1][pos] = insertEndTerms[i];
					fwds[pos] = insertFwdRates[i];
				}
			}
		}
	}


	/*
		@brief Bootstrap yield curve using FRA instruments producing discount factors and forward rates

		@param[out] dfResults				dfResults
		@param[out] fwd						Forward rates being generated
		@param[out] fwd_termsmtx			Forward rate dates
		@param[in] data_fra					FRA instruments
		@param[in] data_libor				Libor instruments
		@param[in] pInter_fw				Interpolator specific to FRAs/Futures
		@param[in] stateVariable			StateVariable
		@param[in] is_fwdswap				Using forward starting instruments?
		@param[in] frequency				Base frequency
		@param[in] liborIndexTerm			Reference rate term
		@param[in] asOfDate					curve AsOfDate
		@param[in] spotdate					Spot date of the curve
		@param[in] spotDateLibor			Spot date of the Libor instruments
		@param[in] firstSwapDate			First swap maturity date used for tension cutoff
		@param[in] implyForwards			Imply forward rates from instruments?
	*/
	void bootstrapFRAs( DiscountFactors & dfResults,
						DoubleArray& fwds,
						DoubleMatrix& fwd_termsmtx,
						const std::vector<AQLObject*>& data_fra,
						const std::vector<AQLObject*>&  data_libor,
						AQLInterpolationBase* pInter_fw,
						const StateVariableEnum& stateVariable,
						bool is_fwdswap,
						const AQLString& frequency,
						const AQLString& liborIndexTerm,
						const AQLDate & asOfDate,
						const AQLDate& spotdate,
						const AQLDate& spotDateLibor,
						const AQLDate& firstSwapDate,
						bool implyForwards,
					    const double linearSplineJoinDate,
					    unsigned int lastLiborPosition,
					    const AQLDate& lastLiborEndDate )
	{
		if (data_fra.empty())
		{
			return;
		}

		bool includeSwapsBeforeMPCSwaps = true;
		bool applyTensionFRAs = false;
		int tensionGapFRAs = 0;

		// set applyTensionFRAs flag            
		const AQLDataHolder *dh = &(data_fra[0]->getData(IR_CALIBRATION_DATA_APPLYTENSION, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			applyTensionFRAs = dynamic_cast<const AQLDataBool &>(dh->get()).get();
		}

		// set TensionGap flag
		tensionGapFRAs = dynamic_cast<const AQLDataInt&> ((data_fra[0]->getData(IR_CALIBRATION_DATA_TENSIONGAP, ISNOTNULL)).get()).get();
		if (applyTensionFRAs && tensionGapFRAs < 1)
		{
			AQLString msg = "#Error: The TensionGap parameter in the FRA conventions table must be a positive integer.";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}

		// get SmoothShortEnd		
		dh = &(data_fra[0]->getData(IR_CALIBRATION_DATA_SMOOTHSHORTEND, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			includeSwapsBeforeMPCSwaps = dynamic_cast<const AQLDataBool &>(dh->get()).get();
		}

		etrading::FuturesTypeEnum futuresType = etrading::SERIAL_FUTURES_BY_RATE;
		dh = &(data_fra[0]->getData(IR_CALIBRATION_DATA_FRAFUTURE_SERIAL_CALC_TYPE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			AQLString strfraFutureType = dynamic_cast<const AQLDataString&>(dh->get());
			futuresType = etrading::toFuturesTypeEnum(strfraFutureType.getCString());
		}

		bool isEOMRoll = false;
		dh = &(data_fra[0]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			isEOMRoll = dynamic_cast<const AQLDataBool &>(dh->get()).get();
		}

		const AQLString roll_conv = etrading::getRollConv(frequency, isEOMRoll);

		const AQLPriceDataSlidingRule& sld = dynamic_cast<const AQLPriceDataSlidingRule&> ((data_fra[0]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
		const AQLPriceDataCalendar& cal = dynamic_cast<const AQLPriceDataCalendar&> ((data_fra[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
		const AQLPriceDataDayCount& dc = dynamic_cast<const AQLPriceDataDayCount&> ((data_fra[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());

		auto size_f = data_fra.size();

		DateVector fraStartDates;
		DateVector fraEndDates;
		DoubleVector fraRates;
		fraStartDates.reserve(size_f);
		fraEndDates.reserve(size_f);
		fraRates.reserve(size_f);

		std::vector<ForwardRate> serialFras;
		serialFras.reserve(size_f);

		//True to indicate all the Fra/Futures are in contiguous months
		bool allContiguousFra = true;
		const bool isFUTURE = false;

		for (size_t i = 0; i < data_fra.size(); i++)
		{

			double rate = dynamic_cast<const AQLDataDouble&> ((data_fra[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
			const AQLDate spotDate = dynamic_cast<const AQLDataDate&> ((data_fra[i]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get()).get();

			AQLDate startDate, endDate;
			etrading::populateFraDates(startDate, endDate, spotDate, data_fra[i], liborIndexTerm, roll_conv, sld, cal, dc);

			// Don't Insert Any Forwards if EndDate >= FirstSwapDate
			if (endDate >= firstSwapDate)
			{
				break;
			}

			etrading::populateFraFutureData(fraStartDates, fraEndDates, fraRates, serialFras, allContiguousFra, futuresType, liborIndexTerm, spotdate, startDate, endDate, rate, isFUTURE);

		}

		AQLPriceDataDayCount dc_act365(ACT_365);

		//If all FRAs are contiguous, use the contiguous routine
		if (allContiguousFra)
		{
			futuresType = etrading::CONTIGUOUS_FUTURES;
		}

		etrading::bootstrapFuturesOrFRAs( dfResults,					// dfResults
										  fwd_termsmtx,					// fwd_termsmtx					Matrix of fwd rate table (start terms and end terms)
										  fwds,							// fwds							A list of fwd rates
										  futuresType,					// futuresType					futuresType
										  serialFras,					// serialFraFutures				Serial fra/futures for the method when SerialCalcType is DF
										  data_libor,					// data_libor					libor instrument data
										  data_fra,						// data_fraFuture				Fra/Future instrument data
										  fraStartDates,				// fraFutureStartDates			Fra/Future start dates
										  fraEndDates,					// fraFutureEndDates			Fra/Future end dates
										  fraRates,						// fraFutureRates				Fra/Future rates
										  asOfDate,						// asOfDate						curve AsOfDate
										  spotdate,						// spotDateSwap					Spot date for curve instruments
										  spotDateLibor,				// spotDateLibor				Spot date for libor deposits
										  *pInter_fw,					// pInter_fw					Future/Fra interpolator
										  liborIndexTerm,				// liborIndexTerm				Reference rate term
										  roll_conv,					// roll_conv					Roll convention
										  sld,							// sliding rule					Business Day Adjustment
										  cal,							// calendar
										  dc,							// dc							Trade day count
										  dc_act365,					// dc_act365					dc_act365 day count
										  stateVariable,				// stateVariable				State variable for future/fra
										  linearSplineJoinDate,			// joinDate						interpolationJoinDateAsDouble
										  lastLiborEndDate,				// lastLiborEndDate				lastLiborEndDate
										  lastLiborPosition,			// lastLiborPosition			lastLiborPosition
										  firstSwapDate,				// firstSwapDate
										  includeSwapsBeforeMPCSwaps,	// includeSwapsBeforeMPCSwaps	includeSwapsBeforeMPCSwaps
										  applyTensionFRAs,				// applyTension					applyTension
										  tensionGapFRAs,				// tensionGap					tensionGap
										  isFUTURE,						// isFuture						True to indicate it's a future
										  implyForwards );				// populateFwdTable				True to populate fwd table
	}

	/*
	@brief Calculate start and end dates of a FRA instrument

	@param[out] startDate		FRA start date
	@param[out] endDate			FRA end date
	@param[in]  spotDate		Spot date of curve
	@param[in]  data_fra		FRA instrument
	@param[in]	liborIndexTerm		The tenor of the curve
	@param[in]	baseFreq		The frequency of the curve
	@param[in]	dc				FRA day count
*/
	void calculateFraDates(AQLDate& startDate,
		AQLDate& endDate,
		const AQLDate& spotDate,
		const AQLObject* data_fra,
		const AQLString& liborIndexTerm,
		const AQLString& baseFreq,
		const AQLPriceDataDayCount& dc)
	{
		AQLString roll_conv("");
		bool isEOMRoll = false;
		const AQLDataHolder* dh = &(data_fra->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			isEOMRoll = dynamic_cast<const AQLDataBool &>(dh->get()).get();
		}

		// set roll convention			
		if (baseFreq == AQ_LUNAR)
		{
			roll_conv = AQ_ROLLCONV_LUNAR;
		}
		else if (isEOMRoll)
		{
			roll_conv = AQ_ROLLCONV_EOM;
		}
		else
		{
			roll_conv = AQ_ROLLCONV_NORMAL;
		}

		const AQLPriceDataSlidingRule& sld = dynamic_cast<const AQLPriceDataSlidingRule&> ((data_fra->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
		const AQLPriceDataCalendar& cal = dynamic_cast<const AQLPriceDataCalendar&> ((data_fra->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());

		populateFraDates(startDate, endDate, spotDate, data_fra, liborIndexTerm, roll_conv, sld, cal, dc);

	}


	/*
		@brief Populate start and end dates of a FRA instrument

		@param[out] startDate		FRA start date
		@param[out] endDate			FRA end date
		@param[in]  spotDate		Spot date of curve
		@param[in]  data_fra		FRA instrument
		@param[in]	liborIndexTerm		The tenor of the curve
		@param[in]	baseFreq		The frequency of the curve
		@param[in]	dc				FRA day count
	*/
	void populateFraDates(AQLDate& startDate,
						  AQLDate& endDate,
						  const AQLDate& spotDate,
						  const AQLObject* data_fra,
						  const AQLString& liborIndexTerm,
						  const AQLString& roll_conv,
						  const AQLPriceDataSlidingRule& sld,
						  const AQLPriceDataCalendar& cal,
						  const AQLPriceDataDayCount& dc)
	{
		const AQLDataHolder* dh = &data_fra->getData(PRICING_DATA_ISDATE, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			const bool is_date = dynamic_cast<const AQLDataBool&> (dh->get()).get();
			if (is_date)
			{
				startDate = dynamic_cast<const AQLDataDate&> ((data_fra->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get()).get();
				endDate = dynamic_cast<const AQLDataDate&> ((data_fra->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get()).get();
			}
			else
			{
				const AQLString sterm_str = dynamic_cast<const AQLDataString&> ((data_fra->getData(PRICING_DATA_STARTTERM, ISNOTNULL)).get()).get();
				startDate = etrading::LADateHelpers::getDate(spotDate, sterm_str, sld, &cal, true, &roll_conv);
				const AQLString tenor_str = dynamic_cast<const AQLDataString&> ((data_fra->getData(PRICING_DATA_TENOR, ISNOTNULL)).get()).get();
				endDate = etrading::LADateHelpers::getDate(startDate, tenor_str, sld, &cal, true, &roll_conv);
			}
		}
		else
		{
			const AQLString& terms_str_x = dynamic_cast<const AQLDataString&> ((data_fra->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			const AQLString& terms_str = changeFRATermFormat(terms_str_x);
			startDate = etrading::LADateHelpers::getDate(spotDate, terms_str, sld, &cal, true, &roll_conv);
			endDate = etrading::LADateHelpers::getDate(startDate, liborIndexTerm, sld, &cal, true, &roll_conv);
		}
	}

	/*
		@brief Bootstrap yield curve using Libor instruments producing discount factors and forward rates

		@param[out] fwd					Forward rates being generated
		@param[out] fwd_termsmtx		Forward rate dates
		@param[in] data_libor			Libor instruments
		@param[in] data_moneymarket		Market market instruments
		@param[in] is_fra_use			Are FRAs used?
		@param[in] is_f_use				Are Futures used?
		@param[in] spotDateSwap			Spot rate of the swap instruments
		@param[in] spotDateLibor		Spot rate of the Libor instruments
		@param[in] liborIndexTerm		Reference rate term
		@param[in] implyForwards		Imply forward rates from instruments?

		@return the index in the Libor vector that points to the current Libor rate
	*/
	DiscountFactors bootstrapLibors( DoubleArray& fwds,
									 DoubleMatrix& fwd_termsmtx,
									 const std::vector<AQLObject*> & data_libor,
									 const MoneyMarketData & data_moneymarket,
									 bool is_fra_use,
									 bool is_f_use,
									 const AQLDate& spotDateSwap,
									 const AQLDate& spotDateLibor,
									 const AQLString& liborIndexTerm,
									 bool implyForwards )
	{
		// Initialize Results with Boundary Condition
		// First Term = 0.0, First DF = 1.0, First Date = SwapSpotDate
		DiscountFactors results;
		results.paymentDatesAsTerms_.push_back(0.0);
		results.discountFactors_.push_back(1.0);
		results.paymentDates_.push_back(spotDateSwap);

		double liborForwardRate = 0.0;
		double liborForwardTerm = 0.0;
		bool isForwardLibor = false;

		//df from spotDateSwap to spotDateLibor or from spotDateLibor to spotDateSwap 
		double discountFactorSpotAdjustment = 1.0;	

		AQLDate thisEndDate;
		AQLString instrumentFrequency;
		size_t size_libor = data_libor.size();
		AQLPriceDataDayCount dc_act365(ACT_365);
		
		results.lastLiborPosition_ = 0;
		for (size_t i = 0; i < size_libor; i++)
		{
			instrumentFrequency = dynamic_cast<const AQLDataString&> ((data_libor[i]->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL)).get()).get();
			instrumentFrequency.toUpper();

			const AQLString& instrumentTerm = dynamic_cast<const AQLDataString&> ((data_libor[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			if ( (is_fra_use || is_f_use) && instrumentTerm != liborIndexTerm)
			{
				continue;
			}

			// Instrument Conventions
			results.lastLiborPosition_							= i;
			double instrumentRate								= dynamic_cast<const AQLDataDouble&> ((data_libor[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
			const AQLPriceDataCalendar& instrumentCalendar		= dynamic_cast<const AQLPriceDataCalendar&> ((data_libor[i]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
			const AQLPriceDataSlidingRule& instrumentBusDayAdj	= dynamic_cast<const AQLPriceDataSlidingRule&> ((data_libor[i]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
			const AQLPriceDataDayCount& instrumentDaycount		= dynamic_cast<const AQLPriceDataDayCount&> ((data_libor[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
			
			// Set EOM Roll
			bool isEOMRoll = false;
			const AQLDataHolder *dh = &(data_libor[i]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				isEOMRoll = dynamic_cast<const AQLDataBool &>(dh->get()).get();
			}

			// Set Roll Convention
			AQLString instrumentRollConvention("");
			if (instrumentFrequency == AQ_LUNAR)
			{
				instrumentRollConvention = AQ_ROLLCONV_LUNAR;
			}
			else if (isEOMRoll)
			{
				instrumentRollConvention = AQ_ROLLCONV_EOM;
			}
			else
			{
				instrumentRollConvention = AQ_ROLLCONV_NORMAL;
			}

			RateConvention instrumentRateConvention	= LACurvePricingObject::setRC(instrumentFrequency);
			AQLPriceDataConvention conv(instrumentDaycount.getDayCount(), instrumentRateConvention);

			// Libor Instrument End Date
			results.lastLiborEndDate_ = etrading::LADateHelpers::getDate(spotDateLibor, instrumentTerm, instrumentBusDayAdj, &instrumentCalendar, true, &instrumentRollConvention);


			double thisDF	= conv.getDF( instrumentRate, spotDateLibor, results.lastLiborEndDate_ );
			double thisTerm	= dc_act365.getTerm( spotDateLibor, results.lastLiborEndDate_ );

			// When date <= spotDateSwap, do not edit results
			if (results.lastLiborEndDate_ > spotDateSwap)
			{
				if (i == 0 || results.lastLiborEndDate_ != thisEndDate)
				{
					results.paymentDates_.push_back(results.lastLiborEndDate_);
					results.paymentDatesAsTerms_.push_back(thisTerm);
					results.discountFactors_.push_back(thisDF);
				}
				else
				{
					// Override previous discount factors when libor dates clash
					results.discountFactors_.back() = thisDF;
				}
			}

			thisEndDate = results.lastLiborEndDate_;
			if (!isForwardLibor && instrumentTerm == liborIndexTerm)
			{
				liborForwardRate	= instrumentRate;
				liborForwardTerm	= thisTerm;
				isForwardLibor		= true;
			}

			// Money Market Overnight O/N Adjustment
			bool applyMoneyMarketOvernightAdjustment = false;
			dh = &(data_libor[0]->getData(IR_CALIBRATION_DATA_ISONFORSPOTADJUST, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				applyMoneyMarketOvernightAdjustment = dynamic_cast<const AQLDataBool &>(dh->get()).get();
			}

			if (is_fra_use || i == 0)
			{
				bool isMoneyMarketAvailable = false;
				double rate_on				= 0.0;
				AQLPriceDataConvention conv_on;
				if (data_moneymarket.size() > 0)
				{
					// Money Market Conventions
					isMoneyMarketAvailable				= true;
					MoneyMarketData::const_iterator it_ = data_moneymarket.begin();
					rate_on								= dynamic_cast<const AQLDataDouble&> ((it_->second->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
					const AQLPriceDataDayCount& dc_on	= dynamic_cast<const AQLPriceDataDayCount&> ((it_->second->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
					RateConvention rc_on				= LACurvePricingObject::setRC(AQ_SIMPLE);
					conv_on								= AQLPriceDataConvention(dc_on.getDayCount(), rc_on);
				}

				if (spotDateLibor > spotDateSwap)
				{
					if ( isMoneyMarketAvailable && applyMoneyMarketOvernightAdjustment)
					{
						discountFactorSpotAdjustment = conv_on.getDF(rate_on, spotDateSwap, spotDateLibor);
					}
					else
					{
						discountFactorSpotAdjustment = conv.getDF(instrumentRate, spotDateSwap, spotDateLibor);
					}
				}
				else if (spotDateLibor < spotDateSwap)
				{
					if ( isMoneyMarketAvailable && applyMoneyMarketOvernightAdjustment )
					{
						discountFactorSpotAdjustment = conv_on.getDF(rate_on, spotDateLibor, spotDateSwap);
					}
					else
					{
						discountFactorSpotAdjustment = conv.getDF(instrumentRate, spotDateLibor, spotDateSwap);
					}
				}
			}
			// ***************************************************

			// For Analytical Risk
			DateVector liborStartAndEndDates(2, spotDateLibor);
			liborStartAndEndDates[1] = results.lastLiborEndDate_;
			if (!data_libor[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, NOCHECK).isDefined())
			{
				data_libor[i]->add(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, new AQLDataDates(liborStartAndEndDates));
			}
			else
			{
				dynamic_cast<AQLDataDates&>(data_libor[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO).get()).set(liborStartAndEndDates);
			}
		}

		// Check Size - Allowing For Boundary Condition 
		AQ_THROW_IF(is_fra_use && results.paymentDatesAsTerms_.size() <= 1, "Invalid Curve Data: Missing 3M or 6M Cash Deposit Instrument(s)");
		
		// Apply Spot Date Adjustments (if required)
		// *************************************************************
		double term_offset = 0.0;
		if (spotDateLibor < spotDateSwap)
		{
			double term = dc_act365.getTerm(spotDateLibor, spotDateSwap);
			for (unsigned int i = 0; i < results.paymentDatesAsTerms_.size() - 1; i++)
			{
				if (results.paymentDatesAsTerms_[i + 1] <= term)
				{
					// ignore existing data points
					continue;
				}
				results.paymentDatesAsTerms_[i + 1] = results.paymentDatesAsTerms_[i + 1] - term;
				results.discountFactors_[i + 1]		= results.discountFactors_[i + 1] / discountFactorSpotAdjustment;
			}
			term_offset = -term;
		}
		else if (spotDateLibor > spotDateSwap)
		{
			double term = dc_act365.getTerm(spotDateSwap, spotDateLibor);
			for (unsigned int i = 0; i < results.paymentDatesAsTerms_.size() - 1; i++)
			{
				results.paymentDatesAsTerms_[i + 1]	= results.paymentDatesAsTerms_[i + 1] + term;
				results.discountFactors_[i + 1]		= results.discountFactors_[i + 1] * discountFactorSpotAdjustment;
			}
			term_offset = term;
		}
		// *************************************************************

		// Update Forward Libors and apply Term Offsets if required
		if (isForwardLibor && implyForwards)
		{
			// Forward Start Date
			fwd_termsmtx[0].push_back(term_offset);
			
			// Forward End Date
			fwd_termsmtx[1].push_back(liborForwardTerm + term_offset);
			
			// Forward Rate
			fwds.push_back(liborForwardRate);
		}

		// Discount Factor Dimension Check
		AQ_REQUIRE( results.paymentDatesAsTerms_.size() == results.discountFactors_.size(), "Invalid Curve Cash Deposit Data: Inconsistent number of payment date terms and discount factors" )
		AQ_REQUIRE( results.paymentDates_.size() == results.discountFactors_.size(), "Invalid Curve Cash Deposit Data: Inconsistent number of payment dates and discount factors" )
		
		// Forwards Dimension Check
		if ( fwd_termsmtx.size() == 2 )
		{	
			AQ_REQUIRE( fwd_termsmtx[0].size() == fwd_termsmtx[1].size(), "Invalid Curve Cash Deposit Data: Number of forward rate start- and end-dates" )
			AQ_REQUIRE( fwd_termsmtx[0].size() == fwds.size(), "Invalid Curve Cash Deposit Data: Inconsistent number of fixing dates and forward rates" )
		}

		return results;
	}

	/*
		@brief Get YieldGen interpolation method for a curve from object pool

		@param[inout]	objHolder			Object holder object representing CurveCalibrationData
		@param[in]		suffix				Name of the curve concerned
		@return			Curve's interpolation
	*/
	AQLPriceDataInterpolation getYieldGenInterpolationByCurveName(AQLObjectHolder& objHolder, const AQLString& suffix)
	{
		AQLDataHolder *dh = &objHolder.getData(IR_CALIBRATION_DATA_INTERPOLATIONYG + suffix, NOCHECK);
		AQLPriceDataInterpolation inter;
		if (dh->isDefined() && !dh->isNull())
		{
			inter = dynamic_cast<AQLPriceDataInterpolation &>(dh->get());

			//Set the joinDate if it has been populated
			dh = &objHolder.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + suffix, NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				double interpolationJoinDateAsDouble = dynamic_cast<AQLDataDouble &>(dh->get());
				inter.setJoinDateAsDouble(interpolationJoinDateAsDouble);
			}
		}
		else
		{
			// Can't find YieldGen. Fall back to regular interpolation data.
			dh = &objHolder.getData(CALIBRATION_DATA_INTERPOLATION + suffix, NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				inter = dynamic_cast<AQLPriceDataInterpolation &>(dh->get());
			}
			else
			{
				dh = &objHolder.getData(CALIBRATION_DATA_INTERPOLATION, NOCHECK);
				if (dh->isDefined() && !dh->isNull())
				{
					inter = dynamic_cast<AQLPriceDataInterpolation &>(dh->get());
				}
				else
				{
					throw AQLCoreInvalidData("#Error: Failed to read interpolation method from the curve.", __FILE__, __LINE__);
				}
			}
		}

		return inter;
	}

	/*
		@brief calc float side pv on spot date

		@param[in] fwdRate_Interpolation :		Forward Rate Index Interpolation using ZeroRateTimesTime state variable
		@param[in] stateVariable :				stateVariable
		@param[in] discFactor_Interpolation :	Discount Factor Interpolation using Discount Factor state variable
		@param[in] spotDiscountFactor :					spotdate DF
		@param[in] spotDateAsTerms :					spotdate term
		@param[in] terms_grid :					grid
		@param[in] asOfDate						curve AsOfDate
		@param[in] accrualDaycount				The accrual daycount basis to use e.g. ACT/360
		@param[in] compoundFreq					The compounding frequency, defaults to SIMPLE
		@param[in] cpd_times :					compounding times (optionally)
		@param[in] term_start :					start term (might be fwd starting, optionally)

		@return floatside pv
	*/
	double calcFloatPV( AQLInterpolationBase &fwdRate_Interpolation,
						const StateVariableEnum& stateVariable,
						AQLInterpolationBase &discFactor_Interpolation,
						const double spotDiscountFactor,
						const double spotDateAsTerms,
						const DoubleArray &terms_grid,
						const AQLDate & asOfDate,
						const DayCountEnum & accrualDaycount,
						const CompoundingFrequencyEnum & compoundFreq,
						const int cpd_times,
						const double term_start,
						const DoubleVector& floatAccrualPeriods,
						const DoubleVector& fixingStarts,
						const DoubleVector& fixingEnds,
						const DoubleVector& fixingTaus )
	{
		double ret = 0.0;
		if (terms_grid.empty())
		{
			return ret;
		}

		double df = 1.0;
		double df_index = 1.0;
		double df_index_b = getInterpolatedDiscountfactor(fwdRate_Interpolation, term_start, stateVariable, asOfDate, accrualDaycount, compoundFreq );
		unsigned int l = 1;

		for (l = 1; l * cpd_times - 1 < terms_grid.size(); l++) // compound grid
		{
			// calc compouding		
			double cpd = 1.0;
			double accuralTau = 0.0;
			double fixingTau = 0.0;
			for (unsigned int j = (l - 1) * cpd_times; j < l * cpd_times; ++j)
			{
				if (fixingStarts.size() == 0)
				{
					// This code snippet is not considering fixings and making an
					// assumption that fixing tau is the same as accrual tau
					df_index = getInterpolatedDiscountfactor( fwdRate_Interpolation, terms_grid[j], stateVariable, asOfDate, accrualDaycount, compoundFreq );
					const double df_ratio = df_index_b / df_index;
					// calc (1+tau*F)^n
					cpd *= df_ratio;
					df_index_b = df_index;
				}
				else
				{
					const double df_s = getInterpolatedDiscountfactor( fwdRate_Interpolation, fixingStarts[j], stateVariable, asOfDate, accrualDaycount, compoundFreq );
					const double df_e = getInterpolatedDiscountfactor( fwdRate_Interpolation, fixingEnds[j], stateVariable, asOfDate, accrualDaycount, compoundFreq );

					const double df_ratio = df_s / df_e;
					const double r = (df_ratio - 1) / fixingTaus[j];
					cpd *= (1 + r * fixingTaus[j]);

					fixingTau += fixingTaus[j];
					accuralTau += floatAccrualPeriods[j];
				}
			}

			df = discFactor_Interpolation.value(terms_grid[l * cpd_times - 1] + spotDateAsTerms) / spotDiscountFactor;

			if (fixingStarts.size() == 0)
			{
				ret += (cpd - 1.0) * df;
			}
			else
			{
				ret += (cpd - 1.0) * df * accuralTau / fixingTau;
			}
		}

		if (terms_grid.size() % cpd_times != 0) // too short term to compound (rest of compound grid)
		{
			for (unsigned int j = (l - 1) * cpd_times; j < terms_grid.size(); j++)
			{
				df = discFactor_Interpolation.value(terms_grid[j] + spotDateAsTerms) / spotDiscountFactor;

				if (fixingStarts.size() == 0)
				{
					// This code snippet is not considring fixings and making an
					// assumption that fixing tau is the same as accrual tau
					df_index = getInterpolatedDiscountfactor(fwdRate_Interpolation, terms_grid[j], stateVariable, asOfDate, accrualDaycount, compoundFreq);
					const double df_ratio = df_index_b / df_index;
					// calc L*tau*df
					ret += (df_ratio - 1.0) * df;
					df_index_b = df_index;
				}
				else
				{
					const double df_s = getInterpolatedDiscountfactor(fwdRate_Interpolation, fixingStarts[j], stateVariable, asOfDate, accrualDaycount, compoundFreq);
					const double df_e = getInterpolatedDiscountfactor(fwdRate_Interpolation, fixingEnds[j], stateVariable, asOfDate, accrualDaycount, compoundFreq);
					const double df_ratio = df_s / df_e;

					// LOG RESULTS
					// const double forwardRate    = (df_ratio - 1) / fixingTaus[j];
					// const double yearFraction   = floatAccrualPeriods[j];
					// const double pvLet          = (df_ratio - 1) / fixingTaus[j] * floatAccrualPeriods[j] * df;

					// CREATE_LOGFILE( pvLet, forwardRate, yearFraction, df);
					// LOGFILE_ADD_COMMENT( "SWAP FLOAT LEG" )

					ret += (df_ratio - 1) / fixingTaus[j] * floatAccrualPeriods[j] * df;
				}
			}
		}

		return ret;
	}

	/*
		@brief *** NEWTON-RAPHSON SOLVER ***
		solve for a smooth short end of the STD curve mitigating/eliminating Libor/Future(FRA) clash.
		We solve for the level of startDF and endDF that allows the libor instruments to be correctly repriced

		@param[inout] startDF					start discount factor of the first future/FRA
		@param[inout] endDF						end discount factor of the first future/FRA
		@param[in] startDate					Start date of the first future/FRA instrument
		@param[in] endDate						End date of the first future/FRA instrument
		@param[in] spotDateLibor				Libor spot date
		@param[in] spotDateSwap					Swap spot date
		@param[in] liborDate					End date of libor fixing
		@param[in] data_libor					The libor instrument object
		@param[in] cashDepositInterpolation		Interpolator used to smooth the front end
		@param[in] stateVariable			    stateVariable
		@param[in] dfResults					DF container of the STD curve
		@param[in] forwardRate					Forward rate of the first future/FRA instrument
		@param[in] forwardConv					Convention object of the first future/FRA instrument
	*/
	void solveSmoothSTDShortEnd(double& startDF,
								double& endDF,
								const AQLDate& startDate,
								const AQLDate& endDate,
								const AQLDate& spotDateLibor,
								const AQLDate& spotDateSwap,
								const AQLDate& liborDate,
								const AQLObject* data_libor,
								const std::unique_ptr<AQLInterpolationBase>& cashDepositInterpolation,
								const StateVariableEnum& stateVariable,
								const DiscountFactors & dfResults,
								const double forwardRate,
								const AQLPriceDataConvention& forwardConv)
	{
		const double DISCFACTOR_FLOOR = 1e-5;

		const bool interpolateOnLogDF = (stateVariable == STATE_VARIABLE_LOG_DF);
		if (interpolateOnLogDF)
		{
			// Log - DiscountFactor State Variable
			// -----------------------------------

			const AQLPriceDataDayCount& liborDC = dynamic_cast<const AQLPriceDataDayCount&> ((data_libor->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
			double liborRate = dynamic_cast<const AQLDataDouble&> ((data_libor->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
			AQLString liborFreq = dynamic_cast<const AQLDataString&> ((data_libor->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL)).get()).get();
			liborFreq.toUpper();

			RateConvention liborRC = LACurvePricingObject::setRC(liborFreq);
			AQLPriceDataConvention liborConv(liborDC.getDayCount(), liborRC);

			AQLPriceDataDayCount dc_act365(ACT_365);
			double startTerm = dc_act365.getTerm(spotDateSwap, startDate);
			double endTerm = dc_act365.getTerm(spotDateSwap, endDate);

			// Imply libor rate using DFs on spot date, first future start date and first future end date

			DoubleVector cashDepositTerms;
			// dfs Column0: date terms
			cashDepositTerms.push_back(dfResults.paymentDatesAsTerms_.front());
			cashDepositTerms.push_back(startTerm);
			cashDepositTerms.push_back(endTerm);

			// Validation Log Discount Factors - Floor Values at Zero, since DF's can't be negative
			AQ_REQUIRE( dfResults.discountFactors_.front() >= 0.0, "Invalid Market Data provided to Smooth Short Start of Curve Algorithm: Short End of Curve has Negative LogDF" )
			startDF			= std::max<double>( startDF, DISCFACTOR_FLOOR );
			endDF			= std::max<double>( endDF, DISCFACTOR_FLOOR );
			
			// dfs column 1: discount factors 
			DoubleVector cashDeposit_LogDFs;
			cashDeposit_LogDFs.push_back(AQLMath::log(dfResults.discountFactors_.front()));	// Transform to LogDF
			cashDeposit_LogDFs.push_back(AQLMath::log(startDF));								// Transform to LogDF
			cashDeposit_LogDFs.push_back(AQLMath::log(endDF));								// Transform to LogDF

			cashDepositInterpolation->set(cashDepositTerms, cashDeposit_LogDFs);

			double liborTerm = dc_act365.getTerm(spotDateLibor, liborDate);
			double liborDF = AQLMath::exp(cashDepositInterpolation->value(liborTerm));	// Transform to DF - cashDepositInterpolation is using LogDF
			double impliedLiborDiff0 = liborConv.getRate(liborDF, spotDateLibor, liborDate);

			// Imply libor rate again once DF on the first future start date has been purturbed
			startDF += 1.0e-5;
			endDF = startDF * forwardConv.getDF(forwardRate, startDate, endDate);

			// Validation Log Discount Factors - Floor Values at Zero, since DF's can't be negative
			startDF			= std::max<double>( startDF, DISCFACTOR_FLOOR );
			endDF			= std::max<double>( endDF, DISCFACTOR_FLOOR );

			cashDeposit_LogDFs[1] = AQLMath::log(startDF);	// Transform to LogDF
			cashDeposit_LogDFs[2] = AQLMath::log(endDF);		// Transform to LogDF
			cashDepositInterpolation->set(cashDepositTerms, cashDeposit_LogDFs);
			liborDF = AQLMath::exp(cashDepositInterpolation->value(liborTerm)); // Transform to DF - cashDepositInterpolation is using LogDF

			double impliedLiborDiff1 = liborRate - liborConv.getRate(liborDF, spotDateLibor, liborDate);

			// Newton-Raphson Parameters
			// -----------------------------
			int loopNum = 1000;
			bool solutionFound = false;
			double delta = 1.0e-10;
			double eps = 1.0e-9;
			double grad_eps = 1.0e-15;
			
			// Newton-Raphson Iteration Loop
			// -----------------------------
			while (loopNum--)
			{
				bool isEnd = true;
				if (AQLMath::abs(impliedLiborDiff0 - impliedLiborDiff1) >= grad_eps)
				{
					isEnd = false;
				}

				if (isEnd)
				{
					break;
				}

				// calculate derivative
				double startDF_temp = startDF;
				startDF_temp += delta;
				endDF = startDF_temp * forwardConv.getDF(forwardRate, startDate, endDate);

				// Validation Log Discount Factors - Floor Values at Zero, since DF's can't be negative
				startDF_temp	= std::max<double>( startDF_temp, DISCFACTOR_FLOOR );
				startDF			= std::max<double>( startDF, DISCFACTOR_FLOOR );
				endDF			= std::max<double>( endDF, DISCFACTOR_FLOOR );

				cashDeposit_LogDFs[1] = AQLMath::log(startDF_temp);	// Transform to LogDF
				cashDeposit_LogDFs[2] = AQLMath::log(endDF);			// Transform to LogDF
				cashDepositInterpolation->set(cashDepositTerms, cashDeposit_LogDFs);
				liborDF = AQLMath::exp(cashDepositInterpolation->value(liborTerm)); // Transform to DF - cashDepositInterpolation is using LogDF

				double impliedLiborDiff1_temp = liborRate - liborConv.getRate(liborDF, spotDateLibor, liborDate);
				double gradient = (impliedLiborDiff1 - impliedLiborDiff1_temp) / delta;
				AQ_REQUIRE( !AQ_IS_EQUAL_ZERO(gradient), "Smooth Curve Short End Algorithm Converged to an Invalid Solution using LogDF State Variable. Please check market data and convexity adjustment levels" )

				// adjust startDF by gradient and go again
				impliedLiborDiff0 = impliedLiborDiff1;

				startDF += impliedLiborDiff1 / gradient;
				endDF = startDF * forwardConv.getDF(forwardRate, startDate, endDate);

				// Validation Log Discount Factors - Floor Values at Zero, since DF's can't be negative
				startDF			= std::max<double>( startDF, DISCFACTOR_FLOOR );
				endDF			= std::max<double>( endDF, DISCFACTOR_FLOOR );

				cashDeposit_LogDFs[1] = AQLMath::log(startDF);		// Transform to LogDF
				cashDeposit_LogDFs[2] = AQLMath::log(endDF);			// Transform to LogDF
				cashDepositInterpolation->set(cashDepositTerms, cashDeposit_LogDFs);
				liborDF = AQLMath::exp(cashDepositInterpolation->value(liborTerm)); // Transform to DF - cashDepositInterpolation is using LogDF

				impliedLiborDiff1 = liborRate - liborConv.getRate(liborDF, spotDateLibor, liborDate);

				if (AQLMath::abs(impliedLiborDiff1) < eps)
				{
					solutionFound = true;
					break;
				}
			}
			// Ensure that a solution is found
			AQ_REQUIRE( solutionFound, "Smooth Curve Short End Algorithm Found No Solution using logDF State Variable. Please check market data and convexity adjustment levels" )
		}
		else
		{
			// Discount Factor State Variable
			// ------------------------------

			const AQLPriceDataDayCount& liborDC = dynamic_cast<const AQLPriceDataDayCount&> ((data_libor->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
			double liborRate = dynamic_cast<const AQLDataDouble&> ((data_libor->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
			AQLString liborFreq = dynamic_cast<const AQLDataString&> ((data_libor->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL)).get()).get();
			liborFreq.toUpper();

			RateConvention liborRC = LACurvePricingObject::setRC(liborFreq);
			AQLPriceDataConvention liborConv(liborDC.getDayCount(), liborRC);

			AQLPriceDataDayCount dc_act365(ACT_365);
			double startTerm = dc_act365.getTerm(spotDateSwap, startDate);
			double endTerm = dc_act365.getTerm(spotDateSwap, endDate);

			// Imply libor rate using DFs on spot date, first future start date and first future end date

			DoubleVector cashDepositTerms;
			
			// dfs Column0: date terms
			cashDepositTerms.push_back(dfResults.paymentDatesAsTerms_.front());
			cashDepositTerms.push_back(startTerm);
			cashDepositTerms.push_back(endTerm);

			// Validation Log Discount Factors - Floor Values at Zero, since DF's can't be negative
			AQ_REQUIRE( dfResults.discountFactors_.front() >= 0.0, "Invalid Market Data provided to Smooth Short Start of Curve Algorithm: Short End of Curve has Negative LogDF" )
			startDF			= std::max<double>( startDF, DISCFACTOR_FLOOR );
			endDF			= std::max<double>( endDF, DISCFACTOR_FLOOR );

			// dfs column 1: discount factors
			DoubleVector cashDeposit_DFs;
			cashDeposit_DFs.push_back(dfResults.discountFactors_.front());
			cashDeposit_DFs.push_back(startDF);
			cashDeposit_DFs.push_back(endDF);

			cashDepositInterpolation->set(cashDepositTerms, cashDeposit_DFs);

			double liborTerm = dc_act365.getTerm(spotDateLibor, liborDate);
			double liborDF = cashDepositInterpolation->value(liborTerm); // cashDepositInterpolation is using DF
			double impliedLiborDiff0 = liborConv.getRate(liborDF, spotDateLibor, liborDate);

			// Imply libor rate again once DF on the first future start date has been purturbed
			startDF += 1.0e-5;
			endDF = startDF * forwardConv.getDF(forwardRate, startDate, endDate);

			cashDeposit_DFs[1] = startDF;
			cashDeposit_DFs[2] = endDF;
			cashDepositInterpolation->set(cashDepositTerms, cashDeposit_DFs);
			liborDF = cashDepositInterpolation->value(liborTerm); // cashDepositInterpolation is using DF

			double impliedLiborDiff1 = liborRate - liborConv.getRate(liborDF, spotDateLibor, liborDate);

			int loopNum = 1000;
			bool solutionFound = false;
			double delta = 1.0e-10;
			double eps = 1.0e-9;
			double grad_eps = 1.0e-15;
			while (loopNum--)
			{
				bool isEnd = true;
				if (AQLMath::abs(impliedLiborDiff0 - impliedLiborDiff1) >= grad_eps)
				{
					isEnd = false;
				}

				if (isEnd)
				{
					break;
				}

				// calculate derivative
				double startDF_temp = startDF;
				startDF_temp += delta;
				endDF = startDF_temp * forwardConv.getDF(forwardRate, startDate, endDate);

				// Validation Log Discount Factors - Floor Values at Zero, since DF's can't be negative
				startDF_temp	= std::max<double>( startDF_temp, 0.0 );
				startDF			= std::max<double>( startDF, 0.0 );
				endDF			= std::max<double>( endDF, 0.0 );

				cashDeposit_DFs[1] = startDF_temp;
				cashDeposit_DFs[2] = endDF;
				cashDepositInterpolation->set(cashDepositTerms, cashDeposit_DFs);
				liborDF = cashDepositInterpolation->value(liborTerm); // cashDepositInterpolation is using DF

				double impliedLiborDiff1_temp = liborRate - liborConv.getRate(liborDF, spotDateLibor, liborDate);
				double gradient = (impliedLiborDiff1 - impliedLiborDiff1_temp) / delta;
				AQ_REQUIRE( !AQ_IS_EQUAL_ZERO(gradient), "Smooth Curve Short End Algorithm Converged to an Invalid Solution using DF State Variable. Please check market data and convexity adjustment levels" )

				// adjust startDF by gradient and go again
				impliedLiborDiff0 = impliedLiborDiff1;

				startDF += impliedLiborDiff1 / gradient;
				endDF = startDF * forwardConv.getDF(forwardRate, startDate, endDate);
				
				// Validation Log Discount Factors - Floor Values at Zero, since DF's can't be negative
				startDF			= std::max<double>( startDF, 0.0 );
				endDF			= std::max<double>( endDF, 0.0 );

				cashDeposit_DFs[1] = startDF;
				cashDeposit_DFs[2] = endDF;
				cashDepositInterpolation->set(cashDepositTerms, cashDeposit_DFs);
				liborDF = cashDepositInterpolation->value(liborTerm); // cashDepositInterpolation is using DF
				impliedLiborDiff1 = liborRate - liborConv.getRate(liborDF, spotDateLibor, liborDate);

				if (AQLMath::abs(impliedLiborDiff1) < eps)
				{
					solutionFound = true;
					break;
				}
			}
			// Ensure that a solution is found
			AQ_REQUIRE( solutionFound, "Smooth Curve Short End Algorithm Found No Solution using DF State Variable. Please check market data and convexity adjustment levels" )
		}
	}

	SwapPaymentSchedule::SwapPaymentSchedule( const AQLDate & startDate,
											  const AQLDate & endDate,
											  const AQLString & frequency,
											  const AQLPriceDataCalendar & calendar,
											  const AQLPriceDataSlidingRule & businessDayAdjustment,
											  const AQLPriceDataDayCount & daycount,
											  const AQLDate & forwardStartingSpotDate )
		: startDate_(startDate), endDate_(endDate), frequency_(frequency), calendar_(&calendar),
		  businessDayAdjustment_(&businessDayAdjustment), daycount_(&daycount), forwardStartingSpotDate_(&forwardStartingSpotDate)
	{
		// Generate the Schedule
		updateAccrualPeriodsAndPaymentDates( startDate_, endDate_, frequency_, *calendar_, *businessDayAdjustment_, *daycount_,
								   paymentDates_, paymentDatesAsTerms_, paymentDateIntervals_, rollEndOfMonth_,
								   forwardStartingSpotDate_ );
	}



	/*!
		@brief calculate swap payment dates, terms and intervals
		@param[in] startDate
		@param[in] endDate
		@param[in] frequency
		@param[in] cal calendar
		@param[in] busDayAdj businessDayAdjustment
		@param[in] dc daycount
		@param[out] cashflowPaymentDates
		@param[out] cashflowPaymentDatesAsTerms (daycount = ACT/365)
		@param[out] accuralPeriods (accrual period between payment Dates, which is incorrect - assumes no fixing or pay lag)
		@param[out] isEOMRoll roll end-of-month
		@param[in] spotDateSwap (swap spot date, optionally)
	*/
	void updateAccrualPeriodsAndPaymentDates( const AQLDate& startDate,
										      const AQLDate& endDate,
										      const AQLString& frequency,
										      const AQLPriceDataCalendar& calendar,
										      const AQLPriceDataSlidingRule& busDayAdj,
										      const AQLPriceDataDayCount& daycount,
										      DateVector& cashflowPaymentDates,
										      DoubleArray& cashflowPaymentDatesAsTerms,
										      DoubleArray& accuralPeriods,
										      bool isEOMRoll,
										      const AQLDate* spotDateSwap)
	{
		// Roll Conventions
		// ****************************************
		AQLDate* firstStubDate	= NULL;
		AQLDate* lastStubDate	= NULL;
		int*	rollDay			= NULL;

		AQLString thisRollConv = AQ_ROLLCONV_NORMAL;
		AQLString* roll_convention;
		
		// TODO: EOM Roll is missing its implementation
		//
		// if( isEOMRoll )
		// {
		// 	  thisRollConv = AQ_ROLLCONV_EOM;
		// }

		if(frequency == AQ_LUNAR)
		{
			thisRollConv = AQ_ROLLCONV_LUNAR;
		}
		roll_convention = &thisRollConv;
		// ****************************************

		etrading::LADateHelpers::generateSchedule(startDate,
												  endDate,
												  frequency,
												  true, //isarrear. Always true.
												  firstStubDate,
												  lastStubDate,
												  rollDay,
												  cashflowPaymentDates,
												  &busDayAdj,
												  &calendar,
												  false, //isStartRoll
												  roll_convention);

		std::sort(cashflowPaymentDates.begin(), cashflowPaymentDates.end());

		// Check of Swap Spot Date is Missing
		// The spot swap date is only needed for forward starting swaps, spot swaps don't require this parameter
		if (spotDateSwap == NULL || spotDateSwap == 0)
		{
			spotDateSwap = &startDate;
		}

		if (cashflowPaymentDates.size() > 0)
		{
			// *** IMPORTANT ***
			// Use ACT/365 instead of ACT/365_ISDA which causes leap-year irregularities
			AQLPriceDataDayCount dc_act365(ACT_365);
			
			AQLDate thisStartDate = startDate;
			for (size_t i = 0; i < cashflowPaymentDates.size(); ++i)
			{
				AQLDate thisEndDate = cashflowPaymentDates[i];
				
				// Discount Factor Lookup Year Fraction - Use internal curve daycount ACT/365
				cashflowPaymentDatesAsTerms.push_back(dc_act365.getTerm(*spotDateSwap, thisEndDate));
				
				// Accrual Period using trade daycount (incorrectly references payment dates & assumes no fixing or pay lag )
				accuralPeriods.push_back(daycount.getTerm(thisStartDate, thisEndDate, false));				

				thisStartDate = thisEndDate;
			}
		}

		// Validate Outputs - Cannot price and calibrate to swaps without a payment date schedule
		AQ_THROW_IF (cashflowPaymentDates.empty() || cashflowPaymentDates.size() < 1, "Invalid Swap Payment Schedule - Cashflow Payment Dates are Missing");

		// Validate Outputs - Ensure we are not including past coupons in our calculations
		AQ_THROW_IF(cashflowPaymentDates[0] <= startDate, "Invalid Swap Payment Schedule - Cashflow Payment Dates are before the Start Date.");
	}

	/*!
		@brief Determine the join date used by linear spline interpolation
		@param[in] lastFuture			Last future calibration instrument
		@param[in] firstSwap			First swap calibration instrument
		@param[in] spotDate_swap		Swap Spot date
		@param[in] is_fwdswap			Is forward starting swap used?
		@return	   Linear spline join date
	*/
	AQLDate determineLinearSplineInterpolationJoinDate(const AQLObject* lastFuture,
													  const AQLObject* firstSwap,
													  const AQLDate& spotDate_swap,
													  bool is_fwdswap
	)
	{
		AQLDate joinDate;
		AQLDate lastFutureEndDate = dynamic_cast<const AQLDataDate&>((lastFuture->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get()).get();
		AQLDate lastFutureStartDate = dynamic_cast<const AQLDataDate&>((lastFuture->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get()).get();

		//-------------------------------------------------------------------------------------------------
		// Find the date schedule of the first swap

		AQLString freq = dynamic_cast<const AQLDataString&> ((firstSwap->getData(IR_CALIBRATION_DATA_FREQUENCY_FLOAT, ISNOTNULL)).get()).get();
		const AQLPriceDataCalendar * cal = &dynamic_cast<const AQLPriceDataCalendar&> ((firstSwap->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
		const AQLPriceDataSlidingRule * sld = &dynamic_cast<const AQLPriceDataSlidingRule&> ((firstSwap->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
		const AQLPriceDataDayCount * dc = &dynamic_cast<const AQLPriceDataDayCount&> ((firstSwap->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());

		bool isEOMRoll = false;
		const AQLDataHolder* dh = &firstSwap->getData(IR_CALIBRATION_DATA_ISEOMROLLSW, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			isEOMRoll = dynamic_cast<const AQLDataBool &>(dh->get()).get();
		}

		AQLString roll_conv;
		if (freq == AQ_LUNAR) roll_conv = AQ_ROLLCONV_LUNAR;
		else if (isEOMRoll) roll_conv = AQ_ROLLCONV_EOM;
		else roll_conv = AQ_ROLLCONV_NORMAL;

		AQLDate startDate, endDate;
		DateVector datesVec;
		DoubleArray thisAccrualPeriod;
		if (is_fwdswap)
		{
			const bool is_date = dynamic_cast<const AQLDataBool&> ((firstSwap->getData(PRICING_DATA_ISDATE, ISNOTNULL)).get()).get();
			if (is_date)
			{
				startDate = dynamic_cast<const AQLDataDate&> ((firstSwap->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get()).get();
				endDate = dynamic_cast<const AQLDataDate&> ((firstSwap->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get()).get();
			}
			else
			{
				const AQLString sterm_str = dynamic_cast<const AQLDataString&> ((firstSwap->getData(PRICING_DATA_STARTTERM, ISNOTNULL)).get()).get();
				startDate = etrading::LADateHelpers::getDate(spotDate_swap, sterm_str, *sld, cal, true, &roll_conv);
				const AQLString tenor_str = dynamic_cast<const AQLDataString&> ((firstSwap->getData(PRICING_DATA_TENOR, ISNOTNULL)).get()).get();
				endDate = etrading::LADateHelpers::getDate(startDate, tenor_str, AQLPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr);
			}

			updateAccrualPeriodsAndPaymentDates(startDate, endDate, freq, *cal, *sld, *dc, datesVec, thisAccrualPeriod, thisAccrualPeriod, isEOMRoll);
		}
		else
		{
			const AQLString& term_str = dynamic_cast<const AQLDataString&> ((firstSwap->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			endDate = etrading::LADateHelpers::getDate(spotDate_swap, term_str, AQLPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr);

			updateAccrualPeriodsAndPaymentDates(spotDate_swap, endDate, freq, *cal, *sld, *dc, datesVec, thisAccrualPeriod, thisAccrualPeriod, isEOMRoll);
		}

		//-------------------------------------------------------------------------------------------------
		// determine the future/swap join date

		AQLDate firstDate, secondDate;
		size_t dateCount = datesVec.size();
		for (size_t i = 0; i < dateCount - 1; ++i)
		{
			if (datesVec[i] < lastFutureEndDate && datesVec[i + 1] >= lastFutureEndDate)
			{
				if (datesVec[i] == lastFutureStartDate)
				{
					// Don't use the last future's start date as join date as empirical study
					// shows it would give too much freedom to later part of the curve.
					continue;
				}
				else
				{
					// Select join date as the first swap Libor fixing date before the last future ends
					joinDate = datesVec[i];
					break;
				}
			}
			else if (datesVec[i] == lastFutureEndDate)
			{
				// Don't use the last future end date as the join date.
				// In this case pick the middle date between the current and previous Libor fixing dates
				firstDate = datesVec[i - 1];
				secondDate = datesVec[i];
				int intervalDays = firstDate.intervalDays(secondDate);
				firstDate.addDays(int(intervalDays >> 1));	// bitwise shift to divide by 2 to get to mid point
				joinDate = firstDate;
			}
		}

		// If joinDate fails to be set by now, set it to be the middle date between the start and end of the last future.
		if (joinDate == AQLDate())
		{
			firstDate = lastFutureStartDate;
			secondDate = lastFutureEndDate;
			int intervalDays = firstDate.intervalDays(secondDate);
			firstDate.addDays(int(intervalDays >> 1));	// bitwise shift to divide by 2 to get to mid point
			joinDate = firstDate;
		}

		return joinDate;
	}


	/*!
		@brief Obtains the ith term point ( eg '15Y' ) from the input market data, and checks for duplicate terms
		@param[in] i			The index into the market data
		@param[in] data_swap	The market data object
		@returns				The term string
	*/
	const AQLString& getMaturityAsTermString(unsigned int i, const std::vector<AQLObject*>& data_item)
	{
		const AQLString& maturityTermString = dynamic_cast<const AQLDataString&> ((data_item[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();

		if (i > 0)
		{
			// Perform a sanity check for duplicates terms.
			// We assume the terms are sorted, so any duplicates will be next to each other.
			const AQLString& previousTermString = dynamic_cast<const AQLDataString&> ((data_item[i - 1]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			if (previousTermString == maturityTermString)
			{
				throw AQLCoreInvalidData((boost::format("#Error: Duplicate Swap Instrument Detected, Duplicate Maturity Term: '%s'.") % maturityTermString.getCString()).str().c_str(), __FILE__, __LINE__);
			}
		}

		return maturityTermString;
	}

	/*! @brief get Grid property val

		@param [inout]	mpStaticData		Property manager object
		@param[in] key
		@param[in] curve
		@param[in] grid
		@return Property data value
	*/
	AQLString getGridStaticData(LAStaticData * mpStaticData, const AQLString &key, const AQLString &curve, const AQLString &grid)
	{
		AQLString suffix = "." + grid + curve;
		suffix.toLower();
		AQLString val = mpStaticData->getStaticData(key + suffix);
		if (val != AQ_NO_DATA)
		{
			return val;
		}
		else
		{
			suffix = curve;
			suffix.toLower();
			return mpStaticData->getStaticData(key + suffix);
		}
	}


	/* @brief	Populate cash instrument data into object pool.
	*			Cash market instruments include ON, TN and Libor market data.
	*
	*  @param [inout]	mpStaticData				Property manager object
	*  @param [inout]	curveCalibrationData		CurveCalibrationData object
	*  @param [inout]	refData						Reference data
	*  @param [inout]	objPool						Reference object to the object pool
	*  @param [in]		currency					Curve currency
	*  @param [in]		marketName					Market name of curve
	*  @param [in]		yieldDataName				Yield data name
	*  @param [in]		staticDataSuffix			Suffix used with property manager
	*  @param [in]		suffix_data					Suffix used with object pool
	*  @param [in]		isSpotUse					Use use given spot date?
	*  @param [in]		isFwdFX						Is using Fx forwards (for the short end)?
	*  @param [in]		asOfDate					As of date of curve
	*  @param [in]		fixingSource				Source of libor fixing
	*  @return			Libor rate that matches spot rate term (if provided)
	*/
	double populateCashInstrumentsToEntityPool(LAStaticData * mpStaticData,
											   AQLString& refData,
											   AQLObjectPool& objPool,
											   const AQLString& currency,
											   const AQLString& marketName,
											   const AQLString& yieldDataName,
											   const AQLString& staticDataSuffix,
											   const AQLString& suffix_data,
											   bool isSpotUse,
											   bool isFwdFX,
											   const AQLDate &asOfDate,
											   const AQLString& fixingSource,
											   const AQLString& spotRateTerm)
	{
		AQLString liborFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_FILE + staticDataSuffix);
		MAFileAccessor liborFile(LAMarketData::getNumFileName(liborFileName));
		AQLStringMatrix liborDataMtx;
		if (!isFwdFX)
		{
			liborFile.readAllData(MARKET_DATA_DELIMITER, liborDataMtx);
			liborFile.close();
		}

		if ((liborDataMtx.size() == 0 || liborDataMtx[0].size() < 2) && !isFwdFX)
		{
			throw AQLCoreInvalidData("#Error: This curve requires Libor fixing rates to be provided as its inputs which are currently missing", __FILE__, __LINE__);
		}

		double firstVal = 0.0;
		if (!isFwdFX)
		{
			firstVal = liborDataMtx[0][1].getDoubleValue();
		}

		bool skipONTN = false;
		AQLString calMStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_MONEYMARKET_CALENDAR + staticDataSuffix);
		if (calMStr == AQ_NO_DATA && fixingSource == ITSELF)
		{
			skipONTN = true;
		}

		AQLObject *mktDataO_N = NULL;
		AQLObject *mktDataT_N = NULL;
		AQLString daycMStr = "";
		AQLString slidingMStr = "";
		if (!skipONTN)
		{
			// create O_N			
			AQLString nameO_N = yieldDataName + "_O_N" + suffix_data;
			const AQLObjectHolder ehois = objPool.getObject(nameO_N);
			if (!ehois.isDefined())
			{
				mktDataO_N = new AQLObject();
				objPool.set(nameO_N, mktDataO_N);
			}
			else
			{
				objPool.getObject(nameO_N).get().clear();
				mktDataO_N = &objPool.getObject(nameO_N).get();
			}

			mktDataO_N->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(nameO_N);

			// set calendar		
			mktDataO_N->add(CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar()).convertFromString(calMStr);
			// set spotdate
			if (isSpotUse)
			{
				AQLDate spotDateM(mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_MONEYMARKET_SPOTDATE + staticDataSuffix).getCString());
				mktDataO_N->add(IR_CALIBRATION_DATA_SPOTDATE, new AQLDataDate(spotDateM));
			}
			// set daycount
			daycMStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_MONEYMARKET_DAYCOUNT + staticDataSuffix).toUpper();
			mktDataO_N->add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(daycMStr);
			// set sliding
			slidingMStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_MONEYMARKET_SLIDINGRULE + staticDataSuffix).toUpper();
			mktDataO_N->add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(slidingMStr);
			// set data type
			mktDataO_N->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_O_N);
			refData += nameO_N + ":";

			// create T_N			
			AQLString nameT_N = yieldDataName + "_T_N" + suffix_data;
			const AQLObjectHolder ehtn = objPool.getObject(nameT_N);
			if (!ehtn.isDefined())
			{
				mktDataT_N = mktDataO_N->clone();
				objPool.set(nameT_N, mktDataT_N);
			}
			else
			{
				mktDataT_N = &objPool.getObject(nameT_N).get();
			}

			mktDataT_N->getData(CALIBRATION_DATA_NAME, ISNOTNULL).convertFromString(nameT_N);
			// set data type
			mktDataT_N->getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL).convertFromString(YIELD_TYPE_T_N);
			refData += nameT_N + ":";
		}

		// get cal and calc spot date
		AQLPriceDataCalendar calL;
		AQLString calLStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_CALENDAR + staticDataSuffix);
		calL.convertFromString(calLStr);
		AQLDate spotDateL;
		if (isSpotUse)
		{
			spotDateL.setDate(mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_SPOTDATE + staticDataSuffix).getCString());
		}
		else
		{
			spotDateL = calL.getBusinessDay(asOfDate, mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_RESETLAG + staticDataSuffix).getIntValue());
		}

		bool isOnSpotAdj = false;
		AQLString strIsOnSpotAdj = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_ISONFORSPOTADJUST + staticDataSuffix).toUpper();
		if (strIsOnSpotAdj != AQ_NO_DATA)
		{
			AQLDataBool tmpIsOnSpotAdj;
			tmpIsOnSpotAdj.convertFromString(strIsOnSpotAdj);
			isOnSpotAdj = tmpIsOnSpotAdj.get();
		}

		bool onValFlg = false;
		bool tnValFlg = false;

		const int liborSize = liborDataMtx.size();
		// use grid
		AQLStringVector liborUseGrid;
		AQLString tmpLiborUseGrid = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_USEGRID + staticDataSuffix).toUpper();
		if (tmpLiborUseGrid != AQ_NO_DATA)
		{
			liborUseGrid = tmpLiborUseGrid.toToken(':');
		}
		// get eomroll
		bool isEOMRollL = false;
		AQLString strEOMRollL = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_ISEOMROLL + staticDataSuffix).toUpper();
		if (strEOMRollL != AQ_NO_DATA)
		{
			AQLDataBool tmpIsEOMRoll;
			tmpIsEOMRoll.convertFromString(strEOMRollL);
			isEOMRollL = tmpIsEOMRoll.get();
		}
		if (isEOMRollL)
		{
			AQLString strEOMDay = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_EOMDAY + staticDataSuffix).toUpper();
			if (strEOMDay != AQ_NO_DATA)
			{
				if (spotDateL.dayOfMonth() != strEOMDay.getIntValue())
				{
					isEOMRollL = false;
				}
			}
			else
			{
				AQLDate eomDate = calL.getEOMDay(spotDateL);
				if (spotDateL != eomDate)
				{
					isEOMRollL = false;
				}
			}
		}

		double spotLibor = DBL_MAX;
		for (int j = 0; j < liborSize; ++j)
		{
			AQLString term = liborDataMtx[j][0].toUpper();
			double rate = liborDataMtx[j][1].getDoubleValue();

			// When 'spotRateTerm' is not specified, add all Libor rates to market data collection
			// Otherwise only add the Libor tenor that is equal to 'spotRateTerm'
			if (spotRateTerm.size() == 0
				|| term.toUpper() == AQLString(spotRateTerm).toUpper())
			{
				if (term.toUpper() == AQLString(spotRateTerm).toUpper())
				{
					spotLibor = rate;
				}

				// get freq
				AQLString freqLStr = getGridStaticData(mpStaticData, currency + STATIC_DATA_KEY_YIELD_LIBOR_FREQUENCY, staticDataSuffix, term).toUpper();
				// get daycount
				AQLString daycLStr = getGridStaticData(mpStaticData, currency + STATIC_DATA_KEY_YIELD_LIBOR_DAYCOUNT, staticDataSuffix, term).toUpper();
				// get sliding
				AQLString slidingLStr = getGridStaticData(mpStaticData, currency + STATIC_DATA_KEY_YIELD_LIBOR_SLIDINGRULE, staticDataSuffix, term).toUpper();

				if (term == "ON" && mktDataO_N != NULL)
				{
					mktDataO_N->add(CALIBRATION_DATA_RATE, new AQLDataDouble(rate / 100.0));
					onValFlg = true;
					continue;
				}
				else if (term == "TN" && mktDataT_N != NULL)
				{
					mktDataT_N->remove(CALIBRATION_DATA_RATE);
					mktDataT_N->add(CALIBRATION_DATA_RATE, new AQLDataDouble(rate / 100.0));
					tnValFlg = true;
					continue;
				}

				AQLObject *mktData = NULL;
				AQLString nameL = yieldDataName + "_LIBOR_" + AQLString(j) + suffix_data;
				const AQLObjectHolder ehlibor = objPool.getObject(nameL);
				if (!ehlibor.isDefined())
				{
					mktData = new AQLObject();
					objPool.set(nameL, mktData);
				}
				else
				{
					objPool.getObject(nameL).get().clear();
					mktData = &objPool.getObject(nameL).get();
				}
				refData += nameL + ":";
				// set name
				mktData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(nameL);
				// set spot date
				mktData->add(IR_CALIBRATION_DATA_SPOTDATE, new AQLDataDate(spotDateL));
				// set calendar, sliding, daycount
				if (term == "SN")
				{
					mktData->add(CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar()).convertFromString(calMStr);
					mktData->add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(daycMStr);
					mktData->add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(slidingMStr);
				}
				else
				{
					mktData->add(CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar()).convertFromString(calLStr);
					mktData->add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(daycLStr);
					mktData->add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(slidingLStr);
				}
				// is on for spot adjust
				mktData->add(IR_CALIBRATION_DATA_ISONFORSPOTADJUST, new AQLDataBool(isOnSpotAdj));
				// set data type
				mktData->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_ZERO);
				// set frequency
				mktData->add(IR_CALIBRATION_DATA_FREQUENCY, new AQLDataString()).convertFromString(freqLStr);
				// set eomroll
				mktData->add(IR_CALIBRATION_DATA_ISEOMROLL, new AQLDataBool(isEOMRollL));
				// set term
				mktData->add(IR_CALIBRATION_DATA_TERM, new AQLDataString()).convertFromString(LAMarketData::convertToMLibTerm(term));
				//grid use
				if (liborUseGrid.size() != 0 && find(liborUseGrid.begin(), liborUseGrid.end(), term) == liborUseGrid.end())
				{
					mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool(false));
				}
				else
				{
					mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool(true));
				}
				// set rate
				mktData->add(CALIBRATION_DATA_RATE, new AQLDataDouble(rate / 100.0));
			}
		}

		// if no data for ON and TN set first Libor data to ON and TN
		if (!onValFlg && !skipONTN)
		{
			mktDataO_N->add(CALIBRATION_DATA_RATE, new AQLDataDouble(firstVal / 100.0));
		}

		if (!tnValFlg && !skipONTN)
		{
			mktDataT_N->remove(CALIBRATION_DATA_RATE);
			mktDataT_N->add(CALIBRATION_DATA_RATE, new AQLDataDouble(firstVal / 100.0));
		}

		return spotLibor;
	}

	/*! @brief Price the PVs of a group of OIS swaps (either outright or Libor-OIS basis swap)

		@param[out] allPVs				Calculated PVs of all swaps
		@param[in]  pInter_yield		Interpolator carrying OIS state variable
		@param[in]  df_inter			Interpolator carrying external discount rates
		@param[in]  size_calcs			Number of swap cash flows that need to be calculated for all calibration swaps
		@param[in]  size_calcs_s		Number of Libor swap cash flows that need to be calculated for all calibration swaps
		@param[in]  calced_sizes		Number of OIS swap cash flows that can be re-used
		@param[in]  calced_sizes_s		Number of Libor swap cash flows that can be re-used
		@param[in]  terms_grids			Terms from spot date to all OIS swap dates
		@param[in]  terms_grids_s		Terms from spot date to all Libor swap dates
		@param[in]  terms_intervals		Accrual terms between all OIS swap accrual dates
		@param[in]  terms_intervals_s	Accrual terms between all Libor swap accrual dates
		@param[in]  marketRate			Market OIS par rate
		@param[in]  marketRate_s		Market Libor swap par rate
		@param[in]  fixingStartDates	OIS swap fixing start dates
		@param[in]  fixingEndDates		OIS swap fixing end dates
		@param[in]  fixingTaus			Fixing terms between all OIS swap fixing dates
		@param[in]  term_strs			Maturity term of all calibration swaps
		@param[in]  is_selfdf			Boolean indicating self-discounting or not
		@param[in]  spotterm			Year fraction from as-of date to spot date
		@param[in]  d_spotdf			Discount factor from spot date to as-of date
		@param[in]  spotdate			Spot date
		@param[in]  swapCompoundingMethods	Compounding method for OIS swaps
		@param[in]  longTermConv		Indication of calibration swap type for the long end of OIS curve
		@param[in]  longTermGen			Generation method for long end calibration swaps
		@param[in]  bOISSwapCalcReset	Calculation reset condition for OIS swaps
		@param[in]  bFullSigmaReset		Cash flow reset condition for OIS swaps
		@param[in]  bLiborSwapCalcReset	Calculation reset condition for Libor legs
		@param[inout]  startterms		Terms from spot to start date of each OIS cash flow
		@param[inout]  endterms			Terms from spot to end date of each OIS cash flow
	*/
	void priceOISSwaps( DoubleVector& allPVs,
						AQLInterpolationBase* pInter_yield,
						AQLInterpolationBase *df_inter,
						const std::vector<size_t>& size_calcs,
						const std::vector<size_t>& size_calcs_s,
						const std::vector<size_t>& calced_sizes,
						const std::vector<size_t>& calced_sizes_s,
						const DoubleMatrix& terms_grids,
						const DoubleMatrix& terms_grids_s,
						const DoubleMatrix& terms_intervals,
						const DoubleMatrix& terms_intervals_s,
						const DoubleVector& marketRates,
						const DoubleVector& marketRates_s,
						const std::vector<DateVector>& fixingStartDates,
						const std::vector<DateVector>& fixingEndDates,
						const DoubleMatrix& fixingTaus,
						const AQLStringVector& term_strs,
						bool is_selfdf,
						double d_spotdf,
						double spotterm,
						const AQLDate& spotdate,
						const std::vector<etrading::OISCompoundingEnum> & swapCompoundingMethodEnums,
						const std::vector<etrading::OISLongTermInstrumentsEnum> & longTermConvEnums,
						const AQLStringVector& longTermGens,
						const std::vector<const AQLPriceDataCalendar*>& cals,
						const std::vector<const AQLPriceDataDayCount*>& dateCounts,
						const std::vector<bool>& bOISSwapCalcReset,
						const std::vector<bool>& bFullSigmaReset,
						const std::vector<bool>& bLiborSwapCalcReset,
						std::vector<DoubleMatrix>& startterms,
						std::vector<DoubleMatrix>& endterms )
	{
		// TODO: The OIS Fixed Leg is Assumed to have the same Annuity as the OIS Float Leg. Also the OIS ParRate and ...
		// ... Par Spread values are sharing the same marketRates container, which is not good.
		double oisFloatLegPV_WithoutSpread = 0.0;
		double oisFloatLegAnnuity = 0.0;
		double oisFixedLegAnnuity = 0.0;
		double liborFloatLegAnnuity = 0.0;

		double previousOisFloatLegPV_WithoutSpread = 0.0;
		double previousOisFloatLegAnnuity = 0.0;
		double previousOisFixedLegAnnuity = 0.0;
		double previousLiborFloatLegAnnuity = 0.0;

		allPVs.clear();
		for (size_t i = 0; i < term_strs.size(); ++i)
		{
			// Reset swap calculation, i.e. do not reuse calculations from earlier swaps
			if (bOISSwapCalcReset[i])
			{
				// if market is changed, reset sums.				
				previousOisFloatLegPV_WithoutSpread = 0.0;
				previousOisFloatLegAnnuity = 0.0;
				previousOisFixedLegAnnuity = 0.0;
				previousLiborFloatLegAnnuity = 0.0;
			}

			// Optimization Step: Full Sigma Reset = Annuity Reset
			if (bFullSigmaReset[i])
			{
				// When odd swap date schedule is encountered, reset all sigmas and will build swap date schedule from the beginning
				// rather than by extending from the previous swap's schedule
				previousOisFloatLegPV_WithoutSpread = 0.0;
				previousOisFloatLegAnnuity = 0.0;
				previousOisFixedLegAnnuity = 0.0;
			}

			// Reset when calibration swap frequency changes
			if (bLiborSwapCalcReset[i])
			{
				previousLiborFloatLegAnnuity = 0.0;
			}

			const OISLongTermInstrumentsEnum longTermConvEnum = longTermConvEnums[i];
			const AQLString longTermGen = longTermGens[i];

			startterms[i].resize(size_calcs[i]);
			endterms[i].resize(size_calcs[i]);

			// Par Rates
			double oisParRate = marketRates[i];
			double oisParSpread = marketRates[i];
			double liborParRate = marketRates_s[i];

			// ---------------------------------------------
			// Compute PV of single OIS swap (either outright or Libor-OIS basis swap)
			double pv = priceSingleOISSwapPV( oisParRate,
											  oisParSpread,
											  liborParRate,
											  pInter_yield,
											  df_inter,
											  oisFloatLegPV_WithoutSpread,
											  oisFloatLegAnnuity,
											  oisFixedLegAnnuity,
											  liborFloatLegAnnuity,
											  previousOisFloatLegPV_WithoutSpread,
											  previousOisFloatLegAnnuity,
											  previousOisFixedLegAnnuity,
											  previousLiborFloatLegAnnuity,
											  size_calcs[i],
											  calced_sizes[i],
											  size_calcs_s[i],
											  calced_sizes_s[i],
											  terms_grids[i],
											  terms_intervals[i],
											  terms_grids_s[i],
											  terms_intervals_s[i],
											  fixingTaus[i],
											  fixingStartDates[i],
											  fixingEndDates[i],
											  is_selfdf,
											  spotterm,
											  d_spotdf,
											  spotdate,
											  swapCompoundingMethodEnums[i],
											  longTermConvEnum,
											  longTermGen,
											  dateCounts[i],
											  cals[i],
											  startterms[i],
											  endterms[i] );

			allPVs.push_back(pv);

			// ---------------------------------------------
			// Record results of the earlier swap
			previousOisFloatLegPV_WithoutSpread = oisFloatLegPV_WithoutSpread;
			previousOisFloatLegAnnuity = oisFloatLegAnnuity;
			previousOisFixedLegAnnuity = oisFixedLegAnnuity;

			if (longTermConvEnum == etrading::LIBOROIS_OIS_LONGTERM_INSTRUMENTS)
			{
				previousLiborFloatLegAnnuity = liborFloatLegAnnuity;
			}
		}
	}

	/*! @brief Price the PV of a single OIS swap (either outright or Libor-OIS basis swap)

	@param[in]      oisParRate			                    Market OIS par rate
	@param[in]      liborOisParSpread		                Market Libor-OIS par spread
	@param[in]      liborParRate		                    Market Libor swap par rate
	@param[in]      pInter_yield		                    Interpolator carrying OIS state variable
	@param[in]      df_inter			                    Interpolator carrying external discount rates
	@param[inout]   oisFloatLegPV_WithoutSpread	            PV of the CURRENT OIS Float leg *** with no OIS Spread ***
	@param[inout]   oisFloatLegAnnuity				        Annuity of the current OIS Float Leg
	@param[inout]   oisFixedLegAnnuity				        Annuity of the current OIS Fixed Leg
	@param[inout]   liborFloatLegAnnuity			        Annuity of the Libor Float leg of the current LIBOR-OIS swap
	@param[in]      previousOisFloatLegPV_WithoutSpread	    PV of the PREVIOUS OIS Float leg - For optimization purposes *** with no OIS Spread ***
	@param[in]      previousOisFloatLegAnnuity				Annuity of the previous OIS Float Leg
	@param[in]      previousOisFixedLegAnnuity				Annuity of the previous OIS Fixed Leg
	@param[in]      previousLiborFloatLegAnnuity            Annuity of the Libor Float leg of the previous LIBOR-OIS swap
	@param[in]      size_calc			                    Number of OIS swap cash flows that need to be calculated for the current swap
	@param[in]      calced_size			                    Number of OIS swap cash flows that can be re-used
	@param[in]      size_calc_s			                    Number of Libor swap cash flows that need to be calculated for the current swap
	@param[in]      calced_size_s		                    Number of Libor swap cash flows that can be re-used
	@param[in]      terms_grids			                    Terms from spot date to all OIS swap dates
	@param[in]      terms_intervals		                    Accrual terms between all OIS swap accrual dates
	@param[in]      terms_grids_s		                    Terms from spot date to all Libor swap dates
	@param[in]      terms_intervals_s	                    Accrual terms between all Libor swap accrual dates
	@param[in]      fixingTaus			                    Fixing terms between all OIS swap fixing dates
	@param[in]      fixingStartDates	                    OIS swap fixing start dates
	@param[in]      fixingEndDates		                    OIS swap fixing end dates
	@param[in]      is_selfdf			                    Boolean indicating self-discounting or not
	@param[in]      spotterm			                    Year fraction from as-of date to spot date
	@param[in]      d_spotdf			                    Discount factor from spot date to as-of date
	@param[in]      spotdate			                    Spot date
	@param[in]      swapAveragingMethodEnum	                Compounding method for OIS swap
	@param[in]      longTermConvEnum	                    Indication of calibration swap type for the long end of OIS curve
	@param[in]      longTermGen			                    Generation method for long end calibration swaps
	@param[in]      dateCount			                    OIS swap date count
	@param[in]      cal					                    OIS swap calendar
	@param[inout]   startterms		                        Terms from spot to start date of each OIS cash flow
	@param[inout]   endterms			                    Terms from spot to end date of each OIS cash flow

	@return PV of the OIS swap (either an outright or a Libor-OIS basis swap)
	*/
	double priceSingleOISSwapPV(double oisParRate,
								double liborOisParSpread,
								double liborParRate,
								AQLInterpolationBase* pInter_yield,
								AQLInterpolationBase* df_inter,
								double& oisFloatLegPV_WithoutSpread,
								double& oisFloatLegAnnuity,
								double& oisFixedLegAnnuity,
								double& liborFloatLegAnnuity,
								double previousOisFloatLegPV_WithoutSpread,
								double previousOisFloatLegAnnuity,
								double previousOisFixedLegAnnuity,
								double previousLiborFloatLegAnnuity,
								size_t size_calc,
								size_t calced_size,
								size_t size_calc_s,
								size_t calced_size_s,
								const DoubleVector& terms_grids,
								const DoubleVector& terms_intervals,
								const DoubleVector& terms_grids_s,
								const DoubleVector& terms_intervals_s,
								const DoubleVector& fixingTaus,
								const DateVector& fixingStartDates,
								const DateVector& fixingEndDates,
								bool is_selfdf,
								double spotterm,
								double d_spotdf,
								const AQLDate& spotdate,
								const etrading::OISCompoundingEnum& swapAveragingMethodEnum,
								const etrading::OISLongTermInstrumentsEnum& longTermConvEnum,
								const AQLString& longTermGen,
								const AQLPriceDataDayCount* dayCount,
								const AQLPriceDataCalendar* cal,
								DoubleMatrix& startterms,
								DoubleMatrix& endterms)
	{
		oisFloatLegPV_WithoutSpread = 0.0;
		oisFloatLegAnnuity = 0.0;
		oisFixedLegAnnuity = 0.0;
		liborFloatLegAnnuity = 0.0;

		DoubleVector dfs_(size_calc);
		DoubleVector dfs_s(size_calc_s);

		// 1.   OIS FLOAT LEG - Annuity & PV Calculation
		// ====================================================================================================
		for (unsigned int j = 0; j < size_calc; ++j)
		{
			// Calculate OIS Float Leg Annuity and Discount Factors
			unsigned int pos = j + calced_size;
			if (is_selfdf)
			{
				dfs_[j] = AQLMath::exp(-pInter_yield->value(terms_grids[pos]) * terms_grids[pos]);
			}
			else
			{
				dfs_[j] = df_inter->value(terms_grids[pos] + spotterm);
				dfs_[j] /= d_spotdf;
			}
			oisFloatLegAnnuity += terms_intervals[pos] * dfs_[j];

			// Calculate the OIS Float Leg Effective Rate and PV
			AQLDate startdate = fixingStartDates[pos];
			AQLDate enddate = fixingEndDates[pos];
			double fixingTau = fixingTaus[pos];

			double effectiveRate = CurveCalibration::calcEffectiveOISRate(startterms[j], endterms[j], swapAveragingMethodEnum, longTermConvEnum, longTermGen, spotdate, startdate, enddate, dayCount, cal, pInter_yield, fixingTau);
			oisFloatLegPV_WithoutSpread += effectiveRate * terms_intervals[pos] * dfs_[j];
		}

		// 2.   OIS FIXED LEG - Annuity & PV Calculation
		// ====================================================================================================

		// TODO: The OIS Fixed Leg Annuity is currently set as the Float Leg Annuity - This assumption needs to be relaxed
		oisFixedLegAnnuity = oisFloatLegAnnuity;

		// 3.   LIBOR FLOAT LEG - Annuity & PV Calculation
		// ====================================================================================================
		if (longTermConvEnum == etrading::LIBOROIS_OIS_LONGTERM_INSTRUMENTS)
		{
			for (unsigned int j = 0; j < size_calc_s; ++j)
			{
				// Calculate the Libor Float Leg Annuity and Discount Factors
				unsigned int pos_s = j + calced_size_s;
				if (is_selfdf)
				{
					dfs_s[j] = AQLMath::exp(-pInter_yield->value(terms_grids_s[pos_s]) * terms_grids_s[pos_s]);
				}
				else
				{
					dfs_s[j] = df_inter->value(terms_grids_s[pos_s] + spotterm);
					dfs_s[j] /= d_spotdf;
				}
				liborFloatLegAnnuity += terms_intervals_s[pos_s] * dfs_s[j];
			}
		}

		// 4.   OPTIMIZATION STEP
		// Here we append and reuse the annuity factors and pvs from previous swaps to optimize performance.
		// For example when pricing a 2Y swap we can reuse information from the 1Y swap. Sometimes this is more confusing than it is helpful.
		// ====================================================================================================
		oisFloatLegPV_WithoutSpread += previousOisFloatLegPV_WithoutSpread;
		oisFloatLegAnnuity += previousOisFloatLegAnnuity;
		oisFixedLegAnnuity += previousOisFixedLegAnnuity;
		liborFloatLegAnnuity += previousLiborFloatLegAnnuity;

		// 5.   OIS TRADE PV CALCULATION
		// ====================================================================================================
		double tradePV = 0.0;
		double oisFixedLegPV = 0.0;
		double oisFloatLegPV = 0.0;
		double oisFloatSpreadPV = 0.0;
		double liborFloatLegPV = 0.0;

		// LOG FILE PARAMETERS: OIS FIXED LEG
		// AQLDate oisFixedLegMaturity                      = fixingEndDates[size_calc-1];
		// DateVector oisFixedLegStartDates                = fixingStartDates;
		// DateVector oisFixedLegEndDates                  = fixingEndDates;
		// DoubleVector oisFixedLegAccrualYearFractions    = terms_intervals;
		// DoubleVector oisFixedLegDiscountFactors         = dfs_;
		// AQLString oisFixedLegCalendar                    = cal->convertToString();
		// AQLString oisDayCount                            = dayCount->convertToString();

		if (longTermConvEnum == etrading::LIBOROIS_OIS_LONGTERM_INSTRUMENTS)
		{
			// LIBOR-OIS INSTRUMENTS
			// OIS Float Leg + OIS Spread + Swap Fixed Leg at Par Rate

			// OIS FLOAT LEG
			oisFloatSpreadPV = liborOisParSpread * oisFloatLegAnnuity;
			oisFloatLegPV = oisFloatLegPV_WithoutSpread + oisFloatSpreadPV;

			// LIBOR FLOAT LEG
			liborFloatLegPV = liborParRate * liborFloatLegAnnuity;

			// TRADE PV
			tradePV = oisFloatLegPV - liborFloatLegPV;

			// Log Calibration Results: LIBOR-OIS
			// This feature is disabled by default and can be enabled using 'meUtilityRecord'
			// CREATE_LOGFILE( oisFixedLegMaturity, longTermConvEnum, tradePV, oisFloatLegPV, liborFloatLegPV, oisFixedLegStartDates, oisFixedLegEndDates, oisFixedLegAccrualYearFractions, oisFixedLegDiscountFactors, oisFixedLegCalendar, oisDayCount, liborOisParSpread, liborParRate, spotterm, oisFixedLegAnnuity )
			// LOGFILE_ADD_COMMENT("OIS CURVE CALIBRATION: LIBOR-OIS TRADE SUMMARY")

		}
		else
		{
			// OUTRIGHT OIS SWAP INSTRUMENTS
			// OIS Float Leg + Fixed Leg at Par Rate

			// OIS FLOAT LEG
			oisFloatLegPV = oisFloatLegPV_WithoutSpread;

			// OIS FIXED LEG
			oisFixedLegPV = oisParRate * oisFixedLegAnnuity;

			// TRADE PV
			tradePV = oisFloatLegPV - oisFixedLegPV;

			// Log Calibration Results: OIS OUTRIGHTS
			// This feature is disabled by default and can be enabled using 'meUtilityRecord'
			// CREATE_LOGFILE( oisFixedLegMaturity, longTermConvEnum, tradePV, oisFloatLegPV, oisFixedLegPV, oisFixedLegStartDates, oisFixedLegEndDates, oisFixedLegAccrualYearFractions, oisFixedLegDiscountFactors, oisFixedLegCalendar, oisDayCount, oisParRate, spotterm, oisFixedLegAnnuity )
			// LOGFILE_ADD_COMMENT("OIS CURVE CALIBRATION: OUTRIGHT-OIS TRADE SUMMARY")
		}

		return tradePV;
	}

	/*! @brief Optimise linear spline join date

		@param[in] interp						Interpolation objects carrying logDF (the state variable of swap curve), usually interpolatorForSwaps
		@param[in] futuresStartDateTerms		Date terms of all futures start dates
		@param[in] futuresEndDateTerms			Date terms of all futures end dates
		@param[in] futuresRates					Equivalent forward rates of all futures contracts
		@param[in] interpDatesAsTerms			Date terms that correspond to all logDFs in 'interp'
		@param[in] interpValues					All logDFs in 'interp'
		@param[in] useNewtonRaphsonMinimizer	Default (TRUE): Use the NewtonRaphson Minimizer (TRUE), much faster or search for solution (FALSE)
		@param[in] searchIntervalDays			Default (1 Days): Controls the search inverval days	

		@return a pair of boolean indicating success and the refined join date
	*/
	InterpolationJoinDate optimizeInterpolationJoinDate( std::unique_ptr<AQLInterpolationBase>& interp,
														 const DoubleArray& futuresStartDateTerms,
														 const DoubleArray& futuresEndDateTerms,
														 const DoubleVector& futuresRates,
														 const DoubleArray& interpDatesAsTerms,
														 const DoubleArray& interpValues,
														 const bool useNewtonRaphsonMinimizer,
														 const size_t searchIntervalDays )
	{
		// Initialize Join Date Results Struct
		InterpolationJoinDate results;
		results.success_=false;
		results.joinDateAsDouble_ = 0.0;

		size_t futuresCount = futuresStartDateTerms.size();
		if (futuresCount < 2)
		{
			return results;
		}

		// Join date zone is set between the start and end of the last future
		double joinDateZoneStart = futuresStartDateTerms.back();
		double joinDateZoneEnd = futuresEndDateTerms.back();
		double joindDateZoneMidPoint = ( joinDateZoneStart + joinDateZoneEnd ) * 0.5;
		AQ_REQUIRE( joinDateZoneEnd >= joinDateZoneStart, "Unable to Optimize Interpolation Join Date: Futures EndDates must be smaller than the StartDates" )

		// Optimization zone is set between the start and end of the second last future		
		double optimizationZoneStart = futuresStartDateTerms[futuresCount - 2];
		double optimizationZoneEnd = futuresEndDateTerms[futuresCount - 2];
		double tau = optimizationZoneEnd - optimizationZoneStart;

		double oneDayTerm = 1 / 365.;

		// Linear interpolation of all futures rates
		std::shared_ptr<AQLLinearInterpolation> futuresRatesInter(new AQLLinearInterpolation(LINEAR_EXTRAPOLATION_TYPE));
		futuresRatesInter->set(futuresStartDateTerms, futuresRates);

		unsigned int i = 0;
		DoubleArray linearFuturesRates;
		while (optimizationZoneStart + i * oneDayTerm < optimizationZoneEnd)
		{
			double linearFwdRate = futuresRatesInter->value(optimizationZoneStart + i * oneDayTerm);
			linearFuturesRates.push_back(linearFwdRate);
			i++;
		}

		// Move the trial date along the joinDateZone to see which date provides the smallest discrepancy
		// against linear forward rates
		double smallestTotalDiffs = DBL_MAX;
		double bestTrialDate = 0.0;
		
		if( useNewtonRaphsonMinimizer )
		{
			// Use Newton-Raphson Minimizer to Find Best Join Date
			// ********************************************************************
			
			// One-dimensional Target function used by the mimimzer:
			// Must list parameters requiring access to within the this lambda function [] parentheses
			// The 'thisJoinDate' value is the variable which the solver will adjust in order to obtain the solution.
			auto function = [optimizationZoneStart, optimizationZoneEnd, &interp, oneDayTerm, tau, linearFuturesRates, interpDatesAsTerms, interpValues]( const double thisJoinDate )
			{
				// Set Interpolator using 'thisJoinDate'
				interp->set(interpDatesAsTerms, interpValues, thisJoinDate);

				double totalDiffs = 0.0;
				unsigned int i = 0;

				// For each trial date, calcualte the aggregated fwd rate differences between linear spline and linear over the optimization zone
				while (optimizationZoneStart + i * oneDayTerm < optimizationZoneEnd)
				{
					// Forward rate from linear spline
					double startDF = interp->value(optimizationZoneStart + i * oneDayTerm);
					double endDF = interp->value(optimizationZoneEnd + i * oneDayTerm);
					double fwdRate = (startDF / endDF - 1.0) / tau;

					// Forward rate from linearly interpolating futures rates
					double linearFwdRate = linearFuturesRates[i];

					double diff = AQLMath::abs(fwdRate - linearFwdRate);

					totalDiffs += diff;
					i++;
				}

				return totalDiffs;
			};

			// Solver Inputs
			const double	initialGuess				= joindDateZoneMidPoint;
			const double	tolerance					= oneDayTerm;
			const size_t	maxIterations				= 100;
			const double	shiftSize					= oneDayTerm;
			const double	intervalLowerBound			= joinDateZoneStart;
			const double	intervalUpperBound			= joinDateZoneEnd;
			const double	defaultValueToUseOnFailure	= joinDateZoneEnd;
		
			// Solver Results Contain: Solution, nInterations and Jacobian
			auto solverResults = etrading::solvers::newtonRaphsonMinimizer( function, initialGuess, tolerance, maxIterations, shiftSize, intervalLowerBound, intervalUpperBound, defaultValueToUseOnFailure );
			
			// Update Best Trial Date
			bestTrialDate = solverResults.solution;
		}
		else
		{ 
			// Test Every Date to Find Best Join Date
			// ********************************************************************
			
			// Store Intermediate Results for Diagnostic Checks
			std::vector<double> totalDiffResultDiagnostics;

			// Lowerbound Start Point
			double trialDate = joinDateZoneStart; 
			
			// Required to Manage UpperBound and Trial Date Precision
			const double datePrecisionControlVariate = oneDayTerm * 0.01; 
			
			while (trialDate <= joinDateZoneEnd + datePrecisionControlVariate)
			{
				// Required to Manage UpperBound and Trial Date Precision
				trialDate = std::min<double>( trialDate, joinDateZoneEnd );

				interp->setJoinDateAsDouble(trialDate);
				interp->set(interpDatesAsTerms, interpValues);

				double totalDiffs = 0.0;
				unsigned int i = 0;

				// For each trial date, calcualte the aggregated fwd rate differences between linear spline and linear over the optimization zone
				while (optimizationZoneStart + i * oneDayTerm * searchIntervalDays < optimizationZoneEnd)
				{
					// Forward rate from linear spline
					double startDF = interp->value(optimizationZoneStart + i * oneDayTerm);
					double endDF = interp->value(optimizationZoneEnd + i * oneDayTerm);
					double fwdRate = (startDF / endDF - 1.0) / tau;

					// Forward rate from linearly interpolating futures rates
					double linearFwdRate = linearFuturesRates[i];

					double diff = AQLMath::abs(fwdRate - linearFwdRate);

					totalDiffs += diff;
					i++;
				}

				// Diagnostics
				totalDiffResultDiagnostics.push_back( totalDiffs );

				if ( totalDiffs < smallestTotalDiffs )
				{
					smallestTotalDiffs = totalDiffs;
					bestTrialDate = trialDate;
				}

				trialDate += oneDayTerm;
			}
			
			// Set the Best Join Date to the interp object
			// Note: Already set for Newton-Raphson Minimizer method above
			interp->set(interpDatesAsTerms, interpValues, bestTrialDate);
		}

		// Pick the date with the smallest diff
		results.success_ = true;
		results.joinDateAsDouble_ = bestTrialDate;
		
		return results;
	}


	/*!
		@brief get date for moneymarket except O_N and T_N
		@param[in] basedate basedate
		@param[in] termstr string of such as "2D_1D" or "2D_1W"
		@param[in] cal calendar
		@param[in] srule holiday sliding rule
		@param[in,out] start start date
		@param[in,out] end end date
	*/
	void getMoneyMarketDates(const AQLDate& basedate, const AQLString& termstr, const AQLPriceDataCalendar& cal, const AQLPriceDataSlidingRule& srule, AQLDate& start, AQLDate& end)
	{
		AQLString str = termstr;
		str.trimLeft();
		str.trimRight();
		AQLStringVector str_v = str.toToken('_');
		if (str_v.size() != 2 || str_v[0].size() < 2 || str_v[1].size() < 2)
		{
			//error
			throw AQLCoreInvalidData((boost::format("#Error: Date Error: Invalid term string %s. Acceptable formats include: '2D_1D' or '2D_1W'") % str.getCString()).str().c_str(), __FILE__, __LINE__);
		}
		unsigned int size = str_v[0].size();
		str = str_v[0].subString(size - 1, size - 1);
		str.toUpper();
		if (str != "D")
		{
			//error
			throw AQLCoreInvalidData((boost::format("#Error: Date Error: Invalid term string: '%s'") % str_v[0].getCString()).str().c_str(), __FILE__, __LINE__);
		}
		AQLString str2 = str_v[0].subString(0, size - 2);
		char * pFirstNonNumber;
        int d = strtol(str2.getCString(), &pFirstNonNumber, 10); // base 10 numbers
		start = cal.getBusinessDay(basedate, d);
		end = start;
		size = str_v[1].size();
		str = str_v[1].subString(size - 1, size - 1);
		str.toUpper();
		str2 = str_v[1].subString(0, size - 2);
		if (str == "D")
		{
            char * pFirstNonNumber;
			d = strtol(str2.getCString(), &pFirstNonNumber, 10);
			end.addDays(d);
		}
		else if (str == "W")
		{
            char * pFirstNonNumber;
			d = strtol(str2.getCString(), &pFirstNonNumber, 10) * 7; // base 10 number
			end.addDays(d);
		}
		else if (str == "M")
		{
            char * pFirstNonNumber;
			end.addMonths(strtol(str2.getCString(), &pFirstNonNumber, 10)); // base 10 number
		}
		else if (str == "Y")
		{
            char * pFirstNonNumber;
			end.addYears(strtol(str2.getCString(), &pFirstNonNumber, 10));  // base 10 number
		}
		else
		{
			//error
			throw AQLCoreInvalidData((boost::format("#Error: Date Error: Invalid term string %s, must use 'D', 'W', 'M' or 'Y'") % termstr.getCString()).str().c_str(), __FILE__, __LINE__);
		}
		end = srule.getDate(end, cal);
	}


	/*!
		@brief Initialise yield curve state variables prior to solving
		@param[out]		stateVariable_rates			Vector that carries the rates as state variable
		@param[out]		stateVariable_grid			Vector that carries the state variable date grids
		@param[in]		objHolder							Object object pointer
		@param[in]		stateVariable						stateVariable
		@param[in]		rates
		@param[in,out] start start date
		@param[in,out] end end date
	*/
	void initialiseStateVariablesForSolving(DoubleArray& stateVariable_rates,
		DoubleArray&stateVariable_grid,
		const AQLObjectHolder& objHolder,
		const StateVariableEnum& stateVariable,
		const DoubleArray& rates,
		const DoubleArray& grids,
		const bool& fastRebuildRequested,
		const AQLString& targetSuffix,
		size_t dataSize)
	{
		bool previousSolutionAvailable = false;
		if (fastRebuildRequested)
		{
			// Attempt to fetch the previous curve state-variable std::vector
			const AQLDataHolder* ahStateVariable = &(objHolder.getData(IR_CALIBRATION_DATA_JACOBIAN_STATE_VARIABLES + targetSuffix, NOCHECK));
			if (ahStateVariable->isDefined() && !ahStateVariable->isNull())
			{
				const DoubleArray& previousSolution = dynamic_cast<const AQLDataDoubles &>(ahStateVariable->get()).get();
				// Sanity check: Verify that the previous solution is the correct size
				if (previousSolution.size() == dataSize)
				{
					for (unsigned int i = 0; i < dataSize; ++i)
					{
						stateVariable_grid.push_back(grids[i]);
						stateVariable_rates.push_back(previousSolution[i]);
					}

					previousSolutionAvailable = true;
				}
			}
		}

		if (!previousSolutionAvailable)
		{
			// No previous solution. Setup a default initial guess
			for (unsigned int i = 0; i < dataSize; ++i)
			{
				stateVariable_grid.push_back(grids[i]);

				double initialLogDF = rates[i] * grids[i];
				//TODO: this should be based on stateVariable
				stateVariable_rates.push_back(initialLogDF);
			}
		}
	}

	// Insert daily zero rates for the  Central Bank Swap/ARR future, when there are swaps before ECB/Futures
	void insertDailyZeroRatesForCentralBankFromShortTermSwaps(DoubleVector& grid,
															DoubleVector& yields,
															DateVector& dates,
															const AQLDate& fromDate,
															const AQLDate& spotdate,
															const AQLDate& shortterm_date,
															const double lastSwapRate,
															const AQLDate& firstCBSStartDate,
															const double firstCBSRate,
															const std::map<AQLDate, std::pair<AQLDate, double>>& mpcSwapRates,
															const double initialDF,
															const AQLPriceDataDayCount& dc_act365,
															const AQLPriceDataCalendar& cal,
															const AQLPriceDataConvention& conv)
	{

		double gapTerm = firstCBSStartDate.intervalDays(fromDate);
		double rateIncrement = (firstCBSRate - lastSwapRate) / gapTerm;

		AQLDate tmp_date = fromDate;

		double df = initialDF;

		auto it = mpcSwapRates.begin();

		while (tmp_date < shortterm_date)	// shortterm_date is end of the Central Bank Swaps section on the curve and can be a user given date date that can come before the last Central Bank Swaps end date
		{
			std::map<AQLDate, std::pair<AQLDate, double>>::const_iterator it_n = it;

			if (it != (--mpcSwapRates.end()) && (++it_n)->first <= tmp_date)
			{
				++it;
			}

			AQLDate n_date = cal.getBusinessDay(tmp_date, 1);
			const double endTerm = dc_act365.getTerm(spotdate, n_date);
			double instanteneousFwdRate(0.0);
			if (n_date < firstCBSStartDate)
			{
				//Directly linear interpolation on forward rate, between LastSwapRate and the first ECB/ArrFuture
				instanteneousFwdRate = (n_date.intervalDays(fromDate)) * rateIncrement + lastSwapRate;
			}
			else
			{
				instanteneousFwdRate = it->second.second;
			}

			df *= conv.getDF(instanteneousFwdRate, tmp_date, n_date);

			grid.push_back(endTerm);

			yields.push_back(-AQLMath::log(df) / endTerm);

			dates.push_back(n_date);

			tmp_date = n_date;
		}

	}

	// Insert daily zero rates for the  Central Bank Swap/ARR future, when there is no short end swap
	void insertDailyZeroRatesForCentralBank(DoubleVector& grid,
											DoubleVector& yields,
											DateVector& dates,
											const AQLDate& fromDate,
											const AQLDate& spotdate,
											const AQLDate& shortterm_date,
											const std::map<AQLDate, std::pair<AQLDate, double>>& mpcSwapRates,
											const double initialDF,
											const AQLPriceDataDayCount& dc_act365,
											const AQLPriceDataCalendar& cal,
											const AQLPriceDataConvention& conv,
											const double lastShortSwapTerm)
	{


		AQLDate tmp_date = fromDate;

		double df = initialDF;

		auto it = mpcSwapRates.begin();

		while (tmp_date < shortterm_date)	// shortterm_date is end of the Central Bank Swaps section on the curve and can be a user given date date that can come before the last Central Bank Swaps end date
		{
			std::map<AQLDate, std::pair<AQLDate, double>>::const_iterator it_n = it;

			if (it != (--mpcSwapRates.end()) && (++it_n)->first <= tmp_date)
			{
				++it;
			}

			AQLDate n_date = cal.getBusinessDay(tmp_date, 1);

			const double endTerm = dc_act365.getTerm(spotdate, n_date);

			double instanteneousFwdRate = it->second.second;

			df *= conv.getDF(instanteneousFwdRate, tmp_date, n_date);

			if (std::isnan(lastShortSwapTerm) || endTerm > lastShortSwapTerm)
			{
				grid.push_back(endTerm);
				yields.push_back(-AQLMath::log(df) / endTerm);
				dates.push_back(n_date);
			}

			tmp_date = n_date;
		}
	}

	// Insert zero rates for the Central Bank Swap/ARR future, by interpolation on rateTime or fwdRates.
	void insertZeroRatesForFutures(DoubleVector& grid,
									DoubleVector& yields,
									DateVector& dates,
									const AQLDate& spotdate,
									const std::unique_ptr<AQLInterpolationBase>& pInter_yield,
									const std::map<AQLDate, std::pair<AQLDate, double>>& futureRates, // key as startDate, values as endDate and fwdRate
									const AQLPriceDataDayCount& dc_act365,
									const AQLPriceDataConvention& conv,
									const AQLPriceDataCalendar& cal,
									const bool interpOnFwdRate)
	{

		if (interpOnFwdRate)
		{
			insertZeroRatesWithLinearFwdRates(grid, yields, dates, spotdate, pInter_yield, futureRates, dc_act365, conv, cal);
		}
		else
		{
			insertZeroRatesWithStepFwdRates(grid, yields, dates, spotdate, pInter_yield, futureRates, dc_act365, conv, cal);
		}

	}

	// Helper function to insert daily zeroRates based on a constant fwd rate 
	void insertDailyZeroRatesByConstantFwdRate(DoubleVector& grid,
											DoubleVector& yields,
											DateVector& dates,
											const AQLDate& spotdate,
											const AQLDate& startDate,
											const AQLDate& endDate,
											const double fwdRateToUse,
											const double initialDF,
											const AQLPriceDataDayCount& dc_act365,
											const AQLPriceDataCalendar& cal,
											const AQLPriceDataConvention& conv)
	{
		//From lastDate to lastFutureEndDate, use flat fwd rate
		AQLDate tempDate = startDate;

		double df = initialDF;

		// *** We need to insert daily DFs so that the daily compounding instruments can be repriced *** 
		while (tempDate < endDate)
		{
			auto endDate = cal.getBusinessDay(tempDate, 1);

			double startTerm = dc_act365.getTerm(spotdate, tempDate);

			// Cal df from the interpolated fwdRate
			df *= conv.getDF(fwdRateToUse, tempDate, endDate);

			double rateTimeEnd = -1.0 * AQLMath::log(df);

			double endTerm = dc_act365.getTerm(spotdate, endDate);

			// r = -lnDF/t
			double zeroRateEnd = rateTimeEnd / endTerm;

			grid.push_back(endTerm);
			yields.push_back(zeroRateEnd);
			dates.push_back(endDate);

			tempDate = endDate;
		}


	}

	// Interpolate on rateTime for the Central Bank Swap, so that the curve section can have step fwd rates
	void insertZeroRatesWithStepFwdRates(DoubleVector& grid,
										DoubleVector& yields,
										DateVector& dates,
										const AQLDate& spotdate,
										const std::unique_ptr<AQLInterpolationBase>& pInter_yield,
										const std::map<AQLDate, std::pair<AQLDate, double>>& futureRates, // key as startDate, values as endDate and fwdRate
										const AQLPriceDataDayCount& dc_act365,
										const AQLPriceDataConvention& conv,
										const AQLPriceDataCalendar& cal)
	{
		size_t totalSize = futureRates.size() + yields.size();

		DateVector dfDates;
		dfDates.reserve(totalSize);
		dfDates.push_back(spotdate);
		dfDates.insert(std::end(dfDates), std::begin(dates), std::end(dates));

		//State variable: Linear interp on negative log DF 
		std::unique_ptr<AQLInterpolationBase> rateTime_inter(dynamic_cast<AQLInterpolationBase *>(pInter_yield->clone()));

		for (auto it : futureRates)
		{

			DoubleVector rateTimeVec (grid.size());
			for (size_t j = 0; j < yields.size(); ++j)
			{
				// State Variable is negative Log discount factor, which is the same as (rate * time), i.e. lnDF = -rt => nLogDF = rt
				rateTimeVec[j] = yields[j] * grid[j];
			}

			//Update the interpolator
			rateTime_inter->set(grid, rateTimeVec);

			//if the map's startDate is earlier than spotDate, use spotDate
			const AQLDate& startDate = (it.first < spotdate) ? spotdate : it.first;
			// endDate: key as startDate, values as endDate and fwdRate
			const AQLDate& endDate = it.second.first;
			// fwdRate: key as startDate, values as endDate and fwdRate
			const double fwdRate = it.second.second;

			double start_term = dc_act365.getTerm(spotdate, startDate);
			double end_term = dc_act365.getTerm(spotdate, endDate);

			double rateTimeStart = rateTime_inter->value(start_term);

			//DF = exp(lnDF)
			double startDF = std::exp(-1.0 * rateTimeStart);
			
			double endDF = startDF * conv.getDF(fwdRate, startDate, endDate);
			double rateTimeEnd = -1.0 * AQLMath::log(endDF);

			// r = -lnDF/t
			double zeroRateStart = rateTimeStart / start_term;
			double zeroRateEnd = rateTimeEnd / end_term;

			etrading::insertDFData(grid, rateTimeVec, dfDates, rateTimeStart, start_term, startDate, yields, zeroRateStart);
			
            // insert daily zeroRates so that the daily compounding instruments can be repriced
			insertDailyZeroRatesByConstantFwdRate(grid,
												  yields,
												  dfDates,
												  spotdate,
												  startDate,
												  endDate,
												  fwdRate,
												  startDF,
												  dc_act365,
												  cal,
												  conv);
		}

	}

	// Interpolate on fwd rate directly, for ARR future, so that the curve section can have linear fwd rates
	void insertZeroRatesWithLinearFwdRates(DoubleVector& grid,
										DoubleVector& yields,
										DateVector& dates,
										const AQLDate& spotdate,
										const std::unique_ptr<AQLInterpolationBase>& pInter_yield,
										const std::map<AQLDate, std::pair<AQLDate, double>>& futureRates, // key as startDate, values as endDate and fwdRate
										const AQLPriceDataDayCount& dc_act365,
										const AQLPriceDataConvention& conv,
										const AQLPriceDataCalendar& cal)
	{
		size_t futureSize = futureRates.size();

		DoubleVector fwdStartTerms;
		fwdStartTerms.reserve(futureSize);

		DoubleVector fwdRates;
		fwdRates.reserve(futureSize);

		for (auto it : futureRates)
		{
			//if the map's startDate is earlier than spotDate, use spotDate
			const AQLDate& startDate = (it.first < spotdate) ? spotdate : it.first;

			// fwdRate: key as startDate, values as endDate and fwdRate
			const double fwdRate = it.second.second;
			double start_term = dc_act365.getTerm(spotdate, startDate);

			// Populate fwdStartTerms and fwdRates for forwardRate interpolator
			fwdStartTerms.push_back(start_term);
			fwdRates.push_back(fwdRate);
		}

		// ARRCurve: directly interpolate on fwdRate, i.e. fwdRate as stateVariable
		std::unique_ptr<AQLInterpolationBase> fwdRate_inter(dynamic_cast<AQLInterpolationBase *>(pInter_yield->clone()));
		fwdRate_inter->set(fwdStartTerms, fwdRates);

		// Populate daily entry to Discount factor table, which is required for daily compounding, so that the trade can be repriced when interpolating on fwdRateTable
		AQLDate firstDate = LADateScheduleHelpers::getDateFromTerm(spotdate, grid.back(), dc_act365);

		double joinDateAsDouble = fwdRate_inter->getJoinDateAsDouble();

		// Insert daily DFs till the last term, use the interpolationJoinDate when it is specified 
		double lastTerm = (joinDateAsDouble != 0) ? joinDateAsDouble : fwdStartTerms.back();
		AQLDate lastDate = LADateScheduleHelpers::getDateFromTerm(spotdate, lastTerm, dc_act365);

		auto lastFutureStartDate = futureRates.rbegin()->first;
		const AQLDate& lastFutureEndDate = futureRates.at(lastFutureStartDate).first;
		const double flatFwdRateForLastFuture = fwdRates.back();

		//initialize the DF to 1 for the spot term 0.0
		double df = (grid.size() == 1) ? 1.0 : AQLMath::exp(-yields.back() * grid.back());

		auto tempDate = firstDate;

		// *** We need to insert daily DFs so that the daily compounding instruments can be repriced *** 
		while (tempDate < lastFutureEndDate)
		{
			auto endDate = cal.getBusinessDay(tempDate, 1);

			double startTerm = dc_act365.getTerm(spotdate, tempDate);

			//From lastDate to lastFutureEndDate, use flat fwd rate
			double interpFwdRate = flatFwdRateForLastFuture;
			if (tempDate < lastDate)
			{
				//Up to lastDate, directly interpolate on fwdRate, i.e. fwdRate as stateVariable
				interpFwdRate = fwdRate_inter->value(startTerm);
			}

			// Cal df from the interpolated fwdRate
			df *= conv.getDF(interpFwdRate, tempDate, endDate);

			double rateTimeEnd = -1.0 * AQLMath::log(df);

			double endTerm = dc_act365.getTerm(spotdate, endDate);

			// r = -lnDF/t
			double zeroRateEnd = rateTimeEnd / endTerm;

			grid.push_back(endTerm);
			yields.push_back(zeroRateEnd);
			dates.push_back(endDate);

			tempDate = endDate;
		}

	}

	// Helper function to get the DF from zero rate
	double getOISdiscountFactor(const double term, const std::unordered_map<double, double>& termYieldMap, const std::unique_ptr<AQLInterpolationBase>& pInter_yield)
	{
		double yield = 0.0;

		//If the yield can be found, use it, otherwise get the yield via interpolation
		auto it = termYieldMap.find(term);
		if (it != termYieldMap.end())
		{
			yield = it->second;
		}
		else
		{
			yield = pInter_yield->value(term);
		}

		double df = AQLMath::exp(-yield * term);

		return df;
	}

	// Helper function to populate the input fixings to the market date object
	void populateHistoricalDataToMarketData(AQLObject *mktData, const AQLString& oisHistFileName)
	{
		MAFileAccessor oisHistFile(LAMarketData::getNumFileName(oisHistFileName));
		AQLStringMatrix oisHistDataMtx;
		oisHistFile.readAllData(MARKET_DATA_DELIMITER, oisHistDataMtx);
		oisHistFile.close();

		size_t fixingsSize = oisHistDataMtx.size();

		if (fixingsSize == 0 || oisHistDataMtx[0].size() < 2)
		{
            AQ_THROW("OIS / ARR fixing data is required and empty")
		}

		DateVector histdates;
		histdates.reserve(fixingsSize);

		DoubleVector histrates;
		histrates.reserve(fixingsSize);

		for (unsigned int j = 0; j < fixingsSize; j++)
		{
			histdates.push_back(AQLDataDate(oisHistDataMtx[j][0]).get());
			histrates.push_back(oisHistDataMtx[j][1].getDoubleValue() * 0.01);
		}
		mktData->add(IR_CALIBRATION_DATA_HISTORICALDATES, new AQLDataDates(histdates));
		mktData->add(IR_CALIBRATION_DATA_HISTORICALRATES, new AQLDataDoubles(histrates));
	}

	// Helper function to get the convexity adjustment for future instruments
	double getConvexityAdjustedFutureRate(AQLObject *mktData, const double futureRate, const AQLDate& asOfDate, const double meanReversion, const AQLPriceDataDayCount& dc)
	{
		// Legacy name 'useConvexAdj' means convexity quoted as a price ... poor name so replaced by convexityQuoteType
		bool useConvexAdj = false;
		bool convexityQuotedAsVol = true;
		const AQLDataHolder *dhUseConvexAdj = &(mktData->getData(PRICING_DATA_USECONVEXADJUSTMENT, NOCHECK));
		if (dhUseConvexAdj->isDefined() && !dhUseConvexAdj->isNull())
		{
			useConvexAdj = dynamic_cast<const AQLDataBool &>(dhUseConvexAdj->get()).get();
			convexityQuotedAsVol = !useConvexAdj;
		}

		// Alias and replacement for legacy useConvexAdj parameter
		const AQLDataHolder *dh = &(mktData->getData(PRICING_DATA_CONVEXITYQUOTETYPE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			AQLString convexityQuoteTypeStr = dynamic_cast<const AQLDataString &>(dh->get()).get();
			convexityQuoteTypeStr.toUpper();
			AQ_REQUIRE( convexityQuoteTypeStr == "VOL" || convexityQuoteTypeStr == "PRICE", "Invalid Futures Convexity Quote Type: ConvexityQuoteType must be VOL or PRICE" )
			convexityQuotedAsVol = ( convexityQuoteTypeStr == "VOL" ) ? true : false;
		
			// TODO: Property Manager does not allow us to clear parameters once set ... hence the below fails ... prioritize the convexityQuoteType parameter for now
			// // Don't allow the legacy name and alias to be used at the same time
			// if ( dhUseConvexAdj->isDefined() && !dhUseConvexAdj->isNull() )
			// {
			// 	AQ_THROW("Invalid Futures Convexity Parameter: Cannot use ConvexityQuoteType and UseConvexAdjustment (ConvexityQuotedAsPrice) parameters at the same time.")
			// }
		}

		// Convexity Quoted as a Price or Volatility
		double convexityAdjustment = 0.0;
		if (convexityQuotedAsVol)
		{
			// *** Convexity Quoted as Vol ***
			// imply convexity adjustment from model using volatility parameters

			// get future volatility
			double volatility = 0.0;
			dh = &(mktData->getData(PRICING_DATA_FUTUREVOLATILITY, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				volatility = dynamic_cast<const AQLDataDouble&> (dh->get()).get();
			}

			if (volatility > 0)
			{
				AQLDate futuresStartDate;
				dh = &(mktData->getData(PRICING_DATA_STARTDATE, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					futuresStartDate = dynamic_cast<const AQLDataDate&> (dh->get()).get();
				}

				AQLDate futuresEndDate;
				dh = &(mktData->getData(PRICING_DATA_ENDDATE, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					futuresEndDate = dynamic_cast<const AQLDataDate&> (dh->get()).get();
				}

				convexityAdjustment = etrading::getCurveEuroDollarConvexityAdjustment(asOfDate, futuresStartDate, futuresEndDate, meanReversion, volatility);

				bool useImprovedConvexityAdjustment = false;
				dh = &(mktData->getData(IR_CALIBRATION_DATA_ISCONVADJPRECISE, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					useImprovedConvexityAdjustment = dynamic_cast<const AQLDataBool &>(dh->get()).get();
				}

				if (useImprovedConvexityAdjustment)
				{

					// refer to a document of Bloomberg about convexity adjust of euro dollar future
					double tau = dc.getTerm(futuresStartDate, futuresEndDate);
					if (tau <= 0.0)
					{
						throw AQLCoreInvalidData("#Error: CurveCalibration::calcDiscountFactor failed. Invalid futures market data, a futures end date is before it's start date", __FILE__, __LINE__);
					}

					convexityAdjustment = (1.0 - AQLMath::exp(-convexityAdjustment * tau)) * (futureRate + 1.0 / tau);
				}
			}
		}
		else
		{
			// *** Convexity Quoted as Price ***
			
			// get convexity adjustment from user input		
			dh = &(mktData->getData(PRICING_DATA_CONVEXADJUSTMENT, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				convexityAdjustment = dynamic_cast<const AQLDataDouble&> (dh->get()).get();
			}
		}

		const double marketRate = futureRate - convexityAdjustment;
		return marketRate;
	}


	/*
	@brief Helper function to check if it's a Central bank swap based on the term string
	*/
	bool isCentralBankSwap(const AQLString& term)
	{
		return (term.findString("BOJ") >= 0         // Bank of Japan
			|| term.findString("EUSF") >= 0         // European Central Bank (Bloomberg Ticker)
			|| term.findString("ECB") >= 0          // European Central Bank
			|| term.findString("BOE") >= 0          // Bank of England
			|| term.findString("CB") >= 0           // *** GENERIC *** Central Bank Swaps
            || term.findString("MPC") >= 0 );       // *** GENERIC *** Monetary Policy Committee Swaps
	}

	/*
	@brief Helper function to check if it's a Future based on the term string
	*/
	bool isFuture(const AQLString& term)
	{
		return (term.findString("SF") >= 0 /** SOFR */
			|| term.findString("FUTURE") >= 0 /** general name */);
	}

	/*
	@brief Helper function to get the instrument type enum based on the term string
	*/
	OISMidTermInstrumentsEnum getOISMidTermInstrumentsEnum(const AQLString& term)
	{

		if (isCentralBankSwap(term))
		{
			return CENTRAL_BANK_SWAP;
		}
		else if (term.findString("FF") >= 0)
		{
			return FED_FUND_FUTURE;
		}
		else if (term.findString("1M") >= 0 && isFuture(term))
		{
			return ONE_MONTH_FUTURE;
		}
		else if (term.findString("3M") >= 0 && isFuture(term))
		{
			return THREE_MONTH_FUTURE;
		}
		//general name, default to 3M
		else if (isFuture(term))
		{
			return THREE_MONTH_FUTURE;
		}
		else
		{
			return NONE_OIS_SHORTTERM_INSTRUMENTS;
		}
	}

	/*
	@brief Helper function to get the default OIS compounding method, based on the instrument type
	*/
	OISCompoundingEnum getDefaultOISCompounding(const OISMidTermInstrumentsEnum& instrumentType, const AQLString& userInputShortTermConvStr)
	{

		OISCompoundingEnum compounding = NONE_OIS_COMPOUNDING;

		//User input as priority
		if (userInputShortTermConvStr.size() > 0)
		{
				compounding = etrading::toOISCompoundingEnum(userInputShortTermConvStr.getCString());
		}
		else
		{
			// If it's FedFundFuture or 1M ARR future, default to Arithmetic Average 
			if (instrumentType == FED_FUND_FUTURE || instrumentType == ONE_MONTH_FUTURE)
			{
				compounding = ARITHMETIC_OIS_COMPOUNDING;
			}
			// If it's CentralBankFuture or 3M ARR future, default to Geometric Compounding
			else if (instrumentType == CENTRAL_BANK_SWAP || instrumentType == THREE_MONTH_FUTURE)
			{
				compounding = GEOMETRIC_OIS_COMPOUNDING;
			}
		}

		return compounding;

	}

	//curveMarketName is the unique staticDataTable, which is different from a curveIndex
	CurveTypeEnum getCurveTypeEnum(const AQLObject& yieldData, const AQLString& curveMarketName)
	{

		std::string curveType = "";

		// Get curve type

		AQLString suffix = (curveMarketName == SWAP || curveMarketName == STD) ? "" : AQLString("_") + curveMarketName;

		const AQLDataHolder* dh = &yieldData.getData(CALIBRATION_DATA_CURVETYPE + suffix);

		if (dh->isDefined() && !dh->isNull())
		{
			curveType = dynamic_cast<const AQLDataString&>(dh->get()).get().getCString();
		}

		return etrading::toCurveTypeEnum(curveType);
	}

	/*
	@brief Calculate forward rates given a list of calibrated discount factors of the curve

	@param[in] inter
	@param[in] stateVariable
	@param[in] grid_swap
	@param[in] tau_swap
	@param[in] generateForwardsFromSwapsOnly
	@param[in] asOfDate
	@param[in] accrualDaycount
	@param[out] fwd_termsmtx
	@param[out] fwds
	*/
	void updateImpliedForwardRates(const AQLInterpolationBase &inter, const StateVariableEnum& stateVariable, const DoubleArray &fixingStarts, const DoubleArray &fixingEnds, const DoubleArray &tau_swap, bool generateForwardsFromSwapsOnly, const AQLDate & asOfDate, const DayCountEnum & accrualDaycount, DoubleMatrix &fwd_termsmtx, DoubleArray &fwds)
	{

		if (fwd_termsmtx.size() != 2)
		{
			throw AQLCoreInvalidData("Error: Unable to calculate forward rates. The fwd_termsmtx size must be 2", __FILE__, __LINE__);
		}
		if (fixingStarts.empty() || fixingEnds.empty())
		{
			throw AQLCoreInvalidData("#Error Unable to calculate forward rates. The fwd_grid is empty", __FILE__, __LINE__);
		}


		DoubleArray fwds_swap(fixingStarts.size());
		for (unsigned int i = 0; i < fixingStarts.size() - 1; ++i)
		{
			const double dfStart = etrading::getInterpolatedDiscountfactor(inter, fixingStarts[i], stateVariable, asOfDate, accrualDaycount);
			const double dfEnd = etrading::getInterpolatedDiscountfactor(inter, fixingEnds[i], stateVariable, asOfDate, accrualDaycount);

			fwds_swap[i] = (dfStart / dfEnd - 1) / tau_swap[i];
		}
		unsigned int pos = 0;
		for (unsigned int i = 0; i < fixingStarts.size() - 1; ++i)
		{
			AQLAlgorithm::locate<DoubleArray, double>(fwd_termsmtx[0], fixingStarts[i], fwd_termsmtx[0].size(), pos);
			if (pos == fwd_termsmtx[0].size())
			{
				fwd_termsmtx[0].push_back(fixingStarts[i]);
				fwd_termsmtx[1].push_back(fixingEnds[i]);
				fwds.push_back(fwds_swap[i]);
			}
			else
			{
				// If a swap payment date is not an existing forward rate date, make it one
				if (fwd_termsmtx[0][pos] != fixingStarts[i])
				{
					fwd_termsmtx[0].insert(fwd_termsmtx[0].begin() + pos, fixingStarts[i]);
					fwd_termsmtx[1].insert(fwd_termsmtx[1].begin() + pos, fixingEnds[i]);
					fwds.insert(fwds.begin() + pos, fwds_swap[i]);
				}
				else
				{
					// On the same date, forward rates derived from the Futures/FRA section can be slightly different from
					// forward rates derived from the Swaps section. We make a decision here which one to use.
					if (generateForwardsFromSwapsOnly)
					{
						fwd_termsmtx[0][pos] = fixingStarts[i];
						fwd_termsmtx[1][pos] = fixingEnds[i];
						fwds[pos] = fwds_swap[i];
					}
				}
			}
		}
	}


	/*
	@brief calc forwardrates for forwardTable

	@param[in] inter			ARR Curve's interpolator
	@param[in] baseDate			AsOfDate of the curve
	@param[in] terms			year fractions from asOfDate
	@param[in] dfs				discount factors
	@param[in] cal				calendar
	@param[in] dc_act365		Day count for discount factor
	@param[in] dc				Day count for trade
	@param[out] fwd_termsmtx    From/to terms of forward rate table
	@param[out] fwds			Forward rates of forward rate table
	*/
	void populateARRCurveForwardRateTable(const AQLInterpolationBase &inter,
										const AQLDate& baseDate,
										const DoubleVector& terms, 
										const DoubleVector& dfs, 
										const AQLPriceDataCalendar& cal, 
										const AQLPriceDataDayCount& dc_act365, 
										const AQLPriceDataDayCount& dc, 
										DoubleMatrix& fwd_termsmtx, 
										DoubleArray& fwds)
	{
		std::unique_ptr<AQLInterpolationBase> discountFactor_inter(dynamic_cast<AQLInterpolationBase *>(inter.clone()));
		discountFactor_inter->set(terms, dfs);

		AQLDate firstDate = etrading::LADateScheduleHelpers::getDateFromTerm(baseDate, terms.front(), dc_act365);
		AQLDate lastDate = etrading::LADateScheduleHelpers::getDateFromTerm(baseDate, terms.back(), dc_act365);

		size_t numberOfFixingDates = firstDate.intervalDays(lastDate);

		std::vector<double> fixingStartTerms;
		std::vector<double> fixingEndTerms;
		std::vector<double> fixingAccrualPeriods;

		fixingStartTerms.reserve(numberOfFixingDates);
		fixingEndTerms.reserve(numberOfFixingDates);
		fixingAccrualPeriods.reserve(numberOfFixingDates);

		size_t lastIndex = (terms.size() - 1);

		for (size_t i = 0; i < terms.size(); ++i)
		{

			auto startDate = etrading::LADateScheduleHelpers::getDateFromTerm(baseDate, terms[i], dc_act365);

			AQLDate endDate;
			if (i == lastIndex)
			{
				endDate = cal.getBusinessDay(startDate, 1);
			}
			else
			{
				endDate = etrading::LADateScheduleHelpers::getDateFromTerm(baseDate, terms[i + 1], dc_act365);
			}

			AQLDate tmp_date = startDate;

			while (tmp_date < endDate)
			{
				const AQLDate nextdate = cal.getBusinessDay(tmp_date, 1);

				double startTerm = dc_act365.getTerm(baseDate, tmp_date);
				double endTerm = dc_act365.getTerm(baseDate, nextdate);
				double tau = dc.getTerm(tmp_date, nextdate);

				fixingStartTerms.push_back(startTerm);
				fixingEndTerms.push_back(endTerm);
				fixingAccrualPeriods.push_back(tau);

				tmp_date = nextdate;

			}

		}

		etrading::updateImpliedForwardRates(*discountFactor_inter, etrading::STATE_VARIABLE_DF, fixingStartTerms, fixingEndTerms, fixingAccrualPeriods, true /*generateForwardsFromSwapsOnly*/, baseDate, dc.dayCountEnum(), fwd_termsmtx, fwds);

	}


	etrading::StateVariableEnum getInterpolationStateVariable(const CurveTypeEnum& curveTypeEnum)
	{
		if (curveTypeEnum == etrading::SWAP_CURVETYPE)
		{
			return STATE_VARIABLE_ZERO_RATE_TIMES_TIME;
		}
		else
		{
			return STATE_VARIABLE_DF;
		}
	}

	// Calculate the Swap Curve Spot Discount Factor and update money market discount factors and dates
	double getSpotDFandUpdateMoneyMarket(DoubleMatrix& df_moneymarket, DateVector& df_moneymarket_date, const std::map<std::pair<AQLDate, AQLDate>, const AQLObject*>&  data_moneymarket, const AQLPriceDataDayCount& dc_act365, const AQLDate& spotDateSwap)
	{
		std::map<std::pair<AQLDate, AQLDate>, const AQLObject*>::const_iterator it, it_last, it_tmp;
		double term_from_asof = 0.0;
		double df = 1.0;
		double dfSpot = 1.0;
		it_last = data_moneymarket.end();
		it_last--;
		for (it = data_moneymarket.begin(); it != data_moneymarket.end(); it++)
		{
			it_tmp = it;
			it_tmp--;
			if (it != data_moneymarket.begin() && it->first.first != it_tmp->first.second)
			{
				//error
				AQLString msg = "#Error: Invalid Money Market data, money market end dates must not overlap the start date of next instrument";
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}

			double rate = dynamic_cast<const AQLDataDouble&> ((it->second->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
			const AQLPriceDataDayCount& dc = dynamic_cast<const AQLPriceDataDayCount&> ((it->second->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
			double term = dc.getTerm(it->first.first, it->first.second, false);
			term_from_asof += dc_act365.getTerm(it->first.first, it->first.second);
			double df_tmp = 1.0 / (1.0 + rate * term);
			df *= df_tmp;
			df_moneymarket[0].push_back(term_from_asof);
			df_moneymarket[1].push_back(df);
			df_moneymarket_date.push_back(it->first.second);

			if (it->first.second < spotDateSwap)
			{
				it_tmp = it;
				it_tmp++;
				if (it == it_last)
				{
					df_tmp *= 1.0 / (1.0 + rate * dc.getTerm(it->first.second, spotDateSwap, false));
				}
				dfSpot *= df_tmp;
			}
			else if (it->first.second > spotDateSwap)
			{
				if (it->first.first < spotDateSwap)
				{
					dfSpot *= 1.0 / (1.0 + rate * dc.getTerm(it->first.first, spotDateSwap, false));
				}
			}
			else
			{
				dfSpot *= df_tmp;
			}
		}

		return dfSpot;
	}

	//
	// Helper Functions for State Variable Control
	// --------------------------------------------------------------------------------------------------

	/*
	@brief Function to evaluate the curve state variable requested, which can be a Zero Rate, Zero Rate times Time, Log DF or DF

	@param[in] zeroRate		        zero rate
	@param[in] accrualPeriod		accrualPeriod
	@param[in] stateVariableType    state variable: STATE_VARIABLE_ZERO_RATE, STATE_VARIABLE_ZERO_RATE_TIMES_TIME, STATE_VARIABLE_LOG_DF, STATE_VARIABLE_DF
	@return    returns the state variable requested, which can be a Zero Rate, Zero Rate times Time, Log DF or DF
	*/
	double getStateVariableValue(const double& zeroRate, const double& accrualPeriod, const StateVariableEnum& stateVariableType)
	{
		double result;

		switch (stateVariableType)
		{
		case STATE_VARIABLE_ZERO_RATE:
			result = zeroRate;
			break;

		case STATE_VARIABLE_ZERO_RATE_TIMES_TIME:
			result = zeroRate * accrualPeriod;
			break;

		case STATE_VARIABLE_LOG_DF:
			result = -zeroRate * accrualPeriod;
			break;

		case STATE_VARIABLE_DF:
			result = AQLMath::exp(-zeroRate * accrualPeriod);
			break;

		default:
			AQ_THROW("Invalid Yield Curve Interpolation State Variable");
			break;
		}

		return result;
	}

	/*
	@brief Function to evaluate the curve state variable requested

	@param[in] rateTime		rateTime
	@param[in] stateVariableType    stateVariableType
	@return    returns the state variable requested
	*/
	double getStateVariableValueFromDF(const double& discountFactor, const StateVariableEnum& stateVariableType)
	{
		double result;
		AQ_REQUIRE( discountFactor >= 0.0, "Invalid State Variable: Calibration Error, Discount Factors cannot be Negative")

		switch (stateVariableType)
		{
		case STATE_VARIABLE_ZERO_RATE_TIMES_TIME:
			result = -AQLMath::log(discountFactor);
			break;
		case STATE_VARIABLE_DF:
			result = discountFactor;
			break;
		case STATE_VARIABLE_LOG_DF:
			result = AQLMath::log(discountFactor);
			break;

		default:
			AQ_THROW("Invalid Yield Curve Interpolation State Variable");
			break;
		}
		return result;
	}

	/*
	@brief Function to evaluate the curve state variable requested, which can be STATE_VARIABLE_ZERO_RATE_TIMES_TIME, STATE_VARIABLE_DF

	@param[in] discountFactor		discountFactor
	@param[in] stateVariableType    stateVariableType
	@return    returns the state variable requested
	*/
	double getStateVariableValueFromRateTime(const double& rateTime, const StateVariableEnum& stateVariableType)
	{
		double result;

		switch (stateVariableType)
		{
		case STATE_VARIABLE_ZERO_RATE_TIMES_TIME:
			result = rateTime;
			break;
		case STATE_VARIABLE_DF:
			result = AQLMath::exp(-rateTime);
			break;

		default:
			AQ_THROW("Invalid Yield Curve Interpolation State Variable");
			break;
		}
		return result;
	}



	/*
	@brief Function to evaluate the curve state variable requested, which can be a Zero Rate, Zero Rate times Time, Log DF or DF

	@param[in] accrualTerm			The accrual term, not the date as a term
	@param[in] stateVariableValue   stateVariableValue
	@param[in] stateVariableType    state variable: STATE_VARIABLE_ZERO_RATE,
													STATE_VARIABLE_ZERO_RATE_TIMES_TIME,
													STATE_VARIABLE_LOG_DF,
													STATE_VARIABLE_DF

	@return    returns the state variable requested, which can be a Zero Rate, Zero Rate times Time, Log DF or DF
	*/
	double getZeroRateFromStateVariable( const double& accrualPeriod,
										 const double& stateVariableValue,
										 const StateVariableEnum& stateVariableType )
	{
		double zeroRate = 0.0;

		// Return a zero rate of 0.0 when term is zero
		const bool isTermZero = AQ_IS_EQUAL_ZERO(accrualPeriod);
		if (isTermZero)
		{
			return zeroRate;
		}

		switch (stateVariableType)
		{
		case STATE_VARIABLE_ZERO_RATE:
			zeroRate = stateVariableValue;
			break;
		case STATE_VARIABLE_ZERO_RATE_TIMES_TIME:
			zeroRate = stateVariableValue / accrualPeriod;
			break;
		case STATE_VARIABLE_LOG_DF:
			zeroRate = -stateVariableValue / accrualPeriod;
			break;
		case STATE_VARIABLE_DF:
			AQ_REQUIRE(AQ_IS_GREATER_THAN_OR_EQUAL_TO_ZERO(stateVariableValue), "Unable to calculate the zero rate from a negative discount factor")
			zeroRate = -AQLMath::log(stateVariableValue) / accrualPeriod;
			break;
		default:
			AQ_THROW("Invalid State Variable used in curve calibration")
				break;
		}

		return zeroRate;
	}


	/*
	@brief Function to interpolate for single discount factor applying the appropriate state variable conversion

	@param[in] paymentDateAsTerm	Payment Date as Term
	@param[in] interpolator		    interpolator class, this should already be set
	@param[in] stateVariableType	state variable: STATE_VARIABLE_ZERO_RATE,
													STATE_VARIABLE_ZERO_RATE_TIMES_TIME,
													STATE_VARIABLE_LOG_DF,
													STATE_VARIABLE_DF
	@param[in]  asOfDate			curve AsOfDate
	@param[in]  accrualDaycount		The accrual daycount basis to use e.g. ACT/360
	@param[in]  compoundFreq		The compounding frequency, defaults to SIMPLE
	@return    returns a single adiscount factor applying the appropriate state variable conversion
	*/
	double getInterpolatedDiscountfactor(const AQLInterpolationBase& interpolator,
										  const double& paymentDateAsTerm,
										  const StateVariableEnum& stateVariableType,
									      const AQLDate & asOfDate,
									      const DayCountEnum & accrualDaycount,
									      const CompoundingFrequencyEnum & compoundFreq )
	{
		double df = 1.0;

		// Return a Discount Factor of 1.0 when term is zero
		const bool isTermZero = AQ_IS_EQUAL_ZERO(paymentDateAsTerm);
		if (isTermZero)
		{
			return df;
		}

		switch (stateVariableType)
		{
		case STATE_VARIABLE_ZERO_RATE:
		{
			// Important!!
			// The forwardTerm here must be the converted from the internal ACT/365 daycount used for date transformation to a term in the curve daycount measure
			// --------------------------------------------------------------------------------------------------------------------------
			const double accrualTerm = accrualPeriod( 0.0, paymentDateAsTerm, asOfDate, accrualDaycount, compoundFreq );
			AQ_REQUIRE( AQ_IS_GREATER_THAN_ZERO( accrualTerm ), "Unable to imply Discount Factor - The fixing start date must be before the fixing end date" )
			
			df = AQLMath::exp(-interpolator.value(paymentDateAsTerm) * accrualTerm);
			break;
		}
		case STATE_VARIABLE_ZERO_RATE_TIMES_TIME:
		{
			df = AQLMath::exp(-interpolator.value(paymentDateAsTerm));
			break;
		}
		case STATE_VARIABLE_LOG_DF:
		{
			df = AQLMath::exp(interpolator.value(paymentDateAsTerm));
			break;
		}
		case STATE_VARIABLE_DF:
		{
			df = interpolator.value(paymentDateAsTerm);
			break;
		}
		default:
			AQ_THROW("Invalid Yield Curve Interpolation State Variable: Must be DF, LogDF, ZeroRate or ZeroRateTimesTime");
			break;
		}

		return df;
	}


	/*
	@brief Function to interpolate for a single forward rate applying the appropriate state variable conversion

	@param[in] fixingStartTerm		Fixing Start Date as Term
	@param[in] fixingEndTerm		Fixing End Date as Term
	@param[in] interpolator		    interpolator class, this should already be set
	@param[in] stateVariableType	state variable: STATE_VARIABLE_ZERO_RATE,
													STATE_VARIABLE_ZERO_RATE_TIMES_TIME,
													STATE_VARIABLE_LOG_DF,
													STATE_VARIABLE_DF
	@param[in]  asOfDate			curve AsOfDate
	@param[in]  accrualDaycount		The accrual daycount basis to use e.g. ACT/360
	@param[in]  compoundFreq		The compounding frequency, defaults to SIMPLE
	@return    returns a single forward rate applying the appropriate state variable conversion
	*/
	double getInterpolatedForwardRate(const double& fixingStartTerm,
									   const double& fixingEndTerm,
									   const AQLInterpolationBase& interpolator,
									   const StateVariableEnum& stateVariableType,
									   const AQLDate & asOfDate,
									   const DayCountEnum & accrualDaycount,
									   const CompoundingFrequencyEnum & compoundFreq )
	{
		double forwardRate = 0.0;
		double startDF = 1.0;
		double endDF = 1.0;

		const bool isStartTermZero = AQ_IS_EQUAL_ZERO(fixingStartTerm);
		const bool isEndTermZero = AQ_IS_EQUAL_ZERO(fixingEndTerm);

		// Important!!
		// The term here must be the converted from the internal ACT/365 daycount used for date transformation to a term in the curve daycount measure
		// --------------------------------------------------------------------------------------------------------------------------
		const double accrualTerm = accrualPeriod( fixingStartTerm, fixingEndTerm, asOfDate, accrualDaycount, compoundFreq );
        AQ_REQUIRE( AQ_IS_GREATER_THAN_ZERO( accrualTerm ), "Unable to imply Forward Rates(s) - The fixing start date must be before the fixing end date" )
        
		// Return a Forward Rate of zero when the forward term is zero
		const bool isAccrualTermZero = AQ_IS_EQUAL_ZERO(accrualTerm);
		if (isAccrualTermZero)
		{
			return forwardRate;
		}

		switch (stateVariableType)
		{
		case STATE_VARIABLE_ZERO_RATE:
		{
			const double startAccrualPeriod = accrualPeriod( 0.0, fixingStartTerm, asOfDate, accrualDaycount, compoundFreq );
			const double endAccrualPeriod	= accrualPeriod( 0.0, fixingEndTerm, asOfDate, accrualDaycount, compoundFreq );

			startDF = isStartTermZero ? 1.0 : AQLMath::exp(-interpolator.value(fixingStartTerm) * startAccrualPeriod);
			endDF = isEndTermZero ? 1.0 : AQLMath::exp(-interpolator.value(fixingEndTerm) * endAccrualPeriod);
			break;
		}
		case STATE_VARIABLE_ZERO_RATE_TIMES_TIME:
		{
			startDF = isStartTermZero ? 1.0 : AQLMath::exp(-interpolator.value(fixingStartTerm));
			endDF = isEndTermZero ? 1.0 : AQLMath::exp(-interpolator.value(fixingEndTerm));
			break;
		}
		case STATE_VARIABLE_LOG_DF:
		{
			startDF = isStartTermZero ? 1.0 : AQLMath::exp(interpolator.value(fixingStartTerm));
			endDF = isEndTermZero ? 1.0 : AQLMath::exp(interpolator.value(fixingEndTerm));
			break;
		}
		case STATE_VARIABLE_DF:
		{
			startDF = isStartTermZero ? 1.0 : interpolator.value(fixingStartTerm);
			endDF = isEndTermZero ? 1.0 : interpolator.value(fixingEndTerm);
			break;
		}
		default:
			AQ_THROW("Invalid Yield Curve Interpolation State Variable: Must be DF, LogDF, ZeroRate or ZeroRateTimesTime");
			break;
		}
		
		AQ_REQUIRE(!AQ_IS_EQUAL_ZERO(endDF), "Unable to calculate the Forward Rate. End Discount Factor is Zero")

		forwardRate = ((startDF / endDF) - 1) / accrualTerm;
		return forwardRate;
	}


	/*
	@brief Function to interpolate for single zero rate applying the appropriate state variable conversion

	@param[in] dateInTermFormat		The date as a year fraction term (ACT/365), not the accrual term
	@param[in] interpolator		    interpolator class, this should already be set
	@param[in] stateVariableType	state variable: STATE_VARIABLE_ZERO_RATE,
													STATE_VARIABLE_ZERO_RATE_TIMES_TIME,
													STATE_VARIABLE_LOG_DF,
													STATE_VARIABLE_DF
	@param[in]  asOfDate			curve AsOfDate
	@param[in]  accrualDaycount		The accrual daycount basis to use e.g. ACT/360
	@param[in]  compoundFreq		The compounding frequency, defaults to SIMPLE
	@return    returns a single zero rate applying the appropriate state variable conversion
	*/
	double getInterpolatedZeroRate( const double& dateInTermFormat,
									const AQLInterpolationBase* interpolator,
									const StateVariableEnum& stateVariableType,
									const AQLDate & asOfDate,
									const DayCountEnum & accrualDaycount,
									const CompoundingFrequencyEnum & compoundFreq )
	{
		double zeroRate = 0.0;

		// Return a zero rate of 0.0 when term is zero
		const bool isTermZero = AQ_IS_EQUAL_ZERO(dateInTermFormat);
		if (isTermZero)
		{
			return zeroRate;
		}

		// Important!!
		// The term here must be the converted from the internal ACT/365 daycount used for date transformation to a term in the curve daycount measure
		// --------------------------------------------------------------------------------------------------------------------------
		const double accrualTerm = accrualPeriod( 0.0, dateInTermFormat, asOfDate, accrualDaycount, compoundFreq );
        AQ_REQUIRE( AQ_IS_GREATER_THAN_ZERO( accrualTerm ), "Unable to imply Zero Rates(s) - The reset start date must be before the reset end date" )

		switch (stateVariableType)
		{
		case STATE_VARIABLE_ZERO_RATE:
			zeroRate = interpolator->value(accrualTerm);
			break;

		case STATE_VARIABLE_ZERO_RATE_TIMES_TIME:
			zeroRate = interpolator->value(dateInTermFormat) / accrualTerm;
			break;

		case STATE_VARIABLE_LOG_DF:
			zeroRate = -interpolator->value(dateInTermFormat) / accrualTerm;
			break;

		case STATE_VARIABLE_DF:
			zeroRate = -AQLMath::log( interpolator->value(dateInTermFormat) ) / accrualTerm;
			break;

		default:
			AQ_THROW("Invalid Yield Curve Interpolation State Variable: Must be DF, LogDF, ZeroRate or ZeroRateTimesTime");
			break;
		}

		return zeroRate;
	}

	// Method to get a discount factor given the state variable
	double getdiscountFactor( const double& paymentDateAsTerm,
							  const AQLInterpolationBase & interpolator,
							  const StateVariableEnum& stateVariableType,
							  const AQLDate & asOfDate,
							  const DayCountEnum & accrualDaycount,
							  const CompoundingFrequencyEnum & compoundFreq )
	{
		const double discountFactor = getInterpolatedDiscountfactor( interpolator,
																	 paymentDateAsTerm,
																	 stateVariableType,
																	 asOfDate,
																	 accrualDaycount,
																	 compoundFreq );
		return discountFactor;
	}

	// Method to get a forward rate given the state variable
	double getForwardRate( const double& fixingStartDateAsTerm,
						   const double& fixingEndDateAsTerm,
						   const AQLInterpolationBase & interpolator,
						   const StateVariableEnum& stateVariableType,
						   const AQLDate & asOfDate,
						   const DayCountEnum & accrualDaycount,
						   const CompoundingFrequencyEnum & compoundFreq )
	{
		const double forwardRate = getInterpolatedForwardRate( fixingStartDateAsTerm,
															   fixingEndDateAsTerm,
															   interpolator,
															   stateVariableType,
															   asOfDate,
															   accrualDaycount,
															   compoundFreq );
		return forwardRate;
	}

	bool isContiguousFraFuture(const AQLDate& spotDate, const AQLDate& startDate, const AQLString& liborIndexTerm, const bool isFuture)
	{
		// Normal by default, specially handling for 3M and 6M curve
		bool isContiguous = true;

		int refMonth = etrading::getFrequencyOrTenorMonth(liborIndexTerm);

		//Future: check month of the startDate
		if (isFuture)
		{
			int fStartMonth = startDate.monthOfYear();

			if (refMonth == 3)
			{
				//Check against 3M, 6M, 9M, 12M
				isContiguous = (fStartMonth == 3 || fStartMonth == 6 || fStartMonth == 9 || fStartMonth == 12);
			}
			else if (refMonth == 6)
			{
				//Check against 6M, 12M
				isContiguous = (fStartMonth == 6 || fStartMonth == 12);
			}
		}
		//FRA: check term between spotDate and startDate
		else
		{
			AQLPriceDataDayCount dc_act365(ACT_365);

			double startTerm = dc_act365.getTerm(spotDate, startDate);

			//A week's tolerance
			const double TERM_TOLERANCE = 0.02;

			if (refMonth == 3)
			{
				isContiguous = (std::fmod(startTerm, 0.25) < TERM_TOLERANCE);

			}
			else if (refMonth == 6)
			{
				isContiguous = (std::fmod(startTerm, 0.5) < TERM_TOLERANCE);
			}
		}

		return isContiguous;
	}


	/*
		@brief Insert DFs for Serial Fra/Futures by interpolating existing DFs and fwdRates

		@param[out] dfResults				dfResults
		@param[out] fwd_termsmtx			Matrix of fwd rate table (start terms and end terms)
		@param[out] fwds					A list of fwd rates
		@param[in]	serialFraFutures		A list of serial fra/futures
		@param[in]  asOfDate				curve AsOfDate
		@param[in]	spotdate				Spot date of the curve
		@param[in]	pInter_fw				Future/Fra interpolator
		@param[in]	liborIndexTerm			Reference rate term
		@param[in]	roll_conv				Roll convention
		@param[in]	sld						Sliding rule
		@param[in]	cal						Calendar
		@param[in]	dc						Trade day count
		@param[in]	dc_act365				dc_act365 day count
		@param[in]	stateVariable			State variable for future/fra
		@param[in]	populateFwdTable		True to populate fwd table
		@param[in]	dayIntervalTolerance	Tolerance between current future's endDate and next future's startDate, Default value is 7 days
	*/
	void insertDFs_SerialFuturesByDF( DiscountFactors & dfResults,
									  DoubleMatrix& fwd_termsmtx,
									  DoubleArray& fwds,
									  const std::vector<ForwardRate>& serialFraFutures,
									  const AQLDate & asOfDate,
									  const AQLDate& spotdate,
									  const AQLInterpolationBase& pInter_fw,
									  const AQLString& liborIndexTerm,
									  const AQLString& roll_conv,
									  const AQLPriceDataSlidingRule& sld,
									  const AQLPriceDataCalendar& cal,
									  const AQLPriceDataDayCount& dc,
									  const AQLPriceDataDayCount& dc_act365,
									  const StateVariableEnum& stateVariable,
									  const bool populateFwdTable,
									  const int dayIntervalTolerance)
	{

		size_t serialFraFutureSize = serialFraFutures.size();

		if (serialFraFutureSize == 0)
		{
			return;
		}

		// Parameter Marshalling
		const DayCountEnum accrualDaycount = dc.dayCountEnum();

		//1) Find the future chains, e.g. chain1: 1M (1Mx3M, 3Mx6M, etc.), chain2: 2M (2Mx4M, 4Mx7M, etc.), etc. 
		std::vector<std::vector<ForwardRate> > serialChainList;
		std::vector<ForwardRate> futureSerialChain;
		serialChainList.reserve(serialFraFutureSize);
		futureSerialChain.reserve(serialFraFutureSize);

		for (unsigned int i = 0; i < serialFraFutureSize; ++i)
		{
			const ForwardRate& curFuture = serialFraFutures[i];

			// Check if the startDate is already in one of the chain
			bool startDateFound = false;
			for (auto chain : serialChainList)
			{
				if (startDateFound)
				{
					break;
				}

				for (auto serialF : chain)
				{
					if (serialF.startDate == curFuture.startDate)
					{
						startDateFound = true;
						break;
					}
				}
			}

			//Add it if the start date is already in the Chain, skip it
			if (startDateFound)
			{
				break;
			}

			//New chain
			futureSerialChain.clear();
			futureSerialChain.push_back(curFuture);

			// If it's not the last future, add all related later futures to this chain
			if (i < serialFraFutureSize - 1)
			{
				//Add the related future to the chain
				for (unsigned int j = i + 1; j < serialFraFutureSize; ++j)
				{
					const ForwardRate& nextFuture = serialFraFutures[j];

					// If the latest future's endDate is the same as next future's start date, add the next future to the same chain
					if (std::abs(futureSerialChain.back().endDate.intervalDays(nextFuture.startDate)) <= dayIntervalTolerance)
					{
						futureSerialChain.push_back(nextFuture);
					}
				}
			}

			//Store the chain to the chainList
			serialChainList.push_back(futureSerialChain);
		}

		//We start from the serialChain with longer endDate because we are working backward, i.e. back out the last future's endDf from existing curve to the first future's startDF
		std::reverse(serialChainList.begin(), serialChainList.end());

		//2) Back out the startDFs of serial futures 

		std::unique_ptr<AQLInterpolationBase> df_inter(dynamic_cast<AQLInterpolationBase *>(pInter_fw.clone()));

		/**
		// Approach 2 : back out the last future's end DF from existing dfs and forward rates
		// df_2 is a Matrix of Terms and DFs
		DoubleVector dfs(df_2[1].size());	// Column 1: DiscountFactors, which we convert to logDF
		for (size_t j = 0; j < df_2[1].size(); ++j)
		{
			dfs[j] = etrading::getStateVariableValueFromDF(df_2[1][j], stateVariableFutureFra);
		}
		df_inter->set(df_2[0], dfs);

		std::unique_ptr<AQLInterpolationBase> fwdRate_inter(dynamic_cast<AQLInterpolationBase *>(pInter_fw.clone()));
		fwdRate_inter->set(fwd_termsmtx[0], fwds);
		*/

		for (auto serialChain : serialChainList)
		{
			/**
			// Approach 2 : back out the last future's end DF from existing dfs and forward rates
			AQLDate crossFutureStartDate = serialChain.back().endDate;
			double crossFutureStartTerm = dc_act365.getTerm(spotdate, crossFutureStartDate);

			AQLDate crossFutureEndDate = etrading::LADateHelpers::getDate(crossFutureStartDate, liborIndexTerm, sld, &cal, true, &roll_conv);
			double crossFutureEndTerm = dc_act365.getTerm(spotdate, crossFutureEndDate);
			double crossFutureTau = dc.getTerm(crossFutureStartDate, crossFutureEndDate);

			double interpFwdRate = fwdRate_inter->value(crossFutureStartTerm);
			double crossFutureEndDF = etrading::getInterpolatedDiscountfactor(*df_inter, crossFutureEndTerm, stateVariableFutureFra);
			double crossFutureStartDF = crossFutureEndDF * (interpFwdRate * crossFutureTau + 1.0);
			*/

			const int lastIndex = serialChain.size() - 1;
			for (int i = lastIndex; i >= 0; --i)
			{
				// df_2 is a Matrix of Terms and DFs
				DoubleVector dfs(dfResults.discountFactors_.size());
				for (size_t j = 0; j < dfResults.discountFactors_.size(); ++j)
				{
					dfs[j] = etrading::getStateVariableValueFromDF(dfResults.discountFactors_[j], stateVariable);
				}
				df_inter->set(dfResults.paymentDatesAsTerms_, dfs);

				const AQLDate& startDate = serialChain[i].startDate;
				AQLDate& endDate = serialChain[i].endDate;
				double fwdRate = serialChain[i].fwdRate;

				double startTerm = dc_act365.getTerm(spotdate, startDate);
				double endTerm = dc_act365.getTerm(spotdate, endDate);

				// Approach 2 : back out the last future's end DF from existing dfs and forward rates
				//double endDF = (i == lastIndex) ? crossFutureStartDF : etrading::getInterpolatedDiscountfactor( *df_inter, endTerm, stateVariableFutureFra, asOfDate, accrualDaycount, compoundingFreq );

				// Approach 1: back out the last future's end DF from existing stateVariable
				double endDF = etrading::getInterpolatedDiscountfactor( *df_inter,
																		endTerm,
																		stateVariable,
																		asOfDate,
																		accrualDaycount,
																		SIMPLE_COMPOUNDING );

				double tau = dc.getTerm(startDate, endDate);
				double startDF = endDF * (fwdRate * tau + 1.0);

				etrading::insertDFData(dfResults, startDF, startTerm, startDate);
				etrading::insertDFData(dfResults, endDF, endTerm, endDate);
				if (populateFwdTable)
				{
					etrading::insertForwardRateData(fwd_termsmtx, fwds, startTerm, endTerm, fwdRate);
				}

			}

		}
	}


	/*
		@brief Insert DFs for Futures with Serials by adjusting the quoted fwd rates

		@param[out] dfResults						dfResults
		@param[out] fwd_termsmtx					Matrix of fwd rate table (start terms and end terms)
		@param[out] fwds							A list of fwd rates
		@param[in]	futureStartDates				Future start dates
		@param[in]	futureEndDates					Future end dates
		@param[in]	futureRates						Future rates
		@param[in]  asOfDate						curve AsOfDate
		@param[in]	spotDateSwap					Swap spot date
		@param[in]	spotDateLibor					Libor spot date
		@param[in]	pInter_fw						Future/Fra interpolator
		@param[in]	dc								Trade day count
		@param[in]	dc_act365						dc_act365 day count
		@param[in]	stateVariableFutureFra			State variable for future/fra
		@param[in]	includeSwapsBeforeMPCSwaps		rue to call the includeSwapsBeforeMPCSwaps method
		@param[in]	data_libor						Libor data
		@param[in]	liborDate						Libor date
		@param[in]	populateFwdTable				True to populate fwd table
		@param[in]	dayIntervalTolerance			Tolerance between current future's endDate and next future's startDate
	*/
	void insertDFs_SerialFuturesByRate( DiscountFactors & dfResults,
										DoubleMatrix& fwd_termsmtx,
										DoubleArray& fwds,
										const DateVector& futureStartDates,
										const DateVector& futureEndDates,
										const DoubleVector& futureRates,
										const AQLDate & asOfDate,
										const AQLDate& spotDateSwap,
										const AQLDate& spotDateLibor,
										const AQLInterpolationBase& pInter_fw,
										const AQLPriceDataDayCount& dc,
										const AQLPriceDataDayCount& dc_act365,
										const StateVariableEnum& stateVariableFutureFra,
										const bool includeSwapsBeforeMPCSwaps,
										const AQLObject* data_libor,
										const AQLDate& liborDate,
										const bool populateFwdTable,
										const int dayIntervalTolerance )
	{

		size_t fSize = futureStartDates.size();

		// 1) Update futureEndDates to be adjacent dates
		DateVector futureAdjustedEndDates(fSize);

		for (unsigned int i = 0; i < fSize; ++i)
		{
			AQLDate endDate = futureEndDates[i];

			if (i < fSize - 1)
			{
				const AQLDate& nextStartDate = futureStartDates[i + 1];

				// Only adjust the endDate that caused due to serial futures, not due to days overlap
				const bool isInOverlapTolerance = (std::abs(endDate.intervalDays(nextStartDate)) <= dayIntervalTolerance);
				if (!isInOverlapTolerance && (endDate > nextStartDate))
				{
					endDate = nextStartDate;
				}
			}
			futureAdjustedEndDates[i] = endDate;
		}

		// 2) Update futureRates: implied rates based on the new future end dates

		DoubleVector futureAdjustedRates(fSize);

		// Start from the last future, work backward
		const int lastIndex = fSize - 1;

		for (int i = lastIndex; i >= 0; --i)
		{
			const double quotedFwdRate = futureRates[i];
			const AQLDate& startDate = futureStartDates[i];
			const AQLDate& quotedEndDate = futureEndDates[i];
			const AQLDate& endDate = futureAdjustedEndDates[i];

			double fwdRateToUse = quotedFwdRate;

			// When it's not last future AND there is an overlap between the current futureEndDate and next futureStartDate, calculate the 'impliedFwdRate' by:
			// DF(-quotedRate * (tau1+tau2)) = DF(-r1 * tau1) * DF (-r2*tau2), from MCM paper: "A Smoothing Algorithm for Short End of Yield Curve, Shaowen Shao, 2009"
			if (i < lastIndex && endDate < quotedEndDate)
			{
				double sumOfOverlappedFwdRateTimesTau = 0.0;

				// starting from the next index, find all the implied rates that within the current quotedStartDate/quotedEndDate range
				for (unsigned int j = i + 1; j < futureAdjustedRates.size(); ++j)
				{
					const double nextFwdRate = futureAdjustedRates[j];
					const AQLDate& nextStartDt = futureStartDates[j];
					const AQLDate& nextEndDt = futureAdjustedEndDates[j];

					const bool isInOverlapTolerance = (std::abs(quotedEndDate.intervalDays(nextStartDt)) <= dayIntervalTolerance);
					// Skip the difference due to days overlap
					if (isInOverlapTolerance)
					{
						continue;
					}

					if (quotedEndDate < nextStartDt)
					{
						break;
					}
					else if (quotedEndDate >= nextEndDt)
					{
						const double tau = dc.getTerm(nextStartDt, nextEndDt);
						sumOfOverlappedFwdRateTimesTau += nextFwdRate * tau;
					}
					else
					{
						// quotedEndDate < nextEndDt && quotedEndDate >= nextStartDt
						double tau = dc.getTerm(nextStartDt, quotedEndDate);
						sumOfOverlappedFwdRateTimesTau += nextFwdRate * tau;
					}
				}

				if (sumOfOverlappedFwdRateTimesTau > 0)
				{
					// update fwdRate
					const double currentTau = dc.getTerm(startDate, endDate);

					//impliedFwd_i = quotedFwd_i * quotedYearFraction_i - sumOf(impliedFwd_j * tau_j), where tau_j is the duration impliedFwd_j contributed to the quotedYearFraction_i  
					const double quotedYearFraction = dc.getTerm(startDate, quotedEndDate);

					fwdRateToUse = (quotedFwdRate * quotedYearFraction - sumOfOverlappedFwdRateTimesTau) / currentTau;
				}
			}

			futureAdjustedRates[i] = fwdRateToUse;

		}

		std::unique_ptr<AQLInterpolationBase> futureDFInterp(dynamic_cast<AQLInterpolationBase *>(pInter_fw.clone()));

		RateConvention rc = LAMathYieldCurve::setRC(AQ_SIMPLE);
		AQLPriceDataConvention conv(dc.getDayCount(), rc);
		DayCountEnum accrualDaycount = dc.dayCountEnum();

		// 3) Insert extra points due to the adjusted future end dates, i.e. endDF and forward tables.
		for (unsigned int i = 0; i < fSize; ++i)
		{
			const AQLDate& startDate = futureStartDates[i];
			const AQLDate& endDate = futureAdjustedEndDates[i];
			const double rate = futureAdjustedRates[i];

			// df_2 is a Matrix of Terms and DFs
			DoubleVector terms = dfResults.paymentDatesAsTerms_;
			DoubleVector logDFs(dfResults.discountFactors_.size());
			for (size_t j = 0; j < dfResults.discountFactors_.size(); ++j)
			{
				logDFs[j] = etrading::getStateVariableValueFromDF(dfResults.discountFactors_[j], stateVariableFutureFra);
			}
			futureDFInterp->set(terms, logDFs);

			// Calculate Discount Factors
			const double start_term = dc_act365.getTerm(spotDateSwap, startDate);
			const double end_term = dc_act365.getTerm(spotDateSwap, endDate);
			double startDF = etrading::getInterpolatedDiscountfactor( *futureDFInterp,
																	  start_term,
																	  stateVariableFutureFra,
																	  asOfDate,
																	  accrualDaycount,
																	  SIMPLE_COMPOUNDING );
			double endDF = startDF * conv.getDF(rate, startDate, endDate);

			// When startDate is earlier or equal to liborDate, need to adjust the startDF/endDF so there there is not kink
			bool OVERRIDE_DF = false;
			if (includeSwapsBeforeMPCSwaps && (startDate <= liborDate) && (liborDate < endDate))
			{
				std::unique_ptr<AQLInterpolationBase> cashDepositInterpolation(dynamic_cast<AQLInterpolationBase *>(pInter_fw.clone()));

				// Solve for the level of startDF and endDF so that they can imply a discount factor at liboDate that
				// allows for the libor instrument to be correctly repriced
				etrading::solveSmoothSTDShortEnd( startDF,
												  endDF,
												  startDate,
												  endDate,
												  spotDateLibor,
												  spotDateSwap,
												  liborDate,
												  data_libor,
												  cashDepositInterpolation,
												  stateVariableFutureFra,
												  dfResults,
												  rate,
												  conv );

				//Only allow override after smoothing
				OVERRIDE_DF = true;
			}


			etrading::insertDFData(dfResults, startDF, start_term, startDate, OVERRIDE_DF);
			etrading::insertDFData(dfResults, endDF, end_term, endDate, OVERRIDE_DF);
			if (populateFwdTable)
			{
				etrading::insertForwardRateData(fwd_termsmtx, fwds, start_term, end_term, rate);
			}

			const AQLDate& endQuotedDate = futureEndDates[i];
			if (endDate < endQuotedDate)
			{
				//*** Need to insert extra end points here to avoid the side effect of interpolating DFs
				const double rateQuoted = futureRates[i];
				const double endQuotedTerm = dc_act365.getTerm(spotDateSwap, endQuotedDate);
				const double endQuotedDF = startDF * conv.getDF(rateQuoted, startDate, endQuotedDate);

				etrading::insertDFData(dfResults, endQuotedDF, endQuotedTerm, endQuotedDate, OVERRIDE_DF);
				if (populateFwdTable)
				{
					etrading::insertForwardRateData(fwd_termsmtx, fwds, start_term, endQuotedTerm, rateQuoted);
				}
			}

		}
	}


	/*
	@brief insertDFs_ContiguousFutures

	@param[out] dfResults						Discount Factor Results
	@param[out] fwd_termsmtx					Matrix of fwd rate table (start terms and end terms)
	@param[out] fwds							A list of fwd rates
	@param[in]	data_libor						libor instrument data
	@param[in]	data_fraFuture					Fra/Future instrument data
	@param[in]	fraFutureStartDates				Fra/Future start dates
	@param[in]	fraFutureEndDates				Fra/Future end dates
	@param[in]	fraFutureRates					Fra/Future rates
	@param[in]  asOfDate						curve AsOfDate
	@param[in]	spotDateSwap					Swap Spot Date
	@param[in]	spotDateLibor					Libor Index Spot Date
	@param[in]	pInter_fw						Future/Fra interpolator
	@param[in]	liborIndexTerm					Libor Index Term
	@param[in]	roll_conv						Roll convention
	@param[in]	dc								Trade day count
	@param[in]	dc_act365						dc_act365 day count
	@param[in]	stateVariable					State variable for future/fra
	@param[in]	interpolationJoinDateAsDouble	interpolationJoinDateAsDouble
	@param[in]	liborDate						liborDate
	@param[in]	liborPos						liborPos
	@param[in]	firstSwapDate					Date of the first swap instrument after FRA/Future
	@param[in]	includeSwapsBeforeMPCSwaps		includeSwapsBeforeMPCSwaps
	@param[in]	applyTension					applyTension
	@param[in]	tensionGap						tensionGap
	@param[in]	isFuture						True to indicate it's a future
	@param[in]	populateFwdTable				True to populate fwd table
	*/
	void insertDFs_ContiguousFutures( DiscountFactors & dfResults,
									  DoubleMatrix& fwd_termsmtx,
									  DoubleArray& fwds,
									  const std::vector<AQLObject*>&  data_libor,
									  const std::vector<AQLObject*>& data_fraFuture,
									  const DateVector& fraFutureStartDates,
									  const DateVector& fraFutureEndDates,
									  const DoubleVector& fraFutureRates,
									  const AQLDate & asOfDate,
									  const AQLDate& spotDateSwap,
									  const AQLDate& spotDateLibor,
									  const AQLInterpolationBase& pInter_fw,
									  const AQLString& liborIndexTerm,
									  const AQLString& roll_conv,
									  const AQLPriceDataDayCount& dc,
									  const AQLPriceDataDayCount& dc_act365,
									  const StateVariableEnum& stateVariable,
									  const double interpolationJoinDateAsDouble,
									  const AQLDate& liborDate,
									  const int liborPos,
									  const AQLDate& firstSwapDate,
									  const bool includeSwapsBeforeMPCSwaps,
									  const bool applyTension,
									  const int tensionGap,
									  const bool isFuture,
									  const bool populateFwdTable )
	{
		// This struct stores interpolation data from bootstrapping routine needed within 'insertSyntheticTensionPoints'
		// Must be initialized outside the instrument data bootstrapping routine.
		etrading::tensionMarketData fraFuturesInstrumentData;

		// Interpolate on Futures on LogDF the same state variable as the Swaps
		std::unique_ptr<AQLInterpolationBase> logDF_FuturesInterpolation(dynamic_cast<AQLInterpolationBase *>(pInter_fw.clone()));
		logDF_FuturesInterpolation->setJoinDateAsDouble(interpolationJoinDateAsDouble);

		RateConvention rc = LAMathYieldCurve::setRC(AQ_SIMPLE);
		AQLPriceDataConvention conv(dc.getDayCount(), rc);
		DayCountEnum accrualDayCount = dc.dayCountEnum();

		// Contiguous futures: not serial futures
		size_t numberOfInstruments = fraFutureStartDates.size();

		for (unsigned int i = 0; i < numberOfInstruments; ++i)
		{
			// Use LogDF as StateVariable so Futures and Swap StateVariables are the Same
			DoubleVector terms = dfResults.paymentDatesAsTerms_;	
			DoubleVector logDFs(dfResults.discountFactors_.size());
			for (size_t j = 0; j < dfResults.discountFactors_.size(); ++j)
			{
				logDFs[j] = etrading::getStateVariableValueFromDF(dfResults.discountFactors_[j], stateVariable);
			}
			logDF_FuturesInterpolation->set(terms, logDFs);

			const AQLDate& startDate = fraFutureStartDates[i];
			const AQLDate& endDate = fraFutureEndDates[i];
			double rate = fraFutureRates[i];

			// Calculate Discount Factors
			// Interpolate on LogDF to be constistent with the Swap instrument section of this curve
			double start_term = dc_act365.getTerm(spotDateSwap, startDate);
			double end_term = dc_act365.getTerm(spotDateSwap, endDate);
			double startDF = etrading::getInterpolatedDiscountfactor( *logDF_FuturesInterpolation,
																	  start_term,
																	  stateVariable,
																	  asOfDate,
																	  accrualDayCount,
																	  SIMPLE_COMPOUNDING );
			double endDF = startDF * conv.getDF(rate, startDate, endDate);

			// When startDate is earlier or equal to liborDate, need to adjust the startDF/endDF so there there is not kink
			bool OVERRIDE_DF = false;
			if (i == 0 && includeSwapsBeforeMPCSwaps && (startDate <= liborDate))
			{
				std::unique_ptr<AQLInterpolationBase> cashDepositInterpolation(dynamic_cast<AQLInterpolationBase *>(pInter_fw.clone()));
				cashDepositInterpolation->setJoinDateAsDouble(interpolationJoinDateAsDouble);

				// Solve for the level of startDF and endDF that allows the libor instruments to be correctly repriced
				etrading::solveSmoothSTDShortEnd( startDF,
												  endDF,
												  startDate,
												  endDate,
												  spotDateLibor,
												  spotDateSwap,
												  liborDate,
												  data_libor[liborPos],
												  cashDepositInterpolation,
												  stateVariable,
												  dfResults,
												  rate,
												  conv );

				//Only allow override after smoothing
				OVERRIDE_DF = true;
			}

			etrading::insertDFData(dfResults, startDF, start_term, startDate, OVERRIDE_DF);
			etrading::insertDFData(dfResults, endDF, end_term, endDate, OVERRIDE_DF);

			
			if (populateFwdTable)
			{
				etrading::insertForwardRateData(fwd_termsmtx, fwds, start_term, end_term, rate);
			}

			// Apply Tension to the Futures part of the Curve by Linear Interpolating on Forwards to Create Synthetic Points
			// -------------------------------------------------------------------------------------------------------------
			bool indexInRange = true;
			if (isFuture)
			{
				const unsigned int dontApplyTensionToLastNFutures = 2;
				indexInRange = (i < numberOfInstruments - dontApplyTensionToLastNFutures);
			}

			if (applyTension && numberOfInstruments > 1 && indexInRange)
			{
				// Use LogDF as StateVariable so Futures and Swaps StateVariables are the Same
				DoubleVector terms = dfResults.paymentDatesAsTerms_;
				DoubleVector logDFs(dfResults.discountFactors_.size());
				for (size_t i = 0; i <dfResults.discountFactors_.size(); ++i)
				{
					logDFs[i] = etrading::getStateVariableValueFromDF(dfResults.discountFactors_[i], stateVariable);
				}
				logDF_FuturesInterpolation->set(terms, logDFs);

				bool isFirstDataPoint = true;
				if (i == 0)
				{
					for (unsigned int i = 0; i < data_libor.size(); i++)
					{
						const AQLString& term_str = dynamic_cast<const AQLDataString&> ((data_libor[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
						if (term_str != liborIndexTerm)
						{
							continue;
						}
						isFirstDataPoint = false;

						AQLString freq = dynamic_cast<const AQLDataString&> ((data_libor[i]->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL)).get()).get();
						freq.toUpper();
						double rate = dynamic_cast<const AQLDataDouble&> ((data_libor[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
						const AQLPriceDataCalendar& cal = dynamic_cast<const AQLPriceDataCalendar&> ((data_libor[i]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
						const AQLPriceDataSlidingRule& sld = dynamic_cast<const AQLPriceDataSlidingRule&> ((data_libor[i]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
						const AQLPriceDataDayCount& dc = dynamic_cast<const AQLPriceDataDayCount&> ((data_libor[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
						bool isEOMRoll = false;
						const AQLDataHolder *dh = &(data_libor[i]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK));
						if (dh->isDefined() && !dh->isNull())
						{
							isEOMRoll = dynamic_cast<const AQLDataBool &>(dh->get()).get();
						}

						// set roll convention
						AQLString roll_conv = getRollConv(freq, isEOMRoll);

						AQLDate endDate = etrading::LADateHelpers::getDate(spotDateLibor, term_str, sld, &cal, true, &roll_conv);

						// Use the money market LIBOR rate as the first data point
						// Note: Must update termsDFMatrix at end of routine
						insertSyntheticTensionPoints( dfResults,
													  fwds,
													  fwd_termsmtx,
													  logDF_FuturesInterpolation.get(),
													  stateVariable,
													  asOfDate,
													  spotDateSwap,
													  spotDateLibor,
													  endDate,
													  rate,
													  true, // isFirstDataPoint = true
													  dc_act365,
													  dc.getDayCount(),
													  tensionGap,
													  fraFuturesInstrumentData,
													  firstSwapDate,
													  populateFwdTable );
						
					}
				}

				// Must update termsDFMatrix at end of routine
				insertSyntheticTensionPoints( dfResults,
											  fwds,
											  fwd_termsmtx,
											  logDF_FuturesInterpolation.get(),
											  stateVariable,
											  asOfDate,
											  spotDateSwap,
											  startDate,
											  endDate,
											  rate,
											  (i == 0 && isFirstDataPoint) ? true : false, // isFirstDataPoint
											  dc_act365,
											  dc.getDayCount(),
											  tensionGap,
											  fraFuturesInstrumentData,
											  firstSwapDate,
											  populateFwdTable);
			}
			// -------------------------------------------------------------------------------------------------------------

			// For Analytical Risk
			DateVector fraFutureCalcDates(2);
			fraFutureCalcDates[0] = startDate; fraFutureCalcDates[1] = endDate;
			if (!data_fraFuture[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, NOCHECK).isDefined())
			{
				data_fraFuture[i]->add(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, new AQLDataDates(fraFutureCalcDates));
			}
			else
			{
				dynamic_cast<AQLDataDates&>(data_fraFuture[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO).get()).set(fraFutureCalcDates);
			}
		}
	}


	/*
	@brief  get rollConvention (LUNAR, EOM, NORMAL)

	@param[in]	freq	Frequency
	@param[in]	isEOMRoll	    End of the Month flag
	*/
	AQLString getRollConv(const AQLString& freq, const bool isEOMRoll)
	{
		AQLString roll_conv("");

		if (freq == AQ_LUNAR)
		{
			roll_conv = AQ_ROLLCONV_LUNAR;
		}
		else if (isEOMRoll)
		{
			roll_conv = AQ_ROLLCONV_EOM;
		}
		else
		{
			roll_conv = AQ_ROLLCONV_NORMAL;
		}
		return roll_conv;

	}

	/*
	@brief Bootstrap Futures

	@param[out] dfResults						dfResults
	@param[out] fwd_termsmtx					Matrix of fwd rate table (start terms and end terms)
	@param[out] fwds							A list of fwd rates
	@param[in]	futuresType						futuresType
	@param[in]	serialFraFutures				Serial fra/futures for the method when SerialCalcType is DF
	@param[in]	data_libor						libor instrument data
	@param[in]	data_fraFuture					Fra/Future instrument data
	@param[in]	fraFutureStartDates				Fra/Future start dates
	@param[in]	fraFutureEndDates				Fra/Future end dates
	@param[in]	fraFutureRates					Fra/Future rates
	@param[in]  asOfDate						curve AsOfDate
	@param[in]	spotDateSwap					Swap Spot Date
	@param[in]	spotDateLibor					Libor Index Spot Date
	@param[in]	pInter_fw						Future/Fra interpolator
	@param[in]	liborIndexTerm					Libor Index Term
	@param[in]	roll_conv						Roll convention
	@param[in]	dc								Trade day count
	@param[in]	dc_act365						dc_act365 day count
	@param[in]	stateVariable					State variable for future/fra
	@param[in]	interpolationJoinDateAsDouble	interpolationJoinDateAsDouble
	@param[in]	liborDate						liborDate
	@param[in]	liborPos						liborPos
	@param[in]	includeSwapsBeforeMPCSwaps		includeSwapsBeforeMPCSwaps
	@param[in]	applyTension					applyTension
	@param[in]	tensionGap						tensionGap
	@param[in]	isFuture						True to indicate it's a future
	@param[in]	populateFwdTable				True to populate fwd table
	*/
	void bootstrapFuturesOrFRAs( DiscountFactors & dfResults,
								  DoubleMatrix& fwd_termsmtx,
								  DoubleArray& fwds,
								  const etrading::FuturesTypeEnum& futuresType,
								  const std::vector<ForwardRate>& serialFraFutures,
								  const std::vector<AQLObject*>&  data_libor,
								  const std::vector<AQLObject*>& data_fraFuture,
								  const DateVector& fraFutureStartDates,
								  const DateVector& fraFutureEndDates,
								  const DoubleVector& fraFutureRates,
								  const AQLDate & asOfDate,
								  const AQLDate& spotDateSwap,
								  const AQLDate& spotDateLibor,
								  const AQLInterpolationBase& pInter_fw,
								  const AQLString& liborIndexTerm,
								  const AQLString& roll_conv,
								  const AQLPriceDataSlidingRule& sld,
								  const AQLPriceDataCalendar& cal,
								  const AQLPriceDataDayCount& dc,
								  const AQLPriceDataDayCount& dc_act365,
								  const StateVariableEnum& stateVariable,
								  const double interpolationJoinDateAsDouble,
								  const AQLDate& liborDate,
								  const int liborPos,
								  const AQLDate& firstSwapDate,
								  const bool includeSwapsBeforeMPCSwaps,
								  const bool applyTension,
								  const int tensionGap,
								  const bool isFuture,
								  const bool populateFwdTable)
	{
		switch (futuresType)
		{
			case etrading::CONTIGUOUS_FUTURES:
			{
				// Regular futures
				etrading::insertDFs_ContiguousFutures(dfResults, fwd_termsmtx, fwds, data_libor, data_fraFuture, fraFutureStartDates, fraFutureEndDates, fraFutureRates, asOfDate, spotDateSwap, spotDateLibor, pInter_fw,
					liborIndexTerm, roll_conv, dc, dc_act365, stateVariable, interpolationJoinDateAsDouble, liborDate, liborPos, firstSwapDate, includeSwapsBeforeMPCSwaps, applyTension, tensionGap, isFuture, populateFwdTable);
				break;
			}
			case etrading::SERIAL_FUTURES_BY_RATE:
			{
				// When the SerialCalcType is RATE, adjusted the future end dates and fwdRates
				insertDFs_SerialFuturesByRate(dfResults, fwd_termsmtx, fwds, fraFutureStartDates, fraFutureEndDates, fraFutureRates, asOfDate, spotDateSwap, spotDateLibor, pInter_fw, dc, dc_act365, stateVariable, includeSwapsBeforeMPCSwaps, data_libor[liborPos], liborDate, populateFwdTable);
				break;
			}
			case etrading::SERIAL_FUTURES_BY_DF:
			{
				// When the SerialCalcType is DF, two steps:

				// 1) call the regular future routine to build the curve
				insertDFs_ContiguousFutures(dfResults, fwd_termsmtx, fwds, data_libor, data_fraFuture, fraFutureStartDates, fraFutureEndDates, fraFutureRates, asOfDate, spotDateSwap, spotDateLibor, pInter_fw,
					liborIndexTerm, roll_conv, dc, dc_act365, stateVariable, interpolationJoinDateAsDouble, liborDate, liborPos, firstSwapDate, includeSwapsBeforeMPCSwaps, false /** applyTensionFutures */, tensionGap, isFuture, populateFwdTable);

				// 2) back out the serial future's start/end DFs using existing curve's DFs and fwdRates
				insertDFs_SerialFuturesByDF(dfResults, fwd_termsmtx, fwds, serialFraFutures, asOfDate, spotDateSwap, pInter_fw, liborIndexTerm, roll_conv, sld, cal, dc, dc_act365, stateVariable, populateFwdTable);

				break;
			}
			default:
			{
				throw etrading::ETradingException("#Error: Invalid Serial Calulation Type, must be 'CONTIGUOUS', 'RATE' or 'DF'");
				break;
			}
		}

	}

	/*
	@brief Populate start/end dates and rates for Fra/Futures

	@param[out]	fraFutureStartDates		Start dates of Fra/Futures
	@param[out]	fraFutureEndDates		End dates of Fra/Futures
	@param[out]	fraFutureRates			Rates of Fra/Futures
	@param[out]	serialFraFutures		Serial fra/futures for the method when SerialCalcType is DF
	@param[out]	allContiguous			True to indicate all the Fra/Futures are in contiguous months
	@param[in]	futuresType				futuresType
	@param[in]	liborIndexTerm			Reference rate term
	@param[in]	spotDate				Spot date of the curve
	@param[in]	startDate				StartDate of Fra/Future
	@param[in]	endDate					EndDate of Fra/Future
	@param[in]	fwdRate					Forward rate of Fra/Future
	@param[in]	isFuture				True to indicate it's a future
	*/
	void populateFraFutureData(DateVector& fraFutureStartDates,
							   DateVector& fraFutureEndDates,
							   DoubleVector& fraFutureRates,
							   std::vector<ForwardRate>& serialFraFutures,
							   bool& allContiguous,
							   const etrading::FuturesTypeEnum& futuresType,
							   const AQLString& liborIndexTerm,
							   const AQLDate& spotDate,
							   const AQLDate& startDate,
							   const AQLDate& endDate,
							   const double fwdRate,
							   const bool isFuture)
	{
		if (!etrading::isContiguousFraFuture(spotDate, startDate, liborIndexTerm, isFuture))
		{
			allContiguous = false;

			//When using DF Calc method, only save the Contiguous into futureStartDates, futureEndDates, futureRates
			if (futuresType == etrading::SERIAL_FUTURES_BY_DF)
			{
				ForwardRate serialQuote;
				serialQuote.startDate = startDate;
				serialQuote.endDate = endDate;
				serialQuote.fwdRate = fwdRate;
				serialFraFutures.push_back(serialQuote);
			}
			else
			{
				fraFutureStartDates.push_back(startDate);
				fraFutureEndDates.push_back(endDate);
				fraFutureRates.push_back(fwdRate);
			}
		}
		else
		{
			fraFutureStartDates.push_back(startDate);
			fraFutureEndDates.push_back(endDate);
			fraFutureRates.push_back(fwdRate);
		}
	}
	// Method to populate MarketQuotes 
	void updateMarketQuotes( ForwardQuotes& marketQuotes,
							 bool& areAllForwardsContiguous,
							 const etrading::FuturesTypeEnum& futuresType,
							 const AQLString& liborIndexTerm,
							 const AQLDate& spotDate,
							 const AQLDate& startDate,
							 const AQLDate& endDate,
							 const double fwdRate,
							 const bool isFuture )
	{
		marketQuotes.startDates_.push_back( startDate );
		marketQuotes.endDates_.push_back( endDate );
		marketQuotes.marketRates_.push_back( fwdRate );
		areAllForwardsContiguous = etrading::isContiguousFraFuture(spotDate, startDate, liborIndexTerm, isFuture);
	}


	/*
	@brief Helper function to populate Central bank swap rates and future rates

	@param[inout]	mpcSwapRates	Central bank/FF swap rates
	@param[inout]	futureRates		future rates
	@param[in]	instrumentType	OISMidTermInstrumentType
	@param[in]	startDate		StartDate of Fra/Future
	@param[in]	endDate			EndDate of Fra/Future
	@param[in]	fwdRate			Forward rate of Fra/Future
	*/
	void populateOISMidInstrumentRate(std::map<AQLDate, std::pair<AQLDate, double>>& mpcSwapRates,
									std::map<AQLDate, std::pair<AQLDate, double>>& futureRates,
									const OISMidTermInstrumentsEnum& instrumentType,
									const AQLDate& startDate,
									const AQLDate& endDate,
									const double fwdRate)
	{

		if (instrumentType == etrading::CENTRAL_BANK_SWAP || instrumentType == etrading::FED_FUND_FUTURE)
		{
			mpcSwapRates[startDate] = std::make_pair(endDate, fwdRate);
		}
		else if (instrumentType == etrading::ONE_MONTH_FUTURE || instrumentType == etrading::THREE_MONTH_FUTURE)
		{
			futureRates[startDate] = std::make_pair(endDate, fwdRate);
		}
	}


	/*
	@brief Insert daily zero rates for the from last short term swap to start date of first mid term instrument (which can be Central Bank Swap/FF/future)
	*/
	void insertZeroRatesFromShortTermSwapToMidInstrument(DoubleVector& grid,
														DoubleVector& yields,
														std::vector<AQLDate>& dates,
														const AQLDate& lastShortTermSwapDate,
														const double lastShortTermSwapRate,
														const std::map<AQLDate, std::pair<AQLDate, double>>& mpcSwapRates,
														const std::map<AQLDate, std::pair<AQLDate, double>>& futureRates,
														const AQLDate& spotdate,
														const AQLPriceDataDayCount& dc_act365,
														const AQLPriceDataCalendar& cal,
														const AQLPriceDataConvention& conv)
	{

		const bool hasCentralBank = (mpcSwapRates.size() > 0);
		const bool hasFuture = (futureRates.size() > 0);

		const AQLDate& firstFutureStart = hasFuture ? futureRates.begin()->first : AQLDate();
		const AQLDate& firstCentralBankStart = hasCentralBank ? mpcSwapRates.begin()->first : AQLDate();

		AQLDate firstMidInstrumentStartDt;
		double firstMidInstrumentRate = 0.0;

		// With Central Bank and Future
		if (hasCentralBank && hasFuture)
		{
			firstMidInstrumentStartDt = (firstCentralBankStart < firstFutureStart) ? firstCentralBankStart : firstFutureStart;
			firstMidInstrumentRate = (firstCentralBankStart < firstFutureStart) ? mpcSwapRates.at(firstMidInstrumentStartDt).second : futureRates.at(firstMidInstrumentStartDt).second;
		}
		// No Future
		else if (hasCentralBank)
		{
			firstMidInstrumentStartDt = firstCentralBankStart;
			firstMidInstrumentRate = mpcSwapRates.at(firstMidInstrumentStartDt).second;
		}
		// No Central Bank
		else if (hasFuture)
		{
			firstMidInstrumentStartDt = firstFutureStart;
			firstMidInstrumentRate = futureRates.at(firstMidInstrumentStartDt).second;
		}


		double gapTerm = firstMidInstrumentStartDt.intervalDays(lastShortTermSwapDate);
		double rateIncrement = (firstMidInstrumentRate - lastShortTermSwapRate) / gapTerm;

		AQLDate tmp_date = lastShortTermSwapDate;

		//Initial DF
		double df = AQLMath::exp(-yields.back() * grid.back());

		while (tmp_date < firstMidInstrumentStartDt)
		{
			AQLDate n_date = cal.getBusinessDay(tmp_date, 1);
			const double endTerm = dc_act365.getTerm(spotdate, n_date);

			//Directly linear interpolation on forward rate, between LastSwapRate and the first ECB/ArrFuture
			double instanteneousFwdRate = (n_date.intervalDays(lastShortTermSwapDate)) * rateIncrement + lastShortTermSwapRate;

			df *= conv.getDF(instanteneousFwdRate, tmp_date, n_date);

			grid.push_back(endTerm);
			yields.push_back(-AQLMath::log(df) / endTerm);
			dates.push_back(n_date);

			tmp_date = n_date;
		}

	}

	/*
	@brief Populate Zero Rates for OIS Mid term instruments (CentralBanks, FedFunds, Futures)
	*/
	void insertZeroRatesForOISMidInstruments(DoubleVector& grid,
											DoubleVector& yields,
											DateVector& dates,
											const AQLDate& spotdate,
											const AQLDate& centralBankFromDate,
											const AQLDate& centralBankShortTermDate,
											const double initialDF,
											const std::map<AQLDate, std::pair<AQLDate, double>>& mpcSwapRates,
											const std::map<AQLDate, std::pair<AQLDate, double>>& futureRates,
											const std::unique_ptr<AQLInterpolationBase>& pInter_yield,
											const AQLPriceDataDayCount& dc_act365,
											const AQLPriceDataCalendar& cal,
											const AQLPriceDataConvention& conv,
											const bool isARRCurve,
											const double lastShortSwapTerm)

	{

		const bool hasCentralBank = (mpcSwapRates.size() > 0);
		const bool hasFuture = (futureRates.size() > 0);

		const AQLDate& firstFutureStart = hasFuture ? futureRates.begin()->first : AQLDate();
		const AQLDate& firstCentralBankStart = hasCentralBank ? mpcSwapRates.begin()->first : AQLDate();

		if (hasCentralBank && hasFuture)
		{

			if (firstCentralBankStart < firstFutureStart)
			{
				//CentralBank
				insertDailyZeroRatesForCentralBank(grid, yields, dates, centralBankFromDate/*fromDate*/, spotdate, centralBankShortTermDate, mpcSwapRates, initialDF, dc_act365, cal, conv, lastShortSwapTerm);

				//Future
				insertZeroRatesForFutures(grid, yields, dates, spotdate, pInter_yield, futureRates, dc_act365, conv, cal, isARRCurve);
			}
			else
			{
				//Future

				// If there is no instrument before, we need to firstly boostrap the first rate using the first future forward rate
				if (yields.size() < 2)
				{
					insertDailyZeroRatesByConstantFwdRate(grid,
													yields,
													dates,
													spotdate,
													spotdate, //fromDate
													firstFutureStart,
													futureRates.at(firstFutureStart).second,
													1.0, //initialDF
													dc_act365,
													cal,
													conv);

				}

				insertZeroRatesForFutures(grid, yields, dates, spotdate, pInter_yield, futureRates, dc_act365, conv, cal, isARRCurve);

				//CentralBank
				const AQLDate centralBankFromDateToUse = (centralBankFromDate < firstCentralBankStart) ? firstCentralBankStart : centralBankFromDate;

				// Calculate the initial DF for Central bank section, based on the latest yield
				const double initialDFToUse = AQLMath::exp(-yields.back() * grid.back());

				insertDailyZeroRatesForCentralBank(grid, yields, dates, centralBankFromDateToUse/*fromDate*/, spotdate, centralBankShortTermDate, mpcSwapRates, initialDFToUse, dc_act365, cal, conv, lastShortSwapTerm);

			}

		}
		else if (hasCentralBank)
		{
			//CentralBank
			insertDailyZeroRatesForCentralBank(grid, yields, dates, centralBankFromDate/*fromDate*/, spotdate, centralBankShortTermDate, mpcSwapRates, initialDF, dc_act365, cal, conv, lastShortSwapTerm);

		}
		else if (hasFuture)
		{

			// Future

			// If there is no instrument before, we need to firstly boostrap the first rate using the first future forward rate
			if (yields.size() < 2)
			{
				insertDailyZeroRatesByConstantFwdRate(grid,
												yields,
												dates,
												spotdate,
												spotdate, //fromDate
												firstFutureStart,
												futureRates.at(firstFutureStart).second,
												1.0, //initialDF
												dc_act365,
												cal,
												conv);
			}

			insertZeroRatesForFutures(grid, yields, dates, spotdate, pInter_yield, futureRates, dc_act365, conv, cal, isARRCurve);

		}
	}


	/*
	@brief Insert DF to the DiscountFactor table for fixing start date and fixing end dates
	
	@param[inout] df_2
	@param[inout] df_2_date
	@param[in] inter
	@param[in] stateVariable
	@param[in] fixingStartTerms
	@param[in] fixingEndTerms
	@param[in] fixingStartDates
	@param[in] fixingEndDates
	@param[in] generateForwardsFromSwapsOnly
	@param[in] asOfDate
	@param[in] accrualDaycount
	*/
	void insertDFsForFixingStartEnds(DoubleMatrix& df_2,
									DateVector& df_2_date, 
									const AQLInterpolationBase &inter,
									const StateVariableEnum& stateVariable, 
									const DoubleArray &fixingStartTerms,
									const DoubleArray &fixingEndTerms,
									const DateVector &fixingStartDates,
									const DateVector &fixingEndDates,
									bool generateForwardsFromSwapsOnly, 
									const AQLDate & asOfDate, 
									const DayCountEnum & accrualDaycount)
	{
		for (unsigned int i = 0; i < fixingStartTerms.size(); ++i)
		{
			const double fixingStartDF = etrading::getInterpolatedDiscountfactor(inter, fixingStartTerms[i], stateVariable, asOfDate, accrualDaycount);
			const double fixingEndDF = etrading::getInterpolatedDiscountfactor(inter, fixingEndTerms[i], stateVariable, asOfDate, accrualDaycount);

			etrading::insertDFData(df_2, df_2_date, fixingStartDF, fixingStartTerms[i], fixingStartDates[i], generateForwardsFromSwapsOnly);
			etrading::insertDFData(df_2, df_2_date, fixingEndDF, fixingEndTerms[i], fixingEndDates[i], generateForwardsFromSwapsOnly);
		}
	}

	/*
	@brief Insert DF to the DiscountFactor table for fixing start date and fixing end dates
	
	@param[out] dfResults
	@param[in] inter
	@param[in] stateVariable
	@param[in] fixingStartTerms
	@param[in] fixingEndTerms
	@param[in] fixingStartDates
	@param[in] fixingEndDates
	@param[in] generateForwardsFromSwapsOnly
	@param[in] asOfDate
	@param[in] accrualDaycount
	*/
	void insertDFsForFixingStartEnds( DiscountFactors & dfResults,
									  const AQLInterpolationBase &inter,
									  const StateVariableEnum& stateVariable,
									  const DoubleArray &fixingStartTerms,
									  const DoubleArray &fixingEndTerms,
									  const DateVector &fixingStartDates,
									  const DateVector &fixingEndDates,
									  bool generateForwardsFromSwapsOnly,
									  const AQLDate & asOfDate,
									  const DayCountEnum & accrualDaycount )
	{
		for (unsigned int i = 0; i < fixingStartTerms.size(); ++i)
		{
			const double fixingStartDF = etrading::getInterpolatedDiscountfactor(inter, fixingStartTerms[i], stateVariable, asOfDate, accrualDaycount);
			const double fixingEndDF = etrading::getInterpolatedDiscountfactor(inter, fixingEndTerms[i], stateVariable, asOfDate, accrualDaycount);

			etrading::insertDFData(dfResults, fixingStartDF, fixingStartTerms[i], fixingStartDates[i], generateForwardsFromSwapsOnly);
			etrading::insertDFData(dfResults, fixingEndDF, fixingEndTerms[i], fixingEndDates[i], generateForwardsFromSwapsOnly);
		}
	}

	/*
	@brief Remove duplicated terms based on the specified tolerance,  tolerance default to 1 day.
	*/
	void removeDuplicatedTerms(DoubleVector& terms, const double TERM_TOLERANCE)
	{
		terms.erase(unique(terms.begin(), terms.end(), [TERM_TOLERANCE](const double a, const double b) { return std::abs(a - b) < TERM_TOLERANCE; }), terms.end());
	}

}