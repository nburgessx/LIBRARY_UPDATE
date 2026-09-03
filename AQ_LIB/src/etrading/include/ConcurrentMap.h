
#pragma once
//
// @File: ConcurrentMap.h
// @Description: this is a C++ implementation of a Concurrent Hashmap; note that it is using mutexes so you are paying up in speed for thread safety
// @Created: 27 Feb 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of AlgoQuantHub.




#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/range/adaptor/map.hpp>


#include <map>
#include <vector>

// boost::copy(m | boost::adaptors::map_keys, std::back_inserter(keys));

// TODO: Look at using the Junction project for some serious applications
// http://preshing.com/20160201/new-concurrent-hash-maps-for-cpp/
// https://github.com/preshing/junction


namespace etrading
{

    // @ConcurrentMap: this is a C++ implementation of a Concurrent Hashmap; note that it is using mutexes so you are paying up in speed for thread safety
    // TODO: figure out a lock-free implementation of a Hashmap
    template<typename Key, typename Value>
    class ConcurrentMap
    {
        typedef Value( *mutate_func )( Value );

    public:
        ConcurrentMap() {};
        bool has( Key k ) const
        {
            boost::shared_lock<boost::shared_mutex> lock( schemaAccess );
            return m.find( k ) != m.end();
        }

        unsigned int size() const
        {
            boost::shared_lock<boost::shared_mutex> lock( schemaAccess );
            return m.size();
        }

        void erase( Key k )
        {
            boost::upgrade_lock<boost::shared_mutex> schemaLock( schemaAccess );
            boost::upgrade_to_unique_lock<boost::shared_mutex> schemaUniqueLock( schemaLock );
            m.erase( k );
            valueAccess.erase( k );
        };

        void clear()
        {
            boost::upgrade_lock<boost::shared_mutex> schemaLock( schemaAccess );
            boost::upgrade_to_unique_lock<boost::shared_mutex> schemaUniqueLock( schemaLock );
            m.clear();
            valueAccess.clear();
        };

        std::vector<Key> keys() const
        {
            boost::shared_lock<boost::shared_mutex> lock( schemaAccess );
            std::vector<Key> keys;
            boost::copy( m | boost::adaptors::map_keys, std::back_inserter( keys ) );
            return keys;
        }


        void set( Key k, Value v )
        {
            boost::shared_lock<boost::shared_mutex> lock( schemaAccess );

            // set k, v
            if( m.find( k ) != m.end() )
            {
                boost::upgrade_lock<boost::shared_mutex> valueLock( *valueAccess[k] );
                boost::upgrade_to_unique_lock<boost::shared_mutex> valueUniqueLock( valueLock );
                m.at( k ) = v;
            }
            // insert k, v
            else
            {
                lock.unlock();
                boost::upgrade_lock<boost::shared_mutex> schemaLock( schemaAccess );
                boost::upgrade_to_unique_lock<boost::shared_mutex> schemaUniqueLock( schemaLock );

                boost::shared_ptr<boost::shared_mutex> mtx = boost::make_shared<boost::shared_mutex>();
                valueAccess.insert( std::pair<Key, boost::shared_ptr<boost::shared_mutex> >( k, mtx ) );
                m.insert( std::pair<Key, Value>( k, v ) );
            }
        }

        /*
         * mutate()
         *
         * IN: key to find record
         *     default value if record doesn't exist
         *     mutate function to change old to new if exist
         *
         * OUT: N/A
         *
         * This method looks for the map record to determine whether it exists
         * if it does, it calls the mutate_func with the value as a parameter
         * and takes it's return as the new value.
         * If it doesn't, then the value is initialized to v.
         */
        void mutate( Key k, Value v, mutate_func mf )
        {
            boost::shared_lock<boost::shared_mutex> lock( schemaAccess );

            // set k, v
            // TODO: Use an iterator. Changes 3 lookups into 1
            if( m.find( k ) != m.end() )
            {
                boost::upgrade_lock<boost::shared_mutex> valueLock( *valueAccess[k] );
                boost::upgrade_to_unique_lock<boost::shared_mutex> valueUniqueLock( valueLock );
                // if found, pass the current value into the mutate function
                m.at( k ) = mf( m.at( k ) );
            }
            // insert k, v
            else
            {
                lock.unlock();
                boost::upgrade_lock<boost::shared_mutex> schemaLock( schemaAccess );
                boost::upgrade_to_unique_lock<boost::shared_mutex> schemaUniqueLock( schemaLock );

                boost::shared_ptr<boost::shared_mutex> mtx = boost::make_shared<boost::shared_mutex>();
                valueAccess.insert( std::pair<Key, boost::shared_ptr<boost::shared_mutex> >( k, mtx ) );
                m.insert( std::pair<Key, Value>( k, v ) );
            }
        }

        Value get( Key k ) const
        {
            boost::shared_lock<boost::shared_mutex> lock( schemaAccess );
            return m.at( k );
        }


        void insert( Key k, Value v )
        {
            boost::upgrade_lock<boost::shared_mutex> schemaLock( schemaAccess );
            boost::upgrade_to_unique_lock<boost::shared_mutex> schemaUniqueLock( schemaLock );

            boost::shared_ptr<boost::shared_mutex> mtx = boost::make_shared<boost::shared_mutex>();
            valueAccess.insert( std::pair<Key, boost::shared_ptr<boost::shared_mutex> >( k, mtx ) );
            m.insert( std::pair<Key, Value>( k, v ) );
        }

        /*
         * Iterators are thread unsafe. (and access the internal map..)
         * I'm not even locking the map at this time...
         * Also, only the const iterator is provided for that reason.
         */
        typename std::map<Key, Value>::const_iterator unsafe_begin()
        {
            return m.begin();
        }

        typename std::map<Key, Value>::const_iterator unsafe_end()
        {
            return m.end();
        }

    private:
        std::map<Key, Value> m;
        std::map<Key, boost::shared_ptr<boost::shared_mutex> > valueAccess;
        mutable boost::shared_mutex schemaAccess;
        // e.g. has, size and keys are const functions and not allowed to change schemaAccess hence mutable required

        ConcurrentMap( const ConcurrentMap<Key, Value>& )
        {
            throw ETradingException( "ConcurrentMap Copy CTOR should never be called" ); // TODO: C++11 =delete
        }

    };

}