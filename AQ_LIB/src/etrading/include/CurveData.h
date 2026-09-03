#pragma once

#include <memory>
#include <map>
#include <algorithm>

#include <boost/type_traits.hpp>

#include "MarketQuote.h"
#include "Cacheable.h"
#include "CoreEnumerations.h"
#include "ETradingException.h"
#include "IsLWOObject.h"
#include "HasVariantMatrixAccess.h"
#include "SchemaObject.h"
#include "AQLCoreTemplateType.h"



namespace etrading
{

    class CurveData : 
        public IsLWOObject, 
        public HasConstInstance<CurveTenorEnum>, 
        public HasVariantMatrixAccess
    {
    public:
        CurveData( const std::string& collectionName, const CurveTenorEnum curveTenorEnum );
        std::shared_ptr<MarketQuote> getMarketQuotes( const MarketQuoteTypeEnum mdtype ) const;
        void addMarketQuotes( std::unique_ptr<MarketQuote>&& ptrMktData );
        void setFRAQuotes( FRAQuotes&& fraData );
        void setSwapQuotes( SwapQuotes&& swapData );
        
        void setIRFuturesQuotes( IRFuturesQuotes&& futuresData );
        void setFXQuotes( FXQuotes&& fxData );
		void setCBSwapQuotes( CentralBankSwapQuotes&& cbSwapData );
		virtual const SchemaObject toSchemaObject() const;
        virtual VariantMatrix getVariantMatrix() const;

        static CurveData createFromValidationInput(
            const std::string& mdcName, 
            const etrading::CurveTenorEnum curveTenorEnum,
            const AQLStringMatrix& oisRates, 
            const AQLStringMatrix& liborOisBasisRates, 
            const AQLStringMatrix& swapRates);
    private:
		const bool isCorrectTenor(const MarketQuote&) const;
        void addQuotesToSchemaObject(const std::shared_ptr<MarketQuote>&,SchemaObject&) const;
        void addQuotesToVariantMatrix(const std::shared_ptr<MarketQuote>& quote,VariantMatrix& variantMatrix, const bool transpose = true) const;

        std::map<MarketQuoteTypeEnum, std::shared_ptr<MarketQuote>> marketDataCollection_; // consider unique_ptr but then copy on access...
        std::shared_ptr<SwapQuotes> oisSwapQuotes_;  
        std::shared_ptr<BasisSwapQuotes> basisSwapQuotes_;  


    };

}

