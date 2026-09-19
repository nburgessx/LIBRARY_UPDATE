#include <limits>
#include <utility>
#include <boost/format.hpp>
#include <boost/date_time.hpp>

#include "AQObjCurve.h"
#include "ETradingException.h"
#include "ContainerUtilities.h"
#include "Variant.h"
#include "CurveBuildProperties.h"
#include "DateUtilities.h"
#include "InterpolationFactory.h"
#include "AQObjUtilities.h"
#include "AQLDateScheduleHelpers.h"
#include "EntityPoolUtilities.h"
#include "AQLMathInterpolationUtilities.h"

namespace etrading
{
    
    AscendingOrderRule< std::vector<boost::gregorian::date> >  AQObjCurve::dateChecker_  ;  // static

    BoundaryRule<> AQObjCurve::posNumerChecker_( 0.0, std::numeric_limits<double>::max() ); // static
    
    AscendingOrderRule<> AQObjCurve::increasingYearFractions_;  // static
    
    const etrading::CachedObjectEnum AQObjCurve::ENUM_TYPE = etrading::CURVE_DEPRECATED; // static

    // TODO: use delegating constructors in C++11

    // CurveBuildProperties is a unique_ptr so it needs to be deep copied
    AQObjCurve::AQObjCurve( const AQObjCurve& instance )
        : IsAQObject( instance.getName() ,AQObjCurve::ENUM_TYPE ),
          HasCurveData( instance ),
          curveBuildProps_( instance.curveBuildProps_ ? new CurveBuildProperties( *( instance.curveBuildProps_.get() ) ) : nullptr ),
          dates_( instance.getDates() ),
          discountFactors_( instance.getDiscountFactors() ),
          forwardRates_( instance.getForwardRates() ),
          datesAsYearFractions_( instance.datesAsYearFractions_ )
    {
    }

    AQObjCurve::AQObjCurve( const std::string& objName )
        :   IsAQObject( objName, AQObjCurve::ENUM_TYPE ), 
            curveBuildProps_(),
            interpolationOnDiscountFactors_(), interpolationOnForwardRates_(),
            dates_(  0 ), discountFactors_( 0 ), forwardRates_( 0 ), datesAsYearFractions_( 0 )
    {};

    AQObjCurve::AQObjCurve( AQObjCurve&& moved )
        :  IsAQObject( moved.getName() , AQObjCurve::ENUM_TYPE ),
           HasCurveData( std::ref(moved)), 
           curveBuildProps_( std::move( moved.curveBuildProps_ ) ),
           dates_( std::move( moved.getDates() ) ),
           discountFactors_( std::move( moved.getDiscountFactors() ) ),
           forwardRates_( std::move( moved.getForwardRates() ) ),
           interpolationOnDiscountFactors_( std::move( moved.interpolationOnDiscountFactors_ ) ),
           interpolationOnForwardRates_( std::move( moved.interpolationOnForwardRates_ ) ),
           datesAsYearFractions_( std::move( moved.datesAsYearFractions_ ) )
    {

    };

    AQObjCurve::AQObjCurve(	const std::string& objName,
                        const std::vector<boost::gregorian::date>& dates,
                        const std::vector<double>& discountFactors,
                        const CurveBuildProperties& curveConvention,
                        const std::vector<double>& forwardRates )
        : IsAQObject( objName , AQObjCurve::ENUM_TYPE )
    {
        setData( dates, discountFactors, forwardRates );
        setCurveBuildStaticDataObject( curveConvention );
        setYearFractions();
    };

    AQObjCurve::AQObjCurve(	const std::string& objName,
                        const std::vector<double>& yearFractionsAsActAct,
                        const std::vector<double>& discountFactors,
                        const CurveBuildProperties& curveConvention )
        : IsAQObject( objName, AQObjCurve::ENUM_TYPE ),
          interpolationOnDiscountFactors_(), interpolationOnForwardRates_(),
          curveBuildProps_( new CurveBuildProperties( curveConvention ) ),
          datesAsYearFractions_( 0 ), discountFactors_( 0 ), dates_( 0 ), forwardRates_( 0 )
    {
        datesAsYearFractions_.reserve( yearFractionsAsActAct.size() );
        discountFactors_.reserve( discountFactors.size() );
        dates_.reserve( discountFactors.size() );
        setData( yearFractionsAsActAct, discountFactors ); // used for checking as well
    };

    AQObjCurve::AQObjCurve(	const std::string& objName,
                        const std::vector<boost::gregorian::date>& dates,
                        const std::vector<double>& yearFractionsAsActAct,
                        const std::vector<double>& discountFactors,
                        const CurveBuildProperties& curveConvention )
        : IsAQObject( objName, AQObjCurve::ENUM_TYPE ),
          interpolationOnDiscountFactors_(), interpolationOnForwardRates_(),
          curveBuildProps_( new CurveBuildProperties( curveConvention ) ),
          datesAsYearFractions_( 0 ), discountFactors_( 0 ), dates_( 0 ), forwardRates_( 0 )
    {
        if( dates.size() != yearFractionsAsActAct.size() )
        {
            { std::ostringstream aqMsg29;
aqMsg29 << "AQObjCurve::AQObjCurve: Dates and Year Fractions are not of the same size (" << dates.size() << " and " << yearFractionsAsActAct.size() << " respectively)"; AQ_THROW( aqMsg29.str() ); }
        }
        dates_.reserve( discountFactors.size() );
        if( !setDates( dates ) )
        {
            { std::ostringstream aqMsg30;
aqMsg30 << "Dates in the CTOR of AQObjCurve are not in ascending order - verify: " << containerAsString( getDisplayableContainer( dates ) ); AQ_THROW( aqMsg30.str() ); }
        }
        datesAsYearFractions_.reserve( yearFractionsAsActAct.size() );
        discountFactors_.reserve( discountFactors.size() );
        setData( yearFractionsAsActAct, discountFactors, false ); // used for checking as well
    };

