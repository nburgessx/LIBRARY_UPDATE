// Include: Google Test Library
#include <gTest/gTest.h>

// AlgoQuantLib Includes
#include "CurveInstruments.h"
#include "AQLBasic.h"
#include "AQLSplineInterpolation.h"


namespace google_test
{
    TEST( TestStateVariableUtilities, UNIT_stateVariableValue )
    {
        double actual = 0.0;
        double expected = 0.0;

        double zeroRate = 0.01;
        double term = 2.0;

        // 1. Check we get back the Zero Rate when it is the state variable
        actual = etrading::getStateVariableValue( zeroRate, term, STATE_VARIABLE_ZERO_RATE );
        expected = zeroRate;
        EXPECT_EQ( expected, actual );

        // 2. Zero Rate times Time Check
        actual = getStateVariableValue( zeroRate, term, STATE_VARIABLE_ZERO_RATE_TIMES_TIME );
        expected = zeroRate * term;
        EXPECT_EQ( expected, actual );

        // 3. Log Discount Factor Check
        actual = getStateVariableValue( zeroRate, term, STATE_VARIABLE_LOG_DF );
        expected = -zeroRate * term;
        EXPECT_EQ( expected, actual );

        // 4. Discount Factor Check
        actual = getStateVariableValue( zeroRate, term, STATE_VARIABLE_DF );
        expected = AQLMath::exp( -zeroRate * term );
        EXPECT_EQ( expected, actual );
    }


    TEST( TestStateVariableUtilities, UNIT_getZeroRateFromStateVariable )
    {
        double actual = 0.0;
        
        double term = 2.0;
        double zeroRate = 0.01;
        double zeroRateTimesTime = zeroRate * term;
        double logDF = -zeroRate * term;
        double df = AQLMath::exp( -zeroRate * term );

		AQLDate asOfDate("20200422");
		etrading::DayCountEnum accrualDaycount(etrading::ACT_365_DAYCOUNT);
		etrading::CompoundingFrequencyEnum compoundFreq(etrading::SIMPLE_COMPOUNDING);

        // 1. Check we get back the Zero Rate when it is the state variable
        actual = getZeroRateFromStateVariable( term, zeroRate, STATE_VARIABLE_ZERO_RATE );
        EXPECT_EQ( zeroRate, actual );

        // 2. Check for zero rate from zero rate times time
        actual = getZeroRateFromStateVariable( term, zeroRateTimesTime, STATE_VARIABLE_ZERO_RATE_TIMES_TIME );
        EXPECT_EQ( zeroRate, actual );

        // 3. Check for zero rate from Log DF
        actual = getZeroRateFromStateVariable( term, logDF, STATE_VARIABLE_LOG_DF );
        EXPECT_EQ( zeroRate, actual );

        // 4. Check for zero rate from DF
        actual = getZeroRateFromStateVariable( term, df, STATE_VARIABLE_DF );
        EXPECT_NEAR( zeroRate, actual, 1e-15 ); // AQLMath::log function precision is high, but not exact

        // 5. Check that the zero rate is zero regardless of state variable value when the term is zero
        actual = getZeroRateFromStateVariable( 0.0, zeroRate, STATE_VARIABLE_ZERO_RATE );
        EXPECT_EQ( 0.0, actual );

        actual = getZeroRateFromStateVariable( 0.0, zeroRateTimesTime, STATE_VARIABLE_ZERO_RATE_TIMES_TIME );
        EXPECT_EQ( 0.0, actual );
        
        actual = getZeroRateFromStateVariable( 0.0, logDF, STATE_VARIABLE_LOG_DF );
        EXPECT_EQ( 0.0, actual );

        actual = getZeroRateFromStateVariable( 0.0, df, STATE_VARIABLE_DF );
        EXPECT_EQ( 0.0, actual );
    }


