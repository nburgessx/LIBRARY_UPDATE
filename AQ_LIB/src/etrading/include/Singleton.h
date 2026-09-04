#pragma once

//
// @Description: This file has couple of template classes that relevant to Singleton and copy behaviour of classes



#include <memory>

#include <boost/type_traits/is_abstract.hpp>

namespace etrading
{

    // This struct provides an interface for the clone method where the type of the desired base pointer can be specified
    //   e.g. class Instrument {}; class ModelInput{};  class InterestRateFuture : public Instrument, public ModelInput
    //   depending on the desired base class interface when copying we can now add to the inheritance list:
    //   : public HasCloneInterface<Instrument> or : public HasCloneInterface<ModelInput> or both (accessible through 1 template function)
    template <typename BASE>
    struct HasCloneInterface
    {
        virtual std::shared_ptr<BASE> clone() const = 0;
    };

    // @Singleton allows for the easy creation of a Singleton interface:
    // 	Derive your class from the templated Singleton, make the CTOR private and add "friend etrading::Singleton<MyClass>"
    // StaticStructureStore has an example
    template <typename T>
    class Singleton
    {
    public:
        static T& getInstance()
        {
            static T instance;
            return instance;
        }
    private:
    };

    // @SingletonLazyInit: Does the same thing as thing as Singleton but is necessary if you need to initialize the singleton with a (dynamic argument) // UNTESTED as of yet
    template <typename T, typename CTORArgument>
    class SingletonLazyInit
    {
        static const CTORArgument s_dummyArg;
    public:
        static T& getInstance( const CTORArgument& ctorArg = s_dummyArg )
        {
            static T instance( ctorArg );
            return instance;
        }
        const CTORArgument& getConstructionArgument() const
        {
            return _arg;  // for display or re-use
        }
    private:
        SingletonLazyInit( const CTORArgument& ctorArg )  : _arg( ctorArg ) {};
        const CTORArgument _arg;  // can be const because initialized only once
    };

    // TODO: move the initialization inside the class once C++11 is available
    template <typename T, typename CTORArgument> const CTORArgument SingletonLazyInit<T, CTORArgument>::s_dummyArg = CTORArgument();


}
