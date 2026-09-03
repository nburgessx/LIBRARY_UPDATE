#include "tryMeLWOInflationPricing.h"

#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "CommonConstants.h"
#include "RecordMacros.h"

#include "ObjectUtilities.h"
#include "LWOUtilities.h"
#include "ExceptionMacros.h"

#include "InflationCurve.h"
#include "ZeroCouponInflationSwap.h"

namespace validation_api
{

	/* @brief Creates an Inflation Curve, calibrated from Zero Coupon Inflation Swap market data
	 * @param [in] objectName        The name of the Inflation Curve object to create
	 * @param [in] propertyNames     A vector of property names corresponding to each label-value block of properties.
	 *                               CURVE_PROPERTIES, ZC_INFLATIONSWAPS, SEASONALITY
	 * @param [in] infoBlocks        A vector of containing the label-value blocks of properties
	 * @param [out]                  The objectName
	 */
	std::string tryMeLWOInflationCurveCreate( const std::string& objectName,
											  const std::vector<std::string>& dataBlockNames,
											  const etrading::JSONInfoBlockTuples& infoBlocks )
	{

		VALID_EXCEPTION_START

		// Perform initial basic sanity checks
		size_t nColumnHeaders = dataBlockNames.size();
		size_t nDataColumns = infoBlocks.size();

		AQ_REQUIRE( nDataColumns > 0, "Invalid InfoBlock Data: Empty InfoBlock - No data provided")
		AQ_REQUIRE( nColumnHeaders == nDataColumns, "Invalid InfoBlock Data: Number of Data Column Headers " + std::to_string(static_cast<long long>(nColumnHeaders)) + " does not match the actual number of Data Columns " + std::to_string(static_cast<long long>(nDataColumns)))

		// Recording of inputs for playback
		if (CreateDataFile::recordEnabled())
		{
			CreateDataFile file(decorateFilename("tryMeLWOInflationCurveCreate_inputs", objectName.c_str()));
			file.write("generatorFunction", "tryMeLWOInflationCurveCreate");
			file.write("objectName", objectName);

			// Write out each propertyName and corresponding block of property config data
			for (unsigned int i = 0; i < dataBlockNames.size(); i++)
			{
				const LAString dataBlockName(dataBlockNames[i].c_str());
				const etrading::VariantMatrix& configData = std::get<2>(infoBlocks[i]);
				file.write(dataBlockName, transpose(configData));
			}
		}

		// Verify that the supplied propertyNames have been set
		const bool hasAnEmptyName = std::any_of(dataBlockNames.cbegin(),
			dataBlockNames.cend(),
			[](const std::string & dataBlockName) -> bool
		{
			return (dataBlockName.empty() || dataBlockName == "");
		});

		AQ_REQUIRE(!hasAnEmptyName, "Invalid InfoBlock: Invalid Data Column Header - One of the Column Names is empty or invalid " + etrading::containerAsString(dataBlockNames))

		// Verify that the supplied propertyNames match the InflationCurveEnum
		std::set<etrading::InflationCurveEnum> enumSet;
		std::for_each(dataBlockNames.cbegin(),
			dataBlockNames.cend(),
			[&enumSet](const std::string & dataBlockName)
		{
			etrading::InflationCurveEnum inflationCurveEnum = etrading::toInflationCurveEnum(dataBlockName);
			enumSet.insert(inflationCurveEnum);
		});

		// Create the Inflation Curve object
		etrading::InflationCurve inflationCurve( objectName, dataBlockNames, infoBlocks );

		// ..  and store in the cache
		etrading::copyToCache<etrading::InflationCurve>( inflationCurve );

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT( objectName );

		VALID_EXCEPTION_END
	}

