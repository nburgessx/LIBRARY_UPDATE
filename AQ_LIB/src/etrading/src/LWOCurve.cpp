#include <limits>
#include <utility>
#include <boost/format.hpp>
#include <boost/date_time.hpp>

#include "LWOCurve.h"
#include "ETradingException.h"
#include "ContainerUtilities.h"
#include "Variant.h"
#include "CurveBuildProperties.h"
#include "DateUtilities.h"
#include "InterpolationFactory.h"
#include "LWOUtilities.h"
#include "LADateScheduleHelpers.h"
#include "EntityPoolUtilities.h"
#include "LAMathInterpolationUtilities.h"

namespace etrading
{
    
    AscendingOrderRule< std::vector<boost::gregorian::date> >  LWOCurve::dateChecker_  ;  // static

    BoundaryRule<> LWOCurve::posNumerChecker_( 0.0, std::numeric_limits<double>::max() ); // static
    
    AscendingOrderRule<> LWOCurve::increasingYearFractions_;  // static
    
    const etrading::CachedObjectEnum LWOCurve::ENUM_TYPE = etrading::CURVE_DEPRECATED; // static

    // TODO: use delegating constructors in C++11

    // CurveBuildProperties is a unique_ptr so it needs to be deep copied
    LWOCurve::LWOCurve( const LWOCurve& instance )
        : IsLWOObject( instance.getName() ,LWOCurve::ENUM_TYPE ),
          HasCurveData( instance ),
          curveBuildProps_( instance.curveBuildProps_ ? new CurveBuildProperties( *( instance.curveBuildProps_.get() ) ) : nullptr ),
          dates_( instance.getDates() ),
          discountFactors_( instance.getDiscountFactors() ),
          forwardRates_( instance.getForwardRates() ),
          datesAsYearFractions_( instance.datesAsYearFractions_ )
    {
    }

    LWOCurve::LWOCurve( const std::string& objName )
        :   IsLWOObject( objName, LWOCurve::ENUM_TYPE ), 
            curveBuildProps_(),
            interpolationOnDiscountFactors_(), interpolationOnForwardRates_(),
            dates_(  0 ), discountFactors_( 0 ), forwardRates_( 0 ), datesAsYearFractions_( 0 )
    {};

    LWOCurve::LWOCurve( LWOCurve&& moved )
        :  IsLWOObject( moved.getName() , LWOCurve::ENUM_TYPE ),
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

    LWOCurve::LWOCurve(	const std::string& objName,
                        const std::vector<boost::gregorian::date>& dates,
                        const std::vector<double>& discountFactors,
                        const CurveBuildProperties& curveConvention,
                        const std::vector<double>& forwardRates )
        : IsLWOObject( objName , LWOCurve::ENUM_TYPE )
    {
        setData( dates, discountFactors, forwardRates );
        setCurveBuildStaticDataObject( curveConvention );
        setYearFractions();
    };

    LWOCurve::LWOCurve(	const std::string& objName,
                        const std::vector<double>& yearFractionsAsActAct,
                        const std::vector<double>& discountFactors,
                        const CurveBuildProperties& curveConvention )
        : IsLWOObject( objName, LWOCurve::ENUM_TYPE ),
          interpolationOnDiscountFactors_(), interpolationOnForwardRates_(),
          curveBuildProps_( new CurveBuildProperties( curveConvention ) ),
          datesAsYearFractions_( 0 ), discountFactors_( 0 ), dates_( 0 ), forwardRates_( 0 )
    {
        datesAsYearFractions_.reserve( yearFractionsAsActAct.size() );
        discountFactors_.reserve( discountFactors.size() );
        dates_.reserve( discountFactors.size() );
        setData( yearFractionsAsActAct, discountFactors ); // used for checking as well
    };

    LWOCurve::LWOCurve(	const std::string& objName,
                        const std::vector<boost::gregorian::date>& dates,
                        const std::vector<double>& yearFractionsAsActAct,
                        const std::vector<double>& discountFactors,
                        const CurveBuildProperties& curveConvention )
        : IsLWOObject( objName, LWOCurve::ENUM_TYPE ),
          interpolationOnDiscountFactors_(), interpolationOnForwardRates_(),
          curveBuildProps_( new CurveBuildProperties( curveConvention ) ),
          datesAsYearFractions_( 0 ), discountFactors_( 0 ), dates_( 0 ), forwardRates_( 0 )
    {
        if( dates.size() != yearFractionsAsActAct.size() )
        {
            throw ETradingException( ( boost::format( "LWOCurve::LWOCurve: Dates and Year Fractions are not of the same size (%i and %i respectively)" )
                                       % dates.size() % yearFractionsAsActAct.size() ).str() );
        }
        dates_.reserve( discountFactors.size() );
        if( !setDates( dates ) )
        {
            throw ETradingException( ( boost::format( "Dates in the CTOR of LWOCurve are not in ascending order - verify: %s" )
                                       % containerAsString( getDisplayableContainer( dates ) ) ).str() );
        }
        datesAsYearFractions_.reserve( yearFractionsAsActAct.size() );
        discountFactors_.reserve( discountFactors.size() );
        setData( yearFractionsAsActAct, discountFactors, false ); // used for checking as well
    };

