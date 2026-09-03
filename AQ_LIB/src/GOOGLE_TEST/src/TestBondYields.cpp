/*
 * @brief			Test File for BondYields.cpp
 * @Created:		22nd February 2017
 * @Author:			Nicholas Burgess
 * @Department:		MHI Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

// Include: Google Test Library
#include <gTest/gTest.h>
#include "InitializeMLibGoogleTest.h"

#include "BondYields.h"
#include "BondFactory.h"
#include "ObjectUtilities.h"
#include "CreateFixedBond.h"
#include "BondEnumerations.h"
#include "CoreEnumerations.h"

namespace
{
    // Test Bond Input File
    extern const std::string bondInputsFile1 = "ETrading/LWObjects/LWOBonds/BondUnitTests/tryMeLWOBondCreate_inputs.csv";
}

namespace google_test
{
    class TestBondFactory : public virtual testing::Test, public virtual google_test::InitializeMLibGoogleTest
    {
        public:

        static etrading::BondPtr createBond( const std::string& inputFile )
        {
            // Build the Bond Object
            CreateFixedBond fixedBond;
            etrading::BondPtr myBondPtr = fixedBond.createFixedBondFromInputFile( inputFile );
            return myBondPtr;
        }
    };

    // Inherit the Create Bond Object Framework from TestBondFactory above
    class TestBondYields : public TestBondFactory {};


    TEST_F( TestBondFactory, UNIT_CreateBond )
    {
        etrading::BondPtr myBondPtr = TestBondFactory::createBond( bondInputsFile1 );
        EXPECT_EQ( std::string("USTREASURY@1") , myBondPtr->getBondObjectName() );
    }


    TEST_F( TestBondYields, UNIT_CalculateSingleDiscountFactorFromBondYield )
    {
        // Create the Bond
        etrading::BondPtr myBondPtr = TestBondFactory::createBond( bondInputsFile1 );

        // Register the Bond in the LWO Cache
        etrading::registerToCache< etrading::Bond >( myBondPtr );

        const std::string bondObjectName        = myBondPtr->getBondObjectName(); 
        const LADate settlementDate             = LADate("20170221");
        const double yield                      = 0.02433; // 0.24330%

        const LADate activeAccrualStartDate     = LADate("20161115");
        const LADate activeAccrualEndDate       = LADate("20170515");

        std::vector< LADate > bondPaymentDatesForIndexation;        

        bondPaymentDatesForIndexation.push_back( LADate("20161115") );
        bondPaymentDatesForIndexation.push_back( LADate("20170515") );
        bondPaymentDatesForIndexation.push_back( LADate("20171115") );
        bondPaymentDatesForIndexation.push_back( LADate("20180515") );
        bondPaymentDatesForIndexation.push_back( LADate("20181115") );
        bondPaymentDatesForIndexation.push_back( LADate("20190515") );
        bondPaymentDatesForIndexation.push_back( LADate("20191115") );
        bondPaymentDatesForIndexation.push_back( LADate("20200515") );
        bondPaymentDatesForIndexation.push_back( LADate("20201116") );
        bondPaymentDatesForIndexation.push_back( LADate("20210517") );
        bondPaymentDatesForIndexation.push_back( LADate("20211115") );
        bondPaymentDatesForIndexation.push_back( LADate("20220516") );
        bondPaymentDatesForIndexation.push_back( LADate("20221115") );
        bondPaymentDatesForIndexation.push_back( LADate("20230515") );
        bondPaymentDatesForIndexation.push_back( LADate("20231115") );
        bondPaymentDatesForIndexation.push_back( LADate("20240515") );
        bondPaymentDatesForIndexation.push_back( LADate("20241115") );
        bondPaymentDatesForIndexation.push_back( LADate("20250515") );
        bondPaymentDatesForIndexation.push_back( LADate("20251117") );
        bondPaymentDatesForIndexation.push_back( LADate("20260515") );
        bondPaymentDatesForIndexation.push_back( LADate("20261116") );


        // Add Bond Input Parameters to the BondYield Struct, since cannot pass the abstract bond class by reference
        etrading::BondYieldParameters bondYieldParameters = myBondPtr->getBondYieldParameters();
        auto activeCashflowIndex          = etrading::getBondActiveCashflowIndex( settlementDate, bondPaymentDatesForIndexation );

		etrading::BondActiveCouponDates activeCouponDates;
		activeCouponDates.priorFirstActiveCouponDate_ = activeAccrualStartDate;
		activeCouponDates.firstActiveCouponDate_ = activeAccrualEndDate;
		activeCouponDates.adjustedFirstActiveCouponDate_ = activeAccrualEndDate;

        const double& accruedAdjustmentFactor = etrading::calculateBondActiveCouponFutureAccrualRatio( settlementDate,
																										activeCouponDates,
																										myBondPtr->getSchedule()->getAccrualDaycount(), 
																										myBondPtr->getSchedule()->getAccrualFrequency(),
																										etrading::ISMA_YIELD);
        
        // Test 1. Check First Coupon Discount Factor, Should be zero since it is in the past
        const LADate couponPaymentDate1     = LADate("20161115");
        const double bondDiscountFactor1    = etrading::calculateSingleDiscountFactorFromBondYield( bondYieldParameters,
                                                                                                    settlementDate,
                                                                                                    activeCashflowIndex,
                                                                                                    couponPaymentDate1,
                                                                                                    bondPaymentDatesForIndexation,
                                                                                                    yield,
                                                                                                    accruedAdjustmentFactor,
                                                                                                    etrading::ISMA_YIELD);
        const double tolerance1             = 0.000000001;
        EXPECT_NEAR( 0.0, bondDiscountFactor1, tolerance1 );


        // Test 2. Check Second / Current Coupon Discount Factor
        const LADate couponPaymentDate2     = LADate("20170515");
        const double bondDiscountFactor2    = etrading::calculateSingleDiscountFactorFromBondYield( bondYieldParameters,
                                                                                                    settlementDate,
                                                                                                    activeCashflowIndex,
                                                                                                    couponPaymentDate2,
                                                                                                    bondPaymentDatesForIndexation,
                                                                                                    yield,
                                                                                                    accruedAdjustmentFactor,
                                                                                                    etrading::ISMA_YIELD);
        const double tolerance2             = 0.000000001;
        EXPECT_NEAR( 0.99447057651606,  bondDiscountFactor2, tolerance2 );


        // Test 3. Check Third Coupon Discount Factor
        const LADate couponPaymentDate3     = LADate("20171115");
        const double bondDiscountFactor3    = etrading::calculateSingleDiscountFactorFromBondYield( bondYieldParameters,
                                                                                                    settlementDate,
                                                                                                    activeCashflowIndex,
                                                                                                    couponPaymentDate3,
                                                                                                    bondPaymentDatesForIndexation,
                                                                                                    yield,
                                                                                                    accruedAdjustmentFactor,
                                                                                                    etrading::ISMA_YIELD);
        const double tolerance3             = 0.000000001;
        EXPECT_NEAR( 0.982518242100902,  bondDiscountFactor3, tolerance3 );

    }
  
     TEST_F( TestBondYields, UNIT_CalculateDiscountFactorsFromBondYield )
    {
        // Create the Bond
        etrading::BondPtr myBondPtr = TestBondFactory::createBond( bondInputsFile1 );

        // Register the Bond in the LWO Cache
        etrading::registerToCache< etrading::Bond >( myBondPtr );

        const std::string bondObjectName        = myBondPtr->getBondObjectName(); 
        const LADate settlementDate             = LADate("20170221");
        const double yield                      = 0.02433; // 0.24330%

        const LADate activeAccrualStartDate     = LADate("20161115");
        const LADate activeAccrualEndDate       = LADate("20170515");

        std::vector< LADate > bondPaymentDatesForIndexation;        

        bondPaymentDatesForIndexation.push_back( LADate("20161115") );
        bondPaymentDatesForIndexation.push_back( LADate("20170515") );
        bondPaymentDatesForIndexation.push_back( LADate("20171115") );
        bondPaymentDatesForIndexation.push_back( LADate("20180515") );
        bondPaymentDatesForIndexation.push_back( LADate("20181115") );
        bondPaymentDatesForIndexation.push_back( LADate("20190515") );
        bondPaymentDatesForIndexation.push_back( LADate("20191115") );
        bondPaymentDatesForIndexation.push_back( LADate("20200515") );
        bondPaymentDatesForIndexation.push_back( LADate("20201116") );
        bondPaymentDatesForIndexation.push_back( LADate("20210517") );
        bondPaymentDatesForIndexation.push_back( LADate("20211115") );
        bondPaymentDatesForIndexation.push_back( LADate("20220516") );
        bondPaymentDatesForIndexation.push_back( LADate("20221115") );
        bondPaymentDatesForIndexation.push_back( LADate("20230515") );
        bondPaymentDatesForIndexation.push_back( LADate("20231115") );
        bondPaymentDatesForIndexation.push_back( LADate("20240515") );
        bondPaymentDatesForIndexation.push_back( LADate("20241115") );
        bondPaymentDatesForIndexation.push_back( LADate("20250515") );
        bondPaymentDatesForIndexation.push_back( LADate("20251117") );
        bondPaymentDatesForIndexation.push_back( LADate("20260515") );
        bondPaymentDatesForIndexation.push_back( LADate("20261116") );


        // Add Bond Input Parameters to the BondYield Struct, since cannot pass the abstract bond class by reference
        etrading::BondYieldParameters bondYieldParameters = myBondPtr->getBondYieldParameters();
        
        // Store the Expected Discount Factors
        std::vector< double > expectedDiscountFactors;
        expectedDiscountFactors.push_back( 0.0 );
        expectedDiscountFactors.push_back( 0.99447057651606 );
        expectedDiscountFactors.push_back( 0.982518242100902 );
        expectedDiscountFactors.push_back( 0.970709560299854 );
        expectedDiscountFactors.push_back( 0.959042804582113 );
        expectedDiscountFactors.push_back( 0.947516269167688 );
        expectedDiscountFactors.push_back( 0.936128268778003 );
        expectedDiscountFactors.push_back( 0.924877138389495 );
        expectedDiscountFactors.push_back( 0.91376123299017 );
        expectedDiscountFactors.push_back( 0.90277892733909 );
        expectedDiscountFactors.push_back( 0.891928615728749 );
        expectedDiscountFactors.push_back( 0.881208711750307 );
        expectedDiscountFactors.push_back( 0.870617648061637 );
        expectedDiscountFactors.push_back( 0.860153876158173 );
        expectedDiscountFactors.push_back( 0.849815866146501 );
        expectedDiscountFactors.push_back( 0.839602106520677 );
        expectedDiscountFactors.push_back( 0.829511103941232 );
        expectedDiscountFactors.push_back( 0.819541383016832 );
        expectedDiscountFactors.push_back( 0.809691486088564 );
        expectedDiscountFactors.push_back( 0.799959973016815 );
        expectedDiscountFactors.push_back( 0.790345420970706 );

        
        // Calculate the Actual Discount Factors
		etrading::BondActiveCouponDates activeCouponDates;
		activeCouponDates.priorFirstActiveCouponDate_ = activeAccrualStartDate;
		activeCouponDates.firstActiveCouponDate_ = activeAccrualEndDate;
		activeCouponDates.adjustedFirstActiveCouponDate_ = activeAccrualEndDate;
        const std::vector< double > actualDiscountFactors = etrading::calculateDiscountFactorsFromBondYield( myBondPtr->getBondYieldParameters(),
                                                                                                             settlementDate,
                                                                                                             activeCouponDates,
                                                                                                             bondPaymentDatesForIndexation,
                                                                                                             yield,
                                                                                                             myBondPtr->getSchedule()->getAccrualDaycount(), 
                                                                                                             etrading::ISMA_YIELD);

        // Compare Results
        const double tolerance = 0.000000001;
        for ( size_t i = 0; i < expectedDiscountFactors.size(); ++i )
        {
            EXPECT_NEAR( expectedDiscountFactors[i], actualDiscountFactors[i], tolerance );
        }

    }
};