	/* @brief Obtains the calibration parameters from the specified inflation curve
	 * @param[in] inflationCurveName	Inflation Curve object name
	 * @param[out]						A matrix containing node dates and calibrated index levels.
	 */
	AnyTypeMatrix tryMeLWOInflationCurveCalibrationParameters( const std::string& inflationCurveName )
	{
		VALID_EXCEPTION_START

		RECORD_INPUTS( inflationCurveName );

		auto inflationCurve = etrading::getInflationCurve( inflationCurveName );

		AnyTypeMatrix result = inflationCurve->getCalibrationParameters();

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
	}

	/* @brief	validation interface for the meLWOInflationZCSwapPVFromIndex method.
	*			Calculates the PV of the specified Zero Coupon Inflation Swap.
	*			This function requires the user to provide the inflation fixing level at the start and end of the trade,
	*			taking into account the correct lag.
	*
	* @param[in]	swapName				Swap object name
	* @param[in]	valuationSettingsLVB	Contains the curveCollection, used for discounting cashflows
	* @param[in]	baseIndex				The inflation level at the effective date of the swap
	* @param[in]	resetIndex				The inflation level at the maturity of the swap
	* @param[in]	legName					If specified, calculate the PV of the single swap leg
	* @returns	The calculated PV value
	*/
	double tryMeLWOInflationZCSwapPVFromIndex( const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const double baseIndex, const double resetIndex, const std::string& legName )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(swapName, valuationSettingsLVB, baseIndex, resetIndex, legName);

		auto swap = etrading::getSwap(swapName);

		AQ_REQUIRE(swap->getSwapType() == etrading::ZERO_COUPON_INFLATION_SWAP, "Swap '" + swapName + "' has incorrect Swap Type of '" + toString(swap->getSwapType()) + "'. It is not a zero coupon inflation swap.");

		auto inflationSwap = std::dynamic_pointer_cast<etrading::ZeroCouponInflationSwap>(swap);
		AQ_REQUIRE(inflationSwap != nullptr, "Swap '" + swapName + "' is not a zero coupon inflation swap swap.");

		double result = inflationSwap->pvFromInflationIndex(valuationSettingsLVB, baseIndex, resetIndex, legName);

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief	validation interface for the meLWOInflationZCSwapPV method.
	*			Calculates the PV of the specified Zero Coupon Inflation Swap.
	*			This function takes an inflation curve as input.
	*
	* @param[in]	swapName				Swap object name
	* @param[in]	inflationCurveName		Inflation Curve object name
	* @param[in]	valuationSettingsLVB	Contains the curveCollection, used for discounting cashflows
	* @param[in]	legName					If specified, calculate the PV of the single swap leg
	* @returns	The calculated PV value
	*/
	double tryMeLWOInflationZCSwapPV( const std::string& swapName, const std::string& inflationCurveName, const LabelValueBlock& valuationSettingsLVB, const std::string& legName )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS( swapName, inflationCurveName, valuationSettingsLVB, legName );

		auto swap = etrading::getSwap( swapName );

		AQ_REQUIRE( swap->getSwapType() == etrading::ZERO_COUPON_INFLATION_SWAP, "Swap '" + swapName + "' has incorrect Swap Type of '" + toString(swap->getSwapType()) + "'. It is not a zero coupon inflation swap." );

		auto inflationSwap = std::dynamic_pointer_cast<etrading::ZeroCouponInflationSwap>( swap );
		AQ_REQUIRE(inflationSwap != nullptr, "Swap '" + swapName + "' is not a zero coupon inflation swap swap.");

		auto inflationCurve = etrading::getInflationCurve( inflationCurveName );

