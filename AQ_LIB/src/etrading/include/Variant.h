#pragma once
//
// @File: Variant.h
// @Description: This Class wraps around a boost::spirit::hold_any value to represent a data structure similar to an excel cell.
//  This class allows a set of different value types to be set and accessed from the same class instance. The types are as defined
// in etrading::ContainedEnumType.
// @Created: 27 Feb 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of AlgoQuantHub.

#include <iostream>
#include <sstream>
#include <locale>
#include <utility>
#include <vector>
#include <boost/format.hpp>
#include <boost/spirit/home/support/detail/hold_any.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <sstream>

#include "ETradingException.h"
#include "DateUtilities.h"
#include "CoreEnumerations.h"
#include "TypeName.h"
#include "AQLDate.h"
#include "AQLString.h"
#include "ExceptionMacros.h"


namespace etrading
{
    // Forward Declarations
    // ==========================================================================================

    class Variant;  // forward declaration to avoid double linking warnings through templates
    std::ostream& operator<<( std::ostream& os, const Variant& cValue );
    typedef std::vector<Variant> VariantVector;
    typedef std::vector<std::vector<Variant>> VariantMatrix;

    // Helper Method(s)
    // ==========================================================================================

	// Transpose a Variant Matrix
	VariantMatrix transpose( const VariantMatrix & input);

    // Converts an AQLStringMatrix to a JSON VariantMatrix, note transposes by default to match the default JSON schema convention
    VariantMatrix toVariantMatrixFromLAStringMatrix( const AQLStringMatrix & laStringMatrix, const bool & transpose = true );

    // Converts a StandardStringMatrix to a JSON VariantMatrix, note transposes by default to match the default JSON schema convention
    VariantMatrix toVariantMatrixFromStandardStringMatrix( const StandardStringMatrix & standardStringMatrix, const bool & transpose = true );

    // Converts a VariantMatrix to AQLStringMatrix, note transposes by default to match the default JSON schema convention
    AQLStringMatrix toAQLStringMatrixFromVariantMatrix( const VariantMatrix & variantMatrix, const bool & transpose = true );

    // Converts a VariantMatrix to StandardStringMatrix, note transposes by default to match the default JSON schema convention
    StandardStringMatrix toStandardStringMatrixFromVariantMatrix( const VariantMatrix & variantMatrix, const bool & transpose = true );

	// Test if a string can be a number
	bool canStringConvertToNumber( const StandardString & str );
	
	// Test if a string can be a number
	bool canStringConvertToNumber( const AQLString & str );
	
	// Test if a string can be a number
	bool canStringConvertToNumber( const char * str );

    // @Variant: This class is the leaf node, representing a Variant type (like an excel cell)
    // ==========================================================================================
    class Variant
    {
    public:
        Variant();

        template <typename S>
        Variant( const S& inputValue )
        {
            setValue( inputValue );
        }
        

        Variant( const char* inputValue );

		// Helper Method - Get Variant value as a string
        std::string getValueAsString() const;

		// Test if a string can be a number
		bool canStringConvertToNumber() const;

        template <typename S>
        S getValue() const
        {
            // 1.  Try Cast to type S using boost::any_cast
            try
            {
                return boost::spirit::any_cast<S>( value_ );
            }
            catch ( boost::spirit::bad_any_cast& )
            {
                // 2.  In the event of a boost::any_cast failure try using stringstream to cast to type S
                // Boost::any_cast fails for some fringe cases, strangely enough variants of type integer fail to cast to double.
                std::string valueAsString = this->getValueAsString();
                try
                {
                    S resultAsTypeS;
                    std::stringstream ss;
                    ss << std::setprecision(20) << valueAsString; // Precision 20 digits
                    ss >> std::setprecision(20) >> resultAsTypeS; // Precision 20 digits
                    if ( ss.fail() )
                    {
                        AQ_THROW("Invalid Variant Data: Unable to cast '" + valueAsString  + "' to " + typeid(S).name() )
                    }
                    return resultAsTypeS;
                }
                catch(...)
                {
                    AQ_THROW("Invalid Variant Data: Unable to cast '" + valueAsString  + "' to " + typeid(S).name() )
                }
            }
        }

