#include "aqObjectsGrids.h"
#include "tryAqObjectsGrid.h"

#include "TypeUtilities.h"          // Swig Marshalling Helper Methods
#include "Variant.h"                // Variant and Variant Matrix Types
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros

SWIG_STRINGMATRIX aqObjectsGridDisplay( const std::string& objectName, const bool& displayColumnNames  )
{
	AQ_API_START
	auto result = validation::tryAqObjectsGridDisplay( objectName );

    auto& columnNames = std::get<1>( result );
    auto& flexibleData = std::get<0>( result );


	// Marshall Output to Standard String Matrix
	SWIG_STRINGMATRIX resultsStringMatrix = swig::fromVariantMatrixToMatrixOfString( flexibleData );

    return resultsStringMatrix;

    AQ_API_END
}