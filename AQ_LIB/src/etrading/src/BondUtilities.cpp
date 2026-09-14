#include "BondUtilities.h"
#include "CommonConstants.h"
#include "LabelValueBlockValidation.h"
#include "AQObjUtilities.h"

#include <string>
#include <vector>
#include <iterator>
#include <algorithm>

namespace etrading
{
	/* @brief			Validate Bond Description Label Value Block Keys
	*  @return		    Returns a vector of expected Bond LVB Keys
	*/
	std::vector<std::string> bondDescriptionLVBKeys()
	{
		const std::string arr[] =
		{
			BOND_KEY::BOND_DESCRIPTION,
			BOND_KEY::BOND_TYPE,
			BOND_KEY::BOND_QUOTE_CONVENTION,
			BOND_KEY::CURRENCY,
			BOND_KEY::ISIN,
			BOND_KEY::YIELD_TYPE,
			BOND_KEY::IS_CLEAN_PRICE,
			BOND_KEY::QUOTED_MARGIN
		};

		std::vector< std::string > expectedKeys(arr, arr + sizeof(arr) / sizeof(arr[0]));
		return expectedKeys;
	}

	/* @brief			Validate Bond Label Value Block Keys, which include description LVB and Schedule LVB
	*  @return		    Returns a vector of expected Bond LVB Keys
	*/
	std::vector<std::string> fixedBondLVBKeys()
	{
		std::vector<std::string> expectedKeys = bondDescriptionLVBKeys();
		std::vector<std::string> scheduleKeys = etrading::FixedSchedule::lvbKeys();
		expectedKeys.insert(expectedKeys.end(), scheduleKeys.begin(), scheduleKeys.end());

		return expectedKeys;
	}

	/* @brief			Validate Bond Label Value Block Keys, which include description LVB and Schedule LVB
	*  @return		    Returns a vector of expected Bond LVB Keys
	*/
	std::vector<std::string> floatBondLVBKeys()
	{
		std::vector<std::string> expectedKeys = bondDescriptionLVBKeys();
		std::vector<std::string> scheduleKeys = etrading::FloatSchedule::lvbKeys();
		expectedKeys.insert(expectedKeys.end(), scheduleKeys.begin(), scheduleKeys.end());

		return expectedKeys;
	}

    /* @brief			Validate Bond Description Label Value Block Keys
	*  @param [in]		keysFromUser		A list of keys from user input
	*  @param [in]		validateKeys		True to do the verification. Default to true
	*  @param [in]		withScheduleKeys	True to include keys from Schedule (Fixed/Float). Default to true
	*/
	void validateBondDescriptionLVBKeys( const LabelValueBlock& bondDescriptionLVB, bool validateKeys, const std::string& LVBname )
	{
		if ( !validateKeys )
		{
			return;
		}

        std::vector< std::string > expectedKeys     = etrading::bondDescriptionLVBKeys();
		std::vector< std::string > actualKeys       = bondDescriptionLVB.getKeys();
        
        validateKeysForLVB( expectedKeys, actualKeys, validateKeys, LVBname );

    }
        
	 /* @brief Create the Bond Label Value Block from a AQObj Free-Object
	 *  @param [in]		freeObject				freeObject
	 *  @param [in]		objectName			    objectName
	 *  @Return     A Label Value Block containing Bond Schedule Data
	 */
	LabelValueBlock createBondLVBFromFreeObject(const FreeObject& freeObject, const std::string& objectName)
	{
		LabelValueBlock bondScheduleLVB;
		const auto schemaNames = freeObject.keyNames();

		for (unsigned int i = 0; i < schemaNames.size(); i++)
		{
			// Search for the Bond Schedule LVB Key
			auto schemaName = schemaNames[i];
			if (schemaName == toString(BOND))
			{
				auto keys = freeObject.getValuesCopy(schemaName, 0);
				auto values = freeObject.getValuesCopy(schemaName, 1);

				bondScheduleLVB = buildSingleLabelValueBlock(fromVariantToStdStringVector(keys), fromVariantToStdStringVector(values));
			}
		}

		return bondScheduleLVB;
	}

