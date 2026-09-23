// aqGridObject.cpp

#include "aqGridObject.h"
#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"          // Swig Marshalling Helper Methods
#include "Variant.h"                // Variant and Variant Matrix Types
#include "JSONInfoBlock.h"          // JSONInfoBlock::createInfoBlock - builds a TableInfo tuple from a VariantMatrix
#include "tryAqToolGrid.h"          // validation::tryAqGridObject...
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros

// data is arbitrary-shape; see the matching note in aqGridObject.h.
#if (!defined(SWIG_R)) && (!defined(SWIGR))
std::string aqGridObjectCreate( const std::string& objectName,
                                 const SWIG_STRINGMATRIX& data,
                                 const bool allowJaggedData )
{
    AQ_API_START

    // Marshall Inputs: build a "TableInfo" tuple (columnNames, columnTypes, data) from the range,
    // the same conversion aqToolObjectMultiGridCreate uses per named grid.
    etrading::VariantMatrix variantMatrix;
    swig::buildVariantMatrix( variantMatrix, data );
    validation::TableInfo tableInfo = etrading::JSONInfoBlock::createInfoBlock( variantMatrix );

    // Call Function and Return Result
    std::string result = validation::tryAqGridObjectCreate( objectName, tableInfo, allowJaggedData );
    return result;

    AQ_API_END
}
#endif

std::string aqGridObjectSave( const std::string& objectName, const std::string& fileNameToWriteTo )
{
    AQ_API_START

    std::string result = validation::tryAqGridObjectSave( objectName, fileNameToWriteTo );
    return result;

    AQ_API_END
}

std::string aqGridObjectLoad( const std::string& fileName )
{
    AQ_API_START

    std::pair<const bool, std::string> result = validation::tryAqGridObjectLoad( fileName );
    return result.second;

    AQ_API_END
}

std::vector<std::string> aqGridObjectNames()
{
    AQ_API_START

    std::vector<std::string> result = validation::tryAqGridObjectNames();
    return result;

    AQ_API_END
}

bool aqGridObjectClearOne( const std::string& objectName )
{
    AQ_API_START

    bool result = validation::tryAqGridObjectClearOne( objectName );
    return result;

    AQ_API_END
}

bool aqGridObjectClearAll()
{
    AQ_API_START

    bool result = validation::tryAqGridObjectClearAll();
    return result;

    AQ_API_END
}
