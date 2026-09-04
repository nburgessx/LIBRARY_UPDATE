// aqMathConvexity.cpp
#include "ConvexityModel.h"
#include "tryAqMathConvexity.h"

// Include: Google Test Library
#include <gTest/gTest.h>


const double testPrecision = 1e-8;


// Test Cases
// ------------------------------

TEST( aqMathLiborConvexityAdjustmentInArrears, UNIT_UnnaturalLibor_ConvexityCorrections_EUR_NormalVol )
{
    // Normal Volatility Parameters
    const double normalVol = 0.0020; // 20 bps
    etrading::VolatilityTypeEnum NORMALVOL = etrading::NORMAL_VOLATILITY;
    etrading::Convexity::VolatilityParameters normalVolParameters( normalVol, NORMALVOL );
    
    // Build Convexity Model
    etrading::ConvexityModel normalConvexityModel( normalVolParameters );

    // Coupon Daycount YearFraction
    const double couponYearFraction = 0.25;

    // Natural Libor Rates
    const double liborRate1  = -0.0029524;
    const double liborRate2  = -0.0029062;
    const double liborRate3  = -0.0026462;
    const double liborRate4  = -0.0023827;
    const double liborRate5  = -0.0020566;
    const double liborRate6  = -0.0016288;
    const double liborRate7  = -0.0010828;
    const double liborRate8  = -0.0005097;
    const double liborRate9  =  0.0000343;
    const double liborRate10 =  0.0006654;
    const double liborRate11 =  0.0013865;
    const double liborRate12 =  0.0021785;
    const double liborRate13 =  0.0029580;
    const double liborRate14 =  0.0036908;
    const double liborRate15 =  0.0043894;
    const double liborRate16 =  0.0050609;
    const double liborRate17 =  0.0057476;
    const double liborRate18 =  0.0064727;
    const double liborRate19 =  0.0072329;
    const double liborRate20 =  0.0080056;

    // Coupon Time to Maturity
    const double timeToMaturity1  = 0.25;        
    const double timeToMaturity2  = 0.50;
    const double timeToMaturity3  = 0.75;
    const double timeToMaturity4  = 1.00;
    const double timeToMaturity5  = 1.25;
    const double timeToMaturity6  = 1.50;
    const double timeToMaturity7  = 1.75;
    const double timeToMaturity8  = 2.00;
    const double timeToMaturity9  = 2.25;
    const double timeToMaturity10 = 2.50;
    const double timeToMaturity11 = 2.75;
    const double timeToMaturity12 = 3.00;
    const double timeToMaturity13 = 3.25;
    const double timeToMaturity14 = 3.50;
    const double timeToMaturity15 = 3.75;
    const double timeToMaturity16 = 4.00;
    const double timeToMaturity17 = 4.25;
    const double timeToMaturity18 = 4.50;
    const double timeToMaturity19 = 4.75;
    const double timeToMaturity20 = 5.00;

    // Unnatural Libor Convexity Adjustments
    const double convexityCorrection1  = normalConvexityModel.liborConvexityAdjustmentInArrears(liborRate1, couponYearFraction, timeToMaturity1);
    const double convexityCorrection2  = normalConvexityModel.liborConvexityAdjustmentInArrears(liborRate2, couponYearFraction, timeToMaturity2);
    const double convexityCorrection3  = normalConvexityModel.liborConvexityAdjustmentInArrears(liborRate3, couponYearFraction, timeToMaturity3);
    const double convexityCorrection4  = normalConvexityModel.liborConvexityAdjustmentInArrears(liborRate4, couponYearFraction, timeToMaturity4);
    const double convexityCorrection5  = normalConvexityModel.liborConvexityAdjustmentInArrears(liborRate5, couponYearFraction, timeToMaturity5);
    const double convexityCorrection6  = normalConvexityModel.liborConvexityAdjustmentInArrears(liborRate6, couponYearFraction, timeToMaturity6);
    const double convexityCorrection7  = normalConvexityModel.liborConvexityAdjustmentInArrears(liborRate7, couponYearFraction, timeToMaturity7);
    const double convexityCorrection8  = normalConvexityModel.liborConvexityAdjustmentInArrears(liborRate8, couponYearFraction, timeToMaturity8);
    const double convexityCorrection9  = normalConvexityModel.liborConvexityAdjustmentInArrears(liborRate9, couponYearFraction, timeToMaturity9);
    const double convexityCorrection10 = normalConvexityModel.liborConvexityAdjustmentInArrears(liborRate10, couponYearFraction, timeToMaturity10);
    const double convexityCorrection11 = normalConvexityModel.liborConvexityAdjustmentInArrears(liborRate11, couponYearFraction, timeToMaturity11);
    const double convexityCorrection12 = normalConvexityModel.liborConvexityAdjustmentInArrears(liborRate12, couponYearFraction, timeToMaturity12);
    const double convexityCorrection13 = normalConvexityModel.liborConvexityAdjustmentInArrears(liborRate13, couponYearFraction, timeToMaturity13);
    const double convexityCorrection14 = normalConvexityModel.liborConvexityAdjustmentInArrears(liborRate14, couponYearFraction, timeToMaturity14);
    const double convexityCorrection15 = normalConvexityModel.liborConvexityAdjustmentInArrears(liborRate15, couponYearFraction, timeToMaturity15);
    const double convexityCorrection16 = normalConvexityModel.liborConvexityAdjustmentInArrears(liborRate16, couponYearFraction, timeToMaturity16);
    const double convexityCorrection17 = normalConvexityModel.liborConvexityAdjustmentInArrears(liborRate17, couponYearFraction, timeToMaturity17);
    const double convexityCorrection18 = normalConvexityModel.liborConvexityAdjustmentInArrears(liborRate18, couponYearFraction, timeToMaturity18);
    const double convexityCorrection19 = normalConvexityModel.liborConvexityAdjustmentInArrears(liborRate19, couponYearFraction, timeToMaturity19);
    const double convexityCorrection20 = normalConvexityModel.liborConvexityAdjustmentInArrears( liborRate20, couponYearFraction, timeToMaturity20 );

    // Expected Results
    const double expectedResult1  = 0.000000250;
    const double expectedResult2  = 0.000000500;
    const double expectedResult3  = 0.000000750;
    const double expectedResult4  = 0.000001001;
    const double expectedResult5  = 0.000001251;
    const double expectedResult6  = 0.000001501;
    const double expectedResult7  = 0.000001750;
    const double expectedResult8  = 0.000002000;
    const double expectedResult9  = 0.000002250;
    const double expectedResult10 = 0.000002500;
    const double expectedResult11 = 0.000002749;
    const double expectedResult12 = 0.000002998;
    const double expectedResult13 = 0.000003248;
    const double expectedResult14 = 0.000003497;
    const double expectedResult15 = 0.000003746;
    const double expectedResult16 = 0.000003995;
    const double expectedResult17 = 0.000004244;
    const double expectedResult18 = 0.000004493;
    const double expectedResult19 = 0.000004741;
    const double expectedResult20 = 0.000004990;

	// Test and Compare Results
	// ---------------------
	EXPECT_NEAR( convexityCorrection1 , expectedResult1 , testPrecision );
	EXPECT_NEAR( convexityCorrection2 , expectedResult2 , testPrecision );
	EXPECT_NEAR( convexityCorrection3 , expectedResult3 , testPrecision );
	EXPECT_NEAR( convexityCorrection4 , expectedResult4 , testPrecision );
	EXPECT_NEAR( convexityCorrection5 , expectedResult5 , testPrecision );
    EXPECT_NEAR( convexityCorrection6 , expectedResult6 , testPrecision );
    EXPECT_NEAR( convexityCorrection7 , expectedResult7 , testPrecision );
    EXPECT_NEAR( convexityCorrection8 , expectedResult8 , testPrecision );
    EXPECT_NEAR( convexityCorrection9 , expectedResult9 , testPrecision );
    EXPECT_NEAR( convexityCorrection10, expectedResult10, testPrecision );
    EXPECT_NEAR( convexityCorrection11, expectedResult11, testPrecision );
    EXPECT_NEAR( convexityCorrection12, expectedResult12, testPrecision );
    EXPECT_NEAR( convexityCorrection13, expectedResult13, testPrecision );
    EXPECT_NEAR( convexityCorrection14, expectedResult14, testPrecision );
    EXPECT_NEAR( convexityCorrection15, expectedResult15, testPrecision );
    EXPECT_NEAR( convexityCorrection16, expectedResult16, testPrecision );
    EXPECT_NEAR( convexityCorrection17, expectedResult17, testPrecision );
    EXPECT_NEAR( convexityCorrection18, expectedResult18, testPrecision );
    EXPECT_NEAR( convexityCorrection19, expectedResult19, testPrecision );
    EXPECT_NEAR( convexityCorrection20, expectedResult20, testPrecision );
}

