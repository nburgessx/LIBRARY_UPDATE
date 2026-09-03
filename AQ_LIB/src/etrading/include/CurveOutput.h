#pragma once

//
// @File: CurveOutput.h
// @Description: This Class owns the outputs of a Curve object (discount factors, forward rates, settings)
// in etrading::ContainedEnumType.
// @Created: 27 Feb 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of Mizuho International.

#include <string>
#include <vector>
#include <utility>
#include <tuple>
#include <boost/date_time.hpp>

#include "CoreEnumerations.h"
#include "RuleInterface.h"
#include "CurveBuildProperties.h"
#include "Translatable.h"
#include "Translated.h"
#include "HasInstance.h"
#include "ExposedInterface.h"




namespace etrading
{

    class CurveOutput
        :	public Cacheable,
          public Translatable,
          public HasName,
          public ExposedInterface
    {
    public:
        CurveOutput( const Translated& translatedObject );
        CurveOutput( const std::string& objName );
        CurveOutput(	const std::string& objName,
                        const std::vector<boost::gregorian::date>& dates,
                        const std::vector<double>& discountFactors,
                        const std::vector<double>& forwardRates,
                        const CurveBuildProperties& curveConvention );
        void setData( const std::vector<boost::gregorian::date>& dates,
                      const std::vector<double>& discountFactors,
                      const std::vector<double>& forwardRates );
        void setCurveConvention( const CurveBuildProperties& curveConvention );

        int getIndexOfDate( const boost::gregorian::date& date ) const;
        std::tuple<boost::gregorian::date, double, double> getData( const unsigned int idx ) const;
        std::string getName() const;
        void clear();
        const std::vector<boost::gregorian::date>& getDates() const;
        const std::vector<double>& getDiscountFactors() const;
        const std::vector<double>& getForwardRates() const;
        const CurveBuildProperties& getCurveConvention() const;

        virtual SerializationResult serialize(	const serialize::SerializationMethodEnum method,
                                                const serialize::SerializationTargetEnum target,
                                                const std::string& targetInfo,
                                                std::vector<std::string>& variableNames,
                                                std::vector<Variant>& variableValues ) const;

        double calculateDiscountFactor( const boost::gregorian::date& date, const etrading::NumericInterpolationEnum interpMethod ) const;
        double calculateDiscountFactor( const std::string& term, const etrading::NumericInterpolationEnum interpMethod ) const;

    private:
        static const etrading::CachedObjectEnum ENUM_TYPE;
        const Translated toTranslated() const;

        std::vector<boost::gregorian::date> dates_;
        std::vector<double> discountFactors_;
        std::vector<double> forwardRates_;
        CurveBuildProperties curveConvention_;

        static AscendingOrderRule< std::vector<boost::gregorian::date> > dateChecker_;
        static BoundaryRule<> dfChecker_;

        void inspectDataFormat(	const std::vector<boost::gregorian::date>& dates,
                                const std::vector<double>& discountFactors,
                                const std::vector<double>& forwardRates ) const;
        bool setDates( const std::vector<boost::gregorian::date>& dates );
        bool setDiscountFactors( const std::vector<double>& discountFactors );
        bool setForwardRates( const std::vector<double>& forwardRates );



    };





}

