// @File: Variant.cpp
// @Description: Implementation of Variant
// @Created: 04 April 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of AlgoQuantHub.

#include <algorithm>
#include <utility>
#include <tuple>
#include <iterator>
#include <functional>
#include <numeric>
#include <exception>
#include <cmath>
#include <string>		// 'find_first_not_of' method
#include <cstdlib>      // strtod (string to double) method

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/assign.hpp>
#include <boost/regex.hpp>

#include "Variant.h"
#include "DateUtilities.h"
#include "ContainerUtilities.h"
#include "ParameterValidation.h"
#include "ScheduleValidation.h"     // Matrix Transpose Method
#include "StaticStructureStore.h"
#include "DebugMacros.h"


namespace etrading
{
    // Helper Method(s)
    // ==========================================================================================

	// Transpose a Variant Matrix
	VariantMatrix transpose(const VariantMatrix & input)
	{
		size_t rowSize = input.size();
		if ( rowSize == 0 ) return input;
        
        // Only allow Rectangular Matrices, we do not support jagged matrices here
        size_t columnSize = input[0].size();
        for (size_t i = 1; i < rowSize; ++i) // Start from base 1 not 0
        {
            AQ_REQUIRE( columnSize == input[i].size(), "Invalid Data - Transpose of jagged matrices is not supported" );
        }
        
        // Transpose Data
		VariantMatrix output( columnSize, VariantVector( rowSize ) );

		for (size_t i = 0; i < rowSize; ++i)
		{
            for (size_t j = 0; j < columnSize; ++j)
			{
				output[j][i] = input[i][j];
			}
		}
		return output;
	}

    // Converts an LAStringMatrix to a JSON VariantMatrix, note transposes by default to match the default JSON schema convention
    VariantMatrix toVariantMatrixFromLAStringMatrix( const LAStringMatrix & laStringMatrix, const bool & transpose )
    {
        AQ_REQUIRE( !laStringMatrix.empty(),      "Invalid Data: Data Matrix is Empty" )
        AQ_REQUIRE( !laStringMatrix[0].empty(),   "Invalid Data: Data Matrix is Empty" )
        
        if ( transpose )
        {
            VariantMatrix variantMatrix( laStringMatrix[0].size() );
            for ( size_t col = 0; col < laStringMatrix[0].size(); ++col )
            {
                VariantVector thisCol( laStringMatrix.size() );
                for ( size_t row = 0; row < laStringMatrix.size(); ++row )
                {
                    thisCol[row] = laStringMatrix[row][col];
                }
                variantMatrix[col] = thisCol;
            }
            return variantMatrix;
        }
        else
        {
            VariantMatrix variantMatrix( laStringMatrix.size() );
            for ( size_t row = 0; row < laStringMatrix.size(); ++row )
            {
                VariantVector thisRow( laStringMatrix[row].size() );
                for ( size_t col = 0; col < laStringMatrix[row].size(); ++col )
                {
                    thisRow[col] = laStringMatrix[row][col];
                }
                variantMatrix[row] = thisRow;
            }
            return variantMatrix;
        }
    }

    // Converts a StandardStringMatrix to a JSON VariantMatrix, note transposes by default to match the default JSON schema convention
    VariantMatrix toVariantMatrixFromStandardStringMatrix( const StandardStringMatrix & standardStringMatrix, const bool & transpose )
    {
        AQ_REQUIRE( !standardStringMatrix.empty(),      "Invalid Data: Data Matrix is Empty" )
        AQ_REQUIRE( !standardStringMatrix[0].empty(),   "Invalid Data: Data Matrix is Empty" )
        
        if ( transpose )
        {
            VariantMatrix variantMatrix( standardStringMatrix[0].size() );
            for ( size_t col = 0; col < standardStringMatrix[0].size(); ++col )
            {
                VariantVector thisCol( standardStringMatrix.size() );
                for ( size_t row = 0; row < standardStringMatrix.size(); ++row )
                {
                    thisCol[row] = standardStringMatrix[row][col];
                }
                variantMatrix[col] = thisCol;
            }
            return variantMatrix;
        }
        else
        {
            VariantMatrix variantMatrix( standardStringMatrix.size() );
            for ( size_t row = 0; row < standardStringMatrix.size(); ++row )
            {
                VariantVector thisRow( standardStringMatrix[row].size() );
                for ( size_t col = 0; col < standardStringMatrix[row].size(); ++col )
                {
                    thisRow[col] = standardStringMatrix[row][col];
                }
                variantMatrix[row] = thisRow;
            }
            return variantMatrix;
        }
    }
    
