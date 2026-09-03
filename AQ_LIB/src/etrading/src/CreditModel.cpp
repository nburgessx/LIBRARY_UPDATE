/*
 * @brief			Class which defines the Credit Model class.
 * @Created:		26 Feb 2018
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */


#include "CreditModel.h"
#include "CreditDefaultSwap.h"
#include "EuropeanIRSwaption.h"  // To price option on CDS
#include "NormalDistribution.h"

#include <vector>
#include <string>
#include <boost/algorithm/string.hpp>
#include "ql/math/integrals/gaussianquadratures.hpp"

#include "Solvers.h"
#include "ParameterValidation.h"
#include "LWOUtilities.h"
#include "ObjectUtilities.h"
#include "DataUtilities.h"
#include "LADateScheduleHelpers.h"


namespace etrading
{

	// @brief	The list of permitted keys in the option parameter LabelValueBlock
	std::vector<std::string> optionParameterLVBKeys()
	{
		const std::string arr[] =
		{
			CREDIT_OPTION_KEYS::PAYER_RECEIVER,
			CREDIT_OPTION_KEYS::STRIKE,
			CREDIT_OPTION_KEYS::EXPIRY,
			CREDIT_OPTION_KEYS::CDS_START_DATE,
			CREDIT_OPTION_KEYS::CDS_MATURITY_DATE,
			CREDIT_OPTION_KEYS::CDS_COUPON,
			CREDIT_OPTION_KEYS::VOLATILITY,
			CREDIT_OPTION_KEYS::FORWARD_SPREAD,
			CREDIT_OPTION_KEYS::OPTION_VALUE,
			CREDIT_OPTION_KEYS::SCALE_STRIKE_BY_SURVIVAL_PROBABILITY,
			CREDIT_OPTION_KEYS::NUMBER_OF_UNDERLIERS,
			CREDIT_OPTION_KEYS::NUMBER_OF_REALIZED_DEFAULTS,
			CREDIT_OPTION_KEYS::DEFAULT_SETTLEMENT_AMOUNT
		};

		std::vector< std::string > expectedKeys(arr, arr + sizeof(arr) / sizeof(arr[0]));
		return expectedKeys;
	}

	/* @brief	Populates a structure with index option parameters obtained from a LabelValueBlock
	*
	*  @param[in]	optionLVB		Specifies option contract parameters and market data in LabelValueBlock format
	*  @param[in]	impliedVolCalc	A boolean flag which determines whether to retrieve parameters for an impliecVol calculation
	*  @returns	A populated parameter structure
	*/
	CreditOptionParameters getCreditOptionParameters( const LabelValueBlock& optionLVB, const bool impliedVolCalc = false )
	{
		// Validate the input optionLVB
		std::vector< std::string > expectedKeys = optionParameterLVBKeys();
		std::vector< std::string > actualKeys   = optionLVB.getKeys();
		const std::string LVBname( "CreditIndexOptionParameters" );
		const bool validateKeys = true;
		validateKeysForLVB( expectedKeys, actualKeys, validateKeys, LVBname );

		// Parse the input LVB into a parameter structure
		CreditOptionParameters params;
		params.payerReceiverSwaptionEnum_ = toPayerReceiverSwaptionEnum( optionLVB.getCompulsoryValueAsString(CREDIT_OPTION_KEYS::PAYER_RECEIVER ) );

		params.optionExpiryDate_ = optionLVB.getCompulsoryValueAsDate( CREDIT_OPTION_KEYS::EXPIRY );
		params.cdsStartDate_	= optionLVB.getCompulsoryValueAsDate( CREDIT_OPTION_KEYS::CDS_START_DATE );
		params.cdsMaturityDate_	= optionLVB.getCompulsoryValueAsDate( CREDIT_OPTION_KEYS::CDS_MATURITY_DATE );

		params.strike_			= optionLVB.getCompulsoryValueAsDouble( CREDIT_OPTION_KEYS::STRIKE );
		params.cdsCoupon_		= optionLVB.getCompulsoryValueAsDouble( CREDIT_OPTION_KEYS::CDS_COUPON );
		params.forwardSpread_	= optionLVB.getCompulsoryValueAsDouble( CREDIT_OPTION_KEYS::FORWARD_SPREAD );

		params.volatility_		= impliedVolCalc ? std::numeric_limits<double>::quiet_NaN() : optionLVB.getCompulsoryValueAsDouble( CREDIT_OPTION_KEYS::VOLATILITY );
		params.optionValue_		= impliedVolCalc ? optionLVB.getCompulsoryValueAsDouble( CREDIT_OPTION_KEYS::OPTION_VALUE ) : std::numeric_limits<double>::quiet_NaN();

		params.scaleStrikeBySurvivalProbability_ = optionLVB.getOptionalValueAsBool( CREDIT_OPTION_KEYS::SCALE_STRIKE_BY_SURVIVAL_PROBABILITY, true );

		params.numberOfUnderliers_			= optionLVB.getOptionalValueAsInt( CREDIT_OPTION_KEYS::NUMBER_OF_UNDERLIERS, 125 );
		params.numberOfRealizedDefaults_	= optionLVB.getOptionalValueAsInt( CREDIT_OPTION_KEYS::NUMBER_OF_REALIZED_DEFAULTS, 0 );
		params.realizedDefaultSettlementAmountOverride_ = optionLVB.getOptionalValueAsDouble( CREDIT_OPTION_KEYS::DEFAULT_SETTLEMENT_AMOUNT, std::numeric_limits<double>::quiet_NaN() );

		return params;
	}

	/* @brief Main Constructor
	 * @param[in] objectName    The name of this CreditModel instance
	 * @param[in] propertyKeys  A vector containing the names of each configuration block
	 * @param[in] infoBlocks    A vector containing the configuation blocks
	 */
	CreditModel::CreditModel( const std::string& objectName,
                              const std::vector<std::string>& propertyKeys,
                              const std::vector<TableInfo>& infoBlocks ) 
						: IsLWOObject(objectName, CREDIT_MODEL), 
						  freeObject_(objectName)
	{

		// Create a FreeObject from each property label-value block, and concatenate to our FreeObject data member
		const bool allowJaggedData = false;
        for( unsigned int gridCounter = 0; gridCounter < infoBlocks.size(); gridCounter++ )
        {
            const std::vector<std::string>& columnNames = std::get<0>( infoBlocks[ gridCounter ] );
            const std::vector<etrading::ContainedTypeEnum>& colTypes = std::get<1>( infoBlocks[ gridCounter ]) ;
            const VariantMatrix& rangeData = std::get<2>( infoBlocks[ gridCounter ] );
            freeObject_ += createFreeObjectFromGrid( objectName, columnNames, colTypes, rangeData, propertyKeys[ gridCounter ], allowJaggedData );
        }

		// Verify that all CreditModel properties are valid and expected
		validateKeys();

		calibrate();
	}

	/* @brief Constructor used by deserialization
	 * @param[in] objectName    The name of this CreditModel instance
	 * @param[in] freeObject    A freeObject constructed from the serialized data
	 */
	CreditModel::CreditModel( const std::string& objectName, const FreeObject& freeObject ) 
					: IsLWOObject(objectName, CREDIT_MODEL ), 
					  freeObject_(freeObject)
	{
		calibrate();
    }

	/* @brief Copy Constructor
	 */
	CreditModel::CreditModel(const CreditModel& rhs) 
		: IsLWOObject( rhs.getRefToName(), CREDIT_MODEL ), 
		  freeObject_( rhs.freeObject_ ),
		  asOfDate_( rhs.asOfDate_ ),
		  spotLag_( rhs.spotLag_ ),
		  spotBusinessDayAdjustment_( rhs.spotBusinessDayAdjustment_ ),
		  spotCalendar_( rhs.spotCalendar_ ),
		  accrualStartDate_( rhs.accrualStartDate_ ),
		  immReferenceDate_( rhs.immReferenceDate_ ),
		  includeAccruedInterest_( rhs.includeAccruedInterest_ ),
		  bondHasRiskyAccruedInterest_( rhs.bondHasRiskyAccruedInterest_ ),
		  recoveryRate_( rhs.recoveryRate_ ),
		  accrualDayCount_( rhs.accrualDayCount_ ),
		  cdsCurveCollection_( rhs.cdsCurveCollection_ ),
		  bondCurveCollection_( rhs.bondCurveCollection_ ),
		  bondDiscountCurve_( rhs.bondDiscountCurve_ ),
		  creditIndex_( rhs.creditIndex_ ),
		  cdsGeneratorName_( rhs.cdsGeneratorName_ ),
		  currency_( rhs.currency_ ),
		  interpolationMethod_( rhs.interpolationMethod_ ),
		  extrapolationMethod_( rhs.extrapolationMethod_ ),
		  hazardRates_( rhs.hazardRates_ ),
		  calibrationDates_( rhs.calibrationDates_),
		  survivalProbabilitiesOnCalibrationDates_( rhs.survivalProbabilitiesOnCalibrationDates_ ),
		  hazardRatesVector_( rhs.hazardRatesVector_ )
	{
	}