    LWOCurve::LWOCurve(	const std::string& objName,
                        const std::vector<boost::gregorian::date>& dates,
                        const std::vector<double>& yearFractionsAsActAct,
                        const std::vector<double>& discountFactors,
                        const std::vector<double>& forwardRates,
                        const CurveBuildProperties& curveConvention )
        :	IsLWOObject( objName, LWOCurve::ENUM_TYPE ),
          interpolationOnDiscountFactors_(), interpolationOnForwardRates_(),
          curveBuildProps_( new CurveBuildProperties( curveConvention ) ),
          datesAsYearFractions_( 0 ), discountFactors_( 0 ), dates_( 0 ), forwardRates_( 0 )
    {
        if( dates.size() != yearFractionsAsActAct.size() )
        {
            throw ETradingException( ( boost::format( "LWOCurve::LWOCurve: Dates and Year Fractions are not of the same size (%i and %i respectively)" )
                                       % dates.size() % yearFractionsAsActAct.size() ).str() );
        }
        if( forwardRates.size() != yearFractionsAsActAct.size() )
        {
            throw ETradingException( ( boost::format( "LWOCurve::LWOCurve: Forward Rates and Year Fractions are not of the same size (%i and %i respectively)" )
                                       % forwardRates.size() % yearFractionsAsActAct.size() ).str() );
        }
        dates_.reserve( discountFactors.size() );
        if( !setDates( dates ) )
        {
            throw ETradingException( ( boost::format( "Dates in the CTOR of LWOCurve are not in ascending order - verify: %s" )
                                       % containerAsString( getDisplayableContainer( dates ) ) ).str() );
        }
        datesAsYearFractions_.reserve( yearFractionsAsActAct.size() );
        discountFactors_.reserve( discountFactors.size() );
        setData( yearFractionsAsActAct, discountFactors, false ); // used for checking as well
        setForwardRates( forwardRates );
    };

    LWOCurve::~LWOCurve()
    {
		// we used to call removeUnderlyingEntityPoolCurve() here
        // but that will cause an issue on moving an LWOCurve or when a locally created LWO curve gets destroyed
	};

    void LWOCurve::removeUnderlyingEntityPoolCurve() const
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

    void LWOCurve::inspectDataFormat(	const std::vector<boost::gregorian::date>& dates,
                                        const std::vector<double>& discountFactors,
                                        const std::vector<double>& forwardRates ) const
    {
        if( discountFactors.size() != dates.size() )
        {
            throw ETradingException( ( boost::format( "The Dates and DiscountFactors in the CTOR of the LWOCurve are not of the same length %i (Dates) and %i (DiscountFactors)" )
                                       % dates.size() % discountFactors.size() ).str() );
        };

        if( forwardRates.size() > 0 )
        {
            if( forwardRates.size() != dates.size() )
            {
                throw ETradingException( ( boost::format( "The number of forward rates needs to equal the number of dates in the CTOR of the LWOCurve if the forward rates are set, current sizes: %i (Dates) and %i (forwardRates)" )
                                           % dates.size() % forwardRates.size() ).str() );
            }
        };

        // TODO: because non business days are supplied to OIS in AlgoQuantLib we do not check this for compatabilitiy.... FIX this on both sides.
        /*
        if(curveBuildProps_ && dates.size() > 0)
        {
        	const auto mlibCalendar = curveBuildProps_->getMlibFixingCalendar();
        	if(mlibCalendar != nullptr)
        	{
        		auto non_bus_day = std::find_if(dates.cbegin(), dates.cend(),
        															[&mlibCalendar](const boost::gregorian::date& dateToCheck) {  return !isBusinessDay(dateToCheck, *mlibCalendar); } );
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

    void LWOCurve::setData(	const std::vector<boost::gregorian::date>& dates,
                            const std::vector<double>& discountFactors,
                            const std::vector<double>& forwardRates )
    {
        inspectDataFormat( dates, discountFactors, forwardRates );
        if( !setDates( dates ) )
        {
            throw ETradingException( ( boost::format( "Dates in the CTOR of LWOCurve are not in ascending order - verify: %s" )
                                       % containerAsString( getDisplayableContainer( dates ) ) ).str() );
        }
        
        setDiscountFactors( discountFactors );
        // TODO: Temporarily relax this check for backwards compatibility ... this is a good check that needs to be reinstated once the curve extrapolation has been fixed
        // Currently Tenor basis curves can extrapolate negatively, which needs resolving, upon which this fix should be re-instated.

        /*if( !setDiscountFactors( discountFactors ) )
        {
            throw ETradingException( ( boost::format( "One or more DiscountFactors in the CTOR of LWOCurve were not positive - verify: %s" )
                                       % containerAsString( getDisplayableContainer( discountFactors ) ) ).str() );
        }*/
        
        setForwardRates( forwardRates );

        setYearFractions();
    };