    TEST( TestStateVariableUtilities, UNIT_getInterpolatedDiscountfactor_byZeroRate )
    {
        // Declare State Variables
        const double time1 = 0.0;
        const double time2 = 1.0;
        const double time3 = 2.0;

        const double zero1 = 0.0;
        const double zero2 = 0.01;
        const double zero3 = 0.02;

        const double df1 = AQLMath::exp( -zero1 * time1 );
        const double df2 = AQLMath::exp( -zero2 * time2 );
        const double df3 = AQLMath::exp( -zero3 * time3 );

		AQLDate asOfDate("20200422");
		etrading::DayCountEnum accrualDaycount(etrading::ACT_365_DAYCOUNT);
		etrading::CompoundingFrequencyEnum compoundFreq(etrading::SIMPLE_COMPOUNDING);

        // Update Vector Data for Interpolator
        DoubleArray timeGrid;
        DoubleArray zeroRates;

        timeGrid.push_back( time1 );
        timeGrid.push_back( time2 );
        timeGrid.push_back( time3 );

        zeroRates.push_back( zero1 );
        zeroRates.push_back( zero2 );
        zeroRates.push_back( zero3 );
        
        // Cubic Spline Interpolation
        AQLSplineInterpolation pInterpolatorDF;
        pInterpolatorDF.set( timeGrid, zeroRates );

        // Check discount factors can be retrieved correctly
        double result1 = getInterpolatedDiscountfactor(pInterpolatorDF, time1, STATE_VARIABLE_ZERO_RATE, asOfDate, accrualDaycount, compoundFreq );
        EXPECT_EQ( df1, result1 );

        double result2 = getInterpolatedDiscountfactor(pInterpolatorDF, time2, STATE_VARIABLE_ZERO_RATE, asOfDate, accrualDaycount, compoundFreq );
        EXPECT_EQ( df2, result2 );

        double result3 = getInterpolatedDiscountfactor(pInterpolatorDF, time3, STATE_VARIABLE_ZERO_RATE, asOfDate, accrualDaycount, compoundFreq );
        EXPECT_EQ( df3, result3 );
    }


    TEST( TestStateVariableUtilities, UNIT_getInterpolatedDiscountfactor_byZeroRateTimesTime )
    {
        // Declare State Variables
        const double time1 = 0.0;
        const double time2 = 1.0;
        const double time3 = 2.0;

        const double zero1 = 0.0;
        const double zero2 = 0.01;
        const double zero3 = 0.02;

        const double zt1 = zero1 * time1;
        const double zt2 = zero2 * time2;
        const double zt3 = zero3 * time3;

        const double df1 = AQLMath::exp( -zt1 );
        const double df2 = AQLMath::exp( -zt2 );
        const double df3 = AQLMath::exp( -zt3 );

		AQLDate asOfDate("20200422");
		etrading::DayCountEnum accrualDaycount(etrading::ACT_365_DAYCOUNT);
		etrading::CompoundingFrequencyEnum compoundFreq(etrading::SIMPLE_COMPOUNDING);

        // Update Vector Data for Interpolator
        DoubleArray timeGrid;
        DoubleArray zeroRateTimesTimeGrid;

        timeGrid.push_back( time1 );
        timeGrid.push_back( time2 );
        timeGrid.push_back( time3 );

        zeroRateTimesTimeGrid.push_back( zt1 );
        zeroRateTimesTimeGrid.push_back( zt2 );
        zeroRateTimesTimeGrid.push_back( zt3 );
        
        // Cubic Spline Interpolation
        AQLSplineInterpolation pInterpolatorDF;
        pInterpolatorDF.set( timeGrid, zeroRateTimesTimeGrid );

        // Check discount factors can be retrieved correctly
        double result1 = getInterpolatedDiscountfactor(pInterpolatorDF, time1, STATE_VARIABLE_ZERO_RATE_TIMES_TIME, asOfDate, accrualDaycount, compoundFreq );
        EXPECT_EQ( df1, result1 );

        double result2 = getInterpolatedDiscountfactor(pInterpolatorDF, time2, STATE_VARIABLE_ZERO_RATE_TIMES_TIME, asOfDate, accrualDaycount, compoundFreq );
        EXPECT_EQ( df2, result2 );

        double result3 = getInterpolatedDiscountfactor(pInterpolatorDF, time3, STATE_VARIABLE_ZERO_RATE_TIMES_TIME, asOfDate, accrualDaycount, compoundFreq );
        EXPECT_EQ( df3, result3 );
    }


