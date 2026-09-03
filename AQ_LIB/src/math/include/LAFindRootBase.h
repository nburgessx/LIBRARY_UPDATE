#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreTemplateType.h"
#include "LACoreFunctionBase.h"
#include <limits>


#define ROOT_PRECISION	std::numeric_limits<double>::epsilon()*1000		// basic precision to be required 

#define ROOT_ITMAX		500// basic maximum iteration number

// ID for LAFindRootBase
#define FN_FINDROOTBASE     1601
// Function name for LAFindRootBase
#define FN_FINDROOTBASE_STR	"fn_findrootbase"


class LAFunctionBase;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class to find dataInstance option
*/
class LAFindRootOption
{
public:
	// constructor
	LAFindRootOption();
	// destructor
	virtual ~LAFindRootOption();	
	
	double mPrecision;		// precision to be required
	unsigned int mIterMax;	// max iteration number
	bool mException;        // throw exception flag
};

/*! 
    @brief Declaration of abstract base class of dataInstance finding

    Declare abstract base class to find dataInstance for a given function and search regions.
	This class has a pure virtual method "findRoot".

*/
class LAFindRootBase : public LACoreFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	LAFindRootBase();
	// destructor
	virtual ~LAFindRootBase();

//  QUERY
								//======================================
								// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual LACoreFunctionBase*		clone() const =0;
								//======================================
								// Return this class type
    virtual function_t          getType() const;

								// pure virtual method for dataInstance finding.
	virtual void				findRoot(const LAFunctionBase& f,
										const std::vector<std::pair<double,double> >& x,
										DoubleArray& out) const = 0;
								// Return option for finding dataInstance.
	LAFindRootOption&			getOption() {return mOption;};
private:

protected:
	LAFindRootOption mOption;	// option for numerical procedure
};