	/* @brief Function to convert the bond accrual frequency string to a year fraction
    *  @param [in]		accrualFrequency  An enum representing the Bond accrual frequency e.g. ANNUAL, SEMI-ANNUAL
	*  @Return          returns accrual year fraction
    */
	double convertBondFrequencyToYearFraction(const FrequencyEnum& accrualFrequency )
	{
		double yearFraction;
		switch ( accrualFrequency )
        {
            case ANNUAL_FREQUENCY:
                
                yearFraction = 1.0;
                return yearFraction;
                break;

            case SEMI_ANNUAL_FREQUENCY:
                
                yearFraction = 0.5;
                return yearFraction;
                break;

            case QUARTERLY_FREQUENCY:
                
                yearFraction = 0.25;
                return yearFraction;
                break;

            case MONTHLY_FREQUENCY:
                
                yearFraction = 1/12.0;
                return yearFraction;
                break;

            case WEEKLY_FREQUENCY:
                
                yearFraction = 1/52.0;
                return yearFraction;
                break;

            case DAILY_FREQUENCY:
                
                yearFraction = 1/365.0;
                return yearFraction;
                break;

            default:
		        AQ_THROW( "Invalid accrual frequency. Only 'ANNUAL', 'SEMI-ANNUAL', 'QUARTERLY', 'MONTHLY', 'WEEKLY' or 'DAILY' supported." );
                break;
        }
	}



    /* @brief   Function to calculate the start index to get the index for the first bond coupon that is not in the past
    *  @param [in]		settlementDate                      The bond settlement date
    *  @param [in]		bondPaymentlDatesForIndexation      A vector of all bond cashflow payment dates, these dates are assumed to be SORTED
	*  @Return          returns the index position of the active cashflow given a vector of dates. Will return the final index position if all cashflows are in the past
    */
    unsigned int getBondActiveCashflowIndex( const AQLDate& settlementDate, const std::vector< AQLDate >& bondPaymentlDatesForIndexation )
    {   
        // In Bloomberg, the first active coupon (payment date) is greater than (not equal) the settle date.
        auto nextDayToSettleDate = settlementDate;
        nextDayToSettleDate.addDays(1);

        auto resultIndex = getBondCashflowIndex(nextDayToSettleDate, bondPaymentlDatesForIndexation);
      
        return resultIndex;
    }

    /* @brief   Function to calculate the start index to get the index for the first bond coupon that is not in the past
    *  @param [in]		settlementDate                      The bond settlement date
    *  @param [in]		bondPaymentDatesForIndexation      A vector of all bond cashflow payment dates, these dates are assumed to be SORTED
	*  @Return          returns the index position of the active cashflow given a vector of dates. Will return the final index position if all cashflows are in the past
    */
    unsigned int getBondCashflowIndex( const AQLDate& searchDate, const std::vector< AQLDate >& bondPaymentlDatesForIndexation )
    {
        const size_t numberOfCashflows = bondPaymentlDatesForIndexation.size();
        AQ_THROW_IF( numberOfCashflows == 0, "The Bond has no Cashflows. Unable to find the active cashflow." );

        // Iterate over the Bond Payment dates, which are sored and return the first cashflow yet to pay i.e. the active bond coupon
        // LOWER_BOND: Find First Element in Payment Dates NOT LESS than the Settlement Date i.e. GREATER THAN OR EQUAL to the Settlement Date
        auto it                         = std::lower_bound( bondPaymentlDatesForIndexation.begin(), bondPaymentlDatesForIndexation.end(), searchDate );
                
        // Return the last index if all dates are historic and in the past
        size_t resultIndex              = numberOfCashflows - 1;
        if ( it == bondPaymentlDatesForIndexation.end() )
        {
            return resultIndex;
        }
        
        // Get the Result Index as the distance from the start of the Payment Date Vector
        resultIndex                     = std::distance( bondPaymentlDatesForIndexation.begin(), it );
        return resultIndex;
    }

