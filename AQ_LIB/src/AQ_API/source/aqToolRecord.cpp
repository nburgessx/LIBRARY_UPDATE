#include "AQLString.h"
#include "TypeUtilities.h"

#include "aqToolRecord.h"
#include "tryAqToolRecord.h"
#include "tryAqToolReplay.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			validation interface for the aqToolVersion function
*  @param [in]		enable		True to enable the recording of tests
*  @param [in]		folder		Output folder
*  @param [in]		repeat		Allowed repeated tests
*  @param [in]		startIndex	Start index for repeated tests
*  @param [in]		maxIndex	Max index for repeated tests
*  @return			A notification string
*/
std::string aqToolRecord( bool enable,
                             const std::string& folder,
                             bool repeat,
                             int startIndex,
                             int maxIndex )
{
    AQ_API_START
    
    // Input Marshalling 
    AQLString myFolder( folder.c_str() );
        
    // Calculate Result
    AQLString result = validation::tryAqToolRecord( enable, myFolder, repeat, startIndex, maxIndex );

    // Return as std::string
    return result.getCString();

    AQ_API_END
}

/* @brief			validation interface for the aqToolReplay function. Replays a recorded input file
*                   back through the validation layer.
*  @param [in]		filepath		Full file path to the recorded test csv file
*  @return			The result string
*/
std::string aqToolReplay( const std::string& filepath )
{
    AQ_API_START

    // Input Marshalling
    AQLString myFilepath( filepath.c_str() );

    // Calculate Result
    AQLString result = validation::tryAqToolReplay( myFilepath );

    // Return as std::string
    return result.getCString();

    AQ_API_END
}
