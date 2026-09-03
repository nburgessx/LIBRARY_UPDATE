/*! @file
    @brief Implementation to manage and hold Object, Data and Function.

*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "LADataInstance.h"
#include "LAPriceDataManager.h"
#include "LAFunctionManager.h"
#include "LADataBasics.h"

#include <string.h>

using namespace std;

/*!
    @brief default constructor
*/
LADataInstance::LADataInstance(void)
: mpDataMstr(NULL),  mpFunctionMstr(NULL), mpRefCount(NULL), 
mObjectMstr(NULL), mObjectPool()
{
	mpDataMstr = new LAPriceDataManager();
	mpFunctionMstr = new LAFunctionManager();
	mpRefCount = new int;
	*mpRefCount = 1;  // initial value of the reference counter is 1

	mObjectPool.setDataInstance(this);
	mObjectMstr.setDataInstance(this);
}

/*!
    @brief copy constructor

    @param[in] dataInstance original object
*/
LADataInstance::LADataInstance(const LADataInstance& dataInstance)
: mpDataMstr(NULL),  mpFunctionMstr(NULL), mpRefCount(NULL), 
mObjectMstr(NULL), mObjectPool()
{
	mObjectPool.setDataInstance(this);
	mObjectMstr.setDataInstance(this);
	try {
		copy(dataInstance);
	}
	catch (LACoreError&)
	{
		delMstrs();
		mObjectPool.clear();
		throw;
	}
}

/*!
    @brief destructor
*/
LADataInstance::~LADataInstance()
{
	delMstrs();
//	mObjectPool.clear();
}


void
LADataInstance::serialize(char*& ptr, unsigned long& length) const
{
	LAString str;
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
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }

	strcpy(ptr, str.getCString());
}

void
LADataInstance::load(char* ptr)
{
	mObjectPool.clear();

	char	*token;
	token = strtok(ptr, "\n");
	while (token != NULL)
	{
		LAString str(token);
		const LAStringVector& names = str.toToken(',');
		token = strtok(NULL, "\n");
		
		LAString str2(token);
		const LAStringVector& values = str2.toToken(',');
		
		if (names.size() == 0 || names[0] != "object_t" || names.size() != values.size())
		{
			throw LACoreInvalidData("Invalid data format",__FILE__, __LINE__);
		}
		
        //get object 
		const LAObjectHolder& h = mObjectMstr.getObject(values[0].getIntValue());
        if (! h.isDefined())
        {
			LAString msg("Invalid Format : Not exist object_t (");
			msg += values[0] + ")";
			throw LACoreInvalidData(msg.getCString(),__FILE__, __LINE__);
        }

        // clone object
        LAObject* e = h.get().clone();
        LAString name("");
        for (unsigned int i = 1; i < names.size(); i++)
        {
			if(values[i] == "") continue;

			// get Data
			LADataHolder* hp = &(e->getData(names[i]));
			if (!hp->isDefined())
			{
				const LADataHolder& att = mpDataMstr->getData(names[i]);
				hp = &(e->add(names[i], att));
			}
			hp->convertFromString(values[i]);
			if (names[i] == "Name")
				name = dynamic_cast<const LADataString&>(hp->get()).get();
		}
		if (name == "")
		{
			delete e;
			throw LACoreInvalidData("Name does not exsist", __FILE__, __LINE__);			
		}
		mObjectPool.set(name, e);

		token = strtok(NULL, "\n");
	}
	
	getReferencePool().completeDependency();

}

void
LADataInstance::update(char* ptr)
{
	char	*token;
	token = strtok(ptr, "\n");
	while (token != NULL)
	{
		LAString str(token);
		const LAStringVector& names = str.toToken(',');
		token = strtok(NULL, "\n");
		
		LAString str2(token);
		const LAStringVector& values = str2.toToken(',');
		
		if (names.size() == 0 || names.size() != values.size())
		{
			throw LACoreInvalidData("Invalid data format",__FILE__, __LINE__);
		}
		
		LAString name("");
		for (unsigned int i = 0; i < names.size(); i++)
		{
			if (names[i] == "Name")
			{
				name = LADataString(values[i]).get();
				break;
			}
		}
		if (name == "")
		{
			throw LACoreInvalidData("Name does not exsist", __FILE__, __LINE__);
		}

		LAObject* e;
		LAObjectHolder objHolder = mObjectPool.getObject(name);
        if (objHolder.isDefined())
			e = &objHolder.get();
		else
		{
			if (names[0] != "object_t")
			{
				throw LACoreInvalidData("Invalid data format",__FILE__, __LINE__);
			}			
			//get object 
			const LAObjectHolder& h = mObjectMstr.getObject(values[0].getIntValue());
			if (! h.isDefined())
			{
				LAString msg("Invalid Format : Not exist object_t (");
				msg += values[0] + ")";
				throw LACoreInvalidData(msg.getCString(),__FILE__, __LINE__);
			}

			// clone object
			e = h.get().clone();
			mObjectPool.set(name, e);
		}
	    
		for (unsigned int i = 0; i < names.size(); i++)
        {
			if(names[i] == "object_t" || values[i] == "") continue;

			// get Data
			LADataHolder* hp = &(e->getData(names[i]));
			if (!hp->isDefined())
			{
				const LADataHolder& att = mpDataMstr->getData(names[i]);
				hp = &(e->add(names[i], att));
			}
			hp->convertFromString(values[i]);		
		}
		token = strtok(NULL, "\n");
	}
	
	getReferencePool().completeDependency();
									
}


/*!
    @brief shallow copy of the LADataInstance object

    @param[in] dataInstance original LADataInstance object

    @return the copied LADataInstance object
*/
LADataInstance&
LADataInstance::copy(const LADataInstance& dataInstance)
{
	if (mpRefCount != NULL)
	{
		delMstrs();
	}
	mObjectPool.clear();

	mpRefCount = dataInstance.mpRefCount;
	++(*mpRefCount);  // incrememnt of reference counter since shallow copy is done as below

	mpDataMstr = dataInstance.mpDataMstr;  // shallow copy
	mpFunctionMstr = dataInstance.mpFunctionMstr; // shallow copy
	mObjectMstr = dataInstance.mObjectMstr;

	mObjectPool = dataInstance.mObjectPool;
	return *this;
}

/*!
    @brief release the memory reserved for the Data Master, Function Master

	nothing is done if other LADataInstance objects using Data Master, Function Master exist
	(in case reference counter is larger than 1)
*/
void
LADataInstance::delMstrs(void)
{
	if (--(*mpRefCount) == 0)
	{
		delete mpDataMstr;
		delete mpFunctionMstr;
		delete mpRefCount;
	}
}
