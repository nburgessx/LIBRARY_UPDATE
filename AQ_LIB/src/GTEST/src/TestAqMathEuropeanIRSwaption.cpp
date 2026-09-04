// TestEuropeanIRSwaption.cpp
#include "tryAqMathEuropeanIRSwaption.h"

// Include: Google Test Library
#include <gTest/gTest.h>

const double testPrecision = 1e-8;


// Test Cases
// ------------------------------

TEST( TestAqMathEuropeanIRSwaption, UNIT_TestCashSettledAnnuity )
{
    // CashAnnuity( Notional (N), SwapRate (r), nCouponsPerYear (m), TenorInYears (t), StubType )
    double              N;
    double              r;
    unsigned int        m;
    double              t;
    std::string         noStub = "NONE";
    double              tau; // year fraction

    N = 1000000.0;
    r = 0.1;
    t = 1.0;

    // Test 1: Annual Swap
    m = 1; 
    tau = 1 / double( m );
    
    double result1      = validation::tryAqMathEuropeanIRSwaptionCashAnnuity( N, r, m, t, noStub );
    double df1          = 1.0 / ( 1 + r * tau );
    
    double annuity1     = N * tau * df1;
    EXPECT_NEAR( result1, annuity1, testPrecision );

    // Test 2: Semi-Annual Swap
    m = 2;
    tau = 1 / double( m );
    
    double result2      = validation::tryAqMathEuropeanIRSwaptionCashAnnuity( N, r, m, t, noStub );
    df1                 = 1.0 / ( 1 + r * tau );
    double df2          = df1 * df1;
    
    double annuity2     = ( N * tau * df1 ) + ( N * tau * df2 );
    EXPECT_NEAR( result2, annuity2, testPrecision );

    // Test 3: Quarterly Swap
    m = 4;
    tau = 1 / double( m );

    double result3      = validation::tryAqMathEuropeanIRSwaptionCashAnnuity( N, r, m, t, noStub );
    df1                 = 1.0 / ( 1 + r * tau );
    df2                 = df1 * df1;
    double df3          = df1 * df1 * df1;
    double df4          = df1 * df1 * df1 * df1;
    
    double annuity3     = ( N * tau * df1 ) + ( N * tau * df2 ) + ( N * tau * df3 ) + ( N * tau * df4 );
    EXPECT_NEAR( result3, annuity3, testPrecision );

    // Test 4: Monthly Swap
    m = 12;
    tau = 1 / double( m );

    double result4      = validation::tryAqMathEuropeanIRSwaptionCashAnnuity( N, r, m, t, noStub );
    df1                 = 1.0 / ( 1 + r * tau );
    df2                 = df1 * df1;
    df3                 = df1 * df1 * df1;
    df4                 = df1 * df1 * df1 * df1;
    double df5          = df1 * df1 * df1 * df1 * df1;
    double df6          = df1 * df1 * df1 * df1 * df1 * df1;
    double df7          = df1 * df1 * df1 * df1 * df1 * df1 * df1;
    double df8          = df1 * df1 * df1 * df1 * df1 * df1 * df1 * df1;
    double df9          = df1 * df1 * df1 * df1 * df1 * df1 * df1 * df1 * df1;
    double df10         = df1 * df1 * df1 * df1 * df1 * df1 * df1 * df1 * df1 * df1;
    double df11         = df1 * df1 * df1 * df1 * df1 * df1 * df1 * df1 * df1 * df1 * df1;
    double df12         = df1 * df1 * df1 * df1 * df1 * df1 * df1 * df1 * df1 * df1 * df1 * df1;
    
    double annuity4     =  ( N * tau * df1 ) + ( N * tau * df2 ) + ( N * tau * df3 ) + ( N * tau * df4 );
    annuity4            += ( N * tau * df5 ) + ( N * tau * df6 ) + ( N * tau * df7 ) + ( N * tau * df8 );
    annuity4            += ( N * tau * df9 ) + ( N * tau * df10 ) + ( N * tau * df11 ) + ( N * tau * df12 );
    EXPECT_NEAR( result4, annuity4, testPrecision );
    
}

