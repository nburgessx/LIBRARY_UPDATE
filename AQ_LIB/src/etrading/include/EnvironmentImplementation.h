#pragma once

#include <memory>
#include "AQObjCurve.h"


namespace etrading
{
    class Environment;

    // this is a namespace that is used for implementation on the environment



    namespace environment_implementation
    {
        // PART A.
        // these settings allow for determining how to copy an underlying pointer in the Cache
        // Usage: copyObject(myInstance) 
        // where myInstance is an instance of MyClass
        // If MyClass has a clone method it will get called else its copy CTOR is used to produce a shared_ptr to a copy
        // cfr. TEST (TestAQObjInfrastructure, EnvironmentImplementation)
        template<typename T>
        struct has_clone
        {
        private:
            typedef char yes;
            typedef struct	
		    { 
			    char array[2];
            } no;

            template<typename C, std::shared_ptr<C> (C::*)() const> struct SFINAE {};
            template<typename C> static yes test( SFINAE<C,&C::clone>* );
            template<typename C> static no  test( ... );
        public:

            static const bool value = sizeof( test<T>( 0 ) ) == sizeof( yes );
        };	
	
        // C++ does not allow partial specialization of functions, hence we need to wrap this in a struct/class
        template < typename T, bool = false>
        struct GetCopyImpl
        {
            std::shared_ptr<T> operator()( const T& instance ) const
            {
                return  std::make_shared<T>( instance );
            }
        };

        template < typename T>
        struct GetCopyImpl<T, true>
        {
            // the T object has a clone method we need to use
            std::shared_ptr<T> operator()( const T& instance ) const
            {
                return instance.clone();
            }
        };

        // PART B.
        // these settings allow for the dynamic execution of different functions through
        // the use of std::string keys
        // these templates/ methods should not be accessed directly (that is the whole point of creating this in the first place)

        // TODO: in C++11 make this a template typedef 
        typedef std::tuple <
				bool ( * ) ( const std::string&, Environment& ),  
				std::vector<std::string> ( * )( Environment& ),
				bool ( * ) ( const std::string&, Environment& ),
				const unsigned int ( * ) ( Environment& ),
				std::shared_ptr<IsAQObject> (*)  ( const std::string&, Environment& )> CachedObjectFunctionSet; 


		// has<T>, keys<T> and deleteObject<T> and accessObjectInterface<T> each associated with a CachedObjectEnum
        template<typename T>
        bool has( const std::string& objectName, Environment& env );

        template<typename T>
        std::vector<std::string> keys( Environment& env );

        template<typename T>
        bool eraseObject( const std::string& objectName, Environment& env );

        template<>
        bool eraseObject<AQObjCurve>( const std::string& objectName, Environment& env ); // needs to be in cpp else instantiation before Environment class defined

        template<typename T>
        const unsigned int eraseAllObjects( Environment& env );

        template<>
        const unsigned int eraseAllObjects<AQObjCurve>( Environment& env );

	template<typename T>
        std::shared_ptr<IsAQObject> accessObjectInterface( const std::string& objectName,  Environment& env );


    }





}