    // Converts a VariantMatrix to LAStringMatrix, note transposes by default to match the default JSON schema convention
    LAStringMatrix toLAStringMatrixFromVariantMatrix( const VariantMatrix & variantMatrix, const bool & transpose )
    {
        AQ_REQUIRE( !variantMatrix.empty(),      "Invalid Data: Data Matrix is Empty" )
        AQ_REQUIRE( !variantMatrix[0].empty(),   "Invalid Data: Data Matrix is Empty" )
       
        if ( transpose )
        {
            LAStringMatrix laStringMatrix( variantMatrix[0].size() );
            for ( size_t col = 0; col < variantMatrix[0].size(); ++col )
            {
                LAStringVector thisCol( variantMatrix.size() );
                for ( size_t row = 0; row < variantMatrix.size(); ++row )
                {
                    thisCol[row] = variantMatrix[row][col].getValueAsString().c_str();
                }
                laStringMatrix[col] = thisCol;
            }
            return laStringMatrix;
        }
        else
        {
            LAStringMatrix laStringMatrix( variantMatrix.size() );
            for ( size_t row = 0; row < variantMatrix.size(); ++row )
            {
                LAStringVector thisRow( variantMatrix[row].size() );
                for ( size_t col = 0; col < variantMatrix[row].size(); ++col )
                {
                    thisRow[col] = variantMatrix[row][col].getValueAsString().c_str();
                }
                laStringMatrix[row] = thisRow;
            }
            return laStringMatrix;
         }
    }

    // Converts a VariantMatrix to StandardStringMatrix, note transposes by default to match the default JSON schema convention
    StandardStringMatrix toStandardStringMatrixFromVariantMatrix( const VariantMatrix & variantMatrix, const bool & transpose )
    {
        AQ_REQUIRE( !variantMatrix.empty(),      "Invalid Data: Data Matrix is Empty" )
        AQ_REQUIRE( !variantMatrix[0].empty(),   "Invalid Data: Data Matrix is Empty" )
       
        if ( transpose )
        {
            StandardStringMatrix standardStringMatrix( variantMatrix[0].size() );
            for ( size_t col = 0; col < variantMatrix[0].size(); ++col )
            {
                StandardStringVector thisCol( variantMatrix.size() );
                for ( size_t row = 0; row < variantMatrix.size(); ++row )
                {
                    thisCol[row] = variantMatrix[row][col].getValueAsString();
                }
                standardStringMatrix[col] = thisCol;
            }
            return standardStringMatrix;
        }
        else
        {
            StandardStringMatrix standardStringMatrix( variantMatrix.size() );
            for ( size_t row = 0; row < variantMatrix.size(); ++row )
            {
                StandardStringVector thisRow( variantMatrix[row].size() );
                for ( size_t col = 0; col < variantMatrix[row].size(); ++col )
                {
                    thisRow[col] = variantMatrix[row][col].getValueAsString();
                }
                standardStringMatrix[row] = thisRow;
            }
            return standardStringMatrix;
        }
    }

	// Test if a string can be a number
	bool canStringConvertToNumber( const StandardString & str )
	{
		if ( str.empty() ) return false;
        
        // Convert to string to double, if pEnd
        char* pEnd;
        strtod( str.c_str(), &pEnd );
        return *pEnd == 0;
	}

	// Test if a string can be a number
	bool canStringConvertToNumber( const LAString & str )
	{
		const StandardString s = str.c_str();
		return canStringConvertToNumber( s );
	}

	// Test if a string can be a number
	bool canStringConvertToNumber( const char * str )
	{
		const StandardString s = str;
		return canStringConvertToNumber( s );
	}
	// Variant Class Methods
    // ==========================================================================================