TEST( TestAqMathEuropeanIRSwaption, UNIT_TestCashSettledAnnuity_WithShortStartStub )
{
    // CashAnnuity( Notional (N), SwapRate (r), nCouponsPerYear (m), TenorInYears (t), StubType )
    double              N;
    double              r;
    unsigned int        m;
    double              t;
    std::string         short_start = "SHORT_START";
    double              tau;        // year fraction
    double              stub_tau;   // stub year fraction

    N = 1000000.0;
    r = 0.1;
    t = 2.1;
    stub_tau = 0.1;

    // Test 1: Annual Swap
    m = 1;
    tau = 1 / double( m );

    double result1      = validation::tryAqMathEuropeanIRSwaptionCashAnnuity( N, r, m, t, short_start );
    double df_stub      = 1.0 / ( 1 + r * stub_tau );           // time 0.1
    double df1          = df_stub * 1.0 / ( 1 + r * tau );      // time 1.1
    double df2          = df1 * 1.0 / ( 1 + r * tau );          // time 2.1
    double annuity1     = ( N * stub_tau * df_stub ) + ( N * tau * df1 ) + ( N * tau * df2 );
    EXPECT_NEAR( result1, annuity1, testPrecision );

    // Test 2: Semi-Annual Swap
    m = 2;
    tau = 1 / double( m );
    stub_tau = 0.1;

    double result2      = validation::tryAqMathEuropeanIRSwaptionCashAnnuity( N, r, m, t, short_start );
    df_stub             = 1.0 / ( 1 + r * stub_tau );           // time 0.1
    df1                 = df_stub * 1.0 / ( 1 + r * tau );      // time 0.6
    df2                 = df1 * 1.0 / ( 1 + r * tau );          // time 1.1
    double df3          = df2 * 1.0 / ( 1 + r * tau );          // time 1.6
    double df4          = df3 * 1.0 / ( 1 + r * tau );          // time 2.1
    double annuity2     = ( N * stub_tau * df_stub ) + ( N * tau * df1 ) + ( N * tau * df2 ) + ( N * tau * df3 ) + ( N * tau * df4 );
    EXPECT_NEAR( result2, annuity2, testPrecision );

}

TEST( TestAqMathEuropeanIRSwaption, UNIT_TestCashSettledAnnuity_WithShortEndStub )
{
    // CashAnnuity( Notional (N), SwapRate (r), nCouponsPerYear (m), TenorInYears (t), StubType )
    double              N;
    double              r;
    unsigned int        m;
    double              t;
    std::string         short_end = "SHORT_END";
    double              tau;        // year fraction
    double              stub_tau;   // stub year fraction

    N = 1000000.0;
    r = 0.1;
    t = 2.1;
    stub_tau = 0.1;

    // Test 1: Annual Swap
    m = 1;
    tau = 1 / double( m );

    double result1      = validation::tryAqMathEuropeanIRSwaptionCashAnnuity( N, r, m, t, short_end );
    double df1          = 1.0 / ( 1 + r * tau );                // time 1.0
    double df2          = df1 * 1.0 / ( 1 + r * tau );          // time 2.0
    double df_stub      = df2 * 1.0 / ( 1 + r * stub_tau );     // time 2.1
    double annuity1     = ( N * tau * df1 ) + ( N * tau * df2 ) + ( N * stub_tau * df_stub );
    EXPECT_NEAR( result1, annuity1, testPrecision );

    // Test 2: Semi-Annual Swap
    m = 2;
    tau = 1 / double( m );
    stub_tau = 0.1;

    double result2      = validation::tryAqMathEuropeanIRSwaptionCashAnnuity( N, r, m, t, short_end );
    df1                 = 1.0 / ( 1 + r * tau );                // time 0.5
    df2                 = df1 * 1.0 / ( 1 + r * tau );          // time 1.0
    double df3          = df2 * 1.0 / ( 1 + r * tau );          // time 1.5
    double df4          = df3 * 1.0 / ( 1 + r * tau );          // time 2.0
    df_stub             = df4 * 1.0 / ( 1 + r * stub_tau );     // time 2.1
    double annuity2     = ( N * tau * df1 ) + ( N * tau * df2 ) + ( N * tau * df3 ) + ( N * tau * df4 ) + ( N * stub_tau * df_stub );
    EXPECT_NEAR( result2, annuity2, testPrecision );

}

