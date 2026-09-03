#pragma once


#include <string>
#include <boost/bimap.hpp>

namespace etrading
{
    // @BidirectionalMap: this class encapsulated a bimap to a string from a templated argument.
    // Basically allows linkage of instances of any type to a std::string instance
    template<typename E> class BidirectionalMap
    {
    private:
        typedef typename boost::bimap<E, std::string > BMEnumType;
        BMEnumType _biMap;
    public:
        BidirectionalMap( const BMEnumType& biMap ) : _biMap( biMap ) {};
        BidirectionalMap() {};
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