	// Helper Method - Get Variant value as a string
    std::string Variant::getValueAsString() const
    {
        std::string result;
        std::stringstream ss;
        etrading::ContainedTypeEnum thisType = type_;

        switch ( thisType )
        {
            case STRING_VALUE:
                result = this->getValue<std::string>();
                break;

            case BOOL_VALUE:
                ss << this->getValue<bool>();
                result = ss.str();
                break;

            case INTEGER_VALUE:
                ss << this->getValue<int>();
                result = ss.str();
                break;

            case DOUBLE_VALUE:
                // Give double values high-precision to best manage double data truncation
                ss << std::setprecision(20) << this->getValue<double>();
                result = ss.str();
                break;

            case DATE_VALUE:
                ss <<  getDateString( *this );
                result = ss.str();
                break;

            default:
                AQ_THROW( "Invalid Variant Data Type" );
                break;
        }

        return result;
    }

	// Test if a variant can be a number
	bool Variant::canStringConvertToNumber() const
	{
		const StandardString str = getValueAsString();
        if (str.empty()) return false;

        // Convert to string to double, if pEnd
        char* pEnd;
        strtod(str.c_str(), &pEnd);
        return *pEnd == 0;
	}

	// Template specializations
	template <>
    Variant::Variant( const LADate& inputValue )
    {
        setValue( toGregorianDateFromLADate(inputValue) );
    }
        
	template <>
    Variant::Variant( const LAString& inputValue )
    {
		setValue(std::string(inputValue.getCString()));
    }


	template <>
    std::vector<Variant> Variant::createVariantVector( const std::vector<Variant>& inputVector, const etrading::ContainedTypeEnum targetType, unsigned int padToSize )
    {
        return inputVector;
    }

	// Template Specializations
    // ------------------------

    // Check if type 'int' is convertible to contained type g
	template <>
    bool Variant::isConvertible(  const std::vector<int>& inputColumn, const etrading::ContainedTypeEnum g )
    {
        return ( g == INTEGER_VALUE ) ||  ( g == VARIANT_VALUE );
    }

	
    // Check if type 'double' is convertible to contained type g
	template <>
    bool Variant::isConvertible( const std::vector<double>& inputColumn, const etrading::ContainedTypeEnum g )
    {
        return ( g == DOUBLE_VALUE ) ||  ( g == VARIANT_VALUE );
    }

    // Check if type 'bool' is convertible to contained type g
	template <>
    bool Variant::isConvertible( const std::vector<bool>& inputColumn, const etrading::ContainedTypeEnum g )
    {
        return ( g == BOOL_VALUE ) ||  ( g == VARIANT_VALUE );
    }

    // Check if type 'boost::gregorian::date' is convertible to contained type g
	template <>
    bool Variant::isConvertible( const std::vector<boost::gregorian::date>& inputColumn, const etrading::ContainedTypeEnum g )
    {
        return ( g == DATE_VALUE ) ||  ( g == VARIANT_VALUE );
    }

    // Check if type 'std::string' is convertible to contained type g
	template <>
    bool Variant::isConvertible( const std::vector<std::string>& inputColumn, const etrading::ContainedTypeEnum g )
    {
        return ( g == STRING_VALUE ) ||  ( g == VARIANT_VALUE );
    }

    // Check if type 'Variant' is convertible to contained type g
	template <>
    bool Variant::isConvertible( const std::vector<Variant>& inputColumn, const etrading::ContainedTypeEnum g )
    {
        if( g == VARIANT_VALUE )
        {
            return true;
        }
		return false;
    }

    // Check if a vector of type 'Variant' is convertible to contained type g
    template <>
    bool Variant::isConvertibleVector( const std::vector<Variant>& inputColumn, const etrading::ContainedTypeEnum g )
    {
        if( g == VARIANT_VALUE )
        {
            return true;
        }
        return ( std::find_if( inputColumn.cbegin(), inputColumn.cend(), [g] ( const Variant & cv )
        {
            return cv.getType() != g;
        } ) == inputColumn.cend() );
    }