    TEST( TestStateVariableUtilities, UNIT_getInterpolatedDiscountfactor_byLogDF )
    {
        // Declare State Variables
        const double time1 = 0.0;
        const double time2 = 1.0;
        const double time3 = 2.0;

        const double zero1 = 0.0;
        const double zero2 = 0.01;
        const double zero3 = 0.02;

        const double logDF1 = zero1 * time1;
        const double logDF2 = zero2 * time2;
        const double logDF3 = zero3 * time3;

        const double df1 = AQLMath::exp( logDF1 );
        const double df2 = AQLMath::exp( logDF2 );
        const double df3 = AQLMath::exp( logDF3 );

		AQLDate asOfDate("20200422");
		etrading::DayCountEnum accrualDaycount(etrading::ACT_365_DAYCOUNT);
		etrading::CompoundingFrequencyEnum compoundFreq(etrading::SIMPLE_COMPOUNDING);

        // Update Vector Data for Interpolator
        DoubleArray timeGrid;
        DoubleArray logDFGrid;

        timeGrid.push_back( time1 );
        timeGrid.push_back( time2 );
        timeGrid.push_back( time3 );

        logDFGrid.push_back( logDF1 );
        logDFGrid.push_back( logDF2 );
        logDFGrid.push_back( logDF3 );
        
        // Cubic Spline Interpolation
        AQLSplineInterpolation pInterpolatorDF;
        pInterpolatorDF.set( timeGrid, logDFGrid );

        // Check discount factors can be retrieved correctly
        double result1 = getInterpolatedDiscountfactor(pInterpolatorDF, time1, STATE_VARIABLE_LOG_DF, asOfDate, accrualDaycount, compoundFreq );
        EXPECT_EQ( df1, result1 );

        double result2 = getInterpolatedDiscountfactor(pInterpolatorDF, time2, STATE_VARIABLE_LOG_DF, asOfDate, accrualDaycount, compoundFreq );
        EXPECT_EQ( df2, result2 );

        double result3 = getInterpolatedDiscountfactor(pInterpolatorDF, time3, STATE_VARIABLE_LOG_DF, asOfDate, accrualDaycount, compoundFreq );
        EXPECT_EQ( df3, result3 );
    }


    TEST( TestStateVariableUtilities, UNIT_getInterpolatedDiscountfactor_byDF )
    {
        // Declare State Variables
        const double time1 = 0.0;
        const double time2 = 1.0;
        const double time3 = 2.0;

        const double df1 = 1.0;
        const double df2 = 0.9;
        const double df3 = 0.8;

		AQLDate asOfDate("20200422");
		etrading::DayCountEnum accrualDaycount(etrading::ACT_365_DAYCOUNT);
		etrading::CompoundingFrequencyEnum compoundFreq(etrading::SIMPLE_COMPOUNDING);

        // Update Vector Data for Interpolator
        DoubleArray timeGrid;
        DoubleArray discountFactors;

        timeGrid.push_back( time1 );
        timeGrid.push_back( time2 );
        timeGrid.push_back( time3 );

        discountFactors.push_back( df1 );
        discountFactors.push_back( df2 );
        discountFactors.push_back( df3 );
        
        // Cubic Spline Interpolation
        AQLSplineInterpolation pInterpolatorDF;
        pInterpolatorDF.set( timeGrid, discountFactors );

        // Check discount factors can be retrieved correctly
        double result1 = getInterpolatedDiscountfactor(pInterpolatorDF, time1, STATE_VARIABLE_DF, asOfDate, accrualDaycount, compoundFreq );
        EXPECT_EQ( df1, result1 );

        double result2 = getInterpolatedDiscountfactor(pInterpolatorDF, time2, STATE_VARIABLE_DF, asOfDate, accrualDaycount, compoundFreq );
        EXPECT_EQ( df2, result2 );

        double result3 = getInterpolatedDiscountfactor(pInterpolatorDF, time3, STATE_VARIABLE_DF, asOfDate, accrualDaycount, compoundFreq );
        EXPECT_EQ( df3, result3 );
    }


