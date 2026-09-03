#pragma once

#include "Variant.h"

#include <string>
#include <vector>

namespace validation
{
	typedef std::tuple<std::vector<std::string>, std::vector<etrading::ContainedTypeEnum>, etrading::VariantMatrix> TableInfo;

    /* @brief Creates a LWOCurveMarketData object, containing all of the curve properties.
	 * @param [in] objectName               The name of the Market Data object
	 * @param [in] marketDataTypes          A vector of market data instrument types corresponding to each label-value block of market data
	 * @param [in] infoBlocks               A vector of containing the label-value blocks of market data
     * @param [out]                         The objectName
	 */
    std::string tryMeLWOCurveMarketDataCreate(  const std::string& objectName,
												const std::vector<std::string>& marketDataKeys,
												const std::vector<TableInfo>& infoBlocks );

    /* @brief Clears All Market Data Bumps and Restores the Existing Curve Market Data Object
	 * @param [in] objectName                       The name of the Market Data object
     * @param [out]                                 The objectName
	 */
    std::string tryMeLWOCurveMarketDataBumpClear( const std::string& objectName );

     /* @brief Bumps a Single Data Block within an Existing Curve Market Data Object
	 * @param [in] objectName                       The name of the Market Data object
     * @param [in] marketDataType                   The market data block or type to bump
     * @param [in] bumpSize                         The BumpSize in Percent
     * @param [in] clearExistingBumps               Clear any existing bumps, defaults to false
     * @param [out]                                 The objectName
	 */
    std::string tryMeLWOCurveMarketDataBumpInstrument( const std::string& objectName, const std::string & marketDataType, const double bumpSize, const bool clearExistingBumps = false );

    /* @brief Bumps a Single Instrument within an Existing Curve Object
	 * @param [in] objectName                       The name of the Market Data object
     * @param [in] marketDataType                   The market data block or type to bump
     * @param [in] bumpSize                         The BumpSize in Percent
     * @param [in] clearExistingBumps               Clear any existing bumps, defaults to false
     * @param [out]                                 The objectName
	 */
    std::string tryMeLWOCurveBumpInstrument( const std::string& objectName, const std::string & marketDataType, const double bumpSize, const bool clearExistingBumps = false );

    /* @brief Bumps an Entire Existing Curve Market Data Object
	 * @param [in] objectName                       The name of the Market Data object
     * @param [in] bumpSize                         The BumpSize in Percent
     * @param [in] onlyBumpOutrightInstruments      Only bump outright instruments, defaults to true
     * @param [out]                                 The objectName
	 */
    std::string tryMeLWOCurveMarketDataBumpAll( const std::string& objectName, const double bumpSize, const bool onlyBumpOutrightInstruments = true );

    /* @brief Bumps all instruments within a given Curve Object
	 * @param [in] objectName                       The name of the Market Data object
     * @param [in] bumpSize                         The BumpSize in Percent
     * @param [in] onlyBumpOutrightInstruments      Only bump outright instruments, defaults to true
     * @param [out]                                 The objectName
	 */
    std::string tryMeLWOCurveBumpAll( const std::string& objectName, const double bumpSize, const bool onlyBumpOutrightInstruments = true );

	/* @brief Displays a LWOCurveMarketData
	* @param [in] marketDataObjectName	The market data object you wish to display
	* @param [in] marketDataKey			The name of the marketData label-value block that you wish to display
    * @param [in] columnIndexToDisplay	Optional Column Number to display (starting from 1). Specify -1 to get all columns
	* @param [out]						A VariantMatrix containing a LabelValue block of properties
	*/
    etrading::VariantMatrix tryMeLWOCurveMarketDataDisplay( const std::string& marketDataObjectName,
												            const std::string& marketDataKey,
                                                            const int columnIndexToDisplay = -1 );

    /* @brief Displays CurveMarketData extracted from a curve object
	* @param [in] curveObjectName		The curve object whose market data you wish to display
	* @param [in] marketDataKey			The name of the marketData label-value block that you wish to display
    * @param [in] columnIndexToDisplay	Optional Column Number to display (starting from 1). Specify -1 to get all columns
	* @param [out]						A VariantMatrix containing a LabelValue block of properties
	*/
	etrading::VariantMatrix tryMeLWOCurveMarketDataDisplayFromCurve( const std::string& curveObjectName,
																	 const std::string& marketDataKey,
																	 const int columnIndexToDisplay = -1 );

    /* @brief Displays a Column LWOCurveMarketData for use with Google Test Calibration Tests
	* @param [in] curveObjectName   The curve object whose market data you wish to display
	* @param [in] marketDataKey     The name of the marketData label-value block that you wish to display
    * @param [in] columnNumber      Optional Column Number to display using column base index 1 and -1 to display all columns
	* @param [out]                  A VariantMatrix containing a LabelValue block of properties
	*/
	etrading::VariantVector tryMeLWOCurveMarketDataColumn( const std::string& curveObjectName,
														   const std::string& marketDataKey,
                                                           const int columnNumber );

}