    AQObjCurve::AQObjCurve(	const std::string& objName,
                        const std::vector<boost::gregorian::date>& dates,
                        const std::vector<double>& yearFractionsAsActAct,
                        const std::vector<double>& discountFactors,
                        const std::vector<double>& forwardRates,
                        const CurveBuildProperties& curveConvention )
        :	IsAQObject( objName, AQObjCurve::ENUM_TYPE ),
          interpolationOnDiscountFactors_(), interpolationOnForwardRates_(),
          curveBuildProps_( new CurveBuildProperties( curveConvention ) ),
          datesAsYearFractions_( 0 ), discountFactors_( 0 ), dates_( 0 ), forwardRates_( 0 )
    {
        if( dates.size() != yearFractionsAsActAct.size() )
        {
            { std::ostringstream aqMsg31;
aqMsg31 << "AQObjCurve::AQObjCurve: Dates and Year Fractions are not of the same size (" << dates.size() << " and " << yearFractionsAsActAct.size() << " respectively)"; AQ_THROW( aqMsg31.str() ); }
        }
        if( forwardRates.size() != yearFractionsAsActAct.size() )
        {
            { std::ostringstream aqMsg32;
aqMsg32 << "AQObjCurve::AQObjCurve: Forward Rates and Year Fractions are not of the same size (" << forwardRates.size() << " and " << yearFractionsAsActAct.size() << " respectively)"; AQ_THROW( aqMsg32.str() ); }
        }
        dates_.reserve( discountFactors.size() );
        if( !setDates( dates ) )
        {
            { std::ostringstream aqMsg33;
aqMsg33 << "Dates in the CTOR of AQObjCurve are not in ascending order - verify: " << containerAsString( getDisplayableContainer( dates ) ); AQ_THROW( aqMsg33.str() ); }
        }
        datesAsYearFractions_.reserve( yearFractionsAsActAct.size() );
        discountFactors_.reserve( discountFactors.size() );
        setData( yearFractionsAsActAct, discountFactors, false ); // used for checking as well
        setForwardRates( forwardRates );
    };

    AQObjCurve::~AQObjCurve()
    {
		// we used to call removeUnderlyingEntityPoolCurve() here
        // but that will cause an issue on moving an AQObjCurve or when a locally created AQObj curve gets destroyed
	};

    void AQObjCurve::removeUnderlyingEntityPoolCurve() const
    {
        if( curveBuildProps_ )
        {
            AQLString curveCollection = curveBuildProps_->curveCollectionName_.c_str();
            AQLString curveIndex = curveBuildProps_->curveIndexName_.c_str();
            if(etrading::isCurveRegistered(curveCollection))
            {
                etrading::removeCurveFromEntityPool(curveCollection,curveIndex);
            }
        }
    };

    void AQObjCurve::inspectDataFormat(	const std::vector<boost::gregorian::date>& dates,
                                        const std::vector<double>& discountFactors,
                                        const std::vector<double>& forwardRates ) const
    {
        if( discountFactors.size() != dates.size() )
        {
            { std::ostringstream aqMsg34;
aqMsg34 << "The Dates and DiscountFactors in the CTOR of the AQObjCurve are not of the same length " << dates.size() << " (Dates) and " << discountFactors.size() << " (DiscountFactors)"; AQ_THROW( aqMsg34.str() ); }
        };

        if( forwardRates.size() > 0 )
        {
            if( forwardRates.size() != dates.size() )
            {
                { std::ostringstream aqMsg35;
aqMsg35 << "The number of forward rates needs to equal the number of dates in the CTOR of the AQObjCurve if the forward rates are set, current sizes: " << dates.size() << " (Dates) and " << forwardRates.size() << " (forwardRates)"; AQ_THROW( aqMsg35.str() ); }
            }
        };

        // TODO: because non business days are supplied to OIS in AlgoQuantLib we do not check this for compatabilitiy.... FIX this on both sides.
        /*
        if(curveBuildProps_ && dates.size() > 0)
        {
        	const auto aqCalendar = curveBuildProps_->getAqFixingCalendar();
        	if(aqCalendar != nullptr)
        	{
        		auto non_bus_day = std::find_if(dates.cbegin(), dates.cend(),
        															[&aqCalendar](const boost::gregorian::date& dateToCheck) {  return !isBusinessDay(dateToCheck, *aqCalendar); } );
        		if(non_bus_day != dates.cend())
        		{
        			throw ETradingException( ( boost::format( "A supplied fixing date (%s) falls on a non-businessday according to the calendar %s" )
        															% toYYYYMMDDFromGregorianDate(*non_bus_day).c_str() % curveBuildProps_->getFixingDayCalendar().c_str()).str() );
        		}
        	} else
        	{
        		auto non_bus_day = std::find_if(dates.cbegin(), dates.cend(), [](const boost::gregorian::date& dateToCheck) {  return !isWeekend(dateToCheck); } );
        		if(non_bus_day != dates.cend())
        		{
        			throw ETradingException( ( boost::format( "A supplied fixing date falls on a weekend: %s" ) % toYYYYMMDDFromGregorianDate(*non_bus_day).c_str() ).str() );
        		}
        	}
        }
        */

    };

    void AQObjCurve::setData(	const std::vector<boost::gregorian::date>& dates,
                            const std::vector<double>& discountFactors,
                            const std::vector<double>& forwardRates )
    {
        inspectDataFormat( dates, discountFactors, forwardRates );
        if( !setDates( dates ) )
        {
            { std::ostringstream aqMsg36;
aqMsg36 << "Dates in the CTOR of AQObjCurve are not in ascending order - verify: " << containerAsString( getDisplayableContainer( dates ) ); AQ_THROW( aqMsg36.str() ); }
        }
        
        setDiscountFactors( discountFactors );
        // TODO: Temporarily relax this check for backwards compatibility ... this is a good check that needs to be reinstated once the curve extrapolation has been fixed
        // Currently Tenor basis curves can extrapolate negatively, which needs resolving, upon which this fix should be re-instated.

        /*if( !setDiscountFactors( discountFactors ) )
        {
            throw ETradingException( ( boost::format( "One or more DiscountFactors in the CTOR of AQObjCurve were not positive - verify: %s" )
                                       % containerAsString( getDisplayableContainer( discountFactors ) ) ).str() );
        }*/
        
        setForwardRates( forwardRates );

        setYearFractions();
    };

