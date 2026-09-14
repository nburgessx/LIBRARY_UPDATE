
#include "MarketQuote.h"
#include "Variant.h"
#include "ContainerUtilities.h"
#include "DataHolder.h"

namespace etrading
{
	/*static*/
	const DataSchema SwapQuotes::SWAPQUOTES_SCHEMA( 
		"SWAPQUOTES_SCHEMA",
		3,
		boost::assign::list_of(STRING_VALUE)(STRING_VALUE)(DOUBLE_VALUE),
		boost::assign::list_of("INFO")("MATURITY")("QUOTE"));

	const DataSchema BasisSwapQuotes::BASISSWAPQUOTES_SCHEMA( 
		"BASISSWAPQUOTES_SCHEMA",
		3,
		boost::assign::list_of(STRING_VALUE)(STRING_VALUE)(DOUBLE_VALUE),
		boost::assign::list_of("INFO")("MATURITY")("QUOTE"));

	const DataSchema CentralBankSwapQuotes::CENTRALBANKSWAPQUOTES_SCHEMA( 
		"CENTRALBANKSWAPQUOTES_SCHEMA",
		5,
		boost::assign::list_of(STRING_VALUE)(STRING_VALUE)(DOUBLE_VALUE)(DATE_VALUE)(DATE_VALUE),
		boost::assign::list_of("INFO")("TERM")("QUOTE")("FROMDATE")("TODATE"));

	const DataSchema FRAQuotes::FRAQUOTES_SCHEMA( 
		"FRAQUOTES_SCHEMA",
		3,
		boost::assign::list_of(STRING_VALUE)(STRING_VALUE)(DOUBLE_VALUE),
		boost::assign::list_of("INFO")("MATURITY")("QUOTE"));

	const DataSchema IRFuturesQuotes::IRFUTURESQUOTES_SCHEMA(
		"IRFUTURESQUOTES_SCHEMA",
		6,
		boost::assign::list_of(STRING_VALUE)(STRING_VALUE)(DOUBLE_VALUE)(DATE_VALUE)(DATE_VALUE)(DOUBLE_VALUE),
		boost::assign::list_of("INFO")("TERM")("QUOTE")("FROMDATE")("TODATE")("CONVEXITYADJ"));

	const DataSchema FXQuotes::FXQUOTES_SCHEMA(
		"FXQUOTES_SCHEMA",
		3,
		boost::assign::list_of(STRING_VALUE)(STRING_VALUE)(DOUBLE_VALUE),
		boost::assign::list_of("INFO")("MATURITY")("QUOTE"));

	const std::vector<boost::regex>FRAQuotes::FRA_TERM_REGEX = boost::assign::list_of( "(\\d{1,2})(X|x|BY|by|By)(\\d{1,2})" );

    const bool FRAQuotes::verifyTenors( const std::vector<std::string>& tenors )
    {
        for( auto counter = 0u; counter < tenors.size(); counter++ )
        {
            if( idxOfRegexMatch( tenors[counter], FRAQuotes::FRA_TERM_REGEX ) >= 0 )
            {
                // TODO: check digits on left and right for increase and match of the curve tenor
                return true; // if one of the formats is matched it is OK
            }
        }
        return false;
    };

	/* NON static */
    MarketQuote::MarketQuote( const MarketQuoteTypeEnum mdt, const CCY currency, const CurveTenorEnum curveTenor )
        : HasConstInstance<MarketQuoteTypeEnum>( mdt ), ccy_( currency ), curveTenorEnum_( curveTenor )
    {};

    const CCY MarketQuote::getCurrency() const
    {
        return ccy_;
    };

    const CurveTenorEnum MarketQuote::getCurveTenorEnum() const
    {
        return curveTenorEnum_;
    };

    TermRateQuotes::TermRateQuotes(	const std::vector<std::string>& terms,
                                    const std::vector<double>& rates,
                                    const MarketQuoteTypeEnum mdt,
                                    const CCY currency,
                                    const CurveTenorEnum curveTenor )
        :	terms_( terms ), rates_( rates ), MarketQuote( mdt, currency, curveTenor )
    {};