TEST( TestAqMathEuropeanIRSwaption, UNIT_TestCashSettledAnnuity_WithLongStartStub )
{
    // CashAnnuity( Notional (N), SwapRate (r), nCouponsPerYear (m), TenorInYears (t), StubType )
    double              N;
    double              r;
    unsigned int        m;
    double              t;
    std::string         long_start = "LONG_START";
    double              tau;        // year fraction
    double              stub_tau;   // stub year fraction

    N = 1000000.0;
    r = 0.1;
    t = 2.1;
    stub_tau = 1.1;

    // Test 1: Annual Swap
    m = 1;
    tau = 1 / double( m );

    double result1      = validation::tryAqMathEuropeanIRSwaptionCashAnnuity( N, r, m, t, long_start );
    double df_stub      = 1.0 / ( 1 + r * stub_tau );               // time 1.1
    double df1          = df_stub * 1.0 / ( 1 + r * tau );          // time 2.1
    double annuity1     = ( N * stub_tau * df_stub ) + ( N * tau * df1 );
    EXPECT_NEAR( result1, annuity1, testPrecision );

    // Test 2: Semi-Annual Swap
    m = 2;
    tau = 1 / double( m );
    stub_tau = 0.6;

    double result2      = validation::tryAqMathEuropeanIRSwaptionCashAnnuity( N, r, m, t, long_start );
    df_stub             = 1.0 / ( 1 + r * stub_tau );               // time 0.7
    df1                 = df_stub * 1.0 / ( 1 + r * tau );          // time 1.1
    double df2          = df1 * 1.0 / ( 1 + r * tau );              // time 1.6
    double df3          = df2 * 1.0 / ( 1 + r * tau );              // time 2.1
    double annuity2     = ( N * stub_tau * df_stub ) + ( N * tau * df1 ) + ( N * tau * df2 ) + ( N * tau * df3 );
    EXPECT_NEAR( result2, annuity2, testPrecision );

}

TEST( TestAqMathEuropeanIRSwaption, UNIT_TestCashSettledAnnuity_WithLongEndStub )
{
    // CashAnnuity( Notional (N), SwapRate (r), nCouponsPerYear (m), TenorInYears (t), StubType )
    double              N;
    double              r;
    unsigned int        m;
    double              t;
    std::string         long_end = "LONG_END";
    double              tau;        // year fraction
    double              stub_tau;   // stub year fraction

    N = 1000000.0;
    r = 0.1;
    t = 2.1;
    stub_tau = 1.1;

    // Test 1: Annual Swap
    m = 1;
    tau = 1 / double( m );

    double result1      = validation::tryAqMathEuropeanIRSwaptionCashAnnuity( N, r, m, t, long_end );
    double df1          = 1.0 / ( 1 + r * tau );                        // time 1.0
    double df_stub      = df1 * 1.0 / ( 1 + r * stub_tau );             // time 2.0
    double annuity1     = ( N * tau * df1 ) + ( N * stub_tau * df_stub );
    EXPECT_NEAR( result1, annuity1, testPrecision );

    // Test 2: Semi-Annual Swap
    m = 2;
    tau = 1 / double( m );
    stub_tau = 0.6;

    double result2  = validation::tryAqMathEuropeanIRSwaptionCashAnnuity( N, r, m, t, long_end );
    df1             = 1.0 / ( 1 + r * tau );                            // time 0.5
    double df2      = df1 * 1.0 / ( 1 + r * tau );                      // time 1.0
    double df3      = df2 * 1.0 / ( 1 + r * tau );                      // time 1.5
    df_stub         = df3 * 1.0 / ( 1 + r * stub_tau );                 // time 2.1
    double annuity2 = ( N * tau * df1 ) + ( N * tau * df2 ) + ( N * tau * df3 ) + ( N * stub_tau * df_stub );
    EXPECT_NEAR( result2, annuity2, testPrecision );

}


