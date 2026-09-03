//
//  CurveInstrumentPricing.h
//  This file was previosuly called YieldCurvePricing.h
//
#include "CurveInstrumentPricing.h"

// Needed to initialise the MLIBQ dataInstance variable
#include "InitializeMLibETrading.h"

// External Includes
#include <cmath>
#include <algorithm>
#include <sstream>
#include <boost/algorithm/string.hpp>

// LA Includes
#include "LACurvePricingObject.h"
#include "LADateHelpers.h"
#include "LACurveForwardRateHelpers.h"
#include "LADateScheduleHelpers.h"

// Internal Includes
#include "LAMathDefine.h"
#include "LABasic.h"
#include "LAPriceDataConvention.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataReference.h"
#include "LAPriceDataInterpolation.h"
#include "LAPriceDataManager.h"
#include "LADataProcedure.h"
#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LAPriceDataInterpolation.h"
#include "LADataMultiReference.h"
#include "LADataMatrix.h"
#include "LADataInstance.h"
#include "LAFunctionUtilities.h"

// Etrading Includes
#include "LACoreComponentManager.h"
#include "CurveValidation.h"
#include "SwapUtilities.h"


namespace 
{
	// Find the index for the next cashflow, so that we can ignore cashflows in the past
	size_t getNextCashFlowIndex(const DateVector& floatAccrualDates, const LADate& asOf)
	{
        unsigned int nextCashflowIndex = 1;
        for (unsigned int i = 0; i < floatAccrualDates.size(); ++i)
        {
            if ( i == floatAccrualDates.size()-1  )
            {
                if ( floatAccrualDates[i] < asOf ) 
                    throw LACoreInvalidData("#Error: Par rate error; The underlying swap has expired.", __FILE__, __LINE__ );

                nextCashflowIndex++;
                break; 
            }
            if ( floatAccrualDates[i+1] >= asOf ) break;
            nextCashflowIndex++;
        }
		return nextCashflowIndex;
	}

	// Find the index for the next cashflow, so that we can ignore cashflows in the past
	size_t getNextCashFlowIndex(const DateVector& fixingStartDates, const DateVector& fixingEndDates, const LADate& asOf)
	{
		unsigned int nextCashflowIndex =0;
		for (unsigned int i = 0; i < fixingStartDates.size(); ++i)
		{
			if (i == fixingStartDates.size() - 1)
			{
				if (fixingStartDates[i] < asOf)
					throw LACoreInvalidData("#Error: The fixing date is earlier than asOfDate.", __FILE__, __LINE__);

				nextCashflowIndex++;
				break;
			}
			if (fixingEndDates[i] >= asOf) break;
			nextCashflowIndex++;
		}
		return nextCashflowIndex;
	}

	// Porduce a compounding rate for each of the accrual periods
	void getCompoundingRates(DoubleArray& equivalentRates, 
							const DateVector& floatAccrualDates, 
							const DoubleVector& accrualDateYearFractions, 
							size_t nextCashflowIndex,
							LADataInstance* dataInstance,
							double floatSpread,
							const LADate& asOf,
							const LAString& curveID,
							const LAString& foreCurveName,							
							const LAString& slidingRule,
							const LAString& calendar,
							const LAString& rollConvention,
							const LAString& dayCount,
							const LAString& interpolation,
							const LAString& oisCompoundingType)
	{
		if (floatAccrualDates[0] < asOf)
		{
			throw LACoreInvalidData("#Error: Only Spot or Forward Starting OIS Swaps supported", __FILE__, __LINE__ );
		}

		// Get equivalent rates over accrual periods
		for( size_t i = 1; i < floatAccrualDates.size(); ++i )
		{
			if (i < nextCashflowIndex)
			{
				equivalentRates[i-1] = 0;
			}
			else
			{
				// Get the equivalent rate of each **non-past** accrual period. 
				// The equivalent rate is obtained either through daily compounding over this period or finding the arithmetic average.
				double equivalentRate = etrading::LACurveForwardRateHelpers::compound(dataInstance,
																                 curveID,
																                 foreCurveName,				// The forecast curve given by user is expected to be an OIS curve
																                 floatAccrualDates[i-1],
																                 floatAccrualDates[i],
																                 floatSpread,
																                 "Business_Days",			//compoundingFrequency,
																                 false,						// is_start_roll
																                 slidingRule,
																                 calendar,
																                 rollConvention,
																                 dayCount,
																                 interpolation,
																                 oisCompoundingType);

				// Rate must be annualised
				equivalentRates[i-1] = equivalentRate / accrualDateYearFractions[i-1];
			}
		}
	}

}

double CurveInstrumentPricing::getParRate( DateVector& fixedAccrualDates,
										   DateVector& fixedPaymentDates,
										   DateVector& floatFixingDates,
										   DateVector& floatAccrualDates,
										   DateVector& floatPaymentDates,
										   LADataInstance* dataInstance,
										   const LAString& curveid,
										   LAString fixedDaycount,
										   LAString floatDaycount,
										   LAString interpolation,
										   LAString foreCurveName,
										   LAString dfCurveName,
										   bool isFWDInter,
										   bool useFirstFixing,
										   double firstFixing,
										   bool useLastFixing,
										   double lastFixing,
										   double floatSpread,
										   bool useFwdData,
										   bool isOIS,
										   LAString oisCompoundingType,
							 			   LAString calendar,
										   LAString rollConvention,
										   LAString slidingRule )
{
	upper( fixedDaycount );
    upper( floatDaycount );
	upper( interpolation );

	const LADataDate& atr = dynamic_cast<const LADataDate& >(dataInstance->getObjectPool().getObject(curveid,ENCHKTYPE_ISDEFINED).get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get());
	const LADate asofdate = atr.get();
	
    //
    // Validate Fixed Leg Accrual & Payment Dates
    //
	int nFixedAccrualDates = fixedAccrualDates.size()-1;
	for( int i = 1; i < nFixedAccrualDates; i++ )
	{
		if( fixedAccrualDates[i-1] >= fixedAccrualDates[i] )
            throw LACoreInvalidData("#Error: Invalid Fixed Leg Accrual Date(s).",__FILE__,__LINE__);
	}

    int nFixedPaymentDates = fixedPaymentDates.size()-1;
	for( int i = 1; i < nFixedPaymentDates; i++ )
	{
		if( fixedPaymentDates[i-1] >= fixedPaymentDates[i] )
            throw LACoreInvalidData("#Error: Invalid Fixed Leg Payment Date(s).",__FILE__,__LINE__);
	}

    //
    // Validate Floating Leg Accrual, Fixing & Payment Dates
    //
    int nFloatAccrualDates = floatAccrualDates.size()-1;
	for( int i = 1; i < nFloatAccrualDates; i++ )
	{
		if( floatAccrualDates[i-1] >= floatAccrualDates[i] )
			throw LACoreInvalidData("#Error: Invalid Float Leg Accrual Date(s).",__FILE__,__LINE__);
	}

    int nFloatFixingDates = floatFixingDates.size()-1;
	for( int i = 1; i < nFloatFixingDates; i++ )
	{
		if( floatFixingDates[i-1] >= floatFixingDates[i] )
			throw LACoreInvalidData("#Error: Invalid Float Leg Fixing Date(s).",__FILE__,__LINE__);
	}

    int nFloatPaymentDates = floatPaymentDates.size()-1;
	for( int i = 1; i < nFloatPaymentDates; i++ )
	{
		if( floatPaymentDates[i-1] >= floatPaymentDates[i] )
			throw LACoreInvalidData("#Error: Invalid Float Leg Payment Date(s).",__FILE__,__LINE__);
	}

	etrading::LACurvePricingObject& yc = etrading::LACurveForwardRateHelpers::getYieldCurveForCurveID(dataInstance,curveid);
	LAString foreInter = LACoreComponentManager::getInterpolation(interpolation);

	LAString dfCurve = etrading::getCurveStaticDataTableName( curveid, dfCurveName );
	LAString dfInter = LACoreComponentManager::getInterpolation(etrading::getCurveInterpolation(curveid, dfCurve).toUpper());

	LAString fixeddc = LACoreComponentManager::getDayCount( fixedDaycount );
    LAString floatdc = LACoreComponentManager::getDayCount( floatDaycount );
	    
    yc.getDayCount().setDayCount( floatdc ); // floatdc
	
    etrading::LACurveForwardRateHelpers::setCalendarForCurveID( yc, "" );
	yc.getSlidingRule().convertFromString( NO_CH );

	if (isFWDInter && !etrading::LACurveForwardRateHelpers::setUpForwardDayCount(dataInstance, curveid, foreCurveName, yc) && !isOIS)
	{
        throw LACoreInvalidData("#Error: Invalid Forward Rates - Unable to generate forward rates for Curve Instrument Calibration & Pricing",__FILE__,__LINE__);
	}
		
	const LAObject& YieldData       = yc.getYieldData().get().get();
	const LADate& asOf              = dynamic_cast<const LADataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();
    
    if ( floatPaymentDates[floatPaymentDates.size()-1] < asOf && fixedPaymentDates[fixedPaymentDates.size()-1] < asOf) 
        throw LACoreInvalidData("#Error: Par rate error; The underlying swap has expired.", __FILE__, __LINE__ );
    
    const LAPriceDataCalendar& cal       = yc.getCalendar();
	const LAPriceDataSlidingRule& sr     = yc.getSlidingRule();

	LADate fixedAccrualStartDate    = sr.getDate( fixedAccrualDates.front(), cal );
    LADate floatAccrualStartDate    = sr.getDate( floatAccrualDates.front(), cal );
    
    LADate fixedAccrualEndDate      = sr.getDate( fixedAccrualDates.back(), cal );
    LADate floatAccrualEndDate      = sr.getDate( floatAccrualDates.back(), cal );

	DayCount dc_act365( ACT_365 );
	LAPriceDataDayCount data_dc_act365( dc_act365 );

    // Get client specified daycount conventions
    LAPriceDataDayCount fixedLegDaycount;
    fixedLegDaycount.convertFromString( fixedDaycount );
        
    LAPriceDataDayCount floatLegDaycount;
    floatLegDaycount.convertFromString( floatDaycount );

	yc.setCurveNameAndInterpolation(dfCurveName, dfInter);
	
    //
    // Calculate Fixed Leg Annuity value using client specified daycount convention
    //
    double annuity = yc.getAnnuity( fixedAccrualDates, fixedPaymentDates, fixedLegDaycount );
	
	// Calculate the floating coupon accrual periods
	DoubleArray accrualDateYearFractions( floatAccrualDates.size() -1 );  
	for( size_t i = 1; i < floatAccrualDates.size(); i++ )
	{
		// "accrualDateYearFractions" used for the coupon period
		accrualDateYearFractions[i-1] = floatLegDaycount.getTerm( floatAccrualDates[i-1], floatAccrualDates[i] );
	}

	size_t nextCashflowIndex = getNextCashFlowIndex(floatAccrualDates, asOf);

	double floaterPV = 0.;
	if(isOIS && oisCompoundingType.size() != 0)
	{		
		std::string curveID(yc.getName().convertToString().getCString());
		size_t pos = curveID.find(etrading::LACurveForwardRateHelpers::YIELD_CURVE_NAME_PREFIX);
		if ( pos != std::string::npos)
		{
			size_t len = std::string(etrading::LACurveForwardRateHelpers::YIELD_CURVE_NAME_PREFIX).length();
			curveID = curveID.substr(pos + len, curveID.length() - len - pos - 1); 
		}

		LAString dayCount = yc.getDayCount().convertToString();

		// Get equivalent rate over accrual periods
		DoubleArray equivalentRates( floatAccrualDates.size()  -1 );		
		getCompoundingRates(equivalentRates, 
							floatAccrualDates, 
							accrualDateYearFractions, 
							nextCashflowIndex,
							yc.getDataInstance(),
							floatSpread,
							asOf,
							LAString(curveID.c_str()),
							foreCurveName,							
							slidingRule,
							calendar,
							rollConvention,
							dayCount,
							interpolation,
							oisCompoundingType);
							
		// Get Discount Factors
		DoubleArray dfs  ( floatPaymentDates.size() -1 );
		yc.setCurveNameAndInterpolation(dfCurveName, dfInter);
		for( size_t i = 1; i < floatPaymentDates.size(); ++i )
		{
			const double term = data_dc_act365.getTerm( asOf, floatPaymentDates[i] );

			// Calculate discount factors using PAYMENT DATES ***
			dfs[i-1] = ( i < nextCashflowIndex ) ? 0.0 : yc.getDF( term, &dc_act365 );
		}		

		// Calculate Floater PV
		for( size_t i = 1; i < floatAccrualDates.size(); i++ )
		{			
			double rate             = equivalentRates[i-1];
			double accrualPeriod    = accrualDateYearFractions[i-1];
			double discFactor       = dfs[i-1];
            
			// Floating Spread is in Basis Points, which we convert to percent here
			floaterPV += rate * accrualPeriod * discFactor;
		}
	}
	else
	{		
		//
		// Calculate Floating Leg Value
		//
		if (foreCurveName != dfCurveName)
		{
			LAString            freq_forecast = ""; 
			LAPriceDataCalendar      cal_forecast; 
			LAPriceDataSlidingRule   sld_forecast; 
			LAPriceDataDayCount      dc_forecast;
			LAString            accessary_forecast = "";
		
			yc.getCurveConvention( freq_forecast, cal_forecast, sld_forecast, dc_forecast, accessary_forecast, foreCurveName );

			DoubleArray rates( floatFixingDates.size()  -1 );
			DoubleArray dfs  ( floatPaymentDates.size() -1 );
        
			DoubleArray fixingDateYearFractions( floatFixingDates.size() -1 );  
			DoubleArray payDateYearFractions( floatPaymentDates.size() -1 );  

			if ( floatFixingDates.size() != floatAccrualDates.size() || floatAccrualDates.size() != floatPaymentDates.size() )
			{
				throw LACoreInvalidData("#Error: Floating schedule error. Inconsistent number of fixing, acrrual and payment dates.", __FILE__, __LINE__ );
			}
			  
			yc.setCurveNameAndInterpolation(foreCurveName, foreInter);

			// Get forward rates from interpolation
			if ( isFWDInter )
			{
				// Optional 'UseFwdData', False (default) = imply forward from discount factors, True = use forward data directly				
				const LAInterpolationBase &fwd_inter = yc.getFWDInterpolation(&foreCurveName, useFwdData);
				for( size_t i = 1; i < floatAccrualDates.size(); ++i )
				{
					const double fixingDateYearFraction     = data_dc_act365.getTerm( asOf, floatFixingDates[i-1] ); 
                
					// Apply the first fixing rate for the first stub, if needed
					if ( i == nextCashflowIndex && useFirstFixing )
					{
						rates[i-1] = firstFixing;
					}
					// Apply the last fixing rate for the last stub, if needed
					else if ( i == floatAccrualDates.size() - 1 && useLastFixing )
					{
						rates[i-1] = lastFixing;
					}
					else
					{
						// *** Calculate rates using FIXING DATES ***
						rates[i-1] = ( i < nextCashflowIndex ) ? 0.0 : fwd_inter.value( fixingDateYearFraction );
                    
					}
				}
			}
			// Get forward rates implied from discount factors
			else
			{
				for( size_t i = 1; i < floatPaymentDates.size(); ++i )
				{
					LADate fixingEndDate = etrading::LADateHelpers::getDate(floatFixingDates[i-1], accessary_forecast, sld_forecast, &cal_forecast, true);
					double fixingFraction = floatLegDaycount.getTerm( floatFixingDates[i-1], fixingEndDate );
				 
					// Apply the first fixing rate for the first stub, if needed
					if ( i == nextCashflowIndex && useFirstFixing )
					{
						rates[i-1] = firstFixing;
					}
					// Apply the last fixing rate for the last stub, if needed
					else if ( i == floatAccrualDates.size() - 1 && useLastFixing )
					{
						rates[i-1] = lastFixing;
					}
					else
					{
						// *** Calculate rates using FIXING DATES ***
						// Note: set past rates to zero						
						
						double fixingStartTerm = data_dc_act365.getTerm( asOf, floatFixingDates[i-1] );
						double fixingEndTerm = data_dc_act365.getTerm( asOf, fixingEndDate );
						double startDF = yc.getDF( fixingStartTerm, &dc_act365 );
						double endDF = yc.getDF( fixingEndTerm, &dc_act365 );
						rates[i-1] = ( i  <nextCashflowIndex ) ? 0.0 : (  startDF / endDF - 1.) / fixingFraction;
					}
				}
			}
		
			// Get Discount Factors
			yc.setCurveNameAndInterpolation(dfCurveName, dfInter);
			for( size_t i = 1; i < floatPaymentDates.size(); ++i )
			{
				const double term = data_dc_act365.getTerm( asOf, floatPaymentDates[i] );

				// *** Calculate discount factors using PAYMENT DATES ***
				// Note: We must ignore past cashflows
				dfs[i-1] = ( i < nextCashflowIndex ) ? 0.0 : yc.getDF( term, &dc_act365 );
			}

			// Calculate Floater PV
			for( size_t i = 1; i < floatAccrualDates.size(); i++ )
			{			
				double rate             = rates[i-1];
				double accrualPeriod    = accrualDateYearFractions[i-1];
				double discFactor       = dfs[i-1];
            
				// Floating Spread is in Basis Points, which we convert to percent here
				floaterPV += ( rate + ( floatSpread / 10000.0 ) ) * accrualPeriod * discFactor;
			}
		}
		else
		{
			// Note: Past Historic Cashflows not supported i.e. negative terms or yearFractions will return an error message.

			// Libor Discounting Scenario
			floaterPV   = yc.getDF( data_dc_act365.getTerm( asOf, floatAccrualStartDate ), &dc_act365 ) 
						- yc.getDF( data_dc_act365.getTerm( asOf, floatAccrualEndDate   ), &dc_act365 );
		}
	}

	return floaterPV / annuity;
}