    /* @brief   Function to calculate the index of a bond's cashflow relative to the first non-past coupon
    *  @param [in]		firstActiveCashflowIndex            The first active coupon (payment) date index
    *  @param [in]		searchPaymentDate                   The search payment date
    *  @param [in]		bondPaymentlDatesForIndexation      A vector of all bond cashflow payment dates, these dates are assumed to be SORTED
	*  @Return          returns the index of the cashflow. The first future cashflow has index 0, the second future cashflow is 1 etc ....
    */
    unsigned int getBondRelativeCashflowIndex( unsigned int& firstActiveCashflowIndex, const AQLDate& searchPaymentDate, const std::vector< AQLDate >& bondPaymentlDatesForIndexation )
    {
        const size_t numberOfCashflows = bondPaymentlDatesForIndexation.size();
        AQ_THROW_IF( numberOfCashflows == 0, "The Bond has no Cashflows. Unable to find the active cashflow." );

        // Check if the searchDate is in the list of bond cashflow Dates
        AQ_THROW_IF( std::find( bondPaymentlDatesForIndexation.begin(), bondPaymentlDatesForIndexation.end(), searchPaymentDate ) == bondPaymentlDatesForIndexation.end(), "Unable to evaluate the Bond Cashflow. Invalid bond payment dates(s) provided." );

        // Get the Active Cashflow Index and SearchPaymentDate Index
        unsigned int resultIndex                        = 0;
        const unsigned int lastPositionIndex            = numberOfCashflows - 1;
        
        // Don't search if we know the payment date if we know it is the last payment, since it is unneccessary. We know the position of the last cashflow.
        if ( searchPaymentDate == bondPaymentlDatesForIndexation[ lastPositionIndex] )
        {
            resultIndex                                 = lastPositionIndex - firstActiveCashflowIndex;
            return resultIndex;
        }
        
        // Calculate the relative index position of the search date to the active coupon
        
        const unsigned int searchPaymentDateIndex       = getBondCashflowIndex( searchPaymentDate, bondPaymentlDatesForIndexation );
        
        // Return zero when the search date index is before the first active cashflow index
        if ( searchPaymentDateIndex < firstActiveCashflowIndex )
        {
            return 0;
        }
        
        resultIndex                                     = searchPaymentDateIndex - firstActiveCashflowIndex;
        return resultIndex;
    }

    /* @brief   Function to extract the first future date from a SORTED vector of historic and future cashflow dates using Bond Payment Dates for Indexation
    *  @param [in]		settlementDate                      The bond settlement date
    *  @param [in]		bondCashflowDates                   A vector of all bond cashflow dates, these dates are assumed to be SORTED. These dates can be the accrual start, end, payment or fixing dates
    *  @param [in]		bondPaymentlDatesForIndexation      A vector of all bond payment dates, these dates are our reference dates to determine if a bond coupon should be considered historic or in the future
	*  @Return          returns the first future cashflow date and index. Today is considered in the future. Will return the last date in the dates vector if all dates are historic and in the past
    */
    std::pair<AQLDate, size_t> getActiveCashflowDateAndIndex( const AQLDate& settlementDate, const std::vector< AQLDate >& bondCashflowDates, const std::vector< AQLDate >& bondPaymentlDatesForIndexation )
    {
        AQ_THROW_IF( bondPaymentlDatesForIndexation.size() != bondCashflowDates.size(), "Inconsistent Bond Schedule. The number of bond cashflows does not match the number of bond payment dates." );
        
        const size_t numberOfCashflows = bondPaymentlDatesForIndexation.size();
        AQ_THROW_IF( numberOfCashflows == 0, "There are no Bond Cashflows to evaluate." );
        
        // Get the Result Index as the distance from the start of the Payment Date Vector
        auto resultIndex = getBondActiveCashflowIndex(settlementDate,  bondPaymentlDatesForIndexation);

        AQLDate cashflowDateResult = bondCashflowDates[ resultIndex ];

        return std::make_pair(cashflowDateResult, resultIndex);
    }

    /* @brief			Calculate the bond full periods from paymentDate1 to paymentDate2
    *  @param [in]		paymentDate1    Payment date one
    *  @param [in]		paymentDate2    Payment date two
    *  @param [in]		bondDaycount   Bond Daycount Basis
    *  @param [in]		bondAccrualFrequency   Bond accrual frequency
    *  @return			Full coupon period days
    */
    double getBondFullCouponPeriodDays(const AQLDate& paymentDate1, const AQLDate& paymentDate2, const DayCountEnum & bondDaycount, const FrequencyEnum & bondAccrualFrequency)
    { 
        double totalCouponDays      = 0.0;
        double yearFraction = convertBondFrequencyToYearFraction( bondAccrualFrequency );

        switch ( bondDaycount )
        {
            case ACT_ACT_DAYCOUNT:
        
                totalCouponDays = paymentDate1.intervalDays( paymentDate2 );     
                break;
            case ACT_365_DAYCOUNT:
            case ACT_365_FJ_DAYCOUNT:
            {
                totalCouponDays = 365.0 * yearFraction;
                break;
            }
            case ACT_360_DAYCOUNT:
            case N30_360_DAYCOUNT:
            case E30_360_DAYCOUNT:
            case E30_360_ISDA_DAYCOUNT:
            {
                totalCouponDays = 360.0 * yearFraction;
                break;
            }
           default:
                AQ_THROW( "Bond only support the following daycount methods: 'ACT/ACT', 'ACT/365', 'NL/365', '30/360', '30/360 ISMA', '30E/360 ISDA', 'ACT/360'" );
                break;
        }

        AQ_THROW_IF( totalCouponDays == 0, "Invalid Bond Coupon Dates. Cannot evaluate a Bond Coupon with full coupon period as zero days." );
        
        return totalCouponDays;
    }

