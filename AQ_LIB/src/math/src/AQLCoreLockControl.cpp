// AQLCoreLockControl.cpp

#include "AQLCoreLockControl.h"


// Initialize Static Variable Default(s)
// -------------------------------------

// DEFAULT: Enable Thread Locks by Default
bool common::AQLCoreLockControl::enableThreadLocks_ = true; 


namespace common
{

    // STATIC: Get the 'enableThreadLocks' parameter
    bool AQLCoreLockControl::enableThreadLocks()
    {
        return enableThreadLocks_;
    }


    // STATIC: Set the 'enableThreadLocks' parameter
    void AQLCoreLockControl::enableThreadLocks( const bool enableThreadLocks )
    {
        enableThreadLocks_ = enableThreadLocks;
    }

}
