#pragma once
//
// @File: ExposedInterface.h
// @Description: 		This class is an example class of how to create base classes for sharing functionality across different concrete objects.
//		As with any inheritance hierarchy these classes can be used for shared interface or common functionality of the derived
//		classes through pointers to this base class (e.g. CreateObjectOnStore in ObjectUtilities).
// @Created: 27 Feb 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of AlgoQuantHub.



#include <string>


namespace etrading
{
    struct ExposedInterface
    {
        virtual std::string exposedFunction() const;
    };

}