    void AQObjCurve::setData ( const  std::vector<double>& yearFractionsAsActAct,
                             const std::vector<double>& discountFactors,
                             const bool setDatesFromFractions )
    {
        if( !AQObjCurve::posNumerChecker_.verify( yearFractionsAsActAct ) || !AQObjCurve::increasingYearFractions_.verify( yearFractionsAsActAct ) )
        {
            { std::ostringstream aqMsg37;
aqMsg37 << "Invalid curve dates. Curve nodes are in the past or decreasing with time"; AQ_THROW( aqMsg37.str() ); }
        }
        datesAsYearFractions_ = 	yearFractionsAsActAct;
        
        setDiscountFactors( discountFactors );
        // TODO: Temporarily relax this check for backwards compatibility ... this is a good check that needs to be reinstated once the curve extrapolation has been fixed
        // Currently Tenor basis curves can extrapolate negatively, which needs resolving, upon which this fix should be re-instated.
       /* if( !setDiscountFactors( discountFactors ) )
        {
            throw ETradingException( ( boost::format( "One or more DiscountFactors in the CTOR of AQObjCurve were not positive - verify: %s" )
                                       % containerAsString( getDisplayableContainer( discountFactors ) ) ).str() );
        }*/

        if( setDatesFromFractions )
        {
            this->setDatesFromFractions();
        }
    };

    bool AQObjCurve::setDates( const std::vector<boost::gregorian::date>& dates )
    {
        if( AQObjCurve::dateChecker_.verify( dates ) )
        {
            dates_ = dates;
            return true;
        }
        return false;
    };

    bool AQObjCurve::setDiscountFactors( const std::vector<double>& discountFactors )
    {

        discountFactors_ = discountFactors;
        return true;

        // TODO: This is a good check ... that we are relaxing temporarily, since Tenor Basis Curves typically only
        // go out 30 years and we are checking for 50 years of daily discount factors which extrapolate negatively.
        // Once the tenor basis extrapolation has been fixed we should reinstate this check and delete the above.
        
        /*if( AQObjCurve::posNumerChecker_.verify( discountFactors ) )
        {
            discountFactors_ = discountFactors;
            return true;
        }
        
        return false;*/
    };

    bool AQObjCurve::setForwardRates( const std::vector<double>& forwardRates )
    {
        // it is OK if the forward rates are an empty vector
        forwardRates_ = forwardRates;
        return true;
    };

    void AQObjCurve::setCurveBuildStaticDataObject( const CurveBuildProperties& cbp )
    {
        curveBuildProps_.reset( new CurveBuildProperties( cbp ) );
        inspectDataFormat( dates_, discountFactors_, forwardRates_ );
        setYearFractions();
    };

    void AQObjCurve::setCurveBuildStaticDataObject( CurveBuildProperties&& curveConvention )
    {
        curveBuildProps_.reset( &curveConvention );
        inspectDataFormat( dates_, discountFactors_, forwardRates_ );
        setYearFractions();
    };

    void AQObjCurve::setYearFractions()
    {
        if( curveBuildProps_ && dates_.size() > 0 )
        {
            unsigned int numOfDates = dates_.size();
            if( numOfDates  > 0 )
            {
                datesAsYearFractions_ = std::vector<double>( numOfDates, 0.0 );
                const auto asOfDate = curveBuildProps_.get()->asOfDate_;
                for( unsigned int dateCounter = 0; dateCounter < numOfDates; dateCounter++ )
                {
                    datesAsYearFractions_[ dateCounter ] = getYearFractionFromDayCount( etrading::ACT_ACT_DAYCOUNT, asOfDate, dates_[dateCounter ] );
                }
            }
            setInterpolationData( curveBuildProps_->interpMethod_ );
        }
    };

    void AQObjCurve::setDatesFromFractions()
    {
        if( curveBuildProps_ != nullptr && datesAsYearFractions_.size() > 0 )
        {
            unsigned int numOfDates = datesAsYearFractions_.size();
            const auto asOfDate = curveBuildProps_.get()->asOfDate_;
            if( numOfDates  > 0 )
            {
                dates_ = std::vector<boost::gregorian::date>( numOfDates );
                const auto asOfDate = curveBuildProps_.get()->asOfDate_;
                for( unsigned int dateCounter = 0; dateCounter < numOfDates; dateCounter++ )
                {
                    dates_[ dateCounter ] = etrading::addYearFraction( asOfDate, datesAsYearFractions_[ dateCounter ] );
                }
            }
            setInterpolationData( curveBuildProps_->interpMethod_ );
        }
    };

    void AQObjCurve::setInterpolationData( const etrading::InterpolationEnum interpMethod ) const
    {
        if( dates_.size() > 0 )
        {
			interpolationOnDiscountFactors_.reset();
            if( !curveBuildProps_ ->isOnlyAllowLookup() )
            {
                interpolationOnDiscountFactors_ = InterpolationFactory::getInstance().generateInterpolation( datesAsYearFractions_, discountFactors_, interpMethod );
                if( forwardRates_.size() == dates_.size() )
                {
                    interpolationOnForwardRates_ = InterpolationFactory::getInstance().generateInterpolation( datesAsYearFractions_, forwardRates_, interpMethod );
                }
            }
            lastInterpUsed_ = interpMethod;
        }
    };

    int AQObjCurve::getIndexOfDate( const boost::gregorian::date& date ) const
    {
        return getIndexOf( date, dates_ );
    };

    std::tuple<boost::gregorian::date, double, double> AQObjCurve::getData( const unsigned int idx ) const
    {
        boost::gregorian::date date = dates_[ idx ];
        double dfRate = discountFactors_[ idx ];
        double fwdRate = ( forwardRates_.size() >  idx ) ? forwardRates_[ idx ] : std::numeric_limits<double>::quiet_NaN();
        return std::make_tuple( date, dfRate, fwdRate );
    };

    std::string AQObjCurve::getName() const
    {
        return getRefToName();
    };

    const std::vector<boost::gregorian::date>& AQObjCurve::getDates() const
    {
        return dates_;
    };