    const std::vector<std::string>& TermRateQuotes::getTerms() const
    {
        return terms_;
    };

    const std::vector<double>& TermRateQuotes::getValues() const
    {
        return rates_;
    };

    VariantMatrix TermRateQuotes::getVariantMatrix() const
    {
        VariantMatrix variantMatrix;
        variantMatrix.emplace_back( Variant::createVariantVector( this->getTerms(), etrading::STRING_VALUE ) );
        variantMatrix.emplace_back( Variant::createVariantVector( this->getValues(), etrading::DOUBLE_VALUE ) );
        return variantMatrix;
    };

    StartEndDatesHolder::StartEndDatesHolder(
        const std::vector<boost::gregorian::date>& startDates,
        const std::vector<boost::gregorian::date>& endDates ) : startDates_( startDates ), endDates_( endDates )
    {};

    const std::vector<boost::gregorian::date>& StartEndDatesHolder::getStartDates() const
    {
        return startDates_;
    };

    const std::vector<boost::gregorian::date>& StartEndDatesHolder::getEndDates() const
    {
        return endDates_;
    };

    SwapQuotes::SwapQuotes( const std::vector<std::string>& tenors,
                            const std::vector<double>& parRates,
                            const SwapQuoteTypeEnum swapType,
                            const CCY currency,
                            const CurveTenorEnum curveTenor )
        :	swapType_( swapType ), TermRateQuotes( tenors, parRates, etrading::IRS_SWAP, currency, curveTenor )
    {};

    const SwapQuoteTypeEnum SwapQuotes::getSwapType() const
    {
        return swapType_;
    };

    VariantMatrix SwapQuotes::getVariantMatrix() const
    {
		// we are making this correspond to SwapQuotes::SWAPQUOTES_SCHEMA;
		// first column: CCY, CurveTenor, mdType, SwapType // second & third = maturity,quote
        VariantMatrix variantMatrix;
		std::vector<std::string> infoData = boost::assign::list_of
			(etrading::toString(getCurrency()))
			(etrading::toString(getCurveTenorEnum()))
			(etrading::toString(getRefToInstance()))
			(etrading::toString(getSwapType()));
        variantMatrix.emplace_back( Variant::createVariantVector( infoData, etrading::STRING_VALUE ) );
		variantMatrix.emplace_back( Variant::createVariantVector( this->getTerms(), etrading::STRING_VALUE ) );
        variantMatrix.emplace_back( Variant::createVariantVector( this->getValues(), etrading::DOUBLE_VALUE ) );

        return variantMatrix;
    };

	DataSchema SwapQuotes::generateDataSchema() const
	{
		return SwapQuotes::SWAPQUOTES_SCHEMA;
	};

	/*static*/ const std::string SwapQuotes::getSchemaName()
	{
		return SwapQuotes::SWAPQUOTES_SCHEMA.getName();
	}

    BasisSwapQuotes::BasisSwapQuotes( const std::vector<std::string>& tenors,
                                      const std::vector<double>& parRates,
                                      const CCY currency,
                                      const CurveTenorEnum fromTenor,
                                      const CurveTenorEnum toTenor )
		: toTenor_( toTenor ), SwapQuotes( tenors, parRates, BASIS_SWAPTYPE, currency, fromTenor )
    {};

    const CurveTenorEnum BasisSwapQuotes::getFromTenor() const
    {
        return getCurveTenorEnum();
    };

    const CurveTenorEnum BasisSwapQuotes::getToTenor() const
    {
        return toTenor_;
    };

