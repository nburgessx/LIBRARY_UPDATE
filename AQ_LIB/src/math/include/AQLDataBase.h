/*! @file
    @brief Class declaration of virtual base classes to get data.
*/


#ifndef AQLBase_h
#define AQLBase_h

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLString.h"
#include "AQLObjectHolder.h"
#include "AQLCoreAutoPtr.h"
#include <vector>

// AQLBase object name 
#define    AQLBASE "Base"

/*! 
    @brief Virtual base class to get data.
*/
class AQLBase
{
public:
//  LIFECYCLE
    // constructor
    AQLBase(const AQLString& name=AQLBASE);
    // destructor
    virtual ~AQLBase(void);

//  QUERY
    // get name of the object
    const AQLString&         getName(void) const {return mName;}
    // get data, AQLObjectHolder doest has no ownership of AQLObject
	virtual Records_var     get(const AQLObjectHolder& objHolder) const=0;

private:
    AQLString                mName;  // name
};

#endif