double CurveInstrumentPricing::getSwapPV( bool&             isFixedRatePayerSwap,
										  double&			notional,
										  DateVector&		fixedAccrualDates,
										  DateVector&		fixedPaymentDates,
										  DateVector&		floatFixingDates,
										  DateVector&		floatAccrualDates,
										  DateVector&		floatPaymentDates,
										  LADataInstance*	dataInstance,
										  const LAString&	curveid,
										  double&			fixedRate,
										  const LAString&	fixeddaycount,
										  double&			floatSpreadInBasisPoints,
										  const LAString&	floatdaycount,
										  const LAString&	interp,
										  const LAString&	foreCurveName,
										  const LAString&	dfCurveName,
										  bool				isFWDInter,
										  bool				useFirstFixing,
										  double			firstFixing,
										  bool				useLastFixing,
										  double			lastFixing,								 
										  bool				useFwdData,
										  bool				isOIS,
										  const LAString&	compoundingMethod,
										  const LAString&	floatCalendar,
										  const LAString&	floatRollConv,
										  const LAString&	slidingRule,
										  const LAString&	floatFrequency)
{
	LAString fixedDaycount = fixeddaycount;
	LAString floatDaycount = floatdaycount;
	LAString interpolation = interp;
	upper( fixedDaycount );
    upper( floatDaycount );
	upper( interpolation );

	const LADataDate& atr = dynamic_cast<const LADataDate& >(dataInstance->getObjectPool().getObject(curveid,ENCHKTYPE_ISDEFINED).get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get());
	const LADate asofdate = atr.get();
	
    //
    // Validate Fixed Leg Accrual & Payment Dates
    //
	int nFixedAccrualDates = fixedAccrualDates.size()-1;
	for( int i = 1; i < nFixedAccrualDates; i++ )
	{
		if( fixedAccrualDates[i-1] >= fixedAccrualDates[i] )
            throw LACoreInvalidData("#Error: Invalid Fixed Leg Accrual Date(s).",__FILE__,__LINE__);
	}

    int nFixedPaymentDates = fixedPaymentDates.size()-1;
	for( int i = 1; i < nFixedPaymentDates; i++ )
	{
		if( fixedPaymentDates[i-1] >= fixedPaymentDates[i] )
            throw LACoreInvalidData("#Error: Invalid Fixed Leg Payment Date(s).",__FILE__,__LINE__);
	}

    //
    // Validate Floating Leg Accrual, Fixing & Payment Dates
    //
    int nFloatAccrualDates = floatAccrualDates.size()-1;
	for( int i = 1; i < nFloatAccrualDates; i++ )
	{
		if( floatAccrualDates[i-1] >= floatAccrualDates[i] )
			throw LACoreInvalidData("#Error: Invalid Float Leg Accrual Date(s).",__FILE__,__LINE__);
	}

    int nFloatFixingDates = floatFixingDates.size()-1;
	for( int i = 1; i < nFloatFixingDates; i++ )
	{
		if( floatFixingDates[i-1] >= floatFixingDates[i] )
			throw LACoreInvalidData("#Error: Invalid Float Leg Fixing Date(s).",__FILE__,__LINE__);
	}

    int nFloatPaymentDates = floatPaymentDates.size()-1;
	for( int i = 1; i < nFloatPaymentDates; i++ )
	{
		if( floatPaymentDates[i-1] > floatPaymentDates[i] )
			throw LACoreInvalidData("#Error: Invalid Float Leg Payment Date(s).",__FILE__,__LINE__);
	}

	etrading::LACurvePricingObject& yc = etrading::LACurveForwardRateHelpers::getYieldCurveForCurveID(dataInstance,curveid);
	LAString foreInter = LACoreComponentManager::getInterpolation(interpolation);

	LAString dfCurve = etrading::getCurveStaticDataTableName( curveid, dfCurveName );
	LAString dfInter = LACoreComponentManager::getInterpolation(etrading::getCurveInterpolation(curveid, dfCurve).toUpper());

	LAString fixeddc = LACoreComponentManager::getDayCount( fixedDaycount );
    LAString floatdc = LACoreComponentManager::getDayCount( floatDaycount );
	    
    yc.getDayCount().setDayCount( floatdc ); // floatdc
	
    etrading::LACurveForwardRateHelpers::setCalendarForCurveID( yc, "" );
	yc.getSlidingRule().convertFromString( NO_CH );

	if (isFWDInter && !etrading::LACurveForwardRateHelpers::setUpForwardDayCount(dataInstance, curveid, foreCurveName, yc) && !isOIS)
	{
        throw LACoreInvalidData("#Error: Invalid Forward Rates - Unable to generate forward rates for Curve Instrument Calibration & Pricing",__FILE__,__LINE__);
	}
	
	const LAObject& YieldData       = yc.getYieldData().get().get();
	const LADate& asOf              = dynamic_cast<const LADataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();
    
    if ( floatPaymentDates[floatPaymentDates.size()-1] < asOf && fixedPaymentDates[fixedPaymentDates.size()-1] < asOf) 
        return 0.0; // Swap has expired return swap pv = 0.0
    
    const LAPriceDataCalendar& cal       = yc.getCalendar();
	const LAPriceDataSlidingRule& sr     = yc.getSlidingRule();

	LADate fixedAccrualStartDate    = sr.getDate( fixedAccrualDates.front(), cal );
    LADate floatAccrualStartDate    = sr.getDate( floatAccrualDates.front(), cal );
    
    LADate fixedAccrualEndDate      = sr.getDate( fixedAccrualDates.back(), cal );
    LADate floatAccrualEndDate      = sr.getDate( floatAccrualDates.back(), cal );

	DayCount dc_act365( ACT_365 );
	LAPriceDataDayCount data_dc_act365( dc_act365 );

    // Get client specified daycount conventions
    LAPriceDataDayCount fixedLegDaycount;
    fixedLegDaycount.convertFromString( fixedDaycount );
        
    LAPriceDataDayCount floatLegDaycount;
    floatLegDaycount.convertFromString( floatDaycount );

	yc.setCurveNameAndInterpolation(dfCurveName, dfInter);
	
    //
    // Calculate Fixed Leg Annuity value using client specified daycount convention
    //
    const double annuity = yc.getAnnuity( fixedAccrualDates, fixedPaymentDates, fixedLegDaycount );
	
    //
    // Calculate Floating Leg Values
    //
	LAString            freq_forecast       = ""; 
	LAPriceDataCalendar      cal_forecast; 
	LAPriceDataSlidingRule   sld_forecast; 
	LAPriceDataDayCount      dc_forecast;
	LAString            accessary_forecast  = "";
		
	yc.getCurveConvention( freq_forecast, cal_forecast, sld_forecast, dc_forecast, accessary_forecast, foreCurveName );
	if (isOIS || etrading::getCurveType(curveid, foreCurveName).toUpper() == OIS)
	{
		accessary_forecast = etrading::fromFrequencyToTerm(floatFrequency);
		sld_forecast.convertFromString(floatRollConv);
		cal_forecast.convertFromString(floatCalendar);
	}

    DoubleArray rates( floatFixingDates.size()  -1 );
    DoubleArray dfs  ( floatPaymentDates.size() -1 );
        
    DoubleArray fixingDateYearFractions( floatFixingDates.size() -1 );  
	DoubleArray accrualDateYearFractions( floatAccrualDates.size() -1 );  
    DoubleArray payDateYearFractions( floatPaymentDates.size() -1 );  

    if ( floatFixingDates.size() != floatAccrualDates.size() || floatAccrualDates.size() != floatPaymentDates.size() )
        throw LACoreInvalidData("#Error: Floating schedule error. Inconsistent number of fixing, acrrual and payment dates.", __FILE__, __LINE__ );
         
    // Calculate the floating coupon accrual periods
    for( size_t i = 1; i < floatAccrualDates.size(); i++ )
    {
        // "accrualDateYearFractions" used for the coupon period
        accrualDateYearFractions[i-1] = floatLegDaycount.getTerm( floatAccrualDates[i-1], floatAccrualDates[i] );
    }

    // Find the index for the next cashflow, so that we can ignore cashflows in the past
    unsigned int nextCashflowIndex = 1;
    for (unsigned int i = 0; i < floatAccrualDates.size(); ++i)
    {
        if ( i == floatAccrualDates.size()-1  )
        {
            if ( floatAccrualDates[i] < asOf ) nextCashflowIndex++;
            break; 
        }
        if ( floatAccrualDates[i+1] >= asOf ) break;
        nextCashflowIndex++;
    }

	const double oneBasisPoint = 0.0001;

    // Get forward rates from interpolation
	if(isOIS && compoundingMethod.size() != 0)
	{
		std::string curveID(yc.getName().convertToString().getCString());
		size_t pos = curveID.find(etrading::LACurveForwardRateHelpers::YIELD_CURVE_NAME_PREFIX);
		if ( pos != std::string::npos)
		{
			size_t len = std::string(etrading::LACurveForwardRateHelpers::YIELD_CURVE_NAME_PREFIX).length();
			curveID = curveID.substr(pos + len, curveID.length() - len - pos - 1); 
		}

		LAString dayCount = yc.getDayCount().convertToString();

		// Get equivalent rate over accrual periods
		getCompoundingRates(rates, 
							floatAccrualDates, 
							accrualDateYearFractions, 
							nextCashflowIndex,
							yc.getDataInstance(),
							floatSpreadInBasisPoints * oneBasisPoint,
							asOf,
							LAString(curveID.c_str()),
							foreCurveName,							
							slidingRule,
							floatCalendar,
							floatRollConv,
							dayCount,
							interpolation,
							compoundingMethod);
	}
	else
	{
		yc.setCurveNameAndInterpolation(foreCurveName, foreInter);
		if ( isFWDInter )
		{
			//const LAInterpolationBase &fwd_inter = getFWDInterpolation(&foreCurveName);
			const LAInterpolationBase &fwd_inter = yc.getFWDInterpolation(&foreCurveName, useFwdData);
	
			for( size_t i = 1; i < floatAccrualDates.size(); i++ )
			{
				fixingDateYearFractions[i-1]            = dc_forecast.getTerm( floatFixingDates[i-1], floatFixingDates[i] );
				const double fixingDateYearFraction     = data_dc_act365.getTerm( asOf, floatFixingDates[i-1] ); 
                
				// Apply the first fixing rate for the first stub, if needed
				if ( i == nextCashflowIndex && useFirstFixing )
				{
					rates[i-1] = firstFixing;
				}
				// Apply the last fixing rate for the last stub, if needed
				else if ( i == floatAccrualDates.size() - 1 && useLastFixing )
				{
					rates[i-1] = lastFixing;
				}
				else
				{
					// *** Calculate rates using FIXING DATES ***
					rates[i-1] = (i<nextCashflowIndex) ? 0.0 : fwd_inter.value( fixingDateYearFraction );
				}
			}
		}
		// Get forward rates implied from discount factors
		else
		{			
			for( size_t i = 1; i < floatPaymentDates.size(); i++ )
			{
				LADate fixingEndDate = etrading::LADateHelpers::getDate(floatFixingDates[i-1], accessary_forecast, sld_forecast, &cal_forecast, true);
				double fixingFraction = floatLegDaycount.getTerm( floatFixingDates[i-1], fixingEndDate );

				// Apply the first fixing rate for the first stub, if needed
				if ( i == nextCashflowIndex && useFirstFixing )
				{
					rates[i-1] = firstFixing;
				}
				// Apply the last fixing rate for the last stub, if needed
				else if ( i == floatAccrualDates.size() - 1 && useLastFixing )
				{
					rates[i-1] = lastFixing;
				}
				else
				{
					// *** Calculate rates using FIXING DATES ***
					rates[i-1] = (i<nextCashflowIndex) ? 0.0 :
						( yc.getDF( data_dc_act365.getTerm( asOf, floatFixingDates[i-1] ), &dc_act365 ) 
						/ yc.getDF( data_dc_act365.getTerm( asOf, fixingEndDate ), &dc_act365 ) - 1.) / fixingFraction;

				}
			}
		}
	}
		
    // Get Discount Factors
	yc.setCurveNameAndInterpolation(dfCurveName, dfInter);
	for( size_t i = 1; i < floatPaymentDates.size(); i++ )
	{
        // *** Calculate discount factors using PAYMENT DATES ***
		dfs[i-1] = (i<nextCashflowIndex) ? 0.0 : yc.getDF( data_dc_act365.getTerm( asOf, floatPaymentDates[i] ), &dc_act365 );
	}

    // PV Parameter Specification
	double swapPV                           = 0.0;
    double fixedLegPV                       = 0.0;
    double floatLegPV                       = 0.0;    

    // PV from Fixed Leg
    fixedLegPV                              = fixedRate * annuity;

    // PV from Float Leg
	for( size_t i = 1; i < floatAccrualDates.size(); i++ )
	{			
		double floatRatePlusSpread          = rates[i-1] + ( floatSpreadInBasisPoints * oneBasisPoint );
        double floatAccrualPeriod           = accrualDateYearFractions[i-1];
        double floatDiscFactor              = dfs[i-1];
            
        floatLegPV += floatRatePlusSpread * floatAccrualPeriod * floatDiscFactor;
	}

    // Swap PV
    swapPV    = notional * ( fixedLegPV - floatLegPV );
    
    if ( isFixedRatePayerSwap )
        swapPV = -swapPV;

	return swapPV;
}