	std::shared_ptr<CreditModel> CreditModel::clone() const
    {
       auto data = std::make_shared<CreditModel>(*this);
       return data;
    }

	DayCountEnum CreditModel::getAccrualDayCountFromGenerator( const SwapGeneratorPtr& cdsGenerator ) const
	{
		std::string accrualDayCount;

		// Fetch the Swap Conventions block from the CDS Swap Generator
		LAStringMatrix swapConventions = cdsGenerator->viewInputParameters( );

		for (size_t i=0; i< swapConventions.size(); i++)
		{
			const LAStringVector& row = swapConventions[i];
			std::string key( row[0].getCString() );
			if ( boost::iequals(key, etrading::IRS_KEY::ACCRUALDAYCOUNT ) )
			{
				accrualDayCount = row[1].getCString();
				break;
			}
		}
		return toDayCountEnum( accrualDayCount );
	}

	/* @brief Populates a default swap expression label value block
	* 
	* @param [in]   asOfDate		The asOf / valuation date
	* @param [in]   cdsGenerator	The swap generator containing swap conventions
	*/
	LabelValueBlock CreditModel::setupSwapExpressionLVBforCalibration( ) const
	{
		// Set up the Swap Expression LVB used for repricing swap calibration instruments
		LAStringVector keys; 
		LAStringVector values;

        keys.reserve(6);
        values.reserve(6);

		// We standardize these trade keys because we are only interested in par rate calculations 
	    keys.push_back( IRS_KEY::PAY_RECEIVE.c_str() );					    values.push_back("PAY");
		keys.push_back( IRS_KEY::NOTIONAL.c_str() );					    values.push_back("1.0");
		keys.push_back( IRS_KEY::EFFECTIVE_DATE.c_str() );				    values.push_back("");
		keys.push_back( IRS_KEY::MATURITY_DATE.c_str() );				    values.push_back("");
		keys.push_back( SWAP_EXPRESSION_KEY::RATE_OR_SPREAD1.c_str() );	    values.push_back("0.0");
		keys.push_back( SWAP_EXPRESSION_KEY::RATE_OR_SPREAD2.c_str() );	    values.push_back("0.0");

		LabelValueBlock swapExpressionLVB = etrading::populateLabelValueBlock( keys, values );

		return swapExpressionLVB;			
	}

	/* @brief Parses the market data and constructs a map from instrument maturity date to calibration data point.
	*        The purpose is to ensure that all calibration points are accessed in the order of increasing maturity,
	*        required by the bootstrap calibration process.
	* @param [in]   referenceDate		A reference start date used to calculate maturity dates from tenors.
	*									For CDS instruments, set to IMMReferenceDate (if available), otherwise the asOf date.
	* @param [in]   marketDataEnum		The type of marketDate used for calibration
	* @returns The MarketDataMap
	*/
	MarketDataMap CreditModel::loadMarketDataMap( const LADate& referenceDate, const CreditModelEnum marketDataEnum )
	{
		// Read the market data and perform sanity checks
		const bool throwIfKeyMissing = false;
		VariantMatrix marketData = getVariantMatrixFromFreeObject ( freeObject_, toString( marketDataEnum ), throwIfKeyMissing );
		const size_t numMarketDataColumns = marketData.size();

		MarketDataMap marketDataMap;
		if ( numMarketDataColumns == 0 )
		{
			// Return an empty map if there are no columns to read.
			return marketDataMap;
		}

		if ( numMarketDataColumns != 2 )
		{
			MLIB_THROW( toString( marketDataEnum ) + ": should contain 2 columns. Found " + std::to_string( static_cast<long long> ( numMarketDataColumns )) + " columns.");
		}

		// Iterate through the calibration marketData rows and for each row determine the instrumentMaturityDate and instrument quote.
		//Supported marketDataEnum types: CDS_MARKETDATA and BOND_MARKETDATA.
		const size_t numMarketDataRows = marketData[0].size();
		for (size_t i=0; i<numMarketDataRows; i++)
		{
			std::string instrumentID;
			LADate instrumentMaturityDate;

			// Only process the row if the data in column 0 is non-blank. i.e. trim blank rows
			std::string dataInColumnZero = marketData[0][i];
			if ( dataInColumnZero != "" )
			{
				switch ( marketDataEnum )
				{
					case CDS_MARKETDATA:
					{
						instrumentID = "";
						std::string  instrumentTenorOrMaturityDate = dataInColumnZero;
						instrumentMaturityDate = validateMaturityDate( referenceDate, instrumentTenorOrMaturityDate.c_str() );
						break;
					}
					case BOND_MARKETDATA:
					{
						instrumentID =	dataInColumnZero;
						auto bondInstrument = getBond( instrumentID );
						instrumentMaturityDate = bondInstrument->getSchedule()->getMaturityDate();
						break;
					}
					default:
						MLIB_THROW("Only CDS_MARKETDATA or BOND_MARKETDATA supported.");
				}

				Variant value = marketData[1][i];
				double instrumentQuote = std::numeric_limits<double>::quiet_NaN();

				switch ( value.getType() )
				{
				case STRING_VALUE:
				{
					std::string stringValue = value.getValue<std::string>();
					boost::trim( stringValue );
					MLIB_REQUIRE( stringValue != "", "Missing instrument quote." );
					char * pFirstNonNumber;
                    instrumentQuote = strtod( stringValue.c_str(), &pFirstNonNumber );
					break;
				}
				case DOUBLE_VALUE:
					instrumentQuote = value.getValue<double>();
					break;
				case INTEGER_VALUE:
					instrumentQuote = static_cast<double> (value.getValue<int>() );
					break;
				default:
					MLIB_THROW( "Unexpected data type in Credit Model Market Data ");
				}
			
				CreditMarketData marketData;
				marketData.instrumentID = instrumentID;
				marketData.instrumentQuote = instrumentQuote;

				if ( marketDataMap.find( instrumentMaturityDate ) != marketDataMap.end() )
				{
					MLIB_THROW( "Found two or more calibration instruments with the same maturity date '"
								+ instrumentMaturityDate.stringWithFormat( "DD-MM-YYYY")
								+ "'." );
				}
				marketDataMap[ instrumentMaturityDate ] = marketData;
			}
		}
		return marketDataMap;
	}

	/* @brief Internal helper: calibrate the credit model to cdsInstrument market data
	 * @param [in]  asOfDate			A reference start date used to calculate maturity dates from tenors.
	 * @param [in]  cdsMarketDataMap	A map from cds maturity dates to CDS spreads
	 * @param [in]  cdsSpreadBump		A parallel shift to be applied to all credit spreads
	 */
	void CreditModel::calibrateToCDS( const LADate& asOfDate, const MarketDataMap& cdsMarketDataMap, const double cdsSpreadBump )
	{
	
		// Additional parameters for creating CDS instruments
		const bool isXccySwap = false;
		LabelValueBlock swapPropertiesLVB;
		LAString premiumLegName;
		LAString protectionLegName;

		// Calculate the effective date for CDS calibration instruments
		const LADate effectiveDate = LADateScheduleHelpers::getDate( asOfDate, spotLag_.c_str(), spotBusinessDayAdjustment_.c_str(), spotCalendar_.c_str() );
		LabelValueBlock swapExpressionLVB( setupSwapExpressionLVBforCalibration(),
                                           IRS_KEY::EFFECTIVE_DATE,
		                                   std::to_string( static_cast<long long>( LADateScheduleHelpers::getExcelDate(effectiveDate) ) ) );
        
		if ( cdsMarketDataMap.size() > 0 )
		{
			// If CDS Instruments have been provided for calibration, require that a CDSCurveCollection data is also present.
			MLIB_REQUIRE( cdsCurveCollection_.size() > 0, "Please Specify a CDSCurveCollection when calibrating to CDS Instruments." );
		}

		// Calibrate to CDSs
		for ( auto it = cdsMarketDataMap.begin(); it != cdsMarketDataMap.end(); ++it )
		{
			const LADate& maturityDate = it->first;
			const CreditMarketData& creditMarketData = it->second;

			MLIB_REQUIRE( accrualStartDate_ <= maturityDate, "Invalid Accrual Start Date: The Accrual Start Date cannot be greater than the CDS maturity date" )
            MLIB_REQUIRE( immReferenceDate_ <= maturityDate, "Invalid IMM Reference Date: The IMM Reference Date cannot be greater than the CDS maturity date" )

            // Create a new swapExpressionLVB
            swapExpressionLVB = LabelValueBlock( swapExpressionLVB,  IRS_KEY::MATURITY_DATE, MLIB_TO_STRING_FROM_INT(LADateScheduleHelpers::getExcelDate( maturityDate ) ) );
			auto swapInstrument = createSwapFromGenerator( creditIndex_, cdsGeneratorName_, swapExpressionLVB, swapPropertiesLVB, isXccySwap );
			std::shared_ptr<CreditDefaultSwap> cdsInstrument = std::dynamic_pointer_cast<CreditDefaultSwap>( swapInstrument );

			// NOTE: The creditModel is updated by this method in order to populate the new calibration point
			const double hazardRate = cdsInstrument->hazardRateFromParSpread( creditMarketData.instrumentQuote + cdsSpreadBump, *this, premiumLegName, protectionLegName );
			if ( hazardRate < 0.0 )
			{
				// This occurs if two CDs maturities are in close proximity with inconsistent prices.
				// In order to match the CDS price the survival probability is required to increase between maturities, which is impossible.
				const LADate& cdsMaturity = cdsInstrument->getLeg(0)->getSchedule()->getMaturityDate();
				MLIB_THROW( "Detected two CDS instruments with clashing maturities and prices. Consider removing the CDS with maturity '"
							+ std::string( cdsMaturity.stringWithFormat( "DD-MM-YYYY" ).getCString() )
							+ "' or the earlier CDS." );
			}
		}
	}

