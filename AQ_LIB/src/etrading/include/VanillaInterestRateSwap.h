#pragma once

#include "LabelValueBlock.h"
#include "InterestRateSwap.h"

class AQLDate;

namespace etrading
{
    /* @brief		Interest rate swap instrument
    */
    class VanillaInterestRateSwap : public InterestRateSwap
    {
    public:

        VanillaInterestRateSwap( const LabelValueBlock& swapLVB );

        /* @brief		Method that returns swap PV
        *  @return		instrument PV
        */
        virtual double pv();

    private:

        bool isYieldCurveReady_;
        bool isFwdInterp_;

    };


}
