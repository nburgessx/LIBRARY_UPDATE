// RiskUtilities.h
#pragma once

#include "LAString.h"
#include <vector>

namespace etrading
{
	// Helper Method to check if the Curve is a Curve List i.e. not a Single curve
	bool isCurveList( const LAString& curveIndex, const LAString& delimiter = ":" );
	
	// Helper Method to generate a vector of curve index names given a list of curve indices as a single concatenated string
	std::vector<LAString> generateCurveList( const LAString& curveIndex, const LAString& delimiter = ":" );

	// Helper Method to generate a vector of static data names given a list of curve indices as a single concatenated string
	std::vector<LAString> generateStaticDataList( const LAString& curveCollection, const LAString& curveIndex, const LAString& delimiter = ":" );

	// Helper Method to generate a concatenated list of Static Data names given a list of curve indices as a single concatenated string
	LAString generateStaticDataListAsString( const LAString& curveCollection, const LAString& curveIndex, const LAString& delimiter = ":" );

}