    const std::vector<double>& AQObjCurve::getDiscountFactors() const
    {
        return discountFactors_;
    };

    const std::vector<double>& AQObjCurve::getForwardRates() const
    {
        return forwardRates_;
    };

    const CurveBuildProperties* AQObjCurve::getCurveBuildStaticDataObject() const
    {
        return curveBuildProps_.get();
    };
    //SerializationResult AQObjCurve::serialize(
    //    const serialize::SerializationMethodEnum method,
    //    const serialize::SerializationTargetEnum target,
    //    const std::string& targetInfo,
    //    std::vector<std::string>& variableNames,
    //    std::vector<Variant>& variableValues ) const
    //{
    //    return toSchemaObject().serialize( method, target, targetInfo, variableNames, variableValues );
    //};

    const SchemaObject AQObjCurve::toSchemaObject() const
    {
        SchemaObject schemaObject( etrading::CURVE_DEPRECATED, getRefToName() );
        // TODO: remove hard-coded string or "RATES", etc.

        auto schemaNames = schemaObject.keyNames();

        const std::string RATES = schemaNames[ 1 ];
        schemaObject.setColumnData( RATES, 0, dates_ ); // or ("RATES","DATE_VALUE", dates)
        schemaObject.setColumnData( RATES, 1, discountFactors_ ); // or ("RATES","DISCOUNT_FACTORS", dates)
        if( forwardRates_.size()  == dates_.size() )
        {
            schemaObject.setColumnData( RATES, 2, forwardRates_ ); // or ("RATES","FORWARD_RATES", dates)
        }
        else
        {
            schemaObject.setColumnData( RATES, 2, std::vector<double>( dates_ .size(), -9999.99 )   );
            // or ("RATES","FORWARD_RATES", dates)
        }

        if( curveBuildProps_ != nullptr )
        {
            schemaObject.adoptDataSchema( curveBuildProps_.get()->toSchemaObject() );
        }
        else
        {
            AQ_THROW( "AQObjCurve::toSchemaObject() - Cannot convert to SchemaObject object when CurveBuildProperties have not been set" );
        }
        return schemaObject;
    };

    bool AQObjCurve::isBeforeAsOf( const boost::gregorian::date& date ) const
    {
        if( !curveBuildProps_ )
        {
            AQ_THROW( "No CurveBuildProperties Supplied" );
        }
        return date < curveBuildProps_.get()->asOfDate_;
    };

    const std::vector<double>& AQObjCurve::getYearFractions() const
    {
        if( interpolationOnDiscountFactors_ )
        {
            return datesAsYearFractions_;
        }
        else
        {
            if( curveBuildProps_ && dates_.size() > 0 && discountFactors_.size() > 0 )
            {
                setInterpolationData( curveBuildProps_->interpMethod_ );
            }

            if( interpolationOnDiscountFactors_ )
            {
                return datesAsYearFractions_;
            }
            else
            {
                AQ_THROW( "Unable to get YearFractions because no CurveBuildProperties or input data has been set" );
            }
        }
    };

    // using ACT_365 from the AsOfDate
    double AQObjCurve::calculateDiscountFactor( const boost::gregorian::date& toDate ) const
    {
        const boost::gregorian::date asOfDate = curveBuildProps_.get()->asOfDate_;

        if( asOfDate == toDate )
        {
            return 1.0;
        }

        if( toDate < asOfDate )
        {
            { std::ostringstream aqMsg41;
aqMsg41 << "AQObjCurve::calculateDiscountFactor(date) => Date supplied (" << toYYYYMMDDFromGregorianDate( toDate ) << ") lies before asOfDate(" << toYYYYMMDDFromGregorianDate( asOfDate ) << ")"; AQ_THROW( aqMsg41.str() ); }
        }


        int toDateIdx = getIndexOfDate( toDate );
        if( toDateIdx >= 0 )
        {
            return discountFactors_[ toDateIdx ];
        }
        else
        {
            double yearFractionForToDate = getYearFractionFromDayCount( etrading::ACT_ACT_DAYCOUNT, asOfDate, toDate );
            // call the interpolate in Entitypool = AQLMathInterpolationUtilities::interpolate
            return calculateDiscountFactor( yearFractionForToDate );
        }
    };

    double AQObjCurve::calculateDiscountFactor( const double yearFraction ) const
    {
        if ( yearFraction < 0.0 )
        {
            { std::ostringstream aqMsg42;
aqMsg42 << "Term starting from AsOfDate must be positive but received yearFraction of : " << yearFraction; AQ_THROW( aqMsg42.str() ); }
        }
        // anything less than a day is not worth interpolatng
        if ( yearFraction < 1.0 / 366.0 )
        {
            return 1.0;
        }

        if( curveBuildProps_ == nullptr )
        {
            { std::ostringstream aqMsg43;
aqMsg43 << "AQObjCurve::calculateDiscountFactor(yearFraction): Cannot calculate Discount Factor because not CurveBuildProperties were set; yearFraction (" << yearFraction << ")"; AQ_THROW( aqMsg43.str() ); }
        }

        if( !curveBuildProps_->isOnlyAllowLookup() )
        {
            
            // Discount Factors stored at Node Points Only
            // ------------------------------------------
            
            // Native AlgoQuantLib Interpolation
            // -------------------------
            const InterpolationEnum interpMethod = curveBuildProps_->interpMethod_;
            double result = AQLMathInterpolationUtilities::interpolate( datesAsYearFractions_, discountFactors_, yearFraction, interpMethod );
            return result;

            // AQObj Independent Interpolation
            // -----------------------------

            // const InterpolationEnum interpMethod = curveBuildProps_->interpMethod_;
            //if( interpMethod != lastInterpUsed_ || interpolationOnDiscountFactors_ == nullptr )
            //{
            //    setInterpolationData( interpMethod );
            //}
            //return interpolationOnDiscountFactors_.get()->interpolate( yearFraction );*/
            
        }
        else
        {
            // Daily Discount Factors Stored
            // ------------------------------
            
            // This section doesn't do any interpolation and is a simple look-up only. This works in the
            // assumption that we have daily discount factors for all data points of interest.            
            
            const boost::gregorian::date asOfDate = curveBuildProps_.get()->asOfDate_;
            auto dateToEvaluate = addYearFraction( asOfDate, yearFraction );
            if( dateToEvaluate >= dates_[ dates_.size() - 1 ] )
            {
                // assume constant past the 51 years
                return discountFactors_[ discountFactors_.size() - 1 ];
            }
            else
            {
                //auto idxToRetrieve = getIndexOfDate(dateToEvaluate); // is better but the below is to mimic the old AlgoQuantLib behaviour
                auto idxToRetrieve = getIndexWithPrecision( datesAsYearFractions_, yearFraction );
                if( idxToRetrieve < 0 )
                {
                    { std::ostringstream aqMsg44;
aqMsg44 << "AQObjCurve::calculateDiscountFactor(yearFraction) LOOKUPONLY : Cannot find discount factor matching date (" << toYYYYMMDDFromGregorianDate( dateToEvaluate ).c_str() << ")"; AQ_THROW( aqMsg44.str() ); }
                }
                else
                {
                    return discountFactors_[ idxToRetrieve ];
                }
            }
        }
    };

