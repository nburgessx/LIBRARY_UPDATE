#pragma once

//
// @File: TypeName.h
// @Description: This is a test program
// @Created: 27 Feb 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of AlgoQuantHub.



//#ifdef _MSC_VER
//#include <typeinfo.h>   // for those among us with a 32bit inclination
//#endif

#include <typeinfo>
#include <string>
#include <list>
#include <vector>
#include <iostream>
#include <map>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/format.hpp>
#include <boost/utility/enable_if.hpp>


namespace etrading
{

    // this template checks whether an argument has const_iterator member
    // e.g. "has_const_iterator<int>::value" will be false "has_const_iterator<std::vector<int>>::value" will be true
    template<typename T>
    struct has_const_iterator
    {
    private:
        typedef char                      yes;
        typedef struct
        {
            char array[2];
        } no;

        template<typename C> static yes test( typename C::const_iterator* );
        template<typename C> static no  test( ... );
    public:
        static const bool value = sizeof( test<T>( 0 ) ) == sizeof( yes );
        typedef T type;
    };

    // this template checks whether an argument has iterator member
    // e.g. "has_iterator<int>::value" will be false "has_iterator<std::vector<int>>::value" will be true
    template <typename T>
    struct has_iterator
    {
        template <typename U>
        static char test( typename U::iterator* x );
        template <typename U>
        static long test( U* x );
        static const bool value = sizeof( test<T>( 0 ) ) == 1;
    };

    template<typename T>
    struct is_container : std::integral_constant < bool, has_const_iterator<T>::value&&
        has_iterator<T>::value >
    { };

    // returns whether the supplied type can be considered a vector of types handled by Variant
    template<typename T>
    struct is_etrading_vector
    {
        static const bool value = false;
    };
    // template specializations for the implementation of the above
    template<>
    struct is_etrading_vector<std::vector<int>>
    {
        static const bool value = true;
    };
    template<>
    struct is_etrading_vector<std::vector<double>>
    {
        static const bool value = true;
    };
    template<>
    struct is_etrading_vector<std::vector<std::string>>
    {
        static const bool value = true;
    };
    template<>
    struct is_etrading_vector<std::vector<boost::gregorian::date>>
    {
        static const bool value = true;
    };
    class Variant; // forward declaration to avoid circular dependency
    template<>
    struct is_etrading_vector<std::vector<Variant>>
    {
        static const bool value = true;
    };

    // checks whether a class can be considered a container of values handled by Contained Value (is overkill): only need is_etrading_vector<T>::value so far
    template<typename T>
    struct is_etrading_container : std::integral_constant < bool, has_const_iterator<T>::value&&
        has_iterator<T>::value&&
        is_etrading_vector<T>::value >
    { };

    //  @TypeName a struct returning the name of the templated type as a std::string. Note that this is self recursive.
    struct TypeName
    {
        template <typename T>
        static typename std::enable_if < !is_container<T>::value, std::string >::type get()
        {
            return typeid( T ).name();
        }

        // a specialization for container types for vector and list we keep the display simple
        template <typename T >
        static typename std::enable_if<is_container<T>::value, std::string>::type get()
        {
            typedef typename T::value_type ElementType;
            std::string containerType( "" );
            if ( boost::is_same<std::vector<ElementType>, T>::value )
            {
                containerType = "(Vector) ";
            }
            if ( boost::is_same<std::list<ElementType>, T>::value )
            {
                containerType = "(List) ";
            }
            return ( boost::format( "Container %s of type %s" ) % containerType % get<ElementType>() ).str();
        }

    };
	
	// a specialization of std::string (which is a container of type std::basic_string) to keep the display simple
    template <>
    typename std::enable_if<is_container<std::string>::value, std::string>::type TypeName::get<std::string>();
}



/*
	// Not using this because it only seems to work on container of simple types...
	// TODO: has_begin_end does not seem to work for std::vector<boost::gregorian::date> => figure out why! => does not work for templated sub-types...?
	// TODO: it seems to work in C++11 ?!

	template <typename T>
	struct has_begin_end
	{
		template<typename C> static char(&f(typename std::enable_if<
			std::is_same<decltype(static_cast<typename C::const_iterator(C::*)() const>(&C::begin)),
			typename C::const_iterator(C::*)() const>::value, void>::type*))[1];

		template<typename C> static char(&f(...))[2];

		template<typename C> static char(&g(typename std::enable_if<
			std::is_same<decltype(static_cast<typename C::const_iterator(C::*)() const>(&C::end)),
			typename C::const_iterator(C::*)() const>::value, void>::type*))[1];

		template<typename C> static char(&g(...))[2];

		static bool const beg_value = sizeof(f<T>(0)) == 1;
		static bool const end_value = sizeof(g<T>(0)) == 1;
	};

*/