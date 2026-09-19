#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLPriceDataType.h"
#include "AQLCoreAppError.h"


class AQLObjectHolder;

#define DATA_REFERENCE   32    // AQLDataReference Data ID


/*! 
    @brief Reference Data class to represent the data (type reference).
    
*/
class AQLDataReference : public AQLPriceDataType
{
public:
    // default constructor
	AQLDataReference(void);
    // copy constructor
    AQLDataReference(const AQLDataReference& attr);
    // constructor
    explicit AQLDataReference(AQLObjectHolder* e); // not hold ownership of the AQLObjectHolder
    // destructor
    virtual ~AQLDataReference(void);

    // deep copy of the data object
    virtual AQLPriceDataType*	clone() const;

    // return (referenced object name) in the string representation
    virtual AQLString		convertToString(void) const;

    // get the referenced Object Holder
    const AQLObjectHolder&	get() const;

    // get the referenced Object Holder
    AQLObjectHolder&			get();

    // set the reference from string representation
    virtual void			convertFromString(const AQLString& str);

    // set the referenced Object Holder wchich does not hold owner ship  
	void                    set(AQLObjectHolder* e);

protected:
    // the contents of the object to be initialized with those of another object
	virtual AQLPriceDataType&	assignment(const AQLPriceDataType& a);

    // compare function (with respect to the other object)
	virtual int				compare(const AQLPriceDataType& a) const;

    //	set a pointer to the Holder of the Data
    virtual void			setHolder(AQLDataHolder* holder);

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
	inline AQLObjectHolder*	getReferencee(void);


	AQLObjectHolder*			mpRef;     // referenced Object Holder
	AQLString				mRefName;  // referenced Object name
};

