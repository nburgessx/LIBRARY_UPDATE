/*
 * @brief			Component curve used in the global curve calibration engine
 * @Created:		16 Jan 2018
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */


#include "SwapComponentCurve.h"
#include "LACurvePricingObject.h"
#include "LAMathDefine.h"
#include "LAInterpolationBase.h"
#include "LAPriceDataInterpolation.h"
#include "LADataMultiReference.h"
#include "LAPriceDataDayCount.h"
#include "LAPriceDataSlidingRule.h"
#include "LADataVector.h"
#include "LADataMatrix.h"
#include "LAPriceDataCalendar.h"
#include "LADataHolder.h"
#include "LAObject.h"
#include "LAObjectHolder.h"
#include "LADataReference.h"
#include "LADate.h"
#include "LABasic.h"
#include "LACoreTemplateType.h"
#include "LAAlgorithm.h"
#include "CurveInstruments.h"
#include "LALinearInterpolation.h"
#include "LALinearSplineInterpolation.h"
#include "LALinearMonotoneSplineInterpolation.h"
#include "ConstantDeclarations.h"
#include "LADateScheduleHelpers.h"
#include "LADateHelpers.h"
#include "ExceptionMacros.h"

const int RATE_PRIORITY_PROXIMITY_DAY_TOLERANCE = 20;

using etrading::DiscountFactors;
using etrading::bootstrapLibors;
using etrading::optimizeInterpolationJoinDate;

/*
    @brief Constructor
*/
SwapComponentCurve::SwapComponentCurve(const LAString& curveName,
	 								const CurveCalibrationData& curveCalibrationData,
									const LADate& baseDate,
									const bool& fastRebuildRequested)
						: GlobalCalibrationComponentCurve(curveName, baseDate, fastRebuildRequested),
						  isSwapTenorAdjust_(false), is_f_use_(false), is_fra_use_(false),
						  isMultiCurve_(true),
						  interpolationStr_(""), 
						  interpolationYGStr_("")
{
	LAString suffix_mkt = (curveName_ == STD) ? "" : "_" + curveName_;
	suffix_mkt.toUpper();

	// get market data
	data_.clear();
	const LADataMultiReference& mr = dynamic_cast<const LADataMultiReference&>(curveCalibrationData.getData(CALIBRATION_DATA_MARKETDATA + suffix_mkt, ISNOTNULL).get());	
	for(unsigned int j = 0; j < mr.getSize(); j++)
	{
		data_.push_back(&mr.get(j).get());
	}
	
	// get isSwapTenorAdjust
	const LADataHolder *dh = &curveCalibrationData.getData(IR_CALIBRATION_DATA_ISSWAPTENORADJUST + suffix_mkt, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		isSwapTenorAdjust_ = dynamic_cast<const LADataBool&>(dh->get());
	}

	if (isSwapTenorAdjust_)
	{
		LAString tenorSwapName = dynamic_cast<const LADataString&> (curveCalibrationData.getData(IR_CALIBRATION_DATA_TENORSWAPNAME, ISNOTNULL).get());
		tenorSwapName.toUpper();
		const LADataMultiReference& mr_tenorSwap = dynamic_cast<const LADataMultiReference&> (curveCalibrationData.getData(CALIBRATION_DATA_MARKETDATA + LAString("_") + tenorSwapName, ISNOTNULL).get());
		for(unsigned int i = 0; i < mr_tenorSwap.getSize(); i++)
		{
			data_.push_back(&mr_tenorSwap.get(i).get());
		}
	}

	// get IsFutureUse
	dh = &curveCalibrationData.getData(IR_CALIBRATION_DATA_ISFUTUREUSE + suffix_mkt, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		is_f_use_ = dynamic_cast<const LADataBool&>(dh->get());
	}

	// get IsFRAUse
	dh = &curveCalibrationData.getData(IR_CALIBRATION_DATA_ISFRAUSE + suffix_mkt, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		is_fra_use_ = dynamic_cast<const LADataBool&>(dh->get());
	}

	// get Interpolation	
	dh = &curveCalibrationData.getData(CALIBRATION_DATA_INTERPOLATION + suffix_mkt, NOCHECK);	
	pInter_.reset();
	if (dh->isDefined() && !dh->isNull())
	{
		LAPriceDataInterpolation tempInterp = dynamic_cast<const LAPriceDataInterpolation &>(dh->get());
		interpolationStr_ = tempInterp.convertToString();
		pInter_ = std::shared_ptr<LAInterpolationBase> (dynamic_cast<LAInterpolationBase*>(tempInterp.getMethod().clone()));
	}
	
	// get YieldGen Interpolation	
	dh = &curveCalibrationData.getData(IR_CALIBRATION_DATA_INTERPOLATIONYG + suffix_mkt, NOCHECK);	
	pInter_yg_.reset();
	if (dh->isDefined() && !dh->isNull())
	{
		LAPriceDataInterpolation tempInterp = dynamic_cast<const LAPriceDataInterpolation &>(dh->get());
		interpolationYGStr_ = tempInterp.convertToString();
		pInter_yg_ = std::shared_ptr<LAInterpolationBase> (dynamic_cast<LAInterpolationBase*>(tempInterp.getMethod().clone()));		
	}
			
	// get Futures/FRA Interpolation	
	dh = &curveCalibrationData.getData(IR_CALIBRATION_DATA_INTERPOLATIONFW + suffix_mkt, NOCHECK);	
	pInter_fw_.reset();
	if (dh->isDefined() && !dh->isNull())
	{
		pInter_fw_ = std::shared_ptr<LAInterpolationBase> (dynamic_cast<LAInterpolationBase*>(dynamic_cast<const LAPriceDataInterpolation &>(dh->get()).getMethod().clone()));		
	}

	// get Rate Priority	
	pRatePriority_.reset();
	dh = &curveCalibrationData.getData(PRICING_DATA_RATEPRIORITY + suffix_mkt, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		const LAStringVector* temp = &(dynamic_cast<const LADataStrings&>(dh->get()).get());
		pRatePriority_.reset(const_cast<LAStringVector*>(temp));
	}

	// Get forward rates from swaps only?	
	generateForwardsFromSwapsOnly_ = true;
	dh = &curveCalibrationData.getData(IR_CALIBRATION_DATA_GENERATEFORWARDSFROMSWAPSONLY, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		generateForwardsFromSwapsOnly_ = dynamic_cast<const LADataBool&>(dh->get());
	}
	
	// Yield curve data as a reference
	yieldDataRef_ = curveCalibrationData.getYieldData();

	// Initiliase the curve in preparation for calibration
	initialise();
}


/*
    @brief Initialise the curve calibration process prior to going into solver
*/
void SwapComponentCurve::initialise()
{
	// Start the calibration process by calibrating using simple cash and forward instruments
	calibrateSwapCurveWithCashAndForwards();
}