TEST( aqMathLiborRateInArrears, UNIT_UnnaturalLibor_Rates_EUR_NormalVol )
{
    // Normal Volatility Parameters
    const double normalVol = 0.0020; // 20 bps
    etrading::VolatilityTypeEnum NORMALVOL = etrading::NORMAL_VOLATILITY;
    etrading::Convexity::VolatilityParameters normalVolParameters( normalVol, NORMALVOL );
    
    // Build Convexity Model
    etrading::ConvexityModel normalConvexityModel( normalVolParameters );

    // Coupon Daycount YearFraction
    const double couponYearFraction = 0.25;

    // EUR Natural Libor Rates
    const double liborRate1  = -0.0029524;
    const double liborRate2  = -0.0029062;
    const double liborRate3  = -0.0026462;
    const double liborRate4  = -0.0023827;
    const double liborRate5  = -0.0020566;
    const double liborRate6  = -0.0016288;
    const double liborRate7  = -0.0010828;
    const double liborRate8  = -0.0005097;
    const double liborRate9  =  0.0000343;
    const double liborRate10 =  0.0006654;
    const double liborRate11 =  0.0013865;
    const double liborRate12 =  0.0021785;
    const double liborRate13 =  0.0029580;
    const double liborRate14 =  0.0036908;
    const double liborRate15 =  0.0043894;
    const double liborRate16 =  0.0050609;
    const double liborRate17 =  0.0057476;
    const double liborRate18 =  0.0064727;
    const double liborRate19 =  0.0072329;
    const double liborRate20 =  0.0080056;

    // Coupon Time to Maturity
    const double timeToMaturity1  = 0.25;        
    const double timeToMaturity2  = 0.50;
    const double timeToMaturity3  = 0.75;
    const double timeToMaturity4  = 1.00;
    const double timeToMaturity5  = 1.25;
    const double timeToMaturity6  = 1.50;
    const double timeToMaturity7  = 1.75;
    const double timeToMaturity8  = 2.00;
    const double timeToMaturity9  = 2.25;
    const double timeToMaturity10 = 2.50;
    const double timeToMaturity11 = 2.75;
    const double timeToMaturity12 = 3.00;
    const double timeToMaturity13 = 3.25;
    const double timeToMaturity14 = 3.50;
    const double timeToMaturity15 = 3.75;
    const double timeToMaturity16 = 4.00;
    const double timeToMaturity17 = 4.25;
    const double timeToMaturity18 = 4.50;
    const double timeToMaturity19 = 4.75;
    const double timeToMaturity20 = 5.00;

    // Unnatural Libor Rates
    const double convexityAdjustedLiborRate1  = normalConvexityModel.liborRateInArrears( liborRate1 , couponYearFraction, timeToMaturity1  );
    const double convexityAdjustedLiborRate2  = normalConvexityModel.liborRateInArrears( liborRate2 , couponYearFraction, timeToMaturity2  );
    const double convexityAdjustedLiborRate3  = normalConvexityModel.liborRateInArrears( liborRate3 , couponYearFraction, timeToMaturity3  );
    const double convexityAdjustedLiborRate4  = normalConvexityModel.liborRateInArrears( liborRate4 , couponYearFraction, timeToMaturity4  );
    const double convexityAdjustedLiborRate5  = normalConvexityModel.liborRateInArrears( liborRate5 , couponYearFraction, timeToMaturity5  );
    const double convexityAdjustedLiborRate6  = normalConvexityModel.liborRateInArrears( liborRate6 , couponYearFraction, timeToMaturity6  );
    const double convexityAdjustedLiborRate7  = normalConvexityModel.liborRateInArrears( liborRate7 , couponYearFraction, timeToMaturity7  );
    const double convexityAdjustedLiborRate8  = normalConvexityModel.liborRateInArrears( liborRate8 , couponYearFraction, timeToMaturity8  );
    const double convexityAdjustedLiborRate9  = normalConvexityModel.liborRateInArrears( liborRate9 , couponYearFraction, timeToMaturity9  );
    const double convexityAdjustedLiborRate10 = normalConvexityModel.liborRateInArrears( liborRate10, couponYearFraction, timeToMaturity10 );
    const double convexityAdjustedLiborRate11 = normalConvexityModel.liborRateInArrears( liborRate11, couponYearFraction, timeToMaturity11 );
    const double convexityAdjustedLiborRate12 = normalConvexityModel.liborRateInArrears( liborRate12, couponYearFraction, timeToMaturity12 );
    const double convexityAdjustedLiborRate13 = normalConvexityModel.liborRateInArrears( liborRate13, couponYearFraction, timeToMaturity13 );
    const double convexityAdjustedLiborRate14 = normalConvexityModel.liborRateInArrears( liborRate14, couponYearFraction, timeToMaturity14 );
    const double convexityAdjustedLiborRate15 = normalConvexityModel.liborRateInArrears( liborRate15, couponYearFraction, timeToMaturity15 );
    const double convexityAdjustedLiborRate16 = normalConvexityModel.liborRateInArrears( liborRate16, couponYearFraction, timeToMaturity16 );
    const double convexityAdjustedLiborRate17 = normalConvexityModel.liborRateInArrears( liborRate17, couponYearFraction, timeToMaturity17 );
    const double convexityAdjustedLiborRate18 = normalConvexityModel.liborRateInArrears( liborRate18, couponYearFraction, timeToMaturity18 );
    const double convexityAdjustedLiborRate19 = normalConvexityModel.liborRateInArrears( liborRate19, couponYearFraction, timeToMaturity19 );
    const double convexityAdjustedLiborRate20 = normalConvexityModel.liborRateInArrears( liborRate20, couponYearFraction, timeToMaturity20 );

    // Expected Results
    const double expectedResult1  = -0.002952150;
    const double expectedResult2  = -0.002905700;
    const double expectedResult3  = -0.002645450;
    const double expectedResult4  = -0.002381699;
    const double expectedResult5  = -0.002055349;
    const double expectedResult6  = -0.001627299;
    const double expectedResult7  = -0.001081050;
    const double expectedResult8  = -0.000507700;
    const double expectedResult9  =  0.000036550;
    const double expectedResult10 =  0.000667900;
    const double expectedResult11 =  0.001389249;
    const double expectedResult12 =  0.002181498;
    const double expectedResult13 =  0.002961248;
    const double expectedResult14 =  0.003694297;
    const double expectedResult15 =  0.004393146;
    const double expectedResult16 =  0.005064895;
    const double expectedResult17 =  0.005751844;
    const double expectedResult18 =  0.006477193;
    const double expectedResult19 =  0.007237641;
    const double expectedResult20 =  0.008010590;


	// Test and Compare Results
	EXPECT_NEAR( convexityAdjustedLiborRate1 , expectedResult1 , testPrecision );
	EXPECT_NEAR( convexityAdjustedLiborRate2 , expectedResult2 , testPrecision );
	EXPECT_NEAR( convexityAdjustedLiborRate3 , expectedResult3 , testPrecision );
	EXPECT_NEAR( convexityAdjustedLiborRate4 , expectedResult4 , testPrecision );
	EXPECT_NEAR( convexityAdjustedLiborRate5 , expectedResult5 , testPrecision );
    EXPECT_NEAR( convexityAdjustedLiborRate6 , expectedResult6 , testPrecision );
    EXPECT_NEAR( convexityAdjustedLiborRate7 , expectedResult7 , testPrecision );
    EXPECT_NEAR( convexityAdjustedLiborRate8 , expectedResult8 , testPrecision );
    EXPECT_NEAR( convexityAdjustedLiborRate9 , expectedResult9 , testPrecision );
    EXPECT_NEAR( convexityAdjustedLiborRate10, expectedResult10, testPrecision );
    EXPECT_NEAR( convexityAdjustedLiborRate11, expectedResult11, testPrecision );
    EXPECT_NEAR( convexityAdjustedLiborRate12, expectedResult12, testPrecision );
    EXPECT_NEAR( convexityAdjustedLiborRate13, expectedResult13, testPrecision );
    EXPECT_NEAR( convexityAdjustedLiborRate14, expectedResult14, testPrecision );
    EXPECT_NEAR( convexityAdjustedLiborRate15, expectedResult15, testPrecision );
    EXPECT_NEAR( convexityAdjustedLiborRate16, expectedResult16, testPrecision );
    EXPECT_NEAR( convexityAdjustedLiborRate17, expectedResult17, testPrecision );
    EXPECT_NEAR( convexityAdjustedLiborRate18, expectedResult18, testPrecision );
    EXPECT_NEAR( convexityAdjustedLiborRate19, expectedResult19, testPrecision );
    EXPECT_NEAR( convexityAdjustedLiborRate20, expectedResult20, testPrecision );
}

