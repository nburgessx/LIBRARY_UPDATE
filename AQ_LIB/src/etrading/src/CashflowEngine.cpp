#include "CashflowEngine.h"
#include "SupervisoryRules.h"

#include "CurveUtilities.h"							// getCurveForwardRates()
#include "SwapUtilities.h"
#include "CommonConstants.h"
#include "CoreEnumerations.h"
#include "LabelValueBlock.h"

#include "AQLDefinitions.h"							// AQLStringMatrix
#include "AQLDateScheduleHelpers.h"					// validateDate()
#include "AQLMathDateUtilities.h"					// getAQLDate( int excelDate )
#include "AQLCurveForwardRateHelpers.h"				// getMultiSpotDiscountFactors

#include <boost/algorithm/string/predicate.hpp>		// boost::iequals
#include <algorithm>								// for std::min() and std::max()
#include <limits>									// std::numeric_limits<double>::quiet_NaN()

#include <boost/algorithm/string/predicate.hpp>		// boost::algorithm::ends_with()

namespace etrading
{

	namespace
	{
		// TODO: Extract this from curve conventions
		LabelValueBlock setUpFloatScheduleConventions( const AQLDate& effectiveDate, const AQLDate& maturityDate, const std::string& frequency )
		{
			
			const std::vector<std::string> EUR_CONVENTION_KEYS = {  "EffectiveDate",
																	"MaturityDate",
																	"AccrualDayCount",
																	"AccrualFrequency",
																	"AccrualBusinessDayAdjustment",
																	"AccrualCalendar",
																	"PaymentFrequency",
																	"PaymentBusinessDayAdjustment",
																	"PaymentCalendar",
																	"PaymentLag",
																	"FixingBusinessDayAdjument",
																	"FixingCalendar",
																	"FixingLag",
																	"FirstStubDate",
																	"LastStubDate",
																	"RollDay" };
			
			const std::vector<std::string> EUR_CONVENTION_VALUES = { effectiveDate.convertDateToString().c_str(), 
																	 maturityDate.convertDateToString().c_str(), 
																	 "ACT/360",
																	 frequency,
																	 "FOLLOWING",
																	 "TGT",
																	 frequency,
																	 "FOLLOWING",
																	 "TGT",
																	 "0D",
																	 "PRECEDING",
																	 "TGT",
																	 "0D",
																	 "",
																	 "",
																	 "" };

			LabelValueBlock floatScheduleConventions( EUR_CONVENTION_KEYS, EUR_CONVENTION_VALUES );


			// Enrich the provided schedule information with dummy default values.
			StandardStringVector addKeys( 3 );
			addKeys[0] = IRS_KEY::SCHEDULE_TYPE;
			addKeys[1] = IRS_KEY::PAY_RECEIVE;
			addKeys[2] = IRS_KEY::NOTIONAL;

			StandardStringVector addValues( 3 );
			addValues[0] = "FLOAT";
			addValues[1] = "PAY";
			addValues[2] = "1.0";

			LabelValueBlock enrichedSwapScheduleLVB( floatScheduleConventions, addKeys, addValues );

			return enrichedSwapScheduleLVB;
		}

		AnyTypeMatrix getScheduleColumn( const SchedulePtr& schedule, const CashflowHeaderEnum cashflowHeaderEnum )
		{
			const bool showBespokeProperties	= false;
			const bool showColumnHeaders		= false;
			const bool convertDatesToExcelFormat = true;
			std::unordered_set < CashflowHeaderEnum, EnumClassHash> columnList = { cashflowHeaderEnum };

			auto results = schedule->view( showBespokeProperties, showColumnHeaders, columnList, convertDatesToExcelFormat );
			auto resultMatrix = results.second;
			return resultMatrix;
		}

		std::vector<double> getScheduleColumnAsDouble( const SchedulePtr& schedule, const CashflowHeaderEnum cashflowHeaderEnum )
		{
			auto resultMatrix = getScheduleColumn( schedule, cashflowHeaderEnum );
			const size_t nRows = resultMatrix.size();
		
			std::vector<double> doubleVector( nRows );
			for ( size_t row = 0; row < nRows; row++ )
			{
				auto anyTypeValue = resultMatrix[ row ][ 0 ];
				const double doubleValue = boost::get<double>( anyTypeValue );
				doubleVector[ row ] = doubleValue;
			}
			return doubleVector;
		}

		std::vector<AQLDate> getScheduleColumnAsDate( const SchedulePtr& schedule, const CashflowHeaderEnum cashflowHeaderEnum )
		{
			auto resultMatrix = getScheduleColumn( schedule, cashflowHeaderEnum );
			const size_t nRows = resultMatrix.size();

			std::vector<AQLDate> dateVector( nRows );
			for ( size_t row = 0; row < nRows; row++ )
			{
				auto anyTypeValue = resultMatrix[ row ][ 0 ];
				int dateAsExcelDouble = boost::get<double>( anyTypeValue );
				AQLDate date = AQLMathDateUtilities::getAQLDate( dateAsExcelDouble );
				dateVector[ row ] = date;
			}
			return dateVector;
		}

	}


	/* @brief	Simple constructor
	*  @param[in]	name	The name of this cashflow engine
	*/
	CashflowEngine::CashflowEngine( const std::string& name )
		:	name_( name ),
			loanPortfolio_(),
			kirb_( 0.0 ),
			elgd_( 0.0 ),
			regulatoryCapitalMaturity_( 0 ),
			nDiversityMetric_( 500 ),						// Default value used in MGEN
			supervisoryType_( NONE_SUPERVISORY_TYPE ),
			poolType_( NONE_POOL_TYPE ),
			effectiveDate_(),
			maturityDate_(),
			// resetDates
			// resetRates
			// discountFactors,
			// countTranches
			// countTrancheFields
			// namedTranches
			// ptfSize
			// periods
			// portfolio_flows
			// triggers is a list
			reinvestmentEndDate_(),
			reinvestmentEndPeriod_( 0 ),
			//liabilityFlows_
			//principal_waterfall = list(),
			//interest_waterfall = list(),
			paymentAveraging_(false)
			// syntheticExcessSpread_()
	{
	}

	/* @brief	Main way in which this CashflowEngine is configured
	*  @param[in]	loanPortfolio		The portfolio of loan cashflows to process
	*  @param[in]	trancheDefinitions	Specifies the tranches within the capital structure
	*  @param[in]	kirb				Regulatory capital amount
	*  @param[in]	elgd				Expected loss given default
	*  @param[in]	regulatoryCapitalMaturity
	*  @param[in]	nDiversityMetric
	*  @param[in]	supervisoryType		Which regulatory / supervisory framework to use. e.g. Legacy or IRBA
	*  @param[in]	poolType			Wholesale or retail
	*  @param[in]	effectiveDate		The effective date of the portfolio
	*  @param[in]	maturityDate		The maturity date of the portfolio
	*  @param[in]	triggers			A set of triggers used to determine if reinvestments can be made
	*  @param[in]	reinvestmentEndDate	Specifies a reinvestment cut-off date
	*  @param[in]	reinvestmentEndPeriod
	*/
	void CashflowEngine::configure( const std::shared_ptr<LoanPortfolio>& loanPortfolio,
									const std::vector<std::shared_ptr<TrancheDefinition> >& trancheDefinitions,
									const double kirb,
									const double elgd,
									const int regulatoryCapitalMaturity,	// mt
									const size_t nDiversityMetric,			// n
									const SupervisoryTypeEnum supervisoryType,
									const PoolTypeEnum poolType,
									const AQLDate& effectiveDate,
									const AQLDate& maturityDate,
									const std::vector<std::shared_ptr<Trigger> >& triggers,
									const AQLDate& reinvestmentEndDate,
									const size_t reinvestmentEndPeriod )
	{
		loanPortfolio_ = loanPortfolio;
		trancheDefinitions_ = trancheDefinitions;

		const size_t nTranches = trancheDefinitions_.size();
		trancheMonthlyScheduleData_.clear();
		trancheMonthlyScheduleData_.resize( nTranches );

		trancheCouponScheduleData_.clear();
		trancheCouponScheduleData_.resize( nTranches );

		kirb_ = kirb;
		elgd_ = elgd;

		regulatoryCapitalMaturity_	= regulatoryCapitalMaturity;
		nDiversityMetric_			= nDiversityMetric;

		supervisoryType_ = supervisoryType;
		poolType_ = poolType;

		effectiveDate_ = effectiveDate;
		maturityDate_ = maturityDate;
		
		triggers_ = triggers;
		
		reinvestmentEndDate_ = reinvestmentEndDate;
		reinvestmentEndPeriod_ = reinvestmentEndPeriod;

		portfolioBalanceSize_ = loanPortfolio_->getBalanceStart(0);
	}
	