    // this is the discount factor from a future payment date (which will be adjusted) to term past that future date (which will not be adjusted)
    double AQObjCurve::calculateDiscountFactor(
        const boost::gregorian::date& futurePaymentDate,
        const double yearFraction,
        const BusinessDayAdjustmentEnum dayAdjustment,
        const std::string& calendar ) const
    {
        // boost::gregorian::date date = curveBuildProps_.get()->asOfDate_;
        auto ptrAqCalendar = getCalendar( trim_to_upper( calendar.c_str() ) );
        if( ptrAqCalendar )
        {
            const boost::gregorian::date asOfDate = curveBuildProps_.get()->asOfDate_;
            boost::gregorian::date fromDate = dayAdjust( futurePaymentDate, dayAdjustment, *ptrAqCalendar );
            double yearFractionForFromDate = getYearFractionFromDayCount( etrading::ACT_ACT_DAYCOUNT, asOfDate, fromDate );
            double dfFromDate = calculateDiscountFactor( yearFractionForFromDate );
            double yearFractionForToDate = yearFractionForFromDate + yearFraction;
            double dfToDate = calculateDiscountFactor( yearFractionForToDate );
            return dfToDate / dfFromDate; // time fraction
        }
        else
        {
            { std::ostringstream aqMsg45;
aqMsg45 << "Unable to retrieve an AlgoQuantLib calendar specified as " << calendar.c_str(); AQ_THROW( aqMsg45.str() ); }
        }
    };

    double AQObjCurve::calculateDiscountFactor(
        const boost::gregorian::date& futurePaymentDate,
        const std::string& termAsString,
        const BusinessDayAdjustmentEnum dayAdjustment,
        const std::string& calendar ) const
    {
        // boost::gregorian::date date = curveBuildProps_.get()->asOfDate_;
        const boost::gregorian::date asOfDate = curveBuildProps_.get()->asOfDate_;
        boost::gregorian::date endDate = addTenorString( futurePaymentDate, termAsString );
        double dfBetweenDates = calculateDiscountFactor( futurePaymentDate, endDate, dayAdjustment, calendar ); // both dates will be adjusted
        return dfBetweenDates;
    };

    double AQObjCurve::calculateDiscountFactor( const boost::gregorian::date& fromDate, const boost::gregorian::date& toDate ) const
    {
        double fromDF = calculateDiscountFactor( fromDate );
        double toDF = calculateDiscountFactor( toDate );
        return toDF / fromDF; // time fraction
    };

    double AQObjCurve::calculateDiscountFactor( const boost::gregorian::date& valuationDate, const boost::gregorian::date& paymentDate,
            const BusinessDayAdjustmentEnum dayAdjustment, const std::string& calendar ) const
    {
        auto ptrAqCalendar = getCalendar( trim_to_upper( calendar.c_str() ) );
        if( ptrAqCalendar )
        {
            auto fromDate = dayAdjust( valuationDate, dayAdjustment, *ptrAqCalendar );
            auto toDate = dayAdjust( paymentDate, dayAdjustment, *ptrAqCalendar );
            return calculateDiscountFactor( fromDate, toDate );
        }
        else
        {
            { std::ostringstream aqMsg46;
aqMsg46 << "Unable to retrieve an AlgoQuantLib calendar specified as " << calendar.c_str(); AQ_THROW( aqMsg46.str() ); }
        }
    };

    double AQObjCurve::calculateDiscountFactor( const std::string& tenorString ) const
    {
        boost::gregorian::date date = adjustFromAsOfDateUsingTenorString( tenorString );
        return calculateDiscountFactor( date );
    };

    double AQObjCurve::calculateDiscountFactor( const std::string& tenorString, const BusinessDayAdjustmentEnum dayAdjustment, const std::string& calendar ) const
    {
        boost::gregorian::date date = curveBuildProps_.get()->asOfDate_;
        date = addTenorString( date, tenorString );

        if( dayAdjustment != NO_CHANGE )
        {
            auto ptrAqCalendar = getCalendar( trim_to_upper( calendar.c_str() ) );
            if( ptrAqCalendar )
            {
                date = dayAdjust( date, dayAdjustment, *ptrAqCalendar );
                return calculateDiscountFactor( date );
            }
            else
            {
                { std::ostringstream aqMsg47;
aqMsg47 << "Unable to retrieve an AlgoQuantLib calendar specified as " << calendar.c_str(); AQ_THROW( aqMsg47.str() ); }
            }
        }
        else
        {
            return calculateDiscountFactor( date );
        }
    };

