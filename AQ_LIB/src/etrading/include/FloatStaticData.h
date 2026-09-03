#pragma once

#include <string>
#include "LegStaticData.h"

namespace etrading
{
    class FloatStaticData : public LegStaticData
    {
    public:

        FloatStaticData();
        FloatStaticData( const LabelValueBlock& marketDataLVB );
        FloatStaticData(const FloatStaticData& rhs);
        virtual ~FloatStaticData() {};

        void validateCurveInput(const AQLString& curveCollection);

        LegStaticDataPtr clone();

        AQLString getForecastCurve() const;
        AQLString getForecastCurveMarketName() const;
        double getFirstFixing() const;
        double getLastFixing() const;
        AQLString getFirstStubCurveIndex() const;
        AQLString getLastStubCurveIndex() const;
		BooleanEnum getFwdInter() const;
        void setFwdInter(const BooleanEnum& fwdInter);

   	private:
        AQLString forecastCurve_;

        //useful dataValues:
        AQLString forecastCurveMarketName_;
        
        AQLString firstStubCurveIndex_;
		AQLString lastStubCurveIndex_;
		double firstFixing_;
		double lastFixing_;
		BooleanEnum fwdInter_;

    };

}
