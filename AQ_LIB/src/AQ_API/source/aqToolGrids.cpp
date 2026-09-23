#include "aqToolGrids.h"
#include "tryAqToolGrid.h"
#include "tryAqToolMultiGrid.h"

#include "TypeUtilities.h"          // Swig Marshalling Helper Methods
#include "Variant.h"                // Variant and Variant Matrix Types
#include "JSONInfoBlock.h"          // JSONInfoBlock::createInfoBlock - builds a TableInfo tuple from a VariantMatrix
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros
#include "ExceptionMacros.h"

SWIG_STRINGMATRIX aqGridObjectDisplay( const std::string& objectName, const bool& displayColumnNames  )
{
	AQ_API_START
	auto result = validation::tryAqGridObjectDisplay( objectName );

    auto& columnNames = std::get<1>( result );
    auto& flexibleData = std::get<0>( result );


	// Marshall Output to Standard String Matrix
	SWIG_STRINGMATRIX resultsStringMatrix = swig::fromVariantMatrixToMatrixOfString( flexibleData );

    return resultsStringMatrix;

    AQ_API_END
}

// Each grid is arbitrary-shape; see the matching note in aqToolGrids.h.
#if (!defined(SWIG_R)) && (!defined(SWIGR))
/* @brief			swig interface for aqToolObjectMultiGridCreate. Create and store a multi-grid from up to three named grids.
*  @param [in]		objectName			Name for the multi-grid object
*  @param [in]		gridNames			The name of each grid within the multi-grid
*  @param [in]		grids				The grids, one string matrix per name
*  @param [in]		allowJaggedData		Optional. Default FALSE. Allow columns of differing length
*  @return			The multi-grid object handle
*/
std::string aqToolObjectMultiGridCreate( const std::string& objectName,
                                          const std::vector<std::string>& gridNames,
                                          const std::vector<SWIG_STRINGMATRIX>& grids,
                                          const bool allowJaggedData )
{
	AQ_API_START

	AQ_REQUIRE( gridNames.size() == grids.size(), "Number of grid names must match number of grids in aqToolObjectMultiGridCreate()." );

	// Marshall Inputs: each named grid becomes a "TableInfo" tuple (columnNames, columnTypes, data)
	std::vector<validation::TableInfo> infoBlocks;
	infoBlocks.reserve( grids.size() );
	for ( const SWIG_STRINGMATRIX& grid : grids )
	{
		etrading::VariantMatrix variantMatrix;
		swig::buildVariantMatrix( variantMatrix, grid );
		infoBlocks.push_back( etrading::JSONInfoBlock::createInfoBlock( variantMatrix ) );
	}

	// Call Function and Return Result
	std::string result = validation::tryAqToolObjectMultiGridCreate( objectName, gridNames, infoBlocks, allowJaggedData );
	return result;

	AQ_API_END
}
#endif

/* @brief			swig interface for aqToolObjectMultiGridDisplay. Display one named grid of a multi-grid as a matrix.
*  @param [in]		objectName		A multi-grid handle
*  @param [in]		gridName		The grid within the multi-grid to display
*  @return			A string matrix of the grid's data
*/
SWIG_STRINGMATRIX aqToolObjectMultiGridDisplay( const std::string& objectName, const std::string& gridName )
{
	AQ_API_START

	auto result = validation::tryAqToolObjectMultiGridDisplay( objectName, gridName );

	// result.first is a FlexibleData (== VariantMatrix); result.second is the column names, not returned here -
	// matches tryAqGridObjectDisplay's use of the same pair shape in aqGridObjectDisplay above
	auto& flexibleData = std::get<0>( result );

	SWIG_STRINGMATRIX resultsStringMatrix = swig::fromVariantMatrixToMatrixOfString( flexibleData );

	return resultsStringMatrix;

	AQ_API_END
}

/* @brief			swig interface for aqToolObjectMultiGridSubNames. The sub-grid names held by a multi-grid.
*  @param [in]		objectName		A multi-grid handle
*  @return			The sub-grid names
*/
std::vector<std::string> aqToolObjectMultiGridSubNames( const std::string& objectName )
{
	AQ_API_START

	std::vector<std::string> result = validation::tryAqToolObjectMultiGridSubNames( objectName );
	return result;

	AQ_API_END
}