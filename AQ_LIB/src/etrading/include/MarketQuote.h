#pragma once


#include <vector>
#include <string>
#include <tuple>
#include <memory>

#include <boost/date_time.hpp>
#include <boost/regex.hpp>

#include "HasInstance.h"

#include "HasVariantMatrixAccess.h"
#include "CoreEnumerations.h"


namespace etrading
{

    class MarketQuote : public HasConstInstance<MarketQuoteTypeEnum>, 
		public HasVariantMatrixAccess,
		public CanGenerateDataSchema   
	{
    public:
        MarketQuote( const MarketQuoteTypeEnum mqt,
                     const CCY currency,
                     const CurveTenorEnum curveTenor );
        const CCY getCurrency() const;
        const CurveTenorEnum getCurveTenorEnum() const;
    private:
        const CCY ccy_;
        const CurveTenorEnum curveTenorEnum_;
		
    };

    class TermRateQuotes : public MarketQuote
    {
    public:
        TermRateQuotes(	const std::vector<std::string>& terms,
                        const std::vector<double>& rates,
                        const MarketQuoteTypeEnum mqt,
                        const CCY currency,
                        const CurveTenorEnum curveTenor );
        const std::vector<std::string>& getTerms() const;
        const std::vector<double>& getValues() const;
        virtual VariantMatrix getVariantMatrix() const;
    private:
        const std::vector<std::string> terms_;
        const std::vector<double> rates_;
    };

    struct StartEndDatesHolder
    {
        StartEndDatesHolder( const std::vector<boost::gregorian::date>& startDates,
                             const std::vector<boost::gregorian::date>& endDates );
        const std::vector<boost::gregorian::date>& getStartDates() const;
        const std::vector<boost::gregorian::date>& getEndDates() const;
    private:
        const std::vector<boost::gregorian::date> startDates_;
        const std::vector<boost::gregorian::date> endDates_;
    };

    struct SwapQuotes : public TermRateQuotes
    {
        SwapQuotes( const std::vector<std::string>& tenors,
                    const std::vector<double>& parRates,
                    const SwapQuoteTypeEnum swapType,
                    const CCY currency,
                    const CurveTenorEnum curveTenor );
        const SwapQuoteTypeEnum getSwapType() const;

        virtual VariantMatrix getVariantMatrix() const;
		virtual DataSchema generateDataSchema() const;
		static const std::string getSchemaName();
    private:
        const SwapQuoteTypeEnum swapType_;
		static const DataSchema SWAPQUOTES_SCHEMA;
    };

    struct BasisSwapQuotes : public SwapQuotes
    {
        BasisSwapQuotes( const std::vector<std::string>& tenors,
                         const std::vector<double>& parRates,
                         const CCY currency,
                         const CurveTenorEnum fromTenor,
                         const CurveTenorEnum toTenor );
        const CurveTenorEnum getFromTenor() const;
        const CurveTenorEnum getToTenor() const;
        virtual VariantMatrix getVariantMatrix() const;
		virtual DataSchema generateDataSchema() const;
		static const std::string getSchemaName();
    private:
        const CurveTenorEnum toTenor_;
		static const DataSchema BASISSWAPQUOTES_SCHEMA;
    };

    struct CentralBankSwapQuotes : public TermRateQuotes, public StartEndDatesHolder
    {
        CentralBankSwapQuotes(
            const std::vector<std::string>& tenors,
            const std::vector<double>& parRates,
            const std::vector<boost::gregorian::date>& startDates,
            const std::vector<boost::gregorian::date>& endDates,
            const CentralBankTypeEnum centralBankType,
            const CCY currency,
            const CurveTenorEnum curveTenor = CURVE_TENOR_1D );
        virtual VariantMatrix getVariantMatrix() const;
		virtual DataSchema generateDataSchema() const;
		static const std::string getSchemaName();
		const CentralBankTypeEnum getCentralBankType()  const;
    private:
        const CentralBankTypeEnum centralBankType_;
		static const DataSchema CENTRALBANKSWAPQUOTES_SCHEMA;
    };

    struct FRAQuotes : public TermRateQuotes
    {
        // the tenors need to be entered in DDxDD
        FRAQuotes( const std::vector<std::string>& tenors,
                   const std::vector<double>& fraRates,
                   const FRAPeriodEnum fraPeriod,
                   const CCY currency );

        virtual VariantMatrix getVariantMatrix() const;
		virtual DataSchema generateDataSchema() const;
		static const std::string getSchemaName();
    private:
        static const bool verifyTenors( const std::vector<std::string>& tenors );
        static const std::vector<boost::regex> FRA_TERM_REGEX;
		static const DataSchema FRAQUOTES_SCHEMA;
    };

    struct IRFuturesQuotes : public TermRateQuotes, public StartEndDatesHolder
    {
        IRFuturesQuotes(
            const std::vector<std::string>& termsTickers,
            const std::vector<double>& quotes,
            const std::vector<boost::gregorian::date>& startDates,
            const std::vector<boost::gregorian::date>& endDates,
            const CCY currency,
            const FRAPeriodEnum fraPeriod = _3M_FRAPERIOD,
			const std::vector<double> convexityModelParameters = std::vector<double>( 0 ));
        const std::vector<double>& getConvexityModelParameters() const;
        virtual VariantMatrix getVariantMatrix() const;
		virtual DataSchema generateDataSchema() const;
		static const std::string getSchemaName();
    private:
        std::vector<double> convexityModelParameters_;
		static const DataSchema IRFUTURESQUOTES_SCHEMA;
    };

	/*
    Term / Ticker	StartDate	EndDate	Rate	Volatility
    ED2 COMB Comdty	21-Sep-16	21-Dec-16	99.1700	0.900%
    ED3 COMB Comdty	21-Dec-16	15-Mar-17	99.0850	0.900%
    ED4  COMB Comdty	15-Mar-17	21-Jun-17	99.0250	0.900%
    ED5  COMB Comdty	21-Jun-17	20-Sep-17	98.9600	0.900%
    ED6  COMB Comdty	20-Sep-17	20-Dec-17	98.8850	0.900%
    ED7  COMB Comdty	20-Dec-17	21-Mar-18	98.8100	0.900%
    ED8  COMB Comdty	21-Mar-18	20-Jun-18	98.7500	0.900%
    ED9 COMB Comdty	20-Jun-18	19-Sep-18	98.6800	0.900%
    ED10  COMB Comdty	19-Sep-18	19-Dec-18	98.6150	0.900%
    ED11  COMB Comdty	19-Dec-18	20-Mar-19	98.5400	0.900%
    ED12  COMB Comdty	20-Mar-19	19-Jun-19	98.4850	0.900%
    */

    class FXQuotes : public TermRateQuotes
    {
    public:
        FXQuotes(	const std::vector<std::string>& tenors,
                    const std::vector<double>& exchangeRates,
                    const CCY currency,
                    const CCY unitCurrency = etrading::USD,
                    const bool isInvertedQuote = false );
        const CCY getAssetMoney() const; // denominator : Unit Currency
        const bool isInverted() const;
 
        virtual VariantMatrix getVariantMatrix() const;
		virtual DataSchema generateDataSchema() const;
		static const std::string getSchemaName();
    private:
        const bool isInverted_;
        const CCY unitsCurrency_;
		static const DataSchema FXQUOTES_SCHEMA;
    };

   /* std::tuple<std::unique_ptr<etrading::SwapQuotes>,std::unique_ptr<etrading::CentralBankSwapQuotes>> 
        getMarketQuotesFromOisConv(const LAStringMatrix& oisConv, const etrading::CCY ccy);*/

}
