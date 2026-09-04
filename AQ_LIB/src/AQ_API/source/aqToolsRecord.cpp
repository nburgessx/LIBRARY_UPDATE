#include "AQLString.h"
#include "TypeUtilities.h"

#include "aqToolsRecord.h"
#include "tryAqToolsRecord.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			validation interface for the aqToolsVersion function
*  @param [in]		enable		True to enable the recording of tests
*  @param [in]		folder		Output folder
*  @param [in]		repeat		Allowed repeated tests
*  @param [in]		startIndex	Start index for repeated tests
*  @param [in]		maxIndex	Max index for repeated tests
*  @return			A notification string
*/
std::string aqToolsRecord( bool enable,
                             const std::string& folder,
                             bool repeat,
                             int startIndex,
                             int maxIndex )
{
    AQ_API_START
    
    // Input Marshalling 
    AQLString myFolder( folder.c_str() );
        
    // Calculate Result
    AQLString result = validation::tryAqToolsRecord( enable, myFolder, repeat, startIndex, maxIndex );

    // Return as std::string
    return result.getCString();
    
    AQ_API_END
}
