#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreTemplateType.h"
#include "LACoreFunctionHolder.h"
#include "LACoreAppError.h"
#include "LAString.h"

#include <map>
#include <vector>

typedef std::vector<function_t>		FunctionTVector; // Function ID vector

/*! 
    @brief Class to master Function used in the application.
*/
class LAFunctionManager 
{
public:
//  LIFECYCLE
    // default constructor
	LAFunctionManager(void);
    // copy constructor
	LAFunctionManager(const LAFunctionManager& m);
    // destructor
	~LAFunctionManager(void);

//  QUERY
    // get the Function Holder corresponding to the specified Function name
    const LACoreFunctionHolder& getFunction(const LAString& name) const;


    // return the array of name of the registered FunctionID
	LAStringVector			getNameArray(void) const;

//  OPERATION
    // set Function object
	const LACoreFunctionHolder&	setFunction(LACoreFunctionBase* eq, const LAString& name);

	// remove the specified Function from Function Master
	void				removeFunction(const LAString& name);

    // remove all of Functions from Function Master
    void				clear();

private:
	// set the default Function class in the Function Master(currently no registration)
	void				setUpDefault(void);

	std::map<LAString, LACoreFunctionHolder> mFuncCollection;	// map of Function(Holder) to be administrated from this Function master (Function Holder name and Function Holder)


};
