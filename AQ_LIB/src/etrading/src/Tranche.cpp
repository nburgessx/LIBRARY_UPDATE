#include "Tranche.h"
#include "Solvers.h"
#include "CommonConstants.h"
#include "LabelValueBlockValidation.h"
#include "ExceptionMacros.h"	// AQ_REQUIRE

#include <cmath>				// std::pow()


namespace etrading
{
	namespace
	{
		StandardStringVector fromTrancheTypeEnumVectorToStringVector( const std::vector<TrancheTypeEnum>& trancheTypeVector )
		{
			const size_t nItems = trancheTypeVector.size();
			StandardStringVector stringVector( nItems );

			for ( size_t i=0; i<nItems; i++ )
			{
				std::string trancheTypeAsString = toString( trancheTypeVector[ i ] );
				stringVector[ i ] = trancheTypeAsString;
			}
			return stringVector;
		}

	}

	/* @brief	Construct a TrancheDefinition from a LabelValue block. of the tranche definition.
	* @param[in]	name			The name of this TrancheDefinition. e.g. Equity, Mezzanine
	* @param[in]	trancheLVB		The label-value block specifying tranche parameters
	* @param[in]	validateKeys	A boolean value specifying whether to validate the keys in the trancheLVB.
	* @returns	The name of the TrancheDefinition object
	*/
	TrancheDefinition::TrancheDefinition( const std::string& name, const LabelValueBlock& trancheLVB, const bool validateKeys )
		: name_( name )
	{
		std::string trancheLVBName = "TrancheLVB";

		validateKeysForLVB( TrancheDefinition::lvbKeys(), trancheLVB.getKeys(), validateKeys );

		trancheSize_	= trancheLVB.getCompulsoryValueAsDouble( TRANCHE_KEYS::TRANCHE_SIZE, trancheLVBName );
		payType_		= toTranchTypeEnum( trancheLVB.getCompulsoryValueAsString( TRANCHE_KEYS::TRANCHE_PAY_TYPE, trancheLVBName ) );
		couponType_		= toTrancheCouponTypeEnum( trancheLVB.getOptionalValueAsString( TRANCHE_KEYS::COUPON_TYPE, "" ) );
		couponRate_		= trancheLVB.getOptionalValueAsDouble( TRANCHE_KEYS::COUPON_RATE, 0.0 );
		couponFloor_	= toBooleanEnum( trancheLVB.getOptionalValueAsString( TRANCHE_KEYS::COUPON_FLOOR, "" ) );
		couponFrequency_ = toFrequencyEnum( trancheLVB.getCompulsoryValueAsString( TRANCHE_KEYS::COUPON_FREQUENCY ) );
		curveCollection_ = trancheLVB.getOptionalValueAsString( TRANCHE_KEYS::CURVE_COLLECTION, "" );
		resetCurve_		= trancheLVB.getOptionalValueAsString( TRANCHE_KEYS::RESET_CURVE, "" );
		discountMargin_	= trancheLVB.getOptionalValueAsDouble( TRANCHE_KEYS::DISCOUNT_MARGIN, 0.0 );
		curveFixingTable_ = trancheLVB.getOptionalValueAsString( TRANCHE_KEYS::FIXING_TABLE, "" );

		validateInputs();
	}


	/* @brief	Constructor of the tranche definition.
	*  @param[in]	name			The name of this tranche ( for example "equity", "mezzanine" )
	*  @param[in]	trancheSize		The size of this tranche i.e. how much loss can this trache absorb.
	*  @param[in]	payType			Specifies whether this tranche pays principal sequentially or pro-rata
	*  @param[in]	couponType		Optional parameter which specifies whether the coupons are fixed or floating
	*  @param[in]	couponRate		Optional coupon rate
	*  @param[in]	couponFloor		Optional coupon floor
	*  @param[in]	couponFrequency	Optional specifies how often coupons are paid
	*  @param[in]	curveCollection	Optional curve collection
	*  @param[in]	resetCurve		Optional curve index used to discount coupons
	*  @param[in]	discountMargin  Optional discount margin. i.e. the fixed spread on top of the forward rate used for discounting
	*  @param[in]	curveFixingTable Optional fixing table, for fixing dates in the past
	*/
	TrancheDefinition::TrancheDefinition( const std::string& name,
										  const double& trancheSize,
										  const TrancheTypeEnum& payType,
										  const TrancheCouponTypeEnum couponType,
										  const double& couponRate,
										  const BooleanEnum couponFloor,
										  const FrequencyEnum couponFrequency,
										  const std::string& curveCollection,
										  const std::string& resetCurve,
										  const double& discountMargin,
										  const std::string& curveFixingTable )
			: name_( name ),
			trancheSize_( trancheSize ),
			payType_( payType ),
			couponRate_( couponRate ),
			couponFloor_( couponFloor ),
			couponFrequency_( couponFrequency ),
			resetCurve_( resetCurve ),
			curveCollection_( curveCollection ),
			discountMargin_( discountMargin ),
			curveFixingTable_( curveFixingTable )
	{
		validateInputs();
	}