	/* @brief Internal helper: calibrate the credit model to cdsInstrument market data
	 * @param [in]  bondMarketDataMap	A map from bond maturity dates to bond instrument names and prices
	 */
	void CreditModel::calibrateToBonds( const MarketDataMap& bondMarketDataMap )
	{
		if ( bondMarketDataMap.size() > 0 )
		{
			// If Bond Instruments have been provided for calibration, require that a BondCurveCollection data is also present.
			MLIB_REQUIRE( bondCurveCollection_.size() > 0, "Please Specify a BondCurveCollection when calibrating to Bond Instruments." );
		}

		const LADate& settlementDate = asOfDate_;
		for ( auto it = bondMarketDataMap.begin(); it != bondMarketDataMap.end(); ++it )
		{
			const LADate& maturityDate = it->first;
			const CreditMarketData& bondMarketData = it->second;

			auto bondInstrument = getBond( bondMarketData.instrumentID );
			const double bondQuote = bondMarketData.instrumentQuote;
			
			// NOTE: The model is updated by this method in order to populate the new calibration point
			const double hazardRate = bondInstrument->hazardRateFromPrice( settlementDate, bondQuote, *this );
			if ( hazardRate < 0.0 )
			{
				// This occurs if two bond maturities are in close proximity with inconsistent prices.
				// In order to match the bond price the survival probability is required to increase between maturities, which is impossible.
				MLIB_THROW( "Detected two bonds with clashing maturities and prices. Consider removing the bond with ISIN '" + bondInstrument->getBondISIN()
							+ "' with maturity '" + bondInstrument->getSchedule()->getMaturityDate().stringWithFormat("DD-MM-YYYY").getCString()
							+ "' or the earlier bond." );
			}
		}
	
	}

	// @brief	Called by constructors to calibrate hazard rates from the provided market data
	void CreditModel::calibrate()
	{
		// Read Credit Model properties
		LabelValueBlock modelProperties = toLabelValueBlock( toString( MODEL_PROPERTIES ) );

		asOfDate_					= modelProperties.getCompulsoryValueAsDate( CREDITMODEL_MODEL_PROPERTIES_KEY::ASOF_DATE );
		spotLag_					= modelProperties.getCompulsoryValue( CREDITMODEL_MODEL_PROPERTIES_KEY::SPOT_LAG );
		spotBusinessDayAdjustment_	= modelProperties.getCompulsoryValue( CREDITMODEL_MODEL_PROPERTIES_KEY::SPOT_BUSINESSDAY_ADJUSTMENT );
		spotCalendar_				= modelProperties.getCompulsoryValue( CREDITMODEL_MODEL_PROPERTIES_KEY::SPOT_CALENDAR );
		accrualStartDate_			= modelProperties.getOptionalValueAsDate( CREDITMODEL_MODEL_PROPERTIES_KEY::ACCRUAL_START_DATE ); 
		immReferenceDate_			= modelProperties.getOptionalValueAsDate( CREDITMODEL_MODEL_PROPERTIES_KEY::IMM_REFERENCE_DATE );
		cdsCurveCollection_			= modelProperties.getOptionalValueAsString( CREDITMODEL_MODEL_PROPERTIES_KEY::CDS_CURVE_COLLECTION, "" );
		currency_					= toCCYEnum( modelProperties.getCompulsoryValue( CREDITMODEL_MODEL_PROPERTIES_KEY::CURRENCY ) ); 
		recoveryRate_               = modelProperties.getCompulsoryValueAsDouble( CREDITMODEL_MODEL_PROPERTIES_KEY::RECOVERY_RATE );
		includeAccruedInterest_     = modelProperties.getCompulsoryValueAsBool( CREDITMODEL_MODEL_PROPERTIES_KEY::INCLUDE_ACCRUED_INTEREST );

		creditIndex_                = modelProperties.getCompulsoryValue( CREDITMODEL_MODEL_PROPERTIES_KEY::CREDIT_INDEX );
		interpolationMethod_        = modelProperties.getCompulsoryValue( CREDITMODEL_MODEL_PROPERTIES_KEY::INTERPOLATION );
		extrapolationMethod_        = modelProperties.getCompulsoryValue( CREDITMODEL_MODEL_PROPERTIES_KEY::EXTRAPOLATION );

        cdsGeneratorName_           = modelProperties.getCompulsoryValue( CREDITMODEL_MODEL_PROPERTIES_KEY::SWAP_GENERATOR );
		auto cdsGenerator           = etrading::getSwapGenerator( cdsGeneratorName_ );
		accrualDayCount_			= getAccrualDayCountFromGenerator( cdsGenerator );

		bondCurveCollection_		= modelProperties.getOptionalValueAsString( CREDITMODEL_MODEL_PROPERTIES_KEY::BOND_CURVE_COLLECTION, "" );
		bondDiscountCurve_			= modelProperties.getOptionalValueAsString( CREDITMODEL_MODEL_PROPERTIES_KEY::BOND_DISCOUNT_CURVE, "OIS" );
		bondHasRiskyAccruedInterest_= modelProperties.getOptionalValueAsBool( CREDITMODEL_MODEL_PROPERTIES_KEY::BOND_HAS_RISKY_ACCRUED_INTEREST, true );

        MLIB_REQUIRE( accrualStartDate_ != LADate() || immReferenceDate_ != LADate(), "Accrual Start Date Required: We must specify either the AccrualStartDate or the active IMMReferenceDate" )

		if ( ! boost::iequals( interpolationMethod_, "PiecewiseConstant" ))
		{
			MLIB_THROW( "Only PiecewiseConstant interpolation is supported.");
		}

		if ( ! boost::iequals( extrapolationMethod_, "Flat" ))
		{
			MLIB_THROW( "Only Flat extrapolation is supported." );
		}

		// Perform consistency checks between the Credit Model and the cdsCurveCollection
		if ( cdsCurveCollection_.size() > 0 )
		{
			const std::string curveCurrency = getCurveCurrency( cdsCurveCollection_.c_str() ).getCString();
			if ( ! boost::iequals( curveCurrency, toString( currency_ ) ))
			{
				MLIB_THROW( "CDSCurveCollection currency does not match Credit Model currency: '" + curveCurrency + "' vs '" + toString( currency_ ) + "." );
			}

			const LADate curveAsOfDate = getCurveAsOfDate( cdsCurveCollection_.c_str() );
			if ( curveAsOfDate != asOfDate_ )
			{
				MLIB_THROW( "CDSCurveCollection as-of date does not match Credit Model as-of date: '" + curveAsOfDate.stringWithFormat() + "' vs '" + asOfDate_.stringWithFormat() + "." );
			}
		}

		if ( bondCurveCollection_.size() > 0 )
		{
			const LADate bondCurveAsOfDate = getCurveAsOfDate( bondCurveCollection_.c_str() );
			if ( bondCurveAsOfDate != asOfDate_ )
			{
				MLIB_THROW( "BondCurveCollection as-of date does not match Credit Model as-of date: '" + bondCurveAsOfDate.stringWithFormat() + "' vs '" + asOfDate_.stringWithFormat() + "." );
			}
		}

		loadMarketDataAndCalibrate();
	}

