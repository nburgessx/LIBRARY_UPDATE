#pragma once

#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"

namespace validation
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
    DoubleVector tryMirGetZeroRate1( AQLDataInstance* dataInstance,
                                     const DoubleVector& terms,
                                     const AQLString& curveId,
                                     const AQLString& frequency,
                                     const AQLString& dayCount,
                                     const AQLString& interpolation,
                                     const AQLString& curveName,
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
    DoubleVector tryMirGetZeroRate2( AQLDataInstance* dataInstance,
                                     const AQLStringVector& terms,
                                     const AQLString& curveId,
                                     const AQLString& frequency,
                                     const AQLString& dayCount,
                                     const AQLString& slidingRule,
                                     const AQLString& calendar,
                                     const AQLString& interpolation,
                                     const AQLString& curveName,
                                     bool isFwdInterp ) ;

}
