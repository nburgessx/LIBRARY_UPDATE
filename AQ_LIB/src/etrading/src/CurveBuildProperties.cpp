#include <boost/assign.hpp>


#include "CurveBuildProperties.h"
#include "ContainerUtilities.h"
#include "SerializationResult.h"
#include "SchemaObject.h"

namespace etrading
{

    /*static*/ const std::vector<std::string> CurveBuildProperties::VARIABLE_NAMES
        = boost::assign::list_of( "CurveType" )( "CurveCollectionName" )( "CurveIndexName" )
          ( "CCY" )( "AsOfDate" )( "InterpolationMethod" )( "FloatingRateCompoundingFrequency" )
          ( "CurveTenor" )( "OISCompoundingMethod" )( "PaymentDayAdjustment" )( "PaymentCalendar" )
          ( "AccrualDayAdjustment" )( "AccrualCalendar" )( "FixingDayAdjustment" )( "FixingCalendar" )( "OnlyAllowLookup" );

    CurveBuildProperties::CurveBuildProperties( const CurveBuildProperties&& ins )
        : IsLWOObject(ins.getRefToName() , etrading::CURVE_BUILD_PROPERTIES ),
          HasConstInstance<CurveTypeEnum>( ins.getCurveTypeEnum() ),
          extrapolationTypeEnum_( ins.extrapolationTypeEnum_ ),
          curveCollectionName_( ins.curveCollectionName_ ),
          curveIndexName_( ins.curveIndexName_ ),
          ccy_( ins.ccy_ ),
          asOfDate_( ins.asOfDate_ ),
          interpMethod_( ins.interpMethod_ ),
          floatRateTenor_( ins.floatRateTenor_ ),
          oisCompoundingMethod_( ins.oisCompoundingMethod_ ),
          floatRateCompoundingFreq_( ins.floatRateCompoundingFreq_ ),
          paymentDayAdjustment_( ins.paymentDayAdjustment_ ),
          //mlibPaymentCalendar_( ins.mlibPaymentCalendar_ ),
          //paymentCalendar_( ins.paymentCalendar_ ),
          accrualDayAdjustment_( ins.accrualDayAdjustment_ ),
          //mlibAccrualCalendar_( ins.mlibAccrualCalendar_ ),
          //accrualCalendar_( ins.accrualCalendar_ ),
          fixingDayAdjustment_( ins.fixingDayAdjustment_ ),
          //mlibFixingCalendar_( ins.mlibFixingCalendar_ ),
          //fixingCalendar_( ins.fixingCalendar_ ),
          interpolationParameters_( ins.interpolationParameters_ ),
          onlyAllowLookup_( ins.onlyAllowLookup_ )
    {
        setPaymentDayCalendar(ins.getPaymentDayCalendar());
        setAccrualDayCalendar(ins.getAccrualDayCalendar());
        setFixingDayCalendar(ins.getFixingDayCalendar());
    };

    CurveBuildProperties::CurveBuildProperties( const std::string& objectName )
        :	extrapolationTypeEnum_( etrading::CONSTANT_EXTRAPOLATION ),
			IsLWOObject(objectName, etrading::CURVE_BUILD_PROPERTIES ),
          HasConstInstance<CurveTypeEnum>( etrading::OIS_CURVETYPE ),
          mlibPaymentCalendar_( nullptr ),
          mlibAccrualCalendar_( nullptr ),
          mlibFixingCalendar_( nullptr ),
          interpolationParameters_( ),
          onlyAllowLookup_( false )
    {};

