#pragma once

//
// @File: NamesBiMap.h
// @Description: This is a test program
// @Created: 27 Feb 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of AlgoQuantHub.




#include <string>
#include <boost/bimap.hpp>

namespace etrading
{
    // @BidirectionalMap: this class encapsulated a bimap to a string from a templated argument.
    // Basically allows linkage of instances of any type to a std::string instance
    template<typename E> class NamesBiMap
    {
    private:
        typedef typename boost::bimap<E, std::string > BMEnumType;
        BMEnumType _biMap;
    public:
        NamesBiMap( const BMEnumType& biMap ) : _biMap( biMap ) {};
        NamesBiMap() {};
        const std::string& getString( const E& e ) const
        {
            return this->_biMap.left.at( e );
        }
        const E& getValue( const std::string& s ) const
        {
            return this->_biMap.right.at( s );
        }
        const int length() const
        {
            return _biMap.left.size();
        };
    private:
    };

}
