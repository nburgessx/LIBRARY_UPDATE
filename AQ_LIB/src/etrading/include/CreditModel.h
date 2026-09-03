/*
 * @brief			Class which defines the Credit Model class.
 * @Created:		26 Feb 2018
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "IsLWOObject.h"
#include "SchemaObject.h"
#include "FreeObject.h"
#include "Variant.h"
#include "LabelValueBlock.h"
#include "CommonConstants.h"
#include "SwapGenerator.h"


namespace etrading
{
	// @brief Contract and market data parameters used to price an index option contract
	struct CreditOptionParameters
	{
		PayerReceiverSwaptionEnum payerReceiverSwaptionEnum_;
		double strike_;
		LADate optionExpiryDate_;
		LADate cdsStartDate_;
		LADate cdsMaturityDate_;
		double cdsCoupon_;
		double volatility_;
		double forwardSpread_;
		double optionValue_;
		bool scaleStrikeBySurvivalProbability_;
		int numberOfUnderliers_;							// How many underliers in the credit index; default is 125
		int numberOfRealizedDefaults_;						// How many defaults have occurred since the credit option began
		double realizedDefaultSettlementAmountOverride_;	// An override specifying the cash payout on defaulted underliers
	};

	// @brief Holds calculated values which are used when a default occurs between option start and valuation time
	struct RealizedDefaults
	{
		double indexSurvivalFactor_;		// A scale factor for the remaining underliers in the index. i.e. with 1 default:  factor = 124/125
		double defaultSettlementAmount_;	// The cash payout on defaulted underliers that the investor is entitled to if they exercise the option
		double defaultSettlementAmountPV_;	// The PV of the settlement amount specified above
	};

	// Forward references
	class CreditDefaultSwap;
	class Bond;
	typedef std::shared_ptr< Bond > BondPtr;

	typedef std::tuple<std::vector<std::string>, std::vector<etrading::ContainedTypeEnum>, VariantMatrix>  TableInfo;

	// @brief Holds each calibration data point used by the CreditModel.
	struct CreditMarketData
	{
		std::string instrumentID;
		double instrumentQuote;
	};

	// @brief Holds a map of all calibration points, sorted by date
	typedef std::map<LADate, CreditMarketData > MarketDataMap;

   class CreditModel : public IsLWOObject
    {
	public:
		/* @brief Main Constructor
		 * @param[in] objectName    The name of this CreditModel instance
		 * @param[in] propertyKeys  A vector containing the names of each configuration block
		 * @param[in] infoBlocks    A vector containing the configuation blocks
		 */
		CreditModel( const std::string& objectName,
                     const std::vector<std::string>& propertyKeys,
                     const std::vector<TableInfo>& infoBlocks );

		/* @brief Constructor used by deserialization
		 * @param[in] objectName    The name of this CreditModel instance
		 * @param[in] freeObject    A freeObject constructed from the serialized data
		 */
		CreditModel( const std::string& objectName,
                     const FreeObject& freeObject );

		/* @brief Copy Constructor
		 */
   		CreditModel(const CreditModel& rhs);

		virtual ~CreditModel() {}

        std::shared_ptr<CreditModel> clone() const;

		/* @brief Used to serialize an instance of this class
		 * @param[out] the populated SchemaObject
		 */
		virtual const SchemaObject toSchemaObject() const;

        // This is a helper method to populate the supplied SchemaObject
        virtual void const toSchemaObject(SchemaObject& schemaObject) const;

		/* @brief Returns the configuration information for the specified propertyKey. If propertyKey is blank, all properties are returned.
		 * @param [in]   propertyKey   The property to be displayed
	     * @param [out]  A VariantMatrix containing a LabelValue block of properties
		 */
		const VariantMatrix viewInputParameters( const std::string& propertyKey ) const;

		/* @brief Returns a LabelValue block containing the configuration information for the specified propertyKey
		 * @param [in]   propertyKey   The property to be displayed
	     * @param [out]  A LabelValue block containing the properties
		 */
		LabelValueBlock toLabelValueBlock( const std::string& propertyKey ) const;

		// ------- Accessor methods --------

		// Calculates the hazardRate for the specified payment date
		double getHazardRate( const LADate& paymentDate ) const;

		bool getIncludeAccruedInterest() const;

		/* @brief	Whether to include the probability of receiving the next coupon
		*			when calculating the Accrued Interest.
		*			true  = multiply the raw accrued interest by the survival probability
		*			false = return the raw accrued interest. 
		*/
		bool bondHasRiskyAccruedInterest() const;

		double getRecoveryRate() const;

		/* @brief	This is the curve collection used when calibrating to / pricing CDS instruments
		*/
		std::string getCDSCurveCollection() const;

		/* @brief	This is the curve collection used when calibrating to / pricing bond instruments
		*/
		std::string getBondCurveCollection() const;

		/* @brief	This is the discount curve to use when discounting the cashflows of bond instruments
		*/
		std::string getBondDiscountCurve() const;

		/* @brief	This is the dayCount used for coupon accrual
		*/
		DayCountEnum getAccrualDayCount() const;

		std::string getCreditIndex() const;

		CCY getCurrency() const;

		LADate getAsOfDate() const;

		// Returns a matrix containing payment dates and hazard rates calibrated from the input market data
		AnyTypeMatrix getCalibrationParameters() const;

		/* @brief	Given a future date, calculates the probability of survival to that date
		*  @param[in]	toDate	The future date to use in the calculation. Must occur after the model as-of date.
		*  @returns	The survival probability
		*/
		double getSurvivalProbability( const LADate& toDate ) const;

		/* @brief	Calculate the probability of survival to "toDate", given the contract has already survived up to "fromDate".
		*  @param[in]	toDate	The future end date to use in the calculation. Must occur after the model as-of date.
		*  @param[in]	fromDate	The future start date i.e. the probability of survival is 1 on this date.
		*							This parameter is allowed to be an empty date i.e. an optional paramweter.
		*  @returns	The survival probability
		*/
		double getSurvivalProbability( const LADate& toDate, const LADate& fromDate ) const;

		/* @brief	Calculate the probability of default in the time period defined by "fromDate" and "toDate".
		*  @param[in]	toDate	The future end date to use in the calculation. Must occur after the model as-of date.
		*  @param[in]	fromDate	The future period start date
		*
		*  @returns	The survival probability
		*/
		double getDefaultProbability( const LADate& toDate, const LADate& fromDate ) const;

		/* @brief	Given a survival probability, calculate the implied survival date
		*			i.e. this function is the inverse of getSurvivalProbability().
		*  @param[in]	survivalProbability	The input survivalProbability	
		*  @returns		The survival date corresponding to the input survivalProbability.
		*/ 
		LADate getImpliedSurvivalDate( const double survivalProbability ) const;

		/* @brief	Given a maturity date, calculates the CDS par-spread and risky annuity implied by the credit model
		*  @param[in]	maturityDate	The maturity date of a CDS
		*  @param[out]	parSpread		The par spread implied by the credit model
		*  @param[out]	riskyAnnuity	The risky annuity implied by the credit model
		*/
		void getParSpreadAndRiskyAnnuityForDate( const LADate& maturityDate, double& parSpread, double& riskyAnnuity ) const;

		/* @brief	Given a start and end date, calculates the forward spread implied by the credit model.
		*			See article "Credit Derivatives Handbook" by JPM p20-21.
		*  @param[in]	startDate	The date when credit protection begins. If this is the model asOf Date,
		*							the function will return the spot credit spread.
		*  @param[in]	endDate		The date when credit protection ends
		*  @returns		The credit spread
		*/
		double getForwardSpread( const LADate& startDate, const LADate& endDate ) const;

		/* @brief	Given a start and end date, calculates the forward spread of a credit index, implied by the credit model.
		*			This calculation follows the Bloomberg approach for a Credit Index.
		*			See Bloomberg whitepaper "Pricing Credit Index Options", 1 March 2012
		*			In summary: Protection leg starts immediately; annuity leg starts on forward start date.
		*
		*  @param[in]	startDate	The date when credit protection begins. If this is the model asOf Date,
		*							the function will return the spot credit spread.
		*  @param[in]	endDate		The date when credit protection ends
		*  @returns		The forward credit spread
		*/
		double getIndexForwardSpread( const LADate& startDate, const LADate& endDate ) const;

		/* @brief Calculates the value of a credit option i.e. option on a CDS instrument
		*  See article "The Valuation of Credit Default Swap Options" by Hull and White
		*
		* @param[in]	payerReceiverSwaptionEnum	Whether the option is a payer (PUT, the right to sell risk, pay CDS spread for protection)
		* 											or receiver (CALL, buy risk, receive CDS spread)
		* @param[in]	strike						The strike spread
		* @param[in]	optionExpiryDate			The expiry date of the option; the effective date of the underlying CDS
		* @param[in]	cdsMaturityDate				The maturity date of the underlying CDS
		* @param[in]	volatility					The volatility of the underlying CDS spread
		* @returns	The calculated option price
		*/
		double getSingleNameKnockoutOptionValue( const PayerReceiverSwaptionEnum payerReceiverSwaptionEnum, const double strike, const LADate& optionExpiryDate, const LADate& cdsMaturityDate, const double volatility ) const;
		
		/* @brief Calculates the value of a credit option i.e. option on a CDS instrument
		*  See article "The Valuation of Credit Default Swap Options" by Hull and White
		*
		* @param[in]	payerReceiverSwaptionEnum	Whether the option is a payer (PUT, the right to sell risk, pay CDS spread for protection)
		* 											or receiver (CALL, buy risk, receive CDS spread)
		* @param[in]	strike						The strike spread
		* @param[in]	optionExpiryDate			The expiry date of the option; the effective date of the underlying CDS
		* @param[in]	cdsStartDate				If the option is exercised, the date on which the underlying CDS is delivered
		* @param[in]	cdsMaturityDate				The maturity date of the underlying CDS
		* @param[in]	volatility					The volatility of the underlying CDS spread
		* @param[in]	forwardSpread				The forward spread at the option expiry date
		* @returns	The calculated option price
		*/
		double getOptionValueFromForward( const PayerReceiverSwaptionEnum payerReceiverSwaptionEnum, const double strike, const LADate& optionExpiryDate, const LADate& cdsStartDate, const LADate& cdsMaturityDate, const double volatility, const double forwardSpread ) const;

		/* @brief Calculates the value of a credit index option i.e. option on a credit index.
		*  See Bloomberg Whitepaper "Pricing Credit Index Options", March 1st, 2012
		*
		* @param[in]	optionLVB					Option parameters such as strike, expiryDate, cdsCoupon, volatility
		* @returns	The calculated option price
		*/
		double getIndexOptionValue( const LabelValueBlock& optionLVB );

		/* @brief Calculates the implied vol of a credit index option, given a target CDS index option quote and CDS index forward at option expiry
		*  See Bloomberg Whitepaper "Pricing Credit Index Options", March 1st, 2012
		*
		* @param[in]	optionLVB					Option parameters such as strike, expiryDate, cdsCoupon, volatility
		* @returns	The implied volatility
		*/
		double getIndexOptionImpliedVol( const LabelValueBlock& optionLVB ) const;

		/* @brief Calculates the vega of a credit index option i.e. option on a credit index.
		*  See Bloomberg Whitepaper "Pricing Credit Index Options", March 1st, 2012
		*
		* @param[in]	optionLVB					Option parameters such as strike, expiryDate, cdsCoupon, volatility
		* @param[in]	volatilityBump				The volatility bump
		* @returns	The calculated option vega
		*/
		double getIndexOptionVega(const LabelValueBlock& optionLVB, const double volatilityBump ) const;

		/* @brief Calculates the CS01 of a credit index option i.e. option on a credit index.
		*  i.e. the sensitivity of the credit index option to a parallel flat shift in credit spreads
		*  See Bloomberg Whitepaper "Pricing Credit Index Options", March 1st, 2012
		*
		* @param[in]	optionLVB					Option parameters such as strike, expiryDate, cdsCoupon, volatility
		* @returns	The calculated option CS01
		*/
		double getIndexOptionCS01( const LabelValueBlock& optionLVB ) const;

		/* @brief Calculates the theta of a credit index option i.e. option on a credit index.
		*  See Bloomberg Whitepaper "Pricing Credit Index Options", March 1st, 2012
		*  Bloomberg define the theta to be "Time-decay in option value for a one (calendar) day decrease in option expiry".
		*  To strictly capture the time-decay only, we reduce the time to expiry by one day and hold everything else constant,
		*  including the calibration.
		*
		* @param[in]	optionLVB					Option parameters such as strike, expiryDate, cdsCoupon, volatility
		* @returns	The calculated option theta
		*/
		double getIndexOptionTheta( const LabelValueBlock& optionLVB ) const;

		/* @brief Calculates the implied vol of a credit option, given a target CDS option quote
		*  See article "The Valuation of Credit Default Swap Options" by Hull and White
		*
		* @param[in]	payerReceiverSwaptionEnum	Whether the option is a payer (PUT, the right to sell risk, pay CDS spread for protection)
		* 											or receiver (CALL, buy risk, receive CDS spread)
		* @param[in]	strike						The strike spread
		* @param[in]	optionExpiryDate			The expiry date of the option; the effective date of the underlying CDS
		* @param[in]	cdsMaturityDate				The maturity date of the underlying CDS
		* @param[in]	targetOptionValue			Calculate the implied vol for this target option value
		* @returns	The implied volatility
		*/
		double getImpliedVol( const PayerReceiverSwaptionEnum payerReceiverSwaptionEnum, const double strike, const LADate& optionExpiryDate, const LADate& cdsMaturityDate, const double targetOptionValue ) const;

		/* @brief Calculates the implied vol of a credit option, given a target CDS option quote and CDS forward at option expiry
		*  See article "The Valuation of Credit Default Swap Options" by Hull and White
		*
		* @param[in]	payerReceiverSwaptionEnum	Whether the option is a payer (PUT, the right to sell risk, pay CDS spread for protection)
		* 											or receiver (CALL, buy risk, receive CDS spread)
		* @param[in]	strike						The strike spread
		* @param[in]	optionExpiryDate			The expiry date of the option; the effective date of the underlying CDS
		* @param[in]	cdsStartDate				If the option is exercised, the date on which the underlying CDS is delivered
		* @param[in]	cdsMaturityDate				The maturity date of the underlying CDS
		* @param[in]	targetOptionValue			Calculate the implied vol for this target option value
		* @param[in]	forwardSpread				The forward CDS spread at option expiry
		* @returns	The implied volatility
		*/
		double getImpliedVolFromForward( const PayerReceiverSwaptionEnum payerReceiverSwaptionEnum, const double strike, const LADate& optionExpiryDate, const LADate& cdsStartDate, const LADate& cdsMaturityDate, const double targetOptionValue, const double forwardSpread ) const;


		/* @brief Updates the hazard rate calibration stored in the model by adding a hazard rate point defined for the specified paymentDate
		 *        This method intended to be used by the calibration process while boostrapping the hazard rates from input CDS spreads.
		 * 
		 * @param [in]   paymentDate	The payment date corresponding to this hazard rate
		 * @param [in]   hazardRate		The estimate of the hazard rate for this payment date
		 */
		void setCalibrationPoint( const LADate& paymentDate, const double hazardRate );

		static std::vector<std::string> model_properties_lvbKeys()
		{
			const std::string arr[] =
			{
				CREDITMODEL_MODEL_PROPERTIES_KEY::ASOF_DATE
				, CREDITMODEL_MODEL_PROPERTIES_KEY::SPOT_LAG
				, CREDITMODEL_MODEL_PROPERTIES_KEY::SPOT_BUSINESSDAY_ADJUSTMENT
				, CREDITMODEL_MODEL_PROPERTIES_KEY::SPOT_CALENDAR
				, CREDITMODEL_MODEL_PROPERTIES_KEY::ACCRUAL_START_DATE
				, CREDITMODEL_MODEL_PROPERTIES_KEY::IMM_REFERENCE_DATE
				, CREDITMODEL_MODEL_PROPERTIES_KEY::CURRENCY
				, CREDITMODEL_MODEL_PROPERTIES_KEY::RECOVERY_RATE
				, CREDITMODEL_MODEL_PROPERTIES_KEY::INCLUDE_ACCRUED_INTEREST
				, CREDITMODEL_MODEL_PROPERTIES_KEY::SWAP_GENERATOR
				, CREDITMODEL_MODEL_PROPERTIES_KEY::CREDIT_INDEX
				, CREDITMODEL_MODEL_PROPERTIES_KEY::INTERPOLATION
				, CREDITMODEL_MODEL_PROPERTIES_KEY::EXTRAPOLATION
				, CREDITMODEL_MODEL_PROPERTIES_KEY::CDS_CURVE_COLLECTION
				, CREDITMODEL_MODEL_PROPERTIES_KEY::BOND_CURVE_COLLECTION
				, CREDITMODEL_MODEL_PROPERTIES_KEY::BOND_DISCOUNT_CURVE
				, CREDITMODEL_MODEL_PROPERTIES_KEY::BOND_HAS_RISKY_ACCRUED_INTEREST
			};

			std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );

			return expectedKeys;
		}

	private:

		/* @brief	Creates a CDS Instrument from conventions stored in the curve's SwapGenerator.
		*			Used in calibration and option pricing
		*
		*  @param[in]	cdsStartDate	Create a CDS with this effective date
		*  @param[in]	cdsMaturityDate	Create a CDS with this maturity date
		*  @returns		A CreditDefaultSwap object
		*/
		std::shared_ptr<CreditDefaultSwap> createCalibrationCDSWithSpecifiedMaturity( const LADate& cdsStartDate, const LADate& cdsMaturityDate ) const;

		/* @brief	Given a start and end date, calculates the forward spread and risky annuity implied by the credit model.
		*			See article "Credit Derivatives Handbook" by JPM  p20-21.

		*  @param[in]	startDate	The start date of the forward contract
		*  @param[in]	endDate		The end date of the forward contract
		*  @returns		The forward spread
		*/
		void getForwardSpreadAndRiskyAnnuity( const LADate& startDate, const LADate& endDate, double& forwardSpread, double& riskyAnnuity ) const;

		/* @brief	Calculates the index option payoff, assuming a lognormal process for credit spread
		*			See Bloomberg whitepaper "Pricing Credit Index Options".
		*
		*  @param[in]	params				A structure holding credit option input deal parameters
		*  @param[in]	realizedDefaults	A structure holding calculated values related to realized defaults
		*  @param[in]	cdsIndex			The underlying CDS index instrument
		*  @param[in]	payerReceiverIndicator	A multiplier factor which indicates PAYER ( +1.0 ) or RECEIVER ( -1.0 )
		*  @param[in]	meanSpread			The calibrated mean of the log normal ditribution which reprices the forward contract
		*  @param[in]	normalVariateSample	A sample from the normal distribution
		*  @param[in]	drift				The drift of the log-normal process, -vol^2 t / 2
		*  @param[in]	volatilityTime		The factor  vol sqrt(t)
		*  @param[in]	riskyAnnuityAtStrike The risky annuity of the underlier, calculated using a flat hazard rate at the strike
		*  @returns	The option payoff value corresponding to the specified the normalVariateSample
		*/
		double optionPayoffFunction( const CreditOptionParameters& params,
									 const RealizedDefaults& realizedDefaults,
									 const std::shared_ptr<CreditDefaultSwap>& cdsIndex,
									 const double payerReceiverIndicator,
									 const double meanSpread,
									 const double normalVariateSample,
									 const double drift,
									 const double volatilityTime,
									 const double riskyAnnuityAtStrike ) const;


		/* @brief Calculates the value of a credit index option i.e. option on a credit index.
		*  See Bloomberg Whitepaper "Pricing Credit Index Options", March 1st, 2012
		*
		* @param[in]	params						Contract and market data parameters used to price an index option contract
		* @param[in]	forwardSpread				The forward spread at the option expiry date
		* @param[in]	volatility					The volatility of the underlying CDS spread
		* @param[in]	applyThetaBump				If true, reduce the time to option expiry by 1 calendar day.
		* @returns	The calculated option price
		*/
		double getIndexOptionValue( const CreditOptionParameters& params, const double forwardSpread, const double volatility, const bool applyThetaBump = false ) const;


		DayCountEnum getAccrualDayCountFromGenerator( const SwapGeneratorPtr& cdsGenerator ) const;
		
		/* @brief Populates a default swap expression label value block
		 * 
		 * @param [in]   asOfDate		The asOf / valuation date
		 * @param [in]   cdsGenerator	The swap generator containing swap conventions
		 */
		LabelValueBlock setupSwapExpressionLVBforCalibration( ) const;

		/* @brief Parses the market data and constructs a map from instrument maturity date to calibration data point.
		 *        The purpose is to ensure that all calibration points are accessed in the order of increasing maturity,
		 *        required by the bootstrap calibration process.
		 * @param [in]   referenceDate		A reference start date used to calculate maturity dates from tenors.
		 *									For CDS instruments, set to IMMReferenceDate (if available), otherwise the asOf date.
		 * @param [in]   marketDataEnum		The type of marketDate used for calibration
		 * @returns The MarketDataMap
		 */
		MarketDataMap loadMarketDataMap( const LADate& referenceDate, const CreditModelEnum marketDataEnum );

		// @brief	Called by constructors to calibrate hazard rates from the provided market data
		void calibrate();

		// @brief	Called by main calibration routine. Allows an optional parallel bump to be applied to CDS spreads
		void loadMarketDataAndCalibrate( const double spreadBump = 0.0 );

		/* @brief Internal helper: calibrate the credit model to cdsInstrument market data
		 * @param [in]  asOfDate			A reference start date used to calculate maturity dates from tenors.
		 * @param [in]  cdsMarketDataMap	A map from cds maturity dates to CDS spreads
		 * @param [in]  cdsSpreadBump		A parallel shift to be applied to all credit spreads
		 */
		void calibrateToCDS( const LADate& asOfDate, const MarketDataMap& cdsMarketDataMap, const double cdsSpreadBump );

		/* @brief Internal helper: calibrate the credit model to cdsInstrument market data
		 * @param [in]  bondMarketDataMap	A map from bond maturity dates to bond instrument names and prices
		 */
		void calibrateToBonds( const MarketDataMap& bondMarketDataMap );

		/* @brief	Performs final setup after the calibration step.
		*			In particular, this method pre-calculates survival probabilities
		*			in order to make getImpliedSurvivalDate() run fast
		*/
		void postCalibrationSetup();
		
		// Post-calibration data members which are initialized by postCalibrationSetup() above.
		// These vectors allow fast lookup of calibration results by array index
		DateVector calibrationDates_;
		DoubleVector survivalProbabilitiesOnCalibrationDates_;
		DoubleVector hazardRatesVector_;

		/* @brief Validate that the supplied toDate and fromDate are consistent with each other and the model asOfDates
		 * @param [in]	toDate		The initial date for survival probability calculations
		 * @param [in]	fromDate	The final date for survival probability calculations
		 */
		void validateDates( const LADate& toDate, const LADate& fromDate ) const;

		/*
		*  @brief  Utility method which validates the property keys of this CreditModel, to verify that all are recognized key names.
		*          Will throw if one of the keys is not expected.
		*/
		void validateKeys() const ;
		
		// The main data store of this object. Allows easy serialization
		FreeObject freeObject_;

		// Parameters populated during calibration step
		LADate asOfDate_;
		std::string spotLag_;
		std::string spotBusinessDayAdjustment_;
		std::string spotCalendar_;
		LADate accrualStartDate_;
		LADate immReferenceDate_;
		bool includeAccruedInterest_;
		double recoveryRate_;
		DayCountEnum accrualDayCount_;
		std::string cdsCurveCollection_;
		std::string creditIndex_;
		std::string cdsGeneratorName_; // The swap generator used to create temporary CDS instruments during the calibration
		CCY currency_;

		std::string bondCurveCollection_;
		std::string bondDiscountCurve_;
		bool bondHasRiskyAccruedInterest_;		// If true, then multiply the bond accrued interest by survival factor

		// Used when interpolating hazard rates
		std::string interpolationMethod_;
		std::string extrapolationMethod_;

		// The output from Calibration: A map of payment dates and corresponding hazardRates
		std::map<LADate, double> hazardRates_; // This allows fast lookup of hazardRate by date.
	};

	typedef std::shared_ptr< CreditModel > CreditModelPtr;
}