    CurveBuildProperties::CurveBuildProperties( const CurveBuildProperties& ins )
		: IsLWOObject(ins.getRefToName(), etrading::CURVE_BUILD_PROPERTIES ),
          HasConstInstance<CurveTypeEnum>( ins.getCurveTypeEnum() ),
          extrapolationTypeEnum_( ins.extrapolationTypeEnum_ ),
          curveCollectionName_( ins.curveCollectionName_ ),
          curveIndexName_( ins.curveIndexName_ ),
          ccy_( ins.ccy_ ),
          asOfDate_( ins.asOfDate_ ),
          interpMethod_( ins.interpMethod_ ),
          floatRateTenor_( ins.floatRateTenor_ ),
          oisCompoundingMethod_( ins.oisCompoundingMethod_ ),
          floatRateCompoundingFreq_( ins.floatRateCompoundingFreq_ ),
          //curveTypeEnum_( ins.getCurveTypeEnum() ),
          paymentDayAdjustment_( ins.paymentDayAdjustment_ ),
          //mlibPaymentCalendar_( ins.mlibPaymentCalendar_ ),
          //paymentCalendar_( ins.paymentCalendar_ ),
          accrualDayAdjustment_( ins.accrualDayAdjustment_ ),
          //mlibAccrualCalendar_( ins.mlibAccrualCalendar_ ),
          //accrualCalendar_( ins.accrualCalendar_ ),
          fixingDayAdjustment_( ins.fixingDayAdjustment_ ),
          //mlibFixingCalendar_( ins.mlibFixingCalendar_ ),
          //fixingCalendar_( ins.fixingCalendar_ ),
          interpolationParameters_( ins.interpolationParameters_ ),
          onlyAllowLookup_( ins.onlyAllowLookup_ )
    {
        // setCalendar(calendar_);
        setPaymentDayCalendar(ins.getPaymentDayCalendar());
        setAccrualDayCalendar(ins.getAccrualDayCalendar());
        setFixingDayCalendar(ins.getFixingDayCalendar());
    }

    /// all daycounts are dc_act365(act_365)  in AlgoQuantLib but we should allow this to be settable later, once we move away
    CurveBuildProperties::CurveBuildProperties(
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
        const std::shared_ptr<const InterpolationParameters>& interpolationParameters,
        const bool onlyAllowLookup
        // ,std::shared_ptr<VariantMatrix>& tenorBasisSettings
    )
        :	curveCollectionName_( curveCollectionName ),
          curveIndexName_( curveIndexName ), ccy_( ccy ), asOfDate_( asOfDate ), interpMethod_( interpMethod ),
          floatRateCompoundingFreq_( floatRateCompoundingFreq ), floatRateTenor_( floatRateTenor ),
          oisCompoundingMethod_( oisCompoundingMethod ),
          extrapolationTypeEnum_( etrading::CONSTANT_EXTRAPOLATION ),
		  IsLWOObject(objectName, etrading::CURVE_BUILD_PROPERTIES),
          HasConstInstance<CurveTypeEnum>( curveTypeEnum ),
          paymentDayAdjustment_( businessDayAdjustment ),
          mlibPaymentCalendar_( nullptr ),
          paymentCalendar_( calendar ),
          accrualDayAdjustment_( businessDayAdjustment ),
          mlibAccrualCalendar_( nullptr ),
          accrualCalendar_( calendar ),
          fixingDayAdjustment_( businessDayAdjustment ),
          mlibFixingCalendar_( nullptr ),
          fixingCalendar_( calendar ),
          interpolationParameters_( interpolationParameters ),
          onlyAllowLookup_( onlyAllowLookup )
    {
        setPaymentDayCalendar( trim_to_upper( calendar.c_str() ) );
        setAccrualDayCalendar( trim_to_upper( calendar.c_str() ) );
        setFixingDayCalendar( trim_to_upper( calendar.c_str() ) );
        assertConsistent();
    };

    CurveBuildProperties::CurveBuildProperties(
        const std::string& objectName,
        const CCY ccy,
        const boost::gregorian::date& asOfDate ) // this is what gets called for an FX FWD curve
        : ccy_( ccy ), asOfDate_( asOfDate ),
          HasConstInstance<CurveTypeEnum>( FWDFXCONST_CURVETYPE ),
		  IsLWOObject( objectName , etrading::CURVE_BUILD_PROPERTIES ),
          onlyAllowLookup_( true )
    {
    };

    CurveBuildProperties::~CurveBuildProperties()
    {
        // do NOT handle the pointer member
    };

    void CurveBuildProperties::setFloatRateCompoundingFreq( const CompoundingFrequencyEnum enumValue )
    {
        floatRateCompoundingFreq_ = enumValue;
        assertConsistent();
    };

    const CompoundingFrequencyEnum CurveBuildProperties::getFloatRateCompoundingFreq() const
    {
        return floatRateCompoundingFreq_;
    };

    const CurveTypeEnum CurveBuildProperties::getCurveTypeEnum() const
    {
        return HasConstInstance<CurveTypeEnum>::getRefToInstance();
    };

