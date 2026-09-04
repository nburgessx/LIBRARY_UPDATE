#pragma once

 // Include: Google Test Library
#include <gTest/gTest.h>

#include "ValuationSettings.h"
#include "Dependency.h"
#include "ReadDataFile.h"
#include "GetGoogleTestFolder.h"
#include "ResultsProcessor.h"
#include "tryAqObjects.h"
#include "tryAqCurvesResults.h"

namespace
{
    const std::string googleTestFolder      = etrading::getGoogleTestFolder();

    const std::string USDCurveOIS           = googleTestFolder + "ETrading/ValuationSettings/USD_OIS_CURVE.JSON";
    const std::string USDCurve3M            = googleTestFolder + "ETrading/ValuationSettings/USD_SWAP_3M_CURVE.JSON";

    const std::string EURCurveOIS           = googleTestFolder + "ETrading/ValuationSettings/EUR_OIS_CURVE.JSON";
    const std::string EURCurve3M            = googleTestFolder + "ETrading/ValuationSettings/EUR_SWAP_3M_CURVE.JSON";
}

namespace google_test
{
	
	// Declare Test Fixture Class
	DECLARE_TEST_FIXTURE( TestValuationSettings )
    DECLARE_TEST_FIXTURE( TestValuationSettingsUsingObjects )

	// Call Test Fixture Class
    TEST_F( TestValuationSettingsUsingObjects, UNIT_CurveHandle_SingleCell )
    {
        // Load Curves
        const std::string curveUSDOIS   = validation::tryAqObjLoad( USDCurveOIS, etrading::JSON );
        const std::string curveUSD3M    = validation::tryAqObjLoad( USDCurve3M, etrading::JSON );

		StandardStringMatrix rawData = { { curveUSD3M } };
		StandardStringMatrix expectedResult = { { "CURVECOLLECTION", "USDYC" } };
		StandardStringMatrix modifiedData = etrading::ValuationSettings( rawData ).getValuationSettingsLVB().toStandardStringMatrix();
		
		// Check Results
		for( size_t row = 0; row < expectedResult.size(); ++row )
		{
			for( size_t col = 0; col < expectedResult[row].size(); ++col )
			{
				EXPECT_EQ( expectedResult[row][col], modifiedData[row][col] );
			}
		}
	}

	// Call Test Fixture Class
    TEST_F( TestValuationSettingsUsingObjects, UNIT_CurveHandle_SingleRow )
    {
        // Load Curves
        const std::string curveUSDOIS   = validation::tryAqObjLoad( USDCurveOIS, etrading::JSON );
        const std::string curveUSD3M    = validation::tryAqObjLoad( USDCurve3M, etrading::JSON );

		StandardStringMatrix rawData = { { "CURVECOLLECTION", curveUSD3M } };
		StandardStringMatrix expectedResult = { { "CURVECOLLECTION", "USDYC" } };
		StandardStringMatrix modifiedData = etrading::ValuationSettings( rawData ).getValuationSettingsLVB().toStandardStringMatrix();
		
		// Check Results
		for( size_t row = 0; row < modifiedData.size(); ++row )
		{
			for( size_t col = 0; col < modifiedData[row].size(); ++col )
			{
				EXPECT_EQ( expectedResult[row][col], modifiedData[row][col] );
			}
		}
	}

	// Call Test Fixture Class
    TEST_F( TestValuationSettingsUsingObjects, UNIT_CurveHandle_SingleRow_SpecialCase )
    {
        // Load Curves
        const std::string curveUSDOIS   = validation::tryAqObjLoad( USDCurveOIS, etrading::JSON );
        const std::string curveUSD3M    = validation::tryAqObjLoad( USDCurve3M, etrading::JSON );

		StandardStringMatrix rawData = { { curveUSD3M, "" } };
		StandardStringMatrix expectedResult = { { "CURVECOLLECTION", "USDYC" } };
		StandardStringMatrix modifiedData = etrading::ValuationSettings( rawData ).getValuationSettingsLVB().toStandardStringMatrix();
		
		// Check Results
		for( size_t row = 0; row < modifiedData.size(); ++row )
		{
			for( size_t col = 0; col < modifiedData[row].size(); ++col )
			{
				EXPECT_EQ( expectedResult[row][col], modifiedData[row][col] );
			}
		}
	}

