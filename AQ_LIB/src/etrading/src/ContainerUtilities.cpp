#include <algorithm>
#include <limits>
#include <utility>
#include <iterator>
#include <boost/algorithm/string.hpp>
#include <boost/range/irange.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/numeric.hpp>
#include <boost/algorithm/string/split.hpp>

#include "ContainerUtilities.h"
#include "DateUtilities.h"
#include "Variant.h"

namespace etrading
{

    // Function to trim a LWO object and remove the object counter if present
    std::string trimObjectCounter( const std::string& inputString )
    {
        std::string result = inputString;
        
        // Remove everything after and including the SEMI-COLON object counter delimiter token
        std::string::size_type pos = inputString.find(':');
        if (pos != std::string::npos)
        {
            result = inputString.substr(0, pos);
        }
        
        result = trim_to_upper( result.c_str() );
        return result;
    }


    void removeAllSubstrings( std::string& s, const std::string& p )
    {
        std::string::size_type n = p.length();
        for ( std::string::size_type idxInString = s.find( p ); idxInString != std::string::npos; idxInString = s.find( p ) )
        {
            s.erase( idxInString, n );
        }
    };

	/* @brief			Returns a new string obtained by appending the "subString" onto the end of "original", using the "delim" delimiter.
	*                   Also checks to make sure that subString does not appear anywhere else inside the original string, and if necessary removes it from the output
	*
    *  @param [in]		original		the original string to use as the base
	*  @param [in]		subString		the substring to append
	*  @param [in]		delim			the delimiter to use:  e.g.  original : subString
    *  @param [out]		Returns a string constructed from  original:subString
    */
    std::string appendSubStringIfNotThere( const std::string& original, const std::string& subString, const std::string& delim )
    {
		std::string outputString;

		std::vector<std::string> tokens;
		boost::algorithm::split( tokens, original, boost::algorithm::is_any_of( delim ) );

		for ( auto it = tokens.begin(); it != tokens.end(); ++it )
		{
			if ( *it != subString)
			{
				// Include the token since it is not equal to the subString
				outputString += *it + delim;
			}
		}
		
		// Append the subString to the output
		outputString += subString;
				
		return outputString;
    };

	/* @brief			Returns a new string obtained adding the "prefix" onto the beginning of "original", using the "delim" delimiter.
	*                   Also checks to make sure that "prefix" does not appear anywhere else inside the original string, and if necessary removes it from the output
	*
	*  @param [in]		original		the original string to use as the base
	*  @param [in]		prefix			the string to use as the prefix
	*  @param [in]		delim			the delimiter to use:  e.g.  prefix : original
	*  @param [out]		Returns a string constructed as "prefix:original"
	*/
	std::string addPrefixStringAndCheckForDuplicates( const std::string& original, const std::string& prefix, const std::string& delim )
	{
		if( original.empty() )
		{
			// Return just the prefix if the original string is empty
			return prefix;
		}
		
		// Prefix the output with the subString
		std::string outputString = prefix;

		std::vector<std::string> tokens;
		boost::algorithm::split( tokens, original, boost::algorithm::is_any_of( delim ) );

		for ( auto it = tokens.begin(); it != tokens.end(); ++it )
		{
			if ( *it != prefix )
			{
				// Include the token since it is not equal to the subString
				outputString += delim + *it;
			}
		}

		return outputString;
	}

    void dropLast( std::string& s, const std::string& p )
    {
        std::string::size_type idxInString = s.find_last_of( p );
        if( idxInString != std::string::npos )
        {
            s.erase( idxInString, s.length() );
        }
    };

    std::vector<double> scaleVector(
        const std::vector<int>& data,
        const double value,
        const double start )
    {
        std::vector<double> retVector;
        std::for_each( data.cbegin(), data.cend(),
                       [&retVector, &value, &start]( const int input )
        {
            retVector.emplace_back( input * value + start );
        }
                     );
        return retVector;
    };

    std::vector<double> stepVector( const double start, const double stepSize, const int numElements )
    {
        const auto intIdxs( boost::copy_range<std::vector<int>>( boost::irange( 0, numElements + 1, 1 ) ) );
        return scaleVector( intIdxs, stepSize, start );
    };

    int getIndexWithPrecision( const std::vector<double>& searchVector, const double searchValue, const double precision )
    {
        auto it = std::lower_bound( searchVector.cbegin(), searchVector.cend(), searchValue );
        if( it == searchVector.cbegin() )
        {
            return 0;
        }
        else if( it == searchVector.cend() )
        {
            return ( searchVector.size() - 1 );
        }
        else
        {
            auto idxOfFirstLargerOrEqual = std::distance( searchVector.cbegin(), it );
            const bool matchesIdx = ( std::abs( *it - searchValue ) < precision );
            // assumes left continuous behaviour
            return  ( !matchesIdx ) ? idxOfFirstLargerOrEqual - 1 : idxOfFirstLargerOrEqual;
        }
    };


    int idxOfRegexMatch( const std::string& inString, const std::vector<boost::regex>& regexVectorToMatch )
    {
        boost::cmatch matches;
        int idx = -1;
        for( size_t regexCounter = 0 ; regexCounter < regexVectorToMatch.size(); regexCounter++ )
        {
            if( boost::regex_match( inString.c_str(), matches, regexVectorToMatch[ regexCounter ] ) )
            {
                idx =  regexCounter;
                break;
            }
        }
        return idx;
    };

	std::vector<double> createDoubleVector(const LAStringVector& inputVector)
	{
		std::vector<double> dblVec(inputVector.size());
		for(auto i = 0u; i < inputVector.size(); i++)
		{
			dblVec[i] = inputVector[i].getDoubleValue();
		}
		return dblVec;
	};

	std::vector<std::string> createStringVector(const LAStringVector& inputVector)
	{
		std::vector<std::string> strVec(inputVector.size());
		for(auto i = 0u; i < inputVector.size(); i++)
		{
			strVec[i] = inputVector[i].getCString();
		}
		return strVec;
	};

	std::vector<boost::gregorian::date> createDateVector(const LAStringVector& inputVector)
	{
		std::vector<boost::gregorian::date> dateVec(inputVector.size());
		for(auto i = 0u; i < inputVector.size(); i++)
		{
			dateVec[i] = etrading::toGregorianDateFromREGEX( std::string(inputVector[i].getCString()));
		}
		return dateVec;
	};

   template <typename T>
    T& popBackToSize(T& theData, unsigned int numCols)
    {
        while(theData.size() > numCols)
        {
            theData.pop_back();
        }
        return theData;
    };

    template std::vector<etrading::Variant>& popBackToSize(std::vector<etrading::Variant>& theData, unsigned int numCols);
    template std::vector<std::vector<etrading::Variant>>& popBackToSize(std::vector<std::vector<etrading::Variant>>& theData, unsigned int numCols);

    template <typename T> T& moldToSize(T& theData, unsigned int numCols, const typename T::value_type& defaultFill)
    {
        while(theData.size() > numCols)
        {
            theData.pop_back();
        }
        while(theData.size() < numCols)
        {
            theData.push_back(defaultFill);
        }
        return theData;
    }

    template std::vector<etrading::Variant>& moldToSize(
        std::vector<etrading::Variant>& theData, 
        unsigned int numCols, 
        const std::vector<etrading::Variant>::value_type& defaultFill);

    template std::vector<std::vector<etrading::Variant>>& moldToSize(
        std::vector<std::vector<etrading::Variant>>& theData, 
        unsigned int numCols,
        const std::vector<std::vector<etrading::Variant>>::value_type& defaultFill);


}