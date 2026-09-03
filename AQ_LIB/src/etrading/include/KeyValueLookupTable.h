#pragma once

#include <string>
#include <unordered_map>
#include <sstream>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>   // For boost::to_upper and boost:iequals i.e. case insensitve string comparison

#include "LACoreTemplateType.h"
#include "ETradingException.h"

namespace etrading
{
    /* @brief	Template class that handles key-value pairs. Keys are always considered of string type.
    */
    template<typename T>
    class KeyValueLookupTable
    {
    public:
        KeyValueLookupTable( const std::unordered_map<std::string, T>& rhs ) : objectMap_( rhs ) {};
        KeyValueLookupTable() {};

        /* @brief	Return the keys of the the named collection
        *  @return	keys
        */
        std::set<std::string> getKeysAsSet() const
        {
            std::set<std::string> keys;
            for( auto itr = objectMap_.begin(); itr != objectMap_.end(); ++itr )
            {
                keys.insert( itr->first );
            }
            return keys;
        }

        /* @brief	Return the keys of the the named collection
        *  @return	keys
        */
        std::vector<std::string> getKeys() const
        {
            std::vector<std::string> keys;
            for( auto itr = objectMap_.begin(); itr != objectMap_.end(); ++itr )
            {
                keys.push_back( itr->first );
            }
            return keys;
        }

        /* @brief	Return the values of the the named collection
        *  @return	values
        */
        std::vector<T> getValues() const
        {
            std::vector<T> values;
            for( auto itr = objectMap_.begin(); itr != objectMap_.end(); ++itr )
            {
                values.push_back( itr->second );
            }
            return values;
        }

        /* @brief	Return the size of the named collection
        *  @return	Size of the named collection
        */
        size_t size() const
        {
            return objectMap_.size();
        }

        /* @brief	Return the value of an optional key.
        *  @param [in]	key			Key for look up
        *  @param [in]	defaultVal	Default value of the key
        *  @return		Value of the look up key
        */
        T getOptionalValue( const std::string& key, const T& defaultVal, const bool makeKeyUppercase = true ) const
        {
            std::string keyUsed = key;
            if ( makeKeyUppercase )
            {
                boost::to_upper( keyUsed );
            }

            auto iter = objectMap_.find( keyUsed );
            if ( iter == objectMap_.end() )
            {
                return defaultVal;
            }

            T ret = iter->second;
            return ret;
        }

        /* @brief	Return the value of a compulsory key.
        *  @param [in]	key				            Key for look up
        *  @param [in]	labelValueBlockName	        Name of the Label Value Block
        *  @return		Value of the look up key
        */
        T getCompulsoryValue( const std::string& key, const std::string& labelValueBlockName = "", const bool makeKeyUppercase = true ) const
        {
            std::string keyUsed = key;
            if ( makeKeyUppercase )
            {
                boost::to_upper( keyUsed );
            }

            auto iter = objectMap_.find( keyUsed );
            if ( iter == objectMap_.end() )
            {
                std::stringstream s;
                s << "#Error: Missing key '" << keyUsed << "'. This key is mandatory and must be included in the Label Value Block ";
                
                // Append the Label Value Block name if known
                if ( labelValueBlockName != "" )
                {
                    s << ", '" + labelValueBlockName + "'.";
                }

                throw LACoreInvalidData( s.str().c_str(), __FILE__, __LINE__ );
            }

            T ret = iter->second;
            return ret;
        }

        bool isEqual( std::unordered_map<std::string, T> const &rhs ) const
        {
            // No predicate needed because there is operator== for pairs already.
            return objectMap_.size() == rhs.size() && std::equal( objectMap_.begin(), objectMap_.end(), rhs.begin() );
        }

    protected:

        /* @brief	Add a key-value pair to the collection
        *  @param [in]	key		    Key to add
        *  @param [in]	value	    Value to add
        *  @param [in]	allowUpdate	True to allow updating a key's value
        */
        void add( const std::string& key, const T& value, bool allowUpdate=true )
        {
            if (objectMap_.find(key) == objectMap_.end() || allowUpdate)
            {
                objectMap_[key] = value;
            }
            else
            {
                throw ETradingException(  ( boost::format( "#Error: LVB has duplicated key '%s'" ) % key ).str()  );
            }
        }

        // The key-value pair collection or object map
        std::unordered_map<std::string, T> objectMap_;

	};


}


