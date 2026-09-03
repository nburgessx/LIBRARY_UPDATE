// CurveBuildDefaults.h
#pragma once

namespace etrading
{
    class CurveBuildDefaults
    {
    public:

        // Accessors
        // ---------

        // Get
        static bool buildCurveUsingForwards();

        // Set
        static void buildCurveUsingForwards( const bool useForwards );

    private:

        // This flag maps directly to the yield curve 'useFwdInterp' parameter
        // Note: True = Use Forwards to build curve and False = Use Discount Factors to build curve
        static bool buildCurveUsingForwards_;

    };
}

