// RiskUtilities.cpp
#include "RiskUtilities.h"
#include "CurveValidation.h"	// for method getCurveStaticDataTableName

namespace etrading
{
	// Helper Method to check if the Curve is a Curve List i.e. not a Single curve
	bool isCurveList( const LAString& curveIndex, const LAString& delimiter)
	{
		// If we find the delimiter in the string then we have a curve list
		std::string curveIndexString = curveIndex.c_str();
		return curveIndexString.find( delimiter.c_str() ) != std::string::npos;
	}

	// Helper Method to generate a vector of curve names given a list of curve indices as a single concatenated string
	std::vector<LAString> generateCurveList( const LAString& curveIndex, const LAString& delimiter )
	{
		std::vector<LAString> curveList;
		std::string curveIndexString = curveIndex.c_str();
		std::string delimiterString = delimiter.c_str();

		size_t i = 0;
		size_t loopGuard = 100;
		LAString thisCurveIndex;

		// Split the input curve index string by delimiter and update the curve list
		while( curveIndexString.size() > 0 && i < loopGuard )
		{
			size_t delimiterPosition = curveIndexString.find( delimiterString );
			if( delimiterPosition != std::string::npos )
			{
				// Split the string by delimiter and update the curve index
				// Note we remove the current curve index from the original curve string and repeat until there are no more delimiters
				thisCurveIndex = curveIndexString.substr( 0, delimiterPosition );
				curveList.push_back( thisCurveIndex );
				curveIndexString = curveIndexString.substr( delimiterPosition + delimiterString.length() );
				i++;
			}
			else
			{
				// No more delimiter tokens so return the curveIndexString
				curveList.push_back( curveIndexString );
				break;
			}
		}

		return curveList;
	}

	// Helper Method to generate a vector of static data names given a list of curve indices as a single concatenated string
	std::vector<LAString> generateStaticDataList( const LAString& curveCollection, const LAString& curveIndex, const LAString& delimiter )
	{
		std::vector<LAString> curveIndexResults = generateCurveList( curveIndex, delimiter );
		for( size_t i = 0; i < curveIndexResults.size(); ++i )
		{
			curveIndexResults[i] = getCurveStaticDataTableName( curveCollection, curveIndexResults[i], false /* uppercase */ );
		}
		return curveIndexResults;
	}

	// Helper Method to generate a single string of concatenated static data table names given a list of curve indices as a single concatenated string
	LAString generateStaticDataListAsString( const LAString& curveCollection, const LAString& curveIndex, const LAString& delimiter )
	{
		LAString staticDataString;
		const LAString objectPoolDelimiter = ":";

		std::vector<LAString> staticDataNamesForCurves = generateStaticDataList( curveCollection, curveIndex, delimiter );
		
		for( size_t i = 0; i < staticDataNamesForCurves.size(); ++i )
		{
			// Create Result List
			if( staticDataString.size() == 0 )
			{
				// Result = String (no colon delimiter)
				staticDataString += staticDataNamesForCurves[i];
			}
			else
			{
				// Result = String1 : String1 (with colon delimiter)
				staticDataString += objectPoolDelimiter + staticDataNamesForCurves[i];
			}
		}
		return staticDataString;
	}
}