	template <>
    std::vector<boost::gregorian::date> Variant::createValueVector( const std::vector<Variant>& inputVector )
    {
        std::vector<boost::gregorian::date> retVec;
        std::transform( inputVector.cbegin(), inputVector.cend(), std::back_inserter( retVec ), []( const Variant & v ) -> boost::gregorian::date
        {
			// Intel compiler requires help deciding which conversion to use.
			// Convert via (const boost::gregorian::date &)
			return static_cast<boost::gregorian::date>( (const boost::gregorian::date &) v );
        } );
        return retVec;
    }


    // this does not get called => global namespace has priority ? check Koenig lookup?
    std::ostream& operator<<( std::ostream& os, const Variant& cValue )
    {
        if( cValue.type_ == DATE_VALUE )
        {
            os <<  Variant::getDateString( cValue );
        }
        else if( cValue.type_ == INTEGER_VALUE )
        {
            os <<  cValue.getValue<int>();
        }
        else if( cValue.type_ == BOOL_VALUE )
        {
            os <<  cValue.getValue<bool>();
        }
        else if( cValue.type_ == DOUBLE_VALUE )
        {
            os <<  cValue.getValue<double>();
        }
        else if( cValue.type_ == STRING_VALUE )
        {
            os <<  cValue.getValue<std::string>();
        }
        return os;
    };

    /*static */  std::vector<etrading::ContainedTypeEnum> Variant::getContainedTypeInfo( const std::vector<std::vector<Variant>>& dataValues )
    {
        const int number_of_cols = dataValues.size();
        std::vector<etrading::ContainedTypeEnum> columnEnumTypes( number_of_cols, etrading::VARIANT_VALUE );
        
        for( int  j = 0; j < number_of_cols; ++j )
        {
			if ( dataValues[j].empty() )
			{ 
				// Set Empty Data to Empty Type
				columnEnumTypes[j] = etrading::EMPTY_VALUE;
			}
			else
			{
				// TODO: There is a problem here, Variant will never be reached if we only have 1 element, since allOfSameType = True
				etrading::ContainedTypeEnum firstEnum = dataValues[j][0].getType();
				auto citBegin = dataValues[ j ].cbegin();
				std::advance( citBegin, 1 );
				const bool allOfSameType = std::all_of( citBegin,
				                                        dataValues[ j ].cend(),
				                                        [&firstEnum] ( const etrading::Variant & cv ) -> bool {  return cv.getType() == firstEnum;  }    );
				columnEnumTypes[j] = allOfSameType ? firstEnum : etrading::VARIANT_VALUE;
			}
        }

        return std::move( columnEnumTypes );
    }

    /*static*/ VariantMatrix  Variant::transpose( const VariantMatrix& flexibleData, const Variant& defaultVariant )
    {
        VariantMatrix variantMatrix;
        // get the maximum number of rows
        unsigned int num_input_rows =   std::accumulate( flexibleData.cbegin(), flexibleData.cend(), -1,
                                                []( const int& prevMin, const std::vector<Variant>& columnData ) -> const int
        {
            const int dataSize = columnData.size();
            if( dataSize > 0 )
            {
                return ( dataSize > prevMin ) ? dataSize : prevMin;
            }
            else
            {
                return prevMin;
            }
        } );

        for( unsigned int inputRowCounter = 0; inputRowCounter < num_input_rows; inputRowCounter++ )
        {
            std::vector<Variant> newVariantColumn;
            for( unsigned int colCounter = 0; colCounter < flexibleData.size(); colCounter++ )
            {
                if( inputRowCounter < flexibleData[colCounter].size() )
                {
                    newVariantColumn.emplace_back(  flexibleData[ colCounter ][ inputRowCounter ] );
                }
                else
                {
                    newVariantColumn.emplace_back( defaultVariant );
                }
            }
            variantMatrix.emplace_back( newVariantColumn );
        }
        return variantMatrix;
    };

    const ContainedTypeEnum Variant::getType() const
    {
        return type_;
    };

    Variant::Variant() : type_( STRING_VALUE ), value_( std::string("") )
    {}; // default value needs to be supplied because we have std::vector members