    const bool CurveBuildProperties::isConsistent() const
    {
        const CurveTypeEnum curveTypeEnum = getCurveTypeEnum();
        if( curveTypeEnum != etrading::FWDFXCONST_CURVETYPE )
        {
            bool isConsistent = ( curveTypeEnum != etrading::OIS_CURVETYPE && oisCompoundingMethod_ == etrading::SIMPLE_COMPOUNDING_METHOD ) || ( curveTypeEnum == etrading::OIS_CURVETYPE );
            isConsistent = isConsistent  && ( ( curveTypeEnum == etrading::OIS_CURVETYPE &&  floatRateTenor_ == etrading::CURVE_TENOR_1D ) || ( curveTypeEnum != etrading::OIS_CURVETYPE &&  floatRateTenor_ != etrading::CURVE_TENOR_1D ) );
            return isConsistent;
        }
        return true;
    };

    void CurveBuildProperties::assertConsistent() const
    {
        if( !isConsistent() )
        {
            // mlibCalendar_.reset();
            throw ETradingException( ( boost::format( "Inconsistency between curve of type (%s) and float rate compounding method (%s) and float rate tenor (%s) or interpolationMethod (%s) and supplied interpolation data" )
                                       % toString( getCurveTypeEnum() ).c_str()
                                       % toString( oisCompoundingMethod_ ).c_str()
                                       % toString( floatRateTenor_ ).c_str()
                                       % toString( interpMethod_ ).c_str() ).str() );
        };
    };

    SerializationResult CurveBuildProperties::serialize(	const serialize::SerializationMethodEnum method,
            const serialize::SerializationTargetEnum target,
            const std::string& targetInfo,
            std::vector<std::string>& variableNames,
            std::vector<Variant>& variableValues ) const
    {
        return toSchemaObject().serialize( method, target, targetInfo, variableNames, variableValues );
    };


    void CurveBuildProperties::setPaymentDayCalendar( const std::string& calendar )
    {
        // TODO: Temp Fix, if multiple holiday cities are provided, use the first only
        // Remove this fix once this code has been extended to accomodate multiple holiday cities
        //-----------------------------------------------------------------------------
        std::string useFirstHolidayCity;
        bool multipleCalendarsSpecified = ( calendar.find(':') != std::string::npos );
        if ( multipleCalendarsSpecified )
        {
            size_t position = calendar.find(':');
            if ( position == 0 )
            {
                throw ETradingException( ( boost::format( "#Error: Curve Build Property Error; holiday city calendars cannot begin with the ':' character." ) ).str() );
            }
            useFirstHolidayCity = calendar.substr( 0, position );

            paymentCalendar_ = trim_to_upper( useFirstHolidayCity.c_str() );
            mlibPaymentCalendar_ = &AQLMathCalendarSet::getCalendar( useFirstHolidayCity.c_str() );
        }
        else
        //-----------------------------------------------------------------------------
        {
            paymentCalendar_ = trim_to_upper( calendar.c_str() );
            mlibPaymentCalendar_ = &AQLMathCalendarSet::getCalendar( paymentCalendar_.c_str() );
        }
    };

    const AQLMathCalendar* CurveBuildProperties::getMlibPaymentCalendar() const
    {
        return mlibPaymentCalendar_;
    };

    const std::string CurveBuildProperties::getPaymentDayCalendar() const
    {
        return paymentCalendar_;
    }

    void CurveBuildProperties::setAccrualDayCalendar( const std::string& calendar )
    {
        // TODO: Temp Fix, if multiple holiday cities are provided, use the first only
        // Remove this fix once this code has been extended to accomodate multiple holiday cities
        //-----------------------------------------------------------------------------
        std::string useFirstHolidayCity;
        bool multipleCalendarsSpecified = ( calendar.find(':') != std::string::npos );
        if ( multipleCalendarsSpecified )
        {
            size_t position = calendar.find(':');
            if ( position == 0 )
            {
                throw ETradingException( ( boost::format( "#Error: Curve Build Property Error; holiday city calendars cannot begin with the ':' character." ) ).str() );
            }
            useFirstHolidayCity = calendar.substr( 0, position );

            accrualCalendar_ = trim_to_upper( useFirstHolidayCity.c_str() );
            mlibAccrualCalendar_ = &AQLMathCalendarSet::getCalendar( useFirstHolidayCity.c_str() );
        }
        else
        //-----------------------------------------------------------------------------
        {
            accrualCalendar_ = trim_to_upper( calendar.c_str() );
            mlibAccrualCalendar_ = &AQLMathCalendarSet::getCalendar( accrualCalendar_.c_str() );
        }
    }

    const AQLMathCalendar* CurveBuildProperties::getMlibAccrualCalendar() const
    {
        return mlibAccrualCalendar_;
    };

