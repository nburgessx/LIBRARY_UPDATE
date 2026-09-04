// TestAqMathConvexity.cpp
#include "ConvexityModel.h"
#include "tryAqMathConvexity.h"

// Include: Google Test Library
#include <gTest/gTest.h>


const double testPrecision = 1e-8;


// Test Cases
// ------------------------------

TEST( TestAqMathVolatility, UNIT_ConvertVol_Roundtrip_ToFrom_Normal_Lognormal )
{
    // Lognormal Volatility 31.50%
    const double lognormalVol = 0.3150;
    
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

    // Convert from Lognormal to Normal
    const double normalVol1  = etrading::Convexity::toNormalVolFromLognormalVol( lognormalVol, liborRate1  );
    const double normalVol2  = etrading::Convexity::toNormalVolFromLognormalVol( lognormalVol, liborRate2  ); 
    const double normalVol3  = etrading::Convexity::toNormalVolFromLognormalVol( lognormalVol, liborRate3  );
    const double normalVol4  = etrading::Convexity::toNormalVolFromLognormalVol( lognormalVol, liborRate4  );
    const double normalVol5  = etrading::Convexity::toNormalVolFromLognormalVol( lognormalVol, liborRate5  );
    const double normalVol6  = etrading::Convexity::toNormalVolFromLognormalVol( lognormalVol, liborRate6  );
    const double normalVol7  = etrading::Convexity::toNormalVolFromLognormalVol( lognormalVol, liborRate7  );
    const double normalVol8  = etrading::Convexity::toNormalVolFromLognormalVol( lognormalVol, liborRate8  );
    const double normalVol9  = etrading::Convexity::toNormalVolFromLognormalVol( lognormalVol, liborRate9  );
    const double normalVol10 = etrading::Convexity::toNormalVolFromLognormalVol( lognormalVol, liborRate10 );
    const double normalVol11 = etrading::Convexity::toNormalVolFromLognormalVol( lognormalVol, liborRate11 );
    const double normalVol12 = etrading::Convexity::toNormalVolFromLognormalVol( lognormalVol, liborRate12 );
    const double normalVol13 = etrading::Convexity::toNormalVolFromLognormalVol( lognormalVol, liborRate13 );
    const double normalVol14 = etrading::Convexity::toNormalVolFromLognormalVol( lognormalVol, liborRate14 );
    const double normalVol15 = etrading::Convexity::toNormalVolFromLognormalVol( lognormalVol, liborRate15 );
    const double normalVol16 = etrading::Convexity::toNormalVolFromLognormalVol( lognormalVol, liborRate16 );
    const double normalVol17 = etrading::Convexity::toNormalVolFromLognormalVol( lognormalVol, liborRate17 );
    const double normalVol18 = etrading::Convexity::toNormalVolFromLognormalVol( lognormalVol, liborRate18 );
    const double normalVol19 = etrading::Convexity::toNormalVolFromLognormalVol( lognormalVol, liborRate19 );
    const double normalVol20 = etrading::Convexity::toNormalVolFromLognormalVol( lognormalVol, liborRate20 );

    // Expected Results
    const double expectedResult1  = 0.0084924315;
    const double expectedResult2  = 0.0085871520;
    const double expectedResult3  = 0.0085461705;
    const double expectedResult4  = 0.0084910455;
    const double expectedResult5  = 0.0082469205;
    const double expectedResult6  = 0.0081922365;
    const double expectedResult7  = 0.0082519290;
    const double expectedResult8  = 0.0081267795;
    const double expectedResult9  = 0.0080012835;
    const double expectedResult10 = 0.0079318260;
    const double expectedResult11 = 0.0079182180;
    const double expectedResult12 = 0.0079522695;
    const double expectedResult13 = 0.0080011260;
    const double expectedResult14 = 0.0080565975;
    const double expectedResult15 = 0.0081182115;
    const double expectedResult16 = 0.0081825660;
    const double expectedResult17 = 0.0082431720;
    const double expectedResult18 = 0.0082979820;
    const double expectedResult19 = 0.0083462715;
    const double expectedResult20 = 0.0083462715;

	// Test and Compare Results
	// ---------------------
	EXPECT_NEAR( normalVol1 , expectedResult1 , testPrecision );
	EXPECT_NEAR( normalVol2 , expectedResult2 , testPrecision );
	EXPECT_NEAR( normalVol3 , expectedResult3 , testPrecision );
	EXPECT_NEAR( normalVol4 , expectedResult4 , testPrecision );
	EXPECT_NEAR( normalVol5 , expectedResult5 , testPrecision );
    EXPECT_NEAR( normalVol6 , expectedResult6 , testPrecision );
    EXPECT_NEAR( normalVol7 , expectedResult7 , testPrecision );
    EXPECT_NEAR( normalVol8 , expectedResult8 , testPrecision );
    EXPECT_NEAR( normalVol9 , expectedResult9 , testPrecision );
    EXPECT_NEAR( normalVol10, expectedResult10, testPrecision );
    EXPECT_NEAR( normalVol11, expectedResult11, testPrecision );
    EXPECT_NEAR( normalVol12, expectedResult12, testPrecision );
    EXPECT_NEAR( normalVol13, expectedResult13, testPrecision );
    EXPECT_NEAR( normalVol14, expectedResult14, testPrecision );
    EXPECT_NEAR( normalVol15, expectedResult15, testPrecision );
    EXPECT_NEAR( normalVol16, expectedResult16, testPrecision );
    EXPECT_NEAR( normalVol17, expectedResult17, testPrecision );
    EXPECT_NEAR( normalVol18, expectedResult18, testPrecision );
    EXPECT_NEAR( normalVol19, expectedResult19, testPrecision );
    EXPECT_NEAR( normalVol20, expectedResult20, testPrecision );

    // Convert back to Lognormal from Normal
    const double lognormalVol1  = etrading::Convexity::toLognormalVolFromNormalVol( normalVol1 , liborRate1  );
    const double lognormalVol2  = etrading::Convexity::toLognormalVolFromNormalVol( normalVol2 , liborRate2  ); 
    const double lognormalVol3  = etrading::Convexity::toLognormalVolFromNormalVol( normalVol3 , liborRate3  );
    const double lognormalVol4  = etrading::Convexity::toLognormalVolFromNormalVol( normalVol4 , liborRate4  );
    const double lognormalVol5  = etrading::Convexity::toLognormalVolFromNormalVol( normalVol5 , liborRate5  );
    const double lognormalVol6  = etrading::Convexity::toLognormalVolFromNormalVol( normalVol6 , liborRate6  );
    const double lognormalVol7  = etrading::Convexity::toLognormalVolFromNormalVol( normalVol7 , liborRate7  );
    const double lognormalVol8  = etrading::Convexity::toLognormalVolFromNormalVol( normalVol8 , liborRate8  );
    const double lognormalVol9  = etrading::Convexity::toLognormalVolFromNormalVol( normalVol9 , liborRate9  );
    const double lognormalVol10 = etrading::Convexity::toLognormalVolFromNormalVol( normalVol10, liborRate10 );
    const double lognormalVol11 = etrading::Convexity::toLognormalVolFromNormalVol( normalVol11, liborRate11 );
    const double lognormalVol12 = etrading::Convexity::toLognormalVolFromNormalVol( normalVol12, liborRate12 );
    const double lognormalVol13 = etrading::Convexity::toLognormalVolFromNormalVol( normalVol13, liborRate13 );
    const double lognormalVol14 = etrading::Convexity::toLognormalVolFromNormalVol( normalVol14, liborRate14 );
    const double lognormalVol15 = etrading::Convexity::toLognormalVolFromNormalVol( normalVol15, liborRate15 );
    const double lognormalVol16 = etrading::Convexity::toLognormalVolFromNormalVol( normalVol16, liborRate16 );
    const double lognormalVol17 = etrading::Convexity::toLognormalVolFromNormalVol( normalVol17, liborRate17 );
    const double lognormalVol18 = etrading::Convexity::toLognormalVolFromNormalVol( normalVol18, liborRate18 );
    const double lognormalVol19 = etrading::Convexity::toLognormalVolFromNormalVol( normalVol19, liborRate19 );
    const double lognormalVol20 = etrading::Convexity::toLognormalVolFromNormalVol( normalVol20, liborRate20 );

    // Test and Compare Results
	// ---------------------
    const double roundtripVol = lognormalVol;

	EXPECT_NEAR( lognormalVol1 , roundtripVol, testPrecision );
	EXPECT_NEAR( lognormalVol2 , roundtripVol, testPrecision );
	EXPECT_NEAR( lognormalVol3 , roundtripVol, testPrecision );
	EXPECT_NEAR( lognormalVol4 , roundtripVol, testPrecision );
	EXPECT_NEAR( lognormalVol5 , roundtripVol, testPrecision );
    EXPECT_NEAR( lognormalVol6 , roundtripVol, testPrecision );
    EXPECT_NEAR( lognormalVol7 , roundtripVol, testPrecision );
    EXPECT_NEAR( lognormalVol8 , roundtripVol, testPrecision );
    EXPECT_NEAR( lognormalVol9 , roundtripVol, testPrecision );
    EXPECT_NEAR( lognormalVol10, roundtripVol, testPrecision );
    EXPECT_NEAR( lognormalVol11, roundtripVol, testPrecision );
    EXPECT_NEAR( lognormalVol12, roundtripVol, testPrecision );
    EXPECT_NEAR( lognormalVol13, roundtripVol, testPrecision );
    EXPECT_NEAR( lognormalVol14, roundtripVol, testPrecision );
    EXPECT_NEAR( lognormalVol15, roundtripVol, testPrecision );
    EXPECT_NEAR( lognormalVol16, roundtripVol, testPrecision );
    EXPECT_NEAR( lognormalVol17, roundtripVol, testPrecision );
    EXPECT_NEAR( lognormalVol18, roundtripVol, testPrecision );
    EXPECT_NEAR( lognormalVol19, roundtripVol, testPrecision );
    EXPECT_NEAR( lognormalVol20, roundtripVol, testPrecision );
}