    TEST( TestStateVariableUtilities, UNIT_getInterpolatedZeroRate_byZeroRate )
    {
        // Declare State Variables
        const double time1 = 0.0;
        const double time2 = 1.0;
        const double time3 = 2.0;

        const double zero1 = 0.0;
        const double zero2 = 0.01;
        const double zero3 = 0.02;

		AQLDate asOfDate("20200422");
		etrading::DayCountEnum accrualDaycount(etrading::ACT_365_DAYCOUNT);
		etrading::CompoundingFrequencyEnum compoundFreq(etrading::SIMPLE_COMPOUNDING);

        // Update Vector Data for Interpolator
        DoubleArray timeGrid;
        DoubleArray zeroRates;

        timeGrid.push_back( time1 );
        timeGrid.push_back( time2 );
        timeGrid.push_back( time3 );

        zeroRates.push_back( zero1 );
        zeroRates.push_back( zero2 );
        zeroRates.push_back( zero3 );
        
        // Cubic Spline Interpolation
        AQLSplineInterpolation pInterpolatorDF;
        pInterpolatorDF.set( timeGrid, zeroRates );

        // Check discount factors can be retrieved correctly
        double result1 = getInterpolatedZeroRate( time1, &pInterpolatorDF, STATE_VARIABLE_ZERO_RATE, asOfDate, accrualDaycount, compoundFreq );
        EXPECT_EQ( zero1, result1 );

        double result2 = getInterpolatedZeroRate( time2, &pInterpolatorDF, STATE_VARIABLE_ZERO_RATE, asOfDate, accrualDaycount, compoundFreq );
        EXPECT_EQ( zero2, result2 );

        double result3 = getInterpolatedZeroRate( time3, &pInterpolatorDF, STATE_VARIABLE_ZERO_RATE, asOfDate, accrualDaycount, compoundFreq );
        EXPECT_EQ( zero3, result3 );
    }

    
    TEST( TestStateVariableUtilities, UNIT_getInterpolatedZeroRate_byZeroRateTimesTime )
    {
        // Declare State Variables
        const double time1 = 0.0;
        const double time2 = 1.0;
        const double time3 = 2.0;

        const double zero1 = 0.0;
        const double zero2 = 0.01;
        const double zero3 = 0.02;

        const double zt1 = zero1 * time1;
        const double zt2 = zero2 * time2;
        const double zt3 = zero3 * time3;

		AQLDate asOfDate("20200422");
		etrading::DayCountEnum accrualDaycount(etrading::ACT_365_DAYCOUNT);
		etrading::CompoundingFrequencyEnum compoundFreq(etrading::SIMPLE_COMPOUNDING);

        // Update Vector Data for Interpolator
        DoubleArray timeGrid;
        DoubleArray zeroRateTimesTimeGrid;

        timeGrid.push_back( time1 );
        timeGrid.push_back( time2 );
        timeGrid.push_back( time3 );

        zeroRateTimesTimeGrid.push_back( zt1 );
        zeroRateTimesTimeGrid.push_back( zt2 );
        zeroRateTimesTimeGrid.push_back( zt3 );
        
        // Cubic Spline Interpolation
        AQLSplineInterpolation pInterpolatorDF;
        pInterpolatorDF.set( timeGrid, zeroRateTimesTimeGrid );

        // Check discount factors can be retrieved correctly
        double result1 = getInterpolatedZeroRate( time1, &pInterpolatorDF, STATE_VARIABLE_ZERO_RATE_TIMES_TIME, asOfDate, accrualDaycount, compoundFreq );
        EXPECT_EQ( zero1, result1 );

        double result2 = getInterpolatedZeroRate( time2, &pInterpolatorDF, STATE_VARIABLE_ZERO_RATE_TIMES_TIME, asOfDate, accrualDaycount, compoundFreq );
        EXPECT_EQ( zero2, result2 );

        double result3 = getInterpolatedZeroRate( time3, &pInterpolatorDF, STATE_VARIABLE_ZERO_RATE_TIMES_TIME, asOfDate, accrualDaycount, compoundFreq );
        EXPECT_EQ( zero3, result3 );
    }


