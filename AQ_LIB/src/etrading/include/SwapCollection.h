/*
 * @brief			Class that carries multiple swaps
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "Swap.h"

namespace etrading
{
    /* @brief		Interest rate swap
    */
    class SwapCollection
    {
    public:

        SwapCollection();

        /* @brief		Constructor
        */
        SwapCollection( const std::vector<SwapPtr>& portfolio );

        /* @brief		Copy Constructor
        */
        SwapCollection( const SwapCollection& rhs );

        /* @brief		Method that returns swaps PV
        *  @return		instrument PV
        */
        double pv(const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames) const;

        /* @brief		Method that returns swaps PV01
        *  @return		instrument PV01
        */
        double pv01(const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames) const;

    private:
        // Swaps
        std::vector<SwapPtr> swaps_;
    };


}
