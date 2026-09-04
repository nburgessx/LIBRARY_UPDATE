#pragma once

//
// @Description: This file wraps up

#include <vector>
#include <string>
#include <memory>
#include <boost/date_time.hpp>

#include "InterpolationParameters.h"
#include "CoreEnumerations.h"
#include "HasInstance.h"
#include "HasSchemaObjectConverter.h"
#include "AQLMathCalendarSet.h"
#include "AQLMathCalendar.h"
#include "Variant.h"
#include "IsAQObject.h"

/*
This class holds the basic properties needed for operating on AQObjCurve objects
*/


namespace etrading
{
    struct CurveBuildProperties
        : public IsAQObject,
          public HasConstInstance<CurveTypeEnum>
    {
        CurveBuildProperties( const CurveBuildProperties& ins );
        CurveBuildProperties( const CurveBuildProperties&& moved );
        CurveBuildProperties( const std::string& objectName );
        /// all daycounts are dc_act365(act_365_ISDA)  in AlgoQuantLib but we should allow this to be settable later, once we move away
        CurveBuildProperties(
            const CurveTypeEnum curveTypeEnum,
            const std::string& objectName,
            const std::string& curveCollectionName,
            const std::string& curveIndexName,
            const CCY ccy,
            const boost::gregorian::date& asOfDate,
            const InterpolationEnum interpMethod,
            const CompoundingFrequencyEnum floatRateCompoundingFreq,
            const CurveTenorEnum floatRateTenor,
            const CompoundingMethodEnum oisCompoundingMethod,
            const BusinessDayAdjustmentEnum businessDayAdjustment,
            const std::string& calendar,
			const std::shared_ptr<const InterpolationParameters>& interpolationParameters = {},
            const bool onlyAllowLookup = false
                                         // ,std::shared_ptr<VariantMatrix>& tenorBasisSettings = nullptr
        );
        CurveBuildProperties(
            const std::string& objectName,
            const CCY ccy,
            const boost::gregorian::date& asOfDate ); // this is what gets called for an FX FWD curve

        ~CurveBuildProperties();

        std::string curveCollectionName_;
        std::string curveIndexName_;
        CCY ccy_;
        boost::gregorian::date asOfDate_;
        InterpolationEnum interpMethod_;
        CurveTenorEnum floatRateTenor_;
        CompoundingMethodEnum oisCompoundingMethod_;

        void setFloatRateCompoundingFreq( const CompoundingFrequencyEnum enumValue );
        void setExtrapolation( const ExtrapolationTypeEnum extrapolationType );

        const CompoundingFrequencyEnum getFloatRateCompoundingFreq() const;
        const CurveTypeEnum getCurveTypeEnum() const;

        void setFixingDayCalendar( const std::string& calendar );
        const std::string getFixingDayCalendar() const;
        const AQLMathCalendar* getAqFixingCalendar() const;

        void setAccrualDayCalendar( const std::string& calendar );
        const std::string getAccrualDayCalendar() const;
        const AQLMathCalendar* getAqAccrualCalendar() const;

        void setPaymentDayCalendar( const std::string& calendar );
        const std::string getPaymentDayCalendar() const;
        const AQLMathCalendar* getAqPaymentCalendar() const;

        const ExtrapolationTypeEnum getExtrapolationTypeEnum() const;

        virtual SerializationResult serialize(	const serialize::SerializationMethodEnum method,
                                                const serialize::SerializationTargetEnum target,
                                                const std::string& targetInfo,
                                                std::vector<std::string>& variableNames,
                                                std::vector<Variant>& variableValues) const;
        const SchemaObject toSchemaObject() const;

        // daycount is "ACT/365"
        BusinessDayAdjustmentEnum fixingDayAdjustment_;
        BusinessDayAdjustmentEnum accrualDayAdjustment_;
        BusinessDayAdjustmentEnum paymentDayAdjustment_;

        const std::shared_ptr<const InterpolationParameters>& getInterpolationParameters() const;

        const bool isOnlyAllowLookup() const;
    private:
        CompoundingFrequencyEnum floatRateCompoundingFreq_;
        //CurveTypeEnum curveTypeEnum_;
        etrading::ExtrapolationTypeEnum extrapolationTypeEnum_;
        bool onlyAllowLookup_;
        std::shared_ptr<const InterpolationParameters> interpolationParameters_;

        std::string fixingCalendar_;
        std::string accrualCalendar_;
        std::string paymentCalendar_;

        // observation pointer so never delete it
        const AQLMathCalendar* aqFixingCalendar_;
        const AQLMathCalendar* aqAccrualCalendar_;
        const AQLMathCalendar* aqPaymentCalendar_;

        const bool isConsistent() const;
        void assertConsistent() const;

        static const std::vector<std::string> VARIABLE_NAMES;
        CurveBuildProperties& operator=( const CurveBuildProperties& rhs ); // TODO: C++11 =delete

    };
}

// std::shared_ptr<VariantMatrix> tenorBasisSettings_;

/*  // this level of detail is not used for now in CBP
CurveBuildProperties(
    const CurveTypeEnum curveTypeEnum,
    const std::string& objectName,
    const std::string& curveCollectionName,
    const std::string& curveIndexName,
    const CCY ccy,
    const boost::gregorian::date& asOfDate,
    const InterpolationEnum interpMethod,
    const CompoundingFrequencyEnum floatRateCompoundingFreq,
    const CurveTenorEnum floatRateTenor,
    const CompoundingMethodEnum oisCompoundingMethod,
    const BusinessDayAdjustmentEnum fixingDayAdjustment,
    const std::string& fixingCalendar,
    const BusinessDayAdjustmentEnum accrualDayAdjustment,
    const std::string& accrualCalendar,
    const BusinessDayAdjustmentEnum paymentDayAdjustment,
    const std::string& paymentCalendar,
	const std::shared_ptr<const InterpolationParameters>& interpolationParameters = nullptr,
	const bool onlyAllowLookup = false
);
*/