TEST( aqMathLiborConvexityAdjustmentInArrears, UNIT_UnnaturalLibor_ConvexityCorrections_USD_LognormalVol )
{
    // Lognormal Volatility Parameters
    const double lognormalVol = 0.3150; // 31.50%
    etrading::VolatilityTypeEnum LOGNORMALVOL = etrading::LOGNORMAL_VOLATILITY;
    etrading::Convexity::VolatilityParameters lognormalVolParameters( lognormalVol, LOGNORMALVOL );
    
    // Build Convexity Model
    etrading::ConvexityModel lognormalConvexityModel( lognormalVolParameters );

    // Coupon Daycount YearFraction
    const double couponYearFraction = 0.25;

    // USD Natural Libor Rates
    const double liborRate1  = 0.02696010;
    const double liborRate2  = 0.02726080;
    const double liborRate3  = 0.02713070;
    const double liborRate4  = 0.02695570;
    const double liborRate5  = 0.02618070;
    const double liborRate6  = 0.02600710;
    const double liborRate7  = 0.02619660;
    const double liborRate8  = 0.02579930;
    const double liborRate9  = 0.02540090;
    const double liborRate10 = 0.02518040;
    const double liborRate11 = 0.02513720;
    const double liborRate12 = 0.02524530;
    const double liborRate13 = 0.02540040;
    const double liborRate14 = 0.02557650;
    const double liborRate15 = 0.02577210;
    const double liborRate16 = 0.02597640;
    const double liborRate17 = 0.02616880;
    const double liborRate18 = 0.02634280;
    const double liborRate19 = 0.02649610;
    const double liborRate20 = 0.02649610;

    // Coupon Time to Maturity
    const double timeToMaturity1  = 0.25;        
    const double timeToMaturity2  = 0.50;
    const double timeToMaturity3  = 0.75;
    const double timeToMaturity4  = 1.00;
    const double timeToMaturity5  = 1.25;
    const double timeToMaturity6  = 1.50;
    const double timeToMaturity7  = 1.75;
    const double timeToMaturity8  = 2.00;
    const double timeToMaturity9  = 2.25;
    const double timeToMaturity10 = 2.50;
    const double timeToMaturity11 = 2.75;
    const double timeToMaturity12 = 3.00;
    const double timeToMaturity13 = 3.25;
    const double timeToMaturity14 = 3.50;
    const double timeToMaturity15 = 3.75;
    const double timeToMaturity16 = 4.00;
    const double timeToMaturity17 = 4.25;
    const double timeToMaturity18 = 4.50;
    const double timeToMaturity19 = 4.75;
    const double timeToMaturity20 = 5.00;

    // Unnatural Libor Convexity Adjustments
    const double convexityCorrection1  = lognormalConvexityModel.liborConvexityAdjustmentInArrears(liborRate1, couponYearFraction, timeToMaturity1);
    const double convexityCorrection2  = lognormalConvexityModel.liborConvexityAdjustmentInArrears(liborRate2, couponYearFraction, timeToMaturity2);
    const double convexityCorrection3  = lognormalConvexityModel.liborConvexityAdjustmentInArrears(liborRate3, couponYearFraction, timeToMaturity3);
    const double convexityCorrection4  = lognormalConvexityModel.liborConvexityAdjustmentInArrears(liborRate4, couponYearFraction, timeToMaturity4);
    const double convexityCorrection5  = lognormalConvexityModel.liborConvexityAdjustmentInArrears(liborRate5, couponYearFraction, timeToMaturity5);
    const double convexityCorrection6  = lognormalConvexityModel.liborConvexityAdjustmentInArrears(liborRate6, couponYearFraction, timeToMaturity6);
    const double convexityCorrection7  = lognormalConvexityModel.liborConvexityAdjustmentInArrears(liborRate7, couponYearFraction, timeToMaturity7);
    const double convexityCorrection8  = lognormalConvexityModel.liborConvexityAdjustmentInArrears(liborRate8, couponYearFraction, timeToMaturity8);
    const double convexityCorrection9  = lognormalConvexityModel.liborConvexityAdjustmentInArrears(liborRate9, couponYearFraction, timeToMaturity9);
    const double convexityCorrection10 = lognormalConvexityModel.liborConvexityAdjustmentInArrears(liborRate10, couponYearFraction, timeToMaturity10);
    const double convexityCorrection11 = lognormalConvexityModel.liborConvexityAdjustmentInArrears(liborRate11, couponYearFraction, timeToMaturity11);
    const double convexityCorrection12 = lognormalConvexityModel.liborConvexityAdjustmentInArrears(liborRate12, couponYearFraction, timeToMaturity12);
    const double convexityCorrection13 = lognormalConvexityModel.liborConvexityAdjustmentInArrears(liborRate13, couponYearFraction, timeToMaturity13);
    const double convexityCorrection14 = lognormalConvexityModel.liborConvexityAdjustmentInArrears(liborRate14, couponYearFraction, timeToMaturity14);
    const double convexityCorrection15 = lognormalConvexityModel.liborConvexityAdjustmentInArrears(liborRate15, couponYearFraction, timeToMaturity15);
    const double convexityCorrection16 = lognormalConvexityModel.liborConvexityAdjustmentInArrears(liborRate16, couponYearFraction, timeToMaturity16);
    const double convexityCorrection17 = lognormalConvexityModel.liborConvexityAdjustmentInArrears(liborRate17, couponYearFraction, timeToMaturity17);
    const double convexityCorrection18 = lognormalConvexityModel.liborConvexityAdjustmentInArrears(liborRate18, couponYearFraction, timeToMaturity18);
    const double convexityCorrection19 = lognormalConvexityModel.liborConvexityAdjustmentInArrears(liborRate19, couponYearFraction, timeToMaturity19);
    const double convexityCorrection20 = lognormalConvexityModel.liborConvexityAdjustmentInArrears( liborRate20, couponYearFraction, timeToMaturity20 );

    // Expected Results
    const double expectedResult1  = 0.0000045334;
    const double expectedResult2  = 0.0000093859;
    const double expectedResult3  = 0.0000141211;
    const double expectedResult4  = 0.0000188222;
    const double expectedResult5  = 0.0000224808;
    const double expectedResult6  = 0.0000269614;
    const double expectedResult7  = 0.0000323226;
    const double expectedResult8  = 0.0000362928;
    const double expectedResult9  = 0.0000400935;
    const double expectedResult10 = 0.0000443486;
    const double expectedResult11 = 0.0000492500;
    const double expectedResult12 = 0.0000548975;
    const double expectedResult13 = 0.0000609934;
    const double expectedResult14 = 0.0000674739;
    const double expectedResult15 = 0.0000743711;
    const double expectedResult16 = 0.0000816583;
    const double expectedResult17 = 0.0000892219;
    const double expectedResult18 = 0.0000970080;
    const double expectedResult19 = 0.0001049807;
    const double expectedResult20 = 0.0001119966;

	// Test and Compare Results
	// ---------------------
	EXPECT_NEAR( convexityCorrection1 , expectedResult1 , testPrecision );
	EXPECT_NEAR( convexityCorrection2 , expectedResult2 , testPrecision );
	EXPECT_NEAR( convexityCorrection3 , expectedResult3 , testPrecision );
	EXPECT_NEAR( convexityCorrection4 , expectedResult4 , testPrecision );
	EXPECT_NEAR( convexityCorrection5 , expectedResult5 , testPrecision );
    EXPECT_NEAR( convexityCorrection6 , expectedResult6 , testPrecision );
    EXPECT_NEAR( convexityCorrection7 , expectedResult7 , testPrecision );
    EXPECT_NEAR( convexityCorrection8 , expectedResult8 , testPrecision );
    EXPECT_NEAR( convexityCorrection9 , expectedResult9 , testPrecision );
    EXPECT_NEAR( convexityCorrection10, expectedResult10, testPrecision );
    EXPECT_NEAR( convexityCorrection11, expectedResult11, testPrecision );
    EXPECT_NEAR( convexityCorrection12, expectedResult12, testPrecision );
    EXPECT_NEAR( convexityCorrection13, expectedResult13, testPrecision );
    EXPECT_NEAR( convexityCorrection14, expectedResult14, testPrecision );
    EXPECT_NEAR( convexityCorrection15, expectedResult15, testPrecision );
    EXPECT_NEAR( convexityCorrection16, expectedResult16, testPrecision );
    EXPECT_NEAR( convexityCorrection17, expectedResult17, testPrecision );
    EXPECT_NEAR( convexityCorrection18, expectedResult18, testPrecision );
    EXPECT_NEAR( convexityCorrection19, expectedResult19, testPrecision );
    EXPECT_NEAR( convexityCorrection20, expectedResult20, testPrecision );
}

