//
// @Description: This is a test program


#include "EnvironmentPool.h"
#include "Environment.h"


namespace etrading
{
    EnvironmentPool::EnvironmentPool() : _ctxts( )
    {
        // std::cout << "EnvironmentPool() called" << std::endl;
    };

    std::shared_ptr<Environment> EnvironmentPool::getEnvironment( const std::string& uid )  const
    {
        boost::shared_lock<boost::shared_mutex> lock( schemaAccess );
        auto it = getEnvironmentIterator( uid );
        if( it == _ctxts.end() )
        {
			return {};
        }
        else
        {
            return *it;
        }
    };

    bool EnvironmentPool::isPresent( const std::string& uid ) const
    {
        if( _ctxts.empty() )
        {
            return false;
        }
        boost::shared_lock<boost::shared_mutex> lock( schemaAccess );
        return ( getEnvironmentIterator( uid ) != _ctxts.end() );
    };

    std::string EnvironmentPool::getDefaultEnvironmentName() const
    {
#if defined(_WIN32) || defined(_WIN64)
        std::string DEFAULT_ENV_NAME = etrading::getUserName() + "@" + etrading::getEnvironmentVariable( "COMPUTERNAME" );
        return DEFAULT_ENV_NAME;
#else
        std::string DEFAULT_ENV_NAME = etrading::getUserName() + "@" + etrading::getEnvironmentVariable( "HOSTNAME" );
        return DEFAULT_ENV_NAME;
#endif
    }

    bool EnvironmentPool::createEnvironment( const std::string& uid )
    {
        if( !isPresent( uid ) )
        {
            boost::upgrade_lock<boost::shared_mutex> schemaLock( schemaAccess );
            boost::upgrade_to_unique_lock<boost::shared_mutex> schemaUniqueLock( schemaLock );
            // cannot be done with std::make_shared<> because the CTOR is private for encapsulation
            _ctxts.emplace_back( std::shared_ptr<Environment>( new Environment( uid ) ) );
            return true;
        }
        else
        {
            return false;
        }
    };

    void EnvironmentPool::resetEnvironment()
    {
        // Create a Lock Scope
        {
            boost::upgrade_lock<boost::shared_mutex> schemaLock( schemaAccess );
            boost::upgrade_to_unique_lock<boost::shared_mutex> schemaUniqueLock( schemaLock );
         
            // Clear Existing Environments / Contexts
             _ctxts.clear();
        }
        
        // Restore the Default Environment (required)
        std::string DEFAULT_ENV_NAME = getDefaultEnvironmentName();
        etrading::EnvironmentPool::getInstance().createEnvironment( DEFAULT_ENV_NAME );
    }

    std::vector<std::shared_ptr<Environment>>::const_iterator
                                           EnvironmentPool::getEnvironmentIterator( const std::string& uid )  const
    {
        if( _ctxts.empty() )
        {
            AQ_THROW( "EnvironmentPool is empty - cannot retrieve any Environment" );
        }
        return std::find_if(	_ctxts.begin(), _ctxts.end(),
                                [&uid]( const std::shared_ptr<Environment>& toCheck ) -> bool
        {
            if( toCheck != nullptr )
            {
                return ( toCheck.get()->getUID() == uid );
            }
            else
            { return false; }
        } );
    };

}