	void TrancheDefinition::validateInputs() const
	{
		switch ( couponFrequency_ )
		{
		case MONTHLY_FREQUENCY:
		case QUARTERLY_FREQUENCY:
		case SEMI_ANNUAL_FREQUENCY:
		case ANNUAL_FREQUENCY:
			break;

		default:
			AQ_THROW( "Unsupported tranche coupon frequency " << toString( couponFrequency_ ) << " . Only MONTHLY, QUARTERLY, SEMI-ANNUAL, ANNUAL is supported." );
			break;
		}
	}


	/* @brief	Main initializer
	*  @param	trancheName			The name of the tranche this TrancheSchedule data belongs to
	*  @param	nPeriods			Specifies the number of periods of schedule data to store
	*  @param	trancheDefinition	Specifies the Trache definition for this tranche schedule data
	*/
	void TrancheScheduleData::construct( const std::string& trancheName,  size_t nPeriods, const std::shared_ptr<TrancheDefinition>& trancheDefinition )
	{
		trancheName_ = trancheName;
		nPeriods_ = nPeriods;

		fixingDate_.clear();			fixingDate_.resize( nPeriods );
		accrualStartDate_.clear();		accrualStartDate_.resize( nPeriods );
		accrualEndDate_.clear();		accrualEndDate_.resize( nPeriods );
		paymentDate_.clear();			paymentDate_.resize( nPeriods );
		balanceStart_.clear();			balanceStart_.resize( nPeriods );
		balanceEnd_.clear();			balanceEnd_.resize( nPeriods );
		lossAllocated_.clear();			lossAllocated_.resize( nPeriods);
		principalAllocated_.clear();	principalAllocated_.resize( nPeriods );
		principalPayment_.clear();		principalPayment_.resize(nPeriods);
		attachmentPoint_.clear();		attachmentPoint_.resize( nPeriods );
		detachmentPoint_.clear();		detachmentPoint_.resize( nPeriods );
		coupon_.clear();				coupon_.resize( nPeriods );
		couponRate_.clear();			couponRate_.resize( nPeriods );		
		payType_.clear();				payType_.resize( nPeriods );

		couponYearFraction_.clear();			couponYearFraction_.resize( nPeriods );
		resetRate_.clear();						resetRate_.resize( nPeriods );
		resetDate_.clear();						resetDate_.resize( nPeriods );
		discountFactorRiskless_.clear();		discountFactorRiskless_.resize( nPeriods );
		discountFactorDiscountMargin_.clear();	discountFactorDiscountMargin_.resize( nPeriods );
		riskWeight_.clear();					riskWeight_.resize( nPeriods );
		pvRiskless_.clear();					pvRiskless_.resize( nPeriods );
		pvDiscountMargin_.clear();				pvDiscountMargin_.resize( nPeriods );
		averageBalance_.clear();				averageBalance_.resize( nPeriods );

		balanceStart_[ 0 ] = trancheDefinition->trancheSize_;
	}
	
	/* @brief	Calculate the PVs of all coupon flows, and store within the tranche schedule
	*/
	void TrancheScheduleData::calculatePVs()
	{
		for ( size_t iPeriod = 0; iPeriod < nPeriods_; iPeriod++ )
		{
			const double totalCouponAmount = coupon_[ iPeriod ] + principalPayment_[ iPeriod ];
			const double pv_riskless = totalCouponAmount * discountFactorRiskless_[ iPeriod ];
			pvRiskless_[ iPeriod ] = pv_riskless;

			const double pv_discountMargin = totalCouponAmount * discountFactorDiscountMargin_[ iPeriod ];
			pvDiscountMargin_[ iPeriod ] = pv_discountMargin;
		}
	}