	// @brief	Called by main calibration routine. Allows an optional parallel bump to be applied to CDS spreads
	void CreditModel::loadMarketDataAndCalibrate( const double cdsSpreadBump )
	{
		// Calculate a referenceDate for CDS calibration instruments.
		// If an accrualStartDate is provided then use this. Otherwise default to the model asOfDate
		const LADate asOfDate = accrualStartDate_ == LADate() ? asOfDate_ : accrualStartDate_;
		// Use the immReferenceDate data if provided, otherwise use the asOfDate calculated earlier.
		const LADate referenceDate = immReferenceDate_ == LADate() ? asOfDate : immReferenceDate_;

		// Read the market data, sort maturities in chronological order and perform sanity checks
		const MarketDataMap cdsMarketDataMap = loadMarketDataMap( referenceDate, CDS_MARKETDATA );
		const MarketDataMap bondMarketDataMap = loadMarketDataMap( referenceDate, BOND_MARKETDATA );
		if ( cdsMarketDataMap.size() == 0 && bondMarketDataMap.size() == 0 )
		{
			MLIB_THROW("Missing calibration data: Please provide either a CDS_MARKETDATA block or BOND_MARKETDATA block.");
		}
		if ( cdsMarketDataMap.size() > 0 && bondMarketDataMap.size() > 0 )
		{
			MLIB_THROW("Conflicting calibration data: Please provide either a CDS_MARKETDATA block or BOND_MARKETDATA block, not both.");
		}

		hazardRates_.clear();

		calibrateToCDS( asOfDate, cdsMarketDataMap, cdsSpreadBump );

		calibrateToBonds( bondMarketDataMap );

		postCalibrationSetup();
	}


	/* @brief	Performs final setup after the calibration step.
	*			In particular, this method pre-calculates survival probabilities
	*			in order to make getImpliedSurvivalDate() run fast
	*/
	void CreditModel::postCalibrationSetup()
	{
		calibrationDates_.clear();
		survivalProbabilitiesOnCalibrationDates_.clear();
		hazardRatesVector_.clear();

		LADate periodEndDate   = asOfDate_;
		calibrationDates_.push_back( periodEndDate );

		double survivalProbability = 1.0;
		survivalProbabilitiesOnCalibrationDates_.push_back( survivalProbability );

		for (auto it = hazardRates_.begin(); it != hazardRates_.end(); ++it )
		{
			periodEndDate = it->first;
			double hazardRate = it->second;

			survivalProbability = getSurvivalProbability( periodEndDate );

			calibrationDates_.push_back( periodEndDate );
			survivalProbabilitiesOnCalibrationDates_.push_back( survivalProbability );
			hazardRatesVector_.push_back( hazardRate );
		}
	}

	/*
	*  @brief  validates the property keys of this CreditModel, to verify that all are recognized key names.
	*          Will throw if one of the keys is not expected.
	*/
	void CreditModel::validateKeys() const
	{
		const std::vector<std::string>& propertyNames = freeObject_.keyNames();

		const bool validateKeys = true;
		for ( size_t i = 0; i < propertyNames.size(); i++)
		{
			const std::string& propertyName = propertyNames[i];
			LabelValueBlock inputLVB = toLabelValueBlock( propertyName );

			etrading::CreditModelEnum CreditModelEnum = etrading::toCreditModelEnum( propertyName );
			switch (CreditModelEnum)
			{
				case MODEL_PROPERTIES:
					validateKeysForLVB( model_properties_lvbKeys(), inputLVB.getKeys(), validateKeys, propertyName );
					break;

				default:
					break;
			}
		}

	}


	/* @brief Used to serialize an instance of this class
	 * @param[out] the populated SchemaObject
	 */
	const SchemaObject CreditModel::toSchemaObject() const
	{
		SchemaObject schemaObject( CREDIT_MODEL, getRefToName());
        toSchemaObject(schemaObject);

        return schemaObject;
	}

    // This is a helper method to populate the supplied SchemaObject
    void const CreditModel::toSchemaObject(SchemaObject& schemaObject) const
   	{
        
		const std::vector<std::string> keyNames = freeObject_.keyNames();
		const std::vector<std::vector<Variant> >& allDataView = freeObject_.viewAllData();

		for (size_t i=0; i < freeObject_.numberOfSchemas(); i++)
		{
			schemaObject.addDataSchema(freeObject_.viewSchema(i));

			const std::string& propertyName = keyNames[i];
			const auto columnIndices = freeObject_.columnsOfSchema( propertyName );
			VariantMatrix variantMatrix;
			std::for_each( columnIndices.cbegin(), columnIndices.cend(), [&variantMatrix, &allDataView]( const int idx )
            {
				variantMatrix.push_back( allDataView[ idx ] );
            } );

			schemaObject.setDataForSchema(propertyName, variantMatrix);
		}
	}

	/* @brief Returns the configuration information for the specified propertyKey. If propertyKey is blank, all properties are returned.
	* @param [in]   propertyKey   The property to be displayed
	* @param [out]  A VariantMatrix containing a LabelValue block of properties
	*/
	const VariantMatrix CreditModel::viewInputParameters(const std::string& propertyKey ) const
	{
		// If no propertyKey is specified, Return a matrix containing all properties
		if (propertyKey == "" || propertyKey == "NONE")
		{
			VariantMatrix inputParameters;

			bool addSeparatorRow = false;
			const std::vector<std::string> keyNames = freeObject_.keyNames();
			for (auto it = keyNames.begin(); it != keyNames.end(); ++it)
			{
				if ( addSeparatorRow )
				{
					std::vector<Variant> row(2, "");
					inputParameters.push_back( row );
				}
				addSeparatorRow = true; // Add a separator before all but the first row

				std::string propertyKey = *it;
				std::vector<Variant> row(2, "");
				row[0] = propertyKey;
				inputParameters.push_back( row );

				VariantMatrix variantMatrix = transpose( getVariantMatrixFromFreeObject ( freeObject_, propertyKey ) );
				for (unsigned int i = 0; i < variantMatrix.size(); i++)
				{
					inputParameters.push_back( variantMatrix[i] );
				}
			}
			return transpose( inputParameters );
		}

		// extract just the data for the specified property
		VariantMatrix inputParameters = getVariantMatrixFromFreeObject ( freeObject_, propertyKey );
		return inputParameters;
	}

	/* @brief Returns a LabelValue block containing the configuration information for the specified propertyKey
	* @param [in]   propertyKey   The property to be displayed
	* @param [out]  A LabelValue block containing the properties
	*/
	LabelValueBlock CreditModel::toLabelValueBlock( const std::string& propertyKey ) const
	{
		LAStringMatrix stringMatrix = getLAStringMatrixFromFreeObject( freeObject_, propertyKey );
		LabelValueBlock lvb( stringMatrix );

		return lvb;
	}

	// Calculates the hazardRate for the specified payment date
	double CreditModel::getHazardRate( const LADate& paymentDate ) const
	{
		if ( hazardRates_.empty() )
		{
			MLIB_THROW( "No hazard rates have been set." );
		}

		// Return iterator to first date which is not less than paymentDate
		auto iter = hazardRates_.lower_bound( paymentDate );

		if ( iter == hazardRates_.end() )
		{
			// Requested payment date is beyond the end of the calculated hazardRates. Extrapolate
			double hazardRate = hazardRates_.rbegin()->second;
			return hazardRate;
		}

		// Support only piecewise constant for the first version
		double hazardRate = iter->second;
		return hazardRate;
	}

	bool CreditModel::getIncludeAccruedInterest() const
	{
		return includeAccruedInterest_;
	}

	/* @brief	Whether to include the probability of receiving the next coupon, when
	*			calculating the Accrued Interest.
	*			true  = multiply the raw accrued interest by the survival probability
	*			false = return the raw accrued interest. 
	*/
	bool CreditModel::bondHasRiskyAccruedInterest() const
	{
		return bondHasRiskyAccruedInterest_;
	}

	double CreditModel::getRecoveryRate() const
	{
		return recoveryRate_;
	}

	/* @brief Updates the hazard rate calibration stored in the model by adding a hazard rate point defined for the specified paymentDate
	*        This method intended to be used by the calibration process while boostrapping the hazard rates from input CDS spreads.
	* 
	* @param [in]   paymentDate	The payment date corresponding to this hazard rate
	* @param [in]   hazardRate		The estimate of the hazard rate for this payment date
	*/
	void CreditModel::setCalibrationPoint( const LADate& paymentDate, const double hazardRate )
	{
		hazardRates_[ paymentDate ] = hazardRate;
	}