    const std::string CurveBuildProperties::getAccrualDayCalendar() const
    {
        return accrualCalendar_;
    }

    void CurveBuildProperties::setFixingDayCalendar( const std::string& calendar )
    {
        // TODO: Temp Fix, if multiple holiday cities are provided, use the first only
        // Remove this fix once this code has been extended to accomodate multiple holiday cities
        //-----------------------------------------------------------------------------
        std::string useFirstHolidayCity;
        bool multipleCalendarsSpecified = ( calendar.find(':') != std::string::npos );
        if ( multipleCalendarsSpecified )
        {
            size_t position = calendar.find(':');
            if ( position == 0 )
            {
                throw ETradingException( ( boost::format( "#Error: Curve Build Property Error; holiday city calendars cannot begin with the ':' character." ) ).str() );
            }
            useFirstHolidayCity = calendar.substr( 0, position );

            fixingCalendar_ = trim_to_upper( useFirstHolidayCity.c_str() );
            mlibFixingCalendar_ = &AQLMathCalendarSet::getCalendar( useFirstHolidayCity.c_str() );
        }
        else
        //-----------------------------------------------------------------------------
        {
            fixingCalendar_ = trim_to_upper( calendar.c_str() );
            mlibFixingCalendar_ = &AQLMathCalendarSet::getCalendar( fixingCalendar_.c_str() );
        }
    }

    const AQLMathCalendar* CurveBuildProperties::getMlibFixingCalendar() const
    {
        return mlibFixingCalendar_;
    };

    const std::string CurveBuildProperties::getFixingDayCalendar() const
    {
        return fixingCalendar_;
    }

    void CurveBuildProperties::setExtrapolation( const ExtrapolationTypeEnum extrapolationType )
    {
        extrapolationTypeEnum_ = extrapolationType;
    }

    const ExtrapolationTypeEnum CurveBuildProperties::getExtrapolationTypeEnum() const
    {
        return extrapolationTypeEnum_ ;
    }

    const std::shared_ptr<const InterpolationParameters>& CurveBuildProperties::getInterpolationParameters() const
    {
        return interpolationParameters_;
    }

    const SchemaObject CurveBuildProperties::toSchemaObject() const
    {

        SchemaObject schemaObject( etrading::CURVE_BUILD_PROPERTIES, getRefToName() );
        schemaObject.setColumnData( "CURVE_BUILD_PROPERTIES", "VARIABLE_NAME", CurveBuildProperties::VARIABLE_NAMES );

        std::vector<Variant> curveConventionValues;
        curveConventionValues.emplace_back( Variant( toString( getCurveTypeEnum() ) ) );
        curveConventionValues.emplace_back( Variant( curveCollectionName_ ) );
        curveConventionValues.emplace_back( Variant( curveIndexName_ ) );
        curveConventionValues.emplace_back( Variant( toString( ccy_ ) ) ); //         // TODO: C++11 'enum class' one day...
        curveConventionValues.emplace_back( Variant( asOfDate_ ) );
        curveConventionValues.emplace_back( Variant( toString( interpMethod_ ) ) );
        curveConventionValues.emplace_back( Variant( toString( floatRateCompoundingFreq_ ) ) );
        curveConventionValues.emplace_back( Variant( toString( floatRateTenor_ ) ) );
        curveConventionValues.emplace_back( Variant( toString( oisCompoundingMethod_ ) ) );
        curveConventionValues.emplace_back( Variant( toString( paymentDayAdjustment_ ) ) );
        curveConventionValues.emplace_back( Variant( paymentCalendar_ ) );
        curveConventionValues.emplace_back( Variant( toString( accrualDayAdjustment_ ) ) );
        curveConventionValues.emplace_back( Variant( accrualCalendar_ ) );
        curveConventionValues.emplace_back( Variant( toString( fixingDayAdjustment_ ) ) );
        curveConventionValues.emplace_back( Variant( fixingCalendar_ ) );
        curveConventionValues.emplace_back( Variant( onlyAllowLookup_ ) );
        schemaObject.setColumnData( "CURVE_BUILD_PROPERTIES", 1, curveConventionValues );

        if( this->interpolationParameters_ != nullptr )
        {
            schemaObject.addDataSchema( this->interpolationParameters_.get()->generateDataSchema() );
            schemaObject.absorbDataSchema( this->interpolationParameters_.get()->toSchemaObject() );
        }

        return schemaObject;
    };

    const bool CurveBuildProperties::isOnlyAllowLookup() const
    {
        return onlyAllowLookup_;
    };


}