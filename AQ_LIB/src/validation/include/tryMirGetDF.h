#pragma once

#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"

namespace validation
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
    DoubleVector tryMirGetDF1( AQLDataInstance* dataInstance,
                               const DoubleVector& terms,
                               const AQLString& curveID,
                               const AQLString& dayCount,
                               const AQLString& interpolation,
                               bool isBasisFlag,
                               const AQLString& curveName );


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
    DoubleVector tryMirGetDF2( AQLDataInstance* dataInstance,
                               const AQLStringVector& terms,
                               const AQLString& curveID,
                               const AQLString& dayCount,
                               const AQLString& slidingRule,
                               const AQLString& calendar,
                               const AQLString& interpolation,
                               bool isBasisFlag,
                               const AQLString& curveName );


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
    DoubleVector tryMirGetDF3( AQLDataInstance* dataInstance,
                               const DateVector& fromDates,
                               const DateVector& toDates,
                               const AQLString& curveID,
                               const AQLString& dayCount,
                               const AQLString& slidingRule,
                               const AQLString& calendar,
                               const AQLString& interpolation,
                               bool isBasisFlag,
                               const AQLString& curveName );


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
    DoubleVector tryMirGetDF4( AQLDataInstance* dataInstance,
                               const DateVector& fromDates,
                               const DoubleVector& terms,
                               const AQLString& curveID,
                               const AQLString& dayCount,
                               const AQLString& slidingRule,
                               const AQLString& calendar,
                               const AQLString& interpolation,
                               bool isBasisFlag,
                               const AQLString& curveName );


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
    DoubleVector tryMirGetDF5( AQLDataInstance* dataInstance,
                               const DateVector& fromDates,
                               const AQLString& term,
                               const AQLString& curveID,
                               const AQLString& dayCount,
                               const AQLString& slidingRule,
                               const AQLString& calendar,
                               const AQLString& interpolation,
                               bool isBasisFlag,
                               const AQLString& curveName );



}