double CurveInstrumentPricing::getSwapDV01( bool             isFixedRatePayerSwap,
                                   double           notional,
                                   DateVector&      fixedAccrualDates,
                                   DateVector&      fixedPaymentDates,
                                   DateVector&      floatFixingDates,
                                   DateVector&      floatAccrualDates,
                                   DateVector&      floatPaymentDates,
                                   LADataInstance*          dataInstance,
                                   const LAString&  curveid,
                                   double&          fixedRate,
                                   LAString         fixedDaycount,
                                   double&          floatSpreadInBasisPoints,
                                   LAString         floatDaycount,
                                   LAString         interpolation,
                                   LAString         foreCurveName,
                                   LAString         dfCurveName,
                                   bool             isFWDInter,
                                   bool             useFirstFixing,
                                   double           firstFixing,
                                   bool             useLastFixing,
                                   double           lastFixing,
								   bool			    isOIS,
								   LAString		    compoundingMethod,
								   LAString		    floatCalendar,
								   LAString		    floatRollConv,
								   LAString		    slidingRule,
								   const LAString&  floatFrequency)
{
	upper( fixedDaycount );
    upper( floatDaycount );
	upper( interpolation );

	const LADataDate& atr = dynamic_cast<const LADataDate& >(dataInstance->getObjectPool().getObject(curveid,ENCHKTYPE_ISDEFINED).get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get());
	const LADate asofdate = atr.get();
	
    //
    // Validate Fixed Leg Accrual & Payment Dates
    //
	int nFixedAccrualDates = fixedAccrualDates.size()-1;
	for( int i = 1; i < nFixedAccrualDates; i++ )
	{
		if( fixedAccrualDates[i-1] >= fixedAccrualDates[i] )
            throw LACoreInvalidData("#Error: Invalid Fixed Leg Accrual Date(s).",__FILE__,__LINE__);
	}

    int nFixedPaymentDates = fixedPaymentDates.size()-1;
	for( int i = 1; i < nFixedPaymentDates; i++ )
	{
		if( fixedPaymentDates[i-1] >= fixedPaymentDates[i] )
            throw LACoreInvalidData("#Error: Invalid Fixed Leg Payment Date(s).",__FILE__,__LINE__);
	}

    //
    // Validate Floating Leg Accrual, Fixing & Payment Dates
    //
    int nFloatAccrualDates = floatAccrualDates.size()-1;
	for( int i = 1; i < nFloatAccrualDates; i++ )
	{
		if( floatAccrualDates[i-1] >= floatAccrualDates[i] )
			throw LACoreInvalidData("#Error: Invalid Float Leg Accrual Date(s).",__FILE__,__LINE__);
	}

    int nFloatFixingDates = floatFixingDates.size()-1;
	for( int i = 1; i < nFloatFixingDates; i++ )
	{
		if( floatFixingDates[i-1] >= floatFixingDates[i] )
			throw LACoreInvalidData("#Error: Invalid Float Leg Fixing Date(s).",__FILE__,__LINE__);
	}

    int nFloatPaymentDates = floatPaymentDates.size()-1;
	for( int i = 1; i < nFloatPaymentDates; i++ )
	{
		if( floatPaymentDates[i-1] >= floatPaymentDates[i] )
			throw LACoreInvalidData("#Error: Invalid Float Leg Payment Date(s).",__FILE__,__LINE__);
	}

    etrading::LACurvePricingObject& yc = etrading::LACurveForwardRateHelpers::getYieldCurveForCurveID(dataInstance,curveid);
	LAString foreInter = LACoreComponentManager::getInterpolation(interpolation);

	LAString dfCurve = etrading::getCurveStaticDataTableName( curveid, dfCurveName );
	LAString dfInter = LACoreComponentManager::getInterpolation(etrading::getCurveInterpolation(curveid, dfCurve).toUpper());

	LAString fixeddc = LACoreComponentManager::getDayCount( fixedDaycount );
    LAString floatdc = LACoreComponentManager::getDayCount( floatDaycount );
	    
    yc.getDayCount().setDayCount( floatdc ); // floatdc
	
    etrading::LACurveForwardRateHelpers::setCalendarForCurveID( yc, "" );
	yc.getSlidingRule().convertFromString( NO_CH );

	if (isFWDInter && !etrading::LACurveForwardRateHelpers::setUpForwardDayCount(dataInstance, curveid, foreCurveName, yc) && !isOIS)
	{
		throw LACoreInvalidData("#Error: Invalid Forward Rates - Unable to generate forward rates for Curve Instrument Calibration & Pricing",__FILE__,__LINE__);
	}
	
	// Constant(s)
    const double oneBasisPoint              = 0.0001;
    const double floatSpreadInPercent       = floatSpreadInBasisPoints * oneBasisPoint;
    
    //
    // 1. Load Yield Curve Data
    // -------------------------------------------------------------------------------
    //
    

    const LAObject& YieldData       = yc.getYieldData().get().get();
	const LADate& asOf              = dynamic_cast<const LADataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();

    // Check if Swap has Expired
    if ( floatPaymentDates[floatPaymentDates.size()-1] < asOf && fixedPaymentDates[fixedPaymentDates.size()-1] < asOf) 
    {
        return 0.0;
    }
        
    const LAPriceDataCalendar& cal       = yc.getCalendar();
	const LAPriceDataSlidingRule& sr     = yc.getSlidingRule();

	LADate fixedAccrualStartDate    = sr.getDate( fixedAccrualDates.front(), cal );
    LADate floatAccrualStartDate    = sr.getDate( floatAccrualDates.front(), cal );
        
    LADate fixedAccrualEndDate      = sr.getDate( fixedAccrualDates.back(), cal );
    LADate floatAccrualEndDate      = sr.getDate( floatAccrualDates.back(), cal );

	DayCount dc_act365( ACT_365 );
	LAPriceDataDayCount data_dc_act365( dc_act365 );

    // Get client specified daycount conventions
    LAPriceDataDayCount fixedLegDaycount;
    fixedLegDaycount.convertFromString( fixedDaycount );
            
    LAPriceDataDayCount floatLegDaycount;
    floatLegDaycount.convertFromString( floatDaycount );

	yc.setCurveNameAndInterpolation(dfCurveName, dfInter);
    

    //
    // 2. Calculate Fixed Leg Vales
    // -------------------------------------------------------------------------------
    //
    

    DoubleArray fixedLegAccrualDateYearFractions( fixedAccrualDates.size() -1 );
    DoubleArray fixedLegDiscFactors( fixedPaymentDates.size() -1 );
        
    if ( fixedAccrualDates.size() != fixedPaymentDates.size() )
        throw LACoreInvalidData("#Error: Fixed schedule error. Inconsistent number of acrrual and payment dates.", __FILE__, __LINE__ );

    for( size_t i = 1; i < fixedAccrualDates.size(); i++ )
    {
        // "fixedLegAccrualDateYearFractions" used for the coupon period
        fixedLegAccrualDateYearFractions[i-1] = fixedLegDaycount.getTerm( fixedAccrualDates[i-1], fixedAccrualDates[i] );
    }

    // Fixed Leg Annuity ( note scaled by notional )
    double AnnuityFixed = notional * yc.getAnnuity( fixedAccrualDates, fixedPaymentDates, fixedLegDaycount );
    

    //
    // 3. Calculate Floating Leg Values
    // -------------------------------------------------------------------------------
    //
	

    LAString            freq_forecast       = ""; 
	LAPriceDataCalendar      cal_forecast; 
	LAPriceDataSlidingRule   sld_forecast; 
	LAPriceDataDayCount      dc_forecast;
	LAString            accessary_forecast  = "";
	    	
	yc.getCurveConvention( freq_forecast, cal_forecast, sld_forecast, dc_forecast, accessary_forecast, foreCurveName );
	if (isOIS || etrading::getCurveType(curveid, foreCurveName).toUpper() == OIS)
	{
		accessary_forecast = etrading::fromFrequencyToTerm(floatFrequency);
		sld_forecast.convertFromString(floatRollConv);
		cal_forecast.convertFromString(floatCalendar);
	}

    DoubleArray floatLegRates( floatFixingDates.size()  -1 );
    DoubleArray floatLegDiscFactors( floatPaymentDates.size() -1 );
            
    DoubleArray fixingDateYearFractions( floatFixingDates.size() -1 );  
	DoubleArray floatLegAccrualDateYearFractions( floatAccrualDates.size() -1 );  
    DoubleArray payDateYearFractions( floatPaymentDates.size() -1 );  

    if ( floatFixingDates.size() != floatAccrualDates.size() || floatAccrualDates.size() != floatPaymentDates.size() )
        throw LACoreInvalidData("#Error: Floating schedule error. Inconsistent number of fixing, acrrual and payment dates.", __FILE__, __LINE__ );
             
    // Calculate the floating coupon accrual periods
    for( size_t i = 1; i < floatAccrualDates.size(); i++ )
    {
        // "floatLegAccrualDateYearFractions" used for the coupon period
        floatLegAccrualDateYearFractions[i-1] = floatLegDaycount.getTerm( floatAccrualDates[i-1], floatAccrualDates[i] );
    }

    // Find the index for the next cashflow, so that we can ignore cashflows in the past
    unsigned int nextCashflowIndex = 1;
    for (unsigned int i = 0; i < floatAccrualDates.size(); ++i)
    {
        if ( i == floatAccrualDates.size()-1  )
        {
            if ( floatAccrualDates[i] < asOf ) nextCashflowIndex++;
            break; 
        }
        if ( floatAccrualDates[i+1] >= asOf ) break;
        nextCashflowIndex++;
    }

    // Get forward rates from interpolation
	if(isOIS && compoundingMethod.size() != 0)
	{
	    std::string curveID(yc.getName().convertToString().getCString());
	    size_t pos = curveID.find(etrading::LACurveForwardRateHelpers::YIELD_CURVE_NAME_PREFIX);
	    if ( pos != std::string::npos)
	    {
	    	size_t len = std::string(etrading::LACurveForwardRateHelpers::YIELD_CURVE_NAME_PREFIX).length();
	    	curveID = curveID.substr(pos + len, curveID.length() - len - pos - 1); 
	    }

	    LAString dayCount = yc.getDayCount().convertToString();

	    // Get equivalent rate over accrual periods
	    getCompoundingRates(floatLegRates, 
	    					floatAccrualDates, 
	    					floatLegAccrualDateYearFractions, 
	    					nextCashflowIndex,
	    					yc.getDataInstance(),
	    					floatSpreadInBasisPoints * oneBasisPoint,
	    					asOf,
	    					LAString(curveID.c_str()),
	    					foreCurveName,							
	    					slidingRule,
	    					floatCalendar,
	    					floatRollConv,
	    					dayCount,
	    					interpolation,
	    					compoundingMethod);
	}
	else
	{
		yc.setCurveNameAndInterpolation(foreCurveName, foreInter);
	    if ( isFWDInter )
	    {
	    	const LAInterpolationBase &fwd_inter = yc.getFWDInterpolation(&foreCurveName);
	    	for( size_t i = 1; i < floatAccrualDates.size(); i++ )
	    	{
	    		const double fixingDateYearFraction     = data_dc_act365.getTerm( asOf, floatFixingDates[i-1] ); 
                    
	    		// Apply the first fixing rate for the first stub, if needed
	    		if ( i == nextCashflowIndex && useFirstFixing )
	    		{
	    			floatLegRates[i-1] = firstFixing;
	    		}
	    		// Apply the last fixing rate for the last stub, if needed
	    		else if ( i == floatAccrualDates.size() - 1 && useLastFixing )
	    		{
	    			floatLegRates[i-1] = lastFixing;
	    		}
	    		else
	    		{
	    			// *** Calculate rates using FIXING DATES ***
	    			floatLegRates[i-1] = (i<nextCashflowIndex) ? 0.0 : fwd_inter.value( fixingDateYearFraction );
	    		}
	    	}
	    }
	    // Get forward rates implied from discount factors
	    else
	    {	    	
	    	for( size_t i = 1; i < floatPaymentDates.size(); i++ )
	    	{
				LADate fixingEndDate = etrading::LADateHelpers::getDate(floatFixingDates[i-1], accessary_forecast, sld_forecast, &cal_forecast, true);
				double fixingFraction = floatLegDaycount.getTerm( floatFixingDates[i-1], fixingEndDate );
					    			
	    		// Apply the first fixing rate for the first stub, if needed
	    		if ( i == nextCashflowIndex && useFirstFixing )
	    		{
	    			floatLegRates[i-1] = firstFixing;
	    		}
	    		// Apply the last fixing rate for the last stub, if needed
	    		else if ( i == floatAccrualDates.size() - 1 && useLastFixing )
	    		{
	    			floatLegRates[i-1] = lastFixing;
	    		}
	    		else
	    		{					
					floatLegRates[i-1] = (i<nextCashflowIndex) ? 0.0 :
	    				( yc.getDF( data_dc_act365.getTerm( asOf, floatFixingDates[i-1] ), &dc_act365 ) 
	    				/ yc.getDF( data_dc_act365.getTerm( asOf, fixingEndDate ), &dc_act365 ) - 1.) / fixingFraction;				
	    		}
	    	}
	    }
	}

    // Get Discount Factors
	yc.setCurveNameAndInterpolation(dfCurveName, dfInter);
    for( size_t i = 1; i < fixedPaymentDates.size(); i++ )
	{
        // *** FIXED LEG - Calculate discount factors using PAYMENT DATES ***
        // Note: All the date vectors have an extra date for the start date, so dates index using i and other parameters i-1
	    fixedLegDiscFactors[i-1] = (i<nextCashflowIndex) ? 0.0 : yc.getDF( data_dc_act365.getTerm( asOf, fixedPaymentDates[i] ), &dc_act365 );
	}
        
    for( size_t j = 1; j < floatPaymentDates.size(); j++ )
	{
        // *** FLOAT LEG Calculate discount factors using PAYMENT DATES ***
        // Note: All the date vectors have an extra date for the start date, so dates index using j and other parameters j-1
	    floatLegDiscFactors[j-1] = (j<nextCashflowIndex) ? 0.0 : yc.getDF( data_dc_act365.getTerm( asOf, floatPaymentDates[j] ), &dc_act365 );
	}

    //
    // 4. PV and Duration Parameters
    // -------------------------------------------------------------------------------
    //

    double fixedLegTimeWeightedPVs                  = 0.0;
    double floatLegTimeWeightedPVs                  = 0.0;
    double floatLegExcludingSpreadTimeWeightedPVs   = 0.0;
    double spreadTimeWeightedPVs                    = 0.0;

    //
    // Fixed Leg
    // Note:    All the date vectors have an extra date for the start date, so dates index using i and other parameters i-1
    //
	double fixedLegPV = 0.0;
    for( size_t i = 1; i < fixedAccrualDates.size(); i++ ) 
    {
        double fixedAccrualPeriod               = 0.0;
        double fixedDiscFactor                  = 0.0;
        double timeToFixedCouponPayment         = 0.0;

        if ( fixedPaymentDates[i] >= asOf )
        {
            fixedAccrualPeriod                  = fixedLegAccrualDateYearFractions[i-1];
            fixedDiscFactor                     = fixedLegDiscFactors[i-1];
            timeToFixedCouponPayment            = data_dc_act365.getTerm( asOf, fixedPaymentDates[i] );
        
            double fixedCashflowPV              = notional * fixedRate * fixedAccrualPeriod * fixedDiscFactor;
            fixedLegPV                         += fixedCashflowPV;
            fixedLegTimeWeightedPVs             += fixedCashflowPV * timeToFixedCouponPayment;
        }
    }


    //
    // Float Leg 
    // Note:    All the date vectors have an extra date for the start date, so dates index using j and other parameters j-1
    //          For the Fixing Dates however we need to use j-1 as an index to capture the start date fixing
    //
	double spreadPV = 0.0;
	double floatLegPV = 0.0;
	double floatLegPVExcludingSpread = 0.0;
    for( size_t j = 1; j < floatAccrualDates.size(); j++ )
	{			
	    if ( floatPaymentDates[j] >= asOf )
        {
            double floatAccrualPeriod           = floatLegAccrualDateYearFractions[j-1];
            double floatDiscFactor              = floatLegDiscFactors[j-1];
            double timeToFloatCouponPayment     = data_dc_act365.getTerm( asOf, floatPaymentDates[j] );
            
            double floatCashflowPV              = notional * floatLegRates[j-1]   * floatLegAccrualDateYearFractions[j-1] * floatDiscFactor;
            double spreadCashflowPV             = notional * floatSpreadInPercent * floatLegAccrualDateYearFractions[j-1] * floatDiscFactor;

            floatLegPV                         += ( floatCashflowPV + spreadCashflowPV );
            floatLegPVExcludingSpread          += floatCashflowPV;
            spreadPV                           += spreadCashflowPV;
        
            floatLegTimeWeightedPVs                 += floatCashflowPV  * timeToFloatCouponPayment;
            floatLegExcludingSpreadTimeWeightedPVs  += floatCashflowPV  * timeToFloatCouponPayment;
            spreadTimeWeightedPVs                   += spreadCashflowPV * timeToFloatCouponPayment;
        }
	}
	
    //
    // 5. Calculate Swap Par Rates
    // -------------------------------------------------------------------------------
    //
    
    double swapParRateMarket                          = ( notional * AnnuityFixed == 0.0 ) ? 0.0 : floatLegPVExcludingSpread / AnnuityFixed;

    //
    // 6. Calculate Macaulay's & Modified Duration Values ( by leg )
    // -------------------------------------------------------------------------------
    //
    
    // Note: Macaulay's Duration Swap excluding Spread is used to evaluate the swap duration which is being priced as
    // Swap PV = Notional * [ ( Fixed Rate - Par Rate ) * Annuity(Fixed) - Spread * Annuity(Float) ]
    //
    // There Macaulay's Duration Swap must evaluate the time adjusted coupons from the fixed leg and float leg (excluding spread)
    // since the floating leg is considered a fixed leg with it's fixed rate being the swap par rate.
    //

    double macaulaysDurationSwapExcludingSpread       = ( fixedLegPV - floatLegExcludingSpreadTimeWeightedPVs == 0.0 ) ? 0.0 
        : ( fixedLegTimeWeightedPVs - floatLegExcludingSpreadTimeWeightedPVs ) / ( fixedLegPV - floatLegPVExcludingSpread );
    
    double modifiedDurationSwapExcludingSpread = ( 1.0 + swapParRateMarket == 0.0 ) ? 0.0 : macaulaysDurationSwapExcludingSpread  / ( 1.0 + swapParRateMarket );
    
    if ( modifiedDurationSwapExcludingSpread < 0 )
    {
        modifiedDurationSwapExcludingSpread *= -1.0;
    }

    double macaulaysDurationSpread = ( spreadPV == 0.0 ) ? 0.0 : spreadTimeWeightedPVs / spreadPV;
    double modifiedDurationSpread  = ( 1.0 + swapParRateMarket == 0.0 ) ? 0.0 : macaulaysDurationSpread / ( 1.0 + swapParRateMarket );

    if ( modifiedDurationSpread < 0 )
    {
        modifiedDurationSpread *= -1.0;
    }

    //
    // 7. Calculate PV & Risk Totals
    // -------------------------------------------------------------------------------
    //
    
    // payRecIndicator is an indicator function, taking a value of 1 for a receiver swap and -1 for a payer swap
    // i.e. payRecIndicator = 1 when receiving fixed coupons and -1 when paying fixed coupons
    double payRecIndicator = isFixedRatePayerSwap ? -1.0 : 1.0;
    
    double swapPVExcludingSpread = payRecIndicator * ( fixedLegPV - floatLegPVExcludingSpread );
    spreadPV = payRecIndicator * -1.0 * spreadPV;
    double swapPV01 = AnnuityFixed * oneBasisPoint;

    double ret = payRecIndicator * swapPV01 + ( swapPVExcludingSpread * modifiedDurationSwapExcludingSpread * oneBasisPoint )
                                      + ( spreadPV * modifiedDurationSpread * oneBasisPoint );

	return ret;
}


