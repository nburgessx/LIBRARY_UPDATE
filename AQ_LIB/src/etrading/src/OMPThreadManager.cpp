#include "OMPThreadManager.h"

namespace etrading
{
    // Short-Cut Helper Method to check Parallel Mode status
    bool getIsOMPEnabled()
    { 
        return OMPThreadManager::getInstance().getIsOMPEnabled();
    }

    // Short-Cut Helper Method to enable/disable Parallel Mode
    void setIsOMPEnabled( const bool enable ) 
    {
        OMPThreadManager::getInstance().setIsOMPEnabled( enable );
    }

}