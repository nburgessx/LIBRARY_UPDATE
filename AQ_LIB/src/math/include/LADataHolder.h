#pragma once

#ifdef __GNUG__
#pragma interface
#endif
#ifdef _MSC_VER
#pragma warning( disable : 4290 )
#endif


#include "LAPriceDataType.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include <unordered_map>

// Forward Declaration(s)
class LAObject;

/*! 
    @brief Class to hold the Data.
    
	It also manages memory of Data(it is not allowded to free of Data object
	in the other class because ownership of Data will be transfered into LADataHolder)
	Also in case of holding the pointer to the Object that the Data object belongs to,
	when the contents of the Data has been updated, Object is notified of it.

	@note be careful abot Data Holder which does not have Data
    (an exception occurs except for isDefine() and constructor)
*/
class LADataHolder
{
public:

	// constructor
    explicit LADataHolder(LAPriceDataType* attr=NULL);
	
	// copy constructor
    LADataHolder(const LADataHolder& attr);
	
	// destructor	
    ~LADataHolder(void);

    // return whether or not that holds the Data object
    /*!
        @retval true hold the Data object
        @retval false not hold the Data object
    */
	bool				isDefined() const {return mpData != NULL;}

	// get data object
    /*!
        @return data object
    */
    const LAPriceDataType&	get(void) const {return *mpData;}

	// get data object
    /*!
        @return data object
    */
    LAPriceDataType&		get(void) {return *mpData;}

	// set (to hold) an Data object
	void				setData(LAPriceDataType* att);

    // assignment operator
    /*!
        @param[in] objectHolder original Data Holder
        @return copied Data Holder
    */
    LADataHolder&		operator=(const LADataHolder& objectHolder) {return copy(objectHolder);}

	// assignment operator
    /*!
        @param[in] objectHolder original Data
        @return copied Data Holder
    */
	LADataHolder&		operator=(const LAPriceDataType& objectHolder)
	{
		setData(objectHolder.clone());
		return *this;
	}

	// relational operator
    /*!
	    make the comparison in the Data ID to retain
        @param[in] objectHolder Data Holder to be compared
        @return if the target is equar to true, false if not
    */
    bool				operator==(const LADataHolder& objectHolder) const
	{ 
		return mpData->cmpType(*(objectHolder.mpData))==0;
	}

	// relational operator
    /*!
	    make the comparison in the Data ID to retain
        @param[in] objectHolder Data Holder to be compared
        @return if the target is large compared to true, false if it is less
    */
    bool				operator<(const LADataHolder& objectHolder) const
	{ 
		return mpData->cmpType(*(objectHolder.mpData))<0;
	}

// Methods used for Proxy of LAPriceDataType
	// return whether or not that the Data object is NULL
    /*!
        @return If NULL, true, false it is not NULL
    */
	bool				isNull(void) const {return mpData->isNull();}

    // return Data ID of hte Data object
    /*!
        @return Data ID
    */
	staticData_t		getType() const {return mpData->getType();}

   // deep copy of the data object
    /*!
        @return the copied object
    */
	LAPriceDataType*	clone() const {return mpData->clone();}

	// return a string representation of the Data object to hold. return "NULL" case of NULL
	/*!
	   the format of the string representation is dependent on the Data for each class.

	   @ return a string representation of Data 
	*/
	LAString			convertToString(void) const {return mpData->convertToString();}

    // based on the string representation of the specified data, it is set to Data. If "NULL" is specified, NULL is set.
    /*!
        string representation to specify depends on the Data
        @param[in] str string
    */
	void				convertFromString(const LAString& str) {mpData->convertFromString(str);}

private:
	// copy the Data Holde by calling clone of the Data that the Data Holder holds
    LADataHolder&		copy(const LADataHolder& attr);


// =======================================================================
// Data Update Methods


// [Warning] expected to be called only from LAPriceDataType
friend class LAPriceDataType;

	// get the (Object) collection that contains this instance
	const LAObject*		getObject(void) const {return mpParent;}

    // get the (Object) collection that contains this instance
	LAObject*			getObject(void) {return mpParent;}

    // notify the Object that the Data is updated, it has to be called from the LAPriceDataType class when Data is changed
	void				update(int type);


// [Warning] expected to be called only from LAObject
friend class LAObject;


    // set collection which holds the Data Holder, and it is cleared by NULL selected
	void					setParent(LAObject* parent);

    LAPriceDataType*		mpData;		// pointer to the Data Holder object that this class has
	LAObject*				mpParent;	// pointer to the Object
};

