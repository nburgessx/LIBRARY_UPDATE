/*
 * @brief			validation interface for mirGetDF method(s)
 * @Created:		24 March 2016
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */


#pragma once

#include "LACoreTemplateType.h"
#include "LADataInstance.h"

namespace validation_api
{
    /* @brief			validation method for mirGetDF1
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		term			An array of terms by year count
    *  @param [in]		curveID			Curve set ID
    *  @param [in]		dayCount		Day count convention. Default to ACT/365
    *  @param [in]		interpolation	Interpolation method. Default to 'SPLINE'
    *  @param [in]		isBasisFlag		Is using basis curve or not. Deprecated input
    *  @param [in]		curveName		Name of the curve where DFs are read off
    *  @return			An array of discount factor
    */
    DoubleVector tryMirGetDF1( LADataInstance* dataInstance,
                               const DoubleVector& terms,
                               const LAString& curveID,
                               const LAString& dayCount,
                               const LAString& interpolation,
                               bool isBasisFlag,
                               const LAString& curveName );


    /* @brief			validation method for mirGetDF2
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		terms			An array of tenor strings
    *  @param [in]		curveID			Curve set ID
    *  @param [in]		dayCount		Day count convention. Default to ACT/365
    *  @param [in]		slidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
    *  @param [in]		calendar		Calendar. Default to 'TKB:LNB'
    *  @param [in]		interpolation	Interpolation method. Default to 'SPLINE'
    *  @param [in]		isBasisFlag		Is using basis curve or not. Deprecated input
    *  @param [in]		curveName		Name of the curve where DFs are read off
    *  @return			An array of discount factor
    */
    DoubleVector tryMirGetDF2( LADataInstance* dataInstance,
                               const LAStringVector& terms,
                               const LAString& curveID,
                               const LAString& dayCount,
                               const LAString& slidingRule,
                               const LAString& calendar,
                               const LAString& interpolation,
                               bool isBasisFlag,
                               const LAString& curveName );


    /* @brief			validation method for mirGetDF3
    *  @param [in]		fromDates		An array of from-dates in YYYYMMDD formate
    *  @param [in]		toDates			An array of to-dates in YYYYMMDD formate
    *  @param [in]		curveID			Curve set ID
    *  @param [in]		dayCount		Day count convention. Default to ACT/365
    *  @param [in]		slidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
    *  @param [in]		calendar		Calendar. Default to 'TKB:LNB'
    *  @param [in]		interpolation	Interpolation method. Default to 'SPLINE'
    *  @param [in]		isBasisFlag		Is using basis curve or not. Deprecated input
    *  @param [in]		curveName		Name of the curve where DFs are read off. Default to 'STD'
    *  @return			A array of discount factors
    */
    DoubleVector tryMirGetDF3( LADataInstance* dataInstance,
                               const DateVector& fromDates,
                               const DateVector& toDates,
                               const LAString& curveID,
                               const LAString& dayCount,
                               const LAString& slidingRule,
                               const LAString& calendar,
                               const LAString& interpolation,
                               bool isBasisFlag,
                               const LAString& curveName );


    /* @brief			validation method for mirGetDF4
    *  @param [in]		fromDates		An array of from-dates in YYYYMMDD formate
    *  @param [in]		terms			An array of year fraction terms
    *  @param [in]		curveID			Curve set ID
    *  @param [in]		dayCount		Day count convention. Default to ACT/365
    *  @param [in]		slidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
    *  @param [in]		calendar		Calendar. Default to 'TKB:LNB'
    *  @param [in]		interpolation	Interpolation method. Default to 'SPLINE'
    *  @param [in]		isBasisFlag		Is using basis curve or not. Deprecated input
    *  @param [in]		curveName		Name of the curve where DFs are read off. Default to 'STD'
    *  @return			A array of discount factors
    */
    DoubleVector tryMirGetDF4( LADataInstance* dataInstance,
                               const DateVector& fromDates,
                               const DoubleVector& terms,
                               const LAString& curveID,
                               const LAString& dayCount,
                               const LAString& slidingRule,
                               const LAString& calendar,
                               const LAString& interpolation,
                               bool isBasisFlag,
                               const LAString& curveName );


    /* @brief			validation method for mirGetDF5
    *  @param [in]		fromDates		An array of from-dates in YYYYMMDD formate
    *  @param [in]		term			A tenor string
    *  @param [in]		curveID			Curve set ID
    *  @param [in]		dayCount		Day count convention. Default to ACT/365
    *  @param [in]		slidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
    *  @param [in]		calendar		Calendar. Default to 'TKB:LNB'
    *  @param [in]		interpolation	Interpolation method. Default to 'SPLINE'
    *  @param [in]		isBasisFlag		Is using basis curve or not. Deprecated input
    *  @param [in]		curveName		Name of the curve where DFs are read off. Default to 'STD'
    *  @return			A array of discount factors
    */
    DoubleVector tryMirGetDF5( LADataInstance* dataInstance,
                               const DateVector& fromDates,
                               const LAString& term,
                               const LAString& curveID,
                               const LAString& dayCount,
                               const LAString& slidingRule,
                               const LAString& calendar,
                               const LAString& interpolation,
                               bool isBasisFlag,
                               const LAString& curveName );



}