	/* @brief	Configures the CashflowEngine using a label value block
	*  @param[in]	configurationLVB		The portfolio of loan cashflows to process
	*  @param[in]	trancheDefinitions		Specifies the tranches within the capital structure
	*  @param[in]	triggers				Specifies triggers used to determine if reinvestments can be made
	*/
	void CashflowEngine::configure( const std::shared_ptr<LoanPortfolio>& loanPortfolio,
									const LabelValueBlock& configurationLVB,
									const std::vector<std::shared_ptr<TrancheDefinition> >& trancheDefinitions,
									const std::vector<std::shared_ptr<Trigger> >& triggers,
									const bool validateKeys )
	{
		loanPortfolio_ = loanPortfolio;
		trancheDefinitions_ = trancheDefinitions;
		triggers_ = triggers;
		
		const size_t nTranches = trancheDefinitions_.size();
		trancheMonthlyScheduleData_.clear();
		trancheMonthlyScheduleData_.resize( nTranches );

		trancheCouponScheduleData_.clear();
		trancheCouponScheduleData_.resize( nTranches );

		std::string trancheLVBName = "CashflowEngineConfigurationLVB";

		validateKeysForLVB( CashflowEngine::lvbKeys(), configurationLVB.getKeys(), validateKeys );

		std::string configurationLVBName = "ConfigurationLVB";
		kirb_						= configurationLVB.getCompulsoryValueAsDouble( CASHFLOW_ENGINE_KEYS::KIRB, configurationLVBName );
		elgd_						= configurationLVB.getCompulsoryValueAsDouble( CASHFLOW_ENGINE_KEYS::ELGD, configurationLVBName );
		regulatoryCapitalMaturity_	= configurationLVB.getCompulsoryValueAsDouble( CASHFLOW_ENGINE_KEYS::REGULATORY_CAPITAL_MATURITY, configurationLVBName );
		nDiversityMetric_			= configurationLVB.getCompulsoryValueAsDouble( CASHFLOW_ENGINE_KEYS::DIVERSITY_METRIC, configurationLVBName );
		supervisoryType_			= toSupervisoryTypeEnum( configurationLVB.getCompulsoryValueAsString( CASHFLOW_ENGINE_KEYS::SUPERVISORY_TYPE, configurationLVBName ) );
		poolType_					= toPoolTypeEnum( configurationLVB.getCompulsoryValueAsString( CASHFLOW_ENGINE_KEYS::POOL_TYPE, configurationLVBName ) );

		effectiveDate_				= configurationLVB.getOptionalValueAsDate( CASHFLOW_ENGINE_KEYS::EFFECTIVE_DATE );
		maturityDate_				= configurationLVB.getOptionalValueAsDate( CASHFLOW_ENGINE_KEYS::MATURITY_DATE );
		reinvestmentEndDate_		= configurationLVB.getOptionalValueAsDate( CASHFLOW_ENGINE_KEYS::REINVESTMENT_END_DATE );
		reinvestmentEndPeriod_		= configurationLVB.getOptionalValueAsDouble(CASHFLOW_ENGINE_KEYS::REINVESTMENT_END_PERIOD, 0. );

		portfolioBalanceSize_ = loanPortfolio_->getBalanceStart(0);
	}

	/* @brief	sets the number of time periods to evaluate the capital structure over
	*  @param[in]	periods	The number of periods
	*/
	void CashflowEngine::setPeriods( const size_t periods )
	{
		nPeriods_ = periods;

		const size_t nTranches = trancheDefinitions_.size();
		trancheMonthlyScheduleData_.clear();
		trancheMonthlyScheduleData_.resize( nTranches );

		trancheCouponScheduleData_.clear();
		trancheCouponScheduleData_.resize( nTranches );
		for (size_t trancheIdx = 0; trancheIdx < nTranches; trancheIdx++)
		{
			const std::shared_ptr<TrancheDefinition> trancheDefinition = trancheDefinitions_[ trancheIdx ];
			TrancheScheduleData& trancheMonthlySchedule = trancheMonthlyScheduleData_[ trancheIdx ];
			trancheMonthlySchedule.construct( trancheDefinition->name_, nPeriods_, trancheDefinition );
		}

		loanPortfolio_->initializeReinvestmentBalance( nPeriods_ );

		// # define data structure to hold all tranches
		portfolioFlows_.initialize( nPeriods_ );

	}

	/* @brief
	*  @param[in]	nDiversityMetric
	*/
	void CashflowEngine::setDiversityMetric( const size_t nDiversityMetric )
	{
		nDiversityMetric_ = nDiversityMetric;
	}

	/*
	void CashflowEngine::resetExcessSpread( const size_t period, const double& excessSpreadAmount )
	{
		AQ_REQUIRE( period < excessSpread_.size(), "Supplied period is outside valid range. Must be below: " << excessSpread_.size() << "; Actial value: " << period );

		// # reset xs spread on given month period
		excessSpread_[ period ] = excessSpreadAmount;

	}
	*/

	/*
	void CashflowEngine::applySyntheticExcessSpread( const size_t period, const double& periodLoss ) const
	{
		AQ_THROW( "Method applySyntheticExcessSpread() is not implemented." );
	}*/


