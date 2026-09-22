// aqBondFutureOptionObject.h

/*
 * @brief			Swig interface for aqBondFutureOptionObject... functions
 */

#pragma once

#include "SwigTypes.h"

#include <string>
#include <vector>

/* @brief			swig interface for aqBondFutureOptionObjectPV. No separate create
*                   function: aqBondOptionObjectCreate creates the object, this prices it
*                   against a bond-future price instead of a bond spot price
*  @param [in]		objectName			BondOption object name
*  @param [in]		valuationSettings	Valuation settings as a key/value matrix, e.g. Bond's valuation date, discountCurve's curveCollection
*  @param [in]		bondFuturePrice		Bond forward price at option expiry date
*  @param [in]		discountRate		Continuously compounded risk free rate
*  @param [in]		discountDayCount	Day count for the discounting
*  @return			Present value of the bond-future option
*/
double aqBondFutureOptionObjectPV( const std::string& objectName,
                                    const SWIG_STRINGMATRIX& valuationSettings,
                                    const double bondFuturePrice,
                                    const double discountRate,
                                    const std::string& discountDayCount );

/* @brief			swig interface for aqBondFutureOptionObjectGreeks
*  @param [in]		greekType			Greek type: Analytical or Numerical
*  @param [in]		objectName			BondOption object name
*  @param [in]		valuationSettings	Valuation settings as a key/value matrix, e.g. Bond's valuation date, discountCurve's curveCollection
*  @param [in]		bondFuturePrice		Bond forward price at option expiry date
*  @param [in]		discountRate		Continuously compounded risk free rate
*  @param [in]		discountDayCount	Day count for the discounting
*  @param [in]		deltaBump			Delta bump size
*  @param [in]		gammaBump			Gamma bump size
*  @param [in]		vegaBump			Vega bump size
*  @param [in]		thetaBump			Theta bump size
*  @param [in]		rhoBump				Rho bump size
*  @param [in]		showColumnHeaders	Optional. Default TRUE. Include a header row
*  @return			Bond-future option Greeks' display
*/
SWIG_STRINGMATRIX aqBondFutureOptionObjectGreeks( const std::string& greekType,
                                                   const std::string& objectName,
                                                   const SWIG_STRINGMATRIX& valuationSettings,
                                                   const double bondFuturePrice,
                                                   const double discountRate,
                                                   const std::string& discountDayCount,
                                                   const double deltaBump,
                                                   const double gammaBump,
                                                   const double vegaBump,
                                                   const double thetaBump,
                                                   const double rhoBump,
                                                   const bool showColumnHeaders = true );