/*
    @brief Calibrate swap curve using only the short end cash and forward instruments
*/
void SwapComponentCurve::calibrateSwapCurveWithCashAndForwards() 
{
	LAObjectHolder objHolder = yieldDataRef_.get();

	//Classify data object by data type 
	std::vector<LAObject*> data_libor, data_swap, data_future, data_mpc_swaps, data_fra, data_tenorswap;
	moneyMarketDataMap_.clear();

	unsigned int size_data = data_.size();
	LAString datatype_str;
	LAPriceDataDayCount dc_act365(ACT_365);
	bool isO_N = false, isT_N = false;
	fwd_termsmtx_.clear();
	fwd_termsmtx_.resize(2);
	fwds_.clear();

	for(unsigned i = 0; i < size_data; i++)
	{
		// check use grid
		const LADataHolder *dh = &data_[i]->getData(IR_CALIBRATION_DATA_GRIDUSEFLAG, NOCHECK);
		if (dh->isDefined() && !dh->isNull() && !dynamic_cast<const LADataBool &>(dh->get()).get()) 
		{
			continue;
		}

		datatype_str = dynamic_cast<const LADataString&> ((data_[i]->getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
		datatype_str.toUpper();
		
		if (datatype_str == ZERO) 
		{
			data_libor.push_back(data_[i]);//libor case
		}
		else if (datatype_str == PAR) 
		{
			data_swap.push_back(data_[i]);//swap case
		}
		else if (datatype_str == FUTURE) 
		{
			data_future.push_back(data_[i]);//future case
		}
		else if (datatype_str == BOJ) 
		{
			data_mpc_swaps.push_back(data_[i]);//mpc case = monetary policy committee swaps
		}
		else if (datatype_str == FEDFUNDRATE) 
		{
			const LADate& sdate = dynamic_cast<const LADataDate&> ((data_[i]->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get()).get();
			//for fedfund first grid
			data_mpc_swaps.push_back(data_[i]);//mpc case = monetary policy committee swaps
		}
		else if (datatype_str == FRA) 
		{
			data_fra.push_back(data_[i]);//fra case
		}
		else if (datatype_str == BASIS) 
		{
			data_tenorswap.push_back(data_[i]);//tenor swap
		}
		else //money market case
		{
			const LAPriceDataCalendar& cal  = dynamic_cast<const LAPriceDataCalendar&> ((data_[i]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
			const LAPriceDataSlidingRule& sld = dynamic_cast<const LAPriceDataSlidingRule&> ((data_[i]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
			if (datatype_str == O_N)
			{
				LADate end = baseDate_;
				end.addDays(1);
				end = sld.getDate(end, cal);
				moneyMarketDataMap_[std::pair<LADate, LADate>(baseDate_, end)] = data_[i];
				isO_N = true;
				DateVector depocalcdates(2,baseDate_);
				depocalcdates[1] = end;
				if(!data_[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO,NOCHECK).isDefined())
				{
					data_[i]->add(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, new LADataDates(depocalcdates));
				}
				else
				{
					dynamic_cast<LADataDates&>(data_[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO).get()).set(depocalcdates);
				}
			}
			else if (datatype_str == T_N)
			{

				LADate start = baseDate_;
				start.addDays(1);
				start = sld.getDate(start, cal);
				LADate end = start;
				end.addDays(1);
				end = sld.getDate(end, cal);
				moneyMarketDataMap_[std::pair<LADate, LADate>(start, end)] = data_[i];
				isT_N = true;
				DateVector depocalcdates(2,start);
				depocalcdates[1] = end;
				if(!data_[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO,NOCHECK).isDefined())
				{
					data_[i]->add(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, new LADataDates(depocalcdates));
				}
				else
				{
					dynamic_cast<LADataDates&>(data_[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO).get()).set(depocalcdates);
				}
			}
			else
			{
				LADate start, end;
				if (datatype_str == "0D_1D") isO_N = true;
				else if (datatype_str == "1D_1D") isT_N = true;
				etrading::getMoneyMarketDates(baseDate_, datatype_str, cal, sld, start, end);
				moneyMarketDataMap_[std::pair<LADate, LADate>(start, end)] = data_[i];
			}
		}
	}

	dc_Libor_ = dynamic_cast<const LAPriceDataDayCount&> ((data_libor[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());

	// ascending sort libor , swap and future data entities  
	sort(data_libor.begin(), data_libor.end(), InstrumentComp());
	sort(data_swap.begin(), data_swap.end(), InstrumentComp());
	sort(data_mpc_swaps.begin(), data_mpc_swaps.end(), InstrumentComp());
	sort(data_tenorswap.begin(), data_tenorswap.end(), InstrumentComp());
	
    if (is_fra_use_) 
	{
		sort(data_fra.begin(), data_fra.end(), InstrumentComp());
	}

	if (is_f_use_) 
	{
		sort(data_future.begin(), data_future.end(), InstrumentComp());
	}

	unsigned int size_l = data_libor.size();    //libor size
	unsigned int size_s = data_swap.size();     //swap size
	unsigned int size_f = data_future.size();   //future size
	unsigned int size_mpc_swaps = data_mpc_swaps.size();      //mpc size = monetary policy committee swaps size
	unsigned int size_fra = data_fra.size();    //3mfra size

    //
    // Calculate the First Swap Date
    // ------------------------------------------------------------
    //
    const LADataHolder* dh = 0;

	LADate firstSwapDate;
    if (is_fra_use_ || is_f_use_)
	{
        MLIB_REQUIRE( size_s > 0, "Unable to build curve: Missing swap calibration instruments" )
		
		const LADate& spotdate = dynamic_cast<const LADataDate&> ((data_swap[0]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get());
		const LAString& term_str = dynamic_cast<const LADataString&> ((data_swap[0]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
		const LAString& freq = dynamic_cast<const LADataString&> ((data_swap[0]->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL)).get()).get();
		const LAPriceDataCalendar& cal  = dynamic_cast<const LAPriceDataCalendar&> ((data_swap[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
		const LAPriceDataSlidingRule& sld  = dynamic_cast<const LAPriceDataSlidingRule&> ((data_swap[0]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
		
        // set eom
        bool eom = false;
		const LADataHolder *dh = &(data_swap[0]->getData(IR_CALIBRATION_DATA_ISEOMROLLSW, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			eom = dynamic_cast<const LADataBool &>(dh->get()).get();
		}
		// set roll convention
		LAString roll_conv("");
		if (freq == LUNAR)
		{
			roll_conv = ROLLCONV_LUNAR;
		}
		else if (eom) 
		{
			roll_conv = ROLLCONV_EOM;
		}
		else 
		{
			roll_conv = ROLLCONV_NORMAL;
		}
		firstSwapDate = etrading::LADateHelpers::getDate(spotdate, term_str, sld, &cal, true, &roll_conv);
	}
    // ------------------------------------------------------------
		
	if (!isO_N || !isT_N)
	{
        throw LACoreInvalidData("#Error: Both O_N and T_N calibration instruments are required", __FILE__, __LINE__);
	}
	
	if (size_l == 0 || size_s == 0)
	{
        throw LACoreInvalidData("#Error: Libor Fixings and Swap calibration instruments cannot be empty", __FILE__, __LINE__);
	}
	
	if (is_fra_use_ && is_f_use_) 
	{
        throw LACoreInvalidData("#Error: We can not use fra and futures calibration instruments at a same time!", __FILE__, __LINE__);
	}
	
	//////////////////////
	//Spot date of Libor//
	//////////////////////
	LADate spotdate_l;
	for (unsigned int i = 0; i < size_l; i++)
	{
		const LADate& spotdate = dynamic_cast<const LADataDate&> ((data_libor[i]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get());
		if (i == 0) 
		{
			spotdate_l = spotdate;
		}
		else if (spotdate_l != spotdate)
		{
            LAString msg = "#Error: Calibration instruments must have the same spotdate";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);			
		}
	}
	const LAPriceDataDayCount &dc_l = dynamic_cast<const LAPriceDataDayCount&> ((data_libor[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());

	///////////////////////////////////////////
	//Spot date, Calendar, Convention of Swap//
	///////////////////////////////////////////
	LADate spotdate_s;
	std::vector<const LAPriceDataCalendar *> cal_s(size_s, 0);
	std::vector<const LAPriceDataSlidingRule *> sld_s(size_s, 0);
	std::vector<const LAPriceDataDayCount *> dc_s(size_s, 0);
	std::vector<const LAPriceDataDayCount *> dc_s_float(size_s, 0);
	LAStringVector freq_s(size_s, "");
	LAStringVector freq_s_float(size_s, "");
	LAStringVector freq_s_cpd(size_s, "");
	cpd_times_.resize(size_s);
	LAString baseFreq = "";
	bool isAllSame_s = true;
	std::vector<bool> eom_s(size_s, false);
	LAStringVector roll_conv_s(size_s, "");

	for (unsigned int i = 0; i < size_s; i++)
	{
		const LADate& spotdate = dynamic_cast<const LADataDate&> ((data_swap[i]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get());
		freq_s[i] = dynamic_cast<const LADataString&> ((data_swap[i]->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL)).get()).get();
		freq_s[i].toUpper();
		if (freq_s[i] == SIMPLE)
		{
			//error
            throw LACoreInvalidData("#Error: Simple frequency is not supported in IRS Market", __FILE__, __LINE__);
		}
		cal_s[i] =  &dynamic_cast<const LAPriceDataCalendar&> ((data_swap[i]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
		sld_s[i]  = &dynamic_cast<const LAPriceDataSlidingRule&> ((data_swap[i]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
		dc_s[i] = &dynamic_cast<const LAPriceDataDayCount&> ((data_swap[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
		const LADataHolder *dh = &data_swap[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT_FLOAT, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			dc_s_float[i] = &dynamic_cast<const LAPriceDataDayCount&> (dh->get());
		}
		dh = &data_swap[i]->getData(IR_CALIBRATION_DATA_FREQUENCY_FLOAT, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			freq_s_float[i] = dynamic_cast<const LADataString&> (dh->get());
		}
	
		dh = &data_swap[i]->getData(IR_CALIBRATION_DATA_FREQUENCY_COMPOUND, NOCHECK); 
		if (dh->isDefined() && !dh->isNull()) 
		{ 
			freq_s_cpd[i] = dynamic_cast<const LADataString&> (dh->get());
		}
		else
		{
			freq_s_cpd[i] = freq_s_float[i];
		}
		
		cpd_times_[i] = etrading::LADateHelpers::calcCompoundingTimes(freq_s_float[i], freq_s_cpd[i]);

		if (i == 0)
		{
			spotdate_s = spotdate;
		}
		else if (!(*cal_s[i] == *cal_s[i - 1]) || !(*sld_s[i] == *sld_s[i - 1]) || !(freq_s[i] == freq_s[i - 1]) || !(*dc_s[i] == *dc_s[i - 1]))
		{
			isAllSame_s = false;
		}
		else if (spotdate_s != spotdate)
		{
            LAString msg = "#Error: Each swap calibration instrument must have the same spotdate";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);			
		}

		if (i == 0)
		{	
			const LADataHolder *dh = &data_swap[i]->getData(IR_CALIBRATION_DATA_BASEFREQUENCY_FLOAT, NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				baseFreq = dynamic_cast<const LADataString&> (dh->get());
			}
		}

		dh = &data_swap[i]->getData(IR_CALIBRATION_DATA_ISEOMROLLSW, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			eom_s[i] = dynamic_cast<const LADataBool &>(dh->get()).get();
		}

		// set roll convention
		if (freq_s[i] == LUNAR) roll_conv_s[i] = ROLLCONV_LUNAR;
		else if (eom_s[i]) roll_conv_s[i] = ROLLCONV_EOM;
		else roll_conv_s[i] = ROLLCONV_NORMAL;
	}

	spotDate_ = spotdate_s;

	// check Swap Market
	LAString refRateTerm;
	if (baseFreq == SEMI_ANNUAL)
	{
		refRateTerm = "6M";
	}
	else if (baseFreq == QUARTERLY)
	{
		refRateTerm = "3M";
	}
	else if (baseFreq == MONTHLY)
	{
		MLIB_THROW_IF( is_fra_use_ && size_fra > 0, "Invalid Swap Curve: Only 3M and 6M Swap curves are allowed to calibrate using FRAs" )
		refRateTerm = "1M";
	}
	else if (baseFreq == ANNUAL)
	{
		MLIB_THROW_IF( is_fra_use_ && size_fra > 0, "Invalid Swap Curve: Only 3M and 6M Swap curves are allowed to calibrate using FRAs")
		MLIB_THROW_IF( is_f_use_ && size_f > 0,		"Invalid Swap Curve: Only 1M, 3M and 6M Swap curves are allowed to calibrate using Futures")
		refRateTerm = "12M";
	}
	else
	{
		MLIB_THROW("Invalid Swap Curve: Swap curve frequency must be 1M, 3M, 6M or 12M.")
	}

	// get fwd swap
	is_fwdswap_ = false;
	dh = &(data_swap[0]->getData(PRICING_DATA_ISFWDSWAP, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		is_fwdswap_ = dynamic_cast<const LADataBool &>(dh->get()).get();
	}
		
	// Locate previous solution and gradient, if available
	LAString targetSuffix = "";
	if (curveName_ != STD)
	{
		targetSuffix = LAString("_") + curveName_;
	}
	targetSuffix.toUpper();

	//By Default, the stateVariable of the interpolator is discount factor
	StateVariableEnum stateVariable = STATE_VARIABLE_ZERO_RATE_TIMES_TIME;
	const LADataHolder* handle = &(objHolder.getData(IR_CALIBRATION_DATA_STATEVARIABLE + targetSuffix, NOCHECK));
	if (handle->isDefined() && !handle->isNull())
	{
		stateVariable = etrading::toStateVariableEnum(dynamic_cast<const LADataString&>(handle->get()).get().getCString());

		if (stateVariable != STATE_VARIABLE_DF && stateVariable != STATE_VARIABLE_ZERO_RATE_TIMES_TIME)
		{
			throw LACoreInvalidData("#Error: For SWAP/STD Curve, interpolator's StateVariable only supports DF or RateTime", __FILE__, __LINE__);
		}
	}

	const StateVariableEnum stateVariableFutureFra = STATE_VARIABLE_LOG_DF;

	//
    // Set the Join Date for the Linear-Spline Interpolation Method
    // ------------------------------------------------------------
    //
    bool pInter_IsHybrid    = false;
	bool pInter_yg_IsHybrid = false;
	bool pInter_fw_IsHybrid = false;
	if (pInter_ != NULL)
	{
		pInter_IsHybrid = ( pInter_->isHybrid() ) ? true : false;
	}
	if (pInter_yg_ != NULL)
	{
		pInter_yg_IsHybrid = ( pInter_yg_->isHybrid()) ? true : false;
	}
	if (pInter_fw_ != NULL)
	{
		pInter_fw_IsHybrid = ( pInter_fw_->isHybrid() ) ? true : false;
	}
    
    // Set Output Variable: linear-spline interpolation join date
    interpolationJoinDateAsDouble_ = 0.0;
	bool joinDateGiven = false;	
	bool calculateJoinDate = true;

    if ( pInter_IsHybrid || pInter_yg_IsHybrid|| pInter_fw_IsHybrid )
    {
		// If a join date is given by user as an input, we use this join date and don't calculate join date;
		// If no user-given join date exists, we will decide whether we always re-calculate the dynamic
		// join date, or only calculate it once and re-use thie join date in subsequent calibrations of the same curve.

		// Calculate the initial linear spline join date
		const LADataHolder* handle = &( objHolder.getData(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + targetSuffix, NOCHECK) );
		if ( handle->isDefined() && !handle->isNull() )
		{
			// Use a linear spline join date supplied by user
			joinDateGiven = true;
			LADate joinDate = dynamic_cast< const LADataDate& >( handle->get() ).get();
			interpolationJoinDate_ = joinDate;
			interpolationJoinDateAsDouble_ = dc_act365.getTerm( spotdate_s, joinDate );
		}
		else
		{
			// Always recalculate the dynamic join date?
			bool alwaysRecalcJoinDate = true;
			const LADataHolder* handle = &( objHolder.getData(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + targetSuffix, NOCHECK) );
			if ( handle->isDefined() && !handle->isNull() )
			{
				alwaysRecalcJoinDate = dynamic_cast< const LADataBool& >( handle->get() ).get();
			}
			
			if (!alwaysRecalcJoinDate)
			{
				handle = &( objHolder.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + targetSuffix, NOCHECK) );
				if ( handle->isDefined() && !handle->isNull() )
				{
					double existingJoinDateAsDouble = dynamic_cast< const LADataDouble& >( handle->get() ).get();
					interpolationJoinDateAsDouble_ = existingJoinDateAsDouble;

					handle = &( objHolder.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE + targetSuffix, NOCHECK) );
					interpolationJoinDate_ = dynamic_cast< const LADataDate& >( handle->get() ).get();

					calculateJoinDate = false;
				}
			}

			// MLib calculates linear spline join date when:
			// 1. User specifies to ALWAYS recalculate join date; or
			// 2. User specifies not to always recalculate join date, but no pre-calculated join date currently exists;

			if (calculateJoinDate)
			{
				double joinDateAsDouble = 0.0;
				if( is_fra_use_ && size_fra > 0 )
				{
					const LADate spotDate               = dynamic_cast< const LADataDate& >( ( data_fra[size_fra-1]->getData( IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL ) ).get() ).get();
					const LAPriceDataDayCount& fraDayCount   = dynamic_cast< const LAPriceDataDayCount& >( ( data_fra[size_fra-1]->getData( IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL ) ).get() );
                
					const LADataHolder *dh              = &data_swap[size_s-1]->getData( IR_CALIBRATION_DATA_BASEFREQUENCY_FLOAT, NOCHECK );
					LAString baseFreq;               
					if ( dh->isDefined() && !dh->isNull() )
					{
						baseFreq = dynamic_cast< const LADataString& >( dh->get() );
					}
					else
					{
						throw LACoreInvalidData("#Error: Linear-Spline Interpolation Error. Unable to calculate the FRA end date needed for the Linear-Spline join date", __FILE__, __LINE__);
					}

					LAString refRateTerm;
					if (baseFreq == SEMI_ANNUAL)
					{
						refRateTerm = "6M";
					}
					else if (baseFreq == QUARTERLY)
					{
						refRateTerm = "3M";
					}
					else
					{
						throw LACoreInvalidData("#Error: STD Curve only supports swaps with 3M or 6M floating leg frequency", __FILE__, __LINE__);
					}

					LADate lastFraStartDate, lastFraEndDate;
					etrading::calculateFraDates( lastFraStartDate, lastFraEndDate, spotDate, data_fra[size_fra-1], refRateTerm, baseFreq, fraDayCount );

					// Do not allow join date to be beyond the first swap maturity date
					if ( lastFraEndDate >= firstSwapDate ) 
					{
						lastFraEndDate = firstSwapDate;
					}

					interpolationJoinDate_ = lastFraEndDate;
					joinDateAsDouble = dc_act365.getTerm( spotdate_s, lastFraEndDate );
				}
				else if ( is_f_use_ && size_f > 0 )
				{			
					interpolationJoinDate_ = etrading::determineLinearSplineInterpolationJoinDate(data_future[size_f - 1], data_swap[0], spotdate_s, is_fwdswap_);
					joinDateAsDouble = dc_act365.getTerm( spotdate_s, interpolationJoinDate_ );
				}

				interpolationJoinDateAsDouble_ = joinDateAsDouble;        
			}
		}

        // Set the Linear-Spline Interpolation Join Date for the Disc Factor Interpolation Table
        if ( pInter_IsHybrid )
        {
            pInter_->setJoinDateAsDouble( interpolationJoinDateAsDouble_ );
        }

        // Set the Linear-Spline Interpolation Join Date for the Swap Rate Interpolation Table
        if ( pInter_yg_IsHybrid )
        {
            pInter_yg_->setJoinDateAsDouble( interpolationJoinDateAsDouble_ );
        }

        // Set the Linear-Spline Interpolation Join Date for the Forward Interpolation Table
        if ( pInter_fw_IsHybrid )
        {
            pInter_fw_->setJoinDateAsDouble( interpolationJoinDateAsDouble_ );
        }
    }
    // ------------------------------------------------------------


	/////////////////
	//Rate Priority//
	/////////////////
	LAStringVector* pRatePriority = pRatePriority_.get();

	bool isFuturePriority = false;// future priority than libor
	bool isSwapPriority = false; //swap priority than other rates
	if (pRatePriority != 0)
	{
		if (is_f_use_ && size_f != 0)
		{
			if (pRatePriority->size() != 3)
			{
                //error
				LAString msg = "#Error: Invalid Rate priority specified.";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			else 
			{
                // Added to prevent Excel crashing due to access violation / non-bounds check on std::vector (*pRatePriority)[1]
                if (pRatePriority->size() < 2)
                {
					LAString msg = "#Error: Invalid Rate priority. This parameter must be a list of colon separated parameters of size 2.";
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
                }

				LAString str = (*pRatePriority)[1];
				str.toUpper();
				if (str == SWAP)
				{
	                //error
                    LAString msg = "#Error: Invalid Rate priority. 'Swap' must not be between other rates";
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);					
				}
			}
		}

		LAString str = (*pRatePriority)[0];
		str.toUpper();
		if (str == SWAP) 
		{
            // Added to prevent Excel crashing due to access violation / non-bounds check on std::vector (*pRatePriority)[1]
            if (pRatePriority->size() < 2)
            {
                LAString msg = "#Error: Invalid Rate priority. Must be a list of colon separated parameters of size 2";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
            }

			isSwapPriority = true;
			str = (*pRatePriority)[1];
			str.toUpper();
			if (is_f_use_ && size_f != 0 && str == FUTURE)
			{
				isFuturePriority = true;
			}
		}
		else if (is_f_use_ && size_f != 0 && str == FUTURE)
		{
			isFuturePriority = true;
		}
	}

	// Bootstrap Libor Cash Deposits
	LAString freq;
	double df_adj_spots_spotl = 1.0;//df from spotdate_s to spotdate_l or from spotdate_l to spotdate_s 

	std::map<std::pair<LADate, LADate>, const LAObject*>::const_iterator it_ = moneyMarketDataMap_.begin();
	const double rate_on = dynamic_cast<const LADataDouble&> ((it_->second->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
	const LAPriceDataDayCount& dc_on  = dynamic_cast<const LAPriceDataDayCount&> ((it_->second->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
	RateConvention rc_on = LAMathYieldCurve::setRC(SIMPLE);
	LAPriceDataConvention conv_on(dc_on.getDayCount(), rc_on);

	bool is_on_adj = false;
	dh = &(data_libor[0]->getData(IR_CALIBRATION_DATA_ISONFORSPOTADJUST, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		is_on_adj = dynamic_cast<const LADataBool &>(dh->get()).get();
	}

	// *************************************************************************************
	// Bootstrap Libor Cash Deposits
	// TODO - Change this to LogDF State Variable to match Swap Instrument calibration
	// *************************************************************************************
	dfResults_ = bootstrapLibors( fwds_,
								  fwd_termsmtx_,
								  data_libor,
								  moneyMarketDataMap_,
								  is_fra_use_,
								  is_f_use_,
								  spotdate_s,
								  spotdate_l,
								  refRateTerm );

	// *************************************************************************************
	// Bootstrap Futures using LogDF State Variable - Same as swap instrument calibration
	// *************************************************************************************		
	DoubleArray futuresStartDateTerms;
	DoubleArray futuresEndDateTerms;
	DoubleArray futuresRates;
	LADate lastFutureStartDate;

	if (is_f_use_ && size_f != 0)
	{
		LAString roll_conv("");		
		int tensionGapFutures = false;
		bool applyTensionFutures = false;	
		bool smoothShortEnd = true;
		
		// Convexity Quote Type Default
		bool convexityQuotedAsVol = true;
		LAString convexityQuoteTypeStr = "VOL";

		if (size_f > 0)
		{
			// get end of month flag
			bool eom = false;
			dh = &(data_future[0]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				eom = dynamic_cast<const LADataBool &>(dh->get()).get();
			}

			// set roll convention		
			if (baseFreq == LUNAR) 
			{
				roll_conv = ROLLCONV_LUNAR;
			}
			else if (eom) 
			{
				roll_conv = ROLLCONV_EOM;
			}
			else 
			{
				roll_conv = ROLLCONV_NORMAL;
			}

			// get apply tension        
			const LADataHolder *ahApplyTensionFutures = &(data_future[0]->getData(IR_CALIBRATION_DATA_APPLYTENSION, NOCHECK));
			if (ahApplyTensionFutures->isDefined() && !ahApplyTensionFutures->isNull())
			{
				applyTensionFutures = dynamic_cast<const LADataBool &>(ahApplyTensionFutures->get()).get();
			}

			// get tension gap
			tensionGapFutures = dynamic_cast<const LADataInt&> ((data_future[0]->getData(IR_CALIBRATION_DATA_TENSIONGAP, ISNOTNULL)).get()).get();
			if ( applyTensionFutures && tensionGapFutures < 1 )
			{
				LAString msg = "#Error: The TensionGap parameter in the Futures conventions table must be a positive integer.";
  				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}

			// *** CONVEXITY QUOTE PARAMETERS ***
			// -------------------------------------------------------------------------------------------------------------

			// 1.	get useConvexAdj (means convexityQuotedAsPrice) - replaced by alias convexityQuotedAsVol
			const LADataHolder *dhConvexityQuotedAsPrice = &(data_future[0]->getData(PRICING_DATA_USECONVEXADJUSTMENT, NOCHECK));
			if (dhConvexityQuotedAsPrice->isDefined() && !dhConvexityQuotedAsPrice->isNull())
			{
				bool convexityQuotedAsPrice = dynamic_cast<const LADataBool &>(dhConvexityQuotedAsPrice->get()).get();
				convexityQuotedAsVol = !convexityQuotedAsPrice;
			}

			// 2.	get convexityQuotedAsVol, This is an alias for useConvexAdj, but also it's inverse
			const LADataHolder *dhConvexityQuoteType = &(data_future[0]->getData(PRICING_DATA_CONVEXITYQUOTETYPE, NOCHECK));
			if (dhConvexityQuoteType->isDefined() && !dhConvexityQuoteType->isNull())
			{
				convexityQuoteTypeStr = dynamic_cast<const LADataString &>(dhConvexityQuoteType->get()).get();
				convexityQuoteTypeStr.toUpper();
				MLIB_REQUIRE( convexityQuoteTypeStr == "VOL" || convexityQuoteTypeStr == "PRICE", "Invalid Futures Convexity Quote Type: ConvexityQuoteType must be VOL or PRICE" )
				convexityQuotedAsVol = ( convexityQuoteTypeStr == "VOL" ) ? true : false;
		
				// TODO: Property Manager does not allow us to clear parameters once set ... hence the below fails ... prioritize the convexityQuoteType parameter for now
				// if ( dhConvexityQuotedAsPrice->isDefined() && !dhConvexityQuotedAsPrice->isNull() )
				// {
				// 	MLIB_THROW("Invalid Futures Convexity Parameter: Cannot use ConvexityQuoteType and UseConvexAdjustment (ConvexityQuotedAsPrice) parameters at the same time.")
				// }
			}

			// Update Convexity Quote String
			convexityQuoteTypeStr = convexityQuotedAsVol ? "VOL" : "PRICE";

			// -------------------------------------------------------------------------------------------------------------

			// get smoothShortEnd		
			const LADataHolder *ahSmoothShortEnd = &(data_future[0]->getData(IR_CALIBRATION_DATA_SMOOTHSHORTEND, NOCHECK));
			if (ahSmoothShortEnd->isDefined() && !ahSmoothShortEnd->isNull())
			{
				smoothShortEnd = dynamic_cast<const LADataBool &>(ahSmoothShortEnd->get()).get();
			}
		}

        // This struct stores interpolation data from bootstrapping routine needed within 'insertSyntheticTensionPoints'
        // Must be initialized outside the instrument data bootstrapping routine.
        etrading::tensionMarketData futuresInstrumentData;

		// Interpolate on Futures on LogDF the same state variable as the Swaps
		std::shared_ptr<LAInterpolationBase> logDF_FuturesInterpolation( dynamic_cast<LAInterpolationBase *>(pInter_fw_->clone()) );
		logDF_FuturesInterpolation->setJoinDateAsDouble( interpolationJoinDateAsDouble_ );
        
		for (unsigned int i = 0; i < size_f; ++i)
		{
			// Use LogDF as StateVariable so Futures and Swap StateVariables are the Same
			// liborDF_ is a Matrix of Terms and DFs
			DoubleVector terms = dfResults_.paymentDatesAsTerms_;
			DoubleVector logDFs( dfResults_.discountFactors_.size() );
			for (size_t i = 0; i < dfResults_.discountFactors_.size(); ++i)
			{
				logDFs[i] = LAMath::log(dfResults_.discountFactors_[i]);
			}
			logDF_FuturesInterpolation->set(terms, logDFs);

			const LAPriceDataDayCount& dc   = dynamic_cast<const LAPriceDataDayCount&> ((data_future[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
			const LADate& sdate = dynamic_cast<const LADataDate&> ((data_future[i]->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get()).get();
			if (sdate <= spotdate_s) 
			{
				continue;
			}

			if (i == size_f - 1)
			{
				lastFutureStartDate = sdate;
			}

			const LADataHolder* dh;
				
            // get term
			const LADate& edate = dynamic_cast<const LADataDate&> ((data_future[i]->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get()).get();
			if (edate >= firstSwapDate) 
			{
				break;				            
			}

			// get rate
			double rate;
			dh = &(data_future[i]->getData(CALIBRATION_DATA_RATE, NOCHECK));
			if (dh->isDefined() && !dh->isNull()) 
			{
				rate = dynamic_cast<const LADataDouble&> (dh->get()).get();
			}
			else 
			{
				double price = dynamic_cast<const LADataDouble&> ((data_future[i]->getData(PRICING_DATA_PRICE, ISNOTNULL)).get()).get(); 
				rate = 1.0 - price * 0.01;
			}			
				
			// get spread
			double spread = 0.0;
			dh = &(data_future[i]->getData(PRICING_DATA_SPREAD, NOCHECK));
			if (dh->isDefined() && !dh->isNull()) 
			{
				spread = dynamic_cast<const LADataDouble&> (dh->get()).get();
			}
			rate += spread;
									
			// Convexity Adjustments, quoted as a volatility or a price
			// ********************************************************
			if ( convexityQuotedAsVol )
			{
				// *** CONVEXITY QUOTED AS A VOLATILITY ***

				// get future volatility
				double f_vol = 0.0;
				dh = &(data_future[i]->getData(PRICING_DATA_FUTUREVOLATILITY, NOCHECK));
				if (dh->isDefined() && !dh->isNull()) 
				{
					f_vol = dynamic_cast<const LADataDouble&> (dh->get()).get();
				}
				            
				if(f_vol > 0.0)
				{
					//Convexity Adjust by Ho-Lee Model or Hull-White (1 Factor) Model
					double start_term = dc_act365.getTerm(baseDate_, sdate);
					double end_term = dc_act365.getTerm(baseDate_, edate);

					if ( start_term > end_term )
					{
                        throw LACoreInvalidData("#Error: CurveCalibration::calcDiscountFactor failed. Invalid futures market data, a futures end date is before it's start date", __FILE__, __LINE__);
					}

					bool isConvAdjPrecise = false;
					dh = &(data_future[i]->getData(IR_CALIBRATION_DATA_ISCONVADJPRECISE, NOCHECK));
					if (dh->isDefined() && !dh->isNull())
					{
						isConvAdjPrecise = dynamic_cast<const LADataBool &>(dh->get()).get();
					}

					// Use the Hull-White 1 Factor Model if the mean reversion parameter is populated else use the Ho-Lee Model
					double meanReversion = 0.0;
					dh = &( data_future[i]->getData( IR_CALIBRATION_DATA_MEANREVERSION, NOCHECK ));
					if( dh->isDefined() && !dh->isNull() )
					{
						meanReversion = dynamic_cast< const LADataDouble & >( dh->get() ).get();
					}
                    
					// The Hull-White mean reversion parameter must be a value between 0 and 1. Throw an error if this is not the case.
					if ( meanReversion < 0.0 || meanReversion > 1.0 )
					{
                        throw LACoreInvalidData("#Error: CurveCalibration::calcDiscountFactor failed. Invalid futures market data, the convexity mean reversion parameter must be a value between 0 and 1", __FILE__, __LINE__);
					}
                    
					if (isConvAdjPrecise)
					{
						// refer to a document of Bloomberg about convexity adjust of euro dollar future
						double tau = dc.getTerm(sdate, edate);
						if (tau <= 0.0)
						{
							throw LACoreInvalidData("#Error: CurveCalibration::calcDiscountFactor failed. Invalid futures market data, a futures end date is before it's start date", __FILE__, __LINE__);
						}

						double rate_continuous = LAMath::log(1. + tau * rate) / tau;
						double conv_adjust_continuous = 0.0;
						if ( meanReversion == 0.0 )
						{
							// DEFAULT METHOD:
							// Use the Ho-Lee Model by default if the HW mean reversion parameter is not populated
							conv_adjust_continuous = 0.5 * start_term * end_term * f_vol * f_vol;
						}
						else
						{
							// ALTERNATIVE METHOD:
							// Use the Hull-White 1 Factor model if the mean reversion parameter is populated
							double a        = meanReversion;    // We already checked to ensure meanReversion > 0
							double T1       = start_term;       // We already checked start_term < end_term
							double T2       = end_term;
							double B_T1_T2  = ( 1.0 - LAMath::exp( -a * ( T2 - T1 ) ) ) / a;
							double B_0_T1   = ( 1.0 - LAMath::exp( -a * T1 ) ) / a;

							// This is the Hull-White 1 Factor Futures Convexity Adjustment
							// See Options, Futures and Other Derivatives 9ed by John Hull
							// Specifically see Technical Note 1 - Convexity Adjustments to Eurodollar Futures
							conv_adjust_continuous = ( B_T1_T2 / ( T2 - T1 ) )
								* ( B_T1_T2 * ( 1 - LAMath::exp( -2 * a * T1 ) ) + ( 2 * a * B_0_T1 * B_0_T1 ) )
								* ( f_vol * f_vol / ( 4 * a ) );
						}

						double conv_adjust_simple =	(1. - LAMath::exp(- conv_adjust_continuous * tau)) * (rate + 1. / tau);
						rate -= conv_adjust_simple;
					}
					else
					{
						if ( meanReversion == 0.0 )
						{
							// DEFAULT METHOD:
							// Use the Ho-Lee Model if the HW mean reversion parameter is not populated
							rate -= 0.5 * start_term * end_term * f_vol * f_vol;
						}
						else
						{
							// ALTERNATIVE METHOD:
							// Use the Hull-White 1 Factor model if the mean reversion parameter is populated
							double a        = meanReversion;    // We already checked to ensure meanReversion > 0
							double T1       = start_term;       // We already checked start_term < end_term
							double T2       = end_term;
							double B_T1_T2  = ( 1.0 - LAMath::exp( -a * ( T2 - T1 ) ) ) / a;
							double B_0_T1   = ( 1.0 - LAMath::exp( -a * T1 ) ) / a;

							// This is the Hull-White 1 Factor Futures Convexity Adjustment
							// See Options, Futures and Other Derivatives 9ed by John Hull
							// Specifically see Technical Note 1 - Convexity Adjustments to Eurodollar Futures
							rate    -= ( B_T1_T2 / ( T2 - T1 ) )
									* ( B_T1_T2 * ( 1 - LAMath::exp( -2 * a * T1 ) ) + ( 2 * a * B_0_T1 * B_0_T1 ) )
									* ( f_vol * f_vol / ( 4 * a ) );
						}
					}
				}
			}
			else
			{
				// *** CONVEXITY QUOTED AS A PRICE ***

				double convexAdj = 0.0;
				dh = &(data_future[i]->getData(PRICING_DATA_CONVEXADJUSTMENT, NOCHECK));
				if (dh->isDefined() && !dh->isNull()) 
				{
					convexAdj = dynamic_cast<const LADataDouble&> (dh->get()).get();
				}
				rate -= convexAdj;
			}
				
			// Interest Rate Convention - Stores instrument daycount and compounding conventions e.g. Simple Interest Act/Act.
			RateConvention rc = LAMathYieldCurve::setRC(SIMPLE);
			LAPriceDataConvention conv(dc.getDayCount(), rc);

			// df
			double start_term = dc_act365.getTerm(spotdate_s, sdate);
			double end_term = dc_act365.getTerm(spotdate_s, edate);
			double startDF = LAMath::exp( logDF_FuturesInterpolation->value(start_term) );
			double endDF = startDF * conv.getDF(rate, sdate, edate);

			futuresRates.push_back(rate);
			futuresStartDateTerms.push_back(start_term);
			futuresEndDateTerms.push_back(end_term);

			if (i == 0 && smoothShortEnd && sdate < dfResults_.lastLiborEndDate_ )
			{
				std::unique_ptr<LAInterpolationBase> cashDepositInterpolation( dynamic_cast<LAInterpolationBase *>(pInter_fw_->clone()) );
				cashDepositInterpolation->setJoinDateAsDouble( interpolationJoinDateAsDouble_ );

				// Solve for the level of startDF and endDF so that they can imply a discount factor at liborDate that
				// allows for the libor instrument to be correctly repriced
				etrading::solveSmoothSTDShortEnd( startDF,
												  endDF,
												  sdate,
												  edate,
												  spotdate_l,
												  spotdate_s,
												  dfResults_.lastLiborEndDate_,
												  data_libor[dfResults_.lastLiborPosition_],
												  cashDepositInterpolation,
												  stateVariableFutureFra,
												  dfResults_,
												  rate,
												  conv );
			}

            // Update Forward Rate and Discount Factor at Start and End of Forward Period
			etrading::insertDFData(dfResults_, startDF, start_term, sdate);
            etrading::insertDFData(dfResults_, endDF, end_term, edate);
			etrading::insertForwardRateData(fwd_termsmtx_, fwds_, start_term, end_term, rate);

            // Apply Tension to the Futures part of the Curve by Linear Interpolating on Forwards to Create Synthetic Points 
            const unsigned int dontApplyTensionToLastNFutures = 2;
			
            if ( applyTensionFutures && size_f > 1 && i < size_f - dontApplyTensionToLastNFutures )
			{
				// Use LogDF as StateVariable so Futures and Swaps StateVariables are the Same
				// liborDF_ is a Matrix of Terms and DFs
				DoubleVector terms = dfResults_.paymentDatesAsTerms_;
				DoubleVector logDFs(dfResults_.discountFactors_.size());
				for (size_t i = 0; i < dfResults_.discountFactors_.size(); ++i)
				{
					logDFs[i] = LAMath::log(dfResults_.discountFactors_[i]);
				}
				logDF_FuturesInterpolation->set(terms, logDFs);
				
				bool isLiborAvailable = false;
				if (i == 0)
				{
					for (unsigned int i = 0; i < size_l; i++)
					{						
						const LAString& term_str = dynamic_cast<const LADataString&> ((data_libor[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
						if (term_str != refRateTerm) 
						{
							continue;		
						}
						
						isLiborAvailable = true;
						freq = dynamic_cast<const LADataString&> ((data_libor[i]->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL)).get()).get();
						freq.toUpper();
						double rate = dynamic_cast<const LADataDouble&> ((data_libor[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
						const LAPriceDataCalendar& cal  = dynamic_cast<const LAPriceDataCalendar&> ((data_libor[i]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
						const LAPriceDataSlidingRule& sld  = dynamic_cast<const LAPriceDataSlidingRule&> ((data_libor[i]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
						const LAPriceDataDayCount& dc = dynamic_cast<const LAPriceDataDayCount&> ((data_libor[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
						bool eom = false;
						dh = &(data_libor[i]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK));
						if (dh->isDefined() && !dh->isNull())
						{
							eom = dynamic_cast<const LADataBool &>(dh->get()).get();
						}
						// set roll convention
						LAString roll_conv("");
						if (freq == LUNAR) roll_conv = ROLLCONV_LUNAR;
						else if (eom) roll_conv = ROLLCONV_EOM;
						else roll_conv = ROLLCONV_NORMAL;
						LADate endDate = etrading::LADateHelpers::getDate(spotdate_l, term_str, sld, &cal, true, &roll_conv);
		
						// Use the money market LIBOR rate as the first data point
						insertSyntheticTensionPoints( dfResults_,
													  fwds_,
												      fwd_termsmtx_,
													  logDF_FuturesInterpolation.get(),
													  stateVariableFutureFra,
												      baseDate_, // curve asOfDate
													  spotdate_s,
												      spotdate_l,
												      endDate,
												      rate,
												      true, // isFirstDataPoint
												      dc_act365,
												      dc.getDayCount(),
												      tensionGapFutures,
												      futuresInstrumentData,
												      firstSwapDate ); // tension cut-off date
					}
				}

                insertSyntheticTensionPoints( dfResults_,
                                              fwds_,
                                              fwd_termsmtx_,
											  logDF_FuturesInterpolation.get(),
											  stateVariableFutureFra,
                                              baseDate_, // curve asOfDate
											  spotdate_s,
                                              sdate,
                                              edate,
                                              rate,
                                              ( i == 0 && !isLiborAvailable) ? true : false , // isFirstDataPoint
                                              dc_act365,
                                              dc.getDayCount(),
                                              tensionGapFutures,
                                              futuresInstrumentData,
                                              firstSwapDate ); // tension cut-off date
            }
                
            // For Analytical Risk
			DateVector futureCalcDates(2);
			futureCalcDates[0] =sdate;futureCalcDates[1] = edate;
			if(!data_future[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO,NOCHECK).isDefined())
			{
				data_future[i]->add(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, new LADataDates(futureCalcDates));
			}
			else
			{
				dynamic_cast<LADataDates&>(data_future[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO).get()).set(futureCalcDates);
			}
		}
	}
	
	// Bootstrap curve using FRA instruments
	if (is_fra_use_)
	{
		LAInterpolationBase* forward_interpolation = pInter_fw_.get();
		etrading::bootstrapFRAs( dfResults_, 
								 fwds_, 
								 fwd_termsmtx_,
								 data_fra,	
								 data_libor, 
								 forward_interpolation, 
								 stateVariableFutureFra,
								 is_fwdswap_,
								 baseFreq, 
								 refRateTerm,
								 baseDate_,			// curve asOfDate
								 spotdate_s, 
								 spotdate_l, 
								 firstSwapDate,
								 true,				// implyAndProduceForwards = true
								 interpolationJoinDateAsDouble_,
								 dfResults_.lastLiborPosition_,
								 dfResults_.lastLiborEndDate_ );
	}


	//////////////////////////
	//prepare swap calibration
	//////////////////////////
		
	LADate l_last = dfResults_.paymentDates_.back();
	DateVector dates_s(size_s);
	DoubleVector rate_s(size_s);
	bool isTimeInter  = false;
	dh = &(data_swap[0]->getData(IR_CALIBRATION_DATA_ISTIMEINTERPOLATIONSW, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		isTimeInter = dynamic_cast<const LADataBool &>(dh->get()).get();
	}

	DoubleVector tenorSwapSpreadVec(size_s, 0.0);
	bool isLeg1BaseFreq = false;
	bool isLeg2Spread = false;
	DoubleArray b_t_grid(1, 0.0);
	DoubleArray b_termstruct_grid(1, 0.0);
	std::shared_ptr<LAInterpolationBase> b_inter;
	LAPriceDataDayCount *pDaycount_tenorswap = NULL;
	LAPriceDataCalendar *pCal_tenorswap = NULL;
	LAPriceDataSlidingRule *pSld_tenorswap = NULL;
	LAString tenorSwapFreq = "";
	bool eom_tenorswap = false;
	LAString roll_conv_ts("");
	if (isSwapTenorAdjust_)
	{
		if (data_tenorswap.size() == 0) 
		{
			throw LACoreInvalidData("#Error: Swap Curves with Tenor Basis instruments are only supported using the STD staticDataTable keyword. Please check the 'IsTenorSwapAdjust' and StaticDataTable / MarketData parameter settings.", __FILE__, __LINE__);
		}
		isLeg2Spread = dynamic_cast<const LADataBool&> ((data_tenorswap[0]->getData(IR_CALIBRATION_DATA_ISAGTSPREAD, ISNOTNULL)).get()).get();
		
		// set frequency 
		const LAString& leg1Freq = dynamic_cast<const LADataString&> ((data_tenorswap[0]->getData(IR_CALIBRATION_DATA_CASHLETFREQUENCY, ISNOTNULL)).get()).get();
		const LAString& leg2Freq = dynamic_cast<const LADataString&> ((data_tenorswap[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETFREQUENCY, ISNOTNULL)).get()).get();
		
		if (isLeg2Spread)
		{
			tenorSwapFreq = leg2Freq;
		}
		else
		{
			tenorSwapFreq = leg1Freq;
		}

		if (leg1Freq == baseFreq) 
		{
			isLeg1BaseFreq = true;		
		}
		else if (leg2Freq == baseFreq) 
		{
			isLeg1BaseFreq = false;
		}
		else 
		{
            throw LACoreInvalidData("#Error: Invalid frequency specified. The Tenor basis swap and underlying swap frequencies do not match!", __FILE__, __LINE__);
		}

		dh = &(data_tenorswap[0]->getData(CALIBRATION_DATA_INTERPOLATION, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			b_inter.reset(dynamic_cast<LAInterpolationBase*> ((dynamic_cast<const LAPriceDataInterpolation&> 
				(data_tenorswap[0]->getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get())).getMethod().clone()) );
		}
		else
		{
			b_inter.reset( dynamic_cast<LAInterpolationBase *>(pInter_yg_->clone()) );
		}

		pDaycount_tenorswap = &dynamic_cast<LAPriceDataDayCount &>(data_tenorswap[0]->getData(IR_CALIBRATION_DATA_CASHLETDAYCOUNT, ISNOTNULL).get());
		pCal_tenorswap = &dynamic_cast<LAPriceDataCalendar &>(data_tenorswap[0]->getData(IR_CALIBRATION_DATA_CASHLETCALENDAR, ISNOTNULL).get());
		pSld_tenorswap = &dynamic_cast<LAPriceDataSlidingRule &>(data_tenorswap[0]->getData(IR_CALIBRATION_DATA_CASHLETSLIDINGRULE, ISNOTNULL).get());
		dh = &(data_tenorswap[0]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			eom_tenorswap = dynamic_cast<const LADataBool &>(dh->get()).get();
		}
		// set roll convention
		if (tenorSwapFreq == LUNAR) 
		{
			roll_conv_ts = ROLLCONV_LUNAR;
		}
		else if (eom_tenorswap) 
		{
			roll_conv_ts = ROLLCONV_EOM;
		}
		else 
		{
			roll_conv_ts = ROLLCONV_NORMAL;
		}

		for (unsigned int i = 0; i < data_tenorswap.size(); i++)
		{
			const LAString &strTerm  = dynamic_cast<const LADataString &>((data_tenorswap[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			double rate = dynamic_cast<const LADataDouble &>((data_tenorswap[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
			LADate tmpDate = etrading::LADateHelpers::getDate(spotdate_s, strTerm, *pSld_tenorswap, pCal_tenorswap, true, &roll_conv_ts);
			double term = pDaycount_tenorswap->getTerm(spotdate_s, tmpDate);
			b_t_grid.push_back(term);
			if (isTimeInter)
			{
				b_termstruct_grid.push_back(rate * term);
			}
			else
			{
				b_termstruct_grid.push_back(rate);
			}
		}
		b_inter->set(b_t_grid, b_termstruct_grid);
	}

	DateVector sdates_s(size_s);
	DateVector edates_s(size_s);
	DateVector edates_s_unadjusted(size_s);
	DateVector dates_s_unadjusted(size_s);
	for (unsigned int i = 0; i < size_s; i++)
	{
		LADate date;
		if (is_fwdswap_)
		{
			const bool is_date = dynamic_cast<const LADataBool&> ((data_swap[i]->getData(PRICING_DATA_ISDATE, ISNOTNULL)).get()).get();
			if (is_date)
			{
				sdates_s[i] = dynamic_cast<const LADataDate&> ((data_swap[i]->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get()).get();
				edates_s[i] = dynamic_cast<const LADataDate&> ((data_swap[i]->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get()).get();
			}
			else
			{
				const LAString sterm_str = dynamic_cast<const LADataString&> ((data_swap[i]->getData(PRICING_DATA_STARTTERM, ISNOTNULL)).get()).get();
				sdates_s[i] = etrading::LADateHelpers::getDate(spotdate_s, sterm_str, *sld_s[i], cal_s[i], true, &roll_conv_s[i]);
				const LAString tenor_str = dynamic_cast<const LADataString&> ((data_swap[i]->getData(PRICING_DATA_TENOR, ISNOTNULL)).get()).get();
				edates_s_unadjusted[i] = etrading::LADateHelpers::getDate(sdates_s[i], tenor_str, LAPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr);
				edates_s[i] = etrading::LADateHelpers::getDate(sdates_s[i], tenor_str, *sld_s[i], cal_s[i], true, &roll_conv_s[i]);
			}
			date = edates_s[i];
		}
		else
		{
			const LAString& term_str = etrading::getMaturityAsTermString( i, data_swap );
			dates_s_unadjusted[i] = etrading::LADateHelpers::getDate(spotdate_s, term_str, LAPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr);
			dates_s[i] = etrading::LADateHelpers::getDate(spotdate_s, term_str, *sld_s[i], cal_s[i], true, &roll_conv_s[i]);
			date = dates_s[i];
		}

		if (pRatePriority != 0 && 
			!isSwapPriority && 
			l_last.intervalDays(date) <= RATE_PRIORITY_PROXIMITY_DAY_TOLERANCE) 
		{
			continue;
		}
		
		const double term = dc_act365.getTerm(spotdate_s, date);
		const double rate = dynamic_cast<const LADataDouble&> ((data_swap[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
		rate_s[i] = rate;
		
		if (isSwapTenorAdjust_)
		{
			if (freq_s_float[i] != baseFreq)
			{
				const LAString& term_str_swap = dynamic_cast<const LADataString&> ((data_swap[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
				const LADate tmpDate = etrading::LADateHelpers::getDate(spotdate_s, term_str_swap, *pSld_tenorswap, pCal_tenorswap, true, &roll_conv_ts);
				const double term_basis = pDaycount_tenorswap->getTerm(spotdate_s, tmpDate);
				if (isTimeInter)
				{
					tenorSwapSpreadVec[i] = b_inter->value(term_basis) / term_basis;
				}
				else
				{
					tenorSwapSpreadVec[i] = b_inter->value(term_basis);
				}					

				if ((isLeg2Spread && !isLeg1BaseFreq) || (!isLeg2Spread && isLeg1BaseFreq)) 
				{
					tenorSwapSpreadVec[i] *= -1.;
				}
			}
		}
	    

        //
        // Rate Priority
        // -------------
        // Rate Priority is set here. This is controlled through the 'RatePriority' flag in the Curve Conventions Table
        // This feature controls and manages conflicting data points.
        //
        // Example:     RatePriority = "Libor:Future:Swap" (First use Libor Rates, Second use Futures, Third use Swaps)
        //

		if (pRatePriority == 0) // no rate priority
		{}
		else if (isSwapPriority) // swap is prior to libor and future
		{
			if (i == 0)
			{
				DoubleArray::iterator /*it, it2,*/ it_f, it2_f;
				it_f= dfResults_.paymentDatesAsTerms_.begin();
				it2_f = dfResults_.discountFactors_.begin();
				it_f++;//skip first element (term = 0.0);
				it2_f++;//skip first element (df = 1.0);
				DateVector::iterator it2_d_f = dfResults_.paymentDates_.begin();
				it2_d_f++;//skip first element (date = basedate)
				for (; it_f != dfResults_.paymentDatesAsTerms_.end(); it_f++, it2_f++, it2_d_f++)
				{
					if ((*it2_d_f).intervalDays(date) <= RATE_PRIORITY_PROXIMITY_DAY_TOLERANCE)
					{
						dfResults_.paymentDatesAsTerms_.erase(it_f, dfResults_.paymentDatesAsTerms_.end());
						dfResults_.discountFactors_.erase(it2_f, dfResults_.discountFactors_.end());
						dfResults_.paymentDates_.erase(it2_d_f, dfResults_.paymentDates_.end());
						break;
					}
				}
			}
		}
	}

	bool is_newton = false;
	dh = &(data_swap[0]->getData(IR_CALIBRATION_DATA_ISNEWTONRAPHSONSW, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		is_newton = dynamic_cast<const LADataBool &>(dh->get()).get();
	}

	bool is_simueq = false;
	dh = &(data_swap[0]->getData(IR_CALIBRATION_DATA_ISSIMULTANEOUSEQSW, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		is_simueq = dynamic_cast<const LADataBool &>(dh->get()).get();
	}		
		
	LAString fixingLag("0D");

	// Eligibility checkings prior to swap solving
	if (!is_newton)
	{
        throw LACoreInvalidData("#Error: Only the Newton-Raphson method is supported", __FILE__, __LINE__); 
	}

	if (!is_simueq)
	{
        throw LACoreInvalidData("#Error: Only the Simultaneous-Equation method is supported", __FILE__, __LINE__); 
	}
	
	std::vector<DateVector> datesVec_ts(size_s);
	datesVec_.resize(size_s);
	datesVec_float_.resize(size_s);

	fixingStartDates_.resize(size_s);
	fixingEndDates_.resize(size_s);
	
	rate_tauVec_.resize(size_s);
	rate_tauVec_float_.resize(size_s);
	rate_tauVec_ts_.resize(size_s);

	terms_gridVec_.resize(size_s);
	terms_gridVec_float_.resize(size_s);
	terms_interval_float_.resize(size_s);
	terms_gridVec_ts_.resize(size_s);
	terms_grid_s_float_.resize(size_s);					

	fixingStarts_.resize(size_s);
	fixingEnds_.resize(size_s);
	fixingTaus_.resize(size_s);
			
	// Set up swap date schedules in preparation for pricing
	for (unsigned int i = 0; i < size_s; i++)
	{
        //
        // Swap Product Set-Up				
        // ------------------------------------------------------------------------------				
		DoubleArray terms_interval, terms_interval_ts;
		if (is_fwdswap_)
		{
			etrading::updateAccrualPeriodsAndPaymentDates(sdates_s[i], edates_s_unadjusted[i], freq_s[i], *cal_s[i], *sld_s[i], *dc_s[i], 
										datesVec_[i], terms_gridVec_[i], terms_interval, eom_s[i], &spotdate_s);
		}
		else
		{
			etrading::updateAccrualPeriodsAndPaymentDates(spotdate_s, dates_s_unadjusted[i], freq_s[i], *cal_s[i], *sld_s[i], *dc_s[i], 
										datesVec_[i], terms_gridVec_[i], terms_interval, eom_s[i]);
		}
		DateVector tmpdates(datesVec_[i]);
		tmpdates.insert(tmpdates.begin(), spotdate_s);
		if(!data_swap[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO,NOCHECK).isDefined())
		{
			data_swap[i]->add(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, new LADataDates(tmpdates));
		}
		else
		{
			dynamic_cast<LADataDates&>(data_swap[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO).get()).set(tmpdates);
		}
		unsigned int size_grid = datesVec_[i].size();
		rate_tauVec_[i].resize(size_grid);
		for (unsigned int j = 0; j < size_grid; j++)
		{
			rate_tauVec_[i][j] = rate_s[i] * terms_interval[j];
		}								
		if (isSwapTenorAdjust_)
		{
			etrading::updateAccrualPeriodsAndPaymentDates(spotdate_s, dates_s_unadjusted[i], tenorSwapFreq, *pCal_tenorswap, *pSld_tenorswap, *pDaycount_tenorswap, datesVec_ts[i], terms_gridVec_ts_[i], terms_interval_ts, eom_tenorswap);
			unsigned int size_grid_ts = datesVec_ts[i].size();
			rate_tauVec_ts_[i].resize(size_grid_ts);
			for (unsigned int j = 0; j < size_grid_ts; j++)
			{
				rate_tauVec_ts_[i][j] = tenorSwapSpreadVec[i] * terms_interval_ts[j];
			}
		}
				
		if (baseFreq != "" && dc_s_float[i])
		{
			// calc floatside date information
			if (is_fwdswap_)
			{
				etrading::updateAccrualPeriodsAndPaymentDates(sdates_s[i], edates_s_unadjusted[i],  baseFreq, *cal_s[i], *sld_s[i], *dc_s_float[i], 
											datesVec_float_[i], terms_gridVec_float_[i], terms_interval_float_[i], eom_s[i], &spotdate_s);
				terms_grid_s_float_[i] = dc_act365.getTerm(spotdate_s, sdates_s[i]);
			}
			else
			{
				etrading::updateAccrualPeriodsAndPaymentDates(spotdate_s, dates_s_unadjusted[i], baseFreq, *cal_s[i], *sld_s[i], *dc_s_float[i], 
											datesVec_float_[i], terms_gridVec_float_[i], terms_interval_float_[i], eom_s[i]);
			}

			// Instatiate fixing start dates using accrual start dates	
			fixingStartDates_[i].push_back(spotdate_s);
			for(size_t j = 0; j < datesVec_float_[i].size() - 1; ++j)
			{
				fixingStartDates_[i].push_back(datesVec_float_[i][j]);							
			}

			// Calculate fixing start dates by applying fixing lag
			fixingStartDates_[i] = etrading::LADateScheduleHelpers::calcDatesWithLag( fixingStartDates_[i],
																		           fixingLag,
																		           *sld_s[i],
																		           cal_s[i],
																		           false,		// isAfter
																		           nullptr ); // This RollConv = Start, End, IMM, EOM or NULL

			// fixing end dates are always a fixed term after the corresponding fixing start dates
			fixingEndDates_[i] = etrading::LADateScheduleHelpers::getMultiDate( fixingStartDates_[i], refRateTerm, sld_s[i]->convertToString(), cal_s[i]->convertToString(), nullptr); // rollconvention* = nullptr

			for(size_t j = 0; j < fixingStartDates_[i].size(); ++j)
			{
				fixingStarts_[i].push_back(dc_act365.getTerm(spotdate_s, fixingStartDates_[i][j]));
				fixingEnds_[i].push_back(dc_act365.getTerm(spotdate_s, fixingEndDates_[i][j]));						
				double fixingTau = dc_s_float[i]->getTerm(fixingStartDates_[i][j], fixingEndDates_[i][j]);
				fixingTaus_[i].push_back(fixingTau);
			}
		}
	}
            		

	//
    // Solve for Swap Data Points
    // ------------------------------------------------------------------------------
			
	// Initialise state variables in preparation of solving
	preSwapSize_ = dfResults_.paymentDatesAsTerms_.size();
	stateVariable_grid_ = dfResults_.paymentDatesAsTerms_;
	stateVariable_rates_.resize(preSwapSize_);
	for (unsigned int j = 0; j < preSwapSize_; j++)
	{
		stateVariable_rates_[j] = -LAMath::log(dfResults_.discountFactors_[j]);
	}

	// Setup a default initial guess
	swapCount_ = size_s;

	DoubleArray maturityGrids;
	for (unsigned int i = 0; i < swapCount_; ++i)
	{
		maturityGrids.push_back(terms_gridVec_[i].back());
	}
		
	etrading::initialiseStateVariablesForSolving(stateVariable_rates_,
												stateVariable_grid_,
												objHolder,
												stateVariable,
												rate_s,
												maturityGrids,
												fastRebuild_,
												targetSuffix,
												swapCount_);
	
	// Initialise interpolationObject which holds the logDF that are being solved for
	pInter_StateVariable_ = std::shared_ptr<LAInterpolationBase>( (dynamic_cast<LAInterpolationBase *>(pInter_yg_->clone())) );
	pInter_StateVariable_->setJoinDateAsDouble( interpolationJoinDateAsDouble_ );
	pInter_StateVariable_->set(stateVariable_grid_, stateVariable_rates_);
	 
	// Optimise linear spline join date when using futures
	if (!joinDateGiven && 
		calculateJoinDate &&
		is_f_use_ && 
		size_f != 0 && 
		pInter_StateVariable_->isHybrid())
	{
		std::unique_ptr<LAInterpolationBase> tempInterp;
		tempInterp = std::unique_ptr<LAInterpolationBase>( (dynamic_cast<LAInterpolationBase *>(pInter_yg_->clone())) );

		InterpolationJoinDate optimizeJoinDate = optimizeInterpolationJoinDate(tempInterp, futuresStartDateTerms, futuresEndDateTerms, futuresRates, stateVariable_grid_, stateVariable_rates_ );

		// If join date optimisation succeeds, set it to all the relevant interpolators;
		// otherwise, do nothing and move on using the original join date.
		if (optimizeJoinDate.success_)
		{
			interpolationJoinDateAsDouble_ = optimizeJoinDate.joinDateAsDouble_;			

			// Recover the join date using interpolationJoinDateAsDouble and make sure
			// the final join date is not a holiday
			double joinDateZoneStart = futuresStartDateTerms.back();
			double fractionFromLastFutureStartToJoin = interpolationJoinDateAsDouble_ - joinDateZoneStart;

			LAPriceDataDayCount dc = *dc_s_float.front();
			LAPriceDataCalendar cal = *cal_s.front();
			LAPriceDataSlidingRule sl = *sld_s.front();
			interpolationJoinDate_ = etrading::LADateScheduleHelpers::getDateFromTerm(lastFutureStartDate, fractionFromLastFutureStartToJoin, dc);
			
			// Make sure the join date is not a holiday
			LADate adjustedJoinDate = etrading::LADateScheduleHelpers::getDate(interpolationJoinDate_, "0D", sl.convertToString(), cal.convertToString());
			if (adjustedJoinDate != interpolationJoinDate_)
			{
				interpolationJoinDate_ = adjustedJoinDate;
				interpolationJoinDateAsDouble_ = dc_act365.getTerm( spotdate_s, interpolationJoinDate_ );
			}
						
			pInter_StateVariable_->setJoinDateAsDouble( interpolationJoinDateAsDouble_ );
			pInter_StateVariable_->set(stateVariable_grid_, stateVariable_rates_);

			// Set the Linear-Spline Interpolation Join Date for the Disc Factor Interpolation Table
			if ( pInter_IsHybrid )
			{
				pInter_->setJoinDateAsDouble( optimizeJoinDate.joinDateAsDouble_ );
			}

			// Set the Linear-Spline Interpolation Join Date for the Swap Rate Interpolation Table
			if ( pInter_yg_IsHybrid )
			{
				pInter_yg_->setJoinDateAsDouble( optimizeJoinDate.joinDateAsDouble_ );
			}

			// Set the Linear-Spline Interpolation Join Date for the Forward Interpolation Table
			if ( pInter_fw_IsHybrid )
			{
				pInter_fw_->setJoinDateAsDouble( optimizeJoinDate.joinDateAsDouble_ );
			}
		}
	}
}


/*
    @brief Calculate the PV of all calibration instruments in the solver
    
	@param[out] allPVs		PV of all instruments
*/
void SwapComponentCurve::priceCalibrationInstruments(DoubleArray& allPVs)
{
	allPVs.clear();
	for (unsigned int i = 0; i < terms_gridVec_.size(); ++i)
	{
		double df = 1.0;
		double val = 0.0;
		if (isMultiCurve_)
		{
			for (unsigned int j = 0; j < terms_gridVec_[i].size(); j++)
			{
				// Discount to spot date
				df = LAMath::exp(-1 * pInter_DF_->value(terms_gridVec_[i][j]) );

				val += rate_tauVec_[i][j] * df;
			}

			if (isSwapTenorAdjust_)
			{
				for (unsigned int j = 0; j < terms_gridVec_ts_[i].size(); j++)
				{
					// Discount to spot date
					df = LAMath::exp(-1 * pInter_DF_->value(terms_gridVec_ts_[i][j]) );

					val += rate_tauVec_ts_[i][j] * df;
				}
			}

			if (is_fwdswap_)
			{
				if (fixingStarts_.size() == 0)
				{
					val -= calcFloatLegPV(pInter_StateVariable_, pInter_DF_, terms_gridVec_float_[i], cpd_times_[i], terms_grid_s_float_[i]);
				}
				else
				{
					val -= calcFloatLegPV(pInter_StateVariable_, pInter_DF_, terms_gridVec_float_[i], cpd_times_[i], terms_grid_s_float_[i], terms_interval_float_[i], fixingStarts_[i], fixingEnds_[i], fixingTaus_[i]);
				}
			}
			else						
			{
				if (fixingStarts_.size() == 0)
				{
					// Maintaining backward compatibility when no fixing dates are not present
					val -= calcFloatLegPV(pInter_StateVariable_, pInter_DF_, terms_gridVec_float_[i], cpd_times_[i], 0.0);
				}
				else
				{
					val -= calcFloatLegPV(pInter_StateVariable_, pInter_DF_, terms_gridVec_float_[i], cpd_times_[i], 0.0, terms_interval_float_[i], fixingStarts_[i], fixingEnds_[i], fixingTaus_[i]);
				}
			}
		}
		else
		{
			for (unsigned int j = 0; j < terms_gridVec_[i].size(); j++)
			{
				df = LAMath::exp(-pInter_StateVariable_->value(terms_gridVec_[i][j]));
				val += rate_tauVec_[i][j] * df;
			}

			if (isSwapTenorAdjust_)
			{
				for (unsigned int j = 0; j < terms_gridVec_ts_[i].size(); j++)
				{
					df = LAMath::exp(-pInter_StateVariable_->value(terms_gridVec_ts_[i][j]));
					val += rate_tauVec_ts_[i][j] * df;
				}
			}
			val += 1.0 *  df;
			val -= 1.0;
		}
		allPVs.push_back(val);
	}
}


/*
	@brief calc float side pv on spot date
    
	@param[in] inter			inter of index
	@param[in] df_inter			inter of DF
	@param[in] terms_grid		grid
	@param[in] cpd_times 		compounding times (optionally)
	@param[in] term_start		start term (might be fwd starting, optionally)
	@param[in] floatAccrualPeriods		Date count fraction of the intervals between accural dates on floating leg
	@param[in] fixingStarts		Date count fraction for all fixing start dates
	@param[in] fixingEnds		Date count fraction for all fixing end dates
	@param[in] fixingTaus		Date count fraction for the time interval between all fixing start and end dates

	@return floatside pv
*/
double SwapComponentCurve::calcFloatLegPV(std::shared_ptr<LAInterpolationBase>& inter, 
										std::shared_ptr<LAInterpolationBase>& df_inter, 
										const DoubleArray &terms_grid, 
										const int cpd_times, 
										const double term_start,
										const DoubleArray& floatAccrualPeriods,
										const DoubleArray& fixingStarts,
										const DoubleArray& fixingEnds,
										const DoubleArray& fixingTaus) const
{
	double ret = 0.0;
	if (terms_grid.empty())
	{
		return ret;
	}

	double df = 1.0;
	double df_index = 1.0;
	double df_index_b = LAMath::exp(-inter->value(term_start));
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
				// This code snippet is not considring fixings and making an
				// assumption that fixing tau is the same as accrual tau
				df_index =  LAMath::exp(-inter->value(terms_grid[j]));
				const double df_ratio = df_index_b / df_index;
				// calc (1+tau*F)^n
				cpd *= df_ratio;
				df_index_b = df_index;
			}
			else
			{
				const double df_s =  LAMath::exp(-inter->value(fixingStarts[j]));
				const double df_e =  LAMath::exp(-inter->value(fixingEnds[j]));
				
				const double df_ratio = df_s / df_e;
				const double r = (df_ratio - 1) / fixingTaus[j];				
				cpd *= (1 + r * fixingTaus[j]);

				fixingTau += fixingTaus[j];
				accuralTau += floatAccrualPeriods[j];
			}
		}

		// Discount to spot date
		df = LAMath::exp(-1 * df_inter->value(terms_grid[l * cpd_times - 1]) );

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
			// Discount to spot date
			df = LAMath::exp(-1 * df_inter->value(terms_grid[j]) );

			if (fixingStarts.size() == 0)
			{
				// This code snippet is not considring fixings and making an
				// assumption that fixing tau is the same as accrual tau
				df_index =  LAMath::exp(-inter->value(terms_grid[j]));
				const double df_ratio = df_index_b / df_index;				
				// calc L*tau*df
				ret += (df_ratio - 1.0) * df;
				df_index_b = df_index;
			}
			else
			{
				const double df_s =  LAMath::exp(-inter->value(fixingStarts[j]));
				const double df_e =  LAMath::exp(-inter->value(fixingEnds[j]));
				const double df_ratio = df_s / df_e;
				ret += (df_ratio -1) / fixingTaus[j] * floatAccrualPeriods[j] * df;
			}
		}	
	}		
	return ret;
}

// Post processing results once instruments have been consumed in calibration steps
void SwapComponentCurve::postProcessing(LAObject& yieldCurveProEntity)
{
	//
    // Update and Insert Swap Data Points
    // ------------------------------------------------------------------------------
	for (unsigned int i = 0; i < swapCount_; ++i)
	{
		// As long as floating leg exists, we use floating leg dates as pillar dates
		if (!datesVec_float_.back().empty())
		{
			for (unsigned int j = 0; j < terms_gridVec_float_[i].size(); ++j)
			{
				etrading::insertDFData(dfResults_, LAMath::exp(-pInter_StateVariable_->value(terms_gridVec_float_[i][j])), terms_gridVec_float_[i][j], datesVec_float_[i][j]);
			}
		}
		else
		{
			for (unsigned int j = 0; j < terms_gridVec_[i].size(); ++j)
			{
				etrading::insertDFData(dfResults_, LAMath::exp(-pInter_StateVariable_->value(terms_gridVec_[i][j])), terms_gridVec_[i][j], datesVec_[i][j]);
			}
		}
	}

	const etrading::StateVariableEnum stateVariable = STATE_VARIABLE_ZERO_RATE_TIMES_TIME;
	const etrading::DayCountEnum liborDaycount = dc_Libor_.dayCountEnum();


	if (!fixingStarts_.empty())
	{
		etrading::insertDFsForFixingStartEnds(dfResults_, *pInter_StateVariable_, stateVariable, fixingStarts_.back(), fixingEnds_.back(), fixingStartDates_.back(), fixingEndDates_.back(), generateForwardsFromSwapsOnly_, baseDate_, liborDaycount);

		etrading::updateImpliedForwardRates(*pInter_StateVariable_, stateVariable, fixingStarts_.back(), fixingEnds_.back(), fixingTaus_.back(), generateForwardsFromSwapsOnly_, baseDate_, liborDaycount, fwd_termsmtx_, fwds_);
	}
	else
	{
		const DateVector *p_dateVec;
		p_dateVec = &datesVec_.back();

		DoubleVector i_starts, i_ends;
		DoubleArray tau_swap(p_dateVec->size());
		tau_swap[0] = dc_Libor_.getTerm(spotDate_, (*p_dateVec)[0]);
		for (unsigned int i = 1; i < tau_swap.size(); ++i)
		{
			tau_swap[i] = dc_Libor_.getTerm((*p_dateVec)[i - 1], (*p_dateVec)[i]);
			i_starts.push_back( terms_gridVec_.back()[i - 1]);
			i_ends.push_back( terms_gridVec_.back()[i - 1]);
		}

		i_starts.insert(i_starts.begin(), 0.0);
		i_ends.push_back(terms_gridVec_.back().back());
			
		etrading::updateImpliedForwardRates(*pInter_StateVariable_, stateVariable, i_starts, i_ends, tau_swap, generateForwardsFromSwapsOnly_, baseDate_, liborDaycount, fwd_termsmtx_, fwds_);
	}

	//////////////////////////////
	//calculate AsofDate base df//
	//////////////////////////////
	DoubleMatrix df_moneymarket(2);
	DateVector df_moneymarket_date;
	LAPriceDataDayCount dc_act365(ACT_365);
	std::map<std::pair<LADate, LADate>, const LAObject*>::const_iterator it, it_last, it_tmp;
	double term_from_asof = 0.0;
	double df = 1.0;
	double dfSpot = 1.0;
	it_last = moneyMarketDataMap_.end();
	it_last--;
	for (it = moneyMarketDataMap_.begin(); it != moneyMarketDataMap_.end(); it++)
	{
		it_tmp = it;
		it_tmp--;
		if (it != moneyMarketDataMap_.begin() && it->first.first != it_tmp->first.second)
		{
			//error
            LAString msg = "#Error: Invalid Money Market data, money market end dates must not overlap the start date of next instrument";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}

		double rate = dynamic_cast<const LADataDouble&> ((it->second->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
		const LAPriceDataDayCount& dc  = dynamic_cast<const LAPriceDataDayCount&> ((it->second->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
		double term = dc.getTerm(it->first.first, it->first.second, false);
		term_from_asof += dc_act365.getTerm(it->first.first, it->first.second);
		double df_tmp = 1.0 / (1.0 + rate * term);					
		df *= df_tmp;
		df_moneymarket[0].push_back(term_from_asof);
		df_moneymarket[1].push_back(df);
		df_moneymarket_date.push_back(it->first.second);

		if (it->first.second < spotDate_)
		{
			it_tmp = it;
			it_tmp++;
			if (it == it_last)
			{
				df_tmp *= 1.0 / (1.0 + rate * dc.getTerm(it->first.second, spotDate_, false));
				//warning
				fprintf(stderr, "Warning!!! Money market instruments shorter than the curve spotdate\n");
			}
			dfSpot *= df_tmp;
		}
		else if (it->first.second > spotDate_)
		{
			if (it->first.first < spotDate_)
			{
				dfSpot *= 1.0 / (1.0 + rate * dc.getTerm(it->first.first, spotDate_, false));
			}
		}
		else
		{
			dfSpot *= df_tmp;
		}		
	}
	
	if (df_moneymarket_date.end() != find(df_moneymarket_date.begin(), df_moneymarket_date.end(), dfResults_.paymentDates_[0]))
	{
		// Erase first element ( df = 1.0, t = 0, date = spodate of swap )
		dfResults_.paymentDatesAsTerms_.erase(dfResults_.paymentDatesAsTerms_.begin());
		dfResults_.discountFactors_.erase(dfResults_.discountFactors_.begin());
		dfResults_.paymentDates_.erase(dfResults_.paymentDates_.begin());
	}

	const double spotTerm = dc_act365.getTerm(baseDate_, spotDate_);
	for (unsigned int i = 0; i < fwd_termsmtx_[0].size(); i++)
	{
		fwd_termsmtx_[0][i] += spotTerm;
		fwd_termsmtx_[1][i] += spotTerm;
	}

	unsigned int size_data2 = dfResults_.discountFactors_.size();
	for (unsigned int i = 0; i < size_data2; i++)
	{
		dfResults_.paymentDatesAsTerms_[i] += spotTerm;
		dfResults_.discountFactors_[i] *= dfSpot;
	}

	
	////////////
	// Output //
	////////////
	DoubleArray swapCurve_OutputTerms	= dfResults_.paymentDatesAsTerms_;
	DoubleArray swapCurve_OutputDFs		= dfResults_.discountFactors_;

	unsigned int pos;
	unsigned int size_data = df_moneymarket_date.size();
	LAAlgorithm::locate(df_moneymarket_date, dfResults_.paymentDates_[0], size_data, pos);
	if (pos == size_data)
	{
		swapCurve_OutputTerms.insert(swapCurve_OutputTerms.begin(), df_moneymarket[0].begin(), df_moneymarket[0].end());
		swapCurve_OutputDFs.insert(swapCurve_OutputDFs.begin(), df_moneymarket[1].begin(), df_moneymarket[1].end());
		dfResults_.paymentDates_.insert(dfResults_.paymentDates_.begin(), df_moneymarket_date.begin(), df_moneymarket_date.end());
	}
	else
	{
		swapCurve_OutputTerms.insert(swapCurve_OutputTerms.begin(), df_moneymarket[0].begin(), df_moneymarket[0].begin() + pos);
		swapCurve_OutputDFs.insert(swapCurve_OutputDFs.begin(), df_moneymarket[1].begin(), df_moneymarket[1].begin() + pos);
		dfResults_.paymentDates_.insert(dfResults_.paymentDates_.begin(), df_moneymarket_date.begin(), df_moneymarket_date.begin() + pos); 
		unsigned int pos2;
		for (unsigned int i = pos; i < size_data; i++)
		{
			LAAlgorithm::locate(dfResults_.paymentDates_, df_moneymarket_date[i], dfResults_.paymentDates_.size(), pos2);
			if (pos2 == dfResults_.paymentDates_.size())
			{
				swapCurve_OutputTerms.push_back(df_moneymarket[0][i]);
				swapCurve_OutputDFs.push_back(df_moneymarket[1][i]);	
				dfResults_.paymentDates_.push_back(df_moneymarket_date[i]);
			}
			else if (dfResults_.paymentDates_[pos2] != df_moneymarket_date[i])
			{
				swapCurve_OutputTerms.insert(swapCurve_OutputTerms.begin() + pos2, df_moneymarket[0][i]);
				swapCurve_OutputDFs.insert(swapCurve_OutputDFs.begin() + pos2, df_moneymarket[1][i]);	
				dfResults_.paymentDates_.insert(dfResults_.paymentDates_.begin() + pos2, df_moneymarket_date[i]);
			}
		}
	}
	
	//add term =0 and df = 1.0
	if (swapCurve_OutputTerms[0] != 0.0)
	{
		swapCurve_OutputTerms.insert(swapCurve_OutputTerms.begin(), 0.0);
		swapCurve_OutputDFs.insert(swapCurve_OutputDFs.begin(), 1.0);
	}

	LAObjectHolder objHolder = yieldDataRef_.get();

	// Get the state variable rates at solution 
	DoubleArray stateVariableAtSolution;
	for (unsigned int i = 0; i < swapCount_; ++i)
	{
		stateVariableAtSolution.push_back(stateVariable_rates_[preSwapSize_ + i]);
	}

	// Note: At the end of the old sequential curve building logic there is a snippet of code that deals with
	// extrapolating curves in the context of simulation. Such a logic has been omitted in here.
	
	// Go through all curve index names and store outputs under these names
	const std::map<LAString, bool>& genCurveGenMap = dynamic_cast<CurveCalibrationData &>(yieldCurveProEntity).getGCurveGenerateMap();
	const std::map<LAString, LAString>& assignedCurveMktMap = dynamic_cast<CurveCalibrationData &>(yieldCurveProEntity).getAssignedCurveMktMap();

	for (std::map<LAString, LAString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
	{
		if (it->second == SWAP)
		{
			// 'curveName' is one of the curve index names or the 'static data table name'
			const LAString& curveName = it->first;
			LAString suffix;
			if (curveName != STD) 
			{
				suffix = "_" + curveName;
			}

			// set result
			objHolder.remove(CALIBRATION_DATA_TERMS + suffix);
			objHolder.remove(CALIBRATION_DATA_FWDTERMSMATRIX + suffix);
			objHolder.remove(IR_CALIBRATION_DATA_DFS + suffix);
            objHolder.remove(IR_CALIBRATION_DATA_FORWARDRATES + suffix);
			objHolder.remove(CALIBRATION_DATA_INTERPOLATION + suffix);
			objHolder.remove(IR_CALIBRATION_DATA_INTERPOLATIONYG + suffix);
			objHolder.remove(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + suffix);
			objHolder.remove(CALIBRATION_DATA_INTERPOLATION_JOINDATE + suffix);
			objHolder.remove(IR_CALIBRATION_DATA_JACOBIAN_STATE_VARIABLES + suffix);
			objHolder.remove(IR_CALIBRATION_DATA_BUILTBYCURVEENGINE + suffix);

			objHolder.add(CALIBRATION_DATA_TERMS + suffix, new LADataDoubles(swapCurve_OutputTerms));
			objHolder.add(CALIBRATION_DATA_FWDTERMSMATRIX + suffix, new LADataDoubleMatrix(fwd_termsmtx_));
			objHolder.add(IR_CALIBRATION_DATA_FORWARDRATES + suffix, new LADataDoubles(fwds_));
            objHolder.add(IR_CALIBRATION_DATA_DFS + suffix, new LADataDoubles(swapCurve_OutputDFs));
			objHolder.add(CALIBRATION_DATA_INTERPOLATION + suffix, new LAPriceDataInterpolation()).convertFromString(interpolationStr_);
			objHolder.add(IR_CALIBRATION_DATA_INTERPOLATIONYG + suffix, new LAPriceDataInterpolation()).convertFromString(interpolationYGStr_);
			objHolder.add(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + suffix, new LADataDouble(interpolationJoinDateAsDouble_));
			objHolder.add(CALIBRATION_DATA_INTERPOLATION_JOINDATE + suffix, new LADataDate(interpolationJoinDate_));
			objHolder.add(IR_CALIBRATION_DATA_JACOBIAN_STATE_VARIABLES + suffix, new LADataDoubles(stateVariableAtSolution));
			objHolder.add(IR_CALIBRATION_DATA_BUILTBYCURVEENGINE + suffix, new LADataBool(true));

			CurveCalibration::setCurveConvention(objHolder, data_, curveName);
		}
	}

	dynamic_cast<CurveCalibrationData &>(yieldCurveProEntity).setGCurveGenerateMap(STD);
	for (std::map<LAString, LAString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
	{
		if (it->second == SWAP)
		{
			const LAString& curveName = it->first;
			LAString suffix;
			if (curveName != STD) 
			{
				suffix = "_" + curveName;
			}
			dynamic_cast<CurveCalibrationData &>(yieldCurveProEntity).setGCurveGenerateMap(curveName);
		}
	}

}