	/* @brief Calculates the NPV of coupon flows at the specified time period and valuation date, using discount margin
	*  @param[in]	callPeriod		calculate the PV of coupons at this time period
	*  @param[in]	valuationDate	PV the coupons to this valuation date
	*  @returns		The PV of all coupons occurring at the specified time period
	*/
	double TrancheScheduleData::calculateNPVDiscountMargin( const size_t callPeriod, const LADate& valuationDate ) const
	{
		// *** Note: valuationDate is not used.

		AQ_REQUIRE( callPeriod < nPeriods_, "Specified callPeriod is outside of range. Maximum tranch period is: " << nPeriods_ << "; specified callPeriod is: " << callPeriod );

		double totalPV = 0.0;
		for ( size_t iPeriod = 0; iPeriod < callPeriod; iPeriod++ )
		{
			const double totalCouponAmount = coupon_[ iPeriod ] + principalPayment_[ iPeriod ];
			const double discountFactor = discountFactorDiscountMargin_[ iPeriod ];
			const double couponPV = totalCouponAmount * discountFactor;
			totalPV += couponPV;
		}

		const double balance_end = balanceEnd_[ callPeriod ];
		const double discountFactor = discountFactorDiscountMargin_[ callPeriod ];
		const double balanceEndPV = balance_end * discountFactor;
		totalPV += balanceEndPV;

		return totalPV;
	}

	/* @brief Calculates the NPV of coupon flows at the specified time period and valuation date
	*  @param[in]	callPeriod		calculate the PV of coupons at this time period
	*  @param[in]	valuationDate	PV the coupons to this valuation date
	*  @returns		The PV of all coupons occurring at the specified time period
	*/
	double TrancheScheduleData::calculateNPV( const double& interestRate, const size_t callPeriod, const LADate& valuationDate ) const
	{
		AQ_REQUIRE( callPeriod < nPeriods_, "Specified callPeriod is outside of range. Maximum tranch period is: " << nPeriods_ << "; specified callPeriod is: " << callPeriod );

		double totalPV = 0.0;

		// MGEN year fraction calculation
		// In a correct yearFraction calculation, what should be the dayCount?
		const LADate& finalPaymentDate = paymentDate_[callPeriod];
		// *** TODO Should we consider days?
		const int intervalMonths = valuationDate.intervalMonths( finalPaymentDate );
		const int intervalYears  = valuationDate.intervalYears( finalPaymentDate );
		const int monthsToFinalPaymentDate = intervalYears * 12 + intervalMonths;
		const double yearFractionToFinalPaymentDate = monthsToFinalPaymentDate / 12.0; // TODO: We do this to match MGEN. However we have better yearFraction functions in MLIB.

		for ( size_t iPeriod = 0; iPeriod < callPeriod; iPeriod++ )
		{
			// MGEN year fraction calculation
			// In a correct yearFraction calculation, what should be the dayCount?
			const LADate& payDate = paymentDate_[ iPeriod ];
			// *** TODO Should we consider days?
			const int intervalMonthsToPayDate = valuationDate.intervalMonths( payDate );
			const int intervalYearsToPayDate  = valuationDate.intervalYears( payDate );
			const int monthsToPaymentDate = intervalYearsToPayDate * 12 + intervalMonthsToPayDate;
			const double yearFraction = monthsToPaymentDate / 12.0;		// TODO: We do this to match MGEN. However we have better yearFraction functions in MLIB.

			const double totalCouponAmount = coupon_[ iPeriod ] + principalPayment_[ iPeriod ];
			const double simpleDiscountFactor = 1.0 / ( std::pow( 1.0 + interestRate, yearFraction ) );

			const double couponPV = totalCouponAmount * simpleDiscountFactor;
			if ( ! std::isnan( couponPV ) )
			{
				totalPV += couponPV;
			}
		}

		const double balance_end = balanceEnd_[ callPeriod ];
		const double simpleDiscountFactor = 1.0 / ( std::pow(1.0 + interestRate, yearFractionToFinalPaymentDate ) );
		const double balanceEndPV = balance_end * simpleDiscountFactor;
		totalPV += balanceEndPV;

		return totalPV;
	}