	// Call Test Fixture Class
    TEST_F( TestValuationSettingsUsingObjects, UNIT_CurveHandle_SingleColumn )
    {
        // Load Curves
        const std::string curveUSDOIS   = validation::tryAqObjLoad( USDCurveOIS, etrading::JSON );
        const std::string curveUSD3M    = validation::tryAqObjLoad( USDCurve3M, etrading::JSON );

		// Single Column Input is for Curve Objects from the Same Curve Collection Only
		StandardStringMatrix rawData = { { curveUSDOIS },
										 { curveUSD3M } };

		StandardStringMatrix expectedResult = { { "CURVECOLLECTION", "USDYC" } };

		StandardStringMatrix modifiedData = etrading::ValuationSettings( rawData ).getValuationSettingsLVB().toStandardStringMatrix();
		
		// Check Results
		for( size_t row = 0; row < expectedResult.size(); ++row )
		{
			for( size_t col = 0; col < expectedResult[row].size(); ++col )
			{
				EXPECT_EQ( expectedResult[row][col], modifiedData[row][col] );
			}
		}
	}

	// Call Test Fixture Class
    TEST_F( TestValuationSettingsUsingObjects, UNIT_CurveHandle_LabelValueBlock )
    {
        // Load Curves
        const std::string curveUSDOIS   = validation::tryAqObjLoad( USDCurveOIS, etrading::JSON );
        const std::string curveUSD3M    = validation::tryAqObjLoad( USDCurve3M, etrading::JSON );
		const std::string curveEUROIS   = validation::tryAqObjLoad( EURCurveOIS, etrading::JSON );
        const std::string curveEUR3M    = validation::tryAqObjLoad( EURCurve3M, etrading::JSON );

		StandardStringMatrix rawData = { { "Leg1:Fixed", curveUSD3M	 },
										 { "Leg2:Float", curveEUR3M	 },
										 { "Leg3:Fee",   curveUSDOIS },
										 { "Leg4:Fee",   curveEUROIS },
										 { "FXSpot",     "1.1003"  } };

		StandardStringMatrix expectedResult = { { "Leg1:Fixed", "USDYC"	},
												{ "Leg2:Float", "EURYC"	},
												{ "Leg3:Fee",   "USDYC"	},
												{ "Leg4:Fee",   "EURYC"	},
												{ "FXSpot",     "1.1003" } };

		StandardStringMatrix modifiedData = etrading::ValuationSettings( rawData ).getValuationSettingsLVB().toStandardStringMatrix();
		
		// Check Results
		for( size_t row = 0; row < expectedResult.size(); ++row )
		{
			for( size_t col = 0; col < expectedResult[row].size(); ++col )
			{
				EXPECT_EQ( expectedResult[row][col], modifiedData[row][col] );
			}
		}
    }
	
	// Call Test Fixture Class
    TEST_F( TestValuationSettingsUsingObjects, UNIT_CurveGroup_SingleCell )
    {
        // Load Curves
        const std::string curveUSDOIS   = validation::tryAqObjLoad( USDCurveOIS, etrading::JSON );
        const std::string curveUSD3M    = validation::tryAqObjLoad( USDCurve3M, etrading::JSON );

		// Create Curve Group
		const std::vector<std::string> USDcurves = { curveUSDOIS, curveUSD3M };
		const std::string curveGroupUSD	= validation::tryAqCurvesGroupCreate( "USDCurves", USDcurves );

		StandardStringMatrix rawData = { { curveGroupUSD } };
		StandardStringMatrix expectedResult = { { "CURVECOLLECTION", "USDYC" } };
		StandardStringMatrix modifiedData = etrading::ValuationSettings( rawData ).getValuationSettingsLVB().toStandardStringMatrix();
		
		// Check Results
		for( size_t row = 0; row < expectedResult.size(); ++row )
		{
			for( size_t col = 0; col < expectedResult[row].size(); ++col )
			{
				EXPECT_EQ( expectedResult[row][col], modifiedData[row][col] );
			}
		}
	}

	// Call Test Fixture Class
    TEST_F( TestValuationSettingsUsingObjects, UNIT_CurveGroup_SingleRow )
    {
        // Load Curves
        const std::string curveUSDOIS   = validation::tryAqObjLoad( USDCurveOIS, etrading::JSON );
        const std::string curveUSD3M    = validation::tryAqObjLoad( USDCurve3M, etrading::JSON );

		// Create Curve Group
		const std::vector<std::string> USDcurves = { curveUSDOIS, curveUSD3M };
		const std::string curveGroupUSD	= validation::tryAqCurvesGroupCreate( "USDCurves", USDcurves );

		StandardStringMatrix rawData = { { "CURVECOLLECTION", curveGroupUSD } };
		StandardStringMatrix expectedResult = { { "CURVECOLLECTION", "USDYC" } };
		StandardStringMatrix modifiedData = etrading::ValuationSettings( rawData ).getValuationSettingsLVB().toStandardStringMatrix();
		
		// Check Results
		for( size_t row = 0; row < modifiedData.size(); ++row )
		{
			for( size_t col = 0; col < modifiedData[row].size(); ++col )
			{
				EXPECT_EQ( expectedResult[row][col], modifiedData[row][col] );
			}
		}
	}