    TEST( TestStateVariableUtilities, UNIT_getInterpolatedZeroRate_byLogDF )
    {
        // Declare State Variables
        const double time1 = 0.0;
        const double time2 = 1.0;
        const double time3 = 2.0;

        const double zero1 = 0.0;
        const double zero2 = 0.01;
        const double zero3 = 0.02;

        const double logDF1 = -zero1 * time1;
        const double logDF2 = -zero2 * time2;
        const double logDF3 = -zero3 * time3;

		AQLDate asOfDate("20200422");
		etrading::DayCountEnum accrualDaycount(etrading::ACT_365_DAYCOUNT);
		etrading::CompoundingFrequencyEnum compoundFreq(etrading::SIMPLE_COMPOUNDING);

        // Update Vector Data for Interpolator
        DoubleArray timeGrid;
        DoubleArray logDFGrid;

        timeGrid.push_back( time1 );
        timeGrid.push_back( time2 );
        timeGrid.push_back( time3 );

        logDFGrid.push_back( logDF1 );
        logDFGrid.push_back( logDF2 );
        logDFGrid.push_back( logDF3 );
        
        // Cubic Spline Interpolation
        AQLSplineInterpolation pInterpolatorDF;
        pInterpolatorDF.set( timeGrid, logDFGrid );

        // Check discount factors can be retrieved correctly
        double result1 = getInterpolatedZeroRate( time1, &pInterpolatorDF, STATE_VARIABLE_LOG_DF, asOfDate, accrualDaycount, compoundFreq );
        EXPECT_EQ( zero1, result1 );

        double result2 = getInterpolatedZeroRate( time2, &pInterpolatorDF, STATE_VARIABLE_LOG_DF, asOfDate, accrualDaycount, compoundFreq );
        EXPECT_EQ( zero2, result2 );

        double result3 = getInterpolatedZeroRate( time3, &pInterpolatorDF, STATE_VARIABLE_LOG_DF, asOfDate, accrualDaycount, compoundFreq );
        EXPECT_EQ( zero3, result3 );
    }


    TEST( TestStateVariableUtilities, UNIT_getInterpolatedZeroRate_byDF )
    {
        // Declare State Variables
        const double time1 = 0.0;
        const double time2 = 1.0;
        const double time3 = 2.0;

        const double zero1 = 0.0;
        const double zero2 = 0.01;
        const double zero3 = 0.02;

		AQLDate asOfDate("20200422");
		etrading::DayCountEnum accrualDaycount(etrading::ACT_365_DAYCOUNT);
		etrading::CompoundingFrequencyEnum compoundFreq(etrading::SIMPLE_COMPOUNDING);

        const double df1 = AQLMath::exp( -zero1 * time1 );
        const double df2 = AQLMath::exp( -zero2 * time2 );
        const double df3 = AQLMath::exp( -zero3 * time3 );

        // Update Vector Data for Interpolator
        DoubleArray timeGrid;
        DoubleArray discountFactors;

        timeGrid.push_back( time1 );
        timeGrid.push_back( time2 );
        timeGrid.push_back( time3 );

        discountFactors.push_back( df1 );
        discountFactors.push_back( df2 );
        discountFactors.push_back( df3 );
        
        // Cubic Spline Interpolation
        AQLSplineInterpolation pInterpolatorDF;
        pInterpolatorDF.set( timeGrid, discountFactors );

        // Check discount factors can be retrieved correctly
        double result1 = getInterpolatedZeroRate( time1, &pInterpolatorDF, STATE_VARIABLE_DF, asOfDate, accrualDaycount, compoundFreq );
        EXPECT_NEAR( zero1, result1, 1e-15 ); // AQLMath::log function precision is high, but not exact

        double result2 = getInterpolatedZeroRate( time2, &pInterpolatorDF, STATE_VARIABLE_DF, asOfDate, accrualDaycount, compoundFreq );
        EXPECT_NEAR( zero2, result2, 1e-15 ); // AQLMath::log function precision is high, but not exact

        double result3 = getInterpolatedZeroRate( time3, &pInterpolatorDF, STATE_VARIABLE_DF, asOfDate, accrualDaycount, compoundFreq );
        EXPECT_NEAR( zero3, result3, 1e-15 ); // AQLMath::log function precision is high, but not exact
    }


}

