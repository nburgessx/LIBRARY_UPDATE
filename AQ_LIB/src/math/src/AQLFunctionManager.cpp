/*! @file
    @brief Implementation to master functions.

*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "AQLFunctionManager.h"

using namespace std;

// temporary Function Holder used when (Holder) has not been set yet Data corresponding to the name of the Data.
static AQLCoreFunctionHolder NULL_FUNCTION_HOLDER;


/*!
    @brief default constructor
*/
AQLFunctionManager::AQLFunctionManager(void)
{
	setUpDefault();
}

/*!
    @brief copy constructor

    @param[in] m original object
*/
AQLFunctionManager::AQLFunctionManager(const AQLFunctionManager& m)
//: mObjCollection(m.mObjCollection), mFuncCollection(m.mFuncCollection)
 : mFuncCollection(m.mFuncCollection)
{
}

/*!
    @brief destructor
*/
AQLFunctionManager::~AQLFunctionManager(void)
{
	clear();
}


/*!
    @brief get the Function Holder corresponding to the specified Function name

   if you do not set the Function, return the temporary Holder

    @param[in] name Function name

    @return Function Holder
*/
const AQLCoreFunctionHolder& 
AQLFunctionManager::getFunction(const AQLString& name) const
{
/*
    map<AQLString, function_t>::const_iterator it = 
								mFuncCollection.find(name);
	if (it == mFuncCollection.end())
	{
		return NULL_FUNCTION_HOLDER;
	}
	return getFunction(it->second);
*/

    map<AQLString, AQLCoreFunctionHolder>::const_iterator it = 
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
vector<AQLString>	
AQLFunctionManager::getNameArray(void) const
{
	vector<AQLString> ret;
	ret.reserve(mFuncCollection.size());
//	map<AQLString, function_t>::const_iterator it;
	map<AQLString, AQLCoreFunctionHolder>::const_iterator it;
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
const AQLCoreFunctionHolder&	
AQLFunctionManager::setFunction(AQLCoreFunctionBase* eq, const AQLString& name)
{
    AQLCoreFunctionHolder& h = mFuncCollection[name];
	if (h.isDefined())
	{
		if (eq->getType() != h.getType())
		{
			AQLString msg("Duplicate name[");
			msg += AQLString(name) + "]";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
AQLFunctionManager::removeFunction(const AQLString& name)
{
	mFuncCollection.erase(name);
}

/*!
    @brief  remove all of Functions from Function Master

    remove the lists of objects to be registered int the Function Master
*/
void				
AQLFunctionManager::clear()
{
	mFuncCollection.clear();
//	mObjCollection.clear();
}

/*!
    @brief set the default Function class in the Function Master(currently no registration)
*/
void
AQLFunctionManager::setUpDefault(void)
{
}