	/* @brief	Perform discount factor calculculations using the yield curve data stored in the capital structure
	*/
	void CashflowEngine::populateYieldCurveData()
	{
		// # build date schedule and bind to liability flows
		AQ_REQUIRE( AQLDateScheduleHelpers::isValidDate( effectiveDate_ ), "Invalid EffectiveDate in CashflowEngine::populateYieldCurveData()" );
		AQ_REQUIRE( AQLDateScheduleHelpers::isValidDate( maturityDate_ ),  "Invalid EffectiveDate in CashflowEngine::populateYieldCurveData()" );

		// # portfolio projection always monthly
		// # back stub

		/*
			First use AQ's date schedule logic to create a monthly schedule. Store the key dates
			of this monthly schedule in the CashflowEngine as vectors of dates.
		 */
		std::string monthlyFrequency = toString( MONTHLY_FREQUENCY );
		LabelValueBlock monthlyConventions = setUpFloatScheduleConventions( effectiveDate_, maturityDate_, monthlyFrequency );

		auto monthlySchedule = createSchedule( "monthlySchedule", monthlyConventions );
		resetDates_					= getScheduleColumnAsDate( monthlySchedule, FIXING_DATE_HEADER );
		portfolioFixingDates_		= getScheduleColumnAsDate( monthlySchedule, FIXING_DATE_HEADER );
		portfolioAccrualStartDates_ = getScheduleColumnAsDate( monthlySchedule, ACCRUAL_START_HEADER );
		portfolioAccrualEndDates_	= getScheduleColumnAsDate( monthlySchedule, ACCRUAL_END_HEADER );
		portfolioPaymentDates_		= getScheduleColumnAsDate( monthlySchedule, PAYMENT_DATE_HEADER );

		/* 
			Examine the portfolioPaymentDates_ to determine how many monthly periods there are in the schedule.
			Store this number of periods by calling setPeriods().
			This also initializes the monthly structure within each tranche.
		*/
		setPeriods( portfolioPaymentDates_.size() );

		// # tranche specific index curves, reset rates and dcf

		/*
			For each tranche, this next section examines the tranche coupon frequency
			(e.g. Monthly, Quarterly, Semi-Annual etc) and generates a coupon schedule.
			This schedule is stored in a structure "trancheCouponScheduleData".
		*/
		const size_t nTranches = trancheDefinitions_.size();
		for ( size_t trancheIdx = 0; trancheIdx < nTranches; trancheIdx++ )
		{
			const std::shared_ptr<TrancheDefinition> currentTrancheDefinition = trancheDefinitions_[ trancheIdx ];
			TrancheScheduleData& currentTrancheCouponScheduleData  = trancheCouponScheduleData_[ trancheIdx ];

			// # support for monthly or quarterly
			// # Note: all tranches must be of same frequency at this time
			const FrequencyEnum frequencyEnum = currentTrancheDefinition->couponFrequency_;	
			switch ( frequencyEnum )
			{
			case MONTHLY_FREQUENCY:
			case QUARTERLY_FREQUENCY:
			case SEMI_ANNUAL_FREQUENCY:
			case ANNUAL_FREQUENCY:
				break;
			
			default:
				AQ_THROW("Unsupported tranche coupon frequency " << toString( frequencyEnum ) << " . Only MONTHLY, QUARTERLY, SEMI-ANNUAL, ANNUAL is supported.");
				break;
			}
			std::string floatFrequency = toString( frequencyEnum );
			LabelValueBlock floatConventions = setUpFloatScheduleConventions( effectiveDate_, maturityDate_, floatFrequency );
			auto floatScheduleForTranche = createSchedule( "floatSchedule", floatConventions );

			currentTrancheCouponScheduleData.couponYearFraction_	= getScheduleColumnAsDouble( floatScheduleForTranche, ACCRUAL_YEAR_FRACTIONS_HEADER );
			currentTrancheCouponScheduleData.resetDate_				= getScheduleColumnAsDate( floatScheduleForTranche, FIXING_DATE_HEADER );
			currentTrancheCouponScheduleData.fixingDate_			= getScheduleColumnAsDate( floatScheduleForTranche, FIXING_DATE_HEADER );
			currentTrancheCouponScheduleData.accrualStartDate_		= getScheduleColumnAsDate( floatScheduleForTranche, ACCRUAL_START_HEADER );
			currentTrancheCouponScheduleData.accrualEndDate_		= getScheduleColumnAsDate( floatScheduleForTranche, ACCRUAL_END_HEADER );
			currentTrancheCouponScheduleData.paymentDate_			= getScheduleColumnAsDate( floatScheduleForTranche, PAYMENT_DATE_HEADER );

			// *** TODO: Deal with this hard-coded information
			AQLString curveIndex( currentTrancheDefinition->resetCurve_ );
			AQLString curveCollection( currentTrancheDefinition->curveCollection_ );
			AQLString staticDataTable = getCurveStaticDataTableName( curveCollection, curveIndex );
			AQLString interpolation = getCurveInterpolation( curveCollection, staticDataTable );
			AQLString bdAdj( "NO_CHANGE" );
			AQLString dayCount( "ACT/365" );
			bool basisFlag( false );
			AQLString calendar;
			calendar = getDefaultCalendarForEmptyString( calendar, curveCollection );

			currentTrancheCouponScheduleData.discountFactorRiskless_ = AQLCurveForwardRateHelpers::getMultiSpotDiscountFactors(   currentTrancheCouponScheduleData.paymentDate_,
																																 getDataInstance(),
																																 curveCollection,
																																 dayCount,
																																 bdAdj,
																																 calendar,
																																 interpolation,
																																 basisFlag,
																																 curveIndex );

			currentTrancheCouponScheduleData.discountFactorDiscountMargin_ = getDiscountFactorsForCurveIndexWithSpread( currentTrancheCouponScheduleData.paymentDate_,
																														currentTrancheDefinition->curveCollection_,
																														currentTrancheDefinition->resetCurve_,
																														currentTrancheDefinition->discountMargin_,
																														currentTrancheDefinition->curveFixingTable_ );
			BusinessDayAdjustmentEnum businessDayAdjustmentEnum( NONE_BUSINESS_DAY_ADJ );
			BooleanEnum fwdInterEnum( NONE_BOOL );

			// # get forwards
			currentTrancheCouponScheduleData.resetRate_ = getCurveForwardRates( currentTrancheCouponScheduleData.resetDate_,
																			    currentTrancheDefinition->curveCollection_,
																			    currentTrancheDefinition->resetCurve_,
																			    businessDayAdjustmentEnum,
																			    calendar,
																			    fwdInterEnum );
		}
	}