TEST( TestAqMathVolatility, UNIT_ConvertVol_Roundtrip_ToFrom_Lognormal_ShiftedLognormal )
{
    // Lognormal Volatility 31.50%
    const double lognormalVol = 0.3150;
    
    // Shifted-Lognormal Volatility ShiftSize 3%
    const double volatilityShiftSize = 0.03;

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

    // Convert from Lognormal to Normal
    const double shiftedLognormalVol1  = etrading::Convexity::toShiftedLognormalVolFromLognormalVol( lognormalVol, liborRate1 , volatilityShiftSize );
    const double shiftedLognormalVol2  = etrading::Convexity::toShiftedLognormalVolFromLognormalVol( lognormalVol, liborRate2 , volatilityShiftSize );
    const double shiftedLognormalVol3  = etrading::Convexity::toShiftedLognormalVolFromLognormalVol( lognormalVol, liborRate3 , volatilityShiftSize );
    const double shiftedLognormalVol4  = etrading::Convexity::toShiftedLognormalVolFromLognormalVol( lognormalVol, liborRate4 , volatilityShiftSize );
    const double shiftedLognormalVol5  = etrading::Convexity::toShiftedLognormalVolFromLognormalVol( lognormalVol, liborRate5 , volatilityShiftSize );
    const double shiftedLognormalVol6  = etrading::Convexity::toShiftedLognormalVolFromLognormalVol( lognormalVol, liborRate6 , volatilityShiftSize );
    const double shiftedLognormalVol7  = etrading::Convexity::toShiftedLognormalVolFromLognormalVol( lognormalVol, liborRate7 , volatilityShiftSize );
    const double shiftedLognormalVol8  = etrading::Convexity::toShiftedLognormalVolFromLognormalVol( lognormalVol, liborRate8 , volatilityShiftSize );
    const double shiftedLognormalVol9  = etrading::Convexity::toShiftedLognormalVolFromLognormalVol( lognormalVol, liborRate9 , volatilityShiftSize );
    const double shiftedLognormalVol10 = etrading::Convexity::toShiftedLognormalVolFromLognormalVol( lognormalVol, liborRate10, volatilityShiftSize );
    const double shiftedLognormalVol11 = etrading::Convexity::toShiftedLognormalVolFromLognormalVol( lognormalVol, liborRate11, volatilityShiftSize );
    const double shiftedLognormalVol12 = etrading::Convexity::toShiftedLognormalVolFromLognormalVol( lognormalVol, liborRate12, volatilityShiftSize );
    const double shiftedLognormalVol13 = etrading::Convexity::toShiftedLognormalVolFromLognormalVol( lognormalVol, liborRate13, volatilityShiftSize );
    const double shiftedLognormalVol14 = etrading::Convexity::toShiftedLognormalVolFromLognormalVol( lognormalVol, liborRate14, volatilityShiftSize );
    const double shiftedLognormalVol15 = etrading::Convexity::toShiftedLognormalVolFromLognormalVol( lognormalVol, liborRate15, volatilityShiftSize );
    const double shiftedLognormalVol16 = etrading::Convexity::toShiftedLognormalVolFromLognormalVol( lognormalVol, liborRate16, volatilityShiftSize );
    const double shiftedLognormalVol17 = etrading::Convexity::toShiftedLognormalVolFromLognormalVol( lognormalVol, liborRate17, volatilityShiftSize );
    const double shiftedLognormalVol18 = etrading::Convexity::toShiftedLognormalVolFromLognormalVol( lognormalVol, liborRate18, volatilityShiftSize );
    const double shiftedLognormalVol19 = etrading::Convexity::toShiftedLognormalVolFromLognormalVol( lognormalVol, liborRate19, volatilityShiftSize );
    const double shiftedLognormalVol20 = etrading::Convexity::toShiftedLognormalVolFromLognormalVol( lognormalVol, liborRate20, volatilityShiftSize );

    // Expected Results
    const double expectedResult1  = 0.1490943924;
    const double expectedResult2  = 0.1499656309;
    const double expectedResult3  = 0.1495898090;
    const double expectedResult4  = 0.1490815757;
    const double expectedResult5  = 0.1467927687;
    const double expectedResult6  = 0.1462713924;
    const double expectedResult7  = 0.1468403604;
    const double expectedResult8  = 0.1456430367;
    const double expectedResult9  = 0.1444251537;
    const double expectedResult10 = 0.1437435394;
    const double expectedResult11 = 0.1436093599;
    const double expectedResult12 = 0.1439447247;
    const double expectedResult13 = 0.1444236143;
    const double expectedResult14 = 0.1449641035;
    const double expectedResult15 = 0.1455604415;
    const double expectedResult16 = 0.1461788539;
    const double expectedResult17 = 0.1467571321;
    const double expectedResult18 = 0.1472767062;
    const double expectedResult19 = 0.1477318169;
    const double expectedResult20 = 0.1477318169;

	// Test and Compare Results
	// ---------------------
	EXPECT_NEAR( shiftedLognormalVol1 , expectedResult1 , testPrecision );
	EXPECT_NEAR( shiftedLognormalVol2 , expectedResult2 , testPrecision );
	EXPECT_NEAR( shiftedLognormalVol3 , expectedResult3 , testPrecision );
	EXPECT_NEAR( shiftedLognormalVol4 , expectedResult4 , testPrecision );
	EXPECT_NEAR( shiftedLognormalVol5 , expectedResult5 , testPrecision );
    EXPECT_NEAR( shiftedLognormalVol6 , expectedResult6 , testPrecision );
    EXPECT_NEAR( shiftedLognormalVol7 , expectedResult7 , testPrecision );
    EXPECT_NEAR( shiftedLognormalVol8 , expectedResult8 , testPrecision );
    EXPECT_NEAR( shiftedLognormalVol9 , expectedResult9 , testPrecision );
    EXPECT_NEAR( shiftedLognormalVol10, expectedResult10, testPrecision );
    EXPECT_NEAR( shiftedLognormalVol11, expectedResult11, testPrecision );
    EXPECT_NEAR( shiftedLognormalVol12, expectedResult12, testPrecision );
    EXPECT_NEAR( shiftedLognormalVol13, expectedResult13, testPrecision );
    EXPECT_NEAR( shiftedLognormalVol14, expectedResult14, testPrecision );
    EXPECT_NEAR( shiftedLognormalVol15, expectedResult15, testPrecision );
    EXPECT_NEAR( shiftedLognormalVol16, expectedResult16, testPrecision );
    EXPECT_NEAR( shiftedLognormalVol17, expectedResult17, testPrecision );
    EXPECT_NEAR( shiftedLognormalVol18, expectedResult18, testPrecision );
    EXPECT_NEAR( shiftedLognormalVol19, expectedResult19, testPrecision );
    EXPECT_NEAR( shiftedLognormalVol20, expectedResult20, testPrecision );

    // Convert back to Lognormal from Normal
    const double lognormalVol1  = etrading::Convexity::toLognormalVolFromShiftedLognormalVol( shiftedLognormalVol1 , liborRate1 , volatilityShiftSize );
    const double lognormalVol2  = etrading::Convexity::toLognormalVolFromShiftedLognormalVol( shiftedLognormalVol2 , liborRate2 , volatilityShiftSize ); 
    const double lognormalVol3  = etrading::Convexity::toLognormalVolFromShiftedLognormalVol( shiftedLognormalVol3 , liborRate3 , volatilityShiftSize );
    const double lognormalVol4  = etrading::Convexity::toLognormalVolFromShiftedLognormalVol( shiftedLognormalVol4 , liborRate4 , volatilityShiftSize );
    const double lognormalVol5  = etrading::Convexity::toLognormalVolFromShiftedLognormalVol( shiftedLognormalVol5 , liborRate5 , volatilityShiftSize );
    const double lognormalVol6  = etrading::Convexity::toLognormalVolFromShiftedLognormalVol( shiftedLognormalVol6 , liborRate6 , volatilityShiftSize );
    const double lognormalVol7  = etrading::Convexity::toLognormalVolFromShiftedLognormalVol( shiftedLognormalVol7 , liborRate7 , volatilityShiftSize );
    const double lognormalVol8  = etrading::Convexity::toLognormalVolFromShiftedLognormalVol( shiftedLognormalVol8 , liborRate8 , volatilityShiftSize );
    const double lognormalVol9  = etrading::Convexity::toLognormalVolFromShiftedLognormalVol( shiftedLognormalVol9 , liborRate9 , volatilityShiftSize );
    const double lognormalVol10 = etrading::Convexity::toLognormalVolFromShiftedLognormalVol( shiftedLognormalVol10, liborRate10, volatilityShiftSize );
    const double lognormalVol11 = etrading::Convexity::toLognormalVolFromShiftedLognormalVol( shiftedLognormalVol11, liborRate11, volatilityShiftSize );
    const double lognormalVol12 = etrading::Convexity::toLognormalVolFromShiftedLognormalVol( shiftedLognormalVol12, liborRate12, volatilityShiftSize );
    const double lognormalVol13 = etrading::Convexity::toLognormalVolFromShiftedLognormalVol( shiftedLognormalVol13, liborRate13, volatilityShiftSize );
    const double lognormalVol14 = etrading::Convexity::toLognormalVolFromShiftedLognormalVol( shiftedLognormalVol14, liborRate14, volatilityShiftSize );
    const double lognormalVol15 = etrading::Convexity::toLognormalVolFromShiftedLognormalVol( shiftedLognormalVol15, liborRate15, volatilityShiftSize );
    const double lognormalVol16 = etrading::Convexity::toLognormalVolFromShiftedLognormalVol( shiftedLognormalVol16, liborRate16, volatilityShiftSize );
    const double lognormalVol17 = etrading::Convexity::toLognormalVolFromShiftedLognormalVol( shiftedLognormalVol17, liborRate17, volatilityShiftSize );
    const double lognormalVol18 = etrading::Convexity::toLognormalVolFromShiftedLognormalVol( shiftedLognormalVol18, liborRate18, volatilityShiftSize );
    const double lognormalVol19 = etrading::Convexity::toLognormalVolFromShiftedLognormalVol( shiftedLognormalVol19, liborRate19, volatilityShiftSize );
    const double lognormalVol20 = etrading::Convexity::toLognormalVolFromShiftedLognormalVol( shiftedLognormalVol20, liborRate20, volatilityShiftSize );

    // Test and Compare Results
	// ---------------------
    const double roundtripVol = lognormalVol;

	EXPECT_NEAR( lognormalVol1 , roundtripVol, testPrecision );
	EXPECT_NEAR( lognormalVol2 , roundtripVol, testPrecision );
	EXPECT_NEAR( lognormalVol3 , roundtripVol, testPrecision );
	EXPECT_NEAR( lognormalVol4 , roundtripVol, testPrecision );
	EXPECT_NEAR( lognormalVol5 , roundtripVol, testPrecision );
    EXPECT_NEAR( lognormalVol6 , roundtripVol, testPrecision );
    EXPECT_NEAR( lognormalVol7 , roundtripVol, testPrecision );
    EXPECT_NEAR( lognormalVol8 , roundtripVol, testPrecision );
    EXPECT_NEAR( lognormalVol9 , roundtripVol, testPrecision );
    EXPECT_NEAR( lognormalVol10, roundtripVol, testPrecision );
    EXPECT_NEAR( lognormalVol11, roundtripVol, testPrecision );
    EXPECT_NEAR( lognormalVol12, roundtripVol, testPrecision );
    EXPECT_NEAR( lognormalVol13, roundtripVol, testPrecision );
    EXPECT_NEAR( lognormalVol14, roundtripVol, testPrecision );
    EXPECT_NEAR( lognormalVol15, roundtripVol, testPrecision );
    EXPECT_NEAR( lognormalVol16, roundtripVol, testPrecision );
    EXPECT_NEAR( lognormalVol17, roundtripVol, testPrecision );
    EXPECT_NEAR( lognormalVol18, roundtripVol, testPrecision );
    EXPECT_NEAR( lognormalVol19, roundtripVol, testPrecision );
    EXPECT_NEAR( lognormalVol20, roundtripVol, testPrecision );
}

