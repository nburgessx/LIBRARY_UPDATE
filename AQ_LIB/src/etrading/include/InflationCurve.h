#pragma once

#include "IsAQObject.h"
#include "SchemaObject.h"
#include "FreeObject.h"
#include "LabelValueBlock.h"
#include "CommonConstants.h"
#include "InterpolationFactory.h"

namespace etrading
{
	// Forward Declaration
	class ZeroCouponInflationSwap;

	/* A structure to hold forward rate information. The forward rate within a curve period is the growth rate in the CPI level
	*  and is specified by the log-return of the CPI level. See Kerkhof p30.
	*/
	struct ForwardRateResults
	{
		AQLDate lowerBracketDate;	// The pillar date of the curve which defines the lower bracket of the period
		AQLDate upperBracketDate;	// The pillar date of the curve which defines the upper bracket of the period
		double forwardRate;			// The forward rate for the period
	};

	typedef std::tuple<std::vector<std::string>, std::vector<etrading::ContainedTypeEnum>, VariantMatrix>  TableInfo;

	// @brief Holds each input market data point used to calibrate the InflationCurve
	struct ZCInflationSwapMarketData
	{
		std::string tenorOrDate;
		double instrumentQuote;		// The break-even par rate of a calibration instrument
	};

	// @brief Holds a map of all calibration points, sorted by date
	typedef std::map<AQLDate, ZCInflationSwapMarketData > InflationMarketDataMap;

	class InflationCurve : public IsAQObject
	{
	public:
		
		InflationCurve( const std::string& objectName,
						const std::vector<std::string>& propertyKeys,
						const std::vector<TableInfo>& infoBlocks );

		InflationCurve( const std::string& objectName,
						const FreeObject& freeObject );

		~InflationCurve();

		/* @brief Used to serialize an instance of this class
		 * @param[out] the populated SchemaObject
		 */
		virtual const SchemaObject toSchemaObject() const;

		// This is a helper method to populate the supplied SchemaObject
		virtual void const toSchemaObject( SchemaObject& schemaObject ) const;

		/* Brief Returns the curveCollection from which a discount curve is obtained.
		*/
		std::string getCurveCollection() const;

		AQLDate getAsOfDate() const;

		CCY getCurrency() const;

		/* @brief	Returns the name of this curve.
		*			This name used internally to label each dummy calibration instrument
		*/
		std::string getInflationIndexName() const;

		/* Brief	Utility method to return the Zero Coupon Inflation Swap market data used to calibrate the curve.
		*			This method mainly used for testing
		*/
		std::vector< ZCInflationSwapMarketData> getZCInflationSwapInputMarketData() const;
			
		/* @brief	Obtains the inflation level for the specified date, adjusting by the fixing lag
		*
		* @param[in]	date					Return the CPI for this date
		* @param[in]	inflationResetType		Specifies how the lag should be taken into account: MonthlyInterpolation or DailyInterpolation
		* @param[in]	fixLag					Adjust the specified date backwards by this lag tenor.
		* @returns	The calculated CPI level
		*/
		double getInflationIndexForDate( const AQLDate& date, const InflationResetTypeEnum& inflationResetType, const std::string& fixLag ) const;

		/* @brief	Returns the inflation level for the specified date, no further lag adjustment.
		*			The inflation level IS adjusted for seasonality
		*			Note: This function is public to allow access from ZeroCouponInflationSwap during calibration.
		*/
		double getMonthlyInflationIndexForLaggedDate( const AQLDate& laggedDate ) const;

		/* @brief	Returns a matrix of node points representing calibrated values
		*			The format is: Date (including lag), IndexLevel
		*/
		AnyTypeMatrix getCalibrationParameters() const;

		/* @brief Updates the inflation index calibration stored in the curve by adding a node point defined for the specified date.
		 *        This method intended to be used by the calibration process while boostrapping the inflation index points from input break-even par-rates.
		 *
		 * @param [in]   date			The date of this calibration point
		 * @param [in]   inflationIndex	The inflation index to set for the specified date
		 */
		void setCalibrationPoint( const AQLDate& date, const double inflationIndex );


		static std::vector<std::string> inflation_curve_properties_lvbKeys()
		{
			const std::string arr[] =
			{
				INFLATION_CURVE_PROPERTIES_KEY::ASOF_DATE
				, INFLATION_CURVE_PROPERTIES_KEY::SPOT_LAG
				, INFLATION_CURVE_PROPERTIES_KEY::SPOT_BUSINESSDAY_ADJUSTMENT
				, INFLATION_CURVE_PROPERTIES_KEY::SPOT_CALENDAR
				, INFLATION_CURVE_PROPERTIES_KEY::CURRENCY
				, INFLATION_CURVE_PROPERTIES_KEY::SWAP_GENERATOR
				, INFLATION_CURVE_PROPERTIES_KEY::INFLATION_INDEX
				, INFLATION_CURVE_PROPERTIES_KEY::ANNUAL_INTERPOLATION
				, INFLATION_CURVE_PROPERTIES_KEY::CURVE_COLLECTION
				, INFLATION_CURVE_PROPERTIES_KEY::FIXING_TABLE
			};

			std::vector<std::string> expectedKeys(arr, arr + sizeof(arr) / sizeof(arr[0]));

			return expectedKeys;
		}

	private:

		/*
		*  @brief  Utility method which validates the property keys of this InflationCurve, to verify that all are recognized key names.
		*          Will throw if one of the keys is not expected.
		*/
		void validateKeys() const;