	// Call Test Fixture Class
    TEST_F( TestValuationSettingsUsingObjects, UNIT_CurveGroup_SingleRow_SpecialCase )
    {
        // Load Curves
        const std::string curveUSDOIS   = validation::tryAqObjLoad( USDCurveOIS, etrading::JSON );
        const std::string curveUSD3M    = validation::tryAqObjLoad( USDCurve3M, etrading::JSON );

		// Create Curve Group
		const std::vector<std::string> USDcurves = { curveUSDOIS, curveUSD3M };
		const std::string curveGroupUSD	= validation::tryAqCurvesGroupCreate( "USDCurves", USDcurves );

		StandardStringMatrix rawData = { { curveGroupUSD, "" } };
		StandardStringMatrix expectedResult = { { "CURVECOLLECTION", "USDYC" } };
		StandardStringMatrix modifiedData = etrading::ValuationSettings( rawData ).getValuationSettingsLVB().toStandardStringMatrix();
		
		// Check Results
		for( size_t row = 0; row < modifiedData.size(); ++row )
		{
			for( size_t col = 0; col < modifiedData[row].size(); ++col )
			{
				EXPECT_EQ( expectedResult[row][col], modifiedData[row][col] );
			}
		}
	}

	// Call Test Fixture Class
    TEST_F( TestValuationSettingsUsingObjects, UNIT_CurveGroup_SingleColumn )
    {
        // Load Curves
        const std::string curveUSDOIS   = validation::tryAqObjLoad( USDCurveOIS, etrading::JSON );
        const std::string curveUSD3M    = validation::tryAqObjLoad( USDCurve3M, etrading::JSON );

		// Create Curve Group
		const std::vector<std::string> USDcurves = { curveUSDOIS, curveUSD3M };
		const std::string curveGroupUSD	= validation::tryAqCurvesGroupCreate( "USDCurves", USDcurves );

		// Single Column Input is for Curve Objects from the Same Curve Collection Only
		StandardStringMatrix rawData = { { curveGroupUSD },
										 { curveGroupUSD } };

		StandardStringMatrix expectedResult = { { "CURVECOLLECTION", "USDYC" } };

		StandardStringMatrix modifiedData = etrading::ValuationSettings( rawData ).getValuationSettingsLVB().toStandardStringMatrix();
		
		// Check Results
		for( size_t row = 0; row < expectedResult.size(); ++row )
		{
			for( size_t col = 0; col < expectedResult[row].size(); ++col )
			{
				EXPECT_EQ( expectedResult[row][col], modifiedData[row][col] );
			}
		}
	}

	// Call Test Fixture Class
    TEST_F( TestValuationSettingsUsingObjects, UNIT_CurveGroup_LabelValueBlock )
    {
        // Load Curves
        const std::string curveUSDOIS   = validation::tryAqObjLoad( USDCurveOIS, etrading::JSON );
        const std::string curveUSD3M    = validation::tryAqObjLoad( USDCurve3M, etrading::JSON );
		const std::string curveEUROIS   = validation::tryAqObjLoad( EURCurveOIS, etrading::JSON );
        const std::string curveEUR3M    = validation::tryAqObjLoad( EURCurve3M, etrading::JSON );

		// Create Curve Group
		const std::vector<std::string> USDcurves = { curveUSDOIS, curveUSD3M };
		const std::vector<std::string> EURcurves = { curveEUROIS, curveEUR3M };
		
		const std::string curveGroupUSD	= validation::tryAqCurvesGroupCreate( "USDCurves", USDcurves );
		const std::string curveGroupEUR	= validation::tryAqCurvesGroupCreate( "EURCurves", EURcurves );
		
		StandardStringMatrix rawData = { { "Leg1:Fixed", curveGroupUSD },
										 { "Leg2:Float", curveGroupEUR },
										 { "Leg3:Fee",   curveGroupUSD },
										 { "Leg4:Fee",   curveGroupEUR },
										 { "FXSpot",     "1.1003"  } };

		StandardStringMatrix expectedResult = { { "Leg1:Fixed", "USDYC"	},
												{ "Leg2:Float", "EURYC"	},
												{ "Leg3:Fee",   "USDYC"	},
												{ "Leg4:Fee",   "EURYC"	},
												{ "FXSpot",     "1.1003" } };

		StandardStringMatrix modifiedData = etrading::ValuationSettings( rawData ).getValuationSettingsLVB().toStandardStringMatrix();
		
		// Check Results
		for( size_t row = 0; row < expectedResult.size(); ++row )
		{
			for( size_t col = 0; col < expectedResult[row].size(); ++col )
			{
				EXPECT_EQ( expectedResult[row][col], modifiedData[row][col] );
			}
		}
    }