double CurveInstrumentPricing::getAssetSwapSpread( const double&    bondPrice,
                                                   DateVector&      fixedAccrualDates,
                                                   DateVector&      fixedPaymentDates,
                                                   DateVector&      floatFixingDates,
                                                   DateVector&      floatAccrualDates,
                                                   DateVector&      floatPaymentDates,
                                                   LADataInstance*          dataInstance,
                                                   const LAString&  curveid,
                                                   double&          fixedRate,
                                                   LAString         fixedDaycount,
                                                   LAString         floatDaycount,
                                                   LAString         interpolation,
                                                   LAString         foreCurveName,
                                                   LAString         dfCurveName,
                                                   bool             isFWDInter,
                                                   bool             useFirstFixing,
                                                   double           firstFixing,
                                                   bool             useLastFixing,
                                                   double           lastFixing, 
                                                   bool             isCleanPrice,
                                                   const LADate&    settlementDate )
{
    upper( fixedDaycount );
    upper( floatDaycount );
	upper( interpolation );

	const LADataDate& atr = dynamic_cast<const LADataDate& >(dataInstance->getObjectPool().getObject(curveid,ENCHKTYPE_ISDEFINED).get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get());
	const LADate asofdate = atr.get();
	
    //
    // Validate Fixed Leg Accrual & Payment Dates
    //
	int nFixedAccrualDates = fixedAccrualDates.size()-1;
	for( int i = 1; i < nFixedAccrualDates; i++ )
	{
		if( fixedAccrualDates[i-1] >= fixedAccrualDates[i] )
            throw LACoreInvalidData("#Error: Invalid Fixed Leg Accrual Date(s).",__FILE__,__LINE__);
	}

    int nFixedPaymentDates = fixedPaymentDates.size()-1;
	for( int i = 1; i < nFixedPaymentDates; i++ )
	{
		if( fixedPaymentDates[i-1] >= fixedPaymentDates[i] )
            throw LACoreInvalidData("#Error: Invalid Fixed Leg Payment Date(s).",__FILE__,__LINE__);
	}

    //
    // Validate Floating Leg Accrual, Fixing & Payment Dates
    //
    int nFloatAccrualDates = floatAccrualDates.size()-1;
	for( int i = 1; i < nFloatAccrualDates; i++ )
	{
		if( floatAccrualDates[i-1] >= floatAccrualDates[i] )
			throw LACoreInvalidData("#Error: Invalid Float Leg Accrual Date(s).",__FILE__,__LINE__);
	}

    int nFloatFixingDates = floatFixingDates.size()-1;
	for( int i = 1; i < nFloatFixingDates; i++ )
	{
		if( floatFixingDates[i-1] >= floatFixingDates[i] )
			throw LACoreInvalidData("#Error: Invalid Float Leg Fixing Date(s).",__FILE__,__LINE__);
	}

    int nFloatPaymentDates = floatPaymentDates.size()-1;
	for( int i = 1; i < nFloatPaymentDates; i++ )
	{
		if( floatPaymentDates[i-1] >= floatPaymentDates[i] )
			throw LACoreInvalidData("#Error: Invalid Float Leg Payment Date(s).",__FILE__,__LINE__);
	}

    etrading::LACurvePricingObject& yc = etrading::LACurveForwardRateHelpers::getYieldCurveForCurveID(dataInstance,curveid);
	LAString foreInter = LACoreComponentManager::getInterpolation(interpolation);

	LAString dfCurve = etrading::getCurveStaticDataTableName( curveid, dfCurveName );
	LAString dfInter = LACoreComponentManager::getInterpolation(etrading::getCurveInterpolation(curveid, dfCurve).toUpper());

	LAString fixeddc = LACoreComponentManager::getDayCount( fixedDaycount );
    LAString floatdc = LACoreComponentManager::getDayCount( floatDaycount );
	    
    yc.getDayCount().setDayCount( floatdc ); // floatdc
	
    etrading::LACurveForwardRateHelpers::setCalendarForCurveID( yc, "" );
	yc.getSlidingRule().convertFromString( NO_CH );

	if (isFWDInter && !etrading::LACurveForwardRateHelpers::setUpForwardDayCount(dataInstance, curveid, foreCurveName, yc))
	{
		throw LACoreInvalidData("#Error: Invalid Forward Rates - Unable to generate forward rates for Curve Instrument Calibration & Pricing",__FILE__,__LINE__);
	}
	
	const LAObject& YieldData       = yc.getYieldData().get().get();
	const LADate& asOf              = dynamic_cast<const LADataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();
    
    if ( floatPaymentDates[floatPaymentDates.size()-1] < asOf && fixedPaymentDates[fixedPaymentDates.size()-1] < asOf) 
        throw LACoreInvalidData("#Error: Asset swap spread error; the underlying swap has expired.", __FILE__, __LINE__ );
    
    const LAPriceDataCalendar& cal       = yc.getCalendar();
	const LAPriceDataSlidingRule& sr     = yc.getSlidingRule();

	LADate fixedAccrualStartDate    = sr.getDate( fixedAccrualDates.front(), cal );
    LADate floatAccrualStartDate    = sr.getDate( floatAccrualDates.front(), cal );
    
    LADate fixedAccrualEndDate      = sr.getDate( fixedAccrualDates.back(), cal );
    LADate floatAccrualEndDate      = sr.getDate( floatAccrualDates.back(), cal );

	DayCount dc_act365( ACT_365 );
	LAPriceDataDayCount data_dc_act365( dc_act365 );

    // Get client specified daycount conventions
    LAPriceDataDayCount fixedLegDaycount;
    fixedLegDaycount.convertFromString( fixedDaycount );
        
    LAPriceDataDayCount floatLegDaycount;
    floatLegDaycount.convertFromString( floatDaycount );

	yc.setCurveNameAndInterpolation(dfCurveName, dfInter);
	
    //
    // Calculate Fixed Leg Annuity value using client specified daycount convention
    // Note: We deduct accrued interest when working with a dirty bond price
    //

    // Asset Swap Calculations and Accrued Interest
    // --------------------------------------------
    // When working with the dirty bond price we need to deduct accrued interest from the Bond fixed leg
	const bool deductAccruedInterest = true;
    const double fixedAnnuity = yc.getAnnuity( fixedAccrualDates, fixedPaymentDates, fixedLegDaycount, settlementDate, deductAccruedInterest, isCleanPrice );
	
    //
    // Calculate Floating Leg Values
    //
	LAString            freq_forecast       = ""; 
	LAPriceDataCalendar      cal_forecast; 
	LAPriceDataSlidingRule   sld_forecast; 
	LAPriceDataDayCount      dc_forecast;
	LAString            accessary_forecast  = "";
		
	yc.getCurveConvention( freq_forecast, cal_forecast, sld_forecast, dc_forecast, accessary_forecast, foreCurveName );

    DoubleArray rates( floatFixingDates.size()  -1 );
    DoubleArray dfs  ( floatPaymentDates.size() -1 );
        
    DoubleArray fixingDateYearFractions( floatFixingDates.size() -1 );  
	DoubleArray accrualDateYearFractions( floatAccrualDates.size() -1 );  
    DoubleArray payDateYearFractions( floatPaymentDates.size() -1 );  

    if ( floatFixingDates.size() != floatAccrualDates.size() || floatAccrualDates.size() != floatPaymentDates.size() )
        throw LACoreInvalidData("#Error: Floating schedule error. Inconsistent number of fixing, acrrual and payment dates.", __FILE__, __LINE__ );

    // Calculate the floating coupon accrual periods
    for( size_t i = 1; i < floatAccrualDates.size(); i++ )
    {
        // "accrualDateYearFractions" used for the coupon period
        accrualDateYearFractions[i-1]           = floatLegDaycount.getTerm( floatAccrualDates[i-1], floatAccrualDates[i] );
    }

    // Find the index for the next cashflow, so that we can ignore cashflows in the past
    unsigned int nextCashflowIndex = 1;
    for (unsigned int i = 0; i < floatAccrualDates.size(); ++i)
    {
        if ( i == floatAccrualDates.size()-1  )
        {
            if ( floatAccrualDates[i] < asOf ) 
                throw LACoreInvalidData("#Error: Asset swap spread error; the underlying swap has expired.", __FILE__, __LINE__ );
			
			nextCashflowIndex++;                
            break; 
        }
        if ( floatAccrualDates[i+1] >= asOf ) break;
        nextCashflowIndex++;
    }

    // Get forward rates from interpolation
	yc.setCurveNameAndInterpolation(foreCurveName, foreInter);
	if ( isFWDInter )
	{
		const LAInterpolationBase &fwd_inter = yc.getFWDInterpolation(&foreCurveName);
		for( size_t i = 1; i < floatAccrualDates.size(); i++ )
		{
			const double fixingDateYearFraction     = data_dc_act365.getTerm( asOf, floatFixingDates[i-1] ); 
                
            // Apply the first fixing rate for the first stub, if needed
            if ( i == nextCashflowIndex && useFirstFixing )
            {
                rates[i-1] = firstFixing;
            }
            // Apply the last fixing rate for the last stub, if needed
            else if ( i == floatAccrualDates.size() - 1 && useLastFixing )
            {
                rates[i-1] = lastFixing;
            }
            else
            {
                // *** Calculate rates using FIXING DATES ***
                rates[i-1] = (i<nextCashflowIndex) ? 0.0 : fwd_inter.value( fixingDateYearFraction );
            }
		}
	}
    // Get forward rates implied from discount factors
	else
	{
		for( size_t i = 1; i < floatPaymentDates.size(); i++ )
		{			
			LADate fixingEndDate = etrading::LADateHelpers::getDate(floatFixingDates[i-1], accessary_forecast, sld_forecast, &cal_forecast, true);
			double fixingFraction = floatLegDaycount.getTerm( floatFixingDates[i-1], fixingEndDate );

            // Apply the first fixing rate for the first stub, if needed
            if ( i == nextCashflowIndex && useFirstFixing )
            {
                rates[i-1] = firstFixing;
            }
            // Apply the last fixing rate for the last stub, if needed
            else if ( i == floatAccrualDates.size() - 1 && useLastFixing )
            {
                rates[i-1] = lastFixing;
            }
            else
            {
                // *** Calculate rates using FIXING DATES ***
				rates[i-1]  = (i<nextCashflowIndex) ? 0.0 : 
                    ( yc.getDF( data_dc_act365.getTerm( asOf, floatFixingDates[i-1] ), &dc_act365 ) 
                    / yc.getDF( data_dc_act365.getTerm( asOf, fixingEndDate ), &dc_act365 ) - 1.) / fixingFraction;			
			}
		}
	}
		
    // Get Discount Factors
	yc.setCurveNameAndInterpolation(dfCurveName, dfInter);
	for( size_t i = 1; i < floatPaymentDates.size(); i++ )
	{
        // *** Calculate discount factors using PAYMENT DATES ***
		dfs[i-1] = (i<nextCashflowIndex) ? 0.0 : yc.getDF( data_dc_act365.getTerm( asOf, floatPaymentDates[i] ), &dc_act365 );
	}

    //
    // Asset Swap Spread Parameter Specification
    //
	double          swapSpread                  = 0.0;
    double          fixedLegPV                  = 0.0;
    double          floatLegPV                  = 0.0;
    double          parParAdjustmentInPercent   = 0.0;
    double          floatAnnuity                = 0.0;
    const double    oneBasisPoint               = 0.0001;

    //
    // Fixed Leg PV
    //
    fixedLegPV                                  = fixedRate * fixedAnnuity;

    //
    // Float Leg PV
    //
    for( size_t i = 1; i < floatAccrualDates.size(); i++ )
	{			
		double floatRate                        = rates[i-1];
        double floatAccrualPeriod               = accrualDateYearFractions[i-1];
        double floatDiscFactor                  = dfs[i-1];
        double floatAccruedInterest             = 0.0;

        floatLegPV                              += floatRate * floatAccrualPeriod * floatDiscFactor;
        floatAnnuity                            += floatAccrualPeriod * floatDiscFactor;
	}

    //
    // Par Par Adjustment
    //
    const double Par                            = 100.0;
    parParAdjustmentInPercent                   = ( Par - bondPrice ) / 100.0;

    //
    // Asset Swap Spread
    //
    if ( floatAnnuity == 0 )
        throw LACoreInvalidData("#Error: Floating schedule error. The float leg annuity value cannot be zero.", __FILE__, __LINE__ );

    // Note: Accrued Interest is deducted from the fixedLegPV ( via the Fixed Annuity ) when working with the dirty bond price
    swapSpread    = ( fixedLegPV - floatLegPV + parParAdjustmentInPercent ) / floatAnnuity;
    
	double ret = swapSpread / oneBasisPoint;

    return ret;
}

