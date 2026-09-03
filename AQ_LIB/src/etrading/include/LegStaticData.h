#pragma once

#include <string>
#include "LabelValueBlock.h"
#include "CommonConstants.h"
#include "CoreEnumerations.h"
#include "Variant.h"
#include "DataSchema.h"
#include "SchemaObject.h"
#include "LabelValueBlockValidation.h"
#include "CurveUtilities.h"
#include "CurveValidation.h"

namespace etrading
{

    class LegStaticData 
	{
    public:    

		LegStaticData();
        LegStaticData( const LabelValueBlock& marketDataLVB );
        LegStaticData(const LegStaticData& rhs);
        virtual ~LegStaticData() {};

        virtual void validateCurveInput(const LAString& curveCollection);
        virtual std::shared_ptr<LegStaticData> clone();

        LAString getLegName() const;
		
        LAString getDiscountCurve() const;
        CCY getCurrency() const;
        CCY getValuationCurrency() const;
		void setValuationCurrency(const CCY& valuationCurrency);
        LAString getDiscountCurveMarketName() const;
        bool isCurveDataLoaded() const;
        void setCurveDataLoaded(bool isCurveDataLoaded);
        CompoundingMethodEnum getCouponCompoundMethod() const;

        //dummy methods, just to avoid downcasting
        virtual LAString getForecastCurve() const;
        virtual LAString getForecastCurveMarketName() const;
        virtual double getFirstFixing() const;
        virtual double getLastFixing() const;
        virtual LAString getFirstStubCurveIndex() const;
        virtual LAString getLastStubCurveIndex() const;
        virtual BooleanEnum getFwdInter() const;
        virtual void setFwdInter(const BooleanEnum& fwdInter);

		LabelValueBlock getInputParameters() const;

        static std::vector<std::string> lvbKeys()
        {
            const std::string arr[] =
            {
				IRS_KEY::LEG_TYPE
				,MARKET_KEY::FORECAST_CURVE //Optional for fixed leg, compulsory for float leg
                ,IRS_KEY::COMPOUND_METHOD //Not required for fixed leg, optional for float leg

                , MARKET_KEY::DISCOUNT_CURVE
                , IRS_KEY::VALUATION_CURRENCY
                , IRS_KEY::CURRENCY

                // fields for fixing
				, IRS_KEY::FIRSTFIXING
                , IRS_KEY::LASTFIXING
                , IRS_KEY::FIRSTSTUBCURVEINDEX
                , IRS_KEY::LASTSTUBCURVEINDEX
				, IRS_KEY::IS_FWD_INTER //This is to allow user to change isFwdInter to true or false

                // fields for fixing
				, IRS_KEY::FLOAT_FIRSTFIXING
                , IRS_KEY::FLOAT_LASTFIXING
                , IRS_KEY::FLOAT_FIRSTSTUBCURVEINDEX
                , IRS_KEY::FLOAT_LASTSTUBCURVEINDEX
            };
            std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );
            return expectedKeys;
        }


    protected:

    	LAString legName_;

        LAString discountCurve_;
        CCY currency_;
        CCY valuationCurrency_;
        CompoundingMethodEnum couponCompoundMethod_;

        //useful dataValues:
        LAString discountCurveMarketName_;
        bool isCurveDataLoaded_;

   		LabelValueBlock inputParameters_;
    };

	typedef std::shared_ptr<LegStaticData> LegStaticDataPtr;

}
