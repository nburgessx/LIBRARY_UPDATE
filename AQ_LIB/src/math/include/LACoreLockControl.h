// LACoreLockControl.h

#pragma once

namespace common
{
    class LACoreLockControl
    {
    public:

        // Accessors
        // ---------

        // Get
        static bool enableThreadLocks();

        // Set
        static void enableThreadLocks( const bool enableThreadLocks );

    private:

        static bool enableThreadLocks_;

    };
}

