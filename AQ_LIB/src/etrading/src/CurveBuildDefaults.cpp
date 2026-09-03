// CurveBuildDefaults.cpp

#include "CurveBuildDefaults.h"
#include <boost/thread/mutex.hpp>


// Initialize Static Variable Default(s)
// -------------------------------------

// Build Curves using Discount Factors by Default
bool etrading::CurveBuildDefaults::buildCurveUsingForwards_ = true; 


namespace etrading
{
    namespace 
    {
        boost::mutex parameterGetMutex;
        boost::mutex parameterSetMutex;
    }

    // STATIC: Get the 'BuildCurveUsingForwards' parameter
    bool CurveBuildDefaults::buildCurveUsingForwards()
    {
        // Make thread safe
        boost::mutex::scoped_lock getGuard( parameterGetMutex );
        return buildCurveUsingForwards_;
    }


    // STATIC: Set the 'BuildCurveUsingForwards' parameter
    void CurveBuildDefaults::buildCurveUsingForwards( const bool useForwards )
    {
        // Make thread safe
        boost::mutex::scoped_lock setGuard( parameterSetMutex );
        buildCurveUsingForwards_ = useForwards;
    }

}