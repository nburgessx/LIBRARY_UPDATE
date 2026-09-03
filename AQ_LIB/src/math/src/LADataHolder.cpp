/*! @file
    @brief Implementation of the class to hold dataValues.
    
*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "LADataHolder.h"
#include "LAObject.h"

/*!
    @brief constructor

	It is not allowded to free of Data object in the other class 
	because ownership of Data will be transfered into LADataHolder.
	Also, attr must be object created from new(malloc is impossible)

    set setHolder() of the Data object to NULL

    @param[in] attr original object

*/
LADataHolder::LADataHolder(LAPriceDataType* attr)
: mpData(attr), mpParent(NULL)
{
    if (mpData != NULL)
        mpData->setHolder(NULL);
}

/*!
    @brief copy constructor

    set after convert a double to a string

    @param[in] attr original object
*/
LADataHolder::LADataHolder(const LADataHolder& attr)
: mpData(NULL), mpParent(NULL)
{
    // Data
    if (attr.mpData != NULL)
    {
        mpData = attr.mpData->clone();
        mpData->setHolder(NULL);
    }
}

/*!
    @brief destructor

    release memory area of Data object
*/
LADataHolder::~LADataHolder(void)
{
    delete mpData;
}

/*!
    @brief set (to hold) an Data object

    ownership of Data object is transfed into the Data Holder,
	and NULL can no be selected(if selected, then an exception occurs)

    @param[in] attr Data 
*/
void                
LADataHolder::setData(LAPriceDataType* attr)
{
    // check argument
    if (attr == NULL ) 
    {   
        throw LACoreInvalidData("NULL Pointer set to LADataHolder", __FILE__, __LINE__);
    }
    // first clear
    if (attr != mpData) 
    {
        delete mpData;
        // and set
        mpData = attr;
        if (mpParent != NULL)
        {
            // set this pointer to setHolder() of Data object
            mpData->setHolder(this);
            // NO_AUTO_UPDATE is reflected by calling update() of AttributeAttributeupdate()
            mpData->update();
        }
        else
        {
            mpData->setHolder(NULL);
        }
    }
}

/*!
    @brief copy Data Holder
    
    @param[in] attr original Data Holder
*/
LADataHolder&
LADataHolder::copy(const LADataHolder& attr)
{
    if (this != &attr)
    {
        if (! attr.isDefined())
        {
            throw LACoreInvalidData("Can not copy the undefined Data", 
                                    __FILE__, __LINE__);            
        }
        // attr must have value
        delete mpData;
        mpData = attr.mpData->clone();
        // notify collection
        if (mpParent != NULL)
        {
            mpData->setHolder(this);
           // NO_AUTO_UPDATE is reflected by calling update() of AttributeAttributeupdate()
            mpData->update(); 
        }
        else
        {
            mpData->setHolder(NULL);
        }
    }
    return *this;
}

/*!
    @brief notify the Object that the Data is updated
    
    it has to be called from the LAPriceDataType class when Data is changed

    @param[in] type update type(TYPE_NORMAL or TYPE_BROADCAST)
*/
void                
LADataHolder::update(int type)
{
    if (mpParent != NULL)
        mpParent->update(type);
}

/*!
set collection which holds the Data Holder, and it is cleared by NULL selected
    @brief set collection which holds the Data Holder
    
    it must be called from LAObject, and it is cleared by NULL selected

    @param[in] parent collection(Object) that this Data Holder has
*/
void            
LADataHolder::setParent(LAObject* parent)
{
    // if NULL is set as parent,
    // then set Data Holder NULL(release Holder set)
    if ((mpParent = parent) == NULL)
    {
        if (mpData != NULL)
            mpData->setHolder(NULL);
    }
    else
    {
        // if Object is set as parent,
        // not need to issue Update from Data
        if (mpData != NULL)
            mpData->setHolder(this);
    }
}
