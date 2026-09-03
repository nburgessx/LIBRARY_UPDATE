#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LAFindRootBase.h"


// ID for LAFindRootBrent
#define FN_FINDROOTBRENT     1602
// Function name for LAFindRootBrent
#define FN_FINDROOTBRENT_STR	"fn_findrootbrent"


class LAFunctionBase;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration for dataInstance finding of a 1-D function with Brent method.

   

*/
class LAFindRootBrent : public LAFindRootBase
{
public:
//  LIFECYCLE
	// constructor
	LAFindRootBrent();
	// destructor
	virtual ~LAFindRootBrent();

//  QUERY
								//======================================
								// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual LACoreFunctionBase*     clone() const;
								//======================================
								// Return this class type
    virtual function_t          getType() const;



	// DataInstance finding method
	virtual void findRoot(const LAFunctionBase& f,
						const std::vector<std::pair<double,double> >& x,
						DoubleArray& out) const;

	LAFindRootBrent & operator=( const LAFindRootBrent & ) { return *this; }

private:

protected:

};

