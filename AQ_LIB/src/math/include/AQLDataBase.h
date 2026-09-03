/*! @file
    @brief Class declaration of virtual base classes to get data.
*/


#ifndef MDBase_h
#define MDBase_h

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLString.h"
#include "AQLObjectHolder.h"
#include "AQLCoreAutoPtr.h"
#include <vector>

// MDBase object name 
#define    MDBASE "Base"

/*! 
    @brief Virtual base class to get data.
*/
class MDBase
{
public:
//  LIFECYCLE
    // constructor
    MDBase(const AQLString& name=MDBASE);
    // destructor
    virtual ~MDBase(void);

//  QUERY
    // get name of the object
    const AQLString&         getName(void) const {return mName;}
    // get data, AQLObjectHolder doest has no ownership of AQLObject
	virtual Records_var     get(const AQLObjectHolder& objHolder) const=0;

private:
    AQLString                mName;  // name
};

#endif
