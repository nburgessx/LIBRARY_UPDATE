/*
 * @brief			OIS component curve used in global curve calibration engine
 * @Created:		11 Jan 2018
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */



#include "OISComponentCurve.h"
#include "LACurvePricingObject.h"
#include "LAMathDefine.h"
#include "LAInterpolationBase.h"
#include "LAPriceDataInterpolation.h"
#include "LADataMultiReference.h"
#include "LAPriceDataDayCount.h"
#include "LAPriceDataSlidingRule.h"
#include "LAPriceDataCalendar.h"
#include "LADataHolder.h"
#include "LAObject.h"
#include "LAObjectHolder.h"
#include "LADataVector.h"
#include "LADate.h"
#include "LABasic.h"
#include "LADataReference.h"
#include "CurveInstruments.h"
#include "LALinearInterpolation.h"
#include "ConstantDeclarations.h"
#include "CurveCalibration.h"
#include "LADateScheduleHelpers.h"

#include <memory>

/*
    @brief Constructor
*/
OISComponentCurve::OISComponentCurve(const LAString& curveName,
	 								const CurveCalibrationData& curveCalibrationData,
									const LADate& baseDate,
									const bool& fastRebuildRequested)
			: GlobalCalibrationComponentCurve(curveName, baseDate, fastRebuildRequested),
			  spotterm_(0)
{
	LAString suffix_mkt = "_" + curveName_;
	suffix_mkt.toUpper();

	// get market data
	data_.clear();
	const LADataMultiReference& mr = dynamic_cast<const LADataMultiReference&>(curveCalibrationData.getData(CALIBRATION_DATA_MARKETDATA + suffix_mkt, ISNOTNULL).get());	
	for(unsigned int j = 0; j < mr.getSize(); j++)
	{
		data_.push_back(&mr.get(j).get());
	}

	// get YieldGen Interpolation	
	const LADataHolder *dh = &curveCalibrationData.getData(IR_CALIBRATION_DATA_INTERPOLATIONYG + suffix_mkt, NOCHECK);
	interpolationYGStr_ = "";	
	pInter_yg_.reset();
	if (dh->isDefined() && !dh->isNull())
	{
		LAPriceDataInterpolation tempInterp = dynamic_cast<const LAPriceDataInterpolation &>(dh->get());
		interpolationYGStr_ = tempInterp.convertToString();
		pInter_yg_ = std::shared_ptr<LAInterpolationBase> (dynamic_cast<LAInterpolationBase*>(tempInterp.getMethod().clone()));		
	}
	
	// Yield curve data as a reference
	yieldDataRef_ = curveCalibrationData.getYieldData();

	// Initiliase the curve in preparation for calibration
	initialise();
}