		double result = inflationSwap->pvFromInflationCurve( valuationSettingsLVB, *inflationCurve, legName);

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
	}


	/* @brief	validation interface for the meLWOInflationZCSwapParRateFromIndex method.
	*			Calculates the par rate of the Zero Coupon Inflation Swap
	*			i.e. the coupon rate of the fixed leg which causes the swap to PV to zero.
	*			This simple calculation requires the user to specify the inflation fixings at start and end of the trade
	*
	* @param[in]	swapName				Swap object name
	* @param[in]	valuationSettingsLVB	The valuation settings containing curveCollections, and valuation date
	* @param[in]	baseIndex				The inflation level at the effective date of the swap
	* @param[in]	resetIndex				The inflation level at the maturity of the swap
	* @returns	The calculated break-even par-rate value
	*/
	double tryMeLWOInflationZCSwapParRateFromIndex( const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const double baseIndex, const double resetIndex )
	{
		VALID_EXCEPTION_START

			// Record Inputs for logs, tests and playback
			RECORD_INPUTS(swapName, valuationSettingsLVB, baseIndex, resetIndex);

		auto swap = etrading::getSwap(swapName);

		AQ_REQUIRE(swap->getSwapType() == etrading::ZERO_COUPON_INFLATION_SWAP, "Swap '" + swapName + "' has incorrect Swap Type of '" + toString(swap->getSwapType()) + "'. It is not a zero coupon inflation swap.");

		auto inflationSwap = std::dynamic_pointer_cast<etrading::ZeroCouponInflationSwap>(swap);
		AQ_REQUIRE(inflationSwap != nullptr, "Swap '" + swapName + "' is not a zero coupon inflation swap swap.");

		double result = inflationSwap->parRateFromInflationIndex(valuationSettingsLVB, baseIndex, resetIndex);

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief	validation interface for the meLWOInflationZCSwapParRate method.
	*			Calculates the Break-even par rate of the specified Zero Coupon Inflation Swap.
	*			This function takes an inflation curve as input
	*
	* @param[in]	swapName				Swap object name
	* @param[in]	inflationCurveName		Inflation Curve object name
	* @param[in]	valuationSettingsLVB	Contains the curveCollection, used for discounting cashflows
	* @returns	The calculated break-even par-rate value
	*/
	double tryMeLWOInflationZCSwapParRate( const std::string& swapName, const std::string& inflationCurveName, const LabelValueBlock& valuationSettingsLVB )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS( swapName, inflationCurveName, valuationSettingsLVB );

		auto swap = etrading::getSwap(swapName);

		AQ_REQUIRE(swap->getSwapType() == etrading::ZERO_COUPON_INFLATION_SWAP, "Swap '" + swapName + "' has incorrect Swap Type of '" + toString(swap->getSwapType()) + "'. It is not a zero coupon inflation swap.");

		auto inflationSwap = std::dynamic_pointer_cast<etrading::ZeroCouponInflationSwap>(swap);
		AQ_REQUIRE(inflationSwap != nullptr, "Swap '" + swapName + "' is not a zero coupon inflation swap swap.");

		auto inflationCurve = etrading::getInflationCurve( inflationCurveName );

		double result = inflationSwap->parRateFromInflationCurve( valuationSettingsLVB, *inflationCurve );

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief	validation interface for the meLWOInflationCPI method.
	*			Obtains the CPI level from the suuplied inflation curve for the specified date, taking into account the specified lag
	*
	* @param[in]	inflationCurveName		Inflation Curve object name
	* @param[in]	date					Return the CPI for this date
	* @param[in]	inflationResetType		Specifies how the lag should be taken into account: MonthlyInterpolation or DailyInterpolation
	* @param[in]	lag						Adjust the specified date backwards by this lag tenor
	* @returns	The calculated CPI level
	*/
	double tryMeLWOInflationCPI( const std::string& inflationCurveName, const LADate& date, const std::string& inflationResetType, const std::string& lag )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS( inflationCurveName, date, inflationResetType, lag );

		auto inflationCurve = etrading::getInflationCurve( inflationCurveName );
		auto inflationResetTypeEnum = etrading::toInflationResetTypeEnum( inflationResetType );

		double result = inflationCurve->getInflationIndexForDate( date, inflationResetTypeEnum, lag );

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

}