    /* @brief			Calculate the bond actual coupon period days from paymentDate1 to paymentDate2
    *  @param [in]		paymentDate1    Payment date one
    *  @param [in]		paymentDate2    Payment date two
    *  @param [in]		bondDaycount    Bond Daycount Basis
    *  @return			Actual coupon period days
    */
    double getBondActualCouponPeriodDays(const AQLDate& paymentDate1, const AQLDate& paymentDate2, const DayCountEnum & bondDaycount)
    { 
        double actualCouponDays      = 0.0;
        switch ( bondDaycount )
        {
            case ACT_360_DAYCOUNT:
            case ACT_365_DAYCOUNT:
            case ACT_ACT_DAYCOUNT:

                actualCouponDays = paymentDate1.intervalDays( paymentDate2 );     
                break;
            case ACT_365_FJ_DAYCOUNT:
            {
                const double yearFraction = getYearFraction( paymentDate1, paymentDate2, bondDaycount );
                actualCouponDays = 365.0 * yearFraction;
                break;
            }
            case N30_360_DAYCOUNT:
            case E30_360_DAYCOUNT:
            case E30_360_ISDA_DAYCOUNT:
            {
                const double yearFraction = getYearFraction( paymentDate1, paymentDate2, bondDaycount );
                actualCouponDays = 360.0 * yearFraction;
                break;
            }
           default:
                AQ_THROW( "Bond only support the following daycount methods: 'ACT/ACT', 'ACT/365', 'NL/365', '30/360', '30/360 ISMA', '30E/360 ISDA', 'ACT/360'" );
                break;
        }

        return actualCouponDays;
    }

    /* @brief			Check if it is JGBBond with issue date earlier than the market convention change date
    *  @param [in]		bondCalculationType Bond calculation type
    *  @param [in]		issueDateStr        Issue date of the bond
    *  @return			True/false
    */
    bool isBondJGBIssuedPriorToMarketConventionChangeDate(const BondCalculationTypeEnum& bondCalculationType, const AQLString& issueDateStr)
    {
        //Bonds that are issued prior to 3/1/2001 are given one extra day in the first coupon period.
        if (isJapaneseGovenmentBond(bondCalculationType))
        {
            AQLDate jgbMarketConventionChangeDate = AQLDate("20010301");
            AQLDate issueDt    = stringToDate( issueDateStr, "#Error: Invalid 'IssueDate'" );

            if (issueDt < jgbMarketConventionChangeDate)
            {
                return true;
            }
        }

        return false;
    }


    /* @brief			Check if the bond calculation type is JGB (JapaneseGovenmentBond)
    *  @param [in]		bondCalculationType   Bond calculation type
    *  @return			True for JGB bonds
    */
    bool isJapaneseGovenmentBond(const BondCalculationTypeEnum& bondCalculationType)
    {
        return (bondCalculationType == TYPE51_JAPANESE_GOVERNMENT_BONDS || bondCalculationType == TYPE235_JAPANESE_GOVERNMENT_BONDS);
    }

    /* @brief			Check if the bond calculation type is Italian
    *  @param [in]		bondCalculationType   Bond calculation type
    *  @return			True for Italian bonds
    */
    bool isItalianGovenmentBond(const BondCalculationTypeEnum& bondCalculationType)
    {
        return (bondCalculationType == TYPE523_ITALY_TRSY_BONDS || bondCalculationType == TYPE527_ITALY_TRSY_BILL || bondCalculationType == TYPE529_ITALY_2Y_TRSY_BILL);
    }