	// Returns a matrix containing payment dates and hazard rates calibrated from the input market data
	AnyTypeMatrix CreditModel::getCalibrationParameters( ) const
	{
		AnyTypeMatrix calibrationParameters;
	
		for ( auto it = hazardRates_.begin(); it != hazardRates_.end(); ++it)
		{
			AnyTypeVector row;
			const LADate maturityDate = it->first;
			const double hazardRate = it->second;

			const int dateAsInt = static_cast<long long> (LADateScheduleHelpers::getExcelDate( maturityDate ));
			row.push_back( dateAsInt );
			row.push_back( hazardRate );

			const double survivalProbability = getSurvivalProbability( maturityDate );
			row.push_back( survivalProbability );

			// Calculate the unconditional defaultProbability from asOfDate to the maturity of this calibration point
			const double defaultProbability = 1 - survivalProbability;
			row.push_back( defaultProbability );

			calibrationParameters.push_back( row );
		}

		return calibrationParameters;
	}

	/* @brief Validate that the supplied toDate and fromDate are consistent with each other and the model asOfDates
	 * @param [in]	toDate		The initial date for survival probability calculations
	 * @param [in]	fromDate	The final date for survival probability calculations
	 */
	void CreditModel::validateDates( const LADate& toDate, const LADate& fromDate ) const
	{
		// If fromDate is specified, perform sanity checks
		if ( fromDate != LADate() )
		{
			if ( fromDate  < asOfDate_ )
			{
				MLIB_THROW( "Invalid fromDate '" + toDate.stringWithFormat() + "' is earlier than model asOfDate '" + asOfDate_.stringWithFormat() + "'." );
			}
			if ( toDate < fromDate )
			{
				MLIB_THROW( "Invalid toDate '" + toDate.stringWithFormat() + "' is earlier than fromDate '" + fromDate.stringWithFormat() + "'." );
			}
		}
	}


	/* @brief Calculate the probability of survival to "toDate", given the contract has already survived up to "fromDate".
	*  @param[in]	toDate	The future end date to use in the calculation. Must occur after the model as-of date.
	*  @param[in]	fromDate	The future start date i.e. the probability of survival is 1 on this date.
	*							This parameter is allowed to be an empty date i.e. an optional paramweter.
	*  @returns	The survival probability
	*/
	double CreditModel::getSurvivalProbability( const LADate& toDate, const LADate& fromDate ) const
	{
		validateDates( toDate, fromDate );

		double survivalProbability = getSurvivalProbability( toDate );

		// fromDate is optional: it is OK for it to contain a default empty date
		if ( fromDate != LADate() )
		{
			const double survivalFrom = getSurvivalProbability( fromDate );
			survivalProbability /= survivalFrom;
		}
		return survivalProbability;
	}

	/* @brief Given a future date, calculates the probability of survival to that date
	*  @param[in]	toDate	The future date to use in the calculation. Must occur after the model as-of date.
	*  @returns	The survival probability
	*/
	double CreditModel::getSurvivalProbability( const LADate& toDate ) const
	{
		if ( hazardRates_.empty() )
		{
			MLIB_THROW( "No hazard rates have been set." );
		}

		if ( toDate < asOfDate_ )
		{
			return 1.0;
		}

		// Integrate the hazard Rate from asOfDate to toDate
		// Assumes PiecewiseConstant interpolation between date points

		double survivalProbability = 1.0;

		LADate periodBeginDate = asOfDate_;
		LADate periodEndDate   = asOfDate_;
		double hazardRate = std::numeric_limits<double>::quiet_NaN();
		for (auto it = hazardRates_.begin(); it != hazardRates_.end(); ++it )
		{
			periodEndDate = it->first;
			hazardRate = it->second;

			if ( periodEndDate <= toDate )
			{
				double yearFraction = getYearFraction( periodBeginDate, periodEndDate, accrualDayCount_, false);	
				double survivalFactor = exp( - hazardRate * yearFraction );
				survivalProbability *= survivalFactor;
				
				// Requested date is exactly on the end of one of the periods
				if ( periodEndDate == toDate )
				{
					break;
				}
			}
			else
			{
				// toDate is before the periodEndDate
				double yearFraction = getYearFraction( periodBeginDate, toDate, accrualDayCount_, false);	
				double survivalFactor = exp( - hazardRate * yearFraction );
				survivalProbability *= survivalFactor;
				break;
			}
			periodBeginDate = periodEndDate;
		}
		
		if ( periodEndDate < toDate )
		{
			// Extrapolate-flat in hazardRate
			double yearFraction = getYearFraction( periodEndDate, toDate, accrualDayCount_, false);	
			double survivalFactor = exp( - hazardRate * yearFraction );
			survivalProbability *= survivalFactor;
		}
		
		return survivalProbability;
	}

	/* @brief	Calculate the probability of default in the time period defined by "fromDate" and "toDate".
	*  @param[in]	toDate	The future end date to use in the calculation. Must occur after the model as-of date.
	*  @param[in]	fromDate	The future period start date
	*
	*  @returns	The survival probability
	*/
	double CreditModel::getDefaultProbability( const LADate& toDate, const LADate& fromDate ) const
	{
		validateDates( toDate, fromDate );

		// We allow a missing value for 'fromDate'. In this case we default to 'asOfDate'.
		LADate fromDt = ( fromDate == LADate() ) ? asOfDate_ : fromDate;

		double defaultProbability = getSurvivalProbability( fromDate ) - getSurvivalProbability( toDate );
		return defaultProbability;
	}

	/* @brief	Given a survival probability, calculate the implied survival date
	*			i.e. this function is the inverse of getSurvivalProbability().
	*  @param[in]	survivalProbability		The input survivalProbability
	*
	*  @returns		The survival date corresponding to the input survivalProbability.
	*/
	LADate CreditModel::getImpliedSurvivalDate( const double targetSurvivalProbability ) const
	{
		if ( targetSurvivalProbability == 1.0 )
		{
			return asOfDate_;
		}

		MLIB_REQUIRE( survivalProbabilitiesOnCalibrationDates_.size() > 1, "Credit Model has not been correctly calibrated.");

		/* Find bracketing survivalProbabilty point.
		 * i.e. find the survivalProbability greater than or equal to the targetSurvivalProbability
		 *
		 * NOTE: We use rbegin() rend() iterators since the survivalProbabilities decay over time,
		 * and lower_bound requires a monotonically increasing array
		 */
		auto it = std::lower_bound( survivalProbabilitiesOnCalibrationDates_.rbegin(), survivalProbabilitiesOnCalibrationDates_.rend(), targetSurvivalProbability );

		const size_t distanceFromFinalCalibrationPoint = std::distance( survivalProbabilitiesOnCalibrationDates_.rbegin(), it );
		const size_t index = hazardRates_.size() - distanceFromFinalCalibrationPoint;
		const LADate dateAtBeginningOfPeriod = calibrationDates_[ index ];
		const double survivalProbabilityAtBeginningOfPeriod = survivalProbabilitiesOnCalibrationDates_[ index ];
		const double survivalFactor = targetSurvivalProbability / survivalProbabilityAtBeginningOfPeriod;
		
		// TODO: Check for extrapolation
		const double hazardRateForPeriod = ( index  < hazardRatesVector_.size() ) ? hazardRatesVector_[ index ] : hazardRatesVector_[ hazardRatesVector_.size() - 1 ];
		const double yearFractionForPeriod = - log( survivalFactor ) / hazardRateForPeriod;
		const LADate impliedSurvivalDate = getDateFromYearFraction( dateAtBeginningOfPeriod, yearFractionForPeriod, accrualDayCount_ );

		return impliedSurvivalDate;
	}

	/* @brief	Given a maturity date, calculates the CDS par-spread and risky annuity implied by the credit model
	*  @param[in]	maturityDate	The maturity date of a CDS
	*  @param[out]	parSpread		The par spread implied by the credit model
	*  @param[out]	riskyAnnuity	The risky annuity implied by the credit model
	*/
	void CreditModel::getParSpreadAndRiskyAnnuityForDate( const LADate& maturityDate, double& parSpread, double& riskyAnnuity ) const
	{
		// If an accrualStartDate is provided then use this. Otherwise default to the model asOfDate
		const LADate asOfDate = accrualStartDate_ == LADate() ? asOfDate_ : accrualStartDate_;

		const LADate effectiveDate = LADateScheduleHelpers::getDate( asOfDate, spotLag_.c_str(), spotBusinessDayAdjustment_.c_str(), spotCalendar_.c_str() );

		// Check if the maturityDate has already passed
		if (maturityDate <= effectiveDate )
		{
			parSpread = 0.;
			riskyAnnuity = 0.;
			return;
		}

		auto cdsInstrument = createCalibrationCDSWithSpecifiedMaturity( effectiveDate, maturityDate );

		LAString premiumLegName		= cdsInstrument->getLeg(0)->getLegName();
		LAString protectionLegName	= cdsInstrument->getLeg(1)->getLegName();

		parSpread		= cdsInstrument->parSpread( *this, premiumLegName, protectionLegName );
		riskyAnnuity	= cdsInstrument->riskyAnnuity( *this, premiumLegName );
	}


