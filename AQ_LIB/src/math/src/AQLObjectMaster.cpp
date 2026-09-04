/*! @file
    @brief Implementation to master object classes.
*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "AQLObjectMaster.h"
#include "AQLObject.h"

using namespace std;

/*!
    @brief default constructor

	AQLObject class is registered in this Object via setUpDefault()

    @param[in] dataInstance AQLDataInstance object that this Object Maseter belongs to
*/
AQLObjectMaster::AQLObjectMaster(AQLDataInstance* dataInstance)
:mpDataInstance(dataInstance), mEntityCollection(), mObjectCollection()
{
	setUpDefault();
}

/*!
    @brief copy constructor

    @param[in] a original object
    @param[in] dataInstance AQLDataInstance object that this class belongs to
*/
AQLObjectMaster::AQLObjectMaster(const AQLObjectMaster& a, AQLDataInstance* dataInstance)
:mpDataInstance(dataInstance)
{
	copy(a);
}

/*!
    @brief destructor
*/
AQLObjectMaster::~AQLObjectMaster(void)
{
}

/*!
    @brief get Object Holder corresponding to the name of the specified Object

    if you do not set the Object, return the temporary Holder

    @param[in] name Object name

    @return Object Holder
*/
const AQLObjectHolder& 
AQLObjectMaster::getObject(const AQLString& name) const
{
	map<AQLString, AQLObjectHolder>::const_iterator it = mEntityCollection.find(name);
	if (it == mEntityCollection.end())
	{
		return NULL_OBJECT_HOLDER;
	}
	return it->second;
}

/*!
    @brief get Object Holder corresponding to the id of the specified Object

    if you do not set the Object, return the temporary Holder

    @param[in] id Object ID

    @return Object Holder
*/
const AQLObjectHolder& 
AQLObjectMaster::getObject(object_t id) const
{
	map<int, AQLObjectHolder>::const_iterator it = mObjectCollection.find(id);
	if (it == mObjectCollection.end())
	{
		return NULL_OBJECT_HOLDER;
	}
	return it->second;
}

/*!
    @brief get registered Object ID Vector

    @return registered Object ID Vector
*/
vector<object_t>		
AQLObjectMaster::getTypeArray(void) const
{
	vector<object_t> ret;
	ret.reserve(mObjectCollection.size());
	map<object_t, AQLObjectHolder>::const_iterator it;
	for (it = mObjectCollection.begin(); it != mObjectCollection.end(); ++it)
	{
		ret.push_back(it->first);
	}
	return ret;
}

/*!
    @brief get registered Object Name Vector

    @return registered Object Name Vector
*/
vector<AQLString>	
AQLObjectMaster::getNameArray(void) const
{
	vector<AQLString> ret;
	ret.reserve(mEntityCollection.size());
	map<AQLString, AQLObjectHolder>::const_iterator it;
	for (it = mEntityCollection.begin(); it != mEntityCollection.end(); ++it)
	{
		ret.push_back(it->first);
	}
	return ret;
}

/*!
    @brief  set Object name and Object object

	Object Master holds the ownership of this Object object (memory release is done automatically)

    @param[in] name Object name
    @param[in] e Object object

    @return Object Holder 
*/
const AQLObjectHolder&	
AQLObjectMaster::setEntity(const AQLString& name, AQLObject* e)
{
	AQLObjectHolder& h = mEntityCollection[name];
	if (h.isDefined())
	{
		AQLString msg("Duplicate name [");
		msg += name + "]";
		throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}
	h.setEntity(e, true);  // ownership moves to Object Holder by delete flag=true
	h.setDataInstance(mpDataInstance);
	return h;
}

/*!
    @brief  register Object

    Object Master holds the ownership of this Object object (memory release is done automatically)

    @param[in] att Object object

    @return Object Holder to register 
*/
const AQLObjectHolder&	
AQLObjectMaster::setEntity(AQLObject* att)
{
	AQLObjectHolder& h = mObjectCollection[att->getType()];
	if (h.isDefined())
	{
		AQLString msg("Duplicate object_t[");
		msg += AQLString(att->getType()) + "]";
		throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}
	h.setEntity(att, true);
	h.setDataInstance(mpDataInstance);
	return h;
}

/*!
    @brief  delete Object specified from the Object Master

    @param[in] name Object name to be deleted
*/
void
AQLObjectMaster::removeEntity(const AQLString& name)
{
	mEntityCollection.erase(name);
}

/*!
    @brief  delete Object with the specified Object ID from the Object Master

    @param[in] id Object ID to be deleted
*/
void
AQLObjectMaster::removeEntity(object_t id)
{
	mObjectCollection.erase(id);
}

/*!
    @brief  delete all Entities from Object Master

	delete all registered Entities from Object Master
*/
void				
AQLObjectMaster::clear()
{
	mObjectCollection.clear();
	mEntityCollection.clear();
}

/*!
    @brief set the AQLDataInstance object which belongs to the Object Master

    @param[in] dataInstance the AQLDataInstance object to be set
*/
void
AQLObjectMaster::setDataInstance(AQLDataInstance* dataInstance)
{
	mpDataInstance = dataInstance;
	setDataInstanceToEntity();
}

//////////////////// PRIVATE ////////////////////////////////
/*!
    @brief set the AQLDataInstance object to all Entities registered the Object Master
*/
void
AQLObjectMaster::setDataInstanceToEntity()
{
	map<object_t, AQLObjectHolder>::iterator it;
	for (it = mObjectCollection.begin(); 
			it != mObjectCollection.end(); ++it)
	{
		it->second.setDataInstance(mpDataInstance);
	}
	map<AQLString, AQLObjectHolder>::iterator it2;
	for (it2 = mEntityCollection.begin(); 
			it2 != mEntityCollection.end(); ++it2)
	{
		it2->second.setDataInstance(mpDataInstance);
	}
}

/*!
    @brief shallow copy of the specified Object Master

    @param[in] e  original Object Master object

    @return the copied Object Master object
*/
AQLObjectMaster&
AQLObjectMaster::copy(const AQLObjectMaster& e)
{
	mObjectCollection = e.mObjectCollection;
	mEntityCollection = e.mEntityCollection;
	setDataInstanceToEntity();
	return *this;
}

/*!
    @brief set AQLObject into the Object Master
*/
void
AQLObjectMaster::setUpDefault(void)
{
	setEntity(new AQLObject());
}