/*
    @brief Initialise the curve calibration process prior to going into solver
*/
void OISComponentCurve::initialise()
{
	//-------------------------------------------------------------------------------
	// Classify data object by data type 

	data_on_.clear();
	data_tn_.clear();

	std::vector<LAObject*> data_mpc_swaps, data_swap; 
	unsigned int size_data = data_.size();
	LAString datatype_str;
	LAPriceDataDayCount dc_act365(ACT_365);
	for(unsigned i = 0; i < size_data; i++)
	{
		// check use grid
		const LADataHolder *dh = &data_[i]->getData(IR_CALIBRATION_DATA_GRIDUSEFLAG, NOCHECK);
		if (dh->isDefined() && !dh->isNull() && !dynamic_cast<const LADataBool &>(dh->get()).get()) continue;

		datatype_str = dynamic_cast<const LADataString&> ((data_[i]->getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
		datatype_str.toUpper();
		
		if (datatype_str == BOJ)
		{
			data_mpc_swaps.push_back(data_[i]);//mpc case = monetary policy committee swaps
		}
		else if (datatype_str == FEDFUNDRATE) 
		{
			//for fed fund first grids
			data_mpc_swaps.push_back(data_[i]);//mpc case = monetary policy committee swaps
		}
		else if (datatype_str == PAR)
		{
			const LAString& term = dynamic_cast<const LADataString&> ((data_[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			if (term == "ON") 
			{
				data_on_.push_back(data_[i]);
			}
			else if (term == "TN")
			{
				data_tn_.push_back(data_[i]);
			}
			else
			{
				data_swap.push_back(data_[i]);//swap case
			}
		}
	}
			
	// ascending sort boj, swap data entities  
	sort(data_mpc_swaps.begin(), data_mpc_swaps.end(), InstrumentComp());
	sort(data_swap.begin(), data_swap.end(), InstrumentComp());
	unsigned int size_mpc_swaps = data_mpc_swaps.size();//central bank swap count
	unsigned int size_s = data_swap.size();//swap count
	unsigned int size_on = data_on_.size();//on count
	unsigned int size_tn = data_tn_.size();//tn count
	if (size_s == 0)
	{
        throw LACoreInvalidData("#Error: Invalid market data. At least one outright OIS swap must be specified", __FILE__, __LINE__);
	}

	// Short term swaps overrule central bank swaps?
	bool shortTermSwapOverrules = false;	
	const LADataHolder *dh = &data_.front()->getData(IR_CALIBRATION_DATA_SHORTTERMSWAPOVERRULES, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		shortTermSwapOverrules = dynamic_cast<const LADataBool &>(dh->get()).get();
	}
	//-------------------------------------------------------------------------------
	//Spot date 

	size_data = size_mpc_swaps + size_s;
	for (unsigned int i = 0; i < size_data; ++i)
	{
		const LAObject *data = 0;
		if (i < size_mpc_swaps)
		{
			data = data_mpc_swaps[i];
		}
		else
		{
			data = data_swap[i - size_mpc_swaps];
		}

		const LADate& spotdateTemp = dynamic_cast<const LADataDate&> ((data->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get());
		if (i == 0) 
		{
			spotDate_ = spotdateTemp;
		}
		else if (spotdateTemp != spotDate_)
		{
            throw LACoreInvalidData("#Error: All market data must have same spotdate", __FILE__, __LINE__);			
		}
	}
	

	//-------------------------------------------------------------------------------
	// Get standard swap conventions

	const LAPriceDataDayCount& dc = dynamic_cast<const LAPriceDataDayCount&> ((data_swap[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
	const LAPriceDataCalendar& cal  = dynamic_cast<const LAPriceDataCalendar&> ((data_swap[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
	const LAPriceDataSlidingRule& sld  = dynamic_cast<const LAPriceDataSlidingRule&> ((data_swap[0]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
	RateConvention rc = LAMathYieldCurve::setRC(SIMPLE);
	LAPriceDataConvention conv(dc.getDayCount(), rc);
		
	// Get shortterm_date
	// If not given directly as an input, short term date is the first central bank swap start date. Failing that, it is the spot date
	LADate shortterm_date = spotDate_;

	dh = &data_swap[0]->getData(IR_CALIBRATION_DATA_SHORTTERMDATE, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		shortterm_date = dynamic_cast<const LADataDate&>(dh->get());
		if (size_mpc_swaps)
		{
			const LADate &mpc_swap_enddate = dynamic_cast<const LADataDate&> ((data_mpc_swaps.back()->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get());
			if (shortterm_date > mpc_swap_enddate)
			{
				shortterm_date = mpc_swap_enddate;
			}
		}
	}
	else
	{
		if (size_mpc_swaps)
		{
			shortterm_date = dynamic_cast<const LADataDate&> ((data_mpc_swaps.back()->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get());
		}
	}	

	shortterm_date = sld.getDate(shortterm_date, cal);

	bool eom = false;
	dh = &(data_swap[0]->getData(IR_CALIBRATION_DATA_ISEOMROLLOIS, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		eom = dynamic_cast<const LADataBool &>(dh->get()).get();
	}
	LAString freq = dynamic_cast<const LADataString&> ((data_swap[0]->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL)).get()).get();
	freq.toUpper();

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

	//-------------------------------------------------------------------------------
	// Group together short end swaps that sit in front of Central Bank Swaps

	LADate firstCBSStartDate;
	if (size_mpc_swaps > 0)
	{
		if (shortTermSwapOverrules)
		{
			if (size_mpc_swaps > 2)
			{
				firstCBSStartDate = dynamic_cast<const LADataDate&> ((data_mpc_swaps[1]->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get());
			}
			else
			{
				firstCBSStartDate = dynamic_cast<const LADataDate&> ((data_mpc_swaps[0]->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get());
			}
		}
		else
		{
			firstCBSStartDate = dynamic_cast<const LADataDate&> ((data_mpc_swaps[0]->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get());
		}
	}

	std::vector<LAObject*> data_swap_curvefront;
	std::vector<LAObject*> data_swap_inSolving;
	size_t i = 0;
	for(i = 0; i < size_s; ++i)
	{
		const LAString& term_str = etrading::getMaturityAsTermString( i, data_swap );
		const LAPriceDataDayCount* dc = &dynamic_cast<const LAPriceDataDayCount&> ((data_swap[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
		const LAPriceDataCalendar* cal  = &dynamic_cast<const LAPriceDataCalendar&> ((data_swap[i]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
		const LAPriceDataSlidingRule* sld  = &dynamic_cast<const LAPriceDataSlidingRule&> ((data_swap[i]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
		
		LADate date = etrading::LADateHelpers::getDate(spotDate_, term_str, *sld, cal, true, &roll_conv);
		double term = dc_act365.getTerm(spotDate_, date);
		LADate date1Y = etrading::LADateHelpers::getDate(spotDate_, "1Y", *sld, cal, true, &roll_conv);
		double term1Y = dc_act365.getTerm(spotDate_, date1Y);

		if (size_mpc_swaps > 0)
		{
			if (date < firstCBSStartDate)
			{
				LAObject* shortEndSwap = data_swap[i];
			
				dh = &shortEndSwap->getData(IR_CALIBRATION_DATA_COMPOUNDINGMETHOD, NOCHECK);
				if (!dh->isDefined() || !dh->isNull())
				{
					// If short term swaps do not already have a user-specified compounding method,default it to Geometric
					LAString compooundMethod = dynamic_cast<const LADataString&> (dh->get());
					shortEndSwap->remove(IR_CALIBRATION_DATA_COMPOUNDINGMETHOD);
					shortEndSwap->add(IR_CALIBRATION_DATA_COMPOUNDINGMETHOD, new LADataString()).convertFromString(compooundMethod);	
				}

				// When Central Bank Swaps are present in the curve, short term swaps can NOT be built from
				// libor swap + basis. This is consistent with existing behaviour.
				data_swap_curvefront.push_back(shortEndSwap);
			}
			else
			{
				LAObject* tempSwap = data_swap[i];
				dh = &tempSwap->getData(IR_CALIBRATION_DATA_COMPOUNDINGMETHOD, NOCHECK);
				if (!dh->isDefined() || !dh->isNull())
				{
					// Remove compounding method in longer tenor swaps to avoid daily compounding in these swaps
					tempSwap->remove(IR_CALIBRATION_DATA_COMPOUNDINGMETHOD);
				}
				data_swap_inSolving.push_back(data_swap[i]);
			}
		}
		else if (size_mpc_swaps == 0)
		{
			// Short term swaps are shorter than 1Y
			if (term < term1Y)
			{
				// When there is no Central Bank Swaps in the curve, if a swap is built from libor swap + basis, it
				// can not be grouped as a short term swap
				const LADataHolder *dh = &data_swap[i]->getData(IR_CALIBRATION_DATA_LONGTERMCONVENTION);
				if (dh->isDefined() && !dh->isNull())
				{
					LAString longTermConv =	dynamic_cast<const LADataString&> (dh->get()).get();
					if (longTermConv == "LOBASIS")
					{
						shortterm_date = date;
						break;
					}
				}
				
				LAObject* shortEndSwap = data_swap[i];
			
				dh = &shortEndSwap->getData(IR_CALIBRATION_DATA_COMPOUNDINGMETHOD, NOCHECK);
				if (!dh->isDefined() || !dh->isNull())
				{					
					// If short term swaps do not already have a user-specified compounding method,default it to Geometric
					LAString compooundMethod = dynamic_cast<const LADataString&> (dh->get());
					shortEndSwap->remove(IR_CALIBRATION_DATA_COMPOUNDINGMETHOD);
					shortEndSwap->add(IR_CALIBRATION_DATA_COMPOUNDINGMETHOD, new LADataString()).convertFromString(compooundMethod);	
				}

				data_swap_inSolving.push_back(shortEndSwap);
			}
			else
			{
				shortterm_date = date1Y;
				
				LAObject* tempSwap = data_swap[i];
				dh = &tempSwap->getData(IR_CALIBRATION_DATA_COMPOUNDINGMETHOD, NOCHECK);
				if (!dh->isDefined() || !dh->isNull())
				{
					// Remove compounding method in longer tenor swaps to avoid daily compounding in these swaps
					tempSwap->remove(IR_CALIBRATION_DATA_COMPOUNDINGMETHOD);
				}
				data_swap_inSolving.push_back(tempSwap);
			}
		}
	}

	size_s = data_swap_inSolving.size();

	size_t shortTermSwapSize = data_swap_curvefront.size();

	//-------------------------------------------------------------------------------
	// Start calibrating curve with instruments
	
	// Calculate DF from OIS swap rates
	DoubleArray terms_grid; 
	DoubleArray terms_interval;
	DateVector dates;
	
	if (pInter_yg_->getType() == FN_LINEARINTERPOLATION)
	{
		pInter_StateVariable_.reset( new LALinearInterpolation(LINEAR_EXTRAPOLATION_TYPE) );
	}
	else
	{
		pInter_StateVariable_.reset( dynamic_cast<LAInterpolationBase *>(pInter_yg_->clone()) );
	}

	//TODO: Change it to be input form user	
	StateVariableEnum stateVariable = STATE_VARIABLE_ZERO_RATE;

	spotterm_ = dc_act365.getTerm(baseDate_, spotDate_);


	/*********************************************************************************************/
	// Start handling short end OIS swaps as well as central bank swaps

	bool includeShortEndSwapsInGlobalSolving = true;

	stateVariable_grid_.clear();
	stateVariable_grid_.push_back(0.0);

	stateVariable_rates_.clear();
	stateVariable_rates_.push_back(0.0);

	if (size_mpc_swaps != 0)
	{
		if (data_swap_curvefront.size() > 0)
		{
			LAObjectHolder objHolder = yieldDataRef_.get();
			LAString dfCurveName("ITSELF");

			// Calibrate curve using the short term swaps
			CurveCalibration::calcOISDFBySwapRates( stateVariable_rates_,			
								                    stateVariable_grid_,			
								                    terms_grid,		
								                    terms_interval,	
								                    dates,
								                    pInter_yg_.get(),			
								                    pInter_yg_.get(),	
								                    data_,			
								                    data_swap_curvefront,
								                    objHolder,				
								                    dc_act365,		
								                    baseDate_,		
								                    spotDate_,		
								                    shortterm_date,	
								                    &dfCurveName,
								                    size_mpc_swaps,			
								                    eom,			
								                    true,
								                    data_swap_curvefront.size());	
		
			includeShortEndSwapsInGlobalSolving = false;

			// Coming out of calcOISDFBySwapRates(), stateVariable_rates_ is zero rate;
			// Convert stateVariable_rates_ to logDF here
			for(size_t i = 0; i < stateVariable_grid_.size(); ++i)
			{
				stateVariable_rates_[i] = stateVariable_rates_[i] * stateVariable_grid_[i];
			}
		}

		//-------------------------------------------------------------------------------
		// Compute the flat OIS rates used in the Central Bank Swaps section 

		std::map<LADate, double> mpcSwapRates;
		for (unsigned int i = 0; i < size_mpc_swaps; ++i)
		{
			const LADate& startdate = dynamic_cast<const LADataDate&> ((data_mpc_swaps[i]->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get());
			const LADate& enddate = dynamic_cast<const LADataDate&> ((data_mpc_swaps[i]->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get());
			const LAPriceDataDayCount& dc = dynamic_cast<const LAPriceDataDayCount&> ((data_mpc_swaps[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
			const LAPriceDataCalendar& cal  = dynamic_cast<const LAPriceDataCalendar&> ((data_mpc_swaps[i]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
			const double rate = dynamic_cast<const LADataDouble&> ((data_mpc_swaps[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
			const LAString& shortTermConv = 
				dynamic_cast<const LADataString&> ((data_mpc_swaps[i]->getData(IR_CALIBRATION_DATA_SHORTTERMCONVENTION, ISNOTNULL)).get()).get();
			if (shortTermConv == "ARITHMETICAVERAGE")
			{
				//for first grid
				if (baseDate_ >= enddate)
				{
					continue;
				}
			
				if (startdate < baseDate_)
				{

					//if exists on rate, then today's ff rate should be the same as on rate.
					double todayffrate = 0.0;
					bool istodayrateexist = false;
					if (data_on_.size() > 0)
					{
						todayffrate = dynamic_cast<const LADataDouble&> ((data_on_[0]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
						istodayrateexist = true;
					}
					double implyrate = CurveCalibration::getAverageRateFromHistRates(startdate, enddate, dc, cal, rate, data_mpc_swaps[i], baseDate_, istodayrateexist, todayffrate);
					mpcSwapRates.insert( std::make_pair(baseDate_, implyrate) );
				}
				else
				{
					mpcSwapRates.insert( std::make_pair(startdate, rate) );
				}
			}
			else
			{
				const double onforward = CurveCalibration::solveOISRate(startdate, enddate, dc, cal, rate);
				mpcSwapRates.insert( std::make_pair(startdate, onforward) );
			}

			DateVector boj_calcdates(2, startdate);
			boj_calcdates[1] = enddate;
			if (!data_mpc_swaps[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO,NOCHECK).isDefined())
			{
				data_mpc_swaps[i]->add(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, new LADataDates(boj_calcdates));
			}
			else
			{
				dynamic_cast<LADataDates&>(data_mpc_swaps[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO).get()).set(boj_calcdates);
			}

		}	

		//-------------------------------------------------------------------------------
		// Bootstrap the central bank swap section (mid curve)

		LADate tmp_date;
		firstCBSStartDate = dynamic_cast<const LADataDate&> ((data_mpc_swaps[0]->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get());
		double firstCBSTerm = dc_act365.getTerm(spotDate_, firstCBSStartDate);
		std::map<LADate, double>::const_iterator it = mpcSwapRates.begin();

		if (data_swap_curvefront.size() != 0)
		{
			// 1. 
			// When short term swaps are provided, bootstrap the central bank swaps section from the end of short terms swaps

			// End date of the last short term swap
			const LAString& term_str = dynamic_cast<const LADataString&> ((data_swap_curvefront.back()->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get());
			LADate lastSwapEndDate = etrading::LADateHelpers::getDate(spotDate_, term_str, sld, &cal, true, &roll_conv);

			double lastYield = stateVariable_rates_.back();
			double lastShortSwapTerm = stateVariable_grid_.back();

			if (lastShortSwapTerm <= firstCBSTerm)
			{
				// Linearly interpolate the gap between the end of the last short term swap and the start of the first central bank swaps

				double lastSwapRate		= dynamic_cast<const LADataDouble&> ((data_swap_curvefront.back()->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
				double firstCBSRate	= dynamic_cast<const LADataDouble&> ((data_mpc_swaps[0]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
				
				double gapTerm = firstCBSStartDate.intervalDays(lastSwapEndDate);
				double rateIncrement = (firstCBSRate - lastSwapRate) / gapTerm;

				tmp_date = lastSwapEndDate;

				double df = LAMath::exp(-lastYield);
				while (tmp_date < shortterm_date)	// shortterm_date is end of the Central Bank Swaps section on the curve and can be a user given date date that can come before the last Central Bank Swaps end date
				{
					std::map<LADate, double>::const_iterator it_n = it;
					if (it != (--mpcSwapRates.end()) && (++it_n)->first <= tmp_date)
					{
						++it;
					}
					
					LADate n_date = cal.getBusinessDay(tmp_date, 1);
					const double term = dc_act365.getTerm(spotDate_, n_date);

					double instanteneousFwdRate(0.0);
					if (n_date < firstCBSStartDate)
					{
						instanteneousFwdRate = (n_date.intervalDays(lastSwapEndDate)) * rateIncrement + lastSwapRate;
					}
					else
					{
						instanteneousFwdRate = it->second;
					}

					df *= conv.getDF(instanteneousFwdRate, tmp_date, n_date);
					
					stateVariable_grid_.push_back(term);
					stateVariable_rates_.push_back(-LAMath::log(df) );

					tmp_date = n_date;
				}

			}
			else if (lastShortSwapTerm > firstCBSTerm)
			{
				// Overlaps between the last short swap and the first n Central Bank Swaps. Default behaviour is that the Central Bank Swaps overrule
						
				if (!shortTermSwapOverrules)
				{
					// The first Central Bank Swaps contract rules over the last short term swaps

					DoubleArray tempYields;
					DoubleArray tempGrid;

					if (stateVariable_rates_.size() != stateVariable_grid_.size())
					{
						throw LACoreInvalidData("#Error Invalid Curve Data: Inconsistent number of zero rates and date terms in OIS curve", __FILE__, __LINE__);
					}

					// Remove all the zero rates after the first Central Bank Swaps start date
					double yield = 0.0;
					double term = 0.0;
					for (size_t i = 0; i < stateVariable_rates_.size(); ++i)
					{
						yield = stateVariable_rates_[i];
						term  = stateVariable_grid_[i];
						if (term < firstCBSTerm)
						{
							tempYields[i] = stateVariable_rates_[i];
							tempGrid[i]   = stateVariable_grid_[i];
						}
					}

					stateVariable_rates_.clear();
					stateVariable_grid_.clear();

					stateVariable_rates_ = tempYields;
					stateVariable_grid_ = tempGrid;

					tmp_date = firstCBSStartDate;
					double df = LAMath::exp(-yield);
					while (tmp_date < shortterm_date)	// shortterm_date is end of the Central Bank Swaps section on the curve and can be a user given date date that can come before the last Central Bank Swaps end date
					{
						std::map<LADate, double>::const_iterator it_n = it;
						if (it != (--mpcSwapRates.end()) && (++it_n)->first <= tmp_date)
						{
							++it;
						}
						LADate n_date = cal.getBusinessDay(tmp_date, 1);
						df *= conv.getDF(it->second, tmp_date, n_date);
						const double term = dc_act365.getTerm(spotDate_, n_date);
			
						stateVariable_grid_.push_back(term);
						stateVariable_rates_.push_back(-LAMath::log(df) );

						tmp_date = n_date;
					}

				}
				else
				{				
					// The last short term swap rules over the first Central Bank Swaps contracts						

					// Central Bank Swaps section starts as soon as the last short term swap is completed
					tmp_date = lastSwapEndDate;
				
					double df = LAMath::exp(-lastYield);
					while (tmp_date < shortterm_date)	// shortterm_date is end of the Central Bank Swaps section on the curve and can be a user given date date that can come before the last Central Bank Swaps end date
					{
						std::map<LADate, double>::const_iterator it_n = it;
						if (it != (--mpcSwapRates.end()) && (++it_n)->first <= tmp_date)
						{
							++it;
						}
						LADate n_date = cal.getBusinessDay(tmp_date, 1);
						df *= conv.getDF(it->second, tmp_date, n_date);
						const double term = dc_act365.getTerm(spotDate_, n_date);
			
						if (term > lastShortSwapTerm)
						{
							// Add DFs derived from Central Bank Swaps only after the short term swaps
							stateVariable_grid_.push_back(term);
							stateVariable_rates_.push_back(-LAMath::log(df) );
						}

						tmp_date = n_date;
					}
				}
			}		
		}
		else
		{
			// 2.
			// When no short term swaps are present, simply bootstrap the Central Bank Swaps section from spot date

			tmp_date = spotDate_;
			double df = 1.0;
			while (tmp_date < shortterm_date)	// shortterm_date is end of the Central Bank Swaps section on the curve and can be a user given date date that can come before the last Central Bank Swaps end date
			{
				std::map<LADate, double>::const_iterator it_n = it;
				if (it != (--mpcSwapRates.end()) && (++it_n)->first <= tmp_date)
				{
					++it;
				}	
				LADate n_date = cal.getBusinessDay(tmp_date, 1);
				df *= conv.getDF(it->second, tmp_date, n_date);
				const double term = dc_act365.getTerm(spotDate_, n_date);
			
				stateVariable_grid_.push_back(term);
				stateVariable_rates_.push_back(-LAMath::log(df));

				tmp_date = n_date;
			}
		}

		if (data_swap_inSolving.size() == 0)
		{
			throw LACoreInvalidData("#Error: No swaps are given to define the long end of the OIS curve.", __FILE__, __LINE__);
		}
	}

	/*********************************************************************************************/
	// Go through all swaps and determine a range of information in preparation of calibration by solving

	// Empty all variables before gathering all relevant information
	LAString longTermConv;
	LAString l_longTermConv;
	LAString longTermGen;	
	
	unsigned int calced_size = 0;
	unsigned int calced_size_s = 0;

	DoubleArray terms_grid_s; 
	DoubleArray terms_interval_s;
	DateVector dates_s;

	LADate lastSwapEndDate = spotDate_;
 
	LAString oldFreq("");
	LAString oldFreq_s("");
	oldFreq.toUpper();
	
	std::vector<double> swapEndDateInTerms;

	size_calcs_.clear();
	size_calcs_s_.clear();
	calced_sizes_.clear();
	calced_sizes_s_.clear();
	terms_grids_.clear();
	terms_grids_s_.clear();
	terms_intervals_.clear();
	terms_intervals_s_.clear();
	marketRates_.clear();
	marketRates_s_.clear();
	fixingStartDates_ois_.clear();
	fixingEndDates_ois_.clear();
	fixingTaus_ois_.clear();
	fixingStartDates_libor_.clear();
	fixingEndDates_libor_.clear();
	fixingTaus_libor_.clear();
	isIRSParRateGiven_.clear();
	term_strs_.clear();
	swapCompoundingMethodEnums_.clear();
	longTermConvEnums_.clear();
	longTermGens_.clear();
	bLiborSwapCalcReset_.clear();
	bOISSwapCalcReset_.clear();
	bFullSigmaReset_.clear();
	cals_.clear();
	dateCounts_.clear();

	dailyTerms_Start_.clear();
	dailyTerms_End_.clear();
		
	// Loop through all OIS swaps and determine key trade info such as schedules as well as key control parameters
	for (unsigned int i = 0; i < size_s; ++i)
	{			
		LAString term_str = dynamic_cast<const LADataString&> ((data_swap_inSolving[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get());			
		term_str.toUpper();
		
		dh = &data_swap_inSolving[i]->getData(IR_CALIBRATION_DATA_LONGTERMCONVENTION);
		if (dh->isDefined() && !dh->isNull())
		{
			longTermConv =	dynamic_cast<const LADataString&> (dh->get()).get();
			longTermGen = dynamic_cast<const LADataString&> ((data_swap_inSolving[i]->getData(IR_CALIBRATION_DATA_LONGTERMGENMETHOD, ISNOTNULL)).get());
		}					

		double marketrate = 0.0;

		const LAPriceDataDayCount* dc;
		const LAPriceDataCalendar* cal;
		const LAPriceDataSlidingRule* sld;
		LAString freq;

		//---------------------------------------------------------
		// Produce full OIS swap schedules
		
		if (longTermConv != "LOBASIS")
		{
			// load ois market data
			marketrate = dynamic_cast<const LADataDouble&> ((data_swap_inSolving[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
			dc = &dynamic_cast<const LAPriceDataDayCount&> ((data_swap_inSolving[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
			cal  = &dynamic_cast<const LAPriceDataCalendar&> ((data_swap_inSolving[i]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
			sld  = &dynamic_cast<const LAPriceDataSlidingRule&> ((data_swap_inSolving[i]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
			freq = dynamic_cast<const LADataString&> ((data_swap_inSolving[i]->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL)).get()).get();
		}		
		else
		{
			// load lobasis market data
			marketrate = dynamic_cast<const LADataDouble&> (data_swap_inSolving[i]->getData(IR_CALIBRATION_DATA_RATE_LOBASIS, ISNOTNULL).get()).get();
			dc = &dynamic_cast<const LAPriceDataDayCount&> ((data_swap_inSolving[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT_LOBASIS, ISNOTNULL)).get());
			cal  = &dynamic_cast<const LAPriceDataCalendar&> ((data_swap_inSolving[i]->getData(IR_CALIBRATION_DATA_CALENDAR_LOBASIS, ISNOTNULL)).get());
			sld  = &dynamic_cast<const LAPriceDataSlidingRule&> ((data_swap_inSolving[i]->getData(IR_CALIBRATION_DATA_SLIDINGRULE_LOBASIS, ISNOTNULL)).get());
			freq = dynamic_cast<const LADataString&> ((data_swap_inSolving[i]->getData(IR_CALIBRATION_DATA_FREQUENCY_LOBASIS, ISNOTNULL)).get()).get();
		}			

		LAString roll_conv("");
		freq.toUpper();
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

		LADate date_unadjusted = etrading::LADateHelpers::getDate(spotDate_, term_str, LAPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr);
		LADate swapEndDate = etrading::LADateHelpers::getDate(spotDate_, term_str, *sld, cal, true, &roll_conv);
		if (
			( ((size_mpc_swaps || shortTermSwapSize) && swapEndDate < shortterm_date) 
			|| (!size_mpc_swaps && i == 0) 
			|| (lastSwapEndDate >= swapEndDate)
			) && !includeShortEndSwapsInGlobalSolving)
		{
			continue; 
		}

		double term = dc_act365.getTerm(spotDate_, swapEndDate);					

		terms_grid.clear();
		terms_interval.clear();
		dates.clear();
		etrading::updateAccrualPeriodsAndPaymentDates(spotDate_, date_unadjusted, freq, *cal, *sld, *dc, dates, terms_grid, terms_interval, eom);

		unsigned int size_cashlet = dates.size();
		if (!size_cashlet)
		{
            throw LACoreInvalidData("#Error: Invalid calibration instruments specified with no cashflows", __FILE__, __LINE__);
		}
					

		//---------------------------------------------------------
		// Arrange to re-use calculation results from earlier swaps
						
		bOISSwapCalcReset_.push_back(false);
		if (longTermConv != l_longTermConv)
		{
			lastSwapEndDate = spotDate_;
			calced_size = 0;
			calced_size_s = 0;

			bOISSwapCalcReset_.back() = true;
		}
		l_longTermConv = longTermConv;

		bFullSigmaReset_.push_back(false);
		if (size_cashlet == calced_size				// 1. Swap with the same cashlets as the last swap is encountered
			|| (oldFreq != freq && oldFreq.size() != 0)		// 2. Swap frequency has changed
			|| ((calced_size > 0) && lastSwapEndDate != dates[calced_size - 1] && size_cashlet > calced_size)	// 3. Current swap has an odd date schedule
			)
		{
			// When odd swap date schedule is encountered, reset all sigmas and will build swap date schedule from the beginning
			// rather than by extending from the previous swap's schedule
				
			if (calced_size == 0 || oldFreq != freq)
			{
				lastSwapEndDate = spotDate_;
			}
			else
			{
				lastSwapEndDate = dates[calced_size - 1];
			}

			calced_size = 0;

			bFullSigmaReset_.back() = true;
		}
		oldFreq = freq;
						
		unsigned int size_calc = size_cashlet - calced_size;
		if (!size_calc)
		{
			continue;
		}
						
		//---------------------------------------------------------
		// load libor swap data
			
		double marketrate_s = 0.0;
		unsigned int size_cashlet_s = 0;
		unsigned int size_calc_s = 0;
		bLiborSwapCalcReset_.push_back(false);
		bool swapParRateGiven = true;

		if (longTermConv == "LOBASIS")
		{
			const LADataHolder *dh = &(data_swap_inSolving[i]->getData(IR_CALIBRATION_DATA_RATE_SWAP, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				marketrate_s = dynamic_cast<const LADataDouble &>(dh->get()).get();
				swapParRateGiven = true;				
			}
			else
			{
				swapParRateGiven = false;
			}
						
			const LAPriceDataDayCount& dc_s = dynamic_cast<const LAPriceDataDayCount&> ((data_swap_inSolving[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT_SWAP, ISNOTNULL)).get());
			const LAPriceDataCalendar& cal_s = dynamic_cast<const LAPriceDataCalendar&> ((data_swap_inSolving[i]->getData(IR_CALIBRATION_DATA_CALENDAR_SWAP, ISNOTNULL)).get());
			const LAPriceDataSlidingRule& sld_s = dynamic_cast<const LAPriceDataSlidingRule&> ((data_swap_inSolving[i]->getData(IR_CALIBRATION_DATA_SLIDINGRULE_SWAP, ISNOTNULL)).get());
			LAString freq_s = dynamic_cast<const LADataString&> ((data_swap_inSolving[i]->getData(IR_CALIBRATION_DATA_FREQUENCY_SWAP, ISNOTNULL)).get()).get();
			freq_s.toUpper();

			if (oldFreq_s.size() != 0 && oldFreq_s != freq_s)
			{
				// Reset when calibration swap frequency changes
				bLiborSwapCalcReset_.back() = true;
				calced_size_s = 0;					
			}
			oldFreq_s = freq_s;

			LAString roll_conv_s("");
			if (freq == LUNAR) roll_conv_s = ROLLCONV_LUNAR;
			else if (eom) roll_conv_s = ROLLCONV_EOM;
			else roll_conv_s = ROLLCONV_NORMAL;

			// scheduling libor swap
			const LADate date_s_unadjusted = etrading::LADateHelpers::getDate(spotDate_, term_str, LAPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr);
			terms_grid_s.clear();
			terms_interval_s.clear();
			dates_s.clear();
			etrading::updateAccrualPeriodsAndPaymentDates(spotDate_, date_s_unadjusted, freq_s, cal_s, sld_s, dc_s, dates_s, terms_grid_s, terms_interval_s, eom);
			size_cashlet_s = dates_s.size();
			if (!size_cashlet_s)
			{
                throw LACoreInvalidData("#Error: Invalid swap calibration instrument specified with no cashflows.", __FILE__, __LINE__);
			}

			size_calc_s = size_cashlet_s - calced_size_s;
			
			// Compute fixing schedule dates
			DateVector tempFixingStartDates;
			tempFixingStartDates.push_back(spotDate_);
			for (size_t j = 0; j < dates.size() - 1; ++j)
			{
				tempFixingStartDates.push_back(dates[j]);
			}

			LAString refRateTerm;
			if (freq == SEMI_ANNUAL)
			{
				refRateTerm = "6M";
			}
			else if (freq == QUARTERLY)
			{
				refRateTerm = "3M";
			}
			else if (freq == MONTHLY)
			{
				refRateTerm = "1M";
			}
			else if (freq == ANNUAL)
			{
				refRateTerm = "1Y";
			}
			else if (freq == BUSINESS_DAYS || freq == DAILY)
			{
				refRateTerm = "1D";
			}
			else
			{
				throw LACoreInvalidData("We support only 6M or 3M in swap floating leg!", __FILE__, __LINE__);
			}

			// fixing end dates are always a fixed term after the corresponding fixing start dates
			DateVector tempFixingEndDates;
			if (freq == ANNUAL && (term_str == "18M" || term_str == "1Y6M") && dates.size() == 2)
			{
				// Special case. Swap with 6M front stub. 
				tempFixingEndDates.push_back(dates[0]);
				tempFixingEndDates.push_back(dates[1]);
			}
			else
			{
				tempFixingEndDates = etrading::LADateScheduleHelpers::getMultiDate(tempFixingStartDates, refRateTerm, sld->convertToString(), cal->convertToString(), nullptr); // rollconvention* = nullptr
			}

			DoubleVector tempFixingTaus;
			for (size_t j = 0; j < tempFixingEndDates.size(); ++j)
			{
				// Never allow any fixing dates to be beyond the end of the current swap
				// Otherwise the last forward rates may be susceptible to extrapolation which is unwanted
				if (tempFixingEndDates[j] > dates.back())
				{
					tempFixingEndDates[j] = dates.back();
				}

				double fixingTau = dc->getTerm(tempFixingStartDates[j], tempFixingEndDates[j]);
				tempFixingTaus.push_back(fixingTau);
			}

			fixingStartDates_ois_.push_back(tempFixingStartDates);
			fixingEndDates_ois_.push_back(tempFixingEndDates);
			fixingTaus_ois_.push_back(tempFixingTaus);
		}
		else
		{
			//---------------------------------------------------------
			// Compute fixing schedule dates of OIS swaps
			DateVector tempFixingStartDates, tempFixingEndDates;
			DoubleArray tempFixingTaus;

			tempFixingStartDates.push_back(spotDate_);
			for (size_t j = 0; j < dates.size() - 1; ++j)
			{
				tempFixingStartDates.push_back(dates[j]);
				tempFixingEndDates.push_back(dates[j]);
				tempFixingTaus.push_back(terms_interval[j]);
			}
			tempFixingEndDates.push_back(dates.back());
			tempFixingTaus.push_back(terms_interval.back());

			fixingStartDates_ois_.push_back(tempFixingStartDates);
			fixingEndDates_ois_.push_back(tempFixingEndDates);
			fixingTaus_ois_.push_back(tempFixingTaus);
		}

		isIRSParRateGiven_.push_back(swapParRateGiven);

		if (swapParRateGiven)
		{
			// Insert empty elements to keep the size of these three variables in line with the rest
			fixingStartDates_libor_.push_back(DateVector());
			fixingEndDates_libor_.push_back(DateVector());
			fixingTaus_libor_.push_back(DoubleVector());
		}
		else
		{
			fixingStartDates_libor_ = fixingStartDates_ois_;
			fixingEndDates_libor_   = fixingEndDates_ois_;
			fixingTaus_libor_ 	    = fixingTaus_ois_;
		}
			
		lastSwapEndDate = dates[size_cashlet - 1];

		// Compounding methods
		LAString swapCompoundingMethod(""); 
		dh = &data_swap_inSolving[0]->getData(IR_CALIBRATION_DATA_COMPOUNDINGMETHOD, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			swapCompoundingMethod =	dynamic_cast<const LADataString&> (dh->get()).get();
		}
		swapCompoundingMethodEnums_.push_back( etrading::toOISCompoundingEnum( swapCompoundingMethod.getCString() ) );
			
		// Calculate the daily date fractions used in daily compounding 
		DoubleMatrix startTermsMat;
		DoubleMatrix endTermsMat;
		startTermsMat.clear();
		endTermsMat.clear();
		if ( 
			( (longTermConv == "" || longTermConv != LOBASIS) && swapCompoundingMethod.size() != 0)	// Short tenor OIS outright swaps
		||  ( longTermConv == LOBASIS && longTermGen == "DAILYAVERAGING") )							// Long tenor OIS outright/basis swaps
		{			
			for (unsigned int j = 0; j < size_calc; ++j)
			{
				DoubleVector startTermsVec;
				DoubleVector endTermsVec;

				unsigned int pos = j + calced_size;
				LADate startDate = fixingStartDates_ois_[i][pos];
				LADate endDate = fixingEndDates_ois_[i][pos];

				LADate tmp_date = startDate;
				while (tmp_date < endDate)
				{
					const LADate nextdate = cal->getBusinessDay(tmp_date, 1);
					startTermsVec.push_back(dc_act365.getTerm(spotDate_, tmp_date));
					endTermsVec.push_back(dc_act365.getTerm(spotDate_, nextdate));
					tmp_date = nextdate;
				}

				startTermsMat.push_back(startTermsVec);
				endTermsMat.push_back(endTermsVec);
			}
		}

		dailyTerms_Start_.push_back(startTermsMat);
		dailyTerms_End_.push_back(endTermsMat);	

		//---------------------------------------------------------
		// Store all sorts of information in vector form
		term_strs_.push_back(term_str);
		
		swapEndDateInTerms.push_back(term);
		cals_.push_back(cal);
		dateCounts_.push_back(dc);

		marketRates_.push_back(marketrate);
		marketRates_s_.push_back(marketrate_s);
			
		size_calcs_.push_back(size_calc);
		size_calcs_s_.push_back(size_calc_s);

		calced_sizes_.push_back(calced_size);
		calced_sizes_s_.push_back(calced_size_s);

		terms_grids_.push_back(terms_grid);
		terms_grids_s_.push_back(terms_grid_s);

		terms_intervals_.push_back(terms_interval);
		terms_intervals_s_.push_back(terms_interval_s);
			
		longTermConvEnums_.push_back( etrading::toOISLongTermInstrumentsEnum( longTermConv.getCString() ) );
		longTermGens_.push_back(longTermGen);

		calced_size = size_cashlet;
		if (longTermConv == "LOBASIS")
		{
			calced_size_s = size_cashlet_s;
		}
	}

	// There can be parts of the curve that have already been calibrated by the time swaps are used. 
	// 'preSwapSize' measures the number of key points that have already
	// been calibrated prior to swaps are used.
		
	// Initialise state variable interpolator
	swapCount_   = term_strs_.size();
	preSwapSize_ = stateVariable_rates_.size();

	LAString suffix_mkt = "_" + curveName_;
	suffix_mkt.toUpper();
	LAObjectHolder objHolder = yieldDataRef_.get();
	etrading::initialiseStateVariablesForSolving(stateVariable_rates_,
												stateVariable_grid_,
												objHolder,
												stateVariable,
												marketRates_,
												swapEndDateInTerms,
												fastRebuild_,
												suffix_mkt,
												swapCount_);
	
	pInter_StateVariable_->set(stateVariable_grid_, stateVariable_rates_);
}


/*! @brief Price all instruments that are included in solving
	
	@param[out]  allPVs			PVs of all the trades
*/
void OISComponentCurve::priceCalibrationInstruments(DoubleVector& allPVs)
{
	double sumCF = 0.0;
	double sumT = 0.0;
	double sumT_s = 0.0;

	double _sumCF = 0.0;
	double _sumT = 0.0;
	double _sumT_s = 0.0;

	LAInterpolationBase* libor_inter = nullptr;
	libor_inter = pInter_Libor_.get();

	allPVs.clear();
	for (size_t i = 0; i < term_strs_.size(); ++i)
	{
		// Reset swap calculation, i.e. do not reuse calculations from earlier swaps
		if (bOISSwapCalcReset_[i])
		{
			// if market is changed, reset sums.				
			_sumCF = 0.0;
			_sumT = 0.0;
			_sumT_s = 0.0;
		}

		if (bFullSigmaReset_[i])
		{
			// When odd swap date schedule is encountered, reset all sigmas and will build swap date schedule from the beginning
			// rather than by extending from the previous swap's schedule
			_sumCF = 0.0;
			_sumT = 0.0;								
		}

		// Reset when calibration swap frequency changes
		if (bLiborSwapCalcReset_[i])
		{
			_sumT_s = 0.0;
		}
			
		const etrading::OISLongTermInstrumentsEnum longTermConv = longTermConvEnums_[i];
		const LAString longTermGen  = longTermGens_[i];

        bool swapParRateGiven = isIRSParRateGiven_[i];
								
		// ---------------------------------------------
		// Compute PV of single OIS swap (either outright or Libor-OIS basis swap)		
		double pv = priceSingleOISSwapPV(marketRates_[i],
										 marketRates_s_[i],
										 sumCF,
										 sumT,
										 sumT_s,
										 _sumCF,
										 _sumT,
										 _sumT_s,
										 size_calcs_[i],
										 calced_sizes_[i],			
										 size_calcs_s_[i],
										 calced_sizes_s_[i],
										 terms_grids_[i],
										 terms_intervals_[i],
										 terms_grids_s_[i],
										 terms_intervals_s_[i],
										 fixingTaus_ois_[i],
										 fixingStartDates_ois_[i],
										 fixingEndDates_ois_[i],
										 swapCompoundingMethodEnums_[i],
										 longTermConv,
										 longTermGens_[i],
										 dateCounts_[i], 
										 cals_[i],
										 dailyTerms_Start_[i],
										 dailyTerms_End_[i],
										 swapParRateGiven,
										 !swapParRateGiven ? fixingStartDates_libor_[i] : DateVector(),
										 !swapParRateGiven ? fixingEndDates_libor_[i] : DateVector(),
										 !swapParRateGiven ? fixingTaus_libor_[i] : DoubleVector(),
										 !swapParRateGiven ? libor_inter : nullptr);

		allPVs.push_back(pv);

		// ---------------------------------------------
		// Record results of the earlier swap
		_sumCF = sumCF;
		_sumT = sumT;

		if ( longTermConv == etrading::LIBOROIS_OIS_LONGTERM_INSTRUMENTS )
		{
			_sumT_s = sumT_s;
		}
	}
}


/*! @brief Price the PV of a single OIS swap (either outright or Libor-OIS basis swap)

	@param[in]  marketRate			    Market OIS par rate 
	@param[in]  marketRate_s		    Market Libor swap par rate 
	@param[inout]  sumCF			    Sum of cash flows of the current OIS swap
	@param[inout]  sumT				    Annuity of the current OIS swap
	@param[inout]  sumT_s			    Annuity of the Libor leg of the current LIBOR-OIS swap
	@param[in]  _sumCF				    Sum of cash flows of the previous OIS swap
	@param[in]  _sumT				    Annuity of the previous OIS swap
	@param[in]  _sumCF_s			    Annuity of the Libor leg of the previous LIBOR-OIS swap
	@param[in]  size_calc			    Number of OIS swap cash flows that need to be calculated for the current swap
	@param[in]  calced_size			    Number of OIS swap cash flows that can be re-used 
	@param[in]  size_calc_s			    Number of Libor swap cash flows that need to be calculated for the current swap
	@param[in]  calced_size_s		    Number of Libor swap cash flows that can be re-used 
	@param[in]  terms_grids			    Terms from spot date to all OIS swap dates
	@param[in]  terms_intervals		    Accrual terms between all OIS swap accrual dates
	@param[in]  terms_grids_s		    Terms from spot date to all Libor swap dates	
	@param[in]  terms_intervals_s	    Accrual terms between all Libor swap accrual dates
	@param[in]  fixingTaus_ois			Fixing terms between all OIS swap fixing dates
	@param[in]  fixingStartDates_ois	OIS swap fixing start dates
	@param[in]  fixingEndDates_ois		OIS swap fixing end dates
	@param[in]  swapCompoundingMethod	Compounding method for OIS swap
	@param[in]  longTermConv		    Indication of calibration swap type for the long end of OIS curve
	@param[in]  longTermGen			    Generation method for long end calibration swaps
	@param[in]  dateCount			    OIS swap date count
	@param[in]  cal					    OIS swap calendar
	@param[inout]  startterms		    Terms from spot to start date of each OIS cash flow
	@param[inout]  endterms			    Terms from spot to end date of each OIS cash flow
	@param[in]  isIRSParRateKnown		Are vanilla interest rate swap's par rates provided		
	@param[in]  fixingStartDates_libor	Libor fixing start dates from swap curve
	@param[in]  fixingEndDates_libor	Libor fixing end dates from swap curve
	@param[in]  fixingTaus_libor		Libor fixing terms 
	@param[in]  libor_inter				Interpolation object for logDF from the swap curve

	@return PV of the OIS swap (either an outright or a Libor-OIS basis swap)
*/
double OISComponentCurve::priceSingleOISSwapPV(double marketRate,
						                       double marketRate_s,
						                       double& sumCF,
						                       double& sumT,
						                       double& sumT_s,
						                       double _sumCF,
						                       double _sumT,
						                       double _sumT_s,
						                       size_t size_calc,
						                       size_t calced_size,			
						                       size_t size_calc_s,
						                       size_t calced_size_s,
						                       const DoubleVector& terms_grids,
						                       const DoubleVector& terms_intervals,
						                       const DoubleVector& terms_grids_s,
						                       const DoubleVector& terms_intervals_s,
						                       const DoubleVector& fixingTaus_ois,
						                       const DateVector& fixingStartDates_ois,
						                       const DateVector& fixingEndDates_ois,
						                       const etrading::OISCompoundingEnum& swapCompoundingMethodEnum,
						                       const etrading::OISLongTermInstrumentsEnum& longTermConvEnum,
						                       const LAString& longTermGen,
						                       const LAPriceDataDayCount* dateCount, 
						                       const LAPriceDataCalendar* cal,
						                       DoubleMatrix& startterms,
						                       DoubleMatrix& endterms,
						                       bool isIRSParRateKnown,
						                       const DateVector& fixingStartDates_libor,
						                       const DateVector& fixingEndDates_libor,
						                       const DoubleVector& fixingTaus_libor,
						                       LAInterpolationBase *libor_inter) const
{
	sumCF = 0.0;
	sumT = 0.0;
	sumT_s = 0.0;
		
	double sumLiborFlows = 0.0;

	LAPriceDataDayCount dc_act365(ACT_365);

	for (unsigned int j = 0; j < size_calc; ++j)
	{
		unsigned int pos = j + calced_size;

		// Discount to spot date
		double df = LAMath::exp(-pInter_StateVariable_->value(terms_grids[pos]));			
			
		sumT += terms_intervals[pos] * df;						

		// calc settle rate & CF
		LADate startdate = fixingStartDates_ois[pos];
		LADate enddate = fixingEndDates_ois[pos];
		double fixingTau = fixingTaus_ois[pos];		

		DoubleArray startT(0), endT(0);
		if (startterms.size() != 0)
		{
			startT = startterms[j];
		}
		
		if (endterms.size() != 0)
		{
			endT = endterms[j];
		}

		double effectiveRate = CurveCalibration::calcEffectiveOISRate(startT,
																	  endT,
																	  swapCompoundingMethodEnum,
																	  longTermConvEnum,
																	  longTermGen,
																	  spotDate_,
																	  startdate,
																	  enddate,
																	  dateCount,
																	  cal,
																	  pInter_StateVariable_.get(),
																	  fixingTau,
																	  true);

		sumCF += effectiveRate * terms_intervals[pos] * df;
	}

	if ( longTermConvEnum == etrading::LIBOROIS_OIS_LONGTERM_INSTRUMENTS )
	{
		// Check the avaiability of libor rate interpolator when required
		if (libor_inter == nullptr && !isIRSParRateKnown)
		{
			throw LACoreInvalidData("Error: Unable to calibrate Libor-OIS swap due to lack of Libor IRS par rate market data.", __FILE__,__LINE__);
		}

		// Calculate annuity 
		for (unsigned int j = 0; j < size_calc_s; ++j)
		{
			unsigned int pos_s = j + calced_size_s;

			// Discount to spot date
			double df = LAMath::exp(-pInter_StateVariable_->value(terms_grids_s[pos_s]) );		

			sumT_s += terms_intervals_s[pos_s] * df;
		}

		// When swap rate is not provided, need to calculate sum of Libor * annuity from the beginning of swap
		if (!isIRSParRateKnown)
		{
			for (unsigned int k = 0; k < size_calc + calced_size; ++k)		// Must not use the fixed leg convention of the Libor swap, i.e. any convention variables ending with '_s'
			{
				// Discount to spot date
				double df = LAMath::exp(-pInter_StateVariable_->value(terms_grids[k]));			

				const double startterm = dc_act365.getTerm(spotDate_, fixingStartDates_libor[k]);
				const double endterm = dc_act365.getTerm(spotDate_, fixingEndDates_libor[k]);
				const double fixingTerm = fixingTaus_libor[k];
				double libor = (LAMath::exp(libor_inter->value(endterm) - libor_inter->value(startterm) ) - 1.0) / fixingTerm;

				sumLiborFlows += libor * terms_intervals[k] * df;
			}
		}

	}

	sumCF += _sumCF;
	sumT += _sumT;
	sumT_s += _sumT_s;
		
	double pv = 0.0;
	if (longTermConvEnum == etrading::LIBOROIS_OIS_LONGTERM_INSTRUMENTS )
	{
		if (isIRSParRateKnown)
		{
			pv = marketRate_s * sumT_s - sumCF - marketRate * sumT;
		}
		else
		{				
			pv = sumLiborFlows - sumCF - marketRate * sumT;
		}
	}
	else
	{
		pv = sumCF - marketRate * sumT;
	}
		
	return pv;
}


/*! @brief Set the internal Libor rate interpolator

	@param[in]  pInter_Libor		Input Libor interpolator 
*/
void OISComponentCurve::setLiborInterp(const std::shared_ptr<LAInterpolationBase>& pInter_Libor) 
{
	// Don't use reset or clone method here as that is not the intended behavior
	pInter_Libor_ = pInter_Libor;
}


/*! 
	@brief Post processing results once instruments have been consumed in calibration steps
*/
void OISComponentCurve::postProcessing(LAObject& yieldCurveProEntity)
{
	LAPriceDataDayCount dc_act365(ACT_365);

	stateVariable_grid_.insert(stateVariable_grid_.begin(), terms_grids_.back().begin(), terms_grids_.back().end());
	std::sort(stateVariable_grid_.begin(), stateVariable_grid_.end());
	stateVariable_grid_.erase(std::unique(stateVariable_grid_.begin(), stateVariable_grid_.end()), stateVariable_grid_.end());	

	DoubleArray oisCurve_terms;
	DoubleArray oisCurve_dfs;

	//-------------------------------------------------------------------------------
	// Append ON and TN rates to the front if provided. 
	// Convert zero rates to dfs
	
	if (spotDate_ > baseDate_)
	{
		size_t size_on = data_on_.size();
		size_t size_tn = data_tn_.size();

		const LAPriceDataDayCount& dc = dynamic_cast<const LAPriceDataDayCount&> ((data_[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
		const LAPriceDataCalendar& cal  = dynamic_cast<const LAPriceDataCalendar&> ((data_[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
		RateConvention rc = LAMathYieldCurve::setRC(SIMPLE);
		LAPriceDataConvention conv(dc.getDayCount(), rc);

		if (size_on > 0 || size_tn > 0)
		{
			LADate date;
			double rate_on = 0.0;			
			if (size_on > 0)
			{
				// from base date to tomorrow
				rate_on = dynamic_cast<const LADataDouble&> ((data_on_[0]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
				const LAPriceDataDayCount& dc_on = dynamic_cast<const LAPriceDataDayCount&> ((data_on_[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
				RateConvention rc_on = LAMathYieldCurve::setRC(SIMPLE);
				LAPriceDataConvention conv_on(dc_on.getDayCount(), rc_on);
			
				date = cal.getBusinessDay(baseDate_, 1);
				oisCurve_dfs.push_back(conv.getDF(rate_on, baseDate_, date));
				
				double term = dc_act365.getTerm(baseDate_, date);
				oisCurve_terms.push_back(term);
			}

			double rate_tn = 0.0;
			if (size_tn > 0 && date < spotDate_)
			{
				// from tomorrow to a day after
				rate_tn = dynamic_cast<const LADataDouble&> ((data_tn_[0]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
				const LAPriceDataDayCount& dc_tn = dynamic_cast<const LAPriceDataDayCount&> ((data_tn_[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
				RateConvention rc_tn = LAMathYieldCurve::setRC(SIMPLE);
				LAPriceDataConvention conv_tn(dc_tn.getDayCount(), rc_tn);
			
				LADate tempDate;
				double on_df = 1.0;
				if(size_on > 0)
				{
					tempDate = date;
					on_df = oisCurve_dfs.back();
				}
				else
				{
					tempDate = baseDate_;
				}

				date = cal.getBusinessDay(tempDate, 1);
				oisCurve_dfs.push_back(on_df * conv.getDF(rate_tn, tempDate, date));

				double term = dc_act365.getTerm(baseDate_, date);
				oisCurve_terms.push_back(term);
			}
						            
            double shortTermRate;
			if (size_tn > 0 && date < spotDate_)
			{
				// If TN is provided, use the TN rate for the rest of the spot period
				shortTermRate = rate_tn;
			}
			else
			{
				if (size_on > 0)
				{
					// If TN is not provide but ON is provided, use ON rate for the rest of the spot period
					shortTermRate = rate_on;
				}
			}
									
			// Go through the rest of the spot period
			LADate lastDate = date;
			date = cal.getBusinessDay(date, 1);
			while (date <= spotDate_)
			{
				double term = dc_act365.getTerm(baseDate_, date);
				oisCurve_terms.push_back(term);
				oisCurve_dfs.push_back(oisCurve_dfs.back() * conv.getDF(shortTermRate, lastDate, date));	
				lastDate = date;
				date = cal.getBusinessDay(date, 1);
			}
			
			const double spotdf = oisCurve_dfs.back();
			for (unsigned int i = 1; i < stateVariable_grid_.size(); ++i)
			{
				oisCurve_terms.push_back(spotterm_ + stateVariable_grid_[i]);
				oisCurve_dfs.push_back(LAMath::exp(-pInter_StateVariable_->value(stateVariable_grid_[i])) * spotdf);		
			}
		}
		else
		{
			const double spotyield = stateVariable_rates_[1];

			// set business date before spotdate
			LADate date = cal.getBusinessDay(baseDate_, 1);
			while (date < spotDate_)
			{
				const double term = dc_act365.getTerm(baseDate_, date);
				oisCurve_terms.push_back(term);
				oisCurve_dfs.push_back(LAMath::exp(-spotyield * term));	
				date = cal.getBusinessDay(date, 1);
			}
			
			oisCurve_terms.push_back(spotterm_);
			const double spotdf = LAMath::exp(-spotyield * spotterm_);

			oisCurve_dfs.push_back(spotdf);
			for (unsigned int i = 1; i < stateVariable_grid_.size(); ++i)
			{
				oisCurve_terms.push_back(spotterm_ + stateVariable_grid_[i]);
				oisCurve_dfs.push_back(LAMath::exp(-pInter_StateVariable_->value(stateVariable_grid_[i])) * spotdf);		
			}
		}
	}
	else
	{
		for (unsigned int i = 1; i < stateVariable_grid_.size(); ++i)
		{
			oisCurve_terms.push_back(stateVariable_grid_[i]);
			oisCurve_dfs.push_back(LAMath::exp(-pInter_StateVariable_->value(stateVariable_grid_[i])));	
		}

	}

	//add term =0 and df = 1.0
	oisCurve_terms.insert(oisCurve_terms.begin(), 0.0);
	oisCurve_dfs.insert(oisCurve_dfs.begin(), 1.0);

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
		LAString currentMkt = it->second;
		LAString mktForCurve = dynamic_cast<CurveCalibrationData &>(yieldCurveProEntity).getMarketForCurve(curveName_);
		if (currentMkt == mktForCurve)
		{
			const LAString& curveName = it->first;
			LAString suffix;
			if (curveName != STD) 
			{
				suffix = "_" + curveName;
			}

			// set result
			objHolder.remove(CALIBRATION_DATA_TERMS + suffix);
			objHolder.remove(IR_CALIBRATION_DATA_DFS + suffix);
			objHolder.remove(CALIBRATION_DATA_INTERPOLATION + suffix);
			objHolder.remove(IR_CALIBRATION_DATA_INTERPOLATIONYG + suffix);
			objHolder.remove(IR_CALIBRATION_DATA_JACOBIAN_STATE_VARIABLES + suffix);
			objHolder.remove(IR_CALIBRATION_DATA_BUILTBYCURVEENGINE + suffix);

			objHolder.add(CALIBRATION_DATA_TERMS + suffix, new LADataDoubles(oisCurve_terms));
            objHolder.add(IR_CALIBRATION_DATA_DFS + suffix, new LADataDoubles(oisCurve_dfs));
			objHolder.add(CALIBRATION_DATA_INTERPOLATION + suffix, new LAPriceDataInterpolation()).convertFromString(interpolationYGStr_);
			objHolder.add(IR_CALIBRATION_DATA_INTERPOLATIONYG + suffix, new LAPriceDataInterpolation()).convertFromString(interpolationYGStr_);
			objHolder.add(IR_CALIBRATION_DATA_JACOBIAN_STATE_VARIABLES + suffix, new LADataDoubles(stateVariableAtSolution));
			objHolder.add(IR_CALIBRATION_DATA_BUILTBYCURVEENGINE + suffix, new LADataBool(true));

			CurveCalibration::setCurveConvention(objHolder, data_, curveName_);
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