    /* @brief			Bond's notinoal exchange at maturity
    *  @param [in]		bondCalculationType     Bond calculation type
    *  @param [in]		effectiveDate           Bond effective date
    *  @param [in]		maturityDate            Bond maturity date
    *  @param [in]		notional                Bond face value
    *  @param [in]		issuePrice              Bond issue price 
    *  @param [in]		taxRate                 Bond tax rate
    *  @param [in]		paymentDates            Bond paymentDates excluding the upfront cashflow if there is one
    *  @return			Bond's notinoal exchange at maturity
    */
    double getBondNotionalExchangeAtMaturity(const BondCalculationTypeEnum& bondCalculationType, const AQLDate& effectiveDate, const AQLDate& maturityDate, double notional, double issuePrice, double taxRate, const std::vector<AQLDate>& paymentDates) 
    {

        double notionalExchangeAtMaturity = notional;

        if (bondCalculationType == TYPE523_ITALY_TRSY_BONDS)
        {
            AQLDate italianBTPSMarketConventionChangeDate = AQLDate("19970101");

            auto firstAccrualStartDt = effectiveDate;
            auto maturityDt = maturityDate;

            if (firstAccrualStartDt < italianBTPSMarketConventionChangeDate)
            {
                if (boost::math::isnan(issuePrice))
                {
                	{ std::ostringstream aqMsg77;
aqMsg77 << "issuePrice is mandatory for bond calculationType523 when first accrualStartDt is earlier than '" << italianBTPSMarketConventionChangeDate.stringWithFormat() << "'"; AQ_THROW( aqMsg77.str() ); }
                }

                if (boost::math::isnan(taxRate))
                {
                    //default taxRate is 12.5%
                	taxRate = 0.125;
                }

                 //1) From BB CalculationType523, Intermediate Value: IV=(100 - ((100-IP)*TR*(DTM-(DTM-DNC))/DTM)*1000, where IP is Issued Price, TR is Tax Rate, DTM is IssuedDateToMaturityDt, DNC is IssuedDateToFirstPaymentDtLaterThan19970101. 

                // IssuedDateToMaturityDt
                int dtm = firstAccrualStartDt.intervalDays(maturityDt);

                // IssuedDateToFirstPaymentDtLaterThan19970101
                auto targetPaymentDtIter = std::find_if(paymentDates.begin(), paymentDates.end(), [italianBTPSMarketConventionChangeDate](const AQLDate& paymentDate) { return paymentDate >= italianBTPSMarketConventionChangeDate;});
                if (targetPaymentDtIter == paymentDates.end())
                {
                	{ std::ostringstream aqMsg78;
aqMsg78 << "cannot find a payment date later than '" << italianBTPSMarketConventionChangeDate.stringWithFormat() << "'"; AQ_THROW( aqMsg78.str() ); }
                }
                AQLDate targetPaymentDt = *targetPaymentDtIter;

                int dnc = firstAccrualStartDt.intervalDays(targetPaymentDt);
                            
                //(DTM-(DTM-DNC))/DTM is simplified into: DNC/DTM
                double yearFraction = dnc * 1.0 / dtm;

                double iv = (100.0 - (100.0 - issuePrice) * taxRate * yearFraction) * 1000.0;

                if (iv < 0)
                {
                	{ std::ostringstream aqMsg79;
aqMsg79 << "Gross principal cannot be negative '" << iv << "'"; AQ_THROW( aqMsg79.str() ); }
                }


                //2) From BB CalculationType523,  Round IV: If IV value right of the decimal <.25 then round down. If value >.75 then round up. If in between round to .5. For instance, 99998.86441 is rounded to 99999
                double integer = iv;
                double fraction = iv - floor(iv);
                if (fraction < 0.25)
                {
                    iv = floor(iv);
                } 
                else if (fraction > 0.75)
                {
                    iv = ceil(iv);
                }
                else
                {
                    iv = floor(iv) + 0.5;
                }

                //3) From BB CalculationType523,  Gross Principal = (IV*faceValue/1000)/100
                double grossPricinpal = (iv * notional * 0.001) * 0.01;

                notionalExchangeAtMaturity = grossPricinpal;
          }
        }

        return notionalExchangeAtMaturity;
    }

