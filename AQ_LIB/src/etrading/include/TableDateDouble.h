#pragma once
//
// @File: TableDateDouble.h
// @Description: This is a test program
// @Created: 27 Feb 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of AlgoQuantHub.

#include <string>
#include <exception>
#include <algorithm>
#include <iostream>
#include <vector>
#include <utility>
#include <boost/any.hpp>
#include <boost/date_time/gregorian_calendar.hpp>

#include "CoreEnumerations.h"
#include "DataHolder.h"
#include "Variant.h"
#include "RuleInterface.h"
#include "SerializationResult.h"
#include "HasInstance.h"
#include "HasDataHolderConversion.h"

namespace etrading
{
    // @TableDateDouble: A Class derived from CachedObject
    // this class represents a Term Structure like object with dates in the first column and double values in the second column
    class TableDateDouble
        :  public DataHolder
    {
    public:
        TableDateDouble( const std::string& tableName );
        TableDateDouble(	const std::string& tableName,
                            const std::vector<boost::gregorian::date>& dates,
                            const std::vector<double>& values );
        int addDataPoint( const boost::gregorian::date& date, const double value );
        bool setDates( const std::vector<boost::gregorian::date>& dates );
        bool setValues( const std::vector<double>& values );

        int getIndexOfDate( const boost::gregorian::date& date ) const;
        std::pair<boost::gregorian::date, double> getData( const unsigned int idx ) const;

        std::string getName() const;
        void clear();

		// implementation of SerializeInterface
		// Bring in the overloaded versions of serialize from IsLWOObject
		using IsLWOObject::serialize;
        virtual SerializationResult serialize(	const serialize::SerializationMethodEnum method,
                                                const serialize::SerializationTargetEnum target,
                                                const std::string& targetInfo,
                                                std::vector<std::string>& variableNames,
                                                std::vector<Variant>& variableValues ) const;
		virtual const SchemaObject toSchemaObject() const; 
    protected:
        const std::vector<Variant>& viewDates() const;
        const std::vector<Variant>& viewValues() const;
    private:
        static const AscendingOrderRule< std::vector<boost::gregorian::date> > DATE_CHECKER;

    };
}
