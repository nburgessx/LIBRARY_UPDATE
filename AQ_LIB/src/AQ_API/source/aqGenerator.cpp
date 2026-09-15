#include "aqGenerator.h"
#include "tryAqGenerator.h"
#include "TypeUtilities.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


std::vector<std::string> aqGeneratorList( const std::string& typeAsString )
{
    AQ_API_START
    std::vector<std::string> result = validation::tryAqGeneratorList( typeAsString );
    return result;
    AQ_API_END
}


SWIG_STRINGMATRIX aqGeneratorDisplay( const std::string& typeAsString, const std::string& generatorName )
{
    AQ_API_START
    etrading::VariantMatrix result = validation::tryAqGeneratorDisplay( typeAsString, generatorName );
    SWIG_STRINGMATRIX resultsStringMatrix = swig::fromVariantMatrixToMatrixOfString( result );
    return resultsStringMatrix;
    AQ_API_END
}


std::string aqGeneratorValidate( const std::string& typeAsString, const std::string& generatorName )
{
    AQ_API_START
    std::string result = validation::tryAqGeneratorValidate( typeAsString, generatorName );
    return result;
    AQ_API_END
}
