// RiskUtilities.h
#pragma once

#include "AQLString.h"
#include <vector>

namespace etrading
{
	// Helper Method to check if the Curve is a Curve List i.e. not a Single curve
	bool isCurveList( const AQLString& curveIndex, const AQLString& delimiter = ":" );
	
	// Helper Method to generate a vector of curve index names given a list of curve indices as a single concatenated string
	std::vector<AQLString> generateCurveList( const AQLString& curveIndex, const AQLString& delimiter = ":" );

	// Helper Method to generate a vector of static data names given a list of curve indices as a single concatenated string
	std::vector<AQLString> generateStaticDataList( const AQLString& curveCollection, const AQLString& curveIndex, const AQLString& delimiter = ":" );

	// Helper Method to generate a concatenated list of Static Data names given a list of curve indices as a single concatenated string
	AQLString generateStaticDataListAsString( const AQLString& curveCollection, const AQLString& curveIndex, const AQLString& delimiter = ":" );

}