// get StubRate
StubRateAndFixingDate CurveInstrumentPricing::getStubRate( const DateVector& fixingDates,
														   const LAStringVector& curveNames,
														   const LAStringVector& curveTenors,
														   const DoubleVector& tenorCurveFixings,
														   const LAString& curveid,									
														   const LAString& stubType,
														   const LAString& interpolation,
														   const LAString& dateCount,
														   const LAString& calendar,
														   const LAString& busDayAdj,
														   const LAString& rollConvention,
														   bool  useNearbyCurve,
														   bool  isFwdInter,
														   bool  useFwdData,
														   const LAString& toleranceTenor,
														   const LAString& useCurveName,
														   const LAString& indexFrequency,
														   bool isRegularSwapSchedule)
{

	DateVector fixingStartDates;
	DateVector fixingEndDates;

	auto stubTypeEnum = toStubTypeEnum(stubType.getCString());

	bool isFrontStub = (stubTypeEnum == NONE_STUBTYPE || stubTypeEnum == SHORT_START_STUBTYPE || stubTypeEnum == LONG_START_STUBTYPE);

	etrading::populateFixingStartEndDates(fixingStartDates, fixingEndDates, fixingDates, toFrequencyEnum(indexFrequency.getCString()), busDayAdj, calendar, rollConvention, isRegularSwapSchedule, isFrontStub);

   StubRateAndFixingDate stubRateAndFixingDate;

    // 1. Stub Data
    // ------------------------------------------------------------------------

    // DataInstance access required for this function
    LADataInstance*	dataInstance = etrading::InitializeMLibETrading::instance().dataInstance();

    LAString INTERPOLATION( interpolation );
	upper( INTERPOLATION );

	LAString DATECOUNT( dateCount );
    upper( DATECOUNT );

	LAString CALENDAR( calendar );
    upper( CALENDAR );

    LAString BUSDAYADJ( busDayAdj );
	upper( BUSDAYADJ );

    //
    // Validate inputs
    //
	// Bounds Check
	AQ_REQUIRE(fixingStartDates.size() == fixingEndDates.size(), "Stub Rate Error: The numbers of Fixing Date(s) and Fixing End Date(s) not matched.")

	size_t nFixingDates = fixingStartDates.size();
	for( size_t i = 0; i < nFixingDates; i++ )
	{
		//For libor in arrear the last dummy fixing date (fixingDates[i]) (which is the maturity date) can be the same as the last fixing date (fixingDate[i-1])
		if(fixingStartDates[i] > fixingEndDates[i] )
		{
            AQ_THROW("Stub Rate Error: Invalid Fixing Date(s)")
		}
	}

    size_t curveCount = curveNames.size();
	size_t nCurveNames = curveNames.size();
	size_t nCurveTenors = curveTenors.size();
	size_t nFixings = tenorCurveFixings.size();
	
    AQ_REQUIRE( curveCount > 1, "Stub Rate Error - At least two curves are required to interpolate for stub rates." )

    if (nCurveNames != nCurveTenors)
	{
        AQ_THROW("Stub Rate Error: The number of curve index names must equal the number of curve fixing values")
	}

	if (nFixings != 0 && nFixings != nCurveNames)
	{
        AQ_THROW("Stub Rate Error: If past fixings are provided, they must be provided for each curve name")
	}

	bool useGivenFixings = nFixings == 0 ? false : true;

	// Retrieve yield curve through curveid
    etrading::LACurvePricingObject& yc = etrading::LACurveForwardRateHelpers::getYieldCurveForCurveID(dataInstance,curveid);
	
	LAString dc = LACoreComponentManager::getDayCount( DATECOUNT );
    yc.getDayCount().setDayCount( dc ); 
	
    etrading::LACurveForwardRateHelpers::setCalendarForCurveID( yc, "" );
	yc.getSlidingRule().convertFromString( NO_CH );
	    	
	// 2. Stub Calculation
    // ------------------------------------------------------------------------

	// Get yield curve AsOf date
	const LAObject& YieldData       = yc.getYieldData().get().get();
	const LADate& asOf              = dynamic_cast<const LADataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();
 
	double stubRate(0.);
	   
    DayCount dc_act_2( ACT_365 );			
	LAPriceDataDayCount dc_act( dc_act_2 );

	LAPriceDataSlidingRule sr;
	sr.convertFromString(BUSDAYADJ);

	LAPriceDataCalendar cal;
	cal.convertFromString(CALENDAR);
		
	LAPriceDataDayCount floatingLegDateCount;
	floatingLegDateCount.convertFromString(DATECOUNT);	

    // Bounds Check
    AQ_REQUIRE(fixingStartDates.size() >= 1, "Stub Rate Error: Unable to calculate the swap stub rate. Invalid fixing dates" )

	// Determine the exact stub period. Must guaranteee that stub end date is after the asOf date.
	LADate stubStart;
	LADate stubEnd;	
	if (isFrontStub)
	{
        stubStart   = fixingStartDates[0];
		stubEnd     = fixingEndDates[0];
        
        // We can't calculate historic stubs without a fixing table, since such stubs have already been fixed.
        if (fixingStartDates[0] < asOf && nFixings == 0 )
	    {
            // Throw an error if the stub rate is in the past, but paying in the future. Such a front stub needs to be set using the 'FirstFixing' parameter
            if ( stubEnd >= asOf )
            {
                AQ_THROW( "Historical Stub Rate Error: Fixing Data Required" );
            }

            // Set the Stub to Zero if it is in the past and the payment date is also in the past
		    stubRate = 0.0;

			// Return Stub Rate and Fixing Date
			stubRateAndFixingDate.stubRate_ = stubRate;
			stubRateAndFixingDate.fixingDate_ = stubStart;
		    return stubRateAndFixingDate;
	    }

		// Skip all expired coupons and get to the first valid cash flow
		size_t nextCashflowIndex = getNextCashFlowIndex(fixingStartDates, fixingEndDates, asOf);
			
		for (size_t i=0; i< fixingStartDates.size(); ++i)
		{
			if (i == nextCashflowIndex)
			{
				stubStart = fixingStartDates[i];
				stubEnd = fixingEndDates[i];
                break;
			}
		}		
	}
	else if (stubTypeEnum == SHORT_END_STUBTYPE || stubTypeEnum == LONG_END_STUBTYPE)
	{
		auto lastFixingIndex = nFixingDates - 1;
		if (fixingEndDates[lastFixingIndex] < asOf)
		{
			// Swap expired. 
			stubRate = 0.;
			
			// Return Stub Rate and Fixing Date
			stubRateAndFixingDate.stubRate_ = stubRate;
			stubRateAndFixingDate.fixingDate_ = stubStart;
		    return stubRateAndFixingDate;
		}

		stubStart = fixingStartDates[lastFixingIndex];
		stubEnd = fixingEndDates[lastFixingIndex];
	}
	else
	{
		AQ_THROW("Invalid stub type. The Stub Type must be None, ShortStart (SS), LongStart (LS), ShortEnd (SE) or LongEnd (LE)");
	}

    // Leave it for reference only, this logic is covered in populateFixingStartEndDates()
	//if (isRegularSwapSchedule)
	//{
	//	// When swap schedule is regular it means there is no stub period
	//	LAString freqTenor = etrading::fromFrequencyToTerm(indexFrequency);
	//	stubEnd = etrading::LADateHelpers::getDate(stubStart, freqTenor, sr, &cal, true, NULL);
	//}

	// 2. Stub Calculation
	// ------------------------------------------------------------------------

	stubRate = getStubRateFromFixingStartEnd(stubStart, stubEnd, curveNames, curveTenors, tenorCurveFixings, curveid, dateCount, calendar, busDayAdj, useNearbyCurve, isFwdInter, useFwdData, toleranceTenor, useCurveName);

	// Return Stub Rate and Fixing Date
	stubRateAndFixingDate.stubRate_ = stubRate;
	stubRateAndFixingDate.fixingDate_ = stubStart;
	return stubRateAndFixingDate;
}