TEST( aqMathLiborRateInArrears, UNIT_UnnaturalLibor_Rates_USD_LognormalVol )
{
    // Lognormal Volatility Parameters
    const double lognormalVol = 0.3150; // 31.50%
    etrading::VolatilityTypeEnum LOGNORMALVOL = etrading::LOGNORMAL_VOLATILITY;
    etrading::Convexity::VolatilityParameters lognormalVolParameters( lognormalVol, LOGNORMALVOL );
    
    // Build Convexity Model
    etrading::ConvexityModel lognormalConvexityModel( lognormalVolParameters );

    // Coupon Daycount YearFraction
    const double couponYearFraction = 0.25;

   // USD Natural Libor Rates
    const double liborRate1  = 0.02696010;
    const double liborRate2  = 0.02726080;
    const double liborRate3  = 0.02713070;
    const double liborRate4  = 0.02695570;
    const double liborRate5  = 0.02618070;
    const double liborRate6  = 0.02600710;
    const double liborRate7  = 0.02619660;
    const double liborRate8  = 0.02579930;
    const double liborRate9  = 0.02540090;
    const double liborRate10 = 0.02518040;
    const double liborRate11 = 0.02513720;
    const double liborRate12 = 0.02524530;
    const double liborRate13 = 0.02540040;
    const double liborRate14 = 0.02557650;
    const double liborRate15 = 0.02577210;
    const double liborRate16 = 0.02597640;
    const double liborRate17 = 0.02616880;
    const double liborRate18 = 0.02634280;
    const double liborRate19 = 0.02649610;
    const double liborRate20 = 0.02649610;

    // Coupon Time to Maturity
    const double timeToMaturity1  = 0.25;        
    const double timeToMaturity2  = 0.50;
    const double timeToMaturity3  = 0.75;
    const double timeToMaturity4  = 1.00;
    const double timeToMaturity5  = 1.25;
    const double timeToMaturity6  = 1.50;
    const double timeToMaturity7  = 1.75;
    const double timeToMaturity8  = 2.00;
    const double timeToMaturity9  = 2.25;
    const double timeToMaturity10 = 2.50;
    const double timeToMaturity11 = 2.75;
    const double timeToMaturity12 = 3.00;
    const double timeToMaturity13 = 3.25;
    const double timeToMaturity14 = 3.50;
    const double timeToMaturity15 = 3.75;
    const double timeToMaturity16 = 4.00;
    const double timeToMaturity17 = 4.25;
    const double timeToMaturity18 = 4.50;
    const double timeToMaturity19 = 4.75;
    const double timeToMaturity20 = 5.00;

    // Unnatural Libor Rates
    const double convexityAdjustedLiborRate1  = lognormalConvexityModel.liborRateInArrears( liborRate1 , couponYearFraction, timeToMaturity1  );
    const double convexityAdjustedLiborRate2  = lognormalConvexityModel.liborRateInArrears( liborRate2 , couponYearFraction, timeToMaturity2  );
    const double convexityAdjustedLiborRate3  = lognormalConvexityModel.liborRateInArrears( liborRate3 , couponYearFraction, timeToMaturity3  );
    const double convexityAdjustedLiborRate4  = lognormalConvexityModel.liborRateInArrears( liborRate4 , couponYearFraction, timeToMaturity4  );
    const double convexityAdjustedLiborRate5  = lognormalConvexityModel.liborRateInArrears( liborRate5 , couponYearFraction, timeToMaturity5  );
    const double convexityAdjustedLiborRate6  = lognormalConvexityModel.liborRateInArrears( liborRate6 , couponYearFraction, timeToMaturity6  );
    const double convexityAdjustedLiborRate7  = lognormalConvexityModel.liborRateInArrears( liborRate7 , couponYearFraction, timeToMaturity7  );
    const double convexityAdjustedLiborRate8  = lognormalConvexityModel.liborRateInArrears( liborRate8 , couponYearFraction, timeToMaturity8  );
    const double convexityAdjustedLiborRate9  = lognormalConvexityModel.liborRateInArrears( liborRate9 , couponYearFraction, timeToMaturity9  );
    const double convexityAdjustedLiborRate10 = lognormalConvexityModel.liborRateInArrears( liborRate10, couponYearFraction, timeToMaturity10 );
    const double convexityAdjustedLiborRate11 = lognormalConvexityModel.liborRateInArrears( liborRate11, couponYearFraction, timeToMaturity11 );
    const double convexityAdjustedLiborRate12 = lognormalConvexityModel.liborRateInArrears( liborRate12, couponYearFraction, timeToMaturity12 );
    const double convexityAdjustedLiborRate13 = lognormalConvexityModel.liborRateInArrears( liborRate13, couponYearFraction, timeToMaturity13 );
    const double convexityAdjustedLiborRate14 = lognormalConvexityModel.liborRateInArrears( liborRate14, couponYearFraction, timeToMaturity14 );
    const double convexityAdjustedLiborRate15 = lognormalConvexityModel.liborRateInArrears( liborRate15, couponYearFraction, timeToMaturity15 );
    const double convexityAdjustedLiborRate16 = lognormalConvexityModel.liborRateInArrears( liborRate16, couponYearFraction, timeToMaturity16 );
    const double convexityAdjustedLiborRate17 = lognormalConvexityModel.liborRateInArrears( liborRate17, couponYearFraction, timeToMaturity17 );
    const double convexityAdjustedLiborRate18 = lognormalConvexityModel.liborRateInArrears( liborRate18, couponYearFraction, timeToMaturity18 );
    const double convexityAdjustedLiborRate19 = lognormalConvexityModel.liborRateInArrears( liborRate19, couponYearFraction, timeToMaturity19 );
    const double convexityAdjustedLiborRate20 = lognormalConvexityModel.liborRateInArrears( liborRate20, couponYearFraction, timeToMaturity20 );

    // Expected Results
    const double expectedResult1  = 0.0269646334;
    const double expectedResult2  = 0.0272701859;
    const double expectedResult3  = 0.0271448211;
    const double expectedResult4  = 0.0269745222;
    const double expectedResult5  = 0.0262031808;
    const double expectedResult6  = 0.0260340614;
    const double expectedResult7  = 0.0262289226;
    const double expectedResult8  = 0.0258355928;
    const double expectedResult9  = 0.0254409935;
    const double expectedResult10 = 0.0252247486;
    const double expectedResult11 = 0.0251864500;
    const double expectedResult12 = 0.0253001975;
    const double expectedResult13 = 0.0254613934;
    const double expectedResult14 = 0.0256439739;
    const double expectedResult15 = 0.0258464711;
    const double expectedResult16 = 0.0260580583;
    const double expectedResult17 = 0.0262580219;
    const double expectedResult18 = 0.0264398080;
    const double expectedResult19 = 0.0266010807;
    const double expectedResult20 = 0.0266080966;


	// Test and Compare Results
	EXPECT_NEAR( convexityAdjustedLiborRate1 , expectedResult1 , testPrecision );
	EXPECT_NEAR( convexityAdjustedLiborRate2 , expectedResult2 , testPrecision );
	EXPECT_NEAR( convexityAdjustedLiborRate3 , expectedResult3 , testPrecision );
	EXPECT_NEAR( convexityAdjustedLiborRate4 , expectedResult4 , testPrecision );
	EXPECT_NEAR( convexityAdjustedLiborRate5 , expectedResult5 , testPrecision );
    EXPECT_NEAR( convexityAdjustedLiborRate6 , expectedResult6 , testPrecision );
    EXPECT_NEAR( convexityAdjustedLiborRate7 , expectedResult7 , testPrecision );
    EXPECT_NEAR( convexityAdjustedLiborRate8 , expectedResult8 , testPrecision );
    EXPECT_NEAR( convexityAdjustedLiborRate9 , expectedResult9 , testPrecision );
    EXPECT_NEAR( convexityAdjustedLiborRate10, expectedResult10, testPrecision );
    EXPECT_NEAR( convexityAdjustedLiborRate11, expectedResult11, testPrecision );
    EXPECT_NEAR( convexityAdjustedLiborRate12, expectedResult12, testPrecision );
    EXPECT_NEAR( convexityAdjustedLiborRate13, expectedResult13, testPrecision );
    EXPECT_NEAR( convexityAdjustedLiborRate14, expectedResult14, testPrecision );
    EXPECT_NEAR( convexityAdjustedLiborRate15, expectedResult15, testPrecision );
    EXPECT_NEAR( convexityAdjustedLiborRate16, expectedResult16, testPrecision );
    EXPECT_NEAR( convexityAdjustedLiborRate17, expectedResult17, testPrecision );
    EXPECT_NEAR( convexityAdjustedLiborRate18, expectedResult18, testPrecision );
    EXPECT_NEAR( convexityAdjustedLiborRate19, expectedResult19, testPrecision );
    EXPECT_NEAR( convexityAdjustedLiborRate20, expectedResult20, testPrecision );
}