	/* @brief	Given a start and end date, calculates the forward spread and risky annuity implied by the credit model.
	*			See article "Credit Derivatives Handbook" by JPM  p20-21.

	*  @param[in]	startDate	The start date of the forward contract
	*  @param[in]	endDate		The end date of the forward contract
	*  @returns		The forward spread
	*/
	void CreditModel::getForwardSpreadAndRiskyAnnuity( const LADate& startDate, const LADate& endDate, double& forwardSpread, double& riskyAnnuity ) const
	{
		double parRateStart;
		double riskyAnnuityStart;
		getParSpreadAndRiskyAnnuityForDate( startDate, parRateStart, riskyAnnuityStart );

		double parRateEnd;
		double riskyAnnuityEnd;
		getParSpreadAndRiskyAnnuityForDate( endDate, parRateEnd, riskyAnnuityEnd );

		riskyAnnuity  = ( riskyAnnuityEnd - riskyAnnuityStart );

		forwardSpread = ( parRateEnd * riskyAnnuityEnd - parRateStart * riskyAnnuityStart ) / riskyAnnuity;
	}

	/* @brief	Given a start and end date, calculates the forward spread implied by the credit model.
	*			See article "Credit Derivatives Handbook" by JPM p20-21.
	*  @param[in]	startDate	The date when credit protection begins. If this is the model asOf Date,
	*							the function will return the spot credit spread.
	*  @param[in]	endDate		The date when credit protection ends
	*  @returns		The credit spread
	*/
	double CreditModel::getForwardSpread( const LADate& startDate, const LADate& endDate ) const
	{
		MLIB_REQUIRE( endDate > startDate, "End Date of forward calculation must occur after Start Date");

		double forwardSpread;
		double riskyAnnuity;
		getForwardSpreadAndRiskyAnnuity( startDate, endDate, forwardSpread, riskyAnnuity );

		return forwardSpread;
	}

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
	double CreditModel::getIndexForwardSpread( const LADate& startDate, const LADate& endDate ) const
	{
		// If an accrualStartDate is provided then use this. Otherwise default to the model asOfDate
		const LADate asOfDate = accrualStartDate_ == LADate() ? asOfDate_ : accrualStartDate_;

		// To match BBG, do not make businessDayAdjustment or calendar adjustment
		const std::string businessDayAdjustment("");
		const std::string calendar("");
		const LADate effectiveDate = LADateScheduleHelpers::getDate( asOfDate, spotLag_.c_str(), businessDayAdjustment.c_str(), calendar.c_str() );

		auto cdsIndexStartingImmediately	= createCalibrationCDSWithSpecifiedMaturity( effectiveDate, endDate );
		auto cdsIndexStartingAtOptionExpiry	= createCalibrationCDSWithSpecifiedMaturity( startDate, endDate );

		const std::string premiumLegName	= "Leg1:Premium";
		const std::string protectionLegName	= "Leg2:Protection";

		const double protectionPV	= cdsIndexStartingImmediately->pv( *this, protectionLegName );
		const double parSpread		= cdsIndexStartingImmediately->parSpread( *this, premiumLegName, protectionLegName );

		MLIB_REQUIRE((recoveryRate_ >= 0 && recoveryRate_ < 1.0), "Require the recovery rate to be positive and strictly less than 1.0");
		const double flatHazardRate = parSpread / ( 1 - recoveryRate_ );

		LabelValueBlock valuationSettingsLVB( VALUATION_SETTING_KEYS::CURVE_COLLECTION, cdsCurveCollection_.c_str() );
		const double forwardRiskyAnnuity	= cdsIndexStartingAtOptionExpiry->riskyAnnuityFromHazardRate( valuationSettingsLVB, flatHazardRate, recoveryRate_, premiumLegName, includeAccruedInterest_ );

		const double indexForwardSpread = protectionPV / forwardRiskyAnnuity;

		return indexForwardSpread;
	}


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
	double CreditModel::getSingleNameKnockoutOptionValue( const PayerReceiverSwaptionEnum payerReceiverSwaptionEnum, const double strike, const LADate& optionExpiryDate, const LADate& cdsMaturityDate, const double volatility ) const
	{
		double forwardSpread;
		double riskyAnnuity;
		getForwardSpreadAndRiskyAnnuity( optionExpiryDate, cdsMaturityDate, forwardSpread, riskyAnnuity );

		const double timeToExpiry = getYearFraction( asOfDate_, optionExpiryDate, accrualDayCount_, false );

		/* When calling the swaption pricer we pass in the forwardSpread in place of the "swapRate" parameter.
		*  This works because the logNormal pricer sets the Black discount rate and carry equal to zero;
		*  The Black formula therefore simplifies to that given in terms of the forward.
		*/
		const double optionValue = EuropeanIRSwaption::lognormalPrice( payerReceiverSwaptionEnum, riskyAnnuity, forwardSpread, strike, volatility, timeToExpiry );

		return optionValue;
	}

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
	double CreditModel::getOptionValueFromForward( const PayerReceiverSwaptionEnum payerReceiverSwaptionEnum, const double strike, const LADate& optionExpiryDate, const LADate& cdsStartDate, const LADate& cdsMaturityDate, const double volatility, const double forwardSpread ) const
	{

		double forward;
		double riskyAnnuity;
		getForwardSpreadAndRiskyAnnuity( cdsStartDate, cdsMaturityDate, forward, riskyAnnuity);

		const double timeToExpiry = getYearFraction( asOfDate_, optionExpiryDate, accrualDayCount_, false);

		/* When calling the swaption pricer we pass in the forwardSpread in place of the "swapRate" parameter.
		*  This works because the logNormal pricer sets the Black discount rate and carry equal to zero;
		*  The Black formula therefore simplifies to that given in terms of the forward.
		*/
		const double optionValue = EuropeanIRSwaption::lognormalPrice( payerReceiverSwaptionEnum, riskyAnnuity, forwardSpread, strike, volatility, timeToExpiry );

		return optionValue;
	}

	/* @brief Calculates the value of a credit index option i.e. option on a credit index.
	*  See Bloomberg Whitepaper "Pricing Credit Index Options", March 1st, 2012
	*
	* @param[in]	optionLVB					Option parameters such as strike, expiryDate, cdsCoupon, volatility
	* @returns	The calculated option price
	*/
	double CreditModel::getIndexOptionValue( const LabelValueBlock& optionLVB )
	{
		CreditOptionParameters params = getCreditOptionParameters( optionLVB );
		return getIndexOptionValue( params, params.forwardSpread_, params.volatility_ );
	}

