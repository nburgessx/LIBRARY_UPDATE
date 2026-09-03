/*! @file
    @brief Class declaration of virtual base classes to get data.
*/


#ifndef MDBase_h
#define MDBase_h

#ifdef __GNUG__
#pragma interface
#endif

#include "LAString.h"
#include "LAObjectHolder.h"
#include "LACoreAutoPtr.h"
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
    MDBase(const LAString& name=MDBASE);
    // destructor
    virtual ~MDBase(void);

//  QUERY
    // get name of the object
    const LAString&         getName(void) const {return mName;}
    // get data, LAObjectHolder doest has no ownership of LAObject
	virtual Records_var     get(const LAObjectHolder& objHolder) const=0;

private:
    LAString                mName;  // name
};

#endif