    std::vector<double> AQObjCurve::calculateDiscountFactor(
        const std::vector<boost::gregorian::date>& valuationDates,
        const std::vector<boost::gregorian::date>& paymentDates ) const
    {
        if( valuationDates.size() != paymentDates.size() )
        {
            { std::ostringstream aqMsg48;
aqMsg48 << "AQObjCurve::calculateDiscountFactor(valuationDates, paymentDates) ; the number of valuationDates (" << valuationDates.size() << ") does not equal the number of paymentDates (" << paymentDates.size() << ")"; AQ_THROW( aqMsg48.str() ); }
        }
        std::vector<double> retVec(valuationDates.size()); // reserve the size
        for( unsigned int dateCounter = 0u; dateCounter < valuationDates.size(); dateCounter++ )
        {
            // double dfHere = calculateDiscountFactor( valuationDates[ dateCounter ], paymentDates[ dateCounter ] );
            // retVec.push_back( dfHere );
			retVec[dateCounter] = calculateDiscountFactor( valuationDates[ dateCounter ], paymentDates[ dateCounter ] );
        }
        return retVec;
    };

    // TODO: implement more efficiently - don't be lazy
    std::vector<double> AQObjCurve::calculateDiscountFactor(
        const std::vector<boost::gregorian::date>& valuationDates,
        const std::vector<boost::gregorian::date>& paymentDates,
        const BusinessDayAdjustmentEnum dayAdjustment, const std::string& calendar ) const
    {
        if( valuationDates.size() != paymentDates.size() )
        {
            { std::ostringstream aqMsg49;
aqMsg49 << "AQObjCurve::calculateDiscountFactor(valuationDates, paymentDates) ; the number of valuationDates (" << valuationDates.size() << ") does not equal the number of paymentDates (" << paymentDates.size() << ")"; AQ_THROW( aqMsg49.str() ); }
        }
        std::vector<double> retVec;
        for( unsigned int dateCounter = 0u; dateCounter < valuationDates.size(); dateCounter++ )
        {
            double dfHere = calculateDiscountFactor( valuationDates[ dateCounter ], paymentDates[ dateCounter ], dayAdjustment, calendar );
            retVec.push_back( dfHere );
        }
        return retVec;
    }

    std::vector<double> AQObjCurve::calculateDiscountFactor( const std::vector<boost::gregorian::date>& paymentDates ) const
    {
        // TODO: would this get sped up if passed the asOfDate (less checks)
        std::vector<double> retVec;
        for( unsigned int dateCounter = 0u; dateCounter < paymentDates.size(); dateCounter++ )
        {
            double dfHere = calculateDiscountFactor( paymentDates[ dateCounter ] );
            retVec.push_back( dfHere );
        }
        return retVec;
    };

    std::vector<double> AQObjCurve::calculateDiscountFactor( const std::vector<double>& yearFractions ) const
    {
        std::vector<double> retVec;
        for( unsigned int fractionCounter = 0u; fractionCounter < yearFractions.size(); fractionCounter++ )
        {
            double dfHere = calculateDiscountFactor( yearFractions[ fractionCounter ] );
            retVec.push_back( dfHere );
        }
        return retVec;
    }

    std::vector<double> AQObjCurve::calculateDiscountFactor( const std::vector<boost::gregorian::date>& futurePaymentDates, const std::vector<double>& yearFractions, const BusinessDayAdjustmentEnum dayAdjustment, const std::string& calendar ) const
    {
        if( yearFractions.size() != futurePaymentDates.size() )
        {
            { std::ostringstream aqMsg50;
aqMsg50 << "AQObjCurve::calculateDiscountFactor(futurePaymentDates, yearFractions,dayAdjustment,calendar) ; the number of futurePaymentDates (" << futurePaymentDates.size() << ") does not equal the number of yearFractions (" << yearFractions.size() << ")"; AQ_THROW( aqMsg50.str() ); }
        }
        std::vector<double> retVec;
        for( unsigned int fractionCounter = 0u; fractionCounter < yearFractions.size(); fractionCounter++ )
        {
            double dfHere = calculateDiscountFactor( futurePaymentDates[ fractionCounter ], yearFractions[ fractionCounter ], dayAdjustment, calendar );
            retVec.push_back( dfHere );
        }
        return retVec;
    };

    std::vector<double> AQObjCurve::calculateDiscountFactor( const std::vector<std::string>& tenorStrings ) const
    {
        std::vector<double> retVec;
        for( unsigned int tenorStringCounter = 0u; tenorStringCounter < tenorStrings.size(); tenorStringCounter++ )
        {
            double dfHere = calculateDiscountFactor( tenorStrings[ tenorStringCounter ] );
            retVec.push_back( dfHere );
        }
        return retVec;
    };

    std::vector<double> AQObjCurve::calculateDiscountFactor( const std::vector<std::string>& tenorStrings, const BusinessDayAdjustmentEnum dayAdjustment, const std::string& calendar ) const
    {
        // TODO: can get sped up by passing asOfDate and the AlgoQuantLib common calendar pointer
        std::vector<double> retVec;
        for( unsigned int dateCounter = 0u; dateCounter < tenorStrings.size(); dateCounter++ )
        {
            double dfHere = calculateDiscountFactor( tenorStrings[ dateCounter ], dayAdjustment, calendar );
            retVec.push_back( dfHere );
        }
        return retVec;
    };

    std::vector<double> AQObjCurve::calculateDiscountFactor(
        const std::vector<boost::gregorian::date>& futurePaymentDates,
        const std::vector<std::string>& termsAsString,
        const BusinessDayAdjustmentEnum dayAdjustment,
        const std::string& calendar ) const
    {
        if( futurePaymentDates.size() != termsAsString.size() )
        {
            { std::ostringstream aqMsg51;
aqMsg51 << "Number of dates (" << futurePaymentDates.size() << ") does not equal the number of terms (" << termsAsString.size() << ")"; AQ_THROW( aqMsg51.str() ); }
        }
        std::vector<double> retVec;
        for( unsigned int dateCounter = 0u; dateCounter < termsAsString.size(); dateCounter++ )
        {
            double dfHere = calculateDiscountFactor( futurePaymentDates[ dateCounter ], termsAsString[ dateCounter ], dayAdjustment, calendar );
            retVec.push_back( dfHere );
        }
        return retVec;
    };

