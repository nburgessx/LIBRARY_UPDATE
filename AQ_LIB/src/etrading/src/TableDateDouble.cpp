// @File: TableDateDouble.cpp
// @Description: Implementation of TableDateDouble
// @Created: 04 April 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of Mizuho International.


#include <vector>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include "TableDateDouble.h"
#include "RuleInterface.h"
#include "CoreEnumerations.h"
#include "ETradingException.h"
#include "StaticStructureStore.h"
#include "ContainerUtilities.h"
#include "StatisticsUtilities.h"
#include "UserUtilities.h"
#include "SerializeInstance.h"
#include <set>

namespace etrading
{

    /*static*/ const AscendingOrderRule< std::vector<boost::gregorian::date> > TableDateDouble::DATE_CHECKER;

    // TODO: replace this with a using directive
	#define SK_NAME toString(TABLE)

    TableDateDouble::TableDateDouble( const std::string& tableName )
        : DataHolder( TABLE ,tableName )
    {};

    TableDateDouble::TableDateDouble(	const std::string& tableName,
                                        const std::vector<boost::gregorian::date>& dates,
                                        const std::vector<double>& values )
        : DataHolder( TABLE, tableName )
    {
        // Check for Duplicate Dates
        std::set< boost::gregorian::date> uniqueDatesSet;
        for( size_t i = 0; i < dates.size(); ++i )
        {
            if ( uniqueDatesSet.find( dates[i] ) != uniqueDatesSet.end() )
            {
                // Convert date to string for error message
                const std::locale dateFormatDDMMYYYY( std::locale::classic(), new boost::gregorian::date_facet("%d-%b-%Y"));
                std::ostringstream os;
                os.imbue(dateFormatDDMMYYYY);
                os << dates[i];

                throw ETradingException( ( boost::format( "#Error: Duplicate dates are not allowed, date %s is a duplicate date" ) % os.str() ).str().c_str() );
            }
            uniqueDatesSet.insert( dates[i] );
        }

        setColumnData( SK_NAME, 0, dates );
        setColumnData( SK_NAME, 1, values );
    }

    int TableDateDouble::addDataPoint( const boost::gregorian::date& date, const double value )
    {
        // getValuesCopy(const std::string& keyName, const unsigned int index, bool isColumn = true) const;
        std::vector<boost::gregorian::date> dates =
            Variant::createValueVector<boost::gregorian::date>( this->getValuesCopy( SK_NAME, 0, true ) );
        std::vector<double> values =
            Variant::createValueVector<double>( this->getValuesCopy( SK_NAME, 1, true ) );

        if( dates.size() != values.size() )
        {
            throw ETradingException( ( boost::format( "Cannot add a data point to a TableDateDouble when dates and values are of unequal size %i (dates) and %i (values)" ) % dates.size() % values.size() ).str().c_str() );
        }

        auto cit = std::find_if( dates.begin(), dates.end(), [&date]( const boost::gregorian::date & el )
        {
            return ( el >= date );
        }  );
        int idxToInsertAt =  ( cit != dates.end() ? cit - dates.begin() : -1 );

        if( idxToInsertAt >= 0 )
        {
            dates.insert( dates.begin() + idxToInsertAt, date );
            values.insert( values.begin() + idxToInsertAt, value );
        }
        else
        {
            dates.push_back( date );
            values.push_back( value );
        }
        setColumnData( SK_NAME, 0, dates );
        setColumnData( SK_NAME, 1, values );
        return idxToInsertAt;
    }

    bool TableDateDouble::setDates( const std::vector<boost::gregorian::date>& dates )
    {
        // Check for Duplicate Dates
        std::set< boost::gregorian::date> uniqueDatesSet;
        for( size_t i = 0; i < dates.size(); ++i )
        {
            if ( uniqueDatesSet.find( dates[i] ) != uniqueDatesSet.end() )
            {
                // Convert date to string for error message
                const std::locale dateFormatDDMMYYYY( std::locale::classic(), new boost::gregorian::date_facet("%d-%b-%Y"));
                std::ostringstream os;
                os.imbue(dateFormatDDMMYYYY);
                os << dates[i];

                throw ETradingException( ( boost::format( "#Error: Duplicate dates are not allowed, date %s is a duplicate date" ) % os.str() ).str().c_str() );
            }
            uniqueDatesSet.insert( dates[i] );
        }

        setColumnData( SK_NAME, 0, dates );
        return true;
    }