TEST( aqMathLiborConvexityAdjustmentForArbitraryFixingDate, CONSISTENCY_CheckBoundaryCondition_WhenArbitraryTimeEqualsLiborInArrears_EUR_NormalVol )
{
    // Normal Volatility Parameters
    const double normalVol = 0.0020; // 20 bps
    etrading::VolatilityTypeEnum NORMALVOL = etrading::NORMAL_VOLATILITY;
    etrading::Convexity::VolatilityParameters normalVolParameters( normalVol, NORMALVOL );
    
    // Build Convexity Model
    etrading::ConvexityModel normalConvexityModel( normalVolParameters );

    // Coupon Daycount YearFraction
    const double couponYearFraction = 0.25;

    // Natural Libor Rates
    const std::vector< double > liborRates = { -0.0029524, 
                                               -0.0029062,
                                               -0.0026462,
                                               -0.0023827,
                                               -0.0020566,
                                               -0.0016288,
                                               -0.0010828,
                                               -0.0005097,
                                                0.0000343,
                                                0.0006654,
                                                0.0013865,
                                                0.0021785,
                                                0.0029580,
                                                0.0036908,
                                                0.0043894,
                                                0.0050609,
                                                0.0057476,
                                                0.0064727,
                                                0.0072329,
                                                0.0080056 };
    
    // Coupon Time to Maturity
    const std::vector< double > maturityTimes = { 0.25,
                                                  0.50,
                                                  0.75,
                                                  1.00,
                                                  1.25,
                                                  1.50,
                                                  1.75,
                                                  2.00,
                                                  2.25,
                                                  2.50,
                                                  2.75,
                                                  3.00,
                                                  3.25,
                                                  3.50,
                                                  3.75,
                                                  4.00,
                                                  4.25,
                                                  4.50,
                                                  4.75,
                                                  5.00 };
    
    // Discount Factors
    const std::vector< double > discountFactors = { 1.000893,
                                                    1.001803,
                                                    1.002716,
                                                    1.003566,
                                                    1.004318,
                                                    1.005071,
                                                    1.005640,
                                                    1.006185,
                                                    1.006446,
                                                    1.006703,
                                                    1.006650,
                                                    1.006593,
                                                    1.006013,
                                                    1.005441,
                                                    1.004863,
                                                    1.004279,
                                                    1.003014,
                                                    1.001729,
                                                    1.000432,
                                                    0.999136 };

    // Unnatural Libor Convexity Adjustments: Compare Boundary Condition when Libor In-Arbitrary Time EQUALS Libor In-Arrears
    ASSERT_EQ( liborRates.size(), maturityTimes.size() );
    for( unsigned int i = 0; i < liborRates.size(); ++i )
    {
        const double startDiscountFactor = ( i == 0 ) ? 1.0 : discountFactors[i-1];
        const double fixingYearFraction = couponYearFraction;

        const double liborInArrearsConvexityCorrection
            = normalConvexityModel.liborConvexityAdjustmentInArrears( liborRates[i] ,
                                                                      couponYearFraction,
                                                                      maturityTimes[i] );

        const double liborInArbitraryTimeConvexityCorrection
            = normalConvexityModel.liborConvexityAdjustmentForArbitraryFixingDate( liborRates[i],
                                                                                   couponYearFraction,
                                                                                   maturityTimes[i],
                                                                                   fixingYearFraction,
                                                                                   startDiscountFactor,
                                                                                   discountFactors[i] );
        
        EXPECT_EQ( liborInArrearsConvexityCorrection, liborInArbitraryTimeConvexityCorrection );
    }
}


TEST( aqMathLiborRateForArbitraryFixingDate, CONSISTENCY_CheckBoundaryCondition_WhenArbitraryTimeEqualsLiborInArrears_EUR_NormalVol )
{
    // Normal Volatility Parameters
    const double normalVol = 0.0020; // 20 bps
    etrading::VolatilityTypeEnum NORMALVOL = etrading::NORMAL_VOLATILITY;
    etrading::Convexity::VolatilityParameters normalVolParameters( normalVol, NORMALVOL );
    
    // Build Convexity Model
    etrading::ConvexityModel normalConvexityModel( normalVolParameters );

    // Coupon Daycount YearFraction
    const double couponYearFraction = 0.25;

    // Natural Libor Rates
    const std::vector< double > liborRates = { -0.0029524, 
                                               -0.0029062,
                                               -0.0026462,
                                               -0.0023827,
                                               -0.0020566,
                                               -0.0016288,
                                               -0.0010828,
                                               -0.0005097,
                                                0.0000343,
                                                0.0006654,
                                                0.0013865,
                                                0.0021785,
                                                0.0029580,
                                                0.0036908,
                                                0.0043894,
                                                0.0050609,
                                                0.0057476,
                                                0.0064727,
                                                0.0072329,
                                                0.0080056 };
    
    // Coupon Time to Maturity
    const std::vector< double > maturityTimes = { 0.25,
                                                  0.50,
                                                  0.75,
                                                  1.00,
                                                  1.25,
                                                  1.50,
                                                  1.75,
                                                  2.00,
                                                  2.25,
                                                  2.50,
                                                  2.75,
                                                  3.00,
                                                  3.25,
                                                  3.50,
                                                  3.75,
                                                  4.00,
                                                  4.25,
                                                  4.50,
                                                  4.75,
                                                  5.00 };
    
    // Discount Factors
    const std::vector< double > discountFactors = { 1.000893,
                                                    1.001803,
                                                    1.002716,
                                                    1.003566,
                                                    1.004318,
                                                    1.005071,
                                                    1.005640,
                                                    1.006185,
                                                    1.006446,
                                                    1.006703,
                                                    1.006650,
                                                    1.006593,
                                                    1.006013,
                                                    1.005441,
                                                    1.004863,
                                                    1.004279,
                                                    1.003014,
                                                    1.001729,
                                                    1.000432,
                                                    0.999136 };

    // Unnatural Libor Convexity Adjustments: Compare Boundary Condition when Libor In-Arbitrary Time EQUALS Libor In-Arrears
    ASSERT_EQ( liborRates.size(), maturityTimes.size() );
    ASSERT_EQ( liborRates.size(), discountFactors.size() );

    for( unsigned int i = 0; i < liborRates.size(); ++i )
    {
        const double startDiscountFactor = ( i == 0 ) ? 1.0 : discountFactors[i-1];
        const double fixingYearFraction = couponYearFraction;

        const double liborInArrearsAdjustedRate
            = normalConvexityModel.liborRateInArrears( liborRates[i],
                                                       couponYearFraction,
                                                       maturityTimes[i] );

        const double liborInArbitraryTimeAdjustedRate
            = normalConvexityModel.liborRateForArbitraryFixingDate( liborRates[i],
                                                                    couponYearFraction,
                                                                    maturityTimes[i],
                                                                    fixingYearFraction,
                                                                    startDiscountFactor,
                                                                    discountFactors[i] );
        
        EXPECT_EQ( liborInArrearsAdjustedRate, liborInArbitraryTimeAdjustedRate );
    }
}


TEST( aqMathLiborConvexityAdjustmentForArbitraryFixingDate, CONSISTENCY_CheckBoundaryCondition_WhenArbitraryTimeEqualsLiborInArrears_USD_LognormalVol )
{
    // Lognormal Volatility Parameters
    const double logNormalVol = 0.3150; // 31.50%
    etrading::VolatilityTypeEnum LOGNORMALVOL = etrading::LOGNORMAL_VOLATILITY;
    etrading::Convexity::VolatilityParameters lognormalVolParameters( logNormalVol, LOGNORMALVOL );
    
    // Build Convexity Model
    etrading::ConvexityModel lognormalConvexityModel( lognormalVolParameters );

    // Coupon Daycount YearFraction
    const double couponYearFraction = 0.25;

    // Natural Libor Rates
    const std::vector< double > liborRates = { 0.02696010, 
                                               0.02726080,
                                               0.02713070,
                                               0.02695570,
                                               0.02618070,
                                               0.02600710,
                                               0.02619660,
                                               0.02579930,
                                               0.02540090,
                                               0.02518040,
                                               0.02513720,
                                               0.02524530,
                                               0.02540040,
                                               0.02557650,
                                               0.02577210,
                                               0.02597640,
                                               0.02616880,
                                               0.02634280,
                                               0.02649610,
                                               0.02649610 };
    
    // Coupon Time to Maturity
    const std::vector< double > maturityTimes = { 0.25,
                                                  0.50,
                                                  0.75,
                                                  1.00,
                                                  1.25,
                                                  1.50,
                                                  1.75,
                                                  2.00,
                                                  2.25,
                                                  2.50,
                                                  2.75,
                                                  3.00,
                                                  3.25,
                                                  3.50,
                                                  3.75,
                                                  4.00,
                                                  4.25,
                                                  4.50,
                                                  4.75,
                                                  5.00 };
    
    // Discount Factors
    const std::vector< double > discountFactors = { 0.994283,
                                                    0.988203,
                                                    0.982065,
                                                    0.975651,
                                                    0.969829,
                                                    0.964143,
                                                    0.958565,
                                                    0.953024,
                                                    0.947701,
                                                    0.942398,
                                                    0.937149,
                                                    0.931870,
                                                    0.926655,
                                                    0.921498,
                                                    0.916229,
                                                    0.910962,
                                                    0.905810,
                                                    0.900596,
                                                    0.895313,
                                                    0.890011 };

    // Unnatural Libor Convexity Adjustments: Compare Boundary Condition when Libor In-Arbitrary Time EQUALS Libor In-Arrears
    ASSERT_EQ( liborRates.size(), maturityTimes.size() );
    ASSERT_EQ( liborRates.size(), discountFactors.size() );

    for( unsigned int i = 0; i < liborRates.size(); ++i )
    {
        const double startDiscountFactor = ( i == 0 ) ? 1.0 : discountFactors[i-1];
        const double fixingYearFraction = couponYearFraction;

        const double liborInArrearsConvexityCorrection
            = lognormalConvexityModel.liborConvexityAdjustmentInArrears( liborRates[i] ,
                                                                         couponYearFraction,
                                                                         maturityTimes[i] );

        const double liborInArbitraryTimeConvexityCorrection
            = lognormalConvexityModel.liborConvexityAdjustmentForArbitraryFixingDate( liborRates[i],
                                                                                      couponYearFraction,
                                                                                      maturityTimes[i],
                                                                                      fixingYearFraction,
                                                                                      startDiscountFactor,
                                                                                      discountFactors[i] );
        
        EXPECT_EQ( liborInArrearsConvexityCorrection, liborInArbitraryTimeConvexityCorrection );
    }
}