    double AQObjCurve::calculateForwardRateUsingDiscountFactors(
        const boost::gregorian::date& accrualFromDate,
        const boost::gregorian::date& accrualToDate,
        const DayCountEnum dayCount,
        const CompoundingFrequencyEnum compFreq ) const
    {
        if( accrualFromDate >= accrualToDate )
        {
            { std::ostringstream aqMsg52;
aqMsg52 << "AQObjCurve::calculateForwardRateUsingDiscountFactors  Accrual Start Date (" << toYYYYMMDDFromGregorianDate( accrualFromDate ).c_str() << ") is on or after Accrual End Date (" << toYYYYMMDDFromGregorianDate( accrualToDate ).c_str() << ")"; AQ_THROW( aqMsg52.str() ); }
        }
        double fromDF = calculateDiscountFactor( accrualFromDate );
        double toDF = calculateDiscountFactor( accrualToDate );
        double termAsDoubleForBetweenDates = getYearFractionFromDayCount( dayCount, accrualFromDate, accrualToDate );
        // getYearFractionFromDayCount( etrading::ACT_ACT_DAYCOUNT, accrualFromDate, accrualToDate );
        return etrading::getRateFromReturn( fromDF / toDF, termAsDoubleForBetweenDates, compFreq );
        // return (fromDF/toDF  - 1.0)*(1.0/termAsDoubleForBetweenDates); // time fraction
    };

    double AQObjCurve::calculateForwardRate( const boost::gregorian::date& unadjustedFixingDate ) const
    {
        if( curveBuildProps_ == nullptr )
        {
            AQ_THROW( "AQObjCurve::calculateForwardRate(fixingDate) : Cannot calculate because CurveBuildProperties were not set" );
        }

        const boost::gregorian::date asOfDate = curveBuildProps_.get()->asOfDate_;
        if( unadjustedFixingDate < asOfDate )
        {
            { std::ostringstream aqMsg54;
aqMsg54 << "AQObjCurve::calculateForwardRate(date) => unadjustedFixingDate supplied (" << toYYYYMMDDFromGregorianDate( unadjustedFixingDate ) << ") lies before asOfDate (" << toYYYYMMDDFromGregorianDate( asOfDate ) << ")"; AQ_THROW( aqMsg54.str() ); }
        }

        // Forward Rates are saved and indexed by an ACT/365 yearFraction
        const bool includeLast = true;
		AQLString dayCount("ACT/365");
        const double yearFractionForFixingDate = AQLDateScheduleHelpers::getTerm( toAQLDateFromGregorianDate( asOfDate ), toAQLDateFromGregorianDate( unadjustedFixingDate ), dayCount, includeLast );

        return calculateForwardRate( yearFractionForFixingDate );
    };

    double AQObjCurve::calculateForwardRate( const double yearFraction ) const
    {
        if ( yearFraction < 0.0 )
        {
            { std::ostringstream aqMsg55;
aqMsg55 << "Cannot forecast a Forward Rate for a fixing date in the past with yearFraction " << yearFraction; AQ_THROW( aqMsg55.str() ); }
        }
        if( curveBuildProps_ == nullptr )
        {
            { std::ostringstream aqMsg56;
aqMsg56 << "AQObjCurve::calculateForwardRate(yearFraction): Cannot calculate forward rate because not CurveBuildProperties were set; yearFraction (" << yearFraction << ")"; AQ_THROW( aqMsg56.str() ); }
        }
        if( forwardRates_.size() == 0 )
        {
            AQ_THROW( "AQObjCurve::calculateForwardRate(yearFraction): Cannot calculate forward rate because no input rates have been set" );
        }

        if( !curveBuildProps_ ->isOnlyAllowLookup() )
        {
            const InterpolationEnum interpMethod = curveBuildProps_->interpMethod_;
            if( interpMethod != lastInterpUsed_ || interpolationOnForwardRates_ == nullptr )
            {
                setInterpolationData( interpMethod );
            }
            return interpolationOnForwardRates_.get()->interpolate( yearFraction );
        }
        else
        {
            //auto idxToRetrieve = getIndexOfDate(dateToEvaluate); // is better but the below is to mimic the old AlgoQuantLib behaviour
            auto idxToRetrieve = getIndexWithPrecision( datesAsYearFractions_, yearFraction );
            if( idxToRetrieve < 0 )
            {
                { std::ostringstream aqMsg58;
aqMsg58 << "AQObjCurve::calculateDiscountFactor(yearFraction) LOOKUPONLY : Cannot find discount factor matching yearFraction (" << yearFraction << ")"; AQ_THROW( aqMsg58.str() ); }
            }
            else
            {
                return forwardRates_[ idxToRetrieve ];
            }
        }
    };

    double AQObjCurve::calculateForwardRate( const std::string& tenorString ) const
    {
        if( forwardRates_.size() != dates_.size() || forwardRates_.size() == 0 )
        {
            { std::ostringstream aqMsg59;
aqMsg59 << "AQObjCurve::calculateForwardRate(tenorString) => cannot interpolate forward rates given " << forwardRates_.size() << " input rates and " << dates_.size() << " dates "; AQ_THROW( aqMsg59.str() ); }
        }
        boost::gregorian::date toDate = adjustFromAsOfDateUsingTenorString( tenorString, AQObjCurve::FIXING_BUSINESSDAYADJUSTMENT ); // curve build props checked in here
        return calculateForwardRate( toDate ); // attempting the same business day adjustment should generate the same day
    };

    std::vector<double> AQObjCurve::calculateForwardRateUsingDiscountFactors(
        const std::vector<boost::gregorian::date>& accrualStartDates,
        const std::vector<boost::gregorian::date>& accrualEndDates,
        const DayCountEnum dayCount,
        const CompoundingFrequencyEnum compFreq ) const
    {
        if( accrualStartDates.size() != accrualEndDates.size() )
        {
            { std::ostringstream aqMsg60;
aqMsg60 << "AQObjCurve::calculateForwardRate(startDates, endDates) ; the number of startDates (" << accrualStartDates.size() << ") does not equal the number of endDates (" << accrualEndDates.size() << ")"; AQ_THROW( aqMsg60.str() ); }
        }
        std::vector<double> retVec;
        for( unsigned int dateCounter = 0u; dateCounter < accrualStartDates.size(); dateCounter++ )
        {
            double fwdRate = calculateForwardRateUsingDiscountFactors( accrualStartDates[ dateCounter ], accrualEndDates[ dateCounter ], dayCount );
            retVec.push_back( fwdRate );
        }
        return retVec;
    };