    VariantMatrix BasisSwapQuotes::getVariantMatrix() const
    {
		// we are making this correspond to SwapQuotes::BASISSWAPQUOTES_SCHEMA;
		// first column: CCY, CurveTenor, ToCurveTenor, mdType, SwapType // second & third = maturity,quote
        VariantMatrix variantMatrix;
		std::vector<std::string> infoData = boost::assign::list_of
			(etrading::toString(getCurrency()))
			(etrading::toString(getCurveTenorEnum()))
			(etrading::toString(getToTenor()))
			(etrading::toString(getRefToInstance()))
			(etrading::toString(getSwapType()));

        variantMatrix.emplace_back( Variant::createVariantVector( infoData, etrading::STRING_VALUE ) );
		variantMatrix.emplace_back( Variant::createVariantVector( this->getTerms(), etrading::STRING_VALUE ) );
        variantMatrix.emplace_back( Variant::createVariantVector( this->getValues(), etrading::DOUBLE_VALUE ) );

        return variantMatrix;
    };

	DataSchema BasisSwapQuotes::generateDataSchema() const
	{
		return BasisSwapQuotes::BASISSWAPQUOTES_SCHEMA;
	};

	/*static*/ const std::string BasisSwapQuotes::getSchemaName()
	{
		return BasisSwapQuotes::BASISSWAPQUOTES_SCHEMA.getName();
	};

    CentralBankSwapQuotes::CentralBankSwapQuotes(
        const std::vector<std::string>& tenors,
        const std::vector<double>& parRates,
        const std::vector<boost::gregorian::date>& startDates,
        const std::vector<boost::gregorian::date>& endDates,
        const CentralBankTypeEnum centralBankType,
        const CCY currency,
        const CurveTenorEnum curveTenor )
        :	centralBankType_( centralBankType ),
          StartEndDatesHolder( startDates, endDates ),
          TermRateQuotes( tenors, parRates, CENTRALBANK_SWAP, currency, curveTenor )
    {};


    VariantMatrix CentralBankSwapQuotes::getVariantMatrix() const
    {
		// we are making this correspond to SwapQuotes::CENTRALBANKSWAPQUOTES_SCHEMA;
		// first column: CCY, CurveTenor, mdType, CentralBankType // second & third = maturity,quote  // fourth & fifth = to- and fromDates
        VariantMatrix variantMatrix;
		std::vector<std::string> infoData = boost::assign::list_of
			(etrading::toString(getCurrency()))
			(etrading::toString(getCurveTenorEnum()))
			(etrading::toString(getRefToInstance()))
			(etrading::toString(getCentralBankType()));

        variantMatrix.emplace_back( Variant::createVariantVector( infoData, etrading::STRING_VALUE ) );
		variantMatrix.emplace_back( Variant::createVariantVector( this->getTerms(), etrading::STRING_VALUE ) );
        variantMatrix.emplace_back( Variant::createVariantVector( this->getValues(), etrading::DOUBLE_VALUE ) );
        variantMatrix.emplace_back( Variant::createVariantVector( this->getStartDates(), etrading::DATE_VALUE ) );
		variantMatrix.emplace_back( Variant::createVariantVector( this->getEndDates(), etrading::DATE_VALUE) );

        return variantMatrix;
    };

    const CentralBankTypeEnum CentralBankSwapQuotes::getCentralBankType()  const
    {
        return centralBankType_;
    };

	DataSchema CentralBankSwapQuotes::generateDataSchema() const
	{
		return CentralBankSwapQuotes::CENTRALBANKSWAPQUOTES_SCHEMA;
	};

	/*static*/ const std::string CentralBankSwapQuotes::getSchemaName()
	{
		return CentralBankSwapQuotes::CENTRALBANKSWAPQUOTES_SCHEMA.getName();
	};

    FRAQuotes::FRAQuotes( const std::vector<std::string>& tenors,
                          const std::vector<double>& fraRates,
                          const FRAPeriodEnum fraPeriod,
                          const CCY currency )
        :	TermRateQuotes( tenors, fraRates, FRA_QUOTE, currency, toCurveTenorEnum( fraPeriod ) )
    {
        if( !FRAQuotes::verifyTenors( tenors ) )
        {
            { std::ostringstream aqMsg146;
aqMsg146 << "Some FRA tenor string are not in a correct format (NNxNN), check:  " << containerAsString( tenors ); AQ_THROW( aqMsg146.str() ); }
        }
        // TODO: C++11 std::any usage
        // if(!std::any(tenors.begin(), tenors.end(), [&](const std::string& inputTenor) { return (idxOfRegexMatch(inputTenor, FRAQuotes::FRA_TERM_REGEX)>=0);   } ))
        // { throw ETradingException( (boost::format("Some FRA tenor string are not in a correct format (NNxNN), check:  %s") % containerAsString(tenors)).str() );  }
    };


