/*
 * @brief			validation interface for mirGetZeroRate method(s)
 * @Created:		01 April 2016
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "LACoreTemplateType.h"
#include "LADataInstance.h"

namespace validation_api
{


    /* @brief			swig interface for mrGetZeroRate1
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		terms			A  list of given terms in the form of year fraction
    *  @param [in]		curveID			ID of the yield curve
    *  @param [in]		frequency		Frequency of the yield curve
    *  @param [in]		dayCount		Day count convention of the yield curve
    *  @param [in]		interpolation	Interpolation method of the yield curve
    *  @param [in]		curveName		Type of the yield curve, default to STD
    *  @param [in]		isFwdInterp		Boolean that decides if direct interpolation on fwd rates is employed
    *  @return			Zero rate over a given term from the curve's asof date
    */
    DoubleVector tryMirGetZeroRate1( LADataInstance* dataInstance,
                                     const DoubleVector& terms,
                                     const LAString& curveId,
                                     const LAString& frequency,
                                     const LAString& dayCount,
                                     const LAString& interpolation,
                                     const LAString& curveName,
                                     bool isFwdInterp ) ;


    /* @brief			swig interface for mrGetZeroRate2
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		terms			A  list of given terms in the form of tenor string
    *  @param [in]		curveID			ID of the yield curve
    *  @param [in]		frequency		Frequency of the yield curve
    *  @param [in]		dayCount		Day count convention of the yield curve
    *  @param [in]		slidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		calendar		Calendar used to calculate zero dates
    *  @param [in]		interpolation	Interpolation method of the yield curve
    *  @param [in]		curveName		Type of the yield curve, default to STD
    *  @param [in]		isFwdInterp		Boolean that decides if direct interpolation on fwd rates is employed
    *  @return			Zero rate over a given term from the curve's asof date
    */
    DoubleVector tryMirGetZeroRate2( LADataInstance* dataInstance,
                                     const LAStringVector& terms,
                                     const LAString& curveId,
                                     const LAString& frequency,
                                     const LAString& dayCount,
                                     const LAString& slidingRule,
                                     const LAString& calendar,
                                     const LAString& interpolation,
                                     const LAString& curveName,
                                     bool isFwdInterp ) ;

}
