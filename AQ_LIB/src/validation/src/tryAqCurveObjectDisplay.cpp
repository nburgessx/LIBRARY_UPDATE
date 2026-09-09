// tryAqCurveObjectDisplay.cpp

/*
 * @brief			validation interface for the aqCurveObjectDisplay method
 */

#include "tryAqCurveObjectDisplay.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "EnvironmentUtilities.h"
#include "ObjectUtilities.h"
#include "CoreEnumerations.h"
#include "CurveGenerator.h"
#include "CurveMarketData.h"
#include "AQObjUtilities.h"
#include "RecordMacros.h"
#include "DataUtilities.h"
#include "ExceptionMacros.h"
#include "tryAqObject.h"
#include "MultiCurveObject.h"
#include "SingleCurveObject.h"

#include <string>
#include <set>
#include <algorithm>
#include <locale>
#include <boost/format.hpp>

using etrading::CreateDataFile;
using etrading::decorateFilename;

namespace
{
	void displaySingleCurve(const etrading::SingleCurveObjectPtr& singleCurveObject, etrading::VariantMatrix& results, bool insertInterCurveSpace = false)
	{
		// 1. Get the Curve Market Data
		auto curveMarketDataObj = singleCurveObject->getCurveMarketDataObj();
		// We must transpose, since the curve Market Data is Inverted - what a stupid thing to have to do!
		const etrading::VariantMatrix curveMarketData = Variant::transpose( curveMarketDataObj->viewInputParameters( etrading::CurveMarketDataEnum::NONE_MARKETDATA, -1 ) ); // NONE_MARKETDATA = Display All, -1 = Display All Columns
		AQ_REQUIRE( curveMarketData.size() != 0, "No curve market data to display" )
		AQ_REQUIRE( curveMarketData[0].size() != 0, "No curve market data to display" )

		// 2. Get the Curve Conventions
		auto curveConventionsObj = singleCurveObject->getCurveGeneratorObj();
		const etrading::VariantMatrix curveConventions = curveConventionsObj->viewInputParameters(""); // PropertyName = "" = Display All Properties
		AQ_REQUIRE( curveConventions.size() != 0, "No curve conventions data to display" )
		AQ_REQUIRE( curveConventions[0].size() != 0, "No curve conventions data to display" )

		// 3. Append the Market Data and Convention Variant Matrices
        
		// Initialize the results with row size equals to the total number of market data + convention rows + columnSpaceDivided and having column size equal to the market data column size which is larger
		// Note We have a minimum of 2 columns
		const size_t curveMarketDataSize = curveMarketData.size();
		const size_t curveConventionsSize = curveConventions.size();
		const size_t columnSpaceDivider = 1;

		results.resize( curveMarketDataSize + curveConventionsSize + columnSpaceDivider, etrading::VariantVector( std::min<size_t>( curveMarketData[0].size(), 2 ), "" ) );

		// Add Curve Conventions to Results
		for (unsigned int i = 0; i < curveConventionsSize; ++i )
		{
			results[ i ] = curveConventions[i];
		}

		// Insert a Column Space Divider
		results[ curveConventionsSize ] = etrading::VariantVector( results[0].size(), "" );

		// Add Curve Market Data to Results
		for (unsigned int j = 0; j < curveMarketDataSize; ++j )
		{
			results[ j + curveConventionsSize + columnSpaceDivider ] = curveMarketData[j];
		}

		if (insertInterCurveSpace)
		{
			// Insert a Column Space Divider
			results.push_back(etrading::VariantVector(results[0].size(), ""));
		}
	}
}

namespace validation
{
    
    /* @brief Displays market data and conventions used for curve calibration
    * @param [in] curveObjectName      The CurveGenerator object you wish to display
    * @param [out]                     A VariantMatrix containing a LabelValue block of curve market data and conventions.
    */
	etrading::VariantMatrix tryAqCurveObjectDisplay( const std::string& curveObjectName )
    {
		VALID_EXCEPTION_START
		
		etrading::VariantMatrix results;

		std::vector<std::string> objectTypes = tryAqObjectType(curveObjectName);
		if (objectTypes.size() == 0)
		{
			AQ_THROW("Unrecognised object type from " + curveObjectName)
		}
		std::string objectTypeUsed = objectTypes[0];
		
		if (objectTypeUsed == etrading::toString(etrading::CachedObjectEnum::CURVE))
		{
			// 1. First create the curve pointer to the correct curve in the Cache
			auto& env = etrading::Environment::defaultEnv();
			auto singleCurveObject = env.accessObject<etrading::SingleCurveObject>( curveObjectName );
			AQ_REQUIRE(singleCurveObject, "AQObj Curve " + curveObjectName + " does not exist")

			displaySingleCurve(singleCurveObject, results);
        }
		else if (objectTypeUsed == etrading::toString(etrading::CachedObjectEnum::MULTICURVE))
		{
			// 1. First create the curve pointer to the correct curve in the Cache
			auto& env = etrading::Environment::defaultEnv();
			auto curveEngineObject = env.accessObject<etrading::MultiCurveObject>(curveObjectName);
			AQ_REQUIRE(curveEngineObject, "AQObj Curve Engine" + curveObjectName + " does not exist")

			std::vector<etrading::SingleCurveObjectPtr> singleCurveObjs = curveEngineObject->getCurveObjects();
			size_t curveCount = singleCurveObjs.size();
			for (size_t i = 0; i < curveCount; ++i)
			{
				etrading::VariantMatrix singleCurveResult;
				displaySingleCurve(singleCurveObjs[i], singleCurveResult, i == curveCount-1 ? false : true /*insertInterCurveSpace*/);

				results.insert(results.end(), singleCurveResult.begin(), singleCurveResult.end());
			}
		}
		else
		{
			AQ_THROW("This function only support CURVE and MULTICURVE types. You gave me: " + objectTypeUsed )
		}
        return results;
        VALID_EXCEPTION_END
    }

}
