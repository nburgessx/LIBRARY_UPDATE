//
// @File: ContainerUtilities.h
// @Description: This is a test program
// @Created: 27 Feb 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of AlgoQuantHub.

#pragma once

#include <iostream>
#include <iterator>
#include <algorithm>
#include <sstream>
#include <string>
#include <stdio.h>
#include <vector>
#include <functional>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/date_time.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>

#include "Variant.h"
#include "TypeName.h"

using etrading::Variant;
using etrading::TypeName;

namespace etrading
{
    template <typename T> T& popBackToSize(T& theData, unsigned int numCols);
    template <typename T> T& moldToSize(T& theData, unsigned int numCols, const typename T::value_type& defaultFill);

    // Function to trim a LWO object and remove the object counter if present
    std::string trimObjectCounter( const std::string& inputString );

    // removes all instances of p from s
    void removeAllSubstrings( std::string& s, const std::string& p );

	/* @brief			Returns a new string obtained by appending the "subString" onto the end of "original", using the "delim" delimiter.
	*                   Also checks to make sure that subString does not appear anywhere else inside the original string, and if necessary removes it from the output
	*
    *  @param [in]		original		the original string to use as the base
	*  @param [in]		subString		the substring to append
	*  @param [in]		delim			the delimiter to use:  e.g.  original : subString
    *  @param [out]		Returns a string constructed from  original:subString
    */
    std::string appendSubStringIfNotThere( const std::string& original, const std::string& subString, const std::string& delim = ":");


	/* @brief			Returns a new string obtained adding the "prefix" onto the beginning of "original", using the "delim" delimiter.
	*                   Also checks to make sure that "prefix" does not appear anywhere else inside the original string, and if necessary removes it from the output
	*
    *  @param [in]		original		the original string to use as the base
	*  @param [in]		prefix			the string to use as the prefix
	*  @param [in]		delim			the delimiter to use:  e.g.  prefix : original
    *  @param [out]		Returns a string constructed from "prefix:original"
    */
	std::string addPrefixStringAndCheckForDuplicates( const std::string& original, const std::string& prefix, const std::string& delim = ":" );


    void dropLast( std::string& s, const std::string& p );

    // takes a vector of ints and transforms it to a vector of doubles
    // by multiplying each element with 'value' and adding 'start' to it
    std::vector<double> scaleVector(
        const std::vector<int>& data,
        const double value,
        const double start );

    // creates a increasing vector of length 'numElements' and starting at 'start' with increment of 'stepSize'
    std::vector<double> stepVector( const double start, const double stepSize, const int numElements );

    // displays STL container elements to std::cout with the seperator equal to sep
    template <typename Container>
    void displayContainer( const Container& container, const char* sep = "," )
    {
        typedef typename Container::value_type Element;
        std::copy( container.cbegin(),
                   container.cend(),
                   std::ostream_iterator<Element>( std::cout, sep ) );
        std::cout << std::endl;
    };

    // captures the STL container elements to a std::string provided overload on ostream <<
    template <typename Container>
    std::string containerAsString( const Container& container, const char* sep = "," )
    {
        typedef typename Container::value_type Element;
        std::stringstream ss;
        std::copy(	container.cbegin(),
                    container.cend(),
                    std::ostream_iterator<Element>( ss, sep ) );
        std::string tempStr = ss.str();
        return tempStr.substr( 0, tempStr.size() - strlen( sep ) );
    };

    template <typename Container>
    bool is_unique( const Container& vec )
    {
        if( is_container<Container>::value )
        {
            Container sortableCopy;
            boost::push_back( sortableCopy, vec );
            std::sort( sortableCopy.begin(), sortableCopy.end() );
            return std::unique( sortableCopy.begin(), sortableCopy.end() ) == sortableCopy.end();
        }
        else
        {
            return true;
        }
    }

    // TODO: generalize this to a custom_zip_function with a 3rd parameter (sd::function<std::vector<ReturnType>(C1::value_type....)> so lambdas can be passed)
    template<typename Container1, typename Container2, typename ReturnType>
    std::vector<ReturnType> zip_paste(  const Container1& c1, const Container2& c2 )
    {
        if( !is_container<Container1>::value || !is_container<Container2>::value )
            throw ETradingException( ( boost::format( "One of the following types is not a Container (custom_for_each): %s %s" )
                                       % TypeName::get<Container1>()
                                       % TypeName::get<Container2>() ).str() );

        std::vector<ReturnType> retValues;
        auto i1 = c1.cbegin();
        auto i2 = c2.cbegin();
        for ( ; ( i1 != c1.cend() ) && ( i2 != c2.cend() ); ++i1, ++i2 )
        {
            retValues.emplace_back(   boost::lexical_cast<ReturnType>( *i1 ) + boost::lexical_cast<ReturnType>( *i2 )  );
        }
        return retValues;
    };

    //TODO: implement a getIndexOf with a supplied std::function
    template <typename T, typename Container>
    int getIndexOf( const T& value, const Container& container )
    {
        if( !container.empty() )
        {
            auto cit = std::find( container.begin(), container.end(), value );
            return  ( cit != container.end() ? cit - container.begin() : -1 );
        }
        else
        {
            return -1;
        }
    };

    // this assumes an increasing vector!
    int getIndexWithPrecision( const std::vector<double>& searchVector, const double searchValue, const double precision = 1E-10 );

    int idxOfRegexMatch( const std::string& inString, const std::vector<boost::regex>& regexVectorToMatch );

    template <typename Container>
    Container getDisplayableContainer( const Container& container, const unsigned int maxSize = 10 )
    {
        if( container.size() > maxSize )
        {
            auto first = container.cbegin();
            return Container( first, first + maxSize );
        }
        else
        {
            return container;
        }
    }

	std::vector<double> createDoubleVector(const LAStringVector&);
	std::vector<std::string> createStringVector(const LAStringVector&);
	std::vector<boost::gregorian::date> createDateVector(const LAStringVector&);


}


/* TODO:  C++11 helper functions for bimaps

		template <typename L, typename R> boost::bimap<L, R> makeBiMap(std::initializer_list<typename boost::bimap<L, R>::value_type> list)
		{			return boost::bimap<L, R>(list.begin(), list.end());		}

		// only insert uppercase strings for bimaps...
		template <typename L> boost::bimap<L, std::string> makeBiMap(std::initializer_list<typename boost::bimap<L, const char*>::value_type> list)
		{
			boost::bimap<L, std::string> localbm;
			typedef typename boost::bimap<L, std::string>::value_type BiMapValueType;
			for ( auto it=list.cbegin(); it!=list.cend(); ++it) {
				std::string tempString(it->right);
				std::transform(tempString.begin(), tempString.end(),tempString.begin(), std::toupper);
				localbm.insert( BiMapValueType(it->left,tempString)   );
			}
			return localbm;
		}

// TODO: in C++11 template for array sizes at compile time
		template <typename T, std::size_t N>
		constexpr std::size_t arraySize(T&[N]) noexcept
		{
			return N;
		};
*/