	double TrancheScheduleData::calculateIRR( const size_t callPeriod, const LADate& valuationDate, const double& targetPrice ) const
	{
		AQ_REQUIRE( callPeriod < nPeriods_, "Specified callPeriod is outside of range. Maximum tranch period is: " << nPeriods_ << "; specified callPeriod is: " << callPeriod );

		AQ_REQUIRE ( callPeriod > 0, "Invalid callPeriod: CallPeriod must be greater than 0 for the IRR calculation." );

		// Newton-Raphson Solver Settings
		double initialGuessForIRR = 0.0;
		const double tolerance = 1e-7;
		const unsigned int maxIterations = 1000;
		const double shiftSize = 1.0e-4;

		// One-dimensional objective function used by the solver:
		// This lambda function captures the settlementDate and yieldCalcType as fixed parameters.
		// The spead is the variable which the solver will adjust in order to obtain the targetPV.

		auto function = [ & ]( const double interestRate )
		{
			const double npv = calculateNPV(interestRate, callPeriod, valuationDate);
			return npv;
		};

		// Solver Results Contain: Solution, nInterations and Jacobian
		const double internalRateOfReturn = solvers::newtonRaphson( function, targetPrice, initialGuessForIRR, tolerance, maxIterations, shiftSize ).solution;

		return internalRateOfReturn;
	}


	DataFrame TrancheScheduleData::outputToLabelledDataFrame() const
	{
		DataFrame dataframe;
		
		dataframe.addColumnWithPrefix( trancheName_,  "FixingDate",			fixingDate_ );
		dataframe.addColumnWithPrefix( trancheName_, "AccrualStartDate",	accrualStartDate_ );
		dataframe.addColumnWithPrefix( trancheName_, "AccrualEndDate",		accrualEndDate_ );
		dataframe.addColumnWithPrefix( trancheName_, "PaymentDate",			paymentDate_ );

		dataframe.addColumnWithPrefix( trancheName_, "BalanceStart",		balanceStart_ );
		dataframe.addColumnWithPrefix( trancheName_, "BalanceEnd",			balanceEnd_ );
		dataframe.addColumnWithPrefix( trancheName_, "LossAllocated",		lossAllocated_ );
		dataframe.addColumnWithPrefix( trancheName_, "PrincipalAllocated",	principalAllocated_ );
		dataframe.addColumnWithPrefix( trancheName_, "PrincipalPayment",	principalPayment_ );
		dataframe.addColumnWithPrefix( trancheName_, "AttachmentPoint",		attachmentPoint_ );
		dataframe.addColumnWithPrefix( trancheName_, "DetachmentPoint",		detachmentPoint_ );
		dataframe.addColumnWithPrefix( trancheName_, "Coupon",				coupon_ );
		dataframe.addColumnWithPrefix( trancheName_, "CouponRate",			couponRate_ );
		dataframe.addColumnWithPrefix( trancheName_, "PayType",				fromTrancheTypeEnumVectorToStringVector( payType_ ) );
		dataframe.addColumnWithPrefix( trancheName_, "CouponYearFraction",	couponYearFraction_ );
		dataframe.addColumnWithPrefix( trancheName_, "ResetRate",			resetRate_ );
		dataframe.addColumnWithPrefix( trancheName_, "ResetDate",			resetDate_ );
		dataframe.addColumnWithPrefix( trancheName_, "DiscountFactorRiskless",		 discountFactorRiskless_ );
		dataframe.addColumnWithPrefix( trancheName_, "DiscountFactorDiscountMargin", discountFactorDiscountMargin_ );
		dataframe.addColumnWithPrefix( trancheName_, "RiskWeight",			riskWeight_ );
		dataframe.addColumnWithPrefix( trancheName_, "PVRiskless",			pvRiskless_ );
		dataframe.addColumnWithPrefix( trancheName_, "PVDiscountMargin",	pvDiscountMargin_ );
		dataframe.addColumnWithPrefix( trancheName_, "AverageBalance",		averageBalance_ );

		return dataframe;
	}

}

