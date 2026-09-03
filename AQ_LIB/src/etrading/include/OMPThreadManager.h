#pragma once

#include "Singleton.h"      // Singleton template
#include <omp.h>

// #Pragma Macro for OMP Parallelized Loop
// *** This macro must be preceeded with the #pragma keyword ***
#define MLIB_PARALLEL_FOR                                               omp parallel for num_threads( omp_get_max_threads() )

// #Pragma Macro for OMP Parallelized Loop with Shared Variable Addition
// *** This macro must be preceeded with the #pragma keyword ***
#define MLIB_PARALLEL_FOR_WITH_SHARED_VARIABLE( sharedVariable )        omp parallel for shared (sharedVariable) num_threads( omp_get_max_threads() )

// #Pragma Macro for OMP Parallelized Loop with Shared Variable Addition
// *** This macro must be preceeded with the #pragma keyword ***
#define MLIB_PARALLEL_FOR_WITH_REDUCTION_ADD( sharedVariable )          omp parallel for reduction (+:sharedVariable) num_threads( omp_get_max_threads() )

// #Pragma Macro for OMP Parallelized Loop with Shared Variable Subtraction
// *** This macro must be preceeded with the #pragma keyword ***
#define MLIB_PARALLEL_FOR_WITH_REDUCTION_SUBTRACT( sharedVariable )     omp parallel for reduction (-:sharedVariable) num_threads( omp_get_max_threads() )

// #Pragma Macro for OMP Parallelized Loop with Shared Variable Multiplication
// *** This macro must be preceeded with the #pragma keyword ***
#define MLIB_PARALLEL_FOR_WITH_REDUCTION_MULTIPLY( sharedVariable )     omp parallel for reduction (*:sharedVariable) num_threads( omp_get_max_threads() )

// #Pragma Macro for OMP Parallelized Loop with Shared Variable Division
// *** This macro must be preceeded with the #pragma keyword ***
#define MLIB_PARALLEL_FOR_WITH_REDUCTION_DIVIDE( sharedVariable )       omp parallel for reduction (/:sharedVariable) num_threads( omp_get_max_threads() )


namespace etrading
{
    //
    // OMPThreadManager, A singleton that holds the OMP Parallelization isOMPEnabled parameter, which is accessible via the getInstance() method - Uses lazy instantiation
    //
    class OMPThreadManager : public Singleton<OMPThreadManager>
    {

    public:

        // Check if OMP Parallization is Enabled
        bool getIsOMPEnabled() const                   { return isOMPEnabled_; }
        
        // Enable / Disable OMP Parallization
        void setIsOMPEnabled( const bool enable )      { isOMPEnabled_ = enable; }

    private:

        // Private Constructur - Singleton
        // Enable OMP Threading and Parallization by default
        // Note however in the Server API within exposed_functions.cpp->setUp() we disable OMP threading
        OMPThreadManager() : isOMPEnabled_( true ) {};
        
        // Singleton friend class
        friend Singleton<OMPThreadManager>;

        // Copy Constructor and Assignment Operators should never be called
        OMPThreadManager( const OMPThreadManager& ) = delete;
        OMPThreadManager & operator=( const OMPThreadManager& ) = delete;

        bool isOMPEnabled_;
    };


    //
    // Helper Methods to Enable/Disable Parallel Mode and Check the Status
    //

    // Short-Cut Helper Method to check Parallel Mode status
    bool getIsOMPEnabled();

    // Short-Cut Helper Method to enable/disable Parallel Mode
    void setIsOMPEnabled( const bool enable );

}