    /* @brief			Convert yield between frequencies
    *  @param [in]		fromYield     Input yield
    *  @param [in]		fromFreq      Frequency of input yield
    *  @param [in]		toFreq        The target yield frequency
    *  @return			yield of target frequency
    */
    double convertYield(double fromYield, const FrequencyEnum&  fromFreq, const FrequencyEnum& toFreq)
    {
        if (toFreq == fromFreq)
        {
            return fromYield;
        }

        double toYield = fromYield;

        // this logic needs to be extended for any freqency 
        if (toFreq == SEMI_ANNUAL_FREQUENCY && fromFreq == ANNUAL_FREQUENCY)
        {
            //formula: semiAnnualYield = 2 * ((1 + annualYield)^(1/2) - 1)
            toYield = 2.0 * (sqrt(1.0 + fromYield) - 1.0);
        } 
        else if (toFreq == ANNUAL_FREQUENCY  && fromFreq == SEMI_ANNUAL_FREQUENCY)
        {
            //formula: (semiAnnualYield/2 +1)^2 - 1 = annualYield
            toYield = std::pow((1 + fromYield * 0.5 ), 2) - 1.0;

        }
        else 
        {
            { std::ostringstream aqMsg80;
aqMsg80 << "Convertion only supports between Semi-Annual and Annual"; AQ_THROW( aqMsg80.str() ); }
        }
    
        return toYield;

    }


	double roundBondAccruedInterestFactor(const double& origAccruedInterestFactor, const BondCalculationTypeEnum& calculationType, const AQLDate& settlementDate)
	{

		double accruedInterestFactor = origAccruedInterestFactor;

		if (isJapaneseGovenmentBond(calculationType))
		{
			//truncate (not rounded) to 7 decimals, to be consistent with Bloomberg 
			accruedInterestFactor = truncateToDecimal(accruedInterestFactor, 7);
		}
		else if (calculationType == TYPE89_FRENCH_COMPOUND_METHOD)
		{
			AQLDate frenchFixedBondMarketConventionChangeDate = AQLDate("20050418");
			if (settlementDate < frenchFixedBondMarketConventionChangeDate)
			{
				//rounded to 3 decimals
				accruedInterestFactor = roundToDecimal(accruedInterestFactor, 3);
			}
			else
			{
				//rounded to 7 decimals
				accruedInterestFactor = roundToDecimal(accruedInterestFactor, 7);
			}

		}
		else if (calculationType == TYPE129_ISMA_CONVENTION)
		{
			//rounded to 7 decimals
			accruedInterestFactor = roundToDecimal(accruedInterestFactor, 7);

		}
		else if (calculationType == TYPE523_ITALY_TRSY_BONDS)
		{
			//rounded to 5 decimals
			accruedInterestFactor = roundToDecimal(accruedInterestFactor, 5);
		}
		else if (calculationType == TYPE23_AUSTRALIAN_GOVERNMENT_BONDS)
		{
			//rounded to 3 decimals
			accruedInterestFactor = roundToDecimal(accruedInterestFactor, 3);
		}

		return accruedInterestFactor;

	}


    /* @brief			Function to format a raw bond price according to its quote convention
    *  @param [in]		rawBondPrice      Frequency of input yield
    *  @param [in]		quoteConvention   The target yield frequency
    *  @return			Formatted Bond Price
    */
    double formatBondPrice( const double& rawBondPrice, const BondQuoteConventionEnum& quoteConvention )
    {
        double result = rawBondPrice;

        switch ( quoteConvention )
        {
            case NO_BOND_QUOTE_CONVENTION :
                // Do nothing - return raw bond price
                return result;

            case QUOTE_IN_32NDS :
                result = ( std::floor( ( rawBondPrice * 32 ) + 0.03125 ) / 32.0 );
                return result;

            case QUOTE_IN_64THS :
                result = ( std::floor( ( rawBondPrice * 64 ) + 0.015625 ) / 64.0 );
                return result;

		    case QUOTE_TO_1_DECIMAL_PLACE  :
                result = std::round( rawBondPrice * 10.0 ) / 10.0;
                return result;

            case QUOTE_TO_2_DECIMAL_PLACES :
                result = std::round( rawBondPrice * 100.0 ) / 100.0;
                return result;

            case QUOTE_TO_3_DECIMAL_PLACES :
                result = std::round( rawBondPrice * 1000.0 ) / 1000.0;
                return result;

            case QUOTE_TO_4_DECIMAL_PLACES :
                result = std::round( rawBondPrice * 10000.0 ) / 10000.0;
                return result;

            case QUOTE_TO_5_DECIMAL_PLACES :
                result = std::round( rawBondPrice * 100000.0 ) / 100000.0;
                return result;
            
            default:
                // Do nothing - return raw bond price
                return result;
        }
        
        // Should never reach here
        return result;

    }