    bool TableDateDouble::setValues( const std::vector<double>& values )
    {
        setColumnData( SK_NAME, 1, values );
        return true;
    }

    int TableDateDouble::getIndexOfDate( const boost::gregorian::date& date ) const
    {
        std::vector<boost::gregorian::date> dates = Variant::createValueVector<boost::gregorian::date>( getValuesCopy( SK_NAME, 0, true ) ); // should we iterate the Variants to avoid the copy ?!
        if( !dates.empty() )
        {
            auto cit = std::find_if( dates.begin(), dates.end(), [&date]( const boost::gregorian::date & el )
            {
                return ( el == date );
            }  );
            return  ( cit != dates.end() ? cit - dates.begin() : -1 );
        }
        else
        {
            return -1;
        }
    }

    const std::vector<Variant>& TableDateDouble::viewDates() const
    {
        return viewData( 0 );
    }

    const std::vector<Variant>& TableDateDouble::viewValues() const
    {
        return viewData( 1 );
    }

    std::string TableDateDouble::getName() const
    {
        return HasName::getRefToName();
    }

    void TableDateDouble::clear()
    {
        clearData();
    }

    std::pair<boost::gregorian::date, double> TableDateDouble::getData( const unsigned int idx ) const
    {
        const std::vector<Variant>& datesData = viewDates();
        if( idx >= datesData.size() )
        {
            throw ETradingException( ( boost::format( "Illegal Date index requested: %i when there are %i dates available" ) % idx %  datesData.size() ).str().c_str()  );
        }
        const std::vector<Variant>& valuesData = viewValues();
        if( idx >= valuesData.size() )
        {
            throw ETradingException( ( boost::format( "Illegal Value index requested: %i when there are %i values available" ) % idx %  valuesData.size() ).str().c_str()  );
        }
        return std::make_pair( datesData[ idx ].getValue<boost::gregorian::date>(), valuesData[ idx ].getValue<double>() );
    }

    SerializationResult TableDateDouble::serialize( const serialize::SerializationMethodEnum method,
            const serialize::SerializationTargetEnum target,
            const std::string& targetInfo,
            std::vector<std::string>& variableNames,
            std::vector<Variant>& variableValues ) const
    {
        variableNames.push_back( DataHolder::OBJECT_NAME_LABEL );
        variableValues .push_back( Variant( getName() ) );
        return serializeInstance( *this, method, target, targetInfo, variableNames, variableValues );
    };

	const SchemaObject TableDateDouble::toSchemaObject() const
	{
		return SchemaObject( static_cast<const DataHolder&>(*this), this->getRefToName());
	};


}


/*
double TableDateDouble::calculateInterpolatedValue(
    const  boost::gregorian::date& date,
    const InterpolationEnum interpolationType ) const
{

    auto firstColumnInfo = getDataFormat( 0 );
    auto secondColumnInfo = getDataFormat( 1 );
    if( firstColumnInfo.first != secondColumnInfo.first )
        throw ETradingException( ( boost::format( "Cannot interpolate a TableDateDouble when dates and values are of unequal size %i (dates) and %i (values)" )
                                   % firstColumnInfo.first
                                   % secondColumnInfo.first ).str().c_str() );
    int existingIdx = getIndexOfDate( date );
    if( existingIdx >= 0 )
    {
        return viewValues()[ existingIdx ].getValue<double>();
    }
    else
        // TODO: export this implementation to another place so it can be shared
        // TODO: actually implement the interpolation
    {
        return etrading::calcMean( Variant::createValueVector<double>( getValuesCopy( SK_NAME, 1, true ) ) );
    }
};
*/