	/* @brief Calculates the value of a credit index option i.e. option on a credit index.
	*  See Bloomberg Whitepaper "Pricing Credit Index Options", March 1st, 2012
	*
	* @param[in]	params						Contract and market data parameters used to price an index option contract
	* @param[in]	forwardSpread				The forward spread at the option expiry date
	* @param[in]	volatility					The volatility of the underlying CDS spread
	* @param[in]	applyThetaBump				If true, reduce the time to option expiry by 1 calendar day.
	* @returns	The calculated option price
	*/
	double CreditModel::getIndexOptionValue( const CreditOptionParameters& params, const double forwardSpread, const double volatility,  const bool applyThetaBump ) const
	{
		MLIB_REQUIRE( params.payerReceiverSwaptionEnum_ == PAYER_SWAPTION || params.payerReceiverSwaptionEnum_ == RECEIVER_SWAPTION, "Unsupported swaption Payer/Receiver type" + toString(params.payerReceiverSwaptionEnum_));
		const double payerReceiverIndicator = params.payerReceiverSwaptionEnum_ == PAYER_SWAPTION ? 1.0 : -1.0;

		// 1. Construct underlying CDS Index instrument
		auto cdsIndex				= createCalibrationCDSWithSpecifiedMaturity( params.cdsStartDate_, params.cdsMaturityDate_ );
		LAString premiumLegName		= cdsIndex->getLeg(0)->getLegName();
		LAString protectionLegName	= cdsIndex->getLeg(1)->getLegName();

		// 2. Calculate Risky Annuity at Strike
		MLIB_REQUIRE ( (recoveryRate_ >=0 && recoveryRate_ < 1.0), "Require the recovery rate to be positive and strictly less than 1.0" );
		const double hazardRateAtStrike = params.strike_ / ( 1 - recoveryRate_ );
		LabelValueBlock valuationSettingsLVB(VALUATION_SETTING_KEYS::CURVE_COLLECTION, cdsCurveCollection_.c_str());
		const double riskyAnnuityAtStrike = cdsIndex->riskyAnnuityFromHazardRate(valuationSettingsLVB, hazardRateAtStrike, recoveryRate_, premiumLegName, includeAccruedInterest_);

		const double survivalProbabilityToOptionExerciseDate = getSurvivalProbability( params.optionExpiryDate_ );
		const double adjustedRiskyAnnuityAtStrike = params.scaleStrikeBySurvivalProbability_ ? riskyAnnuityAtStrike / survivalProbabilityToOptionExerciseDate : riskyAnnuityAtStrike;
		
		// 3. Calibrate to the forward spread
		const double timeToOptionExpiry = getYearFraction(asOfDate_, params.optionExpiryDate_, accrualDayCount_, false);
		const double drift = -0.5 * volatility * volatility * timeToOptionExpiry;
		const double volatilityTime = volatility * sqrt(timeToOptionExpiry);

		const bool assumeFlatCurve = false;
		const double targetForwardPrice = cdsIndex->pvFromSpread( *this, forwardSpread, params.cdsCoupon_, assumeFlatCurve );

		const double meanSpread = cdsIndex->calibrateLogNormalMeanSpread( *this, targetForwardPrice, drift, volatilityTime, params.cdsCoupon_ );

		// 3. Handle Realized Defaults
		RealizedDefaults realizedDefaults;
		realizedDefaults.indexSurvivalFactor_ = (double)(params.numberOfUnderliers_ - params.numberOfRealizedDefaults_) / (double)(params.numberOfUnderliers_);
		realizedDefaults.defaultSettlementAmountPV_ = 0.0;

		// Use the override  value in realizedDefaultSettlementAmountOverride; or calculate our own defaultSettlementAmount
		// where we assume the same recoveryRate for each underlier in the index
		realizedDefaults.defaultSettlementAmount_ = std::isnan( params.realizedDefaultSettlementAmountOverride_ ) ?
				(double)( params.numberOfRealizedDefaults_ ) / (double)( params.numberOfUnderliers_ ) * ( 1.0 - recoveryRate_ )
			:	params.realizedDefaultSettlementAmountOverride_;

		if ( realizedDefaults.defaultSettlementAmount_ != 0.0 )
		{
			// Calculate the discount factor at option expiry
			const LAString discountCurve = cdsIndex->getLeg(0)->getStaticData()->getDiscountCurve();
			std::vector<LADate> paymentDates = { params.optionExpiryDate_ };
			std::vector<double> discountFactors = getCurveDiscountFactors( getAsOfDate(), paymentDates, getCDSCurveCollection(), discountCurve );
			const double discountFactorAtOptionExpiry = discountFactors[ 0 ];
			realizedDefaults.defaultSettlementAmountPV_ = realizedDefaults.defaultSettlementAmount_ * discountFactorAtOptionExpiry;
		}

		// 4. Calculate expected option payoff by integration
		
		// Transform the integration limits from minLimit / maxLimit to +/-1 used by Gauss-Legendre integration routine.
		const double minLimit = -7.0;
		const double maxLimit = 7.0;
		const double limitSpread = 0.5 * ( maxLimit - minLimit );
		const double limitMidpoint = 0.5 * ( maxLimit + minLimit );

		/* Bloomberg define the theta to be "Time-decay in option value for a one (calendar) day decrease in option expiry".
		*  To strictly capture the time-decay only, we reduce the time to expiry by one day and hold everything else constant,
		*  including the calibration.
		*/
		double optionPayoffDrift = drift;
		double optionPayoffVolatilityTime = volatilityTime;
		if ( applyThetaBump )
		{
			LADate thetaBumpDate = asOfDate_;
			thetaBumpDate.addDays( 1 ); // 1 calendar day
			const double bumpedTimeToOptionExpiry = getYearFraction( thetaBumpDate, params.optionExpiryDate_, accrualDayCount_, false );
			
			optionPayoffDrift = -0.5 * volatility * volatility * bumpedTimeToOptionExpiry;
			optionPayoffVolatilityTime = volatility * sqrt( bumpedTimeToOptionExpiry );
		}

		auto payoffFunction = [&]( const double integrationVariable ) -> double
		{
			const double normalVariateSample = limitSpread * integrationVariable + limitMidpoint;

			const double payoff = optionPayoffFunction( params, realizedDefaults, cdsIndex, payerReceiverIndicator, meanSpread, normalVariateSample, optionPayoffDrift, optionPayoffVolatilityTime, adjustedRiskyAnnuityAtStrike );

			const double densityFunction = standardNormalDistributionPDF( normalVariateSample );

			const double integrand = limitSpread * payoff * densityFunction;
			return integrand;
		};

		const size_t nIntegrationPoints = 400;
		QuantLib::GaussLegendreIntegration integrator( nIntegrationPoints );

		// NOTE: Annuities are already discounted to valuation date. No further discount factor required.
		const double optionValue = integrator( payoffFunction );

		return optionValue;
	}

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
	double CreditModel::optionPayoffFunction( const CreditOptionParameters& params,
											  const RealizedDefaults& realizedDefaults,
											  const std::shared_ptr<CreditDefaultSwap>& cdsIndex,
											  const double payerReceiverIndicator,
											  const double meanSpread,
											  const double normalVariateSample,
											  const double drift,
											  const double volatilityTime,
											  const double riskyAnnuityAtStrike ) const
	{

		const double spreadSample = cdsIndex->logNormalSpreadProcess( meanSpread, normalVariateSample, drift, volatilityTime );

		const bool assumeFlatCurve = true;
		const double cdsCoupon = params.cdsCoupon_;
		const double indexValue = realizedDefaults.indexSurvivalFactor_ * cdsIndex->pvFromSpread( *this, spreadSample, cdsCoupon, assumeFlatCurve );

		const double adjustedStrike = (cdsCoupon - params.strike_) * riskyAnnuityAtStrike;

		const double totalValue = payerReceiverIndicator * ( indexValue + adjustedStrike ) + realizedDefaults.defaultSettlementAmountPV_;

		const double payoff = std::max( 0.0, totalValue );

		return payoff;
	}

	/* @brief Calculates the implied vol of a credit index option, given a target CDS index option quote and CDS index forward at option expiry
	*  See Bloomberg Whitepaper "Pricing Credit Index Options", March 1st, 2012
	*
	* @param[in]	optionLVB					Option parameters such as strike, expiryDate, cdsCoupon, volatility
	* @returns	The implied volatility
	*/
	double CreditModel::getIndexOptionImpliedVol( const LabelValueBlock& optionLVB ) const
	{
		// Newton-Raphson Solver Settings

		// Is there a sensible initial guess / approximation I can use? Peter Jaeckel has an article "Let's be rational"
		const double initialGuessForImpliedVol = 0.5;
		const double tolerance = 1.0e-12;
		const double shiftSize = 1e-3;
		const unsigned int maxIterations = 20;

		const bool paramsForImpliedVol = true;
		CreditOptionParameters params = getCreditOptionParameters( optionLVB, paramsForImpliedVol );
		const double targetOptionValue = params.optionValue_;

		// One-dimensional objective function used by the solver:
		// This lambda function captures the valuationSettingsLVB and recoveryRate as fixed parameters.
		// The inputHazardRate is the variable which the solver will adjust in order to obtain the targeCdsSpread.
		auto function = [&]( const double inputVolatility ) -> double
		{
			return getIndexOptionValue( params, params.forwardSpread_, inputVolatility ) - targetOptionValue;
		};

		// Solver Results Contain: Solution, nInterations and Jacobian
		const solvers::SolverResults solverResults = solvers::toms748( function, initialGuessForImpliedVol, tolerance, maxIterations );
		const double impliedVol = solverResults.solution;

		return impliedVol;
	}

	/* @brief Calculates the vega of a credit index option i.e. option on a credit index.
	*  See Bloomberg Whitepaper "Pricing Credit Index Options", March 1st, 2012
	*
	* @param[in]	optionLVB					Option parameters such as strike, expiryDate, cdsCoupon, volatility
	* @param[in]	volatilityBump				The volatility bump
	* @returns	The calculated option vega
	*/
	double CreditModel::getIndexOptionVega(const LabelValueBlock& optionLVB, const double volatilityBump ) const
	{
		CreditOptionParameters params = getCreditOptionParameters( optionLVB );
		const double unBumpedPV = getIndexOptionValue( params, params.forwardSpread_, params.volatility_ );
		const double bumpedPV   = getIndexOptionValue( params, params.forwardSpread_, params.volatility_ + volatilityBump );

		/* The vega is defined to be the PV shift for a 1% bump in vol.
		*  Therefore after dividing through by volatilityBump, we scale by 0.01.
		*/
		const double vega = ( bumpedPV - unBumpedPV ) / volatilityBump * 0.01;
		return vega;
	}

