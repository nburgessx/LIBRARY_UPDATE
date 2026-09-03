// LACoreLockControl.cpp

#include "LACoreLockControl.h"


// Initialize Static Variable Default(s)
// -------------------------------------

// DEFAULT: Enable Thread Locks by Default
bool common::LACoreLockControl::enableThreadLocks_ = true; 


namespace common
{

    // STATIC: Get the 'enableThreadLocks' parameter
    bool LACoreLockControl::enableThreadLocks()
    {
        return enableThreadLocks_;
    }


    // STATIC: Set the 'enableThreadLocks' parameter
    void LACoreLockControl::enableThreadLocks( const bool enableThreadLocks )
    {
        enableThreadLocks_ = enableThreadLocks;
    }

}