	/* @brief		Perform interest calculation for the specified time period
	*  @param[in]	period	The time period for which to calculate the interest amount due
	*/
	void CashflowEngine::payInterest( const size_t period )
	{
		AQ_REQUIRE( period < nPeriods_, "Supplied period is outside valid range. Must be below: " << nPeriods_ << "; Actial value: " << period );

		// # check if this period is a coupon period
		// # tranche in interest_waterfall

		const size_t nTranches = trancheDefinitions_.size();
		for (size_t trancheIdx = 0; trancheIdx < nTranches; trancheIdx++)
		{
			const std::shared_ptr<TrancheDefinition>& currentTrancheDefintion = trancheDefinitions_[ trancheIdx ];
			TrancheScheduleData& currentTrancheCouponScheduleData  = trancheCouponScheduleData_[ trancheIdx ];
			TrancheScheduleData& currentTrancheMonthlyScheduleData = trancheMonthlyScheduleData_[ trancheIdx ];

			// # see if current portfolio payment date exists in  tranche payment dates, i.e. is a coupon date

			/*
				Search the current tranche coupon schedule payment dates to see if it contains the portfolioPaymentDate
				for this time period. If the portfolioPaymentDate is present, it means this time period pays a coupon.
			*/
			const AQLDate& portfolioPaymentDate = portfolioPaymentDates_[ period ];
			auto paymentDateIter = std::find( currentTrancheCouponScheduleData.paymentDate_.begin(),
								   currentTrancheCouponScheduleData.paymentDate_.end(),
								   portfolioPaymentDate );

			if (paymentDateIter != currentTrancheCouponScheduleData.paymentDate_.end() )
			{
				// # find the index of the coupon payment date, use this to find the reset index
				const size_t payCouponPeriodIdx = std::distance( currentTrancheCouponScheduleData.paymentDate_.begin(), paymentDateIter );

				const AQLDate& resetDate =  currentTrancheCouponScheduleData.resetDate_[ payCouponPeriodIdx ];

				auto resetDateIter = std::find( resetDates_.begin(), resetDates_.end(), resetDate );
				AQ_REQUIRE( resetDateIter != resetDates_.end(), "Could not find tranche reset date in resetDates vector" );
				
				const size_t resetPeriod = std::distance( resetDates_.begin(), resetDateIter );

				// We define 'period' in AQ as counting from 0. In MGEN/R 'period' is defined to count up from 1.
				// const int resetPeriod = ( period + 1 ) / currentTrancheDefintion.couponFrequency_ - 1;

				currentTrancheMonthlyScheduleData.couponYearFraction_[ period ]	= currentTrancheCouponScheduleData.couponYearFraction_[ payCouponPeriodIdx ];
				currentTrancheMonthlyScheduleData.resetRate_[ period ]			= currentTrancheCouponScheduleData.resetRate_[ payCouponPeriodIdx ];

				currentTrancheMonthlyScheduleData.resetDate_[ period ]		= currentTrancheCouponScheduleData.resetDate_[ payCouponPeriodIdx ];
				currentTrancheMonthlyScheduleData.paymentDate_[ period ]	= currentTrancheCouponScheduleData.paymentDate_[ payCouponPeriodIdx ];

				currentTrancheMonthlyScheduleData.discountFactorDiscountMargin_[ period ]	= currentTrancheCouponScheduleData.discountFactorDiscountMargin_[ payCouponPeriodIdx ];
				currentTrancheMonthlyScheduleData.discountFactorRiskless_[ period ]			= currentTrancheCouponScheduleData.discountFactorRiskless_[ payCouponPeriodIdx ];

				currentTrancheMonthlyScheduleData.accrualStartDate_[ period ]	= currentTrancheCouponScheduleData.accrualStartDate_[ payCouponPeriodIdx ];
				currentTrancheMonthlyScheduleData.accrualEndDate_[period]		= currentTrancheCouponScheduleData.accrualEndDate_[ payCouponPeriodIdx ];
				currentTrancheMonthlyScheduleData.fixingDate_[ period ]			= currentTrancheCouponScheduleData.fixingDate_[ payCouponPeriodIdx ];

				// # floored interest rate defined at tranche level
				if ( currentTrancheDefintion->couponFloor_ == TRUE_BOOL )
				{
					if (currentTrancheMonthlyScheduleData.resetRate_ [period ] < 0.0 )
					{
						currentTrancheMonthlyScheduleData.resetRate_[ period ] = 0.0;
					}
				}

				const double couponPeriodRate = ( currentTrancheDefintion->couponRate_ / 10000.0 )
													+ currentTrancheMonthlyScheduleData.resetRate_[ period ];

				// #  need to get average balance for intra period  months
				double couponPeriodBalance = 0;

				// #const int startPeriod = (int)period - trancheDefinitions_[trancheIdx].couponFrequency_ + 1;

				// # we need balance at the start of the coupon period
				/*
				if ( paymentAveraging_ )
				{
					for ( int iPeriod = startPeriod; iPeriod <= (int)period; iPeriod++ )
					{
						AQ_REQUIRE( iPeriod >= 0, "Internal error during payment averaging. iPeriod is out of range" );

						couponPeriodBalance += currentTrancheScheduleData.balanceStart_[ iPeriod ];
					}
					couponPeriodBalance = couponPeriodBalance / (double) currentTrancheDefintion.couponFrequency_;
				}
				else
				{ */
					couponPeriodBalance = currentTrancheMonthlyScheduleData.balanceStart_[ resetPeriod ];
				/*
				}
				*/

				// # daycount fraction applied
				currentTrancheMonthlyScheduleData.coupon_[ period ] = couponPeriodBalance * couponPeriodRate * currentTrancheMonthlyScheduleData.couponYearFraction_[ period ];

				// # tracing variables
				currentTrancheMonthlyScheduleData.averageBalance_[ period ]	= couponPeriodBalance;
				currentTrancheMonthlyScheduleData.couponRate_[ period ]		= couponPeriodRate;
			}
			else
			{
				// # if not a paying period then coupon is not applicable
				currentTrancheMonthlyScheduleData.coupon_[ period ]				= std::numeric_limits<double>::quiet_NaN();
				currentTrancheMonthlyScheduleData.couponYearFraction_[ period ]	= std::numeric_limits<double>::quiet_NaN();
				currentTrancheMonthlyScheduleData.resetRate_[ period ]				= std::numeric_limits<double>::quiet_NaN();
				currentTrancheMonthlyScheduleData.discountFactorDiscountMargin_[ period ] = std::numeric_limits<double>::quiet_NaN();
				currentTrancheMonthlyScheduleData.discountFactorRiskless_[ period ]	= std::numeric_limits<double>::quiet_NaN();
				currentTrancheMonthlyScheduleData.averageBalance_[ period ]			= std::numeric_limits<double>::quiet_NaN();
				currentTrancheMonthlyScheduleData.couponRate_[ period ]				= std::numeric_limits<double>::quiet_NaN();
			}
		}
	}

	/* @brief		Allocated losses in the specified time period
	*  @param[in]	period		The time period at which to assign the loss
	*  @param[in]	periodLoss	The loss amount to allocate to tranches
	*/
	void CashflowEngine::payLosses( const size_t period, const double& periodLoss )
	{
		AQ_REQUIRE( period < nPeriods_, "Supplied period is outside valid range. Must be below: " << nPeriods_ << "; Actial value: " << period );

		double unallocatedPeriodLoss = periodLoss;

		//  # first if defined, net off XS spread use r6 class instead
		if ( syntheticExcessSpread_ )
		{
			// # reset synthetic excess spread
			SyntheticExcessSpreadAllocation syntheticExcessSpreadAllocation = syntheticExcessSpread_->getAllocation( period );
			if ( syntheticExcessSpreadAllocation.isResetPeriod )
			{
				// resetting sxs spread to {self$sxs$df$level_start[period]} in period {period}')

				const double levelStart = syntheticExcessSpread_->getAmount() * loanPortfolio_->getBalanceStart( period );
				syntheticExcessSpreadAllocation.levelStart =  levelStart;
			}

			const double excessLossAllocated = std::max( 0.0, std::min( periodLoss, syntheticExcessSpreadAllocation.levelStart ) );
			syntheticExcessSpreadAllocation.used = excessLossAllocated;
			syntheticExcessSpreadAllocation.levelEnd = syntheticExcessSpreadAllocation.levelStart - excessLossAllocated;
			syntheticExcessSpread_->setAllocation( period, syntheticExcessSpreadAllocation );

			unallocatedPeriodLoss -= excessLossAllocated;
		}

		const size_t nTranches = trancheDefinitions_.size();
		for ( size_t trancheIdx = 0; trancheIdx < nTranches; trancheIdx++ )
		{
			TrancheScheduleData& currentTrancheMonthlyScheduleData = trancheMonthlyScheduleData_[ trancheIdx ];

			const double lossAllocated = std::max( 0.0, std::min(currentTrancheMonthlyScheduleData.balanceStart_[ period ], unallocatedPeriodLoss ) );

			currentTrancheMonthlyScheduleData.balanceEnd_[ period ]		= currentTrancheMonthlyScheduleData.balanceStart_[ period ] - lossAllocated;
			currentTrancheMonthlyScheduleData.lossAllocated_[ period ]	= lossAllocated;

			unallocatedPeriodLoss = unallocatedPeriodLoss - lossAllocated;
		}
	}

