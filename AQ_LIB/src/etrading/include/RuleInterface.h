#pragma once
//
// @Description: This file contains classes that verify whether an instance of a container/variable/object follows a certain rule
// this is done through the "virtual bool verify(const A& a)  const = 0;" method. All these rules can be chained together randomly to
// provide for multiple checks through 1 function call.
// e.g.
//  	std::vector<double> aVectorOfNumberThatNeedsToFollowCertainRules = boost::assign::list_of(1.2)(2.231)(2253124.23)(53245252.3521);
//      AscendingOrderRule<> conditionChecker;
//      bool areTheNumbersInAscendingOrder = conditionChecker.verify(aVectorOfNumberThatNeedsToFollowCertainRules);
//      conditionChecker.addEnforcer(BoundaryRule<>(0.0,std::numeric_limits<double>::max()))
//		bool areTheNumbersInAscendingOrderAndPositive = conditionChecker.verify(aVectorOfNumberThatNeedsToFollowCertainRules);
// 		conditionChecker.addEnforcer(BoundaryRule<>(1000.0,2048.16));
//		bool areTheNumbersInAscendingOrderAndPositiveAndBetweenTwoGivenNumbers = conditionChecker.verify(aVectorOfNumberThatNeedsToFollowCertainRules);
//	For further examples, verify the TestBusinessObject.RuleInterface test


#include <typeinfo>
#include <memory>
#include <iostream>
#include <algorithm>
#include <type_traits>
#include <vector>
#include <utility>
#include <boost/format.hpp>
#include <boost/type_traits.hpp>
#include <boost/date_time.hpp>

#include "TypeName.h"
#include "ETradingException.h"
#include "Singleton.h"
#include "AQLMathCalendar.h"

using etrading::is_container;

namespace etrading
{

    /*
    	For examples of use, verify the TestBusinessObject.RuleInterface test
    */
    template<typename A>
    class RuleInterface : public HasCloneInterface<RuleInterface<A>>
    {
    public:
        RuleInterface()  : _nextEnforcer () { };
        virtual bool verify( const A& a )  const = 0;
        bool addRule( RuleInterface<A>&& ruleEnforcer )
        {
            if( &ruleEnforcer != this )
            {
                _nextEnforcer = ruleEnforcer.clone();
                return true;
            }
            return false;
        }
    protected:
        std::shared_ptr<RuleInterface<A>> _nextEnforcer;
        RuleInterface( const RuleInterface<A>& rhs )
        {
			_nextEnforcer.reset();
            if ( rhs._nextEnforcer != nullptr )
            {
                _nextEnforcer = ( ( rhs._nextEnforcer.get() )->clone() );
            }
        }
        RuleInterface& operator=( const RuleInterface<A>& rhs )
        {
            throw ETradingException( "Assignment operator of RuleInterface should never get called" );
        }  // TODO: C++11 =delete
    };

    template <class C>
    class ContainerInterface : public RuleInterface<C>
    {
    public:
        ContainerInterface() : RuleInterface<C>()
        {
            if( !is_etrading_container<C>::value )
            {
                throw ETradingException(  ( boost::format( "Using ContainerInterface on a type that is not a container : %s" ) % TypeName::get<C>() ).str().c_str()  );
            }
        };
        ContainerInterface( const ContainerInterface<C>& rhs ) : RuleInterface<C>( rhs )
        {}
    };

    template <typename A = std::vector<double> >
    class AscendingOrderRule : public ContainerInterface<A>
    {
    public:
        AscendingOrderRule() {};
        typedef typename A::value_type ElementType;
        virtual bool verify( const A& a )  const
        {
            const bool isNonDecreasing = std::is_sorted( a.cbegin(), a.cend() );
            return  isNonDecreasing && ( ( RuleInterface<A>::_nextEnforcer != nullptr ) ? ( *RuleInterface<A>::_nextEnforcer.get() ).verify( a ) : true ) ;
        };
        AscendingOrderRule( const AscendingOrderRule<A>& rhs ) : ContainerInterface<A>( rhs )
        {}
        std::shared_ptr<RuleInterface<A>> clone() const
        {
            return std::shared_ptr<RuleInterface<A>>( new AscendingOrderRule<A>( *this )  );
        }
    private:
    };

    /*
    	Unfortunately, it is not possible to partially specialize a template member function (virtual bool verify(const A& a)  const)
    	without specializing the enclosing template class. So we need to specialize the whole class - partially specialize a
    	template member function will be a C++ feature for the future.
    */
    template <typename A = std::vector<double>, typename N = double, bool IsContainer = is_etrading_container<A>::value>
    class BoundaryRule : public RuleInterface<A>
    {
        typedef typename A::value_type ElementType;
    public:
        BoundaryRule( const N& min, const N& max ) : min_( min ), max_( max )
        {
            if( !std::is_arithmetic<N>::value )
            {
                throw ETradingException( ( boost::format( "The boundaries in BoundaryRule are not of a numeric type: %s" ) % TypeName::get<N>() ).str() );
            }
            if( max < min )
            {
                throw ETradingException( "max > min in the boundaries of BoundaryRule" );
            }

            // TODO: replace this with a verify for all the comparison operators...
            if( !std::is_arithmetic<ElementType>::value )
            {
                throw ETradingException( ( boost::format( "The values in %s are not a numeric type and cannot be checked by BoundaryRule" ) % TypeName::get<A>() ).str() );
            }
        };