TEST( aqMathLiborRateForArbitraryFixingDate, CONSISTENCY_CheckBoundaryCondition_WhenArbitraryTimeEqualsLiborInArrears_USD_LognormalVol )
{
    // Lognormal Volatility Parameters
    const double logNormalVol = 0.3150; // 31.50%
    etrading::VolatilityTypeEnum LOGNORMALVOL = etrading::LOGNORMAL_VOLATILITY;
    etrading::Convexity::VolatilityParameters lognormalVolParameters( logNormalVol, LOGNORMALVOL );
    
    // Build Convexity Model
    etrading::ConvexityModel lognormalConvexityModel( lognormalVolParameters );

    // Coupon Daycount YearFraction
    const double couponYearFraction = 0.25;

    // Natural Libor Rates
    const std::vector< double > liborRates = { 0.02696010, 
                                               0.02726080,
                                               0.02713070,
                                               0.02695570,
                                               0.02618070,
                                               0.02600710,
                                               0.02619660,
                                               0.02579930,
                                               0.02540090,
                                               0.02518040,
                                               0.02513720,
                                               0.02524530,
                                               0.02540040,
                                               0.02557650,
                                               0.02577210,
                                               0.02597640,
                                               0.02616880,
                                               0.02634280,
                                               0.02649610,
                                               0.02649610 };
    
    // Coupon Time to Maturity
    const std::vector< double > maturityTimes = { 0.25,
                                                  0.50,
                                                  0.75,
                                                  1.00,
                                                  1.25,
                                                  1.50,
                                                  1.75,
                                                  2.00,
                                                  2.25,
                                                  2.50,
                                                  2.75,
                                                  3.00,
                                                  3.25,
                                                  3.50,
                                                  3.75,
                                                  4.00,
                                                  4.25,
                                                  4.50,
                                                  4.75,
                                                  5.00 };
    
    // Discount Factors
    const std::vector< double > discountFactors = { 0.994283,
                                                    0.988203,
                                                    0.982065,
                                                    0.975651,
                                                    0.969829,
                                                    0.964143,
                                                    0.958565,
                                                    0.953024,
                                                    0.947701,
                                                    0.942398,
                                                    0.937149,
                                                    0.931870,
                                                    0.926655,
                                                    0.921498,
                                                    0.916229,
                                                    0.910962,
                                                    0.905810,
                                                    0.900596,
                                                    0.895313,
                                                    0.890011 };

    // Unnatural Libor Convexity Adjustments: Compare Boundary Condition when Libor In-Arbitrary Time EQUALS Libor In-Arrears
    ASSERT_EQ( liborRates.size(), maturityTimes.size() );
    ASSERT_EQ( liborRates.size(), discountFactors.size() );

    for( unsigned int i = 0; i < liborRates.size(); ++i )
    {
        const double startDiscountFactor = ( i == 0 ) ? 1.0 : discountFactors[i-1];
        const double fixingYearFraction = couponYearFraction;

        const double liborInArrearsAdjustedRate
            = lognormalConvexityModel.liborRateInArrears( liborRates[i] ,
                                                          couponYearFraction,
                                                          maturityTimes[i] );

        const double liborInArbitraryTimeAdjustedRate
            = lognormalConvexityModel.liborRateForArbitraryFixingDate( liborRates[i],
                                                                       couponYearFraction,
                                                                       maturityTimes[i],
                                                                       fixingYearFraction,
                                                                       startDiscountFactor,
                                                                       discountFactors[i] );
        
        EXPECT_EQ( liborInArrearsAdjustedRate, liborInArbitraryTimeAdjustedRate );
    }
}


TEST( aqMathLiborConvexityAdjustmentForArbitraryFixingDate, UNIT_UnnaturalLibor_ConvexityAdjustmentsLiborInArrears_EUR_ShiftedLognormalVol )
{
    // Shifted-LogNormal Volatility Parameters
    const double shiftedLognormalVol = 0.1100;  // 11.00 %
    const double shiftSize = 0.01;              // Volatility Shift 1.00%
    etrading::VolatilityTypeEnum SHIFTEDLOGNORMALVOL = etrading::SHIFTED_LOGNORMAL_VOLATILITY;
    etrading::Convexity::VolatilityParameters shiftedLognormalVolParameters( shiftedLognormalVol, SHIFTEDLOGNORMALVOL, shiftSize );
    
    // Build Convexity Model
    etrading::ConvexityModel shiftedLognormalConvexityModel( shiftedLognormalVolParameters );

    // Coupon Daycount YearFraction
    const double couponYearFraction = 0.25;

    // Natural Libor Rates
    const std::vector< double > liborRates = { -0.0029524, 
                                               -0.0029062,
                                               -0.0026462,
                                               -0.0023827,
                                               -0.0020566,
                                               -0.0016288,
                                               -0.0010828,
                                               -0.0005097,
                                                0.0000343,
                                                0.0006654,
                                                0.0013865,
                                                0.0021785,
                                                0.0029580,
                                                0.0036908,
                                                0.0043894,
                                                0.0050609,
                                                0.0057476,
                                                0.0064727,
                                                0.0072329,
                                                0.0080056 };
    
    // Coupon Time to Maturity
    const std::vector< double > maturityTimes = { 0.25,
                                                  0.50,
                                                  0.75,
                                                  1.00,
                                                  1.25,
                                                  1.50,
                                                  1.75,
                                                  2.00,
                                                  2.25,
                                                  2.50,
                                                  2.75,
                                                  3.00,
                                                  3.25,
                                                  3.50,
                                                  3.75,
                                                  4.00,
                                                  4.25,
                                                  4.50,
                                                  4.75,
                                                  5.00 };

    // Expected Results - ConvexityAdjustments
    const std::vector< double > expectedConvexityAdjustments = { 0.000000038060,
                                                                 0.000000076733,
                                                                 0.000000123658,
                                                                 0.000000177009,
                                                                 0.000000240852,
                                                                 0.000000321386,
                                                                 0.000000426036,
                                                                 0.000000552257,
                                                                 0.000000695506,
                                                                 0.000000874275,
                                                                 0.000001097706,
                                                                 0.000001371841,
                                                                 0.000001684874,
                                                                 0.000002028324,
                                                                 0.000002403954,
                                                                 0.000002812976,
                                                                 0.000003272026,
                                                                 0.000003796095,
                                                                 0.000004391384,
                                                                 0.000005053255 };

    // Test and Compare Expected Convexity Adjustments
    ASSERT_EQ( liborRates.size(), maturityTimes.size() );
    for( unsigned int i = 0; i < liborRates.size(); ++i )
    {
        const double liborConvexityAdjustmentInArrears = shiftedLognormalConvexityModel.liborConvexityAdjustmentInArrears(liborRates[i], couponYearFraction, maturityTimes[i]);
        EXPECT_NEAR( liborConvexityAdjustmentInArrears, expectedConvexityAdjustments[i], testPrecision );
    }
}


TEST( aqMathLiborConvexityAdjustmentForArbitraryFixingDate, UNIT_UnaturalLibor_Rates_LiborInArrears_EUR_ShiftedLognormalVol )
{
    // Shifted-LogNormal Volatility Parameters
    const double shiftedLognormalVol = 0.1100;  // 11.00 %
    const double shiftSize = 0.01;              // Volatility Shift 1.00%
    etrading::VolatilityTypeEnum SHIFTEDLOGNORMALVOL = etrading::SHIFTED_LOGNORMAL_VOLATILITY;
    etrading::Convexity::VolatilityParameters shiftedLognormalVolParameters( shiftedLognormalVol, SHIFTEDLOGNORMALVOL, shiftSize );
    
    // Build Convexity Model
    etrading::ConvexityModel shiftedLognormalConvexityModel( shiftedLognormalVolParameters );

    // Coupon Daycount YearFraction
    const double couponYearFraction = 0.25;

    // Natural Libor Rates
    const std::vector< double > liborRates = { -0.0029524, 
                                               -0.0029062,
                                               -0.0026462,
                                               -0.0023827,
                                               -0.0020566,
                                               -0.0016288,
                                               -0.0010828,
                                               -0.0005097,
                                                0.0000343,
                                                0.0006654,
                                                0.0013865,
                                                0.0021785,
                                                0.0029580,
                                                0.0036908,
                                                0.0043894,
                                                0.0050609,
                                                0.0057476,
                                                0.0064727,
                                                0.0072329,
                                                0.0080056 };
    
    // Coupon Time to Maturity
    const std::vector< double > maturityTimes = { 0.25,
                                                  0.50,
                                                  0.75,
                                                  1.00,
                                                  1.25,
                                                  1.50,
                                                  1.75,
                                                  2.00,
                                                  2.25,
                                                  2.50,
                                                  2.75,
                                                  3.00,
                                                  3.25,
                                                  3.50,
                                                  3.75,
                                                  4.00,
                                                  4.25,
                                                  4.50,
                                                  4.75,
                                                  5.00 };

    // Expected Results - ConvexityAdjustedRates
    const std::vector< double > expectedConvexityAdjustedRates = { -0.002952361940,
                                                                   -0.002906123267,
                                                                   -0.002646076342,
                                                                   -0.002382522991,
                                                                   -0.002056359148,
                                                                   -0.001628478614,
                                                                   -0.001082373964,
                                                                   -0.000509147743,
                                                                    0.000034995506,
                                                                    0.000666274275,
                                                                    0.001387597706,
                                                                    0.002179871841,
                                                                    0.002959684874,
                                                                    0.003692828324,
                                                                    0.004391803954,
                                                                    0.005063712976,
                                                                    0.005750872026,
                                                                    0.006476496095,
                                                                    0.007237291384,
                                                                    0.008010653255 };
    
    // Test and Compare Expected Convexity Adjustments
    ASSERT_EQ( liborRates.size(), maturityTimes.size() );
    for( unsigned int i = 0; i < liborRates.size(); ++i )
    {
        const double liborRateInArrears = shiftedLognormalConvexityModel.liborRateInArrears( liborRates[i], couponYearFraction, maturityTimes[i] );
        EXPECT_NEAR( liborRateInArrears, expectedConvexityAdjustedRates[i], testPrecision );
    }
}