		/* @brief Returns a LabelValue block containing the configuration information for the specified propertyKey
		 * @param [in]   propertyKey   The property to be displayed
		 * @param [out]  A LabelValue block containing the properties
		 */
		LabelValueBlock propertyToLabelValueBlock( const std::string& propertyKey ) const;

		/* @brief Populates a default swap expression label value block for use with calibration instruments
		 */
		LabelValueBlock setupSwapExpressionLVBforCalibration() const;

		/* @brief	Returns the inflation level for the specified date, no further lag adjustment.
		*			The inflation level is NOT adjusted for seasonality
		*/
		double getUnadjustedInflationIndexForDate( const AQLDate& laggedDate ) const;

		/* @brief	Get the the seasonality adjustment for the specified date, no further lag adjustment.
		*			The calculation is performed using the exponential seasonal model from Kerkhof p30
		*/
		double getSeasonalityFactor( const AQLDate& laggedDate ) const;

		/* @brief	Returns the forward inflation rate for the specified date, no further lag adjustment.
		*			The forward rate is calculated from the CPI level at the curve pillars which bracket the
		*			specified date and is given as the log return of the CPI level. See Kerkhof p30
		*/
		ForwardRateResults getForwardRateForDate( const AQLDate& laggedDate ) const;

		/* @brief	Calculate a fixing date which is lagged back by the amount specified by fixLagTenor.
		*  @param	inputDate		The date to be lagged
		*  @param	@fixLagTenor	The lag amount, expressed as a tenor. A positive tenor "2M" lags the date backwards
		*  @returns	A fixing date which is lagged back from the inputDate by the amount specified by fixLagTenor
		*/
		AQLDate applyFixingLagToDate( const AQLDate& inputDate, const std::string& fixLagTenor ) const;

		/* @brief Parses the market data and constructs a map from instrument maturity date to input market data point.
		 *        The purpose is to ensure that all calibration points are accessed in the order of increasing maturity.
		 *
		 * @param [in]   referenceDate		A reference start date used to calculate maturity dates from tenors.
		 * @returns The InflationMarketDataMap
		 */
		InflationMarketDataMap loadMarketDataMap( const AQLDate& referenceDate ) const;

		/* @brief	Allow the user to specify CPI fixing overrides in the first year, before the first swap instrument,
		*			and where seasonality does not apply
		*/
		void loadCPIFirstYear();

		/* @brief	Load seasonality data into curve. The seasonality data consists of two coluns:
		*			Month	seasonaityAdjustment
		*			The seasonalityAdjustment is in terms of a forward rate, as defined in Kerkhof's inflation book p30
		*/
		void loadSeasonality();

		// @brief	Called by constructors to calibrate inflation curve from the provided market data
		void calibrate();

		/* @brief Internal helper: calibrate the inflation curve using Zero Coupon Inflation Swap quotes
		 * @param [in]  asOfDate			A reference start date used to calculate maturity dates from tenors.
		 * @param [in]  swapGeneratorName	The swap generator used to obtain conventions and to create calibration instruments
		 * @param [in]  marketDataMap		Input market data, a map from maturity date to break-even par rate, 
		 */
		void calibrateToZCInflationSwaps( const AQLDate& asOfDate, const std::string& swapGeneratorName, const InflationMarketDataMap& marketDataMap );

		/* @brief	During calibration, retrieve convention parameters from calibration instrument
		*/
		void storeConventionsFromCalibrationIntrument( const std::shared_ptr<ZeroCouponInflationSwap>& calibrationInstrument );

		/* @brief	As a post calibration step, initialize an interpolator with the discovered node points.
		*			This interpolator allows annual CPI values from the long end of the curve to be estimated:
		*			In this part of the curve node points may be spaced apart 10 or more years, the interpolator
		*			is used to obtain further points with 1Y spacing.
		*/
		void initializeAnnualInterpolator();

		// The main configuration data store of this object. Allows easy serialization
		FreeObject freeObject_;

		// Parameters populated during calibration step
		AQLDate asOfDate_;
		AQLDate asOfDateWithFixlag_;

		std::string spotLag_;
		std::string spotBusinessDayAdjustment_;
		std::string spotCalendar_;

		std::string curveCollection_;
		std::string inflationIndexName_;
		std::string fixingTableName_;
		CCY currency_;

		// Parameters sourced from swap conventions
		InflationResetTypeEnum inflationResetType_;
		FixingTypeEnum fixingAdvanceOrArrears_;
		BusinessDayAdjustmentEnum fixingbusinessDayAdj_;
		std::string fixingCalendar_;
		std::string fixLag_;
		DayCountEnum accrualDayCount_;

	
		// The output from Calibration: A sorted map of maturity dates and corresponding inflation index values
		std::map<AQLDate, double> calibratedInflationPoints_;		// This allows fast lookup of inflation index by date.

		/* In the long end of the curve node points may be spaced apart 10 or more years.
		*  The interpolator is used to obtain further points with 1Y spacing.
		*/
		InterpolationEnum annualInterpolationType_;
		std::shared_ptr<Interpolation> annualInterpolator_;

		// The seasonal adjustment by month. i.e. the January seasonal adjustment is at position 0.
		std::vector<double> monthlySeasonalData_;
	};

	typedef std::shared_ptr< InflationCurve > InflationCurvePtr;

}

