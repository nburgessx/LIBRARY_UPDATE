#pragma once

#include "LabelValueBlock.h"
#include "InterestRateSwap.h"

class LADate;

namespace etrading
{
    /* @brief		Interest rate swap instrument
    */
    class OISSwap : public InterestRateSwap
    {
    public:

        OISSwap( const LabelValueBlock& swapLVB );

        /* @brief		Method that returns swap PV
        *  @return		instrument PV
        */
        virtual double pv();

    private:

        LAString compoundingMethod_;
        bool isYieldCurveReady_;
        bool isFwdInterp_;

    };


}
