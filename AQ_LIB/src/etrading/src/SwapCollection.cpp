/*
 * @brief			Class that carries multiple swaps
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "SwapCollection.h"

namespace etrading
{
    /* @brief		Default constructor
    */
    SwapCollection::SwapCollection()
    {
        swaps_.clear();
    }

    /* @brief		Constructor
    */
    SwapCollection::SwapCollection( const std::vector<SwapPtr>& portfolio )
    {
        swaps_ = portfolio;
    }

    /* @brief		Copy Constructor
    */
    SwapCollection::SwapCollection( const SwapCollection& rhs )
    {
        swaps_	= rhs.swaps_;
    }

    /* @brief		Method that returns Leg PV
    *  @return		instrument PV
    */
    double SwapCollection::pv(const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames) const
    {
        double ret( 0.0 );

        for ( size_t i = 0; i < swaps_.size(); ++i )
        {
            ret += swaps_[i]->pv(valuationSettingsLVB, fixingTableNames, "");
        }
        return ret;
    }

    /* @brief		Method that returns Leg PV01
    *  @return		instrument PV01
    */
    double SwapCollection::pv01(const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames) const
    {
        double ret( 0.0 );
        for ( size_t i = 0; i < swaps_.size(); ++i )
        {
            ret += swaps_[i]->pv01(valuationSettingsLVB, fixingTableNames);
        }
        return ret;
    }

}

