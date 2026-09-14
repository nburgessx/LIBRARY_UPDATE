

#include "CurveData.h"
#include "CoreEnumerations.h"
#include "SchemaObject.h"
#include "ScheduleValidation.h"
#include "Variant.h"

namespace etrading
{

    CurveData::CurveData( const std::string& collectionName, const CurveTenorEnum curveTenorEnum )
        : etrading::HasConstInstance<CurveTenorEnum>( curveTenorEnum ), IsAQObject( collectionName, CURVE_DATA ), oisSwapQuotes_(), basisSwapQuotes_()
    {}
    
    std::shared_ptr<MarketQuote> CurveData::getMarketQuotes( const MarketQuoteTypeEnum mdtype ) const
    {
        // if(mdtype == etrading::BASIS_SWAPTYPE) <-- Wrong enum type
		// { 
        //     return basisSwapQuotes_;
		// }
		// 
        // if(mdtype == etrading::OIS_SWAPTYPE) <-- Wrong enum type
		// {
        //     return oisSwapQuotes_;
		// }

        auto iteratorToTheData = marketDataCollection_.find( mdtype );
        if( iteratorToTheData == marketDataCollection_.end() )
        {
			return {};
        }
        else
        {
            return iteratorToTheData->second;
        }
    };

    void CurveData::addMarketQuotes( std::unique_ptr<MarketQuote>&& ptrMktData )
    {
        if( ptrMktData != nullptr )
        {
            const auto& typeOfQuote =  ptrMktData.get()->getRefToInstance();
            switch( typeOfQuote ) // better than a dynamic cast  or a virtual with a copy
            {
                case FRA_QUOTE:
                {
                    FRAQuotes fraToSet = *( dynamic_cast<FRAQuotes*>( ptrMktData.release() ));
					setFRAQuotes(std::move(fraToSet));
                }
                break;
                case CENTRALBANK_SWAP:
                {
					CentralBankSwapQuotes cbSwapToSet = *( dynamic_cast<CentralBankSwapQuotes*>( ptrMktData.release() ) );
					setCBSwapQuotes(std::move(cbSwapToSet));
                }
                break;
                case IRS_SWAP:
                {
                    SwapQuotes* ptrSwapData = dynamic_cast<SwapQuotes*>( ptrMktData.get() );
                    setSwapQuotes(std::move(*ptrSwapData));
                    /*
                    if( ptrSwapData->getSwapType() == BASIS_SWAPTYPE )
                    {
                        BasisSwapQuotes cbSwapToSet = *( dynamic_cast<BasisSwapQuotes*>( ptrMktData.release() ) );
						setSwapQuotes(std::move(cbSwapToSet));
                    }
                    else
                    {
                        SwapQuotes cbSwapToSet = *( dynamic_cast<SwapQuotes*>( ptrMktData.release() ) );
						setSwapQuotes(std::move(cbSwapToSet));
                    }
                    */
                }
                break;
                case IR_FUTURES:
                {
                    IRFuturesQuotes irFuturesToSet = *( dynamic_cast<IRFuturesQuotes*>( ptrMktData.release() ) );
					setIRFuturesQuotes(std::move(irFuturesToSet));
                }
				break;
                case FXRATES:
                {
                    FXQuotes fxQuotesToSet = *( dynamic_cast<FXQuotes*>( ptrMktData.release() ) );
					setFXQuotes(std::move(fxQuotesToSet));
                }
                break;
                default:
                    { std::ostringstream aqMsg90;
aqMsg90 << "MarketQuote of type " << toString( typeOfQuote ) << " is not accepted in a MarketDataColleciton"; AQ_THROW( aqMsg90.str() ); }
                    break;
            }
        }
    };

    void CurveData::setFRAQuotes( FRAQuotes&& fraData )
    {
		if(isCorrectTenor(fraData))
		{
			marketDataCollection_[ FRA_QUOTE ] = std::make_shared<FRAQuotes>( fraData );
		} 
		else
		{
			{ std::ostringstream aqMsg91;
aqMsg91 << "FRA Quotes had tenor of " << toString(fraData.getCurveTenorEnum()) << " but Market Data (" << this->getRefToName() << ") has tenor " << toString(etrading::HasConstInstance<CurveTenorEnum>::getRefToInstance()) << ": FRA Quotes cannot be accepted"; AQ_THROW( aqMsg91.str() ); }
		}
    };

    void CurveData::setSwapQuotes( SwapQuotes&& swapData )
    {
        if(swapData.getSwapType() == etrading::OIS_SWAPTYPE)
        {
            oisSwapQuotes_ = std::make_shared<SwapQuotes>(std::ref(swapData));
        } 
        else if(swapData.getSwapType() == etrading::BASIS_SWAPTYPE)
        {
            auto basisSwapQuotes = std::make_shared<BasisSwapQuotes>( static_cast<BasisSwapQuotes&>(swapData)); 
            if( basisSwapQuotes->getFromTenor() == etrading::HasConstInstance<CurveTenorEnum>::getRefToInstance() ||
                basisSwapQuotes->getToTenor() == etrading::HasConstInstance<CurveTenorEnum>::getRefToInstance() )
            {
                basisSwapQuotes_ =  basisSwapQuotes;   
            } else
            {
                { std::ostringstream aqMsg92;
aqMsg92 << "Basis Swap Quotes had fromTenor of " << toString(basisSwapQuotes->getFromTenor()) << " and toTenor of " << toString(basisSwapQuotes->getToTenor()) << " but Market Data (" << this->getRefToName() << ") has tenor " << toString(etrading::HasConstInstance<CurveTenorEnum>::getRefToInstance()) << ": Basis Swap Quotes cannot be accepted"; AQ_THROW( aqMsg92.str() ); }   
            }
        }
        else
        {
            // an OIS market data collection needs the ability to hold not 1D swap quotes
            if(isCorrectTenor(swapData) || etrading::HasConstInstance<CurveTenorEnum>::getRefToInstance() == etrading::CURVE_TENOR_1D)
		    {
			    marketDataCollection_[IRS_SWAP] = std::make_shared<SwapQuotes>( swapData );
		    } 
		    else
		    {
			    { std::ostringstream aqMsg93;
aqMsg93 << "Swap Quotes had tenor of " << toString(swapData.getCurveTenorEnum()) << " but Market Data (" << this->getRefToName() << ") has tenor " << toString(etrading::HasConstInstance<CurveTenorEnum>::getRefToInstance()) << ": Swap Quotes cannot be accepted"; AQ_THROW( aqMsg93.str() ); }
		    }
        }
    };


