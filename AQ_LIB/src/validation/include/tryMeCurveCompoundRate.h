/*
 * @brief			validation interface for meCompoundRate method(s)
 * @Created:		19 April 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "LACoreTemplateType.h"

namespace validation_api
{

    /* @brief			validation interface for the meCurveCompoundRate method
    *  @param [in]		startDates			A vector of start dates
    *  @param [in]		endDates			A vector of end dates
    *  @param [in]		curveCollection		ID of the curve set
    *  @param [in]		forecastCurveIndex	Forecasting curve index
    *  @param [in]		frequency			Frequency
    *  @param [in]		spread				Spread in basis points.
    *  @param [in]		stubType			StubType: None, ShortStart (SS), LongStart (LS), ShortEnd (SE) or LongEnd (LE).
    *  @param [in]		rollDayInput		Roll Day conventions, e.g. ENDDATE
    *  @param [in]		calendar			Calendar
    *  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		dayCount			Day count convention
    *  @param [in]		interpolation		Interpolation method
    *  @param [in]		compoundType		Compound Type: NORMAL, FLAT, SIMPLE, SIMPLE, AVERAGE
    *  @param [in]		firstStubDate		End date of the front stub period
    *  @param [in]		lastStubDate		Start date of the end stub period
    *  @return			interest rates calculated based on the startDates, endDates, and compound type
    */
    DoubleVector tryMeCurveCompoundRate( const DateVector& startDates,
                                         const DateVector& endDates,
                                         const LAString& curveCollection,
                                         const LAString& forecastCurveIndex,
                                         const LAString& frequency,
                                         double spread,
                                         const LAString& stubType,
                                         const LAString& rollDayInput,
                                         const LAString& calendar,
                                         const LAString& businessDayAdj,
                                         const LAString& dayCount,
                                         const LAString& interpolation,
                                         const LAString& compoundType,
                                         const LAString& firstStubDate,
                                         const LAString& lastStubDate );


    /* @brief			validation interface for the meCompoundRate method
    *  @param [in]		startDate			Start date
    *  @param [in]		endDate				End date
    *  @param [in]		curveCollection		ID of the curve set
    *  @param [in]		forecastCurveIndex	Forecasting curve index
    *  @param [in]		frequency			Frequency
    *  @param [in]		spread				Spread in basis points.
    *  @param [in]		stubType			StubType: None, ShortStart (SS), LongStart (LS), ShortEnd (SE) or LongEnd (LE).
    *  @param [in]		rollDayInput		Roll Day conventions, e.g. ENDDATE
    *  @param [in]		calendar			Calendar
    *  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		dayCount			Day count convention
    *  @param [in]		interpolation		Interpolation method
    *  @param [in]		compoundType		Compound Type: NORMAL, FLAT, SIMPLE, SIMPLE, AVERAGE
    *  @param [in]		firstStubDate		End date of the front stub period
    *  @param [in]		lastStubDate		Start date of the end stub period
    *  @return			compounding rate calculated based on the startDate, endDate, and compound type
    */
    double tryMeCurveCompoundRate( const LADate& startDate,
                                   const LADate& endDate,
                                   const LAString& curveCollection,
                                   const LAString& forecastCurveIndex,
                                   const LAString& frequency,
                                   double spread,
                                   const LAString& stubType,
                                   const LAString& rollDayInput,
                                   const LAString& calendar,
                                   const LAString& businessDayAdj,
                                   const LAString& dayCount,
                                   const LAString& interpolation,
                                   const LAString& compoundType,
                                   const LAString& firstStubDate,
                                   const LAString& lastStubDate );

    /* @brief			validation interface for the meCurveCompoundRateWithFixingTable method
    *  @param [in]		startDates			A vector of start dates
    *  @param [in]		endDates			A vector of end dates
    *  @param [in]		curveCollection		ID of the curve set
    *  @param [in]		forecastCurveIndex	Forecasting curve index
    *  @param [in]		frequency			Frequency
    *  @param [in]		spread				Spread in basis points.
    *  @param [in]		stubType			StubType: None, ShortStart (SS), LongStart (LS), ShortEnd (SE) or LongEnd (LE).
    *  @param [in]		rollDayInput		Roll Day conventions, e.g. ENDDATE
    *  @param [in]		calendar			Calendar
    *  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		dayCount			Day count convention
    *  @param [in]		interpolation		Interpolation method
    *  @param [in]		compoundType		Compound Type: NORMAL, FLAT, SIMPLE, SIMPLE, AVERAGE
    *  @param [in]		firstStubDate		End date of the front stub period
    *  @param [in]		lastStubDate		Start date of the end stub period
    *  @param [in]		fixingTableName		The cached fixing table name
    *  @param [in]		annualized  		True to return annualized rate. Default to true
    *  @return			interest rates calculated based on the startDates, endDates, and compound type
    */
    DoubleVector tryMeCurveCompoundRateWithFixingTable( const DateVector& startDates,
                                         const DateVector& endDates,
                                         const LAString& curveCollection,
                                         const LAString& forecastCurveIndex,
                                         const LAString& frequency,
                                         double spread,
                                         const LAString& stubType,
                                         const LAString& rollDayInput,
                                         const LAString& calendar,
                                         const LAString& businessDayAdj,
                                         const LAString& dayCount,
                                         const LAString& interpolation,
                                         const LAString& compoundType,
                                         const LAString& firstStubDate,
                                         const LAString& lastStubDate,
                                         const std::string& fixingTableName,
                                         bool annualized=true);

    /* @brief			validation interface for the meCurveCompoundRateWithFixingTable method
    *  @param [in]		startDate			Start date
    *  @param [in]		endDate				End date
    *  @param [in]		curveCollection		ID of the curve set
    *  @param [in]		forecastCurveIndex	Forecasting curve index
    *  @param [in]		frequency			Frequency
    *  @param [in]		spread				Spread in basis points.
    *  @param [in]		stubType			StubType: None, ShortStart (SS), LongStart (LS), ShortEnd (SE) or LongEnd (LE).
    *  @param [in]		rollDayInput		Roll Day conventions, e.g. ENDDATE
    *  @param [in]		calendar			Calendar
    *  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		dayCount			Day count convention
    *  @param [in]		interpolation		Interpolation method
    *  @param [in]		compoundType		Compound Type: NORMAL, FLAT, SIMPLE, SIMPLE, AVERAGE
    *  @param [in]		firstStubDate		End date of the front stub period
    *  @param [in]		lastStubDate		Start date of the end stub period
    *  @param [in]		fixingTableName		The cached fixing table name
    *  @param [in]		annualized  		True to return annualized rate. Default to true
    *  @return			compounding rate calculated based on the startDate, endDate, and compound type
    */
    double tryMeCurveCompoundRateWithFixingTable( const LADate& startDate,
                                   const LADate& endDate,
                                   const LAString& curveCollection,
                                   const LAString& forecastCurveIndex,
                                   const LAString& frequency,
                                   double spread,
                                   const LAString& stubType,
                                   const LAString& rollDayInput,
                                   const LAString& calendar,
                                   const LAString& businessDayAdj,
                                   const LAString& dayCount,
                                   const LAString& interpolation,
                                   const LAString& compoundType,
                                   const LAString& firstStubDate,
                                   const LAString& lastStubDate, 
                                   const std::string& fixingTableName,
                                   bool annualized=true);
}

