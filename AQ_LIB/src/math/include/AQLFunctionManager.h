#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreTemplateType.h"
#include "AQLCoreFunctionHolder.h"
#include "AQLCoreAppError.h"
#include "AQLString.h"

#include <map>
#include <vector>

typedef std::vector<function_t>		FunctionTVector; // Function ID vector

/*! 
    @brief Class to master Function used in the application.
*/
class AQLFunctionManager 
{
public:
    // default constructor
	AQLFunctionManager(void);
    // copy constructor
	AQLFunctionManager(const AQLFunctionManager& m);
    // destructor
	~AQLFunctionManager(void);

    // get the Function Holder corresponding to the specified Function name
    const AQLCoreFunctionHolder& getFunction(const AQLString& name) const;


    // return the array of name of the registered FunctionID
	AQLStringVector			getNameArray(void) const;

    // set Function object
	const AQLCoreFunctionHolder&	setFunction(AQLCoreFunctionBase* eq, const AQLString& name);

	// remove the specified Function from Function Master
	void				removeFunction(const AQLString& name);

    // remove all of Functions from Function Master
    void				clear();

private:
	// set the default Function class in the Function Master(currently no registration)
	void				setUpDefault(void);

	std::map<AQLString, AQLCoreFunctionHolder> mFuncCollection;	// map of Function(Holder) to be administrated from this Function master (Function Holder name and Function Holder)


};
