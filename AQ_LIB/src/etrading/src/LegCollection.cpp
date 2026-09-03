#include "LegCollection.h"
#include "ParameterValidation.h"
#include "SettingsValidation.h"

namespace etrading
{
    /* @brief		Default constructor
    */
    LegCollection::LegCollection()
    {
        legs_.clear();
    }

    /* @brief		Constructor
    */
    LegCollection::LegCollection( const std::vector<LegPtr>& legs )
    {
        legs_ = legs;
    }

    /* @brief		Copy Constructor
    */
    LegCollection::LegCollection( const LegCollection& rhs )
    {
        legs_	= rhs.legs_;
    }

    /* @brief		Add a leg to the collection
    */
    void LegCollection::add( const LegPtr& leg )
    {
        legs_.push_back( leg );
    }

    /* @brief		Remove a leg to the collection
    */
    void LegCollection::remove( const LegPtr& leg )
    {
        legs_.erase(std::remove(legs_.begin(), legs_.end(), leg), legs_.end());
    }


    /* @brief		Method that returns leg based on index
    *  @return		leg
    */
    LegPtr LegCollection::get(size_t t ) const
    {
        return legs_.at(t);
    }

    /* @brief		Method that returns the size of legs
    *  @return		the size of legs
    */
    size_t LegCollection::size() const
    {
        return legs_.size();
    }

    /* @brief		Method that returns legs' PV
    *  @return		legs PV
    */
    double LegCollection::pv(const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames) const
    {
        double ret( 0.0 );
        for ( size_t i = 0; i < legs_.size(); ++i )
        {
            auto leg = legs_[i];

			DataProvider dataProvider(ValuationSettings(valuationSettingsLVB, fixingTableNames, leg->getLegName()));

			ret += leg->pv(dataProvider, false);
		}
        return ret;
    }

    /* @brief		Method that returns legs' annuity with notional
    *  @return		legs' annuity with notional
    */
    double LegCollection::annuityWithNotional(const LabelValueBlock& valuationSettingsLVB) const
    {
        double ret( 0.0 );
        for ( size_t i = 0; i < legs_.size(); ++i )
        {
            auto leg = legs_[i];

			DataProvider dataProvider(ValuationSettings(valuationSettingsLVB, {}, leg->getLegName()));

			ret += leg->annuityWithNotional(dataProvider);
        }
        return ret;
    }

    /* @brief		Method that returns True if the legName exits in legs
    *  @return		True if the legName exits
    */
    bool LegCollection::exists(const LAString& legName) const
    {
       //Check if legName exits
        StringSet legNames;
        for (size_t i = 0; i < legs_.size(); ++i)
        {
            auto leg = get(i);
            auto legNm = leg->getLegName();
            legNames.insert(legNm.toUpper());
        }

        LAString name  = legName; 
        if (legNames.find(name.toUpper()) == legNames.end())  
        {
            return false;
        }
        return true;
    }

    /* @brief		Method that returns the leg with matching legName
    *  @return		The leg with matching legName
    */
    LegPtr LegCollection::findLegByName(const LAString& legName) const
    {
        for (size_t i = 0; i < legs_.size(); ++i)
        {
            auto leg = get(i);
            if (same(leg->getLegName(), legName))
            {
                return leg;
            }
        }
		return LegPtr();
    }

    /* @brief		Method that returns True if the currency exits in legs
    *  @return		True if the legCurrency exits
    */
    bool LegCollection::currencyExists(const CCY& legCurrency) const
    {
        std::set<int> legCurrencies;
        for (size_t i = 0; i < legs_.size(); ++i)
        {
            auto leg = get(i);
            auto legCcy = leg->getStaticData()->getCurrency();
            legCurrencies.insert( legCcy );
        }

        if (legCurrencies.find(legCurrency) == legCurrencies.end())  
        {
            return false;
        }
        return true;
    }

    /* @brief		Method that returns True if every leg has the same currency and false otherwise
    */
    bool LegCollection::isSingleCurrency() const
    {
        bool isSingleCurrency = true;
        etrading::CCY tempCurrency;
        
        // Iterate over every leg and check if the currency is the same
        for (size_t i = 0; i < legs_.size(); ++i)
        {
            if ( i==0 )
            {
                tempCurrency = get(i)->getStaticData()->getCurrency();;
            }
            else 
            {
                etrading::CCY legCurrency = get(i)->getStaticData()->getCurrency();
                
                // If we identify that one trade leg has a different currency exit early and return isSingleCurrency = false
                if ( legCurrency != tempCurrency )
                {
                    return false;
                }

                tempCurrency = legCurrency;
            }
        }

        return isSingleCurrency;
    }

}



