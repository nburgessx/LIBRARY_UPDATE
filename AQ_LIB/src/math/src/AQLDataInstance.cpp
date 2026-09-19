/*! @file
    @brief Implementation to manage and hold Object, Data and Function.
*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "AQLDataInstance.h"
#include "AQLPriceDataManager.h"
#include "AQLFunctionManager.h"
#include "AQLDataBasics.h"

#include <string.h>

using namespace std;

/*!
    @brief default constructor
*/
AQLDataInstance::AQLDataInstance(void)
: mpDataMstr(new AQLPriceDataManager()), mpFunctionMstr(new AQLFunctionManager()),
mObjectMstr(NULL), mObjectPool()
{
	mObjectPool.setDataInstance(this);
	mObjectMstr.setDataInstance(this);
}

/*!
    @brief destructor

    Declared here (not inline in the header, even though the body is now empty) because
    std::unique_ptr<AQLPriceDataManager>/<AQLFunctionManager>'s destructor needs those types
    complete at the point it is instantiated, and the header only forward-declares them.
*/
AQLDataInstance::~AQLDataInstance()
{
//	mObjectPool.clear();
}


void
AQLDataInstance::serialize(char*& ptr, unsigned long& length) const
{
	AQLString str;
	EntityConstIter it;
	for (it = mObjectPool.begin(); it != mObjectPool.end(); it++)
	{
		it->second.print(str);
		str += "\n";
	}
    
	try 
	{
		length = str.size() + 1;
		ptr = new char[length];	
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }

	strcpy(ptr, str.getCString());
}

void
AQLDataInstance::load(char* ptr)
{
	mObjectPool.clear();

	char	*token;
	token = strtok(ptr, "\n");
	while (token != NULL)
	{
		AQLString str(token);
		const AQLStringVector& names = str.toToken(',');
		token = strtok(NULL, "\n");
		
		AQLString str2(token);
		const AQLStringVector& values = str2.toToken(',');
		
		if (names.size() == 0 || names[0] != "object_t" || names.size() != values.size())
		{
			throw AQLCoreInvalidData("Invalid data format",__FILE__, __LINE__);
		}
		
        //get object 
		const AQLObjectHolder& h = mObjectMstr.getObject(values[0].getIntValue());
        if (! h.isDefined())
        {
			AQLString msg("Invalid Format : Not exist object_t (");
			msg += values[0] + ")";
			throw AQLCoreInvalidData(msg.getCString(),__FILE__, __LINE__);
        }

        // clone object
        AQLObject* e = h.get().clone();
        AQLString name("");
        for (unsigned int i = 1; i < names.size(); i++)
        {
			if(values[i] == "") continue;

			// get Data
			AQLDataHolder* hp = &(e->getData(names[i]));
			if (!hp->isDefined())
			{
				const AQLDataHolder& att = mpDataMstr->getData(names[i]);
				hp = &(e->add(names[i], att));
			}
			hp->convertFromString(values[i]);
			if (names[i] == "Name")
				name = dynamic_cast<const AQLDataString&>(hp->get()).get();
		}
		if (name == "")
		{
			delete e;
			throw AQLCoreInvalidData("Name does not exsist", __FILE__, __LINE__);			
		}
		mObjectPool.set(name, e);

		token = strtok(NULL, "\n");
	}
	
	getReferencePool().completeDependency();

}

void
AQLDataInstance::update(char* ptr)
{
	char	*token;
	token = strtok(ptr, "\n");
	while (token != NULL)
	{
		AQLString str(token);
		const AQLStringVector& names = str.toToken(',');
		token = strtok(NULL, "\n");
		
		AQLString str2(token);
		const AQLStringVector& values = str2.toToken(',');
		
		if (names.size() == 0 || names.size() != values.size())
		{
			throw AQLCoreInvalidData("Invalid data format",__FILE__, __LINE__);
		}
		
		AQLString name("");
		for (unsigned int i = 0; i < names.size(); i++)
		{
			if (names[i] == "Name")
			{
				name = AQLDataString(values[i]).get();
				break;
			}
		}
		if (name == "")
		{
			throw AQLCoreInvalidData("Name does not exsist", __FILE__, __LINE__);
		}

		AQLObject* e;
		AQLObjectHolder objHolder = mObjectPool.getObject(name);
        if (objHolder.isDefined())
			e = &objHolder.get();
		else
		{
			if (names[0] != "object_t")
			{
				throw AQLCoreInvalidData("Invalid data format",__FILE__, __LINE__);
			}			
			//get object 
			const AQLObjectHolder& h = mObjectMstr.getObject(values[0].getIntValue());
			if (! h.isDefined())
			{
				AQLString msg("Invalid Format : Not exist object_t (");
				msg += values[0] + ")";
				throw AQLCoreInvalidData(msg.getCString(),__FILE__, __LINE__);
			}

			// clone object
			e = h.get().clone();
			mObjectPool.set(name, e);
		}
	    
		for (unsigned int i = 0; i < names.size(); i++)
        {
			if(names[i] == "object_t" || values[i] == "") continue;

			// get Data
			AQLDataHolder* hp = &(e->getData(names[i]));
			if (!hp->isDefined())
			{
				const AQLDataHolder& att = mpDataMstr->getData(names[i]);
				hp = &(e->add(names[i], att));
			}
			hp->convertFromString(values[i]);		
		}
		token = strtok(NULL, "\n");
	}
	
	getReferencePool().completeDependency();
									
}