	/* @brief		Reinvest an amount in the specified time period
	*  @param[in]	period	The time period at which the reinvestment should take place
	*  @param[in]	amount	The amount to reinvest
	*/
	void CashflowEngine::reinvest( const size_t period, const double& amount )
	{
		AQ_REQUIRE( period < nPeriods_, "Supplied period is outside valid range. Must be below: " << nPeriods_ << "; Actial value: " << period );

		if ( period < reinvestmentEndPeriod_ )
		{
			// # reinvest the availible proceeds
			if ( triggers_.size() > 0 )
			{
				for ( auto& trigger : triggers_ )
				{
					if ( boost::iequals( trigger->getName(), STRUCTURED_CREDIT_KEYS::TRIGGER_NAME_REINVEST ) )
					{
						// # this is where transaction specific formulation may be required - Andrei - any thoughts how we can
						// # make the parameterisation declarative ?
						const double lossGivenLoss = portfolioFlows_.cumulativeProportionLossGivenLoss[ period ];
						const bool triggerStatus = trigger->verify( period, lossGivenLoss );

						if ( triggerStatus )
						{
							loanPortfolio_->reinvestBalance( period, amount );
							portfolioFlows_.reinvestmentBalance[period] = amount;
						}
					}
					else if ( boost::iequals(trigger->getName(), STRUCTURED_CREDIT_KEYS::TRIGGER_NAME_REINVEST_VALE ) )
					{
						AQ_REQUIRE( trancheMonthlyScheduleData_.size() >= 3, "At least 3 tranches are required in order process " + STRUCTURED_CREDIT_KEYS::TRIGGER_NAME_REINVEST_VALE + " Trigger." );
						const TrancheScheduleData& equityTrancheScheduleData	= trancheMonthlyScheduleData_[ 0 ];
						const TrancheScheduleData& mezzTrancheScheduleData1		= trancheMonthlyScheduleData_[ 1 ];
						const TrancheScheduleData& mezzTrancheScheduleData2		= trancheMonthlyScheduleData_[ 2 ];

						const double protectedTrancheNotionalAmount = mezzTrancheScheduleData1.balanceEnd_[ period ] + mezzTrancheScheduleData2.balanceEnd_[ period ];
						const double aggregateLossAmount = portfolioFlows_.cumulativeLossGivenLoss[ period ];
						const double remainingThreshold = equityTrancheScheduleData.balanceEnd_[ period ];

						// # TODO need clarification if sxs to the net off losses for subord event triggers ? docs still being finalised
						// #aggregate_loss_amount = self$portfolio_flows[period, "cumulative_loss_given_loss"] - sum(my_engine$sxs$df$used[1:period])
						// # or
						// #remaining_threshold = self$tranches[[1]]$df$balance_end[period] + sum(my_engine$sxs$df$used[1:period])

						const double subEventLevel = ( aggregateLossAmount - remainingThreshold ) / protectedTrancheNotionalAmount;
						const bool triggerStatus = trigger->verify( period, subEventLevel);

						if (triggerStatus)
						{
							loanPortfolio_->reinvestBalance( period, amount );
							portfolioFlows_.reinvestmentBalance[ period ] = amount;
						}

						trigger->triggerStatus_.value[ period ] = subEventLevel;
						trigger->triggerStatus_.state[ period ] = triggerStatus;
					}
				}

			}
			else
			{
				// No triggers defined, so reinvest
				loanPortfolio_->reinvestBalance( period, amount );
				portfolioFlows_.reinvestmentBalance[period] = amount;
			}
		}
	}