TEST( TestAqMathEuropeanIRSwaption, UNIT_TestMeMathEuropeanIRSwaptionPrice_LogNormalVol )
{
    const double priceTestPrecision = 1e-6;

    // Enum Definitions
    const std::string PAYER         = "PAYER";
    const std::string RECEIVER      = "RECEIVER";
    const std::string LOGNORMAL     = "LOGNORMAL";

    // USD Data as at 25-Jan-2018 for USD ATM 1X5 European Swaption
    const double annuity            = 5.0;          // 5 Year Underlying Swap - We assume Discount Factors of 1.0 for this test
    const double swapRate           = 0.027;        // 2.7%
    const double strikeATM          = 0.027;        // 2.7%
    const double strike0PCT         = 0.0;          // 0.0%
    const double strike10PCT        = 0.1;          // 10.0%
    const double timeToExpiry       = 1.0;          // 1 year to Swaption Expiry
    const double discFactor         = 1.0;
    
    // ATM Volatilities
    const double lognormalVol       = 0.2411;       // 24.11%
    const double NO_VOL_SHIFT       = 0.0;

    // Price Tests
    // ------------------------------------

    // PAYER - ATM
    const double payerSwaptionPrice1X5_ATM = validation::tryAqMathEuropeanIRSwaptionPrice( PAYER, annuity, swapRate, strikeATM, lognormalVol, timeToExpiry, NO_VOL_SHIFT, LOGNORMAL );
    EXPECT_NEAR( payerSwaptionPrice1X5_ATM, 0.012953590993095357, priceTestPrecision );

    // RECEIVER - ATM
    const double receiverSwaptionPrice1X5_ATM = validation::tryAqMathEuropeanIRSwaptionPrice( RECEIVER, annuity, swapRate, strikeATM, lognormalVol, timeToExpiry, NO_VOL_SHIFT, LOGNORMAL );
    EXPECT_NEAR( receiverSwaptionPrice1X5_ATM, 0.012953590993095357, priceTestPrecision );

    // ATM PAYER = ATM RECEIVER
    EXPECT_NEAR( payerSwaptionPrice1X5_ATM, receiverSwaptionPrice1X5_ATM, priceTestPrecision );

    // DEEP OTM
    const double payerSwaptionPrice1X5_OTM = validation::tryAqMathEuropeanIRSwaptionPrice( PAYER, annuity, swapRate, strike10PCT, lognormalVol, timeToExpiry, NO_VOL_SHIFT, LOGNORMAL );
    EXPECT_NEAR( payerSwaptionPrice1X5_OTM, 0.0, priceTestPrecision );
    
    const double receiverSwaptionPrice1X5_OTM = validation::tryAqMathEuropeanIRSwaptionPrice( RECEIVER, annuity, swapRate, strike0PCT, lognormalVol, timeToExpiry, NO_VOL_SHIFT, LOGNORMAL );
    EXPECT_NEAR( receiverSwaptionPrice1X5_OTM, 0.0, priceTestPrecision );

    // DEEP ITM
    const double payerSwaptionPrice1X5_ITM = validation::tryAqMathEuropeanIRSwaptionPrice( PAYER, annuity, swapRate, strike0PCT, lognormalVol, timeToExpiry, NO_VOL_SHIFT, LOGNORMAL );
    EXPECT_NEAR( payerSwaptionPrice1X5_ITM, 0.135000, priceTestPrecision );
    
    const double receiverSwaptionPrice1X5_ITM = validation::tryAqMathEuropeanIRSwaptionPrice( RECEIVER, annuity, swapRate, strike10PCT, lognormalVol, timeToExpiry, NO_VOL_SHIFT, LOGNORMAL );
    EXPECT_NEAR( receiverSwaptionPrice1X5_ITM, 0.365000, priceTestPrecision );
}


