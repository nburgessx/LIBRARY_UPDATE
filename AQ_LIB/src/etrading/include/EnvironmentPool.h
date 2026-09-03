#pragma once
//
// @File: EnvironmentPool.h
// @Description: This is a test program
// @Created: 27 Feb 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of Mizuho International.


#include <memory>
#include <boost/thread.hpp>

#include "Singleton.h"

namespace etrading
{
    class Environment;

    // @EnvironmentPool : a singleton that holds all the different contexts of every client
    class EnvironmentPool : public etrading::Singleton<EnvironmentPool>
    {
    public:
        std::shared_ptr<Environment> getEnvironment( const std::string& uid )  const;
        bool createEnvironment( const std::string& uid );
        void resetEnvironment();
        bool isPresent( const std::string& uid ) const;
    
        std::string getDefaultEnvironmentName() const;
    
    private:
        EnvironmentPool();
        friend Singleton<EnvironmentPool>;

        std::vector< std::shared_ptr<Environment > > _ctxts;
        mutable boost::shared_mutex schemaAccess;

        std::vector<std::shared_ptr<Environment>>::const_iterator
                                               getEnvironmentIterator( const std::string& uid )  const;
        // we don't expect a crazy amount of work context per pool so std::vector is probably efficient enough
    };

}