    std::vector<double> AQObjCurve::calculateForwardRate( const std::vector<boost::gregorian::date>& unadjustedFixingDates ) const
    {
        std::vector<double> retVec;
        for( unsigned int dateCounter = 0u; dateCounter < unadjustedFixingDates.size(); dateCounter++ )
        {
            double fwdRate = calculateForwardRate( unadjustedFixingDates[ dateCounter ] );
            retVec.push_back( fwdRate );
        }
        return retVec;
    };

    std::vector<double> AQObjCurve::calculateForwardRate( const std::vector<double>& yearFractionsAsOfDateToAccrualFromDate ) const
    {
        std::vector<double> retVec;
        for( unsigned int dateCounter = 0u; dateCounter < yearFractionsAsOfDateToAccrualFromDate.size(); dateCounter++ )
        {
            double fwdRate = calculateForwardRate( yearFractionsAsOfDateToAccrualFromDate[ dateCounter] );
            retVec.push_back( fwdRate );
        }
        return retVec;
    };

    const std::pair<const BusinessDayAdjustmentEnum, const AQLCalendar*> AQObjCurve::getBusinessDayAdjust( const BusinessDayAdjustmentType adjType ) const
    {
        if( curveBuildProps_ == nullptr )
        {
            AQ_THROW( "AQObjCurve::getBusinessDayAdjust(adjType) - Cannot business day adjusment when CurveBuildProperties have not been set" );
        }

        if( adjType == AQObjCurve::ACCRUAL_BUSINESSDAYADJUSTMENT )
        {
            return std::make_pair( curveBuildProps_->accrualDayAdjustment_, curveBuildProps_->getAqAccrualCalendar() ) ;
        }
        if( adjType == AQObjCurve::PAYMENT_BUSINESSDAYADJUSTMENT )
        {
            return std::make_pair( curveBuildProps_->paymentDayAdjustment_, curveBuildProps_->getAqPaymentCalendar() ) ;
        }
        if( adjType == AQObjCurve::FIXING_BUSINESSDAYADJUSTMENT )
        {
            return std::make_pair( curveBuildProps_->fixingDayAdjustment_, curveBuildProps_->getAqFixingCalendar() ) ;
        }

        { std::ostringstream aqMsg62;
aqMsg62 << "AQObjCurve::getBusinessDayAdjust(adjType) - Unable to retrieve BusinessDayAdjustmentType (" << adjType << ")"; AQ_THROW( aqMsg62.str() ); }
    };

    boost::gregorian::date AQObjCurve::adjustFromAsOfDateUsingTenorString( const std::string& tenorString, const BusinessDayAdjustmentType adjType ) const
    {
        if( curveBuildProps_ == nullptr )
        {
            AQ_THROW( "AQObjCurve::calculateDiscountFactor(tenorString) - Cannot convert to calculate discount factor when CurveBuildProperties have not been set" );
        };

        boost::gregorian::date date = curveBuildProps_.get()->asOfDate_;
        date = addTenorString( date, tenorString );

        auto adjInfo = getBusinessDayAdjust( adjType );
        const auto busDayAdjustment =  adjInfo.first;
        const auto ptrHolidayCalendar = adjInfo.second;
        if( ptrHolidayCalendar == nullptr )
        {
            { std::ostringstream aqMsg64;
aqMsg64 << "AQObjCurve::calculateDiscountFactor(tenorString) - Missing AlgoQuantLib Calendar for calendar (" << curveBuildProps_->getPaymentDayCalendar().c_str() << ")"; AQ_THROW( aqMsg64.str() ); }
        };
        date = dayAdjust( date, busDayAdjustment, *ptrHolidayCalendar );
        return date;
    };

    const AQLCalendar* AQObjCurve::getCalendar( const std::string& calendar ) const
    {
        const std::string calendarName = trim_to_upper( calendar.c_str() );
        if( curveBuildProps_ )
        {
            if( calendarName == curveBuildProps_->getAccrualDayCalendar() )
            {
                return curveBuildProps_->getAqAccrualCalendar();
            }
            if( calendarName == curveBuildProps_->getFixingDayCalendar() )
            {
                return curveBuildProps_->getAqFixingCalendar();
            }
            if( calendarName == curveBuildProps_->getPaymentDayCalendar() )
            {
                return curveBuildProps_->getAqPaymentCalendar();
            }
        }
        return &AQLCalendarSet::getCalendar( calendarName.c_str() );
    };

    VariantMatrix AQObjCurve::getVariantMatrix() const
    {
        VariantMatrix variantMatrix;
        variantMatrix.emplace_back( Variant::createVariantVector( this->dates_, etrading::DATE_VALUE ) );
        variantMatrix.emplace_back( Variant::createVariantVector( this->datesAsYearFractions_, etrading::DOUBLE_VALUE ) );
        variantMatrix.emplace_back( Variant::createVariantVector( this->discountFactors_, etrading::DOUBLE_VALUE ) );
        if( this->forwardRates_.size() == this->discountFactors_.size() )
        {
            variantMatrix.emplace_back( Variant::createVariantVector( this->forwardRates_, etrading::DOUBLE_VALUE ) );
        }
        return variantMatrix;
    };

    VariantMatrix AQObjCurve::getDiscountFactorMatrix() const
    {
        VariantMatrix variantMatrix;
        
        variantMatrix.emplace_back( Variant::createVariantVector( this->dates_, etrading::DATE_VALUE ) );
        variantMatrix.emplace_back( Variant::createVariantVector( this->discountFactors_, etrading::DOUBLE_VALUE ) );
        
        return variantMatrix;
    };

    VariantMatrix AQObjCurve::getForwardRateMatrix() const
    {
        VariantMatrix variantMatrix;
        
        variantMatrix.emplace_back( Variant::createVariantVector( this->dates_, etrading::DATE_VALUE ) );
        variantMatrix.emplace_back( Variant::createVariantVector( this->forwardRates_, etrading::DOUBLE_VALUE ) );
        
        return variantMatrix;
    };

}
