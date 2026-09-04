#pragma once
//
// @Description: This is a test program

#include <string>
#include <vector>


namespace etrading
{

    // @HasConstInstance a class allowing for a const member (use for Enums in etrading but not necessarily so).
    // the const member is kept separately so so assignment operator can be defined on derived classes
    template<typename E> class HasConstInstance
    {
    public:
        const E& getRefToInstance() const
        {
            return _enumSetting;
        }
    protected:
        HasConstInstance( const E& e ) : _enumSetting( e ) {};

		// The Intel Compiler requires us to specify an operator=() in order to explicitly specify how
		// assignment should work, since we have a const data member.
		HasConstInstance& operator=( const HasConstInstance<E>& rhs )
		{
			// Explicitly do not modify _enumSetting
			return *this;
		}
        const E _enumSetting;
    };

    class HasName
    {
    public:
        const std::string& getRefToName() const
        {
            return name_;
        }
        void changeName( const std::string& e )
        {
            name_ = e;
        }
    protected:
        HasName( const std::string& e ) : name_( e ) {};
        std::string name_;
    };

    template<typename BaseConst>
    struct HasConstBaseCast
    {
        operator const BaseConst& ()
        {
            return static_cast<const BaseConst&>( this );
        }
    };


}
