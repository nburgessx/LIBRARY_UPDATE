
#include <boost/format.hpp>

#include "CurveData.h"
#include "CoreEnumerations.h"
#include "SchemaObject.h"
#include "ScheduleValidation.h"
#include "Variant.h"

namespace etrading
{

    CurveData::CurveData( const std::string& collectionName, const CurveTenorEnum curveTenorEnum )
        : etrading::HasConstInstance<CurveTenorEnum>( curveTenorEnum ), IsLWOObject( collectionName, CURVE_DATA ), oisSwapQuotes_(), basisSwapQuotes_()
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
                    throw ETradingException( ( boost::format( "MarketQuote of type %s is not accepted in a MarketDataColleciton" ) % toString( typeOfQuote ) ).str() );
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
			throw ETradingException( ( boost::format( "FRA Quotes had tenor of %s but Market Data (%s) has tenor %s: FRA Quotes cannot be accepted") 
										% toString(fraData.getCurveTenorEnum()) 
										% this->getRefToName() 
										% toString(etrading::HasConstInstance<CurveTenorEnum>::getRefToInstance()) ).str() );
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
                throw ETradingException( ( boost::format( "Basis Swap Quotes had fromTenor of %s and toTenor of %s but Market Data (%s) has tenor %s: Basis Swap Quotes cannot be accepted") 
										    % toString(basisSwapQuotes->getFromTenor())
                                            % toString(basisSwapQuotes->getToTenor())
										    % this->getRefToName() 
										    % toString(etrading::HasConstInstance<CurveTenorEnum>::getRefToInstance()) ).str() );   
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
			    throw ETradingException( ( boost::format( "Swap Quotes had tenor of %s but Market Data (%s) has tenor %s: Swap Quotes cannot be accepted") 
										    % toString(swapData.getCurveTenorEnum()) 
										    % this->getRefToName() 
										    % toString(etrading::HasConstInstance<CurveTenorEnum>::getRefToInstance()) ).str() );
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
			throw ETradingException( ( boost::format( "IR Futures Quotes had tenor of %s but Market Data (%s) has tenor %s: IR Futures Quotes cannot be accepted") 
										% toString(futuresData.getCurveTenorEnum()) 
										% this->getRefToName() 
										% toString(etrading::HasConstInstance<CurveTenorEnum>::getRefToInstance()) ).str() );
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
			throw ETradingException( ( boost::format( "Central Bank Swap Quotes had tenor of %s but Market Data (%s) has tenor %s: CB Swap Quotes cannot be accepted") 
										% toString(cbSwapData.getCurveTenorEnum()) 
										% this->getRefToName() 
										% toString(etrading::HasConstInstance<CurveTenorEnum>::getRefToInstance()) ).str() );
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