	void populateBondYieldParameters(BondYieldParameters& bondYieldParameters,
									const FrequencyEnum& couponFrequency,
									const FrequencyEnum& yieldFrequency,
									const BondCalculationTypeEnum& calculationType,
									const DayCountEnum&	accrualDayCount,
									const std::string& accrualCalendar,
									const std::string& exDividendTenor)
	{
		bondYieldParameters.couponFrequency_ = couponFrequency;
		bondYieldParameters.yieldFrequency_ = yieldFrequency;
		bondYieldParameters.calculationType_ = calculationType;
		bondYieldParameters.dayCount_ = accrualDayCount;
		bondYieldParameters.calendar_ = accrualCalendar;
	}


	bool excludeCouponInterest(const AQLDate& settlementDate, const AQLDate& exDividendDate)
	{
		if (exDividendDate == AQLDate())
		{
			return false;
		}
		else
		{
			return (settlementDate >= exDividendDate);
		}
	}

	/* @brief			Calculate Fwd Bond's sum of coupon future value
	*  @param [in]		reinvestedCoupons		Coupons between settleDate and forwardSettleDate
	*  @return			Sum of paid coupon future values
	*/
	double calculateReinvestedCouponsFwdValue(const std::vector< BondFwdReinvestedCoupon >& reinvestedCoupons)
	{
		// Received coupons' future value at the forward settle date
		// Formula:  sumOf (coupon_i * (1+ repoRate * couponYearFraction_i)), where couponYearFraction_i is the yearFractions between couponDate_i to forwardSettleDate

		double sumOfReceivedCouponsFwdValue = 0.0;

		const size_t expectedSize = reinvestedCoupons.size();

		for (size_t i = 0; i < expectedSize; ++i)
		{
			//Internal check
			AQ_REQUIRE(!boost::math::isnan(reinvestedCoupons[i].couponFwdValue), "Bond's couponFwdValue cannot be NaN.");

			sumOfReceivedCouponsFwdValue += reinvestedCoupons[i].couponFwdValue;
		}

		return sumOfReceivedCouponsFwdValue;
	}


	/* @brief			Calculate Bond's forward dirtyPrice at forward settle date, from repo rate
	*  @param [in]		dirtyPrice			Bond dirty price at settle date
	*  @param [in]		settleDate			Bond current settle date
	*  @param [in]		forwardSettleDate   Bond forward settle date
	*  @param [in]		repoRate			Bond repo rate
	*  @param [in]		repoDayCount		Day count for repo year fraction
	*  @param [in]		reinvestedCouponFwdValue	Sum of paid voupons between settleDate and forwardSettleDate
	*  @return			Bond forward dirty price
	*/
	double calculateForwardDirtyPrice(const double& dirtyPrice, const AQLDate& settleDate, const AQLDate& forwardSettleDate, const double& repoRate, const DayCountEnum& repoDayCount, const double& reinvestedCouponFwdValue)
	{

		AQ_REQUIRE(settleDate < forwardSettleDate, "Bond's settle date must be earlier than forward settle date.");

		//Formula: bondForwardDirtyPrice = currentDirtyPrice * (1+ repoRate * repoYearFraction) - sumOf (coupon_i * (1+ repoRate * couponYearFraction_i)), where couponYearFraction_i is the yearFractions between couponDate_i to forwardSettleDate

		// 1) fwdDirtyPrice without received coupons:
		const double repoYearFraction = getYearFraction(settleDate, forwardSettleDate, repoDayCount);
		double fwdDirtyPrice = dirtyPrice * (1.0 + repoRate * repoYearFraction);

		// 3) final fwdDirtyPrice = 1) - 2)
		fwdDirtyPrice -= reinvestedCouponFwdValue;

		return fwdDirtyPrice;
	}