//Get Stub rate given fixingStart and fixingEnd dates
double CurveInstrumentPricing::getStubRateFromFixingStartEnd( const LADate& fixingDate,
									                     const LADate& fixingEndDate,
									                     const LAStringVector& curveNames,
									                     const LAStringVector& curveTenors,
									                     const DoubleVector& tenorCurveFixings,
									                     const LAString& curveid,
									                     const LAString& dateCount,
									                     const LAString& calendar,
									                     const LAString& busDayAdj,
									                     bool  useNearbyCurve,
									                     bool  isFwdInter,
									                     bool  useFwdData,
									                     const LAString& toleranceTenor,
									                     const LAString& useCurveName)
{

	// 1. Stub Data
	// ------------------------------------------------------------------------
	LAString DATECOUNT(dateCount);
	upper(DATECOUNT);

	LAString CALENDAR(calendar);
	upper(CALENDAR);

	LAString BUSDAYADJ(busDayAdj);
	upper(BUSDAYADJ);

	//
	// Validate inputs
	//
	size_t curveCount = curveNames.size();
	size_t nCurveNames = curveNames.size();
	size_t nCurveTenors = curveTenors.size();
	size_t nFixings = tenorCurveFixings.size();

	AQ_REQUIRE(curveCount > 1, "Stub Rate Error - At least two curves are required to interpolate for stub rates.")
	
	if (nCurveNames != nCurveTenors)
	{
		AQ_THROW("Stub Rate Error: The number of curve names must equal the number of curve tenors")
	}

	if (nFixings != 0 && nFixings != nCurveNames)
	{
		AQ_THROW("Stub Rate Error: If past fixings are provided, they must be provided for each curve name")
	}

	const bool isFixingDataProvided = nFixings == 0 ? false : true;

	// Default Tolerance Tenor is zero days if not provided
	const LAString stubToleranceTenor = (toleranceTenor.size() == 0) ? "0D" : toleranceTenor;

	// Retrieve yield curve through curveid
	LADataInstance*	dataInstance = etrading::InitializeMLibETrading::instance().dataInstance();
	LACurvePricingObject& yc = LACurveForwardRateHelpers::getYieldCurveForCurveID(dataInstance, curveid);
	const LAObject& YieldData = yc.getYieldData().get().get();

	// Get yield curve AsOf date
	const LADate& asOf = dynamic_cast<const LADataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();

	double stubRate(0.0);

	// We can't calculate historic stubs unless we have a fixing table(s), since such stubs have already been fixed.
	if ( fixingDate < asOf && !isFixingDataProvided )
	{
		// Throw an error if the stub rate is in the past, but paying in the future. Such a front stub needs to be set using the 'FirstFixing' parameter
		if (fixingEndDate > asOf)
		{
			AQ_THROW("Historical Stub Rate Error: Fixing Data Required");
		}

		// Set the Stub to Zero if it is in the past and the payment date is also in the past
		return stubRate;
	}

	if (fixingEndDate < asOf)
	{
		// Swap expired. 
		return stubRate;
	}

	DayCount dc_act_2(ACT_365_ISDA);
	LAPriceDataDayCount dc_act(dc_act_2);

	LAPriceDataSlidingRule sr;
	sr.convertFromString(BUSDAYADJ);

	LAPriceDataCalendar cal;
	cal.convertFromString(CALENDAR);

	LAPriceDataDayCount floatingLegDateCount;
	floatingLegDateCount.convertFromString(DATECOUNT);

	LAString dc = LACoreComponentManager::getDayCount(DATECOUNT);
	yc.getDayCount().setDayCount(dc);

	LACurveForwardRateHelpers::setCalendarForCurveID(yc, "");
	yc.getSlidingRule().convertFromString(NO_CH);


	// 2. Stub Calculation
	// ------------------------------------------------------------------------

	LADate stubStart = fixingDate;
	LADate stubEnd = fixingEndDate;

	// Calculate stub rate - either (1) use a given curve, (2.a) pick the nearest curve, or (2.b) interpolate between adjacent curves
	if (useCurveName.size() != 0 && ! boost::iequals(useCurveName.getCString(), "NATURAL"))
	{
		// (1) Use a user-specified yield curve

		// First check if a fixing has been provided to this curve name
		size_t idx = -1;
		for(size_t i=0; i<curveNames.size(); ++i)
		{
			if (boost::iequals(curveNames[i].getCString(), useCurveName.getCString()))
			{
				idx = i;
				break;
			}
		}

		if (idx == -1)
		{
            AQ_THROW("Stub Rtae Error: The 'useCurveName' column header should also be part of the curveNames list")
		}

		if (isFixingDataProvided)
		{
			// If the fixing for the designated curve has been given, use it
			stubRate = tenorCurveFixings[idx];
		}
		else
		{
			// When we use a user-specified curve, we should use the corresponding curve tenor to calculate the
			// stub end date instead of using the old stub end date
			stubEnd = etrading::LADateHelpers::getDate(stubStart, curveTenors[idx], sr, &cal, true, NULL);

			// Set the right curve to use

            // Set the Curve Type Name and Curve Specific Interpolation                                            
            LAString tempIndexCurve			 = useCurveName;
			LAString tempIndexCurve_mainName = etrading::getCurveStaticDataTableName( curveid, tempIndexCurve );
	        LAString tempIndexInter			 = LACoreComponentManager::getInterpolation( etrading::getCurveInterpolation( curveid,  tempIndexCurve_mainName).toUpper() );
                        
			bool isSwapCurve = true;
			if (isFwdInter)
			{				
				LAString suffix = tempIndexCurve_mainName == "STD" ? "" : LAString("_") + tempIndexCurve_mainName;
				const LADataHolder* dh = &YieldData.getData(CALIBRATION_DATA_CURVETYPE + suffix);
				if ( dh->isDefined() && !dh->isNull() )
				{
					const LAString curveType = dynamic_cast<const LADataString&>( dh->get() ).get();
					if (curveType != SWAP)
					{
						isSwapCurve = false;
					}
				}
			}

			yc.setCurveNameAndInterpolation( useCurveName, tempIndexInter );

			if (isFwdInter && isSwapCurve)
			{
				LAPriceDataDayCount dc;
				if (!etrading::LACurveForwardRateHelpers::setUpForwardDayCount(dataInstance, curveid, useCurveName, dc))
				{
					AQ_THROW( "Stub Rate Error - The STD Swap Curve is invalid and has no forward rates." )
				}
				else
				{
					yc.getDayCount(useCurveName) = dc;
				}

				const LAInterpolationBase &fwd_inter = yc.getFWDInterpolation(&useCurveName, useFwdData);
				const double yearFraction = dc_act.getTerm( asOf, stubStart); 
				stubRate = fwd_inter.value( yearFraction );
			}
			else
			{				
				if (stubStart >= asOf)
				{
					const double lowerAccrualFraction = floatingLegDateCount.getTerm(stubStart, stubEnd);
					stubRate = ( yc.getDF( dc_act.getTerm( asOf, stubStart ), &dc_act_2 ) 
							 / yc.getDF( dc_act.getTerm( asOf, stubEnd ), &dc_act_2 ) - 1.) / lowerAccrualFraction;
				}
				else
				{
					AQ_THROW( "Stub Rate Error - Back dated swaps are not supported when when interpolating on forward rates i.e. when isFwdInter is set to FALSE" )
				}
			}
		}
	}
	else if (useCurveName.size() == 0 || boost::iequals(useCurveName.getCString(), "NATURAL"))
	{
		// (2) Either pick the nearest curve, or interpolate stub rate from nearby standard forward rates		
		
		size_t lowerCurveIndex = 0;
		size_t upperCurveIndex = 0;
		LADate lowerCurveDate;
		LADate upperCurveDate;
        
		// Pick the curves to interpolate from
		LADate firstTenorDate = etrading::LADateHelpers::getDate(stubStart, curveTenors.at(0), sr, &cal, true, NULL);
		LADate lastTenorDate = etrading::LADateHelpers::getDate(stubStart, curveTenors.at(curveTenors.size() - 1), sr, &cal, true, NULL);
		if (stubEnd == firstTenorDate)
		{
			lowerCurveIndex = 0;
			lowerCurveDate = firstTenorDate;

			upperCurveIndex = lowerCurveIndex;
			upperCurveDate = lowerCurveDate;
		}
		else if (stubEnd == lastTenorDate)
		{
			upperCurveIndex = curveCount - 1;
			upperCurveDate = lastTenorDate;

			lowerCurveIndex = upperCurveIndex;
			lowerCurveDate = upperCurveDate;
		}
		else
		{
            // Important: Above we enforce that CurveCount is always greater than 1 !!
			for(size_t i = 0; i < curveCount; ++i)
			{
				LAString curveTenor = curveTenors.at(i);
				LADate tenorEnd = etrading::LADateHelpers::getDate(stubStart, curveTenor, sr, &cal, true, NULL);

                // a) Lower Curve Boundary Condition
                if ( i == 0 )
                {
                    lowerCurveIndex = i;
					lowerCurveDate = tenorEnd;

                    LADate stubEndWithTolerance = etrading::LADateHelpers::getDate(stubEnd, stubToleranceTenor, sr, &cal, true  /*add tolerance tenor*/ , NULL);
                    AQ_REQUIRE( lowerCurveDate <= stubEndWithTolerance , "Stub Rate Extrapolation Error - Stub term is shorter than '" + curveTenor + "'. A curve with tenor less than '" + curveTenor + "' is required when setting StubIndex to 'NATURAL'." )
                }
                // b) Select the two Nearest Curves containing the Stub Date
                else if ( i != curveCount-1 )
                {
                    if (tenorEnd < stubEnd)
				    {
					    lowerCurveIndex = i;
					    lowerCurveDate = tenorEnd;
				    }
				    else
				    {
					    upperCurveIndex = i;
					    upperCurveDate = tenorEnd;
                        
                        // *** IMPORTANT ***
                        // We must break here to ensure that the upper index is not reset in boundary condition step c)
                        break;
				    }
                }
                // c) Upper Curve Boundary Condition
                // If an upper index is not already found set the last curve as the upper curve index, note curves are sorted in coupon frequency order
                // Do not set the upper index again if the upper index already identified in step b)
                else
                {
                    upperCurveIndex = i;
					upperCurveDate = tenorEnd;
                    
                    LADate stubEndWithTolerance = etrading::LADateHelpers::getDate(stubEnd, stubToleranceTenor, sr, &cal, false /*subtract tolerance tenor*/, NULL);
                    AQ_REQUIRE( stubEndWithTolerance <= upperCurveDate, "Stub Rate Extrapolation Error - Stub term is larger than '" + curveTenor + "'. A curve with tenor larger than '" + curveTenor + "' is required when setting StubIndex to 'NATURAL'." )
                }
			}		
		}

		
		{		
			// Try to determine if we can pick a nearby curve to use
			bool isUsingNearbyCurve = false;
			size_t nearbyIndex;
			LADate nearbyDate;
			if (useNearbyCurve)
			{
                LADate lowerToleranceDate = etrading::LADateHelpers::getDate(lowerCurveDate, stubToleranceTenor, sr, &cal, true  /*forward add date*/ , NULL);;
				LADate upperToleranceDate = etrading::LADateHelpers::getDate(upperCurveDate, stubToleranceTenor, sr, &cal, false /*backward add date*/, NULL);;
						
				if (lowerToleranceDate >= stubEnd)
				{
					nearbyIndex = lowerCurveIndex;
					nearbyDate = lowerCurveDate;
					isUsingNearbyCurve = true;
				}
				else if (upperToleranceDate <= stubEnd)
				{
					nearbyIndex = upperCurveIndex;
					nearbyDate = upperCurveDate;
					isUsingNearbyCurve = true;
				}
			}

			// Calculate stub rate now
			if (isUsingNearbyCurve)
			{
				// (2.a) Using nearby curve to get a forward rate out as an approximation of stub rate
				if (isFixingDataProvided)
				{
					stubRate = tenorCurveFixings[nearbyIndex];
				}
				else
				{
                    // Set the Curve Type Name and Curve Specific Interpolation                                            
                    LAString nearbyIndexCurve			= curveNames[nearbyIndex];
					LAString nearbyIndexCurve_mainName	= etrading::getCurveStaticDataTableName( curveid, curveNames[nearbyIndex] );
	                LAString nearbyIndexInter			= LACoreComponentManager::getInterpolation( etrading::getCurveInterpolation( curveid,  nearbyIndexCurve_mainName).toUpper() );                    
					
					bool isSwapCurve = true;
					if (isFwdInter)
					{				
						LAString suffix = nearbyIndexCurve_mainName == "STD" ? "" : LAString("_") + nearbyIndexCurve_mainName;
						const LADataHolder* dh = &YieldData.getData(CALIBRATION_DATA_CURVETYPE + suffix);
						if ( dh->isDefined() && !dh->isNull() )
						{
							const LAString curveType = dynamic_cast<const LADataString&>( dh->get() ).get();
							if (curveType != SWAP)
							{
								isSwapCurve = false;
							}
						}
					}

					yc.setCurveNameAndInterpolation( nearbyIndexCurve, nearbyIndexInter );

                    if (isFwdInter && isSwapCurve)
					{				
						LAPriceDataDayCount dc;
						if (!etrading::LACurveForwardRateHelpers::setUpForwardDayCount(dataInstance, curveid, nearbyIndexCurve, dc))
						{
                            AQ_THROW( "Stub Rate Error - The STD Swap Curve is invalid and has no forward rates." )
						}
						else
						{
							yc.getDayCount(nearbyIndexCurve) = dc;
						}

						const LAInterpolationBase &fwd_inter = yc.getFWDInterpolation(&nearbyIndexCurve, useFwdData);
						const double yearFraction = dc_act.getTerm( asOf, nearbyDate); 
						stubRate = fwd_inter.value( yearFraction );
					}
					else
					{
                        AQ_REQUIRE( nearbyDate >= asOf, "Stub Rate Error - Underlying curves are too short. Curve name '" + nearbyIndexCurve + " ' does not have enough calibration instruments to evaluate the stub rate.")
						
						const double accrualFraction = floatingLegDateCount.getTerm(stubStart, nearbyDate);
						stubRate = ( yc.getDF( dc_act.getTerm( asOf, stubStart ), &dc_act_2 ) 
							       / yc.getDF( dc_act.getTerm( asOf, nearbyDate ), &dc_act_2 ) - 1.) / accrualFraction;
						
					}
				}
			}
			else
			{
				// (2.b) Not able to pick a nearby curve for approximation. Go ahead with linear interpolation.
                
				const double lowerFraction = dc_act.getTerm(lowerCurveDate, stubEnd);
				const double upperFraction = dc_act.getTerm(stubEnd, upperCurveDate);
				const double fullTerm = lowerFraction + upperFraction;			

				if (isFixingDataProvided)
				{
                    // No need to interpolate when on a pillar point.     
                    if ( lowerCurveDate == stubEnd  )
                    {
					    // Return Node Value
                        stubRate = tenorCurveFixings[lowerCurveIndex];
                    }
                    else if ( upperCurveDate == stubEnd )
                    {
                        stubRate = tenorCurveFixings[upperCurveIndex];
                    }
                    else
                    {
                        // Linear Interpolation: Note when we are on a pillar we have fullTerm = 0.0 giving a divide by zero
                        stubRate = tenorCurveFixings[lowerCurveIndex] + (tenorCurveFixings[upperCurveIndex] - tenorCurveFixings[lowerCurveIndex]) * (lowerFraction / fullTerm);
                    }
				}
				else
				{
					double lowerTenorRate(0.);
					double upperTenorRate(0.);
			
					// Calculate the lower and upper forward rates from which the target stub rate is to be interpolated 
                    
                    // Set the Curve Type Name and Curve Specific Interpolation
                    LAString lowerCurveIndexCurve			= curveNames[lowerCurveIndex];
					LAString lowerCurveIndexCurve_mainName	= etrading::getCurveStaticDataTableName( curveid, curveNames[lowerCurveIndex] );
	                LAString lowerCurveIndexInter			= LACoreComponentManager::getInterpolation( etrading::getCurveInterpolation( curveid,  lowerCurveIndexCurve_mainName).toUpper() );

					LAString upperCurveIndexCurve			= curveNames[upperCurveIndex];
					LAString upperCurveIndexCurve_mainName	= etrading::getCurveStaticDataTableName( curveid, curveNames[upperCurveIndex] );
	                LAString upperCurveIndexInter			= LACoreComponentManager::getInterpolation( etrading::getCurveInterpolation( curveid,  upperCurveIndexCurve_mainName).toUpper() );                    

					bool isLowerCurveSwapCurve = true;
					bool isUpperCurveSwapCurve  = true;
					if (isFwdInter)
					{						
						LAString lowerSuffix = lowerCurveIndexCurve_mainName == "STD" ? "" : LAString("_") + lowerCurveIndexCurve_mainName;
						const LADataHolder* dh = &YieldData.getData(CALIBRATION_DATA_CURVETYPE + lowerSuffix);
						if ( dh->isDefined() && !dh->isNull() )
						{
							const LAString curveType = dynamic_cast<const LADataString&>( dh->get() ).get();
							if (curveType != SWAP)
							{
								isLowerCurveSwapCurve = false;
							}
						}
						
						LAString upperSuffix = upperCurveIndexCurve_mainName == "STD" ? "" : LAString("_") + upperCurveIndexCurve_mainName;
						dh = &YieldData.getData(CALIBRATION_DATA_CURVETYPE + upperSuffix);
						if ( dh->isDefined() && !dh->isNull() )
						{
							const LAString curveType = dynamic_cast<const LADataString&>( dh->get() ).get();
							if (curveType != SWAP)
							{
								isUpperCurveSwapCurve = false;
							}
						}
					}

					// get lower rate

					// Set the Curve Type Name and Curve Specific Interpolation                       						
					yc.setCurveNameAndInterpolation( lowerCurveIndexCurve, lowerCurveIndexInter );

                    if (isFwdInter && isLowerCurveSwapCurve)
					{												
						LAPriceDataDayCount dc;
						if (!etrading::LACurveForwardRateHelpers::setUpForwardDayCount(dataInstance, curveid, lowerCurveIndexCurve, dc))
						{
							AQ_THROW( "Stub Rate Error - The STD Swap Curve is invalid and has no forward rates." )
						}
						else
						{
							yc.getDayCount(lowerCurveIndexCurve) = dc;
						}					
					
						const LAInterpolationBase &lowerCurve_fwd_inter = yc.getFWDInterpolation(&lowerCurveIndexCurve, useFwdData);
						const double lowerCurveDateYearFraction = dc_act.getTerm( asOf, stubStart ); 
						lowerTenorRate = lowerCurve_fwd_inter.value( lowerCurveDateYearFraction );
					}
					else
					{
						if (stubStart >= asOf)
						{
                            const double lowerAccrualFraction = floatingLegDateCount.getTerm(stubStart, lowerCurveDate);
							lowerTenorRate = ( yc.getDF( dc_act.getTerm( asOf, stubStart ), &dc_act_2 ) 
									         / yc.getDF( dc_act.getTerm( asOf, lowerCurveDate ), &dc_act_2 ) - 1.) / lowerAccrualFraction;
						}
						else
						{
                            AQ_THROW( "Stub Rate Error - Back dated swaps are not supported when when interpolating on forward rates i.e. when isFwdInter is set to FALSE" )
						}
					}


					// get upper rate
					
					// Set the Curve Type Name and Curve Specific Interpolation                       
					yc.setCurveNameAndInterpolation( upperCurveIndexCurve, upperCurveIndexInter );

					if (isFwdInter && isUpperCurveSwapCurve)
					{
						LAPriceDataDayCount dc;
						if (!etrading::LACurveForwardRateHelpers::setUpForwardDayCount(dataInstance, curveid, upperCurveIndexCurve, dc))
						{
							AQ_THROW( "Stub Rate Error - Unable to interpolate the STD swap curve forward rates. The STD Swap Curve is invalid and has no forward rates." )
						}
						else
						{
							yc.getDayCount(upperCurveIndexCurve) = dc;
						}

						const LAInterpolationBase &upperCurve_fwd_inter = yc.getFWDInterpolation(&upperCurveIndexCurve, useFwdData);
						const double upperCurveDateYearFraction = dc_act.getTerm( asOf, stubStart ); 
						upperTenorRate = upperCurve_fwd_inter.value( upperCurveDateYearFraction );
					}
					else
					{
						if (stubStart >= asOf)
						{
                            const double upperAccrualFraction = floatingLegDateCount.getTerm(stubStart, upperCurveDate);
							upperTenorRate = ( yc.getDF( dc_act.getTerm( asOf, stubStart ), &dc_act_2 ) 
									         / yc.getDF( dc_act.getTerm( asOf, upperCurveDate ), &dc_act_2 ) - 1.) / upperAccrualFraction;
						}
						else
						{
							AQ_THROW( "Stub Rate Error - Back dated swaps are not supported when when interpolating on forward rates i.e. when isFwdInter is set to FALSE" )
						}
					}				
			
					// Linear interpolation
					if (fullTerm == 0.0)
					{
						stubRate = lowerTenorRate;
					}
					else
					{
						stubRate = lowerTenorRate + (upperTenorRate - lowerTenorRate) * (lowerFraction / fullTerm);
					}
				}
			}
		}
	}

	return stubRate;
}