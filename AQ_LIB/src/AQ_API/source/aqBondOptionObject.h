// aqBondOptionObject.h

/*
 * @brief			Swig interface for aqBondOptionObject... functions
 */

#pragma once

#include "SwigTypes.h"

#include <string>
#include <vector>

/* @brief			swig interface for aqBondOptionObjectCreate
*  @param [in]		objectName			BondOption object name
*  @param [in]		optionLVB			Option definition as a key/value matrix
*  @param [in]		validateKeys		Optional. Default TRUE. Check the LVB keys
*  @return			BondOption object handle
*/
std::string aqBondOptionObjectCreate( const std::string& objectName,
                                       const SWIG_STRINGMATRIX& optionLVB,
                                       const bool validateKeys = true );

/* @brief			swig interface for aqBondOptionObjectDisplay, which displays the INPUT parameters of the cached option
*  @param [in]		objectName			BondOption object name
*  @return			Display of the bond-option input parameters
*/
SWIG_STRINGMATRIX aqBondOptionObjectDisplay( const std::string& objectName );

/* @brief			swig interface for aqBondOptionObjectPV
*  @param [in]		objectName			BondOption object name
*  @param [in]		valuationSettings	Valuation settings as a key/value matrix, e.g. Bond's valuation date, discountCurve's curveCollection
*  @param [in]		bondPrice			Bond spot price at settlement date
*  @param [in]		repoRate			Bond's repo rate
*  @param [in]		repoDaycount		Bond's repo daycount
*  @param [in]		discountRate		Continuously compounded risk free rate
*  @param [in]		discountDayCount	Day count for the discounting
*  @return			Present value of the bond option
*/
double aqBondOptionObjectPV( const std::string& objectName,
                              const SWIG_STRINGMATRIX& valuationSettings,
                              const double bondPrice,
                              const double repoRate,
                              const std::string& repoDaycount,
                              const double discountRate,
                              const std::string& discountDayCount );

/* @brief			swig interface for aqBondOptionObjectGreeks
*  @param [in]		greekType			Greek type: Analytical or Numerical
*  @param [in]		objectName			BondOption object name
*  @param [in]		valuationSettings	Valuation settings as a key/value matrix
*  @param [in]		bondSpotPrice		Bond spot price at settlement date
*  @param [in]		repoRate			Bond's repo rate
*  @param [in]		repoDaycount		Bond's repo daycount
*  @param [in]		discountRate		Continuously compounded risk free rate
*  @param [in]		discountDayCount	Day count for the discounting
*  @param [in]		deltaBump			Delta bump size
*  @param [in]		gammaBump			Gamma bump size
*  @param [in]		vegaBump			Vega bump size
*  @param [in]		thetaBump			Theta bump size
*  @param [in]		rhoBump				Rho bump size
*  @param [in]		showColumnHeaders	Optional. Default TRUE. Include a header row
*  @return			Bond option Greeks' display
*/
SWIG_STRINGMATRIX aqBondOptionObjectGreeks( const std::string& greekType,
                                             const std::string& objectName,
                                             const SWIG_STRINGMATRIX& valuationSettings,
                                             const double bondSpotPrice,
                                             const double repoRate,
                                             const std::string& repoDaycount,
                                             const double discountRate,
                                             const std::string& discountDayCount,
                                             const double deltaBump,
                                             const double gammaBump,
                                             const double vegaBump,
                                             const double thetaBump,
                                             const double rhoBump,
                                             const bool showColumnHeaders = true );