TEST( aqMathLiborConvexityAdjustmentForArbitraryFixingDate, CONSISTENCY_CheckBoundaryCondition_WhenArbitraryTimeEqualsLiborInArrears_EUR_ShiftedLognormalVol )
{
    // Shifted-LogNormal Volatility Parameters
    const double shiftedLognormalVol = 0.1100;  // 11.00 %
    const double shiftSize = 0.01;              // Volatility Shift 1.00%
    etrading::VolatilityTypeEnum SHIFTEDLOGNORMALVOL = etrading::SHIFTED_LOGNORMAL_VOLATILITY;
    etrading::Convexity::VolatilityParameters shiftedLognormalVolParameters( shiftedLognormalVol, SHIFTEDLOGNORMALVOL, shiftSize );
    
    // Build Convexity Model
    etrading::ConvexityModel shiftedLognormalConvexityModel( shiftedLognormalVolParameters );

    // Coupon Daycount YearFraction
    const double couponYearFraction = 0.25;

    // Natural Libor Rates
    const std::vector< double > liborRates = { -0.0029524, 
                                               -0.0029062,
                                               -0.0026462,
                                               -0.0023827,
                                               -0.0020566,
                                               -0.0016288,
                                               -0.0010828,
                                               -0.0005097,
                                                0.0000343,
                                                0.0006654,
                                                0.0013865,
                                                0.0021785,
                                                0.0029580,
                                                0.0036908,
                                                0.0043894,
                                                0.0050609,
                                                0.0057476,
                                                0.0064727,
                                                0.0072329,
                                                0.0080056 };
    
    // Coupon Time to Maturity
    const std::vector< double > maturityTimes = { 0.25,
                                                  0.50,
                                                  0.75,
                                                  1.00,
                                                  1.25,
                                                  1.50,
                                                  1.75,
                                                  2.00,
                                                  2.25,
                                                  2.50,
                                                  2.75,
                                                  3.00,
                                                  3.25,
                                                  3.50,
                                                  3.75,
                                                  4.00,
                                                  4.25,
                                                  4.50,
                                                  4.75,
                                                  5.00 };

    // Discount Factors
    const std::vector< double > discountFactors = { 1.000893,
                                                    1.001803,
                                                    1.002716,
                                                    1.003566,
                                                    1.004318,
                                                    1.005071,
                                                    1.005640,
                                                    1.006185,
                                                    1.006446,
                                                    1.006703,
                                                    1.006650,
                                                    1.006593,
                                                    1.006013,
                                                    1.005441,
                                                    1.004863,
                                                    1.004279,
                                                    1.003014,
                                                    1.001729,
                                                    1.000432,
                                                    0.999136 };

    // Consistency Test: Compare Boundary Condition when Libor In-Arbitrary Time EQUALS Libor In-Arrears
    ASSERT_EQ( liborRates.size(), maturityTimes.size() );
    ASSERT_EQ( liborRates.size(), discountFactors.size() );
    for( unsigned int i = 0; i < liborRates.size(); ++i )
    {
        const double startDiscountFactor = ( i == 0 ) ? 1.0 : discountFactors[i-1];
        const double fixingYearFraction = couponYearFraction;

        const double liborInArrearsConvexityCorrection
            = shiftedLognormalConvexityModel.liborConvexityAdjustmentInArrears( liborRates[i], couponYearFraction, maturityTimes[i] );

        const double liborInArbitraryTimeConvexityCorrection
            = shiftedLognormalConvexityModel.liborConvexityAdjustmentForArbitraryFixingDate( liborRates[i],
                                                                                             couponYearFraction,
                                                                                             maturityTimes[i],
                                                                                             fixingYearFraction,
                                                                                             startDiscountFactor,
                                                                                             discountFactors[i] );
        
        EXPECT_EQ( liborInArrearsConvexityCorrection, liborInArbitraryTimeConvexityCorrection );
    }
}


TEST( aqMathLiborConvexityAdjustmentForArbitraryFixingDate, CONSISTENCY_CheckBoundaryCondition_WhenArbitraryTimeEqualsNaturalLibor_EUR_NormalVol )
{
    // Normal Volatility Parameters
    const double normalVol = 0.0020; // 20 bps
    etrading::VolatilityTypeEnum NORMALVOL = etrading::NORMAL_VOLATILITY;
    etrading::Convexity::VolatilityParameters normalVolParameters( normalVol, NORMALVOL );
    
    // Build Convexity Model
    etrading::ConvexityModel normalConvexityModel( normalVolParameters );

    // Coupon Daycount YearFraction
    const double couponYearFraction = 0.25;

    // Natural Libor Rates
    const std::vector< double > liborRates = { -0.0029524, 
                                               -0.0029062,
                                               -0.0026462,
                                               -0.0023827,
                                               -0.0020566,
                                               -0.0016288,
                                               -0.0010828,
                                               -0.0005097,
                                                0.0000343,
                                                0.0006654,
                                                0.0013865,
                                                0.0021785,
                                                0.0029580,
                                                0.0036908,
                                                0.0043894,
                                                0.0050609,
                                                0.0057476,
                                                0.0064727,
                                                0.0072329,
                                                0.0080056 };
    
    // Coupon Time to Maturity
    const std::vector< double > maturityTimes = { 0.25,
                                                  0.50,
                                                  0.75,
                                                  1.00,
                                                  1.25,
                                                  1.50,
                                                  1.75,
                                                  2.00,
                                                  2.25,
                                                  2.50,
                                                  2.75,
                                                  3.00,
                                                  3.25,
                                                  3.50,
                                                  3.75,
                                                  4.00,
                                                  4.25,
                                                  4.50,
                                                  4.75,
                                                  5.00 };
    
    // Discount Factors
    const std::vector< double > discountFactors = { 1.000893,
                                                    1.001803,
                                                    1.002716,
                                                    1.003566,
                                                    1.004318,
                                                    1.005071,
                                                    1.005640,
                                                    1.006185,
                                                    1.006446,
                                                    1.006703,
                                                    1.006650,
                                                    1.006593,
                                                    1.006013,
                                                    1.005441,
                                                    1.004863,
                                                    1.004279,
                                                    1.003014,
                                                    1.001729,
                                                    1.000432,
                                                    0.999136 };

    // Unnatural Libor Convexity Adjustments: Compare Boundary Condition when Libor In-Arbitrary Time EQUALS Libor In-Arrears
    ASSERT_EQ( liborRates.size(), maturityTimes.size() );
    for( unsigned int i = 0; i < liborRates.size(); ++i )
    {
        const double startDiscountFactor = ( i == 0 ) ? 1.0 : discountFactors[i-1];
        const double fixingYearFraction = 0.0; // Zero Fixing Year Fraction implies Natural Libor with no adjustment required

        const double liborInArbitraryTimeConvexityCorrection
            = normalConvexityModel.liborConvexityAdjustmentForArbitraryFixingDate( liborRates[i],
                                                                                   couponYearFraction,
                                                                                   maturityTimes[i],
                                                                                   fixingYearFraction, // Zero Fixing Year Fraction implies Natural Libor with no adjustment required
                                                                                   startDiscountFactor,
                                                                                   discountFactors[i] );
        
        EXPECT_EQ( liborInArbitraryTimeConvexityCorrection, 0.0 );
    }
}


