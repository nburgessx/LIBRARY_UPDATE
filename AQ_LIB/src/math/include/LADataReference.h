#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAPriceDataType.h"
#include "LACoreAppError.h"


class LAObjectHolder;

#define DATA_REFERENCE   32    // LADataReference Data ID


/*! 
    @brief Reference Data class to represent the data (type reference).
    
*/
class LADataReference : public LAPriceDataType
{
public:
//	LIFECYCLE
    // default constructor
	LADataReference(void);
    // copy constructor
    LADataReference(const LADataReference& attr);
    // constructor
    explicit LADataReference(LAObjectHolder* e); // not hold ownership of the LAObjectHolder
    // destructor
    virtual ~LADataReference(void);

//  QUERY
    // deep copy of the data object
    virtual LAPriceDataType*	clone() const;

    // return (referenced object name) in the string representation
    virtual LAString		convertToString(void) const;

    // get the referenced Object Holder
    const LAObjectHolder&	get() const;

    // get the referenced Object Holder
    LAObjectHolder&			get();

//  OPERATOR
    // set the reference from string representation
    virtual void			convertFromString(const LAString& str);

    // set the referenced Object Holder wchich does not hold owner ship  
	void                    set(LAObjectHolder* e);

protected:
    // the contents of the object to be initialized with those of another object
	virtual LAPriceDataType&	assignment(const LAPriceDataType& a);

    // compare function (with respect to the other object)
	virtual int				compare(const LAPriceDataType& a) const;

    //	set a pointer to the Holder of the Data
    virtual void			setHolder(LADataHolder* holder);

	// inform the Object Data that the Data has been updated
    virtual void			update(void);

private:
	// set member functions that the Object of the Data holds to the referenced Object 
	inline void				setReferencer(void);
	// remove the referenced Object from member functions that the Object of the Data holds
	inline void				delReferencer(void);
	// 
	// with registered in the referenced Object Reference Pool, set the referenced Object to member functions that the Data has
	inline void				setReferencee(void);
	// with registered in the referenced Object Reference Pool, get the Object Holder
	inline LAObjectHolder*	getReferencee(void);


	LAObjectHolder*			mpRef;     // referenced Object Holder
	LAString				mRefName;  // referenced Object name
};

