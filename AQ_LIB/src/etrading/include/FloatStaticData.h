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

        void validateCurveInput(const LAString& curveCollection);

        LegStaticDataPtr clone();

        LAString getForecastCurve() const;
        LAString getForecastCurveMarketName() const;
        double getFirstFixing() const;
        double getLastFixing() const;
        LAString getFirstStubCurveIndex() const;
        LAString getLastStubCurveIndex() const;
		BooleanEnum getFwdInter() const;
        void setFwdInter(const BooleanEnum& fwdInter);

   	private:
        LAString forecastCurve_;

        //useful dataValues:
        LAString forecastCurveMarketName_;
        
        LAString firstStubCurveIndex_;
		LAString lastStubCurveIndex_;
		double firstFixing_;
		double lastFixing_;
		BooleanEnum fwdInter_;

    };

}
