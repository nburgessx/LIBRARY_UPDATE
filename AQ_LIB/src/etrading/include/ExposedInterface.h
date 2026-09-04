#pragma once
//
// @Description: 		This class is an example class of how to create base classes for sharing functionality across different concrete objects.
//		As with any inheritance hierarchy these classes can be used for shared interface or common functionality of the derived
//		classes through pointers to this base class (e.g. CreateObjectOnStore in ObjectUtilities).



#include <string>


namespace etrading
{
    struct ExposedInterface
    {
        virtual std::string exposedFunction() const;
    };

}