	/* @brief	Perform principal allocation calculation
	*  @param[in]	period			The time period at which the principal amount should be paid
	*  @param[in]	principalAmount	The amount of principal to assign at the specified period
	*/
	void CashflowEngine::payPrincipal( const size_t period, const double& initialPrincipalCollection)
	{
		AQ_REQUIRE( period < nPeriods_, "Supplied period is outside valid range. Must be below: " << nPeriods_ << "; Actial value: " << period );

		double principalCollection = initialPrincipalCollection;

		OverridePayTypeEnum overridePayType = INACTIVE_OVERRIDE_PAY_TYPE;

		for ( auto trigger : triggers_ )
		{
			AQ_REQUIRE(trancheMonthlyScheduleData_.size() >= 3, "Require three or more tranches for trigger processing.");

			const TrancheScheduleData& equityTranche	= trancheMonthlyScheduleData_[0];
			const TrancheScheduleData& mezzTranche1		= trancheMonthlyScheduleData_[1];
			const TrancheScheduleData& mezzTranche2		= trancheMonthlyScheduleData_[2];

			const double remainingThreshold				= equityTranche.balanceEnd_[ period ];

			// # check the protected tranches, mezz 1 and two 2
			const double protectedTrancheNotionalAmount	= mezzTranche1.balanceEnd_[ period ] + mezzTranche2.balanceEnd_[ period ];

			// Here we check for a pro-rata sequential payoff flip trigger
			if ( boost::iequals( trigger->getName(), STRUCTURED_CREDIT_KEYS::TRIGGER_NAME_PRO_RATA_SEQUENTIAL_VALE ) )
			{
				const double aggregateLossAmount = portfolioFlows_.cumulativeLossGivenLoss[ period ];

				/*
				# TODO need clarification if sxs to the net off losses for subord event triggers? docs still being finalised
				# aggregate_loss_amount = self$portfolio_flows[period,"cumulative_loss_given_loss"] - sum(my_engine$sxs$df$used[1:period])
				# or
				# remaining_threshold = self$tranches[[1]]$df$balance_end[period]+ sum(my_engine$sxs$df$used[1:period])
				*/

				const double subEventLevel = ( aggregateLossAmount - remainingThreshold ) / protectedTrancheNotionalAmount;

				const bool triggerStatus = trigger->verify( period, subEventLevel );

				trigger->triggerStatus_.value[ period ] = subEventLevel;
				trigger->triggerStatus_.state[ period ] = triggerStatus;

				if ( triggerStatus )
				{
					// # override payoff
					overridePayType = INACTIVE_OVERRIDE_PAY_TYPE;
				}
				else
				{
					overridePayType = SEQUENTIAL_OVERRIDE_PAY_TYPE;
				}
			}
			else if ( boost::iequals( trigger->getName(), STRUCTURED_CREDIT_KEYS::TRIGGER_NAME_PRO_RATA_SEQUENTIAL_VALE_CUMULATIVE ) )
			{
				const double cumulativeDefaultBalance = portfolioFlows_.cumulativeDefaultBalance[ period ];
				const double subLevelCumulativeBalance = cumulativeDefaultBalance / ( protectedTrancheNotionalAmount + remainingThreshold );

				const bool triggerStatus = trigger->verify( period, subLevelCumulativeBalance );
				if (triggerStatus)
				{
					// # override payoff
					overridePayType = INACTIVE_OVERRIDE_PAY_TYPE;
				}
				else
				{
					overridePayType = SEQUENTIAL_OVERRIDE_PAY_TYPE;
				}
			}
		}

		double notionalAllProRataTranches	= 0.0;
		double trancheFactorSum				= 0.0;
		double proRataBalance				= 0.0;

		// # calculate notional_all_prorata_tranches
		// # tranche in principal waterall

		// Need to use ints here because we have a count-down loop
		const int nTranches = trancheDefinitions_.size();
		AQ_REQUIRE( nTranches > 0, "At least one tranche must be defined" );

		for ( int trancheIdx = (nTranches - 1); trancheIdx >= 0; trancheIdx-- )
		{
			// # check override
			if ((trancheDefinitions_[trancheIdx]->payType_ == PRO_RATA_TRANCHE_TYPE) && (overridePayType == INACTIVE_OVERRIDE_PAY_TYPE))
			{
				TrancheScheduleData& currentTrancheScheduleData = trancheMonthlyScheduleData_[ trancheIdx ];
				notionalAllProRataTranches += currentTrancheScheduleData.balanceEnd_[ period ];
				currentTrancheScheduleData.payType_[ period ] = PRO_RATA_TRANCHE_TYPE;
			}
		}

		for ( int trancheIdx = ( nTranches - 1 ); trancheIdx >=0 ; trancheIdx-- )
		{
			const std::shared_ptr<TrancheDefinition>& currentTrancheDefinition = trancheDefinitions_[ trancheIdx ];
			TrancheScheduleData& currentTrancheScheduleData = trancheMonthlyScheduleData_[ trancheIdx ];

			// # Check Tranche payment type and check override
			if ( ( currentTrancheDefinition->payType_ == PRO_RATA_TRANCHE_TYPE ) && ( overridePayType == INACTIVE_OVERRIDE_PAY_TYPE ) )
			{
				// # if hits first
				if ( proRataBalance == 0.0 )
				{
					proRataBalance = principalCollection;
				}

				double trancheFactor = 0.0;
				
				if ( notionalAllProRataTranches == 0 )
				{
						trancheFactor = 0.0;
				}
				else
				{
					trancheFactor = currentTrancheScheduleData.balanceEnd_[ period ] / notionalAllProRataTranches;
				}

				const double balanceEnd = currentTrancheScheduleData.balanceEnd_[ period ];
				const double principalAllocated = std::min( proRataBalance * trancheFactor, balanceEnd );

				// # assign the principal allocated to the tranche and recalculate tranche_balance_end
				currentTrancheScheduleData.balanceEnd_[ period ] = currentTrancheScheduleData.balanceEnd_[ period ] - principalAllocated;
				
				// # record principal allocated
				currentTrancheScheduleData.principalAllocated_[ period ] = principalAllocated;

				// # reduce the allocation from the period loss balance
				principalCollection = principalCollection - principalAllocated;
			}
			else if ( currentTrancheDefinition->payType_ == SEQUENTIAL_TRANCHE_TYPE || overridePayType == SEQUENTIAL_OVERRIDE_PAY_TYPE )
			{
				// # where pro rata seq trigger test fail we must overide pro-rata tranche types to sequential
				currentTrancheScheduleData.payType_[ period ] = SEQUENTIAL_TRANCHE_TYPE;

				const double principalAllocated = std::max( 0.0, std::min( currentTrancheScheduleData.balanceEnd_[ period ], principalCollection ) );

				// # assign the principal allocated to the tranche and recalculate tranche_balance_end
				currentTrancheScheduleData.balanceEnd_[ period ] = currentTrancheScheduleData.balanceEnd_[ period ] - principalAllocated;
				
				// # record principal allocated
				currentTrancheScheduleData.principalAllocated_[ period ] = principalAllocated;

				// # reduce the allocation from the period loss balance
				principalCollection = principalCollection - principalAllocated;
			}
			else
			{
				AQ_THROW( "Tranche type must be either 'sequential' or 'pro-rata'" );
			}
		} // Loop over tranches

		// # allocate principal to tranche payouts
		// # first lets see if it is a paying date, we must work back form maturity

		//const size_t monthsToMaturity = nPeriods_ - ( period + 1 );
		// # we only pay coupon on coupon date else zero so period to term must be integer div by freq
		// # is coupon date check - needs stub accomodation
		// # we currently will take freequency defn from tranche 1, all must have same freq

		// All the tranches will have the same coupon frequency.
		// For the time being use the couponFrequency from the first tranche.
		// if ( monthsToMaturity % trancheDefinitions_[ 0 ].couponFrequency_ == 0 )


		// Check if this is a coupon paying period
		const AQLDate& portfolioPaymentDate = portfolioPaymentDates_[ period ];

		const TrancheScheduleData& equityTrancheCouponScheduleData = trancheCouponScheduleData_[ 0 ];
		auto paymentDateIter = std::find( equityTrancheCouponScheduleData.paymentDate_.begin(),
										  equityTrancheCouponScheduleData.paymentDate_.end(),
										  portfolioPaymentDate );

		if ( paymentDateIter != equityTrancheCouponScheduleData.paymentDate_.end() )
		{

			// # find the index of the coupon payment date, use this to find the reset index
			const size_t payPeriodTranche = std::distance( equityTrancheCouponScheduleData.paymentDate_.begin(), paymentDateIter );

			const AQLDate& resetDate = equityTrancheCouponScheduleData.resetDate_[ payPeriodTranche ];

			auto resetDateIter = std::find( resetDates_.begin(), resetDates_.end(), resetDate );
			AQ_REQUIRE( resetDateIter != resetDates_.end(), "Could not find tranche reset date in resetDates vector" );

			const size_t resetPeriod = std::distance( resetDates_.begin(), resetDateIter );

			for (int trancheIdx = 0; trancheIdx < nTranches; trancheIdx++ )
			{
				TrancheScheduleData& currentTrancheMonthlyScheduleData = trancheMonthlyScheduleData_[ trancheIdx ];

				//const int startPeriod = period - trancheDefinitions_[ trancheIdx ].couponFrequency_ + 1; // *** Check this
				const size_t startPeriod = resetPeriod;

				double principalBalancePaid = 0.0;
				for ( size_t iPeriod = startPeriod; iPeriod <= period; iPeriod++ )
				{
					principalBalancePaid = principalBalancePaid + currentTrancheMonthlyScheduleData.principalAllocated_[ period ];
				}
				currentTrancheMonthlyScheduleData.principalPayment_[ period ] = principalBalancePaid;
			}
		}
	}

	/* @brief	Recalculate the tranche attachment and detachment points at the specified time period
	*  @param[in]	period	The time period at which to perform the calculation
	*/
	void CashflowEngine::calculateAttachDetach( const size_t period )
	{
		AQ_REQUIRE( period < nPeriods_, "Supplied period is outside valid range. Must be below: " << nPeriods_ << "; Actial value: " << period );

		double reinvestedBalance = 0.0;
		if (period < reinvestmentEndPeriod_)
		{
			reinvestedBalance = loanPortfolio_->getReinvestmentBalance( period );
		}
		const double poolBalance = loanPortfolio_->getBalanceEnd( period ) + reinvestedBalance;

		// self$portfolio_flows[period, 'balance_end'] < -PoolBalance

		// evaluation from equity bottom up
		// tranche in interest waterfall

		double trancheSubordination = 0.0;
		const size_t nTranches = trancheDefinitions_.size();
		for (size_t trancheIdx = 0; trancheIdx < nTranches; trancheIdx++)
		{
			TrancheScheduleData& currentTrancheScheduleData = trancheMonthlyScheduleData_[ trancheIdx ];

			const double attachmentPoint = ( trancheSubordination / poolBalance );
			currentTrancheScheduleData.attachmentPoint_[ period ] = attachmentPoint;

			const double detachmentPoint = ( currentTrancheScheduleData.balanceEnd_[ period ] + trancheSubordination ) / poolBalance;
			currentTrancheScheduleData.detachmentPoint_[ period ] = detachmentPoint;

			trancheSubordination += currentTrancheScheduleData.balanceEnd_[ period ];
		}
	}
	
