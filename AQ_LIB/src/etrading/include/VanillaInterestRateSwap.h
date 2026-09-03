/*
 * @brief			Class that defines a vanilla interest rate swap instrument
 * @Created:		21 May 2016
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "LabelValueBlock.h"
#include "InterestRateSwap.h"

class LADate;

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
