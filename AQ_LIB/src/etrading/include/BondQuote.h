#pragma once
#include "BondEnumerations.h"
#include <vector>

namespace etrading
{
    // Function to convert a Raw Bond Price to Formatted Bond Quote
    double convertPriceToQuote( const double& rawPrice, const BondQuoteConventionEnum& quoteConvention );

    // Function to convert a Raw Bond Prices to Formatted Bond Quotes
    std::vector<double> convertPricesToQuotes( const std::vector<double>& rawPrices, const BondQuoteConventionEnum& quoteConvention );
}