    void LWOCurve::setData ( const  std::vector<double>& yearFractionsAsActAct,
                             const std::vector<double>& discountFactors,
                             const bool setDatesFromFractions )
    {
        if( !LWOCurve::posNumerChecker_.verify( yearFractionsAsActAct ) || !LWOCurve::increasingYearFractions_.verify( yearFractionsAsActAct ) )
        {
            throw ETradingException(
                ( boost::format( "#Error Invalid curve dates. Curve nodes are in the past or decreasing with time" ) ).str() );
        }
        datesAsYearFractions_ = 	yearFractionsAsActAct;
        
        setDiscountFactors( discountFactors );
        // TODO: Temporarily relax this check for backwards compatibility ... this is a good check that needs to be reinstated once the curve extrapolation has been fixed
        // Currently Tenor basis curves can extrapolate negatively, which needs resolving, upon which this fix should be re-instated.
       /* if( !setDiscountFactors( discountFactors ) )
        {
            throw ETradingException( ( boost::format( "One or more DiscountFactors in the CTOR of LWOCurve were not positive - verify: %s" )
                                       % containerAsString( getDisplayableContainer( discountFactors ) ) ).str() );
        }*/

        if( setDatesFromFractions )
        {
            this->setDatesFromFractions();
        }
    };

    bool LWOCurve::setDates( const std::vector<boost::gregorian::date>& dates )
    {
        if( LWOCurve::dateChecker_.verify( dates ) )
        {
            dates_ = dates;
            return true;
        }
        return false;
    };

    bool LWOCurve::setDiscountFactors( const std::vector<double>& discountFactors )
    {

        discountFactors_ = discountFactors;
        return true;

        // TODO: This is a good check ... that we are relaxing temporarily, since Tenor Basis Curves typically only
        // go out 30 years and we are checking for 50 years of daily discount factors which extrapolate negatively.
        // Once the tenor basis extrapolation has been fixed we should reinstate this check and delete the above.
        
        /*if( LWOCurve::posNumerChecker_.verify( discountFactors ) )
        {
            discountFactors_ = discountFactors;
            return true;
        }
        
        return false;*/
    };

    bool LWOCurve::setForwardRates( const std::vector<double>& forwardRates )
    {
        // it is OK if the forward rates are an empty vector
        forwardRates_ = forwardRates;
        return true;
    };

    void LWOCurve::setCurveBuildStaticDataObject( const CurveBuildProperties& cbp )
    {
        curveBuildProps_.reset( new CurveBuildProperties( cbp ) );
        inspectDataFormat( dates_, discountFactors_, forwardRates_ );
        setYearFractions();
    };

    void LWOCurve::setCurveBuildStaticDataObject( CurveBuildProperties&& curveConvention )
    {
        curveBuildProps_.reset( &curveConvention );
        inspectDataFormat( dates_, discountFactors_, forwardRates_ );
        setYearFractions();
    };

    void LWOCurve::setYearFractions()
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

    void LWOCurve::setDatesFromFractions()
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

    void LWOCurve::setInterpolationData( const etrading::InterpolationEnum interpMethod ) const
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

    int LWOCurve::getIndexOfDate( const boost::gregorian::date& date ) const
    {
        return getIndexOf( date, dates_ );
    };

    std::tuple<boost::gregorian::date, double, double> LWOCurve::getData( const unsigned int idx ) const
    {
        boost::gregorian::date date = dates_[ idx ];
        double dfRate = discountFactors_[ idx ];
        double fwdRate = ( forwardRates_.size() >  idx ) ? forwardRates_[ idx ] : std::numeric_limits<double>::quiet_NaN();
        return std::make_tuple( date, dfRate, fwdRate );
    };

    std::string LWOCurve::getName() const
    {
        return getRefToName();
    };

    const std::vector<boost::gregorian::date>& LWOCurve::getDates() const
    {
        return dates_;
    };

    const std::vector<double>& LWOCurve::getDiscountFactors() const
    {
        return discountFactors_;
    };

