#include "FloatStaticData.h"
#include "SwapValidation.h"

namespace etrading
{
    FloatStaticData::FloatStaticData() : LegStaticData(), forecastCurve_(""), forecastCurveMarketName_(""),
        firstStubCurveIndex_(""), lastStubCurveIndex_(""), firstFixing_(std::numeric_limits<double>::quiet_NaN()),
		lastFixing_(std::numeric_limits<double>::quiet_NaN()), fwdInter_(NONE_BOOL)
    {}
    
    FloatStaticData::FloatStaticData( const LabelValueBlock& marketDataLVB ) : LegStaticData(marketDataLVB)
    {
        const std::string inputLVB = "legPropertiesLVB";

        forecastCurve_	= marketDataLVB.getOptionalValueAsLAString( MARKET_KEY::FORECAST_CURVE, "" );
        forecastCurveMarketName_ = "";

  		firstStubCurveIndex_ = marketDataLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FLOAT_FIRSTSTUBCURVEINDEX, IRS_KEY::FIRSTSTUBCURVEINDEX, "NATURAL");
	    lastStubCurveIndex_ = marketDataLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FLOAT_LASTSTUBCURVEINDEX, IRS_KEY::LASTSTUBCURVEINDEX, "NATURAL");
		firstFixing_ = marketDataLVB.getOptionalValueAsDoubleFromKeys(IRS_KEY::FLOAT_FIRSTFIXING, IRS_KEY::FIRSTFIXING, std::numeric_limits<double>::quiet_NaN() );
		lastFixing_	= marketDataLVB.getOptionalValueAsDoubleFromKeys(IRS_KEY::FLOAT_LASTFIXING, IRS_KEY::LASTFIXING, std::numeric_limits<double>::quiet_NaN() );

		//Get the input from user, default to NONE
		fwdInter_ = toBooleanEnum(marketDataLVB.getOptionalValueAsLAString(IRS_KEY::IS_FWD_INTER, "").getCString());

		// For a OIS float leg, the compounding method is required for OIS float rate calculation
        // For non-OIS float leg, this field is required when the accrual frequencey is less than payment frequency (e.g. 3M vs 6M, or 3M vs AT_MATURITY)
		couponCompoundMethod_ = toCompoundingMethodEnum(marketDataLVB.getOptionalValueAsLAString(IRS_KEY::COMPOUND_METHOD, "").getCString());

	}

    void FloatStaticData::validateCurveInput(const LAString& curveCollection)
    {
        LegStaticData::validateCurveInput(curveCollection);

		if (forecastCurve_.size() != 0)
		{
			forecastCurveMarketName_ = getCurveStaticDataTableName( curveCollection, forecastCurve_ );
		}
	}

    LegStaticDataPtr FloatStaticData::clone()
    {
        LegStaticDataPtr data = LegStaticDataPtr(new FloatStaticData(*this));
        return data;
    }

    FloatStaticData::FloatStaticData(const FloatStaticData& rhs) : LegStaticData(rhs),  
        forecastCurve_(rhs.forecastCurve_),		
		forecastCurveMarketName_(rhs.forecastCurveMarketName_),
        firstStubCurveIndex_(rhs.firstStubCurveIndex_),
		lastStubCurveIndex_(rhs.lastStubCurveIndex_),
		firstFixing_(rhs.firstFixing_),
		lastFixing_(rhs.lastFixing_),
		fwdInter_(rhs.fwdInter_)
    {}

    LAString FloatStaticData::getForecastCurve() const
    {
        return forecastCurve_;
    }

    LAString FloatStaticData::getForecastCurveMarketName() const
    {
		if (forecastCurve_.size() != 0 && forecastCurveMarketName_.size() == 0 )
		{
	    	throw LACoreInvalidData( "#Error: Float's forecastCurveMarketName has not been populated", __FILE__, __LINE__ );
		}
        return forecastCurveMarketName_;
    }

    double FloatStaticData::getFirstFixing() const
    {
        return firstFixing_;
    }
           
    double FloatStaticData::getLastFixing() const
    {
        return lastFixing_;
    }

    LAString FloatStaticData::getFirstStubCurveIndex() const
    {
        return firstStubCurveIndex_;
    }

    LAString FloatStaticData::getLastStubCurveIndex() const
    {
        return lastStubCurveIndex_;
    }

    BooleanEnum FloatStaticData::getFwdInter() const
    {
		return fwdInter_;
    }

    void FloatStaticData::setFwdInter(const BooleanEnum& fwdInter)
    {
		//only set a value if it has not been set by the user
		if (fwdInter_ == NONE_BOOL)
		{
			fwdInter_ = fwdInter;
		}
    }

}