	/* @brief Calculates the CS01 of a credit index option i.e. option on a credit index.
	*  i.e. the sensitivity of the credit index option to a parallel flat shift in credit spreads
	*  See Bloomberg Whitepaper "Pricing Credit Index Options", March 1st, 2012
	*
	* @param[in]	optionLVB					Option parameters such as strike, expiryDate, cdsCoupon, volatility
	* @returns	The calculated option CS01
	*/
	double CreditModel::getIndexOptionCS01( const LabelValueBlock& optionLVB ) const
	{
		const double spreadBump = 1.0 / 1e4;  // 1bp

		CreditOptionParameters params = getCreditOptionParameters( optionLVB );

		const double unbumpedForward = getIndexForwardSpread( params.cdsStartDate_, params.cdsMaturityDate_ );
		const double unBumpedPV = getIndexOptionValue( params, unbumpedForward, params.volatility_ );

		auto bumpedCreditModel = this->clone();
		bumpedCreditModel->loadMarketDataAndCalibrate( spreadBump );

		const double bumpedForward = bumpedCreditModel->getIndexForwardSpread( params.cdsStartDate_, params.cdsMaturityDate_ );
		const double bumpedPV = bumpedCreditModel->getIndexOptionValue( params, bumpedForward, params.volatility_ );

		const double cs01 = bumpedPV - unBumpedPV;
		return cs01;
	}

	/* @brief Calculates the theta of a credit index option i.e. option on a credit index.
	*  See Bloomberg Whitepaper "Pricing Credit Index Options", March 1st, 2012
	*  Bloomberg define the theta to be "Time-decay in option value for a one (calendar) day decrease in option expiry".
	*  To strictly capture the time-decay only, we reduce the time to expiry by one day and hold everything else constant,
	*  including the calibration.
	*
	* @param[in]	optionLVB					Option parameters such as strike, expiryDate, cdsCoupon, volatility
	* @returns	The calculated option theta
	*/
	double CreditModel::getIndexOptionTheta( const LabelValueBlock& optionLVB ) const
	{
		CreditOptionParameters params = getCreditOptionParameters( optionLVB );
		const double unBumpedPV = getIndexOptionValue( params, params.forwardSpread_, params.volatility_ );
		
		const bool applyThetaBump = true;
		const double bumpedPV   = getIndexOptionValue( params, params.forwardSpread_, params.volatility_,  applyThetaBump );

		const double theta = bumpedPV - unBumpedPV;
		return theta;
	}

	/* @brief	Creates a CDS Instrument from conventions stored in the curve's SwapGenerator.
	*			Used in calibration and option pricing
	*
	*  @param[in]	cdsStartDate	Create a CDS with this effective date
	*  @param[in]	cdsMaturityDate	Create a CDS with this maturity date
	*  @returns		A CreditDefaultSwap object
	*/
	std::shared_ptr<CreditDefaultSwap> CreditModel::createCalibrationCDSWithSpecifiedMaturity( const LADate& cdsStartDate, const LADate& cdsMaturityDate ) const
	{
		LabelValueBlock swapExpressionLVB(  setupSwapExpressionLVBforCalibration(),
											IRS_KEY::EFFECTIVE_DATE,
											std::to_string(static_cast<long long>(LADateScheduleHelpers::getExcelDate( cdsStartDate ))));

		LabelValueBlock swapExpressionLVBWithMaturity = LabelValueBlock(swapExpressionLVB,
																		IRS_KEY::MATURITY_DATE,
																		std::to_string(static_cast<long long>(LADateScheduleHelpers::getExcelDate( cdsMaturityDate ))));

		// Additional parameters for creating CDS instrument
		const bool isXccySwap = false;
		LabelValueBlock swapPropertiesLVB;
		auto swapInstrument = createSwapFromGenerator( creditIndex_, cdsGeneratorName_, swapExpressionLVBWithMaturity, swapPropertiesLVB, isXccySwap );
		std::shared_ptr<CreditDefaultSwap> cdsInstrument = std::dynamic_pointer_cast<CreditDefaultSwap>( swapInstrument );

		return cdsInstrument;
	}

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
	double CreditModel::getImpliedVol( const PayerReceiverSwaptionEnum payerReceiverSwaptionEnum, const double strike, const LADate& optionExpiryDate, const LADate& cdsMaturityDate, const double targetOptionValue ) const
	{
		double forwardSpread;
		double riskyAnnuity;
		getForwardSpreadAndRiskyAnnuity( optionExpiryDate, cdsMaturityDate, forwardSpread, riskyAnnuity );

 		// Is there a sensible initial guess / approximation I can use? Peter Jaeckel has an article "Let's be rational"
		const double initialGuessForVolatility = 0.2;
		
		const double timeToExpiry = getYearFraction(asOfDate_, optionExpiryDate, accrualDayCount_, false);

		EuropeanIRSwaption europeanSwaption( payerReceiverSwaptionEnum, riskyAnnuity, forwardSpread, strike, initialGuessForVolatility, timeToExpiry );

		const double impliedVol = europeanSwaption.calculateImpliedVol( targetOptionValue );

		return impliedVol;
	}

	/* @brief Calculates the implied vol of a credit option, given a target CDS option quote and CDS forward at option expiry
	*  See article "The Valuation of Credit Default Swap Options" by Hull and White
	*
	* @param[in]	payerReceiverSwaptionEnum	Whether the option is a payer (PUT, the right to sell risk, pay CDS spread for protection)
	* 											or receiver (CALL, buy risk, receive CDS spread)
	* @param[in]	strike						The strike spread
	* @param[in]	optionExpiryDate			The expiry date of the option; the effective date of the underlying CDS
	* @param[in]	cdsMaturityDate				The maturity date of the underlying CDS
	* @param[in]	targetOptionValue			Calculate the implied vol for this target option value
	* @param[in]	forwardSpread				The forward CDS spread at option expiry
	* @returns	The implied volatility
	*/
	double CreditModel::getImpliedVolFromForward( const PayerReceiverSwaptionEnum payerReceiverSwaptionEnum, const double strike, const LADate& optionExpiryDate, const LADate& cdsStartDate, const LADate& cdsMaturityDate, const double targetOptionValue, const double forwardSpread ) const
	{
		// Newton-Raphson Solver Settings

		// Is there a sensible initial guess / approximation I can use? Peter Jaeckel has an article "Let's be rational"
		const double initialGuessForImpliedVol = 0.2;
		const double tolerance = 1.0e-12;
		const double shiftSize = 1e-10;;
		const unsigned int maxIterations = 1000;

		// One-dimensional objective function used by the solver:
		// This lambda function captures the valuationSettingsLVB and recoveryRate as fixed parameters.
		// The inputHazardRate is the variable which the solver will adjust in order to obtain the targeCdsSpread.
		auto function = [payerReceiverSwaptionEnum, strike, optionExpiryDate, cdsStartDate, cdsMaturityDate, forwardSpread, targetOptionValue, this](const double inputVolatility ) -> double
		{
			return getOptionValueFromForward( payerReceiverSwaptionEnum, strike, optionExpiryDate, cdsStartDate, cdsMaturityDate, inputVolatility, forwardSpread ) - targetOptionValue;
		};

		// Solver Results Contain: Solution, nInterations and Jacobian
		const double impliedVol = solvers::toms748( function, initialGuessForImpliedVol, tolerance, maxIterations ).solution;

		return impliedVol;
	}

	/* @brief	This is the curve collection used when calibrating to / pricing CDS instruments
	*/
	std::string CreditModel::getCDSCurveCollection() const
	{
		return cdsCurveCollection_;
	}

	/* @brief	This is the curve collection used when calibrating to / pricing bond instruments
	*/
	std::string CreditModel::getBondCurveCollection() const
	{
		return bondCurveCollection_;
	}

	/* @brief	This is the discount curve to use when discounting the cashflows of bond instruments
	*/
	std::string CreditModel::getBondDiscountCurve() const
	{
		return bondDiscountCurve_;
	}

	/* @brief	This is the dayCount used for coupon accrual
	*/
	DayCountEnum CreditModel::getAccrualDayCount() const
	{
		return accrualDayCount_;
	}

	CCY CreditModel::getCurrency() const
	{
		return currency_;
	}

	std::string CreditModel::getCreditIndex() const
	{
		return creditIndex_;
	}

	LADate CreditModel::getAsOfDate() const
	{
		return asOfDate_;
	}

}