	/* @brief			Calculate Bond's implied repo rate from bond's forward dirty price at settle date
	*  @param [in]		dirtyPrice			Bond dirty price at settle date
	*  @param [in]		forwardDirtyPrice	Bond's forward dirtyPrice at forward settle date
	*  @param [in]		settleDate			Bond current settle date
	*  @param [in]		forwardSettleDate   Bond forward settle date
	*  @param [in]		repoDayCount		Day count for repo year fraction
	*  @param [in]		reinvestedCoupons		Coupons between settleDate and forwardSettleDate
	*  @return			Bond implied repo rate
	*/
	double calculateRepoRate(const double& dirtyPrice, const double& forwardDirtyPrice, const AQLDate& settleDate, const AQLDate& forwardSettleDate, const DayCountEnum& repoDayCount, const std::vector< BondFwdReinvestedCoupon >& reinvestedCoupons)
	{
		AQ_REQUIRE(settleDate < forwardSettleDate, "Bond's settle date must be earlier than forward settle date.");

		//Formula: bondForwardDirtyPrice = currentDirtyPrice * (1+ repoRate * repoYearFraction) - sumOf (coupon_i * (1+ repoRate * couponYearFraction_i)), where couponYearFraction_i is the yearFractions between couponDate_i to forwardSettleDate
		// => repoRate = [bondForwardDirtyPrice - currentDirtyPrice + sumOf(coupon_i)] / [currentDirtyPrice * repoYearFraction - sumOf(coupon_i * couponYearFraction_i)]

		AQ_REQUIRE(!AQ_IS_EQUAL_ZERO(dirtyPrice), "Bond price cannot be zero.");

		const double repoYearFraction = getYearFraction(settleDate, forwardSettleDate, repoDayCount);

		double sumOfReceivedCouponsValue = 0.0;
		double sumOfReceivedCouponMultipliedYearFraction = 0.0;

		const size_t expectedSize = reinvestedCoupons.size();

		for (size_t i = 0; i < expectedSize; ++i)
		{
			//sumOf(coupon_i)
			sumOfReceivedCouponsValue += reinvestedCoupons[i].couponValue;
			//sumOf(coupon_i * couponYearFraction_i)
			sumOfReceivedCouponMultipliedYearFraction += reinvestedCoupons[i].couponValue * reinvestedCoupons[i].couponFwdYearFraction;
		}

		// repoRate = [bondForwardDirtyPrice - currentDirtyPrice + sumOf(coupon_i)] / [currentDirtyPrice * repoYearFraction - sumOf(coupon_i * couponYearFraction_i)]
		const double repoRate = (forwardDirtyPrice - dirtyPrice + sumOfReceivedCouponsValue) / (dirtyPrice * repoYearFraction - sumOfReceivedCouponMultipliedYearFraction);

		return repoRate;
	}

	/* @brief			Get Bond's implied forward price from quoted future price
	*  @param [in]		futurePrice			Bond future dirty/price at future settle date
	*  @param [in]		conversionFactor	Bond's conversion factor
	*  @return			Bond forward dirty/clean price
	*/
	double toImpliedForwardCleanPrice(const double& futurePrice, const double& conversionFactor)
	{
		//Forluma: forwardPrice = fairFuturePrice * conversionFactor

		// *** Note that FUTURE is a theoretical bond, so the forward price from the future price is always CLEAN price
		const double fwdCleanPrice = futurePrice * conversionFactor;

		return fwdCleanPrice;
	}

	/* @brief			Get the first full coupon days of the long start stub
	*  @param [in]		firstActivePaymentDate			Bond future dirty/price at future settle date
	*  @param [in]		firstPriorVirtualPaymentDate    The date prior to the first active coupon date by one full coupon period
	*  @param [in]		secondPriorVirtualPaymentDate   The date prior to the first active coupon date by two full coupon period
	*  @param [in]		bondCalculationType             Bond calculation type
	*  @param [in]		dayCount						Bond Daycount
	*  @param [in]		frequency						Bond frequency
	*  @return			First full coupon days of the long start stub
	*/
	double getLongStartStubFirstFullCouponDays(const AQLDate& firstActivePaymentDate, const AQLDate& firstPriorVirtualPaymentDate, const AQLDate& secondPriorVirtualPaymentDate, const BondCalculationTypeEnum& bondCalculationType, const DayCountEnum& dayCount, const FrequencyEnum& frequency)
	{
		double fullCouponPeriodDays = 0.0;

		if (bondCalculationType == TYPE1029_SPAIN_GOVERNMENT_BONDS)
		{
			//From BB CalculationType doc, the year basis equals to 366 if Feb 29 is included in the period starting from 1 year prior to the coupon payment to the coupon payment date(inclusive), otherwise year basis is 365.
			fullCouponPeriodDays = (numberOfLongFeb(firstPriorVirtualPaymentDate, firstActivePaymentDate, true /** include coupon payment date*/) > 0) ? 366.0 : 365.0;
		}
		else
		{
			fullCouponPeriodDays = getBondFullCouponPeriodDays(secondPriorVirtualPaymentDate, firstPriorVirtualPaymentDate, dayCount, frequency);
		}

		return fullCouponPeriodDays;
	}

}