#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAString.h"

#define staticData_t int			// type of Data ID

#define DATA_BASE   0           // LAPriceDataType Data ID
								// it must be unique of the library

#define NULL_STR    "NULL"      // NULL string representing NUU value

class LADataHolder;
class LAObject;

/*! 
    @brief Base class to represent Data
    
    All Data classes must be inherited from this Class.
	
    Data basis function is \n
    1. to represent NULL value \n
    2. to convert from string representation \n
    3. to be able to self-replicate by the polymorphism \n
    Data class needs to be implemented as below
	clone() , convertToString() , convertFromString() , assignment() , compare()

    NULL set is also possible so that this class must control NULL value on setNull(false)
	when data is set.
						
*/
class LAPriceDataType
{
public:
//  LIFECYCLE
	// constructor
	//  Attribuute ID=DATA_BASE, set the status NULL
	LAPriceDataType(const staticData_t attr=DATA_BASE);
	// copy constructor
	LAPriceDataType(const LAPriceDataType& attr);
	// destructor	
	virtual ~LAPriceDataType();
     
//  QUERY
	// check Null
    /*!
        @return true if the NULL, false if it is not NULL
    */
	virtual bool				isNull(void) const {return mIsNull;}

    // return Data ID
    /*!
        @return Data ID
    */
	staticData_t			getType() const {return mAttributeType;}

	// make a comparison of the Data ID. If the same ID, it returns 0.
    /*!
        @param[in] objectHolder Data to be compared
        @return (Data ID of the object) Å| (Data ID to be compared)
    */
    int					cmpType(const LAPriceDataType& objectHolder) const 
							{return mAttributeType - objectHolder.mAttributeType;} 

    // deep copy of the data object
    /*!
        inherited class is necessary to implement this method
    */
	virtual LAPriceDataType*  clone() const =0;                         

    // return string representation
    /*!
       inherited class is necessary to implement this method. In case of Null, return Null
    */
    virtual LAString	convertToString(void) const =0;

//  OPERATION
    // convert Data from the string representation
    /*! 
        inherited class needs to implement this method, and NULL is set when selected "NULL"
        @param[in] str string representation
    */
    virtual void		convertFromString(const LAString& str) =0;
 
// OPERATOR
	// assignment operator
    /*!
	    assignment achieved by the method
        @param[in] original Data
        @return the copied Data Holder
    */
    LAPriceDataType&		operator=(const LAPriceDataType& a) {return assignment(a);}

	// relational operator
    /*!
        method implemented in compare
        @param[in] a Data to be compared
        @return true or false
    */
	bool				operator==(const LAPriceDataType& a) const {return compare(a)==0;}
	// relational operator
    /*!
        method implemented in compare
        @param[in] a Data to be compared
        @retval true the compared object is large
		@retval false the compared object is small
    */
	bool				operator<(const LAPriceDataType& a) const {return compare(a)<0;}

protected:
	// the contents of the object to be initialized with those of another object
    /*!
        inherited class is necessary to implement this method
        @param[in] a the Data
    */
	virtual LAPriceDataType&	assignment(const LAPriceDataType& a) =0;

	// compare the contents against the other objecet
    /*!
        inherited class is necessary to implement this method
        @param[in] a Data to be compared
    */
	virtual int			compare(const LAPriceDataType& a) const = 0;

	// delete "(double quotes) from string in the first and end, and store the data
	bool				strToData(const LAString& str, LAString& data) const;
    
	// true if set NULL, and false if remove
    /*!
        @param[in] b true or false
    */
	void				setNull(bool b=true) {mIsNull = b;}	


friend class LADataHolder;
	// inform the Object Data that the Data has been updated, the selected NO_AUTO_UPDATE is neglected when set the argument by update type
	void				update(int type);
	// inform the Object Data that the Data has been updated
	virtual void		update(void);

	// get the pointer to the Object object that holds the Data, in case of not Object, return NULL
	LAObject*			getObject(void);
	// get the pointer to the Object object that holds the Data, in case of not Object, return NULL
	const LAObject*		getObject(void) const;

public:
	//	set a pointer to the Holder of the Data
	virtual void		setHolder(LADataHolder* holder);

private:

	LADataHolder*		mpHolder;			// pointer to the Data Holder

	staticData_t				mAttributeType;		// Data ID
	bool				mIsNull;			// check NULL or not
};