TEST( aqMathLiborRateWithArbitraryFixingDate, CONSISTENCY_CheckBoundaryCondition_WhenArbitraryTimeEqualsNaturalLibor_EUR_NormalVol )
{
    // Normal Volatility Parameters
    const double normalVol = 0.0020; // 20 bps
    etrading::VolatilityTypeEnum NORMALVOL = etrading::NORMAL_VOLATILITY;
    etrading::Convexity::VolatilityParameters normalVolParameters( normalVol, NORMALVOL );
    
    // Build Convexity Model
    etrading::ConvexityModel normalConvexityModel( normalVolParameters );

    // Coupon Daycount YearFraction
    const double couponYearFraction = 0.25;

    // Natural Libor Rates
    const std::vector< double > liborRates = { -0.0029524, 
                                               -0.0029062,
                                               -0.0026462,
                                               -0.0023827,
                                               -0.0020566,
                                               -0.0016288,
                                               -0.0010828,
                                               -0.0005097,
                                                0.0000343,
                                                0.0006654,
                                                0.0013865,
                                                0.0021785,
                                                0.0029580,
                                                0.0036908,
                                                0.0043894,
                                                0.0050609,
                                                0.0057476,
                                                0.0064727,
                                                0.0072329,
                                                0.0080056 };
    
    // Coupon Time to Maturity
    const std::vector< double > maturityTimes = { 0.25,
                                                  0.50,
                                                  0.75,
                                                  1.00,
                                                  1.25,
                                                  1.50,
                                                  1.75,
                                                  2.00,
                                                  2.25,
                                                  2.50,
                                                  2.75,
                                                  3.00,
                                                  3.25,
                                                  3.50,
                                                  3.75,
                                                  4.00,
                                                  4.25,
                                                  4.50,
                                                  4.75,
                                                  5.00 };
    
    // Discount Factors
    const std::vector< double > discountFactors = { 1.000893,
                                                    1.001803,
                                                    1.002716,
                                                    1.003566,
                                                    1.004318,
                                                    1.005071,
                                                    1.005640,
                                                    1.006185,
                                                    1.006446,
                                                    1.006703,
                                                    1.006650,
                                                    1.006593,
                                                    1.006013,
                                                    1.005441,
                                                    1.004863,
                                                    1.004279,
                                                    1.003014,
                                                    1.001729,
                                                    1.000432,
                                                    0.999136 };

    // Unnatural Libor Convexity Adjustments: Compare Boundary Condition when Libor In-Arbitrary Time EQUALS Libor In-Arrears
    ASSERT_EQ( liborRates.size(), maturityTimes.size() );
    ASSERT_EQ( liborRates.size(), discountFactors.size() );

    for( unsigned int i = 0; i < liborRates.size(); ++i )
    {
        const double startDiscountFactor = ( i == 0 ) ? 1.0 : discountFactors[i-1];
        const double fixingYearFraction = 0.0; // Zero Fixing Year Fraction implies Natural Libor with no adjustment required

        const double liborRateForArbitraryTime
            = normalConvexityModel.liborRateForArbitraryFixingDate( liborRates[i],
                                                                    couponYearFraction,
                                                                    maturityTimes[i],
                                                                    fixingYearFraction, // Zero Fixing Year Fraction implies Natural Libor with no adjustment required
                                                                    startDiscountFactor,
                                                                    discountFactors[i] );
        
        EXPECT_EQ( liborRateForArbitraryTime, liborRates[i] );
    }
}


TEST( aqMathLiborConvexityAdjustmentForArbitraryFixingDate, CONSISTENCY_CheckBoundaryCondition_WhenArbitraryTimeEqualsNaturalLibor_USD_LognormalVol )
{
    // Lognormal Volatility Parameters
    const double logNormalVol = 0.3150; // 31.50%
    etrading::VolatilityTypeEnum LOGNORMALVOL = etrading::LOGNORMAL_VOLATILITY;
    etrading::Convexity::VolatilityParameters lognormalVolParameters( logNormalVol, LOGNORMALVOL );
    
    // Build Convexity Model
    etrading::ConvexityModel lognormalConvexityModel( lognormalVolParameters );

    // Coupon Daycount YearFraction
    const double couponYearFraction = 0.25;

    // Natural Libor Rates
    const std::vector< double > liborRates = { 0.02696010, 
                                               0.02726080,
                                               0.02713070,
                                               0.02695570,
                                               0.02618070,
                                               0.02600710,
                                               0.02619660,
                                               0.02579930,
                                               0.02540090,
                                               0.02518040,
                                               0.02513720,
                                               0.02524530,
                                               0.02540040,
                                               0.02557650,
                                               0.02577210,
                                               0.02597640,
                                               0.02616880,
                                               0.02634280,
                                               0.02649610,
                                               0.02649610 };
    
    // Coupon Time to Maturity
    const std::vector< double > maturityTimes = { 0.25,
                                                  0.50,
                                                  0.75,
                                                  1.00,
                                                  1.25,
                                                  1.50,
                                                  1.75,
                                                  2.00,
                                                  2.25,
                                                  2.50,
                                                  2.75,
                                                  3.00,
                                                  3.25,
                                                  3.50,
                                                  3.75,
                                                  4.00,
                                                  4.25,
                                                  4.50,
                                                  4.75,
                                                  5.00 };
    
    // Discount Factors
    const std::vector< double > discountFactors = { 0.994283,
                                                    0.988203,
                                                    0.982065,
                                                    0.975651,
                                                    0.969829,
                                                    0.964143,
                                                    0.958565,
                                                    0.953024,
                                                    0.947701,
                                                    0.942398,
                                                    0.937149,
                                                    0.931870,
                                                    0.926655,
                                                    0.921498,
                                                    0.916229,
                                                    0.910962,
                                                    0.905810,
                                                    0.900596,
                                                    0.895313,
                                                    0.890011 };

    // Unnatural Libor Convexity Adjustments: Compare Boundary Condition when Libor In-Arbitrary Time EQUALS Libor In-Arrears
    ASSERT_EQ( liborRates.size(), maturityTimes.size() );
    ASSERT_EQ( liborRates.size(), discountFactors.size() );

    for( unsigned int i = 0; i < liborRates.size(); ++i )
    {
        const double startDiscountFactor = ( i == 0 ) ? 1.0 : discountFactors[i-1];
        const double fixingYearFraction = 0.0; // Zero Fixing Year Fraction implies Natural Libor with no adjustment required

        const double liborInArbitraryTimeConvexityCorrection
            = lognormalConvexityModel.liborConvexityAdjustmentForArbitraryFixingDate( liborRates[i],
                                                                                      couponYearFraction,
                                                                                      maturityTimes[i],
                                                                                      fixingYearFraction, // Zero Fixing Year Fraction implies Natural Libor with no adjustment required
                                                                                      startDiscountFactor,
                                                                                      discountFactors[i] );
        
        EXPECT_EQ( liborInArbitraryTimeConvexityCorrection, 0.0 );
    }
}


TEST( aqMathLiborRateWithArbitraryFixingDate, CONSISTENCY_CheckBoundaryCondition_WhenArbitraryTimeEqualsNaturalLibor_USD_LognormalVol )
{
    // Lognormal Volatility Parameters
    const double logNormalVol = 0.3150; // 31.50%
    etrading::VolatilityTypeEnum LOGNORMALVOL = etrading::LOGNORMAL_VOLATILITY;
    etrading::Convexity::VolatilityParameters lognormalVolParameters( logNormalVol, LOGNORMALVOL );
    
    // Build Convexity Model
    etrading::ConvexityModel lognormalConvexityModel( lognormalVolParameters );

    // Coupon Daycount YearFraction
    const double couponYearFraction = 0.25;

    // Natural Libor Rates
    const std::vector< double > liborRates = { 0.02696010, 
                                               0.02726080,
                                               0.02713070,
                                               0.02695570,
                                               0.02618070,
                                               0.02600710,
                                               0.02619660,
                                               0.02579930,
                                               0.02540090,
                                               0.02518040,
                                               0.02513720,
                                               0.02524530,
                                               0.02540040,
                                               0.02557650,
                                               0.02577210,
                                               0.02597640,
                                               0.02616880,
                                               0.02634280,
                                               0.02649610,
                                               0.02649610 };
    
    // Coupon Time to Maturity
    const std::vector< double > maturityTimes = { 0.25,
                                                  0.50,
                                                  0.75,
                                                  1.00,
                                                  1.25,
                                                  1.50,
                                                  1.75,
                                                  2.00,
                                                  2.25,
                                                  2.50,
                                                  2.75,
                                                  3.00,
                                                  3.25,
                                                  3.50,
                                                  3.75,
                                                  4.00,
                                                  4.25,
                                                  4.50,
                                                  4.75,
                                                  5.00 };
    
    // Discount Factors
    const std::vector< double > discountFactors = { 0.994283,
                                                    0.988203,
                                                    0.982065,
                                                    0.975651,
                                                    0.969829,
                                                    0.964143,
                                                    0.958565,
                                                    0.953024,
                                                    0.947701,
                                                    0.942398,
                                                    0.937149,
                                                    0.931870,
                                                    0.926655,
                                                    0.921498,
                                                    0.916229,
                                                    0.910962,
                                                    0.905810,
                                                    0.900596,
                                                    0.895313,
                                                    0.890011 };

    // Unnatural Libor Convexity Adjustments: Compare Boundary Condition when Libor In-Arbitrary Time EQUALS Libor In-Arrears
    ASSERT_EQ( liborRates.size(), maturityTimes.size() );
    ASSERT_EQ( liborRates.size(), discountFactors.size() );

    for( unsigned int i = 0; i < liborRates.size(); ++i )
    {
        const double startDiscountFactor = ( i == 0 ) ? 1.0 : discountFactors[i-1];
        const double fixingYearFraction = 0.0; // Zero Fixing Year Fraction implies Natural Libor with no adjustment required

        const double liborRateForArbitraryTime
            = lognormalConvexityModel.liborRateForArbitraryFixingDate( liborRates[i],
                                                                       couponYearFraction,
                                                                       maturityTimes[i],
                                                                       fixingYearFraction, // Zero Fixing Year Fraction implies Natural Libor with no adjustment required
                                                                       startDiscountFactor,
                                                                       discountFactors[i] );
        
        EXPECT_EQ( liborRateForArbitraryTime, liborRates[i] );
    }
}