	/* @brief	Calculates the regulatory capital requirement at the specified time period
	*  @param[in]	period	The period at which to perform the calculation
	*/
	void CashflowEngine::calculateRegulatoryCapital( const size_t period )
	{
		AQ_REQUIRE( period < nPeriods_, "Supplied period is outside valid range. Must be below: " << nPeriods_ << "; Actial value: " << period );

		const size_t nTranches = trancheDefinitions_.size();

		for (size_t trancheIdx = 0; trancheIdx < nTranches; trancheIdx++ )
		{
			TrancheScheduleData currentTrancheScheduleData = trancheMonthlyScheduleData_[ trancheIdx ];

			const size_t maximumLoanPeriod = loanPortfolio_->calculateMaximumLoanPeriod();
			if ( period < maximumLoanPeriod )
			{
				if (currentTrancheScheduleData.balanceEnd_[ period ] <= 0.01 )
				{
					currentTrancheScheduleData.riskWeight_[ period ] = 0.0;
				}
				else
				{
					switch( supervisoryType_ )
					{
						case LEGACY_SUPERVISORY_TYPE:
						{
							currentTrancheScheduleData.riskWeight_[ period ] = legacyRules::supervisoryFormula( currentTrancheScheduleData.attachmentPoint_[ period ],
																											   (currentTrancheScheduleData.detachmentPoint_[ period ]
																												 - currentTrancheScheduleData.attachmentPoint_[ period ] ),
																											 kirb_,
																											 elgd_,
																											 nDiversityMetric_ );
							break;
						}
						case IRBA_SUPERVISORY_TYPE:
						{
							const RankTypeEnum rankType = ( trancheIdx == ( nTranches - 1 )) ? SENIOR_RANK_TYPE : NON_SENIOR_RANK_TYPE;

							const size_t regCapitalMaturityYears = ( regulatoryCapitalMaturity_ - ( period + 1 ) ) / 12;

							currentTrancheScheduleData.riskWeight_[ period ] = secIrbaRules::secIrbaRiskWeight( poolType_,
																											    nDiversityMetric_,
																											    rankType,
																											    kirb_,
																											    elgd_,
																											    regulatoryCapitalMaturity_,
																											    currentTrancheScheduleData.attachmentPoint_[ period ],
																											    currentTrancheScheduleData.detachmentPoint_[ period ] );
							break;
						}
						default:
							AQ_THROW( "supervisory_type must be either 'NEW' or 'OLD'" );
							break;
					}

				}

			}

		}
	}

	/* @brief Main calculation entry point
	*/
	void CashflowEngine::evaluate()
	{
		// Clear away any previous reinvestment results
		loanPortfolio_->clearAllReinvestmentResults();

		populateYieldCurveData();
		const size_t nTranches = trancheDefinitions_.size();

		for ( size_t iPeriod = 0; iPeriod < nPeriods_; iPeriod++ )
		{
			if ( iPeriod > 0 )
			{		
				for ( size_t trancheIdx = 0; trancheIdx < nTranches; trancheIdx++ )
				{
					/*
					# is period loss
					# set this period balance_start to prior period balance_end - this implements cork screw but is not used in first period
					# since it's start point is defined free of loss, prepayment or default
					*/
					TrancheScheduleData& currentTrancheScheduleData = trancheMonthlyScheduleData_[ trancheIdx ];
					currentTrancheScheduleData.balanceStart_[ iPeriod ] = currentTrancheScheduleData.balanceEnd_[ iPeriod - 1 ];
				}
				if ( syntheticExcessSpread_ )
				{
					// # roll forward sxs spread
					SyntheticExcessSpreadAllocation syntheticExcessSpreadAllocation = syntheticExcessSpread_->getAllocation( iPeriod );
					const SyntheticExcessSpreadAllocation previousSyntheticExcessSpreadAllocation = syntheticExcessSpread_->getAllocation( iPeriod - 1 );
					syntheticExcessSpreadAllocation.levelStart = previousSyntheticExcessSpreadAllocation.levelEnd;
					syntheticExcessSpread_->setAllocation( iPeriod, syntheticExcessSpreadAllocation );
				}
			}

			/*
			# we only ever need this period and the previous period
			# Add SXS Credit
			*/
			portfolioFlows_.period[ iPeriod ] = iPeriod;
			portfolioFlows_.balanceStart[ iPeriod ]			= loanPortfolio_->getBalanceStart( iPeriod );
			portfolioFlows_.balanceEnd[ iPeriod ]			= loanPortfolio_->getBalanceEnd( iPeriod );
			portfolioFlows_.principalComponent[ iPeriod ]	= loanPortfolio_->getPrincipalComponent( iPeriod );
 
			// # TODO add a recovery lag
			portfolioFlows_.severityRecovered[ iPeriod ]	= loanPortfolio_->getSeverityRecovered( iPeriod );
			portfolioFlows_.prepayment[ iPeriod ]			= loanPortfolio_->getPrepayment( iPeriod );
			portfolioFlows_.lossGivenLoss[ iPeriod ]		= loanPortfolio_->getLossGivenLoss( iPeriod );
			portfolioFlows_.interestComponent[ iPeriod ]	= loanPortfolio_->getInterestComponent( iPeriod );

			// # cumulative variable capture
			if ( iPeriod > 0 )
			{
				// #subsequent flows we need to buffer
				portfolioFlows_.cumulativeDefaultBalance[ iPeriod ]	= portfolioFlows_.cumulativeDefaultBalance[ iPeriod - 1 ] + loanPortfolio_->getDefaultBalance( iPeriod );
				portfolioFlows_.cumulativeLossGivenLoss[ iPeriod ]	= portfolioFlows_.cumulativeLossGivenLoss[ iPeriod - 1 ] + loanPortfolio_->getLossGivenLoss( iPeriod );
				portfolioFlows_.cumulativeSeverityRecovered[ iPeriod ] = portfolioFlows_.cumulativeSeverityRecovered[ iPeriod - 1 ] + loanPortfolio_->getSeverityRecovered( iPeriod );
			}
			else
			{
				portfolioFlows_.cumulativeDefaultBalance[ iPeriod ]	= loanPortfolio_->getDefaultBalance( iPeriod );
				portfolioFlows_.cumulativeLossGivenLoss[ iPeriod ]	= loanPortfolio_->getLossGivenLoss( iPeriod );
				portfolioFlows_.cumulativeSeverityRecovered[ iPeriod ] = loanPortfolio_->getSeverityRecovered( iPeriod );
			}

			// # cumulative proportion %
			portfolioFlows_.cumulativeProportionDefaultBalance[ iPeriod ]	= portfolioFlows_.cumulativeDefaultBalance[ iPeriod ] / portfolioBalanceSize_;
			portfolioFlows_.cumulativeProportionLossGivenLoss[ iPeriod ]	= portfolioFlows_.cumulativeLossGivenLoss[ iPeriod ] / portfolioBalanceSize_;
			portfolioFlows_.cumulativeProportionSeverityRecovered[ iPeriod ] = portfolioFlows_.cumulativeSeverityRecovered[ iPeriod ] / portfolioBalanceSize_;

			// # total principal collections - these may be reinvested if available
			double principalCollection = loanPortfolio_->calculatePrincipalCollection( iPeriod );
			loanPortfolio_->setPrincipalCollection( iPeriod, principalCollection );

			payInterest( iPeriod );

			/*
			# Note - loss given loss is a property of the porfolio, sxs is a property of transaction
			# therefore we should not adjust portfolio but rather reference the sxs at the tx level
			*/
			const double periodLoss = loanPortfolio_->getLossGivenLoss( iPeriod );
			payLosses( iPeriod, periodLoss );

			// # add sxs spread credits to principal collection
			double syntheticExcessSpreadUsed = 0.0;
			if ( syntheticExcessSpread_ )
			{
				syntheticExcessSpreadUsed = syntheticExcessSpread_->getAllocation( iPeriod ).levelStart - syntheticExcessSpread_->getAllocation( iPeriod ).levelEnd;
			}

			/*
			# TODO the sxs when used is considered as a make whole so losses are converted to principal
			# and eligible for reinvestment
			*/
			principalCollection += syntheticExcessSpreadUsed;
			reinvest( iPeriod, principalCollection );

			// Consider updating balanceEnd here, after the reinvestment calculation has been performed.
			//portfolioFlows_.balanceEnd[iPeriod] = loanPortfolio_->getBalanceEnd(iPeriod);

			/*
			#TODO check if this credit of sxs spread gets deducted twice from principals

			# net reinvestment balance off from principal collections
			*/
			principalCollection -= loanPortfolio_->getReinvestmentBalance( iPeriod );
			payPrincipal( iPeriod, principalCollection );

			calculateAttachDetach( iPeriod );
			calculateRegulatoryCapital( iPeriod );
		}

		liabilityFlows_ = outputPortfolioFlowsToDataFrame();

		for ( size_t trancheIdx = 0; trancheIdx < nTranches; trancheIdx++ )
		{
			TrancheScheduleData& currentTrancheScheduleData = trancheMonthlyScheduleData_[ trancheIdx ];
			currentTrancheScheduleData.calculatePVs();
			DataFrame trancheAsDataFrame = currentTrancheScheduleData.outputToLabelledDataFrame();
			liabilityFlows_.combineColumns( trancheAsDataFrame );
		}
	}

