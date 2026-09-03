#pragma once

#include "Leg.h"

namespace etrading
{
    /* @brief		Interest rate Leg
    */
    class LegCollection
    {
    public:

        LegCollection();

        /* @brief		Constructor
        */
        LegCollection( const std::vector<LegPtr>& legs );

        /* @brief		Copy Constructor
        */
        LegCollection( const LegCollection& rhs );

        /* @brief		Add a leg to the collection
        */
        void add( const LegPtr& leg );

        /* @brief		Remove a leg to the collection
        */
        void remove( const LegPtr& leg );

        /* @brief		Method that returns leg based on index
        *  @return		leg
        */
        LegPtr get( size_t t ) const;

        /* @brief		Method that returns the size of legs
        *  @return		the size of legs
        */
        size_t size() const;

        /* @brief		Method that returns legs' PV
        *  @return		legs PV
        */
        double pv(const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames) const;

        /* @brief		Method that returns legs' annuity with notional
        *  @return		legs' annuity with notional
        */
        double annuityWithNotional(const LabelValueBlock& valuationSettingsLVB) const;

        /* @brief		Method that returns True if the legType exits in legs
        *  @return		True if the legType exits
        */
        bool exists(const LAString& legType) const;

        /* @brief		Method that returns the leg with matching legName
        *  @return		The leg with matching legName
        */
        LegPtr findLegByName(const LAString& legName) const;

        /* @brief		Method that returns True if the currency exits in legs
        *  @return		True if the legCurrency exits
        */
        bool currencyExists(const CCY& legCurrency) const;

        /* @brief		Method that returns True if every leg has the same currency and false otherwise
        */
        bool isSingleCurrency() const;


    private:
        // Legs
        std::vector<LegPtr> legs_;
    };


}
