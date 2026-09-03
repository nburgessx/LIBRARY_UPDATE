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

        virtual void validateCurveInput(const AQLString& curveCollection);
        virtual std::shared_ptr<LegStaticData> clone();

        AQLString getLegName() const;
		
        AQLString getDiscountCurve() const;
        CCY getCurrency() const;
        CCY getValuationCurrency() const;
		void setValuationCurrency(const CCY& valuationCurrency);
        AQLString getDiscountCurveMarketName() const;
        bool isCurveDataLoaded() const;
        void setCurveDataLoaded(bool isCurveDataLoaded);
        CompoundingMethodEnum getCouponCompoundMethod() const;

        //dummy methods, just to avoid downcasting
        virtual AQLString getForecastCurve() const;
        virtual AQLString getForecastCurveMarketName() const;
        virtual double getFirstFixing() const;
        virtual double getLastFixing() const;
        virtual AQLString getFirstStubCurveIndex() const;
        virtual AQLString getLastStubCurveIndex() const;
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

    	AQLString legName_;

        AQLString discountCurve_;
        CCY currency_;
        CCY valuationCurrency_;
        CompoundingMethodEnum couponCompoundMethod_;

        //useful dataValues:
        AQLString discountCurveMarketName_;
        bool isCurveDataLoaded_;

   		LabelValueBlock inputParameters_;
    };

	typedef std::shared_ptr<LegStaticData> LegStaticDataPtr;

}
