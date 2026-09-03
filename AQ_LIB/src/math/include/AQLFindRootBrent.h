#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLFindRootBase.h"


// ID for AQLFindRootBrent
#define FN_FINDROOTBRENT     1602
// Function name for AQLFindRootBrent
#define FN_FINDROOTBRENT_STR	"fn_findrootbrent"


class AQLFunctionBase;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration for dataInstance finding of a 1-D function with Brent method.

   

*/
class AQLFindRootBrent : public AQLFindRootBase
{
public:
//  LIFECYCLE
	// constructor
	AQLFindRootBrent();
	// destructor
	virtual ~AQLFindRootBrent();

//  QUERY
								//======================================
								// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual AQLCoreFunctionBase*     clone() const;
								//======================================
								// Return this class type
    virtual function_t          getType() const;



	// DataInstance finding method
	virtual void findRoot(const AQLFunctionBase& f,
						const std::vector<std::pair<double,double> >& x,
						DoubleArray& out) const;

	AQLFindRootBrent & operator=( const AQLFindRootBrent & ) { return *this; }

private:

protected:

};