	 /* @brief	Check if PoolBalanceEnd minus Balance Ends of all tranches equals 0
	 *			i.e. verify at each timestep that ASSETS (Portfolio Loans) matches LIABILITIES (tranches).
	 *  @param[in]	liabilityFlows		A dataFrame containing the liabilityFlows calculated by evaluate() and returned by getLiabilityFlows()
	 *  @param[out]	validRows			A vector bool indicating whether equilibrium is maintained at each timestep
	 *  @param[out]	residuals			A vector of double indicating the residual at each timestep (ASSETS - LIABILITIES)
	*/
	void CashflowEngine::validateEquilibrium( const DataFrame& liabilityFlows, std::vector<bool>& validRows, std::vector<double>& residuals )
	{
		validRows.clear();
		residuals.clear();

		// Put these constants in common constants?
		const std::string POOL_BALANCE_END = "PoolBalanceEnd";
		const std::string BALANCE_END = "BalanceEnd";

		// The POOL_BALANCE_END is the loan balance at the end of each time period.
		// The outstanding loan amount is forms the ASSET part of the equilibrium
		const std::vector<double> poolBalanceEnd = liabilityFlows.getColumnAsDoubleVector( POOL_BALANCE_END );
		const size_t nPeriods = poolBalanceEnd.size();

		// Store the total of all the Tranche BalanceEnds at each time period
		std::vector<double> totalTrancheBalanceEnd( nPeriods, 0.0 );

		// Iterate through all columns of the DataFrame and locate all columsn ending in BalanceEnd ( excluding PoolBalanceEnd )
		// These columns represent the BalanceEnd in each Tranche.
		// The Tranches represent the LIABILITY side of the equilibrium.
		std::vector<std::string> columnNames = liabilityFlows.getColumnNames();
		const size_t nColumns = columnNames.size();
		for ( size_t i=0; i<nColumns; i++ )
		{
			const std::string columnName = columnNames[i];
			if ( columnName == POOL_BALANCE_END )
			{
				// Skip over POOL_BALANCE_END. Only include Tranche BALANCE_END
				continue;
			}

			if ( boost::algorithm::ends_with( columnName, BALANCE_END ) )
			{
				// Found a Tranche BALANCE_END column. Include in Tranche results
				std::vector<double> trancheBalanceEnd = liabilityFlows.getColumnAsDoubleVector( columnName );
				for ( size_t period=0; period<nPeriods; period++)
				{
					totalTrancheBalanceEnd[ period ] += trancheBalanceEnd[ period ];
				}
			}
		}

		// Calculate residuals i.e. ( ASSETS - LIABILITIES )
		validRows.resize( nPeriods );
		residuals.resize( nPeriods );

		// This tolerance matches that in MGEN.
		const double tolerance = 1.0e-5;

		for ( size_t period = 0; period < nPeriods; period++ )
		{
			const double residual = poolBalanceEnd[ period ] - totalTrancheBalanceEnd[ period ];
			residuals[ period ] = residual;
			validRows[ period ] = ( std::fabs( residual ) < tolerance ) ? true : false;
		}
	}

	/* @brief	Attaches an optional Synthetic Excess Spread tranche to the capital structure
	*  @param[in]	syntheticExcessSpread	The SyntheticExcessSpread object to use
	*/
	void CashflowEngine::setSyntheticExcessSpread( const std::shared_ptr<SyntheticExcessSpread>& syntheticExcessSpread )
	{
		syntheticExcessSpread_ = syntheticExcessSpread;
	}

	/* @brief	A helper function which converts portfolio flows into a dataframe, suitable for output
	*/
	DataFrame CashflowEngine::outputPortfolioFlowsToDataFrame() const
	{
		DataFrame dataFrame;

		dataFrame.addColumn( "Period", portfolioFlows_.period );
		dataFrame.addColumn( "PoolBalanceStart", portfolioFlows_.balanceStart);
		dataFrame.addColumn( "PoolBalanceEnd", portfolioFlows_.balanceEnd);
		dataFrame.addColumn( "PrincipalComponent", portfolioFlows_.principalComponent);
		dataFrame.addColumn( "ReinvestmentBalance", portfolioFlows_.reinvestmentBalance);

		dataFrame.addColumn( "SeverityRecovered", portfolioFlows_.severityRecovered);
		dataFrame.addColumn( "Prepayment", portfolioFlows_.prepayment);
		dataFrame.addColumn( "LossGivenLoss", portfolioFlows_.lossGivenLoss);

		dataFrame.addColumn( "InterestComponent", portfolioFlows_.interestComponent);

		dataFrame.addColumn( "CumulativeDefaults", portfolioFlows_.cumulativeDefaultBalance);
		dataFrame.addColumn( "CumulativeLoss", portfolioFlows_.cumulativeLossGivenLoss);
		dataFrame.addColumn( "CumulativeRecovery", portfolioFlows_.cumulativeSeverityRecovered);

		return dataFrame;
	}

	/* @brief Main output function to retrieve the results of the waterfall calculation
	*  @returns	A dataframe containing the cashflow breakdown and tranche allocation for the lifetime of the portfolio
	*/
	DataFrame CashflowEngine::getLiabilityFlows() const
	{
		return liabilityFlows_;
	}

	/* @brief	Helper function: For a given TrancheDefinition, get the corresponding MonthlySchedule data calculated by evaluate()
	*  @param[in]	trancheDefinition	Specify which tranche we are interested in
	*  @returns		The monthly schedule data for that tranche.
	*/
	TrancheScheduleData CashflowEngine::getTrancheMonthlyScheduleDataForTranche( const std::shared_ptr<TrancheDefinition>& trancheDefinition ) const
	{
		auto trancheDefinitionIter = std::find( trancheDefinitions_.begin(),
												trancheDefinitions_.end(),
												trancheDefinition );

		AQ_REQUIRE( trancheDefinitionIter != trancheDefinitions_.end(), "Cannot find specified Tranche in the CashflowEngine." );
		
		const size_t trancheIdx = std::distance(trancheDefinitions_.begin(), trancheDefinitionIter );

		const TrancheScheduleData  trancheMonthlyScheduleData = trancheMonthlyScheduleData_[ trancheIdx ];
		
		return trancheMonthlyScheduleData;
	}

}