        Variant( const Variant& rhs ); //  NOT defaulted http://stackoverflow.com/questions/24065769/boostspirithold-any-memory-corruption
        Variant& operator=( const Variant& rhs ); //  NOT defaulted http://stackoverflow.com/questions/24065769/boostspirithold-any-memory-corruption

        const ContainedTypeEnum getType() const;
        std::string toString( bool addQuotesIfString = false ) const;
        const bool isEmpty() const;

        operator double() const;
        operator int() const;
        operator std::string() const;
        operator boost::gregorian::date() const;
        operator AQLDate() const;
        operator AQLString() const;


        // ------------------------------------------------------------------
        // Thse Methods Compare Variant with other Non-Variant concrete types
        // ------------------------------------------------------------------
        template <typename Z>
        bool operator==( const Z& z ) const
        {
            return getValue<Z>() == z;
        }

        template <typename Z>
        bool operator<( const Z& z ) const
        {
            return getValue<Z>() < z;
        }

        template <typename Z>
        bool operator>( const Z& z ) const
        {
            return getValue<Z>() > z;
        }

        template <typename Z>
        bool operator<=( const Z& z ) const
        {
            return getValue<Z>() <= z;
        }

        template <typename Z>
        bool operator>=( const Z& z ) const
        {
            return getValue<Z>() >= z;
        }

        // ------------------------------------------------------------------
        // Thse Methods Compare Variant with other Variant types
        // ------------------------------------------------------------------
        bool operator==( const Variant& rhs ) const;
        
        bool operator<( const Variant& rhs ) const;

        bool operator>( const Variant& rhs ) const;        
        
        bool operator<=( const Variant& rhs ) const;
        
        bool operator>=( const Variant& rhs ) const;



        static std::vector<etrading::ContainedTypeEnum> getContainedTypeInfo( const std::vector<std::vector<Variant>>& inputData );


        static VariantMatrix transpose( const VariantMatrix& flexibleData, const Variant& defaultVariant = Variant("") );


        // Template to transform a Vector of Variant to a Vector of a specific type A supplied by the user
        // Note: Variants of Mixed type throw errors if unconvertable to the type specified
        template <typename A>
        static std::vector<A> createValueVector( const std::vector<Variant>& inputVector )
        {
            std::vector<A> retVec;
            std::transform( inputVector.cbegin(), inputVector.cend(), std::back_inserter( retVec ), []( const Variant & v )
            {
                return static_cast<A>( v );
            } );
            return retVec;
        }

        // Template to transform a Matrix of Variant to a Matrix of a specific type A supplied by the user
        // Note: Variants of Mixed type throw errors if unconvertable to the type specified
        template <typename A>
        static std::vector< std::vector<A> > createValueMatrix( const std::vector<std::vector<Variant>>& inputMatrix )
        {
            std::vector< std::vector<A> > resultMatrix;
            for ( unsigned int i = 0u; i < inputMatrix.size(); ++i )
            {
                std::vector<A> columnVector;
                std::transform( inputMatrix.cbegin(), inputMatrix.cend(), std::back_inserter( columnVector ), []( const Variant & v )
                {
                    return static_cast<A>( v );
                } );
                
                resultMatrix.emplace_back( columnVector );
            }
            return resultMatrix;
        }

        template <typename A>
        static std::vector<Variant> createVariantVector( const std::vector<A>& inputVector,
                                                         const etrading::ContainedTypeEnum targetType,
                                                         unsigned int padToSize = 0 )
        {
            if( isConvertibleVector( inputVector, targetType ) )
            {
                std::vector<Variant> vecWrapped;
                vecWrapped.insert( vecWrapped.end(), inputVector.cbegin(), inputVector.cend() );
                while(padToSize > vecWrapped.size())
                {
                    vecWrapped.emplace_back("");
                }
                return vecWrapped;
            }
            else
            {
                std::string errString( boost::str( boost::format( "#Error: createVariantVector: Cannot covert from Vector of %s to Vector Variant with type %s" )
                                                   % TypeName::get<A>()
                                                   % etrading::toString( targetType ) ) );
                throw ETradingException( errString.c_str() );
            }
        };



