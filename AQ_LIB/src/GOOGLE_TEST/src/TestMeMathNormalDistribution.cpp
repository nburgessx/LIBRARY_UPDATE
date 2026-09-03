// TestMeMathNormalDistribution.cpp
#include "NormalDistribution.h"

// Include: Google Test Library
#include <gTest/gTest.h>


double testPrecision = 1e-4;


TEST( TestMeMathNormalDistribution, UNIT_StandardNormalDistribution_CDF )
{
    EXPECT_NEAR( etrading::standardNormalDistribution( 1.0 ), 0.84134, testPrecision );
    EXPECT_NEAR( etrading::standardNormalDistribution( 2.0 ), 0.97725, testPrecision );
    EXPECT_NEAR( etrading::standardNormalDistribution( 3.9 ), 0.99995, testPrecision );

    EXPECT_NEAR( etrading::standardNormalDistribution( -1.0 ), 0.15866, testPrecision );
    EXPECT_NEAR( etrading::standardNormalDistribution( -2.0 ), 0.02275, testPrecision );
    EXPECT_NEAR( etrading::standardNormalDistribution( -3.9 ), 0.00005, testPrecision );

    EXPECT_NEAR( etrading::standardNormalDistribution( 0.000000 ), 0.5, testPrecision );
    EXPECT_NEAR( etrading::standardNormalDistribution( -1000000.0 ), 0.0, testPrecision );
    EXPECT_NEAR( etrading::standardNormalDistribution( +1000000.0 ), 1.0, testPrecision );
}


TEST( TestMeMathNormalDistribution, UNIT_StandardNormalDistribution_PDF )
{
    EXPECT_NEAR( etrading::standardNormalDistributionPDF( -10.0 ), 0.0, testPrecision );
    EXPECT_NEAR( etrading::standardNormalDistributionPDF(  10.0 ), 0.0, testPrecision );
}


TEST( TestMeMathNormalDistribution, UNIT_StandardNormalDistributionInverse_NormalVariate )
{
    EXPECT_NEAR( etrading::standardNormalDistributionInverse( 0.00135 ), -3.0, testPrecision );
    EXPECT_NEAR( etrading::standardNormalDistributionInverse( 0.02275 ), -2.0, testPrecision );
    EXPECT_NEAR( etrading::standardNormalDistributionInverse( 0.15866 ), -1.0, testPrecision );
    EXPECT_NEAR( etrading::standardNormalDistributionInverse( 0.50000 ), 0.0, testPrecision );
    EXPECT_NEAR( etrading::standardNormalDistributionInverse( 0.84134 ), 1.0, testPrecision );
    EXPECT_NEAR( etrading::standardNormalDistributionInverse( 0.97725 ), 2.0, testPrecision );
    EXPECT_NEAR( etrading::standardNormalDistributionInverse( 0.99865 ), 3.0, testPrecision );
}


TEST( TestMeMathNormalDistribution, UNIT_NormalDistribution_CDF )
{
    EXPECT_NEAR( etrading::normalDistribution( 1.0, 0.0, 1.0 ), 0.84134, testPrecision );
    EXPECT_NEAR( etrading::normalDistribution( 2.0, 0.0, 1.0 ), 0.97725, testPrecision );
    EXPECT_NEAR( etrading::normalDistribution( 3.9, 0.0, 1.0 ), 0.99995, testPrecision );
                           
    EXPECT_NEAR( etrading::normalDistribution( -1.0, 0.0, 1.0 ), 0.15866, testPrecision );
    EXPECT_NEAR( etrading::normalDistribution( -2.0, 0.0, 1.0 ), 0.02275, testPrecision );
    EXPECT_NEAR( etrading::normalDistribution( -3.9, 0.0, 1.0 ), 0.00005, testPrecision );
                           
    EXPECT_NEAR( etrading::normalDistribution( 0.000000, 0.0, 1.0 ), 0.5, testPrecision );
    EXPECT_NEAR( etrading::normalDistribution( -1000000, 0.0, 1.0 ), 0.0, testPrecision );
    EXPECT_NEAR( etrading::normalDistribution( +1000000, 0.0, 1.0 ), 1.0, testPrecision );
}


TEST( TestMeMathNormalDistribution, UNIT_NormalDistribution_PDF )
{
    EXPECT_NEAR( etrading::normalDistributionPDF( -10.0, 0.0, 1.0 ), 0.0, testPrecision );
    EXPECT_NEAR( etrading::normalDistributionPDF(  10.0, 0.0, 1.0 ), 0.0, testPrecision );
}


TEST( TestMeMathNormalDistribution, UNIT_NormalDistributionInverse_NormalVariate )
{
    EXPECT_NEAR( etrading::normalDistributionInverse( 0.00135, 0.0, 1.0 ), -3.0, testPrecision );
    EXPECT_NEAR( etrading::normalDistributionInverse( 0.02275, 0.0, 1.0 ), -2.0, testPrecision );
    EXPECT_NEAR( etrading::normalDistributionInverse( 0.15866, 0.0, 1.0 ), -1.0, testPrecision );
    EXPECT_NEAR( etrading::normalDistributionInverse( 0.50000, 0.0, 1.0 ), 0.0, testPrecision );
    EXPECT_NEAR( etrading::normalDistributionInverse( 0.84134, 0.0, 1.0 ), 1.0, testPrecision );
    EXPECT_NEAR( etrading::normalDistributionInverse( 0.97725, 0.0, 1.0 ), 2.0, testPrecision );
    EXPECT_NEAR( etrading::normalDistributionInverse( 0.99865, 0.0, 1.0 ), 3.0, testPrecision );
}