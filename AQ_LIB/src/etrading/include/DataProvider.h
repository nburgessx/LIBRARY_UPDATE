/*
 * @brief			Class the defines the Cashflow DataProvider, which contains the dynamic cashflow data
 *					such as discount factors and float rates
 * @Created:		2 May 2018
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "CoreEnumerations.h"
#include "LACoreTemplateType.h"
#include "LabelValueBlock.h"
#include "CreditModel.h"
#include "ValuationSettings.h"

namespace etrading
{
	struct FloatRateData
	{
		FloatRateData();
		FloatRateData(const double& rate);

		~FloatRateData();

		double resetRate;

		// for libor in advance, the unadjustedFloatRateData and convexity are NAN;
		double unadjustedResetRate;
		double convexity;
	};

	struct CashflowData
	{
		CashflowData();
		~CashflowData();

		double discountFactor;
		FloatRateData floatRateData;
		CCY currency;
		CCY valuationCurrency;
        double fxAsOfDate;
		CompoundingMethodEnum compoundType;

		//Note that only fixedBondCashflow getCompoundRate() handle the case when includeCouponRate is false, as this is required for exdividend bond, when settleDate >= exdividendDate, the couponRate need to be excluded
		bool includeCouponRate;
		double compoundRateOverride;
		double floatSpreadOverride;

	};

	class DataProvider
    {
    public:

		DataProvider(const ValuationSettings& valuationSettings);

		~DataProvider();
		
		CashflowData getCashflowDataExcludingUpfront( size_t i) const;
		CashflowData getCashflowDataIncludingUpfront( size_t i) const;

        CCY getCurrency() const;
		CCY getValuationCurrency() const;

		CompoundingMethodEnum getCompoundType() const;
		double getCompoundRateOverride() const;
		double getFloatSpreadOverride() const;
		ValuationSettings getValuationSettings() const;

		void setUpfrontDiscountFactor( double discountFactor );
		void setDiscountFactors(const DoubleVector& discountFactors );

		void setFloatRates(const std::vector<FloatRateData>& floatRates );
		void setIncludeCouponRates(const BoolVector& includeCouponRates);

		void setCurrency( CCY currency );
		void setValuationCurrency( CCY currency );
		void setCompoundType( CompoundingMethodEnum compoundType );
		void setCompoundRateOverride(const double& compoundRateOverride);
		void setFloatSpreadOverride(const double& floatSpreadOverride);

	private:
		DoubleVector discountFactors_;
		std::vector<FloatRateData> floatRates_;
		BoolVector includeCouponRates_;

		bool hasUpfrontData_;
		double upfrontDiscountFactor_;
		double compoundRateOverride_;
		double floatSpreadOverride_;

		CCY currency_;
		CCY valuationCurrency_;
		CompoundingMethodEnum compoundType_;

		ValuationSettings valuationSettings_;

	};


}