TEST( TestAqMathVolatility, UNIT_ConvertVol_Roundtrip_ToFrom_Normal_ShiftedLognormal )
{
    // Shifted Lognormal Volatility 15.0%
    const double shiftedLognormalVol = 0.150;
    
    // Shifted-Lognormal Volatility ShiftSize 3%
    const double volatilityShiftSize = 0.03;

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

    // Convert from Shifted-LogNormal to Normal Vol
    const double normalVol1  = etrading::Convexity::toNormalVolFromShiftedLognormalVol( shiftedLognormalVol, liborRate1 , volatilityShiftSize );
    const double normalVol2  = etrading::Convexity::toNormalVolFromShiftedLognormalVol( shiftedLognormalVol, liborRate2 , volatilityShiftSize );
    const double normalVol3  = etrading::Convexity::toNormalVolFromShiftedLognormalVol( shiftedLognormalVol, liborRate3 , volatilityShiftSize );
    const double normalVol4  = etrading::Convexity::toNormalVolFromShiftedLognormalVol( shiftedLognormalVol, liborRate4 , volatilityShiftSize );
    const double normalVol5  = etrading::Convexity::toNormalVolFromShiftedLognormalVol( shiftedLognormalVol, liborRate5 , volatilityShiftSize );
    const double normalVol6  = etrading::Convexity::toNormalVolFromShiftedLognormalVol( shiftedLognormalVol, liborRate6 , volatilityShiftSize );
    const double normalVol7  = etrading::Convexity::toNormalVolFromShiftedLognormalVol( shiftedLognormalVol, liborRate7 , volatilityShiftSize );
    const double normalVol8  = etrading::Convexity::toNormalVolFromShiftedLognormalVol( shiftedLognormalVol, liborRate8 , volatilityShiftSize );
    const double normalVol9  = etrading::Convexity::toNormalVolFromShiftedLognormalVol( shiftedLognormalVol, liborRate9 , volatilityShiftSize );
    const double normalVol10 = etrading::Convexity::toNormalVolFromShiftedLognormalVol( shiftedLognormalVol, liborRate10, volatilityShiftSize );
    const double normalVol11 = etrading::Convexity::toNormalVolFromShiftedLognormalVol( shiftedLognormalVol, liborRate11, volatilityShiftSize );
    const double normalVol12 = etrading::Convexity::toNormalVolFromShiftedLognormalVol( shiftedLognormalVol, liborRate12, volatilityShiftSize );
    const double normalVol13 = etrading::Convexity::toNormalVolFromShiftedLognormalVol( shiftedLognormalVol, liborRate13, volatilityShiftSize );
    const double normalVol14 = etrading::Convexity::toNormalVolFromShiftedLognormalVol( shiftedLognormalVol, liborRate14, volatilityShiftSize );
    const double normalVol15 = etrading::Convexity::toNormalVolFromShiftedLognormalVol( shiftedLognormalVol, liborRate15, volatilityShiftSize );
    const double normalVol16 = etrading::Convexity::toNormalVolFromShiftedLognormalVol( shiftedLognormalVol, liborRate16, volatilityShiftSize );
    const double normalVol17 = etrading::Convexity::toNormalVolFromShiftedLognormalVol( shiftedLognormalVol, liborRate17, volatilityShiftSize );
    const double normalVol18 = etrading::Convexity::toNormalVolFromShiftedLognormalVol( shiftedLognormalVol, liborRate18, volatilityShiftSize );
    const double normalVol19 = etrading::Convexity::toNormalVolFromShiftedLognormalVol( shiftedLognormalVol, liborRate19, volatilityShiftSize );
    const double normalVol20 = etrading::Convexity::toNormalVolFromShiftedLognormalVol( shiftedLognormalVol, liborRate20, volatilityShiftSize );

    // Expected Results
    const double expectedResult1  = 0.0085440150;
    const double expectedResult2  = 0.0085891200;
    const double expectedResult3  = 0.0085696050;
    const double expectedResult4  = 0.0085433550;
    const double expectedResult5  = 0.0084271050;
    const double expectedResult6  = 0.0084010650;
    const double expectedResult7  = 0.0084294900;
    const double expectedResult8  = 0.0083698950;
    const double expectedResult9  = 0.0083101350;
    const double expectedResult10 = 0.0082770600;
    const double expectedResult11 = 0.0082705800;
    const double expectedResult12 = 0.0082867950;
    const double expectedResult13 = 0.0083100600;
    const double expectedResult14 = 0.0083364750;
    const double expectedResult15 = 0.0083658150;
    const double expectedResult16 = 0.0083964600;
    const double expectedResult17 = 0.0084253200;
    const double expectedResult18 = 0.0084514200;
    const double expectedResult19 = 0.0084744150;
    const double expectedResult20 = 0.0084744150;

	// Test and Compare Results
	// ---------------------
	EXPECT_NEAR( normalVol1 , expectedResult1 , testPrecision );
	EXPECT_NEAR( normalVol2 , expectedResult2 , testPrecision );
	EXPECT_NEAR( normalVol3 , expectedResult3 , testPrecision );
	EXPECT_NEAR( normalVol4 , expectedResult4 , testPrecision );
	EXPECT_NEAR( normalVol5 , expectedResult5 , testPrecision );
    EXPECT_NEAR( normalVol6 , expectedResult6 , testPrecision );
    EXPECT_NEAR( normalVol7 , expectedResult7 , testPrecision );
    EXPECT_NEAR( normalVol8 , expectedResult8 , testPrecision );
    EXPECT_NEAR( normalVol9 , expectedResult9 , testPrecision );
    EXPECT_NEAR( normalVol10, expectedResult10, testPrecision );
    EXPECT_NEAR( normalVol11, expectedResult11, testPrecision );
    EXPECT_NEAR( normalVol12, expectedResult12, testPrecision );
    EXPECT_NEAR( normalVol13, expectedResult13, testPrecision );
    EXPECT_NEAR( normalVol14, expectedResult14, testPrecision );
    EXPECT_NEAR( normalVol15, expectedResult15, testPrecision );
    EXPECT_NEAR( normalVol16, expectedResult16, testPrecision );
    EXPECT_NEAR( normalVol17, expectedResult17, testPrecision );
    EXPECT_NEAR( normalVol18, expectedResult18, testPrecision );
    EXPECT_NEAR( normalVol19, expectedResult19, testPrecision );
    EXPECT_NEAR( normalVol20, expectedResult20, testPrecision );

    // Convert back to Lognormal from Normal
    const double shiftedLognormalVol1  = etrading::Convexity::toShiftedLognormalVolFromNormalVol( normalVol1 , liborRate1 , volatilityShiftSize );
    const double shiftedLognormalVol2  = etrading::Convexity::toShiftedLognormalVolFromNormalVol( normalVol2 , liborRate2 , volatilityShiftSize ); 
    const double shiftedLognormalVol3  = etrading::Convexity::toShiftedLognormalVolFromNormalVol( normalVol3 , liborRate3 , volatilityShiftSize );
    const double shiftedLognormalVol4  = etrading::Convexity::toShiftedLognormalVolFromNormalVol( normalVol4 , liborRate4 , volatilityShiftSize );
    const double shiftedLognormalVol5  = etrading::Convexity::toShiftedLognormalVolFromNormalVol( normalVol5 , liborRate5 , volatilityShiftSize );
    const double shiftedLognormalVol6  = etrading::Convexity::toShiftedLognormalVolFromNormalVol( normalVol6 , liborRate6 , volatilityShiftSize );
    const double shiftedLognormalVol7  = etrading::Convexity::toShiftedLognormalVolFromNormalVol( normalVol7 , liborRate7 , volatilityShiftSize );
    const double shiftedLognormalVol8  = etrading::Convexity::toShiftedLognormalVolFromNormalVol( normalVol8 , liborRate8 , volatilityShiftSize );
    const double shiftedLognormalVol9  = etrading::Convexity::toShiftedLognormalVolFromNormalVol( normalVol9 , liborRate9 , volatilityShiftSize );
    const double shiftedLognormalVol10 = etrading::Convexity::toShiftedLognormalVolFromNormalVol( normalVol10, liborRate10, volatilityShiftSize );
    const double shiftedLognormalVol11 = etrading::Convexity::toShiftedLognormalVolFromNormalVol( normalVol11, liborRate11, volatilityShiftSize );
    const double shiftedLognormalVol12 = etrading::Convexity::toShiftedLognormalVolFromNormalVol( normalVol12, liborRate12, volatilityShiftSize );
    const double shiftedLognormalVol13 = etrading::Convexity::toShiftedLognormalVolFromNormalVol( normalVol13, liborRate13, volatilityShiftSize );
    const double shiftedLognormalVol14 = etrading::Convexity::toShiftedLognormalVolFromNormalVol( normalVol14, liborRate14, volatilityShiftSize );
    const double shiftedLognormalVol15 = etrading::Convexity::toShiftedLognormalVolFromNormalVol( normalVol15, liborRate15, volatilityShiftSize );
    const double shiftedLognormalVol16 = etrading::Convexity::toShiftedLognormalVolFromNormalVol( normalVol16, liborRate16, volatilityShiftSize );
    const double shiftedLognormalVol17 = etrading::Convexity::toShiftedLognormalVolFromNormalVol( normalVol17, liborRate17, volatilityShiftSize );
    const double shiftedLognormalVol18 = etrading::Convexity::toShiftedLognormalVolFromNormalVol( normalVol18, liborRate18, volatilityShiftSize );
    const double shiftedLognormalVol19 = etrading::Convexity::toShiftedLognormalVolFromNormalVol( normalVol19, liborRate19, volatilityShiftSize );
    const double shiftedLognormalVol20 = etrading::Convexity::toShiftedLognormalVolFromNormalVol( normalVol20, liborRate20, volatilityShiftSize );

    // Test and Compare Results
	// ---------------------
    const double roundtripVol = shiftedLognormalVol;

	EXPECT_NEAR( shiftedLognormalVol1 , roundtripVol, testPrecision );
	EXPECT_NEAR( shiftedLognormalVol2 , roundtripVol, testPrecision );
	EXPECT_NEAR( shiftedLognormalVol3 , roundtripVol, testPrecision );
	EXPECT_NEAR( shiftedLognormalVol4 , roundtripVol, testPrecision );
	EXPECT_NEAR( shiftedLognormalVol5 , roundtripVol, testPrecision );
    EXPECT_NEAR( shiftedLognormalVol6 , roundtripVol, testPrecision );
    EXPECT_NEAR( shiftedLognormalVol7 , roundtripVol, testPrecision );
    EXPECT_NEAR( shiftedLognormalVol8 , roundtripVol, testPrecision );
    EXPECT_NEAR( shiftedLognormalVol9 , roundtripVol, testPrecision );
    EXPECT_NEAR( shiftedLognormalVol10, roundtripVol, testPrecision );
    EXPECT_NEAR( shiftedLognormalVol11, roundtripVol, testPrecision );
    EXPECT_NEAR( shiftedLognormalVol12, roundtripVol, testPrecision );
    EXPECT_NEAR( shiftedLognormalVol13, roundtripVol, testPrecision );
    EXPECT_NEAR( shiftedLognormalVol14, roundtripVol, testPrecision );
    EXPECT_NEAR( shiftedLognormalVol15, roundtripVol, testPrecision );
    EXPECT_NEAR( shiftedLognormalVol16, roundtripVol, testPrecision );
    EXPECT_NEAR( shiftedLognormalVol17, roundtripVol, testPrecision );
    EXPECT_NEAR( shiftedLognormalVol18, roundtripVol, testPrecision );
    EXPECT_NEAR( shiftedLognormalVol19, roundtripVol, testPrecision );
    EXPECT_NEAR( shiftedLognormalVol20, roundtripVol, testPrecision );
}
