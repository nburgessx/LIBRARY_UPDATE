#include "DataProvider.h"

namespace etrading
{

	FloatRateData::FloatRateData() : resetRate(std::numeric_limits<double>::quiet_NaN()), unadjustedResetRate(std::numeric_limits<double>::quiet_NaN()), convexity(0.0)
	{}

	FloatRateData::FloatRateData(const double& rate) : resetRate(rate), unadjustedResetRate(rate), convexity(0.0)
	{}

	FloatRateData::~FloatRateData()
	{}

	CashflowData::CashflowData() : 	discountFactor( std::numeric_limits<double>::quiet_NaN() ),
									floatRateData(FloatRateData()),
									currency( NO_CCY ),
									valuationCurrency( NO_CCY ),
                                    fxAsOfDate(1.0),
									compoundType( NONE_COMPOUNDING_METHOD ),
									includeCouponRate(true),
									compoundRateOverride(std::numeric_limits<double>::quiet_NaN()),
									floatSpreadOverride(std::numeric_limits<double>::quiet_NaN())
	{}

	CashflowData::~CashflowData()
	{}


	DataProvider::DataProvider(const ValuationSettings& valuationSettings)
		: hasUpfrontData_(false),
		upfrontDiscountFactor_(std::numeric_limits<double>::quiet_NaN()),
		currency_(NO_CCY),
		valuationCurrency_(NO_CCY),
		compoundType_( NONE_COMPOUNDING_METHOD ),
		compoundRateOverride_(std::numeric_limits<double>::quiet_NaN()),
		floatSpreadOverride_(std::numeric_limits<double>::quiet_NaN()),
		valuationSettings_(valuationSettings)
	{}

	DataProvider::~DataProvider()
	{}

	CashflowData DataProvider::getCashflowDataExcludingUpfront( size_t i) const
	{
		CashflowData result;

		// CashflowData members are initialised to nan. Only set if we have actual data
		if ( i < discountFactors_.size() )
		{
			result.discountFactor = discountFactors_[i];
		}

		if ( i < floatRates_.size() )
		{
			result.floatRateData = floatRates_[i];
		}

		if (i < includeCouponRates_.size())
		{
			result.includeCouponRate = includeCouponRates_[i];
		}

		result.currency          = currency_;
		result.valuationCurrency = valuationCurrency_;
		result.compoundType      = compoundType_;
        result.fxAsOfDate        = valuationSettings_.getFXAsOfDateRate();
		result.compoundRateOverride = compoundRateOverride_;
		result.floatSpreadOverride = floatSpreadOverride_;

		return result;
	}

	CashflowData DataProvider::getCashflowDataIncludingUpfront( size_t i) const
	{
		if ( hasUpfrontData_ )
		{
			if (i == 0 )
			{
				// populate data for upfront cashflow
				CashflowData result;
				result.discountFactor    = upfrontDiscountFactor_;
				result.currency          = currency_;
				result.valuationCurrency = valuationCurrency_;
				result.compoundType      = compoundType_;
                result.fxAsOfDate         = valuationSettings_.getFXAsOfDateRate();
				result.compoundRateOverride = compoundRateOverride_;
				result.floatSpreadOverride = floatSpreadOverride_;

				return result;
			}
			else
			{
				// Regular cashflow. Index into data vectors
				return getCashflowDataExcludingUpfront( i-1 );
			}
		}
		else
		{
			return getCashflowDataExcludingUpfront( i );
		}
	}

    CCY DataProvider::getCurrency() const
	{
		return currency_;
	}

	CCY DataProvider::getValuationCurrency() const
	{
		return valuationCurrency_;
	}

	CompoundingMethodEnum DataProvider::getCompoundType() const
	{
		return compoundType_;
	}


	double DataProvider::getCompoundRateOverride() const
	{
		return compoundRateOverride_;
	}

	double DataProvider::getFloatSpreadOverride() const
	{
		return floatSpreadOverride_;
	}

	ValuationSettings DataProvider::getValuationSettings() const
	{
		return valuationSettings_;
	}

	void DataProvider::setUpfrontDiscountFactor( double discountFactor )
	{
		hasUpfrontData_ = true;
		upfrontDiscountFactor_ = discountFactor;
	}
	
	void DataProvider::setDiscountFactors(const DoubleVector&  discountFactors )
	{
		discountFactors_ = discountFactors;
	}

	void DataProvider::setFloatRates(const std::vector<FloatRateData>& floatRates)
	{
		floatRates_ = floatRates;
	}

	void DataProvider::setIncludeCouponRates(const BoolVector& includeCouponRates)
	{
		includeCouponRates_ = includeCouponRates;
	}

	void DataProvider::setCurrency( CCY currency )
	{
		currency_ = currency;
	}

	void DataProvider::setValuationCurrency( CCY currency )
	{
		valuationCurrency_ = currency;
	}

	void DataProvider::setCompoundType( CompoundingMethodEnum compoundType )
	{
		compoundType_ = compoundType;
	}

	void DataProvider::setCompoundRateOverride(const double& compoundRateOverride)
	{
		compoundRateOverride_ = compoundRateOverride;
	}

	void DataProvider::setFloatSpreadOverride(const double& floatSpreadOverride)
	{
		floatSpreadOverride_ = floatSpreadOverride;
	}


}