    VariantMatrix FRAQuotes::getVariantMatrix() const
    {

		// we are making this correspond to SwapQuotes::FRAQUOTES_SCHEMA;
		// first column: CCY, CurveTenor (same as FRAPeriod), mdType // second & third = maturity,quote
        VariantMatrix variantMatrix;
		std::vector<std::string> infoData = boost::assign::list_of
			(etrading::toString(getCurrency()))
			(etrading::toString(getCurveTenorEnum()))
			(etrading::toString(getRefToInstance()));

        variantMatrix.emplace_back( Variant::createVariantVector( infoData, etrading::STRING_VALUE ) );
		variantMatrix.emplace_back( Variant::createVariantVector( this->getTerms(), etrading::STRING_VALUE ) );
        variantMatrix.emplace_back( Variant::createVariantVector( this->getValues(), etrading::DOUBLE_VALUE ) );

        return variantMatrix;
	};

	DataSchema FRAQuotes::generateDataSchema() const
	{
		return FRAQuotes::FRAQUOTES_SCHEMA;
	};

	/*static*/ const std::string FRAQuotes::getSchemaName()
	{
		return FRAQuotes::FRAQUOTES_SCHEMA.getName();
	};
	
    IRFuturesQuotes::IRFuturesQuotes(
        const std::vector<std::string>& termsTickers,
        const std::vector<double>& quotes,
        const std::vector<boost::gregorian::date>& startDates,
        const std::vector<boost::gregorian::date>& endDates,
        const CCY currency,
        const FRAPeriodEnum fraPeriod,
		const std::vector<double> convexityModelParameters) :
        convexityModelParameters_( convexityModelParameters ),
        StartEndDatesHolder( startDates, endDates ),
        TermRateQuotes( termsTickers, quotes, IR_FUTURES, currency, toCurveTenorEnum( fraPeriod ) )
    {};


    VariantMatrix IRFuturesQuotes::getVariantMatrix() const
    {

		// we are making this correspond to SwapQuotes::IRFUTURESQUOTES_SCHEMA;
		// first column: CCY, CurveTenor (same as IR Period), mdType // second & third = termtickers,quote  // fourth & fifth = to- and fromDates // sixth=convextiyModelParams
        VariantMatrix variantMatrix;
		std::vector<std::string> infoData = boost::assign::list_of
			(etrading::toString(getCurrency()))
			(etrading::toString(getCurveTenorEnum()))
			(etrading::toString(getRefToInstance()));

		variantMatrix.emplace_back( Variant::createVariantVector( infoData, etrading::STRING_VALUE ) );
		variantMatrix.emplace_back( Variant::createVariantVector( this->getTerms(), etrading::STRING_VALUE ) );
        variantMatrix.emplace_back( Variant::createVariantVector( this->getValues(), etrading::DOUBLE_VALUE ) );
        variantMatrix.emplace_back( Variant::createVariantVector( this->getStartDates(), etrading::DATE_VALUE ) );
		variantMatrix.emplace_back( Variant::createVariantVector( this->getEndDates(), etrading::DATE_VALUE) );
		variantMatrix.emplace_back( Variant::createVariantVector( this->getConvexityModelParameters(), etrading::DOUBLE_VALUE) );
        return variantMatrix;
    };

	const std::vector<double>& IRFuturesQuotes::getConvexityModelParameters() const
	{
		return convexityModelParameters_;		
	};

	DataSchema IRFuturesQuotes::generateDataSchema() const
	{
		return IRFuturesQuotes::IRFUTURESQUOTES_SCHEMA;
	};

