/*
 * @brief			Test File for BondUtilities.cpp
 * @Created:		22nd February 2017
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include <gTest/gTest.h>    // Include: Google Test Library

#include "BondUtilities.h"
#include "BondEnumerations.h"

namespace google_test
{

    TEST( TestBondUtilities, UNIT_GetActiveCashflowDate )
    {
        LADate settlementDateFront("20170221");
        LADate settlementDateBack("20190101");
        LADate settlementDateFarInFuture("20500101");
        LADate settlementDateFarInPast("20010101");

        // Populate Bond Payment Dates, which are used for Cashflow Indexation
        std::vector< LADate >   bondPaymentlDatesForIndexation;
        
        bondPaymentlDatesForIndexation.push_back( LADate("20170515") );
        bondPaymentlDatesForIndexation.push_back( LADate("20171115") );
        bondPaymentlDatesForIndexation.push_back( LADate("20180515") );
        bondPaymentlDatesForIndexation.push_back( LADate("20181115") );
        bondPaymentlDatesForIndexation.push_back( LADate("20190515") );

        // Populate Bond Accrual Dates
        std::vector< LADate >   bondAccrualStartDates;
        bondAccrualStartDates.push_back( LADate("20161115") );
        bondAccrualStartDates.push_back( LADate("20170515") );
        bondAccrualStartDates.push_back( LADate("20171115") );
        bondAccrualStartDates.push_back( LADate("20180515") );
        bondAccrualStartDates.push_back( LADate("20181115") );

        // Test 1. Test front cashflow date
        LADate activeAccrualStartDate1 = etrading::getActiveCashflowDateAndIndex( settlementDateFront, bondAccrualStartDates, bondPaymentlDatesForIndexation ).first;
        EXPECT_EQ( LADate("20161115"), activeAccrualStartDate1 );

        // Test 2. Test for final cashflow date
        LADate activeAccrualStartDate2 = etrading::getActiveCashflowDateAndIndex( settlementDateBack, bondAccrualStartDates, bondPaymentlDatesForIndexation ).first;
        EXPECT_EQ( LADate("20181115"), activeAccrualStartDate2 );

        // Test 3. When all dates are in the past should return the final cashflow date
        LADate activeAccrualStartDate3 = etrading::getActiveCashflowDateAndIndex( settlementDateFarInFuture, bondAccrualStartDates, bondPaymentlDatesForIndexation ).first;
        EXPECT_EQ( LADate("20181115"), activeAccrualStartDate3 );

        // Test 4. When all coupon payment dates are in the future should return the front cashflow date
        LADate activeAccrualStartDate4 = etrading::getActiveCashflowDateAndIndex( settlementDateFarInPast, bondAccrualStartDates, bondPaymentlDatesForIndexation ).first;
        EXPECT_EQ( LADate("20161115"), activeAccrualStartDate4 );
    }



    TEST( TestBondUtilities, UNIT_GetBondCashflowIndexFromSettlementDate )
    {
        LADate settlementDateFront("20170221");
        LADate settlementDateBack("20190101");
        LADate settlementDateFarInFuture("20500101");
        LADate settlementDateFarInPast("20010101");

        // Populate Bond Payment Dates, which are used for Cashflow Indexation
        std::vector< LADate >   bondPaymentlDatesForIndexation;
        
        bondPaymentlDatesForIndexation.push_back( LADate("20170515") );
        bondPaymentlDatesForIndexation.push_back( LADate("20171115") );
        bondPaymentlDatesForIndexation.push_back( LADate("20180515") );
        bondPaymentlDatesForIndexation.push_back( LADate("20181115") );
        bondPaymentlDatesForIndexation.push_back( LADate("20190515") );

        // Test 1. Test front cashflow when settlement date front
        auto activeCashflowIndex1          = etrading::getBondActiveCashflowIndex( settlementDateFront, bondPaymentlDatesForIndexation );
        unsigned int index1 = etrading::getBondRelativeCashflowIndex( activeCashflowIndex1, bondPaymentlDatesForIndexation[0], bondPaymentlDatesForIndexation );
        EXPECT_EQ( 0, index1 );

        // Test 2. Test for final cashflow when settlement date back
        auto activeCashflowIndex2          = etrading::getBondActiveCashflowIndex( settlementDateBack, bondPaymentlDatesForIndexation );
        unsigned int index2 = etrading::getBondRelativeCashflowIndex( activeCashflowIndex2, bondPaymentlDatesForIndexation[4], bondPaymentlDatesForIndexation );
        EXPECT_EQ( 0, index2 );

        // Test 3. Test for final cashflow when settlement date far in the future
        auto activeCashflowIndex3          = etrading::getBondActiveCashflowIndex( settlementDateFarInFuture, bondPaymentlDatesForIndexation );
        unsigned int index3 = etrading::getBondRelativeCashflowIndex( activeCashflowIndex3, bondPaymentlDatesForIndexation[4], bondPaymentlDatesForIndexation );
        EXPECT_EQ( 0, index3 );

        // Test 4. Test for front cashflow when settlement date far in the past
        auto activeCashflowIndex4          = etrading::getBondActiveCashflowIndex( settlementDateFarInPast, bondPaymentlDatesForIndexation );
        unsigned int index4 = etrading::getBondRelativeCashflowIndex( activeCashflowIndex4, bondPaymentlDatesForIndexation[0], bondPaymentlDatesForIndexation );
        EXPECT_EQ( 0, index4 );

        // Test 5. Test for final cashflow when settlement date far in the future
        auto activeCashflowIndex5          = etrading::getBondActiveCashflowIndex( settlementDateFarInFuture, bondPaymentlDatesForIndexation );
        unsigned int index5 = etrading::getBondRelativeCashflowIndex( activeCashflowIndex5, bondPaymentlDatesForIndexation[4], bondPaymentlDatesForIndexation );
        EXPECT_EQ( 0, index5 );

        // Test 6. Test for first cashflow when settlement date far in the past
        auto activeCashflowIndex6          = etrading::getBondActiveCashflowIndex( settlementDateFarInPast, bondPaymentlDatesForIndexation );
        unsigned int index6 = etrading::getBondRelativeCashflowIndex( activeCashflowIndex6, bondPaymentlDatesForIndexation[0], bondPaymentlDatesForIndexation );
        EXPECT_EQ( 0, index6 );

        // Test 7. Test for second cashflow when settlement date far in the past
        auto activeCashflowIndex7          = etrading::getBondActiveCashflowIndex( settlementDateFarInPast, bondPaymentlDatesForIndexation );
        unsigned int index7 = etrading::getBondRelativeCashflowIndex( activeCashflowIndex7, bondPaymentlDatesForIndexation[1], bondPaymentlDatesForIndexation );
        EXPECT_EQ( 1, index7 );

        // Test 8. Test for third cashflow when settlement date far in the past
        auto activeCashflowIndex8          = etrading::getBondActiveCashflowIndex( settlementDateFarInPast, bondPaymentlDatesForIndexation );
        unsigned int index8 = etrading::getBondRelativeCashflowIndex( activeCashflowIndex8, bondPaymentlDatesForIndexation[2], bondPaymentlDatesForIndexation );
        EXPECT_EQ( 2, index8 );

        // Test 9. Test for fourth cashflow when settlement date far in the past
        auto activeCashflowIndex9          = etrading::getBondActiveCashflowIndex( settlementDateFarInPast, bondPaymentlDatesForIndexation );
        unsigned int index9 = etrading::getBondRelativeCashflowIndex( activeCashflowIndex9, bondPaymentlDatesForIndexation[3], bondPaymentlDatesForIndexation );
        EXPECT_EQ( 3, index9 );

        // Test 10. Test for fifth cashflow when settlement date far in the past
        auto activeCashflowIndex10          = etrading::getBondActiveCashflowIndex( settlementDateFarInPast, bondPaymentlDatesForIndexation );
        unsigned int index10 = etrading::getBondRelativeCashflowIndex( activeCashflowIndex10, bondPaymentlDatesForIndexation[4], bondPaymentlDatesForIndexation );
        EXPECT_EQ( 4, index10 );

        // Test 11. Test for first cashflow when settlement date far in the future
        auto activeCashflowIndex11          = etrading::getBondActiveCashflowIndex( settlementDateFarInFuture, bondPaymentlDatesForIndexation );
        unsigned int index11 = etrading::getBondRelativeCashflowIndex( activeCashflowIndex11, bondPaymentlDatesForIndexation[0], bondPaymentlDatesForIndexation );
        EXPECT_EQ( 0, index11 );

        // Test 12. Test for second cashflow when settlement date far in the future
        auto activeCashflowIndex12          = etrading::getBondActiveCashflowIndex( settlementDateFarInFuture, bondPaymentlDatesForIndexation );
        unsigned int index12 = etrading::getBondRelativeCashflowIndex( activeCashflowIndex12, bondPaymentlDatesForIndexation[1], bondPaymentlDatesForIndexation );
        EXPECT_EQ( 0, index12 );

        // Test 13. Test for third cashflow when settlement date far in the future
        auto activeCashflowIndex13          = etrading::getBondActiveCashflowIndex( settlementDateFarInFuture, bondPaymentlDatesForIndexation );
        unsigned int index13 = etrading::getBondRelativeCashflowIndex( activeCashflowIndex13, bondPaymentlDatesForIndexation[2], bondPaymentlDatesForIndexation );
        EXPECT_EQ( 0, index13 );

        // Test 14. Test for fourth cashflow when settlement date far in the future
        auto activeCashflowIndex14          = etrading::getBondActiveCashflowIndex( settlementDateFarInFuture, bondPaymentlDatesForIndexation );
        unsigned int index14 = etrading::getBondRelativeCashflowIndex( activeCashflowIndex14, bondPaymentlDatesForIndexation[3], bondPaymentlDatesForIndexation );
        EXPECT_EQ( 0, index14 );

        // Test 15. Test for fifth cashflow when settlement date far in the future
        auto activeCashflowIndex15          = etrading::getBondActiveCashflowIndex( settlementDateFarInFuture, bondPaymentlDatesForIndexation );
        unsigned int index15 = etrading::getBondRelativeCashflowIndex( activeCashflowIndex15, bondPaymentlDatesForIndexation[4], bondPaymentlDatesForIndexation );
        EXPECT_EQ( 0, index15 );
    }



    TEST( TestBondUtilities, UNIT_GetBondActiveCashflowIndex )
    {
        LADate settlementDateFront("20170221");
        LADate settlementDateBack("20190101");
        LADate settlementDateFarInFuture("20500101");
        LADate settlementDateFarInPast("20010101");

        // Populate Bond Payment Dates, which are used for Cashflow Indexation
        std::vector< LADate >   bondPaymentlDatesForIndexation;
        
        bondPaymentlDatesForIndexation.push_back( LADate("20170515") );
        bondPaymentlDatesForIndexation.push_back( LADate("20171115") );
        bondPaymentlDatesForIndexation.push_back( LADate("20180515") );
        bondPaymentlDatesForIndexation.push_back( LADate("20181115") );
        bondPaymentlDatesForIndexation.push_back( LADate("20190515") );

        // Test 1. Test front cashflow with Index number = 0
        unsigned int index1 = etrading::getBondActiveCashflowIndex( settlementDateFront, bondPaymentlDatesForIndexation );
        EXPECT_EQ( 0, index1 );

        // Test 2. Test for final cashflow with Index = 4, n-1
        unsigned int index2 = etrading::getBondActiveCashflowIndex( settlementDateBack, bondPaymentlDatesForIndexation );
        EXPECT_EQ( 4, index2 );

        // Test 3. When all dates are in the past should return Index number = 4
        unsigned int index3 = etrading::getBondActiveCashflowIndex( settlementDateFarInFuture, bondPaymentlDatesForIndexation );
        EXPECT_EQ( 4, index3 );

        // Test 4. When all coupon payment dates are in the future should return index number = 0
        unsigned int index4 = etrading::getBondActiveCashflowIndex( settlementDateFarInPast, bondPaymentlDatesForIndexation );
        EXPECT_EQ( 0, index4 );
    }

    TEST( TestBondUtilities, UNIT_ConvertBondFrequencyToYearFraction )
    {
        // Test Annual Frequency
        double frequency1 = etrading::convertBondFrequencyToYearFraction( etrading::ANNUAL_FREQUENCY );
        EXPECT_EQ( 1, frequency1 );

        // Test Semi-Annual Frequency
        double frequency2 = etrading::convertBondFrequencyToYearFraction( etrading::SEMI_ANNUAL_FREQUENCY );
        EXPECT_EQ( 0.5, frequency2 );

        // Test Quarterly Frequency
        double frequency3 = etrading::convertBondFrequencyToYearFraction( etrading::QUARTERLY_FREQUENCY );
        EXPECT_EQ( 0.25, frequency3 );

        // Test Monthly Frequency
        double frequency4 = etrading::convertBondFrequencyToYearFraction( etrading::MONTHLY_FREQUENCY );
        EXPECT_EQ( 1.0/12.0, frequency4 );
    }
    
};