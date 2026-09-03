#include "BondQuote.h"
#include "ExceptionMacros.h"

namespace etrading
{
    // Function to convert a Raw Bond Price to Formatted Bond Quote
    double convertPriceToQuote( const double& rawPrice, const BondQuoteConventionEnum& quoteConvention )
    {
        double quote = 0.0;

        switch ( quoteConvention )
        {
            case NO_BOND_QUOTE_CONVENTION:
                quote = rawPrice;
                break;
            case QUOTE_IN_32NDS:
                quote = std::round(rawPrice*32)/32.0;
                break;
            case QUOTE_IN_64THS:
                quote = std::round(rawPrice*64)/64.0;
                break;
            case QUOTE_TO_1_DECIMAL_PLACE:
                quote = std::round(rawPrice*10)/10.0;
                break;
            case QUOTE_TO_2_DECIMAL_PLACES:
                quote = std::round(rawPrice*100)/100.0;
                break;
            case QUOTE_TO_3_DECIMAL_PLACES:
                quote = std::round(rawPrice*1000)/1000.0;
                break;
            case QUOTE_TO_4_DECIMAL_PLACES:
                quote = std::round(rawPrice*10000)/10000.0;
                break;
            case QUOTE_TO_5_DECIMAL_PLACES:
                quote = std::round(rawPrice*100000)/100000.0;
                break;
            default:
                MLIB_THROW("Invalid QuoteConvention Specified")
                break;
        }

        return quote;
    }

    // Function to convert a Raw Bond Prices to Formatted Bond Quotes
    std::vector<double> convertPricesToQuotes( const std::vector<double>& rawPrices, const BondQuoteConventionEnum& quoteConvention )
    {
        MLIB_REQUIRE( rawPrices.size() > 0, "Unable to Convert Prices to Quotes: No Prices Provided." )

        std::vector<double> quotes( rawPrices.size() );
        for ( size_t i = 0; i < rawPrices.size(); ++i )
        {
            quotes[i] = convertPriceToQuote( rawPrices[i], quoteConvention );
        }
        return quotes;
    }
}