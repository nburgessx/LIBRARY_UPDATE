#include "meLWOGrids.h"
#include "tryMeLWOGrid.h"

#include "TypeUtilities.h"          // Swig Marshalling Helper Methods
#include "Variant.h"                // Variant and Variant Matrix Types
#include "APISetUp.h"               // MLIB_API_START and MLIB_API_END Macros

SWIG_STRINGMATRIX meLWOGridDisplay( const std::string& objectName, const bool& displayColumnNames  )
{
	MLIB_API_START
	auto result = validation_api::tryMeLWOGridDisplay( objectName );

    auto& columnNames = std::get<1>( result );
    auto& flexibleData = std::get<0>( result );


	// Marshall Output to Standard String Matrix
	SWIG_STRINGMATRIX resultsStringMatrix = swig::fromVariantMatrixToMatrixOfString( flexibleData );

    return resultsStringMatrix;

    MLIB_API_END
}