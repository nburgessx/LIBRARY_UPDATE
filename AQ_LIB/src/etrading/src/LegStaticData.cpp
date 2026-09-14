#include "LegStaticData.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "ExceptionMacros.h"

namespace etrading
{

    LegStaticData::LegStaticData()  
        : legName_(""),
		discountCurve_(""),		
		currency_( NO_CCY ),		
        valuationCurrency_( NO_CCY ),			
		discountCurveMarketName_(""),				
        couponCompoundMethod_(NONE_COMPOUNDING_METHOD),
		isCurveDataLoaded_(false)
    {}

    LegStaticData::LegStaticData( const LabelValueBlock& marketDataLVB )
    {

   		inputParameters_ = marketDataLVB;

        const std::string inputLVB = "legPropertiesLVB";

		//cast it to upper case so that legName lookup is not case-sensitive (i.e Leg1:Fixed, leg1:fixed, LEG1:FIXED are the same)
        legName_ = marketDataLVB.getCompulsoryValueAsAQLString(IRS_KEY::LEG_TYPE,  inputLVB, true);

        discountCurve_ = marketDataLVB.getCompulsoryValueAsAQLString( MARKET_KEY::DISCOUNT_CURVE, inputLVB );
       
        currency_ = toCCYEnum( marketDataLVB.getOptionalValueAsAQLString( IRS_KEY::CURRENCY ).getCString() );
		if ( currency_ == NO_CCY )
        {
            AQ_THROW( "No currency specified for swap leg" );
        }

        valuationCurrency_ = toCCYEnum( marketDataLVB.getOptionalValueAsAQLString( IRS_KEY::VALUATION_CURRENCY).getCString() );
		if (valuationCurrency_ == NO_CCY)
        {
            valuationCurrency_ = currency_;
        }

        couponCompoundMethod_ = NONE_COMPOUNDING_METHOD;
        
        isCurveDataLoaded_ = false;
    }

    void LegStaticData::validateCurveInput(const AQLString& curveCollection)
    {
        AQ_THROW_IF( curveCollection.size() == 0, "curveCollection must be provided" );

        //Throw exception if the curve has not been built.
        discountCurveMarketName_ = getCurveStaticDataTableName( curveCollection, discountCurve_ );

        auto curveCurrency = toCCYEnum( getCurveCurrency( curveCollection ).getCString() );

        if ( currency_ == NO_CCY )
        {
            AQ_THROW( "No currency specified for swap leg" );
        }
        else if ( curveCurrency != currency_ )
        {
    	    AQ_THROW( "Invalid curve currency: Trade currency is " + toString( currency_ ) + ", curve currency is " + toString( curveCurrency) );
        }

        if (valuationCurrency_ == NO_CCY)
        {
            valuationCurrency_ = currency_;
        }
    }

    LegStaticDataPtr LegStaticData::clone()
    {
        LegStaticDataPtr data = LegStaticDataPtr(new LegStaticData(*this));
        return data;
    }

    LegStaticData::LegStaticData(const LegStaticData& rhs) 
		: legName_(rhs.legName_),
        discountCurve_(rhs.discountCurve_),		
        valuationCurrency_(rhs.valuationCurrency_),			
		currency_(rhs.currency_),		
		discountCurveMarketName_(rhs.discountCurveMarketName_),				
        inputParameters_(rhs.inputParameters_),
        couponCompoundMethod_(rhs.couponCompoundMethod_),
        isCurveDataLoaded_(rhs.isCurveDataLoaded_)
	{}

    LabelValueBlock LegStaticData::getInputParameters() const
    {
        return inputParameters_;
    }

    AQLString LegStaticData::getLegName() const
	{
		return legName_; 
	}

    AQLString LegStaticData::getDiscountCurve() const
    {
        return discountCurve_;
    }

    CCY LegStaticData::getCurrency() const
    {
        return currency_;
    }

    CCY LegStaticData::getValuationCurrency() const
    {
        return valuationCurrency_;
    }

	void LegStaticData::setValuationCurrency(const CCY& valuationCurrency)
    {
        valuationCurrency_ = valuationCurrency;
    }

    AQLString LegStaticData::getDiscountCurveMarketName() const
    {
        return discountCurveMarketName_;
    }

    bool LegStaticData::isCurveDataLoaded() const
    {
        return isCurveDataLoaded_;
    }

    void LegStaticData::setCurveDataLoaded(bool isCurveDataLoaded)
    {
        isCurveDataLoaded_ = isCurveDataLoaded;
    }

    CompoundingMethodEnum LegStaticData::getCouponCompoundMethod() const
    {
        return couponCompoundMethod_;
    }

    //dummy methods, just to avoid downcasting
    AQLString LegStaticData::getForecastCurve() const
    {
    	AQ_THROW( "getForecastCurve method not supported on a fixed leg" );
    }
    
    AQLString LegStaticData::getForecastCurveMarketName() const
    {
    	AQ_THROW( "getForecastCurveMarketName method not supported for this product" );
    }

    double LegStaticData::getFirstFixing() const
    {
    	AQ_THROW( "getFirstFixing method not supported for this product" );
    }

    double LegStaticData::getLastFixing() const
    {
    	AQ_THROW( "getLastFixing method not supported for this product" );
    }
    
    AQLString LegStaticData::getFirstStubCurveIndex() const
    {
    	AQ_THROW( "getFirstStubCurveIndex method not supported for this product" );
    }

    AQLString LegStaticData::getLastStubCurveIndex() const
    {
    	AQ_THROW( "getLastStubCurveIndex method not supported for this product" );
    }

    BooleanEnum LegStaticData::getFwdInter() const
    {
    	AQ_THROW( "getFwdInter method not supported for this product" );
    }

    void LegStaticData::setFwdInter(const BooleanEnum& fwdInter)
    {
    	AQ_THROW( "setFwdInter method not supported for this product" );
    }


}