    Variant::Variant( const char* inputValue )
    {
        // Set null / blank input, note default value is std::string("") with type STRING_VALUE
        if ( inputValue == nullptr || inputValue == std::string("") )
        {
            type_ = STRING_VALUE;
            value_ = std::string("");
            return;
        }

		const std::string valueAsString = inputValue;
        if( isValid<int>( valueAsString ) )
        {
            setValue( boost::lexical_cast<int>( valueAsString ) );
        }
        else if( isValid<double>( valueAsString ) )
        {
            setValue( boost::lexical_cast<double>( valueAsString ) );
        }
        else if( isValid<bool>( valueAsString ) )
        {
            setValue( boost::lexical_cast<bool>( valueAsString ) );
        }
        else
        {
            setValue( std::string( valueAsString ) );
        }
    }

    const bool Variant::isEmpty() const
    {
        return ( value_.empty() || this->toString() == std::string("") );
    };

    // http://stackoverflow.com/questions/24065769/boostspirithold-any-memory-corruption => do NOT use assignment
    // allow for assignment of contained values
    Variant& Variant::operator=( const Variant& rhs )
    {
        if( this != &rhs )
        {
            switch( rhs.type_ )
            {
                case INTEGER_VALUE:
                    setValue( rhs.getValue<int>() );
                    break;

                case BOOL_VALUE:
                    setValue( rhs.getValue<bool>() );
                    break;

                case DOUBLE_VALUE:
                    setValue( rhs.getValue<double>() );
                    break;

                case DATE_VALUE:
                    setValue( rhs.getValue<boost::gregorian::date>() );
                    break;

                case STRING_VALUE:
                    setValue( rhs.getValue<std::string>() );
                    break;

                case VARIANT_VALUE:
                    if( rhs.value_.empty()  && this->value_.empty() )
                    {
                        return *this; // two empty values being assigned to eachother
                    }
                    else if( rhs.value_.empty() )   // being assigned to an empty value
                    {
                        this->value_.reset();
                        this->type_ = VARIANT_VALUE;
                        return *this;
                    }
                    else
                    {
                        // convert to a cstr and use the CTOR to figure out what it is
                        Variant localVariant( rhs.toString().c_str() );
                        ( *this ) = localVariant; // cannot be VARIANT_VALUE again
                        return *this;
                    }
                    // throw ETradingException( "Existing Variant (of VARIANT type) cannot be assigned to a VARIANT type" );
                    break;

                default:
                    break;
            }
        }
        return *this;
    } ;

    Variant::Variant( const Variant& rhs ) : type_( rhs.type_ )
    {
        // because of memory corruption...
        ( *this ) = rhs;
    }

    std::string Variant::toString( bool addQuotesIfString ) const
    {
        if( type_ == DATE_VALUE )
        {
            return addQuotesIfString ? ( boost::format( "\"%s\"" ) % Variant::getDateString( *this ) ).str() : Variant::getDateString( *this );
        }
        else if( type_ == INTEGER_VALUE )
        {
            return boost::lexical_cast<std::string>( this->getValue<int>() );
        }
        else if( type_ == BOOL_VALUE )
        {
            return boost::lexical_cast<std::string>( this->getValue<bool>() );
        }
        else if( type_ == DOUBLE_VALUE )
        {
            return boost::lexical_cast<std::string>( this->getValue<double>() );
        }
        else if( type_ == STRING_VALUE )
        {
            return addQuotesIfString ? ( boost::format( "\"%s\"" ) % getValue<std::string>() ).str() : getValue<std::string>();
        }
        else if( type_ == VARIANT_VALUE )
        {
            if( value_.empty() )
            {
                return  addQuotesIfString ? (boost::format("\"%s\"") % std::string("") ).str() : "";
            }
            else
            {
                auto coreString = boost::lexical_cast<std::string>( value_ );
                return addQuotesIfString ? ( boost::format( "\"%s\"" ) % coreString ).str()  : coreString;
            }
        }
        throw ETradingException( "#Error: The toString method has not been implemented on Variant for this data type" );
    }

