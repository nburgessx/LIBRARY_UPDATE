/*! @file
    @brief Implementation to master functions.

*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "LAFunctionManager.h"

using namespace std;

// temporary Function Holder used when (Holder) has not been set yet Data corresponding to the name of the Data.
static LACoreFunctionHolder NULL_FUNCTION_HOLDER;


/*!
    @brief default constructor
*/
LAFunctionManager::LAFunctionManager(void)
{
	setUpDefault();
}

/*!
    @brief copy constructor

    @param[in] m original object
*/
LAFunctionManager::LAFunctionManager(const LAFunctionManager& m)
//: mObjCollection(m.mObjCollection), mFuncCollection(m.mFuncCollection)
 : mFuncCollection(m.mFuncCollection)
{
}

/*!
    @brief destructor
*/
LAFunctionManager::~LAFunctionManager(void)
{
	clear();
}


/*!
    @brief get the Function Holder corresponding to the specified Function name

   if you do not set the Function, return the temporary Holder

    @param[in] name Function name

    @return Function Holder
*/
const LACoreFunctionHolder& 
LAFunctionManager::getFunction(const LAString& name) const
{
/*
    map<LAString, function_t>::const_iterator it = 
								mFuncCollection.find(name);
	if (it == mFuncCollection.end())
	{
		return NULL_FUNCTION_HOLDER;
	}
	return getFunction(it->second);
*/

    map<LAString, LACoreFunctionHolder>::const_iterator it = 
								mFuncCollection.find(name);
	if (it == mFuncCollection.end())
	{
		return NULL_FUNCTION_HOLDER;
	}
	return it->second;
}

/*!
    @brief get the array of name of the registered FunctionID

    @return the array of name of the registered FunctionID
*/
vector<LAString>	
LAFunctionManager::getNameArray(void) const
{
	vector<LAString> ret;
	ret.reserve(mFuncCollection.size());
//	map<LAString, function_t>::const_iterator it;
	map<LAString, LACoreFunctionHolder>::const_iterator it;
	for (it = mFuncCollection.begin(); it != mFuncCollection.end(); ++it)
	{
		ret.push_back(it->first);
	}
	return ret;
}


/*!
    @brief  set Function object and Function name

	Function Master holds the ownership of this Function object (memory release is done automatically)

	@param[in] eq Function object 
    @param[in] name Function name

    @return Function Holder
*/
const LACoreFunctionHolder&	
LAFunctionManager::setFunction(LACoreFunctionBase* eq, const LAString& name)
{
    LACoreFunctionHolder& h = mFuncCollection[name];
	if (h.isDefined())
	{
		if (eq->getType() != h.getType())
		{
			LAString msg("Duplicate name[");
			msg += LAString(name) + "]";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		else
		{
			delete eq;
			return h;
		}
	}
	else
	{
		h.set(eq, true);
		return h;
	}
}

/*!
    @brief remove the specified Function from Function Master

    @param[in] name function name to be removed
*/
void				
LAFunctionManager::removeFunction(const LAString& name)
{
	mFuncCollection.erase(name);
}

/*!
    @brief  remove all of Functions from Function Master

    remove the lists of objects to be registered int the Function Master
*/
void				
LAFunctionManager::clear()
{
	mFuncCollection.clear();
//	mObjCollection.clear();
}

/*!
    @brief set the default Function class in the Function Master(currently no registration)
*/
void
LAFunctionManager::setUpDefault(void)
{
}