        virtual bool verify( const A& a )  const
        {
            double min = min_;
            double max = max_;
            return ( std::find_if( a.cbegin(), a.cend(), [min, max]( const ElementType & el )
            {
                return ( el < min || el > max );
            }  ) == a.cend() )
            && ( ( RuleInterface<A>::_nextEnforcer != nullptr ) ? ( *RuleInterface<A>::_nextEnforcer.get() ).verify( a ) : true ) ;
        };

        BoundaryRule( const BoundaryRule<A>& rhs )
            : RuleInterface<A>( rhs ), min_( rhs.min_ ), max_( rhs.max_ )
        {} ;

        std::shared_ptr<RuleInterface<A>> clone() const
        {
            return std::shared_ptr<RuleInterface<A>>( new BoundaryRule<A, N>( *this )  );
        };
    private:
        const N min_;
        const N max_;
    };

    // default arguments are not allowed in a partial specialization  (N = A)
    // note that type N is not used anywhere in here... - conversion issues would give unreliable results because loss of precision or signed/unsigned conversions.
    template <typename A, typename N>
    class BoundaryRule<A, N, false> : public RuleInterface<A>
    {
    public:
        BoundaryRule( const A& min, const A& max ) : min_( min ), max_( max )
        {
            if( !std::is_arithmetic<A>::value )
            {
                throw ETradingException( ( boost::format( "BoundaryRule applied on non-numeric type: %s" ) % TypeName::get<A>() ).str() );
            }
            if( max < min )
            {
                throw ETradingException( "max > min in the boundaries of BoundaryRule" );
            }
        };

        virtual bool verify( const A& a )  const
        {
            return ( a >= min_ && a <= max_ ) && ( ( RuleInterface<A>::_nextEnforcer != nullptr ) ? ( *RuleInterface<A>::_nextEnforcer.get() ).verify( a ) : true ) ;
        };

        BoundaryRule( const BoundaryRule<A, N, false>& rhs )
            : RuleInterface<A>( rhs ), min_( rhs.min_ ), max_( rhs.max_ )
        {} ;

        std::shared_ptr<RuleInterface<A>> clone() const
        {
            return std::shared_ptr<RuleInterface<A>>( new BoundaryRule<A, N, false>( *this )  );
        };
    private:
        const A min_;
        const A max_;
    };


    template <typename A = std::vector<boost::gregorian::date>, typename N = double, bool IsContainer = is_etrading_container<A>::value >
    class IsWorkingDayRule
    {
        typedef typename A::value_type ElementType;
    public:
        IsWorkingDayRule( const AQLMathCalendar* ptrCalendar = nullptr ) : ptrCalendar_( ptrCalendar )
        {
            if( ptrCalendar_ == nullptr )
            {
                throw ETradingException( "Cannot initialize IsWorkingDayRule with a NULL ptr for the Calendar!" );
            }
        };

        virtual bool verify( const A& a )  const
        {
            const AQLMathCalendar* ptrCalendar = ptrCalendar_;
            auto non_bus_day = std::find_if( a.cbegin(), a.cend(),
                                             [ptrCalendar]( const A & dateToCheck )
            {
                return !isBusinessDay( dateToCheck, *ptrCalendar );
            } );
            return ( non_bus_day == a.cend() ) && ( ( RuleInterface<A>::_nextEnforcer != nullptr ) ? ( *RuleInterface<A>::_nextEnforcer.get() ).verify( a ) : true ) ;
        };

        IsWorkingDayRule( const IsWorkingDayRule<A>& rhs )
            : RuleInterface<A>( rhs ), ptrCalendar_( rhs.ptrCalendar_ )
        {} ;

        std::shared_ptr<RuleInterface<A>> clone() const
        {
            return std::shared_ptr<RuleInterface<A>>( new IsWorkingDayRule<A, N>( *this )  );
        };

    private:
        const AQLMathCalendar* ptrCalendar_;

    };


    template <typename A, typename N>
    class IsWorkingDayRule<A, N, false> : public RuleInterface<A>
    {
    public:
        IsWorkingDayRule( const AQLMathCalendar* ptrCalendar = nullptr ) : ptrCalendar_( ptrCalendar )
        {
            if( ptrCalendar_ == nullptr )
            {
                throw ETradingException( "Cannot initialize IsWorkingDayRule with a NULL ptr for the Calendar!" );
            }
        };

        virtual bool verify( const A& a )  const
        {
            return ( isBusinessDay( a ) ) && ( ( RuleInterface<A>::_nextEnforcer != nullptr ) ? ( *RuleInterface<A>::_nextEnforcer.get() ).verify( a ) : true ) ;
        };

        IsWorkingDayRule( const IsWorkingDayRule<A, N, false>& rhs )
            : RuleInterface<A>( rhs ), ptrCalendar_( rhs.ptrCalendar_ )
        {} ;

        std::shared_ptr<RuleInterface<A>> clone() const
        {
            return std::shared_ptr<RuleInterface<A>>( new IsWorkingDayRule<A, N, false>( *this )  );
        };
    private:
        const AQLMathCalendar* ptrCalendar_;
    };


}