    Variant::operator double() const
    {
        if( type_ == INTEGER_VALUE )
        {
            return static_cast<double>( getValue<int>() );
        }
        else if( type_ == BOOL_VALUE )
        {
            return static_cast<double>( getValue<bool>() );
        }
        else if( type_ == DATE_VALUE )
        {
            return static_cast<double>(  toExcelDateFromGregorianDate( getValue<boost::gregorian::date>() ) );
        }
        else
        {
            return getValue<double>();
        }
    };

    Variant::operator int() const
    {
        if( type_ == DOUBLE_VALUE )
        {
            return static_cast<int>( getValue<double>() );
        }
        else if( type_ == BOOL_VALUE )
        {
            return static_cast<int>( getValue<bool>() );
        }
        else if( type_ == DATE_VALUE )
        {
            return toExcelDateFromGregorianDate( getValue<boost::gregorian::date>() );
        }
        else
        {
            return getValue<int>();
        }
    };

    Variant::operator std::string() const
    {
        return this->toString();
    };

    Variant::operator boost::gregorian::date() const
    {
        if( type_ == DOUBLE_VALUE )
        {
            return toGregorianDateFromExcelDate( static_cast<int>( getValue<double>() ) );
        }
        else if( type_ == INTEGER_VALUE )
        {
            return toGregorianDateFromExcelDate( getValue<int>() );
        }
        else if( type_ == STRING_VALUE )
        {
            std::string stringRepresentation = this->toString();
            if( couldBeDate( stringRepresentation ) )
            {
                return toGregorianDateFromREGEX( stringRepresentation );
            }
            else
            {
                return getValue<boost::gregorian::date>();    // will throw an exception
            }
        }
        else
        {
            return getValue<boost::gregorian::date>();
        }
    };

    Variant::operator LADate() const
    {
		// Intel compiler requires help deciding which conversion to use.
		// Convert via (const boost::gregorian::date &)
        auto gregorianDate = static_cast< boost::gregorian::date >( (const boost::gregorian::date &) *this );
        LADate mlibDate = toLADateFromGregorianDate( gregorianDate );
        return mlibDate;
    }

    Variant::operator LAString() const
    {
        auto stdString = this->toString();
        LAString mlibString = LAString( stdString.c_str() );
        return mlibString;
    }
   

    // ------------------------------------------------------------------
    // Thse Methods Compare Variant with other Variant types
    // ------------------------------------------------------------------


    bool Variant::operator==( const Variant& rhs ) const
    {
        // Check for mixed type comparisions
        // ---------------------------------
        // Mixed Types are not the same i.e. always false
        if ( type_ != rhs.getType() )
        {
            return false;
        }

        // Same Type comparisions: lhs and rhs types are the same
        // ------------------------------------------------------
        switch ( type_ )
        {
            case DATE_VALUE:
            {
                return LADate( this->getValueAsString().c_str() ) == LADate( rhs.getValueAsString().c_str() );
                break;
            }

            case INTEGER_VALUE:
                return getValue<int>() == rhs.getValue<int>();
                break;

            case DOUBLE_VALUE:
                return getValue<double>() == rhs.getValue<double>();
                break;

            case BOOL_VALUE:
                return getValue<bool>() == rhs.getValue<bool>();
                break;

            case STRING_VALUE:
                return getValue<std::string>() == rhs.getValue<std::string>();
                break;

            default:
                AQ_THROW( "Invalid variant type" );
                // We should never reach here
                return true;
                break;
        }
            
        // We should never reach here
        AQ_THROW("Variant '==' operator failure")
        return true;
    }


    bool Variant::operator<( const Variant& rhs ) const
    {
        // Check for mixed type comparisions
        // ---------------------------------
        if ( type_ != rhs.getType() )
        {
            // Convert mixed variant types to string via string stream for comparison
            return this->getValueAsString() < rhs.getValueAsString();
        }

        // Same Type comparisions: lhs and rhs types are the same
        // ------------------------------------------------------
        switch ( type_ )
        {
            case DATE_VALUE:
            {
                return LADate( this->getValueAsString().c_str() ) < LADate( rhs.getValueAsString().c_str() );
                break;
            }
            case INTEGER_VALUE:
                return getValue<int>() < rhs.getValue<int>();
                break;

            case DOUBLE_VALUE:
                return getValue<double>() < rhs.getValue<double>();
                break;

            case BOOL_VALUE:
                return getValue<bool>() < rhs.getValue<bool>();
                break;

            case STRING_VALUE:
                return getValue<std::string>() < rhs.getValue<std::string>();
                break;

            default:
                AQ_THROW( "Invalid variant type" );
                // We should never reach here
                return true;
                break;
        }
            
        // We should never reach here
        AQ_THROW("Variant '<' operator failure")
        return true;
    }


