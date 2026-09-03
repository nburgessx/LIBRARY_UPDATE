#include "meCurveResults.h"
#include "tryMeCurveResults.h"
#include "TypeUtilities.h"          // Swig Marshalling Helper Methods
#include "Variant.h"                // Variant and Variant Matrix Types
#include "APISetUp.h"               // MLIB_API_START and MLIB_API_END Macros

/* @brief			swig interface for meCurveDisplay
*  @param [in]		curveCollection		ID of the curve set
*  @param [in]		curveIndex			Index of the curve set
*  @return			Matrix of paymentDates and corresponding discount factors from the yield curve
*/
SWIG_STRINGMATRIX meCurveResultsDisplayDiscountFactors (const std::string& curveCollection, const std::string& curveIndex )
{
	MLIB_API_START
    
	etrading::VariantMatrix results = validation_api::tryMeCurveResultsDiscountFactorsDisplayAll(curveCollection, curveIndex);

    // Marshall Output to Standard String Matrix
	SWIG_STRINGMATRIX resultsStringMatrix = swig::fromVariantMatrixToMatrixOfString( results );

    return resultsStringMatrix;

    MLIB_API_END
}