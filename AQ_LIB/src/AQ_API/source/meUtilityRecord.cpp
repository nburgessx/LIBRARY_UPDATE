#include "LAString.h"
#include "TypeUtilities.h"

#include "meUtilityRecord.h"
#include "tryMeUtilityRecord.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			validation interface for the meUtilityVersion function
*  @param [in]		enable		True to enable the recording of tests
*  @param [in]		folder		Output folder
*  @param [in]		repeat		Allowed repeated tests
*  @param [in]		startIndex	Start index for repeated tests
*  @param [in]		maxIndex	Max index for repeated tests
*  @return			A notification string
*/
std::string meUtilityRecord( bool enable,
                             const std::string& folder,
                             bool repeat,
                             int startIndex,
                             int maxIndex )
{
    AQ_API_START
    
    // Input Marshalling 
    LAString myFolder( folder.c_str() );
        
    // Calculate Result
    LAString result = validation::tryMeUtilityRecord( enable, myFolder, repeat, startIndex, maxIndex );

    // Return as std::string
    return result.getCString();
    
    AQ_API_END
}