	/*static*/ const std::string IRFuturesQuotes::getSchemaName()
	{
		return IRFuturesQuotes::IRFUTURESQUOTES_SCHEMA.getName();
	};

    FXQuotes::FXQuotes(	const std::vector<std::string>& terms,
                        const std::vector<double>& exchangeRates,
                        const CCY currency,
                        const CCY unitCurrency,
                        const bool isInvertedQuote )
        : TermRateQuotes( terms, exchangeRates, FXRATES, currency, CURVE_TENOR_1D ), // a curve tenor of 1D is OIS which is closest to spot
          isInverted_( isInvertedQuote ), unitsCurrency_( unitCurrency )
    {};

    const CCY FXQuotes::getAssetMoney() const
    {
        return unitsCurrency_;
    };

    const bool FXQuotes::isInverted() const
    {
        return isInverted_;
    };

    VariantMatrix FXQuotes::getVariantMatrix() const
    {
		// we are making this correspond to SwapQuotes::FXQUOTES_SCHEMA;
		// first column: CCY, unitsCCY, isInverted, CurveTenor, mdType // second & third = tenors,exchangerates 
        VariantMatrix variantMatrix;
		std::vector<std::string> infoData = boost::assign::list_of  
			(etrading::toString(getCurrency()))
			(etrading::toString(getAssetMoney()))
			(isInverted() ? "TRUE" : "FALSE")
			(etrading::toString(getCurveTenorEnum()))
			(etrading::toString(getRefToInstance()));
		variantMatrix.emplace_back( Variant::createVariantVector( infoData, etrading::STRING_VALUE ) );
		variantMatrix.emplace_back( Variant::createVariantVector( this->getTerms(), etrading::STRING_VALUE ) );
        variantMatrix.emplace_back( Variant::createVariantVector( this->getValues(), etrading::DOUBLE_VALUE ) );
        return variantMatrix;
    };

	DataSchema FXQuotes::generateDataSchema() const
	{
		return FXQuotes::FXQUOTES_SCHEMA;
	};

	/*static*/ const std::string FXQuotes::getSchemaName()
	{
		return FXQuotes::FXQUOTES_SCHEMA.getName();
	};

    /*
    std::tuple<std::unique_ptr<etrading::SwapQuotes>,std::unique_ptr<etrading::CentralBankSwapQuotes>> 
        getMarketQuotesFromOisConv(const AQLStringMatrix& oisConv, const etrading::CCY ccy)
    {
        if(oisConv.size() < 2)                    
        {
            throw ETradingException( "There are less than 2 columns in the OIS inputs, need at least 2" );
        }        
        if(oisConv.size() == 2)
        {
            auto oisTenorStrings = etrading::createStringVector(oisConv[0]);
            auto oisSwapValues = etrading::createDoubleVector(oisConv[1]);
            std::unique_ptr<etrading::SwapQuotes> ptrOisSwaps(new SwapQuotes(oisTenorStrings,oisSwapValues, etrading::OIS_SWAPTYPE, ccy, etrading::_1D));
            return std::make_tuple(std::move(ptrOisSwaps),nullptr);
        }
        else
        {
            const bool checkInclusion = (oisConv.size() >= 5);
            std::vector<std::string> centralBackSwapTenorStrings;
            std::vector<double> centralBackSwapParRates;
            std::vector<boost::gregorian::date> startDates;
            std::vector<boost::gregorian::date> endDates;
            for(unsigned int rc=0; rc++; rc < oisConv[0].size())
            {
                //std::string tenorStringDescription = oisConv[0][rc].getCString();
                //if(!etrading::canBeTenorString())
                //{
                //    centralBackSwapTenorStrings.emplace_back(tenorStringDescription);
                //    centralBackSwapParRates.emplace_back(oisConv[1][rc].getDoubleValue());
                //    startDates.emplace_back( etrading::toGregorianDateFromExcelDate( oisConv[2][rc].getIntValue()) );
                //    endDates.emplace_back( etrading::toGregorianDateFromExcelDate( oisConv[3][rc].getIntValue()) );
                //}
            }
        }
            
        }
        */
    
    
    }