    const std::vector<double>& LWOCurve::getForwardRates() const
    {
        return forwardRates_;
    };

    const CurveBuildProperties* LWOCurve::getCurveBuildStaticDataObject() const
    {
        return curveBuildProps_.get();
    };
    //SerializationResult LWOCurve::serialize(
    //    const serialize::SerializationMethodEnum method,
    //    const serialize::SerializationTargetEnum target,
    //    const std::string& targetInfo,
    //    std::vector<std::string>& variableNames,
    //    std::vector<Variant>& variableValues ) const
    //{
    //    return toSchemaObject().serialize( method, target, targetInfo, variableNames, variableValues );
    //};

    const SchemaObject LWOCurve::toSchemaObject() const
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
            throw ETradingException( "LWOCurve::toSchemaObject() - Cannot convert to SchemaObject object when CurveBuildProperties have not been set" );
        }
        return schemaObject;
    };

    bool LWOCurve::isBeforeAsOf( const boost::gregorian::date& date ) const
    {
        if( !curveBuildProps_ )
        {
            throw ETradingException( "No CurveBuildProperties Supplied" );
        }
        return date < curveBuildProps_.get()->asOfDate_;
    };

    const std::vector<double>& LWOCurve::getYearFractions() const
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
                throw ETradingException( "Unable to get YearFractions because no CurveBuildProperties or input data has been set" );
            }
        }
    };

    // using ACT_365 from the AsOfDate
    double LWOCurve::calculateDiscountFactor( const boost::gregorian::date& toDate ) const
    {
        const boost::gregorian::date asOfDate = curveBuildProps_.get()->asOfDate_;

        if( asOfDate == toDate )
        {
            return 1.0;
        }

        if( toDate < asOfDate )
        {
            throw ETradingException( ( boost::format( "LWOCurve::calculateDiscountFactor(date) => Date supplied (%s) lies before asOfDate(%s)" )
                                       % toYYYYMMDDFromGregorianDate( toDate )
                                       % toYYYYMMDDFromGregorianDate( asOfDate ) ).str() );
        }


        int toDateIdx = getIndexOfDate( toDate );
        if( toDateIdx >= 0 )
        {
            return discountFactors_[ toDateIdx ];
        }
        else
        {
            double yearFractionForToDate = getYearFractionFromDayCount( etrading::ACT_ACT_DAYCOUNT, asOfDate, toDate );
            // call the interpolate in Entitypool = LAMathInterpolationUtilities::interpolate
            return calculateDiscountFactor( yearFractionForToDate );
        }
    };

    double LWOCurve::calculateDiscountFactor( const double yearFraction ) const
    {
        if ( yearFraction < 0.0 )
        {
            throw ETradingException( ( boost::format( "Term starting from AsOfDate must be positive but received yearFraction of : %f" ) % yearFraction ).str() );
        }
        // anything less than a day is not worth interpolatng
        if ( yearFraction < 1.0 / 366.0 )
        {
            return 1.0;
        }

        if( curveBuildProps_ == nullptr )
        {
            throw ETradingException( ( boost::format( "LWOCurve::calculateDiscountFactor(yearFraction): Cannot calculate Discount Factor because not CurveBuildProperties were set; yearFraction (%f)" ) % yearFraction ).str() );
        }

        if( !curveBuildProps_->isOnlyAllowLookup() )
        {
            
            // Discount Factors stored at Node Points Only
            // ------------------------------------------
            
            // Native AlgoQuantLib Interpolation
            // -------------------------
            const InterpolationEnum interpMethod = curveBuildProps_->interpMethod_;
            double result = LAMathInterpolationUtilities::interpolate( datesAsYearFractions_, discountFactors_, yearFraction, interpMethod );
            return result;

            // LWO Independent Interpolation
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
                    throw ETradingException( ( boost::format( "LWOCurve::calculateDiscountFactor(yearFraction) LOOKUPONLY : Cannot find discount factor matching date (%s)" )
                                               % toYYYYMMDDFromGregorianDate( dateToEvaluate ).c_str() ).str() );
                }
                else
                {
                    return discountFactors_[ idxToRetrieve ];
                }
            }
        }
    };

    // this is the discount factor from a future payment date (which will be adjusted) to term past that future date (which will not be adjusted)
    double LWOCurve::calculateDiscountFactor(
        const boost::gregorian::date& futurePaymentDate,
        const double yearFraction,
        const BusinessDayAdjustmentEnum dayAdjustment,
        const std::string& calendar ) const
    {
        // boost::gregorian::date date = curveBuildProps_.get()->asOfDate_;
        auto ptrMlibCalendar = getCalendar( trim_to_upper( calendar.c_str() ) );
        if( ptrMlibCalendar )
        {
            const boost::gregorian::date asOfDate = curveBuildProps_.get()->asOfDate_;
            boost::gregorian::date fromDate = dayAdjust( futurePaymentDate, dayAdjustment, *ptrMlibCalendar );
            double yearFractionForFromDate = getYearFractionFromDayCount( etrading::ACT_ACT_DAYCOUNT, asOfDate, fromDate );
            double dfFromDate = calculateDiscountFactor( yearFractionForFromDate );
            double yearFractionForToDate = yearFractionForFromDate + yearFraction;
            double dfToDate = calculateDiscountFactor( yearFractionForToDate );
            return dfToDate / dfFromDate; // time fraction
        }
        else
        {
            throw ETradingException( ( boost::format( "Unable to retrieve an AlgoQuantLib calendar specified as %s" ) % calendar.c_str() ).str() );
        }
    };

    double LWOCurve::calculateDiscountFactor(
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

    double LWOCurve::calculateDiscountFactor( const boost::gregorian::date& fromDate, const boost::gregorian::date& toDate ) const
    {
        double fromDF = calculateDiscountFactor( fromDate );
        double toDF = calculateDiscountFactor( toDate );
        return toDF / fromDF; // time fraction
    };

    double LWOCurve::calculateDiscountFactor( const boost::gregorian::date& valuationDate, const boost::gregorian::date& paymentDate,
            const BusinessDayAdjustmentEnum dayAdjustment, const std::string& calendar ) const
    {
        auto ptrMlibCalendar = getCalendar( trim_to_upper( calendar.c_str() ) );
        if( ptrMlibCalendar )
        {
            auto fromDate = dayAdjust( valuationDate, dayAdjustment, *ptrMlibCalendar );
            auto toDate = dayAdjust( paymentDate, dayAdjustment, *ptrMlibCalendar );
            return calculateDiscountFactor( fromDate, toDate );
        }
        else
        {
            throw ETradingException( ( boost::format( "Unable to retrieve an AlgoQuantLib calendar specified as %s" ) % calendar.c_str() ).str() );
        }
    };

    double LWOCurve::calculateDiscountFactor( const std::string& tenorString ) const
    {
        boost::gregorian::date date = adjustFromAsOfDateUsingTenorString( tenorString );
        return calculateDiscountFactor( date );
    };

    double LWOCurve::calculateDiscountFactor( const std::string& tenorString, const BusinessDayAdjustmentEnum dayAdjustment, const std::string& calendar ) const
    {
        boost::gregorian::date date = curveBuildProps_.get()->asOfDate_;
        date = addTenorString( date, tenorString );

        if( dayAdjustment != NO_CHANGE )
        {
            auto ptrMlibCalendar = getCalendar( trim_to_upper( calendar.c_str() ) );
            if( ptrMlibCalendar )
            {
                date = dayAdjust( date, dayAdjustment, *ptrMlibCalendar );
                return calculateDiscountFactor( date );
            }
            else
            {
                throw ETradingException( ( boost::format( "Unable to retrieve an AlgoQuantLib calendar specified as %s" ) % calendar.c_str() ).str() );
            }
        }
        else
        {
            return calculateDiscountFactor( date );
        }
    };

    std::vector<double> LWOCurve::calculateDiscountFactor(
        const std::vector<boost::gregorian::date>& valuationDates,
        const std::vector<boost::gregorian::date>& paymentDates ) const
    {
        if( valuationDates.size() != paymentDates.size() )
        {
            throw ETradingException( ( boost::format( "LWOCurve::calculateDiscountFactor(valuationDates, paymentDates) ; the number of valuationDates (%i) does not equal the number of paymentDates (%i)" )
                                       % valuationDates.size() % paymentDates.size() ).str() );
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
    std::vector<double> LWOCurve::calculateDiscountFactor(
        const std::vector<boost::gregorian::date>& valuationDates,
        const std::vector<boost::gregorian::date>& paymentDates,
        const BusinessDayAdjustmentEnum dayAdjustment, const std::string& calendar ) const
    {
        if( valuationDates.size() != paymentDates.size() )
        {
            throw ETradingException( ( boost::format( "LWOCurve::calculateDiscountFactor(valuationDates, paymentDates) ; the number of valuationDates (%i) does not equal the number of paymentDates (%i)" )
                                       % valuationDates.size() % paymentDates.size() ).str() );
        }
        std::vector<double> retVec;
        for( unsigned int dateCounter = 0u; dateCounter < valuationDates.size(); dateCounter++ )
        {
            double dfHere = calculateDiscountFactor( valuationDates[ dateCounter ], paymentDates[ dateCounter ], dayAdjustment, calendar );
            retVec.push_back( dfHere );
        }
        return retVec;
    }

    std::vector<double> LWOCurve::calculateDiscountFactor( const std::vector<boost::gregorian::date>& paymentDates ) const
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

    std::vector<double> LWOCurve::calculateDiscountFactor( const std::vector<double>& yearFractions ) const
    {
        std::vector<double> retVec;
        for( unsigned int fractionCounter = 0u; fractionCounter < yearFractions.size(); fractionCounter++ )
        {
            double dfHere = calculateDiscountFactor( yearFractions[ fractionCounter ] );
            retVec.push_back( dfHere );
        }
        return retVec;
    }

    std::vector<double> LWOCurve::calculateDiscountFactor( const std::vector<boost::gregorian::date>& futurePaymentDates, const std::vector<double>& yearFractions, const BusinessDayAdjustmentEnum dayAdjustment, const std::string& calendar ) const
    {
        if( yearFractions.size() != futurePaymentDates.size() )
        {
            throw ETradingException( ( boost::format( "LWOCurve::calculateDiscountFactor(futurePaymentDates, yearFractions,dayAdjustment,calendar) ; the number of futurePaymentDates (%i) does not equal the number of yearFractions (%i)" )
                                       % futurePaymentDates.size() % yearFractions.size() ).str() );
        }
        std::vector<double> retVec;
        for( unsigned int fractionCounter = 0u; fractionCounter < yearFractions.size(); fractionCounter++ )
        {
            double dfHere = calculateDiscountFactor( futurePaymentDates[ fractionCounter ], yearFractions[ fractionCounter ], dayAdjustment, calendar );
            retVec.push_back( dfHere );
        }
        return retVec;
    };

    std::vector<double> LWOCurve::calculateDiscountFactor( const std::vector<std::string>& tenorStrings ) const
    {
        std::vector<double> retVec;
        for( unsigned int tenorStringCounter = 0u; tenorStringCounter < tenorStrings.size(); tenorStringCounter++ )
        {
            double dfHere = calculateDiscountFactor( tenorStrings[ tenorStringCounter ] );
            retVec.push_back( dfHere );
        }
        return retVec;
    };

    std::vector<double> LWOCurve::calculateDiscountFactor( const std::vector<std::string>& tenorStrings, const BusinessDayAdjustmentEnum dayAdjustment, const std::string& calendar ) const
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

    std::vector<double> LWOCurve::calculateDiscountFactor(
        const std::vector<boost::gregorian::date>& futurePaymentDates,
        const std::vector<std::string>& termsAsString,
        const BusinessDayAdjustmentEnum dayAdjustment,
        const std::string& calendar ) const
    {
        if( futurePaymentDates.size() != termsAsString.size() )
        {
            throw ETradingException( ( boost::format( "Number of dates (%i) does not equal the number of terms (%i)" ) % futurePaymentDates.size() % termsAsString.size() ).str() );
        }
        std::vector<double> retVec;
        for( unsigned int dateCounter = 0u; dateCounter < termsAsString.size(); dateCounter++ )
        {
            double dfHere = calculateDiscountFactor( futurePaymentDates[ dateCounter ], termsAsString[ dateCounter ], dayAdjustment, calendar );
            retVec.push_back( dfHere );
        }
        return retVec;
    };

    double LWOCurve::calculateForwardRateUsingDiscountFactors(
        const boost::gregorian::date& accrualFromDate,
        const boost::gregorian::date& accrualToDate,
        const DayCountEnum dayCount,
        const CompoundingFrequencyEnum compFreq ) const
    {
        if( accrualFromDate >= accrualToDate )
        {
            throw ETradingException( ( boost::format( "LWOCurve::calculateForwardRateUsingDiscountFactors  Accrual Start Date (%s) is on or after Accrual End Date (%s)" )
                                       % toYYYYMMDDFromGregorianDate( accrualFromDate ).c_str()
                                       % toYYYYMMDDFromGregorianDate( accrualToDate ).c_str() ).str() );
        }
        double fromDF = calculateDiscountFactor( accrualFromDate );
        double toDF = calculateDiscountFactor( accrualToDate );
        double termAsDoubleForBetweenDates = getYearFractionFromDayCount( dayCount, accrualFromDate, accrualToDate );
        // getYearFractionFromDayCount( etrading::ACT_ACT_DAYCOUNT, accrualFromDate, accrualToDate );
        return etrading::getRateFromReturn( fromDF / toDF, termAsDoubleForBetweenDates, compFreq );
        // return (fromDF/toDF  - 1.0)*(1.0/termAsDoubleForBetweenDates); // time fraction
    };

    double LWOCurve::calculateForwardRate( const boost::gregorian::date& unadjustedFixingDate ) const
    {
        if( curveBuildProps_ == nullptr )
        {
            throw ETradingException( "LWOCurve::calculateForwardRate(fixingDate) : Cannot calculate because CurveBuildProperties were not set" );
        }

        const boost::gregorian::date asOfDate = curveBuildProps_.get()->asOfDate_;
        if( unadjustedFixingDate < asOfDate )
        {
            throw ETradingException( ( boost::format( "LWOCurve::calculateForwardRate(date) => unadjustedFixingDate supplied (%s) lies before asOfDate (%s)" )
                                       % toYYYYMMDDFromGregorianDate( unadjustedFixingDate )
                                       % toYYYYMMDDFromGregorianDate( asOfDate ) ).str() );
        }

        // Forward Rates are saved and indexed by an ACT/365 yearFraction
        const bool includeLast = true;
		AQLString dayCount("ACT/365");
        const double yearFractionForFixingDate = LADateScheduleHelpers::getTerm( toLADateFromGregorianDate( asOfDate ), toLADateFromGregorianDate( unadjustedFixingDate ), dayCount, includeLast );

        return calculateForwardRate( yearFractionForFixingDate );
    };

    double LWOCurve::calculateForwardRate( const double yearFraction ) const
    {
        if ( yearFraction < 0.0 )
        {
            throw ETradingException( ( boost::format( "#Error: Cannot forecast a Forward Rate for a fixing date in the past with yearFraction %f" ) % yearFraction ).str() );
        }
        if( curveBuildProps_ == nullptr )
        {
            throw ETradingException( ( boost::format( "LWOCurve::calculateForwardRate(yearFraction): Cannot calculate forward rate because not CurveBuildProperties were set; yearFraction (%f)" ) % yearFraction ).str() );
        }
        if( forwardRates_.size() == 0 )
        {
            throw ETradingException( "LWOCurve::calculateForwardRate(yearFraction): Cannot calculate forward rate because no input rates have been set" );
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
                throw ETradingException( ( boost::format( "LWOCurve::calculateDiscountFactor(yearFraction) LOOKUPONLY : Cannot find discount factor matching yearFraction (%f)" )
                                           % yearFraction ).str() );
            }
            else
            {
                return forwardRates_[ idxToRetrieve ];
            }
        }
    };

    double LWOCurve::calculateForwardRate( const std::string& tenorString ) const
    {
        if( forwardRates_.size() != dates_.size() || forwardRates_.size() == 0 )
        {
            throw ETradingException( ( boost::format( "LWOCurve::calculateForwardRate(tenorString) => cannot interpolate forward rates given %i input rates and %i dates " )
                                       % forwardRates_.size()
                                       % dates_.size()					 ).str() );
        }
        boost::gregorian::date toDate = adjustFromAsOfDateUsingTenorString( tenorString, LWOCurve::FIXING_BUSINESSDAYADJUSTMENT ); // curve build props checked in here
        return calculateForwardRate( toDate ); // attempting the same business day adjustment should generate the same day
    };

    std::vector<double> LWOCurve::calculateForwardRateUsingDiscountFactors(
        const std::vector<boost::gregorian::date>& accrualStartDates,
        const std::vector<boost::gregorian::date>& accrualEndDates,
        const DayCountEnum dayCount,
        const CompoundingFrequencyEnum compFreq ) const
    {
        if( accrualStartDates.size() != accrualEndDates.size() )
        {
            throw ETradingException( ( boost::format( "LWOCurve::calculateForwardRate(startDates, endDates) ; the number of startDates (%i) does not equal the number of endDates (%i)" )
                                       % accrualStartDates.size() % accrualEndDates.size() ).str() );
        }
        std::vector<double> retVec;
        for( unsigned int dateCounter = 0u; dateCounter < accrualStartDates.size(); dateCounter++ )
        {
            double fwdRate = calculateForwardRateUsingDiscountFactors( accrualStartDates[ dateCounter ], accrualEndDates[ dateCounter ], dayCount );
            retVec.push_back( fwdRate );
        }
        return retVec;
    };

    std::vector<double> LWOCurve::calculateForwardRate( const std::vector<boost::gregorian::date>& unadjustedFixingDates ) const
    {
        std::vector<double> retVec;
        for( unsigned int dateCounter = 0u; dateCounter < unadjustedFixingDates.size(); dateCounter++ )
        {
            double fwdRate = calculateForwardRate( unadjustedFixingDates[ dateCounter ] );
            retVec.push_back( fwdRate );
        }
        return retVec;
    };

    std::vector<double> LWOCurve::calculateForwardRate( const std::vector<double>& yearFractionsAsOfDateToAccrualFromDate ) const
    {
        std::vector<double> retVec;
        for( unsigned int dateCounter = 0u; dateCounter < yearFractionsAsOfDateToAccrualFromDate.size(); dateCounter++ )
        {
            double fwdRate = calculateForwardRate( yearFractionsAsOfDateToAccrualFromDate[ dateCounter] );
            retVec.push_back( fwdRate );
        }
        return retVec;
    };

    const std::pair<const BusinessDayAdjustmentEnum, const AQLMathCalendar*> LWOCurve::getBusinessDayAdjust( const BusinessDayAdjustmentType adjType ) const
    {
        if( curveBuildProps_ == nullptr )
        {
            throw ETradingException( "LWOCurve::getBusinessDayAdjust(adjType) - Cannot business day adjusment when CurveBuildProperties have not been set" );
        }

        if( adjType == LWOCurve::ACCRUAL_BUSINESSDAYADJUSTMENT )
        {
            return std::make_pair( curveBuildProps_->accrualDayAdjustment_, curveBuildProps_->getMlibAccrualCalendar() ) ;
        }
        if( adjType == LWOCurve::PAYMENT_BUSINESSDAYADJUSTMENT )
        {
            return std::make_pair( curveBuildProps_->paymentDayAdjustment_, curveBuildProps_->getMlibPaymentCalendar() ) ;
        }
        if( adjType == LWOCurve::FIXING_BUSINESSDAYADJUSTMENT )
        {
            return std::make_pair( curveBuildProps_->fixingDayAdjustment_, curveBuildProps_->getMlibFixingCalendar() ) ;
        }

        throw ETradingException( ( boost::format( "LWOCurve::getBusinessDayAdjust(adjType) - Unable to retrieve BusinessDayAdjustmentType (%i)" ) % adjType ).str() );
    };

    boost::gregorian::date LWOCurve::adjustFromAsOfDateUsingTenorString( const std::string& tenorString, const BusinessDayAdjustmentType adjType ) const
    {
        if( curveBuildProps_ == nullptr )
        {
            throw ETradingException( "LWOCurve::calculateDiscountFactor(tenorString) - Cannot convert to calculate discount factor when CurveBuildProperties have not been set" );
        };

        boost::gregorian::date date = curveBuildProps_.get()->asOfDate_;
        date = addTenorString( date, tenorString );

        auto adjInfo = getBusinessDayAdjust( adjType );
        const auto busDayAdjustment =  adjInfo.first;
        const auto ptrHolidayCalendar = adjInfo.second;
        if( ptrHolidayCalendar == nullptr )
        {
            throw ETradingException( ( boost::format( "LWOCurve::calculateDiscountFactor(tenorString) - Missing AlgoQuantLib Calendar for calendar (%s)" ) % curveBuildProps_->getPaymentDayCalendar().c_str() ).str() );
        };
        date = dayAdjust( date, busDayAdjustment, *ptrHolidayCalendar );
        return date;
    };

    const AQLMathCalendar* LWOCurve::getCalendar( const std::string& calendar ) const
    {
        const std::string calendarName = trim_to_upper( calendar.c_str() );
        if( curveBuildProps_ )
        {
            if( calendarName == curveBuildProps_->getAccrualDayCalendar() )
            {
                return curveBuildProps_->getMlibAccrualCalendar();
            }
            if( calendarName == curveBuildProps_->getFixingDayCalendar() )
            {
                return curveBuildProps_->getMlibFixingCalendar();
            }
            if( calendarName == curveBuildProps_->getPaymentDayCalendar() )
            {
                return curveBuildProps_->getMlibPaymentCalendar();
            }
        }
        return &AQLMathCalendarSet::getCalendar( calendarName.c_str() );
    };

    VariantMatrix LWOCurve::getVariantMatrix() const
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

    VariantMatrix LWOCurve::getDiscountFactorMatrix() const
    {
        VariantMatrix variantMatrix;
        
        variantMatrix.emplace_back( Variant::createVariantVector( this->dates_, etrading::DATE_VALUE ) );
        variantMatrix.emplace_back( Variant::createVariantVector( this->discountFactors_, etrading::DOUBLE_VALUE ) );
        
        return variantMatrix;
    };

    VariantMatrix LWOCurve::getForwardRateMatrix() const
    {
        VariantMatrix variantMatrix;
        
        variantMatrix.emplace_back( Variant::createVariantVector( this->dates_, etrading::DATE_VALUE ) );
        variantMatrix.emplace_back( Variant::createVariantVector( this->forwardRates_, etrading::DOUBLE_VALUE ) );
        
        return variantMatrix;
    };

}
