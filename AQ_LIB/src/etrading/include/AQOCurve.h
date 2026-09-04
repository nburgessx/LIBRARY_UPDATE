#pragma once

//
// @File: AQOCurve.h
// @Description: This Class owns the outputs of a Curve object (discount factors, forward rates, settings)
// in etrading::ContainedEnumType.
// @Created: 27 Feb 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of AlgoQuantHub.


#include <string>
#include <vector>
#include <utility>
#include <tuple>
#include <memory>
#include <boost/date_time.hpp>

#include "CoreEnumerations.h"
#include "RuleInterface.h"
#include "CurveBuildProperties.h"
#include "SchemaObject.h"
#include "HasInstance.h"
#include "InterpolationFactory.h"
#include "HasVariantMatrixAccess.h"
#include "IsAQObject.h"
#include "CurveData.h"
#include "HasCurveData.h"

namespace etrading
{

    class AQOCurve
        :	public IsAQObject,
            public HasCurveData,
            public HasVariantMatrixAccess
    {

    public:

        AQOCurve( const std::string& objName );

        AQOCurve( AQOCurve&& );

        AQOCurve( const AQOCurve& ); // Only doing this so we can copy from the cache (but do we really need to do that?)

        virtual ~AQOCurve(); // virtual in case we want to make this a base

        /*
        the dates are fixing dates
        the discount factors are the discount factors corresponding to those dates
        (i.e. where the fixing date is the payment date without any day adjustment  applied)
        the curve build properties are properties used to build the curve
        the forward rates are the curve tenor forwards that correspond to the fixing dates in dates
        */
        AQOCurve(	const std::string& objName,
                    const std::vector<boost::gregorian::date>& dates,
                    const std::vector<double>& discountFactors,
                    const CurveBuildProperties& curveConvention,
                    const std::vector<double>& forwardRates = std::vector<double>( 0, std::numeric_limits<double>::quiet_NaN() ) );

        AQOCurve(	const std::string& objName,
                    const std::vector<double>& yearFractionsAsActAct,
                    const std::vector<double>& discountFactors,
                    const CurveBuildProperties& curveConvention );

        // these CTOR exists because the day count converter in AlgoQuantLib is not consistent in reverse,
        // which means that we cannot convert year Fractions back to the dates they came from
        // hence they need to be supplied ....
        AQOCurve(	const std::string& objName,
                    const std::vector<boost::gregorian::date>& dates,
                    const std::vector<double>& yearFractionsAsActAct,
                    const std::vector<double>& discountFactors,
                    const CurveBuildProperties& curveConvention );

        AQOCurve(	const std::string& objName,
                    const std::vector<boost::gregorian::date>& dates,
                    const std::vector<double>& yearFractionsAsActAct,
                    const std::vector<double>& discountFactors,
                    const std::vector<double>& forwardRates,
                    const CurveBuildProperties& curveConvention );

        void setData(   const std::vector<boost::gregorian::date>& dates,
                        const std::vector<double>& discountFactors,
                        const std::vector<double>& forwardRates );

        void setData(		const std::vector<boost::gregorian::date>& dates,
                            const std::vector<double>& discountFactors );

        void setData ( const  std::vector<double>& yearFractionsAsActAct,
                       const std::vector<double>& discountFactors,
                       const bool setDatesFromFractions = true );

        void setCurveBuildStaticDataObject( CurveBuildProperties&& curveConvention );

        void setCurveBuildStaticDataObject( const CurveBuildProperties& curveConvention );

        int getIndexOfDate( const boost::gregorian::date& date ) const;

        std::tuple<boost::gregorian::date, double, double> getData( const unsigned int idx ) const;

        std::string getName() const;

        void clear();

        const std::vector<boost::gregorian::date>& getDates() const;

        const std::vector<double>& getDiscountFactors() const;

        const std::vector<double>& getForwardRates() const;

        const CurveBuildProperties* getCurveBuildStaticDataObject() const;

        const std::vector<double>& getYearFractions() const;

        // implementatiion of the VariantMatrix Access from HasVariantMatrixAccess
        virtual VariantMatrix getVariantMatrix() const;
        virtual VariantMatrix getDiscountFactorMatrix() const;
        virtual VariantMatrix getForwardRateMatrix() const;

        //// implementation of the serialize interface from Cacheable
        //virtual SerializationResult serialize(	const serialize::SerializationMethodEnum method,
        //                                        const serialize::SerializationTargetEnum target,
        //                                        const std::string& targetInfo,
        //                                        std::vector<std::string>& variableNames,
        //                                        std::vector<Variant>& variableValues ) const;

        // using ACT_365 from the AsOfDate

        // this is the discount factor from a forward valuation date without any day adjustment to a future payment date without any day adjustment
        double calculateDiscountFactor( const boost::gregorian::date& valuationDate, const boost::gregorian::date& paymentDate ) const;

        // this is the discount factor from a forward valuation date to a future payment date with adjustment and calendar applied to both dates
        double calculateDiscountFactor(	const boost::gregorian::date& valuationDate, const boost::gregorian::date& paymentDate,
                                        const BusinessDayAdjustmentEnum dayAdjustment, const std::string& calendar ) const;

        // this is the discount factor from the AsOfDate to a future payment date without any day adjustment
        double calculateDiscountFactor( const boost::gregorian::date& paymentDate ) const;

        // this is the discount factor from the AsOfDate to a future payment date determined by adding the yearFraction to the AsOfDate without any day adjustment
        double calculateDiscountFactor( const double yearFraction ) const;

        // this is the discount factor from a future payment date (which will be adjusted) to term past that future date (which will not be adjusted)
        double calculateDiscountFactor( 
            const boost::gregorian::date& futurePaymentDate, 
            const double yearFraction, 
            const BusinessDayAdjustmentEnum dayAdjustment, 
            const std::string& calendar ) const;

        // this is the discount factor from a future payment date (which will be adjusted) to term past that future date (which will be adjusted)
        double calculateDiscountFactor( 
            const boost::gregorian::date& futurePaymentDate, 
            const std::string& termAsString, 
            const BusinessDayAdjustmentEnum dayAdjustment, 
            const std::string& calendar ) const;

        // this is the  discount factor from the AsOfDate to a future payment date determined by adding the tenorString to the AsOfDate without any day adjustment
        double calculateDiscountFactor( const std::string& tenorString ) const;

        // this is the  discount factor from the AsOfDate to a future payment date determined by adding the tenorString to the AsOfDate without the supplied adjustment and calendar
        double calculateDiscountFactor( const std::string& tenorString, const BusinessDayAdjustmentEnum dayAdjustment, const std::string& calendar ) const;

        // the same functions as above but in vector format
        std::vector<double> calculateDiscountFactor( const std::vector<boost::gregorian::date>& valuationDates, const std::vector<boost::gregorian::date>& paymentDates ) const;

        std::vector<double> calculateDiscountFactor( 
            const std::vector<boost::gregorian::date>& valuationDates, 
            const std::vector<boost::gregorian::date>& paymentDates,
            const BusinessDayAdjustmentEnum dayAdjustment, const std::string& calendar ) const;

        std::vector<double> calculateDiscountFactor( const std::vector<boost::gregorian::date>& paymentDates ) const;

        std::vector<double> calculateDiscountFactor( const std::vector<double>& yearFractions ) const;

        std::vector<double> calculateDiscountFactor( 
            const std::vector<boost::gregorian::date>& futurePaymentDates, 
            const std::vector<double>& yearFractions, 
            const BusinessDayAdjustmentEnum dayAdjustment, 
            const std::string& calendar ) const;

        std::vector<double> calculateDiscountFactor( const std::vector<std::string>& tenorStrings ) const;

        std::vector<double> calculateDiscountFactor( const std::vector<std::string>& tenorStrings, const BusinessDayAdjustmentEnum dayAdjustment, const std::string& calendar ) const;

        std::vector<double> calculateDiscountFactor( 
            const std::vector<boost::gregorian::date>& futurePaymentDate, 
            const std::vector<std::string>& termAsString, 
            const BusinessDayAdjustmentEnum dayAdjustment, 
            const std::string& calendar ) const;

        // this calculates the forward rate which is not necessarily of length equal to the curve tenor by calculating the forward rate using the discount factors
        // the first date can be interpreted as an accrualFrom and the second date as an accrualTo date.
        double calculateForwardRateUsingDiscountFactors(
            const boost::gregorian::date& accrualStartDate,
            const boost::gregorian::date& accrualEndDate,
            const DayCountEnum dayCount = etrading::ACT_ACT_DAYCOUNT,
            const CompoundingFrequencyEnum compFreq = etrading::ANNUAL_COMPOUNDING ) const;

        // this interpolates the forward rate whose period is the curve tenor given a fixing date (because the input dates are fixing dates, the interpolation happens with the x-axis based of fixing dates
        double calculateForwardRate( const boost::gregorian::date& fixingDate ) const;

        //  this interpolates the forward rate whose period is the curve tenor given a  year fraction which interpreted to go from the AsOfDate to an accrual start date
        double calculateForwardRate( const double yearFractionFromAsOfDateToAccrualStartDate ) const;

        //  this interpolates the forward rate whose period is the curve tenor given a tenor string which interpreted to go from the AsOfDate to a fixing date
        double calculateForwardRate( const std::string& tenorString ) const;

        // the same functions as above but in vector format
        std::vector<double> calculateForwardRateUsingDiscountFactors( const std::vector<boost::gregorian::date>& accrualFromDate,
                const std::vector<boost::gregorian::date>& accrualToDates,
                const DayCountEnum dayCount = etrading::ACT_ACT_DAYCOUNT,
                const CompoundingFrequencyEnum compFreq = etrading::ANNUAL_COMPOUNDING ) const;

        std::vector<double> calculateForwardRate( const std::vector<boost::gregorian::date>& fixingDate ) const;
        std::vector<double> calculateForwardRate( const std::vector<double>& yearFractionsAsOfDateToAccrualFromDate ) const;
        std::vector<double> calculateForwardRate( const std::vector<std::string>& tenorString ) const;
        void removeUnderlyingEntityPoolCurve() const;

    private:
        static const etrading::CachedObjectEnum ENUM_TYPE;
        const SchemaObject toSchemaObject() const;

        std::vector<double> datesAsYearFractions_;
        std::vector<boost::gregorian::date> dates_;
        std::vector<double> discountFactors_;
        std::vector<double> forwardRates_;
        std::shared_ptr<CurveBuildProperties> curveBuildProps_;

        static AscendingOrderRule< std::vector<boost::gregorian::date> > dateChecker_;
        static BoundaryRule<> posNumerChecker_;
        static AscendingOrderRule<> increasingYearFractions_;

        // for yearFractions posNumerChecker and increasingYearFractions can be combined into 1 checker but
        // then we still need a separate posNumerChecker for the discount factors so a null operation if we do that.


        void inspectDataFormat(	const std::vector<boost::gregorian::date>& dates,
                                const std::vector<double>& discountFactors,
                                const std::vector<double>& forwardRates ) const;

        bool setDates( const std::vector<boost::gregorian::date>& dates );

        bool setDiscountFactors( const std::vector<double>& discountFactors );

        bool setForwardRates( const std::vector<double>& forwardRates );

        bool isBeforeAsOf( const boost::gregorian::date& date ) const;

        void setYearFractions();
        void setDatesFromFractions();


        // changeable from const methods in case the interpolation method gets changed on the fly
        mutable std::shared_ptr<Interpolation> interpolationOnDiscountFactors_;
        mutable std::shared_ptr<Interpolation> interpolationOnForwardRates_;
        mutable InterpolationEnum lastInterpUsed_;

        void setInterpolationData( const etrading::InterpolationEnum interpMethod ) const; // not really const but to emphasize the const

        enum BusinessDayAdjustmentType
        {
            FIXING_BUSINESSDAYADJUSTMENT,
            ACCRUAL_BUSINESSDAYADJUSTMENT,
            PAYMENT_BUSINESSDAYADJUSTMENT
        };

        boost::gregorian::date adjustFromAsOfDateUsingTenorString( const std::string& tenorString, const BusinessDayAdjustmentType adjType = AQOCurve::PAYMENT_BUSINESSDAYADJUSTMENT ) const;

        const std::pair<const BusinessDayAdjustmentEnum, const AQLMathCalendar*> getBusinessDayAdjust( const BusinessDayAdjustmentType ) const;

        const AQLMathCalendar* getCalendar( const std::string& calendarName ) const;



    };





}