    void CurveData::setIRFuturesQuotes( IRFuturesQuotes&& futuresData )
    {
		if(isCorrectTenor(futuresData))
		{
			marketDataCollection_[ IR_FUTURES ] = std::make_shared<IRFuturesQuotes>( futuresData );
		} 
		else
		{
			{ std::ostringstream aqMsg94;
aqMsg94 << "IR Futures Quotes had tenor of " << toString(futuresData.getCurveTenorEnum()) << " but Market Data (" << this->getRefToName() << ") has tenor " << toString(etrading::HasConstInstance<CurveTenorEnum>::getRefToInstance()) << ": IR Futures Quotes cannot be accepted"; AQ_THROW( aqMsg94.str() ); }
		}
    };

    void CurveData::setFXQuotes( FXQuotes&& fxData )
    {
        marketDataCollection_[ FXRATES ] = std::make_shared<FXQuotes>( fxData ); // no check on tenor for FX Quotes
    };

	void CurveData::setCBSwapQuotes( CentralBankSwapQuotes&& cbSwapData )
	{
		if(isCorrectTenor(cbSwapData))
		{
			marketDataCollection_[ CENTRALBANK_SWAP ] = std::make_shared<CentralBankSwapQuotes>( cbSwapData );
		} 
		else
		{
			{ std::ostringstream aqMsg95;
aqMsg95 << "Central Bank Swap Quotes had tenor of " << toString(cbSwapData.getCurveTenorEnum()) << " but Market Data (" << this->getRefToName() << ") has tenor " << toString(etrading::HasConstInstance<CurveTenorEnum>::getRefToInstance()) << ": CB Swap Quotes cannot be accepted"; AQ_THROW( aqMsg95.str() ); }
		}
	}

	const SchemaObject CurveData::toSchemaObject() const
	{
		SchemaObject schemaObject( CURVE_DATA , this->getRefToName());
        addQuotesToSchemaObject(oisSwapQuotes_,schemaObject);
        addQuotesToSchemaObject(basisSwapQuotes_,schemaObject);
		auto cit = marketDataCollection_.cbegin();
		while(cit != marketDataCollection_.cend())
		{
			auto ptrMktData = (*cit).second;
            addQuotesToSchemaObject(ptrMktData, schemaObject);
			cit++;
		}
		return schemaObject;	
	};

    void CurveData::addQuotesToSchemaObject(const std::shared_ptr<MarketQuote>& quote,SchemaObject& schemaObject) const
    {
        if(quote != nullptr)
        {
			auto dataSchema = quote->generateDataSchema();
			schemaObject.addDataSchema(dataSchema);
			schemaObject.setDataForSchema(dataSchema.getName(), quote->getVariantMatrix());
        }
    };

    void CurveData::addQuotesToVariantMatrix(const std::shared_ptr<MarketQuote>& quote,VariantMatrix& variantMatrix, const bool transpose) const
    {
        if(quote != nullptr)
        {
            auto quotesMatrix = transpose? Variant::transpose(quote->getVariantMatrix()) : quote->getVariantMatrix();
            variantMatrix.insert(variantMatrix.end(), quotesMatrix.cbegin(), quotesMatrix.cend());
        }
    };

	const bool CurveData::isCorrectTenor(const MarketQuote& mq) const
	{
		return mq.getCurveTenorEnum() == etrading::HasConstInstance<CurveTenorEnum>::getRefToInstance();	
	};

    VariantMatrix CurveData::getVariantMatrix() const
    {
        VariantMatrix variantMatrix;
        addQuotesToVariantMatrix(oisSwapQuotes_, variantMatrix);
        addQuotesToVariantMatrix(basisSwapQuotes_, variantMatrix);
		auto cit = marketDataCollection_.cbegin();
		while(cit != marketDataCollection_.cend())
		{
			auto ptrMktData = (*cit).second;
            addQuotesToVariantMatrix(ptrMktData, variantMatrix);
			cit++;
		}
		return Variant::transpose(variantMatrix);	
    };


    /*static*/ 
    CurveData CurveData::createFromValidationInput(
        const std::string& mdcName, 
        const etrading::CurveTenorEnum curveTenorEnum,
        const AQLStringMatrix& oisRates, 
        const AQLStringMatrix& liborOisBasisRates, 
        const AQLStringMatrix& swapRates)
    {
        etrading::CurveData mdcOIS(mdcName, curveTenorEnum);




        return mdcOIS;


    
    }



}