    bool Variant::operator>( const Variant& rhs ) const
    {
        // Check for mixed type comparisions
        // ---------------------------------
        if ( type_ != rhs.getType() )
        {
            // Convert mixed variant types to string via string stream for comparison
            return this->getValueAsString() > rhs.getValueAsString();
        }

        // Same Type comparisions: lhs and rhs types are the same
        // ------------------------------------------------------
        switch ( type_ )
        {
            case DATE_VALUE:
            {
                return LADate( this->getValueAsString().c_str() ) > LADate( rhs.getValueAsString().c_str() );
                break;
            }
            case INTEGER_VALUE:
                return getValue<int>() > rhs.getValue<int>();
                break;

            case DOUBLE_VALUE:
                return getValue<double>() > rhs.getValue<double>();
                break;

            case BOOL_VALUE:
                return getValue<bool>() > rhs.getValue<bool>();
                break;

            case STRING_VALUE:
                return getValue<std::string>() > rhs.getValue<std::string>();
                break;

            default:
                AQ_THROW( "Invalid variant type" );
                // We should never reach here
                return true;
                break;
        }
            
        // We should never reach here
        AQ_THROW("Variant '>' operator failure")
        return true;
    }

        
    bool Variant::operator<=( const Variant& rhs ) const
    {
        // Check for mixed type comparisions
        // ---------------------------------
        if ( type_ != rhs.getType() )
        {
            // Convert mixed variant types to string via string stream for comparison
            return this->getValueAsString() <= rhs.getValueAsString();
        }

        // Same Type comparisions: lhs and rhs types are the same
        // ------------------------------------------------------
        switch ( type_ )
        {
            case DATE_VALUE:
            {
                return LADate( this->getValueAsString().c_str() ) <= LADate( rhs.getValueAsString().c_str() );
                break;
            }
            case INTEGER_VALUE:
                return getValue<int>() <= rhs.getValue<int>();
                break;

            case DOUBLE_VALUE:
                return getValue<double>() <= rhs.getValue<double>();
                break;

            case BOOL_VALUE:
                return getValue<bool>() <= rhs.getValue<bool>();
                break;

            case STRING_VALUE:
                return getValue<std::string>() <= rhs.getValue<std::string>();
                break;

            default:
                AQ_THROW( "Invalid variant type" );
                // We should never reach here
                return true;
                break;
        }
            
        // We should never reach here
        AQ_THROW("Variant '<=' operator failure")
        return true;
    }


    bool Variant::operator>=( const Variant& rhs ) const
    {
        // Check for mixed type comparisions
        // ---------------------------------
        if ( type_ != rhs.getType() )
        {
            // Convert mixed variant types to string via string stream for comparison
            return this->getValueAsString() >= rhs.getValueAsString();
        }

        // Same Type comparisions: lhs and rhs types are the same
        // ------------------------------------------------------
        switch ( type_ )
        {
            case DATE_VALUE:
            {
                return LADate( this->getValueAsString().c_str() ) >= LADate( rhs.getValueAsString().c_str() );
                break;
            }
            case INTEGER_VALUE:
                return getValue<int>() >= rhs.getValue<int>();
                break;

            case DOUBLE_VALUE:
                return getValue<double>() >= rhs.getValue<double>();
                break;

            case BOOL_VALUE:
                return getValue<bool>() >= rhs.getValue<bool>();
                break;

            case STRING_VALUE:
                return getValue<std::string>() >= rhs.getValue<std::string>();
                break;

            default:
                AQ_THROW( "Invalid variant type" );
                // We should never reach here
                return true;
                break;
        }
            
        // We should never reach here
        AQ_THROW("Variant '>=' operator failure")
        return true;
    }
}