    TEST_F( TestValuationSettings, UNIT_CurveCollection_SingleCell )
    {
		StandardStringMatrix rawData = { { "USDYC" } };
		StandardStringMatrix expectedResult = { { "CURVECOLLECTION", "USDYC" } };
		StandardStringMatrix modifiedData = etrading::ValuationSettings( rawData ).getValuationSettingsLVB().toStandardStringMatrix();
		
		// Check Results
		for( size_t row = 0; row < expectedResult.size(); ++row )
		{
			for( size_t col = 0; col < expectedResult[row].size(); ++col )
			{
				EXPECT_EQ( expectedResult[row][col], modifiedData[row][col] );
			}
		}
    }

	TEST_F( TestValuationSettings, UNIT_CurveCollection_SingleRow )
    {
		StandardStringMatrix rawData = { { "CURVECOLLECTION", "USDYC" } };
		StandardStringMatrix expectedResult = { { "CURVECOLLECTION", "USDYC" } };
		StandardStringMatrix modifiedData = etrading::ValuationSettings( rawData ).getValuationSettingsLVB().toStandardStringMatrix();
		
		// Check Results
		for( size_t row = 0; row < modifiedData.size(); ++row )
		{
			for( size_t col = 0; col < modifiedData[row].size(); ++col )
			{
				EXPECT_EQ( expectedResult[row][col], modifiedData[row][col] );
			}
		}
    }

	TEST_F( TestValuationSettings, UNIT_CurveCollection_SingleRow_SpecialCase )
    {
		// Special Case: We have a Single Row, however the curve collection is the Key and the Value is Blank
		StandardStringMatrix rawData = { { "USDYC", "" } };
		StandardStringMatrix expectedResult = { { "CURVECOLLECTION", "USDYC" } };
		StandardStringMatrix modifiedData = etrading::ValuationSettings( rawData ).getValuationSettingsLVB().toStandardStringMatrix();
		
		// Check Results
		for( size_t row = 0; row < modifiedData.size(); ++row )
		{
			for( size_t col = 0; col < modifiedData[row].size(); ++col )
			{
				EXPECT_EQ( expectedResult[row][col], modifiedData[row][col] );
			}
		}
    }

	TEST_F( TestValuationSettings, UNIT_CurveCollection_SingleColumn )
    {
		// Single Column Input is for Curve Objects from the Same Curve Collection Only
		StandardStringMatrix rawData = { { "USDYC" },
										 { "USDYC" },
										 { "USDYC" },
										 { "USDYC" } };

		StandardStringMatrix expectedResult = { { "CURVECOLLECTION", "USDYC" } };

		StandardStringMatrix modifiedData = etrading::ValuationSettings( rawData ).getValuationSettingsLVB().toStandardStringMatrix();
		
		// Check Results
		for( size_t row = 0; row < expectedResult.size(); ++row )
		{
			for( size_t col = 0; col < expectedResult[row].size(); ++col )
			{
				EXPECT_EQ( expectedResult[row][col], modifiedData[row][col] );
			}
		}
    }

	TEST_F( TestValuationSettings, UNIT_CurveCollection_LabelValueBlock )
    {
		StandardStringMatrix rawData = { { "Leg1:Fixed", "JPYYC"	},
										 { "Leg2:Float", "USDYC"	},
										 { "Leg3:Fee",   "JPYYC"	},
										 { "Leg4:Fee",   "USDYC"	},
										 { "FXSpot",     "0.009118" } };

		StandardStringMatrix expectedResult = { { "Leg1:Fixed", "JPYYC"	},
												{ "Leg2:Float", "USDYC"	},
												{ "Leg3:Fee",   "JPYYC"	},
												{ "Leg4:Fee",   "USDYC"	},
												{ "FXSpot",     "0.009118" } };

		StandardStringMatrix modifiedData = etrading::ValuationSettings( rawData ).getValuationSettingsLVB().toStandardStringMatrix();
		
		// Check Results
		for( size_t row = 0; row < expectedResult.size(); ++row )
		{
			for( size_t col = 0; col < expectedResult[row].size(); ++col )
			{
				EXPECT_EQ( expectedResult[row][col], modifiedData[row][col] );
			}
		}
    }

}