        //
        // Scalar Variant Conversion
        // ----------------------------------------------------------------------------------------------------------

        // Template - Return false if type 'G' is not a type listed below in the template specialization implementation(s)
        template <typename G>
        static bool isConvertible( const std::vector<G>& inputColumn, const etrading::ContainedTypeEnum g )
        {
            return false;
	    }

        //
        // Variant Vector Conversion
        // ----------------------------------------------------------------------------------------------------------
        
        // Template - Return false if type 'G' is not a type listed below in the template specialization implementation(s)
        template <typename G>
        static bool isConvertibleVector( const std::vector<G>& inputColumn, const etrading::ContainedTypeEnum g )
        {
            return isConvertible( inputColumn, g );
		}


    private:

        friend std::ostream& operator<<( std::ostream& os, const Variant& cValue );
        ContainedTypeEnum type_;
        boost::spirit::hold_any value_;

        // MACRO:
        #define SETVALUE(x,X) void setValue( const x& inputValue )  \
		{                                                           \
			type_ = X;                                              \
			if ( !value_.empty() )                                  \
			{                                                       \
				value_.reset();                                     \
			}                                                       \
			value_ = inputValue;                                    \
		};

        SETVALUE( int, INTEGER_VALUE)
        SETVALUE( double, DOUBLE_VALUE)
        SETVALUE( bool, BOOL_VALUE)
        SETVALUE( boost::gregorian::date, DATE_VALUE)
        SETVALUE( std::string, STRING_VALUE)

        // TODO: remove this and replace clients with the implementation
        static const std::string getDateString( const Variant& cv )
        {
            return etrading::toYYYYMMDDFromGregorianDate( cv.getValue<boost::gregorian::date>() );
        }

    };

	// ------------------------
	// Template specializations 
	// ------------------------

	template <>
    Variant::Variant( const AQLDate& inputValue );
		
	template <>
    Variant::Variant( const AQLString& inputValue );


    // Check if type 'int' is convertible to contained type g
    template <>
    bool Variant::isConvertible( const std::vector<int>& inputColumn, const etrading::ContainedTypeEnum g );

	// Check if type 'double' is convertible to contained type g
    template <>
    bool Variant::isConvertible( const std::vector<double>& inputColumn, const etrading::ContainedTypeEnum g );

	// Check if type 'bool' is convertible to contained type g
    template <>
    bool Variant::isConvertible( const std::vector<bool>& inputColumn, const etrading::ContainedTypeEnum g );

	// Check if type 'boost::gregorian::date' is convertible to contained type g
    template <>
    bool Variant::isConvertible( const std::vector<boost::gregorian::date>& inputColumn, const etrading::ContainedTypeEnum g );

	// Check if type 'std::string' is convertible to contained type g
    template <>
    bool Variant::isConvertible( const std::vector<std::string>& inputColumn, const etrading::ContainedTypeEnum g );

	// Check if type 'Variant' is convertible to contained type g
    template <>
    bool Variant::isConvertible( const std::vector<Variant>& inputColumn, const etrading::ContainedTypeEnum g );

	// Check if a vector of type 'Variant' is convertible to contained type g
    template <>
    bool Variant::isConvertibleVector( const std::vector<Variant>& inputColumn, const etrading::ContainedTypeEnum g );
      


	template <>
    std::vector<Variant> Variant::createVariantVector( const std::vector<Variant>& inputVector, const etrading::ContainedTypeEnum targetType, unsigned int padToSize );

	template <>
    std::vector<boost::gregorian::date> Variant::createValueVector( const std::vector<Variant>& inputVector );

};