TEST( TestAqMathEuropeanIRSwaption, UNIT_TestMeMathEuropeanIRSwaptionPrice_NormalVol )
{
    const double priceTestPrecision = 1e-6;

    // Enum Definitions
    const std::string PAYER         = "PAYER";
    const std::string RECEIVER      = "RECEIVER";
    const std::string NORMAL        = "NORMAL";

    // USD Data as at 25-Jan-2018 for USD ATM 1X5 European Swaption
    const double annuity            = 5.0;          // 5 Year Underlying Swap - We assume Discount Factors of 1.0 for this test
    const double swapRate           = 0.027;        // 2.7%
    const double strikeATM          = 0.027;        // 2.7%
    const double strike0PCT         = 0.0;          // 0.0%
    const double strike10PCT        = 0.1;          // 10.0%
    const double timeToExpiry       = 1.0;          // 1 year to Swaption Expiry
    const double discFactor         = 1.0;
    
    // ATM Volatilities
    const double normalVol          = 0.006070;       // 60.70 basis points or 0.6070%
    const double NO_VOL_SHIFT       = 0.0;
    
    // Price Tests
    // ------------------------------------

    // PAYER - ATM
    const double payerSwaptionPrice1X5_ATM = validation::tryAqMathEuropeanIRSwaptionPrice( PAYER, annuity, swapRate, strikeATM, normalVol, timeToExpiry, NO_VOL_SHIFT, NORMAL );
    EXPECT_NEAR( payerSwaptionPrice1X5_ATM, 0.012107898210183482, priceTestPrecision );

    // RECEIVER - ATM
    const double receiverSwaptionPrice1X5_ATM = validation::tryAqMathEuropeanIRSwaptionPrice( RECEIVER, annuity, swapRate, strikeATM, normalVol, timeToExpiry, NO_VOL_SHIFT, NORMAL );
    EXPECT_NEAR( receiverSwaptionPrice1X5_ATM, 0.012107898210183482, priceTestPrecision );

    // ATM PAYER = ATM RECEIVER
    EXPECT_NEAR( payerSwaptionPrice1X5_ATM, receiverSwaptionPrice1X5_ATM, priceTestPrecision );

    // DEEP OTM
    const double payerSwaptionPrice1X5_OTM = validation::tryAqMathEuropeanIRSwaptionPrice( PAYER, annuity, swapRate, strike10PCT, normalVol, timeToExpiry, NO_VOL_SHIFT, NORMAL );
    EXPECT_NEAR( payerSwaptionPrice1X5_OTM, 0.0, priceTestPrecision );
    
    const double receiverSwaptionPrice1X5_OTM = validation::tryAqMathEuropeanIRSwaptionPrice( RECEIVER, annuity, swapRate, strike0PCT, normalVol, timeToExpiry, NO_VOL_SHIFT, NORMAL );
    EXPECT_NEAR( receiverSwaptionPrice1X5_OTM, 0.0, priceTestPrecision );

    // DEEP ITM
    const double payerSwaptionPrice1X5_ITM = validation::tryAqMathEuropeanIRSwaptionPrice( PAYER, annuity, swapRate, strike0PCT, normalVol, timeToExpiry, NO_VOL_SHIFT, NORMAL );
    EXPECT_NEAR( payerSwaptionPrice1X5_ITM, 0.135000, priceTestPrecision );
    
    const double receiverSwaptionPrice1X5_ITM = validation::tryAqMathEuropeanIRSwaptionPrice( RECEIVER, annuity, swapRate, strike10PCT, normalVol, timeToExpiry, NO_VOL_SHIFT, NORMAL );
    EXPECT_NEAR( receiverSwaptionPrice1X5_ITM, 0.365000, priceTestPrecision );
}
